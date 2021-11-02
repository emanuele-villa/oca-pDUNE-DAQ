#include <string>

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "utility.h"

void print_error(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  printf("  error no: %d, error msg: %s\n", errno, strerror(errno));
  va_end(args);
  return;
}

void exit_if(bool r, const char* format, ...) {
  if (r) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("  error no: %d, error msg: %s\n", errno, strerror(errno));
    va_end(args);
    exit(1);
  }
  return;
}

void* hex2string(char *pt, int length, char* pt_string) {
  int i=0;

  for(i=0;i<(2*length);i++) {	
    if(i%2==0)//even
      switch((pt[i/2]&0xF0)){
      case 0x00:*(pt_string+i)=0x30;break;
      case 0x10:*(pt_string+i)=0x31;break;
      case 0x20:*(pt_string+i)=0x32;break;
      case 0x30:*(pt_string+i)=0x33;break;
      case 0x40:*(pt_string+i)=0x34;break;
      case 0x50:*(pt_string+i)=0x35;break;
      case 0x60:*(pt_string+i)=0x36;break;
      case 0x70:*(pt_string+i)=0x37;break;
      case 0x80:*(pt_string+i)=0x38;break;
      case 0x90:*(pt_string+i)=0x39;break;
      case 0xA0:*(pt_string+i)=0x41;break;
      case 0xB0:*(pt_string+i)=0x42;break;
      case 0xC0:*(pt_string+i)=0x43;break;
      case 0xD0:*(pt_string+i)=0x44;break;
      case 0xE0:*(pt_string+i)=0x45;break;
      case 0xF0:*(pt_string+i)=0x46;break;
      default:*(pt_string+i)=0x30;break;
      }
    else//odd
      switch((pt[i/2]&0x0F)){
      case 0x00:*(pt_string+i)=0x30;break;
      case 0x01:*(pt_string+i)=0x31;break;
      case 0x02:*(pt_string+i)=0x32;break;
      case 0x03:*(pt_string+i)=0x33;break;
      case 0x04:*(pt_string+i)=0x34;break;
      case 0x05:*(pt_string+i)=0x35;break;
      case 0x06:*(pt_string+i)=0x36;break;
      case 0x07:*(pt_string+i)=0x37;break;
      case 0x08:*(pt_string+i)=0x38;break;
      case 0x09:*(pt_string+i)=0x39;break;
      case 0x0A:*(pt_string+i)=0x41;break;
      case 0x0B:*(pt_string+i)=0x42;break;
      case 0x0C:*(pt_string+i)=0x43;break;
      case 0x0D:*(pt_string+i)=0x44;break;
      case 0x0E:*(pt_string+i)=0x45;break;
      case 0x0F:*(pt_string+i)=0x46;break;
      default:*(pt_string+i)=0x30;break;
      }
  }
  *(pt_string+2*length)=0x00;
	
  return NULL;
}
