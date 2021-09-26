#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sched.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <stdint.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "daqserver.h"

daqserver::daqserver(int port){

  int sock, addrlen, new_socket;
  struct sockaddr_in client_addr, server_addr;
  
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if(sock < 0){
    perror("errore creazione socket\n");
  }
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  
  if(bind(sock, (struct sockaddr *) &server_addr , sizeof(server_addr)) < 0){
    perror("errore nel bind\n");
    exit(EXIT_FAILURE);
  }
  else{
    printf("bind corretto...\n");
    fflush(stdout);
  }
  
  if(listen(sock, 1) < 0){
    perror("impossibile ascoltare\n");
    exit(EXIT_FAILURE);
  }
  addrlen = sizeof(client_addr);
  printf("attendo connessioni...\n");
  new_socket = accept(sock, (struct sockaddr *) &client_addr, (socklen_t *) &addrlen);
  if(new_socket < 0){
    perror("errore accettazione\n");
  }
  else{
    printf("connessione riuscita al socket comandi principali: socket %d\n", new_socket);
    close(sock);
  }

  return;
}

/*

  while(1){
    
    char msg[256];
    
    if(read(new_socket, msg, sizeof(msg)) < 0){
      perror("errore nella read\n");
    }
    else{
      if(strcmp(msg, "init") == 0){
	Init(new_socket);
      }
      if(strcmp(msg, "set delay") == 0){
	SetDelay(new_socket);
      }

      if(strcmp(msg, "get event") == 0){

	GetEvent(new_socket);
      }

      if(strcmp(msg, "set mode") == 0){

	SetMode(new_socket);
      }

      if(strcmp(msg, "get event number") == 0){

	GetEventNumber(new_socket);
      }

      if(strcmp(msg, "print all event number") == 0){

	PrintAllEventNumber(new_socket);
      }

      if(strcmp(msg, "event reset") == 0){

	EventReset(new_socket);
      }

      if(strcmp(msg, "OverWriteDelay") == 0){

	OverWriteDelay(new_socket);
      }

      if(strcmp(msg, "Calibrate") == 0){

	Calibrate(new_socket);
      }

      if(strcmp(msg, "WriteCalibPar") == 0){

	WriteCalibPar(new_socket);
      }

      if(strcmp(msg, "SaveCalibrations") == 0){

	SaveCalibrations(new_socket);
      }

      if(strcmp(msg, "intTriggerPeriod") == 0){

	intTriggerPeriod(new_socket);
      }

      if(strcmp(msg, "selectTrigger") == 0){

	selectTrigger(new_socket);
      }

      if(strcmp(msg, "configureTestUnit") == 0){

	configureTestUnit(new_socket);
      }
    }

    bzero(msg, sizeof(msg));

  }

*/
