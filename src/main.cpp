#include <signal.h>
#include <unistd.h>
#include <iostream>

#include "daqserver.h"
#include "utility.h"

// const int nde10 = 1;
// const char* addressde10[nde10] = {"localhost"};
// int portde10[nde10] = {80};
//const int nde10 = 1;
//const char* addressde10[nde10] = {"192.168.2.101"};
//int portde10[nde10] = {5000};
// const int nde10 = 0;
// const char* addressde10[nde10] = {};
// int portde10[nde10] = {};
// const int nde10 = 7;
// const char* addressde10[nde10] = {"192.168.2.101", "192.168.2.102", "192.168.2.103",
				  // "192.168.2.105", "192.168.2.106", "192.168.2.107",
				  // "192.168.2.108"
// };
// int portde10[nde10] = {5000, 5000, 5000, 5000, 5000, 5000, 5000
// };
const int nde10 = 1;
const char* addressde10[nde10] = {"192.168.1.8"};
int portde10[nde10] = {5000};

int verbosity=0;

daqserver* daqsrv = nullptr;
int ControlOn=1;
int StatusOn=0;

void PrintStatus(int dummy) {
  StatusOn=1;
  return;
}

void StopRun(int dummy) {
  ControlOn=0;
  daqsrv->StopListening();
  return;
}

int main(int argc, char *argv[]) {
  if (verbosity>0) {
    printf("--------------------------------\n");
    printf("OCA:\n");
    printf("--------------------------------\n");
  }
  std::cout<<"hash="<<GIT_HASH<<", time="<<COMPILE_TIME<<", branch="<<GIT_BRANCH<<std::endl;
  //this must be done before the `signal` otherwise for StopRun the daqsrv is still NULL
  daqsrv = new daqserver(8888, verbosity);
  daqsrv->SetCmdLenght(64);

  daqsrv->SetListDetectors(nde10, addressde10, portde10, 24);
  daqsrv->SetDetId("192.168.2.103", 302);
  daqsrv->SetPacketLen("192.168.2.103", 0x18A);

  daqsrv->Init();

  sleep(5);
  
  daqsrv->SetCalibrationMode(1);
  //  daqsrv->SetCalibrationMode(0);
  sleep(1);
  daqsrv->SelectTrigger(0);
  //daqsrv->SelectTrigger(1);
    daqsrv->ReadAllRegs();
    std::cout<<"----------Lettura registro 2----------"<<std::endl;
    daqsrv->ReadReg(2);
    std::cout<<"----------Lettura registro 2----------"<<std::endl;

  //is not really working, for now: it is killed as a standard CTRL-C
  //the param sent to StopRun is SIGTERM itself and we need that StopRun accepts a param even if cannot use it
  signal(SIGTERM,StopRun);//killing the PID of the process we call the function StopRun that exits the program in the right way
  signal(SIGINT,StopRun);// sending 'CTRL_C' the program exits in the right way
  signal(SIGQUIT,PrintStatus);//sending 'CTRL \' we print the numbers of taken events

  // quando lo usiamo cosi' lui sta in Listen perenne e aspetta dei comandi dal suo master per passarlo alle de10nano
  daqsrv->ListenCmd();

  // quando invece lo usiamo cosi' lui è master della acquisizione, non e' server di nulla e puo' essere utilizzato per mandare comandi in sequenza
  // la cosa migliore sarebbe fare un classe `daqmaster` che eredita da `daqserver` (o viceversa...) che non e' un server e non aspetta comandi da un master a lui superiore
  //  daqsrv->ReadReg(31);

  if (daqsrv) delete daqsrv;

  printf("%s) Exiting...\n", __METHOD_NAME__);

  return 0;
}
