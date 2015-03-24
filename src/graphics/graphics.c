#include "graphics.h"

#include <stdlib.h>
#include <SDL.h>
#include <SDL_video.h>
#include <signal.h>

#include "util/event.h"
#include "util/log.h"
#include "util/logger.h"
#include "util/opt.h"

ilA_fs ilG_shaders;

void ilG_shaders_addPath(const char *arg)
{
    ilA_adddir(&ilG_shaders, arg, -1);
}

static void sdl_error(void *ptr, int cat, SDL_LogPriority pri, const char *reason)
{
    (void)ptr;
    const char *scat;
    unsigned level;
    switch (cat) {
#define C(n, s) case n: scat = s; break;
        C(SDL_LOG_CATEGORY_APPLICATION, "application"   )
        C(SDL_LOG_CATEGORY_ERROR,       "error"         )
        C(SDL_LOG_CATEGORY_SYSTEM,      "system"        )
        C(SDL_LOG_CATEGORY_AUDIO,       "audio"         )
        C(SDL_LOG_CATEGORY_VIDEO,       "video"         )
        C(SDL_LOG_CATEGORY_RENDER,      "render"        )
        C(SDL_LOG_CATEGORY_INPUT,       "input"         )
        C(SDL_LOG_CATEGORY_CUSTOM,      "custom"        )
        default:
        scat = "unknown";
#undef C
    }
    switch (pri) {
#define C(n, l) case n: level = l; break;
        C(SDL_LOG_PRIORITY_VERBOSE,  5)
        C(SDL_LOG_PRIORITY_DEBUG,    4)
        C(SDL_LOG_PRIORITY_INFO,     3)
        C(SDL_LOG_PRIORITY_WARN,     2)
        C(SDL_LOG_PRIORITY_ERROR,    1)
        C(SDL_LOG_PRIORITY_CRITICAL, 0)
        default:
        level = 3;
#undef C
    }
    il_logmsg log;
    memset(&log, 0, sizeof(il_logmsg));
    char msg_str[64];
    sprintf(msg_str, "SDL %s error", scat);
    log.level = level;
    log.msg = il_string_new(msg_str);
    log.reason = il_string_new((char*)reason);
    il_logger_log(il_logger_cur(), log);
}

static void sdl_setup()
{
    if (SDL_Init(SDL_INIT_NOPARACHUTE) != 0) {
        il_error("SDL_Init: %s", SDL_GetError());
    }
    if (SDL_VideoInit(NULL) != 0) {
        il_error("SDL_VideoInit: %s", SDL_GetError());
    }
    signal(SIGINT, SIG_DFL); // TODO: fuck you sdl, for not letting me disable the signal handler
    signal(SIGTERM, SIG_DFL);
    SDL_LogSetOutputFunction(sdl_error, NULL);
    il_log("Using SDL %s", SDL_GetRevision());
}

bool ilG_module_loaded = false;

int il_load_ilgraphics()
{
    if (ilG_shaders.dirs.length < 1) {
        ilG_shaders_addPath("shaders");
    }
    sdl_setup();
    ilG_module_loaded = true;
    return 0;
}

void ilG_quit()
{
    SDL_VideoQuit();
    SDL_Quit();
}
