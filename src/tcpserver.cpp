/*!
  @file tcpServer.cpp
  @brief TCP server management, tx, and rx function definitions
  @author Niccolò De Paolis (niccolo.depaolis@studenti.unipg.it)
  @author Mattia Barbanera (mattia.barbanera@infn.it)
  @author Matteo Duranti (matteo.duranti@infn.it)
*/
#include "tcpServer.h"
#include "utility.h"

tcpServer::tcpServer(int port, int verb){
  kVerbosity            = verb;
  kListeningOn          = false;
  kCmdLen               = 8;
  kTcpConn              = -1;
  kSockDesc             = -1;
  kPort                 = port;
  kBlocking             = false;
  
  //Reset kAddr to intended values
  memset(&kAddr, 0, sizeof(kAddr));
  kAddr.sin_family      = AF_INET;
  kAddr.sin_port        = htons(kPort);
  kAddr.sin_addr.s_addr = INADDR_ANY;

  Start();
}

tcpServer::~tcpServer(){
  if (kVerbosity>0) {
    printf("%s) Destroying tcpServer\n", __METHOD_NAME__);
  }
  kListeningOn = false;
  close(kSockDesc);
  shutdown(kTcpConn, SHUT_RDWR);
  
  return;
}

void tcpServer::Setup(){
  int optval = 1;

  exit_if(kListeningOn == true, "%s) Socket already setup", __METHOD_NAME__);

  //Create a new socket
  kSockDesc = socket(AF_INET , SOCK_STREAM , 0);
  exit_if(kSockDesc < 0, "%s) Socket creation error", __METHOD_NAME__);

  //Set socket options
  //SO_REUSEADDR to avoid waiting for addresses already in use
  if (setsockopt(kSockDesc, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }
  
  //Binding to port and address
  int result = bind(kSockDesc, (struct sockaddr *) &kAddr,
                  sizeof(kAddr));
  exit_if(result < 0, "%s) Bind error", __METHOD_NAME__);

  if (kVerbosity>0) {
    printf("%s) Correct bind...\n", __METHOD_NAME__);
  }
  fflush(stdout);

  //Listen for new connections
  result = listen(kSockDesc, 100);//Matteo D.
  exit_if(result < 0, "%s) Listening not possibile", __METHOD_NAME__);

  kListeningOn = true;

  return;
}

void tcpServer::AcceptConnection(){
  struct sockaddr_in clientAddr;
  socklen_t addrLen = sizeof(clientAddr);
  int flags, retflags;

  sockaddr_in peerAddr;
  socklen_t peerAddrLen = sizeof(peerAddr);

  //Close open connections, if any
  if (kTcpConn!=-1) {
    shutdown(kTcpConn, SHUT_RDWR);
    kTcpConn = -1;
  }

  if (!kBlocking) {
    //Save the current flags, if any
    flags = fcntl(kSockDesc ,F_GETFL, 0);
    exit_if(flags < 0, "%s) Fcntl failed:", __METHOD_NAME__);
    //Make the socket non-blocking
    retflags=fcntl(kSockDesc, F_SETFL, flags | O_NONBLOCK);
    exit_if(retflags < 0, "%s) Fcntl failed:", __METHOD_NAME__);
  }

  //Stop here until a connection exists
  while (kTcpConn==-1) {
    
    if (kVerbosity>1) {
      printf("%s) Waiting for connections...\n", __METHOD_NAME__);
    }
    
    //Await (blocking) or try (non-blocking) connections
    kTcpConn = accept(kSockDesc, (struct sockaddr *) &clientAddr, &addrLen);
    if (kVerbosity>1) {
      printf("%s) kTcpConn: %d, errno: %d (EAGAIN, EWOULDBLOCK: %d %d)\n",
                __METHOD_NAME__, kTcpConn, errno, EAGAIN, EWOULDBLOCK);
    }
    
    //Return if server is closing
    if (kVerbosity>2) {
      printf("%s) %d\n", __METHOD_NAME__, kListeningOn);
    }
    if (!kListeningOn){
      return;
    }
    
    //If known errors occur, retry; otherwise, exit
    if (errno == EAGAIN || errno == EWOULDBLOCK){
      usleep(100);
      continue;
    } else {
      exit_if(kTcpConn<0, "%s) Negotiation error:", __METHOD_NAME__);
    }
    
    //Setting up of the connections, not in a rush
    sleep(1);
  }
  
  //FIXME Return to blocking socket?
  if (!kBlocking) {
    //Update the connection flags to make it non-blocking
    flags=fcntl(kTcpConn ,F_GETFL, 0);
    exit_if(flags < 0, "%s) Fcntl failed:", __METHOD_NAME__);
    retflags=fcntl(kTcpConn, F_SETFL, flags | O_NONBLOCK);
    exit_if(retflags < 0, "%s) Fcntl failed:", __METHOD_NAME__);
  }
  
  //
  getpeername(kTcpConn, (sockaddr *) &peerAddr, &peerAddrLen);
  printf("%s) Connection succeded: (socket number %d, %s:%d)\n", 
            __METHOD_NAME__, kTcpConn, inet_ntoa(peerAddr.sin_addr),
            ntohs(peerAddr.sin_port));

  return;
}

void tcpServer::Start(){
  Setup();
  AcceptConnection();

  return;
}

void tcpServer::ListenCmd(){

  kListeningOn = true;

  while (kListeningOn){
  
    char msg[LEN];

    //Receive a command of kCmdLen numbers chars (each one in ASCII char),
    //+ 1 for the termination character
    ssize_t readret = read(kTcpConn, msg, ((kCmdLen*8)*sizeof(char)+1));

    if (readret < 0){
      //Error
      if (EAGAIN == errno || EWOULDBLOCK == errno) {
        if (kVerbosity>1) {
          printf("%s) There's nothing to read now; try again later\n", __METHOD_NAME__);
        }
      }
      else {
        print_error("%s) Read error: \n", __METHOD_NAME__);
      }
    }
    else if (readret==0){
      //Stream is over and client disconnected: wait for another connection
      AcceptConnection();
    }
    else {
      //RX ok
      ProcessCmdReceived(msg);
    }

    bzero(msg, sizeof(msg));
  }

  if (kVerbosity>0) {
    printf("%s) Stop Listening\n", __METHOD_NAME__);
  }

  return;
}

void tcpServer::ProcessCmdReceived(char* msg){

  if (kVerbosity>0) {
    printf("%s) %s\n", __METHOD_NAME__, msg);
  }

  return;
}

void tcpServer::StopListening(){
  kListeningOn = false;
  return;
}

int tcpServer::ReplyToCmd(char* msg) {
  //Send msg to socket
  int n = write(kTcpConn, msg, kCmdLen);
  if (n < 0){
    fprintf(stderr, "%s) Error in writing to the socket\n", __METHOD_NAME__);
    return 1;
  }
  
  if (kVerbosity>1) {
    printf("%s) Sent %d bytes\n", __METHOD_NAME__, n);
  }
  
  return 0;
}
