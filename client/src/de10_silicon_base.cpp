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
  feClkDuty  = 0x0004;
  feClkDiv   = 0x0028;
  adcClkDuty = 0x0004;
  adcClkDiv  = 0x0002;
  SetDelay(0x00000145);
  SetMode(0);
  unitsEnCfg = (testUnitCfg&0x00000003) << 8 | (hkEn&0x00000001) << 6 \
                | (testUnitEn&0x00000001) << 1 | (dataEn&0x00000001);
  feClkCfg   = feClkDuty <<16 | feClkDiv;
  adcClkCfg  = adcClkDuty <<16 | adcClkDiv;

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

int de10_silicon_base::Init() {
  //client_send("trigger -off\n");
  //client_send("write -x 040700\n");
  if (verbosity>0) {
    printf("$s) [>>> initializing (reset everything)]\n");
  }
  client_send("init");
  client_receive();
  uint32_t reg_content = 1;
  //char *c = (char *)&reg_content;
  char c[sizeof (uint32_t) * 8 +1];
  sprintf(c, "%x", reg_content);
  client_send(c);
  changeText(c);
  bzero(c, sizeof(c));
  reg_content = 0x02faf080;
  //c = (char *)&reg_content;
  sprintf(c, "%x", reg_content);
  client_send(c);
  changeText(c);
  bzero(c, sizeof(c));
  reg_content = 0x000000ff;
  //c = (char *)&reg_content;
  sprintf(c, "%x", reg_content);
  client_send(c);
  changeText(c);
  bzero(c, sizeof(c));
  reg_content = 0x0000028a;
  //c = (char *)&reg_content;
  sprintf(c, "%x", reg_content);
  client_send(c);
  changeText(c);
  bzero(c, sizeof(c));
  reg_content = 0x00040028;
  //c = (char *)&reg_content;
  sprintf(c, "%x", reg_content);
  client_send(c);
  changeText(c);
  reg_content = 0x00040002;
  //c = (char *)&reg_content;
  sprintf(c, "%x", reg_content);
  client_send(c);
  changeText(c);
  reg_content  = 0x00070145;

  //c = (char *)&reg_content;
  sprintf(c, "%x", reg_content);
  client_send(c);
  changeText(c);

  return 0;
}

int de10_silicon_base::SetDelay(uint32_t delayIn){
  delay = delayIn;
  client_send("set delay");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%x", delay);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::SetMode(uint8_t modeIn) {
  mode=modeIn;
  modeCfg = (mode << 4)&0x00000010;
  client_send("set mode");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", modeCfg);
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
  calEn = calEnIn;
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
  intTrigPeriod = intTrigPeriodIn;
  client_send("intTriggerPeriod");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%x", intTrigPeriod);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::SelectTrigger(uint32_t intTrigEnIn){
  intTrigEn = intTrigEnIn;
  client_send("selectTrigger");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", intTrigEn);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::ConfigureTestUnit(uint32_t testUnitEnIn){
  testUnitEn = testUnitEnIn;
  client_send("configureTestUnit");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", testUnitEnIn);
  client_send(c);
  client_receive();
  return 0;
}
