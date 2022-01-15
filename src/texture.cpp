#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>	
#define snprintf _snprintf
#include <GL/gl.h>
#else
#include <gl.h>
#endif

#include "font.h"
#include "texture.h"
#include <stdlib.h>
#include <string.h>

bool LoadTGA(GLuint *texID, char *filename, struct TextureImage *texture){    
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
	GLubyte		TGAcompare[12];	// Used To Compare TGA Header
	GLubyte		header[6];	// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;	// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;		// Temporary Variable
	GLuint		type=GL_RGBA;	// Set The Default GL Mode To RBGA (32 BPP)

	FILE *file = fopen(filename, "rb");	// Open The TGA File

	if(	file==NULL ||										// Does File Even Exist?
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	// Does The Header Match What We Want?
		fread(header,1,sizeof(header),file)!=sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)									// Did The File Even Exist? *Added Jim Strong*
			return false;									// Return False
		else
		{
			fclose(file);									// If Anything Failed, Close The File
			return false;									// Return False
		}
	}

	texture->width  = header[1] * 256 + header[0];	// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];	// Determine The TGA Height	(highbyte*256+lowbyte)
    
 	if(	texture->width	<=0	||		// Is The Width Less Than Or Equal To Zero
		texture->height	<=0	||		// Is The Height Less Than Or Equal To Zero
		(header[4]!=24 && header[4]!=32))	// Is The TGA 24 or 32 Bit?
	{
		fclose(file);				// If Anything Failed, Close The File
		return false;				// Return False
	}

	texture->bpp	= header[4];			// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel	= texture->bpp/8;		// Divide By 8 To Get The Bytes Per Pixel
	imageSize		= texture->width * texture->height*bytesPerPixel;	// Calculate The Memory Required For The TGA Data

	texture->imageData=(GLubyte *)malloc(imageSize);// Reserve Memory To Hold The TGA Data

	if(	texture->imageData==NULL ||		// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file)!=imageSize)	// Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)		// Was Image Data Loaded
			free(texture->imageData);	// If So, Release The Image Data

		fclose(file);			// Close The File
		return false;			// Return False
	}

	for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)	// Loop Through The Image Data
	{							// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=texture->imageData[i];			// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;		// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose (file);							// Close The File

	// Build A Texture From The Data
	glGenTextures(1, texID);				// Generate OpenGL texture IDs

	glBindTexture(GL_TEXTURE_2D, *texID);			// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Filtered
	
	if (texture->bpp==24)									// Was The TGA 24 Bits
	{
		type = GL_RGB;										// If So Set The 'type' To GL_RGB
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture->width, texture->height, 0, type, GL_UNSIGNED_BYTE, texture->imageData);

	return true;											// Texture Building Went Ok, Return True
}

bool Texture2::loadTexture(char *dir, char *inName){
	char buf[256];
	if(image){
		// we have this texture already.. return OK
		if(!strcmp(inName, name)){
			return true;
		}
		glDeleteTextures(1, &image);
		if(texture.imageData){
			free(texture.imageData);
			texture.imageData = NULL;
		}
		image = 0;
	}
	strncpy(name, inName, sizeof(name));
	snprintf(buf, sizeof(buf), "%s/%s.tga", dir, name);
	return LoadTGA(&image, buf, &texture);
}

Texture2::Texture2(char *dir, char *inName){
	image = 0;
	texture.width = 128;
	texture.height = 128;
	texcolor[0] = 1;
	texcolor[1] = 1;
	texcolor[2] = 1;
	texture.imageData = NULL;

	strcpy(name, "");
	if(loadTexture(dir, inName) == false){
		fprintf(stderr, "failed to load texture %s/%s\n", dir, inName);
		texcolor[1] = 0;
	}
}

Texture2::Texture2(char *inFilename){
	image = 0;
	texture.width = 128;
	texture.height = 128;
	texcolor[0] = 1;
	texcolor[1] = 1;
	texcolor[2] = 1;
	texture.imageData = NULL;

	strcpy(name, "");
	LoadTGA(&image, inFilename, &texture);

}

void Texture2::bind (int modulate)
{
	glBindTexture (GL_TEXTURE_2D,this->image);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
			modulate ? GL_MODULATE : GL_DECAL);
}

/*
void Texture2d::draw2e(int x, int y){
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);

	glPushMatrix();
	glLoadIdentity();

	glTranslatef(x,y,0);
	glListBase(1);

	glScalef(size,size,size);
	glRotatef(180,1,0,0);


	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
}
*/

void Texture2::draw3d(float x, float y, float z, float scale){
	glDisable (GL_LIGHTING);

	glShadeModel (GL_FLAT);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);

	glEnable (GL_TEXTURE_2D);
	glColor3f(1,1,1);

	glBindTexture(GL_TEXTURE_2D, image);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
       	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glPushMatrix();
	glScalef(scale, scale, 1);
	glTranslatef(x,y,z);
	glBegin (GL_QUADS);
	glNormal3f (0,0,1);
	glTexCoord2f(0.0, 0.0); glVertex3f(-2.0, -1.0, 0.0); 
   	glTexCoord2f(0.0, 1.0); glVertex3f(-2.0, 1.0, 0.0); 
   	glTexCoord2f(1.0, 1.0); glVertex3f(0.0, 1.0, 0.0); 
   	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, -1.0, 0.0); 
	glEnd();

	glPopMatrix();
	//glDisable(GL_TEXTURE_2D);
	//glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

}

void Texture2::draw2d(float x, float y, float scalex, float scaley){


	glPushMatrix();
	glLoadIdentity();
	glTranslatef(x,y,0);
	glScalef(scalex, scaley, 1);

	glShadeModel (GL_FLAT);
	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable (GL_TEXTURE_2D);

	glDisable (GL_LIGHTING);
	glDisable (GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glColor3f(texcolor[0],texcolor[1],texcolor[2]);

	glBindTexture(GL_TEXTURE_2D, image);
	glBegin(GL_QUADS);

	//Top-left vertex (corner)
	glTexCoord2f(0,0);
	//glVertex3f(100.0f, 100.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	
	//Bottom-left vertex (corner)
	glTexCoord2f(1,0);
	glVertex3f(texture.width, 0.0f, 0.0f);
	
	//Bottom-right vertex (corner)
	glTexCoord2f(1,1);
	//glVertex3f(100 + texture.width, 100 + texture.height, 0.0f);
	glVertex3f(texture.width, texture.height, 0.0f);
	
	//Top-right vertex (corner)
	glTexCoord2f(0,1);
	glVertex3f(0, texture.height, 0.0f);
	//glVertex3f(100, 100 + texture.height, 0.0f);
	glEnd();

	glPopMatrix();
}

void Texture2::draw(int x, int y){
	//Make certain everything is cleared from the screen before we draw to it
	glPushMatrix();

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Enable texturing
	glEnable(GL_TEXTURE_2D);
	//glRotatef(180,0,1,0);
	//glRotatef(180,0,0,1);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	//Load the texture
	//glBindTexture(GL_TEXTURE_2D, this->image);

	glBegin(GL_QUADS);
	//Top-left vertex (corner)
	glTexCoord2f(0,0);
	glVertex3f(100.0f, 100.0f, 0.0f);
	
	//Bottom-left vertex (corner)
	glTexCoord2f(1,0);
	glVertex3f(228.0f, 100.0f, 0.0f);
	
	//Bottom-right vertex (corner)
	glTexCoord2f(1,1);
	glVertex3f(228.0f, 228.0f, 0.0f);
	
	//Top-right vertex (corner)
	glTexCoord2f(0,1);
	glVertex3f(100.0f, 228.0f, 0.0f);
	glEnd();

	//Disable texturing
	//glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

Texture2::~Texture2(){
	glDeleteTextures(1, &image);

	if(texture.imageData){
		free(texture.imageData);
		texture.imageData = NULL;
	}	

}
