/*!
  @file maka.cpp
  @brief Merger to collect data from remote detectors
  @author Mattia Barbanera (mattia.barbanera@infn.it)
*/

#include "maka.h"
#include "utility.h"

maka::maka(int port, int verb):tcpServer(port, verb){
  //Initialize parameters
  kNEvts = 0;
  runStop();
  clearDetLists();

  //Initialize server and listen for OCA connection 

  //Handshake commands length

}

maka::~maka(){
  runStop();
  clearDetLists();
}


void maka::clearDetLists(){
  kDetAddrs.clear();
  kDetPorts.clear();
}

void maka::addDet(char* _addr, int _port){
  kDetAddrs.push_back(_addr);
  kDetPorts.push_back(_port);
}

void maka::clearDetectors(){
  for (uint32_t ii=0; ii<kDet.size(); ii++) {
    if (kDet[ii]) delete kDet[ii];
  }
  kDet.clear();
}

void maka::setUpDetectors(){
  for (uint32_t ii=0; ii<kDetAddrs.size(); ii++) {
    kDet.push_back(new tcpclient(kDetAddrs[ii], kDetPorts[ii]));
  }
}

void maka::runStart(char* _runType, uint32_t _runNum, uint32_t _runTime){
  kNEvts   = 0;
  kRunning = true;

  //Start clients
  setUpDetectors();

  //Start thread to merge data
  kMerger3d = std::thread(&maka::merger, this, _runType, _runNum, _runTime);
}

void maka::runStop(){
  kRunning = false;

  //Close clients
  clearDetectors();

  //Stop thread
  if (kMerger3d.joinable()) kMerger3d.join();
}

int maka::fileHeader(FILE* _dataFile){

}

auto fileFormatTime = [](unsigned int val, size_t ndigits) {
    std::string sval = std::to_string(val);
    if (sval.length() < ndigits) {
        sval = std::string(ndigits - sval.length(), '0').append(sval);
    }
    return sval;
};

auto fileFormatDate = [](uint32_t timel){
  // Construct human-readable date
  time_t time{timel};
  auto humanTime = *gmtime(&time);

  std::string dateTime;
  dateTime.append(std::to_string(humanTime.tm_year + 1900));
  dateTime.append(fileFormatTime(humanTime.tm_mon + 1, 2));
  dateTime.append(fileFormatTime(humanTime.tm_mday, 2));
  dateTime.append("_");
  dateTime.append(fileFormatTime(humanTime.tm_hour, 2));
  dateTime.append(fileFormatTime(humanTime.tm_min, 2));
  dateTime.append(fileFormatTime(humanTime.tm_sec, 2));

  return dateTime;
};

int maka::merger(char* _runType, uint32_t _runNum, uint32_t _runTime){
  char dataFileName[255];
  unsigned int lastNEvents = 0;
  using clock_type = std::chrono::system_clock;
  //using clock_type = std::chrono::high_resolution_clock; //Increase precision
  
  //Open a file in the kdataPath folder and name it with UTC
  
  // // copy _runType and make it all UPPERCASE
  // std::string _runType_upper{_runType};
  // std::transform(begin(_runType_upper), end(_runType_upper),
  //                   begin(_runType_upper), std::toupper);

  string humanDate = fileFormatDate(_runTime);
  sprintf(dataFileName,"%s/SCD_RUN%05d_%s_%s.dat", kDataPath.data(), _runNum, _runType, humanDate.c_str());

  printf("%s) Opening output file: %s\n", __METHOD_NAME__, dataFileName);
  FILE* dataFileD = fopen(dataFileName,"w");
  if (dataFileD == nullptr) {
    printf("%s) Error: file %s could not be created. Do the data dir %s exist?\n", __METHOD_NAME__, dataFileName, kDataPath.data());
    return -1;
  }

  //----------------------------------------------------------------------------
  //Collect data from clients
  while(kRunning) {
    usleep(100); //?
    auto start = clock_type::now();
    collector(dataFileD);
    auto stop = clock_type::now();

    if(kNEvts != lastNEvents){
      std::cout << "\rEvent " << kNEvts << " last recordEvents took " << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() << " us                            " << std::flush;
      lastNEvents = kNEvts;
    }
  }
  std::cout << '\n';
  
  //----------------------------------------------------------------------------
  //Close file
  fclose(dataFileD);
  printf("%s) File %s closed\n", __METHOD_NAME__, dataFileName);

  return 0;

}

int maka::collector(FILE* _dataFile){
  int readRet = 0;
  int writeRet = 0;
  //std::vector<uint32_t*> evts(det.size(), "");
  uint32_t evtLen = 0;
  uint32_t evtLen_tot = 0;
  std::vector<uint32_t> evt(652);

  // FIX ME: at most 64 detectors
  std::bitset<64> replied{0};
  
  constexpr uint32_t header = 0xfa4af1ca;//FIX ME: this header must be done properly. In particular the real length (written by this master, not the one in the payload, after the SoP word) 
  bool headerWritten = false;

  // FIX ME: replace kRunning with proper timeout
  do {
    for (uint32_t ii=0; ii<kDet.size(); ii++) {
      if(!replied[ii]){
	      uint32_t readSingle = (getEvent(evt, evtLen, ii));
	      readRet += readSingle;
	      if(evtLen){
	        replied[ii] = true;
	      }    
	      evtLen_tot += evtLen;

	      // only write the header when the first board replies
	      if(replied.count() == 1 && !headerWritten){
	        ++kNEvts;
	        fwrite(&header, 4, 1, _dataFile);	  
	        headerWritten = true;
	      }
	      writeRet += fwrite(evt.data(), evtLen, 1, _dataFile);

	      if (kVerbosity>0) {
	        printf("%s) Get event from DE10 %s\n", __METHOD_NAME__, kDetAddrs[ii]);
	        printf("  Bytes read: %d/%d\n", readSingle, evtLen);
	        printf("  Writes performed: %d/%lu\n", writeRet, kDet.size());
	      }
      }
    }
  } while (replied.count() && (replied.count() != kDet.size()) && kRunning);

  //Everything is read and dumped to file
  if (evtLen_tot!=0) {
    if (readRet != (int)evtLen_tot || writeRet != (int)(kDet.size())) {
      printf("%s):\n", __METHOD_NAME__);
      printf("    Bytes read: %d/%u\n", readRet, evtLen_tot);
      printf("    Writes performed: %d/%d\n", writeRet, (int)(kDet.size()));
      return -1;
    }
  }
  else {
    if (kVerbosity>1) {
      printf("%s) Total event lenght was 0\n", __METHOD_NAME__);
    }
  }
  return 0;
}


int maka::getEvent(std::vector<uint32_t>& _evt, uint32_t& _evtLen, int _det){
  //Get the event from HPS and loop here until all data are read
  uint32_t evtRead = 0;
  kDet[_det]->ReceiveInt(_evtLen); //in uint32_t units
  
  if (_evt.size()<_evtLen) _evt.resize(_evtLen);
  _evtLen *= sizeof(uint32_t);  //in byte units
  while (evtRead < _evtLen) {
    evtRead += kDet[_det]->Receive(&_evt[evtRead/sizeof(uint32_t)], _evtLen-evtRead);
  }

  return evtRead;
}