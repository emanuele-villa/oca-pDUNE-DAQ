#include "daqclient.h"
#include <stdio.h>

const char* addressdaq = "localhost"; 
const int portdaq = 9999;

int verbosity=0;

int main(int argc, char *argv[]) {

  if (verbosity>0) {
    printf("--------------------------------\n");
    printf("testOCA:\n");
    printf("--------------------------------\n");
  }
  
  daqclient* daq = new daqclient(addressdaq, portdaq, verbosity);

  daq->Send("init");
  daq->Receive();
  
  return 0;
}
