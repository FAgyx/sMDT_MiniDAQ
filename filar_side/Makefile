#************************************************************
# file: Makefile
#
# 13. Dec. 01  MAJO created
#
#************************************************************
include $(DAQ_ROOT_DIR)/rules/makerules

CC	   = gcc
CPP	   = g++
RM	   = rm -f

#         CONDITIONAL ON GOLA CARD VERSION
#    As of Feb 6, 2004 this conditional does nothing at all
#    Future releases will almost certainly re-implement it
#
#  For the older, Rev 38, one-channel GOLA cards, define the following:
# ONE_CHAN   = -DONE_CHANNEL_CARD
#  For the newer, Rev 46, 4-channel GOLA cards, define instead:
ONE_CHAN   =
#
#         END CONDITIONAL ON GOLA CARD VERSION

CFLAGS	   = -g -m32 -I$(DAQ_SW_DIR)/include -DTSTAMP
#    Removed this from CFLAGS for SL5.5  -DBIGPHYSAREA
LFLAGS     = -L$(DAQ_SW_DIR)/lib -lcmem_rcc -lio_rcc -lrcc_time_stamp \
             -lgetinput -lrcc_error -lm -lDFDebug

.c.o:
	$(CC) $(ONE_CHAN) $(CFLAGS) -c $<

all: csm_utils.o filar_utils.o CSMtest CSMMiniDAQ.exe EBtoy filarscope.exe

filar_utils.o:	filar_utils.c filar_utils.h filar_storage.h

csm_utils.o:	csm_utils.c csm_utils.h filar_storage.h

sockhelp.o:	sockhelp.h
# CSMMiniDAQ.exe: CSMMiniDAQ_display.cpp filar_utils.o CSMMiniDAQ_display.h csm_utils.o sockhelp.o
#	$(CPP) $(CFLAGS) CSMMiniDAQ_display.cpp sockhelp.o csm_utils.o filar_utils.o $(LFLAGS) -o CSMMiniDAQ.exe

CSMMiniDAQ.exe: CSMMiniDAQ_dualCSM.cpp class_CSM.cpp filar_utils.o CSMMiniDAQ_display_new.h csm_utils.o sockhelp.o
	$(CPP) $(CFLAGS) CSMMiniDAQ_dualCSM.cpp  sockhelp.o csm_utils.o filar_utils.o $(LFLAGS) -o CSMMiniDAQ.exe


# CSMMiniDAQ.exe: CSMMiniDAQ_triggerless_AMT.cpp filar_utils.o CSMMiniDAQ.h csm_utils.o sockhelp.o
#	$(CPP) $(CFLAGS) CSMMiniDAQ_triggerless_AMT.cpp sockhelp.o csm_utils.o filar_utils.o $(LFLAGS) -o CSMMiniDAQ.exe



# CSMMiniDAQ.exe: CSMMiniDAQ_AMT_HPTDC.cpp filar_utils.o CSMMiniDAQ.h csm_utils.o sockhelp.o
#	$(CPP) $(CFLAGS) CSMMiniDAQ_AMT_HPTDC.cpp sockhelp.o csm_utils.o filar_utils.o $(LFLAGS) -o CSMMiniDAQ.exe




CSMtest: CSMtest.cpp filar_utils.o csm_utils.o CSMtest.h
	$(CPP) $(CFLAGS) CSMtest.cpp csm_utils.o filar_utils.o $(LFLAGS) -o CSMtest

CSMtestRT: CSMtestRT.c
	$(CC) $(CFLAGS) CSMtestRT.c $(LFLAGS) -o CSMtestRT

EBtoy:	EBtoy.cpp
	$(CPP) EBtoy.cpp -o EBtoy

filarscope.exe: filarscope.cpp
	$(CPP) $(CFLAGS) filarscope.cpp $(LFLAGS) -o filarscope.exe

clean:
	$(RM) CSMtestRT
	$(RM) pciblast
	$(RM) CSMtest
	$(RM) CSMMiniDAQ.exe
	$(RM) filar_utils.o
	$(RM) csm_utils.o
	$(RM) sockhelp.o
	$(RM) filarscope.o
	$(RM) EBtoy
