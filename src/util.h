#ifndef _UTIL_H
#define _UTIL_H

#define BITS_ON(x,b) ((x)|=(b))
#define BITS_OFF(x,b) ((x)&= ~(b))
#define BITS_ONOFF(x,by,bn) {BITS_OFF(x,bn);BITS_ON(x,by);} 
#define BITS_TOGGLE(x,b) ((x)^= (b))
#define BITS(x,m) ((x)&(m))
#define BITS_TO_BOOL(x,m) ((x)&(m)?"Yes":"No")
#define BITS_TO_BOOLINV(x,m) ((x)&(m)?"No":"Yes")
#if defined (WIN32) && defined(DEBUG)
        static void DBG (char *msg,...){
                va_list ap;
                va_start (ap,msg);
                vprintf (msg,ap);
                //fflush(stdout);
        }
#else
        static void DBG(char *msg, ...){ ; }
#endif
#if !defined(WIN32) && defined(DEBUG)
        #define DBG(fmt...) fprintf(stdout,fmt)
#endif

char * va( char *format, ... ) ;
char *strip(char *s);
char *clean(char *s);
void split_nick(const char *nick, int len, char *clan, char *name);
int split_tab(char * tablist, char tokens[][32], int maxTokens);
int parse_delim_int(char *s, int delim, int size, int arr[]);
int parse_int(char *s, int size, int arr[]);
int parse_int2(char *s, int size, int arr[][2]);
int parse_doubles(char *s, int size, double *arr);
int parse_doubles3(char *s, int size, double arr[][3]);
int parse_doubles4(char *s, int size, double arr[][4]);
static char *flag_to_bool(int var, int flag);
int file_exists(const char * filename); //return 1 if file exists
#endif

