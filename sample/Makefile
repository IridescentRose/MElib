TARGET = MElibTest
OBJS = test.o

INCDIR = ../include
CFLAGS = -G0 -Wall -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = ../lib
LDFLAGS = 
LIBS= -lme -lpsprtc -lpspsdk

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = MElib Test

PSP_FW_VERSION=500
PSP_LARGE_MEMORY = 0
BUILD_PRX = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
