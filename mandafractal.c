#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <complex.h>
#include <math.h>
#include "CAM.h"

unsigned char color_scheme[7] = {COLOR_RED, COLOR_YELLOW, COLOR_WHITE, COLOR_GREEN, COLOR_MAGENTA, COLOR_CYAN, COLOR_BLUE};

complex double global_c = 0;

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

unsigned int julia_iterations(complex double z, unsigned int max_iterations){
	unsigned int i;

	for(i = 0; i < max_iterations; i++){
		z = z*z + global_c;
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
	double old_width = 4;
	complex double old_center = 0;
	unsigned int old_num_iterations = 50;
	unsigned char mode = 1;
	unsigned int (*iterations_func)(complex double, unsigned int) = mandelbrot_iterations;
	unsigned char do_render;

	initscr();
	start_color();
	CAM_init();
	keypad(stdscr, TRUE);

	s = CAM_screen_create(stdscr, COLS - 1, LINES);
	while(key_hit != 'q'){
		render(s, iterations_func, center, width, num_iterations);
		CAM_update(s);
		refresh();
		do{
			key_hit = getch();
			do_render = 1;
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
					break;
				case ' ':
					mode = !mode;
					if(mode){
						width = old_width;
						center = old_center;
						num_iterations = old_num_iterations;
						iterations_func = mandelbrot_iterations;
					} else {
						old_width = width;
						old_center = center;
						old_num_iterations = num_iterations;
						global_c = center;
						width = 4;
						center = 0;
						iterations_func = julia_iterations;
					}
					break;
				case 'q':
					break;
				default:
					do_render = 0;
			}
		} while(!do_render);
	}
	CAM_screen_free(s);
	endwin();

	return 0;
}

