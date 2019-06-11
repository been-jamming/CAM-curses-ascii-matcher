#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <complex.h>
#include <math.h>
#include "CAM.h"

unsigned char color_scheme[7] = {COLOR_RED, COLOR_YELLOW, COLOR_WHITE, COLOR_GREEN, COLOR_MAGENTA, COLOR_CYAN, COLOR_BLUE};

unsigned int mandelbrot_iterations(complex double c, unsigned int max_iterations){
	complex double z = 0;
	unsigned int i;

	for(i = 0; i < max_iterations; i++){
		z = z*z + c;
		if(cabs(z) > 2){
			return i;
		}
	}

	return max_iterations;
}

void render(CAM_screen *s, unsigned int (*iterations)(complex double, unsigned int), complex double center, double width, unsigned int max_iterations){
	unsigned int x;
	unsigned int y;
	complex double current_point;
	double height;
	unsigned int num_iterations;
	unsigned char point_color;

	height = ((double) s->height)/s->width*width;

	for(y = 0; y < s->height; y++){
		for(x = 0; x < s->width; x++){
			current_point = ((double) x)/(s->width - 1)*width - width/2 + I*(height/2 - ((double) y)/(s->height - 1)*height) + center;
			num_iterations = iterations(current_point, max_iterations);
			if(num_iterations == max_iterations){
				point_color = COLOR_BLACK;
			} else {
				point_color = color_scheme[((unsigned int) floor(sqrt(num_iterations)))%7];
			}

			CAM_set_pix(s, x, y, point_color);
		}
	}
}

int main(){
	CAM_screen *s;
	int key_hit = '\0';
	unsigned int num_iterations = 50;
	double width = 4;
	complex double center = 0;

	initscr();
	start_color();
	CAM_init();
	keypad(stdscr, TRUE);

	s = CAM_screen_create(stdscr, COLS - 1, LINES);
	while(key_hit != 'q'){
		render(s, mandelbrot_iterations, center, width, num_iterations);
		CAM_update(s);
		refresh();
		key_hit = getch();
		switch(key_hit){
			case KEY_UP:
				center += I*width/4;
				break;
			case KEY_DOWN:
				center -= I*width/4;
				break;
			case KEY_LEFT:
				center -= width/4;
				break;
			case KEY_RIGHT:
				center += width/4;
				break;
			case '+':
				width /= 4;
				break;
			case '-':
				width *= 4;
				break;
			case '>':
				num_iterations *= 2;
				break;
			case '<':
				num_iterations /= 2;
		}
	}
	CAM_screen_free(s);
	endwin();
}

