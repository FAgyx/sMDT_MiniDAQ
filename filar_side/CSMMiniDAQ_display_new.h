#ifndef CSMMiniDAQ_H
#define CSMMiniDAQ_H

//
// Filename: CSMMiniDAQ.h
// Author: T.S. Dai
//
// Description:
// This file defines parameters, variables and function prototypes for
// the CSM MiniDAQ Control.
//

//#include "csm_utils.h"   // In case this was not previously included

#define FALSE                             0
#define TRUE                              1

#define TYPEAMT1                          1
#define TYPEAMT2                          2
#define TYPEAMT3                          3
#define TYPEHPTDC                         4

#define State_Idle                        0
#define State_Starting                    1
#define State_Running                     2
#define State_Paused                      3

#define CMDPROBE                          1
#define CMDDETAILSTATUS                   2
#define CMDGETONEEVENTINIT               10
#define CMDINITDAQ                       11
#define CMDSTARTRUN                      12
#define CMDSTOPRUN                       13
#define CMDPAUSERUN                      14
#define CMDRESUMERUN                     15
#define CMDREQRAWDATA                    20
#define CMDREREQRAWDATA                  21
#define CMDREQONEEVENT                   22
#define CMDREREQONEEVENT                 23
#define CMDREQONESEQEVENT                24
#define CMDREREQONESEQEVENT              25
#define CMDREQEDGES                      30
#define CMDREQPAIR                       31
#define CMDREQHITS                       32
#define CMDREQAVERAGETDCTIME             33
#define CMDREQFRACTIONUSED               34
#define CMDREQAVERAGEWIDTH               35
#define CMDREQDAQINFOR                   50
#define CMDREREQDAQINFOR                 51
#define CMDREQALLDAQINFOR                52
#define CMDQUIT                         255

#define TCPLENGTH                         0
#define TCPCOMMAND                        1
#define TCPCSMNUMBER                      2
#define TCPCSMVERSION                     3
#define TCPSYNCHWORDCONTROL               4
#define TCPMEZZENABLES                    5
#define TCPEDGEPAIRENABLES0               6
#define TCPEDGEPAIRENABLES1               7
#define TCPGOLACARDPAGESIZE               8
#define TCPAMTTYPE                        9
#define TCPSAVERAWDATA                   10
#define TCPCHAMBERTYPE                   11
#define TCPCHAMBERHV                     12
#define TCPSUPPRESSCSMHEADER             13
#define TCPSUPPRESSCSMTRAILER            14
#define TCPSUPPRESSAMTHEADER             15
#define TCPSUPPRESSAMTTRAILER            16
#define TCPRUNNUMBER                     17
#define TCPNUMBEROFEVENT                 18
#define TCPNOMINALTHRESHOLD              19
#define TCPINTEGRATIONGATE               20
#define TCPRUNDOWNCURRENT                21
#define TCPWIDTHSELECTION                22
#define TCPPAIRDEBUG                     23
#define TCPCHECKSYSTEMNOISE              24
#define TCPMAPPINGMDTCHAMBER             25
#define TCPTRIGGERSELECTION              26
#define TCPTRIGGERRATESELECTION          27
#define TCPEXPECTEDTRIGGERRATE           28
#define TCPMAXALLOWEDMEZZS               29
#define TCPANALYSISCONTROL               30
#define TCPSTARTMEZZCARD                 31
#define TCPSPARE07                       32
#define TCPSPARE06                       33
#define TCPSPARE05                       34
#define TCPSPARE04                       35
#define TCPSPARE03                       36
#define TCPSPARE02                       37
#define TCPSPARE01                       38
#define TCPFILENAME                      39

#define CLIENTRUNSTATE                    0
#define CLIENTANAINSTALLED                1
#define CLIENTCERNLIBINSTALLED            2
#define CLIENTPROCESSINGDATA              3

#define LEADINGEDGEENABLE                 0
#define TRAILINGEDGEENABLE                1
#define PAIRENABLE                        2
   
// Define Data Type, use bit 16 to 20 only
#define DATATYPESTATUSONLY       0x00000000
#define DATATYPERAWDATA          0x00010000
#define DATATYPERAWDATACONT      0x00020000
#define DATATYPEEVENT            0x00030000
#define DATATYPEEVENTCONT        0x00040000
#define DATATYPEDAQINFOR         0x00050000
#define DATATYPEALLDAQINFOR      0x00060000
#define DATATYPEEDGES            0x00070000
#define DATATYPEPAIR             0x00080000
#define DATATYPEHITS             0x00090000
#define DATATYPEAVERAGETDCTIME   0x000A0000
#define DATATYPEFRACTIONUSED     0x000B0000
#define DATATYPEAVERAGEWIDTH     0x000C0000
#define DATATYPEDETAILSTATUS     0x000F0000
#define DATATYPEBITMASK          0x000F0000

#define BUFFER_MAX                    0x400   // maximum size of input buffer
#define MAXNUMBERMEZZANINE               18
#define MAXEVTRECORD                 0x1000
#define MAXEVTRECORDM2       MAXEVTRECORD-2

// Define AMT and CSM data type
#define TDC_HEADER                      0xa  // ID code of TDC header
#define TDC_TRAILER                     0xc  // ID code of TDC trailer
#define TDC_MASK                        0x2  // ID code of TDC masked hit
#define TDC_EDGE                        0x3  // ID code of TDC edge timing  
#define TDC_PAIR                        0x4  // ID code of TDC paired timing
#define TDC_ERROR                       0x6  // ID code of TDC error
#define TDC_DEBUG                       0x7  // ID code of TDC debug
#define CSM_WORD                        0x5  // ID code of CSM words
#define CSM_HEAD                        0x9  // sub ID code of CSM header
#define CSM_GOOD                        0xb  // sub ID code of CSM good event trailer
#define	CSM_ABORT                       0xd  // sub ID code of CSM aborted event trailer
#define	CSM_PARITY                      0x1  // sub ID code of CSM parity error code
#define	CSM_ERROR                       0x0  // sub ID code of CSM error code for abort
#define HPTDC_GHEADER                   0x0  // ID code of HPTDC group header
#define HPTDC_GTRAILER                  0x1  // ID code of HPTDC group trailer
#define HPTDC_HEADER                    0x2  // ID code of HPTDC header
#define HPTDC_TRAILER                   0x3  // ID code of HPTDC trailer
#define HPTDC_LEDGE                     0x4  // ID code of HPTDC leading edge timing
#define HPTDC_PAIR                      0x4  // ID code of HPTDC paired timing
#define HPTDC_TEDGE                     0x5  // ID code of HPTDC trailng edge timing
#define HPTDC_ERROR                     0x6  // ID code of HPTDC error
#define HPTDC_DEBUG                     0x7  // ID code of HPTDC debug

#define WORDCOUNTBIT0LOCATION             0
#define WORDCOUNTBITS                 0xFFF
#define BCIDBIT0LOCATION                  0
#define BCIDBITS                      0xFFF
#define EVIDBIT0LOCATION                 12
#define EVIDBITS                      0xFFF
#define MAINIDBIT0LOCATION               28
#define MAINIDBITS                      0xF
#define SUBIDBIT0LOCATION                24
#define SUBIDBITS                       0xF
#define TDCNUMBERBIT0LOCATION            24
#define TDCNUMBERBITS                   0xF
#define TDCNUMBERMASK            0x0F000000
#define CHANNELNUMBERBIT0LOCATION        19
#define CHANNELNUMBERBITS              0x1F
#define CHANNELNUMBERMASK        0x00F80000
#define WIRENUMBERBIT0LOCATION           19
#define WIRENUMBERBITS                0x1FF
#define WIRENUMBERMASK           0x0FF80000
#define FINETIMEBIT0LOCATION              0
#define FINETIMEBITS                   0x1F
#define MASKFLAGSBIT0LOCATION             0
#define MASKFLAGSBITS              0xFFFFFF
#define STYPEBIT0LOCATION                18
#define STYPEBITS                       0x1
#define SERRORBIT0LOCATION               17
#define SERRORBITS                      0x1
#define STDCTIMEBIT0LOCATION              0
#define STDCTIMEBITS                0x1FFFF
#define SHPTDCTIMEBITS              0x7FFFF
#define SCOARSETIMEBIT0LOCATION           5
#define SCOARSETIMEBITS               0xFFF
#define PTDCTIMEBIT0LOCATION              0
#define PTDCTIMEBITS                  0x7FF
#define PHPTDCTIMEBIT0LOCATION            0
#define PHPTDCTIMEBITS                0xFFF
#define PCOARSETIMEBIT0LOCATION           5
#define PCOARSETIMEBITS                0x3F
#define PWIDTHBIT0LOCATION               11
#define PWIDTHBITS                     0xFF
#define PWIDTHHPTDCBIT0LOCATION          12
#define PWIDTHHPTDCBITS                0x7F
#define CSMPARITYBIT0LOCATION            27
#define CSMPARITYBITS            0x08000000
#define ODDPARITYBIT0LOCATION            26
#define ODDPARITYBITS            0x04000000
#define CSMFIFOOV2BIT0LOCATION           25
#define CSMFIFOOV2BITS           0x02000000
#define CSMFIFOOV1BIT0LOCATION           24
#define CSMFIFOOV1BITS           0x01000000
#define NTRIGBIT0LOCATION                24
#define NTRIGBITS                       0xF
#define NEVENTBIT0LOCATION               28
#define NEVENTBITS                      0x7
#define WCONLY                          0x1
#define WCONLYBIT0LOCATION               31
#define WCONLYBITS                      0x1

// Define parameter for raw data
#define ID_SYNCHWORD                    0xD
#define ID_IDLEWORD                     0x0
#define SWORDHEADERMASK          0xF0000000
#define SWORDHEADER              0xD0000000
#define CSMIDLEWORD              0x04000000
#define CSMPARITYERRORBIT        0x08000000
#define CSMODDPARITYBIT          0x04000000
#define CSMDATABIT0LOCATION               0
#define CSMDATABITS              0x03FFFFFF

// Constant for Analysis
#define NUMBERANALYSISCOUNTER           800
#define ANALYSEDEVENT                     0
#define ANALYSEDEVENTHIGH                 1
#define WRONGEVENT                        2
#define WARNINGEVENT                      3
#define DISCARDEDEVENT                    4
#define NUMBERHUGEEVENT                   5
#define WORDCOUNTERERROR                  6
#define CSMWORDCOUNTERERR                 7
#define EVIDMISMATCH                      8
#define BCIDMISMATCH                      9
#define EMPTYEVENT                       10
#define EXTRAWORD                        11
#define EXTRAWORDINTDC                   12   // 18 TDCs, 0 for EvtBuilt
#define CSMHEADEREVIDMISMATCH            30
#define CSMTRAILEREVIDMISMATCH           31
#define CSMABORTEVIDMISMATCH             32
#define CSMBCIDMISMATCH                  33
#define CSMERROR                         34
#define CSMERRORINTDC                    35   // 18 TDCs
#define CSMABORT                         53
#define MISSCSMHEADER                    54
#define EXTRACSMHEADER                   55
#define MISSCSMTRAILER                   56
#define EXTRACSMTRAILER                  57
#define CSMPARITYERROR                   58
#define PARITYERRORINTDC                 59   // 18 TDCs
#define TDCBCIDMISMATCH                  77
#define TDCBCIDMISMATCHINTDC             78   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCHEADEREVIDMISMATCH            96
#define TDCHEADEREVIDMISMATCHINTDC       97   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCTRAILEREVIDMISMATCH          115
#define TDCTRAILEREVIDMISMATCHINTDC     116   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCEDGEERROR                    134
#define TDCEDGEERRORINTDC               135   // 18 TDCs
#define TDCEDGEERRW20                   153
#define TDCEDGEERRW20INTDC              154   // 18 TDCs
#define MISSTDCHEADER                   172
#define EXTRATDCHEADER                  173
#define MISSTDCTRAILER                  174
#define EXTRATDCTRAILER                 175
#define TDCSOFTERROR                    176
#define TDCHARDERROR                    177
#define TDCERROR                        178
#define TDCERRORLIST                    179   // 14 different errors in TDC error word
#define TDCERRORINTDC                   193   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCERROREMPTY                   211
#define HPTDCFATALCHIPERROR             212   // No Mask word from HPTDC, using it for internal fatal chip error
#define HPTDCFATALCHIPERRORINTDC        213   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCMASKWORD                     212
#define TDCMASKWORDINTDC                213   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCMASKEMPTY                    231
#define TDCDEBUGWORD                    232
#define TDCDEBUGWORDINTDC               233   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCHEADERWRONGTDCID             251
#define TDCHEADERMISSTDCID              252   // 18 TDCs, 16 TDC IDs in TDC header (4 bits) for EvtBuilt
#define TDCHEADEREXTRATDCID             270   // 18 TDCs, 16 TDC IDs in TDC header (4 bits) for EvtBuilt
#define TDCTRAILERWRONGTDCID            288
#define TDCTRAILERMISSTDCID             289   // 18 TDCs, 16 TDC IDs in TDC trailer (4 bits) for EvtBuilt
#define TDCTRAILEREXTRATDCID            307   // 18 TDCs, 16 TDC IDs in TDC trailer (4 bits) for EvtBuilt
#define TDCHEADERAFTERITSDATA           325
#define TDCHEADERAFTERITSDATALIST       326   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCTRAILERAHEADDATA             344
#define TDCTRAILERAHEADDATALIST         345   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCWCMISMATCH                   363
#define TDCWCMISMATCHLIST               364   // 18 TDCs, 16 TDCs for EvtBuilt due to 4 bit limit on TDC ID
#define TDCCONTSAMEEDGE                 382
#define TDCCONTSAMEEDGEINTDC            383   // 18 TDCs
#define TDCCONTTRAILINGEDGE             401
#define TDCCONTTRAILINGEDGEINTDC        402   // 18 TDCs
#define TDCUNEXPECTEDDATA               420
#define TDCUNEXPECTEDDATAINTDC          421   // 18 TDCs
#define TDCTIMEWRONGORDER               439
#define TDCTIMEWRONGORDERINTDC          440   // 18 TDCs
#define EVENTNOTDCHITS                  458
#define EVENTNOTDCHITSLIST              459   // 18 TDCs
#define EVENTONLYTDCHDANDTR             477
#define EVENTONLYTDCHDANDTRLIST         478   // 18 TDCs
#define NBCIDERRDUETRIGFIFOOV           496
#define DISCARDEDTRAILINGONLY           497
#define DISCARDEDTRAILINGONLYINTDC      498   // 18 TDCs
#define DISCARDEDLEADINGONLY            516
#define DISCARDEDLEADINGONLYINTDC       517   // 18 TDCs
#define DISCARDEDTDCWORDS               535
#define DISCARDEDTDCWORDSINTDC          536   // 18 TDCs
#define EXPECTEDTDCWCERROR              554
#define EXPECTEDTDCWCERRORINTDC         555   // 18 TDCs
#define UNEXPECTEDTDCWCERROR            573
#define UNEXPECTEDTDCWCERRORINTDC       574   // 18 TDCs
#define WRONGCSMFIFOOVERFLOW            592
#define WRONGCSMFIFOOVERFLOWINTDC       593   // 18 TDCs
#define CSMFIFOOVERFLOW1                611
#define CSMFIFOOVERFLOW1INTDC           612   // 18 TDCs
#define CSMFIFOOVERFLOW2                630
#define CSMFIFOOVERFLOW2INTDC           631   // 18 TDCs
#define CSMFIFOOVERFLOWS                649
#define CSMFIFOOVERFLOWSINTDC           650   // 18 TDCs
#define CSMEVTPRESCALEERR               668
#define CSMEVTPRESCALEERRINTDC          669   // 18 TDCs
#define CSMPAIRERROR                    687
#define CSMPAIRERRORINTDC               688   // 18 TDCs
#define NREQUESTEDSTOP                  706
#define NREQUESTEDPAUSE                 707
#define NTOTALWORD                      708
#define NTOTALWORDHIGH                  709
#define PROCESSEDEVENT                  710
#define PROCESSEDEVENTHIGH              711
#define NDATAWORD                       712
#define NDATAWORDHIGH                   713
#define NEVENTWORD                      714
#define NEVENTWORDHIGH                  715
#define NGOODCYCLE                      716
#define NGOODCYCLEHIGH                  717
#define NEMPTYCYCLE                     718
#define NEMPTYCYCLEHIGH                 719
#define NBADCYCLE                       720
#define NBADCYCLEATBEGINNING            721
#define CURREVENTSIZE                   722
#define MINEVENTSIZE                    723
#define MAXEVENTSIZE                    724
#define CURREVENTID                     725
#define NBUFFERPOINTERERROR             726
#define TXPARITYERROR                   727
#define TXPARITYERRORINTDC              728   // 18 TDC slots
#define LHCCLOCKUNLOCKED                746
#define XMT1CLOCKUNLOCKED               747
#define XMT2CLOCKUNLOCKED               748
#define CSMPHASEERROR                   749
#define I2COPERATIONFAILED              750
#define UNEXPECTEDTTCRXSETUP            751
#define CSMHASERROR                     752
#define NUMBERTRIGGER                   753
#define NUMBERSAMETRIGGER               754
#define NUMBERTRIGGERLOW                755
#define NUMBERUNMATCHEDTRIGGERLOW       756
#define NUMBERTRIGGERHIGH               757
#define NUMBERUNMATCHEDTRIGGERHIGH      758
#define UNKNOWNSWORD                    759

// Define synch word bits
//
#define STATUSTRIGTIMEINSYNCHWORD         1
#define SWORDBIT27               0x08000000
#define SWORDODDPARITY           0x04000000
#define SWORDBIT25               0x02000000
#define SWORDLHCLOCKBIT          0x01000000
#define SWORDXMTLOCK1BIT         0x00800000
#define SWORDXMTLOCK2BIT         0x00400000
#define SWORDPHASEERRORBIT       0x00200000
#define SWORDI2COPERATIONBIT     0x00100000
#define SWORDTTCRXI2CCHECKERROR  0x00080000
#define SWORDCSMERRORBIT         0x00040000
#define SWORDCSMSTATUSMASK       0x01FC0000
#define SWORDCSMSTATUSBIT0LOC            18
#define SWORDCSMSTATUSBITS             0x7F
#define SWORDCONTROLMASK         0x00030000
#define SWORDCONTROLBIT0LOC              16
#define SWORDCONTROLBITS                0x3
#define SWORDSTATUSONLY          0x00000000
#define SWORDTRIGGERTIMELOW      0x00010000
#define SWORDTRIGGERTIMEHIGH     0x00020000
#define SWORDTRIGGERTIMEMASK     0x0000FFFF
#define SWORDTRIGGERTIMEMASK     0x0000FFFF
#define SWORDTRIGGERTIMEBIT0LOC           0
#define SWORDTRIGGERTIMEBITS         0xFFFF


// Define MDT Chamber HV status
//
#define MDTCHAMBERHVOFF                   0
#define MDTCHAMBERHVON                    1
#define MDTCHAMBERHVUNKNOWN               2
#define MDTCHAMBERHVNOTAPPLY              3





//Global Variables
int validBCIDPresetValue, reqEVIDMatch, AMTType;



unsigned int DAQState, runNumber, CSMVersion, GOLAPageSize, mezzEnables, numberOfEvent, dataAnalysisControl;
unsigned int totalDAQTime;
unsigned int buffer[4*BUFFER_MAX], prevBuffer[4*BUFFER_MAX];         // incoming data with 8bit valid data
int pipe1[2], pipe2[2], clientID, CSMNumber, sock, osock, normalQuit, nbRequestedBuf;
pid_t ppid, cpid;
double maxUintValue = 4294967295.0;
//
unsigned int synchWordControl;
int MYTDC_GHEADER, MYTDC_GTRAILER, MYTDC_HEADER, MYTDC_TRAILER, MYTDC_LEDGE, MYTDC_PAIR, MYTDC_TEDGE;
int MYTDC_ERROR,  MYTDC_DEBUG, MYTDC_MASK, MYTDC_EDGE, HPTDC, errWordMask, TDCDivider, widthDivider; 
char CSMDataFilename[256], rawDataFilename[256], dataTopDirName[10], dataDirName[60];
char CSMDataFilename_2[256], rawDataFilename_2[256];
FILE *CSMDataFile_1, *rawDataFile_1, *logFile_1;
FILE *CSMDataFile_2, *rawDataFile_2, *logFile_2;

// Variables for detail status
int anaInstalled, CERNLibInstalled, processingData;
char anaPath[512], CERNLibPath[512], CMDString[2048];

// Variables and routines for Run Condition
int CSMPairDebug, checkSystemNoise, mappingMDTChamber, startMezzCard;
int suppressCSMHeader, suppressCSMTrailer, suppressTDCHeader, suppressTDCTrailer;
int MDTChamberType, MDTChamberHVStatus, saveFlaggedEvent, saveRawData;
int nominalThresholdUsed, nominalThreshold, integrationGate, rundownCurrent, widthSelection;
int minWidth, selectedTrigger, selectedTriggerRate, expectedTriggerRate;

// For single event capture
oneEventInfo_T singleEvInfo;
int singleEvOffset;
int singleEvBufnr;
int singleEvMaxBuffers;
int singleEvInitWasDone;

int enable_CSM2;




class CollectCSMData
{
public:
  unsigned int nEdge[2][24][MAXNUMBERMEZZANINE], nPair[24][MAXNUMBERMEZZANINE], nEvents[MAXNUMBERMEZZANINE];
  unsigned int nGoodHit[24][MAXNUMBERMEZZANINE], nASDNoise[24][MAXNUMBERMEZZANINE];
  unsigned int timeSpectrum[4000][24][MAXNUMBERMEZZANINE], timeWidth[512][24][MAXNUMBERMEZZANINE];
  unsigned int analysisCounter[NUMBERANALYSISCOUNTER], singleCounter[NUMBERANALYSISCOUNTER];

  unsigned int numberFilledFIFOs;
  unsigned int requestForStop, requestForPause;
  char DAQStartedAt[30];
  double timeAverage[24][MAXNUMBERMEZZANINE], timeSigma[24][MAXNUMBERMEZZANINE], fracUsed[24][MAXNUMBERMEZZANINE];
  double widthAverage[24][MAXNUMBERMEZZANINE], widthSigma[24][MAXNUMBERMEZZANINE], fracUsedInWidth[24][MAXNUMBERMEZZANINE];
  int filar_chnl_no;
  int mezzCardEnable[MAXNUMBERMEZZANINE];


  CollectCSMData(int filar_chnl, int openDataFile);
  void EndOfCollecting();
  void DataAssembling();
  void DataAssembling_triggerless();
  void SaveErrorSummaryFile();
  void PrintAnalysisCounter(FILE *file, int cntIndex, char name[], int maxLength);
  void CloseAllFiles();
  void SaveTDCTimeSpectrum();
  void CalculateAverageTDCTimeAndSigma();
  void CalculateAverageWidthAndSigma();
  void RawDataInterpretation(unsigned int data, FILE *file);
  void DataInterpretation(unsigned int data, FILE *file);
  
private:
  int bufnr;
  int datasize, i, j, got, synchWord, copyData, nloop, width;
  int wc, bufptr, gotBadTDCIDAtHD[64], gotBadTDCIDAtTR[64], edge, edgeErrBit, tdcTime,
      previousTDCTime[24][MAXNUMBERMEZZANINE], previousEdgeErr[24][MAXNUMBERMEZZANINE],
      previousEdge[24][MAXNUMBERMEZZANINE], evidOld, ndata, collectHits, collectedEVT;
  int checkCSMOverflows, checkExpectedData;
  int receivedData, nReceivedData[MAXNUMBERMEZZANINE], expected, nUnexpected[MAXNUMBERMEZZANINE];
  unsigned int *dataptr, gotTriggerLow, overflow1, overflow2, diff, badCycleAtBeginning;
  unsigned int triggerNumber, triggerTime, previousTriggerTime, control, dataLow, trigTHist[1000000];
  unsigned int uintSize, data, channel, cycleOK, wordID, tdcID, subID, evid, checksum;
  unsigned int bufEVID[64], bcid[MAXNUMBERMEZZANINE], bufBCID[64], trigOverflow[64];
  unsigned int evtCSMFIFOOV[MAXNUMBERMEZZANINE][64], beginEvent[64], printEvent[64];
  unsigned int mezz, tdc, chan, reconEVID, nwords, gotHeader[64], gotTrailer[64];
  unsigned int nTDCHeader[MAXNUMBERMEZZANINE], nTDCTrailer[MAXNUMBERMEZZANINE], 
               nTDCEdge[MAXNUMBERMEZZANINE], nTDCPair[MAXNUMBERMEZZANINE], 
               nTDCMask[MAXNUMBERMEZZANINE], nTDCError[MAXNUMBERMEZZANINE], 
               nTDCDebug[MAXNUMBERMEZZANINE], nTDCOthers[MAXNUMBERMEZZANINE];
  unsigned int nTrailer, nHeader, previousData[MAXNUMBERMEZZANINE], countData, notSameTrailer;
  unsigned int evtError[64], evtWarning[64], nhits, nTDCHits[MAXNUMBERMEZZANINE][64], sum;
  unsigned int nothers, nPackedEvt, nDataWords, evtNumber, fileSize, rawFileSize, nCommParityError;
  unsigned int DAQStartDate, DAQStartTime, startTime0, currentDAQDate, currentDAQTime;
  int sockWriteCount, bytes, totalBytes;
  time_t localTime;
  struct tm *localDateTime;
  char myDateTime[30], eventFilename[281];
  unsigned int evtRDBuf;
  unsigned int eventBuf[MAXEVTRECORD][64], bufEmpty[64];
  unsigned int evtWRIndex[64], evtSize[MAXNUMBERMEZZANINE][64], evtWRBuf[MAXNUMBERMEZZANINE];  
  unsigned int dataBuf[MAXEVTRECORD];
  int saveRunCondition;	
  int openFile;
  char CSMDataFilename_local[256], rawDataFilename_local[256];
  FILE *eventFile, *CSMDataFile, *rawDataFile, *logFile;
  int lEdgeOn[MAXNUMBERMEZZANINE], tEdgeOn[MAXNUMBERMEZZANINE], pairOn[MAXNUMBERMEZZANINE];
  int nbMezzCard;
  unsigned int mezzEnables;
  int AMTDataMode0, AMTDataMode1;
};

CollectCSMData* p_CollectCSMData_1=NULL;
CollectCSMData* p_CollectCSMData_2=NULL;


void CheckDataAnalysisPackageInstallations(void);
void CheckDataAnalysisProcessingState(void);

// DAQ Routines
//
int OpenGOLACard(void);
void CloseGOLACard(void);
int InitDAQ(void);
int InitToStartRun(int openDataFile);
void CollectCSMData(void);
void PrintAnalysisCounter(FILE *file, int cntIndex, char name[], int maxLength);
void SaveErrorSummaryFile(void);
void SaveTDCTimeSpectrum(void);
void SaveTriggerTimeDistribution(unsigned int *trigTHist);
void CalculateAverageTDCTimeAndSigma(void);
void CalculateAverageWidthAndSigma(void);

void SingleEventInit(void);
int GetNextSingleEvent( unsigned int *rawData, int rawDataSize, unsigned int *builtData, 
			int builtDataSize, unsigned int *dataptr, unsigned int fsize );
int CollectSeqEvent(unsigned int *rawData, int rawDataSize, unsigned int *builtData, int builtDataSize);
int CollectOneEvent(unsigned int *rawData, int rawDataSize, unsigned int *builtData, int builtDataSize);

// Signal Handler Routines
//
void ChildSigUSR1Handler(int sig);
void ChildSigUSR2Handler(int sig);
void ChildSigTERMHandler(int sig);
void ParentSigUSR1Handler(int sig);
void ParentSigUSR2Handler(int sig);
void ParentSigTERMHandler(int sig);
void DAQ_process();

#endif
