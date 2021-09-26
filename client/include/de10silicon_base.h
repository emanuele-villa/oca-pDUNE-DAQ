#ifndef DE10_SILICON_BASE_H
#define DE10_SILICON_BASE_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <vector>

class de10_silicon_base {

public:
  ~de10_silicon_base();
  de10_silicon_base(const char *address, int port);
  int client_send(const char *buffer);
  int client_receive();
  int client_receive_int();
  int client_socket;
  
private:
  int client_connect(const char *address, int port);
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
