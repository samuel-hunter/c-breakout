
#define FPS 60 /* maximum fps */

#define GAME_NAME "Breakout"

#define BGCOLOR 0, 0, 0

#define GAME_WIDTH  800
#define GAME_HEIGHT 600

#define NUM_BRICKS      10
#define BRICK_WIDTH     69
#define BRICK_HEIGHT    20
#define BRICK_WGAP      10
#define BRICK_HGAP       5
#define BRICK_Y_OFFSET  ((BRICK_HEIGHT + BRICK_HGAP)*2)

#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 20
#define PADDLE_SPEED 300

#define BALL_RADIUS       10
#define BALL_SPEED_START 300
#define BALL_ANGLE_START (M_PI * 1.6667)

#define BORDER_SIZE 5

static const Color paddle_color = { .r = 0xee, .g = 0xee, .b = 0xee };
static const Color ball_color   = { .r = 0xff, .g = 0xff, .b = 0xff };


static const Layer levels[][16] = {
	{
		{  400, { .r = 0xff, .g = 0xff, .b = 0xff } },
		{  400, { .r = 0xff, .g = 0xff, .b = 0xff } },
		{  350, { .r = 0xcc, .g = 0xcc, .b = 0xcc } },
		{  350, { .r = 0xcc, .g = 0xcc, .b = 0xcc } },
		{  300, { .r = 0x88, .g = 0x88, .b = 0x88 } },
		{  300, { .r = 0x88, .g = 0x88, .b = 0x88 } },
		{0}
	},
	{
		{  475, { .r = 0xff, .g = 0x00, .b = 0x00 } },
		{  475, { .r = 0xff, .g = 0x00, .b = 0x00 } },
		{  450, { .r = 0xff, .g = 0xff, .b = 0x00 } },
		{  400, { .r = 0x00, .g = 0xff, .b = 0x00 } },
		{  400, { .r = 0x00, .g = 0xff, .b = 0x00 } },
		{  350, { .r = 0x00, .g = 0xff, .b = 0xff } },
		{  300, { .r = 0x00, .g = 0x00, .b = 0xff } },
		{  300, { .r = 0x00, .g = 0x00, .b = 0xff } },
		{0}
	},	
	{
		{  500, { .r = 0xff, .g = 0x00, .b = 0xff } },
		{  475, { .r = 0xff, .g = 0x00, .b = 0x00 } },
		{  475, { .r = 0xff, .g = 0x00, .b = 0x00 } },
		{  450, { .r = 0xff, .g = 0xff, .b = 0x00 } },
		{  400, { .r = 0x00, .g = 0xff, .b = 0x00 } },
		{  400, { .r = 0x00, .g = 0xff, .b = 0x00 } },
		{  350, { .r = 0x00, .g = 0xff, .b = 0xff } },
		{  300, { .r = 0x00, .g = 0x00, .b = 0xff } },
		{  300, { .r = 0x00, .g = 0x00, .b = 0xff } },
		{0}
	}
};

