#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include "time.h"

#include "sdl/SDL.h"
#include "common/base.h"
#include "common/event.h"
#include "graphics/graphics.h"
#include "network/network.h"
//#include "physics/physics.h"
#include "script/script.h"
#include "asset/asset.h"

const char *optstring = "h";

enum {
  HELP = 'h'
};

const struct option long_options[] = {
  {"help", no_argument, 0, HELP},
  {0, 0, 0, 0}
};

const char *help[] = {
  "Prints this page", // -h --help
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

  // build config file
  
  
  
  // build command line overrides
  int c;
  int i = 0;
  int option_index = 0;
  while( (c = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1 ) {
    switch(c) {
      case '?':
        printf("Unrecognised option, ignoring.\n");
        break;
      case HELP:
        while (long_options[i].name != 0) {
          printf("%c  %s  %s\n", (char)long_options[i].val, long_options[i].name, help[i]);
          i++;
        }
        exit(0);
    }
    //printf ("asdf");
  }
  
  
  // initialise engine
  il_Network_init();
  il_Graphics_init();
  //il_Physics_init();
  il_Script_init();
  il_Asset_init();
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
  while (running) {
    gettimeofday(&start,NULL);

    //printf("state: %i\n", state);
    
    if (state == 0) {
      tick();
    } else {
      draw();
    }
    
    update();
    
    if (state == 1) {
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
      printf("Behind on ticks!\n");
    }
    
    if (state == 0) {
      lasttick = start;
    } else {
      lastframe = start;
    }
    
    usleep(sleep.tv_usec);
    
  }
  
  
  // shutdown code (only reached after receiving a IL_BASE_SHUTDOWN event)
  il_Graphics_quit();
  
  return 0;
}
