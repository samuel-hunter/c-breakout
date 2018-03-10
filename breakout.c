#include <stdio.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "util.h"


#define FPS 60 /* maximum fps */

static void setup();
static void run();
static void cleanup();
int tick(int);

static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;

/*
 * Return zero to continue game
 */
int tick(int dt_millis)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
				return -1;
		if (event.type == SDL_KEYDOWN)
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_ESCAPE:
				return -1;
			default:
				break;
			}
	}

	return 0;
}

/*
 * Create window, renderer
 */
void setup()
{
	if (SDL_Init(SDL_INIT_VIDEO))
		die("SDL_INIT: %s\n", SDL_GetError());

	win = SDL_CreateWindow("Breakout",
						   100, 100,
						   800, 600,
						   SDL_WINDOW_SHOWN);
	
	if (!win) {
		SDL_Quit();
		die("SDL_CreateWindow %s\n", SDL_GetError());
	}

	ren = SDL_CreateRenderer(win, -1,
							 SDL_RENDERER_ACCELERATED |
							 SDL_RENDERER_PRESENTVSYNC);
	
	if (!ren) {
		SDL_DestroyWindow(win);
		SDL_Quit();
		die("SDL_CreateRenderer: %s\n", SDL_GetError());
	}
}

void run()
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	long millis = (tp.tv_nsec / 1000000) + (tp.tv_sec * 1000);
	int dt = 0;
	
	while (1) {
		SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(ren);

		if (tick(dt)) return;

		SDL_RenderPresent(ren);

		clock_gettime(CLOCK_MONOTONIC, &tp);
		long newmillis = (tp.tv_nsec / 1000000) + (tp.tv_sec * 1000);
		dt = newmillis - millis;
		millis = newmillis;

		dbprintf(DEBUG_INFO, "FPS %i\n", dt ? (1000 / dt) : 9999);
		
		SDL_Delay(MAX(1000 / FPS - dt, 0));
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
