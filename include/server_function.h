#ifndef _SERVER_FUNCTION_H_
#define _SERVER_FUNCTION_H_

// questo file deve essere splittato in due parti:
// - hpsserver.h
// - server_function.h

// hpsserver.h
// qui ci va la classe: copiare daqserver (i.e. figlia di tcpserver + le funzioni sotto che diventano metodi di classe

extern struct fpgaAddresses baseAddr;

uint32_t receiveWordSocket(int socket);
int sendSocket(int socket, void* msg, uint32_t len);
void *high_priority(void *socket);
void GetEvent(int socket);

// server_function.h
// ci sono queste due funzioni (alleggerite di molto, lo commento nel .c)
void *receiver_slow_control(void *args);
void *receiver_comandi(void *args);

#endif
