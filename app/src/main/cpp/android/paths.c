/*
 * The game's folder on Android: the app's own external files directory,
 *   /storage/emulated/0/Android/data/io.github.dedecidao.dd64/files/
 * which a file manager (or a PC over USB) can reach, and which needs no storage
 * permission. Data/Saves, Data/Logs, dd64.ini and Data/Textures/fonts_hd live
 * there, as next to the executable on PC.
 *
 * config.c and rom.c are built with -DSDL_GetBasePath=DD64A_GetBasePath
 * (CMakeLists.txt), so everything they look for is found here.
 */
#include <SDL3/SDL.h>

static char sBase[1024];

const char *DD64A_GetBasePath(void) {
    if (sBase[0] == '\0') {
        const char *ext = SDL_GetAndroidExternalStoragePath();
        const char *dir = ext != NULL ? ext : SDL_GetAndroidInternalStoragePath();
        if (dir == NULL) {
            dir = ".";
        }
        SDL_snprintf(sBase, sizeof(sBase), "%s/", dir);
    }
    return sBase;
}
