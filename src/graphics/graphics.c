#include "graphics.h"
#include "sdl/SDL.h"
#include "GL/gl.h"

SDL_Surface* canvas;
int width = 800;
int height = 600;
double theta = 0; //This is ONLY for testing purposes!

void il_Graphics_init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	canvas = SDL_SetVideoMode(width, height, 32, SDL_OPENGL| SDL_HWSURFACE); 
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE,8 ); 
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );

	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glFrustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 60.0f);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glLoadIdentity();

}

void il_Graphics_draw() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glRotatef(theta, 0, 1.0f, 0);
	theta += 1;
       	glTranslatef(0, 0, -5.0f);

       	glBegin(GL_TRIANGLES);
	glColor3f(0.0f, 1.0f, 1.0f);
	glVertex3f(0, 1.0f, 0);
	glVertex3f(-1.0f, -1.0f, 0);
	glVertex3f(1.0f, -1.0f, 0);
       	glEnd();

	SDL_GL_SwapBuffers();
}

void il_Graphics_quit() {
	SDL_Quit();
}
