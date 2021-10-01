# Folder structure:
OBJ := obj
OBJARM := objarm
SRC := src
INC := include
EXE := exe

# Compilers:
ifndef ROOTSYS
	CXX = g++
	CC = gcc
	F77 = gfortran
else
	CXX = $(shell root-config --cxx)
	CC  = $(shell root-config --cc)
	F77 = $(shell root-config --f77)
endif

UNAME_S := $(shell uname -s)

CROSS_COMPILE = arm-linux-gnueabihf
ifeq ($(UNAME_S),Linux)
	CCARM = $(CROSS_COMPILE)-g++
	LDARM = $(CROSS_COMPILE)-g++
endif
ifeq ($(UNAME_S),Darwin)
	CCARM = `brew --prefix llvm`/bin/clang --target=$(CROSS_COMPILE) --gcc-toolchain=`brew --prefix $(CROSS_COMPILE)-binutils`
	LDARM = `brew --prefix llvm`/bin/clang --target=$(CROSS_COMPILE) --gcc-toolchain=`brew --prefix $(CROSS_COMPILE)-binutils`
endif

# Root specific (unused for now):
ifdef ROOTSYS
	ROOTCFLAGS    =
	ROOTLIBS      =
	ROOTGLIBS     =
else
	ROOTCFLAGS    = $(shell root-config --cflags)
	ROOTLIBS      = $(shell root-config --libs)
	ROOTGLIBS     = $(shell root-config --glibs)
endif

# DE10 specific:
ALT_DEVICE_FAMILY ?= soc_cv_av
#SOCEDS_DEST_ROOT = /home/depa/intelFPGA/20.1/embedded
HWLIBS_ROOT = $(SOCEDS_DEST_ROOT)/ip/altera/hps/altera_hps/hwlib

# Flags and includes:
INCLUDE= -I$(INC) -I/Users/bozzo/Downloads/gcc-arm-10.3-2021.07-x86_64-arm-none-linux-gnueabihf/arm-none-linux-gnueabihf/include/c++/10.3.1/
ifdef ROOTSYS
	INCLUDE += -I$(ROOTSYS)/include
endif

CFLAGS := -g -Wall -pthread
LDFLAGS := -g -Wall -pthread

CPPFLAGS := $(CFLAGS) $(INCLUDE)
CFLAGSARM := $(CFLAGS) -I$(INCLUDE) -I$(HWLIBS_ROOT)/include -I$(HWLIBS_ROOT)/include/$(ALT_DEVICE_FAMILY) -D$(ALT_DEVICE_FAMILY)

# Objects and sources:
OBJECTS=$(OBJ)/main.o $(OBJ)/de10_silicon_base.o $(OBJ)/tcpclient.o $(OBJ)/daqserver.o $(OBJ)/tcpserver.o $(OBJ)/utility.o
OBJECTSTEST=$(OBJ)/maintest.o $(OBJ)/daqclient.o $(OBJ)/tcpclient.o $(OBJ)/utility.o

OBJECTSHPS := $(OBJARM)/server.o $(OBJARM)/server_function.o $(OBJARM)/highlevelDriversFPGA.o $(OBJARM)/lowlevelDriversFPGA.o

# Executables:
HPSSERVER := $(EXE)/server
OCADAQ := $(EXE)/OCA
OCATEST := $(EXE)/testOCA

# Rules:
all: $(OCADAQ) $(OCATEST) $(HPSSERVER)

daq: $(OCADAQ) $(OCATEST)

hps: $(HPSSERVER)

$(OCADAQ): $(OBJECTS)
	@echo Linking $^ to $@
	@mkdir -pv $(EXE)
	$(CXX) $^ -o $@ $(ROOTGLIBS)

$(OCATEST): $(OBJECTSTEST)
	@echo Linking $^ to $@
	@mkdir -pv $(EXE)
	$(CXX) $^ -o $@ $(ROOTGLIBS)

$(HPSSERVER): $(OBJECTSHPS)
	@echo Linking $^ to $@
	@mkdir -pv $(EXE)
	$(LDARM) $(LDFLAGS) $^ -o $(HPSSERVER)

##SUMMARY: $(TOP)/TakeData/summary.o  $(TOP)lib/libamswire.a
#SUMMARY: $(TOP)/TakeData/summary.o
#	@echo Linking $@ ...
##	$(CXX) $(CFLAGS) $(ROOTCFLAGS) $(CPPFLAGS) -o $@ $(TOP)/TakeData/summary.o -L$(TOP)lib/ -lamswire $(ROOTLIBS)
#	$(CXX) $(CFLAGS) $(ROOTCFLAGS) $(CPPFLAGS) -o $@ $(TOP)/TakeData/summary.o $(ROOTLIBS)
#	ln -fs SUMMARY SUMMARY_MUTE

$(OBJ)/%.o: $(SRC)/%.cpp
	@echo Compiling $< ...
	@mkdir -pv $(OBJ)
	$(CXX) $(CPPFLAGS) -c -o $@ $<

#Objects
$(OBJARM)/%.o: $(SRC)/%.c
	@echo Compiling $< ...
	@mkdir -pv $(OBJARM)
	$(CCARM) $(CFLAGSARM) -c -o $@ $<

clean:
	@echo " Cleaning..."
	@$(RM) -Rfv $(OBJ)
	@$(RM) -Rfv $(OBJARM)
	@$(RM) -Rfv $(EXE)
#	@$(RM) -fv ./SUMMARY
#	@$(RM) -fv ./SUMMARY_MUTE

.PHONY: clean
