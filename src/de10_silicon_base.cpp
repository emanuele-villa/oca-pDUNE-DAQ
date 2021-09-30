#include "de10_silicon_base.h"
#include "utility.h"

de10_silicon_base::de10_silicon_base(const char *address, int port, int verb):tcpclient(address, port, verb){
  //Initialize and compute configurations
  testUnitCfg = 0;
  hkEn = 0;
  ConfigureTestUnit(0);
  dataEn = 1;
  SetIntTriggerPeriod(0x02faf080);
  SetCalibrationMode(0);
  SelectTrigger(0);
  pktLen = 0x0000028A;
  feClkDuty  = 0x00000004;
  feClkDiv   = 0x00000028;
  adcClkDuty = 0x00000004;
  adcClkDiv  = 0x00000002;
  SetDelay(0x00000145);
  SetMode(0);
  detId = 0x000000E3;

  changeText("hello");
  if (verbosity>0) {
    printf("%s) de10 silicon created\n", __METHOD_NAME__);
  }
}

//--------------------------------------------------------------

de10_silicon_base::~de10_silicon_base(){
}

//--------------------------------------------------------------
int de10_silicon_base::readReg(int regAddr){
    client_send("readReg");
    char c[sizeof (uint32_t) * 8 + 1];
    sprintf(c, "%x", regAddr);
    client_send(c);
    client_receive();
    return 0;
}

//FIX ME: use the proper functions or the 2D array to retrieve configurations
int de10_silicon_base::Init() {
  //client_send("trigger -off\n");
  //client_send("write -x 040700\n");
  char c[sizeof (uint32_t) * 8 +1];
  uint32_t regContent = 1;

  if (verbosity>0) {
    printf("$s) [>>> initializing (reset everything)]\n");
  }
  client_send("init");
  client_receive();

  //Register 1
  regContent = (testUnitCfg&0x00000003) << 8 | (hkEn&0x00000001) << 6 \
                | testUnitEn | (dataEn&0x00000001);
  //char *c = (char *)&regContent;
  sprintf(c, "%x", regContent);
  client_send(c);
  changeText(c);
  bzero(c, sizeof(c));

  //Register 2
  regContent = intTrigPeriod|calEn|intTrigEn;
  //c = (char *)&regContent;
  sprintf(c, "%x", regContent);
  client_send(c);
  changeText(c);
  bzero(c, sizeof(c));

  //Register 3
  regContent = detId&0x000000FF;
  //c = (char *)&regContent;
  sprintf(c, "%x", regContent);
  client_send(c);
  changeText(c);
  bzero(c, sizeof(c));

  //Register 4
  regContent = pktLen;
  //c = (char *)&regContent;
  sprintf(c, "%x", regContent);
  client_send(c);
  changeText(c);
  bzero(c, sizeof(c));

  //Register 5
  regContent = ((feClkDuty&0x0000FFFF)<<16) | (feClkDiv&0x0000FFFF);
  //c = (char *)&regContent;
  sprintf(c, "%x", regContent);
  client_send(c);
  changeText(c);

  //Register 6
  regContent = ((adcClkDuty&0x0000FFFF)<<16) | (adcClkDiv&0x0000FFFF);
  //c = (char *)&regContent;
  sprintf(c, "%x", regContent);
  client_send(c);
  changeText(c);

  //Register 7
  reg_content  = delay;
  //c = (char *)&reg_content;
  sprintf(c, "%x", reg_content);
  client_send(c);
  changeText(c);

  return 0;
}

int de10_silicon_base::SetDelay(uint32_t delayIn){
  delay = (delayIn & 0x0000FFFF);
  client_send("set delay");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%x", delay);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::SetMode(uint8_t modeIn) {
  mode=(modeIn << 4)&0x00000010;
  client_send("set mode");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", mode);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::GetEventNumber() {
  //	printf("[>>> getting events]\n");
  client_send("get event number");
  client_receive();
  return 0;
}

//FIX ME: Shall not read event numbers from HPS, but print the ones already got
char* de10_silicon_base::PrintAllEventNumber() {
  static char numbers[1023]="";
  // snprintf(numbers, 1023, "Dampe %02d: %6d", selfaddress, 0);
  // printf("[>>>>>] dampe: %s\n", numbers);
  //client_send("print all event number");
  //client_receive();
  return numbers;
}

int de10_silicon_base::EventReset() {
  // client_send("write -x 020400\n");
  // client_send("write -x 040700\n");
  if (verbosity>0) {
    printf("%s) [>>>>>] resetting events (reinitialize)\n", __METHOD_NAME__);
  }
  client_send("event reset");
  client_receive();
  return 0;
}

int de10_silicon_base::GetEvent(){
  client_send("get event");
  //int ret = 1;
  //int i = 1;
  client_receive_int();
  return 0;
}

//TO DO: there will be another method, in future to really calibrate: put in cal mode, start the trigger, stop the calibration and let the system compute pedestals, sigmas, etc...
int de10_silicon_base::SetCalibrationMode(uint32_t calEnIn){
  calEn = (calEnIn&0x00000001)<<1;
  client_send("Calibrate");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", calEn);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::WriteCalibPar(){
  //client_send("WriteCalibPar");
  //client_receive();
  printf("%s) FIX ME: do not yet implemented in HPS\n", __METHOD_NAME__);
  return 0;
}

int de10_silicon_base::SaveCalibrations(){
  //client_send("SaveCalibrations");
  //client_receive();
  printf("%s) FIX ME: do not yet implemented in HPS\n", __METHOD_NAME__);
  return 0;
}

int de10_silicon_base::SetIntTriggerPeriod(uint32_t intTrigPeriodIn){
  intTrigPeriod = intTrigPeriodIn&0xFFFFFFF0;
  client_send("intTriggerPeriod");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%x", intTrigPeriod);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::SelectTrigger(uint32_t intTrigEnIn){
  intTrigEn = intTrigEnIn&0x00000001;
  client_send("selectTrigger");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", intTrigEn);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::ConfigureTestUnit(uint32_t testUnitEnIn){
  testUnitEn = (testUnitEnIn&0x00000001)<<1;
  client_send("configureTestUnit");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", testUnitEnIn);
  client_send(c);
  client_receive();
  return 0;
}
