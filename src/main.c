#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include "time.h"

#include "SDL/SDL.h"
#include "common/base.h"
#include "common/event.h"
#include "graphics/graphics.h"
#include "network/network.h"
//#include "physics/physics.h"
#include "script/script.h"
//#include "asset/asset.h"
#include "graphics/heightmap.h"
#include "graphics/drawable3d.h"
#include "common/log.h"

const char *optstring = "hl:v::";

enum {
  HELP = 'h',
  LOGFILE = 'l',
  VERBOSE = 'v'
};

const struct option long_options[] = {
  {"help",      no_argument,       0, HELP    },
  {"logfile",   required_argument, 0, LOGFILE },
  {"verbose",   optional_argument, 0, VERBOSE },
  {0, 0, 0, 0}
};

const char *help[] = {
  "Prints this page", // -h --help
  "Sets the file to print output to", // -l --logfile
  "Sets the verbosity level", // -v --verbose
  NULL
};

void update() {
  SDL_Event sdlEvent;
  if (SDL_PollEvent(&sdlEvent)) {
    if (sdlEvent.type == SDL_QUIT) {
      il_Event_Event* quit = malloc(sizeof(il_Event_Event));
      quit->eventid = IL_BASE_SHUTDOWN;
      quit->size = 0;
      il_Event_push(quit);
    } 
  }
  
  // handle events
  while (il_Event_EventQueue_first != NULL) {
    il_Event_handle((il_Event_Event*)il_Event_pop());
    //printf("test\n");
  }
}

void tick() {
  il_Event_Event* tick = malloc(sizeof(il_Event_Event));
  tick->eventid = IL_BASE_TICK;
  tick->size = 0;
  il_Event_push(tick);
}

void draw() {
  il_Event_Event* frame = malloc(sizeof(il_Event_Event));
  frame->eventid = IL_GRAPHICS_TICK;
  frame->size = 0;
  il_Event_push(frame);
}

int running = 1;

void shutdown_callback(il_Event_Event* ev) {
  running = 0;
}

#undef main
int main(int argc, char **argv) {

  #ifdef DEBUG
  il_Common_logfile = stdout;
  #endif

  // build config file
  
  
  
  // build command line overrides
  int c;
  int i = 0;
  int option_index = 0;
  while( (c = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1 ) {
    switch(c) {
      case '?':
        il_Common_log(3, "Unrecognised option, ignoring.\n");
        break;
      case HELP:
        while (long_options[i].name != 0) {
          printf("%c  %s  %s\n", (char)long_options[i].val, long_options[i].name, help[i]);
          i++;
        }
        exit(0);
      case LOGFILE:
        il_Common_logfile = fopen(optarg, "a");
        break;
      case VERBOSE:
        il_Common_loglevel = atoi(optarg)?atoi(optarg):3;
        break;
    }
    //printf ("asdf");
  }
  
  
  // initialise engine
  il_Network_init();
  il_Graphics_init();
  //il_Physics_init();
  il_Script_init();
  //il_Asset_init();
  il_Event_register(IL_BASE_SHUTDOWN, (il_Event_Callback)&shutdown_callback);
  
  
  // finished initialising, send startup event
  il_Event_Event* ev = malloc(sizeof(il_Event_Event));
  ev->eventid = IL_BASE_STARTUP;
  ev->size = 0; // no extra data
  il_Event_push(ev);
  
  update();
  
  struct timeval start;
  struct timeval stop;
  struct timeval sleep;
  struct timeval ticks_temp;
  struct timeval frames_temp;
  struct timeval ticklen;
  ticklen.tv_sec = 0;
  ticklen.tv_usec = IL_BASE_TICK_LENGTH;
  struct timeval framelen;
  framelen.tv_sec = 0;
  framelen.tv_usec = IL_GRAPHICS_TICK_LENGTH;
  struct timeval lasttick;
  gettimeofday(&lasttick,NULL);
  struct timeval lastframe;
  gettimeofday(&lastframe,NULL);
  struct timeval empty;
  timerclear(&empty);
  int state;
  
  int frames_this_second;
  int ticks_this_second;
  int last_second;

  while (running) {
    gettimeofday(&start,NULL);

    //printf("state: %i\n", state);
    
    if (state == 0) {
      tick();
    } else {
      draw();
    }
    
    update();
    
    if (state != 0) {
      il_Graphics_draw();
    }
    
    // calculate time to sleep
    gettimeofday(&stop,NULL);
    
    timersub(&stop, &start, &sleep);
    
    //printf("frame length: %i\n", sleep.tv_usec);
    
    timeradd(&ticklen, &lasttick, &ticks_temp);
    timeradd(&framelen, &lastframe, &frames_temp);

    state = timercmp(&ticks_temp, &frames_temp, >);
    
    if (state == 0) {
      timersub(&ticks_temp, &stop, &sleep);
    } else {
      timersub(&frames_temp, &stop, &sleep);
    }
    
    if (timercmp(&empty, &sleep, >) != 0) {
      sleep = empty;
      /*printf("Behind on ticks!\n");
      printf( "start: %u %u\n"
              "stop: %u %u\n"
              "sleep: %u %i\n"
              "state: %i\n"
              "ticklen: %u %u\n"
              "framelen: %u %u\n"
              "lasttick: %u %u\n"
              "lastframe: %u %u\n",
              start.tv_sec, start.tv_usec, stop.tv_sec, stop.tv_usec, sleep.tv_sec, sleep.tv_usec, state, 
              ticklen.tv_sec, ticklen.tv_usec, framelen.tv_sec, framelen.tv_usec, lasttick.tv_sec, lasttick.tv_usec, 
              lastframe.tv_sec, lastframe.tv_usec );
      */
    }
    
    if (last_second != start.tv_sec) {
      last_second = start.tv_sec;
      il_Common_log(4,"fps: %i\ntps: %i\n", frames_this_second, ticks_this_second);
      frames_this_second = 0;
      ticks_this_second = 0;
    }
    
    if (state == 0) {
      lasttick = start;
      ticks_this_second++;
    } else {
      lastframe = start;
      frames_this_second++;
    }
    
    usleep(sleep.tv_usec);
    
  }
  
  
  // shutdown code (only reached after receiving a IL_BASE_SHUTDOWN event)
  il_Graphics_quit();
  
  return 0;
}
