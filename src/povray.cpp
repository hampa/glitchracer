#include <stdio.h>
#include <assert.h>
#include "povray.h"

FILE *povfile = NULL;
int povclock = 0;

void export_scene(FILE *f){
}

void pov_close(){
	assert(povfile);

	fprintf(povfile, "#end\n");
	fclose(povfile);
	povfile=NULL;

}

void pov_print_frame(int frame){

	fprintf(povfile, "#case (%i)\n", frame);
	export_scene(povfile);
	fprintf(povfile, "#break\n\n");

}

int pov_open(char *filename){

	povclock = 0;
	if((povfile = fopen(filename, "w")) == NULL){
		fprintf(stderr, "Unable to open %s\n", filename);
		return 1;
	}
	fprintf(povfile, "#include \"glossy.inc\"\n");
	fprintf(povfile, "#switch (frame_number)\n");
	return 0;
}

void povshot(char *filename){
	FILE *f;	

	if((f = fopen(filename, "w")) == NULL){
		fprintf(stderr, "Unable to open %s\n", filename);
		perror(filename);
		return;
	}

	fprintf(f, "#include \"glossy.inc\"\n");

	export_scene(f);

	fclose(f);
}

