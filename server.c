#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sched.h>


struct sockaddr_in client_addr, server_addr; 
int client, new_socket,n, ricevuto;


void *high_priority(){

	char msg[4096];
	int n;
	for(int i = 0; i < sizeof(msg); i++){

			strcpy(&msg[i], "1");
		}

	if(n = write(new_socket, msg, sizeof(msg)) < 0){

		fprintf(stderr, "errore scrittura\n");
	}else{

		printf("thread: ho scritto %d bytes\n", n);
	}
	pthread_exit(NULL);
}

void *sender(){

	//qui viene settato il thread ad alta priorità che viene creato se il comando inserito è "casuali"
	pthread_t thread;
	pthread_attr_t attr;
	int ret;
	int new_priority = 20;
	struct sched_param param;

	ret = pthread_attr_init(&attr);
	ret = pthread_attr_getschedparam(&attr, &param);
	param.sched_priority = new_priority;
	ret = pthread_attr_setschedparam(&attr, &param);

	char msg[256];
	while(1){

		scanf("%s", msg);
		if(strcmp(msg, "")){

			if(n = write(new_socket, msg, sizeof(msg)) < 0){

				fprintf(stderr, "errore scrittura\n");
			}else{

				printf("[SERVER] ho scritto\n");
			}

			if(strcmp(msg, "casuali") == 0){

				ret = pthread_create(&thread, &attr, high_priority, NULL);
			}

		}
		
	}
	pthread_exit(NULL);
}


void *receiver(){


	while(1){

		int n;
		char msg[256];
		if(n = read(new_socket, msg, sizeof(msg)) < 0){

			fprintf(stderr, "errore nella read\n");
		}else{

			printf("%s\n", msg);
		}

		bzero(msg, sizeof(msg));
	}

	pthread_exit(NULL);
}


int main(int argc, char *argv[]){

	if(argc < 2){

		fprintf(stderr,"errore, fornire una porta\n");
		exit(1);
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){

		fprintf(stderr, "errore, impossibile creare socket\n");
	}else{

		printf("socket creato\n");
		fflush(stdout);
	}

	int port_no = atoi(argv[1]);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_no);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sock, (struct sockaddr *) &server_addr , sizeof(server_addr)) < 0){

		fprintf(stderr, "errore nel bind\n");
	}else{

		printf("bind corretto\n");
		fflush(stdout);
	}

	if(listen(sock, 5) < 0){

		fprintf(stderr,"impossibile ascoltare\n");
	}
	client = sizeof(client_addr);
	new_socket = accept(sock, (struct sockaddr *) &client_addr, &client);
	if(new_socket < 0){

		fprintf(stderr, "errore accettazione\n");
	}else{

		printf("connessione risucita\n");
		fflush(stdout);
	}

	//a connessione avvenuta creo i due threads
	pthread_t threads;
	pthread_create(&threads, NULL, sender, NULL);
	pthread_create(&threads, NULL, receiver, NULL);
	while(1){




	}	

	return(0);
}