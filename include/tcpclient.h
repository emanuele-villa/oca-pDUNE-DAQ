#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "stdint.h"
#include <cerrno>
#include <system_error>
#include <iostream>

class tcpclient {

protected:
  int verbosity;
  int cmdlenght;
  int client_socket;

public:
  virtual ~tcpclient();
  tcpclient(const char *address, int port, int verb=0);

  virtual void SetCmdLenght(int lenght) { cmdlenght=lenght; }//in number of char

  void SetVerbosity(int verb){ verbosity = verb; }
  int GetVerbosity(){ return verbosity; }

  int Send(const char* buffer);
  int Send(void* buffer, int bytesize);
  int SendCmd(const char* buffer);
  int SendInt(uint32_t par);
  
  int Receive(char* buffer, int bytesize);//the lenght must be know
  int Receive(void* buffer, int bytesize);
  int ReceiveCmdReply(char* buffer);
  int ReceiveInt(uint32_t& par);

protected:
  int client_connect(const char* address, int port);
  int client_send(void* buffer, int bytesize);
  int client_receive(void* buffer, int bytesize);

};

#endif
