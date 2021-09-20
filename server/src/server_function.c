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

// Reset della logica FPGA
void ResetFpga(){
	uint32_t data;
	data = 0x00000003;
	write_register(0, &data);
	data = 0x00000000;
	write_register(0, &data);
}

//Inizializza l'array di registri e resetta la logica FPGA
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
  for(int i = 1; i < 8; i++){
		reg = i;
		data = receive_register_content(socket);
		ret = write_register(reg, &data);
		printf("ho scritto: %x nel registro %d\n", data, reg);
	}

	puts("reset");
	ResetFpga();
	puts("fine reset");
}

// Numero di cicli di clock di attesa tra il trigger e l'hold dei VA
void SetDelay(int socket){
	char msg[sizeof(uint32_t) * 8 + 1];
	uint32_t data;

	int delay = receive_register_content(socket);
	ReadReg(7, &data);
	data = (data & 0xFFFF0000) | (delay & 0x0000ffff);
	write_register(7, &data);

	sprintf(msg, "%s %d", "[SERVER] Delay: ", delay);
	if(write(socket, msg, strlen(msg) + 1) < 0){
		fprintf(stderr, "Errore Scrittura\n");
	}
}

//Configura la modalità: Stop(0), Run(1)
void SetMode(int socket){
	char msg[256];

	int mode;
	uint32_t data;
	mode = receive_register_content(socket);
	if(mode == 0){
		data = 0x00000000;
		write_register(0, &data);
	}
	else if(mode == 1){
		ResetFpga();
		data = 0x00000010;
		write_register(0, &data);
	}

	sprintf(msg, "%s %d", "[SERVER]imposto modalità: ", mode);
	if(write(socket, msg, strlen(msg) + 1) < 0){
		fprintf(stderr, "errore lettura\n");
	}
}

//Cattura il valore del trigger counter interno ed esterno
void GetEventNumber(int socket){
	char msg[256];
	uint32_t external_trigger_counter, internal_trigger_counter;

	ReadReg(23, &external_trigger_counter);
	ReadReg(24, &internal_trigger_counter);

	sprintf(msg, "%s %08u %08u", "[SERVER] Numero evento esterno e interno: ", \
														external_trigger_counter, internal_trigger_counter);
	if(write(socket, msg, strlen(msg)) < 0){
		fprintf(stderr, "errore lettura\n");
	}
}

//Stampa trigger counter interno ed esterno
void PrintAllEventNumber(int socket){
	char msg[256];
	uint32_t external_trigger_counter, internal_trigger_counter;

	ReadReg(23, &external_trigger_counter);
	ReadReg(24, &internal_trigger_counter);

	sprintf(msg, "%s %08u %08u", "[SERVER] Numero evento esterno e interno: ", \
														external_trigger_counter, internal_trigger_counter);
	if(write(socket, msg, strlen(msg)) < 0){
		fprintf(stderr, "errore lettura\n");
	}

	printf("Trigger number: External: %d - Internal: %d\n", \
														external_trigger_counter, internal_trigger_counter);
}

//Reset della logica FPGA
void EventReset(int socket){
	char *msg = "[SERVER] Resetto";

	ResetFpga();

	if(write(socket, msg, strlen(msg) + 1) < 0){
		fprintf(stderr, "errore scrittura");
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

//Come setdelay solo che non legge da file ma prende parametro
void OverWriteDelay(int socket){

	char *msg = "[SERVER] OverWriteDelay";
	if(write(socket, msg, strlen(msg) + 1) < 0){

		fprintf(stderr, "errore scrittura");
	}

	uint32_t data;
	int delay = receive_register_content(socket);
	ReadReg(7, &data);
	data = (data & 0xFFFF0000) | (delay & 0x0000ffff);
	write_register(7, &data);

	sprintf(msg, "%s %04u", "[SERVER] Delay: ", delay);
	if(write(socket, msg, strlen(msg)) < 0){
		fprintf(stderr, "Errore Scrittura su socket\n");
	}
}

//Configura sistema in modalità calibrazione
void Calibrate(int socket){

	char *msg = "[SERVER] Calibrate";
	if(write(socket, msg, strlen(msg) + 1) < 0){
		fprintf(stderr, "errore scrittura");
	}

	int mode = receive_register_content(socket);
	uint32_t data;
	int ret;
	ReadReg(2, &data);
	data = (data & 0xFFFFFFFD) | (mode & 0x00000002);
	ret = write_register(2, &data);

	sprintf(msg, "%s %u", "[SERVER] Calibration enable: ", mode);
	if(write(socket, msg, strlen(msg)) < 0){
		fprintf(stderr, "Errore Scrittura su socket\n");
	}
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
	char msg[sizeof(uint32_t) * 10 + 1];
	uint32_t regContent;
	uint32_t period = receive_register_content(socket);

	ReadReg(2, &regContent);
	regContent = (period & 0xFFFFFFF0) | (regContent & 0x0000000F);
	int ret = write_register(2, &regContent);

	sprintf(msg, "%s %08u", "[SERVER] Trigger period: ", period);
	if(write(socket, msg, strlen(msg)) < 0){
		fprintf(stderr, "Errore Scrittura su socket\n");
	}
}

//Enable/Disable the internal trigger
void selectTrigger(int socket){
	char msg[sizeof(uint32_t) * 8 + 1];
	uint32_t regContent;
	uint32_t intTrig = receive_register_content(socket);

	ReadReg(2, &regContent);
	regContent = (regContent & 0xFFFFFFF0) | (intTrig & 0x00000001);
	int ret = write_register(2, &regContent);

	sprintf(msg, "%s %u", "[SERVER] Trigger enable: ", intTrig);
	if(write(socket, msg, strlen(msg)) < 0){
		fprintf(stderr, "Errore Scrittura su socket\n");
	}
}

//Configure and enable/disable the test unit
void configureTestUnit(int socket){
	char msg[sizeof(uint32_t) * 8 + 1];
	uint32_t regContent;
	uint32_t cmdRx = receive_register_content(socket);

	char testUnitCfg = ((cmdRx&0x300)>>8);
	char testUnitEn  = ((cmdRx&0x2)>>1);
	ReadReg(1, &regContent);
	regContent = (regContent & 0xFFFFFCFD) | (cmdRx & 0x00000302);
	int ret = write_register(1, &regContent);

	sprintf(msg, "%s %x %u", "[SERVER] Test Unit status: ", \
																											testUnitCfg, testUnitEn);
	if(write(socket, msg, strlen(msg)) < 0){
		fprintf(stderr, "Errore Scrittura su socket\n");
	}
}
