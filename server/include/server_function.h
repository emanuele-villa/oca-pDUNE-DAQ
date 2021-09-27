#ifndef _SERVER_FUNCTION_H_
#define _SERVER_FUNCTION_H_


uint32_t receive_register_content(int socket);
int sendSocket(int socket, char * msg);
void *high_priority(void *socket);

void ResetFpga();
void Init(uint32_t * regsContentIn, uint32_t opLen);
void SetDelay(uint32_t delayIn);
void SetMode(uint32_t modeIn);
void GetEventNumber(uint32_t * extTrigCount, uint32_t * intTrigCount);
void PrintAllEventNumber(int socket);
void EventReset();
void Calibrate(uint32_t calibIn);
void intTriggerPeriod(uint32_t periodIn);
void selectTrigger(uint32_t intTrigIn);
void configureTestUnit(uint32_t tuCfg);
void GetEvent(int socket);


#endif
