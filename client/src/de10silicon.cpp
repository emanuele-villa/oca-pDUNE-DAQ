#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include "de10silicon.h"
#include <errno.h>
#include <QApplication>
#include <QObject>
#include <stdint.h>

de10_silicon::de10_silicon(const char *address, int port):de10_silicon_base(address, port){
  return;
}

//--------------------------------------------------------------
de10_silicon::~de10_silicon(){

}
