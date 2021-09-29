#include "daqserver.h"
#include "utility.h"

daqserver::daqserver(int port, int verb):tcpserver(port, verb){
  if (kVerbosity>0){
    printf("%s) daqserver created\n", __METHOD_NAME__);
  }

  addressdet.clear(); 
  portdet.clear();
  
  return;
}

daqserver::~daqserver(){
  if (kVerbosity>0) {
    printf("%s) destroying daqserver\n", __METHOD_NAME__);
  }
  return;
}

void daqserver::SetListDetectors(int nde10, const char* addressde10[], int portde10[]){

  addressdet.clear(); 
  portdet.clear();

  for (int ii=0; ii<nde10; ii++) {
    addressdet.push_back(addressde10[ii]);
    portdet.push_back(portde10[ii]);
    det.push_back(new de10_silicon_base(addressde10[ii], portde10[ii], kVerbosity));
  }
  
  return;
}

void daqserver::ProcessMsgReceived(char* msg){

  if (kVerbosity>-1) {//FIX ME: >-1 perche' ora non fa niene, poi deve fare quelle sotto
    printf("%s) %s\n", __METHOD_NAME__, msg);
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
