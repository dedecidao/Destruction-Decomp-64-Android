package io.github.dedecidao.dd64;

import android.os.Bundle;
import android.view.ViewGroup;

import org.libsdl.app.SDLActivity;

/**
 * The game: SDL's activity running libdd64.so, with the touch controls on top.
 * The ROM path comes from LauncherActivity and goes to the native side as argv[1].
 */
public class GameActivity extends SDLActivity {
    public static final String EXTRA_ROM = "rom";

    private TouchControlsView mTouch;

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL3", "dd64" };
    }

    @Override
    protected String[] getArguments() {
        String rom = getIntent().getStringExtra(EXTRA_ROM);
        if (rom == null || rom.isEmpty()) {
            // Recreated by the system (after an app update, or from recents) with an
            // intent that has no extra: the ROM is where the launcher keeps it
            rom = LauncherActivity.romFile(this).getAbsolutePath();
        }
        return new String[] { rom };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (mLayout != null && Settings.touchEnabled(this)) {
            mTouch = new TouchControlsView(this);
            mLayout.addView(mTouch, new ViewGroup.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mTouch != null) {
            mTouch.refreshGamepadState();
        }
    }
}
