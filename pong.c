#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <curses.h>
#include <math.h>
#include "CAM.h"

CAM_screen *my_screen;
unsigned int player1_y;
unsigned int player2_y;
int ball_x;
int ball_y;
double ball_dx = 1;
double ball_dy = 1;
unsigned int bat_height = 50;
unsigned char quit;

void game_tick(){
	int pressed;
	double r;

	pressed = getch();
	if(pressed == 'e' && player1_y){
		player1_y -= 5;
	} else if(pressed == 'c' && player1_y + bat_height < LINES*13 - 5){
		player1_y += 5;
	}
	if(pressed == KEY_UP && player2_y){
		player2_y -= 5;
	} else if(pressed == KEY_DOWN && player2_y + bat_height < LINES*13 - 5){
		player2_y += 5;
	}
	
	if(pressed == 'q'){
		quit = 1;
	}

	if(ball_dx + ball_x < 0){
		if(ball_y + 5 >= player1_y && ball_y - 5 <= player1_y + bat_height){
			r = ((double) rand())/RAND_MAX;
			ball_dx = -ball_dx + r*r;
		} else {
			ball_dx = -ball_dx;
			quit = 1;
		}
	}
	if(ball_dx + ball_x > COLS*8 - 11){
		if(ball_y + 5 >= player2_y && ball_y - 5 <= player2_y + bat_height){
			r = ((double) rand())/RAND_MAX;
			ball_dx = -ball_dx - r*r;
		} else {
			ball_dx = -ball_dx;
			quit = 1;
		}
	}
	if(ball_dy + ball_y < 0 || ball_dy + ball_y > LINES*13 - 11){
		ball_dy = -ball_dy;
	}

	ball_x += ball_dx;
	ball_y += ball_dy;

	CAM_fill(my_screen, COLOR_WHITE);
	CAM_rect(my_screen, 0, player1_y, 8, player1_y + bat_height, COLOR_BLUE);
	CAM_rect(my_screen, COLS*8 - 9, player2_y, COLS*8 - 1, player2_y + bat_height, COLOR_GREEN);
	CAM_rect(my_screen, ball_x, ball_y, ball_x + 10, ball_y + 10, COLOR_RED);
	CAM_update(my_screen);
	refresh();
}

int main(int argc, char *argv[]){
	initscr();
	refresh();	
	start_color();
	CAM_init();

	struct timespec spec;
	long last_frame;

	srand(time(NULL));
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	my_screen = CAM_screen_create(stdscr, COLS - 1, LINES);
	player1_y = 0;
	player2_y = 0;
	ball_x = 50;
	ball_y = 50;
	quit = 0;
	clock_gettime(CLOCK_MONOTONIC, &spec);
	last_frame = round(spec.tv_nsec/1.0e6) + 1000*spec.tv_sec;

	while(!quit){
		clock_gettime(CLOCK_MONOTONIC, &spec);
		if(round(spec.tv_nsec/1.0e6) + 1000*spec.tv_sec - last_frame > 20){
			last_frame = round(spec.tv_nsec/1.0e6) + 1000*spec.tv_sec;
			game_tick();
		}
	}
	
	endwin();
	CAM_screen_free(my_screen);
}

