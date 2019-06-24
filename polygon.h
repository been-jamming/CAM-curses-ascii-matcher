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
	unsigned char color;
	triangle *next;
};
