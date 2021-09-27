#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

#include "user_register_array.h"
#include "user_avalon_fifo_util.h"
#include "server.h"


uint32_t crc_init(){
    return 0x46af6449;
}

uint32_t crc_update(uint32_t crc, const void *data, size_t data_len){
    const unsigned char *d = (const unsigned char *)data;
    unsigned int i;
    bool bit;
    unsigned char c;

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

uint32_t crc_finalize(uint32_t crc){
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

//Compute the parity of an incoming unsigned 8-bit
bool parity8(uint8_t dataIn){
	//bool b;
	//bool arr[8];
	//for(int i = 0; i < 8; i++){
	//	arr[i] = data & 1;
	//	data >>= 1;
	//}
	//b = arr[0] ^ arr[1] ^arr[2] ^ arr[3] ^ arr[4] ^ arr[5] ^ arr[6] ^ arr[7];
	//return b;

  //gcc specific
  return __builtin_parity(dataIn&0x000000FF);
}

//Compute 4 parity bits of an incoming unsigned 32-bit
uint8_t parity32(uint32_t dataIn){
  uint8_t parity = 0;
  uint8_t nibbles;
  for (int ii = 0; ii < 4; ii++){
    nibbles = (dataIn >> ii*8) & 0xFF;
    parity = parity | ((parity8(nibbles))<<ii);
  }
  return parity;
}

// Leggi Contenuto registro da PIO
void ReadReg(int regAddr, uint32_t *data){
	//Write the address of the register to be read
	*fpgaRegAddr = regAddr;

	//Read the register content
	*data = *fpgaRegCont;

  if(verbose > 0){
    printf("Register addr: %d - content: %08x\n", regAddr, *data);
  }
}

//Write a single register
void singleWriteReg(uint32_t regAddr, uint32_t regContent){
  uint32_t singleWrite[1];
  singleWrite[1] = regContent;
  singleWrite[0] = regAddr;
  writeReg(singleWrite, 2);
}

//Write registers via the HPS2FPGA FIFO
int writeReg(uint32_t * pktContent, int pktLen){
  uint32_t packet[pktLen+6];
  uint8_t parityMsb, parityLsb;
  uint32_t pktCrc;

  //Create the packet header
  pktCrc = crc_init();
  packet[0] = REG_SOP;
  packet[1] = (uint32_t)pktLen+5;
  packet[2] = 0; //@todo add FW version
  pktCrc = crc_update(pktCrc, &packet[2], sizeof(uint32_t));
  packet[3] = REG_HDR1;
  pktCrc = crc_update(pktCrc, &packet[3], sizeof(uint32_t));

  //Create the packet body
  for(int ii=0; ii<pktLen; ii=ii+2){
    parityLsb = parity32(*(pktContent+ii));
    parityMsb = parity32(*(pktContent+ii+1));
    packet[ii+4] = *(pktContent+ii);
    packet[ii+5] = ((parityMsb<<28)&0xF0000000) || ((parityLsb<<24)&0x0F000000) || *(pktContent+ii+1);
    pktCrc = crc_update(pktCrc, &packet[ii+4], sizeof(uint32_t));
    pktCrc = crc_update(pktCrc, &packet[ii+5], sizeof(uint32_t));
  }

  //Create the packet footer
  packet[pktLen+4] = REG_EOP;
  pktCrc = crc_finalize(pktCrc);
  packet[pktLen+5] = pktCrc;

  if (verbose > 1){
    printf("Packet Content:\n");
    for (int jj=0; jj<pktLen+5;jj++){
      printf("%08x\n", packet[jj]);
    }
  }

  //Send the packet
	WriteFifoBurst(CONFIG_FIFO, packet, pktLen+5);

	return(0);
}
