#include <stdio.h>
#include <assert.h>
#include "ode.h"
#include "drawstuff.h"
#include "gr_entity.h"
#include "gr_car.h"
#include "gr_passenger.h"
#include "glitchracer.h"
#include "util.h"
#include "ode_extra.h"
#include "font.h"
#include "tb_math.h"
#include "blood.h"

#define MAX_CONTACTS 8 		// maximum number of contact points per body
#define LENGTH 3.5              // chassis length
#define WIDTH 2.5               // chassis width
#define HEIGHT 1.0              // chassis height
#define RADIUS 0.5              // wheel radius
#define STARTZ 1.0              // starting height of chassis
#define CMASS 10                 // chassis mass
#define WMASS 1                 // wheel mass
#define FMAX 2250                 // car engine fmax
//#define HINGE_SEAT 1

static dReal turn = 0, turn_reset = 0, passenger_turn = 0, speed = 0;       // user commands

dReal seat_forward = 0;
dReal seat_back = 0;
dReal seat_left = 0;
dReal seat_right = 0;

static int pause = 1;
static int step_pause = 1;
static int camera = 0;
static double camera_pos[]={0,0,3};

static dGeomID plane; 
//static dGeomID box[NUM_BODIES];
static dBodyID body[NUM_GEOM];
static dGeomID geom[NUM_GEOM];
struct env_t env[NUM_GEOM];
//static dJointID joint[NUM_JOINTS];
extern struct chassi_t chassis[];
extern struct wheel_t wheels[];
extern struct limb_t limbs[];
extern struct joint_t joints[];
// extern struct bolt_t bolts[];
struct passenger_t passengers[2];

//struct env_t env_destruct[NUM_DESTRUCT];

dWorldID world;
dSpaceID space;
static dJointGroupID contactgroup;
/* ents */
static struct entity_t entities[64];
static int num_entities = 0;

int num_damages=0;
#define NUM_DAMAGES 16
struct damage_t damages[NUM_DAMAGES];

struct player_t players[2];

void create_plane();
void create_world();
void draw_world();
void draw_xyz();
void draw_passenger();
void draw_2d();
void draw_geom(dGeomID g, int flag);
void reset_car(struct player_t *p);
void reset_passenger(struct passenger_t *p, float x, float y, float z);
void reset_geoms();
void destruct_geom(dGeomID g);
void create_passenger(struct passenger_t *p, dReal x, dReal y, dReal z);
int destruct_index = 0;
dGeomID create_body(int type, dReal pos[3], dReal sides[3], float mass, float mass_adjust);

void create_plane(){
	plane = dCreatePlane (space,0,0,1,0);
	entities[num_entities].type = ENT_PLANE;
	entities[num_entities].index = num_entities;
	dGeomSetData(plane, (void*)&entities[num_entities]);
	num_entities++;
}

void destruct_geom(dGeomID g){
	float x,y,z;
        dReal sides[3] = {0.5,0.1,0.3};
	dBodyID b = dGeomGetBody(g);
	const dReal *pos = dBodyGetPosition(b);
	dGeomBoxGetLengths(g, sides);
	dBodyDisable(b);
	x = sides[0] * 0.25f;
	y = sides[1] * 0.25f;
	z = sides[2] * 0.25f;
	dMatrix3 R;
	dRSetIdentity (R);
	
	// printf("destruct_geom %f %f %f\n", pos[0], pos[1], pos[2]);
	for(int i=destruct_index ; i<8+destruct_index; i++){
		dGeomBoxSetLengths(geom[i+NUM_ENV], x*1.9, y*1.9, z*1.9);
		dBodySetRotation(body[i+NUM_ENV], R);
		dBodySetLinearVel(body[i+NUM_ENV], 0, 0, 0);
		dBodySetAngularVel(body[i+NUM_ENV], 0, 0, 0);
	}
	
	dBodySetPosition(body[destruct_index+0+NUM_ENV], pos[0]+x, pos[1]+y, pos[2]-z);
	dBodySetPosition(body[destruct_index+1+NUM_ENV], pos[0]-x, pos[1]+y, pos[2]-z);
	dBodySetPosition(body[destruct_index+2+NUM_ENV], pos[0]+x, pos[1]-y, pos[2]-z);
	dBodySetPosition(body[destruct_index+3+NUM_ENV], pos[0]-x, pos[1]-y, pos[2]-z);

	dBodySetPosition(body[destruct_index+4+NUM_ENV], pos[0]+x, pos[1]+y, pos[2]+z);
	dBodySetPosition(body[destruct_index+5+NUM_ENV], pos[0]-x, pos[1]+y, pos[2]+z);
	dBodySetPosition(body[destruct_index+6+NUM_ENV], pos[0]+x, pos[1]-y, pos[2]+z);
	dBodySetPosition(body[destruct_index+7+NUM_ENV], pos[0]-x, pos[1]-y, pos[2]+z);
	destruct_index += 8;
	if(destruct_index >= NUM_DESTRUCT){
		destruct_index = 0;
	}

	//pause ^= 1;
}

/*
void makeDestructEnv(){
	dMass m;
	//memcpy(&env_destruct, env, sizeof(struct env_t) * NUM_DESTRUCT);
	for(int i=0; i<NUM_DESTRUCT; i++){
		body_destruct[i] = dBodyCreate (world);
		env_destruct[i].color[0] = 1; env_destruct[i].color[1] = 0; env_destruct[i].color[2] = 0; env_destruct[i].color[3] = 1;
		dBodySetPosition (body_destruct[i], 10+(i*2), 0, 1); 
		env_destruct[i].type = GEOM_BOX;
		switch(env_destruct[i].type){
		case GEOM_BOX:
			dMassSetBox (&m, 1, 1, 1, 1); 
			dBodySetMass (body_destruct[i],&m);
			geom_destruct[i] = dCreateBox (space, 1.0, 1.0, 1.0); 
			dGeomSetBody (geom_destruct[i], body_destruct[i]);
			break;
		default:
			assert(0);
		}
		//dBodyDisable(body_destruct[i]);
	}
}
*/

int geom_pos = NUM_GEOM;
void push_geom(){
	int i;
	const dReal *pos = dBodyGetPosition (players[0].body[0]);
	for(i=0; i<NUM_GEOM; i++){
		const dReal * p = dBodyGetPosition (body[i]);
		if(pos[0]>p[0]){
			dBodySetPosition(body[i], (geom_pos*2) + 10, env[i].pos[1], env[1].pos[2]);
			// printf("Moving from pos %f to %f\n", p[0], (geom_pos*2) + 10);
			geom_pos++;
		}
	}
}

void reset_geoms()
{
	dMatrix3 R;
	dRSetIdentity(R);
	for(int i=0; i<NUM_GEOM; i++){
		struct env_t *e = &env[i];
		dBodySetPosition (body[i], e->pos[0], e->pos[1], e->pos[2]);
		dBodySetLinearVel(body[i], 0, 0, 0);
		dBodySetAngularVel(body[i], 0, 0, 0);
		dBodySetRotation(body[i], R);
		dBodySetForce(body[i], 0, 0, 0);
		dBodySetTorque(body[i], 0, 0, 0);
		dBodyEnable(body[i]);
		dGeomBoxSetLengths(geom[i], e->sides[0], e->sides[1], e->sides[2]);
	}
	geom_pos = NUM_GEOM;
}

void create_geoms()
{
	dMass m;
	int i;
	for(i=0; i<NUM_GEOM; i++){
		body[i] = dBodyCreate (world);
		struct env_t *e = &env[i];
		e->index = i;
		e->flag = 0;
		e->id = BODY_CHASSI ;
		e->type = GEOM_BOX;
		e->pos[0] = (i*2) + 10; // forward
		e->pos[1] = (i % 2) ? 55 : -55; // left
		e->pos[2] = 3; // up
		e->sides[0] = e->sides[1] = 1;
		e->sides[2] = 4;
		e->color[0] =  e->color[3] = 1; 
		e->color[1] = e->color[2];
		
		dBodySetPosition (body[i], e->pos[0], e->pos[1], e->pos[2]);
		switch(e->type){
		case GEOM_BOX:
			dMassSetBox (&m, 1, e->sides[0], e->sides[1], e->sides[2]);
			dBodySetMass (body[i],&m);
			geom[i] = dCreateBox (space, e->sides[0], e->sides[1], e->sides[2]);
			dGeomSetBody (geom[i], body[i]);
			break;
		case GEOM_NONE:
			dMassSetBox (&m, 1, e->sides[0], e->sides[1], e->sides[2]);
			dBodySetMass (body[i],&m);
			geom[i] = NULL;
			break;
		case GEOM_SPHERE:
			dQuaternion q;
			dQFromAxisAndAngle (q,1,0,0,M_PI*0.5);
			geom[i] = dCreateSphere (space, e->sides[0]);
			dGeomSetBody (geom[i], body[i]);
			break;
		default : 
			assert(0);
			break;
		}
		dGeomSetData(geom[i], (void*)e);
	}
}

void reset_passenger(struct passenger_t *p, float x, float y, float z)
{
	int i;
	for (i=0; i<NUM_LIMBS; i++) {
		dBodySetPosition (p->ode_body[i], limbs[i].pos[0] + x, limbs[i].pos[1] + y, limbs[i].pos[2] + z);
		dBodyReset(p->ode_body[i]);
	}

	for (i=0; i<NUM_PASSENGER_JOINTS; i++) {
		// dJointAttach( p->ode_joint[i], p->ode_body[joints[i].dna[0]], p->ode_body[joints[i].dna[1]]);
		dJointSetHingeParam(p->ode_joint[i], dParamVel, 0);
		dJointSetHingeParam(p->ode_joint[i], dParamVel2, 0);
		dJointSetHingeParam(p->ode_joint[i], dParamVel3, 0);
		dJointSetHingeParam(p->ode_joint[i], dParamLoStop, joints[i].stop[0]);
		dJointSetHingeParam(p->ode_joint[i], dParamHiStop, joints[i].stop[1]);
	}

	return;
}

void reset_car(struct player_t *p){
	int i;
	dMatrix3 R;
	dRSetIdentity(R);
	for(i=0; i<NUM_BODIES; i++){
		dBodySetPosition (p->body[i], chassis[i].pos[0], chassis[i].pos[1], chassis[i].pos[2]);
		dBodyReset(p->body[i]);
		dBodyEulerRotate(p->body[i], chassis[i].rot);
		/*
		dBodySetLinearVel(p->body[i], 0, 0, 0);
		dBodySetAngularVel(p->body[i], 0, 0, 0);
		dBodySetRotation(p->body[i], R);
		dBodySetForce(p->body[i], 0, 0, 0);
		dBodySetTorque(p->body[i], 0, 0, 0);
		*/
	}
	for (i=0; i<NUM_WHEELS; i++){
		wheels[i].broken = 0;
		dJointAttach (p->joint[i], p->body[wheels[i].body[0]], p->body[ wheels[i].body[1]]  );
		dJointSetHinge2Param(p->joint[i],dParamVel,0);
		dJointSetHinge2Param(p->joint[i],dParamVel2,0);
		dJointSetHinge2Param(p->joint[i],dParamVel3,0);
		

		// rotate the wheels forward
#ifndef WITH_SPHERE
		dQuaternion q;
		dQFromAxisAndAngle (q,1,0,0,M_PI*0.5);
		dBodySetQuaternion (p->body[wheels[i].body[1]], q);
		dBodySetAutoDisableFlag(p->body[wheels[i].body[1]], false);
#endif
	}
	speed = 0;
	turn = 0;
	passenger_turn = 0;
}

void draw_passenger()
{
	int i;
	dMatrix3 R;
	dRSetIdentity(R);
	for (i=0; i<NUM_LIMBS; i++) {
		draw_geom(passengers[0].ode_geom[i], 0);
	}
	
	dsSetColor(0.9, 0.9, 0.9);
	for (i=0; i<NUM_PASSENGER_JOINTS; i++) {
		dVector3 pos;
		dJointGetHingeAnchor(passengers[0].ode_joint[i], pos);	
		dsDrawSphereD(pos, R, 0.1, 0);
	}
}

void create_passenger(struct passenger_t *p, dReal x, dReal y, dReal z)
{
	int i;
	double bp[]={0,0,0};
	for (i=0; i<NUM_LIMBS; i++) {
		bp[0] = limbs[i].pos[0] + x;
		bp[1] = limbs[i].pos[1] + y;
		bp[2] = limbs[i].pos[2] + z;
		p->ode_geom[i] = create_body(limbs[i].type, bp, limbs[i].sides, limbs[i].mass, 0);
				
		p->ode_body[i] = dGeomGetBody(p->ode_geom[i]);
		assert(p->ode_body[i]);
	}
	
	for (i=0; i<NUM_PASSENGER_JOINTS; i++) {
		p->ode_joint[i]  = dJointCreateHinge(world, 0); 
		dJointAttach( p->ode_joint[i], p->ode_body[joints[i].dna[0]], p->ode_body[joints[i].dna[1]]);
		dJointSetHingeAnchor(p->ode_joint[i], joints[i].pos[0]+x , joints[i].pos[1]+y, joints[i].pos[2]+z);
		dJointSetHingeParam(p->ode_joint[i], dParamLoStop, joints[i].stop[0]);
		dJointSetHingeParam(p->ode_joint[i], dParamHiStop, joints[i].stop[1]);
	}

#ifdef SEAT_HINGE
	p->seat_joint = dJointCreateHinge(world, 0);
#else
	p->seat_joint = dJointCreateBall(world, 0);
	p->seat_motor = dJointCreateAMotor(world, 0);
#endif
	dJointAttach( p->seat_joint, players[0].body[BODY_CHASSI], p->ode_body[LIMB_TORSO]);
	const dReal *pos = dBodyGetPosition(players[0].body[BODY_CHASSI]);

#ifdef SEAT_HINGE
	dJointSetHingeAnchor(p->seat_joint, pos[0], pos[1], pos[2]);
	dJointSetHingeAxis(p->seat_joint, 0, 1, 0);
	dJointSetHingeParam(p->seat_joint, dParamLoStop, -1.0);
	dJointSetHingeParam(p->seat_joint, dParamHiStop, 1.0);
#else
	dJointSetBallAnchor(p->seat_joint, pos[0], pos[1], pos[2]);
	

	dJointAttach(p->seat_motor, players[0].body[BODY_CHASSI], p->ode_body[LIMB_TORSO]);

	dJointSetAMotorMode(p->seat_motor, dAMotorEuler);

	dJointSetAMotorParam(p->seat_motor, dParamLoStop, -0.3);
	dJointSetAMotorParam(p->seat_motor, dParamHiStop, 0.3);

	dJointSetAMotorParam(p->seat_motor, dParamLoStop2, -0.3);
	dJointSetAMotorParam(p->seat_motor, dParamHiStop2, 0.3);

	dJointSetAMotorParam(p->seat_motor, dParamLoStop3, -0.3);
	dJointSetAMotorParam(p->seat_motor, dParamHiStop3, 0.3);
	
	dJointSetAMotorNumAxes(p->seat_motor,  3);
	dJointSetAMotorAxis(p->seat_motor,0,1,0,0,1);
	dJointSetAMotorAxis(p->seat_motor,2,2,1,0,0);


	/*
	dJointSetAMotorAxis(p->seat_motor, 0, 1, 1, 0, 0);
	dJointSetAMotorAxis(p->seat_motor, 1, 1, 0, 1, 0);
	dJointSetAMotorAxis(p->seat_motor, 2, 1, 0, 0, 1);
	*/

	dJointSetAMotorParam(p->seat_motor, dParamFMax, 0.00001);
	dJointSetAMotorParam(p->seat_motor, dParamFMax2, 0.00001);
	dJointSetAMotorParam(p->seat_motor, dParamFMax3, 0.00001);

	dJointSetAMotorParam(p->seat_motor, dParamVel, 0);
	dJointSetAMotorParam(p->seat_motor, dParamVel2, 0);
	dJointSetAMotorParam(p->seat_motor, dParamVel3, 0);

#endif
	
}

dGeomID create_body(int type, dReal pos[3], dReal sides[3], float mass, float adjust)
{
	dMass m;
	dMatrix3 M;
	dRSetIdentity(M);
	dBodyID body = dBodyCreate (world);
	dGeomID geom = NULL;
	dBodySetPosition (body, pos[0], pos[1], pos[2]); 
	switch (type) {
		case GEOM_BOX:
			dMassSetBox (&m, mass, sides[0], sides[1], sides[2]);
			if (adjust > 0) {
				// dMassAdjust (&m, CMASS/2.0);
				dMassAdjust (&m, adjust);
			}
			dBodySetMass (body, &m);
			geom = dCreateBox (space, sides[0], sides[1], sides[2]);
			dGeomSetBody (geom, body);
			break;
		case GEOM_NONE:
			/*
			dMassSetBox (&m, 1, sides[0], sides[1], sides[2]);
			dMassAdjust (&m, CMASS/2.0);
			dBodySetMass (body, &m);
			geom = NULL;
			*/
			break;
		case GEOM_SPHERE:
			dQuaternion q;
			//dQFromAxisAndAngle (q, 1, 0, 0, M_PI*0.5);
			//dBodySetQuaternion (body, q);
			dBodySetRotation (body, M);
			dMassSetSphere (&m, mass, sides[0]);
			if (adjust) {
				// dMassAdjust (&m, WMASS);
				dMassAdjust (&m, adjust);
			}
			dBodySetMass (body, &m);
			geom = dCreateSphere (space, sides[0]);
			dGeomSetBody (geom, body);
			break;
		default : 
			break;
	}
	return geom;
}

void create_car(struct player_t *p, dReal x, dReal y){
	dMass m;
	dQuaternion q;
	dMatrix3 M;
	int i;

	dRSetIdentity(M);	
	// chassis body
	for(i=0; i<NUM_BODIES; i++){
		p->body[i] = dBodyCreate (world);
		dBodySetPosition (p->body[i], chassis[i].pos[0], chassis[i].pos[1], chassis[i].pos[2]);
		dBodySetAutoDisableFlag(p->body[i], false);

		switch(chassis[i].type){
			case GEOM_BOX:
				dMassSetBox (&m, chassis[i].mass, chassis[i].sides[0], chassis[i].sides[1], chassis[i].sides[2]);
				// dMassAdjust (&m, CMASS/2.0);
				dBodySetMass (p->body[i],&m);
				// dMassTranslate (&m, 0, 0, -10);	
				p->geom[i] = dCreateBox (space, chassis[i].sides[0], chassis[i].sides[1], chassis[i].sides[2]);
				dGeomSetBody (p->geom[i], p->body[i]);
				break;
			case GEOM_NONE:
				dMassSetBox (&m, 1, chassis[i].sides[0], chassis[i].sides[1], chassis[i].sides[2]);
				// dMassAdjust (&m, CMASS/2.0);
				dBodySetMass (p->body[i],&m);
				p->geom[i] = NULL;
				break;
			case GEOM_SPHERE:
				// dQFromAxisAndAngle (q, 1, 0, 0, M_PI*0.5);
				// dBodySetQuaternion (p->body[i], q);
				dBodySetRotation (p->body[i], M);
				dMassSetSphere (&m, 1, chassis[i].sides[0]);
				dBodySetMass( p->body[i], &m);
				p->geom[i] = dCreateSphere (space, chassis[i].sides[0]);
				dGeomSetBody (p->geom[i], p->body[i]);
				break;
			case GEOM_CYLINDER:
#ifdef WITH_CYLINDER
				dQFromAxisAndAngle (q,1,0,0,M_PI*0.5);
				dBodySetQuaternion (p->body[i],q);
				//dBodySetRotation (p->body[i], M);
				//dBodySetQuaternion (p->body[i], q);
				dMassSetSphere (&m,1, chassis[i].sides[0]);
				// dMassAdjust (&m,WMASS);
				dBodySetMass( p->body[i], &m);
				p->geom[i] = dCreateCylinder (space, chassis[i].sides[0], chassis[i].sides[1]);
				dGeomSetBody (p->geom[i], p->body[i]);
#endif
				break;
			case GEOM_CAPSULE:
#ifdef WITH_CAPSULE
				// dQFromAxisAndAngle (q, 1, 0, 0, M_PI*0.5);
				// dBodySetQuaternion (p->body[i], q);
				// dBodySetRotation (p->body[i], M);
				dBodyEulerRotate(p->body[i], chassis[i].rot);
				// dMassAdjust (&m,WMASS);
				p->geom[i] = dCreateCCylinder (space, chassis[i].sides[0], chassis[i].sides[1]);
				dGeomSetBody (p->geom[i], p->body[i]);
#endif
				break;
			default : 
				break;
		}
	}

	// front and back wheel hinges
	int joint_index = 0;
	for (i=0; i<NUM_WHEELS; i++) {
		p->joint[i] = dJointCreateHinge2 (world, 0);
		assert(p->joint[i]);
		// printf("creating joint %p - %p %p\n",  p->joint[i], p->body[ wheels[i].body[0]], p->body[ wheels[i].body[1]]);
		dJointAttach (p->joint[i], p->body[ wheels[i].body[0]], p->body[ wheels[i].body[1]]  );
		const dReal *a = dBodyGetPosition (p->body [wheels[i].body[1] ]);
		dJointSetHinge2Anchor (p->joint[i], a[0], a[1], a[2]);
		dJointSetHinge2Axis1 (p->joint[i],0,0, (i<2 ? 1 : -1));
		dJointSetHinge2Axis2 (p->joint[i],0,1,0);
		dJointSetHinge2Param (p->joint[i], dParamSuspensionERP,0.8);
		dJointSetHinge2Param (p->joint[i], dParamSuspensionCFM,1e-5);
		// dJointSetHinge2Param (p->joint[i], dParamSuspensionCFM,1e-1);
		dJointSetHinge2Param (p->joint[i], dParamVel2,0);
		dJointSetHinge2Param (p->joint[i], dParamFMax2,FMAX);
		dJointSetFeedback(p->joint[i], &p->feedback[i]);
		joint_index++;
		
		if (wheels[i].steer) {
			dBodySetFiniteRotationMode(p->body[wheels[i].body[1]], true); 
		}
	}

          // lock back wheels along the steering axis
        for (i=0; i<NUM_WHEELS; i++) {
		if (wheels[i].steer) continue;
                // set stops to make sure wheels always stay in alignment
                dJointSetHinge2Param (p->joint[i], dParamLoStop, 0);
                dJointSetHinge2Param (p->joint[i], dParamHiStop, 0);

                // Do this twice... weird ode bug! see mailing list and Jon's carworld
                dJointSetHinge2Param (p->joint[i], dParamLoStop, 0);
                dJointSetHinge2Param (p->joint[i], dParamHiStop, 0);
                dJointSetHinge2Param (p->joint[i], dParamStopERP, 0.99);
                dJointSetHinge2Param (p->joint[i], dParamStopCFM, 0.01);
        }



	/*
	for (i=0; i<NUM_BOLTS; i++) {
		// fprintf(stderr, "creating hinge %i\n", i);
		// p->bolts[i] = dJointCreateHinge (world,0);
		// dJointAttach (p->bolts[i], p->body[bolts[i].dna[0]], p->body[bolts[i].dna[1]]);
		// const dReal *a = dBodyGetPosition (p->body [bolts[i].dna[0]]);
		// const dReal *b = dBodyGetPosition (p->body [bolts[i].dna[1]]);
		float mid[]={ 	
				(mid[0]+mid[0])*0.5f,
				(mid[1]+mid[1])*0.5f,
				(mid[2]+mid[2])*0.5f 
		};
		// dJointSetHingeAnchor(p->bolts[i], mid[0], mid[1], mid[2]);
	}
	*/


	/*
	//center of mass offset body. (hang another copy of the body COMOFFSET units below it by a fixed joint)
	dBodyID b = dBodyCreate (world);
	dBodySetPosition (b,x,y,STARTZ+COMOFFSET);
	dMassSetBox (&m,1,LENGTH,WIDTH,HEIGHT);
	dMassAdjust (&m,CMASS/2.0);
	dBodySetMass (b,&m);
	dJointID j = dJointCreateFixed(world, 0);
	dJointAttach(j, body[bodyI], b);
	dJointSetFixed(j);
	//box[boxI+1] = dCreateBox(space,LENGTH,WIDTH,HEIGHT);
	//dGeomSetBody (box[boxI+1],b);
	 */

	/* camera */
	/*
	body[bodyI] = dBodyCreate(world);		
	dBodySetPosition(body[1], -15, 0, 3);
	box[1] = dCreateBox(space, 0.1, 0.1, 0.1);
	dGeomSetBody(box[1], body[1]);
	dMass m;
	dMassSetBoxTotal(&m, 0.1, 0.1, 0.1, 0.1);
	dBodySetMass(body[1], &m);
	*/

	// ugly
	i=joint_index;
	p->joint[i] = dJointCreateHinge(world, 0);
	dJointAttach(p->joint[i], p->body[BODY_CHASSI], p->body[BODY_CAMERA]);
	dJointSetHingeAnchor(p->joint[i], chassis[BODY_CAMERA].pos[0], chassis[BODY_CAMERA].pos[1], chassis[BODY_CAMERA].pos[2]);
	dJointSetHingeParam (p->joint[i], dParamLoStop, 0);
	dJointSetHingeParam (p->joint[i], dParamHiStop, 0);
	i++;

	// p->joint[i]  = dJointCreateFixed(world, 0);
        //dJointAttach(p->joint[i], p->body[BODY_CHASSI], p->body[BODY_CHASSI_OFF]);
        //dJointSetFixed(p->joint[i]);
}

void create_world(){
	world = dWorldCreate();
	space = dHashSpaceCreate (0);
	contactgroup = dJointGroupCreate (0);

	//dWorldSetGravity (world,0,0,-20.82);
	dWorldSetGravity (world,0,0,-30.82);
	//dWorldSetCFM (world,0.2f);
        //dWorldSetCFM (world, 1e-5);
        // dWorldSetCFM (world, 0.1);
        // dWorldSetERP (world, 0.8);
	dWorldSetERP (world,1.0f); 
        //dWorldSetGravity (world,0,0,-1.5);
        //dWorldSetCFM (world, 1e-5);
        //dWorldSetERP (world, 0.8);

	create_plane();
	create_car(&players[0], 0, 0);
	create_passenger(&passengers[0], 0, 0, 2.5);
	
	//makeDestructEnv();
	create_geoms();
	create_blood();
	enable_blood();  // turn on physics for blood
}

void handle_joints(){
	float vec[3];
	for (int i=0; i<NUM_WHEELS; i++) {
		if(wheels[i].broken) {
			const dReal *pos = dBodyGetPosition (players[0].body[wheels[i].body[1]]);
			const dReal dir[] = {0,0,1};
			// player_bleed(0, pos[0], pos[1], pos[2], dir);
			continue;
		}
		get_joint_feedback(players[0].joint[i], vec);
		//printf("feedback %i %.1f %.1f %.1f\n", i, sqrtf(vec[0]), sqrtf(vec[1]), sqrtf(vec[2]));
		if(sqrtf(vec[2]) * 0.1f > wheels[i].th){
			printf("BREAK %f %f\n", sqrtf(vec[2]), vec[2]);
			wheels[i].broken = 1;
        		dJointAttach(players[0].joint[i], NULL, NULL);
		}
	}
}

void handle_damages(){
	for(int i = 0; i<num_damages; i++){
		struct env_t *e1 = (struct env_t *)dGeomGetData(damages[i].geom[0]);
		struct env_t *e2 = (struct env_t *)dGeomGetData(damages[i].geom[1]);
		//printf("damage %i %f %p %p\n", i, damages[i].impact, (void*)e1 , (void*)e2);
		
		if(e1 && e1->index < NUM_ENV && e1->flag == 0){
			printf("destructing geom %i\n", e1->index);
			e1->flag = 1;
			destruct_geom(damages[i].geom[0]);
			return;
		}
		else if(e2 && e2->index < NUM_ENV && e2->flag == 0){
			printf("destructing geom %i\n", e2->index);
			e2->flag = 1;
			destruct_geom(damages[i].geom[1]);
			return;
		}
	}
	num_damages=0;
}

void draw_2d()
{
	Go2d();
	setFontColor(0,0,0,1);
	const double *pos = dBodyGetPosition(players[0].body[BODY_CHASSI]);
	const double *lvel = dBodyGetLinearVel(players[0].body[BODY_CHASSI]);
	const double *rot = dBodyGetRotation(players[0].body[BODY_WHEEL1]);
	dsRightPrintBitmap(100, 600, va("%.0f km/h", lvel[0]*3.6));	
	dsPrintBitmap(10, 600, va("%.0f m", pos[0]));


	//dsPrintBitmap(10, 140, va("%.1f %.1f %.1f ", rot[0], rot[4], rot[8]));
	//dsPrintBitmap(10, 120, va("%.1f %.1f %.1f ", rot[1], rot[5], rot[9]));
	//dsPrintBitmap(10, 100, va("%.1f %.1f %.1f ", rot[2], rot[6], rot[10]));

	Exit2d();
}

void draw_world(){
	
	int i = 0;	
	const double *pos = dBodyGetPosition(players[0].body[BODY_CHASSI]);
	const double *lvel = dBodyGetLinearVel(players[0].body[BODY_CHASSI]);

	/*
	Go2d();
	dsSetColor(0,0,0);
	glPrint(0, 10, 10, 1, 0, "hello");
	Exit2d();
	*/
	/*
	printf("%.2f pos %.2f %2.f %.2f destpost %.2f %.2f %.2f\n", lvel[0]*3.6, pos[0], pos[1], pos[2], dpos[0], dpos[1], dpos[2]);
	*/
	dsSetColor(0,0,0);
	int offset = (int)pos[0];
	float vel = 1; //fabs(lvel[0]) * 0.1f;
	dMatrix3 R;
	dRSetIdentity(R);
	int road_flag = DRAW_NOLIGHT;
	for(i=-20; i<512;i++){
		double sides[]={2,1000,0.01};
		double pos[]={i + offset, 0, 0};
		if(!((i+offset) % 80)){
			dsSetColor(1,0,1);
			//dsDrawLine(from, to, 32*vel);
			sides[0] = 32; 
			dsDrawBoxD(pos, R, sides, road_flag);
			dsSetColor(0,0,0);
		}
		else if(!((i+offset) % 40)){
			sides[0] = 16; 
			dsSetColor(1,1,0);
			//dsDrawLine(from, to, 16*vel);
			dsDrawBoxD(pos, R, sides, road_flag);
			dsSetColor(0,0,0);
		}
		else if(!((i+offset) % 20)){
			dsSetColor(0,1,0);
			sides[0] = 8; 
			//dsDrawLine(from, to, 8*vel);
			dsDrawBoxD(pos, R, sides, road_flag);
		}
		else if(!((i+offset) % 10)){
			sides[0] = 1; 
			//dsDrawLine(from, to, 4*vel);
			dsDrawBoxD(pos, R, sides, road_flag);
		}
		else if(!((i+offset) % 5)){
			;//dsDrawLine(from, to, 2*vel);
			//dsDrawBoxD(pos, R, sides, 0);
		}	
		else {
			;//dsDrawBoxD(pos, R, sides, 0);
			//dsDrawLine(from, to, 1*vel);
		}
	}

	dsSetColor(1,0,0);
		
	const double *from = dBodyGetPosition(players[0].body[BODY_CAMERA]);
	dsSetLookAt(pos);

	draw_passenger();
	draw_xyz();

	//double f[]={0,0,4};
	if(camera){
		dsSetLookFrom(camera_pos);
	}
	else {
		double from_adjust[]={ from[0], from[1], from[2] > 0.1 ? from[2] :  0.1 };
		dsSetLookFrom(from_adjust);
	}
	for(i=0 ; i<NUM_BODIES-1; i++){
		dsSetColorAlpha(chassis[i].color[0], chassis[i].color[1], chassis[i].color[2], chassis[i].color[3]);
		if(players[0].geom[i]){
			draw_geom(players[0].geom[i], 0);
		}
	}
	for(i=0 ; i<NUM_GEOM; i++){
		if(dBodyIsEnabled(body[i])){
			dsSetColorAlpha(env[i].color[0], env[i].color[1], env[i].color[2], env[i].color[3]);
		}
		else {
			continue;
			dsSetColor(0,0,1);
			
		}
		if(geom[i]){
			draw_geom(geom[i], 0);
		}
	}

	//dsSetColor(1,1,0);
	//draw_geom(box[1], 0);
}

void draw_geom(dGeomID g, int flag){
        dReal radius, length;
        dReal sides[3] = {0.5,0.1,0.3};
        int type = dGeomGetClass(g);

        if (type==dSphereClass){
                dsDrawSphereD(dGeomGetPosition(g),dGeomGetRotation(g), (float)dGeomSphereGetRadius(g), flag);
        }
#ifdef WITH_CAPSULE
        else if(type==dCCylinderClass){
                dGeomCCylinderGetParams(g, &radius, &length);
                dsDrawCappedCylinderD(dGeomGetPosition(g),dGeomGetRotation(g), (float)length, (float)radius, flag);
        }
#endif
#ifdef WITH_CYLINDER
	else if (type == dCylinderClass) {
                dGeomCylinderGetParams(g, &radius, &length);
                dsDrawCylinderD(dGeomGetPosition(g), dGeomGetRotation(g), (float)length, (float)radius);
	}
#endif
        else if (type==dBoxClass){
                dGeomBoxGetLengths(g, sides);
                dsDrawBoxD(dGeomGetPosition(g), dGeomGetRotation(g), sides, flag);
        }
        else if (type=dGeomTransformClass){
                dVector3 actual_pos;
                dMatrix3 actual_R;
                dGeomID g2 = dGeomTransGet(g, actual_pos, actual_R);
                dsDrawSphereD(actual_pos, actual_R, (float)dGeomSphereGetRadius(g2), flag);
        }
        else {
                assert(0);
        }
}

static void nearCallback (void *data, dGeomID o1, dGeomID o2){
	
	int i;
	// exit without doing anything if the two bodies are connected by a joint
	dBodyID b1 = dGeomGetBody(o1);
	dBodyID b2 = dGeomGetBody(o2);

	if (b1 && b2 && dAreConnectedExcluding (b1,b2,dJointTypeContact)) {
		return;
	}
	// dont collide disabled geoms
	if((b1 && b2) && (!dBodyIsEnabled(b1) || !dBodyIsEnabled(b2))){
		return;
	}

	dContact contact[MAX_CONTACTS];   // up to MAX_CONTACTS contacts per box-box
	const double *wheel_rot = dBodyGetRotation(players[0].body[BODY_WHEEL1]);
	double wheel_axis = fabs(wheel_rot[5]);
	for (i=0; i<MAX_CONTACTS; i++) {
		/*
		contact[i].surface.mode = dContactBounce | dContactSoftCFM;
		contact[i].surface.mu = 1;
		contact[i].surface.bounce = 0.1f;
		//velocity over bounce_vel kommer att fa bounce 
		contact[i].surface.bounce_vel = 0.1f;
		contact[i].surface.soft_cfm = 0.1f;
		*/
		int wheel_class = dGeomGetClass(players[0].geom[wheels[0].body[1]]);
		//if (dGeomGetClass(o1) == dSphereClass || dGeomGetClass(o2) == dSphereClass){
		// if (dGeomGetClass(o1) == wheel_class || dGeomGetClass(o2) == wheel_class) {
		if (0) {
               		//contact[i].surface.mu = 20;
               		//contact[i].surface.mu = 1;
               		// contact[i].surface.mu = 20;
				
               		contact[i].surface.mu = 10000 - 10000 * wheel_axis;
			printf("mu %f\n", contact[i].surface.mu);
      			//contact[i].surface.soft_erp = 0.95;
      			contact[i].surface.soft_erp = 0.95;
      			// contact[i].surface.soft_cfm = 0.05;
      			contact[i].surface.soft_cfm = 0.05;
			//contact[i].surface.mode = dContactBounce | dContactSlip1 | dContactSlip2 | dContactSoftERP | dContactSoftCFM | dContactApprox1;
			contact[i].surface.mode = dContactSlip1 | dContactSlip2 | dContactSoftERP | dContactSoftCFM;//| dContactApprox1;
		}
		else {
               		//contact[i].surface.mu = 35.5;
               		//contact[i].surface.mu = 10000;
               		contact[i].surface.mu = 10000 - 10000 * wheel_axis;
      			contact[i].surface.soft_erp = 0.3;
      			contact[i].surface.soft_cfm = 0.05;
      			contact[i].surface.mode = dContactSoftERP | dContactSoftCFM;
			//contact[i].surface.mode = dContactSlip1 | dContactSlip2 | dContactSoftERP | dContactSoftCFM | dContactApprox1;
		}
		//contact[i].surface.mode = dContactBounce | dContactSlip1 | dContactSlip2 | dContactSoftERP | dContactSoftCFM | dContactApprox1;
      		//contact[i].surface.mu = 0.1;
      		//contact[i].surface.soft_erp = 0.95;
      		//contact[i].surface.soft_cfm = 0.05;
		contact[i].surface.slip1 = 0.0;
               	contact[i].surface.slip2 = 0.0;
               	//contact[i].surface.soft_erp = 0.8;
               	//contact[i].surface.soft_cfm = 0.01;
		//contact[i].surface.bounce = 0;
		contact[i].surface.bounce = 0;
		//velocity over bounce_vel kommer att fa bounce 
		//contact[i].surface.bounce_vel = 0.1f;
	}
	struct env_t *e1 = (struct env_t *)dGeomGetData(o1);
	struct env_t *e2 = (struct env_t *)dGeomGetData(o2);
	

	if (int numc = dCollide (o1,o2,MAX_CONTACTS,&contact[0].geom, sizeof(dContact)))
	{
		dMatrix3 RI;
		dRSetIdentity (RI);
		const dReal ss[3] = {0.09,0.09,0.09};
		for(i=0; i<numc; i++){
			dJointID c = dJointCreateContact (world,contactgroup,contact+i);
			dJointAttach (c,b1,b2);
			if(b1 && b2 && num_damages < NUM_DAMAGES){
				if(e1 && e1->index >= NUM_ENV){
					continue;
				}
				if(e2 && e2->index >= NUM_ENV){
					continue;
				}
				
				const dReal *velA = dBodyGetLinearVel(b1);	
				const dReal *velB = dBodyGetLinearVel(b2);	
				damages[num_damages].impact = get_impact(velA, velB);
				if(damages[num_damages].impact > 20.0f){
					damages[num_damages].geom[0] = o1;
                                	damages[num_damages].geom[1] = o2;
					num_damages++;
				}
			}
		}
	}
}

static void simLoop (int do_pause){
	float stepsize = 0.01f;
	int j;
	const dReal *lvel = dBodyGetLinearVel(players[0].body[BODY_CHASSI]);
	const dReal *pos = dBodyGetPosition(players[0].body[BODY_CHASSI]);
	//dBodyAddForce(players[0].body[BODY_CHASSI], 0, 0, -fabs(lvel[0]*10));
	//printf("adding force %f\n", -fabs(lvel[0]*10));
	dMatrix3 R;
	dRSetIdentity(R);
	//dBodySetRotation(players[0].body[BODY_CHASSI], R);
	//dBodySetRotation(players[0].body[BODY_CHASSI_OFF], R);
	//dBodySetPosition(players[0].body[BODY_CHASSI], pos[0], pos[1], chassis[BODY_CHASSI].pos[2]);

  	// steering: turn front wheels.
	// dReal steerv = steer * 0.6;
	dReal steerv = turn * 0.6;

  	dReal curr0 = dJointGetHinge2Angle1(players[0].joint[0]);
  	dReal curr1 = dJointGetHinge2Angle1(players[0].joint[1]);
  	dReal v0 = steerv - curr0;
  	dReal v1 = steerv - curr1;
  	if (v0 > 0.1) v0 = 0.1;
  	if (v0 < -0.1) v0 = -0.1;
  	v0 *= 30.0;
  	if (v1 > 0.1) v1 = 0.1;
  	if (v1 < -0.1) v1 = -0.1;
  	v1 *= 30.0;
	

	// printf("passenger_turn %i\n", passenger_turn);
	dReal rot[]={0,0,v0,v1};	

	for (j=0; j < NUM_WHEELS; j++){
		if(wheels[j].broken) continue;

		// set suspension
		dJointSetHinge2Param (players[0].joint[j], dParamSuspensionERP, 0.35);
		dJointSetHinge2Param (players[0].joint[j], dParamSuspensionCFM, 0.02);
	
		dReal curturn = dJointGetHinge2Angle1 (players[0].joint[j]);
		//dMessage (0,"curturn %e, turn %e, vel %e", curturn, turn, (turn-curturn)*1.0);
		// FRONT WHEELS
		// have no force to them
		if (wheels[j].steer) {
			dJointSetHinge2Param(players[0].joint[j], dParamVel,(turn-curturn)*1.0);
			// dJointSetHinge2Param(players[0].joint[j], dParamVel, rot[j]);
			dJointSetHinge2Param(players[0].joint[j], dParamFMax,dInfinity);
			// dJointSetHinge2Param(players[0].joint[j],dParamVel2,speed);
			dJointSetHinge2Param(players[0].joint[j], dParamFMax2, 0);

			dJointSetHinge2Param(players[0].joint[j], dParamLoStop, -0.75);
			dJointSetHinge2Param(players[0].joint[j], dParamHiStop, 0.75);
			
			// 
			dJointSetHinge2Param(players[0].joint[j], dParamFudgeFactor, 0.1); 
		}
		else {
		// BACK WHEELS
			dJointSetHinge2Param(players[0].joint[j],dParamVel,0);
			dJointSetHinge2Param(players[0].joint[j],dParamVel3,0);
			dJointSetHinge2Param(players[0].joint[j],dParamFMax,1000);
			dJointSetHinge2Param(players[0].joint[j],dParamFMax3,1000);

			// force drive 
			dJointSetHinge2Param(players[0].joint[j],dParamVel2,speed);
			dJointSetHinge2Param(players[0].joint[j],dParamFMax2,FMAX);
		}


		dBodyEnable(dJointGetBody(players[0].joint[j],0));
		dBodyEnable(dJointGetBody(players[0].joint[j],1));
	}

	if (turn_reset) {
		if (turn > 0) {
			turn -= 0.05f;
		}
		else if (turn < 0 ) {
			turn += 0.05f;
		}
	}

  	// dBodySetFiniteRotationAxis(players[0].body[wheels[2].body[1]], 0,0,1);
  	// dBodySetFiniteRotationAxis(players[0].body[wheels[3].body[1]], 0,0,1);
	
	// dJointSetHingeParam(passengers[0].seat_joint, dParamLoStop, -1.0);
	//dJointSetHingeParam(passengers[0].seat_joint, dParamHiStop, 1.0);

#ifdef SEAT_HINGE
	if (passenger_turn > 0) {
		//dJointSetHingeParam(passengers[0].seat_joint, dParamVel, 180);
		//dJointSetHingeParam(passengers[0].seat_joint, dParamFMax, 280);
		dJointSetHingeParam(passengers[0].seat_joint, dParamFMax, 10);
		//dBodyAddRelForce(passengers[0].ode_body[LIMB_HEAD], 0, 400, 0);
		dBodyAddRelForce(passengers[0].ode_body[LIMB_HEAD], 400, 0, 0);
	}
	else if (passenger_turn < 0) {
		//dJointSetHingeParam(passengers[0].seat_joint, dParamVel, -180);
		//dJointSetHingeParam(passengers[0].seat_joint, dParamFMax, 280);
		dJointSetHingeParam(passengers[0].seat_joint, dParamFMax, 10);
		// dBodyAddRelForce(passengers[0].ode_body[LIMB_HEAD], 0, -400, 0);
		dBodyAddRelForce(passengers[0].ode_body[LIMB_HEAD], -400, 0, 0);
	}
#else
	dJointSetAMotorParam(passengers[0].seat_motor, dParamFMax, 300);
	dJointSetAMotorParam(passengers[0].seat_motor, dParamVel, 0);

	if (seat_left) {
		dJointSetAMotorParam(passengers[0].seat_motor, dParamFMax3, 50);
		dJointSetAMotorParam(passengers[0].seat_motor, dParamVel3, 10);
		dBodyAddRelForce(passengers[0].ode_body[LIMB_TORSO], 0, 50, 0);
	}
	else if (seat_right) {
		dJointSetAMotorParam(passengers[0].seat_motor, dParamFMax3, 50);
		dJointSetAMotorParam(passengers[0].seat_motor, dParamVel3, -10);
		dBodyAddRelForce(passengers[0].ode_body[LIMB_TORSO], 0, -50, 0);
	}
	if (seat_forward) {
		dJointSetAMotorParam(passengers[0].seat_motor, dParamFMax2, 50);
		dJointSetAMotorParam(passengers[0].seat_motor, dParamVel2, 10);
		dBodyAddRelForce(passengers[0].ode_body[LIMB_TORSO], 0, 0, -50);
	}
	if (seat_back) {
		dJointSetAMotorParam(passengers[0].seat_motor, dParamFMax2, 50);
		dJointSetAMotorParam(passengers[0].seat_motor, dParamVel2, -10);
		dBodyAddRelForce(passengers[0].ode_body[LIMB_TORSO], 0, 0, 50);
	}
#endif
	/*
	else {
		dJointSetHingeParam(passengers[0].seat_joint, dParamLoStop, 0);
		dJointSetHingeParam(passengers[0].seat_joint, dParamHiStop, 0);
		dJointSetHingeParam(passengers[0].seat_joint, dParamFMax, 0);
		dJointSetHingeParam(passengers[0].seat_joint, dParamVel, 0);
	}
	*/

	if(pause == 0){
		#define REPS 10 
		for(int i=0; i< REPS; i++){
			dSpaceCollide (space, 0,&nearCallback);
			dWorldStep(world, stepsize/REPS);
			// dWorldStepFast1(world, stepsize/REPS, 10);
			dJointGroupEmpty (contactgroup);
		}
	}
	if (step_pause == 1) {
		pause = 1;	
		step_pause = 0;
	}

	handle_damages();
	handle_joints();

	dsSetFov(80 + lvel[0] * 0.05);
	// fprintf(stderr, "FOV %f\n", 80 + lvel[0] * 0.001);
	draw_world();
	draw_blood(0);
	draw_2d();
	push_geom();
	
}

static void mouse (int type, int button, int x, int y){

}

static void command (int cmd, int unicode, int mod, int type){

	passenger_turn = 0;
	seat_forward = seat_back = seat_left = seat_right = 0;

	if (!type) {
		if (cmd == 'a' || cmd == 'd') {
			turn_reset = 1;
		}
	}
	else if (type){ // key down
        	if(cmd == 's'){
                	speed *= 0.5;
		}
        	else if (cmd == 'w'){
                	speed += -3.4;
		}
		else if(cmd == 'd'){
                	turn += 0.1;
                	if (turn > 0.8)
                        	turn = 0.8;
			turn_reset = 0;
		}
        	else if (cmd == 'a'){
                	turn -= 0.1;
                	if (turn < -0.8)
                        	turn = -0.8;
			turn_reset = 0;
		}
#ifdef HINGE_SEAT 
		if (cmd == 'j') {
			passenger_turn = 1;
		}
		else if (cmd == 'k') {
			passenger_turn = -1;
		}
#else
		if (cmd == 'i') {
			seat_forward = 1;
		}
		else if (cmd  == 'j'){
			seat_left = 1;
		}
		else if (cmd  == 'k'){
			seat_back = 1;
		}
		else if (cmd  == 'l'){
			seat_right = 1;
		}
#endif
		else if(cmd == ' '){
			const dReal *pos = dBodyGetPosition(players[0].body[BODY_CHASSI]);
			double newpos[]={pos[0]-1, pos[1], pos[2]};
			dBodySetPosition(players[0].body[BODY_CHASSI],  newpos[0], newpos[1], newpos[2]);
		}
		else if (cmd == 'r'){
			reset_car(&players[0]);
			reset_geoms();
			reset_passenger(&passengers[0], 0, 0, 2.5);
		}
		else if (cmd == 'f'){
			dsSetFov(120);
		}
		else if (cmd == 'p'){
			pause ^= 1;
			printf("pause is %i\n", pause);
		}
		else if (cmd == 'o') {
			step_pause = 1;
			pause = 0;
		}
		else if (cmd == 'c'){
			camera ^= 1;
			const double *pos = dBodyGetPosition(players[0].body[BODY_CAMERA]);
			camera_pos[0] = pos[0];	
			camera_pos[1] = pos[1];	
			camera_pos[2] = pos[2];	
		}
		else if (cmd == '1'){
        		dWorldSetERP (world, dWorldGetERP(world)*0.9);
			printf("ERP %f\n", dWorldGetERP(world));
		}
		else if (cmd == '2'){
        		dWorldSetERP (world, dWorldGetERP(world)*1.1);
			printf("ERP %f\n", dWorldGetERP(world));
		}
		else if (cmd == '3'){
        		dWorldSetCFM(world, dWorldGetCFM(world)*1.1);
			printf("CFM %f\n", dWorldGetCFM(world));
		}
		else if (cmd == '4'){
        		dWorldSetCFM(world, dWorldGetCFM(world)*0.9);
			printf("CFM %f\n", dWorldGetCFM(world));
		}
		else if (cmd == 'x'){
			destruct_geom(geom[0]);
		}
		else if (cmd == 'y'){
			dBodyDisable(body[0]);
		}
	}
}

static void start(){
	DBG("start()\n");
}

static void pick(int shift, int type, double start[3], double end[3], int x, int y){

}

int mainx(int argc, char **argv){
	dsFunctions fn;
	fn.start = &start;
	fn.step = &simLoop;
	fn.command = &command;
	fn.mouse = &mouse;
	fn.pick = &pick;
	fn.stop = 0;

	create_world();

	dsSimulationLoop (argc, argv, 800, 600, &fn);

	return 0;
}

void draw_xyz(){
        //y-green
        //x-red
        //z-blue

        const float c[]={1,1,1};
        const float x[]={2,1,1};
        const float y[]={1,2,1};
        const float z[]={1,1,2};

        //dsSetColor(1,0,0);
        dsSetColorAlpha(1,0,0, 0.01f);
        dsDrawLine(c,x,1);
        //dsSetColor(0,1,0);
        dsSetColorAlpha(0,1,0, 0.01f);
        dsDrawLine(c,y,1);
        //dsSetColor(0,0,1);
        dsSetColorAlpha(0,0,1, 0.01f);
        dsDrawLine(c,z,1);

}
