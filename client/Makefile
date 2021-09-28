CXX = $(shell root-config --cxx)
CC  = $(shell root-config --cc)
F77 = $(shell root-config --f77)

OBJ=obj
SRC=src
INC=include

ROOTCFLAGS    = $(shell root-config --cflags)
ROOTLIBS      = $(shell root-config --libs)
ROOTGLIBS     = $(shell root-config --glibs)

INCLUDE= -I$(INC) -I$(ROOTSYS)/include

CFLAGS= -g
CPPFLAGS+= $(INCLUDE) 

OBJECTS=$(OBJ)/main.o $(OBJ)/de10silicon_base.o $(OBJ)/tcpclient.o $(OBJ)/daqserver.o $(OBJ)/tcpserver.o $(OBJ)/utility.o
OBJECTSTEST=$(OBJ)/maintest.o $(OBJ)/daqclient.o $(OBJ)/tcpclient.o $(OBJ)/utility.o

all: OCA testOCA

OCA: $(OBJECTS)
	@echo Linking $^ to $@
	$(CXX) $^ -o $@ $(ROOTGLIBS)

testOCA: $(OBJECTSTEST)
	@echo Linking $^ to $@
	$(CXX) $^ -o $@ $(ROOTGLIBS)

##SUMMARY: $(TOP)/TakeData/summary.o  $(TOP)lib/libamswire.a
#SUMMARY: $(TOP)/TakeData/summary.o
#	@echo Linking $@ ...
##	$(CXX) $(CFLAGS) $(ROOTCFLAGS) $(CPPFLAGS) -o $@ $(TOP)/TakeData/summary.o -L$(TOP)lib/ -lamswire $(ROOTLIBS)
#	$(CXX) $(CFLAGS) $(ROOTCFLAGS) $(CPPFLAGS) -o $@ $(TOP)/TakeData/summary.o $(ROOTLIBS)
#	ln -fs SUMMARY SUMMARY_MUTE

$(OBJ)/%.o: $(SRC)/%.cpp
	@echo Compiling $< ...
	@if ! [ -d $(OBJ) ] ; then mkdir -pv $(OBJ); fi
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	@rm -fv $(OBJ)/*.o
	@rm -fv ./OCA
	@rm -fv ./testOCA
#	rm -fv ./SUMMARY
#	rm -fv ./SUMMARY_MUTE
