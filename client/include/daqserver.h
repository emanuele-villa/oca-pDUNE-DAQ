#ifndef DAQSERVER_H
#define DAQSERVER_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <vector>

class daqserver {

public:
  ~daqserver();
  daqserver(int port);
  
};

#endif
