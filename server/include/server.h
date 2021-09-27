#ifndef _SERVER_H_
#define _SERVER_H_

void *virtual_base;
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

void *receiver_slow_control(void *args);
void *receiver_comandi(void *args);
int main(int argc, char *argv[]);

#endif
