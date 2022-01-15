#include "tb_math.h"

Vector3 cross(Vector3 v, Vector3 w){
	Vector3 product;
	product.x = v.y*w.z - v.z*w.y;
	product.y = v.z*w.x - v.x*w.z;
	product.z = v.x*w.y - v.y*w.x;
	return(product);
}

double dot(Vector3 v, Vector3 w){
	return (v.x*w.x + v.y*w.y + v.z*w.z);
}

double magnitude(Vector3 v){
	return (sqrt(v.x*v.x + v.y*v.y + v.z*v.z));
}


void normalize(Vector3 &v){
	double len = magnitude(v);
	v.x /= len;
	v.y /= len;
	v.z /= len;
}

double angle_deg(Vector3 A, Vector3 B){
	return acos(dot(A,B)/(magnitude(A)*magnitude(B)))*(180/PI);
}
double angle_rad(Vector3 A, Vector3 B){
	return acos(dot(A,B)/(magnitude(A)*magnitude(B)));
}

// dreal versions
dReal get_vec_mag(const dReal *v){
	return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

// non sqrt version
dReal get_vec_len(const dReal *v){
	return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

dReal get_vec_dot(const dReal *v, const dReal *w){
	return (v[0]*w[0] + v[1]*w[1] + v[2]*w[2]);
}

const dReal * normalize_vec(const dReal *v){
	static dReal n[3];
	n[0] = v[0]; 
	n[1] = v[1]; 
	n[2] = v[2];
	dReal mag = get_vec_mag(v);	
	n[0] /= mag;	
	n[1] /= mag;	
	n[2] /= mag;	
	return n;
} 

const dReal * get_vec_subtract(const dReal *v1, const dReal *v2){
	static dReal v[3];
	v[0]=v1[0]-v2[0];
	v[1]=v1[1]-v2[1];
	v[2]=v1[2]-v2[2];
	return v;
}

dReal get_impact(const dReal *velA, const dReal *velB){

	// 3. subtract velocities
	const dReal *sub = get_vec_subtract(velA, velB);
	dReal smag = get_vec_mag(sub);

	// for spead up
	if(smag < 5.0f){
		return 0.0f;
	}	
	return smag;	
	/*
	// 1. find magnitude of velocities
	dReal mag1 = get_vec_mag(velA);	
	dReal mag2 = get_vec_mag(velB);	

	// 2. find angle between vectors
	dReal dot = get_vec_dot(velA, velB); 
	dReal angle = acos(dot/(mag1*mag2));	

	// angle 0 - 3.14. 
	dReal impact = angle * smag; 

	DBG("mag1 %f mag2 %f dot %f sub %.1f %.1f %.1f smag %f angle %f impact %f\n", 
	mag1, mag2, dot, sub[0], sub[1], sub[2], smag, angle, impact);
	return impact;
	 */
}

void matrix_multiply(double *A, const double *B, double *C)
{
	/* A matrix multiplication (dot product) of two 4x4 matrices.
	   Actually, we are only using matrices with 3 rows and 4 columns. */

	C[0] = A[0]*B[0] + A[1]*B[4] + A[2]*B[8];
	C[1] = A[0]*B[1] + A[1]*B[5] + A[2]*B[9];
	C[2] = A[0]*B[2] + A[1]*B[6] + A[2]*B[10];
	C[3] = A[0]*B[3] + A[1]*B[7] + A[2]*B[11] + A[3];

	C[4] = A[4]*B[0] + A[5]*B[4] + A[6]*B[8];
	C[5] = A[4]*B[1] + A[5]*B[5] + A[6]*B[9];
	C[6] = A[4]*B[2] + A[5]*B[6] + A[6]*B[10];
	C[7] = A[4]*B[3] + A[5]*B[7] + A[6]*B[11] + A[7];

	C[8] = A[8]*B[0] + A[9]*B[4] + A[10]*B[8];
	C[9] = A[8]*B[1] + A[9]*B[5] + A[10]*B[9];
	C[10] = A[8]*B[2] + A[9]*B[6] + A[10]*B[10];
	C[11] = A[8]*B[3] + A[9]*B[7] + A[10]*B[11] + A[11];
}

int h_rotate(dVector3 v, float x, float y, float z, float deg){

	//roll
	v[0] += (float)cos(deg*DEG_TO_RAD)*z;
	v[1] += (float)sin(deg*DEG_TO_RAD)*z;

	//yaw
	v[1] += (float)sin(deg*DEG_TO_RAD)*y;
	v[2] += (float)cos(deg*DEG_TO_RAD)*y;

	//pitch
	v[0] += (float)sin(deg*DEG_TO_RAD)*x;
	v[2] += (float)cos(deg*DEG_TO_RAD)*x;
	return 0;
}

