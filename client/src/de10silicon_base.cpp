#include "de10silicon_base.h"
#include "utility.h"

de10_silicon_base::de10_silicon_base(const char *address, int port, int verb):tcpclient(address, port, verb){
  changeText("hello");
  if (verbosity>0) {
    printf("%s) de10 silicon created\n", __METHOD_NAME__);
  }
}

//--------------------------------------------------------------

de10_silicon_base::~de10_silicon_base(){
}

//--------------------------------------------------------------

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

int de10_silicon_base::SetDelay(){
  client_send("set delay");
  uint32_t delay = 5;
  char c[sizeof (uint32_t) * 8 + 1];
  sprintf(c, "%x", delay);
  client_send(c);
  client_receive();
  return 0;
}

int de10_silicon_base::SetMode() {
  client_send("set mode");
  uint32_t mode = 3;
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

char* de10_silicon_base::PrintAllEventNumber(int log,int JLV1num) {
  int ret=0;
  static char numbers[1023]="";
  // snprintf(numbers, 1023, "Dampe %02d: %6d", selfaddress, 0);
  // printf("[>>>>>] dampe: %s\n", numbers);
  client_send("print all event number");
  client_receive();
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
  int ret = 1;
  int i = 1;
  client_receive_int();
  return 0;
}

int de10_silicon_base::OverWriteDelay(){
  client_send("OverWriteDelay");
  client_receive();
  return 0;
}

int de10_silicon_base::Calibrate(){
  client_send("Calibrate");
  client_receive();
  return 0;
}

int de10_silicon_base::WriteCalibPar(){
  client_send("WriteCalibPar");
  client_receive();
  return 0;
}

int de10_silicon_base::SaveCalibrations(){
  client_send("SaveCalibrations");
  client_receive();
  return 0;
}

int de10_silicon_base::intTriggerPeriod(){
  client_send("intTriggerPeriod");
  client_receive();
  return 0;
}

int de10_silicon_base::selectTrigger(){
  client_send("selectTrigger");
  client_receive();
  return 0;
}

int de10_silicon_base::configureTestUnit(){
  client_send("configureTestUnit");
  client_receive();
  return 0;
}
