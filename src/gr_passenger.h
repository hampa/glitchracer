#ifndef GR_PASSENGER
#define GR_PASSENGER

#include "gr_geom.h"
#include "ode.h"

enum { LIMB_HEAD, LIMB_TORSO, LIMB_L_ARM, LIMB_R_ARM, LIMB_L_LEG, LIMB_R_LEG, NUM_LIMBS };

enum { 	
	JOINT_NECK, 
	JOINT_L_SHOULDER, 
	JOINT_R_SHOULDER, 
	JOINT_L_HIP, 
	JOINT_R_HIP, 
	NUM_PASSENGER_JOINTS 
};

struct limb_t {
	int id;
	int type;
	double sides[3];
	double pos[3];
	float color[4];
	float mass;
};

struct joint_t {
	int id;
	int dna[2];
	int type;
	double pos[3];
	double axis[3];
	double stop[2];
};

struct passenger_t {
	int id;
	struct joint_t joints[NUM_PASSENGER_JOINTS];
	struct limb_t limbs[NUM_LIMBS];
	dBodyID ode_body[NUM_LIMBS];
	dGeomID ode_geom[NUM_LIMBS];
	dJointID ode_joint[NUM_PASSENGER_JOINTS];
	dJointID seat_joint;
	dJointID seat_motor;
};

#endif

