#include "graphics.h"

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <GL/glew.h>

#include <SDL/SDL.h>
#include <sys/time.h>

//#include "graphics/heightmap.h"
//#include "common/heightmap.h"
#include "graphics/camera.h"
#include "common/event.h"
#include "common/input.h"
#include "common/matrix.h"
#include "common/base.h"
#include "common/keymap.h"
#include "graphics/world.h"
#include "common/log.h"
#include "graphics/shape.h"
#include "asset/asset.h"
#include "common/world.h"
#include "graphics/terrain.h"
#include "common/terrain.h"

extern unsigned time(unsigned*);

SDL_Surface* canvas;
int width = 800;
int height = 600;
il_Common_World* world;
il_Common_Keymap * keymap;
il_Graphics_Shape* shape;

void il_Graphics_draw();
void il_Graphics_quit();
static GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity, 
  GLsizei length, const GLchar* message, GLvoid* userParam);

void il_Graphics_init() 
{
  srand((unsigned)time(NULL)); //temp
  
  keymap = calloc(sizeof(il_Common_Keymap), 1);
  il_Common_Keymap_defaults(keymap);
  il_Common_Keymap_parse("keymap.ini", keymap);
  il_Common_log(3, "camera: %s %s %s %s %s %s", keymap->camera_up, keymap->camera_down, keymap->camera_left, keymap->camera_right, keymap->camera_forward, keymap->camera_backward);
  
  // initialise SDL and create a window
  SDL_Init(SDL_INIT_EVERYTHING);
  canvas = SDL_SetVideoMode(width, height, 32, SDL_OPENGL| SDL_HWSURFACE);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8); 
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
  
  // GLEW
  GLenum err = glewInit();

  if (GLEW_OK != err) {
    il_Common_log(0, "glewInit() failed: %s", glewGetErrorString(err));
    abort();
  }
  il_Common_log(3, "Using GLEW %s", glewGetString(GLEW_VERSION));

#ifndef __APPLE__
  if (!GLEW_VERSION_3_1) {
    il_Common_log(1, "GL version 3.1 is required, trying anyway");
    //abort();
  }
#endif
  
  if (GLEW_ARB_debug_output) {
    glDebugMessageCallbackARB(&error_cb, NULL);
    il_Common_log(3, "ARB_debug_output present, enabling advanced errors");
  } else
    il_Common_log(2, "ARB_debug_output missing");
  
  // setup our shader directory
  il_Asset_registerReadDir(il_Common_fromC("shaders"),0);
  
  // GL setup
  glClearColor(0,0,0,1);
  glClearDepth(1.0);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  
  // register events
  il_Event_register(IL_GRAPHICS_TICK, (il_Event_Callback)&il_Graphics_draw, 
    NULL);
  il_Event_register(IL_BASE_SHUTDOWN, (il_Event_Callback)&il_Graphics_quit, 
    NULL);
  
  // create the world
  world = il_Common_World_new();
  il_Graphics_active_world = il_Graphics_World_new_world(world);
  il_Graphics_active_world->camera = il_Graphics_Camera_new(
    il_Common_Positionable_new(world));
  il_Graphics_active_world->camera->movespeed = (sg_Vector3){1,1,1};
  il_Graphics_active_world->camera->projection_matrix = sg_Matrix_perspective(
    75, (float)width/(float)height, 0.25, 100);
  il_Graphics_Camera_setEgoCamKeyHandlers(il_Graphics_active_world->camera, 
    keymap);
  
  il_Common_Positionable * shape_positionable = il_Common_Positionable_new(
    world);
  shape_positionable->position = (sg_Vector3){1,0,0};
  shape = il_Graphics_Shape_new(
    shape_positionable,
    il_Graphics_Box
  );
  if (!shape) {
    il_Common_log(0, "Failed to create demo shape");
    abort();
  }
  
  /*il_Common_Terrain *ter = il_Common_Terrain_new();
  il_Common_Terrain_heightmapFromSeed(
    ter,
    0xD34DB33F, // seed, does nothing atm
    1.0, // resolution
    100 //viewdistance
  );
  il_Graphics_Terrain_new(ter, il_Common_Positionable_new(world));
  */
  
  // start the frame timer
  struct timeval * frame = calloc(1, sizeof(struct timeval));
  *frame = (struct timeval){0, IL_GRAPHICS_TICK_LENGTH};
  il_Event_timer(il_Event_new(IL_GRAPHICS_TICK, 0, NULL), frame);

}

void il_Graphics_draw()
{  
  struct timeval * tv = calloc(1, sizeof(struct timeval));
  il_Common_Positionable* pos;
  il_Graphics_Drawable3d* dr;
  il_Common_WorldIterator* witer = NULL;
  il_Graphics_Drawable3dIterator* diter;
  
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  gettimeofday(tv, NULL);
  
  while ((pos = il_Common_World_iterate(il_Graphics_active_world->world, 
    &witer))) {
  
    diter = NULL;
    while ((dr = il_Graphics_Drawable3d_iterate(pos, &diter))) {
      if (dr->draw) {
        dr->draw(il_Graphics_active_world->camera, dr, tv);
      }
    }
  }
  
  SDL_GL_SwapBuffers();
}

static GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity, 
  GLsizei length, const GLchar* message, GLvoid* user)
{
  /* severity is one of:
  DEBUG_SEVERITY_HIGH_ARB, DEBUG_SEVERITY_MEDIUM_ARB, DEBUG_SEVERITY_LOW_ARB
  */
  (void)id, (void)severity, (void)length, (void)user;
  const char *ssource;
  switch(source) {
    case GL_DEBUG_SOURCE_API_ARB: ssource="API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: ssource="Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: ssource="Shader Compile"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB: ssource="Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION_ARB: ssource="Application"; break;
    case GL_DEBUG_SOURCE_OTHER_ARB: ssource="Other"; break;
    default: ssource="???";
  }
  const char *stype;
  switch(type) {
    case GL_DEBUG_TYPE_ERROR_ARB: stype="Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: 
      stype="Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB: 
      stype="Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY_ARB: stype="Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE_ARB: stype="Performance"; break;
    case GL_DEBUG_TYPE_OTHER_ARB: stype="Other"; break;
    default: stype="???";
  }
  const char *sseverity;
  switch(severity) {
    case GL_DEBUG_SEVERITY_HIGH_ARB: sseverity="HIGH"; break;
    case GL_DEBUG_SEVERITY_MEDIUM_ARB: sseverity="MEDIUM"; break;
    case GL_DEBUG_SEVERITY_LOW_ARB: sseverity="LOW"; break;
    default: sseverity="???";
  }
  fprintf(il_Common_logfile, "OpenGL %s (%s) %s: %s\n", ssource, stype, 
    sseverity, message);
}

void il_Graphics_quit() 
{
  SDL_Quit();
}
