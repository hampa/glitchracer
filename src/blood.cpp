#include "blood.h"
#ifndef TB_SERVER
#include "drawstuff.h"
#endif
#include "ode_extra.h"
//#include "color.h"
#include "glitchracer.h"
#include <assert.h>

int blood_index = 0;
dReal blood_contacts;
int num_blood_contacts = 0;
float blood_color[3];
struct blood_t blood[BLEEDS];
extern dWorldID world;
extern dSpaceID space;


void set_blood_color(float rgb[3]){
	blood_color[0] = rgb[0];
	blood_color[1] = rgb[1];
	blood_color[2] = rgb[2];
}

void disable_blood(){
	for(int i = 0; i<BLEEDS; i++){
		dBodyDisable(blood[i].body);
	}
}

void enable_blood(){
	for(int i = 0; i<BLEEDS; i++){
		if(blood[i].age > 0){
			dBodyEnable(blood[i].body);
		}
	}
}

void remove_blood_particles(){
	for(int i =0; i< BLEEDS; i++){
		if(!dBodyIsEnabled(blood[i].body)){
			continue;	
		}
		if(blood[i].age-- == 0){
			dBodyDisable(blood[i].body);
			dGeomDisable(blood[i].geom);
		}
	}
}

void draw_blood(int flag){
	dsSetColor(1,0,0);
	for(int i =0; i<BLEEDS; i++){
		if(blood[i].age){
			dGeomID g = blood[i].geom;
			// customized blood color should over ride
			dsDrawSphereD(dGeomGetPosition(g), dGeomGetRotation(g), blood[i].radius, 0);
		}
	}
}

void player_bleed2(int color, int orig_limb, const dReal *pos, const dReal *dir, dReal speed, dReal amount){
	// amount 0 - 1
	if(blood_index>=BLEEDS){
		blood_index=0;
	}
	struct blood_t * b = &blood[blood_index++];
	assert(b);

	b->radius = b->random_radius * amount;
	b->age = BLOOD_LIFESPAN;
	b->limb = orig_limb;

	//pprintf("%f %f\n", b->random_radius, amount);
	//assert(b->radius > 0);

	// 1 = 1 meter
	// 0.01 = 1 cm // visible
	if(b->radius < 0.01f){	
		// printf("TO SMALL %f %f %f\n", b->radius, b->random_radius, amount);
		b->radius= 0.01f;	
		return;
	}
	else if (b->radius > 0.05){
		// printf("TO BIG %f %f %f\n", b->radius, b->random_radius, amount);
		b->radius = 0.03f;
	}

	dBodyEnable(b->body);
	dGeomEnable(b->geom);
	dBodySetPosition(b->body, pos[0] + b->random_radius, pos[1] + b->random_radius, pos[2] + b->random_radius);  
	dBodySetAngularVel(b->body , 0, 0, 0);
	dBodySetLinearVel(b->body, dir[0]*speed, dir[1]*speed, dir[2]*speed);
}

void player_bleed(int color, dReal x, dReal y, dReal z, const dReal *dir){
	if(blood_index>=BLEEDS-1){
		blood_index=0;
	}
	for(int i=0; i<1;i++){
		// this particle will stay alive BLOOD_LIFESPAN frames
		blood[blood_index].age = BLOOD_LIFESPAN;
		blood[blood_index].color = color ;
		
		dBodyEnable(blood[blood_index].body);
		dGeomEnable(blood[blood_index].geom);
		dBodySetPosition(blood[blood_index].body, x, y, z);  
		// clear any previous angular velocity
		dBodySetAngularVel(	blood[blood_index].body , 0, 0, 0);
		dBodySetLinearVel(blood[blood_index].body, dir[0]*(0.1*i), dir[1]*(0.1*i), dir[2]*(0.1*i));
		blood_index++;
	}
}

void wipe_blood(){

	blood_contacts = 0;

	for(int i = 0; i < BLEEDS; i++){
		// move to neutral ground
		dBodySetPosition(blood[i].body, 0,i+20,i+2);
		// disable and retire
		dBodyDisable(blood[i].body);
		dGeomDisable(blood[i].geom);
		blood[i].age = 0;
		blood[i].limb = 0;
	}
	blood_index=0;
}

struct blood_t * get_blood(int index){
	return &blood[index];
}

void blood_damping(){
	for(int i=0; i<BLEEDS; i++){
		if(blood[i].age){
			dBodyID body = blood[i].body;
			const dReal *linvel = dBodyGetLinearVel(body);
			const dReal *angvel = dBodyGetAngularVel(body);
			damp_body(body, linvel, angvel);
		}
	}
}

dReal get_blood_contacts(){
	return blood_contacts;
}

int get_num_blood_contacts(){
	return num_blood_contacts;
}

void set_blood_contacts(dReal value){
	blood_contacts = value;
	num_blood_contacts = 0;
}

void add_blood_contacts(int i, dReal amount){
	blood_contacts += (amount * blood[i].radius * 100.0f);
	num_blood_contacts++;
/*
#ifndef TB_SERVER
	if(blood[i].color == COLOR_NONE){
		dsBloodImpactD(dBodyGetPosition(blood[i].body), dBodyGetLinearVel(blood[i].body), blood[i].radius, blood_color); 
	}
	else {
		dsBloodImpactD(dBodyGetPosition(blood[i].body), dBodyGetLinearVel(blood[i].body), blood[i].radius, blood[i].color3f);
	}
#endif
*/
}

void create_blood(){
        dMass m;
        dMassSetBoxTotal(&m, 0.1f,1,1,1);
        for(int i = 0; i < BLEEDS; i++){
        	struct blood_t *b = &blood[i];

                b->body = dBodyCreate(world);
                dBodySetPosition(b->body, 0,10,i+1);
                // Minimum size should be 0.005 (about 1 px)
                // Too sinusodial
                //blood[i].geom = dCreateSphere(space, 0.01f*fabs(sinf(i*0.1)*2)+0.005f); 
                //blood[i].geom = dCreateSphere(space, 0.01f);
                b->radius = 0.01f*fabs(sinf(tanf((float)i))*2)+0.005f;
                b->random_radius = b->radius;
                //DBG("blood radius %f\n", radius);

                //blood[i].geom = dCreateSphere(space, 0.01f);  
                b->geom = dCreateSphere(space, 0.1f);

                b->age = 0;
                dGeomSetBody(b->geom, b->body);
                dBodySetMass(b->body, &m);
                // we are blood
                dGeomSetCategoryBits(b->geom, CAT_BLOOD);
                // we collide with ground only
                //dGeomSetCollideBits(blood[i].geom, CAT_GROUND);
                // TESTING
                dGeomSetCollideBits(b->geom, CAT_GROUND);
                dBodyDisable(b->body);
        }
}
