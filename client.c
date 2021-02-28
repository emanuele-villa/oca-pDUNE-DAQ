#include <stdio.h>
#include <stdlib.h>
#include<time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

#define USER_IO_DIR     (0x01000000)
#define BIT_LED         (0x01000000)
#define BUTTON_MASK     (0x02000000)
void *virtual_base;
int fd;
uint32_t  scan_input;

//variabili per sincronizzazione threads
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pulsante_premuto = PTHREAD_COND_INITIALIZER;


int sock;
struct sockaddr_in server_addr;
struct hostent *server;


int accendi_led(){

	//accende il led per cinque secondi
	clock_t start;
	start = clock();
	start = (double)(start) / CLOCKS_PER_SEC;
	printf("accendo il led\n");
	while(((double) clock() / CLOCKS_PER_SEC) < start + 5){

		alt_setbits_word( ( virtual_base + ( ( uint32_t )( ALT_GPIO1_SWPORTA_DR_ADDR ) & ( uint32_t )( HW_REGS_MASK ) ) ), BIT_LED );

	}
	return( 0 );
}

int spegni_led(){

	//spegne il led per cinque secondi
	clock_t start;
	start = clock();
	start = (double)(start) / CLOCKS_PER_SEC;
	printf("accendo il led\n");
	while(((double) clock() / CLOCKS_PER_SEC) < start + 5){

		alt_clrbits_word( ( virtual_base + ( ( uint32_t )( ALT_GPIO1_SWPORTA_DR_ADDR ) & ( uint32_t )( HW_REGS_MASK ) ) ), BIT_LED );

	}
	return( 0 );
}


//threads sender e receiver 
void *sender(){

	char msg[256];
	int n = 0;
	while(1){

		pthread_mutex_lock(&mtx);
		while(!(~scan_input&BUTTON_MASK)){

			pthread_cond_wait(&pulsante_premuto, &mtx);
		}
		strcpy(msg, "[CLIENT] ciao");
		if(n = write(sock, msg, strlen(msg)) < 0){

			fprintf(stderr, "errore scrittura\n");
		}	
		bzero(msg, sizeof(msg));
		pthread_mutex_unlock(&mtx);
		sleep(1);
	}

	pthread_exit(NULL);

}

void *receiver(){

	int n = 0;
	char msg[4096];
	while(1){

		//si mette in attesa del messaggio 
		if(n = read(sock, msg, sizeof(msg)) < 0){

			fprintf(stderr, "[CLIENT] errore lettura\n");
		}else{
			printf("ho letto %d bytes\n", n);
			printf("%s\n", msg);
			if(strcmp(msg, "accendi") == 0){

				accendi_led();
			}else if(strcmp(msg, "spegni") == 0){

				spegni_led();
			}
		}

		bzero(msg, sizeof(msg));
	}
	pthread_exit(NULL);
}

int main(int argc, char **argv){

	pthread_t threads;
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

	//imposta il led come output
	alt_setbits_word( ( virtual_base + ( ( uint32_t )( ALT_GPIO1_SWPORTA_DDR_ADDR ) & ( uint32_t )( HW_REGS_MASK ) ) ), USER_IO_DIR );	
	if(argc < 3){

		fprintf(stderr, "errore, non abbastanza argomenti\n");
		exit(0);
	}


	int port_no = atoi(argv[2]);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	server = gethostbyname(argv[1]);
	if(sock < 0){

		fprintf(stderr, "errore creazione socket\n");
	}else{

		printf("socket creato correttamente\n");
	}
	if(server == NULL){

		fprintf(stderr, "non c'è questo host\n");
		exit(EXIT_FAILURE);
	}

	bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&server_addr.sin_addr.s_addr,
         server->h_length);
    server_addr.sin_port = htons(port_no);

    if(connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){

			fprintf(stderr, "errore connessione");
		}else{

			printf("connesso\n");
			
		}


    pthread_create(&threads, NULL, sender, NULL);
    pthread_create(&threads, NULL, receiver, NULL);
    while(1){

    	//rileva la pressione del pulsante ed in caso svelgia il thread sender
    	pthread_mutex_lock(&mtx);

    	scan_input = alt_read_word( ( virtual_base + ( ( uint32_t )(  ALT_GPIO1_EXT_PORTA_ADDR ) & ( uint32_t )( HW_REGS_MASK ) ) ) );				
		if(~scan_input&BUTTON_MASK){
			
			pthread_cond_signal(&pulsante_premuto);
		}	

		pthread_mutex_unlock(&mtx);
    }

   	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		exit(0);
	}	
	close( fd );
	close(sock);
	return(0);
}