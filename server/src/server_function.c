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

uint32_t receive_register_content(int socket){
	char msg[sizeof(uint32_t) * 8 + 1];
	char *ptr;
	if(read(socket, msg, sizeof(msg)) < 0){

		fprintf(stderr, "errore lettura\n");
		return -1;
	}else{
		uint32_t data = strtoul(msg, &ptr, 16);
		printf("ho ricevuto: %x\n", data);
		return data;
	}
}
void *high_priority(void *socket){

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
	uint32_t data;
	int ret;
    bzero(msg, sizeof(msg));

    for(int i = 1; i < 8; i++){

    	reg = i;
    	data = receive_register_content(socket);
    	ret = write_register(reg, &data);
    	printf("ho scritto: %x nel registro %d\n", data, reg);
    }

    puts("reset");
    data = 0x00000003;
	write_register(0, &data);
	data = 0x00000000;
	write_register(0, &data);
	puts("fine reset");
}

void SetDelay(int socket){

	char msg[sizeof(uint32_t) * 8 + 1];
	char *ptr;
	printf("ricevo delay\n");
	if(read(socket, msg, sizeof(msg)) < 0){

		fprintf(stderr, "errore lettura\n");
	}else{
		printf("ricevuto\n");
		uint32_t delay = strtoul(msg, &ptr, 16);
		printf("delay: %x\n", delay);
		if(write(socket, msg, strlen(msg) + 1) < 0){

			fprintf(stderr, "errore scrittura\n");
		}else{


		}


	}

	uint32_t data = 0x00000000;
	write_register(0, &data);
	int delay = receive_register_content(socket);
	ReadReg(7, &data);
	delay &= 0x0000ffff;
	data &= 0xFFFF0000;
	data |= delay;
	write_register(7, &data);
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
	int mode;
	uint32_t data;
	mode = receive_register_content(socket);
	if(mode == 0){
		data = 0x00000000;
		write_register(0, &data);
	}
	else if(mode == 1){
		data = 0x00000003;
		write_register(0, &data);
		data = 0x00000008;
		write_register(0, &data);
	}

}

void GetEventNumber(int socket){

	char msg[256];
	int event_number = 2;
	sprintf(msg, "%s %d", "[SERVER]numero evento: ", event_number);
	if(write(socket, msg, strlen(msg)) < 0){

		fprintf(stderr, "errore lettura\n");
	}else{


	}

	uint32_t data = 0x00000000;
	int ret = write_register(0, &data);
	ReadReg(1, &data);
	data &= 0xFFFFFF8F;
	data |= 0x00000050;
	write_register(1, &data);
	uint32_t external_trigger_counter, internal_trigger_counter;

	ReadReg(23, &external_trigger_counter);
	ReadReg(24, &internal_trigger_counter);

}

void PrintAllEventNumber(int socket){

	char msg[256] = "[SERVER] stampo numero eventi";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrittura");
	}else{


	}

	uint32_t data;
	ReadReg(1, &data);
	data &= 0xFFFFFF8F;
	data |= 0x00000050;
	write_register(1, &data);
	uint32_t external_trigger_counter, internal_trigger_counter;

	ReadReg(23, &external_trigger_counter);
	ReadReg(24, &internal_trigger_counter);

	printf("%d\n %d\n", external_trigger_counter, internal_trigger_counter);
}

void EventReset(int socket){

	char *msg = "[SERVER] resetto";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrittura");
	}

	uint32_t data = 0x00000003;
	int ret = write_register(0, &data);
	data = 0x00000000;
	ret = write_register(0, &data);
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

		fprintf(stderr, "errore scrittura");
	}

	int delay;
	delay = receive_register_content(socket);
	uint32_t data;
	ReadReg(7, &data);
	delay &= 0x0000ffff;
	data &= 0xFFFF0000;
	data |= delay;
	write_register(7, &data);
}

void Calibrate(int socket){

	char *msg = "[SERVER] Calibrate";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrittura");
	}

	uint32_t data = 0x00000003;
	int ret = write_register(0, &data);
	data = 0x00000000;
	ret = write_register(0, &data);
	ReadReg(2, &data);
	data &= 0xFFFFFFFD;
	data |= 0x00000002;
	ret = write_register(2, &data);
}

void WriteCalibPar(int socket){

	char *msg = "[SERVER] WriteCalibPar";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrittura");
	}
}

void SaveCalibrations(int socket){

	char *msg = "[SERVER] SaveCalibrations";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrittura");
	}
}

//Update the internal trigger period without changing the other configs
void intTriggerPeriod(int socket){
	char msg[sizeof(uint32_t) * 8 + 1];
	char *ptr;
	printf("Ricevo internal trigger period\n");
	if(read(socket, msg, sizeof(msg)) < 0){
		fprintf(stderr, "Errore lettura\n");
	}else{
		printf("Ricevuto\n");
		uint32_t period = strtoul(msg, &ptr, 16);
		printf("Period: %x clock cycles\n", period);
		if(write(socket, msg, strlen(msg) + 1) < 0){
			fprintf(stderr, "Errore scrittura\n");
		}
	}

	uint32_t regContent;
	ReadReg(2, &regContent);
	regContent = (period & 0xFFFFFFF0) | (regContent & 0x0000000F);
	int ret = write_register(2, &regContent);
}

//Enable/Disable the internal trigger
void selectTrigger(int socket){
	char msg[sizeof(uint32_t) * 8 + 1];
	char *ptr;
	printf("Ricevo internal trigger enable\n");
	if(read(socket, msg, sizeof(msg)) < 0){
		fprintf(stderr, "Errore lettura\n");
	}else{
		printf("Ricevuto\n");
		uint32_t intTrig = strtoul(msg, &ptr, 16);
		printf("Internal trigger enabled: %d\n", intTrig);
		if(write(socket, msg, strlen(msg) + 1) < 0){
			fprintf(stderr, "Errore scrittura\n");
		}
	}

	uint32_t regContent;
	ReadReg(2, &regContent);
	regContent = (regContent & 0xFFFFFFF0) | (intTrig & 0x00000001);
	int ret = write_register(2, &regContent);
}

//Configure and enable/disable the test unit
void configureTestUnit(int socket){
	char msg[sizeof(uint32_t) * 8 + 1];
	char *ptr;
	printf("Ricevo Test-Unit configuration\n");
	if(read(socket, msg, sizeof(msg)) < 0){
		fprintf(stderr, "Errore lettura\n");
	}else{
		printf("Ricevuto\n");
		uint32_t cmdRx = strtoul(msg, &ptr, 16);
		char testUnitCfg = ((cmdRx&0x300)>>8);
		char testUnitEn  = ((cmdRx&0x2)>>1);
		printf("Test Unit Configuration: %x - Test-Unit enable: %d\n", testUnitCfg, testUnitEn);
		if(write(socket, msg, strlen(msg) + 1) < 0){
			fprintf(stderr, "Errore scrittura\n");
		}
	}

	uint32_t regContent;
	ReadReg(1, &regContent);
	regContent = (regContent & 0xFFFFFCFD) | (cmdRx & 0x00000302);
	int ret = write_register(1, &regContent);
}
