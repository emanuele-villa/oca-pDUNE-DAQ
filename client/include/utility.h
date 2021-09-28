#ifndef UTILITY_H
#define UTILITY_H

#include <string>

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

const int LEN = 1024 * 1000;// 1MB data

inline std::string methodName(const std::string& prettyFunction) {
  size_t colons = prettyFunction.find("::");
  size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
  size_t end = prettyFunction.rfind("(") - begin;
  
  return prettyFunction.substr(begin,end) + "()";
}

inline std::string className(const std::string& prettyFunction) {
  size_t colons = prettyFunction.find("::");
  if (colons == std::string::npos)
    return "::";
  size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
  size_t end = colons - begin;
  
  return prettyFunction.substr(begin,end);
}

#define __METHOD_NAME__ methodName(__PRETTY_FUNCTION__).c_str()
#define __CLASS_NAME__ className(__PRETTY_FUNCTION__).c_str()

void print_error(const char* format, ...);
void exit_if(bool r, const char* format, ...);

#endif
