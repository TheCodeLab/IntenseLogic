#include "graphics.h"

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <GL/glew.h>
#include <GL/glfw.h>
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

int width = 800;
int height = 600;
il_Common_World* world;
il_Common_Keymap * keymap;
il_Graphics_Shape* shape;

void il_Graphics_draw();
void il_Graphics_quit();
static GLvoid error_cb(GLenum source, GLenum type, GLuint id, GLenum severity, 
  GLsizei length, const GLchar* message, GLvoid* userParam);

static void GLFWCALL key_cb(int key, int action)
{
  il_Common_log(4, "Key %c", key);
  if (action == GLFW_PRESS)
    il_Event_pushnew(IL_INPUT_KEYDOWN, sizeof(int), &key);
  else
    il_Event_pushnew(IL_INPUT_KEYUP, sizeof(int), &key);
}

static void GLFWCALL mouse_cb(int button, int action)
{
  //il_Common_log(4, "Mouse %i", button);
  if (action == GLFW_PRESS)
    il_Event_pushnew(IL_INPUT_MOUSEDOWN, sizeof(int), &button);
  else
    il_Event_pushnew(IL_INPUT_MOUSEUP, sizeof(int), &button);
}

static void GLFWCALL mousemove_cb(int x, int y)
{
  il_Input_MouseMove mousemove = 
    (il_Input_MouseMove){x, y};
  il_Event_pushnew(IL_INPUT_MOUSEMOVE, sizeof(il_Input_MouseMove), &mousemove);
}

static void GLFWCALL mousewheel_cb(int pos)
{
  il_Input_MouseWheel mousewheel = 
    (il_Input_MouseWheel){0, pos};
  il_Event_pushnew(IL_INPUT_MOUSEWHEEL, sizeof(il_Input_MouseWheel), &mousewheel);
}

void il_Graphics_init() 
{
  srand((unsigned)time(NULL)); //temp
  
  keymap = calloc(sizeof(il_Common_Keymap), 1);
  il_Common_Keymap_defaults(keymap);
  il_Common_Keymap_parse("keymap.ini", keymap);
  il_Common_log(3, "camera: %s %s %s %s %s %s", keymap->camera_up, 
    keymap->camera_down, keymap->camera_left, keymap->camera_right, 
    keymap->camera_forward, keymap->camera_backward);
  
  if (!glfwInit()) {
    il_Common_log(0, "glfwInit() failed");
    abort();
  }
  
  {
    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    il_Common_log(3, "Using GLFW version %i.%i.%i", major, minor, rev);
  }
  
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);
  #ifdef DEBUG
  glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  #endif
  
  if (!glfwOpenWindow(width, height, 8, 8, 8, 8, 32, 8, GLFW_WINDOW)) {
    il_Common_log(0, "glfwOpenWindow() failed");
    abort();
  }
  
  // register glfw stuff
  glfwSetCharCallback(&key_cb);
  glfwSetMouseButtonCallback(&mouse_cb);
  glfwSetMousePosCallback(&mousemove_cb);
  glfwSetMouseWheelCallback(&mousewheel_cb);
  
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
    glDebugMessageCallbackARB((GLDEBUGPROCARB)&error_cb, NULL);
    il_Common_log(3, "ARB_debug_output present, enabling advanced errors");
  } else
    il_Common_log(3, "ARB_debug_output missing");
  
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
  
  glfwSwapInterval(1); // 1:1 ratio of frames to vsyncs
  il_Event_pushnew(IL_GRAPHICS_TICK, 0, NULL); // kick off the draw loop
}

void il_Graphics_draw()
{  
  if (!glfwGetWindowParam(GLFW_OPENED)) {
    il_Event_pushnew(IL_BASE_SHUTDOWN, 0, NULL);
    return;
  }
  il_Common_log(5, "Rendering frame");
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
  
  glfwSwapBuffers(); // locks until the next vsync, hopefully
  il_Event_pushnew(IL_GRAPHICS_TICK, 0, NULL); // render another frame
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
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: ssource="Shader Compiler"; break;
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
  glfwTerminate();
}
