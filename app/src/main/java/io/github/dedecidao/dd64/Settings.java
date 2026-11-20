package io.github.dedecidao.dd64;

import android.content.Context;
import android.content.SharedPreferences;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.List;

/** The launcher's settings, shared with the game activity. */
final class Settings {
    private static final String PREFS = "dd64";
    static final String TOUCH = "touch_enabled";
    static final String TOUCH_OPACITY = "touch_opacity";
    static final String TOUCH_SCALE = "touch_scale";
    static final String HIDE_WITH_GAMEPAD = "touch_hide_with_gamepad";
    /** Steering with arrows: dd64.ini [input] touch_arrows, shared with the game's controls page */
    static final String ARROWS = "touch_arrows";

    private Settings() {}

    static SharedPreferences prefs(Context c) {
        return c.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
    }

    static boolean touchEnabled(Context c) {
        return prefs(c).getBoolean(TOUCH, true);
    }

    static boolean arrows(Context c) {
        return iniInt(c, ARROWS, 0) != 0;
    }

    static void setArrows(Context c, boolean on) {
        setIniInt(c, "input", ARROWS, on ? 1 : 0);
    }

    /** dd64.ini, in the app's external files folder (the game's settings, port/src/config.c) */
    static File ini(Context c) {
        return new File(c.getExternalFilesDir(null), "dd64.ini");
    }

    /** An integer key of dd64.ini, or `def` */
    static int iniInt(Context c, String key, int def) {
        try (BufferedReader r = new BufferedReader(new FileReader(ini(c)))) {
            String line;
            while ((line = r.readLine()) != null) {
                line = line.trim();
                int eq = line.indexOf('=');
                if (eq > 0 && line.substring(0, eq).trim().equals(key)) {
                    return Integer.parseInt(line.substring(eq + 1).trim());
                }
            }
        } catch (Exception e) {
            // no dd64.ini yet (before the first start): the default
        }
        return def;
    }

    /** Sets a key of dd64.ini in its section (made if missing); the game keeps it when it saves */
    static void setIniInt(Context c, String section, String key, int value) {
        List<String> lines = new ArrayList<>();
        try (BufferedReader r = new BufferedReader(new FileReader(ini(c)))) {
            String line;
            while ((line = r.readLine()) != null) {
                lines.add(line);
            }
        } catch (Exception e) {
            // a new file
        }
        String entry = key + " = " + value;
        int sectionAt = -1;
        boolean done = false;
        String current = "";
        for (int i = 0; i < lines.size(); i++) {
            String t = lines.get(i).trim();
            if (t.startsWith("[") && t.endsWith("]")) {
                current = t.substring(1, t.length() - 1).trim();
                if (current.equalsIgnoreCase(section)) {
                    sectionAt = i;
                }
                continue;
            }
            int eq = t.indexOf('=');
            if (current.equalsIgnoreCase(section) && eq > 0 && t.substring(0, eq).trim().equals(key)) {
                lines.set(i, entry);
                done = true;
                break;
            }
        }
        if (!done) {
            if (sectionAt >= 0) {
                lines.add(sectionAt + 1, entry);
            } else {
                lines.add("");
                lines.add("[" + section + "]");
                lines.add(entry);
            }
        }
        try (FileWriter w = new FileWriter(ini(c))) {
            for (String l : lines) {
                w.write(l);
                w.write("\n");
            }
        } catch (Exception e) {
            // the external folder is not available: the switch keeps its old value
        }
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
