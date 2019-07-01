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

double cross_2d(vector2d a, vector2d b){
	return a.x*b.y - a.y*b.x;
}

vector2d subtract_2d(vector2d a, vector2d b){
	return (vector2d) {.x = a.x - b.x, .y = a.y - b.y};
}

vector3d multiply_3d(vector3d a, double b){
	return (vector3d) {.x = a.x*b, .y = a.y*b, .z = a.z*b};
}

vector3d add_3d(vector3d a, vector3d b){
	return (vector3d) {.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

vector3d subtract_3d(vector3d a, vector3d b){
	return (vector3d) {.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

unsigned char line_collision(vector2d p0, vector2d p1, vector2d p2, vector2d p3, double *collision_scalar0, double *collision_scalar1){
	vector2d diff0;
	vector2d diff1;
	double denom;

	diff0 = subtract_2d(p1, p0);
	diff1 = subtract_2d(p3, p2);
	denom = cross_2d(diff0, diff1);

	if(denom == 0){
		return 0;
	}

	*collision_scalar0 = cross_2d(subtract_2d(p2, p0), diff1)/denom;
	*collision_scalar1 = cross_2d(subtract_2d(p2, p0), diff0)/denom;

	if(*collision_scalar0 >= 0 && *collision_scalar0 <= 1 && *collision_scalar1 >= 0 && *collision_scalar1 <= 1){
		return 1;
	} else {
		return 0;
	}
}

unsigned char inside_triangle(vector2d p, vector2d v0, vector2d v1, vector2d v2){
	vector2d segment0;
	vector2d segment1;
	vector2d segment2;

	segment0 = subtract_2d(v0, v1);
	segment1 = subtract_2d(v1, v2);
	segment2 = subtract_2d(v2, v0);

	if((cross_2d(segment0, subtract_2d(v0, v2)) < 0) == (cross_2d(segment0, subtract_2d(v0, p)) < 0) && (cross_2d(segment1, subtract_2d(v1, v0)) < 0) == (cross_2d(segment1, subtract_2d(v1, p)) < 0) && (cross_2d(segment2, subtract_2d(v2, v1)) < 0) == (cross_2d(segment2, subtract_2d(v2, p)) < 0)){
		return 1;
	} else {
		return 0;
	}
}

void get_points_triangle(CAM_screen *s, triangle *tri, double fov){
	tri->screen0 = get_screen_point(tri->p0, fov, s->width/2, s->height/2);
	tri->screen1 = get_screen_point(tri->p1, fov, s->width/2, s->height/2);
	tri->screen2 = get_screen_point(tri->p2, fov, s->width/2, s->height/2);
}

void draw_triangle(CAM_screen *s, triangle *tri, double fov, unsigned char color){
	get_points_triangle(s, tri, fov);
	CAM_triangle(s, (int) tri->screen0.x, (int) tri->screen0.y, (int) tri->screen1.x, (int) tri->screen1.y, (int) tri->screen2.x, (int) tri->screen2.y, color);
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

unsigned char triangle_in_front(triangle *tri1, triangle *tri2){
	double collision_scalar0;
	double collision_scalar1;
	vector3d collision_point0;
	vector3d collision_point1;

	if(line_collision(tri1->screen0, tri1->screen1, tri2->screen0, tri2->screen1, &collision_scalar0, &collision_scalar1)){
		collision_point0 = add_3d(multiply_3d(subtract_3d(tri1->p1, tri1->p0), collision_scalar0), tri1->p0);
		collision_point1 = add_3d(multiply_3d(subtract_3d(tri2->p1, tri2->p0), collision_scalar1), tri2->p0);
		if(collision_point0.z < collision_point1.z){
			return 1;
		} else {
			return 0;
		}
	} else if(line_collision(tri1->screen1, tri1->screen2, tri2->screen0, tri2->screen1, &collision_scalar0, &collision_scalar1)){
		collision_point0 = add_3d(multiply_3d(subtract_3d(tri1->p2, tri1->p1), collision_scalar0), tri1->p1);
		collision_point1 = add_3d(multiply_3d(subtract_3d(tri2->p1, tri2->p0), collision_scalar1), tri2->p0);
		if(collision_point0.z < collision_point1.z){
			return 1;
		} else {
			return 0;
		}
	} else if(line_collision(tri1->screen2, tri1->screen0, tri2->screen0, tri2->screen1, &collision_scalar0, &collision_scalar1)){
		collision_point0 = add_3d(multiply_3d(subtract_3d(tri1->p0, tri1->p2), collision_scalar0), tri1->p2);
		collision_point1 = add_3d(multiply_3d(subtract_3d(tri2->p1, tri2->p0), collision_scalar0), tri2->p0);
		if(collision_point0.z < collision_point1.z){
			return 1;
		} else {
			return 0;
		}
	} else if(line_collision(tri1->screen0, tri1->screen1, tri2->screen1, tri2->screen2, &collision_scalar0, &collision_scalar1)){
		collision_point0 = add_3d(multiply_3d(subtract_3d(tri1->p1, tri1->p0), collision_scalar0), tri1->p0);
		collision_point1 = add_3d(multiply_3d(subtract_3d(tri2->p2, tri2->p1), collision_scalar1), tri2->p1);
		if(collision_point0.z < collision_point1.z){
			return 1;
		} else {
			return 0;
		}
	} else if(line_collision(tri1->screen1, tri1->screen2, tri2->screen1, tri2->screen2, &collision_scalar0, &collision_scalar1)){
		collision_point0 = add_3d(multiply_3d(subtract_3d(tri1->p2, tri1->p1), collision_scalar0), tri1->p1);
		collision_point1 = add_3d(multiply_3d(subtract_3d(tri2->p2, tri2->p1), collision_scalar1), tri2->p1);
		if(collision_point0.z < collision_point1.z){
			return 1;
		} else {
			return 0;
		}
	} else if(line_collision(tri1->screen2, tri1->screen0, tri2->screen1, tri2->screen2, &collision_scalar0, &collision_scalar1)){
		collision_point0 = add_3d(multiply_3d(subtract_3d(tri1->p0, tri1->p2), collision_scalar0), tri1->p2);
		collision_point1 = add_3d(multiply_3d(subtract_3d(tri2->p2, tri2->p1), collision_scalar0), tri2->p1);
		if(collision_point0.z < collision_point1.z){
			return 1;
		} else {
			return 0;
		}
	} else if(line_collision(tri1->screen0, tri1->screen1, tri2->screen2, tri2->screen0, &collision_scalar0, &collision_scalar1)){
		collision_point0 = add_3d(multiply_3d(subtract_3d(tri1->p1, tri1->p0), collision_scalar0), tri1->p0);
		collision_point1 = add_3d(multiply_3d(subtract_3d(tri2->p0, tri2->p2), collision_scalar1), tri2->p2);
		if(collision_point0.z < collision_point1.z){
			return 1;
		} else {
			return 0;
		}
	} else if(line_collision(tri1->screen1, tri1->screen2, tri2->screen2, tri2->screen0, &collision_scalar0, &collision_scalar1)){
		collision_point0 = add_3d(multiply_3d(subtract_3d(tri1->p2, tri1->p1), collision_scalar0), tri1->p1);
		collision_point1 = add_3d(multiply_3d(subtract_3d(tri2->p0, tri2->p2), collision_scalar1), tri2->p2);
		if(collision_point0.z < collision_point1.z){
			return 1;
		} else {
			return 0;
		}
	} else if(line_collision(tri1->screen2, tri1->screen0, tri2->screen2, tri2->screen0, &collision_scalar0, &collision_scalar1)){
		collision_point0 = add_3d(multiply_3d(subtract_3d(tri1->p0, tri1->p2), collision_scalar0), tri1->p2);
		collision_point1 = add_3d(multiply_3d(subtract_3d(tri2->p0, tri2->p2), collision_scalar0), tri2->p2);
		if(collision_point0.z < collision_point1.z){
			return 1;
		} else {
			return 0;
		}
	} else if(dot_3d(tri1->p0, tri2->normal) < dot_3d(tri2->p0, tri2->normal)){
		return 1;
	} else {
		return 0;
	}
}

triangle *allocate_shape(int x0, int y0, int z0, int x1, int y1, int z1, int x2, int y2, int z2, unsigned char color){
	triangle *output;

	output = malloc(sizeof(triangle));
	output->p0.x = x0; output->p0.y = y0; output->p0.z = z0;
	output->p1.x = x1; output->p1.y = y1; output->p1.z = z1;
	output->p2.x = x2; output->p2.y = y2; output->p2.z = z2;
	output->color = color;
	output->next = NULL;
	output->draw_flag = 1;
	return output;
}

triangle create_shape(int x0, int y0, int z0, int x1, int y1, int z1, int x2, int y2, int z2, unsigned char color){
	triangle output;

	output.p0.x = x0; output.p0.y = y0; output.p0.z = z0;
	output.p1.x = x1; output.p1.y = y1; output.p1.z = z1;
	output.p2.x = x2; output.p2.y = y2; output.p2.z = z2;
	output.color = color;
	output.next = NULL;
	output.draw_flag = 1;

	return output;
}
/*
void add_shape(triangle **t, int x0, int y0, int z0, int x1, int y1, int z1, int x2, int y2, int z2, unsigned char color){
	triangle *new_shape;

	new_shape = create_shape(x0, y0, z0, x1, y1, z1, x2, y2, z2, color);
	new_shape->next = *t;
	*t = new_shape;
}*/

void free_shape(triangle *t){
	triangle *next;

	while(t){
		next = t->next;
		free(t);
		t = next;
	}
}

int compare_triangle(const void *t0, const void *t1){
	if(triangle_in_front((triangle *) t0, (triangle *) t1)){
		return 1;
	} else {
		return -1;
	}
}

void draw_shapes(CAM_screen *s, double fov, triangle *t, unsigned int num_shapes){
	unsigned int i;

	for(i = 0; i < num_shapes; i++){
		get_points_triangle(s, t + i, fov);
		calculate_normal_triangle(t + i);
	}

	qsort(t, num_shapes, sizeof(triangle), compare_triangle);

	for(i = 0; i < num_shapes; i++){
		draw_triangle(s, t + i, fov, (t + i)->color);
	}
}

void draw_shape(CAM_screen *s, double fov, triangle *t){
	while(t){
		calculate_normal_triangle(t);
		if(dot_3d(t->center, t->normal) < 0){
			draw_triangle(s, t, fov, t->color);
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

int test(int argc, char **argv){
	CAM_screen *screen;
	triangle cube[4];
	vector3d center;
	double randx;
	double randy;
	double randz;

	center = (vector3d) {.x = 0, .y = 0, .z = 3};

	cube[1] = create_shape(-1, 1, 4, 0, -1, 4, 1, 1, 4, COLOR_BLUE);
	cube[0] = create_shape(-1, 1, 2, 0, -1, 2, 1, 1, 2, COLOR_MAGENTA);
	//cube[2] = create_shape(-1, -1, 4, 1, 1, 4, -1, 1, 4, COLOR_CYAN);
	//cube[3] = create_shape(-1, -1, 4, 1, -1, 4, 1, 1, 4, COLOR_CYAN);
	
	//add_shape(&cube, 1, -1, 2, -1, -1, 2, 1, 1, 2, COLOR_BLUE);
	//add_shape(&cube, 1, -1, 4, 1, -1, 2, 1, 1, 4, COLOR_RED);
	//add_shape(&cube, 1, 1, 4, 1, -1, 2, 1, 1, 2, COLOR_RED);
	//add_shape(&cube, -1, -1, 4, -1, -1, 2, 1, -1, 4, COLOR_MAGENTA);
	//add_shape(&cube, 1, -1, 4, -1, -1, 2, 1, -1, 2, COLOR_MAGENTA);
	//add_shape(&cube, -1, -1, 4, 1, 1, 4, -1, 1, 4, COLOR_CYAN);
	//add_shape(&cube, -1, -1, 4, 1, -1, 4, 1, 1, 4, COLOR_CYAN);
	//add_shape(&cube, -1, -1, 2, -1, -1, 4, -1, 1, 4, COLOR_GREEN);
	//add_shape(&cube, -1, -1, 2, -1, 1, 4, -1, 1, 2, COLOR_GREEN);
	//add_shape(&cube, -1, 1, 2, 1, 1, 4, 1, 1, 2, COLOR_YELLOW);
	//add_shape(&cube, -1, 1, 2, -1, 1, 4, 1, 1, 4, COLOR_YELLOW);
	initscr();
	refresh();	
	start_color();
	CAM_init(1);
	screen = CAM_screen_create(stdscr, COLS - 1, LINES);
	while(getch() != 'q'){
		CAM_fill(screen, COLOR_WHITE);
		draw_shapes(screen, 250, cube, 2);
		CAM_update(screen);
		refresh();
		randx = (double) rand()*0.1/RAND_MAX - 0.04;
		randy = (double) rand()*0.1/RAND_MAX - 0.04;
		randz = (double) rand()*0.1/RAND_MAX - 0.04;
		rotate_triangle_x(cube, center, randx);
		rotate_triangle_y(cube, center, randy);
		rotate_triangle_z(cube, center, randz);
		rotate_triangle_x(cube + 1, center, randx);
		rotate_triangle_y(cube + 1, center, randy);
		rotate_triangle_z(cube + 1, center, randz);
		rotate_triangle_x(cube + 2, center, randx);
		rotate_triangle_y(cube + 2, center, randy);
		rotate_triangle_z(cube + 2, center, randz);
		rotate_triangle_x(cube + 3, center, randx);
		rotate_triangle_y(cube + 3, center, randy);
		rotate_triangle_z(cube + 3, center, randz);
		//rotate_shape_x(cube, center, (double) rand()*0.1/RAND_MAX - 0.04);
		//rotate_shape_y(cube, center, (double) rand()*0.1/RAND_MAX - 0.04);
		//rotate_shape_z(cube, center, (double) rand()*0.1/RAND_MAX - 0.04);
	}

	CAM_screen_free(screen);
	endwin();
	return 0;
}

