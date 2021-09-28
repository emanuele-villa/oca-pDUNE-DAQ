#include "de10silicon_base.h"
#include "utility.h"

de10_silicon_base::de10_silicon_base(const char *address, int port, int verb):tcpclient(address, port, verb){
  //Initialize and compute configurations
  mode = 0;
  testUnitCfg = 0;
  hkEn = 0;
  testUnitEn = 0;
  dataEn = 1;
  intTrigPeriod = 0x02faf080;
  calEn = 0;
  intTrigEn = 0;
  pktLen = 0x0000028A;
  feClkDuty  = 0x0004;
  feClkDiv   = 0x0028;
  adcClkDuty = 0x0004;
  adcClkDiv  = 0x0002;
  delay = 0x00000145;
  modeCfg = (mode << 4)&0x00000010;
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
  client_send("set delay");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%x", delayIn);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::SetMode(uint32_t modeIn) {
  client_send("set mode");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", modeIn);
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

//FIX ME: There is no such method in the HPS; should it be the same as SetDelay?
int de10_silicon_base::OverWriteDelay(uint32_t delayIn){
  client_send("OverWriteDelay");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%x", delayIn);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::Calibrate(uint32_t calEnIn){
  client_send("Calibrate");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", calEnIn);
  client_send(c);
  client_receive();
  return 0;
}

//FIX ME: do not needed in HPS
int de10_silicon_base::WriteCalibPar(){
  //client_send("WriteCalibPar");
  //client_receive();
  return 0;
}

//FIX ME: do not needed in HPS
int de10_silicon_base::SaveCalibrations(){
  //client_send("SaveCalibrations");
  //client_receive();
  return 0;
}

int de10_silicon_base::intTriggerPeriod(uint32_t intTrigPeriodIn){
  client_send("intTriggerPeriod");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%x", intTrigPeriodIn);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::selectTrigger(uint32_t intTrigEnIn){
  client_send("selectTrigger");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", intTrigEnIn);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::configureTestUnit(uint32_t testUnitEnIn){
  client_send("configureTestUnit");
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%d", testUnitEnIn);
  client_send(c);
  client_receive();
  return 0;
}
