#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include "daqclient.h"
#include <errno.h>
#include <stdint.h>

daqclient::daqclient(const char *address, int port){
  printf("daqclient creato\n");
  client_socket = client_connect(address, port);
}

//--------------------------------------------------------------

daqclient::~daqclient(){
  client_send("quit\n");
  if (client_socket != -1) {
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
    client_socket = -1;
  }
}

int daqclient::client_connect(const char *address, int port) {
  struct sockaddr_in server_addr;
  struct hostent *server;
  
  client_socket = socket(AF_INET, SOCK_STREAM, 0);

  if(client_socket < 0){
    perror("errore creazione socket");
    exit(EXIT_FAILURE);
  }
  else{
    printf("socket creato: %d\n", client_socket);
  }
    
  server = gethostbyname(address);
  if(server == NULL){
    fprintf(stderr, "non esiste questo host");
    exit(EXIT_FAILURE);
  }
  
  bzero((char *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
  server_addr.sin_port = htons(port);
  
  if(::connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
    perror("errore connessione");
    exit(EXIT_FAILURE);
  }
  
  printf("connessione riuscita\n");
  
  return client_socket;
}

int daqclient::client_send(const char *buffer) {

  int result;

  //char backup[d_dampe_string_buffer];
  printf("invio\n");
  if ((client_socket != -1) && (buffer)) {
    //memset(backup, 0, d_dampe_string_buffer);
    //snprintf(backup, d_dampe_string_buffer, "%s", buffer);
    if ((write(client_socket, buffer, strlen(buffer)) > 0)) {
      usleep(250000);
      printf("[CLIENT] messaggio inviato correttamente\n");
    }
    else{
      fprintf(stderr, "errore invio");
    }
  }
  
  return true;
}

int daqclient::client_receive_int(){
  
  int n;
  
  int cont = 0;
  
  while(cont < 111 * sizeof(int)){
    int temp;
    n = read(client_socket, &temp, sizeof(temp));
    if(n < 0){
      perror("errore lettura");
    }
    else{
      char c[4];
      char msg[256];
      sprintf(c, "%x", temp);
      //sprintf(msg, "ho letto %d", n);
      //usleep(100000);
      bzero(c, sizeof(c));
      cont += n;
    }
  }

  return n;
}

int daqclient::client_receive(){
  
  char msg[257];
  size_t n = 0;
  printf("in ascolto su socket %d\n", client_socket);
  
  n = read(client_socket, msg, sizeof(msg) - 1);
  
  if(n < 0){
    fprintf(stderr, "errore lettura\n");
  }
  else if(n == 0){
    printf("finito\n");
  }
  else{
    //msg[n] = '\0';
    printf("ho letto: %lu\n", n);
    printf("%s\n",msg);
    usleep(100000);
    
    if(msg[n - 1] == '\0'){
      bzero(msg, sizeof(msg));
      return -1;
    }
  }
  
  bzero(msg, sizeof(msg));
  return n;
}
