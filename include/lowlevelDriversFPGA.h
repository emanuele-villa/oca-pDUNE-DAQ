#ifndef _LOW_LEVEL_DRIVERS_FPGA_H_
#define _LOW_LEVEL_DRIVERS_FPGA_H_

// FIFO types
#define CONFIG_FIFO 1 // FIFO for the configuration tx
#define HK_FIFO     2 // FIFO for the telemetries rx
#define DATA_FIFO   3 // FIFO for the payload rx

//CONFIG-FIFO CONSTANTS
#define REG_SOP  0xAA55CA5A
#define REG_HDR1 0x4EADE500
#define REG_EOP  0xBADC0FEE

extern struct fpgaAddresses baseAddr;
extern uint32_t kGwV;

//Generic FIFOs access
inline uint32_t readFifoLevel (uint32_t* fifoLevelAddr);
inline bool readFifoFull (uint32_t* fifoStatusAddr);
inline bool readFifoAFull (uint32_t* fifoStatusAddr);
inline bool readFifoEmpty (uint32_t* fifoStatusAddr);
inline bool readFifoAEmpty (uint32_t* fifoStatusAddr);
int InitFifo(int FIFO_TYPE, uint32_t aEmptyThr, uint32_t aFullThr, uint8_t interruptEn);
int WriteFifo(int FIFO_TYPE, uint32_t *data);
int WriteFifoBurst(int FIFO_TYPE, uint32_t *data, int length_burst);
int ReadFifo(int FIFO_TYPE, uint32_t *data);
int ReadFifoBurst(int FIFO_TYPE, uint32_t *data, int length_burst, bool flush);
int StatusFifo(int FIFO_TYPE, uint32_t *fifo_level, uint32_t *fifo_full, uint32_t *fifo_empty, uint32_t *fifo_almostfull, uint32_t *fifo_almostempty, uint32_t *almostfull_setting, uint32_t *almostempty_setting);
int ShowStatusFifo(int FIFO_TYPE);
int OverflowController(int FIFO_TYPE);

//Generic functions
uint32_t crc_init();
uint32_t crc_update(uint32_t crc, const void *data, size_t data_len);
uint32_t crc_finalize(uint32_t crc);
bool parity8(uint8_t dataIn);
uint8_t parity32(uint32_t dataIn);

//regArray Access
void ReadReg(int regAddr, uint32_t *data);
int writeReg(uint32_t * pktContent, int pktLen);
void singleWriteReg(uint32_t regAddr, uint32_t regContent);


#endif // _LOW_LEVEL_DRIVERS_FPGA_H_
