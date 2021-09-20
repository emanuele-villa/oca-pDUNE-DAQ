#ifndef SERVER_FUNCTION_H_
#define SERVER_FUNCTION_H_



#include <stdio.h>
#include <unistd.h>

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
int mapping();

#endif
