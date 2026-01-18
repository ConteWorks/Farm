#define _POSIX_C_SOURCE  200112L
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <stdatomic.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "conn.h"

//#define DEBUG

/** 
 * tipo del messaggio
 */
typedef struct msg {
    int len;
    char *str;
} msg_t;


void cleanup() {
    unlink(SOCKNAME);
}

typedef struct elemento
{
       long result;
       char file[255];
       struct elemento *next;
} edl;
typedef edl *ldi;

//stampa la lista di elementi
void stampa(ldi l){
	ldi corr=l;
    while(corr!=NULL)
    {
    	printf("%ld %s\n",corr->result, corr->file);
     	corr=corr->next;
    }
}

//inserisce in maniera ordinata l'elemento nella lista
ldi inserisci(ldi l, long result, char *file){
	ldi new=malloc(sizeof(edl));
	new->result=result;
	strcpy(new->file, file);
	new->next=NULL;
    if (l==NULL)
		l=new;
	else{
		ldi corr=l;
        ldi prec=NULL;
        int trovato=0;
        while (corr!=NULL && !trovato)
       		if(corr->result < result){
            	prec=corr;
                corr=corr->next;
            }
            else trovato++;
           	if (prec==NULL){
            	new->next=l;
                l=new;
             }
             else{
             	prec->next=new;
                new->next=corr;
             }
    }     
    return l;
}

//libera la memoria della lista
void myfree(ldi l){
	ldi corr = l;
	ldi prec;
	while (corr !=NULL){
		prec = corr;
		corr=corr ->next;
		free(prec);
	}
		
}

void collector(){
	cleanup();    
    atexit(cleanup);    

    int listenfd;
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");

    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;    
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    int notused;
    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)), "bind", "");
    SYSCALL_EXIT("listen", notused, listen(listenfd, MAXBACKLOG), "listen", "");
	
	//creo una lista ordinata di elementi per contenere i risultati
	ldi risultati=NULL;

	//si esce dal ciclo quando viene ricevuto un messaggio "close" dal canale socket
	int closed = 0;
    while(!closed) {      
		long connfd;
		SYSCALL_EXIT("accept", connfd, accept(listenfd, (struct sockaddr*)NULL ,NULL), "accept", "");
		#ifdef DEBUG
			printf("collector: connection accepted\n");    	
		#endif

		do {
			msg_t str;
			long r, result = 0;
			int n;
			SYSCALL_EXIT("readn", n, readn(connfd, &str.len, sizeof(int)), "read", "");
			//EOS
			if (n==0){
				break;
			}
			str.str = calloc((str.len), sizeof(char));
			if (!str.str) {
				perror("calloc");
				fprintf(stderr, "Memoria esaurita....\n");
			}		    
			SYSCALL_EXIT("readn", n, readn(connfd, str.str, str.len * sizeof(char)), "read", "");
			if(strcmp(str.str, "close") == 0){
				#ifdef DEBUG
					printf("collector: signal close received\n");    	
				#endif
				closed = 1;
				if(str.str)
					free(str.str);
				break;
			}
			else{
				if(strcmp(str.str, "sigusr1")==0){
					#ifdef DEBUG
						printf("collector: signal stampa received\n");    	
					#endif
					stampa(risultati);
					if(str.str)
						free(str.str);
					break;
				}
				else{
					SYSCALL_EXIT("readn", r, readn(connfd, &result, sizeof(long)), "read", "");
					risultati=inserisci(risultati, result, str.str);
				}
			}
		#ifdef DEBUG
			printf("collector: received %ld %s\n", result, str.str);    	
		#endif
		if(str.str)
				free(str.str);
    	} while(1);
    	close(connfd);
    	#ifdef DEBUG
			printf("collector: connection closed\n");    	
		#endif
    }
    #ifdef DEBUG
    	printf("collector: fine calcolo dei risultati:\n");
    #endif

    //stampa dei risultati
    stampa(risultati);
   	//libero la memoria
    myfree(risultati);

	exit(0);
}


