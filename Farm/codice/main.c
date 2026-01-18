#define _POSIX_C_SOURCE  200112L
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <stdatomic.h>

#include "boundedqueue.h"
#include "util.h"
#include "myutil.h"
#include "conn.h"

//#define DEBUG
int producer(long q, long t, char *d, BQueue_t *coda, char **f, int n_args, atomic_int *termina);
void *Worker(void *arg);
void collector();

atomic_int termina =0; //variabile globale che viene settato quando sopraggiunge un segnale da gestire

static void msg_server(char *msg){
	struct sockaddr_un serv_addr;
	int sockfd;
	SYSCALL_EXIT("socket", sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sun_family = AF_UNIX;    
	strncpy(serv_addr.sun_path,SOCKNAME, strlen(SOCKNAME)+1);

	if(sockfd){
		int notused;
		SYSCALL_EXIT("connect", notused, connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "connect", "");
		int n=strlen(msg)+1;
		SYSCALL_EXIT("writen", notused, writen(sockfd, &n, sizeof(int)), "write", "");
		SYSCALL_EXIT("writen", notused, writen(sockfd, msg, n*sizeof(char)), "write", "");
	}
	close(sockfd);
				
}
// funzione eseguita dal signal handler thread
static void *sigHandler(void *arg) {
    sigset_t *set = (sigset_t*)arg;
	
    for( ;; ) {
		int sig;
		int r = sigwait(set, &sig);
		if (r != 0) {
			errno = r;
			perror("FATAL ERROR 'sigwait'");
			return NULL;
		}

		switch(sig) {
			case SIGINT:
				termina = 1;
				#ifdef DEBUG
					printf("main: sigint ricevuto\n");
				#endif
				break;
			case SIGTERM:
				termina = 1;
				#ifdef DEBUG
					printf("main: sigterm ricevuto\n");
				#endif
				break;
			case SIGQUIT:
				termina = 1;
				#ifdef DEBUG
					printf("main: sigquit ricevuto\n");
				#endif
				break;
			case SIGHUP:
				termina = 1;
				#ifdef DEBUG
					printf("main: sighup ricevuto\n");
				#endif
				break;
			case SIGUSR1: 
				#ifdef DEBUG
					printf("main: sigusr1 ricevuto\n");
				#endif
				msg_server("sigusr1");
				break;
			default:  
				break; 
		}
    }
    return NULL;	   
}

int main(int argc, char *argv[]){
	long n = 4, q = 8, t = 0;
	int  n_found = 0, q_found = 0, d_found = 0, t_found = 0, h_found = 0;
	char *d = NULL;
	char opt;
	
	
	//i nomi dei file.dat che vengono passati come parametri li salvo nell'array f
	char **f=NULL;
	int i = 1;
	int j=0;
	if (argc > 1){
		while(i < argc){
			if(strstr(argv[i], ".dat")){
				if((f=realloc(f,(j+1)*sizeof(char**)))){
					f[j]=argv[i];
					j++;
				}
				else{
					perror("realloc");
					return 0;
				}
			}
			i++;
		}
	}
	int n_args = j;
	#ifdef DEBUG
		for(int k=0; k< n_args; k++)
			printf("%s\n", f[k]);
	#endif	
	//lettura dei parametri 
	while ((opt = getopt(argc, argv, ":n:q:d:t:h")) != -1){
		switch (opt){
			case 'n':
				if ((n = isNumber(optarg)) != -1)
					n_found = 1;
				else
					n_found = -1;	
			break;
			case 'q':
				if ((q = isNumber(optarg)) != -1)
					q_found = 1;
				else
					q_found = -1;	
			break;
			case 'd':
				d = optarg;
				if (d != NULL && isDirectory(d))
					d_found = 1;
				else
					d_found = -1;	
			break;
			case 't':
				if ((t = isNumber(optarg)) != -1)
					t_found = 1;
				else
					t_found = -1;		
			break;
			case 'h':
				h_found = 1;
			break;
			case ':' :
				printf("l'opzione '-%c' richiede un argomento\n", optopt);
			break;
			case '?':
				printf("opzione -%c non riconosciuta\n", optopt);
			break;
			default:
				printf("opzione -%c non riconosciuta\n", optopt);
			break;
			
		}
	}
	if (h_found == 0){
		#ifdef DEBUG
			if (n_found == 1)
				printf("-n: %ld\n",n);
			else if (n_found == -1)
				printf("parametro n non valido\n");
			if (q_found == 1)
				printf("-q: %ld\n",q);
			else if (q_found == -1)
				printf("parametro q non valido\n");
			if (d_found == 1)
				printf("-d: %s\n", d);
			else if (d_found == -1)
				printf("parametro d non valido\n");
			if (t_found == 1)
				printf("-t: %ld\n", t);
			else if (t_found == -1)
				printf("parametro t non valido\n");
		#endif
		if (n_found == -1 || q_found == -1 || d_found == -1 || t_found == -1 || (d_found == 0 && f== NULL)){
			return 0;
		}
		else{
			//maschero i segnali
			sigset_t     mask;
			sigemptyset(&mask);
			sigaddset(&mask, SIGINT); 
			sigaddset(&mask, SIGQUIT);
			sigaddset(&mask, SIGTERM); 
			sigaddset(&mask, SIGHUP);     
			sigaddset(&mask, SIGUSR1);    
			if (pthread_sigmask(SIG_BLOCK, &mask,NULL) != 0) {
				fprintf(stderr, "FATAL ERROR\n");
				abort();
			}
			// ignoro SIGPIPE per evitare di essere terminato da una scrittura su un socket
			struct sigaction s;
			memset(&s,0,sizeof(s));    
			s.sa_handler=SIG_IGN;
			if ( (sigaction(SIGPIPE,&s,NULL) ) == -1 ) {   
				perror("sigaction");
				abort();
			} 
			
			//avvio il processo per il collector
			pid_t pid;
    		pid = fork();
    		if (pid == 0){
    			#ifdef DEBUG
    				printf("Main: fork effettuata, pid: %d -> processo figlio\n", pid);
    			#endif
    			collector();
    		}
    		if (pid > 0){
    			#ifdef DEBUG
    				printf("Main: fork effettuata, pid: %d -> processo padre\n", pid);
    				printf("Main: il pid del processo padre è: %d\n", getpid());
    			#endif
    			
    			//avvio il pthread handler
    			pthread_t sighandler_thread;
				if (pthread_create(&sighandler_thread, NULL, sigHandler, &mask) != 0) {
					fprintf(stderr, "errore nella creazione del signal handler thread\n");
					abort();
				}

    			//avvio la coda concorrente dei task da elaborare
				BQueue_t *coda = initBQueue(q);
				if (!coda) {
					fprintf(stderr, "initBQueue fallita\n");
					exit(errno);
				}
				
				//avvio i consumatori, un thread per ogni consumatore
				pthread_t    *th;
				th     = malloc((n)*sizeof(pthread_t));
				if (!th) {
					fprintf(stderr, "malloc fallita\n");
					exit(EXIT_FAILURE);
				}
				for(int i=0;i<n; ++i)
					if (pthread_create(&th[i], NULL, Worker, coda) != 0) {
						fprintf(stderr, "pthread_create failed (Producer)\n");
						exit(EXIT_FAILURE);
					}
				//avvio il produttore
				producer(q, t, d, coda, f, n_args, &termina);

				
				// produco tanti EOS quanti sono i consumatori
				for(int i=0;i<n; ++i) {
					push(coda, EOS);
				}
				
				// aspetto la terminazione di tutti i consumatori
				for(int i=0;i<n; ++i)
					pthread_join(th[i], NULL);
					
				//chiusura collector	
				msg_server("close");	
				#ifdef DEBUG
					printf("main: message server close send\n");    	
				#endif
				
				//attendo la terminazione del processo collector e termino il programma
				int status;
    			if (wait(&status) == -1) {
		   			perror("wait");
		    		return 0;
				}
				
				//termino il signal handler thread
         		pthread_cancel(sighandler_thread); 
    			if (pthread_join(sighandler_thread, NULL) != 0) {
      				fprintf(stderr, "Error joining sighandler_thread\n");
    			}
    			// libero memoria
				deleteBQueue(coda, NULL);
				free(th);			
				if(f)
					free(f);	
			}
    		if (pid < 0){
    			perror("fork"); 
    			exit(-1); 
    		}
 		} 
	}
	else
		printf("%s -n <nthread> -q <qlen> -d <directory-name>  -t <delay> ''list_of_fileX.dat'' -h\n", argv[0]);	
	return 0;
}
