#include "daqclient.h"
#include <stdio.h>
#include <unistd.h>

#include <ctime>
#include <iostream>

#include "utility.h"

const char* addressdaq = "localhost";
const int portdaq = 10000;

int verbosity=0;

int main(int argc, char *argv[]) {
  //  printf("%d\n", argc);
  if (argc < 2) {
    printf("Usage:\n\t%s <0: calibration, 1: beam> [runnum]\n", argv[0]);
    return 0;
  }
  int beam = atoi(argv[1]);
  unsigned int runnum = 0;
  if (argc == 3) {
    runnum = atoi(argv[2]);
    printf("%d\n", runnum);
    if (runnum > 65535) {
      printf("Error: Run number %d exceeds maximum value of 65535 (16-bit limit)\n", runnum);
      return 1;
    }
  }

  //------------------------------------------------
  char readBack[LEN]="";
  static const int length=16;
  char* command_string=new char[2*length+1];
  //------------------------------------------------

  if (verbosity>0) {
    printf("--------------------------------\n");
    printf("startOCA:\n");
    printf("--------------------------------\n");
  }
  
  daqclient* daq = new daqclient(addressdaq, portdaq, verbosity);
  daq->SetCmdLenght(32);

  std::time_t timeNow = std::time(nullptr);
  uint32_t ts = (uint32_t)timeNow;
  uint32_t tsReord = (ts & 0x000000FF) << 24 | (ts & 0x0000FF00) << 8 | (ts & 0x00FF0000) >> 8 | (ts & 0xFF000000) >> 24;
  //std::cout << std::hex << timeNow << ": ts " << ts << " reordered " << tsReord << std::endl;

  std::cout << std::asctime(std::gmtime(&timeNow))
            << timeNow << " seconds since the Epoch\n";

  //71616b23
  //uint32_t start[4] = {0x080080FF, 0x01001500, 0x010000EE, 0x236B6171};
  // Encoding layout (little-endian bytes of start[1]):
  //   byte0 = run_hi (bits 15..8 of run number)
  //   byte1 = run_lo (bits 7..0 of run number)
  //   byte2 = 0x00 (reserved)
  //   byte3 = run type code:
  //           0x02 -> "0002" (BEAM)
  //           0x04 -> "0004" (CAL)
  // Server builds cmdgroup[1] = b0 b1 b2 b3 (hex), then:
  //   runnum  = first 4 hex chars (b0 b1) -> big-endian 16-bit
  //   runtype = last 4 hex chars  (b2 b3)
  // This replaces earlier misuse of bit24. Beam bit25 OR kept for backward compatibility (has no effect for 0x02 / 0x04).
  uint8_t run_hi = (runnum >> 8) & 0xFF;
  uint8_t run_lo = runnum & 0xFF;
  uint8_t run_type_code = (beam ? 0x02 : 0x04); // 1->BEAM, 0->CAL
  uint32_t start_word1 = (uint32_t)run_type_code << 24 | (uint32_t)0x00 << 16 | (uint32_t)run_lo << 8 | run_hi;
  // Preserve previous beam-bit behavior (bit25) if firmware expects it
  start_word1 |= ((beam & 0x1) << 25);
  uint32_t start[4] = {0x080080FF, start_word1, 0x010000EE, tsReord};
  // Debug (uncomment if needed):
  // printf("Encoded run=%u type=%s word1=0x%08X (hi=%02X lo=%02X)\n", runnum, beam?"BEAM":"CAL", start_word1, run_hi, run_lo);
  daq->Send((void*)start, 4*sizeof(uint32_t));
  daq->ReceiveCmdReply(readBack);//is blocking and this is wanted
  hex2string(readBack,length,command_string);
  printf("%s) Read from DAQ: %s\n", __METHOD_NAME__, command_string);
      
  return 0;
}
