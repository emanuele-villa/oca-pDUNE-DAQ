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
  // Goal: DAQ server forms hex_string (little-endian bytes). It then takes cmdgroup[1]=hex[8:16] and runnum = first 4 chars.
  // Original 8-bit case: start[1]=0x01001500 -> bytes LE: 00 15 00 01 -> cmdgroup[1]="00150001" -> runnum hex "0015".
  // We want for arbitrary 16-bit runnum R (0..65535): cmdgroup[1][:4] == R as 4-hex (zero padded).
  // Let runnum bytes: hi = (R>>8)&0xFF, lo = R & 0xFF.
  // Need LE bytes b0 b1 b2 b3 such that cmdgroup[1] = b0 b1 b2 b3 hex and b0b1 == hi lo (big-endian 16-bit hex representation).
  // Therefore choose b0=hi, b1=lo. We must also set bit24 (0x01000000) => this is byte b3 having bit0x01.
  // Keep original layout leaving b2=0x00 (reserved), b3=0x01 plus beam bit later at bit25.
  // LE bytes -> word = (b3<<24)|(b2<<16)|(b1<<8)|b0 = 0x01000000 | (lo<<8) | hi.
  // After beam OR: adds (beam&1)<<25.
  uint8_t run_hi = (runnum >> 8) & 0xFF;
  uint8_t run_lo = runnum & 0xFF;
  uint32_t start_word1 = 0x01000000 | (uint32_t(run_lo) << 8) | uint32_t(run_hi);
  start_word1 |= ((beam & 0x1) << 25);
  uint32_t start[4] = {0x080080FF, start_word1, 0x010000EE, tsReord};
  // Debug (optional): printf("Encoded run %u -> word1 0x%08X (hi=%02X lo=%02X)\n", runnum, start_word1, run_hi, run_lo);
  daq->Send((void*)start, 4*sizeof(uint32_t));
  daq->ReceiveCmdReply(readBack);//is blocking and this is wanted
  hex2string(readBack,length,command_string);
  printf("%s) Read from DAQ: %s\n", __METHOD_NAME__, command_string);
      
  return 0;
}
