#include "daqclient.h"
#include <stdio.h>
#include <unistd.h>

#include "utility.h"

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

  char readBack[LEN]="";
  
  daq->Send("init");
  // daq->Receive(readBack);//FIX ME: now is blocking.
  // printf("%s) Read from DAQ: %s\n", __METHOD_NAME__, readBack);

  sleep(3);
  
  daq->Send("fava");
  // daq->Receive(readBack);
  // printf("%s) Read from DAQ: %s\n", __METHOD_NAME__, readBack);
  
  return 0;
}
