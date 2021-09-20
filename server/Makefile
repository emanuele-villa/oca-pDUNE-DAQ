#Folder structure
SRCDIR := src
BUILDDIR := build
BINDIR := bin
SRCEXT := c
INCLUDE := -I include

#DE10 specific
ALT_DEVICE_FAMILY ?= soc_cv_av
#SOCEDS_DEST_ROOT = /home/depa/intelFPGA/20.1/embedded
HWLIBS_ROOT = $(SOCEDS_DEST_ROOT)/ip/altera/hps/altera_hps/hwlib

#Compiler
CROSS_COMPILE = arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
ARCH = arm

#Automatically detect source files and produces objects
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJFILES := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g -Wall -I$(HWLIBS_ROOT)/include -I$(HWLIBS_ROOT)/include/$(ALT_DEVICE_FAMILY) -D$(ALT_DEVICE_FAMILY)
LDFLAGS := -g -Wall
LIB := -pthread

TARGET := $(BINDIR)/sever

#Linking
$(TARGET): $(OBJFILES)
	@echo " Linking..."
	@echo " Sources: $(SOURCES)"
	@echo " Objects: $(OBJFILES)"
	@mkdir -p $(BINDIR)
	@echo " $(LD) $(LDFLAGS) $^ -o $(TARGET)"; $(LD) $(LDFLAGS) $^ -o $(TARGET) $(LIB)

#Objects
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<"; $(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

.PHONY: clean
