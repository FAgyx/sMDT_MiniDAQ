// TTCvi.h
// Define TTCvi VME register constant and TTCvi routines
//
#ifndef TTCVI_H
#define TTCVI_H

#define CMDPROBE                          1
#define CMDGETTTCVIBASEADDRESS          101
#define CMDTTCVIBASEADDRESS             102
#define CMDTTCVIBASEREAD16              103
#define CMDTTCVIBASEREAD32              104
#define CMDTTCVIREAD16                  110
#define CMDTTCVIWRITE16                 111
#define CMDTTCVIREAD32                  112
#define CMDTTCVIWRITE32                 113
#define CMDQUIT                         255

#define TCPLENGTH                         0
#define TCPCOMMAND                        1
#define TCPCSMNUMBER                      2
#define TCPTTCVIADDR                      3
#define TCPTTCVIDATA                      4

#define TCPTTCVICSMNUMBER              4096

unsigned int TTCviBase = 0xFF1000;
int gotTTCviTCPData, TTCviTCPVMEStatus, TTCviTCPData;

int SendTTCviTCPCommand(int command, int addr, int data);
void HandleTTCviData(unsigned int data[]);

// Low level routines for access TTCvi registers 
int ReadTTCviReg(int address, int *data);
int WriteTTCviReg(int address, int data);  
int ReadTTCviReg4(int address, int *data);
int WriteTTCviReg4(int address, int data);

// Define TTCvi VME register address
#define EEPROMLOW                0x00
#define EEPROMHIGH               0x4E
#define MANUFACTURERID           0x26
#define BOARDIDANDSN             0x32
#define BOARDREVISIONNO          0x42
#define CSR1                     0x80
#define CSR2                     0x82
#define SOFTWARERESET            0x84
#define SOFTWARETRIGGER          0x86
#define EVENTCOUNTMSW            0x88
#define EVENTCOUNTLSW            0x8A
#define RESETEVTORBITCOUNT       0x8C    // MarkII only
#define RESERVED02				 0x8E
#define BGO0MODE                 0x90
#define INHIBIT0DELAY            0x92
#define INHIBIT0DURATION         0x94
#define BGO0SWGO                 0x96
#define BGO1MODE                 0x98
#define INHIBIT1DELAY            0x9A
#define INHIBIT1DURATION         0x9C
#define BGO1SWGO                 0x9E
#define BGO2MODE                 0xA0
#define INHIBIT2DELAY            0xA2
#define INHIBIT2DURATION         0xA4
#define BGO2SWGO                 0xA6
#define BGO3MODE                 0xA8
#define INHIBIT3DELAY            0xAA
#define INHIBIT3DURATION         0xAC
#define BGO3SWGO                 0xAE
#define BGO0PARAMETER            0xB0
#define BGO0PARAMETERMSW         0xB0
#define BGO0PARAMETERLSW         0xB2
#define BGO1PARAMETER            0xB4
#define BGO1PARAMETERMSW         0xB4
#define BGO1PARAMETERLSW         0xB6
#define BGO2PARAMETER            0xB8
#define BGO2PARAMETERMSW         0xB8
#define BGO2PARAMETERLSW         0xBA
#define BGO3PARAMETER            0xBC
#define BGO3PARAMETERMSW         0xBC
#define BGO3PARAMETERLSW         0xBE
#define LONGVMECYCLE             0xC0
#define LONGVMECYCLEMSW          0xC0
#define LONGVMECYCLELSW          0xC2
#define SHORTVMECYCLE            0xC4
#define TRIGGERWORDMSW           0xC8    // MarkII only
#define TRIGGERWORDLSW           0xCA    // MarkII only

// Define the L1A trigger selection possibilities
#define L1AFROMCHANNEL0             0
#define L1AFROMCHANNEL1             1
#define L1AFROMCHANNEL2             2
#define L1AFROMCHANNEL3             3
#define L1AFROMVME                  4
#define L1ARANDOM                   5
#define L1ACALIBRATION              6      // MARKII only
#define L1ANOSELECTION6             6
#define L1ANOSELECTION7             7

// Define the L1A random trigger rate selection possibilities
#define L1ARANDOM1HZ                0
#define L1ARANDOM100HZ              1
#define L1ARANDOM1KHZ               2
#define L1ARANDOM5KHZ               3
#define L1ARANDOM10KHZ              4
#define L1ARANDOM25KHZ              5
#define L1ARANDOM50KHZ              6
#define L1ARANDOM100KHZ             7

// Define CSR1 bits
#define L1ATRIGGERSELECT            0
#define ORBITSIGNALSELECT           3
#define L1AFIFOFULL                 4
#define L1AFIFOEMPTY                5
#define L1AFIFORESET                6
#define VMETRANSFERPENDING          7
#define BCDELAY                     8
#define RANDOMTRIGGERRATE          12
#define SELECTCOUNT                15      // MARKII only
#define L1ATRIGGERSELECTBITS   0x0007
#define BCDELAYBITS            0x0F00
#define RANDOMTRIGGERRATEBITS  0x7000

// Define CSR2 bits
#define BGOFIFO0EMPTY               0
#define BGOFIFO0FULL                1
#define BGOFIFO1EMPTY               2
#define BGOFIFO1FULL                3
#define BGOFIFO2EMPTY               4
#define BGOFIFO2FULL                5
#define BGOFIFO3EMPTY               6
#define BGOFIFO3FULL                7
#define RETRANSMITBGOFIFO0          8
#define RETRANSMITBGOFIFO1          9
#define RETRANSMITBGOFIFO2         10
#define RETRANSMITBGOFIFO3         11
#define RESETBGOFIFO0              12
#define RESETBGOFIFO1              13
#define RESETBGOFIFO2              14
#define RESETBGOFIFO3              15

// Define B-Go mode bits
#define BGOFRONTPANEL               0
#define BGOCYCLEMODE                1
#define BGOTRANSFERTYPE             2
#define BGOFIFOUSAGE                3
#define BGOSELECTMODE               4      // MARKII Only

// Define Trigger Word Bits (MARKII Only)
#define TRIGWORDADDRESS             0
#define TRIGWORDADDRESSBITS    0x3FFF
#define TRIGWORDSUBADDR             0
#define TRIGWORDSUBADDRBITS      0xFF
#define TRIGWORDACCESS              8
#define TRIGWORDCONTROL             9

// Define TTCvi Type
#define TTCVITYPE01                 1
#define MARKII                      2

// Define TTCvi Run Type
#define TTCVINORMAL                 0
#define TTCVICALIBRATION            1

// Define TTCvi Masks
#define TTCVIDURATIONMASK        0xFF

int TTCStatusAndControlHandle, TTCBaseMessageHandle, BGOControlHandle, TrigSelectHandle;
int TTCviType, TTCviControlsDone = 0, TTCviBGODone = 0;
int selectedTrigger = -1, selectedTriggerRate = -1, measureAllTriggerRate = TRUE;
int saveSavedTrigger, saveSelectedTrigger, saveSelectedTriggerRate;
int setTTCviDefault = TRUE, savedTrigger = -1, triggerSettingSaved = FALSE;
int nButton, trigButton[16], rateButton[16], trigSele[16], triggerSelectDone;
unsigned int BGOFIFOReadAddress[4], BGOFIFOWriteAddress[4], BGOFIFOData[256][4];

// TTCvi status and control panel
int TTCviPanelAndButton(void);
void UpTTCviStatusAndControl(void);
void TTCviDone(void);
void TTCviSetBase(void);
void TTCviBoardID(void);
void TTCviBoardReset(void);
void TTCviEventCounterReset(void);
void TTCviBunchCounterReset(void);
void TTCviEventAndBunchCounterReset(void);
void TTCviSoftwareTrigger(void);
void TTCviReadEventNumber(void);
void TTCviOrbitSignalSelection(void);
void TTCviTriggerSelection(void);
void TTCviRandomTriggerRate(void);
void TTCviResetL1AFIFO(void);
void TTCviResetBGOFIFO0(void);
void TTCviResetBGOFIFO1(void);
void TTCviResetBGOFIFO2(void);
void TTCviResetBGOFIFO3(void);
void TTCviResetBGOFIFOAll(void);
void TTCviRetransmitBGOFIFOControl(void);
void TTCviRetransmitBGOFIFOEnableAll(void);
void TTCviRetransmitBGOFIFODisableAll(void);
void TTCviRunType(void);
void TTCviCalibrationRun(void);
void TTCviNormalDAQRun(void);
void TTCviSelectCount(void);
void TTCviResetTTCviEventOrbitCounter(void);
void TTCviTriggerWord(void);

// TTCvi controls for menu
void TTCviSetTrigger1Hz(void);
void TTCviSetTrigger100Hz(void);
void TTCviSetTrigger1KHz(void);
void TTCviSetTrigger5KHz(void);
void TTCviSetTrigger10KHz(void);
void TTCviSetTrigger25KHz(void);
void TTCviSetTrigger50KHz(void);
void TTCviSetTrigger100KHz(void);
void TTCviSetExternalTriggerChannel0(void);
void TTCviSetExternalTriggerChannel1(void);
void TTCviSetExternalTriggerChannel2(void);
void TTCviSetExternalTriggerChannel3(void);
void TTCviSetUnusedTrigger(void);
void TTCviRestoreTrigger(void);
void TTCviSaveTriggerSettings(void);
void TTCviRestoreTriggerSettings(void);

// TTCvi B-Go control panel and buttons
void UpTTCviBGOControl(void);
void TTCviLoadBGOSetup(void);
void TTCviSaveBGOSetup(void);
void TTCviBGOControlDone(void);
void TTCviBGODefault(void);
void TTCviExecuteBChannelCycle(void);
void TTCviEnableAllFrontPanel(void);
void TTCviDisableAllFrontPanel(void);
void TTCviAllSynchronous(void);
void TTCviAllAsynchronous(void);
void TTCviAllSingleMode(void);
void TTCviAllRepetiveMode(void);
void TTCviAllStartIfFIFONotEmpty(void);
void TTCviAllIgnoreFIFOStatus(void);
void TTCviWriteBGOFIFO0(void);
void TTCviWriteBGOFIFO1(void);
void TTCviWriteBGOFIFO2(void);
void TTCviWriteBGOFIFO3(void);
void TTCviWriteBGOFIFOAll(void);
void TTCviGenerateBGO0Signal(void);
void TTCviGenerateBGO1Signal(void);
void TTCviGenerateBGO2Signal(void);
void TTCviGenerateBGO3Signal(void);
void TTCviGenerateAllBGOSignal(void);
void TTCviHandleBGOFIFO(int BGONumber);

// TTCvi trigger selection panel and buttons
void UpTTCviTriggerSelect(void);
void TTCviTriggerSelectDone(void);
void TTCviTriggerSelect(void);
void MeasureVariesTriggerRate(void);
void TTCviOneShotTriggerIfSelected(void);
void SaveMeasuredTriggerRate(float rate);
float ExpectedTriggerRate(void);
float GetMeasuredTriggerRate(int trigSel0, int trigRate0);
int InternalSoftwareTrigger(void);

// TTCvi routines
int SetTTCviBase(unsigned int baseAddress, int address);
int GetTTCviBase(void);
void GetTTCviBoardID(int printFlag);
void CreatTriggerSelectList(void);
void UpdateTTCviStatusAndControlPanel(void);
void UpdateTTCviBGOControlPanel(void);
void WriteTTCviBGOControlRegisters(void);
void CheckTTCviBGOSettings(void);
void TTCviViewBGOFIFOs(void);
int LoadTTCviBGOSetup(char *filename);
int SaveTTCviBGOSetup(char *filename);
int InitTTCvi(void);
void SetTTCviTriggerAndRate(int triggerSelect, int *triggerRate);
int GetTTCviTriggerRate(void);
int GetTTCviTriggerSelction(void);
int GetTTCviTriggerRateSelction(void);
float MeasuredTTCviTriggerRate(int *numberTrigger);

#endif
