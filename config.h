
#define FPS 60 /* maximum fps */

#define GAME_NAME "Breakout"

#define BGCOLOR 0, 0, 0

#define GAME_WIDTH  800
#define GAME_HEIGHT 600

#define NUM_BRICKS   10
#define BRICK_WIDTH  69
#define BRICK_HEIGHT 20
#define BRICK_WGAP   10
#define BRICK_HGAP    5

#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 20
#define PADDLE_SPEED 300

#define BALL_RADIUS       10
#define BALL_SPEED_START 300

#define BORDER_SIZE 5

static const Color paddle_color = { .r = 0x22, .g = 0x44, .b = 0xbb };
static const Color ball_color   = { .r = 0xff, .g = 0xff, .b = 0xff };

static const Layer layers[] = {
	{  800, { .r = 0xff, .g = 0x00, .b = 0x00 } },
	{  800, { .r = 0xff, .g = 0x00, .b = 0x00 } },
	{  700, { .r = 0xff, .g = 0xff, .b = 0x00 } },
	{  500, { .r = 0x00, .g = 0xff, .b = 0x00 } },
	{  500, { .r = 0x00, .g = 0xff, .b = 0x00 } },
	{  400, { .r = 0x00, .g = 0xff, .b = 0xff } },
	{  300, { .r = 0x00, .g = 0x00, .b = 0xff } },
	{  300, { .r = 0x00, .g = 0x00, .b = 0xff } }
};
