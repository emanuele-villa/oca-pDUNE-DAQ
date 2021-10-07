#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "stdint.h"

class tcpclient {

protected:
  int verbosity;
  int cmdlenght;

public:
  virtual ~tcpclient();
  tcpclient(const char *address, int port, int verb=0);

  void SetCmdLenght(int lenght) { cmdlenght=lenght; }//in number of char

  void SetVerbosity(int verb){ verbosity = verb; }
  int GetVerbosity(){ return verbosity; }

  int Send(const char *buffer);
  int Send(void* buffer, int bytesize);
  int SendCmd(const char *buffer);
  int SendInt(uint32_t par);
  int Receive(char* msg);

protected:
  int client_connect(const char *address, int port);
  int client_send(const char* buffer);
  int client_send(void* buffer, int bytesize);
  int client_receive(void* msg, int lentoread = 256);//FIX ME: questo default è per come faceva
  int client_socket;

};

#endif
