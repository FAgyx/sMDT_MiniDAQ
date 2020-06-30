#ifndef CSM_UTILS_H

#define CSM_UTILS_H

// *************************** csm_utils.h ****************************************

// ****** NOTE: 1/31/04, NOT YET MODIFIED FOR MORE THAN ONE CHANNEL OF 4-CHAN GOLA

// Wanted to use static const int BUILD_SIZE in declarations, but compiler screams
// static const int BUILD_SIZE = 10;
#define BUILD_SIZE 10

#ifdef CSM_UTILS_DEFINE       // For csm_utils.c, do this....

struct sigaction sa;
sigjmp_buf env;

int buildChannels[18];
unsigned int *bufPtrs[BUILD_SIZE];
int bufPtrSize[BUILD_SIZE];
int activeChanCount;
int seqNo[BUILD_SIZE], newSeqNo;
int activeEvid[BUILD_SIZE], matchedCount[BUILD_SIZE], eFlush[BUILD_SIZE];
int haveHeader[BUILD_SIZE][18];
int haveTrailer[BUILD_SIZE][18];

unsigned int noHead[18], noTrail[18];
int incompleteBuild;
int goodBuild;
int forcedFlush;
int activeTriggers;
unsigned int cont;

// The following are not shared, essentially "protected" variables in C++ parlance
int flushEvid;
int bufMaxSize;
int numBufsToVend;
int bufsVended;
unsigned int *pBufsToVend[BUILD_SIZE];
int bufNextUse;
int bufToReturn;
int firstBuildId;
int thisBuildId[18];
unsigned int buffer1[4096];
unsigned int buffer2[4096];
unsigned int buffer3[4096];

// Variables used in repeated calls to getNextEvent
int firstflag, firstValidOffset, TDCChIndex;
unsigned int gsaveLoc;
int cycleOK;
int validDataInCycle;
int haveFlushId;
int cycleQuit;
int quitTimeBuildId;
int quitOnEvFlush;
int notVeryFirstCycle;

#else                         // if NOT csm_utils.c, do this....

#include <signal.h>
#include <setjmp.h>
extern struct sigaction sa;
extern sigjmp_buf env;

extern int buildChannels[18];
extern unsigned int *bufPtrs[BUILD_SIZE];
extern int bufPtrSize[BUILD_SIZE];
extern int activeChanCount;
extern int seqNo[BUILD_SIZE], newSeqNo;
extern int activeEvid[BUILD_SIZE], matchedCount[BUILD_SIZE], eFlush[BUILD_SIZE];
extern int haveHeader[BUILD_SIZE][18];
extern int haveTrailer[BUILD_SIZE][18];

extern unsigned int noHead[18], noTrail[18];
extern int incompleteBuild;
extern int goodBuild;
extern int forcedFlush;
extern int activeTriggers;
extern unsigned int cont;

#endif

#define BD_CSM_HEAD  0x59000000
#define BD_EVID_MSK  0x00fff000
#define BD_BCID_MSK  0x00000fff
#define BD_CSM_TRL   0x5b000000
#define BD_CSM_ETRL  0x5d000000
#define BD_HDR_TRLR  0xF0FFFFFF
#define BD_SHRT_CHAN 0x0000000F
#define BUILD_MASK1  0xF0FFFFFF
#define BUILD_MASK2  0XF03FFFFF
#define BUILD_MASK3  0x00C00000

typedef struct 
{
  int maskArray[18];
  int activeChanCount;
  int checkSignals;
  unsigned int *watchVariable;
  unsigned int watchValue;
  int evid;
  int emptyCycles;
  int badCycleCount;
  unsigned int *dataBuffer;
  int bufLen;
  int bufSize;
  unsigned int *builtEvent;
  int eventLen;
  int eventSize;
}   oneEventInfo_T;

// Function prototypes

  #ifdef __cplusplus
    extern "C" {
  #endif

void buildEvInit( oneEventInfo_T *evInfo );
int daqEvInit( int numbuffersreq, int activeChannel );
int getNextEvent( oneEventInfo_T *evInfo, int flowIsTDM, int *offset, unsigned int *dataptr, unsigned int fsize );

int captureEvent( int flowIsTDM, oneEventInfo_T *evInfo );
void buildInit();
int whichId( unsigned int dataWord, int doPrint );
int checkFlush( int buildID, int chanNo );
void incBigValue( unsigned int *baseValue, unsigned int *overValue, unsigned int increment );
void printBigValue( unsigned int baseValue, unsigned int overValue, char *outString );
int checkParity( unsigned int dataWord, unsigned int *CommPEct );
unsigned int formedTdc( int ichan, unsigned int data );
void setCaptureBuffer( unsigned int *pBuf, int bufMaxSize );
void initCaptureBuffers();
unsigned int * getCaptureBuffer();
void returnCaptureBuffer( unsigned int *pBuf );
void addWordToBuffer( unsigned int *pBuf, unsigned int newWord, int *bufFillSize );

  #ifdef __cplusplus
    }
  #endif

#endif
