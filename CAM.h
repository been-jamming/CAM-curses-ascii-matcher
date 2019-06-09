#include <stdlib.h>
#include <stdint.h>

typedef struct CAM_screen CAM_screen;

struct CAM_screen{
	WINDOW *parent;
	unsigned int char_width;
	unsigned int char_height;
	unsigned int width;
	unsigned int height;
	unsigned char *do_update;
	char *current_characters;
	unsigned char *foreground;
	unsigned char *background;
	uint64_t (*current_buffer)[13];
};

unsigned char CAM_init();

CAM_screen *CAM_screen_create(WINDOW *parent, unsigned int char_width, unsigned int char_height);

void CAM_screen_free(CAM_screen *s);

void CAM_set_pix(CAM_screen *s, unsigned int x, unsigned int y, unsigned int color);

void CAM_fill(CAM_screen *s, unsigned int color);

void CAM_rect(CAM_screen *s, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char color);

void CAM_update_char(CAM_screen *s, unsigned int char_x, unsigned int char_y);

void CAM_update(CAM_screen *s);

