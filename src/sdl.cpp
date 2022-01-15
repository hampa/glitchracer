#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
	#include <gl.h>
	#include <glu.h>
	#include <sys/time.h>
	#include <unistd.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#ifdef LINUX
	#include <sys/time.h>
#endif

#include "SDL.h"
#include "SDL_mixer.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <ode/config.h>
#include "drawstuff.h"
// #include "version.h"
#include "internal.h"
#include "font.h"
#include "gui.h"
#include "gr_sounds.h"
#include "cursor.h"
#include "povray.h"
#include "util.h"


//We use this to lock framerate - although not working very well
//#define TICKS_PER_FRAME 12 //60 FPS (16*60) -> more like 50 fps. 80 FPS on mac


// DENNA ANV"NDS IDAG
#define TICKS_PER_FRAME 16 //60 FPS (16*60) -> more like 50 fps

//#define TICKS_PER_FRAME 16 //30 (33*30)
//#define TICKS_PER_FRAME 32 //30 (33*30)
//#define TICKS_PER_FRAME 64  //15 FPS
//#define TICKS_PER_FRAME 136 //7.5 FPS


#define FRAME_AVG 100  //update every 100 frames
#define QUIT_SLEEP	3	//number of seconds to sleep before quit
// #ifndef DEBUG
#define FIXED_FRAMERATE 1
// #endif


extern int mainx(int c, char **v);

Uint32 tick_tock = SDL_GetTicks();
Uint32 old_tick;
Uint32 frame_tick;
Uint32 quit_time = 0;

int time_passed;
int mix_volume = MIX_MAX_VOLUME;
int extra_time = 0;
static int screen=0;
static int width=0,height=0;

static int run=1;// 1 if simulation running
static int xpause=0;// 1 if in `pause' mode
static int singlestep=0;// 1 if single step key pressed
static int writeframes=0;// 1 if frame files to be written
static int fullscreen=0;
static int use_sound=0;
static int mod_state=0; //if we shift or some modifyer pressed
static int save_mouse_pos=0;
static int mx=0,my=0;

SDL_Surface *screensurface=NULL;
SDL_Cursor *cursor = NULL;

GLuint buffer[1024]; 

Mix_Chunk *sounds[NUM_SOUNDS];
Mix_Chunk *phaser = NULL;
Mix_Music *music = NULL;

int port = 10001;
int client_port = port;

int phaserChannel = -1;
float xmove=0;
float ymove=0;
float zmove=0;
int do_motion=true;
float new_time;
float old_time;
int wasd_key = KEY_NONE;
char caption_message[128];

void InitGL(int Width, int Height);
int screenshot(char *filename, SDL_Surface *screen);
void povshot(char *filename);
void write_screenshots(); // handle F8
void hide_mouse_cursor();
void show_mouse_cursor();

/*********************
framerate calculations
**********************/

#if defined(OSX) || defined(LINUX)
static float gettime(void) {  
  static struct timeval told;
  struct timeval tnew;
  struct timezone tz;
  float ris;

  gettimeofday(&tnew,&tz);
  ris=(float)(tnew.tv_sec-told.tv_sec)+
    (float)(tnew.tv_usec-told.tv_usec)/1000000;
  told=tnew;
  return(ris);
}

#else
static float gettime(){
	return GetTickCount() * 0.001f;

}
#endif

void calculate_framerate(){
	static int old_tick=SDL_GetTicks();
	static int frameNum=1;
    static char strFrameRate[50] = {0};
	int this_tick = SDL_GetTicks();
	
	frameNum++;
	if ((frameNum % FRAME_AVG)==0) {
		sprintf(strFrameRate, "TORIBASH %s fps:%.0f",
		caption_message,
		(1000.0f*FRAME_AVG)/float(this_tick - old_tick));
		SDL_WM_SetCaption(strFrameRate,0);
		old_tick = this_tick;
	}
}

int get_motion_y(int y, int wh){
	return y;
}
int get_y(int y, int wh){
	return wh-y;
}

void dsNetPort(int p){
	port = p;
}

void dsClientPort(int p){
	client_port = p;
}

void musicDone() {
	if(music == NULL){
		return;
	}

  	Mix_HaltMusic();
  	Mix_FreeMusic(music);
  	music = NULL;
}

void dsStartMusic(){
	if(use_sound == 0 || music != NULL){
		return;
	}
	dsMusic(0);
}

void dsStopMusic(){
	musicDone();
}

void dsToggleMusic(){
	if(music == NULL){
		dsMusic(0);
	}
	else {
		musicDone();
	}
}

void dsSetCaption(char *s){
	strncpy(caption_message, s, sizeof(caption_message));
}

void hide_mouse_cursor(){
	if(SDL_WM_GrabInput(SDL_GRAB_ON) != SDL_GRAB_ON){
		fprintf(stderr, "Failed to set GrabInput to SDL_GRAB_ON\n");
	}
	SDL_ShowCursor(SDL_DISABLE);	
	save_mouse_pos=1;
}

void dsSetMouseCursorPos(int x, int y){
	SDL_WarpMouse (x, y);
}

/* show cursor at seleced joint or at last seen */
void show_mouse_cursor(){
	double xyz[3];
	if(SDL_WM_GrabInput(SDL_GRAB_OFF) != SDL_GRAB_OFF){
		fprintf(stderr, "Failed to unset GrabInput\n");
	}
	SDL_ShowCursor  (SDL_ENABLE);
}

void dsMusic(int something){
	if(!use_sound){
		return;
	}

	if(music == NULL) {
		music = Mix_LoadMUS("data/music/music.ogg");
		//music = Mix_LoadMUS("music/music.wav");

		if(music == NULL){
			fprintf(stderr, "Unable to load data/music/music.ogg %s\n",  Mix_GetError());
			return;
		}
		Mix_VolumeMusic(20);
		Mix_PlayMusic(music, -1);
		Mix_HookMusicFinished(musicDone);
	} else {
		Mix_HaltMusic();
		Mix_FreeMusic(music);
		music = NULL;
	}
}

void dsDisableMotion(){
	do_motion=false;	
}

void dsEnableMotion(){
	do_motion=true;
}

void dsScreenCoords(float x, float y, float z, double *xyz){
	GLint viewport[4];
   	GLdouble mvmatrix[16], projmatrix[16];
	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	gluProject ((GLdouble) x, (GLdouble) y, (GLdouble)z, mvmatrix, projmatrix, viewport, &xyz[0], &xyz[1], &xyz[2]);
}

void dsWorldCoords(float x, float y, double *start, double *end){
	GLint viewport[4];
   	GLdouble mvmatrix[16], projmatrix[16];
	int realy;

	glGetIntegerv (GL_VIEWPORT, viewport);
	glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	realy = viewport[3] - (GLint) y - 1;
	gluUnProject ((GLdouble) x, (GLdouble) realy, 0.0, mvmatrix, projmatrix, viewport, &start[0], &start[1], &start[2]);
	gluUnProject ((GLdouble) x, (GLdouble)realy, 1.0, mvmatrix, projmatrix, viewport, &end[0], &end[1], &end[2]);
}

void dsLoop (int channel, int sound){
	if(!use_sound){
		return;
	}

	Mix_PlayChannel(channel, sounds[sound], -1);
}

void dsMixVolume(int channel, int vol){
	Mix_Volume(channel, vol);
}

void dsFadeOut(int channel, int ms){
	Mix_FadeOutChannel(channel,ms);
}
void dsFadeIn(int channel, int ms){
	; //Mix_FadeInChannel(channel,ms);
}

void dsPlay (int channel, int sound, int override){
	dsPlayVolumeLoop(channel, sound, override, 1.0f, 0);
}

void dsPlayVolume(int channel, int sound, int override, float volume){
	dsPlayVolumeLoop(channel, sound, override, 1.0f, 0);
}

void dsChannelVolume(int channel, float volume){
	float vol = volume * mix_volume;
	Mix_Volume(channel, (int)vol);
}

void dsPlayVolumeLoop(int channel, int sound, int override, float volume, int loop){
	if(!use_sound){
		return;
	}

	if(loop){
		loop = -1;
	}

	float vol = volume * mix_volume;

	if(override){
		Mix_Volume(channel, (int)vol);
		Mix_PlayChannel(channel, sounds[sound], loop);
	}
	else {
		if(channel == -1 || Mix_Playing(channel)==0){
			Mix_Volume(channel, (int)vol);
			Mix_PlayChannel(channel, sounds[sound], loop);
		}
	}
}

void dsSetVolume (int volume){
	if(use_sound){
		Mix_Volume(-1, volume);
	}
	mix_volume = volume;
}

int dsGetVolume (){
	if(!use_sound){
		return 0;
	}
	return mix_volume;
	//return Mix_Volume(-1, -1);
}

void dsEnableSound(){
  use_sound=1;
}

void dsDisableSound(){
	use_sound=0;
}

static void printMessage (char *msg1, char *msg2, va_list ap)
{
	fflush (stderr);
	fflush (stdout);
	fprintf (stderr,"\n%s: ",msg1);
	vfprintf (stderr,msg2,ap);
	fprintf (stderr,"\n");
	fflush (stderr);
}

extern "C" void dsError (char *msg, ...)
{
	va_list ap;
	va_start (ap,msg);
	printMessage ("Error",msg,ap);
	exit (1);
}


extern "C" void dsDebug (char *msg, ...)
{
	va_list ap;
	va_start (ap,msg);
	printMessage ("INTERNAL ERROR",msg,ap);
	abort();
}


extern "C" void dsPrint (char *msg, ...)
{
	va_list ap;
	va_start (ap,msg);
	vprintf (msg,ap);
}

void dsListModes(){
 	SDL_Rect **modes;

	/* Get available fullscreen/hardware modes */
	modes=SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_OPENGL);

	/* Check is there are any modes available */
	if(modes == (SDL_Rect **)0){
  		printf("No modes available!\n");
		return;
	}

	/* Check if or resolution is restricted */
	if(modes == (SDL_Rect **)-1){
  		printf("All resolutions available.\n");
	}
	else{

  		/* Print valid modes */
  		printf("Available Modes\n");
  		for(int i=0;modes[i];++i){
    			printf("  %d x %d\n", modes[i]->w, modes[i]->h);
		}
	}
}

int main(int argc, char **argv){
	mainx(argc, argv);
	return 0;
}

static void resizeMainWindow(int _width, int _height, int fullscreen){

	int fsaa = 4;
	int value = 0;
	int flags=SDL_OPENGL|SDL_HWSURFACE;

	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, fsaa);
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 1);
	
	DBG("resizeMainWindow\n");

	if(fullscreen){
		flags=SDL_OPENGL|SDL_FULLSCREEN;
	}

	if ((screensurface = SDL_SetVideoMode(_width, _height, 0, flags)) == NULL ) {
		fprintf(stderr, "Unable to create OpenGL FSAA screen: %s\n", SDL_GetError());
		fprintf(stderr, "Trying without FSAA instead\n");
		
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
        	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0);
		if ((screensurface = SDL_SetVideoMode(_width, _height, 0, flags)) == NULL ) {
			fprintf (stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError ());
			SDL_Quit();
		}
	}	

	height = _height;
	width = _width;

}

static void createMainWindow (int _width, int _height){

	/* Same setup as before */
  	int audio_rate = 22050;
  	Uint16 audio_format = AUDIO_S16;
  	int audio_channels = 2;

	/* set this to 128 for Heizenbox effect */
  	int audio_buffers = 4096;

	if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0 ) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}


#ifdef USE_ACCUM
	SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, 1);       
	SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE,1);      
    SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, 1);
    SDL_GL_SetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, 1);
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 1);
#endif

	atexit(SDL_Quit);

	resizeMainWindow(_width,_height,fullscreen);

	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
    		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
			assert(0);
			dsDisableSound();
  	}


	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

	SDL_EnableUNICODE(1);

	cursor = init_system_cursor(arrow);
	if(cursor){
		SDL_SetCursor(cursor);
	}
	else {
		fprintf(stderr, "Failed to create custom cursor\n");
	}

#ifdef WITH_SOUNDS
	for(int i=0; i<NUM_SOUNDS; i++){
		// Pre load sound effects
		int id = sound_effects[i].id;
		char buf[256];
		snprintf(buf, sizeof(buf), "data/sounds/%s", sound_effects[id].name);
  		sounds[i] = Mix_LoadWAV(buf);
		if(!sounds[i]) {
			fprintf(stderr, "Mix_LoadWAV: %s %s\n", buf, Mix_GetError());
			assert(0);
		}
	}
#endif
	// clear captin and set a default message
	SDL_WM_SetCaption("TORIBASH", NULL);
}

void dsToggleFullscreen(){
	fullscreen^=1;
	dsReloadGraphicsEngine();

}

void dsReloadGraphicsEngine(){
	resizeMainWindow(WINDOW_WIDTH, WINDOW_HEIGHT,fullscreen);
	InitGL(WINDOW_WIDTH, WINDOW_HEIGHT);
	dsStopGraphics();
	dsStartGraphics(width,height);
	//refresh_heads();
}

static void destroyMainWindow(){
	if(cursor){
		SDL_FreeCursor(cursor);
	}
	cursor = NULL;
	 
	Mix_CloseAudio();
	SDL_Quit();
}

// shift x left by i, where i can be positive or negative
#define SHIFTL(x,i) (((i) >= 0) ? ((x) << (i)) : ((x) >> (-i)))

static void captureFrame(int num){
	char s[128];
	sprintf(s,"frame/frame%04d.pov",num);
	//export_scene_to_pov(s);
	printf(s);
}

void savePovFrame(int num){
	pov_print_frame(num);
}

void dsSetFov(float fov){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void InitGL(int Width, int Height)	        // We call this right after our OpenGL window is created.
{
	height = Height;
	width = Width;
	glViewport(0, 0, Width, Height);
	glClearColor(1.0, 1.0, 1.0, 0.0);
   	glClearAccum(1.0, 1.0, 1.0, 0.0);
	glClearDepth(1.0);                            // Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);                         // The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);                      // Enables Depth Testing

	dsSetFov(50.0f);

	glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


	//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

}




void dsPlatformSimLoop (int window_width, int window_height, dsFunctions *fn, int initial_pause)
{
	xpause = initial_pause;

	createMainWindow (window_width, window_height);
	InitGL(window_width, window_height);
	dsStartGraphics (window_width,window_height);

	if (fn->start){
		fn->start();
	}

	// init tb_game
	//init_game();

	int frame = 0;
	tick_tock = SDL_GetTicks();
	frame_tick = 0;
	extra_time = 0;
	while (run) { 
		

#ifdef FIXED_FRAMERATE
		//try to keep atleast 60 fps
		old_tick = tick_tock;

		//whats the new time
		tick_tock = SDL_GetTicks();

		//check time passed
		time_passed = tick_tock - old_tick;

		if(time_passed < TICKS_PER_FRAME - extra_time){
			SDL_Delay(TICKS_PER_FRAME - time_passed);
		}
		tick_tock = SDL_GetTicks();

		if(quit_time && tick_tock > quit_time){
			run=0;
		}
#endif
		SDL_Event event;

		while ( SDL_PollEvent(&event) ) {


			if (event.type == SDL_QUIT) {
				dsStop();
			}
			else if (event.type == SDL_KEYDOWN)
			{
				
				/*****************************
				Moving camera
				*****************************/

				switch(event.key.keysym.sym){
				case SDLK_LALT:
				case SDLK_RALT:
				case SDLK_LMETA:
				case SDLK_RMETA:
					DBG("set mod state %i\n", event.key.keysym.sym);
					hide_mouse_cursor();
					mod_state=1;
					break;
				/*
				case SDLK_UP: // zoom in on player
				case SDLK_w:
					if(event.key.keysym.mod & KMOD_SHIFT){
						BITS_ONOFF(wasd_key, KEY_UP, (KEY_ZOOM|KEY_DOWN));
					}
					else {
						BITS_ONOFF(wasd_key, KEY_ZOOM_IN, KEY_ZOOM_OUT);
					}
					break;
				case SDLK_DOWN: // zoom out from player
				case SDLK_s:
					if(event.key.keysym.mod & KMOD_SHIFT){
						BITS_ONOFF(wasd_key, KEY_DOWN, (KEY_ZOOM|KEY_UP));
					}
					else {
						BITS_ONOFF(wasd_key, KEY_ZOOM_OUT, (KEY_ZOOM_IN|KEY_DOWN));
					}
					break;
				case SDLK_LEFT:
				case SDLK_a:
					BITS_ONOFF(wasd_key, KEY_LEFT, KEY_RIGHT);
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					BITS_ONOFF(wasd_key, KEY_RIGHT, KEY_LEFT);
					break;
				*/
				default: 
					break;
				}
				/****************************
				Writing frames as screenshots
				*****************************/
				if(event.key.keysym.sym == SDLK_F7 && (event.key.keysym.mod & KMOD_CTRL)){
					writeframes ^= 1;
					if (writeframes) {
						frame=0;
						if(pov_open("raytrace/animation.pov")){
							printf("Unable to open raytrace/animation.pov");
							writeframes=0;		
						}
						else {
							printf("Saving frames to raytrace/animation.pov. Hit ctrl-F7 to stop.");
						}
					}
					else {
						printf(va("Wrote %i animation frames", frame));
						pov_close();
					}
				}
				fn->command (event.key.keysym.sym, event.key.keysym.unicode, event.key.keysym.mod, 1);

			}
			else if (event.type == SDL_KEYUP ){
				if (fn->command) {
					/***********************
					turn of camera movement
					************************/
					switch(event.key.keysym.sym){
						case SDLK_LALT:
						case SDLK_RALT:
						case SDLK_LMETA:
						case SDLK_RMETA:
							DBG("set mod state %i mod_state %i\n", event.key.keysym.sym, mod_state);
							show_mouse_cursor();
							mod_state=0; 
							break;
						case SDLK_UP:
						case SDLK_w:
							BITS_OFF(wasd_key, (KEY_ZOOM_IN|KEY_UP));
							break;
						case SDLK_DOWN: 
						case SDLK_s:
							BITS_OFF(wasd_key, (KEY_ZOOM_OUT|KEY_DOWN));
							break;
						case SDLK_LEFT: 
						case SDLK_a:
							BITS_OFF(wasd_key, KEY_LEFT);
							break;
						case SDLK_RIGHT: 
						case SDLK_d:	
							BITS_OFF(wasd_key, KEY_RIGHT);
							break;
						case SDLK_F8: 
							write_screenshots();	
							break;
						default: 
							break;
					}


					/***********************
					toggle fullscreen ctrl-enter
					***********************/
					if((event.key.keysym.mod == 64 && event.key.keysym.sym == 13) || 
						event.key.keysym.mod == 4160){
						if( event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER){
							dsToggleFullscreen();
						}
					}
					//send keys to toribash.cpp
					fn->command (	event.key.keysym.sym,
							event.key.keysym.unicode,
							event.key.keysym.mod,
							0);
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN){
				int x = event.button.x;
				int button_y = get_y(event.button.y, window_height);
				int motion_y = get_motion_y(event.button.y, window_height);
				
				switch(event.button.button){
					case SDL_BUTTON_LEFT:
						double start[3];
						double end[3];
						dsWorldCoords((float)event.button.x, (float)motion_y, start, end);

						if(fn->pick){
							int state = SDL_GetModState();
							fn->pick(state & KMOD_SHIFT, 1, start,end, x, button_y);
						}
						break;
					case SDL_BUTTON_RIGHT:
					// case SDL_BUTTON_WHEELUP:
						mod_state = 1;
						hide_mouse_cursor();
						break;
					default: 
						break;
				}

				if(fn->mouse){
					fn->mouse(event.type, event.button.button, x, button_y);
				}
			}
			else if (event.type == SDL_MOUSEBUTTONUP){
				int y = get_y(event.button.y, window_height);
				if(fn->mouse){
					fn->mouse(event.type, event.button.button, event.button.x, y);
				}
				DBG("event.button.button %i mod_state %i\n", event.button.button, mod_state);	
				if(mod_state && (event.button.button == SDL_BUTTON_RIGHT || event.button.button == SDL_BUTTON_WHEELUP)){
					show_mouse_cursor();
					mod_state=0; 
				}
			}
			else if (event.type == SDL_MOUSEMOTION){
				//TODO.. what here? move camera?
				if (mod_state == 1){
			
					// let the mouse movement start before we start moving
					// we also save x and y pos incase no joint is selected later on
					if(save_mouse_pos){
						mx = event.motion.x;
						my = event.motion.y;
						save_mouse_pos=0;
					}
					else  { 
						dsMotion(1, event.motion.xrel, event.motion.yrel);
					}
				}
				else
				/**************************
				Mouse over
				**************************/
				if(event.button.button == 0){
					double start[3];
					double end[3];
					int motion_y = get_motion_y(event.motion.y, window_height);
					int click_y = get_y(event.motion.y, window_height);
					
					dsWorldCoords((float)event.motion.x, (float)motion_y, start, end);

					if(fn->pick){	
						fn->pick(0, 0, start, end, event.motion.x, click_y);	
					}
				}
			}
			/* Minimize or maximize. We need to refresh GL on SDL 1.2 for this */
			else if(event.active.type == SDL_ACTIVEEVENT && event.active.state & SDL_APPACTIVE){
				// restore openGL screen
				if(event.active.gain) {
					dsReloadGraphicsEngine();
				}
			}
		} //after EVENT while loop

		if(do_motion && wasd_key){
			dsWASDMotion(wasd_key);
			//dsWASDMotionFPS(wasd_key);
		}
		/***************
		Draw graphics.. run simulation.. etc
		***************/
		dsDrawFrame (width,height,fn,xpause && !singlestep);
		singlestep = 0;


		SDL_GL_SwapBuffers();

		if (xpause==0 && writeframes) {
			//captureFrame (frame);
			savePovFrame(++frame); //1 is the first frame
		}
		//calculate this last
		calculate_framerate();
	} //after run

	// save configuration
	//destroy_game();


	//The End
	if (fn->stop) fn->stop();
	dsStopGraphics();
	destroyMainWindow();
}

void dsStop(){
	run = 0;
}

float dsGetQuitTime(){	
	if(quit_time == 0){
		return 0;
	}
	Uint32 now = SDL_GetTicks();	
	if(now >= quit_time){
		return 0;
	}
	float sleep_time = (quit_time - now) * 0.001f;
	return sleep_time;
}

//Something was buggy with main and windows here
/*
extern int mainx(int argc, char **argv);

int main(int c, char **v){
	mainx(c,v);
	return 0;
}
*/

void write_screenshots(){
	struct tm *timeinfo;
	time_t rawtime;
	char timestamp[256];

    time(&rawtime);
	timeinfo = localtime (&rawtime);
	snprintf(timestamp, sizeof(timestamp), "%02i-%02i-%02i-%02i", timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	screenshot(va("screenshot-%s.bmp", timestamp) , screensurface);
	povshot(va("raytrace/screenshot-%s.pov", timestamp));
	printf(va("Wrote screenshot-%s.bmp raytrace/screenshot-%s.pov", timestamp, timestamp));
}




int screenshot(char *filename, SDL_Surface *screen) {
	SDL_Surface *temp;
	unsigned char *pixels;
	int i;

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	0x000000FF, 0x0000FF00, 0x00FF0000, 0
#else
	0x00FF0000, 0x0000FF00, 0x000000FF, 0
#endif
	);
	if (temp == NULL)
		return -1;

	pixels = (unsigned char *)malloc(3 * screen->w * screen->h);
	if (pixels == NULL)
	{
		SDL_FreeSurface(temp);
		return -1;
	}

	glReadPixels(0, 0, screen->w, screen->h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	for (i=0; i<screen->h; i++)
		memcpy(((char *) temp->pixels) + temp->pitch * i, pixels + 3*screen->w * (screen->h-i-1), screen->w*3);
	free(pixels);

	SDL_SaveBMP(temp, filename);
	SDL_FreeSurface(temp);
	return 0;
}




static SDL_Cursor *init_system_cursor(const char *image[])
{
  int i, row, col;
  Uint8 data[4*32];
  Uint8 mask[4*32];
  int hot_x, hot_y;

  i = -1;
  for ( row=0; row<32; ++row ) {
    for ( col=0; col<32; ++col ) {
      if ( col % 8 ) {
        data[i] <<= 1;
        mask[i] <<= 1;
      } else {
        ++i;
        data[i] = mask[i] = 0;
      }
      switch (image[4+row][col]) {
        case 'X':
          data[i] |= 0x01;
          mask[i] |= 0x01;
          break;
        case '.':
          mask[i] |= 0x01;
          break;
        case ' ':
          break;
      }
    }
  }
  sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
  return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}
