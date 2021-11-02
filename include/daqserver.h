#ifndef DAQSERVER_H
#define DAQSERVER_H

#include <vector>
#include <thread>

#include "tcpserver.h"
#include "de10_silicon_base.h"

class daqserver: public tcpserver {

private:
  std::vector<const char*> addressdet;
  std::vector<int> portdet;
  std::vector<de10_silicon_base*> det;
  const char kdataPath[12] = "./data/";
  volatile bool kStart;
  std::thread _3d;
  int calibmode;
  int mode;
  int trigtype;
  unsigned int nEvents = 0;
  
  void ProcessCmdReceived(char* msg);
  
  int recordEvents(FILE* fd);
  
public:
  ~daqserver();
  daqserver(int port, int verb=0);

  void SetListDetectors(int nde10, const char* addressde10[], int portde10[], int detcmdlenght);
  void SetDetId(const char* addressde10, uint32_t _detId);
  void SetPacketLen(const char* addressde10, uint32_t _pktLen);

  void SetDetectorsCmdLenght(int detcmdlenght);

  void SetCalibrationMode(uint32_t mode);
  void SetMode(uint8_t mode);
  void SelectTrigger(uint32_t trig);

  void ResetBoards();

  void ReadAllRegs();
  
  int ReadReg(uint32_t regAddr);
  int Init();
  void Start(char* runtype, uint32_t runnum, uint32_t unixtime);
  void Stop();

};

#endif
