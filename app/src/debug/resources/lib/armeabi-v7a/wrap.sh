#!/system/bin/sh
# Debug builds (armeabi-v7a): start the app's processes with libdd64reserve.so
# preloaded, so that 0x80000000 is reserved for the game's RAM before ART maps its
# JIT cache there. See app/src/main/cpp/android/reserve/dd64reserve.c.
# Android only runs wrap.sh for debuggable apps.
HERE="$(dirname "$0")"
export LD_PRELOAD="$HERE/libdd64reserve.so${LD_PRELOAD:+:$LD_PRELOAD}"
exec "$@"