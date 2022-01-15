#include "glitchracer.h"
#include "gr_passenger.h"

/********
   0
   .
 -.=.-
  . .
  | |

 -y  +y
*********/

struct limb_t limbs[]={
 { LIMB_HEAD, 	GEOM_SPHERE, 		{ 0.4, 0.1, 0.1 }, 		{ 1,  0.0, 2 },			{ 1,0,0,1}, 	1 },
 { LIMB_TORSO, 	GEOM_BOX, 		{ 0.1, 0.1, 0.3 }, 		{ 1,  0.0, 1.5 },		{ 1,0,0,1}, 	1 },
 { LIMB_L_ARM, 	GEOM_BOX, 		{ 0.1, 0.4, 0.1 }, 		{ 1, -0.4, 1.5 },		{ 1,0,0,1}, 	1 },
 { LIMB_R_ARM, 	GEOM_BOX, 		{ 0.1, 0.4, 0.1 }, 		{ 1,  0.4, 1.5 },		{ 1,0,0,1}, 	1 },
 { LIMB_L_LEG, 	GEOM_BOX, 		{ 0.5, 0.1, 0.1 }, 		{ 1, -0.1, 1.1 },		{ 1,0,0,1}, 	1 },
 { LIMB_R_LEG, 	GEOM_BOX, 		{ 0.5, 0.1, 0.1 }, 		{ 1,  0.1, 1.1 },		{ 1,0,0,1}, 	1 }
};

struct joint_t joints[]={
 { JOINT_NECK, 		{ LIMB_TORSO, LIMB_HEAD  }, 	HINGE, 		{ 1.0,  0.0, 1.8 }, 		{ 0, 0, 1},		{ 0, 0.4 }, 	},
 { JOINT_L_SHOULDER, 	{ LIMB_TORSO, LIMB_L_ARM },	HINGE, 		{ 1.0, -0.1, 1.5 }, 		{ 0, 0, 1},		{ 0, 0.4 }, 	},
 { JOINT_R_SHOULDER, 	{ LIMB_TORSO, LIMB_R_ARM },	HINGE, 		{ 1.0,  0.1, 1.5 }, 		{ 0, 0, 1},		{ 0, 0.4 }, 	},
 { JOINT_L_HIP, 	{ LIMB_TORSO, LIMB_L_LEG }, 	HINGE, 		{ 1.0, -0.1, 1.2 }, 		{ 0, 0, 1},		{ 0, 0.4 }, 	},
 { JOINT_R_HIP, 	{ LIMB_TORSO, LIMB_R_LEG }, 	HINGE, 		{ 1.0,  0.1, 1.2 }, 		{ 0, 0, 1},		{ 0, 0.4 }, 	}
};


