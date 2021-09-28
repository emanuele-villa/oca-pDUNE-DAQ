#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

#include "tcpserver.h"
#include "utility.h"

tcpserver::tcpserver(int port, int verb){

  verbosity=verb;
  
  int sock, addrlen;
  struct sockaddr_in client_addr, server_addr;
  
  sock = socket(AF_INET , SOCK_STREAM , 0);
  exit_if(sock<0, "%s) Socket creation error:", __METHOD_NAME__);
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  
  int retbind = bind(sock, (struct sockaddr *) &server_addr , sizeof(server_addr));
  exit_if(retbind<0, "%s) Bind error:", __METHOD_NAME__);

  if (verbosity>0) {
    printf("%s) correct bind...\n", __METHOD_NAME__);
  }
  fflush(stdout);
  
  //  int retlisten=listen(sock, 1);//Nicolo'
  int retlisten=listen(sock, 100);//Matteo D.
  exit_if(retlisten<0, "%s) Listening not possibile:", __METHOD_NAME__);

  addrlen = sizeof(client_addr);
  printf("%s) waiting for connections...\n", __METHOD_NAME__);
  _socket = accept(sock, (struct sockaddr *) &client_addr, (socklen_t *) &addrlen);
  //  FIX ME: capire come ammazzare accept in maniera gracefully
  exit_if(_socket<0, "%s) Negotiation error:", __METHOD_NAME__);
  
  //------------ make the read(socket) non-blocking ---------
  // first line needed to save current flags, if any 
  int flags=fcntl(_socket ,F_GETFL, 0);
  exit_if(flags < 0, "%s) Fcntl failed:", __METHOD_NAME__);
  int retflags=fcntl(_socket, F_SETFL, flags | O_NONBLOCK);
  exit_if(retflags < 0, "%s) Fcntl failed:", __METHOD_NAME__);
  //--------------------------------------------------------

  sockaddr_in peer;
  socklen_t alen = sizeof(peer);
  getpeername(_socket, (sockaddr *) &peer, &alen);

  printf("%s) connection succeded: (socket number %d, %s:%d)\n", __METHOD_NAME__, _socket, inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
  close(sock);

  return;
}

tcpserver::~tcpserver(){
  if (verbosity>0) {
    printf("%s) destroying tcpserver\n", __METHOD_NAME__);
  }
  printf("FIX ME: close the socket\n");
  return;
}

void tcpserver::Listen(){
    
  char msg[LEN];

  ssize_t readret = read(_socket, msg, sizeof(msg));
  printf("readret = %ld\n", readret);
  
  if (readret < 0){
    if (EAGAIN == errno || EWOULDBLOCK == errno) {
      if (verbosity>-1) {
	printf("%s) there's nothing to read now; try again later\n", __METHOD_NAME__);
      }
    }
    else {
      print_error("%s) Read error: \n", __METHOD_NAME__);
    }
  }
  else {
    if (verbosity>-1) {
      printf("%s) %s\n", __METHOD_NAME__, msg);
    }
  }
  
  bzero(msg, sizeof(msg));
  
  return;
}
