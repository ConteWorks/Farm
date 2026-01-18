#define _POSIX_C_SOURCE  200112L
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <pthread.h>
#include <stdatomic.h>

#include "boundedqueue.h"
#include "util.h"
#include "myutil.h"

//#define DEBUG

// Sleep usa la nanosleep
static void *timer(void *arg) {
	long *t = (long*)arg;
	Sleep(*t);
	return NULL;
}

void mySleep(long t, atomic_int *termina){
   	pthread_t slptime;
	if (pthread_create(&slptime, NULL, timer, &t) != 0) {
		fprintf(stderr, "errore nella creazione del signal handler thread\n");
		abort();
	}
	//while (*termina == 0);
	//pthread_cancel(slptime); 
	pthread_join(slptime, NULL);
}


int producer(long q, long t, char *d, BQueue_t *coda, char **f, int n_args, atomic_int *termina){
	//apro la cartella d
	if (d!=NULL){
		DIR *dp;
		struct dirent *dir_p;
		dp = opendir(d);
		if ( dp == NULL ){
			perror("dp null");
			return 0;
		}
		while( ( dir_p = readdir(dp) ) != NULL && *termina == 0){
			char * file = (char*)malloc(DIMFILE* sizeof(char));
			if (strcmp(d, ".") != 0){
				myconcat(file, d, "/\0",dir_p -> d_name);
			}
			else{
				strcpy(file, dir_p ->d_name);
			}
			if (file == NULL) {
				perror("Producer malloc");
				return 0;
			}
			if (!isDirectory(file)){
				#ifdef DEBUG
					printf("Producer: nome file: %s\n", file);
				#endif
				//tra tutti i file presenti nella cartella seleziono solo quelli che hanno ".dat" nel nome
				if(strstr(dir_p -> d_name, ".dat")){
					push(coda, file);
					#ifdef DEBUG
						printf("Producer: file pushed %s\n", file);
					#endif
					//Sleep(t);
				}
				else
					//libero la memoria dei file che non hanno ".dat" nel nome
					free(file);
			}
			else 
				//chiamata ricorsiva della funzione, per entrare nella cartella
				if(!(strcmp(dir_p -> d_name, ".")==0 || strcmp(dir_p -> d_name, "..")==0)){
					#ifdef DEBUG
						printf("Producer: nome cartella: %s\n", file);
					#endif
					producer(q, t, file, coda, f, 0, termina);
					//libero la memoria delle stringhe contenente il nome delle cartelle
					free(file);
				}
				else
					//libero la memoria delle stringhe col nome delle cartelle . e ..
					free(file);
			mySleep(t, termina);
			//Sleep(t);
		}
		closedir(dp);
	}
	//aggiungo i file che sono parametri del main
	if (n_args>0){
		DIR *dp;
		struct dirent *dir_p;
		char *cur = ".";
		dp = opendir(cur);
		if ( dp == NULL ){
			perror("dp null");
			return 0;
		}
		while( ( dir_p = readdir(dp) ) != NULL && *termina == 0){
			char * file = (char*)malloc(DIMFILE* sizeof(char));
			strcpy(file, dir_p ->d_name);
			#ifdef DEBUG
				printf("Producer: si sta valutando il nome del file: %s\n", file);
			#endif
			if (file == NULL) {
				perror("Producer malloc");

			}
			if (!isDirectory(file)){
				#ifdef DEBUG
					printf("Producer: nome file: %s\n", file);
				#endif
				//tra tutti i file presenti nella cartella seleziono solo quelli che hanno ".dat" nel nome
				if(strstr(dir_p -> d_name, ".dat")){
					for(int i = 0; i< n_args; i++){
						if(f[i]) 	
							if(strcmp(f[i], file)==0){
								push(coda, file);
								#ifdef DEBUG
									printf("Producer: file pushed %s\n", file);
								#endif
								//Sleep(t);
							}
					}
				}
				else
					//libero la memoria dei file che non hanno ".dat" nel nome
					free(file);
			}
			else {
				#ifdef DEBUG
					printf("nome cartella: %s\n", file);
				#endif
				//libero la memoria delle stringhe col nome delle cartelle
				free(file);
			}
			mySleep(t, termina);
			//Sleep(t);
		}
		closedir(dp);
	}
	return 0;
}
