#ifndef _POVRAY_H
#define _POVRAY_H

#include <stdio.h>

int pov_open(char *filename);
void pov_close();
void export_scene(FILE *f);
void pov_print_frame(int clock);
void povshot(char *filename);

#endif

