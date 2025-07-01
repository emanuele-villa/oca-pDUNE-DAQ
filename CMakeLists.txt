cmake_minimum_required( VERSION 3.10 FATAL_ERROR )

# Define project
project( oca-pDUNE-DAQ )


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions( -D GIT_HASH=0 )
add_definitions( -D COMPILE_TIME=0 )
add_definitions( -D GIT_BRANCH=0 )

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable( oca-pDUNE-DAQ-MAKA
    src/maka.cpp
    src/makaMerger.cpp
    src/utility.cpp
    src/tcpServer.cpp
    src/tcpclient.cpp
    src/udpSocket.cpp
)

add_executable( oca-pDUNE-DAQ-OCA
    src/main.cpp
    src/de10_silicon_base.cpp
    src/tcpclient.cpp
    src/daqserver.cpp
    src/tcpServer.cpp
    src/utility.cpp
    src/daqConfig.cpp
    src/paperoConfig.cpp
    src/anyoption.cpp
    src/makaClient.cpp
    src/udpSocket.cpp
)

add_executable( oca-pDUNE-DAQ-startOCA
    src/mainstart.cpp
    src/daqclient.cpp
    src/tcpclient.cpp
    src/utility.cpp
)

add_executable( oca-pDUNE-DAQ-stopOCA
    src/mainstop.cpp
    src/daqclient.cpp
    src/tcpclient.cpp
    src/utility.cpp
)
add_executable( oca-pDUNE-DAQ-testOCA
    src/maintest.cpp
    src/daqclient.cpp
    src/tcpclient.cpp
    src/utility.cpp
)


#add_executable( oca-pDUNE-DAQ-PAPERO
#    src/papero.cpp src/hpsDataServer.cpp src/hpsServer.cpp src/tcpServer.cpp src/utility.cpp src/fpgaDriver.cpp src/axiFifo.cpp
#)



install( TARGETS oca-pDUNE-DAQ-MAKA DESTINATION bin )
install( TARGETS oca-pDUNE-DAQ-OCA DESTINATION bin )
install( TARGETS oca-pDUNE-DAQ-startOCA DESTINATION bin )
install( TARGETS oca-pDUNE-DAQ-stopOCA DESTINATION bin )
install( TARGETS oca-pDUNE-DAQ-testOCA DESTINATION bin )
#install( TARGETS oca-pDUNE-DAQ-PAPERO DESTINATION bin )
