#ifndef _SERVER_FUNCTION_H_
#define _SERVER_FUNCTION_H_


uint32_t receiveWordSocket(int socket);
int sendSocket(int socket, char * msg);
void *high_priority(void *socket);
void GetEvent(int socket);
void *receiver_slow_control(void *args);
void *receiver_comandi(void *args);


#endif
