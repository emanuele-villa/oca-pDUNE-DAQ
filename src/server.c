#include <cstdio>
//#include <stdlib.h>
//#include <string.h>
#include <unistd.h>
//#include <pthread.h>
//#include <netinet/in.h>
#include <fcntl.h>
//#include <sys/poll.h>
//#include <sys/ioctl.h>
#include <sys/mman.h>
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include <iostream>

#include "hps_0.h"
#include "user_avalon_fifo_regs.h"
#include "lowlevelDriversFPGA.h"
#include "highlevelDriversFPGA.h"
#include "hpsServer.h"
#include "server.h"

struct fpgaAddresses baseAddr;
uint32_t kGwV = 0;
hpsServer* hpsSrv = nullptr;

int main(int argc, char *argv[]){
  std::cout<<"hash="<<GIT_HASH<<", time="<<COMPILE_TIME<<", branch="<<GIT_BRANCH<<std::endl;
  
  if (argc < 3) {
    printf("Usage:\n\tPAPERO <socket port> <verbosity level>\n");
    return 0;
  }

  baseAddr.verbose = atoi(argv[2]);
  printf("Opening /dev/mem...\n");
	int fd;
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

  printf("Mapping FPGA resources...\n");
	baseAddr.virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );
	if( baseAddr.virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}

  printf("Computing base addresses...\n");
  //the pointer arithmetics done below relies on the type (void*, char*, int*, etc...) of the pointers:
  //if the pointer is void* summing 1 (pointer + 1) means actually summing 4 (since the compiler knows that you have to move of 4 bytes)
  //if the pointer is char* summing 1 (pointer + 1) means actually summing 1 (since the compiler knows that you have to move of 1 bytes)
  //this arithmetics bust be coompatible with the offset defined

  //Base address of the RegisterArray address
  // the shifts are in units of bytes ----
  baseAddr.fpgaRegAddr = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + REGADDR_PIO_BASE) & (unsigned long)(HW_REGS_MASK)));
  //Base address of the RegisterArray readback
  baseAddr.fpgaRegCont = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + REGCONTENT_PIO_BASE) & (unsigned long)(HW_REGS_MASK)));
  //--------------------------------------

  //Base addresses of the Data and CSR of the CONFIG FIFO
  // the shifts are in units of bytes ----
  baseAddr.configFifo = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_HPS_TO_FPGA_IN_BASE) & (unsigned long)(HW_REGS_MASK)));
  baseAddr.configFifoCsr = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_HPS_TO_FPGA_IN_CSR_BASE) & (unsigned long)(HW_REGS_MASK)));
  //--------------------------------------
  // the shifts are in units of 4-bytes --
  baseAddr.configFifoLevel  = baseAddr.configFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_LEVEL_REG;
  baseAddr.configFifoStatus = baseAddr.configFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_STATUS_REG;
  //--------------------------------------

  //Base addresses of the Data and CSR of the HK FIFO
  // the shifts are in units of bytes ----
  baseAddr.hkFifo = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_FPGA_TO_HPS_OUT_BASE) & (unsigned long)(HW_REGS_MASK)));
  baseAddr.hkFifoCsr = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_FPGA_TO_HPS_OUT_CSR_BASE) & (unsigned long)(HW_REGS_MASK)));
  //--------------------------------------
  // the shifts are in units of 4-bytes --
  baseAddr.hkFifoLevel  = baseAddr.hkFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_LEVEL_REG;
  baseAddr.hkFifoStatus = baseAddr.hkFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_STATUS_REG;

  //Base addresses of the Data and CSR of the FAST FIFO
  // the shifts are in units of bytes ----
  baseAddr.FastFifo = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long )(ALT_LWFPGASLVS_OFST + FAST_FIFO_FPGA_TO_HPS_OUT_BASE) & (unsigned long)(HW_REGS_MASK)));
  baseAddr.FastFifoCsr = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long )(ALT_LWFPGASLVS_OFST + FAST_FIFO_FPGA_TO_HPS_OUT_CSR_BASE) & (unsigned long)(HW_REGS_MASK)));
  //--------------------------------------
  // the shifts are in units of 4-bytes --
  baseAddr.FastFifoLevel  = baseAddr.FastFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_LEVEL_REG;
  baseAddr.FastFifoStatus = baseAddr.FastFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_STATUS_REG;
  //--------------------------------------

  //---- only for debug -------------------------------------------------------
  /* sleep(3); */

  /* int pippoint[2] = { 10, 11}; */
  /* double pippodouble[2] = { 4.3, 5.4}; */
  /* bool pippobool[2] {true, false}; */
  /* printf("int: %p %p\n", &pippoint[0], &pippoint[1]); */
  /* printf("double: %p %p\n", &pippodouble[0], &pippodouble[1]); */
  /* printf("bool: %p %p\n", &pippobool[0], &pippobool[1]); */

  /* printf("%d %d\n", *(&pippoint[0]), *(&pippoint[0]+1)); */
  /* printf("%d %d\n", *(&pippoint[0]), *((int*)(((char*)&pippoint[0])+1))); */
    //---------------------------------------------------------------------------
  uint32_t piumone = 0;
  ReadReg(rGW_VER, &kGwV);
  ReadReg(rPIUMONE, &piumone);
  printf("\n/*--- GateWare SHA: %08x ----------------------*/\n", kGwV);
  printf("/*--- Piumone (it must be 0xC1A0C1A0): %08x ---*/\n\n", piumone);
  InitFifo(CONFIG_FIFO, 3, 1000, 0);
  InitFifo(HK_FIFO, 3, 1000, 0);
  InitFifo(DATA_FIFO, 646, 3442, 0);
  /* ShowStatusFifo(CONFIG_FIFO); */
  /* ShowStatusFifo(HK_FIFO); */
  /* ShowStatusFifo(DATA_FIFO); */

  //Connect to the socket and loop forever to receive commands
  printf("Creating a server socket...\n");
  hpsSrv = new hpsServer(atoi(argv[1]), baseAddr.verbose);

  //Accept client connections and receive commands, until client is closed
  while (1) {
    hpsSrv->ListenCmd();
  }
  //Everything done, close the socket
  if (hpsSrv) delete hpsSrv;

  return 0;
}
