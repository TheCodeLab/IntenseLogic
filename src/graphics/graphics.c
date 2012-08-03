#include <stdlib.h>
#include <time.h>

#include "SDL/SDL.h"
#include "GL/gl.h"

#include "graphics.h"
#include "heightmap.h"
#include "camera.h"
#include "common/event.h"
#include "common/input.h"
#include "common/matrix.h"

SDL_Surface* canvas;
int width = 800;
int height = 600;
float heights[4] = {0, 0, 0, 0}; //temp
il_Graphics_Heightmap* h;
il_Graphics_Camera* camera;
float theta;
sg_Vector3 speed = (sg_Vector3){0, 0, 0};

void il_Graphics_init() {
	srand((unsigned)time(NULL)); //temp
	SDL_Init(SDL_INIT_EVERYTHING);
	canvas = SDL_SetVideoMode(width, height, 32, SDL_OPENGL| SDL_HWSURFACE); 
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8); 
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	
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

	h = il_Graphics_Heightmap_new(heights, 20);

	int i;
	for (i = 0; i < 6; i++) {
		il_Graphics_Heightmap_Quad_divide(h->root, 20);
	}
	
	il_Event_register(IL_INPUT_KEYDOWN, (il_Event_Callback)&handleKeyDown);
	il_Event_register(IL_INPUT_KEYUP, (il_Event_Callback)&handleKeyUp);
	
	camera = il_Graphics_Camera_new();

}

void il_Graphics_draw() {

	GLfloat lightPosition[] = {0, 0.5, 0.5, 0.0};

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	il_Graphics_Camera_translate(camera, speed.x, speed.y, speed.z);
	il_Graphics_Camera_render(camera);

	glRotatef(theta, 0, 1, 0);
	theta += 0.1;

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	h->drawable.draw(&h->drawable);

	SDL_GL_SwapBuffers();
}

void handleKeyDown(il_Event_Event* ev) {
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

void handleKeyUp(il_Event_Event* ev) {
	int keyCode = *(int*)&ev->data;
	if (keyCode == SDLK_LEFT || keyCode == SDLK_RIGHT || keyCode == SDLK_a || keyCode == SDLK_d) {
		speed.x = 0;
	} else if (keyCode == SDLK_r || keyCode == SDLK_f) {
		speed.y = 0;
	} else if (keyCode == SDLK_DOWN || keyCode == SDLK_UP || keyCode == SDLK_w || keyCode == SDLK_s) {
		speed.z = 0;
	}
}

void il_Graphics_quit() {
	SDL_Quit();
}
