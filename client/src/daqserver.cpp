#include "daqserver.h"
#include "utility.h"

daqserver::daqserver(int port, int verb):tcpserver(port, verb){
  if (verbosity>0){
    printf("%s) daqserver created\n", __METHOD_NAME__);
  }
  return;
}

daqserver::~daqserver(){
  if (verbosity>0) {
    printf("%s) destroying daqserver\n", __METHOD_NAME__);
  }
  return;
}

/*

  while(1){
    
    char msg[LEN];
    
    if(read(new_socket, msg, sizeof(msg)) < 0){
      perror("errore nella read\n");
    }
    else{
      if(strcmp(msg, "init") == 0){
	Init(new_socket);
      }
      if(strcmp(msg, "set delay") == 0){
	SetDelay(new_socket);
      }

      if(strcmp(msg, "get event") == 0){

	GetEvent(new_socket);
      }

      if(strcmp(msg, "set mode") == 0){

	SetMode(new_socket);
      }

      if(strcmp(msg, "get event number") == 0){

	GetEventNumber(new_socket);
      }

      if(strcmp(msg, "print all event number") == 0){

	PrintAllEventNumber(new_socket);
      }

      if(strcmp(msg, "event reset") == 0){

	EventReset(new_socket);
      }

      if(strcmp(msg, "OverWriteDelay") == 0){

	OverWriteDelay(new_socket);
      }

      if(strcmp(msg, "Calibrate") == 0){

	Calibrate(new_socket);
      }

      if(strcmp(msg, "WriteCalibPar") == 0){

	WriteCalibPar(new_socket);
      }

      if(strcmp(msg, "SaveCalibrations") == 0){

	SaveCalibrations(new_socket);
      }

      if(strcmp(msg, "intTriggerPeriod") == 0){

	intTriggerPeriod(new_socket);
      }

      if(strcmp(msg, "selectTrigger") == 0){

	selectTrigger(new_socket);
      }

      if(strcmp(msg, "configureTestUnit") == 0){

	configureTestUnit(new_socket);
      }
    }

    bzero(msg, sizeof(msg));

  }

*/
