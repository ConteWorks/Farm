#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>

#include "boundedqueue.h"
#include "util.h"
#include "conn.h"

//#define DEBUG


void *Worker(void *arg){
	#ifdef DEBUG
		printf("Nuovo Thread Consumatore\n");
	#endif
	BQueue_t *c = (BQueue_t*)arg;
	
	//questa sleep è un errore, ma mi serve metterla per far partire prima il collector
	//altrimenti ho problemi con la connessione della socket
	sleep(1);
	

	while(1){
		struct sockaddr_un serv_addr;
		int sockfd;
		SYSCALL_EXIT("socket", sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
		memset(&serv_addr, '0', sizeof(serv_addr));

		serv_addr.sun_family = AF_UNIX;    
		strncpy(serv_addr.sun_path,SOCKNAME, strlen(SOCKNAME)+1);

		int notused;
		SYSCALL_EXIT("connect", notused, connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "connect", "");
    
		char *file = pop(c);
		if (file == EOS){
			close(sockfd);
			break;
		}
		long result = 0;
		FILE *fl;
		long fd;
		/* apertura file binario*/
		if ((fl = fopen(file, "rb")) != NULL){
			/*lettura file binario e calcolo result*/
			int i =0;
			int res;
			while (1){
				res = fread(&fd,sizeof(long),1,fl);
				result = result + (fd *i);
				i++;
				fd = 0;		
				if (res != 1)
					break;
			}
			/*chiusura file*/
			fclose(fl);
			#ifdef DEBUG
				printf("Worker - Thread %ld: Fine lettura file %s\n", pthread_self(), file);
				printf("Worker - Thread %ld]: %ld %s\n", pthread_self(), result, file);
			#endif
			int n=strlen(file)+1;
			SYSCALL_EXIT("writen", notused, writen(sockfd, &n, sizeof(int)), "write", "");
			SYSCALL_EXIT("writen", notused, writen(sockfd, file, n*sizeof(char)), "write", "");
			SYSCALL_EXIT("writen", notused, writen(sockfd, &result, sizeof(long)), "write", "");
		}
		if(file)
			free(file);
		close(sockfd);
	}

	pthread_exit(NULL);
}
