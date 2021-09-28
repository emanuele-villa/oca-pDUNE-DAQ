#ifndef DAQCLIENT_H
#define DAQCLIENT_H

#include "tcpclient.h"

class daqclient: public tcpclient {

public:
  ~daqclient();
  daqclient(const char *address, int port, int verb=0);

};

#endif
