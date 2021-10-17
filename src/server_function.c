#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "hwlib.h"
#include <netinet/in.h>
//#include <fcntl.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
//#include <sys/mman.h>
#include <arpa/inet.h>
#include <vector>
#include <iostream>
#include <chrono>

#include "utility.h"

#include "highlevelDriversFPGA.h"
#include "lowlevelDriversFPGA.h"
#include "server_function.h"
#include "server.h"

// hpsserver.cpp

/*
//Receive a single 32-bit word from socket
uint32_t receiveWordSocket(int socket){
  char msg[sizeof(uint32_t) * 8 + 1];
  char *ptr;
  if(read(socket, msg, sizeof(msg)) < 0){
    fprintf(stderr, "Error in reading the socket\n");
    return -1;
  }
  else{
    uint32_t data = strtoul(msg, &ptr, 16);
    printf("Received %08x\n", data);
    return data;
  }
}
*/

//Generic receive from socket
int receiveSocket(int socket, void* msg, uint32_t len){
  int n;
  n = read(socket, msg, len);
  if (n < 0) {
    fprintf(stderr, "Error in reading the socket\n");
    return -1;
  }
  return 0;
}

//Send a string to socket
int sendSocket(int socket, void* msg, uint32_t len){
  int n;
  n = write(socket, msg, len);
  if(n < 0){
    fprintf(stderr, "Error in writing to the socket\n");
    return 1;
  }
  if (baseAddr.verbose > 3) printf("%s) Sent %d bytes\n", __METHOD_NAME__, n);
  return 0;
}

//------------------------------------------------------------------------

// hpsserver.cpp
// commento solo la receiver_comandi

void *receiver_slow_control(void *args){

  int len, rc, on = 1;
  int listen_sd = -1, new_sd = -1;
  char buffer[80];
  struct sockaddr_in addr;
  int timeout;
  struct pollfd fds[200];
  int nfds = 1, current_size = 0;
  char *port = (char*)args;
  int porta = atoi(port);

  listen_sd = socket(AF_INET, SOCK_STREAM, 0);
  if(listen_sd < 0){

    perror("socket() fallita");
    exit(-1);
  }

  rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
  if(rc < 0){

    perror("setopt() fallita");
    close(listen_sd);
    exit(-1);
  }

  rc = ioctl(listen_sd, FIONBIO, (char *)&on);
  if(rc < 0){

    perror("ioctl() fallita");
    close(listen_sd);
    exit(-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(porta);

  rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
  if(rc < 0){

    perror("bind() fallita");
    close(listen_sd);
    exit(-1);
  }

  rc = listen(listen_sd, 32);
  if(rc < 0){

    perror("listen() fallita");
    close(listen_sd);
    exit(-1);
  }

  memset(fds, 0, sizeof(fds));

  fds[0].fd = listen_sd;
  fds[0].events = POLLIN;

  timeout = (3 * 60* 1000);

  while(1){

    printf("faccio poll\n");
    rc = poll(fds, nfds, timeout);
    if(rc < 0){

      perror("errore poll()");
      close(listen_sd);
      exit(-1);
    }

    current_size = nfds;
    printf("Current size %d\n", current_size);
    if(fds[0].revents & POLLIN){

      struct sockaddr_in cliaddr;
      int addrlen = sizeof(cliaddr);
      new_sd = accept(listen_sd, (struct sockaddr *)&cliaddr, (socklen_t * __restrict__ )&addrlen);
      printf("connessione da parte di %d accettata\n", new_sd);
      for(int i = 0; i < 200; i++){

	if(fds[i].fd == 0){

	  fds[i].fd = new_sd;
	  fds[i].events = POLLIN;
	  nfds++;
	  break;
	}
      }
    }

    for(int i = 1; i < 200; i++){

      if(fds[i].fd > 0 && fds[i].revents & POLLIN){

	rc = read(fds[i].fd, buffer, sizeof(buffer));
	if(rc < 0){

	  perror("errore poll()");
	  exit(-1);
	}else if(rc == 0){

	  close(fds[i].fd);
	  fds[i].fd = -1;
	  continue;
	}

	len = rc;
	printf("[SERVER] ho ricevuto %d bytes, da %d:: %s\n", len, fds[i].fd, buffer);

	rc = write(fds[i].fd, buffer, len);
	if(rc < 0){

	  perror("errore write()");
	  exit(-1);
	}



      }
    }
  }

  pthread_exit(NULL);
}

int sockOpener(void* args, int* sockIn) {
  // questa parte diventa un banale
  // hpsserver* hsrv = new hpsserver(bla, bla, bla)
  // dare controllata che tutta 'sta roba sia, uguale, nel costruttore di tcpserver
  // in teoria si' perche' io da qui ho copiato
  // ma sicuramente almeno manca il pezzo committato da Nicolo' dopo:
  // https://github.com/PerugiaOverNetDAQ/oca/commit/b8daee0873b71e149a75e278ae9f00c0b8d2b702

  char* port = (char*)args;
  int porta = atoi(port);
  struct sockaddr_in server_addr;
  int n =0;

  printf("TCP/IP socket: Opening... ");
  *sockIn = socket(AF_INET , SOCK_STREAM , 0);
  if(*sockIn < 0){
    perror("Error in socket creation...\n");
    return 1;
  }

  //Avoid "Address already in use" issue at server startup
  if (setsockopt(*sockIn , SOL_SOCKET, SO_REUSEADDR,&n, sizeof(int)) == -1) {
    perror("setsockopt failed...\n");
    //exit(1);
    return 2;
  }
  printf("ok\n");

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(porta);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  printf("TCP/IP socket: binding... ");
  if(bind(*sockIn, (struct sockaddr *) &server_addr , sizeof(server_addr)) < 0){
    perror("Error\n");
    //exit(EXIT_FAILURE);
    return EXIT_FAILURE;
  }else{
    printf("ok\n");
    fflush(stdout);
  }

  printf("TCP/IP socket: listening... ");
  if(listen(*sockIn, 1) < 0){
    perror("Error\n");
    //exit(EXIT_FAILURE);
    return EXIT_FAILURE;
  }
  printf("ok\n");

  return 0;
}

void* receiver_comandi(int* sockIn){
  int addrLen, openConn;
  struct sockaddr_in client_addr;

  /* ShowStatusFifo(CONFIG_FIFO); */
  /* printf("%s) %u\n", __METHOD_NAME__, *(baseAddr.configFifo)); */
  /* printf("%s) %u\n", __METHOD_NAME__, *(baseAddr.configFifoCsr)); */
  /* InitFifo(CONFIG_FIFO, 5, 1000, 0); */
  /* ShowStatusFifo(CONFIG_FIFO); */

  addrLen = sizeof(client_addr);
  printf("Waiting for a client to connect...\n");
  openConn = accept(*sockIn, (struct sockaddr *) &client_addr, (socklen_t *) &addrLen);
  if(openConn < 0){
    perror("Error in accepting socket connection\n");
  }
  else{
    uint32_t trash;
    printf("%s) Connection open: (socket number %d, %s:%d)\n", __METHOD_NAME__, openConn, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    // printf("\nRegister array initial content:\n");
    // for(int j=0; j<32; j++){
    //   ReadReg(j, &trash);
    // }
    // printf("\n");
  }
  //Handshaking to set the same command length of the client
  int cmdLen = 24;
  receiveSocket(openConn, &cmdLen, sizeof(cmdLen));
  printf("%s) Updating command length to %d\n", __METHOD_NAME__, cmdLen);
  sendSocket(openConn, &cmdLen, sizeof(cmdLen));

  //FIX ME fetch the GW version from gitlab and not from FPGA
  ReadReg(rGW_VER, &kGwV);


  //-----------------------------------------------------
  // questa parte sara' un metodo di hpsserver

  //------------------------------------------------------
  // questa parte sarà il Listen() (di tcpserver)
  // che a sua volta chiama il
  // virtual void ProcessMsgReceived(char* msg);
  // che e' specializzato/implementato in hpsserver

  uint32_t okVal  = 0xb01af1ca;
  uint32_t badVal = 0x000cacca;
  bool kControl = true;
  uint32_t evtCount = 0;
  using clock_type = std::chrono::system_clock;
  auto startRunTime = clock_type::now();
  while(kControl) {
    char msg[256]="";
    int bytesRead=0;

    //Read the command
    bytesRead=read(openConn, msg, cmdLen);

    //Check if the read is ok and process its content
    if(bytesRead < 0) {
      // Error: check for specific errors
      if (EAGAIN==errno || EWOULDBLOCK==errno) {
        printf("%s) errno: %d\n", __METHOD_NAME__, errno);
      }
      else {
        printf("%s) Read error: (%d)\n", __METHOD_NAME__, errno);
        perror("Read error");
      }
    }
    else if (bytesRead==0) {
      kControl=false;
      printf("%s) Client closed the connection\n", __METHOD_NAME__);
    }
    else {
      if(strcmp(msg, "cmd=init") == 0){
        uint32_t regsContent[14];
	uint32_t singleReg = 0;

        if (baseAddr.verbose > 1) printf("%s) Starting init...\n", __METHOD_NAME__);
        //Receive the whole content (apart from reg rGOTO_STATE)
        for(int ii = 0; ii < 7; ii++){
	  receiveSocket(openConn, &singleReg, sizeof(singleReg));
          regsContent[ii*2]   = singleReg;
          regsContent[ii*2+1] = (uint32_t)ii+1;
        }

        Init(regsContent, 14);

        sendSocket(openConn, &okVal, sizeof(okVal));
      }
      else if(strcmp(msg, "cmd=readReg") == 0){
        uint32_t regAddr = 0;
        uint32_t regContent;
        receiveSocket(openConn, &regAddr, sizeof(regAddr));
        printf("Send read request...\n");
        ReadReg(regAddr, &regContent);
        sendSocket(openConn, &regContent, sizeof(regContent));
      }
      else if((strcmp(msg, "cmd=setDelay")==0)||(strcmp(msg, "cmd=overWriteDelay")==0)){
        uint32_t delay = 0;
        receiveSocket(openConn, &delay, sizeof(delay));
        SetDelay(delay);
        sendSocket(openConn, &okVal, sizeof(okVal));
      }
      else if(strcmp(msg, "cmd=setMode") == 0){
        uint32_t mode = 0;
        receiveSocket(openConn, &mode, sizeof(mode));
        SetMode(mode);
        sendSocket(openConn, &okVal, sizeof(okVal));
      }
      else if(strcmp(msg, "cmd=getEventNumber") == 0){
        uint32_t extTrigCount, intTrigCount;

        GetEventNumber(&extTrigCount, &intTrigCount);

        sendSocket(openConn, &extTrigCount, sizeof(extTrigCount));
        sendSocket(openConn, &intTrigCount, sizeof(intTrigCount));
      }
      else if(strcmp(msg, "cmd=eventReset") == 0){
        evtCount = 0;
        startRunTime = clock_type::now();
        EventReset();
        sendSocket(openConn, &okVal , sizeof(okVal));
      }
      else if(strcmp(msg, "cmd=calibrate") == 0){
        uint32_t calib = 0;
	receiveSocket(openConn, &calib, sizeof(calib));
        Calibrate(calib);
        sendSocket(openConn, &okVal, sizeof(calib));
      }
      else if(strcmp(msg, "cmd=writeCalibPar") == 0){
        printf("%s) WriteCalibPar not supported\n", __METHOD_NAME__);
        //sendSocket(openConn, &badVal, sizeof(badVal));
      }
      else if(strcmp(msg, "cmd=saveCalib") == 0){
        printf("%s) SaveCalibrations not supported\n", __METHOD_NAME__);
        //sendSocket(openConn, &badVal, sizeof(badVal));
      }
      else if(strcmp(msg, "cmd=intTrigPeriod") == 0){
        uint32_t period = 0;
	receiveSocket(openConn, &period, sizeof(period));
        intTriggerPeriod(period);
        sendSocket(openConn, &okVal, sizeof(okVal));
      }
      else if(strcmp(msg, "cmd=selectTrigger") == 0){
        uint32_t intTrig = 0;
	receiveSocket(openConn, &intTrig, sizeof(intTrig));
        selectTrigger(intTrig);
        sendSocket(openConn, &okVal, sizeof(okVal));
      }
      else if(strcmp(msg, "cmd=configTestUnit") == 0){
        uint32_t tuCfg = 0;
	receiveSocket(openConn, &tuCfg, sizeof(tuCfg));
        char testUnitCfg = ((tuCfg&0x300)>>8);
        char testUnitEn  = ((tuCfg&0x2)>>1);
        configureTestUnit(tuCfg);
        if (baseAddr.verbose > 1) {
          printf("Test unit cfg: %d - en: %d\n", testUnitCfg, testUnitEn);
        }
        sendSocket(openConn, &okVal, sizeof(okVal));
      }
      else if(strcmp(msg, "cmd=getEvent") == 0){
	//	printf("%s-%d) Qui!\n", __METHOD_NAME__, __LINE__);
	std::vector<uint32_t> evt;

        int evtLen=0;

        //Get an event from FPGA
        int evtErr = getEvent(evt, &evtLen);
        if (baseAddr.verbose > 3) printf("getEvent result: %d\n", evtErr);

        //Send the eventLen to the socket
        sendSocket(openConn, &evtLen, sizeof(evtLen));
        //Send the event to the socket
        if (evtLen>0) {
          sendSocket(openConn, evt.data(), evtLen*sizeof(uint32_t));
          evtCount++;
          if (evtCount % 1000 == 0) {
            auto evt1000 = clock_type::now();
            std::cout << "Event count : " << evtCount << " in " << std::chrono::duration_cast<std::chrono::seconds>(evt1000 - startRunTime).count() << " s\n";
          }
        }
        if (baseAddr.verbose > 3) printf("%s) Event sent\n", __METHOD_NAME__);
      }
      else if (strcmp(msg, "cmd=setCmdLenght") == 0) {
        receiveSocket(openConn, &cmdLen, sizeof(cmdLen));
        printf("%s) Updating command length to %d\n", __METHOD_NAME__, cmdLen);
        sendSocket(openConn, &cmdLen, sizeof(cmdLen));
      }
      else if (strcmp(msg, "cmd=quit") == 0) {
        printf("FIX ME: Close connection and socket...\n");
        //kControl=false;
      }
      else {
        printf("%s) Unkown message: %s\n", __METHOD_NAME__, msg);
        sendSocket(openConn, &badVal, sizeof(badVal));
      }
    }
  }

  //-------------------------------------------------

  //pthread_exit(NULL);
  return nullptr;
}
