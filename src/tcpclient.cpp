#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
  cmdlenght=8;//in number of char/bytes
}

tcpclient::~tcpclient(){
  //  client_send("ciao");
  //  printf("%s) client_socket = %d\n", __METHOD_NAME__, client_socket);
  if (client_socket != -1) {
    //    close(client_socket);
    shutdown(client_socket, SHUT_RDWR);
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
    printf("%s) connection error: (socket number %d, %s:%d)\n", __METHOD_NAME__, client_socket, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    perror("connection error: ");
    exit(EXIT_FAILURE);
  }

  if (verbosity>0) {
    printf("%s) connection succeded\n", __METHOD_NAME__);
  }

  return client_socket;
}

int tcpclient::client_send(const char* buffer) {
  return client_send((void*)buffer, strlen(buffer));
}

int tcpclient::client_send(const char* buffer, int bytesize) {
  return client_send((void*)buffer, bytesize);
}

int tcpclient::client_send(void* buffer, int bytesize) {

  int result = 0;

  //char backup[d_dampe_string_buffer];
  if (verbosity>0) {
    printf("%s) message sending\n", __METHOD_NAME__);
  }
  if ((client_socket != -1) && buffer) {
    //memset(backup, 0, d_dampe_string_buffer);
    //snprintf(backup, d_dampe_string_buffer, "%s", buffer);
    //    result = write(client_socket, buffer, strlen(buffer))
    result = write(client_socket, buffer, bytesize);
    if (result > 0) {
      usleep(250000);//FIX ME: really needed?
      if (verbosity>0) {
        printf("%s) [CLIENT] message sent correctly\n", __METHOD_NAME__);
      }
      //      changeText("inviato");
    }
    else {
      fprintf(stderr, "%s) error on sending", __METHOD_NAME__);
      result = -1;
    }
  }
  else {
    fprintf(stderr, "%s) error on socket", __METHOD_NAME__);
    result = -2;
  }

  return result;
}

int tcpclient::client_receive(char* buffer, int bytesize){//the lenght must be know
  return client_receive((void*)buffer, bytesize);
}

int tcpclient::client_receive(void* msg, int bytesize){

  size_t n = 0;

  if (verbosity>0) {
    printf("%s) listening on socker number %d\n", __METHOD_NAME__, client_socket);
  }

  n = read(client_socket, msg, bytesize);

  if(n < 0){
    fprintf(stderr, "%s) Receiving error\n", __METHOD_NAME__);
  }
  else if(n == 0){
    if (verbosity>0) {
      printf("%s) Nothing to read\n", __METHOD_NAME__);
    }
  }
  else{
    //msg[n] = '\0';
    if (verbosity>0) {
      printf("%s) Bytes read: %lu\n", __METHOD_NAME__, n);
      printf("%s) %s\n", __METHOD_NAME__, (char*)msg);
      usleep(100000);
    }

    //FIX ME: a cosa serve?
    //if(msg[n - 1] == '\0'){
    //  bzero(msg, sizeof(msg));
    //  return -1;
    //}
  }

  return n;
}

//---------------------------------------------------------------

int tcpclient::Receive(char* buffer, int bytesize){
  
  int ret = client_receive(buffer, bytesize);
  
  if (ret <= 0) {
    strcpy(buffer, "");
  }
  
  return ret;
}

int tcpclient::Receive(void* buffer, int bytesize){
  return client_receive(buffer, bytesize);  
}

int tcpclient::ReceiveCmdReply(char* buffer){
  return Receive(buffer, (cmdlenght*8)*sizeof(char)+1);
}

int tcpclient::Send(const char *buffer) {
  return client_send(buffer);
}

int tcpclient::Send(void* buffer, int bytesize) {
  return client_send(buffer, bytesize);
}
 
int tcpclient::SendCmd(const char *buffer){
  char c[sizeof(char) * 8 * cmdlenght + 1];
  sprintf(c, "cmd=%s", buffer);
  return client_send(c);
}

int tcpclient::SendInt(uint32_t par){
  char c[sizeof(uint32_t) * 8 + 1];
  sprintf(c, "%08x", par);
  return client_send(c);
}
