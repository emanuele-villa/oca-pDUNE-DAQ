#include <stdio.h>
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

#include "hps_0.h"
#include "user_avalon_fifo_regs.h"
#include "lowlevelDriversFPGA.h"
#include "highlevelDriversFPGA.h"
#include "server_function.h"
#include "server.h"

struct fpgaAddresses baseAddr;

int main(int argc, char *argv[]){
  baseAddr.verbose = 2; //@todo pass the verbose as argument
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
  //Base address of the RegisterArray address
  baseAddr.fpgaRegAddr = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + REGADDR_PIO_BASE) & (unsigned long)(HW_REGS_MASK)));
  //Base address of the RegisterArray readback
  baseAddr.fpgaRegCont = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + REGCONTENT_PIO_BASE) & (unsigned long)(HW_REGS_MASK)));

  //Base addresses of the Data and CSR of the CONFIG FIFO
  baseAddr.configFifo = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_HPS_TO_FPGA_IN_BASE) & (unsigned long)(HW_REGS_MASK)));
  baseAddr.configFifoCsr = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_HPS_TO_FPGA_IN_CSR_BASE) & (unsigned long)(HW_REGS_MASK)));
  baseAddr.configFifoLevel  = baseAddr.configFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_LEVEL_REG;
  baseAddr.configFifoStatus = baseAddr.configFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_STATUS_REG;

  //Base addresses of the Data and CSR of the HK FIFO
  baseAddr.hkFifo = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_FPGA_TO_HPS_OUT_BASE) & (unsigned long)(HW_REGS_MASK)));
  baseAddr.hkFifoCsr = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long)(ALT_LWFPGASLVS_OFST + FIFO_FPGA_TO_HPS_OUT_CSR_BASE) & (unsigned long)(HW_REGS_MASK)));
  baseAddr.hkFifoLevel  = baseAddr.hkFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_LEVEL_REG;
  baseAddr.hkFifoStatus = baseAddr.hkFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_STATUS_REG;

  //Base addresses of the Data and CSR of the FAST FIFO
  baseAddr.FastFifo = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long )(ALT_LWFPGASLVS_OFST + FAST_FIFO_FPGA_TO_HPS_OUT_BASE) & (unsigned long)(HW_REGS_MASK)));
  baseAddr.FastFifoCsr = (uint32_t*)((unsigned long)baseAddr.virtual_base + ((unsigned long )(ALT_LWFPGASLVS_OFST + FAST_FIFO_FPGA_TO_HPS_OUT_CSR_BASE) & (unsigned long)(HW_REGS_MASK)));
  baseAddr.FastFifoLevel  = baseAddr.FastFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_LEVEL_REG;
  baseAddr.FastFifoStatus = baseAddr.FastFifoCsr + (unsigned long)ALTERA_AVALON_FIFO_STATUS_REG;

  //@todo Create a dedicated thread infrastructure
  //@todo e.g., add a semaphore to use the shared resources
  //printf("Creating threads...\n");
  //pthread_t threads;
  //pthread_create(&threads, NULL, receiver_comandi, argv[1]);
  //pthread_create(&threads, NULL, receiver_slow_control, argv[2]);

  //Connect to the socket and loop forever to receive commands
  receiver_comandi(argv[1]);

  //while(1){
  //  //FIX ME: cosi'?
  //}

  //pthread_join(threads, 0);
  return 0;
}
