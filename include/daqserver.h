#ifndef DAQSERVER_H
#define DAQSERVER_H

#include <vector>
#include <pthread.h>

#include "tcpserver.h"
#include "de10_silicon_base.h"

class daqserver: public tcpserver {

private:
  std::vector<const char*> addressdet;
  std::vector<int> portdet;
  std::vector<de10_silicon_base*> det;
  const char kdataPath[12] = "../../data/";
  volatile bool kStart;
  pthread_t threadStart;
  int calibmode;
  int trigtype;

  void ProcessCmdReceived(char* msg);
  
  int recordEvents(FILE* fd);
  
public:
  ~daqserver();
  daqserver(int port, int verb=0);

  void SetListDetectors(int nde10, const char* addressde10[], int portde10[], int detcmdlenght);

  void SetDetectorsCmdLenght(int detcmdlenght);

  void SetCalibrationMode(uint32_t mode);
  void SelectTrigger(uint32_t trig);

  void ReadAllRegs();
  
  int ReadReg(uint32_t regAddr);
  int Init();
  void* Start();
  void* Stop();

};

#endif
