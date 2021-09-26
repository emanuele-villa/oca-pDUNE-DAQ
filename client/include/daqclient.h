#ifndef DAQCLIENT_H
#define DAQCLIENT_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <vector>

class daqclient {

public:
  ~daqclient();
  daqclient(const char *address, int port);
  int client_send(const char *buffer);
  int client_receive();
  int client_receive_int();
  int client_socket;
  
private:
  int client_connect(const char *address, int port);

};

#endif
