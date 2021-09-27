#include "daqclient.h"
#include <stdio.h>
#include <utility.h>

daqclient::daqclient(const char *address, int port, int verb):tcpclient(address, port, verb){
  if (verbosity>0) {
    printf("%s, daqclient created\n", __METHOD_NAME__);
  }
  return;
}

//--------------------------------------------------------------

daqclient::~daqclient(){
}
