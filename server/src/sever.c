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
#include "server_function.h" //funzioni dei comandi che rispondono al client 
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"

#define HW_REGS_BASE ( ALT_STM_OFST )		// Physical base address: 0xFC000000
#define HW_REGS_SPAN ( 0x04000000 )			// Span Physical address: 64 MB
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

#define TRUE             1
#define FALSE            0


void *receiver_slow_control(void *args){

  	int len, rc, on = 1;
	int listen_sd = -1, new_sd = -1;
	char buffer[80];
	struct sockaddr_in addr;
	int timeout; 
	struct pollfd fds[200];
	int nfds = 1, current_size = 0, i, j;
	int end_server = 0, close_conn = 0; 
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
            new_sd = accept(listen_sd, (struct sockaddr *)&cliaddr, &addrlen);
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
	
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if(sock < 0){

		perror("errore creazione socket\n");
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(porta);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sock, (struct sockaddr *) &server_addr , sizeof(server_addr)) < 0){

		perror("errore nel bind\n");
		exit(EXIT_FAILURE);
	}else{

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

	}else{

		printf("connessione riuscita al socket comandi principali: socket %d\n", new_socket);
		close(sock);
			
	}
	
	while(1){

		char msg[256];
		if(read(new_socket, msg, sizeof(msg)) < 0){

			perror("errore nella read\n");
		}else{

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
		}

		bzero(msg, sizeof(msg));

	}
	pthread_exit(NULL);
}


int main(int argc, char *argv[]){

	//MAPPING
	extern void *virtual_base;
	int fd;
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );

	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}


	pthread_t threads;
	pthread_create(&threads, NULL, receiver_comandi, argv[1]);
	pthread_create(&threads, NULL, receiver_slow_control, argv[2]);

	while(1){

	}


	pthread_join(threads, 0);
	return 0;
}
