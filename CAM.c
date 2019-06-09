#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <curses.h>
#include "CAM.h"

//Monospace font information
#include "font.h"

unsigned char CAM_init(){
	unsigned char i;
	unsigned char j;
	unsigned char k;
	unsigned char current_color;
	uint64_t current_line;

	if(!has_colors() || COLOR_PAIRS < 65){
		return 1;
	}

	current_color = 1;
	for(i = 0; i < 8; i++){
		for(j = 0; j < 8; j++){
			init_pair(current_color, i, j);
			current_color++;
		}
	}

	for(i = 0; i < 96; i++){
		for(j = 0; j < 13; j++){
			current_line = 0;
			for(k = 0; k < 8; k++){
				current_line <<= 8;
				current_line |= ascii_chars[i][j];
			}
			ascii_chars64[i][12 - j] = current_line;
		}
	}

	return 0;
}

CAM_screen *CAM_screen_create(WINDOW *parent, unsigned int char_width, unsigned int char_height){
	CAM_screen *output;

	output = malloc(sizeof(CAM_screen));
	output->parent = parent;
	output->char_width = char_width;
	output->char_height = char_height;
	output->width = char_width*8;
	output->height = char_height*13;
	output->do_update = calloc(char_width*char_height, sizeof(unsigned char));
	output->current_characters = malloc(char_width*char_height*sizeof(char));
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

void CAM_set_pix(CAM_screen *s, unsigned int x, unsigned int y, unsigned int color){
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

void CAM_fill(CAM_screen *s, unsigned int color){
	unsigned int i;
	unsigned int j;
	unsigned int k;

	for(i = 0; i < s->char_width; i++){
		for(j = 0; j < s->char_height; j++){
			for(k = 0; k < 13; k++){
				s->current_buffer[s->char_width*j + i][k] = 0xFFULL<<(8*color);
			}
			s->current_characters[s->char_width*j + i] = ' ';
			s->background[s->char_width*j + i] = color;
		}
	}
}

void CAM_rect(CAM_screen *s, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char color){
	unsigned int x;
	unsigned int y;

	for(x = x1; x <= x2; x++){
		for(y = y1; y <= y2; y++){
			CAM_set_pix(s, x, y, color);
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
	unsigned char char_x;
	unsigned char char_y;

	move(0, 0);
	for(char_y = 0; char_y < s->char_height; char_y++){
		for(char_x = 0; char_x < s->char_width; char_x++){
			if(s->do_update[s->char_width*char_y + char_x]){
				CAM_update_char(s, char_x, char_y);
				s->do_update[s->char_width*char_y + char_x] = 0;
			}
			attron(COLOR_PAIR(s->foreground[s->char_width*char_y + char_x]*8 + s->background[s->char_width*char_y + char_x] + 1));
			wprintw(s->parent, "%c", s->current_characters[s->char_width*char_y + char_x]);
		}
		wprintw(s->parent, "\n");
	}
}

