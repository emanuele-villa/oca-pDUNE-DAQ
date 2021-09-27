#ifndef DE10_SILICON_BASE_H
#define DE10_SILICON_BASE_H

#include <string>
#include <vector>

#include "tcpclient.h"

class de10_silicon_base: public tcpclient {

public:
  ~de10_silicon_base();
  de10_silicon_base(const char *address, int port, int verb=0);
  
public:
  virtual void changeText(const std::string& new_text) {};
  virtual void sendData(std::vector<double> event) {};

  int Init();
  int SetDelay();
  int SetMode();
  int GetEventNumber();
  char* PrintAllEventNumber(int log=1,int JLV1num=0);
  int EventReset();
  int GetEvent();
  int OverWriteDelay();
  int Calibrate();
  int WriteCalibPar();
  int SaveCalibrations();
  int intTriggerPeriod();
  int selectTrigger();
  int configureTestUnit();
};

#endif
