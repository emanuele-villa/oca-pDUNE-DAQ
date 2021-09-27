#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"


#include "hps_0.h"
#include "user_avalon_fifo_regs.h"
#include "user_avalon_fifo_util.h"
#include "user_register_array.h"
#include "server_function.h"
#include "server.h"


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
          regsContent[ii*2]   = receive_register_content(new_socket);
          regsContent[ii*2+1] = (uint32_t)ii;
        }

        Init(regsContent, 14);
			}

      if(strcmp(msg, "readReg") == 0){
        uint32_t regAddr = receive_register_content(new_socket);
        uint32_t regContent;

        printf("Send read request...\n");
        ReadReg(regAddr, &regContent);

        sprintf(replyStr, "%s %u: %08x", "[SERVER] Reg", regAddr, regContent);
        sendSocket(new_socket, replyStr);
      }

			if((strcmp(msg, "set delay")==0)||(strcmp(msg, "OverWriteDelay")==0)){
      	uint32_t delay = receive_register_content(new_socket);

				SetDelay(delay);

        sprintf(replyStr, "%s %d", "[SERVER] Delay: ", delay);
        sendSocket(new_socket, replyStr);
			}

			if(strcmp(msg, "set mode") == 0){
        uint32_t mode = receive_register_content(new_socket);
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
        uint32_t calib = receive_register_content(new_socket);

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
        uint32_t period = receive_register_content(new_socket);

        intTriggerPeriod(period);

        sprintf(replyStr, "%s %08u", "[SERVER] Trigger period: ", period);
        sendSocket(new_socket, replyStr);
			}

      if(strcmp(msg, "selectTrigger") == 0){
        uint32_t intTrig = receive_register_content(new_socket);

        selectTrigger(intTrig);

        sprintf(replyStr, "%s %u", "[SERVER] Trigger enable: ", intTrig);
        sendSocket(new_socket, replyStr);
			}

      if(strcmp(msg, "configureTestUnit") == 0){
        uint32_t tuCfg = receive_register_content(new_socket);
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


int main(int argc, char *argv[]){
  verbose = 2; //@todo pass the verbose as argument
  printf("Opening /dev/mem...\n");
	int fd;
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

  printf("Mapping FPGA resources...\n");
	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );
	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}

  printf("Computing base addresses...\n");
  //Base address of the RegisterArray address
  fpgaRegAddr = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + REGADDR_PIO_BASE) & (unsigned long)(HW_REGS_MASK));
  //Base address of the RegisterArray readback
  fpgaRegCont = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + REGCONTENT_PIO_BASE) & (unsigned long)(HW_REGS_MASK));

  //Base addresses of the Data and CSR of the CONFIG FIFO
  configFifo = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_HPS_TO_FPGA_IN_BASE) & (unsigned long)(HW_REGS_MASK));
  configFifoCsr = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_HPS_TO_FPGA_IN_CSR_BASE) & (unsigned long)(HW_REGS_MASK));
  configFifoLevel = configFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_LEVEL_REG;
  configFifoStatus = configFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_STATUS_REG;

  //Base addresses of the Data and CSR of the HK FIFO
  hkFifo = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_FPGA_TO_HPS_OUT_BASE) & (unsigned long)(HW_REGS_MASK));
  hkFifoCsr = virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_FPGA_TO_HPS_OUT_CSR_BASE) & (unsigned long)(HW_REGS_MASK));
  hkFifoLevel = hkFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_LEVEL_REG;
  hkFifoStatus = hkFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_STATUS_REG;

  //Base addresses of the Data and CSR of the FAST FIFO
  FastFifo = virtual_base + ((unsigned long )(ALT_LWFPGASLVS_OFST + FAST_FIFO_FPGA_TO_HPS_OUT_BASE) & (unsigned long)(HW_REGS_MASK));
  FastFifoCsr = virtual_base + ((unsigned long )(ALT_LWFPGASLVS_OFST + FAST_FIFO_FPGA_TO_HPS_OUT_CSR_BASE) & (unsigned long)(HW_REGS_MASK));
  FastFifoLevel = FastFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_LEVEL_REG;
  FastFifoStatus = FastFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_STATUS_REG;

  printf("Creating threads...\n");
	pthread_t threads;
	pthread_create(&threads, NULL, receiver_comandi, argv[1]);
	pthread_create(&threads, NULL, receiver_slow_control, argv[2]);

	while(1){

	}


	pthread_join(threads, 0);
	return 0;
}
