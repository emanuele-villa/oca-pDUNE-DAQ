/*!
  @file makaMerger.cpp
  @brief Merger to collect data from remote detectors
  @author Mattia Barbanera (mattia.barbanera@infn.it)
*/

#include "makaMerger.h"
#include "utility.h"

makaMerger::makaMerger(int port, int verb, bool _net):tcpServer(port, verb){
  //Initialize parameters
  kNEvts = 0;
  kNEvtsCal = 0;
  kNEvtsBeam = 0;

  kCmdLen = 24;
  runStop();
  clearDetLists();
  cpRx = new configPacket();
  spRx = new startPacket();

  //Initialize server
  if (_net){
    kAddr.sin_family      = AF_INET; //For network communication
  } else {
    kAddr.sin_family      = AF_UNIX; //For in-system communication
  }
  kBlocking = true;
  Setup();

}

makaMerger::~makaMerger(){
  StopListening();
  runStop();
  clearDetLists();
  delete cpRx;
  delete spRx;
}

/*------------------------------------------------------------------------------
  Detector list and set-up
------------------------------------------------------------------------------*/
void makaMerger::clearDetLists(){
  kDetIds.clear();
  kDetAddrs.clear();
  kDetPorts.clear();
}

void makaMerger::addDet(uint32_t _id, char* _addr, int _port){
  kDetIds.push_back(_id);
  kDetAddrs.push_back(_addr);
  kDetPorts.push_back(_port);
}

void makaMerger::clearDetectors(){
  for (uint32_t ii=0; ii<kDet.size(); ii++) {
    if (kDet[ii]) delete kDet[ii];
  }
  kDet.clear();
}

void makaMerger::setUpDetectors(){
  for (uint32_t ii=0; ii<kDetAddrs.size(); ii++) {
    kDet.push_back(new tcpclient(kDetAddrs[ii].c_str(), (int)kDetPorts[ii], kVerbosity));
  }
}
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
  Run managment
------------------------------------------------------------------------------*/
void makaMerger::runStart(){
  kNEvts   = 0;
  kNEvtsCal = 0;
  kNEvtsBeam = 0;
  kRunning = true;

  printf("%s) Setup Detectors\n", __METHOD_NAME__);
  //Start clients
  setUpDetectors();
  //Create UDP server for On-line Monitor
  omClient = new udpClient(kUdpAddr, kUdpPort, false);

  printf("%s) Spawn thread\n", __METHOD_NAME__);
  //Start thread to merge data
  kMerger3d = std::thread(&makaMerger::merger, this);
}

void makaMerger::runStop(int _sleep){
  sleep(_sleep);
  
  printf("%s) Stop run\n", __METHOD_NAME__);
  kRunning = false;

  printf("%s) Clear detectors\n", __METHOD_NAME__);
  //Close clients
  clearDetectors();

  printf("%s) Stop thread\n", __METHOD_NAME__);
  //Stop thread
  if (kMerger3d.joinable()) kMerger3d.join();

  printf("%s) Delete UDP Server\n", __METHOD_NAME__);
  //UDP Server for OM
  delete omClient;
}
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
  Merger, collector
------------------------------------------------------------------------------*/
int makaMerger::fileHeader(FILE* _dataFile){
  size_t writeRet = 0;
  uint32_t tempData;

  tempData = 0xb01adeee;
  writeRet += fwrite(&tempData, 4, 1, _dataFile); //Known word
  writeRet += fwrite(&kRunTime, 4, 1, _dataFile); //UNIX time of the run
  writeRet += fwrite(GIT_HASH, 4, 1, _dataFile); //MAKA git hash

  uint8_t type = 0x01;
  uint16_t version = 0x0200;
  tempData =  ((type & 0x0f) << 28)
            | ((version & 0x0fff) << 16)
            | (kDetAddrs.size() & 0xffff);
  writeRet += fwrite(&tempData, 4, 1, _dataFile); //Type, Data Version, # detectors

  for (auto id : kDetIds){
    writeRet += fwrite(&id, 2, 1, _dataFile); //Detector ID[n]
  }
  //0 padding to 32 bits
  if (kDetAddrs.size()%2 == 1){
    tempData = 0;
    writeRet += fwrite(&tempData, 2, 1, _dataFile); //0 Padding
  }

  return 0;
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

int makaMerger::merger(){
  char dataFileName[255];
  unsigned int lastNEvents = 0;
  using clock_type = std::chrono::system_clock;
  //using clock_type = std::chrono::high_resolution_clock; //Increase precision
  
  //Open a file in the kdataPath folder and name it with UTC
  
  // // copy kRunType and make it all UPPERCASE
  // std::string kRunType_upper{kRunType};
  // std::transform(begin(kRunType_upper), end(kRunType_upper),
  //                   begin(kRunType_upper), std::toupper);

  string humanDate = fileFormatDate(kRunTime);
  if (kDataToFile)
    sprintf(dataFileName,"%s/SCD_RUN%05d_%s_%s.dat", kDataPath.data(), kRunNum, kRunType.c_str(), humanDate.c_str());
  else
    sprintf(dataFileName,"/dev/null");

  printf("%s) Opening output file: %s\n", __METHOD_NAME__, dataFileName);
  FILE* dataFileD = fopen(dataFileName,"w");
  if (dataFileD == nullptr) {
    printf("%s) Error: file %s could not be created. Do the data dir %s exist?\n", __METHOD_NAME__, dataFileName, kDataPath.data());
    return -1;
  }
  //Write header to data file
  fileHeader(dataFileD);

  //----------------------------------------------------------------------------
  //Collect data from clients
  while(kRunning) {
    usleep(100); //?
    auto start = clock_type::now();
    collector(dataFileD);
    auto stop = clock_type::now();

    //if(kNEvts != lastNEvents){
    //    if(kNEvts%10 == 0){
      std::cout << "\rEvent " << kNEvts << " (Cal: " << kNEvtsCal << " - Phys: "\
                << kNEvtsBeam << " ) last recordEvents took " << std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() << " us                            " << std::flush;
      //lastNEvents = kNEvts;
      //    }
  }
  std::cout << std::endl;
  
  //----------------------------------------------------------------------------
  //Close file
  fclose(dataFileD);
  printf("%s) File %s closed\n", __METHOD_NAME__, dataFileName);

  return 0;

}

int makaMerger::collector(FILE* _dataFile){
  int readRet = 0;
  int writeRet = 0;
  //std::vector<uint32_t*> evts(det.size(), "");
  uint32_t evtLen = 0;
  uint32_t evtLen_tot = 0;
  std::vector<uint32_t> evt(652);

  // FIX ME: at most 64 detectors
  std::bitset<64> replied{0};
  
  uint32_t evtHeader;
  uint32_t evtLenHeader = sizeof(uint32_t)*(kDet.size() * 651 + 3);
  bool headerWritten = false;
  struct timespec utc_time;
  //long long sec;
  //long nsec;
  bool dataToOm = kDataToOm & (kNEvts%kOmPreScale==0 ? true : false);
  //printf("%s) dataToOm value: %s", __METHOD_NAME__, dataToOm?"true":"false");
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
          evtHeader = 0xfa4af1ca;
	        fwrite(&evtHeader, 4, 1, _dataFile); //Known word
          if (dataToOm) omClient->Tx(&evtHeader, 4);
          
          //UTC time for synchronization
          clock_gettime(CLOCK_REALTIME, &utc_time);
          fwrite(&utc_time, sizeof(utc_time), 1, _dataFile);
          //sec  = utc_time.tv_sec;
          //nsec = utc_time.tv_nsec;

          //FIX ME: Use real lenght of the event, not this pre-computed one
          fwrite(&evtLenHeader, 4, 1, _dataFile); //Event length
          if (dataToOm) omClient->Tx(&evtLenHeader, 4);
          
          fwrite(&kNEvts, 4, 1, _dataFile); //Event number
          if (dataToOm) omClient->Tx(&kNEvts, 4);

          //FIX ME: Use real type of the event, not this pre-computed one
          evtHeader =  0x10000000 | (kDetAddrs.size() & 0xffff);
          fwrite(&evtHeader, 4, 1, _dataFile); //
          if (dataToOm) omClient->Tx(&evtHeader, 4);
          
          //Separate cal and physics event counters
          uint32_t i2cWord = evt[6];
          bool i2cType = i2cWord & 0x1;
          //printf("%s) I2C word: %08x - Trigger Type: %d\n", __METHOD_NAME__, i2cWord, i2cType);

          kNEvtsCal += !i2cType;
          kNEvtsBeam += i2cType;

          ++kNEvts;
	        headerWritten = true;
	      }
	      writeRet += fwrite(evt.data(), evtLen, 1, _dataFile); //Event to file
        if (dataToOm) omClient->Tx(evt.data(), evtLen); //Event to OM

	      if (kVerbosity>0) {
	        printf("%s) Get event from DE10 %s\n", __METHOD_NAME__,\
                    kDetAddrs[ii].c_str());
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


int makaMerger::getEvent(std::vector<uint32_t>& _evt, uint32_t& _evtLen, int _det){
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
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
  makaMerger TCP server
------------------------------------------------------------------------------*/
void* makaMerger::listenCmd(){
  //Accept new connections and handshake commands length
  AcceptConnection();
  cmdLenHandshake();

  bool listenCmd = true;
  int bytesRead  = 0;
  while(listenCmd) {
    char msg[256]="";
    bytesRead = 0;

    //Read the command
    bytesRead = Rx(msg, kCmdLen);

    //Check if the read is ok and process its content
    if(bytesRead < 0) {
      if (EAGAIN==errno || EWOULDBLOCK==errno) {
        printf("%s) errno: %d\n", __METHOD_NAME__, errno);
      }
      else {
        print_error("%s) Read error: (%d)\n", __METHOD_NAME__, errno);
      }
    }
    else if (bytesRead==0) {
      listenCmd = false;
      printf("%s) Client closed the connection\n", __METHOD_NAME__);
    }
    else {
      processCmds(msg);
    }
    bzero(msg, sizeof(msg));
  }

  return nullptr;
}

void makaMerger::cmdLenHandshake(){
  Rx(&kCmdLen, sizeof(int));
  printf("%s) Updating command length to %d\n", __METHOD_NAME__, kCmdLen);
  Tx(&kCmdLen, sizeof(int));
  return;
}

void makaMerger::processCmds(char* msg){
  if (strcmp(msg, "cmd=setup") == 0) {
    cmdReply("setup");

    clearDetLists();

    printf("%s) Received setup command\n", __METHOD_NAME__);
    
    int pktLen = 0; //Packet length in bytes
    void* rxData;

    //Receive length and configPacket
    Rx(&pktLen, sizeof(int));
    
    rxData = malloc(pktLen);
    Rx(rxData, pktLen);


    //Deserialize data into configPacket class
    cpRx->des((uint32_t*)rxData);
    printf("Configurations received:\n");
    cpRx->dump();
    
    //Copy configuration data
    kDataPath = cpRx->dataPath;
    kDetIds   = cpRx->ids;
    kDetPorts = cpRx->ports;
    kDetAddrs = cpRx->addrs;
    kDataToFile = cpRx->dataToFile;
    kDataToOm = cpRx->dataToOm;
    kOmPreScale = cpRx->omPreScale;
    
    free(rxData);
    
    Tx(&kOkVal, sizeof(kOkVal));

  }
  else if (strcmp(msg, "cmd=runStart") == 0) {
    cmdReply("runStart");

    printf("%s) Received runStart command\n", __METHOD_NAME__);
    
    int pktLen = 0; //Packet length in bytes
    void* rxData;

    //Receive length and startPacket
    Rx(&pktLen, sizeof(int));
    
    rxData = (uint32_t*)malloc(pktLen);
    Rx(rxData, pktLen);

    //Deserialize data into configPacket class
    spRx->des((uint32_t*)rxData);
    printf("Start configurations received:\n");
    spRx->dump();
    
    //Copy configuration data
    kRunType = spRx->type;
    kRunNum  = spRx->num;
    kRunTime = spRx->time;

    //Start run
    printf("%s) Starting run %u: %s - %x ...\n", __METHOD_NAME__, kRunNum,
              kRunType.c_str(), kRunTime);
    runStart();
    
    free(rxData);

    Tx(&kOkVal, sizeof(kOkVal));
  }
  else if (strcmp(msg, "cmd=runStop") == 0) {
    cmdReply("runStop");

    printf("%s) Stop run %u with %u events.\n", __METHOD_NAME__, kRunNum, kNEvts);
    runStop();
    Tx(&kOkVal, sizeof(kOkVal));
  }
  else {
    printf("%s) Unknown message: %s\n", __METHOD_NAME__, msg);
    Tx(&kBadVal, sizeof(kBadVal));
  }
}

void makaMerger::cmdReply(const char* cmd){
  char cmdReadBack[256]="";
  sprintf(cmdReadBack, "rcv=%s", cmd);
  Tx(cmdReadBack, kCmdLen);
}

//------------------------------------------------------------------------------
