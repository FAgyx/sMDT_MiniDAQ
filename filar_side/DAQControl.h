#ifndef DAQControl_H
#define DAQControl_H
//
// Filename: DAQControl.h
// Author: T.S. Dai
//
// Description:
// This file contains prototypes for all of the DAQ Control functions.
//

#define TYPEAMT1                          1
#define TYPEAMT2                          2
#define TYPEAMT3                          3

#define State_Idle                        0
#define State_Starting                    1
#define State_Running                     2
#define State_Paused                      3

#define CMDCHAMBERID                      1
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
#define CMDREQLEADINGEDGE                30
#define CMDREQTRAILINGEDGE               31
#define CMDREQPAIR                       32
#define CMDREQDAQINFOR                   50
#define CMDREREQDAQINFOR                 51
#define CMDREQALLDAQINFOR                52
#define CMDQUIT                         255

#define TCPLENGTH                         0
#define TCPCOMMAND                        1
#define TCPNUMBERTCPNODES                 2
#define TCPCSMVERSION                     3
#define TCPMEZZENABLES                    4
#define TCPEDGEPAIRENABLES0               5
#define TCPEDGEPAIRENABLES1               6
#define TCPGOLACARDPAGESIZE               7
#define TCPAMTTYPE                        8
#define TCPRUNNUMBER                      9
#define TCPNUMBEROFEVENT                 10
#define TCPFILENAME                      11

#define LEADINGEDGEENABLE                 0
#define TRAILINGEDGEENABLE                1
#define PAIRENABLE                        2

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
#define SCOARSETIMEBIT0LOCATION           5
#define SCOARSETIMEBITS               0xFFF
#define PTDCTIMEBIT0LOCATION              0
#define PTDCTIMEBITS                  0x7FF
#define PCOARSETIMEBIT0LOCATION           5
#define PCOARSETIMEBITS                0x3F
#define PWIDTHBIT0LOCATION               11
#define PWIDTHBITS                     0xFF
#define NTRIGBIT0LOCATION                24
#define NTRIGBITS                       0xF
#define NEVENTBIT0LOCATION               28
#define NEVENTBITS                      0x7
#define WCONLY                          0x1
#define WCONLYBIT0LOCATION               31
#define WCONLYBITS                      0x1

// Constant for Analysis
#define NUMBERANALYSISCOUNTER           600
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
#define TDCUNEXPECTEDDATA               401
#define TDCUNEXPECTEDDATAINTDC          402   // 18 TDCs
#define TDCTIMEWRONGORDER               420
#define TDCTIMEWRONGORDERINTDC          421   // 18 TDCs
#define EVENTNOTDCHITS                  439
#define EVENTNOTDCHITSLIST              440   // 18 TDCs
#define EVENTONLYTDCHDANDTR             458
#define EVENTONLYTDCHDANDTRLIST         459   // 18 TDCs
#define NBCIDERRDUETRIGFIFOOV           477
#define NREQUESTEDPAUSE                 478
#define NTOTALWORD                      479
#define NTOTALWORDHIGH                  480
#define PROCESSEDEVENT                  481
#define PROCESSEDEVENTHIGH              482
#define NDATAWORD                       483
#define NDATAWORDHIGH                   484
#define NEVENTWORD                      485
#define NEVENTWORDHIGH                  486
#define NGOODCYCLE                      487
#define NGOODCYCLEHIGH                  488
#define NEMPTYCYCLE                     489
#define NEMPTYCYCLEHIGH                 490
#define NBADCYCLE                       491
#define CURREVENTSIZE                   492
#define MINEVENTSIZE                    493
#define MAXEVENTSIZE                    494
#define CURREVENTID                     495
#define NBUFFERPOINTERERROR             496
#define TXPARITYERROR                   497
#define TXPARITYERRORINTDC              498   // 18 TDC slots
#define CONTINUEEVENTWITHERROR          516   // 10 different counter 

int DAQControlHandle, DAQInforHandle, ErrorReportHandle, SerialNumberHandle;
int runState, newRunNumber, runStartedWithTCPClient, validBCIDPresetValue;
int eventReady, getOneEventInitDone, getOneSequentialEventInitDone, initDAQDone;
// Analysis controls and variables
int enableAnalysis, firstCallAnalysis, showErrorReport, eventID, dataType, toTCPClients;
unsigned int analysisCounter[NUMBERANALYSISCOUNTER], totalNumberEvent, totalNumberEventHigh;
unsigned int nEdge[2][24][MAXNUMBERMEZZANINE], nPair[24][MAXNUMBERMEZZANINE], nEvents[MAXNUMBERMEZZANINE];

//
// Function prototypes
//
void DAQControlStartUp(void);
int DAQControlButton(void);
void Quit(void);
void StartOrStopDAQ(void);
void StartDAQ(void);
void StopDAQ(void);
void PauseOrResumeDAQ(void);
void PauseDAQ(void);
void ResumeDAQ(void);
void InitDAQ(void);
void SetDAQRunType(void);
void SetRunNumberButton(void);
void SetRunNumber(void);
void DisconnectAllTCPClients(void);
void GetRawData(void);
void GetOneEvent(void);
void GetOneSequentialEvent(void);
void ObtainGOLANumber(int GOLANumber);
int SendTCPCommand(int GOLANumber, int command);
int WaitForTCPClientReply(void);
void UpdateTCPStatus(int newConnection);
void HandleVariesData(unsigned int data[]);
void PrintOutData(unsigned int *data, FILE *file);
void DataInterpretation(unsigned int data, FILE *file);

// Serial Number Panel
void UpSerialNumber(void);
void SerialNumberDone(void);

// DAQ error report and data monitor routine prototype
//
void UpErrorReport(void);
void ErrorReportClose(void);
void SetErrorReportCounter(int panelIndex, int counterIndex);
void SaveErrorSummaryFile(void);
void ErrorSummaryPrint(FILE *sumFile, int i, char name[], int maxLength);
void SaveRunLogfile(void);
void SaveResultFile(void);
void SaveScanResults(void);

// DAQ Infor Routine Prototype
//
void DAQControlInforColor(void);
void CloseDAQInfor(void);
void DAQInforControl(void);
void CollectAndDisplayDAQInfor(void);
void CollectDAQInfor(int counter[]);
void CopyDAQInfor(int counter[]);

// Variables and routines for Auto Run
int AutoRunSetupHandle, autoRunSetupDone;
unsigned int stopRunAtEventOn, stopRunAtEvent, restartRunAtEventOn, restartRunAtEvent;
unsigned int stopAfterNRunOn, stopAfterNRun, restartRunAfterSecOn, restartRunAfterSec;
unsigned int restartRunAtHangTimeOn, restartRunAtHangTime, autoSaveRegisterOn;
unsigned int resetDAQAtEventErrorOn, resetDAQAtEventError;
unsigned int stopAfterNError, stopAfterNErrorOn;
// Auto Run Routine Prototypes
//
int AutoRunSetupPanelAndButton(void);
void UpAutoRunSetup(void);
void AutoRunSetupDone(void);
void AutoRunSetupAllOn(void);
void AutoRunSetupAllOff(void);
void GetAutoRunControls(void);
void AutoRunControl(int restartRun);
void AutoRunSelectControlFile(void);
int GetNewRunSetups(int downloadSetups);

#endif
