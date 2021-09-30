#ifndef _SERVER_H_
#define _SERVER_H_

#define rGOTO_STATE       0
#define rUNITS_EN         1
#define rTRIGBUSY_LOGIC   2
#define rDET_ID           3
#define rPKT_LEN          4
#define rFE_CLK_PARAM     5
#define rADC_CLK_PARAM    6
#define rMSD_PARAM        7
#define rGW_VER           16
#define rINT_TS_MSB       17
#define rINT_TS_LSB       18
#define rEXT_TS_MSB       19
#define rEXT_TS_LSB       20
#define rWARNING          21
#define rBUSY             22
#define rEXT_TRG_COUNT    23
#define rINT_TRG_COUNT    24
#define rFDI_FIFO_NUMWORD 25
#define rPIUMONE          31

struct fpgaAddresses {
  void * virtual_base;
  uint32_t * fpgaRegAddr;
  uint32_t * fpgaRegCont;
  uint32_t * configFifo;
  uint32_t * configFifoCsr;
  uint32_t * configFifoLevel;
  uint32_t * configFifoStatus;
  uint32_t * hkFifo;
  uint32_t * hkFifoCsr;
  uint32_t * hkFifoLevel;
  uint32_t * hkFifoStatus;
  uint32_t * FastFifo;
  uint32_t * FastFifoCsr;
  uint32_t * FastFifoLevel;
  uint32_t * FastFifoStatus;
  int verbose;
};

int main(int argc, char *argv[]);

#endif
