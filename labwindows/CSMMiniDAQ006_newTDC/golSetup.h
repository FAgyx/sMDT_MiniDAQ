#ifndef golSetup_H
#define golSetup_H
//
// Filename: golSetup.h
// Author: T.S. Dai
//
// Description:
// This file contains prototypes for all of the GOL JTAG Setup functions.

#define GOLSETUPLENGTH            55
#define GOLBSCANLENGTH            57

// Define GOL Setup Bits
#define LOSSOFLOCKCOUNTER          0
#define CONFIGLINKMODE             9
#define CONFIGWORDMODE             8
#define LINKCONTROLSTATEC         10
#define LINKCONTROLSTATEB         12
#define LINKCONTROLSTATEA         14
#define WAITTIMEFORREADY          16
#define LOSSOFLOCKTIME            21
#define PLLLOCKTIME               24
#define LOSSOFLOCKCONTROL         28
#define LOSSOFLOCKCNTCONTROL      29
#define LOCKCONTROL               30
#define DATATRANSCONTROL          31
#define CHARGEPUMPCURRENT         32
#define TESTSIGNALS               37
#define FLAGBITSCONTROL           39
#define DRIVERCURRSTRENGTH        40
#define CURRENTCONTROL            47
#define HAMMINGCHECKSUM           48

// Define GOL lock state machine status
#define GOLOUTOFLOCK               0
#define GOLLOCKED                  1
#define GOLREADY                   2
#define GOLTXLOSSLOCKCOUNTER       3

// Define GOL Boundary-Scan bits
#define GOLBSCANSDAOUT             0
#define GOLBSCANREADY              1
#define GOLBSCANTESTSHIFT          2
#define GOLBSCANSDAIN              3
#define GOLBSCANSCLIN              4
#define GOLBSCANI2CADDR1           5
#define GOLBSCANI2CADDR2           6
#define GOLBSCANI2CADDR3           7
#define GOLBSCANI2CADDR4           8
#define GOLBSCANI2CADDR5           9
#define GOLBSCANI2CADDR6          10
#define GOLBSCANFF                11
#define GOLBSCANFLAG0             12
#define GOLBSCANFLAG1             13
#define GOLBSCANTXER              14
#define GOLBSCANTXEN              15
#define GOLBSCANDIN00             16
#define GOLBSCANDIN01             17
#define GOLBSCANDIN02             18
#define GOLBSCANDIN03             19
#define GOLBSCANDIN04             20
#define GOLBSCANDIN05             21
#define GOLBSCANDIN06             22
#define GOLBSCANDIN07             23
#define GOLBSCANDIN08             24
#define GOLBSCANDIN09             25
#define GOLBSCANDIN10             26
#define GOLBSCANDIN11             27
#define GOLBSCANDIN12             28
#define GOLBSCANDIN13             29
#define GOLBSCANDIN14             30
#define GOLBSCANDIN15             31
#define GOLBSCANDIN16             32
#define GOLBSCANDIN17             33
#define GOLBSCANDIN18             34
#define GOLBSCANDIN19             35
#define GOLBSCANDIN20             36
#define GOLBSCANDIN21             37
#define GOLBSCANDIN22             38
#define GOLBSCANDIN23             39
#define GOLBSCANDIN24             40
#define GOLBSCANDIN25             41
#define GOLBSCANDIN26             42
#define GOLBSCANDIN27             43
#define GOLBSCANDIN28             44
#define GOLBSCANDIN29             45
#define GOLBSCANDIN30             46
#define GOLBSCANDIN31             47
#define GOLBSCANCONFIPLL          48
#define GOLBSCANCONFIID0          49
#define GOLBSCANCONFIID1          50
#define GOLBSCANCONFNEGEDGE       51
#define GOLBSCANCONFWMODE         52
#define GOLBSCANCONFLASER         53
#define GOLBSCANCONFGLINK         54
#define GOLBSCANRESERVED          55
#define GOLBSCANRESETB            56

int GOLSetupHandle;
int GOLSetupArray[GOLSETUPLENGTH], GOLBScanArray[GOLBSCANLENGTH];
int validGOLStatus;
int confClock, confLinkMode, confWordMode, confLaser, GOLReady, GOLReset;
int tx_er, tx_en, testShift, linkStatusA, linkStatusB, linkStatusC;
int numberOfLockLost, hammingChecksum;

//
// Function prototypes
//
void UpGOLSetupControl(void);
int GOLSetupPanelAndButton(void);
void GOLSetupDone(void);
void SaveGOLSetup(void);
void GOLCurrentControl(void);
void GOLDriverStrength(void);

void GetGOLConfigure(void);
void UpdateGOLSetupPanel(void);
float GOLClockPeriod(void);
void CreatPLLLockTimeList(void);
void CreatWaitTimeList(void);
void CreatLossOfLockTimeList(void);
void LoadGOLSetupArray(void);

#endif
