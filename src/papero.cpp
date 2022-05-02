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
#include "hpsServer.h"

hpsServer* hpsSrv = nullptr;

int main(int argc, char *argv[]){
  std::cout<<"hash="<<GIT_HASH<<", time="<<COMPILE_TIME<<", branch="<<GIT_BRANCH<<std::endl;
  
  if (argc < 3) {
    printf("Usage:\n\tPAPERO <socket port> <verbosity level>\n");
    return 0;
  }

  //---- Debug of int/double/bool dimensions -----------------------------------
  /* sleep(3); */

  /* int pippoint[2] = { 10, 11}; */
  /* double pippodouble[2] = { 4.3, 5.4}; */
  /* bool pippobool[2] {true, false}; */
  /* printf("int: %p %p\n", &pippoint[0], &pippoint[1]); */
  /* printf("double: %p %p\n", &pippodouble[0], &pippodouble[1]); */
  /* printf("bool: %p %p\n", &pippobool[0], &pippobool[1]); */

  /* printf("%d %d\n", *(&pippoint[0]), *(&pippoint[0]+1)); */
  /* printf("%d %d\n", *(&pippoint[0]), *((int*)(((char*)&pippoint[0])+1))); */
  //----------------------------------------------------------------------------
  

  //Connect to the socket and loop forever to receive commands
  printf("Creating a server socket...\n");
  hpsSrv = new hpsServer(atoi(argv[1]), atoi(argv[2]));

  //Accept client connections and receive commands, until client is closed
  while (1) {
    hpsSrv->ListenCmd();
  }
  //Everything done, close the socket
  if (hpsSrv) delete hpsSrv;

  return 0;
}
