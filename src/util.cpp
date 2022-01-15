// from carmack
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
//#include <ode/ode.h>
#include "util.h"

char * va( char *format, ... ) {
        va_list         argptr;
        static char             string[2][32000];       // in case va is called by nested functions
        static int              index = 0;
        char    *buf;

        buf = string[index & 1];
        index++;

        va_start (argptr, format);
        vsprintf (buf, format,argptr);
        va_end (argptr);

        return buf;
}

char *strip(char *s){

        int i;

        i = strlen(s) - 1;

        while(s[i] == ' ' || s[i] == '\n' || s[i] == '\t' || s[i] == '\r'){
                s[i--] = '\0';
        }

        while(*s == ' ' || *s == '\t'){
                s++;
        }
        return s;
}

char *clean(char *s){

        int i;

        i = strlen(s) - 1;

        while(s[i] == ' ' || s[i] == '\n' || s[i] == '\t' || s[i] == '\r' || s[i] == '"'){
                s[i--] = '\0';
        }

        while(*s == ' ' || *s == '\t' || *s == '"'){
                s++;
        }
        return s;
}

int split_tab(char * tablist, char tokens[][32], int maxTokens){
        int numTokens = 0;
        int i = 0;

	/* empty */
	if(!strcmp(tablist, "") || !strcmp(tablist, "\n")){
		*tokens[0] = 0;
		return 0;
	}

        while (*tablist && numTokens < maxTokens){
                if (*tablist == '\t' || *tablist == '\n'){
                        i = tokens[numTokens++][i] = 0;
                }
                else {
                        tokens[numTokens][i++] = *tablist;
                }
                tablist++;
        }
	*tokens[numTokens] = 0;
        return numTokens+1;
}

void split_nick(const char *nick, int len, char *clan, char *name){

	char *start = strstr(nick , "[");
	char *end = strstr(nick , "]");

	strcpy(clan, "");
	strcpy(name, "");


	if(start && end && end[1] && end-start>1){
		strncpy (name, end + 1, len);
		strncpy(clan, start + 1, len);
		clan[end-start-1]='\0';
	}
	else {
		strcpy(clan, "");
		strncpy(name, nick, len);
	}
}

int parse_delim_int(char *s, int delim, int size, int arr[]){
        int ret = 0;
		
		memset(arr, 0, size * sizeof(int));
		// handle both \r\n and \n
        while (*s && *s != '\n' && *s != '\r' && ret < size){
			if(*s != delim){
				arr[ret++] = strtol(s, &s, 10);
			}
			else {
                s++;
			}
		}

        return ret;
}

int parse_int(char *s, int size, int arr[]){
        int ret = 0;
	// handle both \r\n and \n
        while (*s && *s != '\n' && *s != '\r' && ret < size){
                arr[ret++] = strtol(s, &s, 10);
        }
        return ret;
}

int parse_int2(char *s, int size, int arr[][2]){
        int ret = 0;
        int x = 0;
        while (*s && *s != '\r' && *s != '\n' && ret < size){
                arr[ret][x++] = strtol(s, &s, 10);
                if(x==2){
                        ret++;
                        x=0;
                }
        }
        return ret;
}

int parse_doubles(char *s, int size, double *arr){
        int ret = 0;

        while (*s && *s != '\r' && *s != '\n' && ret < size){
                arr[ret++] = strtod(s, &s);
        }
        return ret;
} 
int parse_doubles3(char *s, int size, double arr[][3]){
        int ret = 0;
        int x = 0;
        while (*s && *s != '\r' && *s != '\n' && ret < size){
                arr[ret][x++] = strtod(s, &s);
                if(x==3){
                        ret++;
                        x=0;
                }
        }
        return ret;
}


int parse_doubles4(char *s, int size, double arr[][4]){
        int ret = 0;
        int x = 0;
        while (*s && *s != '\r' && *s != '\n' && ret < size){
                arr[ret][x++] = strtod(s, &s);
                if(x==4){
                        ret++;
                        x=0;
                }
        }
        return ret;
}


static char *flag_to_bool(int var, int flag){
	if(var & flag){
		return "Yes";
	}
	else {
		return "No";
	}
}


int file_exists(const char * filename) {
    if (FILE * file = fopen(filename, "r")) {
        fclose(file);
        return 1;
    }
    return 0;
}

