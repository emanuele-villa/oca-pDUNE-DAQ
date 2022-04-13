#ifndef DAQSERVER_H
#define DAQSERVER_H

#include <vector>
#include <thread>

#include "tcpServer.h"
#include "de10_silicon_base.h"

#include "daqConfig.h"
#include "paperoConfig.h"

extern daqConfig::configParams daqConf;

class daqserver: public tcpServer {

protected:
  int kCmdLen;    //!< Length of incoming TCP commands 

private:
  std::vector<const char*> addressdet;
  std::vector<int> portdet;
  std::vector<de10_silicon_base*> det;
  std::string kdataPath = "./data/"; //!< Data file path
  volatile bool kStart; //!< Start event recording
  std::thread _3d;//!< Thread handle
  int calibmode;  //!< Calibration enable
  int mode;       //!< '1': Run, '0': Stop
  int trigtype;   //!< Trigger Type: if '1', internal
  unsigned int nEvents = 0; //!< Acquired event number
  paperoConfig::vectorParam paperoConfVector; //!< Papero configuration vector
  
  /*!
    Send a reply to received commands
  */
  int ReplyToCmd(char* msg);
  
  /*!
    Printing the message received from the client(s)
  */
  void ProcessCmdReceived(char* msg);
  
  int recordEvents(FILE* fd);
  
public:
  ~daqserver();
  daqserver(int port, int verb, std::string paperoCfgPath);

  /*!
    Setup clients to configure the detectors
  */
  void SetUpConfigClients();

  /*!
    Define the length of the receiving commands
  */
  void SetCmdLenght(int lenght) {
    kCmdLen = lenght;
  }

  /*!
    Create lists for detector IP addresses, ports, and object addresses
  */
  void SetListDetectors();

  void SetDetId(const char* addressde10, uint32_t _detId);
  void SetPacketLen(const char* addressde10, uint32_t _pktLen);

  void SetDetectorsCmdLenght(int detcmdlenght);

  void SetCalibrationMode(uint32_t mode);
  void SetMode(uint8_t mode);
  void SelectTrigger(uint32_t trig);

  void ResetBoards();

  void ReadAllRegs();
  
  int ReadReg(uint32_t regAddr);
  int Init();
  void Start(char* runtype, uint32_t runnum, uint32_t unixtime);
  void Stop();

  /*!
    Receive commands and call the appropriate function
  */
  void ListenCmd();

};

#endif
