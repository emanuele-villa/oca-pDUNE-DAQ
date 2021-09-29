#ifndef DAQSERVER_H
#define DAQSERVER_H

#include <vector>

#include "tcpserver.h"
#include "de10_silicon_base.h"

class daqserver: public tcpserver {

private:
  std::vector<const char*> addressdet; 
  std::vector<int> portdet;
  std::vector<de10_silicon_base*> det;
  
public:
  ~daqserver();
  daqserver(int port, int verb=0);

  virtual void ProcessMsgReceived(char* msg);

  virtual void SetListDetectors(int nde10, const char* addressde10[], int portde10[]);
  
};

#endif
