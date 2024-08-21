#include "daqclient.h"
#include <stdio.h>
#include <unistd.h>

#include <ctime>
#include <iostream>

#include "utility.h"

const char* addressdaq = "localhost";
const int portdaq = 10000;

int verbosity=0;

int main(int argc, char *argv[]) {
  //  printf("%d\n", argc);
  if (argc < 2) {
    printf("Usage:\n\t%s <0: calibration, 1: beam> [runnum]\n", argv[0]);
    return 0;
  }
  int beam = atoi(argv[1]);
  unsigned int runnum = 21;
  if (argc == 3) {
    runnum = atoi(argv[2]);
    printf("%d\n", runnum);
  }

  //------------------------------------------------
  char readBack[LEN]="";
  static const int length=16;
  char* command_string=new char[2*length+1];
  //------------------------------------------------

  if (verbosity>0) {
    printf("--------------------------------\n");
    printf("startOCA:\n");
    printf("--------------------------------\n");
  }
  
  daqclient* daq = new daqclient(addressdaq, portdaq, verbosity);
  daq->SetCmdLenght(32);

  std::time_t timeNow = std::time(nullptr);
  uint32_t ts = (uint32_t)timeNow;
  uint32_t tsReord = (ts & 0x000000FF) << 24 | (ts & 0x0000FF00) << 8 | (ts & 0x00FF0000) >> 8 | (ts & 0xFF000000) >> 24;
  //std::cout << std::hex << timeNow << ": ts " << ts << " reordered " << tsReord << std::endl;

  std::cout << std::asctime(std::gmtime(&timeNow))
            << timeNow << " seconds since the Epoch\n";

  //71616b23
  //uint32_t start[4] = {0x080080FF, 0x01001500, 0x010000EE, 0x236B6171};
  uint32_t start[4] = {0x080080FF, runnum << 8, 0x010000EE, tsReord};
  start[1] = start[1] | ((beam & 0x1) << 25);
  daq->Send((void*)start, 4*sizeof(uint32_t));
  daq->ReceiveCmdReply(readBack);//is blocking and this is wanted
  hex2string(readBack,length,command_string);
  printf("%s) Read from DAQ: %s\n", __METHOD_NAME__, command_string);
      
  return 0;
}
