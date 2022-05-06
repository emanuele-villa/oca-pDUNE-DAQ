/*!
  @file hpsDataServer.cpp
  @brief TCP server to transfer scientific data to merger
  @author Mattia Barbanera (mattia.barbanera@infn.it)
  @author Matteo Duranti (matteo.duranti@infn.it)
*/

#include "hpsDataServer.h"

extern fpgaDriver* fpga;

hpsDataServer::hpsDataServer(int port, int verb):tcpServer(port, verb){
  //kBlocking = true;
  kIsRunning  = false;
  kEvtCount   = 0;
  //kData3d     = nullptr;

  //Setup TCP server
  Setup();
}

hpsDataServer::~hpsDataServer(){
  stopRun();
}

void hpsDataServer::startRun(){
  kIsRunning = true;

  //Await connection from merger
  AcceptConnection();

  //Spawn a thread to stream data from FPGA 
  kData3d = std::thread(&hpsDataServer::dataReading, this);
}

void hpsDataServer::stopRun(){
  //Stop the run, if any
  kIsRunning  = false;

  //Join the thread, if any
  if (kData3d.joinable()){
    kData3d.join();
  }

  //Kill connection to merger, if any
  StopListening();
}

void hpsDataServer::getSendEvt(vector<uint32_t>& evt){
  static int evtLen = -1;

  //Get an event from FPGA
  int evtErr = fpga->getEvent(evt, &evtLen);
  if (kVerbosity > 4) printf("getEvent result: %d\n", evtErr);
  
  //Send the event to the socket
  if (evtLen>0) { //ectLen == 0 if no event available
    Tx(evt.data(), evtLen*sizeof(uint32_t));
    kEvtCount++;
    if (kEvtCount % 1000 == 0) {
      auto evt1000 = chrono::system_clock::now();
      cout << "Event count : " << kEvtCount << " in " << chrono::duration_cast<chrono::milliseconds>(evt1000-kStartRunTime).count() << " s\n";
    }
  }
  if (kVerbosity > 3) printf("%s) Event sent\n", __METHOD_NAME__);
}

void hpsDataServer::dataReading(){
  //static evt so that the size (changed inside getEvent) is not continuosly changing
  //FIXME: static needed? Or create a bigger pool with reserve?
  static vector<uint32_t> evt;
	//std::vector<uint32_t> evt;
  
  kEvtCount = 0;
  

  ////Send a 'Start of run' word to the merger
  //uint32_t startMsg = 0xc0cac01a;
  //Tx(&startMsg, sizeof(startMsg));
  
  //Enable triggers in FPGA
  fpga->SetMode(0x10);
  
  //While kIsRunning, read events from FPGA and send them to merger
  kStartRunTime = chrono::system_clock::now();
  while (kIsRunning){
    //Free AXI resource available for other users (limit: 10 kHz)
    //usleep(100);
    
    getSendEvt(evt);
  }
  
  //When kIsRunning=false, disable triggers in FPGA and empty FIFOs
  fpga->SetMode(0);
  for (int ff=0; ff<100; ff++){
    getSendEvt(evt);
  }
  printf("%s) End of Event: Events still available after 100 readings.\n",
          __METHOD_NAME__);
  
}