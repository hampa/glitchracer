#include <ode/ode.h>

#ifndef _ODE_EXTRA_H
#define _ODE_EXTRA_H

#define ANGULAR_DAMPENING 0.00001
#define LINEAR_DAMPENING  0.001 // 0.002 -> 0.02

void damp_body(dBodyID body, const dReal *lin, const dReal *ang);
void damp_body_al(dBodyID body, const dReal *lin, const dReal *ang, dReal ldamp, dReal adamp);
dGeomID dGeomTransGet(dGeomID g, dVector3 actual_pos, dMatrix3 actual_R);
dReal get_ray_distance(dGeomID ray, dGeomID geom, dReal x, dReal y, dReal z);
void get_joint_feedback(dJointID joint, float *vec);
void dBodyReset(dBodyID body);
void dBodyEulerRotate(dBodyID body, dReal euler[3]);

#endif
