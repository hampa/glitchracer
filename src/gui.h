#ifndef _GUI_H
#define _GUI_H

//#define WINDOW_WIDTH   1344	
//#define WINDOW_HEIGHT  840	
#define WINDOW_WIDTH   1280
#define WINDOW_HEIGHT 800	 
//#define WINDOW_WIDTH  	1680 
//#define WINDOW_HEIGHT 	1050 
//#define WINDOW_WIDTH   800	
//#define WINDOW_HEIGHT 	600	

void dsNetPort(int);
void dsClientPort(int);
void dsPlay(int chan, int sound, int override);
void dsPlayVolume(int channel, int sound, int override, float volume);
void dsPlayVolumeLoop(int channel, int sound, int override, float volume, int loop);
void dsChannelVolume(int channel, float volume);
void dsSetVolume (int volume);
int  dsGetVolume ();
void dsFadeIn(int chan, int ms);
void dsFadeOut(int chan, int ms);
void dsMixVolume(int chan, int volume);
void dsLoop(int chan, int sound);
void dsEnableSound();
void dsDisableSound();
void dsSetShadows(int a);
void dsMusic(int);
void dsStartMusic();
void dsStopMusic();
void musicDone();
void dsWorldCoords(float x, float y , double *, double *);
void dsScreenCoords(float x, float y, float z, double *xy);
void dsSetCaption(char *s);
void dsReloadGraphicsEngine();
void dsSetFov(float fov);
void dsToggleFullscreen();
void dsToggleMusic();
void dsEnableMotion();
void dsDisableMotion();
void dsSetMouseCursorPos(int x, int y);
float dsGetQuitTime();
void dsStop();
void dsListModes();

#endif
