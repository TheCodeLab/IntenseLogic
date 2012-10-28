#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include "time.h"
#include <event2/event.h>
#include <string.h>

#include "SDL/SDL.h"
#include "common/base.h"
#include "common/input.h"
#include "common/event.h"
#include "graphics/graphics.h"
#include "network/network.h"
//#include "physics/physics.h"
#include "script/script.h"
#include "asset/asset.h"
//#include "graphics/heightmap.h"
#include "graphics/drawable3d.h"
#include "common/log.h"
#include "common/entity.h"

extern struct event_base * il_Event_base;
extern void il_Common_init();

const char *optstring = "hl:v::r:p:";

enum {
  HELP = 'h',
  LOGFILE = 'l',
  VERBOSE = 'v',
  RUN = 'r',
  PATH = 'p'
};

const struct option long_options[] = {
  {"help",      no_argument,       0, HELP    },
  {"logfile",   required_argument, 0, LOGFILE },
  {"verbose",   optional_argument, 0, VERBOSE },
  {"run",       required_argument, 0, RUN     },
  {"path",      required_argument, 0, PATH    },
  {0, 0, 0, 0}
};

const char *help[] = {
  "Prints this page",                 // -h --help
  "Sets the file to print output to", // -l --logfile
  "Sets the verbosity level",         // -v --verbose
  "Runs script",                      // -r --run
  "Adds an asset search path",        // -p --path
  NULL
};

void update(il_Event_Event * ev, void * ctx) {
  (void)ev;
  (void)ctx;
  SDL_Event sdlEvent;
  while (SDL_PollEvent(&sdlEvent)) {
    switch (sdlEvent.type) {
      case (SDL_QUIT): {
        il_Event_pushnew(IL_BASE_SHUTDOWN, 0, NULL);
        break;
      }
      case (SDL_KEYDOWN): {
        il_Event_pushnew(IL_INPUT_KEYDOWN, sizeof(int), &sdlEvent.key.keysym.sym);
        break;
      }
      case (SDL_KEYUP): {
        il_Event_pushnew(IL_INPUT_KEYUP, sizeof(int), &sdlEvent.key.keysym.sym);
        break;
      }
      case (SDL_MOUSEMOTION): {
        il_Input_MouseMove mousemove = 
          (il_Input_MouseMove){sdlEvent.motion.xrel, sdlEvent.motion.yrel};
        // the provided data pointer is memcpy'd and not preserved after its 
        // stack frame exits, so this is fine
        il_Event_pushnew(IL_INPUT_MOUSEMOVE, sizeof(il_Input_MouseMove), &mousemove);
        break;
      }
      /*case (SDL_MOUSEWHEEL): {
        il_Input_MouseWheel mousewheel = 
          (il_Input_MouseWheel){sdlEvent.wheel.x, sdlEvent.wheel.y};
        il_Event_pushnew(IL_INPUT_MOUSEWHEEL, sizeof(il_Input_MouseWheel), &mousewheel);
        break;
      }*/
      case (SDL_MOUSEBUTTONDOWN): {
        int button = sdlEvent.button.button;
        il_Event_pushnew(IL_INPUT_MOUSEDOWN, sizeof(int), &button);
        break;
      }
      case (SDL_MOUSEBUTTONUP): {
        int button = sdlEvent.button.button;
        il_Event_pushnew(IL_INPUT_MOUSEUP, sizeof(int), &button);
        break;
      }
      default: break;
    }
  }
}

int running = 1;

void shutdown_callback(il_Event_Event* ev) {
  (void)ev;
  il_Common_log(3, "Shutting down.");
  event_base_loopbreak(il_Event_base);
}

/*#ifdef __APPLE__
#else
    #undef main
#endif*/

char *strtok_r(char *str, const char *delim, char **saveptr);

int main(int argc, char **argv) {
  
  #if defined(WIN32) && !defined(DEBUG)
  il_Common_logfile = fopen("nul", "w");
  #else
  il_Common_logfile = fopen("/dev/null", "w");
  #endif
  
  #ifdef DEBUG
  il_Common_logfile = stdout;
  il_Common_loglevel = IL_COMMON_LOGNOTICE;
  #endif
  
  il_Common_log(3, "Initialising engine.");
  
  // search path priority (lower to highest): 
  // defaults, config files, environment variables, command line options
  
  // read environment variables
  char *path = getenv("IL_PATH");
  if (path) {
    char *saveptr = NULL;
    char *str = strdup(path);
    char *token;
    
    token = strtok_r(str, ":", &saveptr);
    while(token) {
      il_Asset_registerReadDir(il_Common_fromC(token), 3);
      token = strtok_r(NULL, ":", &saveptr);
    }
  } else {
    // reasonable defaults
    il_Asset_registerReadDir(il_Common_fromC((char*)"."), 4);
    il_Asset_registerReadDir(il_Common_fromC((char*)"config"), 4);
    il_Asset_registerReadDir(il_Common_fromC((char*)"shaders"), 4);
  }
  
  // build command line overrides
  int c;
  int i = 0;
  int option_index = 0;
  const char * scripts[argc];
  int n_scripts = 0;
  while( (c = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1 ) {
    switch(c) {
      case '?':
        il_Common_log(2, "Unrecognised option, ignoring.");
        break;
      case HELP:
        printf("IntenseLogic - Game engine\n\tUsage: %s [options]\n\n", argv[0]);
        while (long_options[i].name != 0) {
          printf("-%c\t--%s\t%s\n", (char)long_options[i].val, long_options[i].name, help[i]);
          i++;
        }
        exit(0);
      case LOGFILE:
        il_Common_logfile = fopen(optarg, "a");
        break;
      case VERBOSE:
        il_Common_loglevel = atoi(optarg)?atoi(optarg):4;
        break;
      case RUN:
        scripts[n_scripts] = optarg;
        n_scripts++;
        break;
      case PATH:
        il_Asset_registerReadDir(il_Common_fromC(optarg), 1);
        break;
    }
    //printf ("asdf");
  }
  
  // build config file
  
  
  il_Common_log(3, "Asset paths loaded");
  
  // I have no idea why I have to use this piece of code
  #ifdef WIN32
  WSADATA WSAData;
  WSAStartup(0x101, &WSAData);
  #endif
  
  // initialise engine
  il_Common_init();
  
  // register the updater first so it gets called first (no prioritisation 
  // system needed yet, or really important)
  il_Event_register(IL_BASE_TICK, (il_Event_Callback)&update, NULL);
  
  il_Network_init();
  il_Graphics_init();
  //il_Physics_init();
  il_Script_init();
  //il_Asset_init();
  il_Event_register(IL_BASE_SHUTDOWN, (il_Event_Callback)&shutdown_callback, NULL);
  
  // finished initialising, send startup event
  il_Event_pushnew(IL_BASE_STARTUP, 0, NULL);
  
  // Run startup scripts
  for (i = 0; i < n_scripts; i++) {
    il_Script_loadfile(scripts[i]);
  }
  
  // main loop
  il_Common_log(3, "Starting main loop");
  event_base_loop(il_Event_base, 0);
  
  // shutdown code (only reached after receiving a IL_BASE_SHUTDOWN event)
  
  return 0;
}
