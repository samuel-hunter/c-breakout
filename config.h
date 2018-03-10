
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


Layer layers[] = {
	{ 10, { .r = 0xff, .g = 0x00, .b = 0x00 } },
	{  8, { .r = 0xff, .g = 0xff, .b = 0x00 } },
	{  5, { .r = 0x00, .g = 0xff, .b = 0x00 } },
	{  3, { .r = 0x00, .g = 0xff, .b = 0xff } },
	{  3, { .r = 0x00, .g = 0x00, .b = 0xff } }
};
