/*!
  @file daqConfig.cpp
  @brief DAQ configuration methods.
*/

#include "daqConfig.h"

/*!
  
 */
void daqConfig::readConfigFromFile(const std::string& filePath)
{
  std::ifstream is;
  openInputFile(filePath, is);
  config(is);
  is.close();
}

/*!
  
 */
void daqConfig::openInputFile(const std::string& filePath, std::ifstream& inFile)
{
  std::cout << "From file " << filePath << " read ";
  inFile.open(filePath);
  if (not(inFile)) {
    std::cout << " could not open file " << filePath << ". Abort." << std::endl;
    exit(1);
  }
}

/*!
  
 */
int daqConfig::config(std::istream& is)
{
  int linesRead = 0;
  while(!is.eof())
  {
    char pip = is.peek();
    // Ignore comment or empty lines
    while((pip == '#') | (pip == '\n'))
    {
      is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      pip = is.peek();
    }
    
    readOption<bool>(conf.listenClient, is);
    readOption<int>(conf.portClient, is);
    readOption<std::string>(conf.dataFolder, is);
    readOption<int>(conf.clientCmdLen, is);
    readOption<bool>(conf.intTrigEn, is);
    readOption<bool>(conf.calMode, is);
    linesRead++;
    
    //Ignore newline
    if (is.peek() == '\n') is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  };
  std::cout << linesRead << " configuration lines." << std::endl;
  return linesRead;
}