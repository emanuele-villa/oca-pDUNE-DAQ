#include "de10silicon_base.h"
#include "daqserver.h"

// const int nde10 = 1;
// const char* addressde10[nde10] = {"localhost"}; 
// const int portde10[nde10] = {80};
const int nde10 = 0;
const char* addressde10[nde10] = {}; 
const int portde10[nde10] = {};

int main(int argc, char *argv[]) {
  printf("prova\n");

  de10_silicon_base* det[nde10];
  for (int ii=0; ii<nde10; ii++) {
    det[ii] = new de10_silicon_base(addressde10[ii], portde10[ii]);
  }

  daqserver* daqsrv = new daqserver(9999);
  daqsrv->Listen();
  
  return 0;
}
