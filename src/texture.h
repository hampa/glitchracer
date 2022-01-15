#ifndef _TEXTURE_H
#define _TEXTURE_H

#ifdef WIN32
#include <GL/gl.h>
#else
#include <gl.h>
#endif

struct	TextureImage {
	GLubyte	*imageData;	// Image Data (Up To 32 Bits)
	GLuint	bpp;		// Image Color Depth In Bits Per Pixel.
	GLuint	width;		// Image Width
	GLuint	height;		// Image Height
	GLuint	texID;		// Texture ID Used To Select A Texture
};			// Structure Name


bool LoadTGA(GLuint *texID, char *filename, struct TextureImage *texture);

class Texture2 {
public:
	Texture2(char *filename);
	Texture2(char *dir, char *inName);
	~Texture2();

	void draw(int x, int y);
	void draw2d(float x, float y, float scalex, float scaley);
	void draw3d(float x, float y, float z, float scale);
	void bind(int modulate);
	bool loadTexture(char *dir, char *inName);	

	char name[256];
	GLuint image;		//This is our texture
	struct TextureImage texture;
	float texcolor[3];
};

#endif

