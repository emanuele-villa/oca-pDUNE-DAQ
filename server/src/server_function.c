#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>
#include "server_function.h"


//metodi che leggono e scrivono registri da fare



void *high_priority(void *socket){

	srand(time(NULL));
	int sock = *(int*) socket;
	printf("alta priorità socket: %d\n", sock);
	
	int num[640];
	int count = 640;
	for(int i = 0; i < 640; i++){

		int r = random() % 100;
		num[i] = r;
	}

	int n;
	n = write(sock, &num, sizeof(num));
	if(n < 0){

		perror("errore scrittura:");
	}

	pthread_exit(NULL);
}

void Init(int socket){

	char msg[256] = "[SERVER] faccio init";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrittura");

	}else{

		printf("[SERVER] comando inviato. %s\n", msg);
	}
}

void SetDelay(int socket){

	char msg[256];
	printf("ricevo delay\n");
	if(read(socket, msg, sizeof(msg)) < 0){

		fprintf(stderr, "errore lettura\n");
	}else{
		printf("ricevuto\n");
		//strcpy(data, msg);
		int delay = atoi(msg);
		sprintf(msg, "%s %d", "imposto delay:", delay);
		if(write(socket, msg, strlen(msg) + 1) < 0){

			fprintf(stderr, "errore scrittura\n");
		}else{


		}
		

	}
	bzero(msg, sizeof(msg));

}

void SetMode(int socket){

	char msg[256];
	if(read(socket, msg, sizeof(msg)) < 0){

		fprintf(stderr, "errore lettura\n");
	}else{

		int mode = atoi(msg);
		sprintf(msg, "%s %d", "[SERVER]imposto modalità: ", mode);
		if(write(socket, msg, strlen(msg) + 1) < 0){

			fprintf(stderr, "errore lettura\n");
		}else{


		}
	}

	bzero(msg, sizeof(msg));
}

void GetEventNumber(int socket){

	char msg[256];
	int event_number = 2;
	sprintf(msg, "%s %d", "[SERVER]numero evento: ", event_number);
	if(write(socket, msg, strlen(msg)) < 0){

		fprintf(stderr, "errore lettura\n");
	}else{


	}
}

void PrintAllEventNumber(int socket){

	char msg[256] = "[SERVER] stampo numero eventi";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrittura");
	}else{


	}
}

void EventReset(int socket){

	char *msg = "[SERVER] resetto";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrttura");
	}
}

//manda un evento (pacchetto fast data fifo)
void GetEvent(int socket){

	pthread_t t;
	pthread_attr_t attr;
	int ret;
	int new_priority = 20;
	struct sched_param param;

	ret = pthread_attr_init(&attr);
	ret = pthread_attr_getschedparam(&attr, &param);
	param.sched_priority = new_priority;
	ret = pthread_attr_setschedparam(&attr, &param);

	printf("socket: %d\n", socket);
	if(pthread_create(&t, &attr, &high_priority, &socket) < 0){

		perror("thread create");
	}
	pthread_join(t, 0);

}

//come setdelay solo che non legge da file ma prende parametro
void OverWriteDelay(int socket){

	char *msg = "[SERVER] OverWriteDelay";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrttura");
	}
}

void Calibrate(int socket){

	char *msg = "[SERVER] Calibrate";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrttura");
	}
}

void WriteCalibPar(int socket){

	char *msg = "[SERVER] WriteCalibPar";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrttura");
	}
}

void SaveCalibrations(int socket){

	char *msg = "[SERVER] SaveCalibrations";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrttura");
	}
}