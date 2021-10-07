#ifndef DE10_SILICON_BASE_H
#define DE10_SILICON_BASE_H

#include <string>
#include <vector>

#include "tcpclient.h"

class de10_silicon_base: public tcpclient {

private:
  //FIX ME: trasformare le variabili sotto in std::map in cui la key è un nome logico
  //(cosi' potremmo anche fare un metodo generico Set("pincopallino", ...)
  //e il value è una pair (indirizzo registro, contenuto)

  //Configuration variables
  uint32_t mode;
  uint32_t testUnitCfg;
  uint32_t hkEn;
  uint32_t testUnitEn;
  uint32_t dataEn;
  uint32_t intTrigPeriod;
  uint32_t calEn;
  uint32_t intTrigEn;
  uint32_t pktLen;
  uint32_t feClkDuty;
  uint32_t feClkDiv;
  uint32_t adcClkDuty;
  uint32_t adcClkDiv;
  uint32_t delay;
  uint32_t detId;
  const static uint32_t kevtLen = 652;

public:
  ~de10_silicon_base();
  de10_silicon_base(const char *address, int port, int verb=0);

  virtual void changeText(const std::string& new_text) {};
  virtual void sendData(std::vector<double> event) {};

  int readReg(int regAddr, uint32_t &regCont);
  int Init();
  int SetDelay(uint32_t delayIn);
  int SetMode(uint8_t modeIn);
  int GetEventNumber();
  char* PrintAllEventNumber();
  int EventReset();
  int GetEvent(uint32_t* evt);
  int SetCalibrationMode(uint32_t calEnIn);
  int WriteCalibPar();
  int SaveCalibrations();
  int SetIntTriggerPeriod(uint32_t intTrigPeriodIn);
  int SelectTrigger(uint32_t intTrigEnIn);
  int ConfigureTestUnit(uint32_t testUnitEnIn);
  inline static int getEvtLen(){return kevtLen;};
};

#endif
