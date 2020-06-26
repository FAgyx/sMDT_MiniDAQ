#ifndef csmSetup_H
#define csmSetup_H
//
// Filename: csmSetup.h
// Author: T.S. Dai
//
// Description:
// This file contains prototypes for all of the CSM JTAG Setup functions.
#define MAXNUMBERMEZZANINE        18
#define CSMCSMSTATUSLENGTH        28
#define CSMMEZZSTATUSLENGTH         MAXNUMBERMEZZANINE
#define CSMFULLPHASELENGTH         1+CSMMEZZSTATUSLENGTH+4*MAXNUMBERMEZZANINE
#define CSMTTCRXSTATUSLENGTH     160
#define CSMIDCODELENGTH           32
#define CSMVERSIONDATELENGTH      32
#define CSMSTATUSLENGTH             CSMCSMSTATUSLENGTH+CSMMEZZSTATUSLENGTH+CSMTTCRXSTATUSLENGTH
#define CSMSETUPLENGTH            72

// Define CSM status bits
//
#define CSMVERSION                 0
#define GOLREADYBIT               12
#define TTCRXREADYBIT             13
#define LHCLOCKBIT                14
#define XMTLOCK1BIT               15
#define XMTLOCK2BIT               16
#define ORUNUSEDBIT               17
#define TTCLOADSTATUSBIT          18
#define CSMSTATE                  19
#define PHASEERRORBIT             23
#define I2COPERATIONBIT           24
#define TTCRXI2CCHECKERROR        25
#define TTCRXDUMPCHECKERROR       26
#define CSMERRORBIT               27
#define PARITYERROR               28         // 18 bits
#define TTCRXREADBACKBITS         46         // 160 bits
#define TTCRXFINEDELAY1           46
#define TTCRXFINEDELAY2           54
#define TTCRXCOARSEDELAY          62
#define TTCRXCONTROL              70
#define TTCRXSERRORCOUNT0         78
#define TTCRXSERRORCOUNT8         86
#define TTCRXDERRORCOUNT          94 
#define TTCRXSEUERRORCOUNT       102
#define TTCRXID0                 110
#define TTCRXMASTERMODEAID8      118
#define TTCRXMASTERMODEBI2CID    126
#define TTCRXCONFIG1             134
#define TTCRXCONFIG2             142
#define TTCRXCONFIG3             150
#define TTCRXSTATUSREGISTER      158
#define TTCRXBUNCHCOUNTER00      166
#define TTCRXBUNCHCOUNTER08		 174
#define TTCRXEVENTCOUNTER00      182
#define TTCRXEVENTCOUNTER08      190
#define TTCRXEVENTCOUNTER16      198
#define TTCRXREADBACKHIGH        205

// Define CSM setup bits
//
#define TDCENABLES                 0
#define MEZZJTAGENABLE            18
#define CSMDAQCONTROL             19
#define AMTCOMMANDDELAY           20
#define MEZZ80MHZOPERATION        27
#define MEZZPAIRWIDTHRESOLUTION   28
#define MEZZCSMPAIRMODE           31
#define MEZZCSMPAIRDEBUGMODE      32
#define CSMSPAREBIT00             33
#define EMPTYCYCLECONTROL         34
#define SYNCHWORDCONTROL          35
#define GOLDIFF                   36
#define GOLLD0                    37
#define GOLLD1                    38
#define GOLPLL                    39
#define GOLLASER                  40
#define GOLNEG                    41
#define GOLMODE                   42
#define ENABLEGOLTDI              43
#define MAXNUMBERMEZZ             44
#define CSMSPAREBIT01             49
#define ENABLETTCRXTDI            50
#define TTCRXUSEPROM              51
#define CSMNEXTSTATE              52
#define CSMCMD                    56
#define DISABLEBCIDMATCH          57
#define DISABLEEVIDMATCH          58
#define PASSALLAMTDATA            59
#define DROPAMTHDANDTR            60
#define MEZZPAIRPRESCALE          61
#define MEZZBUFFTHRESHOLD1        64
#define MEZZBUFFTHRESHOLD2        68

// Define CSM commands
//
#define CMDIDLE                    0
#define CMDRESETFPGA               1
#define CMDRESETTTCRX              2
#define CMDRESETGOL                3
#define CMDWAITCLKLOCK             4
#define CMDRESETANDWAITCLK25       5
#define CMDSAMPLEAMTPHASE          6
#define CMDJTAGRESET               7
#define CMDTTCRXLOAD               8
#define CMDWAITPROM                9
#define CMDRESETAMT               10
#define CMDRESETERROR             11
#define CMD12UNKNOWN              12
#define CMD13UNKNOWN              13
#define CMD14UNKNOWN              14
#define CMD15UNKNOWN              15

// Define TTCrx Status Register bits
//
#define TTCRXSTSALWAYS0BITS     0x0F
#define TTCRXSTSAUTORESETFLAG      4
#define TTCRXSTSFRAMESYNCH         5
#define TTCRXSTSDLLREADY           6
#define TTCRXSTSPLLREADY           7

int CSMSetupHandle, TTCrxInfoHandle;
int CSMSetupArray[CSMSETUPLENGTH], CSMStatusArray[CSMSTATUSLENGTH];
int downloadCSMSetupDone, downloadedCSMSetupArray[CSMSETUPLENGTH];
int CSMVersion, CSMNextState, validCSMStatus, readbackTTCrxRegisterDone;
int CSMPairWidthResolution, CSMPairMode, CSMPairDebug, CSMMaxAllowedMezzs;
int CSMVersionDate, AMTParityError, AMTPhaseError, CSMLastNextState;
int AMTPhaseSamples[MAXNUMBERMEZZANINE], AMTFullPhaseError, AMTFullPhaseErrorBit;
char CSMCMDText[16][80];

//
// Function prototypes
//
void UpCSMSetupControl(void);
void UpTTCrxInfoControl(void);
int CSMSetupPanelAndButton(void);
void TTCrxInfoClose(void);
void CSMSetupDone(void);
void SaveCSMSetup(void);
void SetAllMezzanineCard(void);
void ClearAllMezzanineCard(void);
void CSMDAQControl(void);
void CreatGOLLDList(void);
void TTCrxPROMUsage(void);
void PutCSMNextState(void);
void TurnOffUnwantedMezzCards(void);
void SetCSMModeAccordingToLinkRate(void);
void ResetCSM(void);
void ResetCSMError(void);
void ResetGOL(void);
void ResetTTCrx(void);
void ResetAndDownloadTTCrxMethod1(void);
void ResetAndDownloadTTCrxMethod2(void);
void ResetMezzanineCards(void);
void StartCSMDAQ(void);
void StopCSMDAQ(void);
void StopCSMDAQAndTurnOffMezzJTAGSignals(void);
void CSMSampleAMTPhase(void);
int GetAMTPhaseLockStatus(void);
void UpdateCSMControlBits(void);
void UpdateCSMSetupPanel(void);
void LoadCSMSetupArray(void);

#endif
