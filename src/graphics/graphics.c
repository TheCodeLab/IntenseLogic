#include "graphics.h"
#include "sdl/SDL.h"

SDL_Surface* canvas;

void il_Graphics_init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	canvas = SDL_SetVideoMode(800, 600, 32, SDL_DOUBLEBUF);
}

void il_Graphics_quit() {
	SDL_Quit();
}
