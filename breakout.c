#include <stdio.h>
#include <math.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "util.h"


// Quick macro to put color in SDL_SetRenderDrawColor
#define TRUECOLOR(color) color->r, color->g, color->b

typedef struct Color {
	unsigned short r, g, b;
} Color;

typedef struct Layer {
	int speed;
	Color color;
} Layer;

typedef struct Sprite {
	double x, y;
} Sprite;

typedef struct Brick {
	double x, y;
	const Layer *layer;
	struct Brick *next;
} Brick;

typedef struct Ball {
	double x, y;
	double speed, angle;
	double xvel, yvel;
} Ball;


#include "config.h"

static void drawsprite(const Sprite*, int, int, const Color*);
static void   makebrick(const Layer*, int, int);
static Brick *breakbrick(Brick*);
static void   emptybrickstack();
static void movepaddle(double);
static void resetpaddle();
static int  ballcollideswith(SDL_Rect*);
static void moveball();
static void setballspeed(double, double);
static void drawtext(const char*, int, int);
static int  tick(double, int);
static void setuplevel(size_t);
static void setup();
static void run();
static void cleanup();

// SDL
static SDL_Window *win = NULL;
static SDL_Renderer *ren = NULL;
static TTF_Font *hudfont = NULL;

// Game
static Brick *brickstack = NULL;
static Sprite *paddle = NULL;
static Ball *ball = NULL;
static size_t level = 0; // size_t because it accesses an array
static unsigned int lives = STARTING_LIVES;
static unsigned int score = 0;


void drawsprite(const Sprite *s, int w, int h, const Color *color)
{
	SDL_SetRenderDrawColor(ren, TRUECOLOR(color), SDL_ALPHA_OPAQUE);
	SDL_Rect rect = { .x = s->x + game_area.x, .y = s->y + game_area.y,
					  .w = w, .h = h };
	SDL_RenderFillRect(ren, &rect);
}

void makebrick(const Layer *layer, int x, int y)
{
	Brick *b = ecalloc(1, sizeof(Brick));
	*b = (Brick){ .x = x, .y = y, .layer = layer, .next = brickstack };
	brickstack = b;
}

/*
 * Returns the next brick
 */
Brick *breakbrick(Brick *brick)
{
	Brick *temp = NULL;
    double points = brick->layer->speed;
	double speed = brick->layer->speed;
    for (int i = 0; i < level/LEN(levels); i++)
        speed *= PRESTIGE_FACT;
	
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

	// Stitch out brick from list linking
	temp->next = brick->next;
	free(brick);
	
	temp = temp->next;
	
 finish:
    score += points;
	if (ball->speed < speed)
		setballspeed(speed, ball->angle);
	
	return temp;
}

void emptybrickstack()
{
	while (brickstack) {
		Brick *tmp = brickstack->next;
		free(brickstack);
		brickstack = tmp;
	}
}

void movepaddle(double dist)
{
	paddle->x += dist;
	paddle->x = CLAMP(paddle->x, 0,
					  game_area.w-PADDLE_WIDTH);

	if (!ball->speed)
		setballspeed(BALL_SPEED_START, BALL_ANGLE_START);
}

void resetpaddle()
{
	// Setup paddle
	paddle->x = (game_area.w - PADDLE_WIDTH) / 2;
	paddle->y = game_area.h - PADDLE_HEIGHT;

	// Setup ball
	ball->x = paddle->x + PADDLE_WIDTH/2 - BALL_SIZE/2;
	ball->y = paddle->y - BALL_SIZE - 3;
	ball->speed = ball->angle = ball->xvel = ball->yvel = 0;
}

/*
 * Returns:
 *   0 if no collision
 *   1 if top of ball collides
 *   2 if bottom of ball collides
 *   3 if left of ball collides
 *   4 if right of ball collides
 */
int ballcollideswith(SDL_Rect *rect)
{
	int bl, br, bu, bd, // ball (left, right, up, down)
		rl, rr, ru, rd; // rect (...)
	br = (bl = ball->x) + BALL_SIZE;
	bd = (bu = ball->y) + BALL_SIZE;
	rr = (rl = rect->x) + rect->w;
	rd = (ru = rect->y) + rect->h;

	
	int xdiff = MIN(br, rr) - MAX(bl, rl),
		ydiff = MIN(bd, rd) - MAX(bu, ru);

	if (xdiff > 0 && ydiff > 0)
		dbprintf(DEBUG_BALL, "\nxdiff: %d\tydiff: %d\n", xdiff, ydiff);
	

	if (WITHIN(bu, ru, rd) && br >= rl && bl <= rr && xdiff >= ydiff)
		return 1;
	if (WITHIN(bl, rl, rr) && bd >= ru && bu <= rd && ydiff >= xdiff)
		return 3;
	if (WITHIN(br, rl, rr) && bd >= ru && bu <= rd && ydiff >= xdiff)
		return 4;
	if (WITHIN(bd, ru, rd) && br >= rl && bl <= rr && xdiff >= ydiff)
		return 2;

	return 0;
}

/*
 * Return:
 *   1 if ball hit floor.
 *   2 if ball breaks last brick.
 *   0 otherwise.
 */
void moveball(double dt)
{
	ball->x += ball->xvel * dt;
	ball->y += ball->yvel * dt;

	// Restrict ball within game
	if (ball->x <= 0) {
		// Ball hit left of area
		ball->x = 0;
		setballspeed(ball->speed, M_PI - ball->angle);
	} else if (ball->x + BALL_SIZE >= game_area.w) {
		// Ball hit right of area
		ball->x = game_area.w - BALL_SIZE;
		setballspeed(ball->speed, M_PI - ball->angle);
	}

	if (ball->y <= 0) {
		// Ball hit top of area
		ball->y = 0;
		setballspeed(ball->speed, -ball->angle);
	}

	// Paddle collision detection; give the paddle a ludicrously
	// large height to prevent the ball getting under
	SDL_Rect pr = { .x = paddle->x, .y = paddle->y,
					.w = PADDLE_WIDTH, .h = 500 };
	if (ballcollideswith(&pr))
		// Depending on position of paddle, change ball angle to
		//  within [5pi/6, pi/6]
		setballspeed(ball->speed,
					 M_PI * (0.8333 - 0.6667 *
							 (ball->x - paddle->x + BALL_SIZE) /
							 (PADDLE_WIDTH + BALL_SIZE)));


	// Brick collision detection
	Brick *b = brickstack;
	while (b) {
		SDL_Rect r = { .x = b->x, .y = b->y,
					   .w = BRICK_WIDTH, .h = BRICK_HEIGHT };
		int edge = ballcollideswith(&r);
		switch (edge) {
		case 1: // Top edge hits brick
			ball->y = b->y + BRICK_HEIGHT + 1;
			b = breakbrick(b);
			setballspeed(ball->speed, -ball->angle);
			break;
		case 2: // Bottom edge hits brick
			ball->y = b->y - BALL_SIZE - 1;
			b = breakbrick(b);
			setballspeed(ball->speed, -ball->angle);
			break;
		case 3: // Left edge hits brick
			ball->x = b->x + BRICK_WIDTH + 1;
			b = breakbrick(b);
			setballspeed(ball->speed, M_PI - ball->angle);
			break;
		case 4: // Right edge hits brick
			ball->x = b->x - BALL_SIZE - 1;
			b = breakbrick(b);
			setballspeed(ball->speed, M_PI - ball->angle);
			break;
		default: // No collision
			b = b->next;
		}
	}
}

void setballspeed(double speed, double angle)
{
	double xvel = speed * cos(angle);
	// Negate y, because up in math is down in computers
	double yvel = speed * -sin(angle);

	dbprintf(DEBUG_BALL, "\nSPD: %.2f\tANGLE: %.2f\nX: %.2f\tY: %.2f\n",
			 speed, angle, xvel, yvel);

	ball->xvel = xvel;
	ball->yvel = yvel;
	ball->speed = speed;
	ball->angle = angle;
}

void drawtext(const char *string, int x, int y)
{
	SDL_Surface *surf = TTF_RenderText_Solid(hudfont, string, hud_color);
	SDL_Texture *txt  = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect rect = { .x = x, .y = y};
    SDL_QueryTexture(txt, NULL, NULL, &rect.w, &rect.h);
	SDL_RenderCopy(ren, txt, NULL, &rect);

    SDL_DestroyTexture(txt);
    SDL_FreeSurface(surf);
}

/*
 * Return a negative value to stop the game.
 * Any other number will be passed down to `state`.
 */
static double transitiontime = 0;
int tick(double dt, int state)
{
	dbprintf(DEBUG_GAME, "tick(%f, %i)\n", dt, state);
	
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
				return -1;
		else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
			switch (event.key.keysym.scancode) {
			case SDL_SCANCODE_ESCAPE:
			case SDL_SCANCODE_Q:
				return -1;
			default:
				break;
			}
#ifdef CHEATING_FEATURES			
		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			ball->x = x - game_area.x - BALL_SIZE/2;
			ball->y = y - game_area.y - BALL_SIZE/2;
#endif
		}
		

	}

	if (state == 0) {
		// Normal state
		const uint8_t *kbstate = SDL_GetKeyboardState(NULL);
		if (kbstate[SDL_SCANCODE_A] || kbstate[SDL_SCANCODE_LEFT])
			movepaddle(-PADDLE_SPEED*dt);
		if (kbstate[SDL_SCANCODE_D] || kbstate[SDL_SCANCODE_RIGHT])
			movepaddle(PADDLE_SPEED*dt);
#ifdef CHEATING_FEATURES
		if (kbstate[SDL_SCANCODE_W]) {
			// Automatically destroy all bricks
			emptybrickstack();
		}
#endif // CHEATING_FEATURES
		
		moveball(dt);
		
		if (ball->y + BALL_SIZE >= game_area.h)
			state = 1;
		else if (brickstack == NULL)
			state = 2;

		// Set up new level in 1.5 seconds
		if (state)
			transitiontime = 1.5;
		

		drawsprite(paddle, PADDLE_WIDTH, PADDLE_HEIGHT, &paddle_color);
		drawsprite((Sprite*)ball, BALL_SIZE, BALL_SIZE, &ball_color);
	} else if (state == 1) {
		// Losing state
		drawsprite(paddle, PADDLE_WIDTH, PADDLE_HEIGHT, &paddle_color);

		transitiontime -= dt;
		if (transitiontime <= 0) {
			--lives;
			if (lives) {
				resetpaddle();
			} else {
				lives = STARTING_LIVES;
				level = 0;
                score = 0;
				setuplevel(level);
			}
			
			return 0;
		}
	} else if (state == 2) {
		// Winning state
		drawsprite((Sprite*)paddle, PADDLE_WIDTH, PADDLE_HEIGHT, &paddle_color);
		drawsprite((Sprite*)ball, BALL_SIZE, BALL_SIZE, &ball_color);

		transitiontime -= dt;
		if (transitiontime <= 0) {
			setuplevel(++level);
			if (level % LEVELS_PER_LIFE == 0)
				++lives;
			return 0;
		}
	}

	for (Brick *b = brickstack; b; b = b->next)
		drawsprite((Sprite*)b, BRICK_WIDTH, BRICK_HEIGHT,
				   &b->layer->color);

    // Draw aesthetics
    SDL_Rect rect; // for drawing rects
    const int hud_y =  BORDER_SIZE;
    SDL_SetRenderDrawColor(ren, 255, 255, 255, SDL_ALPHA_OPAQUE);

    // Score
    char *scorestr = ecalloc(1, 100);
    snprintf(scorestr, 100, "1UP %010u      LVL %02zu", score, level+1);
    drawtext(scorestr, game_area.x, hud_y + BORDER_SIZE);

    // Lives
    rect.w = rect.h = BALL_SIZE;
    rect.y = hud_y;
    rect.x = GAME_WIDTH;
    for (int i = 0; i < lives; i++) {
        rect.x -= BALL_SIZE + BORDER_SIZE;
        SDL_RenderFillRect(ren, &rect);
    }

	// Draw Border
    SDL_RenderDrawLine(ren, 0, HUD_SPACE, GAME_WIDTH, HUD_SPACE);
	rect = (SDL_Rect){ .x = 0, .y = 0, .w = GAME_WIDTH, .h = GAME_HEIGHT };
	SDL_RenderDrawRect(ren, &rect);
		
	return state;
}

void setuplevel(size_t lvl)
{
	dbprintf(DEBUG_GAME, "setuplevel(%u)\n", lvl);
	const Layer *level = levels[lvl % LEN(levels)];
	
	resetpaddle();
	emptybrickstack();

	// Set up bricks
	for (int l = 0; l < LEVEL_LAYERS; l++) {
		if (!(level[l].speed)) continue; // Size of 0 signify an empty layer
		for (int i = 0; i < NUM_BRICKS; i++)
			makebrick(level+l, i*(BRICK_WIDTH + BRICK_WGAP),
					  l*(BRICK_HEIGHT + BRICK_HGAP));
	}
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
		die("SDL_CreateWindow: %s\n", SDL_GetError());
	}

	ren = SDL_CreateRenderer(win, -1,
							 SDL_RENDERER_ACCELERATED |
							 SDL_RENDERER_PRESENTVSYNC);
	
	if (!ren) {
		cleanup();
		die("SDL_CreateRenderer: %s\n", SDL_GetError());
	}

	if (TTF_Init()) {
		cleanup();
		die("TTF_Init(): %s\n");
	}

	if (!(hudfont = TTF_OpenFont("PressStart2P.ttf", 12))) {
		cleanup();
		die("TTF_OpenFont()\n");
	}
	
	paddle = ecalloc(1, sizeof(Sprite));
	ball   = ecalloc(1, sizeof(Ball));
	setuplevel(level);
}

void run()
{
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	double oldtime = (double)tp.tv_nsec / 1000000000 + tp.tv_sec;
	double dt = 0;
	int state = 0;
	
	while (1) {
		SDL_SetRenderDrawColor(ren, BGCOLOR, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(ren);

		state = tick(dt, state);
		if (state < 0) return;

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
	dbprintf(DEBUG_GAME, "cleanup()\n");
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
    TTF_Quit();
	SDL_Quit();

	// Free up allocated memory like a good kid	
	free(ball);
	free(paddle);
	emptybrickstack();
}

int main(int argc, char **argv)
{
	setup();
	run();
	cleanup();
	
	return 0;
}
