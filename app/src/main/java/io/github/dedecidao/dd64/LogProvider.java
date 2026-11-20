package io.github.dedecidao.dd64;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.OpenableColumns;

import java.io.File;
import java.io.FileNotFoundException;

/**
 * Hands the last session's log (Data/Logs/dd64.log) to the app the player shares
 * it with (the launcher's "Share log"), read-only and only through the URI the
 * share grants. A provider of its own: the project has no androidx (FileProvider).
 */
public class LogProvider extends ContentProvider {
    static final String FILE_NAME = "dd64.log";

    static Uri uri(android.content.Context c) {
        return Uri.parse("content://" + c.getPackageName() + ".logs/" + FILE_NAME);
    }

    static File logFile(android.content.Context c) {
        return new File(c.getExternalFilesDir(null), "Data/Logs/" + FILE_NAME);
    }

    @Override
    public boolean onCreate() {
        return true;
    }

    private File file(Uri uri) throws FileNotFoundException {
        if (!FILE_NAME.equals(uri.getLastPathSegment())) {
            throw new FileNotFoundException(uri.toString());
        }
        File f = logFile(getContext());
        if (!f.isFile()) {
            throw new FileNotFoundException(f.getPath());
        }
        return f;
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        return ParcelFileDescriptor.open(file(uri), ParcelFileDescriptor.MODE_READ_ONLY);
    }

    @Override
    public String getType(Uri uri) {
        return "text/plain";
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] args, String sort) {
        File f;
        try {
            f = file(uri);
        } catch (FileNotFoundException e) {
            return null;
        }
        String[] cols = projection != null ? projection : new String[] { OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE };
        Object[] row = new Object[cols.length];
        for (int i = 0; i < cols.length; i++) {
            if (OpenableColumns.DISPLAY_NAME.equals(cols[i])) {
                row[i] = "wrecked-metal-" + FILE_NAME;
            } else if (OpenableColumns.SIZE.equals(cols[i])) {
                row[i] = f.length();
            }
        }
        MatrixCursor c = new MatrixCursor(cols, 1);
        c.addRow(row);
        return c;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        throw new UnsupportedOperationException();
    }

    @Override
    public int delete(Uri uri, String selection, String[] args) {
        throw new UnsupportedOperationException();
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] args) {
        throw new UnsupportedOperationException();
    }
}
