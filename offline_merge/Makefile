#************************************************************
# file: Makefile
#
# 13. Dec. 01  MAJO created
#
#************************************************************


CC	   = gcc
CPP	   = g++
RM	   = rm -f


# CFLAGS	   = -g -m32 -I$(DAQ_SW_DIR)/include -DTSTAMP
# #    Removed this from CFLAGS for SL5.5  -DBIGPHYSAREA
# LFLAGS     = -L$(DAQ_SW_DIR)/lib -lcmem_rcc -lio_rcc -lrcc_time_stamp \
#              -lgetinput -lrcc_error -lm -lDFDebug

.c.o:
	$(CC) $(ONE_CHAN) $(CFLAGS) -c $<

all:  OfflineMerge DeleteMeta Increment

OfflineMerge: OfflineMerge.cpp
	$(CPP) $(CFLAGS) OfflineMerge.cpp $(LFAGS) -o OfflineMerge

DeleteMeta: DeleteMeta.cpp
	$(CPP) $(CFLAGS) DeleteMeta.cpp $(LFAGS) -o DeleteMeta

Increment: IncrementHeaders.cpp
	$(CPP) $(CFLAGS) IncrementHeaders.cpp $(LFAGS) -o Increment

clean:
	$(RM) OfflineMerge 
	$(RM) DeleteMeta
	$(RM) Increment

