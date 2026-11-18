package io.github.dedecidao.dd64;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Start screen: pick the ROM once (Storage Access Framework, no storage
 * permission), then play.
 *
 * The ROM is read through the document picker, converted to .z64 byte order,
 * checked against the SHA1 of Destruction Derby 64 (USA) and kept in the app's
 * private files (files/rom/dd64.z64). It never leaves the device.
 */
public class LauncherActivity extends Activity {
    static final String ROM_SHA1 = "a2c0799e13566d1b723299592cfcac9387f25fa7";
    static final int ROM_SIZE = 0x1000000;
    private static final int REQ_PICK_ROM = 1;

    private final ExecutorService mIo = Executors.newSingleThreadExecutor();
    private final Handler mMain = new Handler(Looper.getMainLooper());
    private TextView mStatus;
    private Button mPlay, mPick;

    static File romFile(Activity a) {
        return new File(new File(a.getFilesDir(), "rom"), "dd64.z64");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_launcher);
        mStatus = findViewById(R.id.status);
        mPlay = findViewById(R.id.play);
        mPick = findViewById(R.id.pick_rom);

        mPick.setOnClickListener(v -> pickRom());
        mPlay.setOnClickListener(v -> play());

        CheckBox touch = findViewById(R.id.touch_enabled);
        touch.setChecked(Settings.touchEnabled(this));
        touch.setOnCheckedChangeListener((b, on) -> Settings.prefs(this).edit().putBoolean(Settings.TOUCH, on).apply());

        CheckBox hide = findViewById(R.id.touch_hide_with_gamepad);
        hide.setChecked(Settings.hideWithGamepad(this));
        hide.setOnCheckedChangeListener((b, on) ->
                Settings.prefs(this).edit().putBoolean(Settings.HIDE_WITH_GAMEPAD, on).apply());

        bindSeek(R.id.touch_opacity, Settings.TOUCH_OPACITY, 10, 100, 60);
        bindSeek(R.id.touch_scale, Settings.TOUCH_SCALE, 60, 160, 100);

        TextView folder = findViewById(R.id.data_folder);
        File ext = getExternalFilesDir(null);
        folder.setText(getString(R.string.data_folder, ext != null ? ext.getAbsolutePath() : "?"));

        refresh();
    }

    private void bindSeek(int id, String key, int min, int max, int def) {
        SeekBar s = findViewById(id);
        s.setMax(max - min);
        s.setProgress(Settings.prefs(this).getInt(key, def) - min);
        s.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar bar, int p, boolean user) {
                Settings.prefs(LauncherActivity.this).edit().putInt(key, p + min).apply();
            }
            @Override public void onStartTrackingTouch(SeekBar bar) {}
            @Override public void onStopTrackingTouch(SeekBar bar) {}
        });
    }

    private void refresh() {
        boolean have = romFile(this).isFile() && romFile(this).length() == ROM_SIZE;
        mPlay.setEnabled(have);
        mStatus.setText(have ? R.string.rom_ready : R.string.rom_missing);
        mPick.setText(have ? R.string.pick_other_rom : R.string.pick_rom);
    }

    private void pickRom() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        i.addCategory(Intent.CATEGORY_OPENABLE);
        i.setType("*/*");
        startActivityForResult(i, REQ_PICK_ROM);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQ_PICK_ROM && resultCode == RESULT_OK && data != null && data.getData() != null) {
            importRom(data.getData());
        }
    }

    private void importRom(Uri uri) {
        mPick.setEnabled(false);
        mPlay.setEnabled(false);
        mStatus.setText(R.string.rom_checking);
        mIo.execute(() -> {
            String error = null;
            try {
                byte[] rom = readAll(uri);
                error = normalizeAndCheck(rom);
                if (error == null) {
                    File out = romFile(this);
                    File dir = out.getParentFile();
                    if (dir != null && !dir.isDirectory() && !dir.mkdirs()) {
                        throw new IOException("mkdir " + dir);
                    }
                    File tmp = new File(dir, "dd64.z64.tmp");
                    try (FileOutputStream f = new FileOutputStream(tmp)) {
                        f.write(rom);
                    }
                    if (!tmp.renameTo(out)) {
                        throw new IOException("rename " + tmp);
                    }
                }
            } catch (IOException | OutOfMemoryError e) {
                error = getString(R.string.rom_read_error, String.valueOf(e.getMessage()));
            }
            final String err = error;
            mMain.post(() -> {
                mPick.setEnabled(true);
                refresh();
                if (err != null) {
                    mStatus.setText(err);
                }
            });
        });
    }

    private byte[] readAll(Uri uri) throws IOException {
        byte[] buf = new byte[ROM_SIZE];
        int n = 0;
        try (InputStream in = getContentResolver().openInputStream(uri)) {
            if (in == null) throw new IOException("no stream");
            int r;
            while (n < ROM_SIZE && (r = in.read(buf, n, ROM_SIZE - n)) > 0) {
                n += r;
            }
            if (n != ROM_SIZE || in.read() != -1) {
                throw new IOException(getString(R.string.rom_wrong_size));
            }
        }
        return buf;
    }

    /** Converts .v64/.n64 to .z64 in place; null if it is the right ROM, else the reason. */
    private String normalizeAndCheck(byte[] d) {
        int b0 = d[0] & 0xFF, b1 = d[1] & 0xFF;
        if (b0 == 0x37 && b1 == 0x80) { // .v64: halfwords swapped
            for (int i = 0; i < d.length; i += 2) {
                byte t = d[i]; d[i] = d[i + 1]; d[i + 1] = t;
            }
        } else if (b0 == 0x40 && b1 == 0x12) { // .n64: words reversed
            for (int i = 0; i < d.length; i += 4) {
                byte t0 = d[i], t1 = d[i + 1];
                d[i] = d[i + 3]; d[i + 1] = d[i + 2]; d[i + 2] = t1; d[i + 3] = t0;
            }
        }
        if ((d[0] & 0xFF) != 0x80 || (d[1] & 0xFF) != 0x37) {
            return getString(R.string.rom_not_n64);
        }
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-1");
            byte[] h = md.digest(d);
            StringBuilder sb = new StringBuilder();
            for (byte x : h) sb.append(String.format("%02x", x & 0xFF));
            if (!sb.toString().equals(ROM_SHA1)) {
                return getString(R.string.rom_wrong_sha1, sb.toString());
            }
        } catch (Exception e) {
            return e.toString();
        }
        return null;
    }

    private void play() {
        Intent i = new Intent(this, GameActivity.class);
        i.putExtra(GameActivity.EXTRA_ROM, romFile(this).getAbsolutePath());
        startActivity(i);
    }

    @Override
    protected void onDestroy() {
        mIo.shutdown();
        super.onDestroy();
    }
}
