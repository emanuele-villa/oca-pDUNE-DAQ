#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include "user_avalon_fifo_regs.h"
#include "user_avalon_fifo_util.h"
#include "hps_0.h"
#include "user_register_array.h"



extern void *virtual_base;			// Indirizzo base dell'area di memoria virtuale (variabile globale).

uint32_t data_array[4093];		// Array di 4093 elementi di tipo uint32_t, da svuotare in lettura.
uint32_t level;				// Livello di riempimento della FIFO.
uint32_t full;					// bit di "full" della FIFO.
uint32_t empty;				// bit di "empty" della FIFO.
uint32_t almostfull;			// bit di "almostfull" della FIFO.
uint32_t almostempty;			// bit di "almostempty" della FIFO.
uint32_t almostfull_setting_;		// livello di "almostfull" della FIFO.
uint32_t almostempty_setting_;		// livello di "almostempty" della FIFO.




uint32_t crc_init(void)
{
    return 0x46af6449;
}

uint32_t crc_update(uint32_t crc, const void *data, size_t data_len)
{
    const unsigned char *d = (const unsigned char *)data;
    unsigned int i;
    bool bit;
    unsigned char c;
    puts("crc");
    while (data_len--) {
        c = d[data_len];
        for (i = 0; i < 8; i++) {
            bit = crc & 0x80000000;
            crc = (crc << 1) | ((c >> (7 - i)) & 0x01);
            if (bit) {
                crc ^= 0x04c11db7;
            }
        }
        crc &= 0xffffffff;
    }
    return crc & 0xffffffff;
}

uint32_t crc_finalize(uint32_t crc)
{
    unsigned int i;
    bool bit;

    for (i = 0; i < 32; i++) {
        bit = crc & 0x80000000;
        crc <<= 1;
        if (bit) {
            crc ^= 0x04c11db7;
        }
    }
    return crc & 0xffffffff;
}

void ReadReg(int regAddr, uint32_t *data){
	//Write the address of the register to be read
	//*fpgaRegAddr = regAddr;

	//Read the register content
	//*data = *fpgaRegCont;
}

int write_register(uint16_t reg, uint32_t *value){

	bool res[6];
	for(int j = 0; j < 4; j++){

		res[j] = parity8((uint8_t)((*value >> 8 * j) & 0x000000ff));
		
	}


	for(int j = 0; j < 2; j++){

		res[j + 4] = parity8((uint8_t)((reg >> 8 * j) & 0x000000ff));
	}

	uint8_t result = 0; 
	for(int i = 5; i > 0; i--){

		result |= res[i];
		result <<= 1;

	}

	result |= res[0];


	uint32_t address, parity, crc;
	crc = crc_init();
	parity = result; 
	parity <<= 24; 
	address = reg;

	uint32_t packet[8]; 
	packet[0] = 0xAA55CA5A;
	packet[1] = 7;
	packet[2] = 0;
	packet[3] = 0x4EADE500;
	packet[4] = *value;
	packet[5] = parity | address;
	packet[6] = 0xBADC0FEE;

	printf("%x\n", packet[5]);
	printf("%x\n", packet[4]);

	for(int i = 0; i < 7; i++){

		if(i == 0 || i == 1 || i == 6)
			continue;
		crc = crc_update(crc, &packet[i], sizeof(uint32_t));
	}

	crc = crc_finalize(crc);

	packet[7] = crc;

	puts("faccio write burst");
	int ret = WriteFifoBurst(CONFIG_FIFO, packet, 8);
	puts("ho fatto write burst");
	//uint32_t output[8];
	//ret = StatusFifo(HK_FIFO, &level, &full, &empty, &almostfull, &almostempty, &almostfull_setting_, &almostempty_setting_);
	//ret = ReadFifoBurst(HK_FIFO, data_array, level);
	
	return(0);
}


bool parity8(uint8_t data){

	bool b;
	bool arr[8];

	for(int i  = 0; i < 8; i++){

		arr[i] = data & 1;
		data >>= 1;
		
	}

	b = arr[0] ^ arr[1] ^arr[2] ^ arr[3] ^ arr[4] ^ arr[5] ^ arr[6] ^ arr[7];

	return b;
}