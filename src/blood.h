#include <ode/ode.h>

#ifndef _BLOOD_H
#define _BLOOD_H

#define BLEEDS 256 
//#define BLEEDS 64
#define BLOOD_LIFESPAN 50 
struct blood_t {
	dGeomID geom;
	dBodyID body;
	int age;	
	int color; // custom color index
	float color3f[3]; // custom color rgb value
	int limb; // originating limb
	float radius;
	float random_radius;
};

void create_blood();  // create new blood
void set_blood_color(float rgb[3]);
void enable_blood();  // turn on physics for blood
void disable_blood(); // freeze blood in midair
void wipe_blood();    // remove and disable all blood from players
void draw_blood(int flag);
void player_bleed(int color, dReal x, dReal y, dReal z, const dReal *dir); // bleed from position and direction of hit
void player_bleed2(int color, int orig_limb, const dReal *pos, const dReal *dir, dReal speed, dReal amount);
struct blood_t * get_blood(int index);
void blood_damping();
dReal get_blood_contacts();
int get_num_blood_contacts();
void set_blood_contacts(dReal value);
void add_blood_contacts(int index, dReal amount);

#endif

