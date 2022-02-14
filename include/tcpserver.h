/*!
  @file tcpServer.h
  @brief TCP server class
  @author Mattia Barbanera (mattia.barbanera@infn.it)
  @author Matteo Duranti (matteo.duranti@infn.it)
*/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

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
    int kPort;      //!< Port
    struct sockaddr_in kAddr; //!< Address
    bool kBlocking;

    /*!
      Create, bind, and configure socket
    */
    void Setup();

    /*!
      Open connections after socket binding
    */
    void AcceptConnection();

    /*!
      Setup and AcceptConnections
    */
    void Start();

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
