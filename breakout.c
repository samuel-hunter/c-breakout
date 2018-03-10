#include <stdio.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "util.h"


#define TRUECOLOR(color) color.r, color.g, color.b

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

typedef struct Paddle {
	int x;
	int y;
} Paddle;

typedef struct Ball {
	int x;
	int y;
} Ball;


#include "config.h"

static Brick *newbrick(const Layer*, int, int);
static void drawbrick(Brick*);
static void drawpaddle();
static void movepaddle(int);
static void drawball();
static int tick(int);
static void setup();
static void run();
static void cleanup();

static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;
static Brick *brickstack = NULL;
static Paddle *paddle = NULL;
static Ball *ball = NULL;


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
	SDL_SetRenderDrawColor(ren, TRUECOLOR(brick->layer->color),
						   SDL_ALPHA_OPAQUE);

	SDL_Rect rect = { .x = brick->x, .y = brick->y,
					  .w = BRICK_WIDTH, .h = BRICK_HEIGHT };
	SDL_RenderFillRect(ren, &rect);
}

void drawpaddle()
{
	SDL_SetRenderDrawColor(ren, TRUECOLOR(paddle_color),
						   SDL_ALPHA_OPAQUE);
	SDL_Rect rect = { .x = paddle->x, .y = paddle->y,
					  .w = PADDLE_WIDTH, .h = PADDLE_HEIGHT };
	SDL_RenderFillRect(ren, &rect);
}

void movepaddle(int dist)
{
	paddle->x += dist;
	paddle->x = CLAMP(paddle->x, BORDER_SIZE,
					  GAME_WIDTH-PADDLE_WIDTH-BORDER_SIZE);
}

void drawball()
{
	SDL_SetRenderDrawColor(ren, TRUECOLOR(ball_color), SDL_ALPHA_OPAQUE);

	double error = (double) -BALL_RADIUS;
	double x = (double)BALL_RADIUS - 0.5;
	double y = 0.5;
	double cx = (double) ball->x - 0.5;
	double cy = (double) ball->y - 0.5;

	while (x >= y) {
		SDL_RenderDrawPoint(ren, (int)(cx + x), (int)(cy + y));
		SDL_RenderDrawPoint(ren, (int)(cx + y), (int)(cy + x));

		if (x) {
			SDL_RenderDrawPoint(ren, (int)(cx - x), (int)(cy + y));
			SDL_RenderDrawPoint(ren, (int)(cx + y), (int)(cy - x));
		}

		if (y) {
			SDL_RenderDrawPoint(ren, (int)(cx + x), (int)(cy - y));
			SDL_RenderDrawPoint(ren, (int)(cx - y), (int)(cy + x));
		}

		if (x && y) {
			SDL_RenderDrawPoint(ren, (int)(cx - x), (int)(cy - y));
			SDL_RenderDrawPoint(ren, (int)(cx - y), (int)(cy - x));
		}

		error += y;
		y++;
		error += y;

		if (error >= 0) {
			x--;
			error -= x * 2;
		}
	}
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
		if (event.type == SDL_KEYDOWN && !event.key.repeat)
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_ESCAPE:
				return 1;
			case SDL_SCANCODE_Q:
				return 1;
			default:
				break;
			}
	}

	const uint8_t *state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT])
		movepaddle(-PADDLE_SPEED*dt_millis/1000);
	if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT])
		movepaddle(PADDLE_SPEED*dt_millis/1000);

	for (Brick *b = brickstack; b; b = b->next)
		drawbrick(b);

	drawpaddle();
	drawball();

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
	// Set up SDL
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

	// Set up paddle
	paddle = ecalloc(1, sizeof(Paddle));
	paddle->x = (GAME_WIDTH - PADDLE_WIDTH) / 2;
	paddle->y = GAME_HEIGHT - PADDLE_HEIGHT - BORDER_SIZE;

	// Set up ball
	ball = ecalloc(1, sizeof(Ball));
	ball->x = paddle->x + (PADDLE_WIDTH/2);
	ball->y = paddle->y - BALL_RADIUS;
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

		dbprintf(DEBUG_GAME, "FPS %i\n", 1000 / MAX(1, dt));
		
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
