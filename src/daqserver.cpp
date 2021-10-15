#include "daqserver.h"
#include "utility.h"
#include <sys/time.h>
//#include <TDatime.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctime>
#include <bitset>
#include <iostream>
#include <chrono>
#include <algorithm>

daqserver::daqserver(int port, int verb) : tcpserver(port, verb), m_evt(652){
  if (kVerbosity>0){
    printf("%s) daqserver created\n", __METHOD_NAME__);
  }

  addressdet.clear();
  portdet.clear();

  kStart=false;

  calibmode=0;
  mode=0;
  trigtype=0;

  return;
}

daqserver::~daqserver(){
  if (kVerbosity>0) {
    printf("%s) destroying daqserver\n", __METHOD_NAME__);
  }

  for (uint32_t ii=0; ii<det.size(); ii++) {
    if (det.at(ii)) delete det.at(ii);
    if (kVerbosity>0) {
      printf("%s) destroying DE10 %d\n", __METHOD_NAME__, ii);
    }
  }

  return;
}

void daqserver::SetListDetectors(int nde10, const char* addressde10[], int portde10[], int detcmdlenght){

  addressdet.clear();
  portdet.clear();

  for (int ii=0; ii<nde10; ii++) {
    addressdet.push_back(addressde10[ii]);
    portdet.push_back(portde10[ii]);
    det.push_back(new de10_silicon_base(addressde10[ii], portde10[ii], ii, detcmdlenght, kVerbosity));
  }

  return;
}

void daqserver::SetDetectorsCmdLenght(int detcmdlenght){

  for (int ii=0; ii<(int)(det.size()); ii++) {
    det[ii]->SetCmdLenght(detcmdlenght);
  }

  return;
}

void daqserver::SetCalibrationMode(uint32_t mode){

  calibmode = mode;

  for (int ii=0; ii<(int)(det.size()); ii++) {
    det[ii]->SetCalibrationMode(calibmode);
  }

  return;
}

void daqserver::SetMode(uint8_t _mode){

  mode = _mode;

  for (int ii=0; ii<(int)(det.size()); ii++) {
    det[ii]->SetMode(mode);
  }

  return;
}

void daqserver::SelectTrigger(uint32_t trig){

  trigtype = trig;

  for (int ii=0; ii<(int)(det.size()); ii++) {
    det[ii]->SelectTrigger(trigtype);
  }

  return;
}

void daqserver::ProcessCmdReceived(char* msg){

  if (kVerbosity>-1) {//FIX ME: >-1 perche' ora non fa niene, poi deve fare quelle sotto
    printf("%s) |%s| (lenght = %lu)\n", __METHOD_NAME__, msg, strlen(msg));
  }

  if(strstr(msg, "cmd=") != NULL) { //out commands: "cmd=xxxx"

    if (strcmp(msg, "cmd=Init") == 0){
      printf("%s) Init()\n", __METHOD_NAME__);
      Init();
      ReplyToCmd(msg);
      //FIX ME: only for now to test
      // for (int ii=0; ii<32; ii++) {
      // 	printf("%s) Reading reg %d\n", __METHOD_NAME__, ii);
      // 	ReadReg(ii);
      // }
    }
    else if (strcmp(msg, "cmd=Wait") == 0){//essentially for test
      printf("%s) Wait()\n", __METHOD_NAME__);
      printf("sleeping for 30s: "); fflush(stdout);
      for (int ii=0; ii<30; ii++) {
	printf("%d... ", ii); fflush(stdout);
	sleep(1);
      }
      printf("\n");
      ReplyToCmd(msg);
    }

  }
  else {//possibly a chinese command
    // command
    static const char* btcmd ="FF800008";
    static const char* start ="EE000001";
    static const char* stop  ="EE000000";
    static const char* beam  ="0001";
    static const char* cal   ="0000";

    static const int length=16;
    char command_string[2*length+1] = "";
    hex2string(msg, length, command_string);

    //    printf("%s\n", command_string);

    char cmdgroup[4][32] = {"", "", "", ""};
    for (int ii=0; ii<4; ii++) {
      strncpy(cmdgroup[ii], &command_string[ii*8], 8);
      if (kVerbosity>0) {
	printf("%s\n", cmdgroup[ii]);
      }
    }

    //check the command
    if (strcmp(btcmd, cmdgroup[0])==0) {//is a chinese command
      if (strcmp(start,cmdgroup[2])==0) {//start daq
	printf("%s) Start()\n", __METHOD_NAME__);
	char runtype[32] = "";
	strncpy(runtype, &cmdgroup[1][4], 4);
	char sruntype[32] = "";
	if (strcmp(beam,runtype)==0) {
	  sprintf(sruntype, "BEAM");
	  SetCalibrationMode(0);
	}
	else if (strcmp(cal,runtype)==0) {
	  sprintf(sruntype, "CAL");
	  SetCalibrationMode(1);
	}
	else {
	  printf("%s) Not a valid run type %s\n", __METHOD_NAME__, runtype);
	  sprintf(sruntype, "????");
	}

	// ignore consecutive Start commands
	if(kStart){
	  ReplyToCmd("Consecutive starts received. Ignoring last command.");
	  return;
	}

	char srunnum[32] = "";
	strncpy(srunnum,  &cmdgroup[1][0], 4);
	char* ptr;
	uint32_t runnum = strtol(srunnum, &ptr, 16);
	uint32_t unixtime = strtol(cmdgroup[3], &ptr, 16);
	std::time_t t = unixtime;
	if (kVerbosity>0) {
	  printf("runtype=%s (-> %s), runnum=%s (%u), unixtime=%u (%s -> %s)\n", runtype, sruntype, srunnum, runnum, unixtime, cmdgroup[3], asctime(localtime(&t)));
	}
	//Spawn a thread to read events. Stop() will join the thread
	nEvents = 0;
	_3d = std::thread(&daqserver::Start, this, sruntype, runnum, unixtime);
	ReplyToCmd(msg);
	//	}
      }
      else if(strcmp(stop,cmdgroup[2])==0) {//stop daq
	printf("%s) Stop()\n", __METHOD_NAME__);
	Stop();
	ReplyToCmd(msg);
      }
      else {
	printf("%s) not a valid sub-command: %s (%s)\n\n", __METHOD_NAME__, cmdgroup[2], command_string);
	sprintf(msg, "NOT-A-VALID-SUBCMD");
	ReplyToCmd(msg);
      }
    }
    else {
      printf("%s) not a valid command: %s\n\n", __METHOD_NAME__, command_string);
      sprintf(msg, "NOT-A-VALID-CMD");
      ReplyToCmd(msg);
    }

  }

  return;
}

void daqserver::ReadAllRegs(){

  for (int ii=0; ii<32; ii++) {
    printf("%s) Reading reg %d\n", __METHOD_NAME__, ii);
    ReadReg(ii);
  }

  return;
}

int daqserver::ReadReg(uint32_t regAddr) {
  int ret=0;
  uint32_t regCont=0; //FIX ME Shall be vector

  for (uint32_t ii=0; ii<det.size(); ii++) {
    ret |= (det.at(ii)->readReg(regAddr, regCont)<<ii);
    if (kVerbosity>-1) {
      printf("%s) Read from DE10 %d: %08x\n", __METHOD_NAME__, ii, regCont);
    }
  }
  return ret;
}

int daqserver::Init() {
  int ret = 0;

  for (uint32_t ii=0; ii<det.size(); ii++) {
    ret |= (det.at(ii)->Init())<<1;
    if (kVerbosity>0) {
      printf("%s) Init of DE10 %d\n", __METHOD_NAME__, ii);
    }
  }

  return ret;
}


//Read the events from all of the DE10 and write them in binary to the .dat file
int daqserver::recordEvents() {

  int readRet = 0;
  int writeRet = 0;
  //std::vector<uint32_t*> evts(det.size(), "");
  uint32_t evtLen = 0;
  uint32_t evtLen_tot = 0;

  // FIX ME: at most 64 DE10
  std::bitset<64> replied{0};

  constexpr uint32_t pippo = 0xfa4af1ca;
  bool headerWritten = false;

  // FIX ME: replace kStart with proper timeout
  do {
    for (uint32_t ii = 0; ii < det.size(); ii++) {
      if (!replied[ii]) {
        det.at(ii)->AskEvent();
      }
    }

    for (uint32_t ii = 0; ii < det.size(); ii++) {
      if (!replied[ii]) {
        readRet += (det.at(ii)->GetEvent(m_evt, evtLen));
        if (evtLen) {
          replied[ii] = true;
        }
        evtLen_tot += evtLen;

        // only write the header when the first board replies
        if (replied.count() == 1 && !headerWritten) {
          ++nEvents;
          QueueForWrite({pippo});
          headerWritten = true;
        }
        // 	writeRet += fwrite(m_evt.data(), evtLen, 1, fd);
        QueueForWrite(m_evt);

        if (kVerbosity > 0) {
          printf("%s) Get event from DE10 %s\n", __METHOD_NAME__, addressdet[ii]);
          printf("  Bytes read: %d/%d\n", readRet, evtLen);
        }
      }
    }
  } while (replied.count() && (replied.count() != det.size()) && kStart);

  //Everything is read and dumped to file
  if (evtLen_tot!=0) {
    if (readRet != (int)evtLen_tot || writeRet != (int)(det.size())) {
      printf("%s):\n", __METHOD_NAME__);
      printf("    Bytes read: %d/%u\n", readRet, evtLen_tot);
      printf("    Writes performed: %d/%d\n", writeRet, (int)(det.size()));
      return -1;
    }
  }
  else {
    if (kVerbosity>1) {
      printf("%s) total event lenght was 0\n", __METHOD_NAME__);
    }
  }
  return 0;
}

auto format_time_values = [](unsigned int val, size_t ndigits) {
    std::string sval = std::to_string(val);
    if (sval.length() < ndigits) {
        sval = std::string(ndigits - sval.length(), '0').append(sval);
    }
    return sval;
};

void daqserver::Start(char* runtype, uint32_t runnum, uint32_t unixtime) {
  //Open a file in the kdataPath folder and name it with UTC
  char dataFileName[255];

  auto format_human_date = [](uint32_t timel){
      // Construct human-readable date
      time_t time{timel};
      auto humanTime = *gmtime(&time);

      std::string dateTime;
      dateTime.append(std::to_string(humanTime.tm_year + 1900));
      dateTime.append(format_time_values(humanTime.tm_mon + 1, 2));
      dateTime.append(format_time_values(humanTime.tm_mday, 2));
      dateTime.append("_");
      dateTime.append(format_time_values(humanTime.tm_hour, 2));
      dateTime.append(format_time_values(humanTime.tm_min, 2));
      dateTime.append(format_time_values(humanTime.tm_sec, 2));

      return dateTime;
  };

  // // copy runtype and make it all UPPERCASE
  // std::string runtype_upper{runtype};
  // std::transform(begin(runtype_upper), end(runtype_upper), begin(runtype_upper), std::toupper);

  std::string humanDate = format_human_date(unixtime);
  sprintf(dataFileName,"%s/SCD_RUN%05d_%s_%s.dat", kdataPath, runnum, runtype, humanDate.c_str());

  printf("%s) Opening output file: %s\n", __METHOD_NAME__, dataFileName);

  FILE* dataFileD;
  dataFileD = fopen(dataFileName,"w");
  if (dataFileD == nullptr) {
    printf("%s) Error: file %s could not be created. Do the data dir %s exist?\n", __METHOD_NAME__, dataFileName, kdataPath);
    return;
  }

  SetMode(1);

  //Dump events to the file until Stop is received
  kStart = true;
  unsigned int lastNEvents = 0;
  altro_3d = std::thread(&daqserver::WriteEvents, this, dataFileD);

  while(kStart) {
    usleep(10);
    auto start = std::chrono::high_resolution_clock::now();
    recordEvents();
    auto stop = std::chrono::high_resolution_clock::now();

    if(nEvents != lastNEvents){
      std::cout << "\rEvent " << nEvents << " last recordEvents took " << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() << " us                            " << std::flush;
      lastNEvents = nEvents;
    }
  }
  std::cout << '\n';

  //Close the file and terminate thread
  fclose(dataFileD);
  printf("%s) File %s closed\n", __METHOD_NAME__, dataFileName);
}

void daqserver::Stop() {
  if(kStart){
    kStart = false;
    printf("Joining threads...\n");
    _3d.join();
    altro_3d.join();
    printf("...joined\n");
  }

  SetMode(0);
  sleep(10);

  //FIX ME: metterci un while che fa N GetEvent()

  if (kVerbosity > 0) printf("%s) Thread stopped succesfully\n", __METHOD_NAME__);
}

void daqserver::QueueForWrite(const std::vector<uint32_t> &data) {
  std::lock_guard<std::mutex> lock{m_evtWriteMtx};
  std::copy(begin(data), end(data), std::back_inserter(m_evtWriteBuffer));
}

void daqserver::WriteEvents(FILE *fd) {

  do {
    {
      std::lock_guard<std::mutex> lock{m_evtWriteMtx};
      fwrite(m_evtWriteBuffer.data(), m_evtWriteBuffer.size() * sizeof(decltype(m_evtWriteBuffer)::value_type), 1, fd);
      m_evtWriteBuffer.clear();
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
  } while(kStart || !m_evtWriteBuffer.empty());
}

