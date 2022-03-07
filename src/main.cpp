#include <signal.h>
#include <unistd.h>
#include <iostream>

#include "anyoption.h"

#include "daqserver.h"
#include "utility.h"
#include "daqConfig.h"
#include "paperoConfig.h"

/*const int nde10 = 7;
const char* addressde10[nde10] = {"192.168.2.101", "192.168.2.102", "192.168.2.103",
  			  "192.168.2.105", "192.168.2.106", "192.168.2.107",
  			  "192.168.2.108"
};
int portde10[nde10] = {5000, 5000, 5000, 5000, 5000, 5000, 5000
};
*/
const int nde10 = 1;
const char* addressde10[nde10] = {"192.168.2.101"};
int portde10[nde10] = {5000};

daqConfig::configParams daqConf;

daqserver* daqsrv = nullptr;
int ControlOn=1;
int StatusOn=0;

void PrintStatus(int dummy) {
  StatusOn=1;
  return;
}

void StopRun(int dummy) {
  ControlOn=0;
  daqsrv->StopListening();
  return;
}

struct commandLineOptions
{
  string ocaCfg;
  string paperoCfg;
  int verbose;
};

/*

*/
int commandLineParser(int argc, char *argv[], commandLineOptions &clOpt, vector<string> & args) {
  AnyOption *opt = new AnyOption();

  /* Set preferences  */
  // opt->noPOSIX(); /* do not check for POSIX style character options */
  opt->setVerbose(); /* print warnings about unknown options */
  opt->autoUsagePrint(true); /* print usage for bad options */

  /* Set the usage/help   */
  opt->addUsage("Usage: ");
  opt->addUsage("");
  opt->addUsage(" -h  --help		Prints this help ");
  opt->addUsage(" -v  --verbose <level>	Verbose level");
  opt->addUsage(" -p  --papero <path>	PAPERO config file");
  opt->addUsage(" -o  --oca <path>	OCA config file");
  opt->addUsage("");

  /* Set the option strings/characters */

  /* Options will be checked only on the command line and not in
   * option/resource file */
  opt->setCommandFlag("help", 'h'); /* a flag (takes no argument), supporting long and short form */
  opt->setCommandOption("verbose", 'v');  /* an option (takes an argument), supporting only long form */
  opt->setCommandOption("papero", 'p');
  opt->setCommandOption("oca", 'o');

  /* Process the commandline and resource file */

  /* go through the command line and get the options  */
  opt->processCommandArgs(argc, argv);

  //if (!opt->hasOptions()) { /* print usage if no options */
  //  opt->printUsage();
  //  delete opt;
  //  exit(1);
  //}

  /* Get the values */
  if (opt->getFlag("help") || opt->getFlag('h'))
  {
    opt->printUsage();
    exit(0);
  }
  if (opt->getValue('v') != NULL || opt->getValue("verbose") != NULL)
  {
    clOpt.verbose = stoi(opt->getValue('v'));
  }
  else
  {
    clOpt.verbose = 1;
  }
  if (opt->getValue('p') != NULL || opt->getValue("papero") != NULL)
  {
    clOpt.paperoCfg = opt->getValue('p');
  }
  else
  {
    clOpt.paperoCfg = "./config/papero.cfg";
  }

  if (opt->getValue('o') != NULL || opt->getValue("oca") != NULL)
  {
    clOpt.ocaCfg = opt->getValue('o');
  }
  else
  {
    clOpt.ocaCfg = "./config/oca.cfg";
  }
  
  /* Get the actual arguments after the options */
  for (int i = 0; i < opt->getArgc(); i++) {
    cout << opt->getArgv(i) << endl;
    args.push_back(opt->getArgv(i));
  }

  /* Done */
  delete opt;
  return 0;
}

int main(int argc, char *argv[]) {
  std::cout<<"---------------------------------------------------------------------"<<std::endl;
  std::cout<<"OCA hash="<<GIT_HASH<<", time="<<COMPILE_TIME<<", branch="<<GIT_BRANCH<<std::endl;
  std::cout<<"---------------------------------------------------------------------"<<std::endl;
  
  commandLineOptions opt;
  vector<string> args;

  commandLineParser(argc, argv, opt, args);

  daqConfig daqConfig(opt.ocaCfg);
  daqConf = daqConfig.getParams();

  paperoConfig paperoConfig(opt.paperoCfg);
  

  //this must be done before the `signal` otherwise for StopRun the daqsrv is still NULL
  //Setup the client side of OCA
  daqsrv = new daqserver(daqConf.portClient, opt.verbose);
  daqsrv->SetCmdLenght(daqConf.clientCmdLen);

  daqsrv->SetListDetectors(daqConf.nDet, addressde10, portde10, daqConf.hpsCmdLen);
  //Ladder1
  daqsrv->SetDetId("192.168.2.103", 302);
  daqsrv->SetPacketLen("192.168.2.103", 0x18A);

  daqsrv->Init();

  sleep(5);
  
  daqsrv->SetCalibrationMode(1);
  //  daqsrv->SetCalibrationMode(0);
  sleep(1);
  daqsrv->SelectTrigger(0);
  //  daqsrv->SelectTrigger(1);
  //  daqsrv->ReadAllRegs();
  daqsrv->ReadReg(31);

  //is not really working, for now: it is killed as a standard CTRL-C
  //the param sent to StopRun is SIGTERM itself and we need that StopRun accepts a param even if cannot use it
  signal(SIGTERM,StopRun);//killing the PID of the process we call the function StopRun that exits the program in the right way
  signal(SIGINT,StopRun);// sending 'CTRL_C' the program exits in the right way
  signal(SIGQUIT,PrintStatus);//sending 'CTRL \' we print the numbers of taken events

  // quando lo usiamo cosi' lui sta in Listen perenne e aspetta dei comandi dal suo master per passarlo alle de10nano
  daqsrv->ListenCmd();

  // quando invece lo usiamo cosi' lui è master della acquisizione, non e' server di nulla e puo' essere utilizzato per mandare comandi in sequenza
  // la cosa migliore sarebbe fare un classe `daqmaster` che eredita da `daqserver` (o viceversa...) che non e' un server e non aspetta comandi da un master a lui superiore
  //  daqsrv->ReadReg(31);

  if (daqsrv) delete daqsrv;

  printf("%s) Exiting...\n", __METHOD_NAME__);

  return 0;
}
