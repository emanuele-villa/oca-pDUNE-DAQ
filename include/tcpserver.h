#ifndef TCPSERVER_H
#define TCPSERVER_H

class tcpserver {

protected:
  int kVerbosity;
  int cmdlenght;
  int kSocket;//FIX ME: this name is a shit. Change
  int kSock;
  volatile bool kListeningOn;

  void AcceptConnection();
  
public:
  virtual ~tcpserver();
  tcpserver(int port, int verb=0);

  void SetCmdLenght(int lenght) { cmdlenght = lenght; }

  void SetVerbosity(int verb){ kVerbosity = verb; }
  int GetVerbosity(){ return kVerbosity; }

  void StopListening();
  
  virtual void ListenCmd();
  virtual void ProcessCmdReceived(char* msg);
};

#endif
