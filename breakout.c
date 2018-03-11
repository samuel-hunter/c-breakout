#include <stdio.h>
#include <math.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "util.h"


// Quick macro to put in colorin SDL_SetRenderDrawColor
#define TRUECOLOR(color) color.r, color.g, color.b

typedef struct Color {
	unsigned short r;
	unsigned short g;
	unsigned short b;
} Color;

typedef struct Layer {
	double speed;
	Color color;
} Layer;

typedef struct Brick {
	int x;
	int y;
	const Layer *layer;
	Color color;
	struct Brick *next;
} Brick;

typedef struct Paddle {
	double x;
	double y;
} Paddle;

typedef struct Ball {
	double x;
	double y;

	double speed;
	double angle;
	
	double xvel;
	double yvel;
} Ball;


#include "config.h"

static Brick *newbrick(const Layer*, int, int);
static void drawbrick(Brick*);
static Brick *breakbrick(Brick*);
static void drawpaddle();
static void movepaddle(double);
static void drawball();
static void moveball();
static void setballspeed(double, double);
static int tick(double);
static void reset_paddle();
static void setup_level(size_t);
static void setup();
static void run();
static void cleanup();

static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;
static Brick *brickstack = NULL;
static Paddle *paddle = NULL;
static Ball *ball = NULL;
static size_t level = 0;


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

/*
 * Returns the next brick
 */
Brick *breakbrick(Brick *brick)
{
	Brick *temp = NULL;
	double speed = brick->layer->speed;
	
	if (brick == brickstack) {
		brickstack = brickstack->next;
		free(brick);

		temp = brickstack;
		goto finish;
	}

	// Select the previous brick
	for (temp = brickstack; temp && temp->next != brick; temp = temp->next);
	if (!temp) {
		dbprintf(DEBUG_BRICK | DEBUG_WARN, "Brick %p not found\n", brick);
		return NULL; // Do nothing if brick not found
	}

	dbprintf(DEBUG_BRICK, "Stitching brick %p to %p (removing %p)\n",
			 temp, brick->next, brick);
	temp->next = brick->next;
	free(brick);
	
	temp = temp->next;
	
 finish:

	if (ball->speed < speed) {
		int xdir = SIGNUM(ball->xvel);
		int ydir = SIGNUM(ball->yvel);
		setballspeed(speed, ball->angle);

		ball->xvel = xdir * ABS(ball->xvel);
		ball->yvel = ydir * ABS(ball->yvel);
	}

	if (!brickstack) {
		setup_level(++level);
	}
	
	return temp;
}

void drawpaddle()
{
	SDL_SetRenderDrawColor(ren, TRUECOLOR(paddle_color),
						   SDL_ALPHA_OPAQUE);
	SDL_Rect rect = { .x = (int)paddle->x, .y = (int)paddle->y,
					  .w = PADDLE_WIDTH, .h = PADDLE_HEIGHT };
	SDL_RenderFillRect(ren, &rect);
}

void movepaddle(double dist)
{
	paddle->x += dist;
	paddle->x = CLAMP(paddle->x, BORDER_SIZE,
					  GAME_WIDTH-PADDLE_WIDTH-BORDER_SIZE);

	if (!ball->speed)
		setballspeed(BALL_SPEED_START, BALL_ANGLE_START);
}

void drawball()
{
	SDL_SetRenderDrawColor(ren, TRUECOLOR(ball_color), SDL_ALPHA_OPAQUE);

	SDL_Rect rect = { .x = ball->x - BALL_RADIUS, .y = ball->y - BALL_RADIUS,
				 .w = BALL_RADIUS*2, .h = BALL_RADIUS*2 };
	SDL_RenderFillRect(ren, &rect);
}

void moveball(double dt)
{
	ball->x += ball->xvel * dt;
	ball->y += ball->yvel * dt;

	// Restrict ball within game
	if (ball->x - BALL_RADIUS <= BORDER_SIZE) {
		ball->x = BORDER_SIZE + BALL_RADIUS;
		ball->xvel = ABS(ball->xvel);
	} else if (ball->x + BALL_RADIUS >= GAME_WIDTH - BORDER_SIZE) {
		ball->x = GAME_WIDTH - BORDER_SIZE - BALL_RADIUS;
		ball->xvel = -ABS(ball->xvel);
	}

	if (ball->y - BALL_RADIUS <= BORDER_SIZE) {
		ball->y = BORDER_SIZE + BALL_RADIUS;
		ball->yvel = ABS(ball->yvel);
	} else if (ball->y + BALL_RADIUS >= GAME_HEIGHT - BORDER_SIZE) {
		reset_paddle();
	}

	// Paddle collision detection
	if (WITHIN(ball->x, paddle->x, paddle->x + PADDLE_WIDTH) &&
		ball->y + BALL_RADIUS > paddle->y) {
		setballspeed(ball->speed, M_PI * 0.667 * (ball->x - paddle->x)/PADDLE_WIDTH - M_PI * 0.833);
	}


	// Brick collision detection
	Brick *b = brickstack;
	while (b) {
		if (WITHIN(ball->x, b->x, b->x + BRICK_WIDTH) &&
			WITHIN(ball->y - BALL_RADIUS, b->y, b->y + BRICK_HEIGHT)) {
			// Top of ball hit brick
			ball->y = b->y + BRICK_HEIGHT + BALL_RADIUS;
			b = breakbrick(b);
			ball->yvel = ABS(ball->yvel);
			continue;
		} else if (WITHIN(ball->x, b->x, b->x + BRICK_WIDTH) &&
				   WITHIN(ball->y + BALL_RADIUS, b->y, b->y + BRICK_HEIGHT)) {
			// Bottom of ball hit brick
			ball->y = b->y - BALL_RADIUS;
			b = breakbrick(b);
			ball->yvel = -ABS(ball->yvel);
			continue;
		} else if (WITHIN(ball->x - BALL_RADIUS, b->x, b->x + BRICK_WIDTH) &&
				   WITHIN(ball->y, b->y, b->y + BRICK_HEIGHT)) {
			// Left of ball hit brick
			ball->x = b->x + BRICK_WIDTH + BALL_RADIUS;
			b = breakbrick(b);
			ball->xvel = ABS(ball->xvel);
			continue;
		} else if (WITHIN(ball->x + BALL_RADIUS, b->x, b->x + BRICK_WIDTH) &&
				   WITHIN(ball->y, b->y, b->y + BRICK_HEIGHT)) {
			// Right of ball hit brick
			ball->x = b->x - BALL_RADIUS;
			b = breakbrick(b);
			ball->xvel = -ABS(ball->xvel);
			continue;
		}

		b = b->next;
	}
}

void setballspeed(double speed, double angle)
{
	double xvel = speed * cos(angle);
	double yvel = speed * sin(angle);

	dbprintf(DEBUG_BALL, "\nSPD: %.2f\tANGLE: %.2f\nX: %.2f\tY: %.2f\n",
			 speed, angle, xvel, yvel);

	ball->xvel = xvel;
	ball->yvel = yvel;
	ball->speed = speed;
	ball->angle = angle;
}

/*
 * Return zero to continue game
 */
int tick(double dt)
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
		movepaddle(-PADDLE_SPEED*dt);
	if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT])
		movepaddle(PADDLE_SPEED*dt);

	moveball(dt);

	for (Brick *b = brickstack; b; b = b->next)
		drawbrick(b);

	drawpaddle();
	drawball();

	SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_Rect rect = { .x = 0, .y = 0, .w = GAME_WIDTH, .h = GAME_HEIGHT };
	SDL_RenderDrawRect(ren, &rect);

	return 0;
}

void reset_paddle()
{
	// Setup paddle
	paddle->x = (GAME_WIDTH - PADDLE_WIDTH) / 2;
	paddle->y = GAME_HEIGHT - PADDLE_HEIGHT - BORDER_SIZE;

	// Setup ball
	ball->x = paddle->x + PADDLE_WIDTH/2;
	ball->y = paddle->y - BALL_RADIUS - 3;
	ball->speed = ball->angle = ball->xvel = ball->yvel = 0;
}

void setup_level(size_t lvl)
{
	const Layer *level = levels[lvl % LEN(levels)];
	reset_paddle();

	// Remove preexisting bricks (if any)
	Brick *b = brickstack;
	while (b) {
		Brick *tmp = b->next;
		free(b);
		b = tmp;
	}

	// Set up bricks
	for (int l = 0; level[l].speed > 0; l++)
		for (int i = 0; i < NUM_BRICKS; i++)
			newbrick(level+l, i*(BRICK_WIDTH + BRICK_WGAP) + BRICK_WGAP,
					 l*(BRICK_HEIGHT + BRICK_HGAP) + BRICK_HGAP + BRICK_Y_OFFSET);
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

	paddle = ecalloc(1, sizeof(Paddle));
	ball = ecalloc(1, sizeof(Ball));
	setup_level(level);
}

void run()
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	double oldtime = (double)tp.tv_nsec / 1000000000 + tp.tv_sec;
	double dt = 0;
	
	while (1) {
		SDL_SetRenderDrawColor(ren, BGCOLOR, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(ren);

		if (tick(dt)) return;

		SDL_RenderPresent(ren);

		clock_gettime(CLOCK_MONOTONIC, &tp);
		double newtime = (double)tp.tv_nsec / 1000000000 + tp.tv_sec;
		dt = newtime - oldtime;
		oldtime = newtime;

		dbprintf(DEBUG_GAME, "FPS %.2f\n", 1.0 / MAX(0.0001, dt));
		
		SDL_Delay(MAX(1000 / FPS - dt*1000, 0));
	}
}

void cleanup()
{
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	// Free up allocated memory like a good kid
	free(ball);
	free(paddle);
	Brick *b = brickstack;
	while (b) {
		Brick *next = b->next;
		free(b);
		b = next;
	}
}

int main(int argc, char **argv)
{
	setup();
	run();
	cleanup();
	
	return 0;
}
