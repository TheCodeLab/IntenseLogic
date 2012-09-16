#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include "time.h"
#include <event2/event.h>

#include "SDL/SDL.h"
#include "common/base.h"
#include "common/input.h"
#include "common/event.h"
#include "graphics/graphics.h"
#include "network/network.h"
//#include "physics/physics.h"
#include "script/script.h"
//#include "asset/asset.h"
#include "graphics/heightmap.h"
#include "graphics/drawable3d.h"
#include "common/log.h"
#include "common/entity.h"

extern struct event_base * il_Event_base;
extern void il_Common_init();

const char *optstring = "hl:v::r:";

enum {
  HELP = 'h',
  LOGFILE = 'l',
  VERBOSE = 'v',
  RUN = 'r'
};

const struct option long_options[] = {
  {"help",      no_argument,       0, HELP    },
  {"logfile",   required_argument, 0, LOGFILE },
  {"verbose",   optional_argument, 0, VERBOSE },
  {"run",       required_argument, 0, RUN },
  {0, 0, 0, 0}
};

const char *help[] = {
  "Prints this page",                 // -h --help
  "Sets the file to print output to", // -l --logfile
  "Sets the verbosity level",         // -v --verbose
  "Runs script",                      // -r --run
  NULL
};

void update(il_Event_Event * ev, void * ctx) {
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
          (il_Input_MouseMove){sdlEvent.motion.x, sdlEvent.motion.y};
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
  il_Common_log(3, "Shutting down.\n");
  event_base_loopbreak(il_Event_base);
}

/*#ifdef __APPLE__
#else
    #undef main
#endif*/
int main(int argc, char **argv) {

  printf("test\n");
  
  #ifdef DEBUG
  il_Common_logfile = stdout;
  #endif
  
  il_Common_log(3, "Initialising engine.");

  // build config file
  
  
  
  // build command line overrides
  int c;
  int i = 0;
  int option_index = 0;
  while( (c = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1 ) {
    switch(c) {
      case '?':
        il_Common_log(2, "Unrecognised option, ignoring.\n");
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
        il_Common_loglevel = atoi(optarg)?atoi(optarg):3;
        break;
      case RUN:
        il_Script_loadfile(optarg);
        break;
    }
    //printf ("asdf");
  }
  
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
  
  // main loop
  event_base_loop(il_Event_base, 0);
  
  // shutdown code (only reached after receiving a IL_BASE_SHUTDOWN event)
  
  return 0;
}
