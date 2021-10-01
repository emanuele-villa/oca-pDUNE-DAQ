#include "daqclient.h"
#include <stdio.h>

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

  daq->Send("init");
  char readBack[LEN]="";
  daq->Receive(readBack);
  printf("%s) Read from DAQ: %s\n", __METHOD_NAME__, readBack);

  return 0;
}
