#ifndef _DRAWSTUFF_H
#define _DRAWSTUFF_H

#define KEY_NONE		0
#define KEY_ZOOM_IN		1
#define KEY_ZOOM_OUT		2
#define KEY_ZOOM		(KEY_ZOOM_IN | KEY_ZOOM_OUT)
#define KEY_LEFT		4
#define KEY_RIGHT		8
#define KEY_UP			16
#define KEY_DOWN		32


enum {
	DRAW_PLAIN,
	DRAW_OUTLINE,
	DRAW_SHADOW, 
	DRAW_TEXTURE,
	DRAW_IMPACT,
	DRAW_BLOOD,
	DRAW_JOINT_HOLD,
	DRAW_JOINT_RELAX,
	DRAW_NOLIGHT
};

enum {
	TEXTURE_HELP,
	TEXTURE_SPLATT,
	TEXTURE_LOGO,
	TEXTURE_SPONSOR,
	NUM_TEXTURES,
};


/* Used for callback from ODE */
typedef struct dsFunctions {
	int version;			/* put DS_VERSION here */
	/* version 1 data */
	void (*start)();		/* called before sim loop starts */
	void (*step) (int pause);	/* called before every frame */
	void (*command) (int cmd, int unicode, int mod, int type);	/* called if a command key is pressed */
	void (*mouse) (int type, int button, int x, int y);	/* called if a command key is pressed */
	void (*motion) (int mod, int x, int y);	/* called if a command key is pressed */
	void (*pick) (int shift, int type, double start[3], double end[3], int x, int y); //called if picking (mouse select)	
	void (*stop)();		/* called after sim loop exits */
} dsFunctions;


/* the main() function should fill in the dsFunctions structure then
 * call this.
 */
void dsSimulationLoop (int argc, char **argv, int window_width, int window_height, struct dsFunctions *fn);

/* set and get the camera position. xyz is the cameria position (x,y,z).
 * hpr contains heading, pitch and roll numbers in degrees. heading=0
 * points along the x axis, pitch=0 is looking towards the horizon, and
 * roll 0 is "unrotated".
 */
void dsSetCamera(int angle, float z, float hyp);
void dsSetCameraHyp(float hyp);
double dsGetCameraAngleFPS();
void dsSetCameraAngleHeight(float deg, float height);
void dsSetViewpoint (float xyz[3], float hpr[3]);
void dsGetViewpoint (float xyz[3], float hpr[3]);
void dsSetLookAt(const double xyz[3]);
void dsSetLookFrom(const double xyz[3]);
void dsSetLookShake(float xyz[3]);
const float * dsGetLookFrom();
const float * dsGetLookAt();

void dsSetTexture (int texture_number);
void dsSetHeadTexture (int player);
void dsLoadHeadTexture(int player, char *name, char *fallback);
void dsSetColor (float red, float green, float blue);
void dsSetColorAlpha (float red, float green, float blue, float alpha);
void dsSetBackgroundColor (float red, float green, float blue);

/* draw objects.
 *   - pos[] is the x,y,z of the center of the object.
 *   - R[] is a 3x3 rotation matrix for the object, stored by row like this:
 *        [ R11 R12 R13 0 ]
 *        [ R21 R22 R23 0 ]
 *        [ R31 R32 R33 0 ]
 *   - sides[] is an array of x,y,z side lengths.
 *   - all cylinders are aligned along the z axis.
 */

void dsDrawDisk(float x, float y, float size, float rot, int blend);
void dsDrawSquare(float x, float y, float z, float rot, float thickness, float side);
void dsDrawImage2d(int texture, float x, float y);
void dsDrawImage3d(int texture, float x, float y, float z);
void dsDrawScaledImage2d(int texture, float x, float y, float scale);
void dsDrawBox (const float pos[3], const float R[12], const float sides[3], int flag);
void dsDrawModel (const float pos[3], const float R[12]);
void dsDrawBox2 (const float pos[3], const float R[12], const float sides[3], 
float speed, float, float, float);
void dsDrawSphere (const float pos[3], const float R[12], float radius, int flag);
void dsDrawSphereHalf (const float pos[3], const float R[12], float radius, float yaw, float pitch, float roll, int flag);
void dsDrawTriangle (const float pos[3], const float R[12],
		     const float *v0, const float *v1, const float *v2, int solid);
void dsDrawCylinder (const float pos[3], const float R[12],
		     float length, float radius);
void dsDrawCappedCylinder (const float pos[3], const float R[12],
			   float length, float radius, int flag);
void dsDrawLine (const float pos1[3], const float pos2[3], float width);

/* these drawing functions are identical to the ones above, except they take
 * double arrays for `pos' and `R'.
 */
void dsDrawBoxD (const double pos[3], const double R[12], const double sides[3], int flag);
void dsDrawArrowD (const double pos[3], const double R[12], const double sides[3]);
void dsDrawBoxD2 (const double pos[3], const double R[12], const double sides[3], float, float, float, float);
void dsDrawSphereD (const double pos[3], const double R[12], const float radius, int flag);
void dsDrawModelD (const double pos[3], const double R[12]);
void dsDrawSphereHalfD (const double pos[3], const double R[12], const float radius, float y, float p, float r, int);
void dsDrawTriangleD (const double pos[3], const double R[12],
		      const float *v0, const float *v1, const float *v2, int solid);
void dsDrawCylinderD (const double pos[3], const double R[12],
		      float length, float radius);
void dsDrawCappedCylinderD (const double pos[3], const double R[12],
			    float length, float radius, int flag);
void dsDrawLineD (const double pos1[3], const double pos2[3], float width);

/* Set the drawn quality of the objects. Higher numbers are higher quality,
 * but slower to draw. This must be set before the first objects are drawn to
 * be effective.
 */
void dsSetSphereQuality (int n);		/* default = 1 */
void dsSetCappedCylinderQuality (int n);	/* default = 3 */

/* handle motion with WASD keys and arrows */
void dsWASDMotion(int mode);
void dsWASDMotionFPS(int mode);

void dsDisableEnviroment();
void dsEnableEnviroment();
void dsSetFov(float fov);

#endif
