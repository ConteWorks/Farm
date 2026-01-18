#if !defined (MYUTIL_H)
#define MYUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>



//funzione che verifica se il parametro passato è un numero (positivo)
static inline long isNumber(const char* s) {
   char* e = NULL;
   long val = strtol(s, &e, 10);
   if (e != NULL && *e == (char)0)
   	if (val > 0)
   		return val; 
   return -1;
}

//funzione che verifica se il parametro passato è il path di una cartella
static inline int isDirectory(char* nomedir){
	struct stat info;
	if ( stat(nomedir,&info)== -1)
		return 0; 
	if (S_ISDIR(info.st_mode))
		return 1;
	return 0;
}

//funzione che effettua la sleep in millisecondi
static inline void Sleep(long milliseconds)
{
    // usa nanosleep()
    struct timespec ts;
    ts.tv_sec  = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

//concatena le stringhe "s1 separator s2" e salva la stringa risultante in destination
static inline void myconcat(char *destination, char *s1, char *separator, char *s2){
	//destination = (char*)malloc(DIMFILE* sizeof(char));
	strcpy(destination, s1);
	strcat(strcat(destination, separator),s2);
}

#endif /* MYUTIL_H */
