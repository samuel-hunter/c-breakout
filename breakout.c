#include <stdio.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "util.h"


typedef struct Color {
	unsigned short r;
	unsigned short g;
	unsigned short b;
} Color;

typedef struct Layer {
	int speed;
	Color color;
} Layer;

typedef struct Brick {
	int x;
	int y;
	const Layer *layer;
	int speed;
	Color color;
	struct Brick *next;
} Brick;


#include "config.h"

static Brick *newbrick(const Layer*, int, int);
static void drawbrick(Brick*);
static int tick(int);
static void setup();
static void run();
static void cleanup();

static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;
static Brick *brickstack = NULL;


Brick *newbrick(const Layer *layer, int x, int y)
{
	Brick *b = ecalloc(1, sizeof(Brick));
	b->x = x;
	b->y = y;
	b->layer = layer;

	b->next = brickstack;
	brickstack = b;

	return b;
}

void drawbrick(Brick *brick)
{
	SDL_SetRenderDrawColor(ren, brick->layer->color.r, brick->layer->color.g,
						   brick->layer->color.b, SDL_ALPHA_OPAQUE);

	SDL_Rect rect = { .x = brick->x, .y = brick->y,
					  .w = BRICK_WIDTH, .h = BRICK_HEIGHT };
	SDL_RenderFillRect(ren, &rect);
}

/*
 * Return zero to continue game
 */
int tick(int dt_millis)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
				return 1;
		if (event.type == SDL_KEYDOWN)
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_ESCAPE:
				return 1;
			case SDL_SCANCODE_Q:
				return 1;
			default:
				break;
			}
	}

	for (Brick *b = brickstack; b; b = b->next)
		drawbrick(b);

	SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_Rect rect = { .x = 0, .y = 0, .w = GAME_WIDTH, .h = GAME_HEIGHT };
	SDL_RenderDrawRect(ren, &rect);

	return 0;
}

/*
 * Create window, renderer
 */
void setup()
{
	if (SDL_Init(SDL_INIT_VIDEO))
		die("SDL_INIT: %s\n", SDL_GetError());

	win = SDL_CreateWindow(GAME_NAME,
						   100, 100,
						   GAME_WIDTH, GAME_HEIGHT,
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


	// Set up bricks
	for (int l = 0; l < LEN(layers); l++)
		for (int i = 0; i < NUM_BRICKS; i++)
			newbrick(&layers[l], i*(BRICK_WIDTH + BRICK_WGAP) + BRICK_WGAP,
					 l*(BRICK_HEIGHT + BRICK_HGAP) + BRICK_HGAP);
}

void run()
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	long millis = (tp.tv_nsec / 1000000) + (tp.tv_sec * 1000);
	int dt = 0;
	
	while (1) {
		SDL_SetRenderDrawColor(ren, BGCOLOR, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(ren);

		if (tick(dt)) return;

		SDL_RenderPresent(ren);

		clock_gettime(CLOCK_MONOTONIC, &tp);
		long newmillis = (tp.tv_nsec / 1000000) + (tp.tv_sec * 1000);
		dt = newmillis - millis;
		millis = newmillis;

		dbprintf(DEBUG_INFO, "FPS %i\n", 1000 / MAX(1, dt));
		
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
