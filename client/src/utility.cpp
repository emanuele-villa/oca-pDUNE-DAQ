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
