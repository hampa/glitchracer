#include <ode/ode.h>

#ifndef _TB_MATH_H
#define _TB_MATH_H

#define PI (3.14159265358979323846)
#define DEG_TO_RAD (M_PI/180.0)
#define RAD_TO_DEG (180.0/M_PI)

struct Vector3
{
	double x;
	double y;
	double z;
};

// Vector3 vector functions
Vector3 cross(Vector3 v, Vector3 w);
double dot(Vector3 v, Vector3 w);
double magnitude(Vector3 v);
void normalize(Vector3 &v);
double angle_deg(Vector3 A, Vector3 B);
double angle_rad(Vector3 A, Vector3 B);

// dReal vector functions
dReal get_vec_mag(const dReal *v);
dReal get_vec_len(const dReal *v);// non sqrt magnitude version
dReal get_vec_dot(const dReal *v, const dReal *w);
const dReal * normalize_vec(const dReal *v);
const dReal * get_vec_subtract(const dReal *v1, const dReal *v2);
dReal get_impact(const dReal *velA, const dReal *velB);

// Misc functions
void matrix_multiply(double *A, const double *B, double *C);
int h_rotate(dVector3 v, float x, float y, float z, float deg);

#endif

