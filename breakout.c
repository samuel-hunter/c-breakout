#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "util.h"


#define FPS 60

static void setup();
static void run();
static void cleanup();

static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;


void setup()
{
	if (SDL_Init(SDL_INIT_VIDEO))
		die(SDL_GetError());

	win = SDL_CreateWindow("Breakout",
						   100, 100,
						   800, 600,
						   SDL_WINDOW_SHOWN);
	
	if (!win) {
		SDL_Quit();
		die(SDL_GetError());
	}

	ren = SDL_CreateRenderer(win, -1,
							 SDL_RENDERER_ACCELERATED |
							 SDL_RENDERER_PRESENTVSYNC);
	
	if (!ren) {
		SDL_DestroyWindow(win);
		SDL_Quit();
		die(SDL_GetError());
	}
}

void run()
{
	while (1) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				return;
			if (event.type == SDL_KEYDOWN)
				switch (event.key.keysym.scancode) {
				case SDL_SCANCODE_ESCAPE:
					return;
				default:
					break;
				}
		}

		SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(ren);

		// Do neat stuff here

		SDL_RenderPresent(ren);
		SDL_Delay(1000 / FPS);
	}
}

void cleanup()
{
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

int main(int argc, char **argv)
{
	setup();
	run();
	cleanup();
	
	return 0;
}
