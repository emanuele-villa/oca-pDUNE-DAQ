#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

#include "tcpserver.h"
#include "utility.h"

tcpserver::tcpserver(int port, int verb){

  verbosity=verb;
  
  int sock, addrlen;
  struct sockaddr_in client_addr, server_addr;
  
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock < 0) {
    perror("Socket creation error: \n");
  }
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  
  if (bind(sock, (struct sockaddr *) &server_addr , sizeof(server_addr)) < 0){
    perror("Bind error\n");
    exit(EXIT_FAILURE);
  }
  else {
    if (verbosity>0) {
      printf("%s) correct bind...\n", __METHOD_NAME__);
    }
    fflush(stdout);
  }
  
  if (listen(sock, 1) < 0){
    perror("listening not possibile: \n");
    exit(EXIT_FAILURE);
  }
  addrlen = sizeof(client_addr);
  if (verbosity) {
    printf("%s) waiting for connections...\n", __METHOD_NAME__);
  }
  _socket = accept(sock, (struct sockaddr *) &client_addr, (socklen_t *) &addrlen);
  if (_socket < 0){
    perror("Negotiation error: \n");
  }
  else {
    if (verbosity>0) {
      printf("%s) connection succeded: (socket number %d)\n", __METHOD_NAME__, _socket);
    }
    close(sock);
  }

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

  while(1){
    
    char msg[256];
    
    if(read(_socket, msg, sizeof(msg)) < 0){
      perror("Read error: \n");
    }
    else {
      if (verbosity>0) {
	printf("%s) %s\n", __METHOD_NAME__, msg);
      }
    }

    bzero(msg, sizeof(msg));
  }
  
  return;
}
