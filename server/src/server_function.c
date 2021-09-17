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
#include "user_avalon_fifo_regs.h"
#include "user_avalon_fifo_util.h"
#include "hps_0.h"
#include <sys/mman.h>
#include <fcntl.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "user_register_array.h"

#define HW_REGS_BASE ( ALT_STM_OFST )		// Physical base address: 0xFC000000
#define HW_REGS_SPAN ( 0x04000000 )			// Span Physical address: 64 MB
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

//metodi che leggono e scrivono registri da fare

extern void *virtual_base;			// Indirizzo base dell'area di memoria virtuale (variabile globale).
int error;							// Flag per la segnalazione di errori.
int fd;								// File descriptor degli indirizzi fisici.							// Tipo di operazione che si vuole compiere sulla FIFO (lettura/scrittura).
int num_el;							// Numero di scritture consecutive della FIFO che si vogliono compiere.
int el;								// Indice del numero di scritture della FIFO.
uint32_t new_value;					// Valore singolo con il quale verrà caricata la FIFO.
uint32_t v_data_array[1021];		// Array di 1021 elementi di tipo uint32_t, da riempire in scrittura.
uint32_t p_data_array[1021];		// Array di 1021 elementi di tipo uint32_t, da svuotare in lettura.
uint32_t f_data_array[4093];		// Array di 4093 elementi di tipo uint32_t, da svuotare in lettura.
uint32_t value_read;				// Valore in uscita dalla FIFO.
uint32_t fifo_level;				// Livello di riempimento della FIFO.
uint32_t fifo_full;					// bit di "full" della FIFO.
uint32_t fifo_empty;				// bit di "empty" della FIFO.
uint32_t fifo_almostfull;			// bit di "almostfull" della FIFO.
uint32_t fifo_almostempty;			// bit di "almostempty" della FIFO.
uint32_t almostfull_setting;		// livello di "almostfull" della FIFO.
uint32_t almostempty_setting;		// livello di "almostempty" della FIFO.

int receive_register_content(int socket){
	char msg[4];
	if(read(socket, msg, sizeof(msg)) < 0){

		fprintf(stderr, "errore lettura\n");
	}else{
		//strcpy(data, msg);
		int data = atoi(msg);
		return data;
	}
}
void *high_priority(void *socket){

	/*srand(time(NULL));
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
	}*/
	int n;
	int sock = *(int *)socket;
	uint32_t length;
	error = StatusFifo(DATA_FIFO, &fifo_level, &fifo_full, &fifo_empty, &fifo_almostfull, &fifo_almostempty, &almostfull_setting, &almostempty_setting);
	error = ReadFifo(DATA_FIFO, &value_read);
	printf("start: %x\n", value_read);
	if(value_read == 0xBABA1AFA){

		error = ReadFifo(DATA_FIFO, &length);
		printf("errore lettura: %d\n", error);
		printf("lunghezza pacchetto: %d\n", length);
	}

	uint32_t packet[length + 1];
	packet[0] = 0xBABA1AFA;
	packet[1] = length;
	error = ReadFifoBurst(DATA_FIFO, packet + 2, length - 1);
	printf("lettura burst error: %d\n", error);
	printf("ho inviato questi dati.\n");
	for(int i = 0; i < length; i++){

		printf("%x\n", packet[i]);
	}
	
	n = write(sock, &packet, sizeof(packet));
	if(n < 0){

		perror("errore scrittura");
	}else{

		printf("ho inviato: %d\n", n);
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

	uint16_t reg;
	int ret;
    bzero(msg, sizeof(msg));

    for(int i = 1; i < 8; i++){

    	reg = i;
    	data = receive_register_content(socket);
    	ret = write_register(reg, &data);
    }

	write_register(0, 0x00000003);
	write_register(0, 0x00000000);
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