#ifndef jtagControl_H
#define jtagControl_H
//
// Filename: jtagControl.h
// Author: T.S. Dai
//
// Description:
// This file contains prototypes for all of the JTAG Control functions.
//

// Define compile options
//
#define DEBUGVERIFYFPGAORPROM
#define WRITESVFFILE

int TitleHandle, JTAGControlHandle, SelectActionHandle, ConfirmVFPHandle, Menu00Handle;
int CSMIOsHandle, MezzJTAGStatusHandle;

//
// Function prototypes
//
void JTAGControlStartUp(void);
void JTAGControlInitialization(void);
void UserFunction(void);
int JTAGControlPanelAndButton(void);
void ResetTAPButton(void);
void StartOrStopCSMDAQ(void);
void StartOrStopCSMDAQButton(void);
void SampleAMTPhaseOrVerifyFPGA(void);
void SampleAMTPhase(void);
void ResetCSMOrVerifyPROM(void);
void InitCSMDAQ(void);
void SetNumberCSMButton(void);
void SetNumberCSM(void);
void SetCSMNumber(void);
void SetJTAGRateDivisorButton(void);
void SaveAndRecallJTAGControlParas(void);
void LoadProfile(void);
void SaveProfile(void);
void SaveDefaultProfile(void);
void ClearDefaultProfile(void);
void GenerateAllTextFilesForDAQ(void);
void GenerateAllActSeqFilesForDAQ(void);
void DownloadSetup(void);
void SaveSetupToSVFFile(void);
void JTAGChainPROM(void);
void JTAGChainCSMChip(void);
void JTAGChainGOLButton(void);
void JTAGChainGOL(void);
void JTAGChainTTCrxButton(void);
void JTAGChainTTCrx(void);
void JTAGChainCSM(void);
void JTAGChainMezzCardsButton(void);
void JTAGChainMezzCards(void);
void JTAGChainControl(void);
void ClearSetupStatus(void);
void HelpButton(void);
void AboutDone(void);
void HelpAbout(void);
void UpdateJTAGControlPanelInfor(void);
void GetJTAGChainOnList(void);
int CSMSetupStatus(void);
int GOLSetupStatus(void);
int TTCrxSetupStatus(void);
int MezzCardSetupStatus(int mezzCardNumber);
void GetMezzCardEnables(void);
void SetMezzCardEnables(void);
int CSMBoardStatus(void);
void UpdateJTAGControlPanel(void);

void LoadSavedProfile(int loadProfile);
int PanelIndexInFile(int panelID, int setupType, int mezzNumber, int ASDNumber);
void SaveAllPanelState(char *fileName, int saveLevel);
void RecallAllPanelState(char *fileName);

//
// Function prototypes for CSM IOs control
//
void UpCSMIOsControl(void);
int CSMIOsPanelAndButton(void);
void CSMIOsDone(void);
void CSMIOsSet(void);
void CSMIOsRead(void);
void CSMIOsProgramFPGAFromPROM(void);
void CSMIOsResetCSMBoard(void);
void CSMIOsSelectSoftwareTDO(void);
void CSMIOsSelectHardwareTDO(void);
void UpdateCSMIOsPanel(int readOnly);

// Define PROM (XC18V04, XCFxxS and XCFxxP) IDs
// Note: For PROM ID, bit 16, 28, 29, 30 and 31 are uncertain,
//       where bit 29 to 31 are version number.
//
#define PROMIDLENGTH                    32
#define PROMIDMASK0             0x0FFEFFFF
#define XC18V04ID               0x05026093
#define PROMIDMASK1             0x0FFFFFFF
#define XCF01SID                0x05044093
#define XCF02SID                0x05045093
#define XCF04SID                0x05046093
#define XCF08PID                0x05057093
#define XCF16PID                0x05058093
#define XCF32PID                0x05059093
// Define PROM (XC18V04, XCFxxS and XCFxxP) JTAG Instructions
//
int PROMINSTLENGTH, PROMID, PROMBYPASS;
#define PROMEXTEST                    0x00
#define PROMSAMPLEPRELOAD             0x01
#define PROMSERASE                    0x0A
#define PROMFVFY3                     0xE2
#define PROMISPEN                     0xE8
#define PROMFPGM                      0xEA
#define PROMFADDR                     0xEB
#define PROMFERASE                    0xEC
#define PROMFDATA0                    0xED
#define PROMCONFIG                    0xEE
#define PROMFVFY0                     0xEF
#define PROMISPEX                     0xF0
#define PROMFDATA3                    0xF3
#define PROMCLAMP                     0xFA
#define PROMHIGHZ                     0xFC
#define PROMUSERCODE                  0xFD
#define PROMIDCODE                    0xFE

#define PROMXSC_DATA_RDPT           0x0004
#define PROMXSC_DATA_UC             0x0006
#define PROMXSC_DATA_CC             0x0007
#define PROMXSC_DATA_DONE           0x0009
#define PROMXSC_DATA_CCB            0x000C
#define PROMXSC_BLANK_CHECK         0x000D
#define PROMXSC_DATA_SUCR           0x000E
#define PROMISC_NOOP                0x00E0
#define PROMXSC_OP_STATUS           0x00E3
#define PROMXSC_ADDRESS_DUMP        0x00E6
#define PROMISC_ENABLE              0x00E8
#define PROMXSC_ENABLEC             0x00E9
#define PROMISC_PROGRAM             0x00EA
#define PROMISC_ADDRESS_SHIFT       0x00EB
#define PROMISC_EREASE              0x00EC
#define PROMISC_DATA_SHIFT          0x00ED
#define PROMXSC_READ                0x00EF
#define PROMXSC_CONFIG              0x00EE
#define PROMISC_DISABLE             0x00F0
#define PROMXSC_MFG_READ            0x00F1
#define PROMXSC_DATA_BTC            0x00F2
#define PROMXSC_CLR_STATUS          0x00F4
#define PROMXSC_DATA_BLANK          0x00F5
#define PROMXSC_CONFIG_PLUS         0x00F6
#define PROMXSC_DATA_WRPT           0x00F7
#define PROMISC_READ                0x00F8
#define PROMXSC_UNLOCK              0xAA55

// Define XC2V1000 ID
//
#define VERTEXIIIDLENGTH                32
#define VERTEXIIIDMASK          0x0FFFFFFF
#define XC2V1000ID              0x01028093
#define XC2V1500ID              0x01030093
#define XC2V2000ID              0x01038093
#define XC2V3000ID              0x01040093
// Define Vertex-II chip (XC2Vxxxx chips) JTAG Instructions
//
#define VERTEXIIINSTLENGTH               6
#define VERTEXIIEXTEST                0x00
#define VERTEXIISAMPLE                0x01
#define VERTEXIIUSER1                 0x02
#define VERTEXIIUSER2                 0x03
#define VERTEXIICFGOUT                0x04
#define VERTEXIICFGIN                 0x05
#define VERTEXIIINTEST                0x07
#define VERTEXIIUSERCODE              0x08
#define VERTEXIIIDCODE                0x09
#define VERTEXIIHIGHZ                 0x0A
#define VERTEXIIJPROGB                0x0B
#define VERTEXIIJSTART                0x0C
#define VERTEXIIJSHUTDOWN             0x0D
#define VERTEXIIBYPASS                0x3F

// Define AX1000 ID
//
#define AX1000IDLENGTH                  32
#define AX1000ID                0x0750605F
// Define AX1000 JTAG Instructions
//
#define AX1000INSTLENGTH                 5
#define AX1000EXTEST                  0x00
#define AX1000PRELOAD                 0x01
#define AX1000INTEST                  0x02
#define AX1000USERCODE                0x03
#define AX1000IDCODE                  0x04
#define AX1000HIGHZ                   0x0E
#define AX1000CLAMP                   0x0F
#define AX1000DIAGNOSTIC              0x10
#define AX1000BYPASS                  0x1F

// Define GOL ID
//
#define GOLIDLENGTH                     32
#define GOLID                   0x14535049
// Define GOL JTAG Instructions
//
#define GOLINSTLENGTH                    5
#define GOLEXTEST                     0x00
#define GOLDEVICEID                   0x01
#define GOLCONFRW                     0x09
#define GOLCONFRO                     0x0A
#define GOLCORETEST                   0x0B
#define GOLBYPASS                     0x1F

// Define TTCrx ID
//
#define TTCRXIDLENGTH                   32
#define TTCRXID                 0x1545408F
// Define TTCrx JTAG Instructions
//
#define TTCRXINSTLENGTH                  4
#define TTCRXEXTEST                    0x0
#define TTCRXIDCODE                    0x1
#define TTCRXINTEST                    0x3
#define TTCRXBYPASS                    0xF

// Define CSM (Vertex-II) ID
//
#define CSMIDLENGTH                     32
#define CSM1ID                  0x43534D31
#define CSM2ID                  0x43534D33
#define CSM3ID                  0x43534D35
#define CSMID                   0x43534D37
// Define CSM (Vertex-II) JTAG Instructions
//
#define CSMINSTLENGTH                    6
#define CSMFULLRW                     0x03
#define CSMIDCODE                     0x09
#define CSMANYUSER                    0x30
#define CSMFULLRO                     0x31
#define CSMTTCRXSTATUS                0x32
#define CSMAMTPARITYERROR             0x33
#define CSMCSMSTATUS                  0x34
#define CSMAMTPHASEERROR              0x35
#define CSMCONFIGRO                   0x36
#define CSMCONFIGRW                   0x37
#define CSMCSMPARMRO                  0x38
#define CSMCSMPARMRW                  0x39
#define CSMVERSIONDATE                0x3A
#define CSMROBITS                     0x3B
#define CSMAMTFULLPHASE               0x3C
#define CSMBYPASS                     0x3F

// Define AMTs ID code
//
#define AMTIDLENGTH                     32
#define AMT1ID                  0x18B83131 // AMT1 IDCODE
#define AMT2ID                  0x18B85031 // AMT2 IDCODE
#define AMT3ID                  0x38B85031 // AMT3 IDCODE

// Define JTAG Instructions for AMT2/AMT3 Plus ASD
// Where bit 5 is parity bit
//
//
#define AMTINSTLENGTH                    5
#define AMTEXTEST                     0x00 // boundary scantest of inter-chip connects    AMT EXTEST
#define AMTIDCODE                     0x11 // scan out chip id code                       AMT IDCODE
#define AMTSAMPLE                     0x12 // sample all pins via boundary scan registers AMT SAMPLE
#define AMTINTEST                     0x03 // test chip with boundary scan registers      AMT INTEST
#define AMTCONTROL                    0x18 // read/write of control data                  AMT CONTROL
#define AMTASDCONTROL                 0x09 // ASD control, AMT2/AMT3 only
#define AMTSTATUS                     0x0A // read status information                     AMT STATUS
#define AMTCORETEST                   0x1B // access to internal test scan register       AMT CORETEST
#define AMTBIST                       0x1C // built in self test for memories             AMT BIST
#define AMTGOUTPORT                   0x1D // general purpose output port, AMT2/AMT3 only AMT GOUTPORT
#define AMTBYPASS                     0x0F // bypass mode                                 AMT BYPASS


//Modified by Xiangting
//JTAG Instructions for HPTDC
#define HPTDCID							0x8470DACE									
#define HPTDCIDCODE_MASK				0xFFFFFFFF

#define HPTDCINSTLENGTH					5
#define HPTDCEXTEST						0x00
#define HPTDCIDCODE						0x11
#define HPTDCSAMPLE						0x12
#define HPTDCINTEST						0x03
#define HPTDCBIST						0x14
#define HPTDCSCAN						0x05
#define HPTDCREADOUT					0x17
#define HPTDCSETUP						0x18
#define HPTDCCONTROL					0x09
#define HPTDCSTATUS						0x0A
#define HPTDCCORETEST					0x1B
#define HPTDCBYPASS						0x0F


//JTAG Instructions for A3P250

#define A3P250ID						0x1BA141CF
//#define A3P250ID						0x12345678 

#define A3P250IDCODE_MASK 				0x06FFFFFF	//A3P250 IDCODE mask. The bits 31..27 and 24 can vary with fabrication lot.
#define A3P250UJTAG_IDCODE 				0x12345678

#define A3P250INSTLENGTH				8

#define A3P250IDCODE					0x70
#define A3P250ASD_WRITE					0x71
#define A3P250ASD_READ 					0x72
#define A3P250FPGA_CW 					0x73
#define A3P250FPGA_CR 					0x74
#define A3P250FPGA_SR 					0x75
#define A3P250I2C_MS_W					0x76
#define A3P250I2C_MS_R 					0x77
#define A3P250BYPASS 					0xFF

//END


// Define CSM board related JTAG device IDs
//
#define CSMPROMID0              0xF5057093
#define CSM1PROMID              0x05026093
#define CSM2PROMID              0x05036093
#define CSMCHIPID				0x01038093
#define CSM1CHIPID		        0x21028093
#define CSM2CHIPID              0x21028093
#define CSM3CHIPID				  AX1000ID

#define JTAGINSTRLENGTH                480
#define MAXJTAGWORDS                  2000
#define MAXJTAGARRAY                  32*MAXJTAGWORDS
#define PBITSLENGTH               10000000

// Define Vertex FPGA Internal Configuration Registers
//
#define VERTEXICRADDRCRC              0x00
#define VERTEXICRADDRFAR              0x01
#define VERTEXICRADDRFDRI             0x02
#define VERTEXICRADDRFDRO             0x03
#define VERTEXICRADDRCMD              0x04
#define VERTEXICRADDRCTL              0x05
#define VERTEXICRADDRMASK             0x06
#define VERTEXICRADDRSTATUS           0x07
#define VERTEXICRADDRLOUT             0x08
#define VERTEXICRADDRCOR              0x09
#define VERTEXICRADDRMFWR             0x0a
#define VERTEXICRADDRFLR              0x0b
#define VERTEXICRADDRIDCODE           0x0e

// Define Vertex FPGA CMD register Commands
//
#define VERTEXCMDWCFG                  0x1
#define VERTEXCMDMFWR                  0x2
#define VERTEXCMDDGHIGH                0x3
#define VERTEXCMDRCFG                  0x4
#define VERTEXCMDSTART                 0x5
#define VERTEXCMDRCAP                  0x6
#define VERTEXCMDRCRC                  0x7
#define VERTEXCMDAGHIGH                0x8
#define VERTEXCMDSWITCH                0x9
#define VERTEXCMDGRESTORE              0xa
#define VERTEXCMDSHUTDOWN              0xb
#define VERTEXCMDGCAPTURE              0xc
#define VERTEXCMDDESYNCH               0xd

// Define Configuration Option Register (COR)
#define VERTEXCORGWECYCLE                0
#define VERTEXCORGWECYCLEMASK          0x7
#define VERTEXCORGTSCYCLE                3
#define VERTEXCORGTSCYCLEMASK         0x38
#define VERTEXCORLOCKCYCLE               6
#define VERTEXCORLOCKCYCLEMASK       0x1C0
#define VERTEXCORMATCHCYCLE              9
#define VERTEXCORMATCHCYCLEMASK      0xE00
#define VERTEXCORDONECYCLE              12
#define VERTEXCORDONECYCLEMASK      0x7000
#define VERTEXCORSSCLKSRC               15
#define VERTEXCORSSCLKSRCMASK      0x18000
#define VERTEXCOROSCFSEL                17
#define VERTEXCOROSCFSELMASK      0x7E0000
#define VERTEXCORSINGLE                 23
#define VERTEXCORDRIVEDONE              24
#define VERTEXCORDONEPIPE               25
#define VERTEXCORSHUTRSTDCM             26
#define VERTEXCORSHUTRSTDCI             27
#define VERTEXCORRESERVED               28
#define VERTEXCORCRCBYPASS              29

// Define VertexII FPGA Configuration Status Register Bits
//
#define VERTEXCSRCRCERROR                0
#define VERTEXCSRRESERVED0               1
#define VERTEXCSRDCMLOCK                 2
#define VERTEXCSRDCIMATCH                3
#define VERTEXCSRINERROR                 4
#define VERTEXCSRGTSCFGB                 5
#define VERTEXCSRGWE                     6
#define VERTEXCSRGHIGHB                  7
#define VERTEXCSRMODE0                   8
#define VERTEXCSRMODE1                   9
#define VERTEXCSRMODE2                  10
#define VERTEXCSRINITB                  11
#define VERTEXCSRDONE                   12
#define VERTEXCSRIDERROR                13
#define VERTEXCSRRESERVED1              14
#define VERTEXCSRRESERVED2              15
#define VERTEXCSRZEROBITS       0xFFFF0000

// Define Xilinx FGPA Packet Header
//
#define XPH2WORDCOUNTBIT0LOCATION        0
#define XPH2WORDCOUNTBITS        0x7FFFFFF
#define XPH2WORDCOUNTMASK       0x07FFFFFF
#define XPHWORDCOUNTBIT0LOCATION         0
#define XPHWORDCOUNTBITS             0x7FF
#define XPHWORDCOUNTMASK        0x000007FF
#define XPHBYTEADDRESSBIT0LOCATION      11
#define XPHBYTEADDRESSBITS             0x3
#define XPHBYTEADDRESSMASK      0x00001800
#define XPHREGADDRESSBIT0LOCATION       13
#define XPHREGADDRESSBITS           0x3FFF
#define XPHREGADDRESSMASK       0x07FFE000
#define XPHOPERATIONBIT0LOCATION        27
#define XPHOPERATIONBITS               0x3
#define XPHOPERATIONMASK        0x18000000
#define XPHTYPEBIT0LOCATION             29
#define XPHTYPEBITS                    0x7
#define XPHTYPEMASK             0xE0000000
#define XPHTYPE1READ            0x28000000
#define XPHTYPE1WRITE           0x30000000
#define XPHTYPE2READ            0x48000000
#define XPHTYPE2WRITE           0x50000000

// Define counters for checking FPGA and PROM
//
#define VFPNUMBERCOUNTER                32
#define VFPTIME                          0
#define VFPNUMBERVERIFYLOOP              1
#define VFPDATAERROR                     2
#define VFPSINGLEDATAERROR               3
#define VFPMULTIPLEDATAERROR             4
#define VFPUNKNOWNTDCWORDS               5
#define VFPUNKNOWNDATAWORDS              6
#define VFPJTAGVERIFY                    7
#define VFPCSMSETUPERROR                 8
#define VFPCSMSETUPNBITERROR             9
#define VFPCSMSTATUSERROR               10
#define VFPCSMERROR                     11
#define VFPTTCRXERROR                   12
#define VFPGOLERROR                     13
#define VFPCSMSETUPERRORONLY            14
#define VFPCSMSTATUSERRORONLY           15
#define VFPDATAERRORONLY                16
#define VFPJTAGFAILURE                  17
#define VFPNUMBERFPGAVERIFY             18
#define VFPFPGAERROR                    19
#define VFPFPGANBITERROR                20
#define VFPFPGAWBIT10000                21
#define VFPFPGANBITWBIT10000            22
#define VFPNUMBERPROMVERIFY             23
#define VFPPROMERROR                    24
#define VFPPROMNBITERROR                25
#define VFPPROMWBIT10000                26
#define VFPPROMNBITWBIT10000            27
#define VFPINITDAQDUETOERROR            28
#define VFPREPROGRAMFPGA                29
#define VFPREPROGRAMPROM                30
#define VFPUNRECOVERABLE                31

unsigned int VFPCounter[VFPNUMBERCOUNTER], confirmVerify, needCheckNumberMezzCard;
unsigned int instrLength, instrArray[JTAGINSTRLENGTH], secondInstrArray[JTAGINSTRLENGTH];
unsigned int dataLength, dataArray[MAXJTAGARRAY], maskArray[MAXJTAGARRAY], readbackArray[MAXJTAGARRAY];
unsigned int JTAGWords[MAXJTAGWORDS], nBitstreamBytes;
int requestVerifyJTAG, requestVerifyPROM, requestVerifyFPGA, requestProgramPROM, requestProgramFPGA;
int requestResetINITDAQ, nProgramDataWords, nProgramMaskWords;
int firstTimeVerifyFPGA, programDataBits[PBITSLENGTH], programMaskBits[PBITSLENGTH];
int PROMOn, XC2V1000On, XC2V2000On, AX1000On, GOLOn, TTCrxOn, CSMOn, mezzCardsOn, oldGOLOn, oldTTCrxOn;
int gotPROM, gotCSMChip, gotXC2V1000, gotXC2V2000, gotAX1000, gotGOL, gotTTCrx, gotCSM, gotAMT, gotUnknown;
int CSMChipID, CSMType, downloadSetup, sampleAMTPhase, turnOffMezzCardJTAG, searchAMTStatus;
int useMessageBox, downloadCSMStatus, downloadGOLStatus, downloadTTCrxStatus, unlockJTAGChain;
int numberCSMDownload, numberBadCSMDownload, numberMezzDownload, numberBadMezzDownload;
int downloadAMTStatus[MAXNUMBERMEZZANINE], downloadASDStatus[MAXNUMBERMEZZANINE];
int PROMOffset, vertexIIOffset, AX1000Offset, GOLOffset, TTCrxOffset, CSMOffset, mezzOffset[MAXNUMBERMEZZANINE];
int PROMInstr, vertexIIInstr, AX1000Instr, GOLInstr, TTCrxInstr, CSMInstr, mezzInstr[MAXNUMBERMEZZANINE];
//Modified by Xiangting
int downloadHPTDCStatus[MAXNUMBERMEZZANINE], downloadA3P250Status[MAXNUMBERMEZZANINE];
int HPTDCOffset[MAXNUMBERMEZZANINE],A3P250Offset[MAXNUMBERMEZZANINE];
int HPTDCInstr[MAXNUMBERMEZZANINE], A3P250Instr[MAXNUMBERMEZZANINE];


//End
int waitTimeInMillionSecond, zeroAllMaskWord, ELMBStorageNumber, thresholdSettingMethod;
int writeSVFFile, oldNumberCSM, numberCSM, oldCSMNumber, CSMNumber, normalJTAG, action;
int pauseDAQToVerifyFPGA, pauseDAQToVerifyPROM, zeroArray[MAXJTAGARRAY];
FILE *radTestLogFile;

// Function prototype for writting Xilinx SVF file
//
void TurnOffMezzCardJTAG(FILE *file);
void AllJTAGDeviceInBYPASS(FILE *file, int option);
void WriteJTAGInstructions(FILE *file);
void WriteJTAGData(FILE *file);
void WriteJTAGDataOnly(FILE *file, int writeMaskBits);
void PROMJTAGComments(int instruction, FILE *file);
void VertexIIJTAGComments(int instruction, FILE *file);
void AX1000JTAGComments(int instruction, FILE *file);
void GOLJTAGComments(int instruction, FILE *file);
void TTCrxJTAGComments(int instruction, FILE *file);
void CSMJTAGComments(int instruction, FILE *file);
void AMTJTAGComments(int instruction, FILE *file, int mezzNumber);
void JTAGComments(int instruction, FILE *file, int IDCode);

void HPTDCJTAGComments(int instruction, FILE *file, int mezzNumber);
void A3P250JTAGComments(int instruction, FILE *file, int mezzNumber);

// Function prototype for programming PROM and FPGA (VertexII)
//
void ProgramVertexIIFPGAButton(void);
void ProgramVertexIIFPGA(void);
void ProgramVertexIIFPGAFromPROM(void);
void ProgramPROMButton(void);
void ProgramPROM(void);
void GetFPGAConfigurationStatus(void);
void ShutdownFPGA(void);
void VerifyVertexIIFPGAButton(void);
void VerifyPROMButton(void);
int VerifyVertexIIFPGA(void);
int VerifyPROM(void);
int VerifyBitFile(int printFlag);
int VerifyMaskFile(int printFlag);
int VerifyRBBFile(int printFlag);
int ConfirmToVerifyFPGAAndPROM(void);
void WriteToFPGAAndPROMLogFile(char str[]);
void SelectRBDFile(void);
void SelectRBBFile(void);
void CheckCSMDAQEanbleAndMezzJTAGEnable(void);
void PROMInstruction(FILE *file, int instr, int tckCycles);
void PROMData(FILE *file, int bitLength, int data[], int mask, int tckCycles);
void FPGAInstruction(FILE *file, int instr, int tckCycles);
void FPGAData(FILE *file, int bitLength, int data[], int mask, int tckCycles, int endData);
void FPGABinaryData(FILE *file, int bitLength, int data[], int mask, int tckCycles, int endData);

// Function prototype for downloading varies devices
//
void DownloadCSMSetup(void);
void DownloadGOLSetup(void);
void DownloadTTCrxSetup(void);
void DownloadMezzCardSetup(void);
void DownloadAMTSetup(void);
void DownloadASDSetup(void);

//Modified by Xiangting
void DownloadA3P250Setup(void);
void DownloadHPTDCSetup(void);

void DownloadHPTDCControl_1(void);
void DownloadHPTDCControl_2(void);
void DownloadHPTDCControl_3(void);
void DownloadHPTDCControl_4(void);
void DownloadHPTDCControl_5(void);
void DownloadHPTDCControl_6(void);
// Function prototype for getting varies device status
//
void GetCSMStatus(void);
void GetGOLStatus(void);
void GetGOLBoundaryScan(void);
void GetTTCrxStatus(void);
void GetAMTStatus(void);



void TurnOnOffGOLAndTTCrxFromJTAGChain(void);
void FillReadbackArray(char *str);
int CheckFullJTAGDataArray(void);
int CheckDataArray(int IDCode, int mezzNumber, int from, int length);
void CheckCSMSetupAndStatus(void);
void CheckCSMAndTTCrxSetup(void);
void CheckGOLSetup(void);
void CheckAMTSetup(void);
void CheckASDSetup(void);

// Varibles for detecting devices in JTAG chain
unsigned int numberDevice, deviceNumber, numberExpectedDevice;
unsigned int deviceID[100], IDCodeInstr[100], lengthInstr[100];

// Function prototype for detecting devices in JTAG chain
//
int FindDevice(int instIDCode, int length, unsigned int expectedID);
void GetDeviceList(void);
void SetupJTAGChain(void);
void AutoSetupJTAGChain(void);


// Function prototype for JTAG diagnostics
//
void GetAllDeviceID(void);
void GetPROMID(void);
void GetCSMChipID(void);
void GetGOLID(void);
void GetTTCrxID(void);
void GetCSMID(void);
void GetAllAMTID(void);

// Function prototype for varies JTAG device operations
//
int CSMJTAGOperation(int instr, int verify);
int GOLJTAGOperation(int instr, int verify);
int TTCrxJTAGOperation(int instr, int verify);
int AMTJTAGOperation(int instr, int verify);
int ASDJTAGOperation(int instr, int verify);

// Function prototype to search mezzanine card in the JTAG chain
//
void UpMezzCardJTAGStatus(void);
void SearchMezzanineCardsAndSetupTest(void);

// Variables and function prototype for NIKEF action and sequence files
//
int newInstructionList, newBitArrayList;
char actFileName[60];
FILE *actFile, *seqFile, *logFile;

void OpenActionFile(int csm);
void CloseActionFile(void);
void WriteCommentToActionFile(char *comment);
void WriteJTAGInstructionToActionFile(int IDCode, int instruction, int mezzNumber);
void WriteJTAGDataToActionFile(int length, int offset);

void OpenSequenceFile(char *filename);
void CloseSequenceFile(void);
void WriteCommentToSequenceFile(char *comment);
void WriteActionToSequenceFile(char *actionFile);
#define NEWASD_NUMBER 99
#define NEWTDC_NUMBER 99

#endif
