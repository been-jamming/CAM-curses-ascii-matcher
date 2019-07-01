typedef struct{
	double x;
	double y;
} vector2d;

typedef struct{
	double x;
	double y;
	double z;
} vector3d;

typedef struct triangle triangle;

struct triangle {
	vector3d p0;
	vector3d p1;
	vector3d p2;
	vector3d normal;
	vector3d center;
	vector2d screen0;
	vector2d screen1;
	vector2d screen2;
	unsigned char color;
	unsigned char draw_flag;
	triangle *next;
};

void draw_triangle(CAM_screen *s, triangle *tri, double fov, unsigned char color);

void calculate_normal_triangle(triangle *t);

void rotate_triangle_x(triangle *t, vector3d center, double angle);

void rotate_triangle_y(triangle *t, vector3d center, double angle);

void rotate_triangle_z(triangle *t, vector3d center, double angle);

