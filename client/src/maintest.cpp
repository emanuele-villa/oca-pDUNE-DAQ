#include "daqclient.h"

const char* addressdaq = "localhost"; 
const int portdaq = 9999;

int main(int argc, char *argv[]) {

  printf("test\n");
  
  daqclient* daq = new daqclient(addressdaq, portdaq);
  
  return 0;
}
