#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <unistd.h>

#include <netdb.h>

#include "tcpclient.h"

#include "utility.h"

tcpclient::tcpclient(const char *address, int port, int verb){
  verbosity=verb;
  if (verbosity>0) {
    printf("%s) tcpclient created\n", __METHOD_NAME__);
  }
  client_socket = client_connect(address, port);
}

tcpclient::~tcpclient(){
  client_send("quit\n");
  if (client_socket != -1) {
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
    client_socket = -1;
  }
}

//--------------------------------------------------------------

int tcpclient::client_connect(const char *address, int port) {
  struct sockaddr_in server_addr;
  struct hostent *server;

  client_socket = socket(AF_INET, SOCK_STREAM, 0);

  if(client_socket < 0){
    perror("socket creation error: ");
    exit(EXIT_FAILURE);
  }
  else{
    if (verbosity>0) {
      printf("%s) socket created (number %d)\n", __METHOD_NAME__, client_socket);
    }
  }

  server = gethostbyname(address);
  if(server == NULL){
    fprintf(stderr, "%s) host not existing: %s", __METHOD_NAME__, address);
    exit(EXIT_FAILURE);
  }

  bzero((char *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
  server_addr.sin_port = htons(port);

  if(::connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
    perror("connection error: ");
    exit(EXIT_FAILURE);
  }

  if (verbosity>0) {
    printf("%s) connection succeded\n", __METHOD_NAME__);
  }

  return client_socket;
}

int tcpclient::client_send(const char *buffer) {

  int result;

  //char backup[d_dampe_string_buffer];
  if (verbosity>0) {
    printf("%s) message sending\n", __METHOD_NAME__);
  }
  if ((client_socket != -1) && (buffer)) {
    //memset(backup, 0, d_dampe_string_buffer);
    //snprintf(backup, d_dampe_string_buffer, "%s", buffer);
    if ((write(client_socket, buffer, strlen(buffer)) > 0)) {
      usleep(250000);
      if (verbosity>0) {
	printf("%s) [CLIENT] message sent correctly\n", __METHOD_NAME__);
      }
      //      changeText("inviato");
    }
    else {
      fprintf(stderr, "%s) error on sending", __METHOD_NAME__);
    }
  }

  return true;
}

int tcpclient::client_receive_int(){

  int n;

  int cont = 0;

  while(cont < 651 * sizeof(int)){
    int temp;
    n = read(client_socket, &temp, sizeof(temp));
    if(n < 0){
      perror("reading error: ");
    }
    else{
      char c[4];
      char msg[256];
      sprintf(c, "%08x", temp);
      //sprintf(msg, "ho letto %d", n);
      //usleep(100000);
      bzero(c, sizeof(c));
      cont += n;
    }
  }

  //  changeText("fine");
  
  return n;//FIX ME: tocca ritornare temp
}

int tcpclient::client_receive(){
  
  char msg[LEN];

  size_t n = 0;
  if (verbosity>0) {
    printf("%s) listening on socker number %d\n", __METHOD_NAME__, client_socket);
  }
  
  n = read(client_socket, msg, sizeof(msg) - 1);//FIX ME: giusto leggere "grande a piacere"?
  
  if(n < 0){
    fprintf(stderr, "%s) receiving error\n", __METHOD_NAME__);
  }
  else if(n == 0){
    if (verbosity>0) {
      printf("%s) receiving finished\n", __METHOD_NAME__);
    }
  }
  else{
    //msg[n] = '\0';
    if (verbosity>0) {
      printf("%s) read: %lu\n", __METHOD_NAME__, n);
      printf("%s) %s\n", __METHOD_NAME__, msg);
      usleep(100000);
    }

    if(msg[n - 1] == '\0'){
      bzero(msg, sizeof(msg));
      return -1;
    }
  }

  bzero(msg, sizeof(msg));
  return n;
}
