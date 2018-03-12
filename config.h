
#define FPS 60 /* maximum fps */

#define GAME_NAME "Breakout"

// These are three arguments because it's easier to insert them into
//  SDL_SetRenderDrawColor.
#define BGCOLOR 0, 0, 0
#define PADDLE_COLOR 0xee, 0xee, 0xee
#define BALL_COLOR 0xff, 0xff, 0xff

#define GAME_WIDTH  800
#define GAME_HEIGHT 600

#define NUM_BRICKS      10
#define BRICK_WIDTH     70
#define BRICK_HEIGHT    20
#define BRICK_WGAP      10
#define BRICK_HGAP       5
#define BRICK_Y_OFFSET  ((BRICK_HEIGHT + BRICK_HGAP)*3)

#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 20
#define PADDLE_SPEED 300

#define BALL_SIZE         20
#define BALL_SPEED_START 300
#define BALL_ANGLE_START (M_PI * 0.3333) // 60°

// Invisible border that the ball and paddle respects.
#define BORDER_SIZE 5

// Comment this out if you're a real gamer, because real gamers don't
//  need conveniences to debug their programs.
#define CHEATING_FEATURES 1

#define STARTING_LIVES 3

static const SDL_Rect game_area =
	{ .x = BORDER_SIZE, .y = BORDER_SIZE,
	  .w = GAME_WIDTH - BORDER_SIZE*2, .h = GAME_HEIGHT - BORDER_SIZE*2 };

// Each Layer[] is a level. Each Layer of bricks contains
//  its color and the speed it makes the ball go.
// {0}'s signify empty layers
#define LEVEL_LAYERS 16
static const Layer levels[][LEVEL_LAYERS] = {
	{
		{0},
		{0},
		{0},
		{0},
		{  400, { .r = 0xff, .g = 0xff, .b = 0xff } },
		{  400, { .r = 0xff, .g = 0xff, .b = 0xff } },
		{  350, { .r = 0xcc, .g = 0xcc, .b = 0xcc } },
		{  350, { .r = 0xcc, .g = 0xcc, .b = 0xcc } },
		{  300, { .r = 0x88, .g = 0x88, .b = 0x88 } },
		{  300, { .r = 0x88, .g = 0x88, .b = 0x88 } }
	},
	{
		{0},
		{0},
		{0},
		{0},
		{  475, { .r = 0xff, .g = 0x00, .b = 0x00 } },
		{  475, { .r = 0xff, .g = 0x00, .b = 0x00 } },
		{  450, { .r = 0xff, .g = 0xff, .b = 0x00 } },
		{  400, { .r = 0x00, .g = 0xff, .b = 0x00 } },
		{  400, { .r = 0x00, .g = 0xff, .b = 0x00 } },
		{  350, { .r = 0x00, .g = 0xff, .b = 0xff } },
		{  300, { .r = 0x00, .g = 0x00, .b = 0xff } },
		{  300, { .r = 0x00, .g = 0x00, .b = 0xff } }
	},	
	{
		{0},
		{0},
		{  500, { .r = 0xff, .g = 0x00, .b = 0xff } },
		{  500, { .r = 0xff, .g = 0x00, .b = 0xff } },
		{  475, { .r = 0xff, .g = 0x00, .b = 0x00 } },
		{  475, { .r = 0xff, .g = 0x00, .b = 0x00 } },
		{  450, { .r = 0xff, .g = 0xff, .b = 0x00 } },
		{  400, { .r = 0x00, .g = 0xff, .b = 0x00 } },
		{  400, { .r = 0x00, .g = 0xff, .b = 0x00 } },
		{  350, { .r = 0x00, .g = 0xff, .b = 0xff } },
		{  300, { .r = 0x00, .g = 0x00, .b = 0xff } },
		{  300, { .r = 0x00, .g = 0x00, .b = 0xff } }
	}
};

