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
  uint8_t  mode;
  uint32_t modeCfg;
  uint8_t  testUnitCfg;
  uint8_t  hkEn;
  uint8_t  testUnitEn;
  uint8_t  dataEn;
  uint32_t unitsEnCfg;
  uint32_t intTrigPeriod;
  uint32_t calEn;
  uint32_t intTrigEn;
  uint32_t pktLen;
  uint16_t feClkDuty;
  uint16_t feClkDiv;
  uint32_t feClkCfg;
  uint16_t adcClkDuty;
  uint16_t adcClkDiv;
  uint32_t adcClkCfg;
  uint32_t delay;
  
public:
  ~de10_silicon_base();
  de10_silicon_base(const char *address, int port, int verb=0);

  virtual void changeText(const std::string& new_text) {};
  virtual void sendData(std::vector<double> event) {};

  int readReg(int regAddr);
  int Init();
  int SetDelay(uint32_t delayIn);
  int SetMode(uint8_t modeIn);
  int GetEventNumber();
  char* PrintAllEventNumber();
  int EventReset();
  int GetEvent();
  int SetCalibrationMode(uint32_t calEnIn);
  int WriteCalibPar();
  int SaveCalibrations();
  int SetIntTriggerPeriod(uint32_t intTrigPeriodIn);
  int SelectTrigger(uint32_t intTrigEnIn);
  int ConfigureTestUnit(uint32_t testUnitEnIn);
};

#endif
