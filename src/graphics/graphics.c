#include "graphics.h"

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <SDL/SDL.h>
#include <sys/time.h>

#include "graphics/heightmap.h"
#include "common/heightmap.h"
#include "camera.h"
#include "common/event.h"
#include "common/input.h"
#include "common/matrix.h"
#include "common/base.h"
#include "common/keymap.h"
#include "graphics/world.h"
#include "common/log.h"

extern unsigned time(unsigned*);

SDL_Surface* canvas;
int width = 800;
int height = 600;
float heights[4] = {100, 0,0,0}; //temp
il_Graphics_Heightmap* h;
float theta;
sg_Vector3 speed;
il_Common_World* world;
il_Common_Keymap * keymap;

/*static void handleKeyDown(il_Event_Event* ev);
static void handleKeyUp(il_Event_Event* ev);*/
void il_Graphics_draw();
void il_Graphics_quit();

void il_Graphics_init() {
  srand((unsigned)time(NULL)); //temp
  
  keymap = calloc(sizeof(il_Common_Keymap), 1);
  il_Common_Keymap_defaults(keymap);
  il_Common_Keymap_parse("keymap.ini", keymap);
  il_Common_log(3, "camera: %s %s %s %s %s %s\n", keymap->camera_up, keymap->camera_down, keymap->camera_left, keymap->camera_right, keymap->camera_forward, keymap->camera_backward);
  
  // initialise SDL and create a window
  SDL_Init(SDL_INIT_EVERYTHING);
  canvas = SDL_SetVideoMode(width, height, 32, SDL_OPENGL| SDL_HWSURFACE);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8); 
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
  
  // immediate mode stuff
  glFrontFace(GL_CW);
  glClearColor(0, 0, 0, 0);
  glClearDepth(1.0);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_FLAT);
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-2, 2, -1, 1, 1.0f, 1000.0f);
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_TEXTURE_2D);
  glLoadIdentity();
  GLfloat diffuse[] = { 1.0, 1.0, 1.0};
  GLfloat lightPosition[] = {0, 0.5, 0.5, 0.0};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  // demo code that needs to go away someday
  h = il_Graphics_Heightmap_new(il_Common_Heightmap_new(heights));
  //h->drawable.positionable->position = (sg_Vector3){-5, 0, -5};
  int i;
  for (i = 0; i < 7; i++) {
    il_Common_Heightmap_Quad_divide(h->heightmap->root, 0, NULL);
  }
  speed = (sg_Vector3){0, 0, 0};
  
  // register events
  il_Event_register(IL_GRAPHICS_TICK, (il_Event_Callback)&il_Graphics_draw, NULL);
  il_Event_register(IL_BASE_SHUTDOWN, (il_Event_Callback)&il_Graphics_quit, NULL);
  
  // create the world
  world = il_Common_World_new();
  il_Graphics_active_world = il_Graphics_World_new_world(world);
  il_Graphics_active_world->camera = il_Graphics_Camera_new(il_Common_Positionable_new(world));
  il_Graphics_active_world->camera->movespeed = (sg_Vector3){1,1,1};
  il_Graphics_Camera_setEgoCamKeyHandlers(il_Graphics_active_world->camera, keymap);
  
  // start the frame timer
  struct timeval * frame = malloc(sizeof(struct timeval));
  *frame = (struct timeval){0, IL_GRAPHICS_TICK_LENGTH};
  il_Event_timer(il_Event_new(IL_GRAPHICS_TICK, 0, NULL), frame);

}

void il_Graphics_draw() {
  GLfloat lightPosition[] = {0, 0.5, 0.5, 0.0};
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  //il_Graphics_Camera_translate(il_Graphics_active_world->camera, speed);
  il_Graphics_Camera_render(il_Graphics_active_world->camera);
  theta += 0.1;
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

  struct timeval * tv = malloc(sizeof(struct timeval));
  gettimeofday(tv, NULL);

  h->drawable.draw(il_Graphics_active_world->camera, &h->drawable, tv);

  SDL_GL_SwapBuffers();
}

/*static void handleKeyDown(il_Event_Event* ev) {
  int keyCode = *(int*)&ev->data;
  if (keyCode == SDLK_LEFT || keyCode == SDLK_a) {
    speed.x = -0.1f;
  } else if (keyCode == SDLK_RIGHT || keyCode == SDLK_d) {
    speed.x = 0.1f;
  }
  if (keyCode == SDLK_r) {
    speed.y = 0.1f;
  } else if (keyCode == SDLK_f) {
    speed.y = -0.1f;
  }
  if (keyCode == SDLK_DOWN || keyCode == SDLK_s) {
    speed.z = 0.1f;
  } else if (keyCode == SDLK_UP || keyCode == SDLK_w) {
    speed.z = -0.1f;
  }
}

static void handleKeyUp(il_Event_Event* ev) {
  int keyCode = *(int*)&ev->data;
  if (keyCode == SDLK_LEFT || keyCode == SDLK_RIGHT || keyCode == SDLK_a || keyCode == SDLK_d) {
    speed.x = 0;
  } else if (keyCode == SDLK_r || keyCode == SDLK_f) {
    speed.y = 0;
  } else if (keyCode == SDLK_DOWN || keyCode == SDLK_UP || keyCode == SDLK_w || keyCode == SDLK_s) {
    speed.z = 0;
  }
}*/

void il_Graphics_quit() {
  SDL_Quit();
}
