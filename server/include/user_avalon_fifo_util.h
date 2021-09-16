#ifndef USER_AVALON_FIFO_UTIL_H_
#define USER_AVALON_FIFO_UTIL_H_

// FIFO_TYPE disponibili
#define CONFIG_FIFO			 1		// MACRO per selezionare la FIFO per la ricezione dei dati di configurazione.
#define HK_FIFO				 2		// MACRO per selezionare la FIFO per la trasmissione dei dati di telemetria.
#define DATA_FIFO			 3		// MACRO per selezionare la FIFO per la trasmissione dei dati di payload.


#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>


// Dichiarazione di un set di funzioni per compiere operazioni sulle FIFO
int InitFifo(int FIFO_TYPE, uint32_t AE, uint32_t AF);								// Inizializzazione della FIFO.
int WriteFifo(int FIFO_TYPE, uint32_t *data);										// Scrittura della FIFO.
int WriteFifoBurst(int FIFO_TYPE, uint32_t *data, int length_burst);				// Scrittura della FIFO con un burst di dati.
int ReadFifo(int FIFO_TYPE, uint32_t *data);										// Lettura della FIFO.
int ReadFifoBurst(int FIFO_TYPE, uint32_t *data, int length_burst);					// Lettura di un burst di dati dalla FIFO.
int StatusFifo(int FIFO_TYPE, uint32_t *fifo_level, uint32_t *fifo_full, uint32_t *fifo_empty, uint32_t *fifo_almostfull, uint32_t *fifo_almostempty, uint32_t *almostfull_setting, uint32_t *almostempty_setting);		// Lettura stato della FIFO.
int ShowStatusFifo(int FIFO_TYPE);													// Stampa a video lo stato della FIFO.
int OverflowController(int FIFO_TYPE);												// Controllo del bit di overflow della FIFO.



#endif /* USER_AVALON_FIFO_UTIL_H_ */
