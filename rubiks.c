#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include "CAM.h"
#include "polygon.h"

triangle create_triangle(vector3d a, vector3d b, vector3d c, unsigned char color){
	return create_shape(a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z, color);
}

void create_cube(triangle *t, vector3d center, double width, unsigned char color_top, unsigned char color_bottom, unsigned char color_left, unsigned char color_right, unsigned char color_front, unsigned char color_back){
	vector3d tlf;
	vector3d tlb;
	vector3d trf;
	vector3d trb;
	vector3d blf;
	vector3d blb;
	vector3d brf;
	vector3d brb;

	tlf = (vector3d) {.x = center.x - width/2, .y = center.y - width/2, .z = center.z - width/2};
	tlb = (vector3d) {.x = center.x - width/2, .y = center.y - width/2, .z = center.z + width/2};
	trf = (vector3d) {.x = center.x + width/2, .y = center.y - width/2, .z = center.z - width/2};
	trb = (vector3d) {.x = center.x + width/2, .y = center.y - width/2, .z = center.z + width/2};
	blf = (vector3d) {.x = center.x - width/2, .y = center.y + width/2, .z = center.z - width/2};
	blb = (vector3d) {.x = center.x - width/2, .y = center.y + width/2, .z = center.z + width/2};
	brf = (vector3d) {.x = center.x + width/2, .y = center.y + width/2, .z = center.z - width/2};
	brb = (vector3d) {.x = center.x + width/2, .y = center.y + width/2, .z = center.z + width/2};

	t[0] = create_triangle(tlf, tlb, trb, color_top);
	t[1] = create_triangle(tlf, trb, trf, color_top);
	t[2] = create_triangle(tlf, blb, tlb, color_left);
	t[3] = create_triangle(tlf, blf, blb, color_left);
	t[4] = create_triangle(trf, brb, trb, color_right);
	t[5] = create_triangle(trf, brf, brb, color_right);
	t[6] = create_triangle(brb, blf, brf, color_bottom);
	t[7] = create_triangle(blf, blb, brb, color_bottom);
	t[8] = create_triangle(trf, tlf, blf, color_front);
	t[9] = create_triangle(brf, trf, blf, color_front);
	t[10] = create_triangle(tlb, blb, trb, color_back);
	t[11] = create_triangle(trb, blb, brb, color_back);
}

triangle triangles[12];

int main(int argc, char **argv){
	CAM_screen *screen;

	initscr();
	refresh();
	start_color();
	CAM_init(1);
	screen = CAM_screen_create(stdscr, COLS - 1, LINES);
	create_cube(triangles, (vector3d) {.x = 0, .y = 0, .z = 2}, 1, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_BLACK, COLOR_WHITE);
	//triangles[0] = create_shape(-1, -1, 3, 1, -1, 3, 0, 1, 3, COLOR_YELLOW);
	//triangles[1] = create_shape(1, -1, 3, 1, 1, 3, 2, -1, 3, COLOR_BLUE);
	//triangles[0] = create_triangle((vector3d) {.x = -1, .y = -1, .z = 3}, (vector3d) {.x = 1, .y = -1, .z = 3}, (vector3d) {.x = 0, .y = 1, .z = 3}, COLOR_YELLOW);
	//triangles[1] = create_triangle((vector3d) {.x = 1, .y = -1, .z = 3}, (vector3d) {.x = 1, .y = 1, .z = 3}, (vector3d) {.x = 2, .y = -1, .z = 3}, COLOR_BLUE);
	CAM_fill(screen, COLOR_WHITE);
	CAM_update(screen);
	refresh();
	draw_shapes(screen, 250, triangles, 12);
	CAM_update(screen);
	refresh();
	getch();
	CAM_screen_free(screen);
	endwin();
}

