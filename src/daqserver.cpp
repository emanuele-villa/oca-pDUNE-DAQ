#include "daqserver.h"
#include "utility.h"
#include <sys/time.h>
//#include <TDatime.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctime>

daqserver* singletonDqSrv=NULL;
void _Start(char* runtype, uint32_t runnum, uint32_t unixtime){
  singletonDqSrv->Start(runtype, runnum, unixtime);
  return;
}

daqserver::daqserver(int port, int verb):tcpserver(port, verb){
  if (kVerbosity>0){
    printf("%s) daqserver created\n", __METHOD_NAME__);
  }

  addressdet.clear();
  portdet.clear();

  kStart=false;

  calibmode=0;
  mode=0;
  trigtype=0;
  
  if(singletonDqSrv==NULL) singletonDqSrv=this;
  else printf("%s) Class already exists\n", __METHOD_NAME__);
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
    det.push_back(new de10_silicon_base(addressde10[ii], portde10[ii], ii, detcmdlenght, kVerbosity));
  }

  return;
}

void daqserver::SetDetectorsCmdLenght(int detcmdlenght){

  for (int ii=0; ii<(int)(det.size()); ii++) {
    det[ii]->SetCmdLenght(detcmdlenght);
  }

  return;
}

void daqserver::SetCalibrationMode(uint32_t mode){

  calibmode = mode;
  
  for (int ii=0; ii<(int)(det.size()); ii++) {
    det[ii]->SetCalibrationMode(calibmode);
  }
  
  return;
}

void daqserver::SetMode(uint8_t _mode){

  mode = _mode;
  
  for (int ii=0; ii<(int)(det.size()); ii++) {
    det[ii]->SetMode(mode);
  }
  
  return;
}
  
void daqserver::SelectTrigger(uint32_t trig){

  trigtype = trig;
  
  for (int ii=0; ii<(int)(det.size()); ii++) {
    det[ii]->SelectTrigger(trigtype);
  }

  return;
}

void daqserver::ProcessCmdReceived(char* msg){

  if (kVerbosity>-1) {//FIX ME: >-1 perche' ora non fa niene, poi deve fare quelle sotto
    printf("%s) |%s| (lenght = %lu)\n", __METHOD_NAME__, msg, strlen(msg));
  }

  if(strstr(msg, "cmd=") != NULL) { //out commands: "cmd=xxxx"

    if (strcmp(msg, "cmd=Init") == 0){
      printf("%s) Init()\n", __METHOD_NAME__);
      Init();
      ReplyToCmd(msg);
      //FIX ME: only for now to test
      // for (int ii=0; ii<32; ii++) {
      // 	printf("%s) Reading reg %d\n", __METHOD_NAME__, ii);
      // 	ReadReg(ii);
      // }
    }
    else if (strcmp(msg, "cmd=Wait") == 0){//essentially for test
      printf("%s) Wait()\n", __METHOD_NAME__);
      printf("sleeping for 30s: "); fflush(stdout);
      for (int ii=0; ii<30; ii++) {
	printf("%d... ", ii); fflush(stdout);
	sleep(1);
      }
      printf("\n");
      ReplyToCmd(msg);
    }

  }
  else {//possibly a chinese command
    // command
    static const char* btcmd ="FF800008";
    static const char* start ="EE000001";
    static const char* stop  ="EE000000";
    static const char* beam  ="0001";
    static const char* cal   ="0000";

    static const int length=16;
    char command_string[2*length+1] = "";
    hex2string(msg, length, command_string);

    //    printf("%s\n", command_string);

    char cmdgroup[4][32] = {"", "", "", ""};
    for (int ii=0; ii<4; ii++) {
      strncpy(cmdgroup[ii], &command_string[ii*8], 8);
      if (kVerbosity>-1) {//FIX ME: should be >0
	printf("%s\n", cmdgroup[ii]);
      }
    }
    
    //check the command
    if (strcmp(btcmd, cmdgroup[0])==0) {//is a chinese command
      if (strcmp(start,cmdgroup[2])==0) {//start daq
	printf("%s) Start()\n", __METHOD_NAME__);
	char runtype[32] = "";
	strncpy(runtype, &cmdgroup[1][4], 4);
	char sruntype[32] = "";
	if (strcmp(beam,runtype)==0) {
	  sprintf(sruntype, "beam");
	  SetCalibrationMode(0);
	}
	else if (strcmp(cal,runtype)==0) {
	  sprintf(sruntype, "cal");
	  SetCalibrationMode(1);
	}
	else {
	  printf("%s) Not a valid run type %s\n", __METHOD_NAME__, runtype);
	  sprintf(sruntype, "????");
	}
	char srunnum[32] = "";
	strncpy(srunnum,  &cmdgroup[1][0], 4);
	uint32_t runnum = atoi(srunnum);
	char* ptr;
	uint32_t unixtime = strtol(cmdgroup[3], &ptr, 16);
	std::time_t t = unixtime;
	if (kVerbosity>-1) {//FIX ME: should be >0
	  printf("runtype=%s (-> %s), runnum=%s (%u), unixtime=%u (%s -> %s)\n", runtype, sruntype, srunnum, runnum, unixtime, cmdgroup[3], asctime(localtime(&t)));
	}
	//Spawn a thread to read events. Stop() will join the thread
	// if (pthread_create(&threadStart, NULL, _Start, (void*)0)) {
	//   printf("%s) Error creating thread", __METHOD_NAME__);
        // }
	// else {
	_3d = std::thread(&daqserver::Start, this, runtype, runnum, unixtime);
	ReplyToCmd(msg);
	//	}
      }
      else if(strcmp(stop,cmdgroup[2])==0) {//stop daq
	printf("%s) Stop()\n", __METHOD_NAME__);
	Stop();
	ReplyToCmd(msg);
      }
      else {
	printf("%s) not a valid sub-command: %s (%s)\n\n", __METHOD_NAME__, cmdgroup[2], command_string);
	sprintf(msg, "NOT-A-VALID-SUBCMD");
	ReplyToCmd(msg);
      }
    }
    else {
      printf("%s) not a valid command: %s\n\n", __METHOD_NAME__, command_string);
      sprintf(msg, "NOT-A-VALID-CMD");
      ReplyToCmd(msg);
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

void daqserver::ReadAllRegs(){

  for (int ii=0; ii<32; ii++) {
    printf("%s) Reading reg %d\n", __METHOD_NAME__, ii);
    ReadReg(ii);
  }

  return;
}

int daqserver::ReadReg(uint32_t regAddr) {
  int ret=0;
  uint32_t regCont=0; //FIX ME Shall be vector

  for (uint32_t ii=0; ii<det.size(); ii++) {
    ret |= (det.at(ii)->readReg(regAddr, regCont)<<ii);
    if (kVerbosity>-1) {
      printf("%s) Read from DE10 %d: %08x\n", __METHOD_NAME__, ii, regCont);
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


//Read the events from all of the DE10 and write them in binary to the .dat file
int daqserver::recordEvents(FILE* fd) {
  
  int readRet = 0;
  int writeRet = 0;
  //std::vector<uint32_t*> evts(det.size(), "");
  uint32_t evtLen = 0;
  uint32_t evtLen_tot = 0;
  std::vector<uint32_t> evt(652);
  
  //FIX ME: mandare prima il comando a tutte le DE10 e poi leggere pian piano

  for (uint32_t ii=0; ii<det.size(); ii++) {
    //ret += (det.at(ii)->GetEvent(evts[ii]));
    readRet += (det.at(ii)->GetEvent(evt, evtLen));
    evtLen_tot += evtLen;
    sleep(1);
    writeRet += fwrite(evt.data(), evtLen, 1, fd);
    if (kVerbosity>1) {
      printf("%s) Get event from DE10 %s\n", __METHOD_NAME__, addressdet[ii]);
      printf("  Bytes read: %d/%d\n", readRet, evtLen);
      printf("  Writes performed: %d/1\n", writeRet);

    }
  }

  //Everything is read and dumped to file
  if (evtLen_tot!=0) {
    if (readRet != (int)evtLen_tot || writeRet != (int)(det.size())) {
      printf("%s):\n", __METHOD_NAME__);
      printf("    Bytes read: %d/%u\n", readRet, evtLen_tot);
      printf("    Writes performed: %d/%d\n", writeRet, (int)(det.size()));
      return -1;
    }
  }
  else {
    if (kVerbosity>1) {
      printf("%s) total event lenght was 0\n", __METHOD_NAME__);
    }
  }
  return 0;
}

void daqserver::Start(char* runtype, uint32_t runnum, uint32_t unixtime) {
  //Open a file in the kdataPath folder and name it with UTC
  char dataFileName[255];
  // int runnum = time(NULL);
  //FIX ME:
  // - use the unixtime and runtype and build the agreed file name
  // - implement the file header
  sprintf(dataFileName,"%s/%d.dat", kdataPath, runnum);
  FILE* dataFileD;
  dataFileD = fopen(dataFileName,"w");
  if (dataFileD == NULL) {
    printf("%s) Error: file %s could not be created. Do the data dir %s exist?\n", __METHOD_NAME__, dataFileName, kdataPath);
    return;
  }

  SetMode(1);
  
  //Dump events to the file until Stop is received
  kStart = true;
  while(kStart) {
    recordEvents(dataFileD);
  }
  
  //Close the file and terminate thread
  fclose(dataFileD);
  if (kVerbosity > 0) printf("%s) File closed\n", __METHOD_NAME__);
}

void daqserver::Stop() {
  kStart = false;
  _3d.join();

  SetMode(0);
  sleep(10);

  //FIX ME: metterci un while che fa N GetEvent()
  
  if (kVerbosity > 0) printf("%s) Thread stopped succesfully\n", __METHOD_NAME__);
}
