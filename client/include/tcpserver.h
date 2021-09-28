#ifndef TCPSERVER_H
#define TCPSERVER_H

class tcpserver {

protected:
  int verbosity;
  int _socket;
  
public:
  ~tcpserver();
  tcpserver(int port, int verb=0);

  void SetVerbosity(int verb){ verbosity = verb; }
  int GetVerbosity(){ return verbosity; }
  
  virtual void Listen();
};

#endif
