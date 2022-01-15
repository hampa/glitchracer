#include "glitchracer.h"
#include "gr_car.h"
/******

x ---- x 
  ----
x ---- x

********/

struct chassi_t chassis[]={
 { BODY_CHASSI, 	GEOM_BOX, 		{ 2.0, 0.2, 0.3 }, 		{ 0, 0, 1.3 },		{0,0,0}, 20, { 1,0,0,1}, 	},
 // { BODY_CHASSI_OFF, 	GEOM_NONE, 		{ 6.0, 2.0, 1.0 }, 		{ 0, 0, 1 },		{0,0,0}, 1, { 1,0,0,1}, 	},
 { BODY_SEAT, 		GEOM_BOX, 		{ 0.1, 0.5, 0.1 }, 		{ -1, 0, 1.5 },		{0,0,0}, 1, { 1,0,0,1}, 	},

 { BODY_WHEEL1 , 	GEOM_CYLINDER, 		{ 0.7, 0.2, 0.5 }, 		{ -2.0, 0.0, 1.0 },	{90,0,0}, 1, { 1,0,0,1},	},
 //{ BODY_WHEEL2 , 	GEOM_CYLINDER, 		{ 0.7, 0.1, 0.5 }, 		{ -3.0, -0.2, 1.0 },	{90,0,0}, 1, { 1,1,0,1},	},
 { BODY_WHEEL3 , 	GEOM_CYLINDER, 		{ 0.7, 0.1, 0.5 }, 		{ 2.0, 0.0, 0.5 },	{90,0,0}, 1, { 0,1,0,1},	},
 //{ BODY_WHEEL4 , 	GEOM_CYLINDER, 		{ 0.7, 0.1, 0.5 }, 		{ 3.0, -0.2, 0.5 },	{90,0,0}, 1, { 0,1,1,1},	},

/*
 { BODY_WHEEL1 , 	GEOM_CAPSULE, 		{ 0.7, 0.1, 0.5 }, 		{ -3.0, 0.2, 1.0 },	{90,0,0}, { 1,0,0,1},	},
 { BODY_WHEEL2 , 	GEOM_CAPSULE, 		{ 0.7, 0.1, 0.5 }, 		{ -3.0, -0.2, 1.0 },	{90,0,0}, { 1,1,0,1},	},
 { BODY_WHEEL3 , 	GEOM_CAPSULE, 		{ 0.7, 0.1, 0.5 }, 		{ 3.0, 0.2, 0.5 },	{90,0,0}, { 0,1,0,1},	},
 { BODY_WHEEL4 , 	GEOM_CAPSULE, 		{ 0.7, 0.1, 0.5 }, 		{ 3.0, -0.2, 0.5 },	{90,0,0}, { 0,1,1,1},	},
*/
/*
 { BODY_WHEEL1 , 	GEOM_SPHERE, 		{ 0.7, 0.1, 0.5 }, 		{ -3.0, 1.2, 1.0 },	{90,0,0}, { 1,0,0,1},	},
 //{ BODY_WHEEL2 , 	GEOM_SPHERE, 		{ 0.7, 0.1, 0.5 }, 		{ -3.0, -1.2, 1.0 },	{90,0,0}, { 1,1,0,1},	},
 { BODY_WHEEL3 , 	GEOM_SPHERE, 		{ 0.7, 0.1, 0.5 }, 		{ 3.0, 1.2, 0.5 },	{90,0,0}, { 0,1,0,1},	},
 //{ BODY_WHEEL4 , 	GEOM_SPHERE, 		{ 0.7, 0.1, 0.5 }, 		{ 3.0, -1.2, 0.5 },	{90,0,0}, { 0,1,1,1},	},
*/

 { BODY_CAMERA , 	GEOM_NONE, 		{ 0.01, 0.01, 0.01 }, 		{ -5.0, -0.0, 3.0 },	{0,0,0}, 1, { 0,0,1,1},	 }
};

struct bolt_t bolts[]=
{
	BOLT_SEAT, { BODY_CHASSI, BODY_SEAT } 
};

/*
struct env_t env_old[]={
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 10, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 12, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 14, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 16, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 18, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 20, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 22, 5, 1},		{ 1,0,0,1}	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 24, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 26, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 27, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 28, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 30, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 32, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 34, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 36, 5, 1},		{ 1,0,0,1} 	},
 { BODY_CHASSI, 	GEOM_BOX, 		{ 1.0, 1.0, 1.0 }, 		{ 38, 5, 1},		{ 1,0,0,1} 	}
};
*/

struct wheel_t wheels[]={
{{ BODY_CHASSI, BODY_WHEEL1 }, 200000.0f, 0, 0},
//{{ BODY_CHASSI, BODY_WHEEL2 }, 200000.0f, 0, 0},
{{ BODY_CHASSI, BODY_WHEEL3 }, 200000.0f, 0, 1},
//{{ BODY_CHASSI, BODY_WHEEL4 }, 200000.0f, 0, 1}
};

