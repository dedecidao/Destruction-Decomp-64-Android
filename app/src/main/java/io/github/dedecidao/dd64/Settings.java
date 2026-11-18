package io.github.dedecidao.dd64;

import android.content.Context;
import android.content.SharedPreferences;

/** The launcher's settings, shared with the game activity. */
final class Settings {
    private static final String PREFS = "dd64";
    static final String TOUCH = "touch_enabled";
    static final String TOUCH_OPACITY = "touch_opacity";
    static final String TOUCH_SCALE = "touch_scale";
    static final String HIDE_WITH_GAMEPAD = "touch_hide_with_gamepad";

    private Settings() {}

    static SharedPreferences prefs(Context c) {
        return c.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
    }

    static boolean touchEnabled(Context c) {
        return prefs(c).getBoolean(TOUCH, true);
    }

    static boolean hideWithGamepad(Context c) {
        return prefs(c).getBoolean(HIDE_WITH_GAMEPAD, true);
    }

    /** 0.1 .. 1.0 */
    static float touchOpacity(Context c) {
        return prefs(c).getInt(TOUCH_OPACITY, 60) / 100f;
    }

    /** 0.6 .. 1.6 */
    static float touchScale(Context c) {
        return prefs(c).getInt(TOUCH_SCALE, 100) / 100f;
    }
}
