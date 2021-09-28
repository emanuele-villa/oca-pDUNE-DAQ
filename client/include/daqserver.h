#ifndef DAQSERVER_H
#define DAQSERVER_H

#include "tcpserver.h"

class daqserver: public tcpserver {
  
public:
  ~daqserver();
  daqserver(int port, int verb=0);

  virtual void ProcessMsgReceived(char* msg);
};

#endif
