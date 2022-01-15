#include <stdio.h>

#ifndef __FONT_H
#define __FONT_H

#ifdef WIN32
#include <GL/gl.h>
#else
#include <gl.h>
#include <glu.h>
#endif
#include "texture.h"

enum {
	FONT_BIG,
	FONT_SMALL,
	FONT_MEDIUM,
	NUM_FONTS
};

#define FONT_SIZE_HUGE		2
//#define FONT_SIZE_BIG		0.9
//#define FONT_SIZE_NORMAL	0.4
//#define FONT_SIZE_SMALL		0.3
#define FONT_SIZE_BIG		1.0
#define FONT_SIZE_NORMAL	1.0
#define FONT_SIZE_SMALL		1.0

enum {
	FONT_TOP_Y			= 0,
	FONT_TOP2_Y			= 50,
	FONT_TOP3_Y			= 60,
	FONT_TOP4_Y			= 70,
	FONT_TOP5_Y			= 80,	// rank and stuff
	FONT_TOP6_Y			= 100,	// rank and stuff
	FONT_DISK_Y 			= 30,
	FONT_SPEC_Y			= 120,
	FONT_BOTTOM_Y			= 80,
	FONT_HEADER			= 30, 
	FONT_FOOTER			= 30,
	FONT_LIST_START			= 100,
	FONT_BROWSER_HEADER 		= 110,
	FONT_BROWSER_START		= 120,
	FONT_BROWSER_INC		= 23,
	FONT_LIST_INC			= 30,
	FONT_BITMAP_LIST_INC		= 20,
	FONT_MENU_START			= 130,
	FONT_LIST_X				= 150,
	FONT_INPUT_X			= 20,
	FONT_INPUT_Y			= 130,
	FONT_BROWSER_X1			= 20,  // no 
	FONT_BROWSER_X2			= 60,  // ip
	//FONT_BROWSER_X3			= 290, // players
	FONT_BROWSER_X4			= 280, // players data
	FONT_BROWSER_X5			= 410, // clock 
	FONT_BROWSER_X6			= 505, // desc
	FONT_BROWSER_PAGE		= 480, // 10 - 20 page thingy
	FONT_BROWSER_DETAILS_Y		= 380,
	FONT_BROWSER_BUDDIES_HEADER_Y 	= 140,
	FONT_BROWSER_BUDDIES_Y		= 110,
	FONT_INPUT_INC			= 30,
	FONT_ABOUT_Y			= 130,
	FONT_ABOUT_INC			= 50,
	FONT_CHAT_Y				= 90,
	FONT_CHAT_X				= 10,
	FONT_CHAT_INC			= 20,
	FONT_CENTER_PRINT_Y		= 360,
	FONT_CENTER_PRINT_INC	= 50
};

struct font_t {
	int id;
	char *file;
	int fullascii;
	int size; 	// number of glyphs.. should be 256
	int side; 	// size of each glyph in the texture (512/16, 1024/16 etc) 
	int shrink; 	// contract fonts a bit 
	GLuint texid;
	GLuint displaylist;
	struct TextureImage texture;
}; 
// 384, 368
static struct font_t fonts[]={
	{ FONT_BIG, "data/textures/font_big.tga", 	0, 256,  	64, 	25, 	0, 0, NULL },
	//{ FONT_SMALL, "data/textures/font_ascii.tga", 	1, 256, 	32, 20, 0, 0},
	{ FONT_SMALL, "data/textures/font_ascii.tga", 	1, 256, 	16, 	7, 	0, 0, NULL },
	//{ FONT_SMALL, "data/textures/font_ascii.tga", 	1, 256, 	32, 0, 0, 0},
	{ FONT_MEDIUM, "data/textures/font_medium.tga", 0, 256, 	32, 	15, 	0, 0, NULL },
	{ NUM_FONTS, NULL, 				0, 0, 		0, 	0, 	0, 0, NULL }
};


int InitFontGL(int width, int height);
void KillFont();
void BuildFont(int id);
void glPrint(int fontid, float x, float y, float size, int set, char *text);
void dsPrint2D(int id, float x, float y, float size, char *message);
void dsCenterPrint(int id, float y, float size, char *message);
void dsRightPrint(int id, float x, float y, float size, char *message);
void dsRightPrintBitmap(float x, float y, char *string);
void dsPrintBitmap(float x, float y, char *string);
void showMessage(int id, GLfloat x, GLfloat y, char *message, GLfloat size);
void setFontColor(float r, float g, float b, float alpha);
float getCenterX(int fontid, char *message); 
void dsCenterPrintBitmap(float y, char *message);
float get_font_scale(int id);
void Exit2d();
void Go2d();

#endif
