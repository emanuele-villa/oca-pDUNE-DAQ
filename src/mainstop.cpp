#include "daqclient.h"
#include <stdio.h>
#include <unistd.h>

#include "utility.h"

const char* addressdaq = "localhost";
const int portdaq = 8888;

int verbosity=0;

int main(int argc, char *argv[]) {


  //------------------------------------------------
  char readBack[LEN]="";
  static const int length=16;
  char* command_string=new char[2*length+1];
  //------------------------------------------------
  
  if (verbosity>0) {
    printf("--------------------------------\n");
    printf("stopOCA:\n");
    printf("--------------------------------\n");
  }
  
  daqclient* daq = new daqclient(addressdaq, portdaq, verbosity);
  daq->SetCmdLenght(32);
  
  uint32_t stop[4] = {0x080080FF, 0x01001500, 0x000000EE, 0x5B8D6161};
  daq->Send((void*)stop, 4*sizeof(uint32_t));
  daq->ReceiveCmdReply(readBack);//is blocking and this is wanted
  hex2string(readBack,length,command_string);
  printf("%s) Read from DAQ: %s\n", __METHOD_NAME__, command_string);
  
  return 0;
}
