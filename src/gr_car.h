#include "glitchracer.h"
#include "gr_geom.h"
#include "ode.h"
#ifndef _GR_CAR_H
#define _GR_CAR_H

enum { BODY_CHASSI, 
	// BODY_CHASSI_OFF, 
	BODY_SEAT, 
	BODY_WHEEL1, 
	//BODY_WHEEL2, 
	BODY_WHEEL3, 
	//BODY_WHEEL4, 
	BODY_CAMERA, NUM_BODIES };
enum { BOLT_SEAT, NUM_BOLTS };

struct chassi_t {
	int id;
	int type;
	double sides[3];
	double pos[3];
	double rot[3];
	double mass;
	float color[4];
};

struct bolt_t {
	int id;
	int dna[2];
};

struct env_t {
	int index;
	int id;
	int type;
	int flag;
	double sides[3];
	double pos[3];
	float color[4];
};

#define NUM_ENV 64 
#define NUM_DESTRUCT  64	
#define NUM_GEOM (NUM_ENV+NUM_DESTRUCT)

struct player_t {
	dJointID bolts[NUM_BOLTS];
	dGeomID geom[NUM_BODIES];
	dBodyID body[NUM_BODIES];
	dJointID joint[NUM_JOINTS];
	struct dJointFeedback feedback[NUM_JOINTS];
};

struct wheel_t{
	int body[2];
	float th;
	int broken;
	int steer;
};


struct damage_t {
        struct dJointFeedback jf;
        int limb[2]; // contain both joints and bodypars
        int limbtype[2]; //0 = body, !0 = joint
        int player[2];
	dGeomID geom[2];
        dReal pos[3];
        dReal impact;
        unsigned char cause; // Cause of damage in case not done by a player
};

#endif
