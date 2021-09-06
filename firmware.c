//************************* SEZIONE DICHIARATIVA *************************//

#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "user_avalon_fifo_regs.h"
#include "user_avalon_fifo_util.h"
#include "hps_0.h"

#define HW_REGS_BASE ( ALT_STM_OFST )		// Physical base address: 0xFC000000
#define HW_REGS_SPAN ( 0x04000000 )			// Span Physical address: 64 MB
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )


//************************* SEZIONE DI MAPPING *************************//

int main() {

	extern void *virtual_base;			// Indirizzo base dell'area di memoria virtuale (variabile globale).
	int error;							// Flag per la segnalazione di errori.
	int fd;								// File descriptor degli indirizzi fisici.
	char op;							// Tipo di operazione che si vuole compiere sulla FIFO (lettura/scrittura).
	int num_el;							// Numero di scritture consecutive della FIFO che si vogliono compiere.
	int el;								// Indice del numero di scritture della FIFO.
	uint32_t new_value;					// Valore singolo con il quale verrà caricata la FIFO.
	uint32_t v_data_array[1021];		// Array di 1021 elementi di tipo uint32_t, da riempire in scrittura.
	uint32_t p_data_array[1021];		// Array di 1021 elementi di tipo uint32_t, da svuotare in lettura.
	uint32_t f_data_array[4093];		// Array di 4093 elementi di tipo uint32_t, da svuotare in lettura.
	uint32_t value_read;				// Valore in uscita dalla FIFO.
	uint32_t fifo_level;				// Livello di riempimento della FIFO.
	uint32_t fifo_full;					// bit di "full" della FIFO.
	uint32_t fifo_empty;				// bit di "empty" della FIFO.
	uint32_t fifo_almostfull;			// bit di "almostfull" della FIFO.
	uint32_t fifo_almostempty;			// bit di "almostempty" della FIFO.
	uint32_t almostfull_setting;		// livello di "almostfull" della FIFO.
	uint32_t almostempty_setting;		// livello di "almostempty" della FIFO.

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



	//************************* SEZIONE PRINCIPALE *************************//
	printf("\n");
	error = InitFifo(CONFIG_FIFO, 1, 1020);		// Inizializza la CONFIG_FIFO con almostempty=1 e almostfull=1020.
	printf("InitFifo error = %d\n", error);
	error = InitFifo(HK_FIFO, 1, 1020);			// Inizializza la HK_FIFO con almostempty=1 e almostfull=1020.
	printf("InitFifo error = %d\n", error);
	error = InitFifo(DATA_FIFO, 1, 4086);		// Inizializza la DATA_FIFO con almostempty=1 e almostfull=4086.
	printf("InitFifo error = %d\n", error);
	
	error = ShowStatusFifo(CONFIG_FIFO);		// Mostra lo stato attuale della CONFIG_FIFO.
	printf("ShowStatusFifo error = %d\n", error);
	error = ShowStatusFifo(HK_FIFO);			// Mostra lo stato attuale della HK_FIFO.
	printf("ShowStatusFifo error = %d\n", error);
	error = ShowStatusFifo(DATA_FIFO);			// Mostra lo stato attuale della DATA_FIFO.
	printf("ShowStatusFifo error = %d\n", error);

	while(1)
	{
		printf("\n");
		printf("Quale operazione vuoi effettuare?  r=read FIFO, p=readburst FIFO, w=write FIFO, v=writeburst FIFO, t=test lettura fifo\n");
		scanf("%c",&op);												// Scelta del tipo di operazione da compiere (lettura/scrittura).

		if (op=='w'){
			printf("\n");
			printf("Quanti valori vuoi inserire?  1=min, %d=max\n",FIFO_HPS_TO_FPGA_IN_CSR_FIFO_DEPTH - 3);		// In caso di scrittura, scegli quanti valori inserire.
			scanf("%d",&num_el);

			for (el=1;el<=num_el;el++){
			printf("\n");
			printf("Valore numero %d:\n",el);
			scanf("%x",&new_value);										// Preleva da tastiera il valore a 32 bit da inserire.

			error = WriteFifo(CONFIG_FIFO, &new_value);					// Scrivi il dato nella FIFO.
			printf("WriteFifo error = %d\n", error);					// Se la FIFO è piena (1021/1021) error=2, altrimenti error=0.
			}
		
			printf("\n");
			error = StatusFifo(CONFIG_FIFO, &fifo_level, &fifo_full, &fifo_empty, &fifo_almostfull, &fifo_almostempty, &almostfull_setting, &almostempty_setting);	// Stampa a video lo stato della CONFIG_FIFO.
			printf("CONFIG_FIFO -  L: %d/%d, F: %d, E: %d, AF: %d, AE: %d", fifo_level, FIFO_HPS_TO_FPGA_IN_CSR_FIFO_DEPTH - 3, fifo_full, fifo_empty, fifo_almostfull, fifo_almostempty);

			printf("\n");
			printf("\n");
			printf("\n");
			scanf("%*c");
		}
		else if(op=='v'){
			printf("\n");
			printf("Quanti valori vuoi inserire?  1=min, 1021=max\n");		// In caso di burst, scegli quanti valori deve avere la raffica.
			scanf("%d",&num_el);
			
			for (el=0;el<num_el;el++){
				printf("Inserisci l'elemento numero %d: ",el+1);
				scanf("%x",&v_data_array[el]);
			}
			printf("Avvio burst dati in scrittura..\n");
			error = WriteFifoBurst(CONFIG_FIFO, v_data_array, num_el);		// Se si tenta di scrivere più valori di quelli che ne può contenere la FIFO error=2, altrimenti error=0;
			printf("WriteFifoBurst error = %d\n", error);
			
			printf("\n");
			error = StatusFifo(CONFIG_FIFO, &fifo_level, &fifo_full, &fifo_empty, &fifo_almostfull, &fifo_almostempty, &almostfull_setting, &almostempty_setting);	// Stampa a video lo stato della CONFIG_FIFO.
			printf("CONFIG_FIFO -  L: %d/%d, F: %d, E: %d, AF: %d, AE: %d", fifo_level, FIFO_HPS_TO_FPGA_IN_CSR_FIFO_DEPTH - 3, fifo_full, fifo_empty, fifo_almostfull, fifo_almostempty);

			printf("\n");
			printf("\n");
			printf("\n");
			scanf("%*c");
		}
		else if (op=='r'){										// In caso op=r, esegui la lettura dell'uscita della FIFO.
			scanf("%*c");
			printf("Quale fifo vuoi leggere?  h=HK_FIFO, d=DATA_FIFO\n");
			scanf("%c",&op);
			
			if (op=='h'){										// In caso op=h, esegui la lettura dell'uscita della HK_FIFO.
				error = ReadFifo(HK_FIFO, &value_read);
			
				if (error == 0){
					printf("0x%08x", value_read);
					error = StatusFifo(HK_FIFO, &fifo_level, &fifo_full, &fifo_empty, &fifo_almostfull, &fifo_almostempty, &almostfull_setting, &almostempty_setting);	// Stampa a video lo stato della HK_FIFO.
					printf("       -       L: %d/%d, F: %d, E: %d, AF: %d, AE: %d", fifo_level, FIFO_FPGA_TO_HPS_OUT_CSR_FIFO_DEPTH - 3, fifo_full, fifo_empty, fifo_almostfull, fifo_almostempty);
					
					printf("\n");
					printf("\n");
					printf("\n");
					scanf("%*c");
				}
				else{
					printf("ReadFifo error = %d\n", error);		// Se la FIFO è vuota error=2, altrimenti error=0;
					
					printf("\n");
					printf("\n");
					printf("\n");
					scanf("%*c");
				}
			}
			
			else if (op=='d'){									// In caso op=d, esegui la lettura dell'uscita della DATA_FIFO.
				error = ReadFifo(DATA_FIFO, &value_read);
			
				if (error == 0){
					printf("0x%08x", value_read);
					error = StatusFifo(DATA_FIFO, &fifo_level, &fifo_full, &fifo_empty, &fifo_almostfull, &fifo_almostempty, &almostfull_setting, &almostempty_setting);	// Stampa a video lo stato della HK_FIFO.
					printf("       -       L: %d/%d, F: %d, E: %d, AF: %d, AE: %d", fifo_level, FAST_FIFO_FPGA_TO_HPS_OUT_CSR_FIFO_DEPTH - 3, fifo_full, fifo_empty, fifo_almostfull, fifo_almostempty);
					
					printf("\n");
					printf("\n");
					printf("\n");
					scanf("%*c");
				}
				else{
					printf("ReadFifo error = %d\n", error);		// Se la FIFO è vuota error=2, altrimenti error=0;
					
					printf("\n");
					printf("\n");
					printf("\n");
					scanf("%*c");
				}
			}
			else
			printf("Operazione non valida\n");
			
		}
		else if (op=='p'){														// In caso op=r, esegui la lettura di un burst di dati dall'uscita della FIFO.
			scanf("%*c");
			printf("Quale fifo vuoi leggere?  h=HK_FIFO, d=DATA_FIFO\n");
			scanf("%c",&op);
			
			if (op=='h'){														// In caso op=h, esegui la lettura dell'uscita della HK_FIFO.
				printf("\n");
				printf("Quanti valori vuoi leggere?  1=min, 1021=max\n");		// In caso di burst, scegli quanti valori deve avere la raffica.
				scanf("%d",&num_el);
			
				printf("Avvio burst dati in lettura..\n");
				error = ReadFifoBurst(HK_FIFO, p_data_array, num_el);
			
				if (error == 0){
					printf("\n");
					printf("Valori caricati:\n");
				
					for (el=0;el<num_el;el++){
						printf("Elemento numero %d: ",el+1);
						printf("0x%08x\n", p_data_array[el]);
					}
				}
				else
					printf("ReadFifoBurst error = %d\n", error);			// Se si tenta di leggere più valori di quelli contenuti nella FIFO error=2, altrimenti error=0;
				
				printf("\n");
				error = StatusFifo(HK_FIFO, &fifo_level, &fifo_full, &fifo_empty, &fifo_almostfull, &fifo_almostempty, &almostfull_setting, &almostempty_setting);	// Stampa a video lo stato della CONFIG_FIFO.
				printf("HK_FIFO   -    L: %d/%d, F: %d, E: %d, AF: %d, AE: %d\n", fifo_level, FIFO_FPGA_TO_HPS_OUT_CSR_FIFO_DEPTH - 3, fifo_full, fifo_empty, fifo_almostfull, fifo_almostempty);
				printf("\n");
				
				printf("\n");
				printf("\n");
				printf("\n");
				scanf("%*c");
			}
			
			else if (op=='d'){														// In caso op=d, esegui la lettura dell'uscita della DATA_FIFO.
				printf("\n");
				error = StatusFifo(DATA_FIFO, &fifo_level, &fifo_full, &fifo_empty, &fifo_almostfull, &fifo_almostempty, &almostfull_setting, &almostempty_setting);	// Stampa a video lo stato della CONFIG_FIFO.
				printf("livello fifo: %d\n", fifo_level);
				printf("Quanti valori vuoi leggere?  1=min, 4093=max\n");
				// In caso di burst, scegli quanti valori deve avere la raffica.
				scanf("%d",&num_el);
			
				printf("Avvio burst dati in lettura..\n");
				error = ReadFifoBurst(DATA_FIFO, f_data_array, num_el);
			
				if (error == 0){
					printf("\n");
					printf("Valori caricati:\n");
				
					/*for (el=0;el<num_el;el++){
						printf("Elemento numero %d: ",el+1);
						printf("0x%08x\n", f_data_array[el]);
					}*/

					printf("primo elemento: %x\n", f_data_array[0]);
					printf("ultimo elemento: %x\n", f_data_array[num_el]);
				}
				else
					printf("ReadFifoBurst error = %d\n", error);			// Se si tenta di leggere più valori di quelli contenuti nella FIFO error=2, altrimenti error=0;
				
				printf("\n");
				error = StatusFifo(DATA_FIFO, &fifo_level, &fifo_full, &fifo_empty, &fifo_almostfull, &fifo_almostempty, &almostfull_setting, &almostempty_setting);	// Stampa a video lo stato della CONFIG_FIFO.
				printf("DATA_FIFO   -    L: %d/%d, F: %d, E: %d, AF: %d, AE: %d\n", fifo_level, FAST_FIFO_FPGA_TO_HPS_OUT_CSR_FIFO_DEPTH - 3, fifo_full, fifo_empty, fifo_almostfull, fifo_almostempty);
				printf("\n");
				printf("\n");
				printf("\n");
				printf("\n");
				scanf("%*c");
	
					
			}


			//printf("numero errori rilevati: %d\n", errors);
			
		}else if(op == 't'){

			printf("inizio lettura....\n");
			uint32_t msec = 0; 
			uint32_t trigger = 10000; 

			clock_t start = clock(), difference;
			uint32_t dati = 0; 
			uint32_t fifo_lette = 0; 


			do{

				difference = clock() - start;
				msec = difference * 1000 / CLOCKS_PER_SEC;
				error = StatusFifo(DATA_FIFO, &fifo_level, &fifo_full, &fifo_empty, &fifo_almostfull, &fifo_almostempty, &almostfull_setting, &almostempty_setting);	// Stampa a video lo stato della CONFIG_FIFO.
				printf("livello fifo: %d\n", fifo_level);
				error = ReadFifoBurst(DATA_FIFO, f_data_array, fifo_level);

				dati += fifo_level;
				fifo_lette++;

			}while(msec < trigger);

			printf("sono passati %d msecondi e ho letto %d dati da %d fifo\n", msec, dati, fifo_lette);

			//qui fa solo 20 letture e stampa ogni volta il livello della fifo

			/*for(int i = 0; i < 20; i++){

				error = StatusFifo(DATA_FIFO, &fifo_level, &fifo_full, &fifo_empty, &fifo_almostfull, &fifo_almostempty, &almostfull_setting, &almostempty_setting);	// Stampa a video lo stato della CONFIG_FIFO.
				printf("livello fifo: %d\n", fifo_level);

				error = ReadFifoBurst(DATA_FIFO, f_data_array, fifo_level);
				if(error != 0)
					printf("c'è stato un errore\n");
				else
					error = ShowStatusFifo(DATA_FIFO);	// Stampa a video lo stato della fifo.
				

			}*/



		}

}

	//************************* SEZIONE DI CHIUSURA *************************//

	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	close( fd );

	return( 0 );
}
