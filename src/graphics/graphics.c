#include "graphics.h"
#include "SDL/SDL.h"
#include "GL/gl.h"
#include "heightmap.h"

SDL_Surface* canvas;
int width = 800;
int height = 600;
double theta = 0; //This is ONLY for testing purposes!
float heights[4] = {0, 0, 0, 0};
il_Graphics_Heightmap* h;

void il_Graphics_init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	canvas = SDL_SetVideoMode(width, height, 32, SDL_OPENGL| SDL_HWSURFACE); 
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE,8 ); 
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
	
	glFrontFace(GL_CW);

	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLfloat diffuse[] = { 0.0, 1.0, 0.0};
	GLfloat ambient[] = {0.4, 0.4, 0.4};
   	GLfloat lightPosition[] = { 0.0, 1.0, 0.0, 0.0};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, diffuse);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glFrustum(-2, 2, -1, 1, 1.0f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glLoadIdentity();
	
	h = il_Graphics_Heightmap_new(heights);
	h->size = 20;

	int i;
	for (i = 0; i < 6; i++) {
		il_Graphics_Heightmap_Quad_divide(h->root, 15);
	}

}

void il_Graphics_draw() {

	GLfloat lightPosition[] = { 0, 0.5, 0.5, 0.0};

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glTranslatef(-10, -5, -30);
	glRotatef(20, 1, 0, 0);
	glRotatef(theta, 0, 1, 0);
	theta += 1;

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	h->drawable->draw(h);

	SDL_GL_SwapBuffers();
}

void il_Graphics_quit() {
	SDL_Quit();
}
