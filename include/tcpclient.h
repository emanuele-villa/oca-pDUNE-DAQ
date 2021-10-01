#ifndef TCPCLIENT_H
#define TCPCLIENT_H

class tcpclient {

protected:
  int verbosity;

public:
  virtual ~tcpclient();
  tcpclient(const char *address, int port, int verb=0);

  void SetVerbosity(int verb){ verbosity = verb; }
  int GetVerbosity(){ return verbosity; }

  int Send(const char *buffer) { return client_send(buffer); }
  int Receive(char* msg);

protected:
  int client_connect(const char *address, int port);
  int client_send(const char *buffer);
  int client_receive(char* msg);
  int client_receive_int();
  int client_socket;

};

#endif
