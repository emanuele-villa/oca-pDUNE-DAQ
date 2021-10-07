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

  for (uint32_t ii=0; ii<det.size(); ii++) {
    if (det.at(ii)) delete det.at(ii);
    if (kVerbosity>0) {
      printf("%s) destroying DE10 %d\n", __METHOD_NAME__, ii);
    }
  }
  
  return;
}

void daqserver::SetListDetectors(int nde10, const char* addressde10[], int portde10[], int detcmdlenght){

  addressdet.clear();
  portdet.clear();

  for (int ii=0; ii<nde10; ii++) {
    addressdet.push_back(addressde10[ii]);
    portdet.push_back(portde10[ii]);
    det.push_back(new de10_silicon_base(addressde10[ii], portde10[ii], kVerbosity));
  }

  SetDetectorsCmdLenght(detcmdlenght);
  
  return;
}

void daqserver::SetDetectorsCmdLenght(int detcmdlenght){

  for (int ii=0; ii<(int)(det.size()); ii++) {
    det[ii]->SetCmdLenght(detcmdlenght);
  }  

  return;
}

void daqserver::ProcessCmdReceived(char* msg){

  if (kVerbosity>-1) {//FIX ME: >-1 perche' ora non fa niene, poi deve fare quelle sotto
    printf("%s) |%s| (lenght = %lu)\n", __METHOD_NAME__, msg, strlen(msg));
  }

  if(strstr(msg, "cmd=") != NULL) { //out commands: "cmd=xxxx"
  
    if (strcmp(msg, "Init") == 0){
      Init();
      //FIX ME: only for now to test
      for (int ii=0; ii<32; ii++) {
	ReadReg(ii);
      }
    }
  }
  else {//possibly a chinese command
    // command
    static const char* start ="FF80000800000000EE00000100000000";
    static const char* stop  ="FF80000800000000EE00000000000000";
    
    static const int length=16;
    char* command_string=new char[2*length];
    hex2string(msg,length,command_string);
    
    //    printf("%s\n", command_string);

    //check the command
    if(strcmp(start,command_string)==0) {//start daq
      printf("%s) Start()\n", __METHOD_NAME__);
      //      Start();
    }
    else if(strcmp(stop,command_string)==0) {//stop daq
      printf("%s) Stop()\n", __METHOD_NAME__);
      //      Stop();
    }
    else {
      printf("%s) not a valid command: %s\n", __METHOD_NAME__, command_string);
    }
    
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

  return;
}
  
int daqserver::ReadReg(uint32_t regAddr) {
  int ret=0;
  uint32_t regCont=0; //FIX ME Shall be vector

  for (uint32_t ii=0; ii<det.size(); ii++) {
    ret |= (det.at(ii)->readReg(regAddr, regCont)<<ii);
    if (kVerbosity>0) {
      printf("%s) Read from DE10 %d: %x\n", __METHOD_NAME__, ii, regCont);
    }
  }
  return ret;
}

int daqserver::Init() {
  int ret = 0;
  
  for (uint32_t ii=0; ii<det.size(); ii++) {
    ret |= (det.at(ii)->Init())<<1;
    if (kVerbosity>0) {
      printf("%s) Init of DE10 %d\n", __METHOD_NAME__, ii);
    }
  }

  return ret;
}
