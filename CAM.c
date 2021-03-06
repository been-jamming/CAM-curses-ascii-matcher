#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <curses.h>
#include "CAM.h"

//Monospace font information
#include "font.h"

static unsigned int CAM_color_start;
static unsigned char CAM_do_pixel_handler;
static unsigned char (*CAM_pixel_handler)(unsigned int x, unsigned int y, unsigned char COLOR);

struct bresenham_state{
	int x0;
	int x1;
	int y0;
	int y1;
	int diff;
	int dx;
	int dy;
	int x_inc;
	int current_x;
	int current_y;
};

static struct bresenham_state create_bresenham_state(int x0, int y0, int x1, int y1){
	int temp;
	struct bresenham_state output;

	if(y0 > y1){
		temp = x1;
		x1 = x0;
		x0 = temp;
		temp = y1;
		y1 = y0;
		y0 = temp;
	}

	output = (struct bresenham_state) {.x0 = x0, .y0 = y0, .x1 = x1, .y1 = y1};
	output.dx = x1 - x0;
	output.dy = y1 - y0;
	if(output.dx < 0){
		output.x_inc = -1;
		output.dx = -output.dx;
	} else {
		output.x_inc = 1;
	}
	output.diff = 2*output.dy - output.dx;
	output.current_x = x0;
	output.current_y = y0;

	return output;
}

static void next_bresenham_state(struct bresenham_state *state){
	if(state->diff >= 0){
		state->current_x += state->x_inc;
		state->diff -= 2*state->dy;
	}
	if(state->diff < 0){
		state->current_y++;
		state->diff += 2*state->dx;
	}
}

unsigned char CAM_init(unsigned int color_start){
	unsigned char i;
	unsigned char j;
	unsigned char k;
	unsigned char current_color;
	uint64_t current_line;

	if(!has_colors()){
		return 1;
	}

	current_color = color_start;
	for(i = 0; i < 8; i++){
		for(j = 0; j < 8; j++){
			init_pair(current_color, i, j);
			current_color++;
		}
	}

	CAM_color_start = color_start;

	for(i = 0; i < 95; i++){
		for(j = 0; j < 13; j++){
			current_line = 0;
			for(k = 0; k < 8; k++){
				current_line <<= 8;
				current_line |= ascii_chars[i][j];
			}
			ascii_chars64[i][12 - j] = current_line;
		}
	}

	CAM_do_pixel_handler = 0;

	return 0;
}

CAM_screen *CAM_screen_create(WINDOW *parent, unsigned int char_width, unsigned int char_height){
	CAM_screen *output;

	output = malloc(sizeof(CAM_screen));
	if(!output){
		return NULL;
	}
	output->parent = parent;
	output->char_width = char_width;
	output->char_height = char_height;
	output->width = char_width*8;
	output->height = char_height*13;
	output->do_update = calloc(char_width*char_height, sizeof(unsigned char));
	output->current_characters = malloc(char_width*char_height*sizeof(char));
	if(!output->current_characters){
		free(output);
		return NULL;
	}
	memset(output->current_characters, ' ', char_width*char_height*sizeof(char));
	output->foreground = calloc(char_width*char_height, sizeof(unsigned char));
	output->background = calloc(char_width*char_height, sizeof(unsigned char));
	output->current_buffer = calloc(char_width*char_height, sizeof(uint64_t)*13);

	return output;
}

void CAM_screen_free(CAM_screen *s){
	free(s->do_update);
	free(s->current_characters);
	free(s->foreground);
	free(s->background);
	free(s->current_buffer);
	free(s);
}

void CAM_dirty_screen(CAM_screen *s){
	int char_x;
	int char_y;

	for(char_x = 0; char_x < s->char_width; char_x++){
		for(char_y = 0; char_y < s->char_height; char_y++){
			s->do_update[s->char_width*char_y + char_x] = 1;
		}
	}
}

void CAM_set_pixel_handler(unsigned char (*pixel_handler)(unsigned int x, unsigned int y, unsigned char color)){
	CAM_pixel_handler = pixel_handler;
	CAM_do_pixel_handler = 1;
}

void CAM_unset_pixel_handler(){
	CAM_do_pixel_handler = 0;
}

void CAM_set_pix(CAM_screen *s, unsigned int x, unsigned int y, unsigned char color){
	unsigned int char_x;
	unsigned int char_y;
	unsigned char char_x_offset;
	unsigned char char_y_offset;

	char_x = x/8;
	char_x_offset = x%8;
	char_y = y/13;
	char_y_offset = y%13;

	s->current_buffer[s->char_width*char_y + char_x][char_y_offset] &= ~(0x8080808080808080ULL>>char_x_offset);
	s->current_buffer[s->char_width*char_y + char_x][char_y_offset] |= 0x80ULL<<(8*color)>>char_x_offset;
	s->do_update[s->char_width*char_y + char_x] = 1;
}

void CAM_handler_set_pix(CAM_screen *s, unsigned int x, unsigned int y, unsigned char color){
	unsigned int char_x;
	unsigned int char_y;
	unsigned char char_x_offset;
	unsigned char char_y_offset;

	if(CAM_pixel_handler(x, y, color)){
		char_x = x/8;
		char_x_offset = x%8;
		char_y = y/13;
		char_y_offset = y%13;

		s->current_buffer[s->char_width*char_y + char_x][char_y_offset] &= ~(0x8080808080808080ULL>>char_x_offset);
		s->current_buffer[s->char_width*char_y + char_x][char_y_offset] |= 0x80ULL<<(8*color)>>char_x_offset;
		s->do_update[s->char_width*char_y + char_x] = 1;
	}
}

unsigned char CAM_get_pix(CAM_screen *s, unsigned int x, unsigned int y){
	unsigned int char_x;
	unsigned int char_y;
	unsigned char char_x_offset;
	unsigned char char_y_offset;
	uint64_t color_bits;
	unsigned char color = 0;

	char_x = x/8;
	char_x_offset = x%8;
	char_y = y/13;
	char_y_offset = y%13;

	color_bits = s->current_buffer[s->char_width*char_y + char_x][char_y_offset]&(0x8080808080808080ULL>>char_x_offset);

	if(color_bits&0xFFFFFFFF00000000ULL){
		color += 4;
	}
	if(color_bits&0xFFFF0000FFFF0000ULL){
		color += 2;
	}
	if(color_bits&0xFF00FF00FF00FF00ULL){
		color += 1;
	}

	return color;
}

void CAM_fill(CAM_screen *s, unsigned int color){
	unsigned int i;
	unsigned int j;
	unsigned char k;

	if(!CAM_do_pixel_handler){
		for(i = 0; i < s->char_width; i++){
			for(j = 0; j < s->char_height; j++){
				for(k = 0; k < 13; k++){
					s->current_buffer[s->char_width*j + i][k] = 0xFFULL<<(8*color);
				}
				s->current_characters[s->char_width*j + i] = ' ';
				s->background[s->char_width*j + i] = color;
			}
		}
	} else {
		for(i = 0; i < s->width; i++){
			for(j = 0; j < s->height; j++){
				CAM_handler_set_pix(s, i, j, color);
			}
		}
	}
}

void CAM_fill_char(CAM_screen *s, unsigned int char_x, unsigned int char_y, unsigned char color){
	unsigned int i;
	unsigned int j;

	if(!CAM_do_pixel_handler){
		for(i = 0; i < 13; i++){
			s->current_buffer[s->char_width*char_y + char_x][i] = 0xFFULL<<(8*color);
		}
		s->current_characters[s->char_width*char_y + char_x] = ' ';
		s->background[s->char_width*char_y + char_x] = color;
		s->do_update[s->char_width*char_y + char_x] = 0;
	} else {
		for(i = char_x*8; i <= char_x*8 + 7; i++){
			for(j = char_y*13; j <= char_y*13 + 12; j++){
				CAM_handler_set_pix(s, i, j, color);
			}
		}
	}
}

void CAM_vertical_line(CAM_screen *s, int x, int y0, int y1, unsigned char color){
	int y;

	if(y0 > y1){
		y = y0;
		y0 = y1;
		y1 = y;
	}

	if(x < 0 || x >= s->width || y0 >= s->height || y1 < 0){
		return;
	}
	if(y0 < 0){
		y0 = 0;
	}
	if(y1 >= s->height){
		y1 = s->height - 1;
	}

	for(y = y0; y <= y1; y++){
		if(!CAM_do_pixel_handler){
			CAM_set_pix(s, x, y, color);
		} else {
			CAM_handler_set_pix(s, x, y, color);
		}
	}
}

void CAM_horizontal_line(CAM_screen *s, int x0, int x1, int y, unsigned char color){
	unsigned char mask_x0;
	unsigned char mask_x1;
	unsigned char mask;
	uint64_t mask64;
	uint64_t mask64_x0 = 0;
	uint64_t mask64_x1 = 0;
	unsigned int char_x0;
	unsigned int char_x1;
	unsigned int char_y;
	unsigned int i;
	unsigned int temp_x;

	if(y < 0 || y >= s->height){
		return;
	}

	if(x0 > x1){
		temp_x = x0;
		x0 = x1;
		x1 = temp_x;
	}

	if(x0 >= s->width){
		return;
	}
	if(x0 < 0){
		x0 = 0;
	}
	if(x1 < 0){
		return;
	}
	if(x1 >= s->width){
		x1 = s->width - 1;
	}

	if(!CAM_do_pixel_handler){
		mask_x0 = 0xFF>>(x0%8);
		mask_x1 = 0xFF<<(7 - x1%8);

		for(i = 0; i < 8; i++){
			mask64_x0 <<= 8;
			mask64_x1 <<= 8;

			mask64_x0 |= mask_x0;
			mask64_x1 |= mask_x1;
		}

		char_x0 = x0/8;
		char_x1 = x1/8;
		char_y = y/13;

		for(i = char_x0; i <= char_x1; i++){
			mask = 0xFF;
			mask64 = 0xFFFFFFFFFFFFFFFFULL;
			if(i == char_x0){
				mask &= mask_x0;
				mask64 &= mask64_x0;
			}
			if(i == char_x1){
				mask &= mask_x1;
				mask64 &= mask64_x1;
			}
			s->current_buffer[s->char_width*char_y + i][y%13] &= ~mask64;
			s->current_buffer[s->char_width*char_y + i][y%13] |= ((uint64_t) mask)<<(8*color);
			s->do_update[s->char_width*char_y + i] = 1;
		}
	} else {
		for(i = x0; i <= x1; i++){
			CAM_handler_set_pix(s, i, y, color);
		}
	}
}

void CAM_line(CAM_screen *s, int x0, int y0, int x1, int y1, unsigned char color){
	struct bresenham_state state;

	state = create_bresenham_state(x0, y0, x1, y1);
	if(!CAM_do_pixel_handler){
		while(state.current_x != state.x1){
			CAM_set_pix(s, state.current_x, state.current_y, color);
			next_bresenham_state(&state);
		}
	} else {
		while(state.current_x != state.x1){
			CAM_handler_set_pix(s, state.current_x, state.current_y, color);
			next_bresenham_state(&state);
		}
	}
	CAM_set_pix(s, state.current_x, state.current_y, color);
}

void CAM_circle(CAM_screen *s, int x0, int y0, int radius, unsigned char color){
	int x;
	int y;
	int x_change;
	int y_change;
	int radius_error;

	x = radius;
	y = 0;
	x_change = 1 - 2*radius;
	y_change = 1;
	radius_error = 0;
	while(x >= y){
		CAM_horizontal_line(s, x0 - x, x0 + x, y0 + y, color);
		CAM_horizontal_line(s, x0 - x, x0 + x, y0 - y, color);
		CAM_horizontal_line(s, x0 - y, x0 + y, y0 + x, color);
		CAM_horizontal_line(s, x0 - y, x0 + y, y0 - x, color);
		y++;
		radius_error += y_change;
		y_change += 2;
		if(2*radius_error + x_change > 0){
			x--;
			radius_error += x_change;
			x_change += 2;
		}
	}
}

void CAM_flat_triangle(CAM_screen *s, int corner_x, int corner_y, int x0, int x1, int y, unsigned char color){
	struct bresenham_state state0;
	struct bresenham_state state1;
	int current_y;

	if(corner_y == y || x0 == x1){
		return;
	}

	state0 = create_bresenham_state(corner_x, corner_y, x0, y);
	state1 = create_bresenham_state(corner_x, corner_y, x1, y);
	current_y = state0.current_y;

	while((current_y <= corner_y && current_y >= y) || (current_y <= y && current_y >= corner_y)){
		while(state0.current_y == current_y){
			next_bresenham_state(&state0);
		}
		while(state1.current_y == current_y){
			next_bresenham_state(&state1);
		}
		CAM_horizontal_line(s, state0.current_x, state1.current_x, current_y, color);
		if(state0.dy > 0){
			current_y++;
		} else {
			current_y--;
		}
	}
}

void CAM_triangle(CAM_screen *s, int x0, int y0, int x1, int y1, int x2, int y2, unsigned char color){
	int middle_x = 0;
	int middle_y = 0;
	int corner1_x = 0;
	int corner1_y = 0;
	int corner2_x = 0;
	int corner2_y = 0;
	int split_x = 0;

	if((y0 >= y1 && y0 <= y2) || (y0 >= y2 && y0 <= y1)){
		middle_x = x0;
		middle_y = y0;
		corner1_x = x1;
		corner1_y = y1;
		corner2_x = x2;
		corner2_y = y2;
	} else if((y1 >= y0 && y1 <= y2) || (y1 >= y2 && y1 <= y0)){
		middle_x = x1;
		middle_y = y1;
		corner1_x = x0;
		corner1_y = y0;
		corner2_x = x2;
		corner2_y = y2;
	} else if((y2 >= y0 && y2 <= y1) || (y2 >= y1 && y2 <= y0)){
		middle_x = x2;
		middle_y = y2;
		corner1_x = x0;
		corner1_y = y0;
		corner2_x = x1;
		corner2_y = y1;
	}

	if(middle_y == corner1_y){
		CAM_flat_triangle(s, corner2_x, corner2_y, middle_x, corner1_x, corner1_y, color);
		return;
	} else if(middle_y == corner2_y){
		CAM_flat_triangle(s, corner1_x, corner1_y, middle_x, corner2_x, corner2_y, color);
		return;
	}

	split_x = corner1_x + (middle_y - corner1_y)*(corner1_x - corner2_x)/(corner1_y - corner2_y);
	CAM_flat_triangle(s, corner1_x, corner1_y, middle_x, split_x, middle_y, color);
	CAM_flat_triangle(s, corner2_x, corner2_y, middle_x, split_x, middle_y, color);
}

void CAM_rect(CAM_screen *s, int x0, int y0, int x1, int y1, unsigned char color){
	unsigned int char_x0;
	unsigned int char_y0;
	unsigned int char_x1;
	unsigned int char_y1;
	unsigned char edge_mask_x0;
	unsigned char edge_mask_x1;
	uint64_t edge_mask_x0_64 = 0;
	uint64_t edge_mask_x1_64 = 0;
	unsigned int char_x;
	unsigned int char_y;
	unsigned char current_mask;
	uint64_t current_mask_64;
	int min_y;
	int max_y;
	unsigned char i;

	if(!CAM_do_pixel_handler){
		edge_mask_x0 = 0xFF>>(x0%8);
		edge_mask_x1 = (0xFF<<(7 - x1%8))&0xFF;

		for(i = 0; i < 8; i++){
			edge_mask_x0_64 <<= 8;
			edge_mask_x1_64 <<= 8;

			edge_mask_x0_64 |= edge_mask_x0;
			edge_mask_x1_64 |= edge_mask_x1;
		}

		char_x0 = x0/8;
		char_y0 = y0/13;
		char_x1 = x1/8;
		char_y1 = y1/13;

		for(char_y = char_y0; char_y <= char_y1; char_y++){
			for(char_x = char_x0; char_x <= char_x1; char_x++){
				if(char_x == char_x0 || char_x == char_x1 || char_y == char_y0 || char_y == char_y1){
					if(char_x == char_x0 && char_x == char_x1){
						current_mask = edge_mask_x0&edge_mask_x1;
						current_mask_64 = edge_mask_x0_64&edge_mask_x1_64;
					} else if(char_x == char_x0){
						current_mask = edge_mask_x0;
						current_mask_64 = edge_mask_x0_64;
					} else if(char_x == char_x1){
						current_mask = edge_mask_x1;
						current_mask_64 = edge_mask_x1_64;
					} else {
						current_mask = 0xFF;
						current_mask_64 = 0xFFFFFFFFFFFFFFFFULL;
					}

					min_y = 0;
					max_y = 12;
					if(char_y == char_y0){
						min_y = y0%13;
					}
					if(char_y == char_y1){
						max_y = y1%13;
					}

					for(i = min_y; i <= max_y; i++){
						s->current_buffer[s->char_width*char_y + char_x][i] &= ~current_mask_64;
						s->current_buffer[s->char_width*char_y + char_x][i] |= ((uint64_t) current_mask)<<(8*color);
						s->do_update[s->char_width*char_y + char_x] = 1;
					}
				} else {
					CAM_fill_char(s, char_x, char_y, color);
				}
			}
		}
	} else {
		while(x0 <= x1){
			while(y0 <= y1){
				CAM_handler_set_pix(s, x0, y0, color);
				y0++;
			}
			x0++;
		}
	}
}

void CAM_update_char(CAM_screen *s, unsigned int char_x, unsigned int char_y){
	uint64_t current_counts;
	uint64_t current_row_count;
	uint64_t reverse_counts;
	uint64_t reverse_row_count;
	unsigned char i;
	unsigned char j;
	int best_score;
	unsigned char best_foreground = 0;
	unsigned char best_background = 0;
	unsigned char foreground = 0;
	unsigned char background = 0;
	int greatest_count;
	int greatest_reverse;
	char best_char = ' ';

	best_score = -1;
	for(i = 0; i < 95; i++){
		//Calculate the number of incorrect pixels represented by each character/color combination
		current_counts = 0;
		reverse_counts = 0;
		for(j = 0; j < 13; j++){
			current_row_count = ascii_chars64[i][j]&s->current_buffer[s->char_width*char_y + char_x][j];
			current_row_count = ((current_row_count&0xAAAAAAAAAAAAAAAAULL)>>1) + (current_row_count&0x5555555555555555ULL);
			current_row_count = ((current_row_count&0xCCCCCCCCCCCCCCCCULL)>>2) + (current_row_count&0x3333333333333333ULL);
			current_row_count = ((current_row_count&0xF0F0F0F0F0F0F0F0ULL)>>4) + (current_row_count&0x0F0F0F0F0F0F0F0FULL);
			current_counts += current_row_count;

			reverse_row_count = (~ascii_chars64[i][j])&s->current_buffer[s->char_width*char_y + char_x][j];
			reverse_row_count = ((reverse_row_count&0xAAAAAAAAAAAAAAAAULL)>>1) + (reverse_row_count&0x5555555555555555ULL);
			reverse_row_count = ((reverse_row_count&0xCCCCCCCCCCCCCCCCULL)>>2) + (reverse_row_count&0x3333333333333333ULL);
			reverse_row_count = ((reverse_row_count&0xF0F0F0F0F0F0F0F0ULL)>>4) + (reverse_row_count&0x0F0F0F0F0F0F0F0FULL);
			reverse_counts += reverse_row_count;
		}

		greatest_count = -1;
		greatest_reverse = -1;
		for(j = 0; j < 8; j++){
			if((int) (current_counts&0xFF) > greatest_count){
				greatest_count = current_counts&0xFF;
				foreground = j;
			}
			if((int) (reverse_counts&0xFF) > greatest_reverse){
				greatest_reverse = reverse_counts&0xFF;
				background = j;
			}

			current_counts >>= 8;
			reverse_counts >>= 8;
		}

		if(best_score < greatest_count + greatest_reverse){
			best_score = greatest_count + greatest_reverse;
			best_foreground = foreground;
			best_background = background;
			best_char = ' ' + i;
			if(best_score == 104){
				break;
			}
		}
	}

	s->current_characters[s->char_width*char_y + char_x] = best_char;
	s->foreground[s->char_width*char_y + char_x] = best_foreground;
	s->background[s->char_width*char_y + char_x] = best_background;
	s->do_update[s->char_width*char_y + char_x] = 0;
}

void CAM_update(CAM_screen *s){
	unsigned int char_x;
	unsigned int char_y;

	wmove(s->parent, 0, 0);
	for(char_y = 0; char_y < s->char_height; char_y++){
		for(char_x = 0; char_x < s->char_width; char_x++){
			if(s->do_update[s->char_width*char_y + char_x]){
				CAM_update_char(s, char_x, char_y);
				s->do_update[s->char_width*char_y + char_x] = 0;
			}
			wattron(s->parent, COLOR_PAIR(s->foreground[s->char_width*char_y + char_x]*8 + s->background[s->char_width*char_y + char_x] + CAM_color_start));
			wprintw(s->parent, "%c", s->current_characters[s->char_width*char_y + char_x]);
		}
		wprintw(s->parent, "\n");
	}
}

