#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "hwlib.h"

#include "utility.h"

#include "lowlevelDriversFPGA.h"
#include "highlevelDriversFPGA.h"
#include "server.h"

// Reset della logica FPGA
void ResetFpga(){
	uint32_t data[4096];
	int flushErr=0;
	//Set to high the regArray bits of reset
	singleWriteReg((uint32_t)rGOTO_STATE, 0x00000003);

	//Flush the FastData Fifo
	//!@todo Flush also the hk
	flushErr = ReadFifoBurst(DATA_FIFO, data, 0, true);
	printf("%s) Flushed %d words\n", __METHOD_NAME__, flushErr);

	//Remove regArray reset
	singleWriteReg((uint32_t)rGOTO_STATE, 0x00000000);
}

//Inizializza l'array di registri e resetta la logica FPGA
void Init(uint32_t * regsContentIn, uint32_t opLen){
	//Configure the whole regArray (except register rGOTO_STATE)
	writeReg(regsContentIn, opLen);

	//Reset the FPGA
	ResetFpga();
}

// Numero di cicli di clock di attesa tra il trigger e l'hold dei VA
void SetDelay(uint32_t delayIn){
	uint32_t regContent;
	ReadReg(rMSD_PARAM, &regContent);
	regContent = (regContent & 0xFFFF0000) | (delayIn & 0x0000ffff);
	singleWriteReg(rMSD_PARAM, regContent);
}

//Configura la modalità: Stop(0), Run(1)
void SetMode(uint32_t modeIn){
  singleWriteReg(rGOTO_STATE, modeIn);
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
	singleWriteReg(rTRIGBUSY_LOGIC, regContent);
}

//Update the internal trigger period without changing the other configs
void intTriggerPeriod(uint32_t periodIn){
	uint32_t regContent;
	ReadReg(rTRIGBUSY_LOGIC, &regContent);
	regContent = (periodIn & 0xFFFFFFF0) | (regContent & 0x0000000F);
	singleWriteReg(rTRIGBUSY_LOGIC, regContent);
}

//Enable/Disable the internal trigger
void selectTrigger(uint32_t intTrigIn){
	uint32_t regContent;
	ReadReg(rTRIGBUSY_LOGIC, &regContent);
	regContent = (regContent & 0xFFFFFFFE) | (intTrigIn & 0x00000001);
	singleWriteReg(rTRIGBUSY_LOGIC, regContent);
}

//Configure and enable/disable the test unit
void configureTestUnit(uint32_t tuCfg){
	uint32_t regContent;
	ReadReg(rUNITS_EN, &regContent);
	regContent = (regContent & 0xFFFFFCFD) | (tuCfg & 0x00000302);
	singleWriteReg(rUNITS_EN, regContent);
}

//Receive one event from the FastDATA FIFO
int getEvent(std::vector<uint32_t>& evt, int* evtLen){

  int readErr;
  uint32_t pktLen;

  uint32_t valueRead=0;  //First value in output of the FIFO
  uint32_t fifoLevel=0;  //FIFO used words
  uint32_t fifoFull=0;   //Full flag
  uint32_t fifoEmpty=0;  //Empty flag
  uint32_t fifoAFull=0;  //Almost-Full flag
  uint32_t fifoAEmpty=0; //Almost-Empty flag
  uint32_t aFullThr=0;   //Almost-Full threshold
  uint32_t aEmptyThr=0;  //Almost-Empty threshold

  //Read the status of the FIFO and return if almost-empty
  readErr = StatusFifo(DATA_FIFO, &fifoLevel, &fifoFull, &fifoEmpty, &fifoAFull, &fifoAEmpty, &aFullThr, &aEmptyThr);
  ShowStatusFifo(DATA_FIFO);
  //  printf("%u\n", fifoLevel);
  if (fifoEmpty==1){
    *evtLen = 0;
    evt.clear();
    if (baseAddr.verbose>2) {
      printf("Fifo Empty. \n");
    }
    return 0;
  }

  //Read the first word and make sure it's the SoP
  readErr = ReadFifo(DATA_FIFO, &valueRead);
  if(valueRead != DATA_SOP){
    fprintf(stderr, "First value of event not SoP: %08x\n", valueRead);
    return -1;
  }

  //Read the packet length
  readErr = ReadFifo(DATA_FIFO, &pktLen);

  uint32_t packet[pktLen + 1];
  packet[0] = DATA_SOP;
  packet[1] = pktLen;
  readErr = ReadFifoBurst(DATA_FIFO, packet + 2, pktLen - 1, false);
  if (readErr < 0){
    fprintf(stderr, "Error in reading event\n");
    return -1;
  }

  if (baseAddr.verbose > 1){
    printf("Event:\n");
    for(uint32_t i = 0; i < pktLen+1; i++){
      printf("%08x\n", packet[i]);
    }
  }

  *evtLen = pktLen;
  evt.resize(pktLen+1);
  memcpy(evt.data(), packet, pktLen+1);
  
  return 0;
}
