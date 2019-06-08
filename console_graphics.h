#include <stdlib.h>
#include <stdint.h>

typedef struct screen screen;

struct screen{
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

