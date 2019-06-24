#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "curses.h"
#include "CAM.h"
#include "polygon.h"

static vector2d get_screen_point(vector3d v, double fov, double center_x, double center_y){
	vector2d output;

	output.x = v.x*fov/v.z + center_x;
	output.y = v.y*fov/v.z + center_y;

	return output;
}

double dot_3d(vector3d a, vector3d b){
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

void draw_triangle(CAM_screen *s, triangle tri, double fov, unsigned char color){
	vector2d point0;
	vector2d point1;
	vector2d point2;

	point0 = get_screen_point(tri.p0, fov, s->width/2, s->height/2);
	point1 = get_screen_point(tri.p1, fov, s->width/2, s->height/2);
	point2 = get_screen_point(tri.p2, fov, s->width/2, s->height/2);

	CAM_triangle(s, (int) point0.x, (int) point0.y, (int) point1.x, (int) point1.y, (int) point2.x, (int) point2.y, color);
}

void calculate_normal_triangle(triangle *t){
	t->normal.x = (t->p0.y - t->p1.y)*(t->p0.z - t->p2.z) - (t->p0.z - t->p1.z)*(t->p0.y - t->p2.y);
	t->normal.y = (t->p0.z - t->p1.z)*(t->p0.x - t->p2.x) - (t->p0.x - t->p1.x)*(t->p0.z - t->p2.z);
	t->normal.z = (t->p0.x - t->p1.x)*(t->p0.y - t->p2.y) - (t->p0.y - t->p1.y)*(t->p0.x - t->p2.x);

	t->center.x = (t->p0.x + t->p1.x + t->p2.x)/3;
	t->center.y = (t->p0.y + t->p1.y + t->p2.y)/3;
	t->center.z = (t->p0.z + t->p1.z + t->p2.z)/3;
}

void rotate_vector3d_x(vector3d *v, vector3d center, double angle){
	double new_y;
	double new_z;
	double c;
	double s;

	v->y -= center.y;
	v->z -= center.z;
	c = cos(angle);
	s = sin(angle);

	new_y = c*v->y - s*v->z + center.y;
	new_z = s*v->y + c*v->z + center.z;

	v->y = new_y;
	v->z = new_z;
}

void rotate_vector3d_y(vector3d *v, vector3d center, double angle){
	double new_x;
	double new_z;
	double c;
	double s;

	v->x -= center.x;
	v->z -= center.z;
	c = cos(angle);
	s = sin(angle);

	new_x = c*v->x - s*v->z + center.x;
	new_z = s*v->x + c*v->z + center.z;

	v->x = new_x;
	v->z = new_z;
}

void rotate_vector3d_z(vector3d *v, vector3d center, double angle){
	double new_x;
	double new_y;
	double c;
	double s;

	v->x -= center.x;
	v->y -= center.y;
	c = cos(angle);
	s = sin(angle);

	new_x = c*v->x - s*v->y + center.x;
	new_y = s*v->x + c*v->y + center.y;

	v->x = new_x;
	v->y = new_y;
}

void rotate_triangle_x(triangle *t, vector3d center, double angle){
	rotate_vector3d_x(&(t->p0), center, angle);
	rotate_vector3d_x(&(t->p1), center, angle);
	rotate_vector3d_x(&(t->p2), center, angle);
}

void rotate_triangle_y(triangle *t, vector3d center, double angle){
	rotate_vector3d_y(&(t->p0), center, angle);
	rotate_vector3d_y(&(t->p1), center, angle);
	rotate_vector3d_y(&(t->p2), center, angle);
}

void rotate_triangle_z(triangle *t, vector3d center, double angle){
	rotate_vector3d_z(&(t->p0), center, angle);
	rotate_vector3d_z(&(t->p1), center, angle);
	rotate_vector3d_z(&(t->p2), center, angle);
}

triangle *create_shape(int x0, int y0, int z0, int x1, int y1, int z1, int x2, int y2, int z2, unsigned char color){
	triangle *output;

	output = malloc(sizeof(triangle));
	output->p0.x = x0; output->p0.y = y0; output->p0.z = z0;
	output->p1.x = x1; output->p1.y = y1; output->p1.z = z1;
	output->p2.x = x2; output->p2.y = y2; output->p2.z = z2;
	output->color = color;
	output->next = NULL;
	return output;
}

void add_shape(triangle **t, int x0, int y0, int z0, int x1, int y1, int z1, int x2, int y2, int z2, unsigned char color){
	triangle *new_shape;

	new_shape = create_shape(x0, y0, z0, x1, y1, z1, x2, y2, z2, color);
	new_shape->next = *t;
	*t = new_shape;
}

void free_shape(triangle *t){
	triangle *next;

	while(t){
		next = t->next;
		free(t);
		t = next;
	}
}

void draw_shape(CAM_screen *s, double fov, triangle *t){
	while(t){
		calculate_normal_triangle(t);
		if(dot_3d(t->center, t->normal) < 0){
			draw_triangle(s, *t, fov, t->color);
		}
		t = t->next;
	}
}

void rotate_shape_x(triangle *t, vector3d center, double angle){
	while(t){
		rotate_triangle_x(t, center, angle);
		t = t->next;
	}
}

void rotate_shape_y(triangle *t, vector3d center, double angle){
	while(t){
		rotate_triangle_y(t, center, angle);
		t = t->next;
	}
}

void rotate_shape_z(triangle *t, vector3d center, double angle){
	while(t){
		rotate_triangle_z(t, center, angle);
		t = t->next;
	}
}

int main(int argc, char **argv){
	CAM_screen *screen;
	triangle *cube;
	vector3d center;

	center = (vector3d) {.x = 0, .y = 0, .z = 3};

	cube = create_shape(1, 1, 2, -1, -1, 2, -1, 1, 2, COLOR_BLUE);
	add_shape(&cube, 1, -1, 2, -1, -1, 2, 1, 1, 2, COLOR_BLUE);
	add_shape(&cube, 1, -1, 4, 1, -1, 2, 1, 1, 4, COLOR_RED);
	add_shape(&cube, 1, 1, 4, 1, -1, 2, 1, 1, 2, COLOR_RED);
	add_shape(&cube, -1, -1, 4, -1, -1, 2, 1, -1, 4, COLOR_WHITE);
	add_shape(&cube, 1, -1, 4, -1, -1, 2, 1, -1, 2, COLOR_WHITE);
	add_shape(&cube, -1, -1, 4, 1, 1, 4, -1, 1, 4, COLOR_CYAN);
	add_shape(&cube, -1, -1, 4, 1, -1, 4, 1, 1, 4, COLOR_CYAN);
	add_shape(&cube, -1, -1, 2, -1, -1, 4, -1, 1, 4, COLOR_GREEN);
	add_shape(&cube, -1, -1, 2, -1, 1, 4, -1, 1, 2, COLOR_GREEN);
	add_shape(&cube, -1, 1, 2, 1, 1, 4, 1, 1, 2, COLOR_YELLOW);
	add_shape(&cube, -1, 1, 2, -1, 1, 4, 1, 1, 4, COLOR_YELLOW);
	initscr();
	refresh();	
	start_color();
	CAM_init(1);
	screen = CAM_screen_create(stdscr, COLS - 1, LINES);
	while(getch() != 'q'){
		CAM_fill(screen, COLOR_BLACK);
		draw_shape(screen, 150, cube);
		CAM_update(screen);
		refresh();
		rotate_shape_x(cube, center, 0.1);
		rotate_shape_y(cube, center, -0.075);
		rotate_shape_z(cube, center, 0.05);
	}

	CAM_screen_free(screen);
	endwin();
	free_shape(cube);
	return 0;
}

