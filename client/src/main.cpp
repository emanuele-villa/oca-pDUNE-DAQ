#include <signal.h>
#include <unistd.h>

#include "de10silicon_base.h"
#include "daqserver.h"

// const int nde10 = 1;
// const char* addressde10[nde10] = {"localhost"}; 
// const int portde10[nde10] = {80};
const int nde10 = 0;
const char* addressde10[nde10] = {}; 
const int portde10[nde10] = {};

int verbosity=0;

daqserver* daqsrv = NULL;
int ControlOn=1;
int StatusOn=0;

void PrintStatus(int dummy) {
  StatusOn=1;
  return;
}

void StopRun(int dummy) {
  ControlOn=0;
  return;
}

int main(int argc, char *argv[]) {
  if (verbosity>0) {
    printf("--------------------------------\n");
    printf("OCA:\n");
    printf("--------------------------------\n");
  }

  de10_silicon_base* det[nde10];
  for (int ii=0; ii<nde10; ii++) {
    det[ii] = new de10_silicon_base(addressde10[ii], portde10[ii], verbosity);
  }

  //this must be done before the `signal` otherwise for StopRun the daqsrv is still NULL
  daqsrv = new daqserver(9999, verbosity);

  //the param sent to StopRun is SIGTERM itself and we need that StopRun accepts a param even if cannot use it
  signal(SIGTERM,StopRun);//killing the PID of the process we call the function StopRun that exits the program in the right way
  signal(SIGINT,StopRun);// sending 'CTRL_C' the program exits in the right way
  signal(SIGQUIT,PrintStatus);//sending 'CTRL \' we print the numbers of taken events

  while (ControlOn){
    printf("%d\n", ControlOn);
    sleep(1);
    
    daqsrv->Listen();
  }

  if (daqsrv) delete daqsrv;
  
  return 0;
}
