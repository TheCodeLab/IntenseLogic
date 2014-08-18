#include "graphics.h"

#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <signal.h>

#include "common/event.h"
#include "util/log.h"
#include "util/logger.h"
#include "util/opt.h"

ilA_fs ilG_shaders;

static void quit();

void ilG_shaders_addPath(const char *arg)
{
    ilA_adddir(&ilG_shaders, arg, -1);
}

static void sdl_error(void *ptr, int cat, SDL_LogPriority pri, const char *msg)
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
    il_logmsg *log = il_logmsg_new(1);
    il_logmsg_setLevel(log, level);
    char buf[64];
    sprintf(buf, "SDL %s error", scat);
    il_logmsg_copyMessage(log, msg);
    il_logmsg_copyBtString(log, 0, buf);
    il_logger_log(il_logger_stderr, log); // TODO: Log to appropriate location
}

void ilG_registerSdlInputBackend();
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
    ilG_registerSdlInputBackend();
}

const char **il_preload_ilgraphics()
{
    static const char *deps[] = {
        "ilcommon",
        "ilutil",
        NULL
    };
    return deps;
}

void il_configure_ilgraphics(il_modopts *opts)
{
    if (!opts) {
        return;
    }
    for (unsigned i = 0; i < opts->args.length; i++) {
        il_opt *opt = &opts->args.data[i];
        char *arg = strndup(opt->arg.str, opt->arg.len);
#define option(l) if (il_opts_cmp(opt->name, il_optslice_s(l)))
        option("shaders") {
            ilG_shaders_addPath(arg);
        }
    }
}

int il_load_ilgraphics()
{
    if (ilG_shaders.dirs.length < 1) {
        ilG_shaders_addPath("shaders");
    }
    sdl_setup();
    ilE_register(ilE_shutdown, ILE_DONTCARE, ILE_MAIN, &quit, il_value_nil());
    return 0;
}

static void quit(const il_value *data, il_value *ctx)
{
    (void)data, (void)ctx;
    SDL_VideoQuit();
    SDL_Quit();
}
