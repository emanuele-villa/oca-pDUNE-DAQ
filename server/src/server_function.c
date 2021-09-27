#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "hwlib.h"

#include "user_avalon_fifo_util.h"
#include "user_register_array.h"
#include "server_function.h"
#include "server.h"


int error;							// Flag per la segnalazione di errori.
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

//Send the incoming string to socket
int sendSocket(int socket, char * msg){
	if(write(socket, msg, strlen(msg)) < 0){
		fprintf(stderr, "Errore Scrittura su socket\n");
		return 1;
	}
	return 0;
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
	for(int i = 0; i < length+1; i++){
		printf("%08x\n", packet[i]);
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
	uint32_t regContent;
	regContent = 0x00000003;
	write_register(rGOTO_STATE, &regContent);
	regContent = 0x00000000;
	write_register(rGOTO_STATE, &regContent);
}

////Inizializza l'array di registri e resetta la logica FPGA
//void Init(uint32_t * regsContentIn, uint32_t opLen){
//	//Configure the whole regArray (except register rGOTO_STATE)
//	writeRegMulti(regsContentIn, opLen);
//
//	//Reset the FPGA
//	ResetFpga();
//}

//Inizializza l'array di registri e resetta la logica FPGA
void Init(int socket){
	uint16_t reg;
	uint32_t data;
  for(int i = 1; i < 8; i++){
		reg = i;
		data = receive_register_content(socket);
		write_register(reg, &data);
		printf("ho scritto: %x nel registro %d\n", data, reg);
	}

	puts("reset");
	ResetFpga();
	puts("fine reset");
}

// Numero di cicli di clock di attesa tra il trigger e l'hold dei VA
void SetDelay(uint32_t delayIn){
	uint32_t regContent;
	ReadReg(rMSD_PARAM, &regContent);
	regContent = (regContent & 0xFFFF0000) | (delayIn & 0x0000ffff);
	write_register(rMSD_PARAM, &regContent);
}

//Configura la modalità: Stop(0), Run(1)
void SetMode(uint32_t modeIn){
	uint32_t regContent;

	if(modeIn == 0){
		regContent = 0x00000000;
	}
	else if(modeIn == 1){
		ResetFpga();
		regContent = 0x00000010;
	}

	write_register(rGOTO_STATE, &regContent);
}

//Cattura il valore del trigger counter interno ed esterno
void GetEventNumber(uint32_t * extTrigCount, uint32_t * intTrigCount){
	ReadReg(rEXT_TRG_COUNT, extTrigCount);
	ReadReg(rINT_TRG_COUNT, intTrigCount);
}

//Reset della logica FPGA
void EventReset(){
	ResetFpga();
}

//Configura sistema in modalità calibrazione
void Calibrate(uint32_t calibIn){
	uint32_t regContent;
	ReadReg(rTRIGBUSY_LOGIC, &regContent);
	regContent = (regContent & 0xFFFFFFFD) | (calibIn & 0x00000002);
	write_register(rTRIGBUSY_LOGIC, &regContent);
}

//Update the internal trigger period without changing the other configs
void intTriggerPeriod(uint32_t periodIn){
	uint32_t regContent;
	ReadReg(rTRIGBUSY_LOGIC, &regContent);
	regContent = (periodIn & 0xFFFFFFF0) | (regContent & 0x0000000F);
	write_register(rTRIGBUSY_LOGIC, &regContent);
}

//Enable/Disable the internal trigger
void selectTrigger(uint32_t intTrigIn){
	uint32_t regContent;
	ReadReg(rTRIGBUSY_LOGIC, &regContent);
	regContent = (regContent & 0xFFFFFFFE) | (intTrigIn & 0x00000001);
	write_register(rTRIGBUSY_LOGIC, &regContent);
}

//Configure and enable/disable the test unit
void configureTestUnit(uint32_t tuCfg){
	uint32_t regContent;
	ReadReg(rUNITS_EN, &regContent);
	regContent = (regContent & 0xFFFFFCFD) | (tuCfg & 0x00000302);
	write_register(rUNITS_EN, &regContent);
}

//manda un evento (pacchetto fast data fifo)
void GetEvent(int socket){

	pthread_t t;
	pthread_attr_t attr;
	int new_priority = 20;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_getschedparam(&attr, &param);
	param.sched_priority = new_priority;

	printf("socket: %d\n", socket);
	if(pthread_create(&t, &attr, &high_priority, &socket) < 0){

		perror("thread create");
	}
	pthread_join(t, 0);

}
