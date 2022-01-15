#include <ode/ode.h>
#include "tb_math.h"
#include "ode_extra.h"

// misc functions that ODE API should have included
//#define ANGULAR_DAMPENING 0.0001 // 0.005 -> 0.05
void damp_body(dBodyID body, const dReal *lin, const dReal *ang){
	damp_body_al(body, lin, ang, LINEAR_DAMPENING, ANGULAR_DAMPENING);
	//damp_body_al(body, lin, ang, 1,ANGULAR_DAMPENING);
}

void damp_body_al(dBodyID body, const dReal *lin, const dReal *ang, dReal ldamp, dReal adamp){

			dMass mass; 
			dBodyGetMass(body, &mass);
			dBodyAddForce(body, 
			-mass.mass * lin[0] * ldamp, 
			-mass.mass * lin[1] * ldamp, 
			-mass.mass * lin[2] * ldamp);

			dReal damp[3];
			
			damp[0] = ang[0]*ang[0]*ang[0]*adamp;
			damp[1] = ang[1]*ang[1]*ang[1]*adamp;
			damp[2] = ang[2]*ang[2]*ang[2]*adamp;
			
			/*
			damp[0] = ANGULAR_DAMPENING;
			damp[0] = ANGULAR_DAMPENING;
			damp[0] = ANGULAR_DAMPENING;
			*/
			
			dBodyAddTorque(body, 
			-mass.mass * damp[0], 
			-mass.mass * damp[1], 
			-mass.mass * damp[2]);
}

dGeomID dGeomTransGet(dGeomID g, dVector3 actual_pos, dMatrix3 actual_R){
	const dReal *pos = dGeomGetPosition(g);
	const dReal *R = dGeomGetRotation(g);
	dGeomID g2 = dGeomTransformGetGeom (g);
	const dReal *pos2 = dGeomGetPosition (g2);
	const dReal *R2 = dGeomGetRotation (g2);
	dMULTIPLY0_331 (actual_pos,R,pos2);
	actual_pos[0] += pos[0];
	actual_pos[1] += pos[1];
	actual_pos[2] += pos[2];
	dMULTIPLY0_333 (actual_R,R,R2);
	return g2;
}

// return the unsquared distance from x,y,z and where ray intersects with geom
// return -1 if no hit where found
dReal get_ray_distance(dGeomID ray, dGeomID geom, dReal x, dReal y, dReal z){
	dContactGeom contact;
	if (int numc = dCollide (ray,geom, 0,&contact, sizeof(dContact))) {

		// find distance from mouse camera click position
		dReal dx = contact.pos[0]-x;
		dReal dy = contact.pos[1]-y;
		dReal dz = contact.pos[2]-z;
		// no need to sqrtf the distance
		return dx*dx + dy*dy + dz*dz; 
	}
	return -1; 
}

void get_joint_feedback(dJointID joint, float *vec){
        struct dJointFeedback *jf = dJointGetFeedback(joint);
        vec[0] = jf->f1[0]*jf->f1[0] + jf->t1[0]*jf->t1[0];
        vec[1] = jf->f1[1]*jf->f1[1] + jf->t1[1]*jf->t1[1];
        vec[2] = jf->f1[2]*jf->f1[2] + jf->t1[2]*jf->t1[2];
}

void dBodyReset(dBodyID body)
{
	dMatrix3 R;
	// save this as a global for speed up?
	dRSetIdentity(R);

	dBodySetLinearVel(body, 0, 0, 0);
	dBodySetAngularVel(body, 0, 0, 0);
	dBodySetRotation(body, R);
	dBodySetForce(body, 0, 0, 0);
	dBodySetTorque(body, 0, 0, 0);
}

void dBodyEulerRotate(dBodyID body, dReal euler[3])
{
        if(euler[0] == 0 && euler[1] == 0 && euler[2] == 0){
                return;
        }
        dMatrix3 R;
        dRFromEulerAngles(R, euler[0] * DEG_TO_RAD , euler[1] * DEG_TO_RAD, euler[2] * DEG_TO_RAD);
	dBodySetRotation(body, R);
}

