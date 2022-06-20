/*!
  \file makaConfig.h
  \brief Class for configuration and (de)serialization
  \author Mattia Barbanera (mattia.barbanera@infn.it)
*/

#include <vector>

//#pragma pack(push,1)
#pragma pack(1)
class configPacket {

  protected:
    uint32_t pktLen;
    uint32_t detNum;
    uint32_t pathLen;
    std::vector<uint32_t> addrsSize;

  public:
    std::vector<uint32_t> ports;
    std::vector<const char*> addrs;
    char* dataPath;
    uint32_t* msg;

    configPacket(std::vector<uint32_t> _ports, std::vector<const char*> _addrs,\
                  char* _dataPath) {
      ports = _ports;
      addrs = _addrs;
      dataPath = _dataPath;
      sizeUpdate();
    };
    
    configPacket(){
      pktLen = 0;
      detNum = 0;
      pathLen = 0;
      dataPath = nullptr;
      msg = nullptr;
    };

    ~configPacket(){
      if (dataPath != nullptr) free(dataPath);
      if (msg != nullptr) free(msg);
    };

    //! \brief Serialize all the struct fields into a uint32_t* buffer
    void ser() {
      sizeUpdate();
      msg = (uint32_t*)malloc(pktLen);
      uint32_t* outInt = msg;
      uint32_t jj, mm;
  
      //Serialize uint32_t
      *outInt = pktLen;
      outInt++;
      *outInt = detNum;
      outInt++;
      *outInt = pathLen;
      outInt++;
  
      //Serialize ports
      for (auto port : ports) {
        *outInt = port;
        outInt++;
      }
  
      //Serialize address sizes
      for (auto addrSize : addrsSize) {
        *outInt = addrSize;
        outInt++;
      }
  
      char* outChar = (char*)outInt;
      //Serialize addresses
      for (jj=0; jj<detNum; jj++) {
        for (mm=0; mm<addrsSize[jj]; mm++){
          *outChar = addrs[jj][mm];
          outChar++;
        }
      }

      //Serialize data path
      for (mm=0; mm<pathLen; mm++){
          *outChar = dataPath[mm];
          outChar++;
        }

    }

    //! \brief Deserialize a uint32_t* buffer into fields of the struct
    void des(uint32_t* _in) {
      uint32_t desTemp;
      uint32_t* inInt = _in;
      uint32_t jj;

      //Deserialize uint32_t
      pktLen = *inInt;
      inInt++;
      detNum = *inInt;
      inInt++;
      pathLen = *inInt;
      inInt++;
      for (jj=0; jj<detNum; jj++) {
        //Deserialize port
        desTemp = *inInt;
        inInt++;
        ports.push_back(desTemp);

        //Debug
        printf("Ports %d\n", ports[jj]);
      }

      for (jj=0; jj<detNum; jj++) {
        //Deserialize address length
        desTemp = *inInt;
        inInt++;
        addrsSize.push_back(desTemp);

        //Debug
        printf("AddrsSize %d\n", addrsSize[jj]);
      }

      char* inChar = (char*)inInt;
      //Deserialize addresses
      for (jj=0; jj<detNum; jj++) {
        char addrTmp[16];
        bzero(addrTmp, sizeof(addrTmp));
        memcpy(addrTmp, inChar, addrsSize[jj]);
        inChar += addrsSize[jj];
        //bzero(addrTmp, sizeof(addrTmp));
        //for (mm=0; mm<addrsSize[jj]; mm++){
        //  addrTmp[mm] = *inChar;
        //  inChar++;
        //}
        addrs.push_back(addrTmp);

        //Debug
        printf("Addresses %s (%d)\n", addrs[jj], addrsSize[jj]);
      }

      //Deserialize path
      dataPath = (char*) malloc(pathLen);
      memcpy(dataPath, inChar, pathLen);
      inChar += pathLen;

    }

    //! \brief Print all the class values
    void dump() {
      printf("Configurations packet:\n");
      printf("  Packet length:        %u\n",  pktLen);
      printf("  Number of detectors:  %u\n",  detNum);
      printf("  Path Length:          %u\n",  pathLen);
      printf("  Path:                 %s\n",  dataPath);
      printf("  Detectors:\n");
      for (size_t ii=0; ii<ports.size(); ii++){
        printf("      %lu:                %s:%u,%u\n", ii, addrs[ii], ports[ii], addrsSize[ii]);
      }
    }

    //! \brief << operator overload for streams
    friend std::ostream& operator <<(std::ostream& _os, const configPacket & _cp) {
      _os << "Configurations packet:  " << std::endl;
      _os << "  Packet length:        " << _cp.pktLen << std::endl
          << "  Number of detectors:  " << _cp.detNum << std::endl
          << "  Path Length:          " << _cp.pathLen << std::endl
          << "  Path:                 " << _cp.dataPath << std::endl;
      _os << "  Detectors:" << std::endl;
      for (size_t ii=0; ii<_cp.ports.size(); ii++){
        _os << "      " << ii << ":                " << _cp.addrs[ii] << ":"
            << _cp.ports[ii] << ", " << _cp.addrsSize[ii] << std::endl;
      }
      return _os;
    }

    //! \brief Update all the size fields
    void sizeUpdate() {
      if (ports.size() != addrs.size()) {
        printf("Error: Port and address sizes mismatch\n");
        exit(1);
      }

      detNum = ports.size();

      addrsSize.clear();
      uint32_t sizeAddrTot = 0;
      for (auto addr : addrs) {
        addrsSize.push_back(strlen(addr));
        sizeAddrTot += strlen(addr);
      }

      pathLen = strlen(dataPath);

      pktLen = sizeof(pktLen) + sizeof(detNum) + sizeof(pathLen) + detNum*(sizeof(ports [0]) + sizeof(addrsSize[0]))\
                 + sizeAddrTot + pathLen;

    }

};
//#pragma pack(pop)
//}__attribute__((packed));