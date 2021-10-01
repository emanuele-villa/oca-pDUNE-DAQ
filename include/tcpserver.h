#ifndef TCPSERVER_H
#define TCPSERVER_H

class tcpserver {

protected:
  int kVerbosity;
  int kSocket;//FIX ME: this name is a shit. Change
  int kSock;
  volatile bool kListeningOn;

  void AcceptConnection();
  
public:
  virtual ~tcpserver();
  tcpserver(int port, int verb=0);

  void SetVerbosity(int verb){ kVerbosity = verb; }
  int GetVerbosity(){ return kVerbosity; }

  void StopListening();
  
  virtual void Listen();
  virtual void ProcessMsgReceived(char* msg);
};

#endif
