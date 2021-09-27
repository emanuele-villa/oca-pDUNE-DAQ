#ifndef _SERVER_FUNCTION_H_
#define _SERVER_FUNCTION_H_


uint32_t receive_register_content(int socket);
void *high_priority(void *socket);

void ResetFpga();
void Init(int socket);
void SetDelay(int socket);
void SetMode(int socket);
void GetEventNumber(int socket);
void PrintAllEventNumber(int socket);
void EventReset(int socket);
void GetEvent(int socket);
void OverWriteDelay(int socket);
void Calibrate(int socket);
void WriteCalibPar(int socket);
void SaveCalibrations(int socket);
void intTriggerPeriod(int socket);
void selectTrigger(int socket);
void configureTestUnit(int socket);

#endif
