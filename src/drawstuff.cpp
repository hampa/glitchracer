#ifdef WIN32
#include <windows.h>
#endif

#if defined(WIN32) && !defined(GL_BGR)
#define GL_BGR GL_BGR_EXT
#endif

#include "SDL.h"
#include <ode/config.h>
#ifdef LINUX
	#include <GL/freeglut.h>
#else
	#include <GLUT/glut.h>
#endif

#ifdef WIN32
	#include <GL/gl.h>
	#include <GL/glu.h>
#else
	#include <gl.h>
	#include <glu.h>
	#include <unistd.h>
#endif

#include <assert.h>
#include "drawstuff.h"
#include "font.h"
#include "texture.h"
//#include "color.h"
//#include "button.h"
#include "internal.h"
//#include "beatdown.h"


#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define RAD_TO_DEG (180.0/M_PI)
#define DEG_TO_RAD (M_PI/180.0)

// light vector. LIGHTZ is implicitly 1
#define LIGHTX (1.0f)
#define LIGHTY (0.4f)
#define SHADOW_INTENSITY (0.8f)
#define LIGHT_AMBIENT 0.3f      // ambient intesity
#define LIGHT_DIFFUSE 0.9f      // diffusecolor
#define LIGHT_SPECULAR 0.5f     // specular color 
#define LIGHT_SHININESS 0.25f   // shininess


float background_color[3];	
float camera_angle = 0;
float camera_fps_angle = 0; 
float camera_hyp = 4.0f;
float camera_height = 2.1f;
float shading_offset = 0.05f;
float ground_scale = 1.0f/1.0f;	// ground texture scale (1/size)
const float ground_ofsx = 0.5;		// offset of ground texture
const float ground_ofsy = 0.5;
const float sky_scale = 1.0f/4.0f;	// sky texture scale (1/size)
const float sky_height = 1.0f;		// sky height above viewpoint
int w_width;
int w_height;
static GLuint listnum[2]; 
float lookat[3];
float lookfrom[3];
static float color[4] = {0,0,0,0};	// current r,g,b,alpha color
GLUquadricObj *kukensphere=NULL; 
GLUquadricObj *kukendisk=NULL; 
void generateDisplayLists();
void update_lookfrom();

//***************************************************************************
// misc mathematics stuff
#ifndef dCROSS
#define dCROSS(a,op,b,c) \
  (a)[0] op ((b)[1]*(c)[2] - (b)[2]*(c)[1]); \
  (a)[1] op ((b)[2]*(c)[0] - (b)[0]*(c)[2]); \
  (a)[2] op ((b)[0]*(c)[1] - (b)[1]*(c)[0]);
#endif


inline float dDOT (const float *a, const float *b)
  { return ((a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2]); }


static void normalizeVector3 (float v[3])
{
  float len = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
  if (len <= 0.0f) {
    v[0] = 1;
    v[1] = 0;
    v[2] = 0;
  }
  else {
    len = 1.0f / (float)sqrt(len);
    v[0] *= len;
    v[1] *= len;
    v[2] *= len;
  }
}

//***************************************************************************
// PPM image object

typedef unsigned char byte;

//***************************************************************************
// the current drawing state (for when the user's step function is drawing)

static int tnum = 0;			// current texture number

static void setCamera (float x, float y, float z, float h, float p, float r)
{
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
	glRotatef (90, 0,0,1);
	glRotatef (90, 0,1,0);
	glRotatef (r, 1,0,0);
	glRotatef (p, 0,1,0);
	glRotatef (-h, 0,0,1);
	glTranslatef (-x,-y,-z);
}

// sets the material color, not the light color
static void setColor (float r, float g, float b, float alpha)
{
	GLfloat light_ambient[4],light_diffuse[4],light_specular[4];
	light_ambient[0] = r*LIGHT_AMBIENT;
	light_ambient[1] = g*LIGHT_AMBIENT;
	light_ambient[2] = b*LIGHT_AMBIENT;
	light_ambient[3] = alpha;
	light_diffuse[0] = r*LIGHT_DIFFUSE;
	light_diffuse[1] = g*LIGHT_DIFFUSE;
	light_diffuse[2] = b*LIGHT_DIFFUSE;
	light_diffuse[3] = alpha;
	light_specular[0] = LIGHT_SPECULAR;
	light_specular[1] = LIGHT_SPECULAR;
	light_specular[2] = LIGHT_SPECULAR;
	light_specular[3] = 1;
	glMaterialfv (GL_FRONT, GL_AMBIENT, light_ambient);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, light_diffuse);
	glMaterialfv (GL_FRONT, GL_SPECULAR, light_specular);
	glMaterialf (GL_FRONT, GL_SHININESS, LIGHT_SHININESS * 128);
}

// convert from ODE matrix to OpenGL Matrix
static void setTransform (const float pos[3], const float R[12])
{
	GLfloat matrix[16];
	matrix[0]=R[0];
	matrix[1]=R[4];
	matrix[2]=R[8];
	matrix[3]=0;
	matrix[4]=R[1];
	matrix[5]=R[5];
	matrix[6]=R[9];
	matrix[7]=0;
	matrix[8]=R[2];
	matrix[9]=R[6];
	matrix[10]=R[10];
	matrix[11]=0;
	matrix[12]=pos[0];
	matrix[13]=pos[1];
	matrix[14]=pos[2];
	matrix[15]=1;
	glPushMatrix();
	glMultMatrixf (matrix);
}

// set shadow projection transform
static void setShadowTransform()
{
	GLfloat matrix[16];
	for (int i=0; i<16; i++) matrix[i] = 0;
	matrix[0]=1;
	matrix[5]=1;
	matrix[8]=-LIGHTX;
	matrix[9]=-LIGHTY;
	matrix[15]=1;
	glPushMatrix();
	glMultMatrixf (matrix);
}

static void drawBox (const float sides[3])
{
	float lx = sides[0]*0.5f;
	float ly = sides[1]*0.5f;
	float lz = sides[2]*0.5f;

	// sides
	glBegin (GL_TRIANGLE_STRIP);
	glNormal3f (-1,0,0);
	glVertex3f (-lx,-ly,-lz);
	glVertex3f (-lx,-ly,lz);
	glVertex3f (-lx,ly,-lz);
	glVertex3f (-lx,ly,lz);
	glNormal3f (0,1,0);
	glVertex3f (lx,ly,-lz);
	glVertex3f (lx,ly,lz);
	glNormal3f (1,0,0);
	glVertex3f (lx,-ly,-lz);
	glVertex3f (lx,-ly,lz);
	glNormal3f (0,-1,0);
	glVertex3f (-lx,-ly,-lz);
	glVertex3f (-lx,-ly,lz);
	glEnd();

	// top face
	glBegin (GL_TRIANGLE_FAN);
	glNormal3f (0,0,1);
	glVertex3f (-lx,-ly,lz);
	glVertex3f (lx,-ly,lz);
	glVertex3f (lx,ly,lz);
	glVertex3f (-lx,ly,lz);
	glEnd();

	// bottom face
	glBegin (GL_TRIANGLE_FAN);
	glNormal3f (0,0,-1);
	glVertex3f (-lx,-ly,-lz);
	glVertex3f (-lx,ly,-lz);
	glVertex3f (lx,ly,-lz);
	glVertex3f (lx,-ly,-lz);
	glEnd();
}


// This is recursively subdivides a triangular area (vertices p1,p2,p3) into
// smaller triangles, and then draws the triangles. All triangle vertices are
// normalized to a distance of 1.0 from the origin (p1,p2,p3 are assumed
// to be already normalized). Note this is not super-fast because it draws
// triangles rather than triangle strips.

static void drawPatch (float p1[3], float p2[3], float p3[3], int level)
{
	int i;
	if (level > 0) {
		float q1[3],q2[3],q3[3];		 // sub-vertices
		for (i=0; i<3; i++) {
			q1[i] = 0.5f*(p1[i]+p2[i]);
			q2[i] = 0.5f*(p2[i]+p3[i]);
			q3[i] = 0.5f*(p3[i]+p1[i]);
		}
		float length1 = (float)(1.0/sqrt(q1[0]*q1[0]+q1[1]*q1[1]+q1[2]*q1[2]));
		float length2 = (float)(1.0/sqrt(q2[0]*q2[0]+q2[1]*q2[1]+q2[2]*q2[2]));
		float length3 = (float)(1.0/sqrt(q3[0]*q3[0]+q3[1]*q3[1]+q3[2]*q3[2]));
		for (i=0; i<3; i++) {
			q1[i] *= length1;
			q2[i] *= length2;
			q3[i] *= length3;
		}
		drawPatch (p1,q1,q3,level-1);
		drawPatch (q1,p2,q2,level-1);
		drawPatch (q1,q2,q3,level-1);
		drawPatch (q3,q2,p3,level-1);
  }
  else {
    glNormal3f (p1[0],p1[1],p1[2]);
    glVertex3f (p1[0],p1[1],p1[2]);
    glNormal3f (p2[0],p2[1],p2[2]);
    glVertex3f (p2[0],p2[1],p2[2]);
    glNormal3f (p3[0],p3[1],p3[2]);
    glVertex3f (p3[0],p3[1],p3[2]);
  }
}


// draw a sphere of radius 1
static int sphere_quality = 1;
void generateDisplayLists()
{
	// icosahedron data for an icosahedron of radius 1.0
# define ICX 0.525731112119133606f
# define ICZ 0.850650808352039932f
	static GLfloat idata[12][3] = {
		{-ICX, 0, ICZ},
		{ICX, 0, ICZ},
		{-ICX, 0, -ICZ},
		{ICX, 0, -ICZ},
		{0, ICZ, ICX},
		{0, ICZ, -ICX},
		{0, -ICZ, ICX},
		{0, -ICZ, -ICX},
		{ICZ, ICX, 0},
		{-ICZ, ICX, 0},
		{ICZ, -ICX, 0},
		{-ICZ, -ICX, 0}
	};

	static int index[20][3] = {
		{0, 4, 1},	  {0, 9, 4},
		{9, 5, 4},	  {4, 5, 8},
		{4, 8, 1},	  {8, 10, 1},
		{8, 3, 10},   {5, 3, 8},
		{5, 2, 3},	  {2, 7, 3},
		{7, 10, 3},   {7, 6, 10},
		{7, 11, 6},   {11, 0, 6},
		{0, 1, 6},	  {6, 1, 10},
		{9, 0, 11},   {9, 11, 2},
		{9, 2, 5},	  {7, 2, 11},
	};

	listnum[0] = glGenLists (1);
	glNewList (listnum[0], GL_COMPILE);
	glBegin (GL_TRIANGLES);
	for (int i=0; i<20; i++) {
		drawPatch (&idata[index[i][2]][0],&idata[index[i][1]][0],
				&idata[index[i][0]][0],sphere_quality);
	}
	glEnd();
	glEndList();

	listnum[1] = glGenLists(2);
	glNewList(listnum[1], GL_COMPILE);
	glutSolidSphere(1, 12, 12);
	glEndList();
}

static void drawSphere(){
	assert(listnum[0]);
	glCallList (listnum[0]);
}

// glut sphere. A bit slower but looks better
static void drawSphere2(){
	assert(listnum[1]);
	glCallList (listnum[1]);
}

static void drawSphereShadow (float px, float py, float pz, float radius)
{
	// calculate shadow constants based on light vector
	static int init=0;
	static float len2,len1,scale;
	if (!init) {
		len2 = LIGHTX*LIGHTX + LIGHTY*LIGHTY;
		len1 = 1.0f/(float)sqrt(len2);
		scale = (float) sqrt(len2 + 1);
		init = 1;
	}

	// map sphere center to ground plane based on light vector
	px -= LIGHTX*pz;
	py -= LIGHTY*pz;

	const float kx = 0.96592582628907f;
	const float ky = 0.25881904510252f;
	float x=radius, y=0;

	glBegin (GL_TRIANGLE_FAN);
	for (int i=0; i<24; i++) {
		// for all points on circle, scale to elongated rotated shadow and draw
		float x2 = (LIGHTX*x*scale - LIGHTY*y)*len1 + px;
		float y2 = (LIGHTY*x*scale + LIGHTX*y)*len1 + py;
		glTexCoord2f (x2*ground_scale+ground_ofsx,y2*ground_scale+ground_ofsy);
		glVertex3f (x2,y2,0);

		// rotate [x,y] vector
		float xtmp = kx*x - ky*y;
		y = ky*x + kx*y;
		x = xtmp;
	}
	glEnd();
}


static void drawTriangle (const float *v0, const float *v1, const float *v2, int solid){
	float u[3],v[3],normal[3];
	u[0] = v1[0] - v0[0];
	u[1] = v1[1] - v0[1];
	u[2] = v1[2] - v0[2];
	v[0] = v2[0] - v0[0];
	v[1] = v2[1] - v0[1];
	v[2] = v2[2] - v0[2];
	dCROSS (normal,=,u,v);
	normalizeVector3 (normal);

	glBegin(solid ? GL_TRIANGLES : GL_LINE_STRIP);
	glNormal3fv (normal);
	glVertex3fv (v0);
	glVertex3fv (v1);
	glVertex3fv (v2);
	glEnd();
}


// draw a capped cylinder of length l and radius r, aligned along the x axis
static int capped_cylinder_quality = 3;

static void drawCappedCylinder (float l, float r){
	int i,j;
	float tmp,nx,ny,nz,start_nx,start_ny,a,ca,sa;
	// number of sides to the cylinder (divisible by 4):
	const int n = capped_cylinder_quality*4;

	l *= 0.5;
	a = float(M_PI*2.0)/float(n);
	sa = (float) sin(a);
	ca = (float) cos(a);

	// draw cylinder body
	ny=1; nz=0;		  // normal vector = (0,ny,nz)
	glBegin (GL_TRIANGLE_STRIP);
	for (i=0; i<=n; i++) {
		glNormal3d (ny,nz,0);
		glVertex3d (ny*r,nz*r,l);
		glNormal3d (ny,nz,0);
		glVertex3d (ny*r,nz*r,-l);
		// rotate ny,nz
		tmp = ca*ny - sa*nz;
		nz = sa*ny + ca*nz;
		ny = tmp;
	}
	glEnd();

	// draw first cylinder cap
	start_nx = 0;
	start_ny = 1;
	for (j=0; j<(n/4); j++) {
		// get start_n2 = rotated start_n
		float start_nx2 =  ca*start_nx + sa*start_ny;
		float start_ny2 = -sa*start_nx + ca*start_ny;
		// get n=start_n and n2=start_n2
		nx = start_nx; ny = start_ny; nz = 0;
		float nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
		glBegin (GL_TRIANGLE_STRIP);
		for (i=0; i<=n; i++) {
			glNormal3d (ny2,nz2,nx2);
			glVertex3d (ny2*r,nz2*r,l+nx2*r);
			glNormal3d (ny,nz,nx);
			glVertex3d (ny*r,nz*r,l+nx*r);
			// rotate n,n2
			tmp = ca*ny - sa*nz;
			nz = sa*ny + ca*nz;
			ny = tmp;
			tmp = ca*ny2- sa*nz2;
			nz2 = sa*ny2 + ca*nz2;
			ny2 = tmp;
		}
		glEnd();
		start_nx = start_nx2;
		start_ny = start_ny2;
	}

	// draw second cylinder cap
	start_nx = 0;
	start_ny = 1;
	for (j=0; j<(n/4); j++) {
		// get start_n2 = rotated start_n
		float start_nx2 = ca*start_nx - sa*start_ny;
		float start_ny2 = sa*start_nx + ca*start_ny;
		// get n=start_n and n2=start_n2
		nx = start_nx; ny = start_ny; nz = 0;
		float nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
		glBegin (GL_TRIANGLE_STRIP);
		for (i=0; i<=n; i++) {
			glNormal3d (ny,nz,nx);
			glVertex3d (ny*r,nz*r,-l+nx*r);
			glNormal3d (ny2,nz2,nx2);
			glVertex3d (ny2*r,nz2*r,-l+nx2*r);
			// rotate n,n2
			tmp = ca*ny - sa*nz;
			nz = sa*ny + ca*nz;
			ny = tmp;
			tmp = ca*ny2- sa*nz2;
			nz2 = sa*ny2 + ca*nz2;
			ny2 = tmp;
		}
		glEnd();
		start_nx = start_nx2;
		start_ny = start_ny2;
	}

	glPopMatrix();
}

// draw a cylinder of length l and radius r, aligned along the z axis
static void drawCylinder (float l, float r, float zoffset){
	int i;
	float tmp,ny,nz,a,ca,sa;
	const int n = 24;	// number of sides to the cylinder (divisible by 4)

	l *= 0.5;
	a = float(M_PI*2.0)/float(n);
	sa = (float) sin(a);
	ca = (float) cos(a);

	// draw cylinder body
	ny=1; nz=0;		  // normal vector = (0,ny,nz)
	glBegin (GL_TRIANGLE_STRIP);
	for (i=0; i<=n; i++) {
		glNormal3d (ny,nz,0);
		glVertex3d (ny*r,nz*r,l+zoffset);
		glNormal3d (ny,nz,0);
		glVertex3d (ny*r,nz*r,-l+zoffset);
		// rotate ny,nz
		tmp = ca*ny - sa*nz;
		nz = sa*ny + ca*nz;
		ny = tmp;
	}
	glEnd();

	// draw top cap
	glShadeModel (GL_FLAT);
	ny=1; nz=0;		  // normal vector = (0,ny,nz)
	glBegin (GL_TRIANGLE_FAN);
	glNormal3d (0,0,1);
	glVertex3d (0,0,l+zoffset);
	for (i=0; i<=n; i++) {
		if (i==1 || i==n/2+1)
			setColor (color[0]*0.75f,color[1]*0.75f,color[2]*0.75f,color[3]);
		glNormal3d (0,0,1);
		glVertex3d (ny*r,nz*r,l+zoffset);
		if (i==1 || i==n/2+1)
			setColor (color[0],color[1],color[2],color[3]);

		// rotate ny,nz
		tmp = ca*ny - sa*nz;
		nz = sa*ny + ca*nz;
		ny = tmp;
	}
	glEnd();

	// draw bottom cap
	ny=1; nz=0;		  // normal vector = (0,ny,nz)
	glBegin (GL_TRIANGLE_FAN);
	glNormal3d (0,0,-1);
	glVertex3d (0,0,-l+zoffset);
	for (i=0; i<=n; i++) {
		if (i==1 || i==n/2+1)
			setColor (color[0]*0.75f,color[1]*0.75f,color[2]*0.75f,color[3]);
		glNormal3d (0,0,-1);
		glVertex3d (ny*r,nz*r,-l+zoffset);
		if (i==1 || i==n/2+1)
			setColor (color[0],color[1],color[2],color[3]);

		// rotate ny,nz
		tmp = ca*ny + sa*nz;
		nz = -sa*ny + ca*nz;
		ny = tmp;
	}
	glEnd();
}

static void setupOutlineDrawingMode(){
	glDisable (GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST); 
	glDisable(GL_LIGHTING); 
	glShadeModel(GL_FLAT);
	glColor4f(0,0,0, color[3]);
}

// current camera position and orientation
static float view_xyz[3];	// position x,y,z
static float view_hpr[3];	// heading, pitch, roll (degrees)

// initialize the above variables
static void initMotionModel(){
	view_xyz[0] = 2;
	view_xyz[1] = 0;
	view_xyz[2] = 1;
	view_hpr[0] = 180;
	view_hpr[1] = 0;
	view_hpr[2] = 0;
}

static void wrapCameraAngles(){
	for (int i=0; i<3; i++) {
		while (view_hpr[i] > 180) {
			view_hpr[i] -= 360;
		}
		while (view_hpr[i] < -180) {
			view_hpr[i] += 360;
		}
	}
}

/***********************************/
// public API dsXXX functions 

void dsWASDMotionFPS(int mode){
	if(mode & KEY_LEFT){
		camera_fps_angle -= 3;
	}
	else if (mode & KEY_RIGHT){
		camera_fps_angle += 3;
	}
	if(mode & (KEY_LEFT | KEY_RIGHT)){
		if(camera_fps_angle>360) {
			camera_fps_angle-=360;
		}
		else if(camera_fps_angle<0) {
			camera_fps_angle+=360; 
		}
	}
}

// handle WASD arrow motion from gui.cpp
// Shift WS moves camera view up and down
void dsWASDMotion(int mode){
	if(mode & KEY_ZOOM_IN){
		camera_hyp -= 0.3f; 
	}
	else if (mode & KEY_ZOOM_OUT){
		camera_hyp += 0.3f; 
	}
	else if (mode & KEY_LEFT){
		camera_angle -= 3;
	}
	else if (mode & KEY_RIGHT){
		camera_angle += 3;
	}
	else if (mode & KEY_UP){
		camera_height += 0.1;
	}
	else if (mode & KEY_DOWN){
		camera_height -= 0.1;
	}
	else {
		assert(0);
		return;
	}

	if(mode & (KEY_LEFT | KEY_RIGHT)){
		if(camera_angle>360) {
			camera_angle-=360;
		}
		else if(camera_angle<0) {
			camera_angle+=360; 
		}
	}
	else if (mode & (KEY_UP | KEY_DOWN)){
		if(camera_height < 0.5){
			camera_height = 0.5;
		}
		else if (camera_height > 5){
			camera_height = 5;
		}
	}
	update_lookfrom();
}

// call this to update the current camera position. the bits in `mode' say
// if the left (1), middle (2) or right (4) mouse button is pressed, and
// (deltax,deltay) is the amount by which the mouse pointer has moved.
void dsMotion (int mode, int deltax, int deltay) {
	camera_angle += (float)deltax * 0.2f;;
	camera_hyp += (float)deltay * 0.01f;
	update_lookfrom();
}

static int use_fullscreen=0;
static int use_textures=1;		// 1 if textures to be drawn
static int use_shadows=0;		// 1 if shadows to be drawn
int head_idx = 0;

static Texture2 *logo_texture = 0;
static Texture2 *head_textures[2]={0,0};
static Texture2 *textures[NUM_TEXTURES];


char message[32];


void dsStartGraphics (int width, int height)
{
  	int no =  6;
  	//if (fn->version >= 2 && fn->path_to_textures) prefix = fn->path_to_textures;

  	//regenerate gllist
	memset(listnum, 0, sizeof(listnum));

	//save globals
  	w_width = width;
  	w_height = height;
	int num_buttons = 6;
	int space = 16;
	int size = 64;
	int offset = width-(num_buttons*size+((num_buttons-1)*space));
	int ypos = size + space;
	offset /= 2;
	//offset=168;

	if(!InitFontGL(width, height)){
		fprintf(stderr, "FAILed to load fonts\n");
		exit(1);
	}
	assert(kukensphere == NULL);
	assert(kukendisk == NULL);

	// generate objects display lists
	generateDisplayLists();
	
	kukensphere=gluNewQuadric();
	kukendisk=gluNewQuadric();
	gluQuadricTexture(kukensphere, GL_TRUE);

	//head_textures[0] = new Texture2("heads", "tori");
	//head_textures[1] = new Texture2("heads", "uke");
	textures[TEXTURE_HELP] = new Texture2("data/textures", "help");
	textures[TEXTURE_SPLATT] = new Texture2("data/textures", "splatt1");
	textures[TEXTURE_LOGO] = new Texture2("data/textures", "logo");
	textures[TEXTURE_SPONSOR] = new Texture2("data/textures", "sponsor");

	// clear this here
	glClearColor(background_color[0], background_color[1], background_color[2], 0);

}


void dsStopGraphics() { 
	if(kukensphere){
		gluDeleteQuadric(kukensphere);
		kukensphere = NULL;
	}
	if(kukendisk){
		gluDeleteQuadric(kukendisk);
		kukendisk = NULL;
	}
	KillFont();

	glDeleteLists(listnum[1],2);
	glDeleteLists(listnum[0],1);

	delete head_textures[0];
	delete head_textures[1];

	for(int i=0; i<NUM_TEXTURES; i++){
		assert(textures[i]);
		delete textures[i];
		textures[i] = 0;
	}

}
 
double dsGetCameraAngle(){
	return (double)camera_angle;
}

double dsGetCameraAngleFPS(){
	return (double)camera_fps_angle;
}

void dsSetCameraAngle(float deg){
	camera_angle=deg;
	update_lookfrom();
}

void dsSetCameraAngleHeight(float deg, float height){
	camera_angle=deg;
	camera_height=height;
}

void dsGetCameraVector(double *vec){
	vec[0]=lookat[0]-lookfrom[0];
	vec[1]=lookat[1]-lookfrom[1];
	vec[2]=lookat[2]-lookfrom[2];
}

void update_lookfrom(){
	lookfrom[1] = sinf(camera_angle*DEG_TO_RAD)*camera_hyp+lookat[1];
	lookfrom[0] = cosf(camera_angle*DEG_TO_RAD)*camera_hyp+lookat[0];
	lookfrom[2] = camera_height;
}

void dsSetLookAt(const double xyz[3]){
	lookat[0] = (float)xyz[0];
	lookat[1] = (float)xyz[1];
	lookat[2] = (float)xyz[2];
	//update lookfrom aswell
	update_lookfrom();
}

void dsSetLookFrom(const double xyz[3]){
	lookfrom[0] = xyz[0];
	lookfrom[1] = xyz[1];
	lookfrom[2] = xyz[2];
	camera_height = xyz[2];
}

const float * dsGetLookFrom(){
	return lookfrom;
}

const float * dsGetLookAt(){
	return lookat;
}

/* background color. Usually white but mods can change it */
void dsSetBackgroundColor(float r, float g, float b){
	background_color[0] = r;	
	background_color[1] = g;	
	background_color[2] = b;	
	glClearColor(r, g, b, 0);
}

void dsDrawFrame (int width, int height, dsFunctions *fn, int pause){
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);   
	glClearDepth(1.0f);

	glEnable (GL_LIGHTING);
  	glEnable (GL_LIGHT0);
  	glDisable (GL_TEXTURE_2D);
  	glDisable (GL_TEXTURE_GEN_S);
  	glDisable (GL_TEXTURE_GEN_T);
  	glShadeModel (GL_FLAT);
  	glEnable (GL_DEPTH_TEST);
  	glDepthFunc (GL_LESS);
  	glCullFace (GL_BACK);
  	glFrontFace (GL_CCW);

	glLoadIdentity();

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();

  	gluLookAt ( lookfrom[0], lookfrom[1], lookfrom[2], lookat[0], lookat[1], lookat[2], 0.0, 0.0, 1.0);

  	static GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  	static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
  	static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = { lookfrom[0], lookfrom[1], 1.0, 1.0 };

  	glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
  	glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  	glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv (GL_LIGHT0, GL_POSITION, light_position);

  	glColor3f (1.0, 1.0, 1.0);

	assert(fn->step);
	fn->step (pause);
}

int dsGetShadows(){
	return use_shadows;
}

void dsSetShadows (int a){
	use_shadows = (a != 0);
}

int dsGetTextures(){
	return use_textures;
}

void dsEnableEnviroment(){
	use_shadows = 1;
}

void dsDisableEnviroment(){
	use_shadows = 0;
}

void dsSetTextures (int a){
	use_textures = (a != 0);
}

void dsSetHeadTexture(int player){
	assert(player == 0 || player == 1);
	head_idx = player;
}	

void dsLoadHeadTexture(int player, char *name, char *fallback){
	assert(player == 0 || player == 1);
	if(head_textures[player]->loadTexture("heads", name) == false){
		head_textures[player]->loadTexture("heads", fallback);
	}
}

// sets lighting and texture modes, sets current color
static void setupDrawingMode(){
	glEnable (GL_LIGHTING);
	if (tnum) {
		if (use_textures) {
			glEnable (GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, head_textures[0]->image);
				
			glEnable (GL_TEXTURE_GEN_S);
			glEnable (GL_TEXTURE_GEN_T);
			glTexGeni (GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
			glTexGeni (GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
			static GLfloat s_params[4] = {1.0f,1.0f,0.0f,1};
			static GLfloat t_params[4] = {0.817f,-0.817f,0.817f,1};
		}
		else {
			glDisable (GL_TEXTURE_2D);
		}
	}
	else {
		glDisable (GL_TEXTURE_2D);
	}
	setColor (color[0],color[1],color[2],color[3]);

	if (color[3] < 1) {
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
}


static void setShadowDrawingMode()
{
	glDisable (GL_LIGHTING);
	if (use_textures) {
		assert(0);
		glEnable (GL_TEXTURE_2D);
		//ground_texture->bind (1);
		glColor3f (SHADOW_INTENSITY,SHADOW_INTENSITY,SHADOW_INTENSITY);
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_TEXTURE_GEN_S);
		glEnable (GL_TEXTURE_GEN_T);
		glTexGeni (GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
		glTexGeni (GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
		static GLfloat s_params[4] = {ground_scale,0,0,ground_ofsx};
		static GLfloat t_params[4] = {0,ground_scale,0,ground_ofsy};
		glTexGenfv (GL_S,GL_EYE_PLANE,s_params);
		glTexGenfv (GL_T,GL_EYE_PLANE,t_params);
	}
	else {
		glDisable (GL_TEXTURE_2D);
		glColor4f (background_color[0]*SHADOW_INTENSITY, 
					background_color[1]*SHADOW_INTENSITY, 
					background_color[2]*SHADOW_INTENSITY, 0.9);
	}
	glDepthRange (0,0.9999);
}


void dsSimulationLoop (int argc, char **argv,
		int window_width, int window_height,
		dsFunctions *fn)
{

	use_textures=0;
	use_shadows=0;

	background_color[0]  = background_color[1] =  background_color[2] = 1.0f;

	// look for flags that apply to us
	int initial_pause = 0;
	for (int i=1; i<argc; i++) {
		if (strcmp(argv[i],"-notex")==0) use_textures = 0;
		if (strcmp(argv[i],"-full")==0) use_fullscreen = 1;
		if (strcmp(argv[i],"-noshadow")==0) use_shadows = 0;
		if (strcmp(argv[i],"-noshadows")==0) use_shadows = 0;
		if (strcmp(argv[i],"-pause")==0) initial_pause = 1;
	}

#ifdef LINUX
	// freeglut3 wants this
	glutInit(&argc, argv); 	
#endif

	initMotionModel();
	dsPlatformSimLoop (window_width,window_height,fn,initial_pause);
}

void dsSetViewpoint (float xyz[3], float hpr[3]){
	if (xyz) {
		view_xyz[0] = xyz[0];
		view_xyz[1] = xyz[1];
		view_xyz[2] = xyz[2];
	}
	if (hpr) {
		view_hpr[0] = hpr[0];
		view_hpr[1] = hpr[1];
		view_hpr[2] = hpr[2];
		wrapCameraAngles();
	}
}

void dsSetCameraHyp(float hyp){
	camera_hyp=hyp;
	lookfrom[1] = sinf(camera_angle*DEG_TO_RAD)*camera_hyp+lookat[1];
	lookfrom[0] = cosf(camera_angle*DEG_TO_RAD)*camera_hyp+lookat[2];
}

void dsSetCamera(int angle, float z, float hyp){

	camera_hyp=hyp;
	camera_angle=angle;
	camera_height=z;
	lookfrom[1] = sinf(camera_angle*DEG_TO_RAD)*camera_hyp+lookat[1];
	lookfrom[0] = cosf(camera_angle*DEG_TO_RAD)*camera_hyp+lookat[2];
	lookfrom[2]=z;
}

void dsGetViewpoint (float xyz[3], float hpr[3])
{
	if (xyz) {
		xyz[0] = view_xyz[0];
		xyz[1] = view_xyz[1];
		xyz[2] = view_xyz[2];
	}
	if (hpr) {
		hpr[0] = view_hpr[0];
		hpr[1] = view_hpr[1];
		hpr[2] = view_hpr[2];
	}
}


void dsSetTexture (int texture_number)
{
	tnum = texture_number;
}


void dsSetColor (float red, float green, float blue)
{
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = 1;
	glColor3f(red, green, blue);
}


void dsSetColorAlpha (float red, float green, float blue,
		float alpha)
{
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;
	glColor4f(red, green, blue, alpha);
}

void dsDrawBox (const float pos[3], const float R[12], const float sides[3], int flag)
{
	float s2[]={sides[0]+shading_offset, sides[1]+shading_offset, sides[2]+shading_offset};

	if(flag==DRAW_SHADOW){
		setShadowDrawingMode();
		setShadowTransform(); //does pushmatrix
		setTransform (pos,R); //does pushmatrix
		drawBox (sides);
		glPopMatrix();
		glPopMatrix();
		glPopMatrix();
		glDepthRange (0,1);
	}
	else if(flag==DRAW_OUTLINE){
		setTransform(pos,R); //does pushmatrix
		setupOutlineDrawingMode();
		drawBox(s2);
		glPopMatrix();
	}
	else if (flag == DRAW_NOLIGHT){
		setTransform(pos,R); //does pushmatrix
		setupDrawingMode();
		glEnable(GL_DEPTH_TEST);
        	glDisable(GL_LIGHTING);
		drawBox(sides);
		glPopMatrix();
        	glEnable(GL_LIGHTING);
	}
	else {
		setTransform(pos,R); //does pushmatrix
		setupDrawingMode();
		glEnable(GL_DEPTH_TEST);
        	glEnable(GL_LIGHTING);
		glColor3f(1,1,1);
		drawBox(sides);
		glPopMatrix();
	}
}

void dsDrawSphereHalf (
		const float pos[3], const float R[12], 
		float radius, 
		float yaw, float pitch, float roll,
		int flag)
{

	GLdouble eqn[4]={0,0,0,0};	

	assert (flag == DRAW_JOINT_HOLD || flag == DRAW_JOINT_RELAX);


	glShadeModel (GL_FLAT);

	setupDrawingMode();
	setTransform(pos,R); //does pushmatrix

	glScaled(radius, radius, radius);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_CLIP_PLANE0);

	int diskrot = 1;
	if(pitch<0){
		eqn[2]=1;
		glClipPlane(GL_CLIP_PLANE0, eqn);
		glRotatef(90,1,0,0);
	}
	else if (pitch>0) {
		eqn[2]=-1;
		glClipPlane(GL_CLIP_PLANE0, eqn);
		glRotatef(-90,1,0,0);
	}
	else if(roll<0) {
		eqn[1]=-1;
		glClipPlane(GL_CLIP_PLANE0, eqn);
		glRotatef(-180,1,0,0);
	}
	else if (roll>0) {
		eqn[1]=1;
		glClipPlane(GL_CLIP_PLANE0, eqn);
	}
	else if(yaw<0) {
		eqn[1]=1;
		glClipPlane(GL_CLIP_PLANE0, eqn);
		glRotatef(90,0,1,0);
	}
	else if (yaw>0) {
		eqn[1]=-1;
		glClipPlane(GL_CLIP_PLANE0, eqn);
		glRotatef(90,0,1,0);
		diskrot = -1;
	}


	glEnable(GL_NORMALIZE);
	drawSphere2();

	glDisable(GL_CLIP_PLANE0);

	if(flag == DRAW_JOINT_HOLD){
		glRotatef(90, diskrot, 0, 0);
		gluDisk(kukendisk, 0, 1, 24, 1);
	}
	glDisable(GL_NORMALIZE);

	glPopMatrix();
}

void dsDrawImage2d(int tex, float  x, float y){
	textures[tex]->draw2d(x, y, 1, 1);
}

void dsDrawScaledImage2d(int tex, float x, float y, float scale){
	textures[tex]->draw2d(x, y, scale, scale);
}

void dsDrawImage3d(int tex, float x, float y, float z){
	textures[tex]->draw3d(x, y, z, 1);
}

void dsDrawSquare(float x, float y, float z, float rot, float thickness, float side){

	float xx = side*0.5;
	float yy = side*0.5;

	glPushMatrix();
	setupDrawingMode();
	glColor4f (color[0],color[1],color[2], color[3]);
	glLineWidth (1.5);
  	glShadeModel (GL_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);

	glTranslatef(x, y, z);
  	glBegin (GL_LINES);
	// glColor3f (1,0,0);
  	glVertex3f (-xx, -yy, z);
  	glVertex3f (xx,-yy, z);

	// glColor3f (0,1,0);
  	glVertex3f (xx, -yy, z);
  	glVertex3f (xx,yy, z);

	// glColor3f (0,0,1);
  	glVertex3f (xx, yy, z);
  	glVertex3f (-xx,yy, z);

	// glColor3f (0,0,0);
  	glVertex3f (-xx, yy, z);
  	glVertex3f (-xx,-yy, z);
  	glEnd();
	glPopMatrix();
}

void dsDrawDisk(float x, float y, float size, float deg, int blend){
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);

	glTranslatef(x,y,0);

	setColor (color[0],color[1],color[2],color[3]);
	
	if(blend){
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	else {
		glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
	gluPartialDisk(kukendisk, 0, size, 32, 2, 0, deg);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
} 

void dsDrawSphere (const float pos[3], const float R[12], float radius, int flag)
{

	glShadeModel (GL_FLAT);
	if(flag==DRAW_SHADOW){
		setShadowDrawingMode();
		glDisable(GL_LIGHTING);
		glDepthRange (0,0.9999);
		drawSphereShadow (pos[0],pos[1],pos[2],radius);
		glDepthRange (0,1);
		glPopMatrix();
		glPopMatrix();
		glPopMatrix();
		glDepthRange (0,1);
	}
	else if(flag==DRAW_IMPACT){
		glPushMatrix();
		setupDrawingMode();
		glDisable(GL_LIGHTING);
		glColor4f(color[0],color[1], color[2], color[3]);
		glTranslatef(pos[0], pos[1], pos[2]);
		glScaled(radius, radius, radius);
		gluDisk(kukendisk, 0.8, 1, 32, 1);
		glPopMatrix();
	}
	else if(flag==DRAW_OUTLINE){
		setTransform(pos,R); //does pushmatrix
		setupOutlineDrawingMode();
		float r = radius+shading_offset*0.5f;
		glScaled(r,r,r);
		glEnable(GL_NORMALIZE);
		drawSphere();
		glDisable(GL_NORMALIZE);
		glPopMatrix();
	}
	else if (flag==DRAW_BLOOD){
		setupDrawingMode();
		glDisable(GL_LIGHTING);
		GLfloat matrix[16];
		glGetFloatv (GL_MODELVIEW_MATRIX, matrix);
		setTransform(pos,matrix); //does pushmatrix
		glColor4f(color[0], color[1], color[2], color[3]);
		//drawSphere();
		glScaled(radius, radius, radius);
		gluDisk(kukendisk, 0, 1, 8, 1);

		// didnt blend properly
		/*
		glEnable (GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable (GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, env_textures[0]->image);
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2d(1,1); glVertex3f(2,2,2); // Top Right
		glTexCoord2d(0,1); glVertex3f(-2,2,2); // Top Left
		glTexCoord2d(1,0); glVertex3f(2,-2,2); // Bottom Right
		glTexCoord2d(0,0); glVertex3f(-2,-2,2); // Bottom Left
		glEnd();
		*/

		glEnable(GL_LIGHTING);
		glPopMatrix();
	}
	else if(flag == DRAW_TEXTURE){
		use_textures=0;
		setupDrawingMode();
		setTransform (pos,R);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);
		glScaled(radius, radius, radius);
		
		//glShadeModel(GL_SMOOTH);
		//drawSphere();
		//glShadeModel(GL_FLAT);
		//use_textures=0;
		//hampa.. THIS IS TOO SLOW
		glShadeModel(GL_SMOOTH);
		//glDisable(GL_LIGHTING);
		
		
		
		glEnable (GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, head_textures[head_idx]->image);
		//glEnable (GL_TEXTURE_GEN_S);
		//glEnable (GL_TEXTURE_GEN_T);
		//glTexGeni (GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
		//glTexGeni (GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
		//glScalef(2,2,2); // katamari
		gluSphere(kukensphere, 1.0, 8, 8);
		//drawSphere();
		glDisable(GL_NORMALIZE);
		glPopMatrix();
		use_textures=0;
	}
	else {
		setupDrawingMode();
        	setTransform (pos,R);
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_LIGHTING);
                glEnable(GL_NORMALIZE);
                glColor3f(1,1,1);
                glScaled(radius, radius, radius);

                glShadeModel(GL_SMOOTH);
                		
				drawSphere();

		glDisable(GL_NORMALIZE);
		glPopMatrix();
		glShadeModel (GL_FLAT);
	}
}


void dsDrawTriangle (const float pos[3], const float R[12],
		const float *v0, const float *v1,
		const float *v2, int solid)
{
	setupDrawingMode();
	glShadeModel (GL_FLAT);
	setTransform (pos,R);
	drawTriangle (v0, v1, v2, solid);
	glPopMatrix();
}


void dsDrawCylinder (const float pos[3], const float R[12],
		float length, float radius)
{
	setupDrawingMode();
	glShadeModel (GL_SMOOTH);
	setTransform (pos,R);
	drawCylinder (length,radius,0);
	glPopMatrix();

	if (use_shadows) {
		setShadowDrawingMode();
		setShadowTransform();
		setTransform (pos,R);
		drawCylinder (length,radius,0);
		glPopMatrix();
		glPopMatrix();
		glDepthRange (0,1);
	}
}


void dsDrawCappedCylinder (const float pos[3], const float R[12],
		float length, float radius, int flag)
{

	if(flag==DRAW_SHADOW){
		setShadowDrawingMode();
		setShadowTransform();
		setTransform (pos,R);
		drawCappedCylinder (length,radius);
		glPopMatrix();
		glPopMatrix();
		glPopMatrix();
		glDepthRange (0,1);
	}
	else if(flag==DRAW_OUTLINE){
		setTransform (pos,R);
		setupOutlineDrawingMode();
		//glDisable(GL_DEPTH_TEST);
		//glDisable(GL_LIGHTING);
		//glColor4f(0,0,0, color[3]);
		drawCappedCylinder (length+shading_offset,radius+shading_offset/2.0f);
		glPopMatrix();
	}
	else {
		setupDrawingMode();
		setTransform (pos,R);
		glEnable(GL_DEPTH_TEST);
        	glEnable(GL_LIGHTING);
		glColor3f(1,1,1);
		// glScalef(2,1,1);
		drawCappedCylinder (length,radius);
		glPopMatrix();
	}
}


void dsDrawLine (const float pos1[3], const float pos2[3], float width)
{
	setupDrawingMode();

	glColor3f (color[0],color[1],color[2]);
	glDisable (GL_LIGHTING);
	glDisable (GL_TEXTURE);
	glLineWidth (width);
  	glShadeModel (GL_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);

  	glBegin (GL_LINES);
  	glVertex3f (pos1[0],pos1[1],pos1[2]);
  	glVertex3f (pos2[0],pos2[1],pos2[2]);
  	glEnd();

}

void dsDrawBoxD (const double pos[3], const double R[12], const double sides[3], int flag) {
	int i;
	float pos2[3],R2[12],fsides[3];
	for (i=0; i<3; i++) pos2[i]=(float)pos[i];
	for (i=0; i<12; i++) R2[i]=(float)R[i];
	for (i=0; i<3; i++) fsides[i]=(float)sides[i];
	dsDrawBox (pos2,R2,fsides,flag);
}

void dsDrawSphereHalfD (const double pos[3], const double R[12], float radius, 
		float yaw, float pitch, float roll, int flag){
	int i;
	float pos2[3],R2[12];
	for (i=0; i<3; i++) pos2[i]=(float)pos[i];
	for (i=0; i<12; i++) R2[i]=(float)R[i];
	dsDrawSphereHalf(pos2,R2,radius,yaw,pitch,roll, flag);
}

void dsDrawSphereD (const double pos[3], const double R[12], float radius, int flag)
{
	int i;
	float pos2[3],R2[12];
	for (i=0; i<3; i++) pos2[i]=(float)pos[i];
	for (i=0; i<12; i++) R2[i]=(float)R[i];
	dsDrawSphere (pos2,R2,radius,flag);
}


void dsDrawTriangleD (const double pos[3], const double R[12],
		const float *v0, const float *v1,
		const float *v2, int solid)
{
	int i;
	float pos2[3],R2[12];
	for (i=0; i<3; i++) pos2[i]=(float)pos[i];
	for (i=0; i<12; i++) R2[i]=(float)R[i];
	dsDrawTriangle (pos2,R2,v0,v1,v2, solid);
}


void dsDrawCylinderD (const double pos[3], const double R[12],
		float length, float radius)
{
	int i;
	float pos2[3],R2[12];
	for (i=0; i<3; i++) pos2[i]=(float)pos[i];
	for (i=0; i<12; i++) R2[i]=(float)R[i];
	dsDrawCylinder (pos2,R2,length,radius);
}


void dsDrawCappedCylinderD (const double pos[3], const double R[12],
		float length, float radius, int flags)
{
	int i;
	float pos2[3],R2[12];
	for (i=0; i<3; i++) pos2[i]=(float)pos[i];
	for (i=0; i<12; i++) R2[i]=(float)R[i];
	dsDrawCappedCylinder (pos2,R2,length,radius,flags);
}


void dsDrawLineD (const double _pos1[3], const double _pos2[3], float width)
{
	int i;
	float pos1[3],pos2[3];
	for (i=0; i<3; i++) pos1[i]=(float)_pos1[i];
	for (i=0; i<3; i++) pos2[i]=(float)_pos2[i];
	dsDrawLine (pos1,pos2,width);
}


void dsSetSphereQuality (int n)
{
	sphere_quality = n;
}


void dsSetCappedCylinderQuality (int n)
{
  capped_cylinder_quality = n;
}



