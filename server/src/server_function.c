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

#include "highlevelDriversFPGA.h"
#include "lowlevelDriversFPGA.h"
#include "server_function.h"
#include "server.h"

//Receive a single 32-bit word from socket
uint32_t receiveWordSocket(int socket){
	char msg[sizeof(uint32_t) * 8 + 1];
	char *ptr;
	if(read(socket, msg, sizeof(msg)) < 0){
		fprintf(stderr, "Error in reading the socket\n");
		return -1;
	}else{
		uint32_t data = strtoul(msg, &ptr, 16);
		printf("Received %08x\n", data);
		return data;
	}
}

//Send a string to socket
int sendSocket(int socket, char * msg){
	if(write(socket, msg, strlen(msg)) < 0){
		fprintf(stderr, "Error in writing to the socket\n");
		return 1;
	}
	return 0;
}

//Acquire a packet from the FPGA and forward it to the socket
void *high_priority(void *socket){
	uint32_t evt;
	int evtLen=0;
	int n;
	int sock = *(int *)socket;

	//Get an event from FPGA
	int evtErr = getEvent(&evt, evtLen);
	if (verbose > 1) printf("getEvent result: %d\n", evtErr);

	//Send the event to the socket
	n = write(sock, &evt, evtLen);
	if(n < 0){
		perror("Error in writing an event to the socket\n");
	}else{
		if (verbose > 1) printf("Sent %d bytes\n", n);
	}

	//Kill the thread
	pthread_exit(NULL);
}

//Spawn a thread to send an event to socket
void GetEvent(int socket){
	pthread_t t;
	pthread_attr_t attr;
	int new_priority = 20;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_getschedparam(&attr, &param);
	param.sched_priority = new_priority;

	if (verbose > 1) printf("socket: %d\n", socket);
	if(pthread_create(&t, &attr, &high_priority, &socket) < 0){
		perror("Error in creating high_priority thread\n");
	}
	pthread_join(t, 0);

}

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
		if(fds[0].revents & POLLIN){

			struct sockaddr_in cliaddr;
            int addrlen = sizeof(cliaddr);
            new_sd = accept(listen_sd, (struct sockaddr *)&cliaddr, (socklen_t *restrict)&addrlen);
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

void *receiver_comandi(void *args){

	char *port = (char*)args;
	int porta = atoi(port);
	int sock, addrlen, new_socket;
	struct sockaddr_in client_addr, server_addr;
	int n = 1;

  printf("TCP/IP socket: Opening\n");
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if(sock < 0){

		perror("errore creazione socket\n");
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(porta);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (setsockopt(sock , SOL_SOCKET, SO_REUSEADDR,&n, sizeof(int)) == -1) {
    	perror("setsockopt");
    	exit(1);
	}

  printf("TCP/IP socket: binding... ");
	if(bind(sock, (struct sockaddr *) &server_addr , sizeof(server_addr)) < 0){

		perror("errore nel bind\n");
		exit(EXIT_FAILURE);
	}else{

		printf("ok\n");
		fflush(stdout);
	}

  printf("TCP/IP socket: listening\n");
	if(listen(sock, 1) < 0){

		perror("impossibile ascoltare\n");
		exit(EXIT_FAILURE);
	}
	addrlen = sizeof(client_addr);
	printf("attendo connessioni...\n");
	new_socket = accept(sock, (struct sockaddr *) &client_addr, (socklen_t *) &addrlen);
	if(new_socket < 0){

		perror("errore accettazione\n");

	}else{

		printf("connessione riuscita al socket comandi principali: socket %d\n", new_socket);
		close(sock);

	}

  //Stampa del contenuto del Register Array
  int j;
  uint32_t trash;
  printf("\n");
  printf("Contenuto iniziale del Register Array:\n");
  for(j=0; j<32; j++){
    ReadReg(j, &trash);
  }
  printf("\n");
  //Fine della stampa

	while(1){

		char msg[256];
    char replyStr[256];
		if(read(new_socket, msg, sizeof(msg)) < 0){

			perror("errore nella read\n");
		}else{

			if(strcmp(msg, "init") == 0){
        uint32_t regsContent[14];

        sprintf(replyStr, "%s", "[SERVER] Starting Init. Send data...");
        printf("%s\n", replyStr);
        sendSocket(new_socket, replyStr);

        //Receive the whole content (apart from reg rGOTO_STATE)
        for(int ii = 0; ii < 7; ii++){
          regsContent[ii*2]   = receiveWordSocket(new_socket);
          regsContent[ii*2+1] = (uint32_t)ii;
        }

        Init(regsContent, 14);
			}

      if(strcmp(msg, "readReg") == 0){
        uint32_t regAddr = receiveWordSocket(new_socket);
        uint32_t regContent;

        printf("Send read request...\n");
        ReadReg(regAddr, &regContent);

        sprintf(replyStr, "%s %u: %08x", "[SERVER] Reg", regAddr, regContent);
        sendSocket(new_socket, replyStr);
      }

			if((strcmp(msg, "set delay")==0)||(strcmp(msg, "OverWriteDelay")==0)){
      	uint32_t delay = receiveWordSocket(new_socket);

				SetDelay(delay);

        sprintf(replyStr, "%s %d", "[SERVER] Delay: ", delay);
        sendSocket(new_socket, replyStr);
			}

			if(strcmp(msg, "set mode") == 0){
        uint32_t mode = receiveWordSocket(new_socket);
				SetMode(mode);
        sprintf(replyStr, "%s %d", "[SERVER] Setting mode: ", mode);
        sendSocket(new_socket, replyStr);
			}

			if(strcmp(msg, "get event number") == 0){
        uint32_t extTrigCount, intTrigCount;

				GetEventNumber(&extTrigCount, &intTrigCount);

        sprintf(replyStr, "%s %08u %08u", "[SERVER] Events number (int, ext): ", \
                            extTrigCount, intTrigCount);
        sendSocket(new_socket, replyStr);
			}

			if(strcmp(msg, "print all event number") == 0){
        uint32_t extTrigCount, intTrigCount;

				GetEventNumber(&extTrigCount, &intTrigCount);

        sprintf(replyStr, "%s %08u %08u", "[SERVER] Events number (int, ext): ", \
                            extTrigCount, intTrigCount);
        printf("%s\n",replyStr);
        sendSocket(new_socket, replyStr);
			}

			if(strcmp(msg, "event reset") == 0){
				EventReset();
        sprintf(replyStr, "%s", "[SERVER] Reset ok");
        sendSocket(new_socket, replyStr);
			}

			if(strcmp(msg, "Calibrate") == 0){
        uint32_t calib = receiveWordSocket(new_socket);

        Calibrate(calib);

        sprintf(replyStr, "%s %d", "[SERVER] Calibration enable: ", calib);
        sendSocket(new_socket, replyStr);
			}

			if(strcmp(msg, "WriteCalibPar") == 0){
        sprintf(replyStr, "%s", "[SERVER] WriteCalibPar");
        sendSocket(new_socket, replyStr);
			}

			if(strcmp(msg, "SaveCalibrations") == 0){
        sprintf(replyStr, "%s", "[SERVER] SaveCalibrations");
        sendSocket(new_socket, replyStr);
			}

      if(strcmp(msg, "intTriggerPeriod") == 0){
        uint32_t period = receiveWordSocket(new_socket);

        intTriggerPeriod(period);

        sprintf(replyStr, "%s %08u", "[SERVER] Trigger period: ", period);
        sendSocket(new_socket, replyStr);
			}

      if(strcmp(msg, "selectTrigger") == 0){
        uint32_t intTrig = receiveWordSocket(new_socket);

        selectTrigger(intTrig);

        sprintf(replyStr, "%s %u", "[SERVER] Trigger enable: ", intTrig);
        sendSocket(new_socket, replyStr);
			}

      if(strcmp(msg, "configureTestUnit") == 0){
        uint32_t tuCfg = receiveWordSocket(new_socket);
      	char testUnitCfg = ((tuCfg&0x300)>>8);
      	char testUnitEn  = ((tuCfg&0x2)>>1);

        configureTestUnit(tuCfg);

        sprintf(replyStr, "%s %x %u", "[SERVER] Test Unit status: ", \
                  testUnitCfg, testUnitEn);
        sendSocket(new_socket, replyStr);
			}

      if(strcmp(msg, "get event") == 0){

        GetEvent(new_socket);
      }
		}

		bzero(msg, sizeof(msg));

	}
	pthread_exit(NULL);
}
