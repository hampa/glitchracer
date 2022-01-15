#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>	
#endif

#ifdef LINUX
	#include <GL/freeglut.h>
#else
	#include <GLUT/glut.h>
#endif

#include "font.h"


GLuint font_base[NUM_FONTS];
int window_width = 800;  
int window_height = 600;  

void Go2d(){
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, window_width, 0, window_height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
}

void Exit2d(){	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

int InitFontGL(int width, int height){

	window_width = width;
	window_height = height;

	//struct TextureImage texture;
	for(int i=0;i<NUM_FONTS; i++){
		assert(fonts[i].id == i);
	
		if (!LoadTGA(&fonts[i].texid, fonts[i].file, &fonts[i].texture)) {
			assert(0);
			fprintf(stderr, "Unable to load font %s\n", fonts[i].file);
			return false;
		}
		BuildFont(i);
	}
	return true;
}

void KillFont(){
	for(int i=0;i<NUM_FONTS; i++){
		glDeleteLists(fonts[i].displaylist,fonts[i].size);
		glDeleteTextures(1, &fonts[i].texid);
		assert(fonts[i].texture.imageData);
		free(fonts[i].texture.imageData);
		fonts[i].texture.imageData = NULL;
	}
}

void BuildFont(int id) {
	assert(id <= NUM_FONTS);
	int side = fonts[id].side;
	float glyphsize = 1.0f/(float)16;

	fonts[id].displaylist = glGenLists(fonts[id].size);
	glBindTexture(GL_TEXTURE_2D, fonts[id].texid);
		
	for (int loop1=0; loop1<fonts[id].size; loop1++)
	{
		float cx=float(loop1%16)/16.0f;
		float cy=float(loop1/16)/16.0f;

		glNewList(fonts[id].displaylist+loop1,GL_COMPILE);
		int err = glGetError();
		if(err != GL_NO_ERROR){
			fprintf(stderr, "BuildFont error %i\n", err);
		}
		assert(err == GL_NO_ERROR);
			glBegin(GL_QUADS);	
				glTexCoord2f(cx,1.0f-cy-glyphsize);
				glVertex2d(0, side);

				glTexCoord2f(cx+glyphsize,1.0f-cy-glyphsize);	// Texture Coord (Bottom Right)
				glVertex2i(side, side);// Vertex Coord (Bottom Right)

				glTexCoord2f(cx+glyphsize,1.0f-cy-0.001f);	// Texture Coord (Top Right)
				glVertex2i(side , 0);		// Vertex Coord (Top Right)

				glTexCoord2f(cx, 1.0f-cy-0.001f);	// Texture Coord (Top Left)
				glVertex2i(0,0);			// Vertex Coord (Top Left)
			glEnd();					// Done Building Our Quad (Character)
			glTranslated(side-fonts[id].shrink, 0, 0); 			// Move To The Right Of The Character
		glEndList();						// Done Building The Display List
	}								// Loop Until All 256 Are Built
}

void glPrint(int id, float x, float y, float size, int set, char *text){
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fonts[id].texid);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);

	glPushMatrix();
	glLoadIdentity();

	glTranslatef(x,y,0);
	if(fonts[id].fullascii){
		glListBase(fonts[id].displaylist);
	}
	else {
		//glListBase(fonts[id].displaylist - 32 + 128);
		glListBase(fonts[id].displaylist - 32);
	}

	glScalef(size,size,size);
	glRotatef(180,1,0,0);

	glCallLists(strlen(text),GL_UNSIGNED_BYTE, text);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
}

float get_font_scale(int id){
	return fonts[id].side - (float)fonts[id].shrink;	
}

void dsPrint2D(int id, float x, float y, float size, char *message){
	showMessage(id, x, y, message, size);
}

void dsRightPrint(int id, float xoffset, float y, float size, char *message){
	float font_scale = get_font_scale(id);	
	float width = strlen(message) * size * font_scale;
	float x = window_width-xoffset-width;
	x -= font_scale * size * 0.4f;
	showMessage(id, x, y, message, size);
}

float getCenterX(int id, char *message){
	float font_scale = get_font_scale(id);
	float width = strlen(message) * font_scale;
	float x = (window_width-width)*0.5f;
	x -= font_scale * 0.4f;
	return x;
}

void dsCenterPrint(int id, float y, float size, char *message){
	float x = getCenterX(id, message);
	showMessage(id, x, y, message, size);
}

/*
void dsCenterPrintBitmap(float y, char *message){
	float len = (float)glutBitmapLength(GLUT_BITMAP_8_BY_13, (unsigned char *)message);
	dsPrintBitmap((window_width*0.5f) - (len*0.5f), y, message );
}
*/

void showMessage(int id, GLfloat x, GLfloat y, char *message, GLfloat size){
	glPrint(id, x, y, size, 0, message);
}

void setFontColor(float r, float g, float b, float alpha){
	glColor4f(r, g, b, alpha);
}

void dsRightPrintBitmap(float x, float y, char *string){
	float len = (float)glutBitmapLength(GLUT_BITMAP_8_BY_13, (unsigned char *)string);
	//dsPrintBitmap(window_width - x - len, y, string);
	showMessage(FONT_MEDIUM, window_width - x - len, y, string, 1.0f);
}

void dsCenterPrintBitmap(float y, char *message){
	float len = (float)glutBitmapLength(GLUT_BITMAP_8_BY_13, (unsigned char *)message);
	showMessage(FONT_MEDIUM, (window_width*0.5f) - (len*0.5f), y, message, 1.0f );
}

void dsPrintBitmap(float x, float y, char *string){
	showMessage(FONT_MEDIUM, x, y, string, 1.0f);
}
/*
void dsPrintBitmap(float x, float y, char *string){
	int len, i;

	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);

	glRasterPos2f(x, y);
	len = (int) strlen(string);
	for(i=0 ; i< len; i++){
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, string[i]);
	}

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	
	glPopMatrix();

}
*/
