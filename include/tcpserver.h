/*!
  @file tcpServer.h
  @brief TCP server class
  @author Mattia Barbanera (mattia.barbanera@infn.it)
  @author Matteo Duranti (matteo.duranti@infn.it)
*/

#ifndef TCPSERVER_H
#define TCPSERVER_H

/*!
  TCP server class
*/
class tcpServer {

  protected:
    int kVerbosity; //!< Verbosity level
    int kCmdLen;    //!< Length of incoming TCP commands 
    int kSockDesc;  //!< Socket descriptor before opening connections
    int kTcpConn;   //!< Accepted and open TCP connection
    volatile bool kListeningOn; //!< Turn on/off the listenings of commands

    /*!
      Open connections after socket binding
    */
    void AcceptConnection();

    /*!
      Printing the message received from the client(s)
    */
    virtual void ProcessCmdReceived(char* msg);

    /*!
      Send a reply to received commands
    */
    int ReplyToCmd(char* msg);

  public:
    tcpServer(int port, int verb=0);
    virtual ~tcpServer();

    void SetVerbosity(int verb){ kVerbosity = verb; }

    int GetVerbosity(){ return kVerbosity; }

    /*!
      Define the length of the receiving commands
    */
    void SetCmdLenght(int lenght) {
      kCmdLen = lenght;
    }

    /*!
      Stop incoming connections
    */
    void StopListening();

    /*!
      Receive commands and call the appropriate function
    */
    virtual void ListenCmd();
};

#endif
