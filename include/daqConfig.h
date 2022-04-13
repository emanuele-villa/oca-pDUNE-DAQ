/*!
  @file daqConfig.h
  @brief Read DAQ configuration parameters from a .cfg file.
 */

#ifndef _daqConfig_H
#define _daqConfig_H

#include <string>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <limits>
#include <fstream>
#include <sstream>

/*!
  Read configuration parameters from file and populate a struct
*/
class daqConfig
{
  
  public:
    daqConfig(const std::string& filePath){
      conf = {true, 8888, "./data/", 64, 0, 0};
      readConfigFromFile(filePath);
    };

    //!Definition of configuration parameters struct
    struct configParams {
      bool listenClient;      //!Listen to external TCP client
      int portClient;         //!TCP server port number
      std::string dataFolder; //!Data folder
      int clientCmdLen;       //!Command length w.r.t. TCP client
      bool intTrigEn;         //!Internal trigger enable
      bool calMode;           //!Calibration mode enable
    };

    //! Open configuration file and read it in an istream
    void readConfigFromFile(const std::string& filePath);

    //!
    configParams getParams(){ return conf;};

  private:
    //! Configuration parameters
    configParams conf;

    //! Open an input file and get the streamer object
    void openInputFile(const std::string& filePath,
                              std::ifstream& inputFile);

    //! Read and apply configuration
    int config(std::istream& is);

    //! Generic function to read a single option
    template <typename T>
    void readOption(T& option, std::istream& is)
    {
      if (is.peek() == '\n') exit(1);
      is >> option;
    }

};

#endif //_daqConfig_H