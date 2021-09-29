#ifndef TCPSERVER_H
#define TCPSERVER_H

class tcpserver {

protected:
  int kVerbosity;
  int kSocket;
  volatile bool kListeningOn;
  
public:
  ~tcpserver();
  tcpserver(int port, int verb=0);

  void SetVerbosity(int verb){ kVerbosity = verb; }
  int GetVerbosity(){ return kVerbosity; }

  void StopListening();
  
  virtual void Listen();
  virtual void ProcessMsgReceived(char* msg);
};

#endif
