// jtagControl.c version 5.x                         by T.S.Dai
//
#ifdef _MSC_VER           // Defined for Miscrsoft Visual C++
  #include "MVC_Stuff.h"
#elif defined _CVI_       // Otherwise compile in CVI environment
  #include <userint.h>
  #include <utility.h>
  #include <ansi_c.h>
#endif
#include <nivxi.h>
#include <cvirte.h>       // Needed if linking in external compiler; harmless otherwise
#include <formatio.h>
#include "interface.h"
#include "DAQGUI.h"
#include "csmSetup.h"
#include "ttcrxSetup.h"
#include "golSetup.h"
#include "mezzSetup.h"
#include "TCPServer.h"
#include "jtagDriver.h"
#include "jtagControl.h"

char bitFilePath[5192], mcsFilePath[5192], maskFilePath[5192], msdFilePath[5192];
char rbdFilePath[5192], rbbFilePath[5192], bitStreamTextFName[80], fileDirType[5192];
extern int RunConditionHandle, CSMTestStatus;
extern char currSetupFile[], parameterFile[];
extern defaultProfileSaved, gotJTAGDriver;
extern void PauseDAQ(void);
extern void ResumeDAQ(void);
extern void UpdateTCPStatus(int newConnection);
extern void TTCviEventCounterReset(void);
extern void TTCviBunchCounterReset(void);
extern void TTCviEventAndBunchCounterReset(void);
extern unsigned int TTCviBase;

// JTAG Control Start Up
void JTAGControlStartUp(void) {
  int csm, i, mezz;
  char fname[80];
  
#ifdef WRITESVFFILE
  writeSVFFile = TRUE;
#else
  writeSVFFile = FALSE;
#endif
  PROMID = XC18V04ID;
  PROMINSTLENGTH = 8;
  PROMBYPASS = 0xFF;
  if (CSMChipID == XC2V1000ID) {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, ATTR_LABEL_TEXT, " XC2V1000 ");
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, ATTR_ITEM_NAME, "Get CSM Chip (XC2V1000) ID");
  }
  else if (CSMChipID == XC2V2000ID) {
    PROMID = XCF08PID;
    PROMINSTLENGTH = 16;
    PROMBYPASS = 0xFFFF;
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, ATTR_LABEL_TEXT, " XC2V2000 ");
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, ATTR_ITEM_NAME, "Get CSM Chip (XC2V2000) ID");
  }
  else if (CSMChipID == AX1000ID) {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, ATTR_LABEL_TEXT, "  AX1000  ");
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, ATTR_ITEM_NAME, "Get CSM Chip (AX1000) ID");
  }
  else {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, ATTR_LABEL_TEXT, " CSM-Chip ");
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, ATTR_ITEM_NAME, "Get CSM Chip ID");
  }
  if (SetDir("bitFiles") == 0) SetDir("..");
  else MakeDir("bitFiles");
  strcpy(bitFilePath, "");
  strcpy(mcsFilePath, "");
  strcpy(maskFilePath, "");
  strcpy(msdFilePath, "");
  strcpy(rbdFilePath, "");
  strcpy(rbbFilePath, "");
  strcpy(bitStreamTextFName, "");
  firstTimeVerifyFPGA = TRUE;
  DeleteFile(currSetupFile);
  waitTimeInMillionSecond = 0;
  zeroAllMaskWord = FALSE;
  ELMBStorageNumber = -1;
  if (gotJTAGDriver) action = DOWNLOAD;
  else action = TEXTFILES;
  numberCSM = 1;
  CSMNumber = 0;
  actFile = NULL;
  seqFile = NULL;
  newInstructionList = TRUE;
  newBitArrayList = TRUE;
  doingInitCSMDAQ = FALSE;
  downloadCSMSetupDone = FALSE;
  numberCSMDownload = 0;
  numberBadCSMDownload = 0;
  numberMezzDownload = 0;
  numberBadMezzDownload = 0;
  CSMVersionDate = 0;
  AMTParityError = 0;
  AMTPhaseError = 0;
  for (i = 0; i < 4; i++) AMTPhaseSamples[i] = 0;
  AMTFullPhaseError = 0;
  AMTFullPhaseErrorBit = 0;
  strcpy(CSMCMDText[CMDIDLE], "Next CSM State: Idle State");
  strcpy(CSMCMDText[CMDRESETFPGA], "Next CSM State: Reset CSM FPGA Chip");
  strcpy(CSMCMDText[CMDRESETTTCRX], "Next CSM State: Reset TTCrx Chip");
  strcpy(CSMCMDText[CMDRESETGOL], "Next CSM State: Reset GOL Chip");
  strcpy(CSMCMDText[CMDWAITCLKLOCK], "Next CSM State: Wait Clock Lock");
  strcpy(CSMCMDText[CMDRESETANDWAITCLK25], "Next CSM State: Reset and Wait Clock25");
  strcpy(CSMCMDText[CMDSAMPLEAMTPHASE], "Next CSM State: Sample AMT Phase");
  strcpy(CSMCMDText[CMDJTAGRESET], "Next CSM State: JTAG Reset for TTCrx and GOL");
  strcpy(CSMCMDText[CMDTTCRXLOAD], "Next CSM State: Reset and Load TTCrx");
  strcpy(CSMCMDText[CMDWAITPROM], "Next CSM State: Wait Load TTCrx (PROM)");
  strcpy(CSMCMDText[CMDRESETAMT], "Next CSM State: Reset AMT");
  strcpy(CSMCMDText[CMDRESETERROR], "Next CSM State: Reset Error");
  strcpy(CSMCMDText[CMD12UNKNOWN], "Next CSM State: Undefined CSM State");
  strcpy(CSMCMDText[CMD13UNKNOWN], "Next CSM State: Undefined CSM State");
  strcpy(CSMCMDText[CMD14UNKNOWN], "Next CSM State: Undefined CSM State");
  strcpy(CSMCMDText[CMD15UNKNOWN], "Next CSM State: Undefined CSM State");
  ReadParameters(PARAALL);
  if (SetDir("PARAS") == 0) {
    for (csm = 0; csm < numberCSM; csm++) {
      sprintf(fname, "CSM%04d%s", csm, currSetupFile);
      DeleteFile(fname);
    }
    SetDir("..");
  }
  pauseDAQToVerifyFPGA = TRUE;
  pauseDAQToVerifyPROM = FALSE;
  requestVerifyJTAG = FALSE;
  requestVerifyPROM = FALSE;
  requestVerifyFPGA = FALSE;
  requestProgramPROM = FALSE;
  requestProgramFPGA = FALSE;
  requestResetINITDAQ = FALSE;  
  for (i = 0; i < MAXJTAGARRAY; i++) zeroArray[i] = 0;
  nominalThresholdUsed = FALSE;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    for (i = 0; i < 3; i++) {
      NTThre[i][mezz] = -1;
      NTHyst[i][mezz] = -1;
    }
  } 
  JTAGControlInitialization();
}


// JTAG Control Initialization
void JTAGControlInitialization(void) {
  int i, mezz, enable;

  useMessageBox = FALSE;
  PROMOffset = -1;
  vertexIIOffset = -1;
  AX1000Offset = -1;
  GOLOffset = -1;
  TTCrxOffset = -1;
  CSMOffset = -1;
  downloadCSMStatus = -1;
  downloadGOLStatus = -1;
  downloadTTCrxStatus = -1;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    for (i = 0; i < 3; i++) {
      NTThre[i][mezz] = -1;
      NTHyst[i][mezz] = -1;
    }
    downloadAMTStatus[mezz] = -1;
    downloadASDStatus[mezz] = -1;
  }
  validCSMStatus = FALSE;
  validGOLStatus = FALSE;
  sampleAMTPhase = FALSE;
  turnOffMezzCardJTAG = FALSE;
  needCheckNumberMezzCard = TRUE;
  MezzCardParameterInit();
  SetMezzCardEnables();
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, &enable);
  if (!enable) {
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, &enable);
    SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, enable);
    CSMSetupArray[MEZZJTAGENABLE] = enable;
  }
  UpdateJTAGControlPanel();     // Update JTAG control panel
  SaveParameters(PARAALL);
}


// Load JTAG Control Panel and define user buttons
int JTAGControlPanelAndButton(void) {
  // Define the Title panel handles
  if ((TitleHandle = LoadPanel(0, "DAQGUI.uir", P_TITLE)) < 0) return -1;
  SetPanelAttribute(TitleHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  // Define the JTAG Control panel handles
  if ((JTAGControlHandle = LoadPanel(0, "DAQGUI.uir", P_JTAGCTRL)) < 0) return -1;
  SetPanelAttribute(JTAGControlHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  if ((Menu00Handle = LoadMenuBar(JTAGControlHandle, "DAQGUI.uir", MENU00)) < 0) return -1;
  // Define the select action panel handles
  if ((SelectActionHandle = LoadPanel(0, "DAQGUI.uir", P_SELACT)) < 0) return -1;
  SetPanelAttribute(SelectActionHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  // Define the panel handles for confirmation to verify FPGA and PROM
  if ((ConfirmVFPHandle = LoadPanel(0, "DAQGUI.uir", P_SELVFP)) < 0) return -1;
  SetPanelAttribute(ConfirmVFPHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  // Define mezzanine card JTAG status panel handles
  if ((MezzJTAGStatusHandle = LoadPanel(0, "DAQGUI.uir", P_MEZZJTAG)) < 0) return -1;
  SetPanelAttribute(MezzJTAGStatusHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);

  SetupUserButton(Menu00Handle, MENU00_FILE_LOADPROFILE, LoadProfile);
  SetupUserButton(Menu00Handle, MENU00_FILE_SAVEPROFILE, SaveProfile);
  SetupUserButton(Menu00Handle, MENU00_FILE_SAVEDEFAULTPROFILE, SaveDefaultProfile);
  SetupUserButton(Menu00Handle, MENU00_FILE_CLEARDEFAULTPROFILE, ClearDefaultProfile);
  SetupUserButton(Menu00Handle, MENU00_FILE_GTEXTFILES, GenerateAllTextFilesForDAQ);
  SetupUserButton(Menu00Handle, MENU00_FILE_GACTSEQFILES, GenerateAllActSeqFilesForDAQ);
  SetupUserButton(Menu00Handle, MENU00_EXE_PROGRAMFPGA, ProgramVertexIIFPGAButton);
  SetupUserButton(Menu00Handle, MENU00_EXE_PROGRAMPROM, ProgramPROMButton);
  SetupUserButton(Menu00Handle, MENU00_EXE_PROGRAMFPGAFROMPROM, ProgramVertexIIFPGAFromPROM);
  SetupUserButton(Menu00Handle, MENU00_EXE_VERIFYFPGA, VerifyVertexIIFPGAButton);
  SetupUserButton(Menu00Handle, MENU00_EXE_VERIFYPROM, VerifyPROMButton);
  SetupUserButton(Menu00Handle, MENU00_EXE_SHUTDOWNFPGA, ShutdownFPGA);
  SetupUserButton(Menu00Handle, MENU00_EXE_FPGACONFSTATUS, GetFPGAConfigurationStatus);
  SetupUserButton(Menu00Handle, MENU00_JTAG_SETUPJTAGCHAIN, AutoSetupJTAGChain);
  SetupUserButton(Menu00Handle, MENU00_JTAG_RESETTAP, ResetTAPButton);
  SetupUserButton(Menu00Handle, MENU00_JTAG_SEARCHMEZZCARD, UpMezzCardJTAGStatus);
  SetupUserButton(Menu00Handle, MENU00_JTAG_GETDEVICELIST, GetDeviceList);
  SetupUserButton(Menu00Handle, MENU00_JTAG_GETALLDEVICEID, GetAllDeviceID);
  SetupUserButton(Menu00Handle, MENU00_JTAG_GETPROMID, GetPROMID);
  SetupUserButton(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, GetCSMChipID);
  SetupUserButton(Menu00Handle, MENU00_JTAG_GETGOLID, GetGOLID);
  SetupUserButton(Menu00Handle, MENU00_JTAG_GETTTCRXID, GetTTCrxID);
  SetupUserButton(Menu00Handle, MENU00_JTAG_GETCSMID, GetCSMID);
  SetupUserButton(Menu00Handle, MENU00_JTAG_CLEARSETUPSTATUS, ClearSetupStatus);
  SetupUserButton(Menu00Handle, MENU00_HELP_BUTTONS, HelpButton);
  SetupUserButton(Menu00Handle, MENU00_HELP_ABOUT, HelpAbout);
  SetupUserButton(TitleHandle, P_TITLE_PICTURE, AboutDone);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, StartOrStopCSMDAQ);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_RESETCSMCHIP, ResetCSMOrVerifyPROM);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_SAMPLEAMTPHASE, SampleAMTPhaseOrVerifyFPGA);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_NUMBERCSM, SetNumberCSMButton);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, SetCSMNumber);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, SetJTAGRateDivisorButton);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_PROM, JTAGChainPROM);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, JTAGChainCSMChip);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_GOL, JTAGChainGOLButton);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_TTCRX, JTAGChainTTCrxButton);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_CSM, JTAGChainCSM);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, JTAGChainMezzCardsButton);
  SetupUserButton(JTAGControlHandle, P_JTAGCTRL_JTAGCHAINCONTROL, JTAGChainControl);
  SetupUserButton(ConfirmVFPHandle, P_SELVFP_DEFAULT, PanelDefault);
  SetupUserButton(ConfirmVFPHandle, P_SELVFP_LOADPROM, ProgramPROMButton);
  SetupUserButton(ConfirmVFPHandle, P_SELVFP_SELECTRBDFILE, SelectRBDFile);
  SetupUserButton(ConfirmVFPHandle, P_SELVFP_DONOTHING, PanelDone);
  SetupUserButton(MezzJTAGStatusHandle, P_MEZZJTAG_REDO, SearchMezzanineCardsAndSetupTest);
  SetupUserButton(MezzJTAGStatusHandle, P_MEZZJTAG_CLOSE, PanelDone);

  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_CSMID, CSMJTAGOperation, CSMIDCODE, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_CSMVERSIONDATE, CSMJTAGOperation, CSMVERSIONDATE, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_AMTPARITYERROR, CSMJTAGOperation, CSMAMTPARITYERROR, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_AMTPHASEERROR, CSMJTAGOperation, CSMAMTPHASEERROR, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_AMTFULLPHASE, CSMJTAGOperation, CSMAMTFULLPHASE, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_CSMSTATUS, CSMJTAGOperation, CSMCSMSTATUS, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_TTCRXSTATUS, CSMJTAGOperation, CSMTTCRXSTATUS, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_CSMROBITS, CSMJTAGOperation, CSMROBITS, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_CSMFULLRW, CSMJTAGOperation, CSMFULLRW, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_CSMFULLRO, CSMJTAGOperation, CSMFULLRO, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_CSMCONFIGRW, CSMJTAGOperation, CSMCONFIGRW, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_CSMCONFIGRO, CSMJTAGOperation, CSMCONFIGRO, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_CSMPARARW, CSMJTAGOperation, CSMCSMPARMRW, TRUE);
  SetupUserButton2(Menu00Handle, MENU00_JTAG_DIAG_CSMPARARO, CSMJTAGOperation, CSMCSMPARMRO, TRUE);

  if (JTAGDRIVERTYPE == CSM0) {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_MIN_VALUE, 0);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_MAX_VALUE, 7);
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, 4);
  }
  else if (JTAGDRIVERTYPE == CORELIS11491) {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_MIN_VALUE, 0);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_MAX_VALUE, 15);
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, 9);
  }
  else if (JTAGDRIVERTYPE == CANELMB) {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_MIN_VALUE, 0);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_MAX_VALUE, 3);
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, 0);
    SetMenuBarAttribute(Menu00Handle, MENU00_EXE_PROGRAMFPGA, ATTR_DIMMED, TRUE);
  }
  else {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_MIN_VALUE, 0);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_MAX_VALUE, 0);
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, 0);
  }
  
  return 0;
}


void ResetTAPButton(void) {
  if (controlOption == NORM) {
    ResetTAP();
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on JTAG Reset",
                 "Issue a JTAG Reset Command.");
    controlOption = NORM;
  }
}


void StartOrStopCSMDAQ(void) {
  int i;
  char str[40], *strp;
  
  GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, str);
  for (i = 0; i <= strlen(str); i++) str[i] = tolower(str[i]);
  strp = strstr(str, "check");
  if (strp != NULL) requestVerifyJTAG = TRUE;
  else {
    useMessageBox = FALSE;
    strp = strstr(str, "start");
    if (controlOption == NORM) {
      UpdateCSMControlBits();
      SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
      SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDIDLE);
      if (strp == NULL) SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
      else SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 1);
      CSMDAQControl();
      CSMSetupDone();
      if (strp == NULL) printf("CSM DAQ Stopped!\n");
      else printf("CSM DAQ Started!\n");
    }
  }
  StartOrStopCSMDAQButton();
}


void StartOrStopCSMDAQButton(void) {
  int enable, checkMode;
  
  GetCtrlVal(ConfirmVFPHandle, P_SELVFP_CHECKMODE, &checkMode);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, &enable);
  if (controlOption == NORM) {
    if (verifyFPGAAndPROM && (checkMode <= 0))
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Check JTAG & CSM");
    else if (enable == 0)
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Start CSM DAQ");
    else
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Stop CSM DAQ");
  }
  else if (controlOption == HELP) {
    if (verifyFPGAAndPROM && (checkMode <= 0))
      MessagePopup("Help on Check JTAG & CSM",
                   "Push this button to check JTAG chain and CSM status.");
    else if (enable == 0)
      MessagePopup("Help on Start CSM DAQ Button",
                   "Push this button to enable DAQ at CSM.");
    else
      MessagePopup("Help on Stop CSM DAQ Button",
                   "Push this button to disable DAQ at CSM.");
    controlOption = NORM;
  }
}


void SampleAMTPhaseOrVerifyFPGA(void) {
  int i;
  char str[40], *strp;
  
  GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_SAMPLEAMTPHASE, ATTR_LABEL_TEXT, str);
  for (i = 0; i <= strlen(str); i++) str[i] = tolower(str[i]);
  strp = strstr(str, "verify");
  if (strp == NULL) SampleAMTPhase();
  else if (controlOption == NORM) {
    requestVerifyJTAG = TRUE;
    requestVerifyFPGA = TRUE;
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Verify FPGA Button",
                 "Push this button to verify FPGA configuration.\nNote! The request ignored if FPGA under verification.");
    controlOption = NORM;
  }
}


void SampleAMTPhase(void) {
  int panel, version, checkMode;
  
  if (controlOption == NORM) {
    UpdateCSMControlBits();
    if (needCheckNumberMezzCard) {
      CheckNumberMezzCard(0);
      needCheckNumberMezzCard = FALSE;
    }
    sampleAMTPhase = FALSE;
    if (nbMezzCard > 0) {
      BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
      if ((MezzCardsClockMode() != AMTEDGECONTINE) && (version >= 10)) {   // Need change AMT clock mode
        printf("Change mezzanine card clock mode to 'Edge Continue' to sample AMT Phase.\n");
        sampleAMTPhase = TRUE;
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, 1);
        SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
        if (doingInitCSMDAQ) DownloadAllMezzCardSetup();
        else DownloadSetup();
        printf("Mezzanine card clock mode is changed to 'Edge Continue'.\n");
      }
      if (CSMStatusArray[CSMERRORBIT] == 1) ResetCSMError();
      CSMSampleAMTPhase();
      if (sampleAMTPhase) {                         // Need change back AMT clock mode
        printf("Change mezzanine card clock mode to user set value.\n");
        sampleAMTPhase = FALSE;
        DownloadAllMezzCardSetup();
        printf("mezzanine card clock mode is changed to user set value.\n");
      }
      if (action == DOWNLOAD) {
        if (!doingInitCSMDAQ) GetCSMStatus();
		printf("GetCSMStatus() is in void SampleAMTPhase()\n");
		
        if ((CSMStatusArray[PHASEERRORBIT] == 1) || (AMTPhaseError != 0)) {
          CSMJTAGOperation(CSMAMTPHASEERROR, FALSE);
          if (CSMVersionDate > 2005021522) CSMJTAGOperation(CSMAMTFULLPHASE, FALSE);
        }
      }
      printf("Sample AMT Phase Done.\n");
    }
    else printf("No mezzanine card is enabled, and 'Sample AMT Phase' is ignored!\n");
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Sample AMT Phase Button",
                 "Push this button to sample AMT Phase.");
    controlOption = NORM;
  }
}


void ResetCSMOrVerifyPROM(void) {
  int i;
  char str[40], *strp;
  
  GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_RESETCSMCHIP, ATTR_LABEL_TEXT, str);
  for (i = 0; i <= strlen(str); i++) str[i] = tolower(str[i]);
  strp = strstr(str, "verify");
  if (strp == NULL) ResetCSM();
  else if (controlOption == NORM) {
    requestVerifyJTAG = TRUE;
    requestVerifyPROM = TRUE;
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Verify PROM Button",
                 "Push this button to verify PROM content.\nNote! The request ignored if PROM under verification");
    controlOption = NORM;
  }
}


void InitCSMDAQ(void) {
  int version, panel;
  
  if (controlOption == NORM) {
    useMessageBox = FALSE;
    panel = GetActivePanel();
    if (panel == SelectActionHandle) PanelDone();
    printf("Start to initialize CSM DAQ.\n");
    doingInitCSMDAQ = TRUE;
    if (action == DOWNLOAD) ResetTAP();
    UpdateCSMControlBits();
    version = -1;
    if (CSMOn) {
      if (MDTChamberTest) {
        ResetCSM();
        ResetAndDownloadTTCrxMethod2();
      }
      else {
        if (CSMStatusArray[CSMERRORBIT] == 1) ResetCSMError();
        ResetAndDownloadTTCrxMethod1();
      }
      BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
    }
    if ((version < 0) || (version > 10)) {
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, 1);
      SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
      UpdateCSMControlBits();
      GetJTAGChainOnList();
    }
    if ((!gotTTCvi) && mezzCardsOn) {
      if (needCheckNumberMezzCard) {
        CheckNumberMezzCard(0);
        needCheckNumberMezzCard = FALSE;
      }
      if (nbMezzCard > 0) AMTGlobalReset();
    }
    downloadSetup = TRUE;
    SaveSetupToSVFFile();        // Download GOL and CSM settings
    downloadSetup = FALSE;
    if (CSMOn) {
      SampleAMTPhase();
      if (nbMezzCard <= 0) {
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, 0);
        SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 0);
        DownloadCSMSetup();
      }
    }
    printf("Get CSM Board Status.\n");
    GetCSMStatus();
	
	printf("GetCSMStatus() is invoid InitCSMDAQ(void)\n");
	
    if (action == DOWNLOAD) {
      printf("Get AMT Phase Sample Bits\n");
      CSMJTAGOperation(CSMAMTPHASEERROR, FALSE);
      if (CSMVersionDate > 2005021522) CSMJTAGOperation(CSMAMTFULLPHASE, FALSE);
    }
    if (gotTTCvi) {
      TTCviEventCounterReset();
      TTCviBunchCounterReset();
      TTCviEventAndBunchCounterReset();
    }
    doingInitCSMDAQ = FALSE;
    printf("CSM DAQ initialization DONE.\n");
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on InitCSMDAQ Button",
                 "Push this button to perform CSM DAQ initialization.");
    controlOption = NORM;
  }
}


void SetNumberCSMButton(void) {
  oldNumberCSM = numberCSM;
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_NUMBERCSM, &numberCSM);
  if (numberCSM <= 0) numberCSM = 1;
  else if (oldNumberCSM != numberCSM) {
    LoopAndCheckUserButtonAndFunction(SetNumberCSMButton, SetNumberCSM);
  }
}


void SetNumberCSM(void) {
  int resp = 1, csm;
  char str[512], fname[80];
  
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_NUMBERCSM, &numberCSM);
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, &oldCSMNumber);
  if (numberCSM <= 0) numberCSM = 1;
  else if (oldNumberCSM != numberCSM) {
    sprintf(str, "Are you sure to change number of CSM from %d to %d?\n"
                 "If confirmed, all CSM settings will be reset to default.\n",
                 oldNumberCSM, numberCSM);
    resp = ConfirmPopup("Confirm Changing Number CSM", str);
    if (resp != 0) {
      if (SetDir("PARAS") == 0) {
        for (csm = 0; csm < oldNumberCSM; csm++) {
          sprintf(fname, "CSM%04d%s", csm, currSetupFile);
          DeleteFile(fname);
          sprintf(fname, "CSM%04d%s", csm, parameterFile);
          DeleteFile(fname);
        }
        SetDir("..");
      }
    }
  }
  if (resp == 0) numberCSM = oldNumberCSM;
  SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_NUMBERCSM, numberCSM);
  SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, ATTR_MIN_VALUE, 0);
  SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, ATTR_MAX_VALUE, numberCSM-1);
  SaveParameters(PARANUMBERCSM);
  CSMNumber = oldCSMNumber;
  SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, CSMNumber);
  SetCSMNumber();
}


void SetCSMNumber(void) {
  
  oldCSMNumber = CSMNumber;
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, &CSMNumber);
  if (oldCSMNumber != CSMNumber) {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_NUMBERCSM, ATTR_DIMMED, TRUE);
    LoopAndCheckUserButtonAndFunction(SetCSMNumber, SaveAndRecallJTAGControlParas);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_NUMBERCSM, ATTR_DIMMED, FALSE);
  }
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, &CSMNumber);
  if (CSMNumber >= numberCSM) {
    CSMNumber = 0;
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, CSMNumber);
  }
  SaveParameters(PARACURRENTCSMNUMBER);  
}


void SaveAndRecallJTAGControlParas(void) {
  int savedCSMNumber, size;
  char targetFile[80], sourceFile[80];

  if (SetDir("PARAS") == 0) SetDir("..");
  else MakeDir("PARAS");
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, &savedCSMNumber);
  SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, oldCSMNumber);
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, &CSMNumber);
  SaveParameters(PARACURRENTCSMNUMBER);
  sprintf(targetFile, "PARAS/CSM%04d%s", CSMNumber, currSetupFile);
  CopyFile(currSetupFile, targetFile);
  sprintf(targetFile, "PARAS/CSM%04d%s", CSMNumber, parameterFile);
  CopyFile(parameterFile, targetFile);
  if (defaultProfileSaved) {
    sprintf(targetFile, "PARAS/CSM%04default.prf", CSMNumber);
    CopyFile("default.prf", targetFile);
  }
  if (savedCSMNumber != CSMNumber) {
    CSMNumber = savedCSMNumber;
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, CSMNumber);
    SaveParameters(PARACURRENTCSMNUMBER);
    sprintf(targetFile, "PARAS/%s", parameterFile);
    CopyFile(parameterFile, targetFile);
    SetDir("PARAS");
    sprintf(sourceFile, "CSM%04d%s", CSMNumber, parameterFile);
    sprintf(targetFile, "../%s", parameterFile);
    if (GetFileInfo(sourceFile, &size)) {
      CopyFile(sourceFile, targetFile);
      CopyFile(sourceFile, parameterFile);
    }
    else {
      defaultProfileSaved = FALSE;
      SaveParameters(PARADEFAULTSAVED);
      CopyFile(parameterFile, targetFile);
    }
    ReadParameters(PARADEFAULTSAVED);
    if (defaultProfileSaved) {
      sprintf(sourceFile, "CSM%04default.prf", CSMNumber);
      strcpy(targetFile, "../default.prf");
      if (CopyFile(sourceFile, targetFile) != 0) {
        SetDir("..");
        DeleteFile("default.prf");
        SetDir("PARAS");
      }
    }
    sprintf(sourceFile, "CSM%04d%s", CSMNumber, currSetupFile);
    sprintf(targetFile, "../%s", currSetupFile);
    if (GetFileInfo(sourceFile, &size)) {
      if (CopyFile(sourceFile, targetFile) != 0) {
        SetDir("..");
        DeleteFile(currSetupFile);
        SetDir("PARAS");
      }
    }
    else if (defaultProfileSaved) {
      sprintf(sourceFile, "CSM%04default.prf", CSMNumber);
      if (CopyFile(sourceFile, targetFile) != 0) {
        SetDir("..");
        DeleteFile(currSetupFile);
        SetDir("PARAS");
      }
    }
    SetDir("..");
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, &CSMNumber);
    SaveParameters(PARACURRENTCSMNUMBER);  
    JTAGControlInitialization();
  }
}


void SetJTAGRateDivisorButton(void) {
  int divider;
  
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, &divider);
  SetJTAGRateDivider(divider);
}


void LoadProfile(void) {
  if (controlOption == NORM) LoadSavedProfile(2);
  else if (controlOption == HELP) {
    MessagePopup("Load Profile", 
                 "Load the selected profile and CSM/TTCrx/GOL/Mezzanine Card settings\n"
                 "The settings will be downloaded if confirmed");
    controlOption = NORM;
  }
}


void SaveProfile(void) {
  char path[5192];
  
  if (controlOption == NORM) {
    if (FileSelectPopup("", "*.prf", "", "Save Control Panels and JTAG Settings",
                        VAL_SAVE_BUTTON, 0, 1, 1, 0, path)) {
      SaveAllPanelState(path, 0);
      MessagePopup("File Saved", "Data stored to file.");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Save Profile", 
                 "Save current control panels and the CSM/TTCrx/GOL/Mezzanine Card settings to a file.");
    controlOption = NORM;
  }
}					

void SaveDefaultProfile(void) {
  int runType, ctrlItem, nlist, item, resp;
  char typeStr0[256], typeStr1[256], str[1024];
  
  if (controlOption == NORM) {
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_RUNTYPE, &runType);
    if (runType != NORMALALLCALIBCHANNELOFF) {
      GetCtrlIndex(JTAGControlHandle, P_JTAGCTRL_RUNTYPE, &ctrlItem);
      GetLabelFromIndex(JTAGControlHandle, P_JTAGCTRL_RUNTYPE, ctrlItem, typeStr0);
      GetNumListItems(JTAGControlHandle, P_JTAGCTRL_RUNTYPE, &nlist);
      for (item = 0; item < nlist; item++) {
        GetValueFromIndex(JTAGControlHandle, P_JTAGCTRL_RUNTYPE, item, &runType);
        if (runType == NORMALALLCALIBCHANNELOFF) {
          GetLabelFromIndex(JTAGControlHandle, P_JTAGCTRL_RUNTYPE, item, typeStr1);
          break;
        }
      }
      sprintf(str, "Warning! You are going to save default profile with special run type\n\n"
                   "   ===> %s\n\n"
                   "especially it is not good for doing MDT chamber test!\n"
                   "The recommended run type for default profile is\n\n"
                   "   ===> %s\n\n"
                   "Are you sure to save default profile with special run type?\n",
                   typeStr0, typeStr1);
      resp = ConfirmPopup("Confirm Save Default Profile with Special Run Type", str);
      if (resp == 0) return;
    }
    SaveAllPanelState("default.prf", 0);
    defaultProfileSaved = 1;
    SaveParameters(PARADEFAULTSAVED);
    MessagePopup("Default Profile Saved", "Default Profile was saved into <default.prf>.");
  }
  else if (controlOption == HELP) {
    MessagePopup("Save Default Profile", 
                 "Save current control panels and the CSM/TTCrx/GOL/Mezzanine Card settings to as DEFAULT.");
    controlOption = NORM;
  }
}


void ClearDefaultProfile(void) {
  int resp, size;
  
  if (controlOption == NORM) {
    resp = ConfirmPopup("Confirm to Clear Default Profile", 
                        "Are you sure to clear default profile <default.prf>?");
    if (resp) {
      if (GetFileInfo("default.prf", &size)) DeleteFile("default.prf");
      defaultProfileSaved = 0;
      SaveParameters(PARADEFAULTSAVED);
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Clear Default Profile", 
                 "Clear DEFAULT profile which is used at startup and for varies DEFAULT button.");
    controlOption = NORM;
  }
}


// Load Saved Profile
// 
void LoadSavedProfile(int loadProfile) {
  // loadProfile = 0           Do nothing
  //               1           Load default profile
  //               2           Load selected profile
  char path[5192], str[20];
  int resp, size, recallProfile = 0, gotFunc, dimmed, savedMeasureTriggerRate;

  if (loadProfile == 1) {
    strcpy(path, "default.prf");
    recallProfile = TRUE;
    if (!GetFileInfo(path, &size)) {
      defaultProfileSaved = 0;
      SaveParameters(PARADEFAULTSAVED);
    }
  }
  else if (loadProfile > 1) {
    recallProfile = FileSelectPopup("", "*.prf", "", "Retrieve CSM/GOL/TTCrx/MezzCard Settings",
                                    VAL_LOAD_BUTTON, 0, 1, 1, 0, path);
  }
  if (recallProfile && GetFileInfo(path, &size)) {
    RecallAllPanelState(path);
    SetJTAGRateDivisorButton();
    LoadCSMSetupArray();
    GetMezzCardEnables();
    JTAGControlInitialization();
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, &CSMOn);
    HidePanel(JTAGControlHandle);
    DisplayPanel(SelectActionHandle);
    SetActivePanel(SelectActionHandle);
    savedMeasureTriggerRate = measureTriggerRate;
    measureTriggerRate = FALSE;
    gotFunc = 0;
    while (gotFunc == 0) {
      gotFunc = CheckUserButtons(PanelDone);
#ifdef TTCVIDRIVERVIATCP
      dimmed = FALSE;
      if (measureTriggerRate) {
        savedMeasureTriggerRate = measureTriggerRate;
        measureTriggerRate = FALSE;
      }
      else if (TTCviTCPConnected && (!gotTTCvi)) dimmed = TRUE;
      SetCtrlAttribute(SelectActionHandle, P_SELACT_INITDAQ, ATTR_DIMMED, dimmed);
      SetCtrlAttribute(SelectActionHandle, P_SELACT_DOWNLOAD, ATTR_DIMMED, dimmed);
      SetCtrlAttribute(SelectActionHandle, P_SELACT_DONOTHING, ATTR_DIMMED, dimmed);
#endif
    }
    measureTriggerRate = savedMeasureTriggerRate;
    if (gotFunc == 1) PanelDone();
    else {
      HidePanel(SelectActionHandle);
      DisplayPanel(JTAGControlHandle);
      SetActivePanel(JTAGControlHandle);
    }
    UpdateJTAGControlPanel();     // Update JTAG control panel
  }
  else if (recallProfile && !GetFileInfo(path, &size)) {
    printf("Profile <%s> does not exist, please check it.\n", path);
  }
}


void GenerateAllTextFilesForDAQ(void)
{
  int csm, oldNormalJTAG;
  char filename[80];
  
  if (controlOption == NORM) {
    oldNormalJTAG = normalJTAG;
    printf("Start to generate all text files for DAQ.\n");
    action = TEXTFILES;
    for (csm = 0; csm < numberCSM; csm++) {
      GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, &oldCSMNumber);
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, csm);
      SaveAndRecallJTAGControlParas();
      PanelRecall(CSMSetupHandle);
      PanelRecall(TTCrxSetupHandle);
      PanelRecall(GOLSetupHandle);
      SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
      SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDIDLE);
      SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
      SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
      LoadCSMSetupArray();
      
      UpdateCSMControlBits();
      printf("All DAQ text files are generated for CSM %d.\n", csm);
    }
    if (oldNormalJTAG != 0) {
      action = DOWNLOAD;
      normalJTAG = oldNormalJTAG;
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Generate All Text Files For DAQ", 
                 "Generate all necessary text files for DAQ.");
    controlOption = NORM;
  }
}


void GenerateAllActSeqFilesForDAQ(void)
{
  int csm, enable, step, oldNormalJTAG;
  FILE *nullFile;
  
  if (controlOption == NORM) {
    oldNormalJTAG = normalJTAG;
    nullFile = NULL;
    if (SetDir("action") == 0) SetDir("..");
    else MakeDir("action");
    waitTimeInMillionSecond = 0;
    zeroAllMaskWord = FALSE;
    ELMBStorageNumber = -1;
    printf("Start to generate all action and sequence files for DAQ.\n");
    action = ACTSEQFILES;
    for (csm = 0; csm < numberCSM; csm++) {
      GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, &oldCSMNumber);
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, csm);
      SaveAndRecallJTAGControlParas();
      PanelRecall(CSMSetupHandle);
      PanelRecall(TTCrxSetupHandle);
      PanelRecall(GOLSetupHandle);
      SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
      SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDIDLE);
      SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
      SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
      LoadCSMSetupArray();
      
      UpdateCSMControlBits();
      if (CSMOn) {
        sprintf(actFileName, "action/resetCSM%04d.act", csm);
        ELMBStorageNumber = 4;
        waitTimeInMillionSecond = 1000;
        zeroAllMaskWord = TRUE;
        OpenActionFile(csm);
        WriteCommentToActionFile("Reset CSM board");
        WriteCommentToActionFile("Need to wait for 0.800 second at least");
        ResetCSM();
        CloseActionFile();
        waitTimeInMillionSecond = 0;
        zeroAllMaskWord = FALSE;

        sprintf(actFileName, "action/downloadCSMTTCrx%04d.act", csm);
        ELMBStorageNumber = 5;
        waitTimeInMillionSecond = 1;
        OpenActionFile(csm);
        WriteCommentToActionFile("Reset TTCrx and Download CSM & TTCrx Setup");
        WriteCommentToActionFile("Need to wait for 0.00040 second (40us) at least");
        ResetAndDownloadTTCrxMethod2();
        CloseActionFile();
        waitTimeInMillionSecond = 0;

        sprintf(actFileName, "action/getCSMStatus%04d.act", csm);
        ELMBStorageNumber = 8;
        OpenActionFile(csm);
        WriteCommentToActionFile("Get CSM Status");
        GetCSMStatus();
		
		printf("GetCSMStatus() is in void GenerateAllActSeqFilesForDAQ(void)\n"); 
		
        CloseActionFile();

		if (GOLOn) {
          sprintf(actFileName, "action/downloadCSMGOL%04d.act", csm);
          ELMBStorageNumber = 6;
          OpenActionFile(csm);
          WriteCommentToActionFile("Download CSM & GOL Setup");
          downloadSetup = TRUE;
          SaveSetupToSVFFile();
          downloadSetup = FALSE;
          CloseActionFile();
        }

        sprintf(actFileName, "action/sampleAMTPhase%04d.act", csm);
        ELMBStorageNumber = 7;
        OpenActionFile(csm);
        WriteCommentToActionFile("Sample AMT Phase");
        CSMSampleAMTPhase();
        CloseActionFile();

        ELMBStorageNumber = -1;
        GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, &enable);
        if (enable) {
          sprintf(actFileName, "action/startCSMDAQMezzJTAGOn%04d.act", csm);
          OpenActionFile(csm);
          WriteCommentToActionFile("Start CSM DAQ, where JTAG signals for mezzanine card are on");
          StartCSMDAQ();
          CloseActionFile();
          SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
          LoadCSMSetupArray();

          sprintf(actFileName, "action/stopCSMDAQMezzJTAGOn%04d.act", csm);
          OpenActionFile(csm);
          WriteCommentToActionFile("Stop CSM DAQ, where JTAG signals for mezzanine card are on");
          StopCSMDAQ();
          CloseActionFile();
        }
        
        printf("Preparing to generate start CSM DAQ file for CSM %d\n", csm);
        ELMBStorageNumber = 10;
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, 0);
        SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 0);
        sprintf(actFileName, "action/startCSMDAQMezzJTAGOff%04d.act", csm);
        OpenActionFile(csm);
        WriteCommentToActionFile("Start CSM DAQ, where JTAG signals for mezzanine card are off");
        StartCSMDAQ();
        CloseActionFile();
        sprintf(actFileName, "action/getCSMStatusInRunMezzJTAGOff%04d.act", csm);
        ELMBStorageNumber = 12;
        OpenActionFile(csm);
        WriteCommentToActionFile("Get CSM Status during Run where JTAG Signals to Mezzanine Card Are Off");
        GetCSMStatus();
		
		printf("GetCSMStatus() is in void GenerateAllActSeqFilesForDAQ(void)\n");    		
		
        CloseActionFile();
        SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, enable);
        SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
        LoadCSMSetupArray();
        GetJTAGChainOnList();

        sprintf(actFileName, "action/stopCSMDAQMezzJTAGOff%04d.act", csm);
        ELMBStorageNumber = 11;
        OpenActionFile(csm);
        WriteCommentToActionFile("Stop CSM DAQ, where JTAG signals for mezzanine card are off");
        StopCSMDAQ();
        CloseActionFile();
        sprintf(actFileName, "action/getCSMStatusMezzJTAGOff%04d.act", csm);
        ELMBStorageNumber = 13;
        OpenActionFile(csm);
        WriteCommentToActionFile("Get CSM Status where JTAG Signals to Mezzanine Card Are Off");
        GetCSMStatus();
        CloseActionFile();

        sprintf(actFileName, "action/turnOffMezzJTAG%04d.act", csm);
        ELMBStorageNumber = 9;
        OpenActionFile(csm);
        WriteCommentToActionFile("Turn Off JTAG for mezzanine cards");
        SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 0);
        LoadCSMSetupArray();
        TurnOffMezzCardJTAG(nullFile);
        CloseActionFile();

        sprintf(actFileName, "action/turnOnMezzJTAG%04d.act", csm);
        ELMBStorageNumber = -1;
        OpenActionFile(csm);
        WriteCommentToActionFile("Turn On JTAG for mezzanine cards");
        SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
        DownloadCSMSetup();
        CloseActionFile();
      }
      ELMBStorageNumber = -1;
      if (GOLOn) {
        sprintf(actFileName, "action/downloadGOL%04d.act", csm);
        OpenActionFile(csm);
        WriteCommentToActionFile("Download GOL Setup");
        DownloadGOLSetup();
        CloseActionFile();

      }
      if (mezzCardsOn) {
        sprintf(actFileName, "action/getAllAMTID%04d.act", csm);
        OpenActionFile(csm);
        WriteCommentToActionFile("Get AMT ID for all mezzanine cards");
        GetAllAMTID();
        CloseActionFile();

        printf("Preparing to generate mezzanine card JTAG settings for CSM %d\n", csm);
        DownloadAllMezzCardSetup();
        mezzCardSetupAll = 1;
        sampleAMTPhase = TRUE;
        sprintf(actFileName, "action/downloadAMTClockOn%04d.act", csm);
        ELMBStorageNumber = 1;
        OpenActionFile(csm);
//Modified by Xiangting		
//        WriteCommentToActionFile("Download AMT Setup (clock on) for all mezzanine cards");


 //       DownloadAMTSetup();
        CloseActionFile();

        sampleAMTPhase = FALSE;
        sprintf(actFileName, "action/downloadAMT%04d.act", csm);
        ELMBStorageNumber = 2;
        OpenActionFile(csm);
//Modified by Xiangting			
//        WriteCommentToActionFile("Download AMT Setup for all mezzanine cards");

//        DownloadAMTSetup();
        CloseActionFile();

        sprintf(actFileName, "action/downloadASD%04d.act", csm);
        ELMBStorageNumber = 3;
        OpenActionFile(csm);
//Modified by Xiangting	
//        WriteCommentToActionFile("Download ASD Setup for all mezzanine cards");
//        DownloadASDSetup();
        CloseActionFile();
      }

      ELMBStorageNumber = -1;
      waitTimeInMillionSecond = 0;
      // JTAG operation for InitDAQ, assuming all device on!
      //   1. Reset CSM
      //   2. Reset TTCrx, Download CSM & TTCrx and Enable Mezzanine Card JTAG
      //   3. Download AMTs (Clock on)
      //   4. Download ASDs
      //   5. Download CSM and GOL
      //   6. Sample AMT Phase
      //   7. Download AMTs (Clock as user setting)
      //   8. Download ASDs
      //   9. Get CSM Status
      OpenSequenceFile("action/INITDAQ.seq");
      step = 1;
      if (seqFile != NULL) {
        fprintf(seqFile, "# Perform DAQ initialization for CSM with following steps:\n");
        if (CSMOn) {
          fprintf(seqFile, "# %3d. Reset CSM (Need wait at least 0.800 second);\n", step++);
          fprintf(seqFile, "# %3d. Reset TTCrx, Download CSM & TTCrx and Enable Mezzanine Card JTAG;\n", step++);
          if (mezzCardsOn) {
            fprintf(seqFile, "# %3d. Download AMT (Clock On);\n", step++);
            fprintf(seqFile, "# %3d. Download ASD;\n", step++);
          }
          if (GOLOn) fprintf(seqFile, "# %3d. Download CSM and GOL;\n", step++);
          fprintf(seqFile, "# %3d. Sample AMT Phase;\n", step++);
        }
        if (mezzCardsOn) {
          fprintf(seqFile, "# %3d. Download AMT (Clock as user setting);\n", step++);
          fprintf(seqFile, "# %3d. Download ASD;\n", step++);
        }
        if (CSMOn) {
          fprintf(seqFile, "# %3d. Get CSM Status;\n", step++);
        }
      }
      if (CSMOn) {
        waitTimeInMillionSecond = 1000;
        WriteActionToSequenceFile("resetCSM");
        waitTimeInMillionSecond = 1;
        WriteActionToSequenceFile("downloadCSMTTCrx");
        waitTimeInMillionSecond = 0;
        if (mezzCardsOn) {
          WriteActionToSequenceFile("downloadAMTClockOn");
          WriteActionToSequenceFile("downloadASD");
        }
        if (GOLOn) WriteActionToSequenceFile("downloadCSMGOL");
        WriteActionToSequenceFile("sampleAMTPhase");
      }
      if (mezzCardsOn) {
        WriteActionToSequenceFile("downloadAMT");
        WriteActionToSequenceFile("downloadASD");
      }
      if (CSMOn) {
        WriteActionToSequenceFile("getCSMStatus");
      }
      CloseSequenceFile();

      // JTAG operation for Reset CSM Board!
      if (CSMOn) {
        OpenSequenceFile("action/resetCSMBoard.seq");
        if (seqFile != NULL) {
          fprintf(seqFile, "# Perform CSM Board Reset:\n");
          waitTimeInMillionSecond = 1000;
          WriteActionToSequenceFile("resetCSM");
          waitTimeInMillionSecond = 0;
        }
      }
      CloseSequenceFile();

      // JTAG operation Sample AMT Phase with following steps!
      //   1. Download AMTs (Clock on)
      //   2. Download ASDs
      //   3. Sample AMT Phase
      //   4. Download AMTs (Clock as user setting)
      //   5. Download ASDs
      //   6. Get CSM Status
      OpenSequenceFile("action/sampleAMTPhase.seq");
      step = 1;
      if (seqFile != NULL) {
        fprintf(seqFile, "# Perform Sample AMT Phase with following steps:\n");
        if (CSMOn) {
          if (mezzCardsOn) {
            fprintf(seqFile, "# %3d. Download AMT (Clock On);\n", step++);
            fprintf(seqFile, "# %3d. Download ASD;\n", step++);
          }
          fprintf(seqFile, "# %3d. Sample AMT Phase;\n", step++);
        }
        if (mezzCardsOn) {
          fprintf(seqFile, "# %3d. Download AMT (Clock as user setting);\n", step++);
          fprintf(seqFile, "# %3d. Download ASD;\n", step++);
        }
        if (CSMOn) {
          fprintf(seqFile, "# %3d. Get CSM Status;\n", step++);
        }
      }
      if (CSMOn) {
        if (mezzCardsOn) {
          WriteActionToSequenceFile("downloadAMTClockOn");
          WriteActionToSequenceFile("downloadASD");
        }
        WriteActionToSequenceFile("sampleAMTPhase");
      }
      if (mezzCardsOn) {
        WriteActionToSequenceFile("downloadAMT");
        WriteActionToSequenceFile("downloadASD");
      }
      if (CSMOn) {
        WriteActionToSequenceFile("getCSMStatus");
      }
      CloseSequenceFile();

      // JTAG operation Start CSM DAQ with following steps!
      //   1. Turn Off Mezzanine Card JTAG Signals
      //   2. Start CSM DAQ
      //   3. Get CSM Status
      step = 1;
      if (CSMOn) {
        OpenSequenceFile("action/startCSMDAQ.seq");
        if (seqFile != NULL) {
          fprintf(seqFile, "# Perform start CSM DAQ with following steps:\n");
          fprintf(seqFile, "# %3d. Turn Off Mezzanine Card JTAG Signals;\n", step++);
          fprintf(seqFile, "# %3d. Start CSM DAQ;\n", step++);
          fprintf(seqFile, "# %3d. Get CSM Status;\n", step++);
        }
        WriteActionToSequenceFile("turnOffMezzJTAG");
        WriteActionToSequenceFile("startCSMDAQMezzJTAGOff");
        WriteActionToSequenceFile("getCSMStatusInRunMezzJTAGOff");
      }
      CloseSequenceFile();

      // JTAG operation Stop CSM DAQ with following steps!
      //   1. Turn Off Mezzanine Card JTAG Signals
      //   2. Stop CSM DAQ
      //   3. Get CSM Status
      step = 1;
      if (CSMOn) {
        OpenSequenceFile("action/stopCSMDAQ.seq");
        if (seqFile != NULL) {
          fprintf(seqFile, "# Perform stop CSM DAQ with following steps:\n");
          fprintf(seqFile, "# %3d. Turn Off Mezzanine Card JTAG Signals;\n", step++);
          fprintf(seqFile, "# %3d. Stop CSM DAQ;\n", step++);
          fprintf(seqFile, "# %3d. Get CSM Status;\n", step++);
        }
        WriteActionToSequenceFile("turnOffMezzJTAG");
        WriteActionToSequenceFile("stopCSMDAQMezzJTAGOff");
        WriteActionToSequenceFile("getCSMStatusMezzJTAGOff");
      }
      CloseSequenceFile();

      printf("All DAQ action and sequence files are generated for CSM %d.\n", csm);
    }
    if (oldNormalJTAG != 0) {
      action = DOWNLOAD;
      normalJTAG = oldNormalJTAG;
    }
    waitTimeInMillionSecond = 0;
    zeroAllMaskWord = FALSE;
    ELMBStorageNumber = -1;
  }
  else if (controlOption == HELP) {
    MessagePopup("Generate All Action/Sequence Files For DAQ", 
                 "Generate all necessary action and sequence files for DAQ.");
    controlOption = NORM;
  }
}


int PanelIndexInFile(int panelID, int setupType, int mezzNumber, int ASDNumber)
{
  int index = 9999;

  if (panelID == JTAGControlHandle)        index = 0;
  else if (panelID == CSMIOsHandle)        index = 1;
  else if (panelID == CSMSetupHandle)      index = 2;
  else if (panelID == TTCrxSetupHandle)    index = 3;
  else if (panelID == GOLSetupHandle)      index = 4;
  else if (panelID == IndMezzEnableHandle) index = 5;
  else if (setupType) {                  // Setup all
    if (panelID == AMTSetupHandle)      index = 6;
    else if (panelID == AMTErrorHandle) index = 7;
    else if (panelID == ASDSetupHandle) {
      if ((ASDNumber <=0) || (ASDNumber > MAXNUMBERASD))
        printf("Invalid ASD number %d\n", ASDNumber);
      else index = 10 + (ASDNumber - 1);
    }
  }
  else {
    if ((mezzNumber < 0) || (mezzNumber >= MAXNUMBERMEZZANINE))
      printf("Invalid Mezzanie number %d\n", mezzNumber);
    else if (panelID == AMTSetupHandle) index = 10 + MAXNUMBERASD + mezzNumber;
    else if (panelID == AMTErrorHandle) index = 10 + MAXNUMBERASD + MAXNUMBERMEZZANINE + mezzNumber;
    else if (panelID == ASDSetupHandle) {
      if ((ASDNumber <= 0) || (ASDNumber > MAXNUMBERASD))
        printf("Invalid ASD number %d\n", ASDNumber);
      else index = 10 + 2*MAXNUMBERMEZZANINE + (mezzNumber + 1)*MAXNUMBERASD + (ASDNumber - 1);
    }
  }

  return index;
}


void SaveAllPanelState(char *fileName, int saveLevel) {
  int index, mezz, ASDNumber, on;
  
  printf("Saving Panel States to file <%s>.\n", fileName);
  index = PanelIndexInFile(JTAGControlHandle, 1, 0, 0);
  SavePanelState(JTAGControlHandle, fileName, index);
  index = PanelIndexInFile(CSMIOsHandle, 1, 0, 0);
  SavePanelState(CSMIOsHandle, fileName, index);
  index = PanelIndexInFile(CSMSetupHandle, 1, 0, 0);
  SavePanelState(CSMSetupHandle, fileName, index);
  index = PanelIndexInFile(TTCrxSetupHandle, 1, 0, 0);
  SavePanelState(TTCrxSetupHandle, fileName, index);
  index = PanelIndexInFile(GOLSetupHandle, 1, 0, 0);
  SavePanelState(GOLSetupHandle, fileName, index);
  index = PanelIndexInFile(IndMezzEnableHandle, 1, 0, 0);
  SavePanelState(IndMezzEnableHandle, fileName, index);
  printf("Save Mezzanine Card Common Setup Panel State.\n");
  index = PanelIndexInFile(AMTSetupHandle, 1, 0, 0);
  if (RecallPanelState(AMTSetupHandle, currSetupFile, index) != 0) DefaultPanel(AMTSetupHandle);
  SavePanelState(AMTSetupHandle, fileName, index);
  index = PanelIndexInFile(AMTErrorHandle, 1, 0, 0);
  if (RecallPanelState(AMTErrorHandle, currSetupFile, index) != 0) DefaultPanel(AMTErrorHandle);
  SavePanelState(AMTErrorHandle, fileName, index);
  for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
    index = PanelIndexInFile(ASDSetupHandle, 1, 0, ASDNumber);
    if (RecallPanelState(ASDSetupHandle, currSetupFile, index) != 0) DefaultPanel(ASDSetupHandle);
    SavePanelState(ASDSetupHandle, fileName, index);
  }
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    GetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezz], &on);
    if (on == 1) {
      printf("Save Mezzanine Card %d Setup Panel State.\n", mezz);
      index = PanelIndexInFile(AMTSetupHandle, 0, mezz, 0);
      if (RecallPanelState(AMTSetupHandle, currSetupFile, index) != 0) DefaultPanel(AMTSetupHandle);
      SavePanelState(AMTSetupHandle, fileName, index);
      index = PanelIndexInFile(AMTErrorHandle, 0, mezz, 0);
      if (RecallPanelState(AMTErrorHandle, currSetupFile, index) != 0) DefaultPanel(AMTErrorHandle);
      SavePanelState(AMTErrorHandle, fileName, index);
      for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
        index = PanelIndexInFile(ASDSetupHandle, 0, mezz, ASDNumber);
        if (RecallPanelState(ASDSetupHandle, currSetupFile, index) != 0) DefaultPanel(ASDSetupHandle);
        SavePanelState(ASDSetupHandle, fileName, index);
      }
    }
    else if (saveLevel > 0) {  // Using default if mezzanine card not selected
      printf("Save Mezzanine Card %d Setup Panel State.\n", mezz);
      index = PanelIndexInFile(AMTSetupHandle, 0, mezz, 0);
      DefaultPanel(AMTSetupHandle);
      SavePanelState(AMTSetupHandle, fileName, index);
      index = PanelIndexInFile(AMTErrorHandle, 0, mezz, 0);
      DefaultPanel(AMTErrorHandle);
      SavePanelState(AMTErrorHandle, fileName, index);
      for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
        index = PanelIndexInFile(ASDSetupHandle, 0, mezz, ASDNumber);
        DefaultPanel(ASDSetupHandle);
        SavePanelState(ASDSetupHandle, fileName, index);
      }
    }
  }
  printf("Panel States have been saved to file <%s>.\n", fileName);
}


void RecallAllPanelState(char *fileName) {
  int index, mezz, ASDNumber, on;
  
  printf("Recall Panel States from file <%s>.\n", fileName);
  index = PanelIndexInFile(JTAGControlHandle, 1, 0, 0);
  RecallPanelState(JTAGControlHandle, fileName, index);
  SavePanelState(JTAGControlHandle, currSetupFile, index);
  UpdateTCPStatus(FALSE);
  index = PanelIndexInFile(CSMIOsHandle, 1, 0, 0);
  RecallPanelState(CSMIOsHandle, fileName, index);
  SavePanelState(CSMIOsHandle, currSetupFile, index);
  index = PanelIndexInFile(CSMSetupHandle, 1, 0, 0);
  RecallPanelState(CSMSetupHandle, fileName, index);
  SavePanelState(CSMSetupHandle, currSetupFile, index);
  index = PanelIndexInFile(TTCrxSetupHandle, 1, 0, 0);
  RecallPanelState(TTCrxSetupHandle, fileName, index);
  SavePanelState(TTCrxSetupHandle, currSetupFile, index);
  index = PanelIndexInFile(GOLSetupHandle, 1, 0, 0);
  RecallPanelState(GOLSetupHandle, fileName, index);
  SavePanelState(GOLSetupHandle, currSetupFile, index);
  index = PanelIndexInFile(IndMezzEnableHandle, 1, 0, 0);
  RecallPanelState(IndMezzEnableHandle, fileName, index);
  SavePanelState(IndMezzEnableHandle, currSetupFile, index);
  printf("Recall Mezzanine Card Common Setup Panel State.\n");
  index = PanelIndexInFile(AMTSetupHandle, 1, 0, 0);
  RecallPanelState(AMTSetupHandle, fileName, index);
  SavePanelState(AMTSetupHandle, currSetupFile, index);
  index = PanelIndexInFile(AMTErrorHandle, 1, 0, 0);
  RecallPanelState(AMTErrorHandle, fileName, index);
  SavePanelState(AMTErrorHandle, currSetupFile, index);
  for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
    index = PanelIndexInFile(ASDSetupHandle, 1, 0, ASDNumber);
    RecallPanelState(ASDSetupHandle, fileName, index);
    SavePanelState(ASDSetupHandle, currSetupFile, index);
  }
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    GetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezz], &on);
    if (on == 1) {  // Only recall the settings if mezz. card is selected
      printf("Recall Mezzanine Card %d Setup Panel State.\n", mezz);
      index = PanelIndexInFile(AMTSetupHandle, 0, mezz, 0);
      RecallPanelState(AMTSetupHandle, fileName, index);
      SavePanelState(AMTSetupHandle, currSetupFile, index);
      index = PanelIndexInFile(AMTErrorHandle, 0, mezz, 0);
      RecallPanelState(AMTErrorHandle, fileName, index);
      SavePanelState(AMTErrorHandle, currSetupFile, index);
      for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
        index = PanelIndexInFile(ASDSetupHandle, 0, mezz, ASDNumber);
        RecallPanelState(ASDSetupHandle, fileName, index);
        SavePanelState(ASDSetupHandle, currSetupFile, index);
      }
    }
  }
  printf("Recall Panel States from file <%s> done.\n", fileName);
}


void DownloadSetup(void) {
  int panel;
  
  panel = GetActivePanel();
  if (panel == SelectActionHandle) PanelDone();
  UpdateCSMControlBits();
  GetJTAGChainOnList();
  if (CSMOn) DownloadTTCrxSetup();
  if (mezzCardsOn) DownloadAllMezzCardSetup();
  if ((!CSMOn) && (!GOLOn)) return;
  downloadSetup = TRUE;
  SaveSetupToSVFFile();
  downloadSetup = FALSE;
  UpdateJTAGControlPanel();     // Update JTAG control panel
}


void SaveSetupToSVFFile(void) {
  char path[5192];
  int i, saveSetup, nError;
  FILE *SVFFile;

  if (controlOption == NORM) {
    if (downloadSetup) {
      strcpy(path, "currentSetup.svf");
      saveSetup = TRUE;
    }
    else
      saveSetup = FileSelectPopup("", "*.svf", "", "Save current setup to SVF file",
                                  VAL_SAVE_BUTTON, 0, 1, 1, 0, path);
    if (saveSetup) {
      if (SVFFile = fopen(path, "w")) {
        LoadCSMSetupArray();
        LoadGOLSetupArray();
        LoadTTCrxSetupArray();
        GetJTAGChainOnList();
        AllJTAGDeviceInBYPASS(SVFFile, 0);
        instrLength = 0;
        if (mezzCardsOn) {
          for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
            if (CSMSetupArray[TDCENABLES+i] == 1) {
				if(i==NEWTDC_NUMBER){
				  IntToBinary(TDC_BYPASS, instrLength, TDC_INSTR_LENGTH, instrArray, JTAGINSTRLENGTH);
	              IntToBinary(TDC_BYPASS, instrLength, TDC_INSTR_LENGTH, secondInstrArray, JTAGINSTRLENGTH);
	              instrLength += TDC_INSTR_LENGTH;
				}
				else{
//Modified by Xiangting
	              IntToBinary(HPTDCBYPASS, instrLength, HPTDCINSTLENGTH, instrArray, JTAGINSTRLENGTH);
	              IntToBinary(HPTDCBYPASS, instrLength, HPTDCINSTLENGTH, secondInstrArray, JTAGINSTRLENGTH);
	              instrLength += HPTDCINSTLENGTH;
	              HPTDCJTAGComments(HPTDCBYPASS, SVFFile, i);
	              WriteJTAGInstructionToActionFile(HPTDCID, HPTDCBYPASS, i);
	              IntToBinary(A3P250BYPASS, instrLength, A3P250INSTLENGTH, instrArray, JTAGINSTRLENGTH);
	              IntToBinary(A3P250BYPASS, instrLength, A3P250INSTLENGTH, secondInstrArray, JTAGINSTRLENGTH);
	              instrLength += A3P250INSTLENGTH;
	              A3P250JTAGComments(A3P250BYPASS, SVFFile, i);
	              WriteJTAGInstructionToActionFile(A3P250ID, A3P250BYPASS, i);	
				}
			/*				
              IntToBinary(AMTBYPASS, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
              IntToBinary(AMTBYPASS, instrLength, AMTINSTLENGTH, secondInstrArray, JTAGINSTRLENGTH);
              instrLength += AMTINSTLENGTH;
              AMTJTAGComments(AMTBYPASS, SVFFile, i);
              WriteJTAGInstructionToActionFile(AMT3ID, AMTBYPASS, i);
			 */
            }
          }
        }
//End
        if (CSMOn) {
          IntToBinary(CSMFULLRW, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          IntToBinary(CSMFULLRO, instrLength, CSMINSTLENGTH, secondInstrArray, JTAGINSTRLENGTH);
          instrLength += CSMINSTLENGTH;
          CSMJTAGComments(CSMFULLRW, SVFFile);		  
          WriteJTAGInstructionToActionFile(CSMID, CSMFULLRW, 0);
        }
        if (TTCrxOn) {
          IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, secondInstrArray, JTAGINSTRLENGTH);
          instrLength += TTCRXINSTLENGTH;
          TTCrxJTAGComments(TTCRXBYPASS, SVFFile);		  
          WriteJTAGInstructionToActionFile(TTCRXID, TTCRXBYPASS, 0);
        }
        if (GOLOn) {
          IntToBinary(GOLCONFRW, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          IntToBinary(GOLCONFRO, instrLength, GOLINSTLENGTH, secondInstrArray, JTAGINSTRLENGTH);
          instrLength += GOLINSTLENGTH;
          GOLJTAGComments(GOLCONFRW, SVFFile);		  
          WriteJTAGInstructionToActionFile(GOLID, GOLCONFRW, 0);
        }
        if (AX1000On) {
          IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
          IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, secondInstrArray, JTAGINSTRLENGTH);
          instrLength += AX1000INSTLENGTH;
          AX1000JTAGComments(AX1000BYPASS, SVFFile);		  
        }
        if (XC2V1000On || XC2V2000On) {
          IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, secondInstrArray, JTAGINSTRLENGTH);
          instrLength += VERTEXIIINSTLENGTH;
          VertexIIJTAGComments(VERTEXIIBYPASS, SVFFile);		  
          if (XC2V2000On) WriteJTAGInstructionToActionFile(XC2V2000ID, VERTEXIIBYPASS, 0);
          else WriteJTAGInstructionToActionFile(XC2V1000ID, VERTEXIIBYPASS, 0);
        }
        if (PROMOn) {
          IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, secondInstrArray, JTAGINSTRLENGTH);
          instrLength += PROMINSTLENGTH;
          PROMJTAGComments(PROMBYPASS, SVFFile);		  
          WriteJTAGInstructionToActionFile(PROMID, PROMBYPASS, 0);
        }
        WriteJTAGInstructions(SVFFile);
        dataLength = 0;
        if (mezzCardsOn) {
          for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
            if (CSMSetupArray[TDCENABLES+i] == 1) {
				if(i==NEWTDC_NUMBER){
				  HPTDCOffset[i] = dataLength;
	              dataArray[dataLength] = 1;
	              maskArray[dataLength++] = 0;
				}
				else{
					
	//Modified by Xiangting 				
	              HPTDCOffset[i] = dataLength;
	              dataArray[dataLength] = 1;
	              maskArray[dataLength++] = 0;
	              WriteJTAGDataToActionFile(1, HPTDCOffset[i]);
	              A3P250Offset[i] = dataLength;
	              dataArray[dataLength] = 1;
	              maskArray[dataLength++] = 0;
	              WriteJTAGDataToActionFile(1, A3P250Offset[i]);
			  
	//              mezzOffset[i] = dataLength;
	//              dataArray[dataLength] = 1;
	//              maskArray[dataLength++] = 0;
	//              WriteJTAGDataToActionFile(1, mezzOffset[i]);
				}
            }
          }
        }
        if (CSMOn) {
          CSMOffset = dataLength;
          for (i = dataLength; i < dataLength+CSMSTATUSLENGTH; i++) {
            if (i == dataLength) {
              dataArray[i] = 1;
              maskArray[i] = 0;
            }
            else if (i < dataLength+12) {
              dataArray[i] = 0;
              maskArray[i] = 0;
            }
            else {
              dataArray[i] = 0;
              maskArray[i] = 0;
            }
          }
          dataLength += CSMSTATUSLENGTH;
          for (i = dataLength; i < dataLength+CSMSETUPLENGTH; i++) {
            dataArray[i] = CSMSetupArray[i-dataLength];
            maskArray[i] = 1;
            downloadedCSMSetupArray[i-dataLength] = dataArray[i];
          }
          dataLength += CSMSETUPLENGTH;
          for (i = dataLength; i < dataLength+TTCRXSETUPLENGTH; i++) {
            dataArray[i] = TTCrxSetupArray[i-dataLength];
            maskArray[i] = 1;
          }
          dataLength += TTCRXSETUPLENGTH;
          WriteJTAGDataToActionFile(TTCRXSETUPLENGTH+CSMSETUPLENGTH+CSMSTATUSLENGTH, CSMOffset);
        }
        if (TTCrxOn) {
          TTCrxOffset = dataLength;
          dataArray[dataLength] = 1;
          maskArray[dataLength++] = 0;
          WriteJTAGDataToActionFile(1, TTCrxOffset);
        }
        if (GOLOn) {
          GOLOffset = dataLength;
          for (i = dataLength; i < dataLength+GOLSETUPLENGTH; i++) {
            dataArray[i] = GOLSetupArray[i-dataLength];
            if (i < dataLength+WAITTIMEFORREADY) maskArray[i] = 0;
            else if (i < dataLength+HAMMINGCHECKSUM) maskArray[i] = 1;
            else maskArray[i] = 0;
          }
          dataLength += GOLSETUPLENGTH;
          WriteJTAGDataToActionFile(GOLSETUPLENGTH, GOLOffset);
        }
        if (AX1000On) {
          AX1000Offset = dataLength;
          dataArray[dataLength] = 1;
          maskArray[dataLength++] = 0;
          WriteJTAGDataToActionFile(1, AX1000Offset);
        }
        if (XC2V1000On || XC2V2000On) {
          vertexIIOffset = dataLength;
          dataArray[dataLength] = 1;
          maskArray[dataLength++] = 0;
          WriteJTAGDataToActionFile(1, vertexIIOffset);
        }
        if (PROMOn) {
          PROMOffset = dataLength;
          dataArray[dataLength] = 1;
          maskArray[dataLength++] = 0;
          WriteJTAGDataToActionFile(1, PROMOffset);
        }
        WriteJTAGData(SVFFile);
        if (downloadSetup && (action == DOWNLOAD)) {
          JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
          JTAGScanAllData(dataLength, dataArray, readbackArray);
          JTAGScanAllInstruction(instrLength, secondInstrArray, readbackArray);
          JTAGScanAllData(dataLength, dataArray, readbackArray);
          nError = CheckFullJTAGDataArray();
		  
		  printf("nError = CheckFullJTAGDataArray();\n");
          if (CSMOn && GOLOn) {
            downloadCSMSetupDone = TRUE;
            if ((downloadCSMStatus == 0) && (downloadGOLStatus == 0)) {
              validGOLStatus = TRUE;
              for (i = 0; i < WAITTIMEFORREADY; i++) GOLSetupArray[i] = readbackArray[GOLOffset+i];
              for (i = HAMMINGCHECKSUM; i < GOLSETUPLENGTH; i++) GOLSetupArray[i] = readbackArray[GOLOffset+i];
              GetGOLConfigure();
              printf("Current setup for GOL/CSM is downloaded successfully through JTAG.\n");
            }
            else
              printf("Failed in downloading current setup for GOL/CSM through JTAG, downloadGOLStatus = %d, downloadCSMStatus = %d\n", downloadGOLStatus, downloadCSMStatus);
            GetCSMStatus();
			printf("GetCSMStatus() is in void SaveSetupToSVFFile()\n");
          }
          else if (CSMOn) {
            downloadCSMSetupDone = TRUE;
            if (downloadCSMStatus == 0) {
              for (i = 0; i < CSMSTATUSLENGTH; i++) CSMStatusArray[i] = readbackArray[CSMOffset+i];
              printf("Current setup for CSM is downloaded successfully through JTAG.\n");
            }
            else
              printf("Failed in downloading current setup for CSM through JTAG, downloadCSMStatus = %d\n", downloadCSMStatus);
            GetCSMStatus();
          }
          else if (GOLOn) {
            if (downloadGOLStatus == 0) {
              validGOLStatus = TRUE;
              for (i = 0; i < WAITTIMEFORREADY; i++) GOLSetupArray[i] = readbackArray[GOLOffset+i];
              for (i = HAMMINGCHECKSUM; i < GOLSETUPLENGTH; i++) GOLSetupArray[i] = readbackArray[GOLOffset+i];
               GetGOLConfigure();
               printf("Current setup for GOL is downloaded successfully through JTAG.\n");
            }
            else
              printf("Failed in downloading current setup for GOL through JTAG, downloadGOLStatus = %d\n", downloadGOLStatus);
            GetGOLConfigure();
            GetGOLStatus();
          }
          if (nError == 0)
            printf("Current setup is downloaded successfully through JTAG.\n");
          else
            printf("Failed in downloading current setup through JTAG.\n");
          if (downloadCSMSetupDone) CSMLastNextState = CSMNextState;
        }
        AllJTAGDeviceInBYPASS(SVFFile, 1);
        fclose(SVFFile);
        if (!downloadSetup) {
          printf("Current setup has been saved into SVF file <%s>!\n", path);
          MessagePopup("Setup Saved", "Current setup has been saved into Xilinx SVF file.");
        }
      }
      else {
        downloadSetup = FALSE;
        printf("Unable to open SVF file <%s>!\n", path);
        MessagePopup("Failed to Open SVF File",
                     "Unable to open SVF file, please check your disk and filename!");
      }
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Save Setup to SVF file", 
                 "Save current setups to Xilinx SVF file.");
    controlOption = NORM;
  }
}


void JTAGChainPROM(void) {
  int enable;

  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, &enable);
  if (controlOption == NORM) {
    if (enable) SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETPROMID, ATTR_DIMMED, FALSE);
    else SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETPROMID, ATTR_DIMMED, TRUE);
    SaveParameters(PARAALL);
    GetJTAGChainOnList();
  }
  else if (controlOption == HELP) {
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, ~enable&1);
    MessagePopup("Help on PROM Button",
                 "Include or exclude PROM in JTAG chain.\n\n"
                 "========== Please Pay Attention ==========\n"
                 "It is user responsibility to setup JTAG chain according to the hardware,\n"
                 "otherwise the JTAG will not be functional properly.");
  }
  controlOption = NORM;
}


void JTAGChainCSMChip(void) {
  int enable;

  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, &enable);
  if (controlOption == NORM) {
    if (enable) SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, ATTR_DIMMED, FALSE);
    else SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, ATTR_DIMMED, TRUE);
    SaveParameters(PARAALL);
    GetJTAGChainOnList();
  }
  else if (controlOption == HELP) {
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, ~enable&1);
    if (CSMChipID == XC2V1000ID) {
      MessagePopup("Help on XC2V1000 Button",
                   "Include or exclude XC2V1000 in JTAG chain.\n\n"
                   "========== Please Pay Attention ==========\n"
                   "It is user responsibility to setup JTAG chain according to the hardware,\n"
                   "otherwise the JTAG will not be functional properly.");
    }
    else if (CSMChipID == XC2V2000ID) {
      MessagePopup("Help on XC2V2000 Button",
                   "Include or exclude XC2V2000 in JTAG chain.\n\n"
                   "========== Please Pay Attention ==========\n"
                   "It is user responsibility to setup JTAG chain according to the hardware,\n"
                   "otherwise the JTAG will not be functional properly.");
    }
    else if (CSMChipID == AX1000ID) {
      MessagePopup("Help on AX1000 Button",
                   "Include or exclude AX1000 in JTAG chain.\n\n"
                   "========== Please Pay Attention ==========\n"
                   "It is user responsibility to setup JTAG chain according to the hardware,\n"
                   "otherwise the JTAG will not be functional properly.");
    }
    else {
      MessagePopup("Help on CSMChip Button",
                   "Include or exclude CSMChip (XC2V1000/AX1000) in JTAG chain.\n\n"
                   "========== Please Pay Attention ==========\n"
                   "It is user responsibility to setup JTAG chain according to the hardware,\n"
                   "otherwise the JTAG will not be functional properly.");
    }
  }
  controlOption = NORM;
}


void JTAGChainGOLButton(void) {
  int enable;
  
  if (controlOption == NORM) {
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, &enable);
    SetCtrlVal(CSMSetupHandle, P_CSM_GOLTDI, enable);
    PanelSave(CSMSetupHandle);
    if (CSMOn) TurnOnOffGOLAndTTCrxFromJTAGChain();
  }
  JTAGChainGOL();
}


void JTAGChainGOL(void) {
  static int enable, oldEnable = -1, dimmed, colorCode;
  char str[10];

  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, &enable);
  if (controlOption == NORM) {
    if (enable) dimmed = FALSE;
    else dimmed = TRUE;
    if ((!enable) && (GOLSetupStatus() == -1)) {
      sprintf(str, "Not Apply");
      colorCode = VAL_MAGENTA;
    }
    else if (GOLSetupStatus() == -1) {
      sprintf(str, "Unknown");
      colorCode = VAL_MAGENTA;
    }
    else if (GOLSetupStatus() == 0) {
      sprintf(str, "Success");
      colorCode = VAL_GREEN;
    }
    else {
      sprintf(str, "Failed");
      colorCode = VAL_RED;
    }
    ReplaceTextBoxLine(JTAGControlHandle, P_JTAGCTRL_GOLSETUPSTATUS, 0, str);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_GOLSETUPSTATUS, ATTR_TEXT_BGCOLOR, colorCode);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_SETUPGOL, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETGOLID, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLCONTROL, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(TTCrxSetupHandle, P_TTCRX_GOLCONTROL, ATTR_DIMMED, dimmed);
    GetJTAGChainOnList();
    if (enable != oldEnable) {
      SaveParameters(PARAALL);
      oldEnable = enable;
    }
  }
  else if (controlOption == HELP) {
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, ~enable&1);
    MessagePopup("Help on GOL Button",
                 "Include or exclude GOL in JTAG chain.\n\n"
                 "========== Please Pay Attention ==========\n"
                 "It is user responsibility to setup JTAG chain according to the hardware,\n"
                 "otherwise the JTAG will not be functional properly.");
  }
  controlOption = NORM;
}


void JTAGChainTTCrxButton(void) {
  int enable;
  
  if (controlOption == NORM) {
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, &enable);
    SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXTDI, enable);
    PanelSave(CSMSetupHandle);
    if (CSMOn) TurnOnOffGOLAndTTCrxFromJTAGChain();
  }
  JTAGChainTTCrx();
}


void JTAGChainTTCrx(void) {
  static int enable, oldEnable = -1, dimmed, colorCode;
  char str[10];

  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, &enable);
  if (controlOption == NORM) {
    if (enable) dimmed = FALSE;
    else dimmed = TRUE;
    if ((!enable) && (TTCrxSetupStatus() == -1)) {
      sprintf(str, "Not Apply");
      colorCode = VAL_MAGENTA;
    }
    else if (TTCrxSetupStatus() == -1) {
      sprintf(str, "Unknown");
      colorCode = VAL_MAGENTA;
    }
    else if (TTCrxSetupStatus() == 0) {
      sprintf(str, "Success");
      colorCode = VAL_GREEN;
    }
    else {
      sprintf(str, "Failed");
      colorCode = VAL_RED;
    }
    ReplaceTextBoxLine(JTAGControlHandle, P_JTAGCTRL_TTCRXSETUPSTATUS, 0, str);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_TTCRXSETUPSTATUS, ATTR_TEXT_BGCOLOR, colorCode);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_RESETCSM, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_SETUPCSM, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_SETUPTTCRX, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMID, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(GOLSetupHandle, P_GOL_CSMCONTROL, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(GOLSetupHandle, P_GOL_TTCRXCONTROL, ATTR_DIMMED, dimmed);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, &enable);
    if (enable) SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETTTCRXID, ATTR_DIMMED, FALSE);
    else SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETTTCRXID, ATTR_DIMMED, TRUE);
    GetJTAGChainOnList();
    if (enable != oldEnable) {
      SaveParameters(PARAALL);
      oldEnable = enable;
    }
  }
  else if (controlOption == HELP) {
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, &enable);
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, ~enable&1);
    MessagePopup("Help on TTCrx Button",
                 "Include or exclude TTCrx in JTAG chain.\n"
                 "But to setup TTCrx, VertexII (CSM) must be in JTAG Chain!\n\n"
                 "========== Please Pay Attention ==========\n"
                 "It is user responsibility to setup JTAG chain according to the hardware,\n"
                 "otherwise the JTAG will not be functional properly.");
  }
  controlOption = NORM;
}


void JTAGChainCSM(void) {
  static int enable, oldEnable = -1, dimmed, colorCode, checkMode;
  char str[10];

  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, &enable);
  GetCtrlVal(ConfirmVFPHandle, P_SELVFP_CHECKMODE, &checkMode);
  if (verifyFPGAAndPROM && (checkMode < 2)) {
    if (checkMode < 1) {
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_DIMMED, FALSE);
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Check JTAG & CSM");
    }
    else {
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_DIMMED, TRUE);
    }
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_RESETCSMCHIP, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_RESETCSMCHIP, ATTR_LABEL_TEXT, "Verify PROM");
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_SAMPLEAMTPHASE, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_SAMPLEAMTPHASE, ATTR_LABEL_TEXT, "Verify FPGA");
  }
  else if (verifyFPGAAndPROM) {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_DIMMED, TRUE);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_RESETCSMCHIP, ATTR_DIMMED, TRUE);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_SAMPLEAMTPHASE, ATTR_DIMMED, TRUE);
  }
  else {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_RESETCSMCHIP, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_RESETCSMCHIP, ATTR_LABEL_TEXT, "Reset CSM Chip");
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_SAMPLEAMTPHASE, ATTR_LABEL_TEXT, "Sample AMT Phase");
  }
  if (controlOption == NORM) {
    if (enable) dimmed = FALSE;
    else dimmed = TRUE;
    if ((!enable) && (CSMSetupStatus() == -1)) {
      sprintf(str, "Not Apply");
      colorCode = VAL_MAGENTA;
    }
    else if (CSMSetupStatus() == -1) {
      sprintf(str, "Unknown");
      colorCode = VAL_MAGENTA;
    }
    else if (CSMSetupStatus() == 0) {
      sprintf(str, "Success");
      colorCode = VAL_GREEN;
    }
    else {
      sprintf(str, "Failed");
      colorCode = VAL_RED;
    }
    ReplaceTextBoxLine(JTAGControlHandle, P_JTAGCTRL_CSMSETUPSTATUS, 0, str);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMSETUPSTATUS, ATTR_TEXT_BGCOLOR, colorCode);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_RESETCSM, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_SETUPCSM, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_SETUPTTCRX, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMID, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(GOLSetupHandle, P_GOL_CSMCONTROL, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(GOLSetupHandle, P_GOL_TTCRXCONTROL, ATTR_DIMMED, dimmed);
    JTAGChainTTCrx();
    JTAGChainMezzCards();
    GetJTAGChainOnList();
    StartOrStopCSMDAQButton();
    if (enable != oldEnable) {
      SaveParameters(PARAALL);
      oldEnable = enable;
    }
  }
  else if (controlOption == HELP) {
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, ~enable&1);
    MessagePopup("Help on CSM Button",
                 "Include or exclude CSM in JTAG chain.\n\n"
                 "========== Please Pay Attention ==========\n"
                 "It is user responsibility to setup JTAG chain according to the hardware,\n"
                 "otherwise the JTAG will not be functional properly.");
  }
  controlOption = NORM;
}


void JTAGChainMezzCardsButton(void) {
  int enable, version, resp = TRUE;
  
  if (controlOption == NORM) {
    BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, &enable);
    if ((version < 10) && enable) {
      resp = ConfirmPopup("Confirm to Include Mezzanine Card(s)",
                          "Mezzanine card(s) should not included in JTAG chain for CSM version < 10.\n"
                          "Do you still want to include mezzanine card(s) in JTAG chain?");
      if (resp <= 0) {
        enable = 0;
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, enable);
      }
    }
    else if ((nbMezzCard <= 0) && enable) {
      CheckNumberMezzCard(0);
      if (nbMezzCard <= 0) enable = 0;
    }
    SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, enable);
    PanelSave(CSMSetupHandle);
    if (resp && CSMOn) DownloadCSMSetup();
    else LoadCSMSetupArray();
  }
  JTAGChainMezzCards();
}


void JTAGChainMezzCards(void) {
  static int first = TRUE, enable, oldEnable = -1, dimmed, colorCode, mezz, nbad, ngood, i;
  char str[40], *strp;

  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, &mezzCardsOn);
  if (controlOption == NORM) {
    if ((!mezzCardsOn) && (!downloadMezzSetupDone)) {
      sprintf(str, "Not Apply");
      colorCode = VAL_MAGENTA;
    }
    else {
      nbad = 0;
      ngood = 0;
      for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
        if (MezzCardSetupStatus(mezz) > 0) nbad++;
        else if (MezzCardSetupStatus(mezz) == 0) ngood++;
      }
      if (nbMezzCard <= 0) {
        sprintf(str, "No MezzCard");
        colorCode = VAL_MAGENTA;
      }
      else if (!downloadMezzSetupDone) {
        sprintf(str, "Unknown");
        colorCode = VAL_MAGENTA;
      }
      else if ((nbad == 0) && (ngood == 0)) {
        sprintf(str, "Unknown");
        colorCode = VAL_MAGENTA;
      }
      else if ((nbad == 0) && (ngood > 0)) {
        sprintf(str, "Success (%d mezz)", ngood);
        colorCode = VAL_GREEN;
      }
      else {
        sprintf(str, "%d Failed", nbad);
        sprintf(str, "%d Failed, %2d OK", nbad, ngood);
        colorCode = VAL_RED;
      }
    }
    ReplaceTextBoxLine(JTAGControlHandle, P_JTAGCTRL_MEZZSETUPSTATUS, 0, str);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_MEZZSETUPSTATUS, ATTR_TEXT_BGCOLOR, colorCode);
    if (CSMSetupArray[MEZZJTAGENABLE] == 0) mezzCardsOn = FALSE;
    if (mezzCardsOn && (nbMezzCard > 0)) dimmed = FALSE;
    else dimmed = TRUE;
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_MEZZSETUPALL, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_NOMINALTHRESHOLD, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETALLAMTID, ATTR_DIMMED, dimmed);
    if (!downloadMezzSetupDone) dimmed = TRUE;
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_MEZZSETUPIND, ATTR_DIMMED, dimmed);
    if (downloadMezzSetupDone) dimmed = FALSE;
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, &enable);
    GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_SAMPLEAMTPHASE, ATTR_LABEL_TEXT, str);
    for (i = 0; i <= strlen(str); i++) str[i] = tolower(str[i]);
    strp = strstr(str, "verify");
    if ((strp == NULL) && ((enable == 0) || verifyFPGAAndPROM)) dimmed = TRUE;
    else dimmed = FALSE;
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_SAMPLEAMTPHASE, ATTR_DIMMED, dimmed);
    GetJTAGChainOnList();
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, &enable);
    if (enable != oldEnable) {
      SaveParameters(PARAALL);
      oldEnable = enable;
    }
  }
  else if (controlOption == HELP) {
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, ~mezzCardsOn&1);
    MessagePopup("Help on MezzCard Button",
                 "Include or exclude Mezzanine cards in JTAG chain.\n"
                 "========== Please Pay Attention ==========\n"
                 "It is user responsibility to setup JTAG chain according to the hardware,\n"
                 "otherwise the JTAG will not be functional properly.");
  }
  controlOption = NORM;
}


// Lock and Unlock JTAG Chain
void JTAGChainControl(void) {
  int dimmed;

  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGCHAINCONTROL, &unlockJTAGChain);
  if (controlOption == NORM) {
    if (unlockJTAGChain) dimmed = FALSE;
    else dimmed = TRUE;
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_PROM, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_GOL, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_TTCRX, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSM, ATTR_DIMMED, dimmed);
  }
  else if (controlOption == HELP) {
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGCHAINCONTROL, ~unlockJTAGChain&1);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGCHAINCONTROL, &unlockJTAGChain);
    if (unlockJTAGChain) {
      MessagePopup("Help on Lock JTAG Chain Button",
                   "Lock JTAG Include/exclude Control Buttons.\n");
    }
    else {
      MessagePopup("Help on Unlock JTAG Chain Button",
                   "Unlock JTAG Include/exclude Control Buttons.\n");
    }
  }
  controlOption = NORM;
}


// Clear Setup Status
void ClearSetupStatus(void) {
  int mezz;
  
  downloadCSMStatus = -1;
  downloadGOLStatus = -1;
  downloadTTCrxStatus = -1;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
//Modified by Xiangting
	downloadHPTDCStatus[mezz]=-1;
	downloadA3P250Status[mezz]=-1;
//	downloadAMTStatus[mezz] = -1;
//    downloadASDStatus[mezz] = -1;
  }
  UpdateJTAGControlPanel();     // Update JTAG control panel
  SaveParameters(PARAALL);
}


// Update JTAG Control Panel Information.
void UpdateJTAGControlPanelInfor(void) {
  static int nlist = 0, item, ctrlItem, ctrlVal = -1, val, time0, dtime, dimmed;
  
  ResetTextBox(JTAGControlHandle, P_JTAGCTRL_DATETIME, WeekdayTimeString(time(NULL)));
  GetNumListItems(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, &val);
  if (val != nlist) time0 = (int) time(NULL);
  nlist = val;
  if (nlist > 1) {
    GetCtrlIndex(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, &item);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, &val);
    if ((val != ctrlVal) || (item != ctrlItem)) {
      ctrlItem = item;
      ctrlVal = val;
      time0 = (int) time(NULL);
    }
    else {
      dtime = ((int) time(NULL)) - time0;
      if (dtime >= 30) {
        item++;
        if (item >= nlist) item = 0;
        GetValueFromIndex(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, item, &val);
        SetCtrlIndex(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, item);
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, val);
      }
    }
  }
  if (action == DOWNLOAD) dimmed = FALSE;
  else dimmed = TRUE;
  SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_SETUPJTAGCHAIN, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_RESETTAP, ATTR_DIMMED, dimmed);
}


void GetJTAGChainOnList(void) {
  int enable, dimmed, mezz;
  
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, &PROMOn);
  if (CSMChipID == XC2V1000ID) {
    XC2V2000On = FALSE;
    AX1000On = FALSE;
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, &XC2V1000On);
  }
  else if (CSMChipID == XC2V2000ID) {
    XC2V1000On = FALSE;
    AX1000On = FALSE;
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, &XC2V2000On);
  }
  else {
    XC2V1000On = FALSE;
    XC2V2000On = FALSE;
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, &AX1000On);
  }
  oldGOLOn = GOLOn;
  oldTTCrxOn = TTCrxOn;
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, &GOLOn);
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, &TTCrxOn);
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, &CSMOn);
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, &mezzCardsOn);
  if (CSMSetupArray[MEZZJTAGENABLE] == 0) mezzCardsOn = FALSE;
  enable = CSMOn | GOLOn | mezzCardsOn;
  if (enable == 0) dimmed = TRUE;
  else dimmed = FALSE;
  if (enable || PROMOn)
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETALLDEVICEID, ATTR_DIMMED, FALSE);
  else
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETALLDEVICEID, ATTR_DIMMED, TRUE);
  PROMOffset = -1;
  vertexIIOffset = -1;
  AX1000Offset = -1;
  GOLOffset = -1;
  TTCrxOffset = -1;
  CSMOffset = -1;
//Modified by Xiangting
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
	  HPTDCOffset[mezz] = -1;
	  A3P250Offset[mezz] = -1;
  }
}


//  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) mezzOffset[mezz] = -1;
//}


int CSMSetupStatus(void) {
  return downloadCSMStatus;
}


int GOLSetupStatus(void) {
  return downloadGOLStatus;
}


int TTCrxSetupStatus(void) {
  return downloadTTCrxStatus;
}


int MezzCardSetupStatus(int mezzCardNumber) {
  int status=0;
  if(mezzCardNumber==NEWTDC_NUMBER){
    if (downloadHPTDCStatus[mezzCardNumber] == -1) status = -1;
  }
  else{
    if (downloadHPTDCStatus[mezzCardNumber] == -1) status = -1;
    else if (downloadA3P250Status[mezzCardNumber] == -1) status = -1;
    else status = downloadHPTDCStatus[mezzCardNumber] + downloadA3P250Status[mezzCardNumber];    
  }
  return status;  
  
//Modified by Xiangting

//  if (downloadAMTStatus[mezzCardNumber] == -1) status = -1;
//  else if (downloadASDStatus[mezzCardNumber] == -1) status = -1;
//  else status = downloadAMTStatus[mezzCardNumber] + downloadASDStatus[mezzCardNumber];
//  return status;
}


void GetMezzCardEnables(void) {
  int mezz, oldMezzEnables;
  
  oldMezzEnables = mezzEnables;
  nbMezzCard = 0;
  mezzEnables = 0;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    nbMezzCard += CSMSetupArray[TDCENABLES+mezz];
    mezzEnables |= (CSMSetupArray[TDCENABLES+mezz] << mezz);
  }
  SaveParameters(PARAMEZZENABLES);
  if (mezzEnables != oldMezzEnables) {
    downloadMezzSetupDone = FALSE;
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      downloadHPTDCStatus[mezz] = -1;
      downloadA3P250Status[mezz] = -1;
    }
  }
  nbMezzCard = 0;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) if (((mezzEnables>>mezz)&1) == 1) nbMezzCard++;
}


void SetMezzCardEnables(void) {
  int mezz;
  
  mezzEnables = ReadParameters(PARAMEZZENABLES);
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) 
    CSMSetupArray[TDCENABLES+mezz] = (mezzEnables >> mezz) & 1;
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ00, CSMSetupArray[TDCENABLES+0]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ01, CSMSetupArray[TDCENABLES+1]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ02, CSMSetupArray[TDCENABLES+2]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ03, CSMSetupArray[TDCENABLES+3]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ04, CSMSetupArray[TDCENABLES+4]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ05, CSMSetupArray[TDCENABLES+5]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ06, CSMSetupArray[TDCENABLES+6]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ07, CSMSetupArray[TDCENABLES+7]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ08, CSMSetupArray[TDCENABLES+8]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ09, CSMSetupArray[TDCENABLES+9]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ10, CSMSetupArray[TDCENABLES+10]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ11, CSMSetupArray[TDCENABLES+11]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ12, CSMSetupArray[TDCENABLES+12]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ13, CSMSetupArray[TDCENABLES+13]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ14, CSMSetupArray[TDCENABLES+14]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ15, CSMSetupArray[TDCENABLES+15]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ16, CSMSetupArray[TDCENABLES+16]);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ17, CSMSetupArray[TDCENABLES+17]);
  PanelSave(CSMSetupHandle);
  GetMezzCardEnables();
}


int CSMBoardStatus(void) {
  int status = -1, mezz, nlist, list, color, val;
  
  if (CSMSetupStatus() == 0) status = 0;
  else if (CSMSetupStatus() > 0) status = 1;
  if ((status == -1) && (GOLSetupStatus() == 0)) status = 0;
  else if (GOLSetupStatus() > 0) status = 1;
  if ((status == -1) && (TTCrxSetupStatus() == 0)) status = 0;
  else if (TTCrxSetupStatus() > 0) status = 1;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if ((status == -1) && (MezzCardSetupStatus(mezz) == 0)) status = 0;
    if (MezzCardSetupStatus(mezz) > 0) status = 1;
  }

  GetNumListItems(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, &nlist);
  DeleteListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, 0, nlist);
  list = 0;
  if (status == -1) {
    color = VAL_MAGENTA;
    InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "Unknown", status);
  }
  else {
    if (status == 1) {
      color = VAL_RED;
      InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "JTAG Failed", status);
    }
    if (validCSMStatus) {
      if (CSMStatusArray[CSMERRORBIT] == 1) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "CSM Has Error", status);
        printf("*** CSM Has Error\n");
      }
      if (CSMStatusArray[GOLREADYBIT] == 0) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "GOL NotReady (From CSM)", status);
        printf("*** GOL NotReady (From CSM)\n");
      }
      if (CSMStatusArray[TTCRXREADYBIT] == 0) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "TTCrx NotReady (From CSM)", status);
        printf("*** TTCrx NotReady (From CSM)\n");
      }
      if (CSMStatusArray[LHCLOCKBIT] == 0) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "LHC Clock Unlocked", status);
        printf("*** LHC Clock Unlocked\n");
      }
//      if (CSMStatusArray[XMTLOCK1BIT] == 0) {
//        status++;
//        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "XMT Clock 1 Unlocked", status);
//        printf("*** XMT Clock 1 Unlocked\n");
//      }
      if (CSMStatusArray[XMTLOCK2BIT] == 0) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "XMT Clock 2 Unlocked", status);
        printf("*** XMT Clock 2 Unlocked\n");
      }
      if (CSMStatusArray[TTCLOADSTATUSBIT] == 1) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "TTC PROM Load Error", status);
        printf("*** TTC PROM Load Error\n");
      }
      if (CSMStatusArray[PHASEERRORBIT] == 1) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "AMT Phase Lock Error", status);
        printf("*** AMT Phase Lock Error\n");
      }
    }
    if (validGOLStatus) {
      if (GOLBScanArray[GOLBSCANREADY] == 0) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "GOL NotReady (From GOL)", status);
        printf("*** GOL NotReady (From GOL)\n");
      }
      BinaryToInt(&val, LINKCONTROLSTATEA, 2, GOLSetupArray);
      if (val == GOLOUTOFLOCK) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "LinkA: PLL Unlocked", status);
        printf("*** LinkA: PLL Unlocked\n");
      }
      else if (val == GOLLOCKED) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "LinkA: PLL has aquired lock", status);
        printf("*** LinkA: PLL has aquired lock\n");
      }
      BinaryToInt(&val, LINKCONTROLSTATEB, 2, GOLSetupArray);
      if (val == GOLOUTOFLOCK) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "LinkB: PLL Unlocked", status);
        printf("*** LinkB: PLL Unlocked\n");
      }
      else if (val == GOLLOCKED) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "LinkB: PLL has aquired lock", status);
        printf("*** LinkB: PLL has aquired lock\n");
      }
      BinaryToInt(&val, LINKCONTROLSTATEC, 2, GOLSetupArray);
      if (val == GOLOUTOFLOCK) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "LinkC: PLL Unlocked", status);
        printf("*** LinkC: PLL Unlocked\n");
      }
      else if (val == GOLLOCKED) {
        status++;
        InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "LinkC: PLL has aquired lock", status);
        printf("*** LinkC: PLL has aquired lock\n");
      }
    }
    if (status == 0) {
      color = VAL_GREEN;
      InsertListItem(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, list++, "No Error Found", status);
    }
    else color = VAL_RED;
  }
  SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, ATTR_TEXT_BGCOLOR, color);
  
  return status;
}


void UpdateJTAGControlPanel(void) {
  JTAGChainCSMChip();
  JTAGChainGOL();
  JTAGChainTTCrx();
  JTAGChainCSM();
  JTAGChainMezzCards();
  validCSMStatus = TRUE;
  CSMBoardStatus();
}


//-----------------------------------------------------------------------
//
// Help Menu Routines Start Here
//
//-----------------------------------------------------------------------
void HelpButton(void) {
  if (controlOption == NORM) {
    MessagePopup("Help",
                 "Push any active button below for it's help text.\n"
                 "Note: For JTAG settings, consult corresponding manuals (GOL, TTCrx and AMT/ASD).");
    controlOption = HELP;
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Help Button",
                 "Push this button, then push any other button and the help on that button will then pop up.\n"
                 "Normal functionality of the button will return after viewing the help text.\n");
    controlOption = NORM;
  }
}


void AboutDone(void) {
  HidePanel(TitleHandle);
}

void HelpAbout(void) {
  DisplayPanel(TitleHandle); 
}


//
// Utility functions related with panel handling
//
// Close active panel and return to JTAG control panel 
void PanelDone(void) {
  int panel;
  
  panel = GetActivePanel();
  HidePanel(panel);
  DisplayPanel(JTAGControlHandle);
  SetActivePanel(JTAGControlHandle);
}


// Set Active Panel to Default
void PanelDefault(void) {
  int panel, resp, value, index;
  
  panel = GetActivePanel();
  resp = ConfirmPopup("Default Confirmation",
                      "Are you sure you want default setting for the panel?");
  if (resp == 1) {
    GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &value);
    if (defaultProfileSaved) {
      index = PanelIndexInFile(panel, mezzCardSetupAll, mezzCardNb, selectedASD);
      if (RecallPanelState(panel, "default.prf", index) != 0) {
        if ((panel == AMTSetupHandle) || (panel == AMTErrorHandle) || (panel == ASDSetupHandle)) {
          index = PanelIndexInFile(panel, 1, mezzCardNb, selectedASD);
          if (RecallPanelState(panel, "default.prf", index) != 0) DefaultPanel(panel);
        }
        else DefaultPanel(panel);
      }
    }
    else DefaultPanel(panel);
    if (panel == GOLSetupHandle) {
      GetGOLConfigure();
      GetGOLStatus();
      UpdateGOLSetupPanel();
    }
    else if (panel == TTCrxSetupHandle) {
      PutClock1DelayValue();
      PutClock2DelayValue();
      PutTestOutputSelection();
      PutFrequencyCheckPeriod();
    }
    else if (panel == CSMSetupHandle) {
      GetCSMStatus();
	  
	  printf("GetCSMStatus() is in void PanelDefault()\n");
	  
      UpdateCSMSetupPanel();
      CreatGOLLDList();
      PutCSMNextState();
    }
    SetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, value);
    if (mezzCardSetupAll == 1) {
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MIN_VALUE, 1);
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MAX_VALUE, MAXNUMBERMEZZANINE);
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_TEXT, "# of MezzCard");
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_LEFT, VAL_AUTO_CENTER);
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_CTRL_MODE, VAL_INDICATOR);
    }
    else {
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MIN_VALUE, 0);
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MAX_VALUE, MAXNUMBERMEZZANINE-1);
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_TEXT, "MezzCard ID");
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_LEFT, VAL_AUTO_CENTER);
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_CTRL_MODE, VAL_HOT);
    }
  }
}


// Cancel changes made to the active panel
void PanelCancel(void) {
  int panel, resp, index;
  
  panel = GetActivePanel();
  if (panel == CSMSetupHandle)
    resp = ConfirmPopup("CSM Setup Cancellation Confirmation",
                        "Are you sure to discard all changes for CSM setup?");
  else if (panel == TTCrxSetupHandle)
    resp = ConfirmPopup("TTCrx Setup Cancellation Confirmation",
                        "Are you sure to discard all changes for TTCrx setup?");
  else if (panel == GOLSetupHandle)
    resp = ConfirmPopup("GOL Setup Cancellation Confirmation",
                        "Are you sure to discard all changes for GOL setup?");
  else
    resp = ConfirmPopup("Cancellation Confirmation",
                        "Are you sure to discard all changes for the panel?");
  
  if (resp == 1) {
    // Recall the panel states
    index = PanelIndexInFile(panel, mezzCardSetupAll, mezzCardNb, 0);
    RecallPanelState(panel, currSetupFile, index);
    HidePanel(panel);
    DisplayPanel(JTAGControlHandle);
    SetActivePanel(JTAGControlHandle);
  }
}


// Save panel into currSetupFile
void PanelSave(int panel) {
  int index;
  
  index = PanelIndexInFile(panel, mezzCardSetupAll, mezzCardNb, selectedASD);
  SavePanelState(panel, currSetupFile, index);
}


void PanelRecall(int panel) {
  int index, value;
  
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &value);
  index = PanelIndexInFile(panel, mezzCardSetupAll, mezzCardNb, selectedASD);
  if (RecallPanelState(panel, currSetupFile, index) != 0) {
    if ((panel == AMTSetupHandle) || (panel == AMTErrorHandle) || (panel == ASDSetupHandle)) {
      index = PanelIndexInFile(panel, 1, mezzCardNb, selectedASD);
      if (RecallPanelState(panel, currSetupFile, index) != 0) {
        if (defaultProfileSaved) {
          index = PanelIndexInFile(panel, mezzCardSetupAll, mezzCardNb, selectedASD);
          if (RecallPanelState(panel, "default.prf", index) != 0) {
            index = PanelIndexInFile(panel, 1, mezzCardNb, selectedASD);
            if (RecallPanelState(panel, "default.prf", index) != 0) DefaultPanel(panel);
          }
          else DefaultPanel(panel);
        }
        else DefaultPanel(panel);
      }
    }
    else if (defaultProfileSaved) {
      if (RecallPanelState(panel, "default.prf", index) != 0) {
        if ((panel == AMTSetupHandle) || (panel == AMTErrorHandle) || (panel == ASDSetupHandle)) {
          index = PanelIndexInFile(panel, 1, mezzCardNb, selectedASD);
          if (RecallPanelState(panel, "default.prf", index) != 0) DefaultPanel(panel);
        }
        else DefaultPanel(panel);
      }
    }
    else DefaultPanel(panel);
  }
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, value);
  if (mezzCardSetupAll == 1) {
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MIN_VALUE, 1);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MAX_VALUE, MAXNUMBERMEZZANINE);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_TEXT, "# of MezzCard");
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_LEFT, VAL_AUTO_CENTER);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_CTRL_MODE, VAL_INDICATOR);
  }
  else {
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MIN_VALUE, 0);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MAX_VALUE, MAXNUMBERMEZZANINE-1);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_TEXT, "MezzCard ID");
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_LEFT, VAL_AUTO_CENTER);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_CTRL_MODE, VAL_HOT);
  }
  index = PanelIndexInFile(panel, mezzCardSetupAll, mezzCardNb, selectedASD);
  SavePanelState(panel, currSetupFile, index);
}


// Save used parameter into a file
void SaveParameters(int paraIndex) {
  FILE *paraFile;
  char str[80], *strp;
  int wantedMezzEnables = 0, enable, inforPrecision = 2, DIMParaControl = 0;
  
  normalJTAG = 1;
  thresholdSettingMethod = 1;
  if (paraFile = fopen(parameterFile, "r")) {
    while (feof(paraFile) == 0) {
      fgets(str, 80, paraFile);
      if (paraIndex != PARARUNNUMBER) {
        strp = strstr(str, "Run Number");
        if (strp != NULL) {
          sscanf(str+23, "%u", &runNumber);
        }
      }
      strp = strstr(str, "Normal JTAG Operation");
      if (strp != NULL) {
        sscanf(str+23, "%d", &normalJTAG);
      }
      if (paraIndex != PARANUMBERCSM) {
        strp = strstr(str, "Number of CSM");
        if (strp != NULL) {
          sscanf(str+23, "%d", &numberCSM);
          if (numberCSM <= 0) numberCSM = 1;
          SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_NUMBERCSM, numberCSM);
        }
      }
      if (paraIndex != PARACURRENTCSMNUMBER) {
        strp = strstr(str, "Current CSM Number");
        if (strp != NULL) {
          sscanf(str+23, "%d", &CSMNumber);
          SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, CSMNumber);
        }
      }
      strp = strstr(str, "Mezz. Card Enables");
      if (strp != NULL) {
        sscanf(str+23, "%x", &wantedMezzEnables);
      }
      if (paraIndex != PARANOMINALTHRESHOLD) {
        strp = strstr(str, "Nominal Threshold(mV)");
        if (strp != NULL) {
          sscanf(str+23, "%d", &nominalThreshold);
          SetCtrlVal(RunConditionHandle, P_RUNCOND_NTHRESHOLD, nominalThreshold);
        }
      }
      if (paraIndex != PARACSM0BASE) {
        strp = strstr(str, "CSM0 Base");
        if (strp != NULL) {
          sscanf(str+23, "%x", &CSM0Base);
        }
      }
      if (paraIndex != PARACO11491BASE) {
        strp = strstr(str, "Corelis 1149.1 Base");
        if (strp != NULL) {
          sscanf(str+23, "%x", &Co11491Base);
        }
      }
      if (paraIndex != PARAVMIC2510BASE) {
        strp = strstr(str, "VMIC 2510 Base");
        if (strp != NULL) {
          sscanf(str+23, "%x", &VMIC2510Base);
        }
      }
      strp = strstr(str, "DIM Para Controls");
      if (strp != NULL) {
        sscanf(str+23, "%u", &DIMParaControl);
      }
      if ((DIMParaControl%2) == 1) {
        strp = strstr(str, "JTAG Server");
        if (strp != NULL) {
          sscanf(str+23, "%s", JTAGServerName);
        }
      }
      if (((DIMParaControl>>1)%2) == 1) {
        strp = strstr(str, "CAN Node ID");
        if (strp != NULL) {
          sscanf(str+23, "%d", &CANNode);
        }
      }
      if (((DIMParaControl>>2)%2) == 1) {
        strp = strstr(str, "ELMB Node ID");
        if (strp != NULL) {
          sscanf(str+23, "%d", &ELMBNode);
        }
      }
      if (paraIndex != PARATTCVIBASE) {
        strp = strstr(str, "TTCvi Base");
        if (strp != NULL) {
          sscanf(str+23, "%x", &TTCviBase);
        }
      }
      strp = strstr(str, "DAQ Infor Precision");
      if (strp != NULL) {
        sscanf(str+23, "%u", &inforPrecision);
        if (inforPrecision > 4) inforPrecision = 4;
      }
      strp = strstr(str, "Threshold Set Method");
      if (strp != NULL) {
        sscanf(str+23, "%d", &thresholdSettingMethod);
      }
      if (paraIndex != PARABITSTREAMLENGTH) {
        strp = strstr(str, "Bit Stream Length");
        if (strp != NULL) {
          sscanf(str+23, "%d", &nBitstreamBytes);
        }
      }
      if (paraIndex != PARABITSTREAMFILE) {
        strp = strstr(str, "Bit Stream Text File");
        if (strp != NULL) {
          sscanf(str+23, "%s", bitStreamTextFName);
        }
      }
    }
    fclose(paraFile);
  }
  oldNumberCSM = numberCSM;
  if (paraFile = fopen(parameterFile, "w")) {
    fprintf(paraFile, "Run Number           : %10u\n", runNumber);
    fprintf(paraFile, "Used JTAG Control Parameters\n");
    fprintf(paraFile, "Normal JTAG Operation: %10d\n", normalJTAG);
    fprintf(paraFile, "Default Profile Saved: %10d\n", defaultProfileSaved);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_NUMBERCSM, &numberCSM);
    fprintf(paraFile, "Number of CSM        : %10d\n", numberCSM);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, &CSMNumber);
    fprintf(paraFile, "Current CSM Number   : %10d\n", CSMNumber);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, &enable);
    fprintf(paraFile, "PROM in JTAG Chain   : %10d\n", enable);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, &enable);
    fprintf(paraFile, "CSMChip in JTAG Chain: %10d\n", enable);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, &enable);
    fprintf(paraFile, "GOL in JTAG Chain    : %10d\n", enable);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, &enable);
    fprintf(paraFile, "TTCrx in JTAG Chain  : %10d\n", enable);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, &enable);
    fprintf(paraFile, "CSM in JTAG Chain    : %10d\n", enable);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, &enable);
    fprintf(paraFile, "Mezz. in JTAG Chain  : %10d\n", enable);
    if (paraIndex == PARAALL || paraIndex == PARAMEZZENABLES) wantedMezzEnables = mezzEnables;
    fprintf(paraFile, "Mezz. Card Enables   : 0x%08X\n", wantedMezzEnables);
    fprintf(paraFile, "Nominal Threshold(mV): %10d\n", nominalThreshold);
    fprintf(paraFile, "CSM0 Base            : 0x%08X\n", CSM0Base);
    fprintf(paraFile, "Corelis 1149.1 Base  : 0x%08X\n", Co11491Base);
    fprintf(paraFile, "VMIC 2510 Base       : 0x%08X\n", VMIC2510Base);
    if (JTAGDRIVERTYPE == CANELMB) {
      fprintf(paraFile, "// DIM Para Controls\n");
      fprintf(paraFile, "//        0 = Using Program Default\n");
      fprintf(paraFile, "//    bit 0 = Enable to Read JTAG Server Name from this file\n");
      fprintf(paraFile, "//    bit 1 = Enable to Read CAN Node ID from this file\n");
      fprintf(paraFile, "//    bit 2 = Enable to Read ELMB Node ID from this file\n");
      fprintf(paraFile, "DIM Para Controls    : %10d\n", DIMParaControl);
      fprintf(paraFile, "JTAG Server          : %10s\n", JTAGServerName);
      fprintf(paraFile, "CAN Node ID          : %10d\n", CANNode);
      fprintf(paraFile, "ELMB Node ID         : %10d\n", ELMBNode);
      fprintf(paraFile, "ELMB Version         : %10s\n", ELMBVersion);
    }
    fprintf(paraFile, "TTCvi Base           : 0x%08X\n", TTCviBase);
    fprintf(paraFile, "// DAQ infor precision (# decimal points) for average event size and data flow rate.\n");
    fprintf(paraFile, "// Allowed value is from 0 to 4, only the number is allowd to be changed.\n");
    fprintf(paraFile, "DAQ Infor Precision  : %10u\n", inforPrecision);
    fprintf(paraFile, "// Method to set nominal ASD threshold by using V offset\n");
    fprintf(paraFile, "//   0 = threshold + 0.5*(VOffMax + VOffMin)\n");
    fprintf(paraFile, "//   1 = ATLAS official way to set ASD threshold\n");
    fprintf(paraFile, "//       threshold + 0.5*(VOffMax + VOffMin)    for V offset span within 12mV\n");
    fprintf(paraFile, "//       threshold + 0.5*(VOffMax + VOffMin) -2 for V offset span within 12-14mV\n");
    fprintf(paraFile, "//       threshold + 0.5*(VOffMax + VOffMin) -4 for V offset span within 14-16mV\n");
    fprintf(paraFile, "Threshold Set Method : %10u\n", thresholdSettingMethod);
    fprintf(paraFile, "DAQ Infor Precision  : %10u\n", inforPrecision);
    if (nBitstreamBytes > 0) {
      fprintf(paraFile, "Bit Stream Length    : %10d\n", nBitstreamBytes);
      fprintf(paraFile, "Bit Stream Text File : %s\n", bitStreamTextFName);
    }
    fclose(paraFile);
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_RUNNUMBER, runNumber);
  }
  if (!gotJTAGDriver) normalJTAG = 0;
  if (normalJTAG == 0) action = TEXTFILES;
  if (action != DOWNLOAD) normalJTAG = 0;
}


// Read parameter from a file
int ReadParameters(int paraIndex) {
  FILE *paraFile;
  int enable, inforPrecision = 2, DIMParaControl = 0;
  char str[80], *strp;
  
  normalJTAG = 1;
  thresholdSettingMethod = 1;
  if (paraFile = fopen(parameterFile, "r")) {
    while (feof(paraFile) == 0) {
      fgets(str, 80, paraFile);
      strp = strstr(str, "Run Number");
      if (strp != NULL) {
        sscanf(str+23, "%u", &runNumber);
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_RUNNUMBER, runNumber);
      }
      strp = strstr(str, "Normal JTAG Operation");
      if (strp != NULL) {
        sscanf(str+23, "%d", &normalJTAG);
      }
      strp = strstr(str, "Default Profile Saved");
      if (strp != NULL) {
        sscanf(str+23, "%d", &defaultProfileSaved);
      }
      strp = strstr(str, "Number of CSM");
      if (strp != NULL) {
        sscanf(str+23, "%d", &numberCSM);
        if (numberCSM <= 0) numberCSM = 1;
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_NUMBERCSM, numberCSM);
      }
      strp = strstr(str, "Current CSM Number");
      if (strp != NULL) {
        sscanf(str+23, "%d", &CSMNumber);
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMNUMBER, CSMNumber);
      }
      strp = strstr(str, "PROM in JTAG Chain");
      if (strp != NULL) {
        sscanf(str+23, "%d", &enable);
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, enable);
      }
      strp = strstr(str, "CSMChip in JTAG Chain");
      if (strp != NULL) {
        sscanf(str+23, "%d", &enable);
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, enable);
      }
      strp = strstr(str, "GOL in JTAG Chain");
      if (strp != NULL) {
        sscanf(str+23, "%d", &enable);
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, enable);
      }
      strp = strstr(str, "TTCrx in JTAG Chain");
      if (strp != NULL) {
        sscanf(str+23, "%d", &enable);
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, enable);
      }
      strp = strstr(str, "CSM in JTAG Chain");
      if (strp != NULL) {
        sscanf(str+23, "%d", &enable);
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, enable);
      }
      strp = strstr(str, "Mezz. in JTAG Chain");
      if (strp != NULL) {
        sscanf(str+23, "%d", &enable);
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, enable);
        SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, enable);
      }
      strp = strstr(str, "Mezz. Card Enables");
      if (strp != NULL) {
        sscanf(str+23, "%x", &mezzEnables);
      }
      strp = strstr(str, "Nominal Threshold(mV)");
      if (strp != NULL) {
        sscanf(str+23, "%d", &nominalThreshold);
        SetCtrlVal(RunConditionHandle, P_RUNCOND_NTHRESHOLD, nominalThreshold);
      }
      strp = strstr(str, "CSM0 Base");
      if (strp != NULL) {
        sscanf(str+23, "%x", &CSM0Base);
      }
      strp = strstr(str, "Corelis 1149.1 Base");
      if (strp != NULL) {
        sscanf(str+23, "%x", &Co11491Base);
      }
      strp = strstr(str, "VMIC 2510 Base");
      if (strp != NULL) {
        sscanf(str+23, "%x", &VMIC2510Base);
      }
      strp = strstr(str, "DIM Para Controls");
      if (strp != NULL) {
        sscanf(str+23, "%u", &DIMParaControl);
      }
      if ((DIMParaControl%2) == 1) {
        strp = strstr(str, "JTAG Server");
        if (strp != NULL) {
          sscanf(str+23, "%s", JTAGServerName);
        }
      }
      if (((DIMParaControl>>1)%2) == 1) {
        strp = strstr(str, "CAN Node ID");
        if (strp != NULL) {
          sscanf(str+23, "%d", &CANNode);
        }
      }
      if (((DIMParaControl>>2)%2) == 1) {
        strp = strstr(str, "ELMB Node ID");
        if (strp != NULL) {
          sscanf(str+23, "%d", &ELMBNode);
        }
      }
      strp = strstr(str, "TTCvi Base");
      if (strp != NULL) {
        sscanf(str+23, "%x", &TTCviBase);
      }
      strp = strstr(str, "DAQ Infor Precision");
      if (strp != NULL) {
        sscanf(str+23, "%u", &inforPrecision);
        if (inforPrecision > 4) inforPrecision = 4;
      }
      strp = strstr(str, "Threshold Set Method");
      if (strp != NULL) {
        sscanf(str+23, "%d", &thresholdSettingMethod);
      }
      strp = strstr(str, "Bit Stream Length");
      if (strp != NULL) {
        sscanf(str+23, "%d", &nBitstreamBytes);
      }
      strp = strstr(str, "Bit Stream Text File");
      if (strp != NULL) {
        sscanf(str+23, "%s", bitStreamTextFName);
      }
    }
    fclose(paraFile);
  }
  if (!gotJTAGDriver) normalJTAG = 0;
  if (normalJTAG == 0) action = TEXTFILES;
  if (action != DOWNLOAD) normalJTAG = 0;
  JTAGChainPROM();
  JTAGChainCSMChip();
  JTAGChainGOL();
  JTAGChainTTCrx();
  JTAGChainCSM();
  JTAGChainMezzCards();
  oldNumberCSM = numberCSM;
  SetNumberCSM();
  if (paraIndex == PARANORMALJTAG) return normalJTAG;
  else if (paraIndex == PARADEFAULTSAVED) return defaultProfileSaved;
  else if (paraIndex == PARANUMBERCSM) return numberCSM;
  else if (paraIndex == PARACURRENTCSMNUMBER) return CSMNumber;
  else if (paraIndex == PARAMEZZENABLES) return mezzEnables;
  else if (paraIndex == PARANOMINALTHRESHOLD) return nominalThreshold;
  else if (paraIndex == PARACSM0BASE) return CSM0Base;
  else if (paraIndex == PARACO11491BASE) return Co11491Base;
  else if (paraIndex == PARAVMIC2510BASE) return VMIC2510Base;
  else if (paraIndex == PARATTCVIBASE) return TTCviBase;
  else if (paraIndex == PARAINFORPRECISION) return inforPrecision;
  else if (paraIndex == PARABITSTREAMLENGTH) return nBitstreamBytes;
  else return 0;
}


//
// Function for CSM IOs control
//
void UpCSMIOsControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    UpdateCSMIOsPanel(TRUE);
    DisplayPanel(CSMIOsHandle);
    SetActivePanel(CSMIOsHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on CSM IOs Control",
                 "Bring up CSM IOs Control Panel.\n"
                 "Note: For CSM IO settings, consult the CSM manual.");
  }
  controlOption = NORM;
}


// Load CSM IOs Control Panel and define user buttons
int CSMIOsPanelAndButton(void) {
  // Define the CSM IOs Control panel handles
  if ((CSMIOsHandle = LoadPanel(0, "DAQGUI.uir", P_CSMIO)) < 0) return -1;
  SetPanelAttribute(CSMIOsHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);

  // CSM IOs Control Panel Buttons
  SetupUserButton(Menu00Handle, MENU00_EXE_CSMIO, UpCSMIOsControl);
  SetupUserButton(Menu00Handle, MENU00_EXE_PROGRAMFPGAVIAIO, CSMIOsProgramFPGAFromPROM);
  SetupUserButton(Menu00Handle, MENU00_EXE_RESETCSM, CSMIOsResetCSMBoard);
  SetupUserButton(Menu00Handle, MENU00_EXE_SELECTSOFTTDO, CSMIOsSelectSoftwareTDO);
  SetupUserButton(Menu00Handle, MENU00_EXE_SELECTHARDTDO, CSMIOsSelectHardwareTDO);
  SetupUserButton(CSMIOsHandle, P_CSMIO_DONE, CSMIOsDone);
  SetupUserButton(CSMIOsHandle, P_CSMIO_DEFAULT, PanelDefault);
  SetupUserButton(CSMIOsHandle, P_CSMIO_CANCEL, PanelCancel);
  SetupUserButton(CSMIOsHandle, P_CSMIO_SETIOS, CSMIOsSet);
  SetupUserButton(CSMIOsHandle, P_CSMIO_READIOS, CSMIOsRead);
  SetupUserButton(CSMIOsHandle, P_CSMIO_PROGRAMFPGA, CSMIOsProgramFPGAFromPROM);
  SetupUserButton(CSMIOsHandle, P_CSMIO_RESETCSM, CSMIOsResetCSMBoard);
  SetupUserButton(CSMIOsHandle, P_CSMIO_SELECTSOFTTDC, CSMIOsSelectSoftwareTDO);
  SetupUserButton(CSMIOsHandle, P_CSMIO_SELECTHARDTDC, CSMIOsSelectHardwareTDO);

  return 0;
}


// Close CSM IOs Control Panel
void CSMIOsDone(void) {
  UpdateCSMIOsPanel(FALSE);
  printf("Setting CSM IO is done.\n");
  SetupJTAGChain();
  HidePanel(CSMIOsHandle);
  DisplayPanel(JTAGControlHandle);
  SetActivePanel(JTAGControlHandle);
}


void CSMIOsSet(void) {
  UpdateCSMIOsPanel(FALSE);
  printf("Set CSM IOs is done\n");
}


void CSMIOsRead(void) {
  UpdateCSMIOsPanel(TRUE);
  printf("Read CSM IOs is done\n");
}


void CSMIOsProgramFPGAFromPROM(void) {
  SetCtrlVal(CSMIOsHandle, P_CSMIO_PROGRAMFPGABAR, FALSE);
  UpdateCSMIOsPanel(FALSE);
  SetCtrlVal(CSMIOsHandle, P_CSMIO_PROGRAMFPGABAR, TRUE);
  UpdateCSMIOsPanel(FALSE);
  // Need to wait at least 40ms to program FPGA from PROM
  printf("Program FPGA from PROM via IO pin is issued, wait 100ms. \n");
  WaitInMillisecond(100);
  printf("Program FPGA from PROM via IO pin is done.\n");
  SetupJTAGChain();
}


void CSMIOsResetCSMBoard(void) {
  SetCtrlVal(CSMIOsHandle, P_CSMIO_RESETLEVEL, TRUE);
  UpdateCSMIOsPanel(FALSE);
  SetCtrlVal(CSMIOsHandle, P_CSMIO_RESETLEVEL, FALSE);
  UpdateCSMIOsPanel(FALSE);
  printf("Reset CSM Board via IO pin is done.\n");
  SetupJTAGChain();
}


void CSMIOsSelectSoftwareTDO(void) {
  SetCtrlVal(CSMIOsHandle, P_CSMIO_BHARDWARETDO, TRUE);
  SetCtrlVal(CSMIOsHandle, P_CSMIO_BSOFTWARETDO, FALSE);
  UpdateCSMIOsPanel(FALSE);
  printf("Software TDO in is selected.\n");
  SetupJTAGChain();
}


void CSMIOsSelectHardwareTDO(void) {
  SetCtrlVal(CSMIOsHandle, P_CSMIO_BHARDWARETDO, FALSE);
  SetCtrlVal(CSMIOsHandle, P_CSMIO_BSOFTWARETDO, TRUE);
  UpdateCSMIOsPanel(FALSE);
  printf("Hardware TDO in is selected.\n");
  SetupJTAGChain();
}


void UpdateCSMIOsPanel(int readOnly) {
  int color, okColor, errColor, warningColor, i, val, rval;
  
  CSMTestStatus = 0;
  GetCtrlVal(CSMIOsHandle, P_CSMIO_BHARDWARETDO, &dataArray[0]);
  GetCtrlVal(CSMIOsHandle, P_CSMIO_BSOFTWARETDO, &dataArray[1]);
  GetCtrlVal(CSMIOsHandle, P_CSMIO_RESETLEVEL, &dataArray[2]);
  GetCtrlVal(CSMIOsHandle, P_CSMIO_PROGRAMFPGABAR, &dataArray[3]);
  if (readOnly) {
    if (ReadCSMIOs(readbackArray) == 0) {
      for (i = 0; i < 4; i++) dataArray[i] = readbackArray[i];
    }
    else CSMTestStatus = -1;
    SetCtrlVal(CSMIOsHandle, P_CSMIO_BHARDWARETDO, dataArray[0]);
    SetCtrlVal(CSMIOsHandle, P_CSMIO_BSOFTWARETDO, dataArray[1]);
    SetCtrlVal(CSMIOsHandle, P_CSMIO_RESETLEVEL, dataArray[2]);
    SetCtrlVal(CSMIOsHandle, P_CSMIO_PROGRAMFPGABAR, dataArray[3]);
  }
  else {
    WriteCSMIOs(dataArray);
    if (dataArray[2] == 1) ClearSetupStatus();
    else if ((dataArray[3] == 0) && (CSMType == CSM)) ClearSetupStatus();
  }
  val = 0;
  for (i = 0; i < 4; i++) val |= (dataArray[i] << i);
  SetCtrlVal(CSMIOsHandle, P_CSMIO_WANTEDIOS, val);
  if (ReadCSMIOs(readbackArray) == 0) {
    color = VAL_WHITE;
    okColor = VAL_GREEN;
    errColor = VAL_RED;
    warningColor = VAL_YELLOW;
    SetCtrlVal(CSMIOsHandle, P_CSMIO_CSMERROR, readbackArray[4]);
    SetCtrlVal(CSMIOsHandle, P_CSMIO_I2COPERATION, readbackArray[5]);
    SetCtrlVal(CSMIOsHandle, P_CSMIO_READYS, readbackArray[6]);
    if (readbackArray[4] == 1) {
      CSMTestStatus = 1;
      if (readbackArray[6] == 1) CSMTestStatus = 3;
    }
    else if (readbackArray[6] == 1) CSMTestStatus = 2;
    rval = 0;
    for (i = 0; i < 4; i++) rval |= (readbackArray[i] << i);
    SetCtrlVal(CSMIOsHandle, P_CSMIO_READBACKIOS, rval);
    if (val != rval) {
      SetCtrlAttribute(CSMIOsHandle, P_CSMIO_READBACKIOS, ATTR_TEXT_BGCOLOR, warningColor);
      CSMTestStatus = 10;
    }
    else
      SetCtrlAttribute(CSMIOsHandle, P_CSMIO_READBACKIOS, ATTR_TEXT_BGCOLOR, okColor);
  }
  else {
    CSMTestStatus = -2;
    color = VAL_BLACK;
    okColor = VAL_BLACK;
    errColor = VAL_BLACK;
    warningColor = VAL_BLACK;
  }
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_CSMERROR, ATTR_ON_COLOR, errColor);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_CSMERROR, ATTR_OFF_COLOR, okColor);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_I2COPERATION, ATTR_ON_COLOR, warningColor);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_I2COPERATION, ATTR_OFF_COLOR, okColor);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_READYS, ATTR_ON_COLOR, errColor);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_READYS, ATTR_OFF_COLOR, okColor);
  PanelSave(CSMIOsHandle);
  HandleUserButtons(PanelDone);
}


// Functions for writting Xilinx SVF file
//
void TurnOffMezzCardJTAG(FILE *file) {
  static int enable, offJTAGDone = FALSE, i;
  
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, &enable);
  if (enable != 0) {
    offJTAGDone = FALSE;
    return;
  }
  if (file != NULL) {
    if (offJTAGDone) return;
    offJTAGDone = TRUE;
  }
  turnOffMezzCardJTAG = TRUE;
  AllJTAGDeviceInBYPASS(file, 2);
  instrLength = 0;
  
 //Modified by Xiangting 
 /* 
  for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
    IntToBinary(AMTBYPASS, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += AMTINSTLENGTH;
    if (file != NULL) AMTJTAGComments(AMTBYPASS, file, i);
    else WriteJTAGInstructionToActionFile(AMT3ID, AMTBYPASS, i);
  }
 */
  for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
	IntToBinary(HPTDCBYPASS, instrLength, HPTDCINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += HPTDCINSTLENGTH;
    if (file != NULL) HPTDCJTAGComments(HPTDCBYPASS, file, i);
    else WriteJTAGInstructionToActionFile(HPTDCID, HPTDCBYPASS, i);
	IntToBinary(A3P250BYPASS, instrLength, A3P250INSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += A3P250INSTLENGTH;
    if (file != NULL) A3P250JTAGComments(A3P250BYPASS, file, i);
    else WriteJTAGInstructionToActionFile(A3P250ID, A3P250BYPASS, i);
  }
  
  

//End
  
  if (CSMOn) {
    IntToBinary(CSMFULLRW, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += CSMINSTLENGTH;
    if (file != NULL) CSMJTAGComments(CSMFULLRW, file);		  
    else WriteJTAGInstructionToActionFile(CSMID, CSMFULLRW, 0);
  }
  if (TTCrxOn) {
    IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += TTCRXINSTLENGTH;
    if (file == NULL) WriteJTAGInstructionToActionFile(TTCRXID, TTCRXBYPASS, 0);
  }
  if (GOLOn) {
    IntToBinary(GOLBYPASS, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += GOLINSTLENGTH;
    if (file != NULL) GOLJTAGComments(GOLBYPASS, file);		  
    else WriteJTAGInstructionToActionFile(GOLID, GOLBYPASS, 0);
  }
  if (AX1000On) {
    IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += AX1000INSTLENGTH;
    if (file != NULL) AX1000JTAGComments(AX1000BYPASS, file);		  
    else WriteJTAGInstructionToActionFile(AX1000ID, AX1000BYPASS, 0);
  }
  if (XC2V1000On || XC2V2000On) {
    IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += VERTEXIIINSTLENGTH;
    if (file != NULL) VertexIIJTAGComments(VERTEXIIBYPASS, file);		  
    else {
      if (XC2V2000On) WriteJTAGInstructionToActionFile(XC2V2000ID, VERTEXIIBYPASS, 0);
      else WriteJTAGInstructionToActionFile(XC2V1000ID, VERTEXIIBYPASS, 0);
    }
  }
  if (PROMOn) {
    IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += PROMINSTLENGTH;
    if (file != NULL) PROMJTAGComments(PROMBYPASS, file);		  
    else WriteJTAGInstructionToActionFile(PROMID, PROMBYPASS, 0);
  }
  WriteJTAGInstructions(file);
  dataLength = 0;
  for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
	  
//Modified by Xiangting
	  
//    mezzOffset[i] = dataLength;
//    dataArray[dataLength] = 1;
//    maskArray[dataLength++] = 0;
//    if (file == NULL) WriteJTAGDataToActionFile(1, mezzOffset[i]);

	HPTDCOffset[i]=dataLength;
	dataArray[dataLength] = 1;
	maskArray[dataLength++] = 0;
	if (file == NULL) WriteJTAGDataToActionFile(1, HPTDCOffset[i]);
	A3P250Offset[i]=dataLength;
	dataArray[dataLength] = 1;
	maskArray[dataLength++] = 0;
	if (file == NULL) WriteJTAGDataToActionFile(1, A3P250Offset[i]);	
  }
  if (CSMOn) {
    CSMOffset = dataLength;
    for (i = dataLength; i < dataLength+CSMSTATUSLENGTH; i++) {
      dataArray[i] = CSMStatusArray[i-dataLength];
      if (i == dataLength) {
        dataArray[i] = 1;
        maskArray[i] = 0;
      }
      else if (i < dataLength+12) {
        dataArray[i] = 0;
        maskArray[i] = 0;
      }
      else maskArray[i] = 0;
    }
    dataLength += CSMSTATUSLENGTH;
    for (i = dataLength; i < dataLength+CSMSETUPLENGTH; i++) {
      dataArray[i] = CSMSetupArray[i-dataLength];
      maskArray[i] = 1;
      downloadedCSMSetupArray[i-dataLength] = dataArray[i];
    }
    dataLength += CSMSETUPLENGTH;
    for (i = dataLength; i < dataLength+TTCRXSETUPLENGTH; i++) {
      dataArray[i] = TTCrxSetupArray[i-dataLength];
      maskArray[i] = 1;
    }
    dataLength += TTCRXSETUPLENGTH;
    if (file == NULL) WriteJTAGDataToActionFile(TTCRXSETUPLENGTH+CSMSETUPLENGTH+CSMSTATUSLENGTH, CSMOffset);
  }
  if (TTCrxOn) {
    TTCrxOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
    if (file == NULL) WriteJTAGDataToActionFile(1, TTCrxOffset);
  }
  if (GOLOn) {
    GOLOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
    if (file == NULL) WriteJTAGDataToActionFile(1, GOLOffset);
  }
  if (AX1000On) {
    AX1000Offset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
    if (file == NULL) WriteJTAGDataToActionFile(1, AX1000Offset);
  }
  if (XC2V1000On || XC2V2000On) {
    vertexIIOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
    if (file == NULL) WriteJTAGDataToActionFile(1, vertexIIOffset);
  }
  if (PROMOn) {
    PROMOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
    if (file == NULL) WriteJTAGDataToActionFile(1, PROMOffset);
  }
  WriteJTAGData(file);
  if (action == DOWNLOAD) {
    JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
    JTAGScanAllData(dataLength, dataArray, readbackArray);
    if (CSMOn) {
      downloadCSMSetupDone = TRUE;
      CSMLastNextState = CSMNextState;
    }
  }
  turnOffMezzCardJTAG = FALSE;
}


void AllJTAGDeviceInBYPASS(FILE *file, int option) {
  int i;
  
  if (file == NULL) return;
  if (option == 0) {
    if (writeSVFFile) {
      fprintf(file, "// SVF File generated by CSM JTAG Control on %s\n", WeekdayTimeString(time(NULL))); 
      fprintf(file, "//\n");
      fprintf(file, "TRST OFF;\n");
      fprintf(file, "ENDIR IDLE;\n");
      fprintf(file, "ENDDR IDLE;\n");
      fprintf(file, "STATE RESET IDLE;\n");
    }
    TurnOffMezzCardJTAG(file);
  }
  if (!writeSVFFile) return;
  instrLength = 0;
  if (mezzCardsOn || turnOffMezzCardJTAG) {
    for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
      if ((CSMSetupArray[TDCENABLES+i] == 1) || turnOffMezzCardJTAG) {
        if(i==NEWTDC_NUMBER){
          IntToBinary(TDC_BYPASS, instrLength, TDC_INSTR_LENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += TDC_INSTR_LENGTH;
        }
        else{
              //Modified by Xiangting
    
//        IntToBinary(AMTBYPASS, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
//        instrLength += AMTINSTLENGTH;
//        AMTJTAGComments(AMTBYPASS, file, i);
          IntToBinary(HPTDCBYPASS, instrLength, HPTDCINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += HPTDCINSTLENGTH;
          HPTDCJTAGComments(HPTDCBYPASS, file, i);
          IntToBinary(A3P250BYPASS, instrLength, A3P250INSTLENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += A3P250INSTLENGTH;
          A3P250JTAGComments(A3P250BYPASS, file, i);
          //END
        }
		  }
    }
  }
  if (CSMOn) {
    IntToBinary(CSMBYPASS, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += CSMINSTLENGTH;
    CSMJTAGComments(CSMBYPASS, file);		  
  }
  if (TTCrxOn) {
    IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += TTCRXINSTLENGTH;
    TTCrxJTAGComments(TTCRXBYPASS, file);		  
  }
  if (GOLOn) {
    IntToBinary(GOLBYPASS, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += GOLINSTLENGTH;
    GOLJTAGComments(GOLBYPASS, file);		  
  }
  if (AX1000On) {
    IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += AX1000INSTLENGTH;
    AX1000JTAGComments(AX1000BYPASS, file);		  
  }
  if (XC2V1000On || XC2V2000On) {
    IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += VERTEXIIINSTLENGTH;
    VertexIIJTAGComments(VERTEXIIBYPASS, file);		  
  }
  if (PROMOn) {
    IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += PROMINSTLENGTH;
    PROMJTAGComments(PROMBYPASS, file);		  
  }
  WriteJTAGInstructions(file);
  if (option == 1) {
    fprintf(file, "RUNTEST 12 TCK;\n");
  }
}


void WriteJTAGInstructions(FILE *file) {
  int nWords, nBits, i, smask, smaskBits[32];
  char sform[5];

  if (!writeSVFFile) return;
  if ((file == NULL) || (instrLength <= 0)) return;
  ArrayToWord(JTAGWords, &nWords, instrArray, instrLength);
  fprintf(file, "SIR %d TDI (", instrLength);
  nBits = instrLength - (nWords-1)*32;
  sprintf(sform, "%%0%dx", (nBits-1)/4+1);
  fprintf(file, sform, JTAGWords[nWords-1]);
  for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
  fprintf(file, ") SMASK (");
  for (i = 0; i < nBits; i++) smaskBits[i] = 1;
  ArrayToWord(&smask, &i, smaskBits, nBits);
  fprintf(file, sform, smask);
  for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", 0xFFFFFFFF);
  fprintf(file, ");\n");
}


void WriteJTAGData(FILE *file) {/*
  int nWords, nBits, i, i0, smask, smaskBits[32], l;
  char sform[5];

  if ((file == NULL) || (dataLength <= 0)) return;
  if (CSMOn && (CSMNextState == CMDRESETFPGA)) {
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
    LoadCSMSetupArray();
    i0 = CSMOffset+CSMSTATUSLENGTH;
    for (i = i0; i < i0+CSMSETUPLENGTH; i++) dataArray[i] = CSMSetupArray[i-i0];
    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      printf("Waiting to finish Reset CSM Chip (VertexII)\n");
      WaitInMillisecond(1000);     // Wait for 1 seconds
    }
  }
  if (writeSVFFile) {
    ArrayToWord(JTAGWords, &nWords, dataArray, dataLength);
    nBits = dataLength - (nWords-1)*32;
    sprintf(sform, "%%0%dx", (nBits-1)/4+1);
    fprintf(file, "SDR %d TDI (", dataLength);
    fprintf(file, sform, JTAGWords[nWords-1]);
    for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
    fprintf(file, ") SMASK (");
    for (i = 0; i < nBits; i++) smaskBits[i] = 1;
    ArrayToWord(&smask, &i, smaskBits, nBits);
    fprintf(file, sform, smask);
    for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", 0xFFFFFFFF);
    fprintf(file, ");\n");
  }
  if (turnOffMezzCardJTAG) return;

  if (writeSVFFile) {
    WriteJTAGInstructions(file);
    ArrayToWord(JTAGWords, &nWords, dataArray, dataLength);
    nBits = dataLength - (nWords-1)*32;
    sprintf(sform, "%%0%dx", (nBits-1)/4+1);
    fprintf(file, "SDR %d TDI (", dataLength);
    fprintf(file, sform, JTAGWords[nWords-1]);
    for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
    fprintf(file, ") SMASK (");
    for (i = 0; i < nBits; i++) smaskBits[i] = 1;
    ArrayToWord(&smask, &i, smaskBits, nBits);
    fprintf(file, sform, smask);
    for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", 0xFFFFFFFF);
    fprintf(file, ");\n");
  }
  if (CSMOn && (CSMNextState == CMDRESETFPGA)) {
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDIDLE);
    PanelSave(CSMSetupHandle);
    LoadCSMSetupArray();
    i0 = CSMOffset+CSMSTATUSLENGTH;
    for (i = i0; i < i0+CSMSETUPLENGTH; i++) dataArray[i] = CSMSetupArray[i-i0];
    if (writeSVFFile) {
      WriteJTAGInstructions(file);
      ArrayToWord(JTAGWords, &nWords, dataArray, dataLength);
      nBits = dataLength - (nWords-1)*32;
      sprintf(sform, "%%0%dx", (nBits-1)/4+1);
      fprintf(file, "SDR %d TDI (", dataLength);
      fprintf(file, sform, JTAGWords[nWords-1]);
      for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
      fprintf(file, ") SMASK (");
      for (i = 0; i < nBits; i++) smaskBits[i] = 1;
      ArrayToWord(&smask, &i, smaskBits, nBits);
      fprintf(file, sform, smask);
      for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", 0xFFFFFFFF);
      fprintf(file, ");\n");

      WriteJTAGInstructions(file);
      ArrayToWord(JTAGWords, &nWords, dataArray, dataLength);
      nBits = dataLength - (nWords-1)*32;
      sprintf(sform, "%%0%dx", (nBits-1)/4+1);
      fprintf(file, "SDR %d TDI (", dataLength);
      fprintf(file, sform, JTAGWords[nWords-1]);
      for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
      fprintf(file, ") SMASK (");
      for (i = 0; i < nBits; i++) smaskBits[i] = 1;
      ArrayToWord(&smask, &i, smaskBits, nBits);
      fprintf(file, sform, smask);
      for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", 0xFFFFFFFF);
      fprintf(file, ");\n");
    }
    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      WaitInMillisecond(100);     // Wait for 0.1 seconds
    }
  }

  if (!writeSVFFile) return;
  WriteJTAGInstructions(file);
  ArrayToWord(JTAGWords, &nWords, dataArray, dataLength);
  nBits = dataLength - (nWords-1)*32;
  sprintf(sform, "%%0%dx", (nBits-1)/4+1);
  fprintf(file, "SDR %d TDI (", dataLength);
  fprintf(file, sform, JTAGWords[nWords-1]);
  for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
  fprintf(file, ") SMASK (");
  for (i = 0; i < nBits; i++) smaskBits[i] = 1;
  ArrayToWord(&smask, &i, smaskBits, nBits);
  fprintf(file, sform, smask);
  for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", 0xFFFFFFFF);
  fprintf(file, ") TDO (");
  fprintf(file, sform, JTAGWords[nWords-1]);
  for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
  fprintf(file, ") MASK (");
  ArrayToWord(JTAGWords, &nWords, maskArray, dataLength);
  fprintf(file, sform, JTAGWords[nWords-1]);
  for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
  fprintf(file, ");\n");*/
}


void WriteJTAGDataOnly(FILE *file, int writeMaskBits) {
  int nWords, nBits, i, smask, smaskBits[32];
  char sform[5];

  if (!writeSVFFile) return;
  ArrayToWord(JTAGWords, &nWords, dataArray, dataLength);
  nBits = dataLength - (nWords-1)*32;
  sprintf(sform, "%%0%dx", (nBits-1)/4+1);
  fprintf(file, "SDR %d TDI (", dataLength);
  fprintf(file, sform, JTAGWords[nWords-1]);
  for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
  //if (writeMaskBits) ArrayToWord(JTAGWords, &nWords, maskArray, dataLength);
  //for (i = nWords-2; i >= 0; i--) {
  //  fprintf(file, "%08x", JTAGWords[i]);
  //}
  //ArrayToWord(JTAGWords, &nWords, dataArray, dataLength);
  fprintf(file, ") SMASK (");
  for (i = 0; i < nBits; i++) smaskBits[i] = 1;
  ArrayToWord(&smask, &i, smaskBits, nBits);
  fprintf(file, sform, smask);
  for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", 0xFFFFFFFF);
  if (writeMaskBits) {
    fprintf(file, ") TDO (");
    fprintf(file, sform, JTAGWords[nWords-1]);
    for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
    fprintf(file, ") MASK (");
    ArrayToWord(JTAGWords, &nWords, maskArray, dataLength);
    fprintf(file, sform, JTAGWords[nWords-1]);
    for (i = nWords-2; i >= 0; i--) fprintf(file, "%08x", JTAGWords[i]);
  }
  fprintf(file, ");\n");
}


void PROMJTAGComments(int instruction, FILE *file) {
  char str[80], fstr[120];

  if (!writeSVFFile) return;
  if (PROMINSTLENGTH == 16) {
    if (instruction == PROMEXTEST)
      sprintf(str, "// Loading device PROM with 'EXTEST' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMSAMPLEPRELOAD)
      sprintf(str, "// Loading device PROM with 'SAMPLE/PRELOAD' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_DATA_RDPT)
      sprintf(str, "// Loading device PROM with 'XSC_DATA_RDPT' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_DATA_UC)
      sprintf(str, "// Loading device PROM with 'XSC_DATA_UC' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_DATA_CC)
      sprintf(str, "// Loading device PROM with 'XSC_DATA_CC' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_DATA_DONE)
      sprintf(str, "// Loading device PROM with 'XSC_DATA_DONE' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_DATA_CCB)
      sprintf(str, "// Loading device PROM with 'XSC_DATA_CCB' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_BLANK_CHECK)
      sprintf(str, "// Loading device PROM with 'XSC_BLANK_CHECK' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_DATA_SUCR)
      sprintf(str, "// Loading device PROM with 'XSC_DATA_SUCR' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMISC_NOOP)
      sprintf(str, "// Loading device PROM with 'ISC_NOOP' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_OP_STATUS)
      sprintf(str, "// Loading device PROM with 'XSC_OP_STATUS' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_ADDRESS_DUMP)
      sprintf(str, "// Loading device PROM with 'XSC_ADDRESS_DUMP' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMISC_ENABLE)
      sprintf(str, "// Loading device PROM with 'ISC_ENABLE' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_ENABLEC)
      sprintf(str, "// Loading device PROM with 'XSC_ENABLEC' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMISC_PROGRAM)
      sprintf(str, "// Loading device PROM with 'ISC_PROGRAM' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMISC_ADDRESS_SHIFT)
      sprintf(str, "// Loading device PROM with 'ISC_ADDRESS_SHIFT' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMISC_EREASE)
      sprintf(str, "// Loading device PROM with 'ISC_EREASE' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMISC_DATA_SHIFT)
      sprintf(str, "// Loading device PROM with 'ISC_DATA_SHIFT' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_READ)
      sprintf(str, "// Loading device PROM with 'XSC_READ' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_CONFIG)
      sprintf(str, "// Loading device PROM with 'XSC_CONFIG' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMISC_DISABLE)
      sprintf(str, "// Loading device PROM with 'ISC_DISABLE' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_MFG_READ)
      sprintf(str, "// Loading device PROM with 'XSC_MFG_READ' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_DATA_BTC)
      sprintf(str, "// Loading device PROM with 'XSC_DATA_BTC' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_CLR_STATUS)
      sprintf(str, "// Loading device PROM with 'XSC_CLR_STATUS' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_DATA_BLANK)
      sprintf(str, "// Loading device PROM with 'XSC_DATA_BLANK' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_CONFIG_PLUS)
      sprintf(str, "// Loading device PROM with 'XSC_CONFIG_PLUS' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_DATA_WRPT)
      sprintf(str, "// Loading device PROM with 'XSC_DATA_WRPT' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMISC_READ)
      sprintf(str, "// Loading device PROM with 'ISC_READ' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMCLAMP)
      sprintf(str, "// Loading device PROM with 'CLAMP' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMHIGHZ)
      sprintf(str, "// Loading device PROM with 'HIGHZ' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMUSERCODE)
      sprintf(str, "// Loading device PROM with 'USERCODE' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMIDCODE)
      sprintf(str, "// Loading device PROM with 'IDCODE' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMXSC_UNLOCK)
      sprintf(str, "// Loading device PROM with 'XSC_UNLOCK' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMBYPASS)
      sprintf(str, "// Loading device PROM with 'BYPASS' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else
      sprintf(str, "// Loading device PROM with unknown instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
  }
  else {
    if (instruction == PROMEXTEST)
      sprintf(str, "// Loading device PROM with 'EXTEST' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMSAMPLEPRELOAD)
      sprintf(str, "// Loading device PROM with 'SAMPLE/PRELOAD' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMSERASE)
      sprintf(str, "// Loading device PROM with 'SERASE' (soft erase) instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMFVFY3)
      sprintf(str, "// Loading device PROM with 'FVFY3' (frame verify) instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMISPEN)
      sprintf(str, "// Loading device PROM with 'ISPEN' (enable isp mode) instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMFPGM)
      sprintf(str, "// Loading device PROM with 'FPGM' (program) instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMFADDR)
      sprintf(str, "// Loading device PROM with 'FADDR' (frame address) instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMFERASE)
      sprintf(str, "// Loading device PROM with 'FERASE' (erase) instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMFDATA0)
      sprintf(str, "// Loading device PROM with 'FDATA0' (frame data) instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMCONFIG)
      sprintf(str, "// Loading device PROM with 'CONFIG' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMFVFY0)
      sprintf(str, "// Loading device PROM with 'FVFY0' (frame verify) instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMISPEX)
      sprintf(str, "// Loading device PROM with 'ISPEX' (exit from isp mode) instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMFDATA3)
      sprintf(str, "// Loading device PROM with 'FDATA3' (frame data) instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMCLAMP)
      sprintf(str, "// Loading device PROM with 'CLAMP' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMHIGHZ)
      sprintf(str, "// Loading device PROM with 'HIGHZ' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMUSERCODE)
      sprintf(str, "// Loading device PROM with 'USERCODE' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMIDCODE)
      sprintf(str, "// Loading device PROM with 'IDCODE' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else if (instruction == PROMBYPASS)
      sprintf(str, "// Loading device PROM with 'BYPASS' instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
    else
      sprintf(str, "// Loading device PROM with unknown instruction 0x%%0%dX", (PROMINSTLENGTH+1)/4);
  }
  sprintf(fstr, str, instruction);
  if (file == NULL) printf("%s\n", fstr);
  else fprintf(file, "%s\n", fstr);
}


void VertexIIJTAGComments(int instruction, FILE *file) {
  char str[80], chip[10];
  
  if (!writeSVFFile) return;
  strcpy(chip, "XC2V1000");
  if (XC2V2000On) strcpy(chip, "XC2V2000");
  if (instruction == VERTEXIIEXTEST)
    sprintf(str, "// Loading device %s with 'EXTEST' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIISAMPLE)
    sprintf(str, "// Loading device %s with 'SAMPLE' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIIUSER1)
    sprintf(str, "// Loading device %s with 'USER1' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIIUSER2)
    sprintf(str, "// Loading device %s with 'USER2' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIICFGOUT)
    sprintf(str, "// Loading device %s with 'CFGOUT' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIICFGIN)
    sprintf(str, "// Loading device %s with 'CFGIN' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIIINTEST)
    sprintf(str, "// Loading device %s with 'INTEST' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIIUSERCODE)
    sprintf(str, "// Loading device %s with 'USERCODE' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIIIDCODE)
    sprintf(str, "// Loading device %s with 'IDCODE' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIIHIGHZ)
    sprintf(str, "// Loading device %s with 'HIGHZ' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIIJPROGB)
    sprintf(str, "// Loading device %s with 'JPROGB' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIIJSTART)
    sprintf(str, "// Loading device %s with 'JSTART' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIIJSHUTDOWN)
    sprintf(str, "// Loading device %s with 'JSHUTDOWN' instruction 0x%02X", chip, instruction);
  else if (instruction == VERTEXIIBYPASS)
    sprintf(str, "// Loading device %s with 'BYPASS' instruction 0x%02X", chip, instruction);
  else
    sprintf(str, "// Loading device %s with unknown instruction 0x%02X", chip, instruction);
  if (file == NULL) printf("%s\n", str);
  else fprintf(file, "%s\n", str);
}


void AX1000JTAGComments(int instruction, FILE *file) {
  char str[80];
  
  if (!writeSVFFile) return;
  if (instruction == AX1000EXTEST)
    sprintf(str, "// Loading device AX1000 with 'EXTEST' instruction 0x%02X", instruction);
  else if (instruction == AX1000PRELOAD)
    sprintf(str, "// Loading device AX1000 with 'PRELOAD' instruction 0x%02X", instruction);
  else if (instruction == AX1000INTEST)
    sprintf(str, "// Loading device AX1000 with 'INTEST' instruction 0x%02X", instruction);
  else if (instruction == AX1000USERCODE)
    sprintf(str, "// Loading device AX1000 with 'USERCODE' instruction 0x%02X", instruction);
  else if (instruction == AX1000IDCODE)
    sprintf(str, "// Loading device AX1000 with 'IDCODE' instruction 0x%02X", instruction);
  else if (instruction == AX1000HIGHZ)
    sprintf(str, "// Loading device AX1000 with 'HIGHZ' instruction 0x%02X", instruction);
  else if (instruction == AX1000CLAMP)
    sprintf(str, "// Loading device AX1000 with 'CLAMP' instruction 0x%02X", instruction);
  else if (instruction == AX1000DIAGNOSTIC)
    sprintf(str, "// Loading device AX1000 with 'DIAGNOSTIC' instruction 0x%02X", instruction);
  else if (instruction == AX1000BYPASS)
    sprintf(str, "// Loading device AX1000 with 'BYPASS' instruction 0x%02X", instruction);
  else
    sprintf(str, "// Loading device AX1000 with unknown instruction 0x%02X", instruction);
  if (file == NULL) printf("%s\n", str);
  else fprintf(file, "%s\n", str);
}


void GOLJTAGComments(int instruction, FILE *file) {
  char str[80];
  
  if (!writeSVFFile) return;
  if (instruction == GOLEXTEST)
    sprintf(str, "// Loading device GOL with 'EXTEST' instruction 0x%02X", instruction);
  else if (instruction == GOLDEVICEID)
    sprintf(str, "// Loading device GOL with 'DEVICE_ID' instruction 0x%02X", instruction);
  else if (instruction == GOLCONFRW)
    sprintf(str, "// Loading device GOL with 'CONFRW' instruction 0x%02X", instruction);
  else if (instruction == GOLCONFRO)
    sprintf(str, "// Loading device GOL with 'CONFRO' instruction 0x%02X", instruction);
  else if (instruction == GOLCORETEST)
    sprintf(str, "// Loading device GOL with 'CORETEST' instruction 0x%02X", instruction);
  else if (instruction == GOLBYPASS)
    sprintf(str, "// Loading device GOL with 'BYPASS' instruction 0x%02X", instruction);
  else
    sprintf(str, "// Loading device GOL with unknown instruction 0x%02X", instruction);
  if (file == NULL) printf("%s\n", str);
  else fprintf(file, "%s\n", str);
}


void TTCrxJTAGComments(int instruction, FILE *file) {
  char str[80];
  
  if (!writeSVFFile) return;
  if (instruction == TTCRXEXTEST)
    sprintf(str, "// Loading device TTCrx with 'EXTEST' instruction 0x%02X", instruction);
  else if (instruction == TTCRXIDCODE)
    sprintf(str, "// Loading device TTCrx with 'IDCODE' instruction 0x%02X", instruction);
  else if (instruction == TTCRXINTEST)
    sprintf(str, "// Loading device TTCrx with 'INTEST' instruction 0x%02X", instruction);
  else if (instruction == TTCRXBYPASS)
    sprintf(str, "// Loading device TTCrx with 'BYPASS' instruction 0x%02X", instruction);
  else
    sprintf(str, "// Loading device TTCrx with unknown instruction 0x%02X", instruction);
  if (file == NULL) printf("%s\n", str);
  else fprintf(file, "%s\n", str);
}


void CSMJTAGComments(int instruction, FILE *file) {
  char str[80];
  
  if (!writeSVFFile) return;
  if (instruction == CSMFULLRO)
    sprintf(str, "// Loading device CSM with 'FULLRO' instruction 0x%02X", instruction);
  else if (instruction == CSMFULLRW)
    sprintf(str, "// Loading device CSM with 'FULLRW' instruction 0x%02X", instruction);
  else if (instruction == CSMIDCODE)
    sprintf(str, "// Loading device CSM with 'IDCODE' instruction 0x%02X", instruction);
  else if (instruction == CSMANYUSER)
    sprintf(str, "// Loading device CSM with 'ANYUSER' instruction 0x%02X", instruction);
  else if (instruction == CSMTTCRXSTATUS)
    sprintf(str, "// Loading device CSM with 'TTCRXSTATUS' instruction 0x%02X", instruction);
  else if (instruction == CSMAMTPARITYERROR)
    sprintf(str, "// Loading device CSM with 'AMTPARITYERROR' instruction 0x%02X", instruction);
  else if (instruction == CSMCSMSTATUS)
    sprintf(str, "// Loading device CSM with 'CSMSTATUS' instruction 0x%02X", instruction);
  else if (instruction == CSMAMTPHASEERROR)
    sprintf(str, "// Loading device CSM with 'AMTPHASEERROR' instruction 0x%02X", instruction);
  else if (instruction == CSMCONFIGRO)
    sprintf(str, "// Loading device CSM with 'CONFIGRO' instruction 0x%02X", instruction);
  else if (instruction == CSMCONFIGRW)
    sprintf(str, "// Loading device CSM with 'CONFIGRW' instruction 0x%02X", instruction);
  else if (instruction == CSMCSMPARMRO)
    sprintf(str, "// Loading device CSM with 'CSMPARMRO' instruction 0x%02X", instruction);
  else if (instruction == CSMCSMPARMRW)
    sprintf(str, "// Loading device CSM with 'CSMPARMRW' instruction 0x%02X", instruction);
  else if (instruction == CSMVERSIONDATE)
    sprintf(str, "// Loading device CSM with 'VERSIONDATE' instruction 0x%02X", instruction);
  else if (instruction == CSMROBITS)
    sprintf(str, "// Loading device CSM with 'ROBITS' instruction 0x%02X", instruction);
  else if (instruction == CSMAMTFULLPHASE)
    sprintf(str, "// Loading device CSM with 'AMTFULLPHASE' instruction 0x%02X", instruction);
  else if (instruction == CSMBYPASS)
    sprintf(str, "// Loading device CSM with 'BYPASS' instruction 0x%02X", instruction);
  else
    sprintf(str, "// Loading device CSM with unknown instruction 0x%02X", instruction);
  if (file == NULL) printf("%s\n", str);
  else fprintf(file, "%s\n", str);
}

//Modified by Xiangting

void A3P250JTAGComments(int instruction, FILE *file, int mezzNumber) {
	char str[80];
	if (!writeSVFFile) return;
	if (instruction == A3P250UJTAG_IDCODE)
		sprintf(str, "// Loading device A3P250 with 'IDCODE' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == A3P250ASD_WRITE)
		sprintf(str, "// Loading device A3P250 with 'ASD_WRITE' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == A3P250ASD_READ)
		sprintf(str, "// Loading device A3P250 with 'ASD_READ' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == A3P250FPGA_CW)
		sprintf(str, "// Loading device A3P250 with 'FPGA_CW' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == A3P250FPGA_CR)
		sprintf(str, "// Loading device A3P250 with 'FPGA_CR' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == A3P250FPGA_SR)
		sprintf(str, "// Loading device A3P250 with 'FPGA_SR' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == A3P250I2C_MS_W)
		sprintf(str, "// Loading device A3P250 with 'I2C_MS_W' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == A3P250I2C_MS_R)
		sprintf(str, "// Loading device A3P250 with 'I2C_MS_R' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == A3P250BYPASS)
		sprintf(str, "// Loading device A3P250 with 'BYPASS' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else
		sprintf(str, "// Loading device A3P250 with unknown instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	if (file == NULL) printf("%s\n", str);
	else fprintf(file, "%s\n", str);

}

void HPTDCJTAGComments(int instruction, FILE *file, int mezzNumber) {
	char str[80];
	if (!writeSVFFile) return;
	if (instruction == HPTDCEXTEST)
		sprintf(str, "// Loading device HPTDC with 'HPTDCEXTEST' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCIDCODE)
		sprintf(str, "// Loading device HPTDC with 'HPTDCIDCODE' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCSAMPLE)
		sprintf(str, "// Loading device HPTDC with 'HPTDCSAMPLE' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCINTEST)
		sprintf(str, "// Loading device HPTDC with 'HPTDCINTEST' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCBIST)
		sprintf(str, "// Loading device HPTDC with 'HPTDCBIST' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCSCAN)
		sprintf(str, "// Loading device HPTDC with 'HPTDCSCAN' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCREADOUT)
		sprintf(str, "// Loading device HPTDC with 'HPTDCREADOUT' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCSETUP)
		sprintf(str, "// Loading device HPTDC with 'HPTDCSETUP' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCCONTROL)
		sprintf(str, "// Loading device HPTDC with 'HPTDCCONTROL' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCSTATUS)
		sprintf(str, "// Loading device HPTDC with 'HPTDCSTATUS' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCCORETEST)
		sprintf(str, "// Loading device HPTDC with 'HPTDCCORETEST' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else if (instruction == HPTDCBYPASS)
		sprintf(str, "// Loading device HPTDC with 'HPTDCBYPASS' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	else
		sprintf(str, "// Loading device HPTDC with unknown instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
	if (file == NULL) printf("%s\n", str);
	else fprintf(file, "%s\n", str);

}


//END

void AMTJTAGComments(int instruction, FILE *file, int mezzNumber) {
  char str[80];
  
  if (!writeSVFFile) return;
  if (instruction == AMTEXTEST)
    sprintf(str, "// Loading device AMT with 'EXTEST' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else if (instruction == AMTIDCODE)
    sprintf(str, "// Loading device AMT with 'IDCODE' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else if (instruction == AMTSAMPLE)
    sprintf(str, "// Loading device AMT with 'SAMPLE' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else if (instruction == AMTINTEST)
    sprintf(str, "// Loading device AMT with 'INTEST' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else if (instruction == AMTCONTROL)
    sprintf(str, "// Loading device AMT with 'CONTROL' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else if (instruction == AMTASDCONTROL)
    sprintf(str, "// Loading device AMT with 'ASDCONTROL' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else if (instruction == AMTSTATUS)
    sprintf(str, "// Loading device AMT with 'STATUS' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else if (instruction == AMTCORETEST)
    sprintf(str, "// Loading device AMT with 'CORETEST' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else if (instruction == AMTBIST)
    sprintf(str, "// Loading device AMT with 'BIST' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else if (instruction == AMTGOUTPORT)
    sprintf(str, "// Loading device AMT with 'GOUTPORT' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else if (instruction == AMTBYPASS)
    sprintf(str, "// Loading device AMT with 'BYPASS' instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  else
    sprintf(str, "// Loading device AMT with unknown instruction 0x%02X for mezz. card %d", instruction, mezzNumber);
  if (file == NULL) printf("%s\n", str);
  else fprintf(file, "%s\n", str);
}


void JTAGComments(int instruction, FILE *file, int IDCode) {
  if (!writeSVFFile) return;
  if (IDCode == PROMID) PROMJTAGComments(instruction, file);
  else if ((IDCode&VERTEXIIIDMASK) == XC2V1000ID) VertexIIJTAGComments(instruction, file);
  else if ((IDCode&VERTEXIIIDMASK) == XC2V2000ID) VertexIIJTAGComments(instruction, file);
  else if (IDCode == AX1000ID) AX1000JTAGComments(instruction, file);
  else if (IDCode == GOLID) GOLJTAGComments(instruction, file);
  else if (IDCode == TTCRXID) TTCrxJTAGComments(instruction, file);
  else if (IDCode == CSM1ID) CSMJTAGComments(instruction, file);
  else if (IDCode == CSM2ID) CSMJTAGComments(instruction, file);
  else if (IDCode == CSM3ID) CSMJTAGComments(instruction, file);
  else if (IDCode == CSMID) CSMJTAGComments(instruction, file);
//  else if (IDCode == AMT2ID) AMTJTAGComments(instruction, file, -1);
//  else if (IDCode == AMT3ID) AMTJTAGComments(instruction, file, -1);
  
//Modified by Xiangting
  else if (IDCode == HPTDCID) HPTDCJTAGComments(instruction, file, -1);
  else if (IDCode == A3P250ID) A3P250JTAGComments(instruction, file, -1);

//End  
  else if (file == NULL)
    printf("// Loading unknown device with unknown instruction 0x%02X\n", instruction);
  else
    fprintf(file, "// Loading unknown device with unknown instruction 0x%02X\n", instruction);
}


// Function for programming PROM and FPGA (VertexII)
//
void ProgramVertexIIFPGAButton(void) {
  int i, gotFile;
  
  if (controlOption == NORM) {
    CheckCSMDAQEanbleAndMezzJTAGEnable();
    if (strlen(bitFilePath) <= 0) strcpy(fileDirType, "bitFiles\\");
    else {
      strcpy(fileDirType, bitFilePath);
      for (i = strlen(fileDirType); i >= 0; i--) {
        if (fileDirType[i] != '\\') fileDirType[i] = '\0';
        else break;
      }
    }
    strcat(fileDirType, "*.bit");
    gotFile = FileSelectPopup("", fileDirType, "", "Vertex II Program File (.bit file)",
                              VAL_LOAD_BUTTON, 0, 1, 1, 0, bitFilePath);
    if (gotFile) ProgramVertexIIFPGA();
    else printf("Program Vertex II Chip is cancelled.\n");
  }
  else if (controlOption == HELP) {
    MessagePopup("Program Vertex II Chip", 
                 "Program Vertex II Chip through JTAG by using its bit file.");
    controlOption = NORM;
  }
}


void ProgramVertexIIFPGA(void) {
  int bitData[256], data, totalBits, nSteps, nwords, i, endData, sleepPolicy, JTAGDimmed, INITDimmed;
  float percentage;
  char path[256], str[256], message[128];
  FILE *SVFFile, *bitstream;
  
  strcpy(bitStreamTextFName, "bitstreamFromBITFile.txt");
  SaveParameters(PARABITSTREAMFILE);
  if (VerifyBitFile(1) != 0) return;
  if (bitstream = fopen(bitStreamTextFName, "r")) {
    strcpy(path, "programFPGA.svf");
    if (SVFFile = fopen(path, "w")) {
      useMessageBox = TRUE;
      sleepPolicy = GetSleepPolicy();
      SetSleepPolicy(VAL_SLEEP_NONE);
      GetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, &JTAGDimmed);
      SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, TRUE);
      GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, &INITDimmed);
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
      GlobalEnableSerial();
      sprintf(message, "Starting to program FPGA, shutdown FPGA first ...\n");
      ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
      printf("%s\n", message);
      printf("The FPGA program file is <%s>\n", bitFilePath);
      ShutdownFPGA();
      if (action == DOWNLOAD) ResetTAP();
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      FPGAInstruction(SVFFile, VERTEXIIBYPASS, 12);
      FPGAInstruction(SVFFile, VERTEXIICFGIN, 0);
      bitData[0] = XPHTYPE1WRITE | (VERTEXICRADDRCMD << XPHREGADDRESSBIT0LOCATION) | 1;
      bitData[1] = VERTEXCMDAGHIGH;
      bitData[2] = 0;
      bitData[3] = 0;
      for (nwords = 0; nwords < 4; nwords++) {
        data = bitData[nwords];
        bitData[nwords] = 0;
        for (i = 0; i < 32; i++) bitData[nwords] |= (((data >> (31-i)) & 1) << i);
      }
      FPGAData(SVFFile, 4*32, bitData, 0, 0, TRUE);
      FPGAInstruction(SVFFile, VERTEXIICFGIN, 0);
      nSteps = 0;
      totalBits = 0;
      nwords = 0;
      while (TRUE) {
        while (nwords < 128) {
          if (feof(bitstream) != 0) {
            nwords--;
            break;
          }
          fscanf(bitstream, "%08x", &data);
          bitData[nwords] = 0;
          for (i = 0; i < 8; i++) bitData[nwords] |= (((data >> 4*(7-i)) & 0xF) << (4*i));
          nwords++;
        }
        if (nwords <= 0) break;
        nSteps++;
        totalBits += (32*nwords);
        if ((totalBits/8) >= nBitstreamBytes) endData = TRUE;
        else endData = FALSE;
        FPGAData(SVFFile, 32*nwords, bitData, 0, 0, endData);
        nwords = 0;
        percentage = 100.0 * (((float) (totalBits / 8)) / ((float) nBitstreamBytes));
        sprintf(message, "It takes while to program FPGA ..., %.0f percentage is done.\n", percentage);
        ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
        HandleUserButtonAndFunction(PanelDone);
      }
      if (action == DOWNLOAD) ResetTAP();
      FPGAInstruction(SVFFile, VERTEXIIJSTART, 32);
      if (action == DOWNLOAD) ResetTAP();
      printf("Reset FPAG DCM by performing AGHIGH\n");
      FPGAInstruction(SVFFile, VERTEXIICFGIN, 0);
      bitData[0] = XPHTYPE1WRITE | (VERTEXICRADDRCMD << XPHREGADDRESSBIT0LOCATION) | 1;
      bitData[1] = VERTEXCMDAGHIGH;
      bitData[2] = 0;
      bitData[3] = 0;
      for (nwords = 0; nwords < 4; nwords++) {
        data = bitData[nwords];
        bitData[nwords] = 0;
        for (i = 0; i < 32; i++) bitData[nwords] |= (((data >> (31-i)) & 1) << i);
      }
      FPGAData(SVFFile, 4*32, bitData, 0, 36, TRUE);
      FPGAInstruction(SVFFile, VERTEXIICFGIN, 0);
      bitData[0] = XPHTYPE1WRITE | (VERTEXICRADDRCMD << XPHREGADDRESSBIT0LOCATION) | 1;
      bitData[1] = VERTEXCMDDGHIGH;
      bitData[2] = 0;
      bitData[3] = 0;
      for (nwords = 0; nwords < 4; nwords++) {
        data = bitData[nwords];
        bitData[nwords] = 0;
        for (i = 0; i < 32; i++) bitData[nwords] |= (((data >> (31-i)) & 1) << i);
      }
      FPGAData(SVFFile, 4*32, bitData, 0, 36, TRUE);
      if (action == DOWNLOAD) ResetTAP();
      FPGAInstruction(SVFFile, VERTEXIIBYPASS, 0);
      FPGAData(SVFFile, 1, bitData, 0, 0, TRUE);
      FPGAData(SVFFile, 1, bitData, 0, 0, TRUE);
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
      firstTimeVerifyFPGA = TRUE;
      sprintf(message, "Total %d bits have been programmed into FPGA with %d steps.\n", totalBits, nSteps);
      ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
      printf("%s\n", message);
      if (action == DOWNLOAD) {
        ResetTAP();
        WaitInMillisecond(1200);        // Wait 1.2s after issued the program command 
        SetupJTAGChain();
        if (gotCSM) ResetCSM();
      }
      ClearSetupStatus();
      SetSleepPolicy(sleepPolicy);
      SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, JTAGDimmed);
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
      GlobalDisableSerial();
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
    fclose(bitstream);
  }
  else {
    printf(".bitstream file <%s> does not exist.\n", bitStreamTextFName);
    printf("Can not program VertexII chip, please check the file.\n");
  }
}


void ProgramVertexIIFPGAFromPROM(void) {
  char path[256];
  FILE *SVFFile;

  if (controlOption == NORM) {
    strcpy(path, "programVertexIIFPGAFromPROM.svf");
    if (SVFFile = fopen(path, "w")) {
      PROMInstruction(SVFFile, -1, 0);
      PROMInstruction(SVFFile, PROMBYPASS, 0);
      PROMInstruction(SVFFile, PROMCONFIG, 0);
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
      printf("JTAG command is issued to program VertexII Chip.\n");
      strcpy(bitStreamTextFName, "bitstreamFromMCSFile.txt");
      SaveParameters(PARABITSTREAMFILE);
      if (strlen(mcsFilePath) > 4) {
        strncpy(maskFilePath, mcsFilePath, strlen(mcsFilePath)-4);
        maskFilePath[strlen(mcsFilePath)-4] = '\0';
        strcat(maskFilePath, ".msk");
        strncpy(msdFilePath, mcsFilePath, strlen(mcsFilePath)-4);
        msdFilePath[strlen(mcsFilePath)-4] = '\0';
        strcat(msdFilePath, ".msd");
        strncpy(rbdFilePath, mcsFilePath, strlen(mcsFilePath)-4);
        rbdFilePath[strlen(mcsFilePath)-4] = '\0';
        strcat(rbdFilePath, ".rbd");
        strncpy(rbbFilePath, mcsFilePath, strlen(mcsFilePath)-4);
        rbbFilePath[strlen(mcsFilePath)-4] = '\0';
        strcat(rbbFilePath, ".rbb");
      }
      WaitInMillisecond(1000);          // Wait 1.0s after issued the program command 
      if (action == DOWNLOAD) {
        if (verifyFPGAAndPROM) {
          ResetTAP();
          WaitInMillisecond(100);      // Wait 0.1s after issued the program command
        }
        else {
          WaitInMillisecond(1000);          // Wait 1.0s after issued the program command 
          SetupJTAGChain();
        }
        ClearSetupStatus();
      }
      firstTimeVerifyFPGA = TRUE;
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Program VertexII Chip from PROM", 
                 "Issue a JTAG Command to PROM to program VertexII Chip.");
    controlOption = NORM;
  }
}


void ProgramPROMButton(void) {
  int panel, i, gotFile;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    if (panel == ConfirmVFPHandle) PanelDone();
    CheckCSMDAQEanbleAndMezzJTAGEnable();
    if (strlen(mcsFilePath) <= 0) strcpy(fileDirType, "bitFiles\\");
    else {
      strcpy(fileDirType, mcsFilePath);
      for (i = strlen(fileDirType); i >= 0; i--) {
        if (fileDirType[i] != '\\') fileDirType[i] = '\0';
        else break;
      }
    }
    strcat(fileDirType, "*.mcs");
    gotFile = FileSelectPopup("", fileDirType, "", "PROM (18V04) Program File (.mcs file)",
                              VAL_LOAD_BUTTON, 0, 1, 1, 0, mcsFilePath);
    if (gotFile) {
      confirmVerify = TRUE;
      ProgramPROM();
    }
    else {
      confirmVerify = FALSE;
      printf("Program PROM 18V04 is cancelled.\n");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Program PROM 18V04", 
                 "Program PROM 18V04 Chip through JTAG by using its mcs file.");
    controlOption = NORM;
  }
}


void ProgramPROM(void) {
  int bitData[512], first, fstatus, data, nbits, i, i0, j, strDone, totalBits, nSteps, length;
  int sleepPolicy, JTAGDimmed, INITDimmed;
  float lineNumber, oldLineNumber, numberLines;
  char path[256], str[256], message[128];
  FILE *SVFFile, *mcsFile, *textFile;
  
  strcpy(bitStreamTextFName, "bitstreamFromMCSFile.txt");
  if (strlen(mcsFilePath) <= 0) {
    printf("No MCS file is defined, can not program PROM.\n");
    return;
  }
  if (mcsFile = fopen(mcsFilePath, "r")) {
    // Check MCS file first to make sure
    numberLines = 0.0;
    while (TRUE) {
      fstatus = ferror(mcsFile);
      if (fstatus != 0) {
        printf("*** Error in reading MCS file with fstats=0x%X!\n", fstatus);
        printf("*** Program PROM will be aborted.\n"); 
        return;
      }
      if (feof(mcsFile) != 0) break;
      fscanf(mcsFile, "%s", str);
      numberLines += 1.0;
      if (str[0] != ':') fstatus = -1;
      else {
        sscanf(&str[1], "%2x", &data);
        if ((data != 0x02) && (data != 0x10) && (data != 0x04) && (data != 0x0c) && (data != 0x00)) fstatus = -2;
      }
      if (fstatus < 0) {
        printf("%s", str);
        printf("0x%02x\n", data);
        printf("\n");
        printf("------- Abort Programming PROM ------\n");
        printf("The file <%s> is not good MCS file.\n", mcsFilePath);
        printf("Program PROM will be aborted.\n");
        printf("Please check and replace the MCS file.\n"); 
        printf("\n");
        return;    
      }
    }
    rewind(mcsFile);
    lineNumber = 0.0;
    oldLineNumber = lineNumber;
    strcpy(path, "programPROM.svf");
    if (SVFFile = fopen(path, "w")) {
      useMessageBox = TRUE;
      sleepPolicy = GetSleepPolicy();
      SetSleepPolicy(VAL_SLEEP_NONE);
      GetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, &JTAGDimmed);
      SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, TRUE);
      GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, &INITDimmed);
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
//      GlobalEnableSerial();
      length = 0;
      textFile = fopen(bitStreamTextFName, "w");
      sprintf(message, "Starting to program PROM, takes a while ...\n");
      ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
      printf("%s\n", message);
      printf("The PROM program file is <%s>\n", mcsFilePath);
      PROMInstruction(SVFFile, -1, 0);
//      PROMInstruction(SVFFile, PROMBYPASS, 0);
//      BinaryToInt(&data, PROMOffset, PROMINSTLENGTH, readbackArray);
//      PROMInstruction(SVFFile, PROMIDCODE, 0);
//      bitData[0] = 0;
//      PROMData(SVFFile, 32, bitData, 0, 0);
//      BinaryToInt(&data, PROMOffset, 32, readbackArray);
//      if (PROMINSTLENGTH != 16) {
//        PROMInstruction(SVFFile, PROMISPEX, 110000);
//        PROMInstruction(SVFFile, PROMBYPASS, 0);
//        BinaryToInt(&data, PROMOffset, PROMINSTLENGTH, readbackArray);
//        PROMInstruction(SVFFile, PROMBYPASS, 0);
//        PROMInstruction(SVFFile, PROMIDCODE, 0);
//        bitData[0] = 0;
//        PROMData(SVFFile, 32, bitData, 0, 0);
//        BinaryToInt(&data, PROMOffset, 32, readbackArray);
//        PROMInstruction(SVFFile, PROMISPEX, 110000);
//        PROMInstruction(SVFFile, PROMBYPASS, 0);
//        BinaryToInt(&data, PROMOffset, PROMINSTLENGTH, readbackArray);
//        PROMInstruction(SVFFile, PROMBYPASS, 0);
//        bitData[0] = 0;
//        PROMData(SVFFile, 1, bitData, 0, 0);
//        PROMInstruction(SVFFile, PROMBYPASS, 0);
//        BinaryToInt(&data, PROMOffset, PROMINSTLENGTH, readbackArray);
//        if (writeSVFFile) {
//          fprintf(SVFFile, "STATE RESET;\n");
//        }
//        if (action == DOWNLOAD) ResetTAP();
//      }
//      
      if (PROMINSTLENGTH == 16) {
        PROMInstruction(SVFFile, PROMISC_ENABLE, 0);
        bitData[0] = 0x3;
        PROMData(SVFFile, 8, bitData, 0, 0);
        PROMInstruction(SVFFile, PROMXSC_UNLOCK, 0);
        bitData[0] = 0x31;
        PROMData(SVFFile, 24, bitData, 0, 0);
        PROMInstruction(SVFFile, PROMISC_EREASE, 1);
        bitData[0] = 0x31;
        PROMData(SVFFile, 24, bitData, 0, 0);
        // Wait 10 seconds to erase PROM
        sprintf(message, "Erase PROM has been issued, where 10 seconds waiting is needed, be patient...\n");
        ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
        HandleUserButtonAndFunction(PanelDone);
        PROMInstruction(SVFFile, -3, 10000);
        PROMInstruction(SVFFile, PROMIDCODE, 0);
        bitData[0] = 0;
        PROMData(SVFFile, 32, bitData, 0, 0);
        BinaryToInt(&data, PROMOffset, 32, readbackArray);
        PROMInstruction(SVFFile, PROMISC_ENABLE, 0);
        bitData[0] = 0x3;
        PROMData(SVFFile, 8, bitData, 0, 0);
        PROMInstruction(SVFFile, PROMXSC_DATA_BTC, 0);
        bitData[0] = 0xffffffe0;
        PROMData(SVFFile, 32, bitData, 0, 0);
      }
      else {
        PROMInstruction(SVFFile, PROMISPEN, 0);
        bitData[0] = 0x34;
        PROMData(SVFFile, 6, bitData, 0, 0);
        PROMInstruction(SVFFile, PROMFADDR, 0);
        bitData[0] = 0x1;
        PROMData(SVFFile, 16, bitData, 0, 1);
        PROMInstruction(SVFFile, -2, 1);
        PROMInstruction(SVFFile, PROMFERASE, 1);
        // Wait 15 seconds to erase PROM
        sprintf(message, "Erase PROM has been issued, where 15 seconds waiting is needed, be patient...\n");
        ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
        HandleUserButtonAndFunction(PanelDone);
        PROMInstruction(SVFFile, -3, 15000);
        PROMInstruction(SVFFile, PROMISPEX, 1);
        PROMInstruction(SVFFile, -3, 110);    // Wait 110 milliseconds
//        PROMInstruction(SVFFile, PROMISPEX, 110000);
        if (writeSVFFile) {
          fprintf(SVFFile, "STATE RESET;\n");
        }
        if (action == DOWNLOAD) ResetTAP();
        PROMInstruction(SVFFile, PROMISPEN, 0);
        bitData[0] = 0x34;
        PROMData(SVFFile, 6, bitData, 0, 0);
      }

      nSteps = 0;
      totalBits = 0;
      first = TRUE;
      nbits = 0;
      j = 0;
      strDone = TRUE;
      while (TRUE) {
        if (PROMINSTLENGTH == 16) bitData[511] = 256;
        else bitData[511] = 4096;
        for (i = 0; i < 256; i++) bitData[i] = 0;
        while (nbits < bitData[511]) {
          if (feof(mcsFile) != 0) break;
          if (strDone) {
            i0 = 7;
            fscanf(mcsFile, "%s", str);
            lineNumber += 1.0;
          }
          sscanf(&str[1], "%2x", &data);
          if ((data == 0x10) || (data == 0x04) || (data == 0x0C)) {
            for (i = i0+2; i < strlen(str)-2; i += 2) {
              sscanf(&str[i], "%02x", &data);
              fprintf(textFile, "%01x%01x", data&0xF, (data>>4)&0xF);
              length++;
              if (length%40 == 0) fprintf(textFile, "\n");
              bitData[j/4] |= ((data&0xFF) << (8*(j%4)));
              nbits += 8;
              if (nbits == bitData[511]) {
                j = 0;
                i0 = i;
                strDone = FALSE;
                break;
              }
              else j++;
            }
          }
          strDone = TRUE;
        }
        if (nbits <= 0) break;
        while (nbits < bitData[511]) {
          bitData[j/4] |= (0xFF << (8*(j%4)));
          nbits += 8;
          if (nbits == bitData[511]) {
            j = 0;
            break;
          }
          else j++;
        }
        nSteps++;
        totalBits += nbits;
        if (PROMINSTLENGTH == 16) {
          if (first) {
            PROMInstruction(SVFFile, PROMISC_PROGRAM, 120);
            PROMInstruction(SVFFile, PROMBYPASS, 0);
          }
          else {
            PROMInstruction(SVFFile, PROMISC_PROGRAM, 1);
            PROMInstruction(SVFFile, -3, 1);    // Wait 1 milliseconds
          }
          PROMInstruction(SVFFile, PROMISC_DATA_SHIFT, 0);
          PROMData(SVFFile, nbits, bitData, 0, 0);
          if (first) {
            first = FALSE;
            PROMInstruction(SVFFile, PROMISC_ADDRESS_SHIFT, 0);
            bitData[0] = 0x0;
            PROMData(SVFFile, 24, bitData, 0, 0);
          }
        }
        else {
          PROMInstruction(SVFFile, PROMFDATA0, 0);
          PROMData(SVFFile, nbits, bitData, 0, 1);
          PROMInstruction(SVFFile, -2, 1);
          if (first) {
            first = FALSE;
            PROMInstruction(SVFFile, PROMFADDR, 0);
            bitData[0] = 0x0;
            PROMData(SVFFile, 16, bitData, 0, 1);
            PROMInstruction(SVFFile, -2, 1);
          }
          PROMInstruction(SVFFile, PROMFPGM, 1);
          PROMInstruction(SVFFile, -3, 14);     // Wait 14 milliseconds
        }
        nbits = 0;
        if ((numberLines > 0.0) && ((lineNumber-oldLineNumber) > 500)) {
          oldLineNumber = lineNumber;
          sprintf(message, "It takes while to program PROM ..., %.0f percentage is done.\n", 100.0*lineNumber/numberLines);
          ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
          HandleUserButtonAndFunction(PanelDone);
        }
      }
      nBitstreamBytes = length;
      SaveParameters(PARABITSTREAMLENGTH);
      fclose(textFile);
      if (PROMINSTLENGTH == 16) {
        PROMInstruction(SVFFile, PROMISC_PROGRAM, 1);
        PROMInstruction(SVFFile, -3, 1);      // Wait 1 milliseconds
        PROMInstruction(SVFFile, PROMXSC_OP_STATUS, 0);
        bitData[0] = 0x0;
        PROMData(SVFFile, 8, bitData, 0, 0);
        PROMInstruction(SVFFile, PROMXSC_DATA_SUCR, 0);
        bitData[0] = 0xfffc;
        PROMData(SVFFile, 16, bitData, 0, 0);
        PROMInstruction(SVFFile, PROMISC_PROGRAM, 60);
        PROMInstruction(SVFFile, PROMXSC_DATA_CCB, 0);
        bitData[0] = 0xfff9;
        PROMData(SVFFile, 16, bitData, 0, 0);
        PROMInstruction(SVFFile, PROMISC_PROGRAM, 60);
        PROMInstruction(SVFFile, PROMXSC_DATA_DONE, 0);
        bitData[0] = 0xce;
        PROMData(SVFFile, 8, bitData, 0, 0);
        PROMInstruction(SVFFile, PROMISC_PROGRAM, 60);
        PROMInstruction(SVFFile, PROMISC_DISABLE, 50);
        PROMInstruction(SVFFile, PROMBYPASS, 0);
      }
      else {
        PROMInstruction(SVFFile, PROMFADDR, 0);
        bitData[0] = 0x1;
        PROMData(SVFFile, 16, bitData, 0, 1);
        PROMInstruction(SVFFile, PROMSERASE, 1);
        PROMInstruction(SVFFile, -3, 37);     // Wait 37 milliseconds
//        PROMInstruction(SVFFile, PROMSERASE, 37000);
        PROMInstruction(SVFFile, PROMISPEX, 1);
        PROMInstruction(SVFFile, -3, 11);     // Wait 11 milliseconds
//        PROMInstruction(SVFFile, PROMISPEX, 11000);
      }
      PROMInstruction(SVFFile, PROMBYPASS, 0);
      PROMData(SVFFile, 1, bitData, 0, 0);
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
      sprintf(message, "Total %d bits have been programmed into PROM with %d steps.\n", totalBits, nSteps);
      ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
      printf("%s\n", message);
      SetupJTAGChain();
      if (gotPROM) if (VerifyPROM() == 0) ProgramVertexIIFPGAFromPROM();
      else if (!(gotXC2V1000 || gotXC2V2000)) ProgramVertexIIFPGAFromPROM();
      SetSleepPolicy(sleepPolicy);
      SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, JTAGDimmed);
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
//      GlobalDisableSerial();
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
    fclose(mcsFile);
  }
  else {
    printf(".mcs file <%s> does not exist.\n", mcsFilePath);
    printf("Can not program PROM, please check .mcs file.\n");
  }
}


void GetFPGAConfigurationStatus(void) {
  int bitData[256], nwords, i, data;
  char path[256];
  FILE *SVFFile;

  if (controlOption == NORM) {
    strcpy(path, "getFPGAConfigurationStatus.svf");
    if (SVFFile = fopen(path, "w")) {
      if (action == DOWNLOAD) ResetTAP();
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      FPGAInstruction(SVFFile, VERTEXIIBYPASS, 0);
      FPGAInstruction(SVFFile, VERTEXIICFGIN, 0);
      bitData[0] = 0xffffffff;
      bitData[1] = 0xaa995566;
      bitData[2] = XPHTYPE1READ | (VERTEXICRADDRSTATUS << XPHREGADDRESSBIT0LOCATION) | 2;
      bitData[3] = 0;
      bitData[4] = 0;
      bitData[5] = 0;
      for (nwords = 0; nwords < 6; nwords++) {
        data = bitData[nwords];
        bitData[nwords] = 0;
        for (i = 0; i < 32; i++) bitData[nwords] |= (((data >> (31-i)) & 1) << i);
      }
      FPGAData(SVFFile, 6*32, bitData, 0, 0, TRUE);
      FPGAInstruction(SVFFile, VERTEXIICFGOUT, 0);
      data = 0x000018ec;
      bitData[0] = 0;
      for (i = 0; i < 32; i++) bitData[0] |= (((data >> (31-i)) & 1) << i);
      FPGAData(SVFFile, 32, bitData, 1, 0, TRUE);
      BinaryToInt(&bitData[1], vertexIIOffset, 32, readbackArray);
      if (bitData[0] != bitData[1]) {
        printf("Wrong FPGA Configuration Status: expected=0x%08x got=0x%08x\n", bitData[0], bitData[1]);
      }
      FPGAInstruction(SVFFile, VERTEXIICFGIN, 0);
      bitData[0] = XPHTYPE1WRITE | (VERTEXICRADDRCMD << XPHREGADDRESSBIT0LOCATION) | 2;
      bitData[1] = VERTEXCMDRCRC;
      bitData[2] = 0;
      bitData[3] = 0;
      for (nwords = 0; nwords < 4; nwords++) {
        data = bitData[nwords];
        bitData[nwords] = 0;
        for (i = 0; i < 32; i++) bitData[nwords] |= (((data >> (31-i)) & 1) << i);
      }
      FPGAData(SVFFile, 4*32, bitData, 0, 0, TRUE);
      FPGAInstruction(SVFFile, VERTEXIIBYPASS, 0);
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      if (action == DOWNLOAD) ResetTAP();
      fclose(SVFFile);
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Read FPGA Configuration Status", 
                 "Push this button to read FPGA configuration status register.");
    controlOption = NORM;
  }
}


void ShutdownFPGA(void) {
  int bitData[256], nwords, i, data;
  char path[256];
  FILE *SVFFile;

  if (controlOption == NORM) {
    strcpy(path, "shutdownFPGA.svf");
    if (SVFFile = fopen(path, "w")) {
      if (action == DOWNLOAD) ResetTAP();
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      FPGAInstruction(SVFFile, VERTEXIIBYPASS, 12);
      FPGAInstruction(SVFFile, VERTEXIICFGIN, 0);
      bitData[0] = 0xffffffff;
      bitData[1] = 0xaa995566;
      bitData[2] = XPHTYPE1WRITE | (VERTEXICRADDRCMD << XPHREGADDRESSBIT0LOCATION) | 1;
      bitData[3] = VERTEXCMDRCRC;
      bitData[4] = 0;
      bitData[5] = 0;
      for (nwords = 0; nwords < 6; nwords++) {
        data = bitData[nwords];
        bitData[nwords] = 0;
        for (i = 0; i < 32; i++) bitData[nwords] |= (((data >> (31-i)) & 1) << i);
      }
      FPGAData(SVFFile, 6*32, bitData, 0, 0, TRUE);
      FPGAInstruction(SVFFile, VERTEXIIJSHUTDOWN, 36);
      FPGAInstruction(SVFFile, VERTEXIIBYPASS, 12);
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
      printf("Shutdown FPGA is sent out via JTAG.\n");
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("FPGA shutdown", 
                 "Push this button to perform FPGA shutdown sequence.");
    controlOption = NORM;
  }
}


void VerifyVertexIIFPGAButton(void) {
  int i, gotFile;
  
  if (controlOption == NORM) {
    CheckCSMDAQEanbleAndMezzJTAGEnable();
    if (strlen(bitFilePath) <= 0) strcpy(fileDirType, "bitFiles\\");
    else {
      strcpy(fileDirType, bitFilePath);
      for (i = strlen(fileDirType); i >= 0; i--) {
        if (fileDirType[i] != '\\') fileDirType[i] = '\0';
        else break;
      }
    }
    strcat(fileDirType, "*.bit");
    gotFile = FileSelectPopup("", fileDirType, "", "Vertex II Program File (.bit file)",
                              VAL_LOAD_BUTTON, 0, 1, 1, 0, bitFilePath);
    if (gotFile) {
      strcpy(bitStreamTextFName, "bitstreamFromBITFile.txt");
      SaveParameters(PARABITSTREAMFILE);
      if (VerifyBitFile(1) != 0) return;
      VerifyVertexIIFPGA();
    }
    else printf("Verify Vertex II Chip is cancelled.\n");
  }
  else if (controlOption == HELP) {
    MessagePopup("Verify Vertex II Chip", 
                 "Verify Vertex II Chip through JTAG by using its bit file.");
    controlOption = NORM;
  }
}


void VerifyPROMButton(void) {
  int i, gotFile;
  
  if (controlOption == NORM) {
    CheckCSMDAQEanbleAndMezzJTAGEnable();
    if (strlen(mcsFilePath) <= 0) strcpy(fileDirType, "bitFiles\\");
    else {
      strcpy(fileDirType, mcsFilePath);
      for (i = strlen(fileDirType); i >= 0; i--) {
        if (fileDirType[i] != '\\') fileDirType[i] = '\0';
        else break;
      }
    }
    strcat(fileDirType, "*.mcs");
    gotFile = FileSelectPopup("", fileDirType, "", "PROM (18V04) Program File (.mcs file)",
                              VAL_LOAD_BUTTON, 0, 1, 1, 0, mcsFilePath);
    if (gotFile) VerifyPROM();
    else printf("Verify PROM 18V04 is cancelled.\n");
  }
  else if (controlOption == HELP) {
    MessagePopup("Verify PROM 18V04", 
                 "Verify PROM 18V04 Chip through JTAG by using its mcs file.");
    controlOption = NORM;
  }
}


int VerifyVertexIIFPGA(void) {
  int bitData[1024], data, totalBits, totalCheckedBits, nSteps, nwords, i, j, endData;
  int nError, length, offset, sleepPolicy, JTAGDimmed, INITDimmed;
  float percentage, oldPercentage;
  char path[256], str[256], message[128], *strp;
  FILE *SVFFile, *bitstream, *readbackFile;
  
  ReadParameters(PARABITSTREAMFILE);
  if (strlen(bitStreamTextFName) <= 0) {
    MessagePopup("Need FPGA bitstream",
                 "Bitstream for FPGA is unknow, user must program FPGA first!");
    return -1;
  }
  if (firstTimeVerifyFPGA) {
    GetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, &JTAGDimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, TRUE);
    GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, &INITDimmed);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
    VerifyMaskFile(1);
    VerifyRBBFile(1);
    if (bitstream = fopen(rbdFilePath, "r")) {
      useMessageBox = TRUE;
      sleepPolicy = GetSleepPolicy();
      SetSleepPolicy(VAL_SLEEP_NONE);
      sprintf(message, "It takes while to read readback bitstream file ..., please be patient!\n");
      ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
      printf("Read readback bitstream file <%s>\n", rbdFilePath);
      for (i = 0; i < 8; i++) {
        fgets(str, 80, bitstream);
        printf("%s", str);
        // Get bitstream length
        strp = strstr(str, "Bits:");
        if (strp != NULL) sscanf(str+6, "%d", &nProgramDataWords);
      }
      nProgramDataWords /= 32;
      totalBits = 0;
      while (feof(bitstream) == 0) {
        strcpy(str, "");
        fgets(str, 80, bitstream);
        if (strlen(str) >= 32) for (i = 0; i < 32; i++) sscanf(str+i, "%1d", &programDataBits[totalBits++]);
        if ((totalBits%0xfff) == 0) {
          sprintf(message, "Reading bitstream file, got %d bits of %d.\n", totalBits, 32*nProgramDataWords);
          ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
          HandleUserButtonAndFunction(PanelDone);
        }
      }
      if (totalBits != (32*nProgramDataWords)) {
        sprintf(message, "Unexpected total number of bits readback bitstream file may be corrupted.\n");
        ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
        printf("%s\n", message);
        printf("Readback bitstream length: expected = %d got = %d\n", 32*nProgramDataWords, totalBits); 
        printf("Readback bitstream file <%s>.\n", rbdFilePath);
        SetSleepPolicy(sleepPolicy);
        SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, JTAGDimmed);
        SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
        fclose(bitstream);
        return -2;
      }
      fclose(bitstream);
      SetSleepPolicy(sleepPolicy);
      SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, JTAGDimmed);
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
    }
    else {
      printf("readback bitstream file <%s> does not exist.\n", rbdFilePath);
      printf("Can not verify VertexII chip, please check the file.\n");
      SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, JTAGDimmed);
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
      return -11;
    }
    if (nProgramMaskWords != nProgramDataWords) {
      if (bitstream = fopen(msdFilePath, "r")) {
        useMessageBox = TRUE;
        sleepPolicy = GetSleepPolicy();
        SetSleepPolicy(VAL_SLEEP_NONE);
        GetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, &JTAGDimmed);
        SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, TRUE);
        GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, &INITDimmed);
        SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
        sprintf(message, "It takes while to read ASCII MASK file ..., please be patient!\n");
        ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
        printf("Read ASCII MASK file <%s>\n", msdFilePath);
        for (i = 0; i < 8; i++) {
          fgets(str, 80, bitstream);
          printf("%s", str);
          // Get bitstream length
          strp = strstr(str, "Bits:");
          if (strp != NULL) sscanf(str+6, "%d", &nProgramMaskWords);
        }
        nProgramMaskWords /= 32;
        totalBits = 0;
        while (feof(bitstream) == 0) {
          strcpy(str, "");
          fgets(str, 80, bitstream);
          if (strlen(str) >= 32) for (i = 0; i < 32; i++) sscanf(str+i, "%1d", &programMaskBits[totalBits++]);
          if ((totalBits%0xfff) == 0) {
            sprintf(message, "Read ASCII MASK file, got %d bits of %d.\n", totalBits, 32*nProgramMaskWords);
            ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
            HandleUserButtonAndFunction(PanelDone);
          }
        }
        if (totalBits != (32*nProgramMaskWords)) {
          sprintf(message, "Unexpected total number of bits ASCII MASK file may be corrupted.\n");
          ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
          printf("%s\n", message);
          printf("ASCII MASK length: expected = %d got = %d\n", 32*nProgramMaskWords, totalBits); 
          printf("ASCII MASK file <%s>.\n", msdFilePath);
          SetSleepPolicy(sleepPolicy);
          SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, JTAGDimmed);
          SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
          fclose(bitstream);
          return -3;
        }
        fclose(bitstream);
        SetSleepPolicy(sleepPolicy);
      }
      else {
        printf("ASCII MASK file <%s> does not exist.\n", msdFilePath);
        printf("Can not verify VertexII chip, please check the file.\n");
        SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, JTAGDimmed);
        SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
        return -12;
      }
      SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, JTAGDimmed);
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
    }
  }

  oldPercentage = 0.0;
  nError = 0;
  strcpy(path, "verifyFPGA.svf");
  if (SVFFile = fopen(path, "w")) {
    useMessageBox = TRUE;
    sleepPolicy = GetSleepPolicy();
    SetSleepPolicy(VAL_SLEEP_NONE);
    GetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, &JTAGDimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, TRUE);
    GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, &INITDimmed);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
    GlobalEnableSerial();
#ifdef DEBUGVERIFYFPGAORPROM
    readbackFile = fopen("bitstreamReadback.txt", "w");
#endif
    sprintf(message, "Starting to verify FPGA bitstream, takes a while ...\n");
    ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
    printf("%s\n", message);
    if (pauseDAQToVerifyFPGA && (runState == State_Running)) {
      PauseDAQ();
      WaitInMillisecond(100);
    }
    if (action == DOWNLOAD) ResetTAP();
    AllJTAGDeviceInBYPASS(SVFFile, 0);
    FPGAInstruction(SVFFile, VERTEXIIBYPASS, 12);
    // FPGAInstruction(SVFFile, VERTEXIIJSHUTDOWN, 36);
    FPGAInstruction(SVFFile, VERTEXIICFGIN, 0);
    if (firstTimeVerifyFPGA) {
      bitData[0] = 0xffffffff;
      bitData[1] = 0xaa995566;
      bitData[2] = XPHTYPE1WRITE | (VERTEXICRADDRCMD << XPHREGADDRESSBIT0LOCATION) | 1;
      bitData[3] = VERTEXCMDRCFG;
      bitData[4] = 0;
      bitData[5] = 0;
      for (nwords = 0; nwords < 6; nwords++) {
        data = bitData[nwords];
        bitData[nwords] = 0;
        for (i = 0; i < 32; i++) bitData[nwords] |= (((data >> (31-i)) & 1) << i);
      }
      FPGAData(SVFFile, 6*32, bitData, 0, 36, TRUE);
    }
    else {
      bitData[0] = 0xffffffff;
      bitData[1] = 0xaa995566;
      bitData[2] = XPHTYPE1WRITE | (VERTEXICRADDRFAR << XPHREGADDRESSBIT0LOCATION) | 1;
      bitData[3] = 0;
      bitData[4] = XPHTYPE1WRITE | (VERTEXICRADDRCMD << XPHREGADDRESSBIT0LOCATION) | 1;
      bitData[5] = VERTEXCMDRCFG;
      bitData[6] = XPHTYPE1READ | (VERTEXICRADDRFDRO << XPHREGADDRESSBIT0LOCATION) | 0;
      bitData[7] = XPHTYPE2READ | nProgramDataWords;
      bitData[8] = 0;
      bitData[9] = 0;
      for (nwords = 0; nwords < 10; nwords++) {
        data = bitData[nwords];
        bitData[nwords] = 0;
        for (i = 0; i < 32; i++) bitData[nwords] |= (((data >> (31-i)) & 1) << i);
      }
      FPGAData(SVFFile, 10*32, bitData, 0, 36, TRUE);
    }
    FPGAInstruction(SVFFile, VERTEXIICFGOUT, 0);
    nSteps = 0;
    totalBits = 0;
    totalCheckedBits = 0;
    nwords = 0;
    length = 0;
    endData = FALSE;
    while (TRUE) {
      bitData[1023] = 512;
      if (runState == State_Running) bitData[1023] = 128;
      for (i = 0; i < bitData[1023]; i++) {
        nwords++;
        totalBits += 32;
        if (totalBits >= (32*nProgramDataWords)) {
          endData = TRUE;
          break;
        }
        else endData = FALSE;
      }
      if (pauseDAQToVerifyFPGA && (runState == State_Running)) {
        PauseDAQ();
        WaitInMillisecond(100);
      }
      FPGABinaryData(SVFFile, 32*nwords, &programDataBits[totalBits-32*nwords], 3, 0, endData);
      if (nSteps == 0) offset = vertexIIOffset;
      else offset = 0;
      if (!firstTimeVerifyFPGA) {
        bitData[1023] = 0;
        for (i = 0; i < 32*nwords; i++) {
          j = totalBits - 32*nwords + i;
          if (programMaskBits[j] == 0) {
            if (programDataBits[j] != readbackArray[i+offset]) bitData[1023]++;
            totalCheckedBits++;
          }
        }
        if (bitData[1023] != 0) {
          nError += bitData[1023];
          printf("nSteps = %4d, nError = %d\n", nSteps, nError);
        }
      }
#ifdef DEBUGVERIFYFPGAORPROM
      for (i = 0; i < nwords; i++) {
        BinaryToInt(&data, 32*i+offset, 32, readbackArray);
        fprintf(readbackFile, "%01x%01x", data&0xF, (data>>4)&0xF);
        fprintf(readbackFile, "%01x%01x", (data>>8)&0xF, (data>>12)&0xF);
        fprintf(readbackFile, "%01x%01x", (data>>16)&0xF, (data>>20)&0xF);
        fprintf(readbackFile, "%01x%01x", (data>>24)&0xF, (data>>28)&0xF);
        length++;
        if ((length%10) == 0) fprintf(readbackFile, "\n");
      }
#endif
      nSteps++;
      nwords = 0;
      percentage = 100.0 * (((float) totalBits) / ((float) (32*nProgramDataWords)));
      if ((percentage - oldPercentage) >= 5.0) {
        sprintf(message, "It takes while to verify FPGA ..., %.0f percentage is done.\n", percentage);
        ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
        HandleUserButtonAndFunction(PanelDone);
        oldPercentage = percentage;
      }
      if (endData) break;
    }
#ifdef DEBUGVERIFYFPGAORPROM
    fclose(readbackFile);
#endif
    FPGAInstruction(SVFFile, VERTEXIIBYPASS, 0);
    FPGAData(SVFFile, 1, bitData, 0, 0, TRUE);
    FPGAData(SVFFile, 1, bitData, 0, 0, TRUE);
    AllJTAGDeviceInBYPASS(SVFFile, 1);
    fclose(SVFFile);
    if (pauseDAQToVerifyFPGA && (runState == State_Paused)) ResumeDAQ();
    if (nError == 0) sprintf(message, "Total %d bits (%d verified) have been readback from FPGA with %d steps.\n", totalBits, totalCheckedBits, nSteps);
    else sprintf(message, "Find %d wrong bits out of %d checked bits from FPGA (total %d bits).\n", nError, totalCheckedBits, totalBits);
    ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
    printf("%s\n", message);
    SetupJTAGChain();
    if (firstTimeVerifyFPGA) {
      firstTimeVerifyFPGA = FALSE;
      nError = VerifyVertexIIFPGA();
    }
    SetSleepPolicy(sleepPolicy);
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, JTAGDimmed);
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
    GlobalDisableSerial();
  }
  else {
    printf("Unable to open SVF file <%s>!\n", path);
    MessagePopup("Failed to Open SVF File",
                 "Unable to open SVF file, please check your disk and filename!");
    return -20;
  }

  return nError;
}


int VerifyPROM(void) {
  int bitData[512], fstatus, data, nbits, i, i0, j, strDone, totalBits, nSteps;
  int nError, totalError, length, sleepPolicy, JTAGDimmed, INITDimmed, address;
  float lineNumber, numberLines;
  char path[256], str[256], message[128];
  FILE *SVFFile, *mcsFile;
  
  nError = 0;
  totalError = 0;
  if (strlen(mcsFilePath) <= 0) {
    printf("No MCS file is defined, can not verify PROM.\n");
    return -1;
  }
  if (mcsFile = fopen(mcsFilePath, "r")) {
    // Check MCS file first to make sure
    numberLines = 0.0;
    while (TRUE) {
      fstatus = ferror(mcsFile);
      if (fstatus != 0) {
        printf("*** Error in reading MCS file with fstats=0x%X!\n", fstatus);
        printf("*** Verify PROM will be aborted.\n"); 
        return -2;
      }
      if (feof(mcsFile) != 0) break;
      fscanf(mcsFile, "%s", str);
      numberLines += 1.0;
      if (str[0] != ':') fstatus = -1;
      else {
        sscanf(&str[1], "%2x", &data);
        if ((data != 0x02) && (data != 0x10) && (data != 0x04) && (data != 0x0c) && (data != 0x00)) fstatus = -2;
      }
      if (fstatus < 0) {
        printf("%s", str);
        printf("0x%02x\n", data);
        printf("\n");
        printf("------- Abort to Verify PROM ------\n");
        printf("The file <%s> is not good MCS file.\n", mcsFilePath);
        printf("Verify PROM will be aborted.\n");
        printf("Please check and replace the MCS file.\n"); 
        printf("\n");
        return -3;    
      }
    }
    rewind(mcsFile);
    lineNumber = 0.0;
    strcpy(path, "verifyPROM.svf");
    if (SVFFile = fopen(path, "w")) {
      useMessageBox = TRUE;
      sleepPolicy = GetSleepPolicy();
      SetSleepPolicy(VAL_SLEEP_NONE);
      GetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, &JTAGDimmed);
      SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, TRUE);
      GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, &INITDimmed);
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
//      GlobalEnableSerial();
      sprintf(message, "Starting to verify PROM, takes a while ...\n");
      ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
      printf("%s\n", message);
      if (pauseDAQToVerifyPROM && (runState == State_Running)) {
        PauseDAQ();
        WaitInMillisecond(100);
      }
      PROMInstruction(SVFFile, -1, 0);
      if (PROMINSTLENGTH == 16) {
        PROMInstruction(SVFFile, PROMISC_ENABLE, 0);
        bitData[0] = 0x3;
        PROMData(SVFFile, 8, bitData, 0, 0);
      }
      else {
//        PROMInstruction(SVFFile, PROMBYPASS, 0);
//        PROMInstruction(SVFFile, PROMIDCODE, 0);
//        bitData[0] = 0;
//        PROMData(SVFFile, 32, bitData, 0, 0);
//        PROMInstruction(SVFFile, PROMISPEX, 110000);
//        PROMInstruction(SVFFile, PROMBYPASS, 0);
//        PROMInstruction(SVFFile, PROMBYPASS, 0);
//        PROMInstruction(SVFFile, PROMIDCODE, 0);
//        bitData[0] = 0;
//        PROMData(SVFFile, 32, bitData, 0, 0);
//        PROMInstruction(SVFFile, PROMISPEN, 0);
//        bitData[0] = 0x34;
//        PROMData(SVFFile, 6, bitData, 0, 0);
//        PROMInstruction(SVFFile, PROMISPEX, 110000);
//        PROMInstruction(SVFFile, PROMIDCODE, 0);
//        bitData[0] = 0;
//        PROMData(SVFFile, 32, bitData, 0, 0);
        PROMInstruction(SVFFile, PROMISPEN, 0);
        bitData[0] = 0x34;
        PROMData(SVFFile, 6, bitData, 0, 0);
      }
      nSteps = 0;
      totalBits = 0;
      nError = 0;
      nbits = 0;
      j = 0;
      strDone = TRUE;
      while (TRUE) {
        PROMInstruction(SVFFile, PROMFADDR, 0);
        bitData[511] = 8192;
        if (PROMINSTLENGTH == 16) bitData[0] = 0x400 * nSteps;
        else bitData[0] = 0x40 * nSteps;
        address = bitData[0];
        if (PROMINSTLENGTH == 16) PROMData(SVFFile, 24, bitData, 0, 0);
        else {
          PROMData(SVFFile, 16, bitData, 0, 1);
          PROMInstruction(SVFFile, -2, 1);
        }
        for (i = 0; i < 256; i++) bitData[i] = 0;
        while (nbits < bitData[511]) {
          if (feof(mcsFile) != 0) break;
          if (strDone) {
            i0 = 7;
            fscanf(mcsFile, "%s", str);
            lineNumber += 1.0;
          }
          sscanf(&str[1], "%2x", &data);
          if ((data == 0x10) || (data == 0x04) || (data == 0x0C)) {
            for (i = i0+2; i < strlen(str)-2; i += 2) {
              sscanf(&str[i], "%02x", &data);
              bitData[j/4] |= ((data&0xFF) << (8*(j%4)));
              nbits += 8;
              if (nbits == bitData[511]) {
                j = 0;
                i0 = i;
                strDone = FALSE;
                break;
              }
              else j++;
            }
          }
          strDone = TRUE;
        }
        if (nbits <= 0) break;
        while (nbits < bitData[511]) {
          bitData[j/4] |= (0xFF << (8*(j%4)));
          nbits += 8;
          if (nbits == bitData[511]) {
            j = 0;
            break;
          }
          else j++;
        }
        if (PROMINSTLENGTH == 16) PROMInstruction(SVFFile, PROMXSC_READ, 15);
        else {
          PROMInstruction(SVFFile, PROMFVFY0, 1);
          PROMInstruction(SVFFile, -3, 1);    // Wait 1 milliseconds
        }
        for (i = 0; i < bitData[511]; i++) readbackArray[i] = 0;
        if (pauseDAQToVerifyPROM && (runState == State_Running)) {
          PauseDAQ();
          WaitInMillisecond(100);
        }
        PROMData(SVFFile, nbits, bitData, 1, 0);
        if ((nError = CheckFullJTAGDataArray()) != 0) {
          totalError += nError;
          printf("nSteps = %4d, address = 0x%06x, nError = %d\n", nSteps, address, nError);
        }
        nSteps++;
        totalBits += nbits;
        nbits = 0;
        if (numberLines > 0.0) {
          sprintf(message, "It takes while to Verify PROM ..., %.0f percentage is done.\n", 100.0*lineNumber/numberLines);
          ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
          HandleUserButtonAndFunction(PanelDone);
        }
      }
      if (PROMINSTLENGTH == 16) {
        PROMInstruction(SVFFile, PROMXSC_OP_STATUS, 0);
        bitData[0] = 0x0;
        PROMData(SVFFile, 8, bitData, 0, 0);
        PROMInstruction(SVFFile, PROMISC_DISABLE, 1);
        PROMInstruction(SVFFile, -3, 1);      // Wait 1 milliseconds
      }
      else {
        PROMInstruction(SVFFile, PROMISPEX, 1);
        PROMInstruction(SVFFile, -3, 110);    // Wait 110 milliseconds
//        PROMInstruction(SVFFile, PROMISPEX, 110000);
      }
      PROMInstruction(SVFFile, PROMBYPASS, 0);
      PROMInstruction(SVFFile, PROMBYPASS, 0);
      bitData[0] = 0x0;
      PROMData(SVFFile, 1, bitData, 0, 0);
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
      if (pauseDAQToVerifyPROM && (runState == State_Paused)) ResumeDAQ();
      if (totalError == 0) sprintf(message, "Total %d PROM program bits have been verified with %d steps.\n", totalBits, nSteps);
      else sprintf(message, "Find %d wrong bits out of %d PROM bits with %d steps.\n", totalError, totalBits, nSteps);
      ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, message);
      printf("%s\n", message);
      SetSleepPolicy(sleepPolicy);
      SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, JTAGDimmed);
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
//      GlobalDisableSerial();
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
      return -4;
    }
    fclose(mcsFile);
  }
  else {
    printf(".mcs file <%s> does not exist.\n", mcsFilePath);
    printf("Can not verify PROM, please check .mcs file.\n");
    return -5;
  }
  return nError;
}


int VerifyBitFile(int printFlag) {
  unsigned int length, l0, i, j, data, bit[8], nbytes, retbytes, sleepPolicy;
  unsigned char buf[1024];
  FILE *bitFile, *textFile;
  
  if (strlen(bitFilePath) <= 0) {
    printf("No BIT file is defined.\n");
    return 1;
  }
  if (bitFile = fopen(bitFilePath, "rb")) {
    useMessageBox = TRUE;
    sleepPolicy = GetSleepPolicy();
    SetSleepPolicy(VAL_SLEEP_NONE);
    printf("Starting to Verify the BIT file <%s>\n", bitFilePath);
    strncpy(maskFilePath, bitFilePath, strlen(bitFilePath)-4);
    maskFilePath[strlen(bitFilePath)-4] = '\0';
    strcat(maskFilePath, ".msk");
    strncpy(msdFilePath, bitFilePath, strlen(bitFilePath)-4);
    msdFilePath[strlen(bitFilePath)-4] = '\0';
    strcat(msdFilePath, ".msd");
    strncpy(rbdFilePath, bitFilePath, strlen(bitFilePath)-4);
    rbdFilePath[strlen(bitFilePath)-4] = '\0';
    strcat(rbdFilePath, ".rbd");
    strncpy(rbbFilePath, bitFilePath, strlen(bitFilePath)-4);
    rbbFilePath[strlen(bitFilePath)-4] = '\0';
    strcat(rbbFilePath, ".rbb");
    // check field 1 (header)
    if (fread(buf, 1, 2, bitFile) != 2) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 2;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length != 9) {
      printf("Unexpected bit file header length (expected = 9, got = %d).\n", length);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 3;
    }
    if (fread(buf, 1, length, bitFile) != length) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 4;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected bit file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 5;
    }
    // check field 2
    if (fread(buf, 1, 2, bitFile) != 2) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      return 12;
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length != 1) {
      printf("Unexpected length (expected = 1, got = %d).\n", length);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 13;
    }
    // check field 3
    if (fread(buf, 1, 1, bitFile) != 1) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 21;
    }
    if (buf[0] != 'a') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected bit control character (expected = a, got = %c).\n", buf[0]);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 25;
    }
    if (fread(buf, 1, 2, bitFile) != 2) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 22;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 23;
    }
    if (fread(buf, 1, length, bitFile) != length) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 24;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected bit file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 25;
    }
    if (printFlag > 0) printf("Bit file is created from file %s", buf);
    // check field 4
    if (fread(buf, 1, 1, bitFile) != 1) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 31;
    }
    if (buf[0] != 'b') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected bit control character (expected = b, got = %c).\n", buf[0]);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 35;
    }
    if (fread(buf, 1, 2, bitFile) != 2) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 32;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 33;
    }
    if (fread(buf, 1, length, bitFile) != length) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 34;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected bit file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 35;
    }
    if (printFlag > 0) printf(" for Xilinx chip %s.\n", buf);
    // check field 5
    if (fread(buf, 1, 1, bitFile) != 1) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 41;
    }
    if (buf[0] != 'c') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected bit control character (expected = b, got = %c).\n", buf[0]);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 45;
    }
    if (fread(buf, 1, 2, bitFile) != 2) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 42;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 43;
    }
    if (fread(buf, 1, length, bitFile) != length) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 44;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected bit file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 45;
    }
    if (printFlag > 0) printf("Bit file is created on %s.", buf);
    // check field 6
    if (fread(buf, 1, 1, bitFile) != 1) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 51;
    }
    if (buf[0] != 'd') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected bit control character (expected = b, got = %c).\n", buf[0]);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 55;
    }
    if (fread(buf, 1, 2, bitFile) != 2) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 52;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 53;
    }
    if (fread(buf, 1, length, bitFile) != length) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 54;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected bit file data (expected = 0, got = %d).\n", buf[length-1]);
      return 55;
    }
    if (printFlag > 0) printf(" at %s.\n", buf);
    // check field 7
    if (fread(buf, 1, 1, bitFile) != 1) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 61;
    }
    if (buf[0] != 'e') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected bit control character (expected = b, got = %c).\n", buf[0]);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 65;
    }
    if (fread(buf, 1, 4, bitFile) != 4) {
      printf("Error in reading BIT file <%s>.", bitFilePath);
      fclose(bitFile);
      SetSleepPolicy(sleepPolicy);
      return 62;
    }
    length = (((int) buf[0]) << 24) | (((int) buf[1]) << 16) | (((int) buf[2]) << 8) | buf[3];
    if (printFlag > 0) printf("Bitstream length is %u bytes.\n", length);
    nBitstreamBytes = length;
    SaveParameters(PARABITSTREAMLENGTH);
    ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, "Verify Bit file in progress and write bitstream to a text file");
    HandleUserButtonAndFunction(PanelDone);
    l0 = length%40;
    if (l0 != 0) nbytes = l0;
    else nbytes = 40;
    textFile = fopen(bitStreamTextFName, "w");
    while (length > 0) {
//      retbytes = 0;
//      for (j = 0; j < nbytes; j++) {
//        i = fread(&buf[j], 1, 1, bitFile);
//        if (i == 1) retbytes++;
//        else {
//          printf("i = %d\n", i);
//          break;
//        }
//      }
      retbytes = fread(buf, 1, nbytes, bitFile);
      for (j = 0; j < retbytes; j++) {
        length--;
        data = (int) buf[j];
        if (data != 0) {
          for (i = 0; i < 8; i++) bit[i] = (data >> i) & 1;
          data = 0;
          for (i = 0; i < 8; i++) data |= (bit[7-i] << i);
        }
        fprintf(textFile, "%01x%01x", data&0xF, (data>>4)&0xF);
        //fprintf(textFile, "%02x", buf[j]);
        if ((length-l0+40)%40 == 0) fprintf(textFile, "\n");
      }
      if (retbytes != nbytes) {
        printf("Error in reading BIT file <%s>.\n", bitFilePath);
        printf("      remain bytes %d: # expected bytes = %d  # readback bytes = %d.\n", length, nbytes, retbytes);  
        fclose(bitFile);
        SetSleepPolicy(sleepPolicy);
        fclose(textFile);
        return 66;
      }
      nbytes = 40;
    }
    fclose(textFile);
    while (feof(bitFile) == 0) if (fread(buf, 1, 1, bitFile) == 1) printf("%02x ", buf[0]);
    fclose(bitFile);
    printf("Verify the BIT file done.\n");
    SetSleepPolicy(sleepPolicy);
  }
  else {
    printf("Unable to open BIT file <%s>!\n", bitFilePath);
    return -1;
  }
  
  return 0;
}


int VerifyMaskFile(int printFlag) {
  unsigned int length, l0, i, j, data, bit[8], nbytes, retbytes, totalBits, sleepPolicy;
  unsigned char buf[1024];
  FILE *maskFile, *textFile;
  
  nProgramMaskWords = 0;
  if (strlen(maskFilePath) <= 0) {
    printf("No MASK file is defined.\n");
    return 1;
  }
  if (maskFile = fopen(maskFilePath, "rb")) {
    useMessageBox = TRUE;
    sleepPolicy = GetSleepPolicy();
    SetSleepPolicy(VAL_SLEEP_NONE);
    printf("Starting to Verify the MASK file <%s>\n", maskFilePath);
    // check field 1 (header)
    if (fread(buf, 1, 2, maskFile) != 2) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 2;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length != 9) {
      printf("Unexpected MASK file header length (expected = 9, got = %d).\n", length);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 3;
    }
    if (fread(buf, 1, length, maskFile) != length) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 4;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected MASK file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 5;
    }
    // check field 2
    if (fread(buf, 1, 2, maskFile) != 2) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 12;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length != 1) {
      printf("Unexpected length (expected = 1, got = %d).\n", length);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 13;
    }
    // check field 3
    if (fread(buf, 1, 1, maskFile) != 1) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 21;
    }
    if (buf[0] != 'a') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected control character (expected = a, got = %c).\n", buf[0]);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 25;
    }
    if (fread(buf, 1, 2, maskFile) != 2) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 22;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 23;
    }
    if (fread(buf, 1, length, maskFile) != length) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      return 24;
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
    }
    if (buf[length-1] != 0) {
      printf("Unexpected MASK file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 25;
    }
    if (printFlag > 0) printf("MASK file is created from file %s", buf);
    // check field 4
    if (fread(buf, 1, 1, maskFile) != 1) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 31;
    }
    if (buf[0] != 'b') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected control character (expected = b, got = %c).\n", buf[0]);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 35;
    }
    if (fread(buf, 1, 2, maskFile) != 2) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 32;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 33;
    }
    if (fread(buf, 1, length, maskFile) != length) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 34;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected MASK file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 35;
    }
    if (printFlag > 0) printf(" for Xilinx chip %s.\n", buf);
    // check field 5
    if (fread(buf, 1, 1, maskFile) != 1) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 41;
    }
    if (buf[0] != 'c') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected control character (expected = b, got = %c).\n", buf[0]);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 45;
    }
    if (fread(buf, 1, 2, maskFile) != 2) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 42;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 43;
    }
    if (fread(buf, 1, length, maskFile) != length) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 44;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected MASK file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 45;
    }
    if (printFlag > 0) printf("MASK file is created on %s.", buf);
    // check field 6
    if (fread(buf, 1, 1, maskFile) != 1) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 51;
    }
    if (buf[0] != 'd') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected control character (expected = b, got = %c).\n", buf[0]);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 55;
    }
    if (fread(buf, 1, 2, maskFile) != 2) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 52;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 53;
    }
    if (fread(buf, 1, length, maskFile) != length) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 54;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected MASK file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 55;
    }
    if (printFlag > 0) printf(" at %s.\n", buf);
    // check field 7
    if (fread(buf, 1, 1, maskFile) != 1) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 61;
    }
    if (buf[0] != 'e') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected control character (expected = b, got = %c).\n", buf[0]);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 65;
    }
    if (fread(buf, 1, 4, maskFile) != 4) {
      printf("Error in reading MASK file <%s>.", maskFilePath);
      fclose(maskFile);
      SetSleepPolicy(sleepPolicy);
      return 62;
    }
    length = (((int) buf[0]) << 24) | (((int) buf[1]) << 16) | (((int) buf[2]) << 8) | buf[3];
    nProgramMaskWords = length / 4;
    if (printFlag > 0) printf("Maskstream length is %u bytes.\n", length);
    ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, "Verify Mask file in progress");
    HandleUserButtonAndFunction(PanelDone);
    l0 = length%40;
    if (l0 != 0) nbytes = l0;
    else nbytes = 40;
    totalBits = 0;
    textFile = fopen("bitstreamMask.txt", "w");
    while (length > 0) {
      retbytes = fread(buf, 1, nbytes, maskFile);
      for (j = 0; j < retbytes; j++) {
        length--;
        data = (int) buf[j];
        if (data != 0) {
          for (i = 0; i < 8; i++) bit[i] = (data >> i) & 1;
          data = 0;
          for (i = 0; i < 8; i++) data |= (bit[7-i] << i);
        }
        fprintf(textFile, "%01x%01x", data&0xF, (data>>4)&0xF);
        if ((length-l0+40)%40 == 0) fprintf(textFile, "\n");
        IntToBinary(data, totalBits, 8, programMaskBits, PBITSLENGTH);
        totalBits += 8;
      }
      if (retbytes != nbytes) {
        printf("Error in reading MASK file <%s>.\n", maskFilePath);
        printf("      remain bytes %d: # expected bytes = %d  # readback bytes = %d.\n", length, nbytes, retbytes);  
        nProgramMaskWords -= length / 4;
        fclose(maskFile);
        SetSleepPolicy(sleepPolicy);
        fclose(textFile);
        return 66;
      }
      nbytes = 40;
    }
    fclose(textFile);
    while (feof(maskFile) == 0) if (fread(buf, 1, 1, maskFile) == 1) printf("%02x ", buf[0]);
    fclose(maskFile);
    printf("Verify the MASK file done.\n");
    SetSleepPolicy(sleepPolicy);
  }
  else {
    printf("Unable to open MASK file <%s>!\n", maskFilePath);
    return -1;
  }
  
  return 0;
}


int VerifyRBBFile(int printFlag) {
  unsigned int length, l0, i, j, data, bit[8], nbytes, retbytes, totalBits, sleepPolicy;
  unsigned char buf[1024];
  FILE *rbbFile, *textFile;
  
  if (strlen(rbbFilePath) <= 0) {
    printf("No Readback Bitstream Binary file is defined.\n");
    return 1;
  }
  if (rbbFile = fopen(rbbFilePath, "rb")) {
    useMessageBox = TRUE;
    sleepPolicy = GetSleepPolicy();
    SetSleepPolicy(VAL_SLEEP_NONE);
    printf("Starting to Verify the Readback Bitstream Binary file <%s>\n", rbbFilePath);
    // check field 1 (header)
    if (fread(buf, 1, 2, rbbFile) != 2) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 2;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length != 9) {
      printf("Unexpected Readback Bitstream Binary file header length (expected = 9, got = %d).\n", length);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 3;
    }
    if (fread(buf, 1, length, rbbFile) != length) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 4;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected Readback Bitstream Binary file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 5;
    }
    // check field 2
    if (fread(buf, 1, 2, rbbFile) != 2) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 12;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length != 1) {
      printf("Unexpected length (expected = 1, got = %d).\n", length);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 13;
    }
    // check field 3
    if (fread(buf, 1, 1, rbbFile) != 1) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 21;
    }
    if (buf[0] != 'a') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected control character (expected = a, got = %c).\n", buf[0]);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 25;
    }
    if (fread(buf, 1, 2, rbbFile) != 2) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 22;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 23;
    }
    if (fread(buf, 1, length, rbbFile) != length) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      return 24;
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
    }
    if (buf[length-1] != 0) {
      printf("Unexpected Readback Bitstream Binary file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 25;
    }
    if (printFlag > 0) printf("Readback Bitstream Binary file is created from file %s", buf);
    // check field 4
    if (fread(buf, 1, 1, rbbFile) != 1) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 31;
    }
    if (buf[0] != 'b') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected control character (expected = b, got = %c).\n", buf[0]);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 35;
    }
    if (fread(buf, 1, 2, rbbFile) != 2) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 32;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 33;
    }
    if (fread(buf, 1, length, rbbFile) != length) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 34;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected Readback Bitstream Binary file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 35;
    }
    if (printFlag > 0) printf(" for Xilinx chip %s.\n", buf);
    // check field 5
    if (fread(buf, 1, 1, rbbFile) != 1) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 41;
    }
    if (buf[0] != 'c') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected control character (expected = b, got = %c).\n", buf[0]);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 45;
    }
    if (fread(buf, 1, 2, rbbFile) != 2) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 42;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 43;
    }
    if (fread(buf, 1, length, rbbFile) != length) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 44;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected Readback Bitstream Binary file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 45;
    }
    if (printFlag > 0) printf("Readback Bitstream Binary file is created on %s.", buf);
    // check field 6
    if (fread(buf, 1, 1, rbbFile) != 1) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 51;
    }
    if (buf[0] != 'd') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected control character (expected = b, got = %c).\n", buf[0]);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 55;
    }
    if (fread(buf, 1, 2, rbbFile) != 2) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 52;
    }
    length = (((int) buf[0]) << 8) | buf[1];
    if (length >= 1024) {
      printf("Unexpected length (expected < 1024, got = %d).\n", length);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 53;
    }
    if (fread(buf, 1, length, rbbFile) != length) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 54;
    }
    if (buf[length-1] != 0) {
      printf("Unexpected Readback Bitstream Binary file data (expected = 0, got = %d).\n", buf[length-1]);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 55;
    }
    if (printFlag > 0) printf(" at %s.\n", buf);
    // check field 7
    if (fread(buf, 1, 1, rbbFile) != 1) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 61;
    }
    if (buf[0] != 'e') {
      if (printFlag > 0) printf("\n");
      printf("Unexpected control character (expected = b, got = %c).\n", buf[0]);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 65;
    }
    if (fread(buf, 1, 4, rbbFile) != 4) {
      printf("Error in reading Readback Bitstream Binary file <%s>.", rbbFilePath);
      fclose(rbbFile);
      SetSleepPolicy(sleepPolicy);
      return 62;
    }
    length = (((int) buf[0]) << 24) | (((int) buf[1]) << 16) | (((int) buf[2]) << 8) | buf[3];
    if (printFlag > 0) printf("Readback bitstream length is %u bytes.\n", length);
    ResetTextBox(JTAGControlHandle, P_JTAGCTRL_MESSAGE, "Verify Readback Bitstream Binary file in progress");
    HandleUserButtonAndFunction(PanelDone);
    l0 = length%40;
    if (l0 != 0) nbytes = l0;
    else nbytes = 40;
    totalBits = 0;
    textFile = fopen("bitstreamRBB.txt", "w");
    while (length > 0) {
      retbytes = fread(buf, 1, nbytes, rbbFile);
      for (j = 0; j < retbytes; j++) {
        length--;
        data = (int) buf[j];
        if (data != 0) {
          for (i = 0; i < 8; i++) bit[i] = (data >> i) & 1;
          data = 0;
          for (i = 0; i < 8; i++) data |= (bit[7-i] << i);
        }
        fprintf(textFile, "%01x%01x", data&0xF, (data>>4)&0xF);
        if ((length-l0+40)%40 == 0) fprintf(textFile, "\n");
//        IntToBinary(data, totalBits, 8, programMaskBits, PBITSLENGTH);
        totalBits += 8;
      }
      if (retbytes != nbytes) {
        printf("Error in reading Readback Bitstream Binary file <%s>.\n", rbbFilePath);
        printf("      remain bytes %d: # expected bytes = %d  # readback bytes = %d.\n", length, nbytes, retbytes);  
        fclose(rbbFile);
        SetSleepPolicy(sleepPolicy);
        fclose(textFile);
        return 66;
      }
      nbytes = 40;
    }
    fclose(textFile);
    while (feof(rbbFile) == 0) if (fread(buf, 1, 1, rbbFile) == 1) printf("%02x ", buf[0]);
    fclose(rbbFile);
    printf("Verify the Readback Bitstream Binary file done.\n");
    SetSleepPolicy(sleepPolicy);
  }
  else {
    printf("Unable to open Readback Bitstream Binary file <%s>!\n", rbbFilePath);
    return -1;
  }
  
  return 0;
}


int ConfirmToVerifyFPGAAndPROM(void) {
  static int gotFunc, nRadTest = 0, gotFilename, resp, checkMode, dimmed;
  int savedMeasureTriggerRate;
  char fname[80], str[256];
  
  radTestLogFile = NULL;
  HidePanel(JTAGControlHandle);
  DisplayPanel(ConfirmVFPHandle);
  SetActivePanel(ConfirmVFPHandle);
  savedMeasureTriggerRate = measureTriggerRate;
  measureTriggerRate = FALSE;
  gotFunc = 0;
  while (gotFunc <= 1) {
    gotFunc = CheckUserButtons(PanelDefault);
    if (gotFunc == 1) PanelDefault();
    GetCtrlVal(ConfirmVFPHandle, P_SELVFP_CHECKMODE, &checkMode);
    if (checkMode <= 1) dimmed = TRUE;
    else dimmed = FALSE;
    SetCtrlAttribute(ConfirmVFPHandle, P_SELVFP_VERIFYFPGATIME, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(ConfirmVFPHandle, P_SELVFP_VERIFYPROMTIME, ATTR_DIMMED, dimmed);
#ifdef TTCVIDRIVERVIATCP
    dimmed = FALSE;
    if (measureTriggerRate) {
      savedMeasureTriggerRate = measureTriggerRate;
      measureTriggerRate = FALSE;
    }
    else if (TTCviTCPConnected && (!gotTTCvi)) dimmed = TRUE;
    SetCtrlAttribute(ConfirmVFPHandle, P_SELVFP_DEFAULT, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(ConfirmVFPHandle, P_SELVFP_LOADPROM, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(ConfirmVFPHandle, P_SELVFP_SELECTRBDFILE, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(ConfirmVFPHandle, P_SELVFP_DONOTHING, ATTR_DIMMED, dimmed);
#endif
  }
  HidePanel(ConfirmVFPHandle);
  DisplayPanel(JTAGControlHandle);
  SetActivePanel(JTAGControlHandle);
  if (confirmVerify) {
    if (SetDir("log") == 0) SetDir("..");
    else MakeDir("log");
    nRadTest++;
    nRadTest %= 1000;
    gotFilename = TRUE;
    sprintf(fname, "log/radTest%08s_%03d.log", DateStringNoSpace(time(NULL)), nRadTest);
    while (radTestLogFile = fopen(fname, "r")) {
      fclose(radTestLogFile);
      nRadTest++;
      sprintf(fname, "log/radTest%8s_%03d.log", DateStringNoSpace(time(NULL)), nRadTest);
      if (nRadTest == 999) break;
    }
    if (radTestLogFile = fopen(fname, "r")) {
      sprintf(str, "File <%s> exists.\nDo you want to overwrite it?", fname);
      gotFilename = ConfirmPopup("Overwrite File Confirmation", str);
      fclose(radTestLogFile);
    }
    if (gotFilename) {
      if ((radTestLogFile = fopen(fname, "w"))) {
        fprintf(radTestLogFile, "Logfile for the FPGA and PROM automatical Verification\n");
        fprintf(radTestLogFile, "Start Date and Time    : %60s\n", WeekdayTimeString(time(NULL)));
        fprintf(radTestLogFile, "PROM MCS File          : %60s\n", mcsFilePath);
        fprintf(radTestLogFile, "FPGA BIT File          : %60s\n", bitFilePath);
        fprintf(radTestLogFile, "FPGA MASK Binary File  : %60s\n", maskFilePath);
        fprintf(radTestLogFile, "FPGA MASK File         : %60s\n", msdFilePath);
        fprintf(radTestLogFile, "Readback ASCII File    : %60s\n", rbdFilePath);
        fprintf(radTestLogFile, "Readback Binary File   : %60s\n", rbbFilePath);
      }
      else {
        nRadTest--;
        sprintf(str, "Unable to open logfile <%s>\nDo you wish to continue without logfile?", fname);
        resp = ConfirmPopup("Continue without Logfile", str);
        if (resp <= 0) confirmVerify = FALSE;
      }
      GetCtrlVal(ConfirmVFPHandle, P_SELVFP_PAUSEDAQTOVERIFYFPGA, &pauseDAQToVerifyFPGA);
      GetCtrlVal(ConfirmVFPHandle, P_SELVFP_PAUSEDAQTOVERIFYPROM, &pauseDAQToVerifyPROM);
    }
    else confirmVerify = FALSE;
  }
  return confirmVerify;
}


void WriteToFPGAAndPROMLogFile(char str[]) {
  char str0[80];
  
  strcpy(str0, WeekdayTimeString(time(NULL)));
  printf(" %s --> %s", str0, str);
  if (radTestLogFile != NULL) fprintf(radTestLogFile, " %s --> %s", str0, str);
}


void SelectRBDFile(void) {
  int panel, i, gotFile;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    if (panel == ConfirmVFPHandle) PanelDone();
    if (strlen(rbdFilePath) <= 0) strcpy(fileDirType, "bitFiles\\");
    else {
      strcpy(fileDirType, rbdFilePath);
      for (i = strlen(fileDirType); i >= 0; i--) {
        if (fileDirType[i] != '\\') fileDirType[i] = '\0';
        else break;
      }
    }
    strcat(fileDirType, "*.rbd");
    gotFile = FileSelectPopup("", fileDirType, "", "Readback Bitstream File (.rbd file)",
                              VAL_LOAD_BUTTON, 0, 1, 1, 0, rbdFilePath);
    if (gotFile) {
      ProgramVertexIIFPGAFromPROM();
      confirmVerify = TRUE;
      strncpy(rbbFilePath, rbdFilePath, strlen(rbdFilePath)-4);
      rbbFilePath[strlen(rbdFilePath)-4] = '\0';
      strcat(rbbFilePath, ".rbb");
      strncpy(bitFilePath, rbdFilePath, strlen(rbdFilePath)-4);
      bitFilePath[strlen(rbdFilePath)-4] = '\0';
      strcat(bitFilePath, ".bit");
      strncpy(mcsFilePath, rbdFilePath, strlen(rbdFilePath)-4);
      mcsFilePath[strlen(rbdFilePath)-4] = '\0';
      strcat(mcsFilePath, ".mcs");
      strncpy(maskFilePath, rbdFilePath, strlen(rbdFilePath)-4);
      maskFilePath[strlen(rbdFilePath)-4] = '\0';
      strcat(maskFilePath, ".msk");
      strncpy(msdFilePath, rbdFilePath, strlen(rbdFilePath)-4);
      msdFilePath[strlen(rbdFilePath)-4] = '\0';
      strcat(msdFilePath, ".msd");
    }
    else {
      confirmVerify = FALSE;
      printf("Readback bitstream file is not selected, cancel FPGA verification.\n");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Select Readback Bitstream File", 
                 "Push this button to select readback bitstream file (.rbd file).");
    controlOption = NORM;
  }
}


void SelectRBBFile(void) {
  int panel, i, gotFile;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    if (panel == ConfirmVFPHandle) PanelDone();
    if (strlen(rbbFilePath) <= 0) strcpy(fileDirType, "bitFiles\\");
    else {
      strcpy(fileDirType, rbbFilePath);
      for (i = strlen(fileDirType); i >= 0; i--) {
        if (fileDirType[i] != '\\') fileDirType[i] = '\0';
        else break;
      }
    }
    strcat(fileDirType, "*.rbb");
    gotFile = FileSelectPopup("", fileDirType, "", "Readback Bitstream Binary File (.rbb file)",
                              VAL_LOAD_BUTTON, 0, 1, 1, 0, rbbFilePath);
    if (gotFile) {
      ProgramVertexIIFPGAFromPROM();
      confirmVerify = TRUE;
      strncpy(rbdFilePath, rbbFilePath, strlen(rbbFilePath)-4);
      rbdFilePath[strlen(rbbFilePath)-4] = '\0';
      strcat(rbdFilePath, ".rbd");
      strncpy(bitFilePath, rbbFilePath, strlen(rbbFilePath)-4);
      bitFilePath[strlen(rbbFilePath)-4] = '\0';
      strcat(bitFilePath, ".bit");
      strncpy(mcsFilePath, rbbFilePath, strlen(rbbFilePath)-4);
      mcsFilePath[strlen(rbbFilePath)-4] = '\0';
      strcat(mcsFilePath, ".mcs");
      strncpy(maskFilePath, rbbFilePath, strlen(rbbFilePath)-4);
      maskFilePath[strlen(rbbFilePath)-4] = '\0';
      strcat(maskFilePath, ".msk");
      strncpy(msdFilePath, rbbFilePath, strlen(rbbFilePath)-4);
      msdFilePath[strlen(rbbFilePath)-4] = '\0';
      strcat(msdFilePath, ".msd");
    }
    else {
      confirmVerify = FALSE;
      printf("Readback bitstream binary file is not selected, cancel FPGA verification.\n");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Select Readback Bitstream File", 
                 "Push this button to select readback bitstream binary file (.rbd file).");
    controlOption = NORM;
  }
}


void CheckCSMDAQEanbleAndMezzJTAGEnable(void) {
  int enable, resp;
  char str[256];
  
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, &enable);
  if (enable && mezzCardsOn)
    strcpy(str, "CSM DAQ and mezzanine card JTAG signals are enabled, do you wish to turn them off?\n");
  else if (mezzCardsOn)
    strcpy(str, "Mezzanine card JTAG signals are enabled, do you wish to turn them off?\n");
  if (enable)
    strcpy(str, "CSM DAQ is enabled, do you wish to turn it off?\n");
  if (enable || mezzCardsOn) {
    resp = ConfirmPopup("Turn Off Confirmation", str);
    if (resp) StopCSMDAQAndTurnOffMezzJTAGSignals();
  }
}


void PROMInstruction(FILE *file, int instr, int tckCycles) {
  int i, divider, waitTime;
  
  if (file == NULL) return;
  if (instr < 0) {
    if (instr == -1) {
      fprintf(file, "// SVF File generated by CSM JTAG Control on %s\n", WeekdayTimeString(time(NULL))); 
      fprintf(file, "//\n");
      fprintf(file, "TRST OFF;\n");
      fprintf(file, "ENDIR IDLE;\n");
      fprintf(file, "ENDDR IDLE;\n");
      fprintf(file, "STATE RESET IDLE;\n");
      if (action == DOWNLOAD) ResetTAP();
    }
    else if ((instr == -2) && (tckCycles > 0)) {
      fprintf(file, "RUNTEST %d TCK;\n", tckCycles);
      if (action == DOWNLOAD) {
        if (RunTCKAtTAPIdle(tckCycles) != 0) {
          GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, &divider);
          waitTime = (tckCycles * (25 * (1 << (divider + 1)))) / 1000000;
          if ((tckCycles != 0) && (waitTime == 0)) waitTime = 1;
          while (waitTime > 0) {
            i = 50 - waitTime;
            if (i <= 0) {
              HandleUserButtonAndFunction(PanelDone);
              i = 50;
            }
            else i = waitTime;
            WaitInMillisecond(i);
            waitTime -= i;
          }
        }
      }
    }
    else if ((instr == -3) && (tckCycles > 0)) {
      waitTime = tckCycles;           // waitTime in millisecond
      tckCycles = 1000 * tckCycles;   
      fprintf(file, "// Wait %d ms = %d TCK (assumming JTAG clock period 1us)\n", waitTime, tckCycles);
      fprintf(file, "RUNTEST %d TCK;\n", tckCycles);
      if (action == DOWNLOAD) {
        while (waitTime > 0) {
          i = 50 - waitTime;
          if (i <= 0) {
            HandleUserButtonAndFunction(PanelDone);
            i = 50;
          }
          else i = waitTime;
          WaitInMillisecond(i);
          waitTime -= i;
        }
      }
    }
    return;
  }
  instrLength = 0;
  if (mezzCardsOn || turnOffMezzCardJTAG) {
    for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
      if ((CSMSetupArray[TDCENABLES+i] == 1) || turnOffMezzCardJTAG) {
          if(i==NEWTDC_NUMBER){
            IntToBinary(TDC_BYPASS, instrLength, TDC_INSTR_LENGTH, instrArray, JTAGINSTRLENGTH);
            IntToBinary(TDC_BYPASS, instrLength, TDC_INSTR_LENGTH, secondInstrArray, JTAGINSTRLENGTH);
            instrLength += TDC_INSTR_LENGTH;
		  }
          else{ 
		//Modified by Xiangting
		        IntToBinary(HPTDCBYPASS, instrLength, HPTDCINSTLENGTH, instrArray, JTAGINSTRLENGTH);
		        instrLength += HPTDCINSTLENGTH;
		        HPTDCJTAGComments(HPTDCBYPASS, file, i);
		        IntToBinary(A3P250BYPASS, instrLength, A3P250INSTLENGTH, instrArray, JTAGINSTRLENGTH);
		        instrLength += A3P250INSTLENGTH;
		        A3P250JTAGComments(A3P250BYPASS, file, i);
		  
		//        IntToBinary(AMTBYPASS, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
		//        instrLength += AMTINSTLENGTH;
//        AMTJTAGComments(AMTBYPASS, file, i);
		 }
      }
    }
  }
//End  
  
  if (CSMOn) {
    IntToBinary(CSMBYPASS, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += CSMINSTLENGTH;
    CSMJTAGComments(CSMBYPASS, file);		  
  }
  if (TTCrxOn) {
    IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += TTCRXINSTLENGTH;
    TTCrxJTAGComments(TTCRXBYPASS, file);		  
  }
  if (GOLOn) {
    IntToBinary(GOLBYPASS, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += GOLINSTLENGTH;
    GOLJTAGComments(GOLBYPASS, file);		  
  }
  if (AX1000On) {
    IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += AX1000INSTLENGTH;
    AX1000JTAGComments(AX1000BYPASS, file);		  
  }
  if (XC2V1000On || XC2V2000On) {
    IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += VERTEXIIINSTLENGTH;
    VertexIIJTAGComments(VERTEXIIBYPASS, file);		  
  }
  if (PROMOn) {
    PROMOffset = instrLength;
    IntToBinary(instr, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += PROMINSTLENGTH;
    PROMJTAGComments(instr, file);		  
  }
  WriteJTAGInstructions(file);
  if (tckCycles > 0) fprintf(file, "RUNTEST %d TCK;\n", tckCycles);
  if (action == DOWNLOAD) {
    JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
    if (RunTCKAtTAPIdle(tckCycles) != 0) {
      GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, &divider);
      waitTime = (tckCycles * (25 * (1 << (divider + 1)))) / 1000000;
      if ((tckCycles != 0) && (waitTime == 0)) waitTime = 1;
      while (waitTime > 0) {
        i = 50 - waitTime;
        if (i <= 0) {
          HandleUserButtonAndFunction(PanelDone);
          i = 50;
        }
        else i = waitTime;
        WaitInMillisecond(i);
        waitTime -= i;
      }
    }
  }
}


void PROMData(FILE *file, int bitLength, int data[], int mask, int tckCycles) {
  int i, divider, waitTime;
  
  if (file == NULL) return;
  dataLength = 0;
  if (mezzCardsOn) {
    for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
      if (CSMSetupArray[TDCENABLES+i] == 1) {
        if(i==NEWTDC_NUMBER){
          HPTDCOffset[i] = dataLength;
          dataArray[dataLength] = 1;
          maskArray[dataLength++] = 0;
        }
        else{
//Modified by Xiangting	
          HPTDCOffset[i] = dataLength;
          dataArray[dataLength] = 1;
          maskArray[dataLength++] = 0;
          A3P250Offset[i] = dataLength;
          dataArray[dataLength] = 1;
          maskArray[dataLength++] = 0;
  		
//        mezzOffset[i] = dataLength;
//        dataArray[dataLength] = 1;
//        maskArray[dataLength++] = 0;
        }
      }
    }
  }
  if (CSMOn) {
    CSMOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
  }
  if (TTCrxOn) {
    TTCrxOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
  }
  if (GOLOn) {
    GOLOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
  }
  if (AX1000On) {
    AX1000Offset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
  }
  if (XC2V1000On || XC2V2000On) {
    vertexIIOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
  }
  if (PROMOn) {
    PROMOffset = dataLength;
    for (i = 0; i < bitLength; i++) {
      dataArray[dataLength] = (data[i/32] >> (i%32)) & 1;
      maskArray[dataLength++] = mask % 2;
    }
  }
  WriteJTAGDataOnly(file, mask);
  if (tckCycles > 0) fprintf(file, "RUNTEST %d TCK;\n", tckCycles);
  if (action == DOWNLOAD) {
    JTAGScanAllData(dataLength, dataArray, readbackArray);
    if (RunTCKAtTAPIdle(tckCycles) != 0) {
      GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, &divider);
      waitTime = (tckCycles * (25 * (1 << (divider + 1)))) / 1000000;
      if ((tckCycles != 0) && (waitTime == 0)) waitTime = 1;
      while (waitTime > 0) {
        i = 50 - waitTime;
        if (i <= 0) {
          HandleUserButtonAndFunction(PanelDone);
          i = 50;
        }
        else i = waitTime;
        WaitInMillisecond(i);
        waitTime -= i;
      }
    }
  }
}


void FPGAInstruction(FILE *file, int instr, int tckCycles) {
  int i, divider, waitTime;
  
  if (file == NULL) return;
  if (instr < 0) {
    fprintf(file, "// SVF File generated by CSM JTAG Control on %s\n", WeekdayTimeString(time(NULL))); 
    fprintf(file, "//\n");
    fprintf(file, "TRST OFF;\n");
    fprintf(file, "ENDIR IDLE;\n");
    fprintf(file, "ENDDR IDLE;\n");
    fprintf(file, "STATE RESET IDLE;\n");
    return;
  }
  //JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, instr, TTCRXBYPASS, GOLBYPASS, instr, instr, PROMBYPASS);  
  instrLength = 0;
  if (mezzCardsOn || turnOffMezzCardJTAG) {
    for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
      if ((CSMSetupArray[TDCENABLES+i] == 1) || turnOffMezzCardJTAG) {
        if(i==NEWTDC_NUMBER){
          IntToBinary(TDC_BYPASS, instrLength, TDC_INSTR_LENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += TDC_INSTR_LENGTH;
        }
		  
//Modified by Xiangting
        else{		  
          IntToBinary(HPTDCBYPASS, instrLength, HPTDCINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += HPTDCINSTLENGTH;
          HPTDCJTAGComments(HPTDCBYPASS, file, i);
          IntToBinary(A3P250BYPASS, instrLength, A3P250INSTLENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += A3P250INSTLENGTH;
          A3P250JTAGComments(A3P250BYPASS, file, i);
		
//        IntToBinary(AMTBYPASS, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
//        instrLength += AMTINSTLENGTH;
//        AMTJTAGComments(AMTBYPASS, file, i);
        }
      }
    }
  }
  if (CSMOn) {
    IntToBinary(instr, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += CSMINSTLENGTH;
    CSMJTAGComments(CSMBYPASS, file);		  
  }
  if (TTCrxOn) {
    IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += TTCRXINSTLENGTH;
    TTCrxJTAGComments(TTCRXBYPASS, file);		  
  }
  if (GOLOn) {
    IntToBinary(GOLBYPASS, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += GOLINSTLENGTH;
    GOLJTAGComments(GOLBYPASS, file);		  
  }
  if (AX1000On) {
    IntToBinary(instr, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += AX1000INSTLENGTH;
    AX1000JTAGComments(instr, file);		  
  }
  if (XC2V1000On || XC2V2000On) {
    IntToBinary(instr, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += VERTEXIIINSTLENGTH;
    VertexIIJTAGComments(instr, file);		  
  }
  if (PROMOn) {
    IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
    instrLength += PROMINSTLENGTH;
    PROMJTAGComments(PROMBYPASS, file);		  
  }
  if (tckCycles > 0) fprintf(file, "RUNTEST %d TCK;\n", tckCycles);
  if (action == DOWNLOAD) {
    JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
    if (RunTCKAtTAPIdle(tckCycles) != 0) {
      GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, &divider);
      waitTime = (tckCycles * (25 * (1 << (divider + 1)))) / 1000000;
      if ((tckCycles != 0) && (waitTime == 0)) waitTime = 1;
      while (waitTime > 0) {
        i = 50 - waitTime;
        if (i <= 0) {
          HandleUserButtonAndFunction(PanelDone);
          i = 50;
        }
        else i = waitTime;
        WaitInMillisecond(i);
        waitTime -= i;
      }
    }
  }
}


void FPGAData(FILE *file, int bitLength, int data[], int mask, int tckCycles, int endData) {
  static int i, divider, waitTime, firstData = TRUE;
  
  if (file == NULL) return;
  dataLength = 0;
  if (firstData) {
    firstData = FALSE;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if(i==NEWTDC_NUMBER){
            HPTDCOffset[i] = dataLength;
            dataArray[dataLength] = 1;
            maskArray[dataLength++] = 0;
          }
          else{
//Modified by Xiangting
            HPTDCOffset[i] = dataLength;
            dataArray[dataLength] = 1;
            maskArray[dataLength++] = 0;
            A3P250Offset[i] = dataLength;
            dataArray[dataLength] = 1;
            maskArray[dataLength++] = 0;	  
//          mezzOffset[i] = dataLength;
//          dataArray[dataLength] = 1;
//          maskArray[dataLength++] = 0;
          }
        }
      }
    }
    if (CSMOn) {
      CSMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
    }
    if (TTCrxOn) {
      TTCrxOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
    }
    if (GOLOn) {
      GOLOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
    }
  }
  if (AX1000On) {
    AX1000Offset = dataLength;
    for (i = 0; i < bitLength; i++) {
      dataArray[dataLength] = (data[i/32] >> (i%32)) & 1;
      maskArray[dataLength++] = mask % 2;
    }
  }
  if (XC2V1000On || XC2V2000On) {
    vertexIIOffset = dataLength;
    for (i = 0; i < bitLength; i++) {
      dataArray[dataLength] = (data[i/32] >> (i%32)) & 1;
      maskArray[dataLength++] = mask % 2;
    }
  }
  if (PROMOn && endData) {
    firstData = TRUE;
    PROMOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
  }
  WriteJTAGDataOnly(file, mask);
  if (tckCycles > 0) fprintf(file, "RUNTEST %d TCK;\n", tckCycles);
  if (action == DOWNLOAD) {
    JTAGScanData(dataLength, dataArray, readbackArray, endData);
    if (RunTCKAtTAPIdle(tckCycles) != 0) {
      GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, &divider);
      waitTime = (tckCycles * (25 * (1 << (divider + 1)))) / 1000000;
      if ((tckCycles != 0) && (waitTime == 0)) waitTime = 1;
      while (waitTime > 0) {
        i = 50 - waitTime;
        if (i <= 0) {
          HandleUserButtonAndFunction(PanelDone);
          i = 50;
        }
        else i = waitTime;
        WaitInMillisecond(i);
        waitTime -= i;
      }
    }
  }
}


void FPGABinaryData(FILE *file, int bitLength, int data[], int mask, int tckCycles, int endData) {
  static int i, divider, waitTime, firstData = TRUE;
  
  if (file == NULL) return;
  dataLength = 0;
  if (firstData) {
    firstData = FALSE;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if(i==NEWTDC_NUMBER){
            HPTDCOffset[i] = dataLength;
            dataArray[dataLength] = 1;
            maskArray[dataLength++] = 0;
          }
          else{
            //Modified by Xiangting
            HPTDCOffset[i] = dataLength;
            dataArray[dataLength] = 1;
            maskArray[dataLength++] = 0;
            A3P250Offset[i] = dataLength;
            dataArray[dataLength] = 1;
            maskArray[dataLength++] = 0;
		  
//          mezzOffset[i] = dataLength;
//          dataArray[dataLength] = 1;
//          maskArray[dataLength++] = 0;
          }
        }
      }
    }
    if (CSMOn) {
      CSMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
    }
    if (TTCrxOn) {
      TTCrxOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
    }
    if (GOLOn) {
      GOLOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
    }
  }
  if (AX1000On) {
    AX1000Offset = dataLength;
    for (i = 0; i < bitLength; i++) {
      dataArray[dataLength] = data[i];
      maskArray[dataLength++] = mask % 2;
    }
  }
  if (XC2V1000On || XC2V2000On) {
    vertexIIOffset = dataLength;
    for (i = 0; i < bitLength; i++) {
      dataArray[dataLength] = data[i];
      maskArray[dataLength++] = mask % 2;
    }
  }
  if (PROMOn && endData) {
    firstData = TRUE;
    PROMOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
  }
#ifdef DEBUGVERIFYFPGAORPROM
  WriteJTAGDataOnly(file, mask&0x1);
#endif
  if (tckCycles > 0) fprintf(file, "RUNTEST %d TCK;\n", tckCycles);
  if (action == DOWNLOAD) {
    if ((mask&2) == 0) JTAGScanData(dataLength, dataArray, readbackArray, endData);
    else JTAGScanData(dataLength, zeroArray, readbackArray, endData);
    if (RunTCKAtTAPIdle(tckCycles) != 0) {
      GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, &divider);
      waitTime = (tckCycles * (25 * (1 << (divider + 1)))) / 1000000;
      if ((tckCycles != 0) && (waitTime == 0)) waitTime = 1;
      if (waitTime > 0) {
        i = 50 - waitTime;
        if (i <= 0) {
          HandleUserButtonAndFunction(PanelDone);
          i = 50;
        }
        else i = waitTime;
        WaitInMillisecond(i);
        waitTime -= i;
      }
    }
  }
}


// Function for downloading varies devices
//
void DownloadCSMSetup(void) {
  char path[256];
  static int i, tryAgain = TRUE;
  FILE *SVFFile;
  
  if (controlOption == NORM) {
    strcpy(path, "downloadCSMSetup.svf");
    if (SVFFile = fopen(path, "w")) {
      LoadCSMSetupArray();
      LoadGOLSetupArray();
      LoadTTCrxSetupArray();
      GetJTAGChainOnList();
      AllJTAGDeviceInBYPASS(SVFFile, 0);

      JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMFULLRW, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);  
      JTAGdownload_instr(secondInstrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMFULLRO, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);  
  
      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMFULLRO, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        if (CSMNextState == CMDRESETTTCRX) WaitInMillisecond(1000);       // Wait for 1 second
        else if (CSMNextState == CMDRESETGOL) WaitInMillisecond(1000);    // Wait for 1 second
        else if (CSMNextState == CMDRESETERROR) WaitInMillisecond(1000);  // Wait for 1 second
        else if (CSMNextState == CMDTTCRXLOAD) WaitInMillisecond(1000);   // Wait for 1 second
        JTAGScanAllInstruction(instrLength, secondInstrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        CheckFullJTAGDataArray();
		printf("DownloadCSMSetup is ongoing\n");
		printf("DownloadCSMSetup is ongoing\n");
        if (CSMOn) {
          downloadCSMSetupDone = TRUE;
          CSMLastNextState = CSMNextState;
          if (downloadCSMStatus == 0) {
            for (i = 0; i < CSMSTATUSLENGTH; i++) CSMStatusArray[i] = readbackArray[CSMOffset+i];
            printf("CSM and TTCrx setup is downloaded successfully through JTAG.\n");
          }
          else {
            numberBadCSMDownload++;
            printf("Failed in downloading CSM and TTCrx setup through JTAG, downloadCSMStatus = %d\n", downloadCSMStatus);
          }
          numberCSMDownload++;
        }
        UpdateJTAGControlPanel();     // Update JTAG control panel
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
      if (tryAgain && (action == DOWNLOAD) && (downloadCSMStatus != 0)) {
        tryAgain = FALSE;
        printf("Try to download CSM and TTCrx setup again.\n");
        DownloadCSMSetup();
      }
      else tryAgain = TRUE;
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Download CSM Setup", 
                 "Download Current CSM Setup through JTAG.");
    controlOption = NORM;
  }
}


void DownloadGOLSetup(void) {
  char path[256];
  int i;
  FILE *SVFFile;
  
  if (controlOption == NORM) {
    strcpy(path, "downloadGOLSetup.svf");
    if (SVFFile = fopen(path, "w")) {
      LoadCSMSetupArray();
      LoadGOLSetupArray();
      LoadTTCrxSetupArray();
      GetJTAGChainOnList();
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      
      JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLCONFRW, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);  
      JTAGdownload_instr(secondInstrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLCONFRO, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);  

      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLCONFRO, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        JTAGScanAllInstruction(instrLength, secondInstrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        CheckFullJTAGDataArray();
		
		printf("DownloadGOLSetup is ongoing\n");
		printf("DownloadGOLSetup is ongoing\n");
		
		
        if (downloadGOLStatus == 0)
          printf("GOL setup is downloaded successfully through JTAG.\n");
        else
          printf("Failed in downloading GOL setup through JTAG, downloadGOLStatus = %d\n", downloadGOLStatus);
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Download GOL Setup", 
                 "Download Current GOL Setup through JTAG.");
    controlOption = NORM;
  }
}


void DownloadTTCrxSetup(void) {
  int cmd, PROMUsage;
  
  if (controlOption == NORM) {
    ResetAndDownloadTTCrxMethod1();
  }
  else if (controlOption == HELP) {
    MessagePopup("Download TTCrx Setup", 
                 "Download Current TTCrx Setup through JTAG.");
    controlOption = NORM;
  }
}


void DownloadMezzCardSetup(void) {
  int version, mezz, failed;
  FILE *file;
  
  if (controlOption == NORM) {
    BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
    if ((version < 10) || (nbMezzCard <= 0)) return;
    if (action == DOWNLOAD) {
      if (mezzCardSetupAll) 
        printf("\nProgramming all the Mezzanine Cards (=%d)\n", nbMezzCard);
      else
        printf("\nProgramming the Mezzanine Card %d individually\n", mezzCardNb);
    }
 //Modified by Xiangting   
	//DownloadAMTSetup();
    //DownloadASDSetup();
	if (file = fopen("ASD_setup_debug.txt", "a"));
	DownloadHPTDCSetup();
	DownloadHPTDCControl(1);
	DownloadHPTDCControl(2);
	DownloadHPTDCControl(3);
	DownloadHPTDCControl(4);
	DownloadHPTDCControl(5);
	DownloadHPTDCControl(6);
	
    DownloadA3P250Setup();
	
	
    DownloadMDTTDCSetup(TDC_SETUP0_INSTR,0);
	fprintf(file,"TDC_SETUP0_INSTR:\n");
	fprintSDR_TDI(file);
    fprintSDR_TDO(file);
    fprint_mask(file); 
    check_data(file);
	
    DownloadMDTTDCSetup(TDC_SETUP1_INSTR,0);
	fprintf(file,"TDC_SETUP1_INSTR:\n");
	fprintSDR_TDI(file);
    fprintSDR_TDO(file);
    fprint_mask(file); 
    check_data(file);
	
	DownloadMDTTDCSetup(TDC_CONTROL0_INSTR,1);
	fprintf(file,"TDC_CONTROL0_INSTR_1:\n");
	fprintSDR_TDI(file);
    fprintSDR_TDO(file);
    fprint_mask(file); 
    check_data(file);
	   
  	DownloadMDTTDCSetup(TDC_CONTROL0_INSTR,2);
	fprintf(file,"TDC_CONTROL0_INSTR_2:\n");
	fprintSDR_TDI(file);
    fprintSDR_TDO(file);
    fprint_mask(file); 
    check_data(file);
	
	DownloadMDTTDCSetup(TDC_CONTROL0_INSTR,3);
	fprintf(file,"TDC_CONTROL0_INSTR_3:\n");
	fprintSDR_TDI(file);
    fprintSDR_TDO(file);
    fprint_mask(file); 
    check_data(file);  
	fclose(file);
//End
	downloadMezzSetupDone = TRUE;
    numberMezzDownload++;
    failed = FALSE;
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      if (CSMSetupArray[TDCENABLES+mezz] == 1) {
        if(mezz==NEWTDC_NUMBER){
          if (downloadHPTDCStatus[mezz] != 0) failed = TRUE;
        }
        else{
          if (downloadHPTDCStatus[mezz] != 0) failed = TRUE;
          if (downloadA3P250Status[mezz] != 0) failed = TRUE;
        }
        
//		if (downloadAMTStatus[mezz] != 0) failed = TRUE;
//        if (downloadASDStatus[mezz] != 0) failed = TRUE;
      }
    }
    if (failed) numberBadMezzDownload++;
  }
  else if (controlOption == HELP) {
    if (mezzCardSetupAll)
      MessagePopup("Download Common Mezzanine Card Setup", 
                   "Download Current Common Mezzanine Card Setup through JTAG to all mezzanine cards.");
    else
      MessagePopup("Download Individual Mezzanine Card Setup", 
                   "Download Current Individual Mezzanine Card Setup through JTAG.");
    controlOption = NORM;
  }
}


//Modified by Xiangting


void fprintSDR_TDI(FILE *file){
  int i;
  fprintf(file,"SDR TDI:"); 
  for(i=0;i<dataLength;i++)
	  fprintf(file,"%d",dataArray[i]);
  fprintf(file,"\n");
}

void fprintSDR_TDO(FILE *file){
  int i;
  fprintf(file,"SDR TDO:"); 
  for(i=0;i<dataLength;i++)
	  fprintf(file,"%d",readbackArray[i]);
  fprintf(file,"\n");
}
void fprint_mask(FILE *file){
  int i;
  fprintf(file,"mask_Ar:"); 
  for(i=0;i<dataLength;i++)
	  fprintf(file,"%d",maskArray[i]);
  fprintf(file,"\n");
}

void check_data(FILE *file){
  int i;
  int error=0;
  for(i=0;i<dataLength;i++)
	  if(maskArray[i]==1)
		  if(dataArray[i]!=readbackArray[i])
			  error++;
  if(error)
      fprintf(file,"%d unmatched bits!\n",error);
  else
	  fprintf(file,"All bits matched.\n");
}


void DownloadA3P250Setup(void) {

  char path[256];
  int i, j,instr, length,l,asd, hyst, thre, sizelong;
  FILE *SVFFile;
  FILE *file;
  if (file = fopen("ASD_setup_debug.txt", "a")); 

  strcpy(path, "downloadA3P250Setup.svf");
  if (SVFFile = fopen(path, "w")) {
	//LoadA3P250SetupArray();	

    JTAGdownload_instr(instrArray, TDC_ASD_CONFIG_INSTR, HPTDCBYPASS, A3P250ASD_WRITE, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
    JTAGdownload_instr(secondInstrArray, TDC_ASD_CONFIG_INSTR, HPTDCBYPASS, A3P250ASD_READ, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
    
    JTAGdownload_data(TDC_ASD_CONFIG_INSTR, HPTDCBYPASS, A3P250ASD_WRITE, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);

    if (action == DOWNLOAD) {
	  //dataLength++; 
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
  
      JTAGScanAllInstruction(instrLength, secondInstrArray, readbackArray); 
      JTAGScanAllData(dataLength, dataArray, readbackArray);
	  
	  sizelong = sizeof(dataArray)/sizeof(dataArray[0]);
	  printf("The size of dataArray is %d\n", sizelong);
	  sizelong = sizeof(readbackArray)/sizeof(dataArray[0]);

	  printf("The size of readbackArray is %d\n", sizelong);  	  
	  
	  printf("DownloadA3P250Setup is ongoing\n");
	  
	  //for(i=0;i<300;i++){
	//	printf("dataArray[%d]=%d,readbackArray[%d]=%d\n",i,dataArray[i],i,readbackArray[i]);            
	  
	 // }
	  
      CheckFullJTAGDataArray();

	  																			
	  printf("DownloadA3P250Setup is ongoing\n");
	  
	  
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) {
            if (downloadA3P250Status[i] == 0)
              printf("A3P250 setup is downloaded successfully through JTAG for mezzanine card %d.\n", i);
            else  
              printf("Failed in downloading A3P250 setup through JTAG for mezzanine card %d, downloadA3P250Status = %d\n", i, downloadA3P250Status[i]);
          }
          else if(mezzCardNb == i) {
            if (downloadA3P250Status[i] == 0)
              printf("A3P250 setup is downloaded successfully through JTAG for mezzanine card %d.\n", i);
            else  
              printf("Failed in downloading A3P250 setup through JTAG for mezzanine card %d, downloadA3P250Status = %d\n", i, downloadA3P250Status[i]);
          }
        }
      }
    }
    AllJTAGDeviceInBYPASS(SVFFile, 1);
    fclose(SVFFile);
  }
  else {
    printf("Unable to open SVF file <%s>!\n", path);
    MessagePopup("Failed to Open SVF File",
                 "Unable to open SVF file, please check your disk and filename!");
  }
  fprintf(file,"Download A3P250 finished!\n");
  fprintSDR_TDI(file);
  fprintSDR_TDO(file);
  fprint_mask(file);
  check_data(file);
  fclose(file);
  
}


void DownloadHPTDCSetup(void) {
  char path[256];
  int i, j,instr, length, l, asd, hyst, thre;
  FILE *SVFFile;
  FILE *file;
  if (file = fopen("ASD_setup_debug.txt", "a"));
  strcpy(path, "downloadHPTDCSetup.svf");
  if (SVFFile = fopen(path, "w")) {
	  LoadHPTDCSetupArray();	
    GetJTAGChainOnList();
    AllJTAGDeviceInBYPASS(SVFFile, 0);
    
    JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCSETUP, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);    
    JTAGdownload_data(TDC_BYPASS, HPTDCSETUP, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
  
    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      CheckFullJTAGDataArray();
	  printf("DownloadHPTDCSetup is ongoing\n");
	  printf("DownloadHPTDCSetup is ongoing\n"); 
		  
		  
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if(i!=NEWTDC_NUMBER){
            if (mezzCardSetupAll) {
              if (downloadHPTDCStatus[i] == 0)
                printf("HPTDC setup is downloaded successfully through JTAG for mezzanine card %d.\n", i);
              else  
                printf("Failed in downloading HPTDC setup through JTAG for mezzanine card %d, downloadHPTDCStatus = %d\n", i, downloadHPTDCStatus[i]);
            }
            else if(mezzCardNb == i) {
              if (downloadHPTDCStatus[i] == 0)
                printf("HPTDC setup is downloaded successfully through JTAG for mezzanine card %d.\n", i);
              else  
                printf("Failed in downloading HPTDC setup through JTAG for mezzanine card %d, downloadHPTDCStatus = %d\n", i, downloadHPTDCStatus[i]);
            }
          }          
        }
      }
    }
    AllJTAGDeviceInBYPASS(SVFFile, 1);
    fclose(SVFFile);
  }
  else {
    printf("Unable to open SVF file <%s>!\n", path);
    MessagePopup("Failed to Open SVF File",
                 "Unable to open SVF file, please check your disk and filename!");
  }	
  fprintf(file,"Download HPTDCSetup finished!");
  //fprintSDR_TDI(file);
  //fprintSDR_TDO(file);
  //fprint_mask(file); 
  check_data(file);
  fclose(file);
}



//END
/*
void DownloadAMTSetup(void) {
  char path[256];
  int i, instr, length, l, asd, hyst, thre;
  FILE *SVFFile;

  strcpy(path, "downloadAMTSetup.svf");
  if (SVFFile = fopen(path, "w")) {
    LoadCSMSetupArray();
    LoadGOLSetupArray();
    LoadTTCrxSetupArray();
    GetJTAGChainOnList();
    AllJTAGDeviceInBYPASS(SVFFile, 0);
    instrLength = 0;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) instr = AMTCONTROL;
          else if(mezzCardNb == i) instr = AMTCONTROL;
          else instr = AMTBYPASS;
          IntToBinary(instr, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += AMTINSTLENGTH;
          AMTJTAGComments(instr, SVFFile, i);
          WriteJTAGInstructionToActionFile(AMT3ID, instr, i);
        }
      }
    }
    if (CSMOn) {
      IntToBinary(CSMBYPASS, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += CSMINSTLENGTH;
      CSMJTAGComments(CSMBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(CSMID, CSMBYPASS, 0);
    }
    if (TTCrxOn) {
      IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += TTCRXINSTLENGTH;
      TTCrxJTAGComments(TTCRXBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(TTCRXID, TTCRXBYPASS, 0);
    }
    if (GOLOn) {
      IntToBinary(GOLBYPASS, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += GOLINSTLENGTH;
      GOLJTAGComments(GOLBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(GOLID, GOLBYPASS, 0);
    }
    if (AX1000On) {
      IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += AX1000INSTLENGTH;
      AX1000JTAGComments(AX1000BYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(AX1000ID, AX1000BYPASS, 0);
    }
    if (XC2V1000On || XC2V2000On) {
      IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += VERTEXIIINSTLENGTH;
      VertexIIJTAGComments(VERTEXIIBYPASS, SVFFile);		  
      if (XC2V2000On) WriteJTAGInstructionToActionFile(XC2V2000ID, VERTEXIIBYPASS, 0);
      else WriteJTAGInstructionToActionFile(XC2V1000ID, VERTEXIIBYPASS, 0);
    }
    if (PROMOn) {
      IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += PROMINSTLENGTH;
      PROMJTAGComments(PROMBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(PROMID, PROMBYPASS, 0);
    }
    WriteJTAGInstructions(SVFFile);
    dataLength = 0;
    if (mezzCardsOn) {
      nominalThresholdUsed = TRUE;
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          for (l = 0; l < AMTS_SETUP+AMTSETUP_LENGTH; l++) {
            statusSetupCheck[l] = basicSetupArray[l];
            basicSetupArray[l] = mezzSetupArray[l][i];
          }
          IntToBinary(i, AMTS_SETUP+AMTS_TDC_ID, 4, basicSetupArray, SETUP_LENGTH); 
          if (mezzCardSetupAll) length = AMTSETUP_LENGTH;
          else if(mezzCardNb == i) length = AMTSETUP_LENGTH;
          else length = 1;
          if (length == AMTSETUP_LENGTH) {
            for (l = 0; l < AMTS_SETUP+AMTSETUP_LENGTH; l++) mezzSetupArray[l][i] = basicSetupArray[l];
            if (sampleAMTPhase) {
              basicSetupArray[AMTS_SETUP+AMTS_ENB_SER] = 1;
              IntToBinary(AMTEDGECONTINE, AMTS_SETUP+AMTS_STROBE_SEL, 2, basicSetupArray, SETUP_LENGTH);
            }
            for (asd = 0; asd < 3; asd++) {
              ReversedBinaryToInt(&thre, asd*ASD_SETUP_LENGTH+ASD_MAIN_THRESH, 8, basicSetupArray);
              ReversedBinaryToInt(&hyst, asd*ASD_SETUP_LENGTH+ASD_HYSTERESIS, 4, basicSetupArray);
              if ((thre != NTThre[asd][i]) || (hyst != NTHyst[asd][i])) nominalThresholdUsed = FALSE;
            }
          }
          mezzOffset[i] = dataLength;
          for (l = 0; l < length; l++) {
            if (length == AMTSETUP_LENGTH) {
              dataArray[dataLength] = basicSetupArray[AMTS_SETUP+l];
              maskArray[dataLength++] = 1;
            }
            else {
              dataArray[dataLength] = 1;
              maskArray[dataLength++] = 0;
            }
          }
          for (l = 0; l < AMTS_SETUP+AMTSETUP_LENGTH; l++) {
            basicSetupArray[l] = statusSetupCheck[l];
          }
          WriteJTAGDataToActionFile(length, mezzOffset[i]);
        }
      }
    }
    if (CSMOn) {
      CSMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, CSMOffset);
    }
    if (TTCrxOn) {
      TTCrxOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, TTCrxOffset);
    }
    if (GOLOn) {
      GOLOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, GOLOffset);
    }
    if (AX1000On) {
      AX1000Offset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, AX1000Offset);
    }
    if (XC2V1000On || XC2V2000On) {
      vertexIIOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, vertexIIOffset);
    }
    if (PROMOn) {
      PROMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, PROMOffset);
    }
    WriteJTAGData(SVFFile);
    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      CheckFullJTAGDataArray();
	  printf("DownloadAMTSetup is ongoing\n");
	  printf("DownloadAMTSetup is ongoing\n");
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) {
            if (downloadAMTStatus[i] == 0)
              printf("AMT setup is downloaded successfully through JTAG for mezzanine card %d.\n", i);
            else  
              printf("Failed in downloading AMT setup through JTAG for mezzanine card %d, downloadAMTStatus = %d\n", i, downloadAMTStatus[i]);
          }
          else if(mezzCardNb == i) {
            if (downloadAMTStatus[i] == 0)
              printf("AMT setup is downloaded successfully through JTAG for mezzanine card %d.\n", i);
            else  
              printf("Failed in downloading AMT setup through JTAG for mezzanine card %d, downloadAMTStatus = %d\n", i, downloadAMTStatus[i]);
          }
        }
      }
    }
    AllJTAGDeviceInBYPASS(SVFFile, 1);
    fclose(SVFFile);
  }
  else {
    printf("Unable to open SVF file <%s>!\n", path);
    MessagePopup("Failed to Open SVF File",
                 "Unable to open SVF file, please check your disk and filename!");
  }
}


void DownloadASDSetup(void) {
  char path[256];
  int i, instr, length, l, arrayCode[ASD_SETUP_LENGTH*3+1];
  FILE *SVFFile;

  arrayCode[0] = 0;
  for (i = 1; i < ASD_SETUP_LENGTH*3+1; i++) arrayCode[i] = basicSetupArray[i-1];
  ReverseArray(arrayCode, 1, ASD_SETUP_LENGTH);
  ReverseArray(arrayCode, ASD_SETUP_LENGTH+1, ASD_SETUP_LENGTH);
  ReverseArray(arrayCode, ASD_SETUP_LENGTH*2+1, ASD_SETUP_LENGTH);
  
  strcpy(path, "downloadASDSetup.svf");
  if (SVFFile = fopen(path, "w")) {
    LoadCSMSetupArray();
    LoadGOLSetupArray();
    LoadTTCrxSetupArray();
    GetJTAGChainOnList();
    AllJTAGDeviceInBYPASS(SVFFile, 0);
    instrLength = 0;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) instr = AMTASDCONTROL;
          else if(mezzCardNb == i) instr = AMTASDCONTROL;
          else instr = AMTBYPASS;
          IntToBinary(instr, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += AMTINSTLENGTH;
          AMTJTAGComments(instr, SVFFile, i);
          WriteJTAGInstructionToActionFile(AMT3ID, instr, i);
        }
      }
    }
    if (CSMOn) {
      IntToBinary(CSMBYPASS, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += CSMINSTLENGTH;
      CSMJTAGComments(CSMBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(CSMID, CSMBYPASS, 0);
    }
    if (TTCrxOn) {
      IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += TTCRXINSTLENGTH;
      TTCrxJTAGComments(TTCRXBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(TTCRXID, TTCRXBYPASS, 0);
    }
    if (GOLOn) {
      IntToBinary(GOLBYPASS, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += GOLINSTLENGTH;
      GOLJTAGComments(GOLBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(GOLID, GOLBYPASS, 0);
    }
    if (AX1000On) {
      IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += AX1000INSTLENGTH;
      AX1000JTAGComments(AX1000BYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(AX1000ID, AX1000BYPASS, 0);
    }
    if (XC2V1000On || XC2V2000On) {
      IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += VERTEXIIINSTLENGTH;
      VertexIIJTAGComments(VERTEXIIBYPASS, SVFFile);		  
      if (XC2V2000On) WriteJTAGInstructionToActionFile(XC2V2000ID, VERTEXIIBYPASS, 0);
      else WriteJTAGInstructionToActionFile(XC2V1000ID, VERTEXIIBYPASS, 0);
    }
    if (PROMOn) {
      IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += PROMINSTLENGTH;
      PROMJTAGComments(PROMBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(PROMID, PROMBYPASS, 0);
    }
    WriteJTAGInstructions(SVFFile);
    dataLength = 0;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          for (l = 0; l < AMTS_SETUP+AMTSETUP_LENGTH; l++) {
            statusSetupCheck[l] = basicSetupArray[l];
            basicSetupArray[l] = mezzSetupArray[l][i];
          }
          arrayCode[0] = 0;
          for (l = 1; l < ASD_SETUP_LENGTH*3+1; l++) arrayCode[l] = basicSetupArray[l-1];
          ReverseArray(arrayCode, 1, ASD_SETUP_LENGTH);
          ReverseArray(arrayCode, ASD_SETUP_LENGTH+1, ASD_SETUP_LENGTH);
          ReverseArray(arrayCode, ASD_SETUP_LENGTH*2+1, ASD_SETUP_LENGTH);
          IntToBinary(i, AMTS_SETUP+AMTS_TDC_ID, 4, basicSetupArray, SETUP_LENGTH); 
          if (mezzCardSetupAll) length = ASD_SETUP_LENGTH*3+1;
          else if(mezzCardNb == i) length = ASD_SETUP_LENGTH*3+1;
          else length = 1;
          mezzOffset[i] = dataLength;
          for (l = 0; l < length; l++) {
            if (length == (ASD_SETUP_LENGTH*3+1)) {
              dataArray[dataLength] = arrayCode[l];
              maskArray[dataLength++] = 1;
            }
            else {
              dataArray[dataLength] = 1;
              maskArray[dataLength++] = 0;
            }
          }
          for (l = 0; l < AMTS_SETUP+AMTSETUP_LENGTH; l++) {
            basicSetupArray[l] = statusSetupCheck[l];
          }
          WriteJTAGDataToActionFile(length, mezzOffset[i]);
        }
      }
    }
    if (CSMOn) {
      CSMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, CSMOffset);
    }
    if (TTCrxOn) {
      TTCrxOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, TTCrxOffset);
    }
    if (GOLOn) {
      GOLOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, GOLOffset);
    }
    if (AX1000On) {
      AX1000Offset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, AX1000Offset);
    }
    if (XC2V1000On || XC2V2000On) {
      vertexIIOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, vertexIIOffset);
    }
    if (PROMOn) {
      PROMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, PROMOffset);
    }
    WriteJTAGData(SVFFile);
    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      CheckFullJTAGDataArray();
	  printf("DownloadASDSetup is ongoing\n");
	  printf("DownloadASDSetup is ongoing\n");
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) {
            if (downloadASDStatus[i] == 0)
              printf("ASD setup is downloaded successfully through JTAG for mezzanine card %d.\n", i);
            else  
              printf("Failed in downloading ASD setup through JTAG for mezzanine card %d, downloadASDStatus = %d\n", i, downloadASDStatus[i]);
          }
          else if(mezzCardNb == i) {
            if (downloadASDStatus[i] == 0)
              printf("ASD setup is downloaded successfully through JTAG for mezzanine card %d.\n", i);
            else  
              printf("Failed in downloading ASD setup through JTAG for mezzanine card %d, downloadASDStatus = %d\n", i, downloadASDStatus[i]);
          }
        }
      }
    }
    AllJTAGDeviceInBYPASS(SVFFile, 1);
    fclose(SVFFile);
  }
  else {
    printf("Unable to open SVF file <%s>!\n", path);
    MessagePopup("Failed to Open SVF File",
                 "Unable to open SVF file, please check your disk and filename!");
  }
}
*/

// Function for getting varies device status
//
void GetCSMStatus(void) {
  char path[256];
  int i, execCSMCMD;
  FILE *SVFFile;

  if (controlOption == NORM) {
    validCSMStatus = FALSE;
    strcpy(path, "getCSMStatus.svf");
    if (SVFFile = fopen(path, "w")) {
      GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &execCSMCMD);
      if ((action == DOWNLOAD) && !downloadCSMSetupDone) {
        SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 0);
        LoadCSMSetupArray();
        LoadGOLSetupArray();
        LoadTTCrxSetupArray();
      }
      GetJTAGChainOnList();
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      if (CSMOn) {
        if ((action == DOWNLOAD) && !downloadCSMSetupDone) 
          JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMFULLRW, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
        else 
          JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMFULLRO, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
      }

      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMFULLRW, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
  
      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        if (CSMOn && !downloadCSMSetupDone) {
          JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
          JTAGScanAllData(dataLength, dataArray, readbackArray);
          downloadCSMSetupDone = TRUE;
          CSMLastNextState = CSMNextState;
        }
        CheckFullJTAGDataArray();
		printf("GetCSMStatus is ongoing\n");
		printf("GetCSMStatus is ongoing\n");		
        validCSMStatus = TRUE;
        for (i = 0; i < CSMSTATUSLENGTH; i++) CSMStatusArray[i] = readbackArray[CSMOffset+i];
        UpdateJTAGControlPanel();     // Update JTAG control panel
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
      SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, execCSMCMD);
      LoadCSMSetupArray();
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Get CSM Status", 
                 "Get Current CSM Status through JTAG.");
    controlOption = NORM;
  }
}


void GetGOLStatus(void) {
  char path[256];
  int i;
  FILE *SVFFile;

  if (controlOption == NORM) {
    validGOLStatus = FALSE;
    strcpy(path, "getGOLStatus.svf");
    if (SVFFile = fopen(path, "w")) {
      LoadCSMSetupArray();
      LoadGOLSetupArray();
      LoadTTCrxSetupArray();
      GetJTAGChainOnList();
      AllJTAGDeviceInBYPASS(SVFFile, 0);

      JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLCONFRO, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLCONFRO, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
      
      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        validGOLStatus = TRUE;
        for (i = 0; i < WAITTIMEFORREADY; i++) GOLSetupArray[i] = readbackArray[GOLOffset+i];
        for (i = HAMMINGCHECKSUM; i < GOLSETUPLENGTH; i++) GOLSetupArray[i] = readbackArray[GOLOffset+i];
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
      UpdateJTAGControlPanel();
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Get GOL Status", 
                 "Get Current GOL Status through JTAG.");
    controlOption = NORM;
  }
}


void GetGOLBoundaryScan(void) {
  char path[256];
  int i;
  FILE *SVFFile;

  if (controlOption == NORM) {
    if (!validGOLStatus) return;
    strcpy(path, "getGOLBoundaryScan.svf");
    if (SVFFile = fopen(path, "w")) {
      LoadCSMSetupArray();
      LoadGOLSetupArray();
      LoadTTCrxSetupArray();
      GetJTAGChainOnList();
      AllJTAGDeviceInBYPASS(SVFFile, 0);

      JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLEXTEST, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);

      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLEXTEST, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        for (i = 0; i < GOLBSCANLENGTH; i++) GOLBScanArray[i] = readbackArray[GOLOffset+i];
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
      UpdateJTAGControlPanel();
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Get GOL Boundary Scan Registers", 
                 "Access GOL Boundary Scan Registers and Fetch Them.");
    controlOption = NORM;
  }
}


void GetTTCrxStatus(void) {
}

/*
void GetAMTStatus(void) {
  char path[256];
  int i, instr, length, l, gotStatus, val, statusArray[AMTSTATUSLENGTH];
  FILE *SVFFile;

  printf("GetAMTStatus\n");
  mezzCardSetupAll = TRUE;
  strcpy(path, "getAMTStatus.svf");
  if (SVFFile = fopen(path, "w")) {
    LoadCSMSetupArray();
    LoadGOLSetupArray();
    LoadTTCrxSetupArray();
    GetJTAGChainOnList();
    AllJTAGDeviceInBYPASS(SVFFile, 0);
    instrLength = 0;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) instr = AMTSTATUS;
          else if(mezzCardNb == i) instr = AMTSTATUS;
          else instr = AMTBYPASS;
          IntToBinary(instr, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += AMTINSTLENGTH;
          AMTJTAGComments(instr, SVFFile, i);
          WriteJTAGInstructionToActionFile(AMT3ID, instr, i);
        }
      }
    }
    if (CSMOn) {
      IntToBinary(CSMBYPASS, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += CSMINSTLENGTH;
      CSMJTAGComments(CSMBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(CSMID, CSMBYPASS, 0);
    }
    if (TTCrxOn) {
      IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += TTCRXINSTLENGTH;
      TTCrxJTAGComments(TTCRXBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(TTCRXID, TTCRXBYPASS, 0);
    }
    if (GOLOn) {
      IntToBinary(GOLBYPASS, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += GOLINSTLENGTH;
      GOLJTAGComments(GOLBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(GOLID, GOLBYPASS, 0);
    }
    if (AX1000On) {
      IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += AX1000INSTLENGTH;
      AX1000JTAGComments(AX1000BYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(AX1000ID, AX1000BYPASS, 0);
    }
    if (XC2V1000On || XC2V2000On) {
      IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += VERTEXIIINSTLENGTH;
      VertexIIJTAGComments(VERTEXIIBYPASS, SVFFile);		  
      if (XC2V2000On) WriteJTAGInstructionToActionFile(XC2V2000ID, VERTEXIIBYPASS, 0);
      else WriteJTAGInstructionToActionFile(XC2V1000ID, VERTEXIIBYPASS, 0);
    }
    if (PROMOn) {
      IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += PROMINSTLENGTH;
      PROMJTAGComments(PROMBYPASS, SVFFile);		  
      WriteJTAGInstructionToActionFile(PROMID, PROMBYPASS, 0);
    }
    WriteJTAGInstructions(SVFFile);
    dataLength = 0;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          mezzOffset[i] = dataLength;
          if (mezzCardSetupAll) length = AMTSTATUSLENGTH;
          else if(mezzCardNb == i) length = AMTSTATUSLENGTH;
          else length = 1;
          for (l = 0; l < length; l++) {
            dataArray[dataLength] = AMTStatusArray[l][i];
            maskArray[dataLength++] = 0;
          }
        }
      }
    }
    if (CSMOn) {
      CSMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, CSMOffset);
    }
    if (TTCrxOn) {
      TTCrxOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, TTCrxOffset);
    }
    if (GOLOn) {
      GOLOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, GOLOffset);
    }
    if (AX1000On) {
      AX1000Offset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, AX1000Offset);
    }
    if (XC2V1000On || XC2V2000On) {
      vertexIIOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, vertexIIOffset);
    }
    if (PROMOn) {
      PROMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, PROMOffset);
    }
    WriteJTAGData(SVFFile);
    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) gotStatus = TRUE;
          else if(mezzCardNb == i) gotStatus = TRUE;
          else gotStatus = FALSE;
          if (gotStatus) {
            for (l = 0; l < AMTSTATUSLENGTH; l++) {
              AMTStatusArray[l][i] = readbackArray[mezzOffset[i]+l];
              statusArray[l] = AMTStatusArray[l][i];
            }
            printf("AMT Status for mezzanine card %d\n", i);
            printf("coarse error              : %d\n", statusArray[AMTK_course_err]); 
            printf("channel select error      : %d\n", statusArray[AMTK_chan_sel_err]);
            printf("l1 buffer parity error    : %d\n", statusArray[AMTK_L1_buf_parity_err]);
            printf("trigger fifo parity error : %d\n", statusArray[AMTK_trig_fifo_err]);
            printf("trigger matching error    : %d\n", statusArray[AMTK_trig_match_err]);
            printf("readout fifo parity error : %d\n", statusArray[AMTK_ro_parity_err]);
            printf("readout state error       : %d\n", statusArray[AMTK_ro_state_err]);
            printf("setup parity error        : %d\n", statusArray[AMTK_setup_parity_err]);
            printf("jtag instruction error    : %d\n", statusArray[AMTK_jtag_instr_err]);
            printf("setup parity              : %d\n", statusArray[AMTK_setup_read_parity]);
            printf("readout fifo full         : %d\n", statusArray[AMTK_ro_fifo_full]);
            printf("readout fifo empty        : %d\n", statusArray[AMTK_ro_fifo_empty]);
            BinaryToInt(&val, AMTK_L1_write_address, 8, statusArray);
            printf("l1 write address          : %d\n", val);
            printf("l1 overflow               : %d\n", statusArray[AMTK_l1_overflow]);
            printf("l1 overflow recover       : %d\n", statusArray[AMTK_l1_overflow_rcvr]);
            printf("l1 nearly full            : %d\n", statusArray[AMTK_l1_nearly_full]);
            printf("l1 empty                  : %d\n", statusArray[AMTK_l1_empty]);
            BinaryToInt(&val, AMTK_L1_read_address, 8, statusArray);
            printf("l1 read address           : %d\n", val);
            printf("l1 running                : %d\n", statusArray[AMTK_l1_running]);
            printf("trigger fifo full         : %d\n", statusArray[AMTK_trig_fifo_full]);
            printf("trigger fifo nearly full  : %d\n", statusArray[AMTK_trig_fifo_nearly_full]);
            printf("trigger fifo empty        : %d\n", statusArray[AMTK_trig_fifo_empty]);
            BinaryToInt(&val, AMTK_start_address, 8, statusArray);
            printf("l1 start address          : %d\n", val);
            BinaryToInt(&val, AMTK_trig_fifo_occy, 3, statusArray);
            printf("trigger fifo occy         : %d\n", val);
            BinaryToInt(&val, AMTK_course_count, 14, statusArray);
            printf("coarse counter            : %d\n", val);
            BinaryToInt(&val, AMTK_ro_fifo_occy, 6, statusArray);
            printf("readout fifo occy         : %d\n", val);
            BinaryToInt(&val, AMTK_zeros, 2, statusArray);
            printf("zeros                     : %d\n", val);
            BinaryToInt(&val, AMTK_general_in, 4, statusArray);
            printf("general in                : %d\n", val);
          }
        }
      }
    }
    AllJTAGDeviceInBYPASS(SVFFile, 1);
    fclose(SVFFile);
  }
  else {
    printf("Unable to open SVF file <%s>!\n", path);
    MessagePopup("Failed to Open SVF File",
                 "Unable to open SVF file, please check your disk and filename!");
  }
}
*/

void TurnOnOffGOLAndTTCrxFromJTAGChain(void) {
  char path[256];
  static int i, tryAgain = TRUE;
  FILE *SVFFile;
  
  if (controlOption == NORM) {
    strcpy(path, "downloadCSMSetup.svf");
    if (SVFFile = fopen(path, "w")) {
      LoadCSMSetupArray();
      LoadGOLSetupArray();
      LoadTTCrxSetupArray();
      GetJTAGChainOnList();
      instrLength = 0;
      if (mezzCardsOn) {
        for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
          if (CSMSetupArray[TDCENABLES+i] == 1) {
            if(i==NEWTDC_NUMBER){
              IntToBinary(TDC_BYPASS, instrLength, TDC_INSTR_LENGTH, instrArray, JTAGINSTRLENGTH);
              instrLength += TDC_INSTR_LENGTH;
            }
            else{
              //Xiangting
              IntToBinary(HPTDCBYPASS, instrLength, HPTDCINSTLENGTH, instrArray, JTAGINSTRLENGTH);
              instrLength += HPTDCINSTLENGTH;
              HPTDCJTAGComments(HPTDCBYPASS, SVFFile, i);
              WriteJTAGInstructionToActionFile(HPTDCID, HPTDCBYPASS, i);
              IntToBinary(A3P250BYPASS, instrLength, A3P250INSTLENGTH, instrArray, JTAGINSTRLENGTH);
              instrLength += A3P250INSTLENGTH;
              A3P250JTAGComments(A3P250BYPASS, SVFFile, i);
              WriteJTAGInstructionToActionFile(A3P250ID, A3P250BYPASS, i);  
      
//            IntToBinary(AMTBYPASS, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
//            instrLength += AMTINSTLENGTH;
//            AMTJTAGComments(AMTBYPASS, SVFFile, i);
//            WriteJTAGInstructionToActionFile(AMT3ID, AMTBYPASS, i);
            }
			  

          }
        }
      }
      if (CSMOn) {
        IntToBinary(CSMFULLRW, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
        instrLength += CSMINSTLENGTH;
        CSMJTAGComments(CSMFULLRW, SVFFile);		  
        WriteJTAGInstructionToActionFile(CSMID, CSMFULLRW, 0);
      }
      if (oldTTCrxOn) {
        IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
        instrLength += TTCRXINSTLENGTH;
        TTCrxJTAGComments(TTCRXBYPASS, SVFFile);		  
        WriteJTAGInstructionToActionFile(TTCRXID, TTCRXBYPASS, 0);
      }
      if (oldGOLOn) {
        IntToBinary(GOLBYPASS, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
        instrLength += GOLINSTLENGTH;
        GOLJTAGComments(GOLBYPASS, SVFFile);		  
        WriteJTAGInstructionToActionFile(GOLID, GOLBYPASS, 0);
      }
      if (AX1000On) {
        IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
        instrLength += AX1000INSTLENGTH;
        AX1000JTAGComments(AX1000BYPASS, SVFFile);		  
        WriteJTAGInstructionToActionFile(AX1000ID, AX1000BYPASS, 0);
      }
      if (XC2V1000On || XC2V2000On) {
        IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
        instrLength += VERTEXIIINSTLENGTH;
        VertexIIJTAGComments(VERTEXIIBYPASS, SVFFile);		  
        if (XC2V2000On) WriteJTAGInstructionToActionFile(XC2V2000ID, VERTEXIIBYPASS, 0);
        else WriteJTAGInstructionToActionFile(XC2V1000ID, VERTEXIIBYPASS, 0);
      }
      if (PROMOn) {
        IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
        instrLength += PROMINSTLENGTH;
        PROMJTAGComments(PROMBYPASS, SVFFile);		  
        WriteJTAGInstructionToActionFile(PROMID, PROMBYPASS, 0);
      }
      WriteJTAGInstructions(SVFFile);
      dataLength = 0;
      if (mezzCardsOn) {
        for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
          if (CSMSetupArray[TDCENABLES+i] == 1) {
            if(i==NEWTDC_NUMBER){
              HPTDCOffset[i] = dataLength;
              dataArray[dataLength] = 1;
              maskArray[dataLength++] = 0;
            }
            else{
              //Modified by Xiangting
              HPTDCOffset[i] = dataLength;
              dataArray[dataLength] = 1;
              maskArray[dataLength++] = 0;
              WriteJTAGDataToActionFile(1, HPTDCOffset[i]);
              A3P250Offset[i] = dataLength;
              dataArray[dataLength] = 1;
              maskArray[dataLength++] = 0;
              WriteJTAGDataToActionFile(1, A3P250Offset[i]);
      
//            mezzOffset[i] = dataLength;
//            dataArray[dataLength] = 1;
//            maskArray[dataLength++] = 0;
//            WriteJTAGDataToActionFile(1, mezzOffset[i]);
            }            
          }
        }
      }
      if (CSMOn) {
        CSMOffset = dataLength;
        for (i = dataLength; i < dataLength+CSMSTATUSLENGTH; i++) {
          dataArray[i] = CSMStatusArray[i-dataLength];
          if (i == dataLength) {
            dataArray[i] = 1;
            maskArray[i] = 0;
          }
          else if (i < dataLength+12) {
            dataArray[i] = 0;
            maskArray[i] = 0;
          }
          else maskArray[i] = 0;
        }
        dataLength += CSMSTATUSLENGTH;
        for (i = dataLength; i < dataLength+CSMSETUPLENGTH; i++) {
          dataArray[i] = CSMSetupArray[i-dataLength];
          maskArray[i] = 0;
          downloadedCSMSetupArray[i-dataLength] = dataArray[i];
        }
        dataLength += CSMSETUPLENGTH;
        for (i = dataLength; i < dataLength+TTCRXSETUPLENGTH; i++) {
          dataArray[i] = TTCrxSetupArray[i-dataLength];
          maskArray[i] = 0;
        }
        dataLength += TTCRXSETUPLENGTH;
        WriteJTAGDataToActionFile(TTCRXSETUPLENGTH+CSMSETUPLENGTH+CSMSTATUSLENGTH, CSMOffset);
      }
      if (oldTTCrxOn) {
        TTCrxOffset = dataLength;
        dataArray[dataLength] = 1;
        maskArray[dataLength++] = 0;
        WriteJTAGDataToActionFile(1, TTCrxOffset);
      }
      if (oldGOLOn) {
        GOLOffset = dataLength;
        dataArray[dataLength] = 1;
        maskArray[dataLength++] = 0;
        WriteJTAGDataToActionFile(1, GOLOffset);
      }
      if (AX1000On) {
        AX1000Offset = dataLength;
        dataArray[dataLength] = 1;
        maskArray[dataLength++] = 0;
        WriteJTAGDataToActionFile(1, AX1000Offset);
      }
      if (XC2V1000On || XC2V2000On) {
        vertexIIOffset = dataLength;
        dataArray[dataLength] = 1;
        maskArray[dataLength++] = 0;
        WriteJTAGDataToActionFile(1, vertexIIOffset);
      }
      if (PROMOn) {
        PROMOffset = dataLength;
        dataArray[dataLength] = 1;
        maskArray[dataLength++] = 0;
        WriteJTAGDataToActionFile(1, PROMOffset);
      }
      WriteJTAGData(SVFFile);
      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        if (CSMOn) {
          downloadCSMSetupDone = TRUE;
          CSMLastNextState = CSMNextState;
        }
        UpdateJTAGControlPanel();     // Update JTAG control panel
        printf("CSM Setup has been downloaded to turn on/off GOL/TTCrx from JTAG Chain.\n");
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Turn On/Off GOL/TTCrx From JTAG Chain", 
                 "Turn On/Off GOL/TTCrx From JTAG Chain by Downloading Current CSM Setup through JTAG.");
    controlOption = NORM;
  }
}


void FillReadbackArray(char *str) {
  int i, strl, val, addr;
  
  for (i = 0; i <= strlen(str); i++) str[i] = tolower(str[i]);
  if (strstr(str, "tdo captured =") != NULL) {
    addr = 0;
    strl = strlen(str);
    for (i = strl-1; i >= 0; i--) {
      if (str[i] == ' ' || str[i] == '\t' || str[i] == 'x') break;
      else if (str[i] != '\0' && str[i] != '\n') {
        sscanf(&str[i], "%1x", &val);
        IntToBinary(val, addr, 4, readbackArray, MAXJTAGARRAY);
        addr += 4;
      }
    }
  }
  else if (strstr(str, "success") != NULL) {
    for (i = 0; i < dataLength; i++) readbackArray[i] = dataArray[i];
  }
}


int CheckFullJTAGDataArray(void) {
  int mezz, offset1, offset2, nDevice, IDCode, mezzNumber, nError = 0;

  nDevice = 0;
  mezz = MAXNUMBERMEZZANINE;
  if (mezzCardsOn) {
    for (mezz = MAXNUMBERMEZZANINE-1; mezz >= 0; mezz--) {
      if (CSMSetupArray[TDCENABLES+mezz] == 1) {
		  
		nDevice++;
		if(mezz == NEWTDC_NUMBER){
			if (nDevice == 1) {
	          offset1 = HPTDCOffset[mezz];
	          IDCode = TDC_ID;
	          mezzNumber = mezz;
			}
			else if (nDevice == 2) {
			  offset2 = HPTDCOffset[mezz];
	          nError += CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1);
	          nDevice = 1;
	          offset1 = offset2;
	          IDCode = TDC_ID;
	          mezzNumber = mezz; 
	        } 
		}
		else{ 
			if (nDevice == 1) {
				offset1 = HPTDCOffset[mezz];
				offset2 = A3P250Offset[mezz];
				IDCode = HPTDCID;
				mezzNumber = mezz;  
	 			nError +=CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1); 
				mezzNumber = mezz;  
				IDCode = A3P250ID;
				offset1 = offset2;
				nDevice = 1;
			}
			else if (nDevice == 2){
				offset2 = HPTDCOffset[mezz]; 
				nError +=CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1);
				nDevice = 1;
		        offset1 = offset2;
		        IDCode = HPTDCID;
				offset2 = A3P250Offset[mezz];
				mezzNumber = mezz;  
	 			nError +=CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1);
				mezzNumber = mezz;  
				IDCode = A3P250ID;
				offset1 = offset2;
				nDevice = 1;
			}
		}
	  }
	}
  }
	

//Modified by Xiangting


		
	/*	
        if (nDevice == 1) {
          offset1 = mezzOffset[mezz];
          IDCode = AMT3ID;
          mezzNumber = mezz;
        }
        else if (nDevice == 2) {
		  offset1 = HPTDCOffset[mezz];
          offset2 = A3P250Offset[mezz];
          nError += CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1);
          nDevice = 1;
          offset1 = offset2;
          IDCode = AMT3ID;
          mezzNumber = mezz;
        }
	*/

  if (CSMOn) {
    nDevice++;
    if (nDevice == 1) {
      offset1 = CSMOffset;
      mezzNumber = mezz;
	  //printf("nDevice = %d, offset1 =%d for CSM\n",nDevice,offset1);
    }
    else if (nDevice == 2) {
      offset2 = CSMOffset;
	  //printf("nDevice = %d, offset2 =%d for CSM\n",nDevice,offset2);
	  printf("Check A3P250 when CSM is on\n");   	  
      nError += CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1);
	  
	  printf("Check A3P250 when CSM is on\n");
	  
      nDevice = 1;
      offset1 = offset2;
      mezzNumber = mezz;
    }
    if (CSMType == CSM) IDCode = CSMID;
    else if (CSMType == CSM1) IDCode = CSM1ID;
    else if (CSMType == CSM2) IDCode = CSM2ID;
    else if (CSMType == CSM3) IDCode = CSM3ID;
    else IDCode = CSMID;
  }
  if (TTCrxOn) {
    nDevice++;
    if (nDevice == 1) {
      offset1 = TTCrxOffset;
	  //printf("nDevice = %d, offset1 =%d for TTCrx\n",nDevice,offset1);
      IDCode = TTCRXID;
      mezzNumber = mezz;
    }
    else if (nDevice == 2) {
      offset2 = TTCrxOffset;
	  //printf("nDevice = %d, offset2 =%d for TTCrx\n",nDevice,offset2); 
      nError += CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1);
      nDevice = 1;
      offset1 = offset2;
      IDCode = TTCRXID;
      mezzNumber = mezz;
    }
  }
  if (GOLOn) {
    nDevice++;
    if (nDevice == 1) {
      offset1 = GOLOffset;
	  //printf("nDevice = %d, offset1 =%d for GOL\n",nDevice,offset1); 
      IDCode = GOLID;
      mezzNumber = mezz;
    }
    else if (nDevice == 2) {
      offset2 = GOLOffset;
	  //printf("nDevice = %d, offset2 =%d for GOL\n",nDevice,offset2);
      nError += CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1);
      nDevice = 1;
      offset1 = offset2;
      IDCode = GOLID;
      mezzNumber = mezz;
    }
  }
  if (AX1000On) {
    nDevice++;
    if (nDevice == 1) {
      offset1 = AX1000Offset;
      IDCode = AX1000ID;
      mezzNumber = mezz;
    }
    else if (nDevice == 2) {
      offset2 = AX1000Offset;
      nError += CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1);
      nDevice = 1;
      offset1 = offset2;
      IDCode = AX1000ID;
      mezzNumber = mezz;
    }
  }
  if (XC2V1000On || XC2V2000On) {
    nDevice++;
    if (nDevice == 1) {
      offset1 = vertexIIOffset;
	  //printf("nDevice = %d, offset1 =%d for vertex\n",nDevice,offset1);
      IDCode = XC2V1000ID;
      if (XC2V2000On) IDCode = XC2V2000ID;
      mezzNumber = mezz;
    }
    else if (nDevice == 2) {
      offset2 = vertexIIOffset;
	  //printf("nDevice = %d, offset2 =%d for vertex\n",nDevice,offset2);
      nError += CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1);
      nDevice = 1;
      offset1 = offset2;
      IDCode = XC2V1000ID;
      if (XC2V2000On) IDCode = XC2V2000ID;
      mezzNumber = mezz;
    }
  }
  if (PROMOn) {
    nDevice++;
    if (nDevice == 1) {
      offset1 = PROMOffset;
	  //printf("nDevice = %d, offset1 =%d for PROM\n",nDevice,offset1);
      IDCode = PROMID;
      mezzNumber = mezz;
    }
    else if (nDevice == 2) {
      offset2 = PROMOffset;
	  //printf("nDevice = %d, offset2 =%d for PROM\n",nDevice,offset2); 
      nError += CheckDataArray(IDCode, mezzNumber, offset1, offset2-offset1);
      nDevice = 1;
      offset1 = offset2;
      IDCode = PROMID;
      mezzNumber = mezz;
    }
  }
  if (nDevice == 1) {
    printf("Check A3P250 when there are just one JTAG device\n");
	//printf("IDCode=%08x, mezzNumber=%d, offset1= %d, length=%d\n", IDCode,mezzNumber,offset1,dataLength-offset1);
    nError += CheckDataArray(IDCode, mezzNumber, offset1, dataLength-offset1);
    printf("Check A3P250 when there are just one JTAG device\n"); 
  }
  return nError;
}


int CheckDataArray(int IDCode, int mezzNumber, int from, int length) {
  static int i, j, k, nOne, nError, printError, oldError = 0, isCSM;
  
  		  
  isCSM  = (IDCode == CSMID);
  isCSM |= (IDCode == CSM1ID);
  isCSM |= (IDCode == CSM2ID);
  isCSM |= (IDCode == CSM3ID);
  nOne = 0;
  nError = 0;
  printError = FALSE;
  //printf("IDCode=%08x, mezzNumber=%d, from= %d, length=%d\n", IDCode,mezzNumber,from,length);
  for (i = from; i < from+length; i++) if (readbackArray[i] == 0) printError = TRUE;
  for (i = from; i < from+length; i++) { 
    if (maskArray[i] == 1) {
      nOne++;
      if (dataArray[i] != readbackArray[i]) {
        if (printError && (length < 512))
          printf("IDCode = 0x%08x, from = %d, bit = %d, dataArray = %d, readbackArray %d\n", IDCode, from, i, dataArray[i], readbackArray[i]);
        nError++;
      }
    }
  }
  if (!printError && (nError > 0)) {
    printf("IDCode = 0x%08x, from = %d, length = %d, all 1 in readback array!\n", IDCode, from, length);
  }
  else if (printError) {
    if (isCSM && (oldError != 0)) {
      for (i = from; i < from+length;) {
        k = i + 40;
        if (k > (from+length)) k = from + length;
        printf("i = %4d ", i);
        for (j = i; j < k; j++) printf("%d", dataArray[j]);
        printf("\n");
        printf("         ");
        for (j = i; j < k; j++) printf("%d", readbackArray[j]);
        printf("\n");
        i = j;
      }
    }
    if (isCSM) oldError = nError;
    if (isCSM && (nError != 0)) {
      printf("Number JTAG Bit Error = %d\n", nError);
      for (i = from; i < from+length;) {
        k = i + 40;
        if (k > (from+length)) k = from + length;
        printf("i = %4d ", i);
        for (j = i; j < k; j++) printf("%d", dataArray[j]);
        printf("\n");
        printf("         ");
        for (j = i; j < k; j++) printf("%d", readbackArray[j]);
        printf("\n");
        i = j;
      }
    }
  }
  
  
  if (nOne > 0) {
	  
	if (IDCode == HPTDCID||IDCode == TDC_ID){

		downloadHPTDCStatus[mezzNumber] = nError;	
	}
	
	else if (IDCode == A3P250ID){
	    downloadA3P250Status[mezzNumber] = nError;  		
		
	}
	
	/*  
    if ((IDCode == AMT3ID) || (IDCode == AMT2ID)) {
      if (length == 180) downloadAMTStatus[mezzNumber] = nError;
      else if (length == 160) downloadASDStatus[mezzNumber] = nError;
      else {
        downloadAMTStatus[mezzNumber] = nError;
        downloadASDStatus[mezzNumber] = nError;
      }
    }
    else if (IDCode == AMT1ID) {
      downloadAMTStatus[mezzNumber] = nError;
      downloadASDStatus[mezzNumber] = nError;
    }
	
	*/	
    else if (IDCode == TTCRXID) downloadTTCrxStatus = nError;
    else if (IDCode == GOLID) downloadGOLStatus = nError;
    else if (isCSM) {
      downloadCSMStatus = nError;
      downloadTTCrxStatus = nError;
    }
  }
  
  return nError;
}


void CheckCSMSetupAndStatus(void) {
  char path[256];
  int i, execCSMCMD;
  FILE *SVFFile;

  if (controlOption == NORM) {
    validCSMStatus = FALSE;
    strcpy(path, "checkCSMSetupAndStatus.svf");
    if (SVFFile = fopen(path, "w")) {
      GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &execCSMCMD);
      if ((action == DOWNLOAD) && !downloadCSMSetupDone) {
        SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 0);
        LoadCSMSetupArray();
        LoadTTCrxSetupArray();
      }

      GetJTAGChainOnList();
      if (CSMOn) {
        if ((action == DOWNLOAD) && !downloadCSMSetupDone)
          JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMFULLRW, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
        else
          JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMFULLRO, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
      }

      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMFULLRW, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        if (CSMOn && !downloadCSMSetupDone) {
          JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
          JTAGScanAllData(dataLength, dataArray, readbackArray);
          downloadCSMSetupDone = TRUE;
          CSMLastNextState = CSMNextState;
        }
        CheckFullJTAGDataArray();
		
		printf("CheckCSMSetupAndStatus is ongoing\n");
		printf("CheckCSMSetupAndStatus is ongoing\n");
		
        validCSMStatus = TRUE;
        for (i = 0; i < CSMSTATUSLENGTH; i++) CSMStatusArray[i] = readbackArray[CSMOffset+i];
        UpdateJTAGControlPanel();     // Update JTAG control panel
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
      SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, execCSMCMD);
      LoadCSMSetupArray();
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Check CSM Setup And Status", 
                 "Check Current CSM Setup and Status through JTAG.");
    controlOption = NORM;
  }
}


void CheckCSMAndTTCrxSetup(void) {
  CheckCSMSetupAndStatus();
}


void CheckGOLSetup(void) {
  int i;
  
  if (controlOption == NORM) {
    LoadCSMSetupArray();
    LoadTTCrxSetupArray();
    LoadGOLSetupArray();

    JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLCONFRO, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);    

    JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLCONFRO, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);   
    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      CheckFullJTAGDataArray();
	  
	  printf("CheckGOLSetup is ongoing\n");
	  printf("CheckGOLSetup is ongoing\n");
	  
      UpdateJTAGControlPanel();     // Update JTAG control panel
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Check GOL Setup", 
                 "Check Current GOL Setup through JTAG.");
    controlOption = NORM;
  }
}

/*
void CheckAMTSetup(void) {
  int version, i, instr, length, l;
  
  if (controlOption == NORM) {
    version = -1;
    if (CSMOn) BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
    if ((version < 0) || (version > 10)) {
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, 1);
      SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
      UpdateCSMControlBits();
      GetJTAGChainOnList();
    }
    DownloadCSMSetup();
	mezzCardSetupAll = TRUE;
    instrLength = 0;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) instr = AMTCONTROL;
          else if(mezzCardNb == i) instr = AMTCONTROL;
          else instr = AMTBYPASS;
          IntToBinary(instr, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += AMTINSTLENGTH;
          WriteJTAGInstructionToActionFile(AMT3ID, instr, i);
        }
      }
    }
    if (CSMOn) {
      IntToBinary(CSMBYPASS, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += CSMINSTLENGTH;
      WriteJTAGInstructionToActionFile(CSMID, CSMBYPASS, 0);
    }
    if (TTCrxOn) {
      IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += TTCRXINSTLENGTH;
      WriteJTAGInstructionToActionFile(TTCRXID, TTCRXBYPASS, 0);
    }
    if (GOLOn) {
      IntToBinary(GOLBYPASS, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += GOLINSTLENGTH;
      WriteJTAGInstructionToActionFile(GOLID, GOLBYPASS, 0);
    }
    if (AX1000On) {
      IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += AX1000INSTLENGTH;
      WriteJTAGInstructionToActionFile(AX1000ID, AX1000BYPASS, 0);
    }
    if (XC2V1000On || XC2V2000On) {
      IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += VERTEXIIINSTLENGTH;
      if (XC2V2000On) WriteJTAGInstructionToActionFile(XC2V2000ID, VERTEXIIBYPASS, 0);
      else WriteJTAGInstructionToActionFile(XC2V1000ID, VERTEXIIBYPASS, 0);
    }
    if (PROMOn) {
      IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += PROMINSTLENGTH;
      WriteJTAGInstructionToActionFile(PROMID, PROMBYPASS, 0);
    }
    dataLength = 0;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          for (l = 0; l < AMTS_SETUP+AMTSETUP_LENGTH; l++) {
            statusSetupCheck[l] = basicSetupArray[l];
            basicSetupArray[l] = mezzSetupArray[l][i];
          }
          IntToBinary(i, AMTS_SETUP+AMTS_TDC_ID, 4, basicSetupArray, SETUP_LENGTH); 
          if (mezzCardSetupAll) length = AMTSETUP_LENGTH;
          else if(mezzCardNb == i) length = AMTSETUP_LENGTH;
          else length = 1;
          if (length == AMTSETUP_LENGTH) {
            for (l = 0; l < AMTS_SETUP+AMTSETUP_LENGTH; l++) mezzSetupArray[l][i] = basicSetupArray[l];
            if (sampleAMTPhase) {
              basicSetupArray[AMTS_SETUP+AMTS_ENB_SER] = 1;
              IntToBinary(AMTEDGECONTINE, AMTS_SETUP+AMTS_STROBE_SEL, 2, basicSetupArray, SETUP_LENGTH);
            }
          }
          mezzOffset[i] = dataLength;
          for (l = 0; l < length; l++) {
            if (length == AMTSETUP_LENGTH) {
              dataArray[dataLength] = basicSetupArray[AMTS_SETUP+l];
              maskArray[dataLength++] = 1;
            }
            else {
              dataArray[dataLength] = 1;
              maskArray[dataLength++] = 0;
            }
          }
          for (l = 0; l < AMTS_SETUP+AMTSETUP_LENGTH; l++) {
            basicSetupArray[l] = statusSetupCheck[l];
          }
          WriteJTAGDataToActionFile(length, mezzOffset[i]);
        }
      }
    }
    if (CSMOn) {
      CSMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, CSMOffset);
    }
    if (TTCrxOn) {
      TTCrxOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, TTCrxOffset);
    }
    if (GOLOn) {
      GOLOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, GOLOffset);
    }
    if (AX1000On) {
      AX1000Offset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, AX1000Offset);
    }
    if (XC2V1000On || XC2V2000On) {
      vertexIIOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, vertexIIOffset);
    }
    if (PROMOn) {
      PROMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, PROMOffset);
    }
    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      CheckFullJTAGDataArray();
	  
	  printf("CheckAMTSetup is ongoing\n");
	  printf("CheckAMTSetup is ongoing\n");
	  
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) {
            if (downloadAMTStatus[i] == 0)
              printf("AMT setup is checked OK through JTAG for mezzanine card %d.\n", i);
            else  
              printf("Check AMT setup failed through JTAG for mezzanine card %d, downloadAMTStatus = %d\n", i, downloadAMTStatus[i]);
          }
          else if(mezzCardNb == i) {
            if (downloadAMTStatus[i] == 0)
              printf("AMT setup is checked OK through JTAG for mezzanine card %d.\n", i);
            else  
              printf("Check AMT setup failed through JTAG for mezzanine card %d, downloadAMTStatus = %d\n", i, downloadAMTStatus[i]);
          }
        }
      }
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Check AMT Setup", 
                 "Check Current AMT Setup through JTAG.");
    controlOption = NORM;
  }
}


void CheckASDSetup(void) {
  int version, i, instr, length, l, arrayCode[ASD_SETUP_LENGTH*3+1];
  
  if (controlOption == NORM) {
    version = -1;
    if (CSMOn) BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
    if ((version < 0) || (version > 10)) {
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, 1);
      SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
      UpdateCSMControlBits();
      GetJTAGChainOnList();
    }
    DownloadCSMSetup();
	mezzCardSetupAll = TRUE;
    instrLength = 0;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) instr = AMTASDCONTROL;
          else if(mezzCardNb == i) instr = AMTASDCONTROL;
          else instr = AMTBYPASS;
          IntToBinary(instr, instrLength, AMTINSTLENGTH, instrArray, JTAGINSTRLENGTH);
          instrLength += AMTINSTLENGTH;
          WriteJTAGInstructionToActionFile(AMT3ID, instr, i);
        }
      }
    }
    if (CSMOn) {
      IntToBinary(CSMBYPASS, instrLength, CSMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += CSMINSTLENGTH;
      WriteJTAGInstructionToActionFile(CSMID, CSMBYPASS, 0);
    }
    if (TTCrxOn) {
      IntToBinary(TTCRXBYPASS, instrLength, TTCRXINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += TTCRXINSTLENGTH;
      WriteJTAGInstructionToActionFile(TTCRXID, TTCRXBYPASS, 0);
    }
    if (GOLOn) {
      IntToBinary(GOLBYPASS, instrLength, GOLINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += GOLINSTLENGTH;
      WriteJTAGInstructionToActionFile(GOLID, GOLBYPASS, 0);
    }
    if (AX1000On) {
      IntToBinary(AX1000BYPASS, instrLength, AX1000INSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += AX1000INSTLENGTH;
      WriteJTAGInstructionToActionFile(AX1000ID, AX1000BYPASS, 0);
    }
    if (XC2V1000On || XC2V2000On) {
      IntToBinary(VERTEXIIBYPASS, instrLength, VERTEXIIINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += VERTEXIIINSTLENGTH;
      if (XC2V2000On) WriteJTAGInstructionToActionFile(XC2V2000ID, VERTEXIIBYPASS, 0);
      else WriteJTAGInstructionToActionFile(XC2V1000ID, VERTEXIIBYPASS, 0);
    }
    if (PROMOn) {
      IntToBinary(PROMBYPASS, instrLength, PROMINSTLENGTH, instrArray, JTAGINSTRLENGTH);
      instrLength += PROMINSTLENGTH;
      WriteJTAGInstructionToActionFile(PROMID, PROMBYPASS, 0);
    }
    dataLength = 0;
    if (mezzCardsOn) {
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          for (l = 0; l < AMTS_SETUP+AMTSETUP_LENGTH; l++) {
            statusSetupCheck[l] = basicSetupArray[l];
            basicSetupArray[l] = mezzSetupArray[l][i];
          }
          arrayCode[0] = 0;
          for (l = 1; l < ASD_SETUP_LENGTH*3+1; l++) arrayCode[l] = basicSetupArray[l-1];
          ReverseArray(arrayCode, 1, ASD_SETUP_LENGTH);
          ReverseArray(arrayCode, ASD_SETUP_LENGTH+1, ASD_SETUP_LENGTH);
          ReverseArray(arrayCode, ASD_SETUP_LENGTH*2+1, ASD_SETUP_LENGTH);
          IntToBinary(i, AMTS_SETUP+AMTS_TDC_ID, 4, basicSetupArray, SETUP_LENGTH); 
          if (mezzCardSetupAll) length = ASD_SETUP_LENGTH*3+1;
          else if(mezzCardNb == i) length = ASD_SETUP_LENGTH*3+1;
          else length = 1;
          mezzOffset[i] = dataLength;
          for (l = 0; l < length; l++) {
            if (length == (ASD_SETUP_LENGTH*3+1)) {
              dataArray[dataLength] = arrayCode[l];
              maskArray[dataLength++] = 1;
            }
            else {
              dataArray[dataLength] = 1;
              maskArray[dataLength++] = 0;
            }
          }
          for (l = 0; l < AMTS_SETUP+AMTSETUP_LENGTH; l++) {
            basicSetupArray[l] = statusSetupCheck[l];
          }
          WriteJTAGDataToActionFile(length, mezzOffset[i]);
        }
      }
    }
    if (CSMOn) {
      CSMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, CSMOffset);
    }
    if (TTCrxOn) {
      TTCrxOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, TTCrxOffset);
    }
    if (GOLOn) {
      GOLOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, GOLOffset);
    }
    if (AX1000On) {
      AX1000Offset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, AX1000Offset);
    }
    if (XC2V1000On || XC2V2000On) {
      vertexIIOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, vertexIIOffset);
    }
    if (PROMOn) {
      PROMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
      WriteJTAGDataToActionFile(1, PROMOffset);
    }
    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      CheckFullJTAGDataArray();
	  
	  printf("CheckASDSetup is ongoing\n");
	  printf("CheckASDSetup is ongoing\n");
	  
	  
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if (mezzCardSetupAll) {
            if (downloadASDStatus[i] == 0)
              printf("ASD setup is checked through JTAG for mezzanine card %d.\n", i);
            else  
              printf("Check ASD setup is failed through JTAG for mezzanine card %d, downloadASDStatus = %d\n", i, downloadASDStatus[i]);
          }
          else if(mezzCardNb == i) {
            if (downloadASDStatus[i] == 0)
              printf("ASD setup is checked through JTAG for mezzanine card %d.\n", i);
            else  
              printf("Check ASD setup is failed through JTAG for mezzanine card %d, downloadASDStatus = %d\n", i, downloadASDStatus[i]);
          }
        }
      }
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Check ASD Setup", 
                 "Check Current ASD Setup through JTAG.");
    controlOption = NORM;
  }
}
*/

int FindDevice(int instIDCode, int length, unsigned int expectedID) {
  char path[256];
  int i, j, foundDevice = FALSE, PROM, vertexII;
  unsigned int IDCode = 0xFFFFFFFF;
  FILE *SVFFile;

  if ((numberExpectedDevice <= 0) || (numberExpectedDevice > 30)) numberExpectedDevice = 30;
  strcpy(path, "getIDCode.svf");
  if (SVFFile = fopen(path, "w")) {
    fprintf(SVFFile, "// SVF File generated by CSM JTAG Control on %s\n", WeekdayTimeString(time(NULL))); 
    fprintf(SVFFile, "//\n");
    fprintf(SVFFile, "TRST OFF;\n");
    fprintf(SVFFile, "ENDIR IDLE;\n");
    fprintf(SVFFile, "ENDDR IDLE;\n");
    fprintf(SVFFile, "STATE RESET IDLE;\n");
    instrLength = 0;
    for (i = numberExpectedDevice-1; i >= 0; i--) {
      IntToBinary(0xFF, instrLength, 8, instrArray, JTAGINSTRLENGTH);
      instrLength += 8;
      JTAGComments(0xFF, SVFFile, 0);
    }
    WriteJTAGInstructions(SVFFile);
    instrLength = 0;
    for (i = numberExpectedDevice-1; i > numberDevice; i--) {
      IntToBinary(0xFF, instrLength, 8, instrArray, JTAGINSTRLENGTH);
      instrLength += 8;
      JTAGComments(0xFF, SVFFile, 0);
    }
    IntToBinary(instIDCode, instrLength, length, instrArray, JTAGINSTRLENGTH);
    instrLength += length;
    JTAGComments(instIDCode, SVFFile, expectedID);
    for (i = 0; i < numberDevice; i++) {
      IntToBinary(IDCodeInstr[i], instrLength, lengthInstr[i], instrArray, JTAGINSTRLENGTH);
      instrLength += lengthInstr[i];
      JTAGComments(IDCodeInstr[i], SVFFile, deviceID[i]);
    }
    WriteJTAGInstructions(SVFFile);
    dataLength = 0;
    for (i = numberExpectedDevice-1; i > numberDevice; i--) {
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
    }
    if ((expectedID == AMT2ID) || (expectedID == AMT3ID))
      IntToBinary(0, dataLength, 32, dataArray, MAXJTAGARRAY);
    else
      IntToBinary(expectedID, dataLength, 32, dataArray, MAXJTAGARRAY);
    for (i = 0; i < 32; i++) maskArray[dataLength++] = 1;
    for (i = 0; i < numberDevice; i++) {
      IntToBinary(deviceID[i], dataLength, 32, dataArray, MAXJTAGARRAY);
      for (j = 0; j < 32; j++) maskArray[dataLength++] = 1;
    }
    WriteJTAGData(SVFFile);
    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      for (i = 0; i < numberExpectedDevice-numberDevice; i++) {
        BinaryToInt(&IDCode, i, 32, readbackArray);
        PROM = FALSE;
        if (expectedID == XC18V04ID) {
          if ((IDCode&PROMIDMASK0) == expectedID) PROM = TRUE;
        }
        else if ((expectedID == XCF01SID) || (expectedID == XCF02SID) || (expectedID == XCF04SID) ||
                 (expectedID == XCF08PID) || (expectedID == XCF16PID) || (expectedID == XCF32PID)) {
          if ((IDCode&PROMIDMASK1) == expectedID) PROM = TRUE;
        }
        vertexII = FALSE;
        if ((expectedID == XC2V1000ID) || (expectedID == XC2V2000ID)) {
          if ((IDCode&VERTEXIIIDMASK) == expectedID) vertexII = TRUE;
        }
        if ((IDCode == expectedID) || PROM || vertexII) {
          foundDevice = TRUE;
          if (numberDevice == 0) numberExpectedDevice = i+1;
          else {
            for (j = numberDevice; j > 0; j--) {
              deviceID[j] = deviceID[j-1];
              IDCodeInstr[j] = IDCodeInstr[j-1];
              lengthInstr[j] = lengthInstr[j-1];
            }
          }
          deviceID[0] = IDCode;
          IDCodeInstr[0] = instIDCode;
          lengthInstr[0] = length;
          numberDevice++;
          break;
        }
      }
    }
    instrLength = 0;
    for (i = numberExpectedDevice-1; i >= 0; i--) {
      IntToBinary(0xFF, instrLength, 8, instrArray, JTAGINSTRLENGTH);
      instrLength += 8;
      JTAGComments(0xFF, SVFFile, 0);
    }
    WriteJTAGInstructions(SVFFile);
    fprintf(SVFFile, "RUNTEST 12 TCK;\n");
    fclose(SVFFile);
  }
  return foundDevice;
}


void GetDeviceList(void) {
  int i, foundDevice;
  if (controlOption == HELP) {
    MessagePopup("Help on Get Device List",
                 "Push this button to get device list in JTAG chain.");
    controlOption = NORM;
    return;
  }
  gotPROM = FALSE;
  gotXC2V1000 = FALSE;
  gotXC2V2000 = FALSE;
  gotAX1000 = FALSE;
  gotGOL = FALSE;
  gotTTCrx = FALSE;
  gotCSM = FALSE;
  numberDevice = 0;
  numberExpectedDevice = 30;
  while (numberDevice != numberExpectedDevice) {
    foundDevice = FALSE;
    if (!gotPROM && !gotAX1000) {
      foundDevice = FindDevice(AX1000IDCODE, AX1000INSTLENGTH, AX1000ID);
      if (foundDevice) gotAX1000 = TRUE;
      else {
        PROMINSTLENGTH = 8;
        PROMBYPASS = 0xFF;
        foundDevice = FindDevice(PROMIDCODE, PROMINSTLENGTH, XC18V04ID);
      }
      if (!foundDevice) {
        PROMINSTLENGTH = 16;
        PROMBYPASS = 0xFFFF;
        foundDevice = FindDevice(PROMIDCODE, PROMINSTLENGTH, XCF08PID);
      }
      if (!foundDevice) {
        PROMINSTLENGTH = 16;
        PROMBYPASS = 0xFFFF;
        foundDevice = FindDevice(PROMIDCODE, PROMINSTLENGTH, XCF16PID);
      }
      if (!foundDevice) {
        PROMINSTLENGTH = 16;
        PROMBYPASS = 0xFFFF;
        foundDevice = FindDevice(PROMIDCODE, PROMINSTLENGTH, XCF32PID);
      }
      if (!foundDevice) {
        PROMINSTLENGTH = 8;
        PROMBYPASS = 0xFF;
        foundDevice = FindDevice(PROMIDCODE, PROMINSTLENGTH, XCF01SID);
      }
      if (!foundDevice) {
        PROMINSTLENGTH = 8;
        PROMBYPASS = 0xFF;
        foundDevice = FindDevice(PROMIDCODE, PROMINSTLENGTH, XCF02SID);
      }
      if (!foundDevice) {
        PROMINSTLENGTH = 8;
        PROMBYPASS = 0xFF;
        foundDevice = FindDevice(PROMIDCODE, PROMINSTLENGTH, XCF04SID);
      }
      if (foundDevice) gotPROM = TRUE;
    }
    else if ((!gotXC2V1000) && (!gotXC2V2000)) {
      foundDevice = FindDevice(VERTEXIIIDCODE, VERTEXIIINSTLENGTH, XC2V2000ID);
      if (foundDevice) gotXC2V2000 = TRUE;
      else {
        foundDevice = FindDevice(VERTEXIIIDCODE, VERTEXIIINSTLENGTH, XC2V1000ID);
        if (foundDevice) gotXC2V1000 = TRUE;
      }
    }
    if (!foundDevice && !gotGOL) {
      foundDevice = FindDevice(GOLDEVICEID, GOLINSTLENGTH, GOLID);
      if (foundDevice) gotGOL = TRUE;
    }
    if (!foundDevice && !gotTTCrx) {
      foundDevice = FindDevice(TTCRXIDCODE, TTCRXINSTLENGTH, TTCRXID);
      if (foundDevice) gotTTCrx = TRUE;
    }
    if (!foundDevice && !gotCSM) {
      foundDevice = FindDevice(CSMIDCODE, CSMINSTLENGTH, CSM1ID);
      if (!foundDevice) foundDevice = FindDevice(CSMIDCODE, CSMINSTLENGTH, CSM2ID);
      if (!foundDevice) foundDevice = FindDevice(CSMIDCODE, CSMINSTLENGTH, CSM3ID);
      if (foundDevice) gotCSM = TRUE;
    }
    if (!foundDevice) foundDevice = FindDevice(HPTDCIDCODE, HPTDCINSTLENGTH, AMT2ID);
    if (!foundDevice) foundDevice = FindDevice(A3P250IDCODE, A3P250INSTLENGTH, AMT3ID);
    if (!foundDevice) break;
    printf("Number found JTAG device = %d\n", numberDevice);
  }
  if (numberDevice == 0) printf("No valid JTAG device was found in JTAG chain.\n");
  else {
    printf("Number JTAG devices: Expected = %d  Found = %d\n", numberExpectedDevice, numberDevice);
    for (i = 0; i < numberDevice; i++) {
      printf("JTAG device %2d with IDCode = 0x%08X ", i+1, deviceID[i]);
      if ((deviceID[i]&PROMIDMASK0) == XC18V04ID) printf("(PROM XC18V04)\n");
      else if ((deviceID[i]&PROMIDMASK1) == XCF08PID) printf("(PROM XCF08P)\n");
      else if ((deviceID[i]&PROMIDMASK1) == XCF16PID) printf("(PROM XCF16P)\n");
      else if ((deviceID[i]&PROMIDMASK1) == XCF32PID) printf("(PROM XCF32P)\n");
      else if ((deviceID[i]&PROMIDMASK1) == XCF01SID) printf("(PROM XCF01S)\n");
      else if ((deviceID[i]&PROMIDMASK1) == XCF02SID) printf("(PROM XCF02S)\n");
      else if ((deviceID[i]&PROMIDMASK1) == XCF04SID) printf("(PROM XCF04S)\n");
      else if ((deviceID[i]&VERTEXIIIDMASK) == XC2V2000ID) printf("(XC2V2000)\n");
      else if ((deviceID[i]&VERTEXIIIDMASK) == XC2V1000ID) printf("(XC2V1000)\n");
      else if (deviceID[i] == AX1000ID) printf("(AX1000)\n");
      else if (deviceID[i] == GOLID) printf("(GOL)\n");
      else if (deviceID[i] == TTCRXID) printf("(TTCrx)\n");
      else if (deviceID[i] == CSM1ID) printf("(CSM1)\n");
      else if (deviceID[i] == CSM2ID) printf("(CSM2)\n");
      else if (deviceID[i] == CSM3ID) printf("(CSM3)\n");
      else if (deviceID[i] == CSMID) printf("(CSM)\n");
      else if (deviceID[i] == A3P250ID) printf("(A3P250)\n");
      else if (deviceID[i] == HPTDCID) printf("(HPTDC)\n");
      else printf("(Unknown)\n");
    }
  }
}


void SetupJTAGChain(void) {
  char path[256];
  int i, j, k, dimmed, color, foundBarDevice, numberBarDevice, numberUnknown;
  unsigned int IDCode = 0xFFFFFFFF, IDCodeBar, PROMIDCodeBar0, PROMIDCodeBar1, vertexIIIDCodeBar;
  FILE *SVFFile;

  CSMType = UNKNOWN;
  downloadCSMSetupDone = FALSE;
  gotPROM = FALSE;
  gotCSMChip = FALSE;
  gotXC2V1000 = FALSE;
  gotXC2V2000 = FALSE;
  gotAX1000 = FALSE;
  gotGOL = FALSE;
  gotTTCrx = FALSE;
  gotCSM = FALSE;
  gotAMT = FALSE;
  gotUnknown = FALSE;
  numberExpectedDevice = 30;
  strcpy(path, "getIDCode.svf");
  if (SVFFile = fopen(path, "w")) {
    fprintf(SVFFile, "// SVF File generated by CSM JTAG Control on %s\n", WeekdayTimeString(time(NULL))); 
    fprintf(SVFFile, "//\n");
    fprintf(SVFFile, "TRST OFF;\n");
    fprintf(SVFFile, "ENDIR IDLE;\n");
    fprintf(SVFFile, "ENDDR IDLE;\n");
    fprintf(SVFFile, "STATE RESET IDLE;\n");
    instrLength = 0;
    dataLength = 0;
    for (i = numberExpectedDevice-1; i >= 0; i--) {
      IntToBinary(0, dataLength, 32, dataArray, MAXJTAGARRAY);
      IntToBinary(0, dataLength, 32, maskArray, MAXJTAGARRAY);
      dataLength += 32;
    }
    WriteJTAGData(SVFFile);
    if (action == DOWNLOAD) {
      ResetTAP();
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      ResetTAP();
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, FALSE);
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, FALSE);
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, FALSE);
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, FALSE);
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, FALSE);
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, FALSE);
      for (j = 0; j < 4; j++) {
        numberDevice = 0;
        numberBarDevice = 0;
        numberUnknown = 0;
        if (j == 1) {
          printf("Shift up JTAG output bits (bit 0 is forced to be 1), dataLength = %d\n", dataLength); 
          readbackArray[0] = 1;
          for (k = dataLength-1; k >= 0; k--) readbackArray[k+1] = readbackArray[k];
        }
        else if (j == 2) {
          printf("Shift up JTAG output bits (bit 0 is forced to be 0), dataLength = %d\n", dataLength); 
          readbackArray[0] = 0;
        }
        else if (j == 3) {
          printf("Shift down JTAG output bits, dataLength = %d\n", dataLength); 
          for (k = 2; k < dataLength+1; k++) readbackArray[k-2] = readbackArray[k];
        }
        for (i = 0; i < numberExpectedDevice; i++) {
          BinaryToInt(&IDCode, 32*i, 32, readbackArray);
          if ((IDCode&PROMIDMASK0) == XC18V04ID) {
            PROMID = XC18V04ID;
            PROMINSTLENGTH = 8;
            PROMBYPASS = 0xFF;
            gotPROM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, TRUE);
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (PROM XC18V04)\n", numberDevice, IDCode);
          }
          else if ((IDCode&PROMIDMASK1) == XCF08PID) {
            CSMType = CSM;
            PROMID = XCF08PID;
            PROMINSTLENGTH = 16;
            PROMBYPASS = 0xFFFF;
            gotPROM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, TRUE);
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (PROM XCF08P)\n", numberDevice, IDCode);
          }
          else if ((IDCode&PROMIDMASK1) == XCF16PID) {
            PROMID = XCF16PID;
            PROMINSTLENGTH = 16;
            PROMBYPASS = 0xFFFF;
            gotPROM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, TRUE);
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (PROM XCF16P)\n", numberDevice, IDCode);
          }
          else if ((IDCode&PROMIDMASK1) == XCF32PID) {
            PROMID = XCF32PID;
            PROMINSTLENGTH = 16;
            PROMBYPASS = 0xFFFF;
            gotPROM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, TRUE);
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (PROM XCF32P)\n", numberDevice, IDCode);
          }
          else if ((IDCode&PROMIDMASK1) == XCF01SID) {
            PROMID = XCF01SID;
            PROMINSTLENGTH = 8;
            PROMBYPASS = 0xFF;
            gotPROM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, TRUE);
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (PROM XCF01S)\n", numberDevice, IDCode);
          }
          else if ((IDCode&PROMIDMASK1) == XCF02SID) {
            PROMID = XCF02SID;
            PROMINSTLENGTH = 8;
            PROMBYPASS = 0xFF;
            gotPROM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, TRUE);
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (PROM XCF02S)\n", numberDevice, IDCode);
          }
          else if ((IDCode&PROMIDMASK1) == XCF04SID) {
            PROMID = XCF04SID;
            PROMINSTLENGTH = 8;
            PROMBYPASS = 0xFF;
            gotPROM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, TRUE);
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (PROM XCF04S)\n", numberDevice, IDCode);
          }
          else if ((IDCode&VERTEXIIIDMASK) == XC2V1000ID) {
            CSMChipID = XC2V1000ID;
            SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, ATTR_LABEL_TEXT, " XC2V1000 ");
            SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, ATTR_ITEM_NAME, "Get CSM Chip (XC2V1000) ID");
            gotCSMChip = TRUE;
            gotXC2V1000 = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, TRUE);
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (XC2V1000)\n", numberDevice, IDCode);
          }
          else if ((IDCode&VERTEXIIIDMASK) == XC2V2000ID) {
            CSMType = CSM;
            CSMChipID = XC2V2000ID;
            SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, ATTR_LABEL_TEXT, " XC2V2000 ");
            SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, ATTR_ITEM_NAME, "Get CSM Chip (XC2V2000) ID");
            gotCSMChip = TRUE;
            gotXC2V2000 = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, TRUE);
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (XC2V2000)\n", numberDevice, IDCode);
          }
          else if (IDCode == AX1000ID) {
            CSMType = CSM3;
            CSMChipID = AX1000ID;
            SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, ATTR_LABEL_TEXT, "  AX1000  ");
            SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, ATTR_ITEM_NAME, "Get CSM Chip (AX1000) ID");
            gotCSMChip = TRUE;
            gotAX1000 = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, TRUE);
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (AX1000)\n", numberDevice, IDCode);
          }
          else if (IDCode == GOLID) {
            gotGOL = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, TRUE);
            SetCtrlVal(CSMSetupHandle, P_CSM_GOLTDI, TRUE);
            PanelSave(CSMSetupHandle);
            LoadCSMSetupArray();
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (GOL)\n", numberDevice, IDCode);
          }
          else if (IDCode == TTCRXID) {
            gotTTCrx = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, TRUE);
            SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXTDI, TRUE);
            PanelSave(CSMSetupHandle);
            LoadCSMSetupArray();
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (TTCrx)\n", numberDevice, IDCode);
          }
          else if (IDCode == CSM1ID) {
            gotCSM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, TRUE);
            numberDevice++;
            CSMType = CSM1;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (CSM1)\n", numberDevice, IDCode);
          }
          else if (IDCode == CSM2ID) {
            gotCSM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, TRUE);
            numberDevice++;
            CSMType = CSM2;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (CSM2)\n", numberDevice, IDCode);
          }
          else if (IDCode == CSM3ID) {
            gotCSM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, TRUE);
            numberDevice++;
            CSMType = CSM3;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (CSM3)\n", numberDevice, IDCode);
          }
          else if (IDCode == CSMID) {
            gotCSM = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, TRUE);
            numberDevice++;
            CSMType = CSM;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (CSM)\n", numberDevice, IDCode);
          }
		/*  
          else if (IDCode == AMT3ID) {
            gotAMT = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, TRUE);
            SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, TRUE);
            PanelSave(CSMSetupHandle);
            LoadCSMSetupArray();
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (AMT3)\n", numberDevice, IDCode);
          }
          else if (IDCode == AMT2ID) {
            gotAMT = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, TRUE);
            SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, TRUE);
            PanelSave(CSMSetupHandle);
            LoadCSMSetupArray();
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (AMT2)\n", numberDevice, IDCode);
          }
          else if (IDCode == AMT1ID) {
            gotAMT = TRUE;
            SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, TRUE);
            SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, TRUE);
            PanelSave(CSMSetupHandle);
            LoadCSMSetupArray();
            numberDevice++;
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (AMT1)\n", numberDevice, IDCode);
		  }
		*/
//Modified by Xiangting

			
			
		  else if ((IDCode&A3P250IDCODE_MASK) == A3P250ID) {
			gotAMT = TRUE;
			SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, TRUE);
            SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, TRUE);
			PanelSave(CSMSetupHandle);
            LoadCSMSetupArray();
            numberDevice++; 		  
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (A3P250)\n", numberDevice, IDCode);
		  }
		  
		  else if (IDCode == HPTDCID) {
			gotAMT = TRUE;
			SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, TRUE);
            SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, TRUE);
			PanelSave(CSMSetupHandle);
            LoadCSMSetupArray();
            numberDevice++; 		  
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (HPTDC)\n", numberDevice, IDCode);
		  }
		  else if (IDCode == TDC_ID) {
			gotAMT = TRUE;
			SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, TRUE);
            SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, TRUE);
			PanelSave(CSMSetupHandle);
            LoadCSMSetupArray();
            numberDevice++; 		  
            if (!verifyFPGAAndPROM)
              printf("Got JTAG Device %2d with IDCode = 0x%08X (TDC V2)\n", numberDevice, IDCode);
		  }
          else if ((IDCode != 0) && (IDCode != 0xFFFFFFFF)) {
            gotUnknown = TRUE;
            numberDevice++;
            IDCodeBar = ~IDCode;
            PROMIDCodeBar0 = IDCodeBar & PROMIDMASK0;
            PROMIDCodeBar1 = IDCodeBar & PROMIDMASK1;
            vertexIIIDCodeBar = IDCode & VERTEXIIIDMASK;
            foundBarDevice = FALSE;
            if ((PROMIDCodeBar0 == XC18V04ID) ||
                (PROMIDCodeBar1 == XCF01SID) || (PROMIDCodeBar1 == XCF02SID) || (PROMIDCodeBar1 == XCF04SID) ||
                (PROMIDCodeBar1 == XCF08PID) || (PROMIDCodeBar1 == XCF16PID) || (PROMIDCodeBar1 == XCF32PID) ||
                (vertexIIIDCodeBar == XC2V1000ID) || (vertexIIIDCodeBar == XC2V2000ID) ||
                (IDCodeBar == AX1000ID) || (IDCodeBar == GOLID) || (IDCodeBar == TTCRXID) ||
                (IDCodeBar == CSM1ID) || (IDCodeBar == CSM2ID) || (IDCodeBar == CSM3ID) || (IDCodeBar == CSMID) ||
                (IDCodeBar == HPTDCID) || (IDCodeBar == A3P250ID)) {
              foundBarDevice = TRUE;
              numberBarDevice++;
            }
            else numberUnknown++;
            if (!verifyFPGAAndPROM) {
              if (foundBarDevice) {
                printf("Got JTAG Device %2d with IDCode = 0x%08X (=~0x%08X, Complement Device!)\n", numberDevice, IDCode, IDCodeBar);
              }
              else {
                printf("Got JTAG Device %2d with IDCode = 0x%08X (Unknown Device)\n", numberDevice, IDCode);
              }
            }
          }
        }
        if (numberDevice != numberUnknown) {
          if ((j == 1) || (j == 2)) {
            if (numberUnknown == 0) {
              printf("Lost first bit of JTAG output!\n"); 
              printf("Possible reasons: 1) Miss one JTAG clock;\n"); 
              printf("                  2) Wrong JTAG clock, such as initially it is low.\n");
            }
            else if (j == 2) {
              printf("Possible lost a bit in the JTAG output!\n"); 
              printf("Possible reasons: 1) Unstable JTAG clock;\n"); 
              printf("                  2) Problem in JTAG drivers.\n"); 
            }
          }
          else if (j == 3) {
            if (numberUnknown == 0) {
              printf("An extra bit at beginning of JTAG output!\n"); 
              printf("Possible reasons: 1) Extra JTAG clock;\n"); 
              printf("                  2) Problem in JTAG drivers.\n");
            }
            else {
              printf("Possible an extra bit in the JTAG output!\n"); 
              printf("Possible reasons: 1) Unstable JTAG clock;\n"); 
              printf("                  2) Problem in JTAG drivers.\n"); 
            }
          }
          if ((numberUnknown == 0) || (j == 0) || (j == 2)) break;
        }
        if (numberDevice == 0) break;
      }
      if (!gotPROM) SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_PROM, FALSE);
      if (!gotCSMChip) {
        SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, FALSE);
        SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMCHIP, ATTR_LABEL_TEXT, " CSM-Chip ");
        SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_GETCSMCHIPID, ATTR_ITEM_NAME, "Get CSM Chip ID");
      }
      if (numberDevice <= 0) {
        printf("Did not find any JTAG device, keep user settings for JTAG chain.\n");
        ReadParameters(PARAALL);
      }
      if (numberBarDevice == 1) {
        printf("Found %d complement JTAG device, most likely TDO is reversed!\n", numberBarDevice);
      }
      else if (numberBarDevice > 1) {
        printf("Found %d complement JTAG devices, most likely TDO is reversed!\n", numberBarDevice);
      }
      UpdateJTAGControlPanel();
    }
    instrLength = 0;
    for (i = numberExpectedDevice-1; i >= 0; i--) {
      IntToBinary(0xFF, instrLength, 8, instrArray, JTAGINSTRLENGTH);
      instrLength += 8;
      JTAGComments(0xFF, SVFFile, 0);
    }
    WriteJTAGInstructions(SVFFile);
    fprintf(SVFFile, "RUNTEST 12 TCK;\n");
    fclose(SVFFile);
  }
  if (gotCSM) CSMJTAGOperation(CSMVERSIONDATE, FALSE);
  if (CSMVersionDate > 2005021522) dimmed = FALSE;
  else dimmed = TRUE;
  SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_DIAG_AMTFULLPHASE, ATTR_DIMMED, dimmed);
  // CSM JTAG Instruction CSMROBITS was miscoded at early version, disable related command button
  if (CSMVersionDate > 2005021617) dimmed = FALSE;
  else dimmed = TRUE;
  SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_DIAG_CSMROBITS, ATTR_DIMMED, dimmed);
  SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMTYPE, CSMType);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMTYPE, CSMType);
  color = VAL_WHITE;
  if (CSMType == CSM1) dimmed = TRUE;
  else {
    if (CSMType == UNKNOWN) color = VAL_RED;
    dimmed = FALSE;
  }
  SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_CSMTYPE, ATTR_TEXT_BGCOLOR, color);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMTYPE, ATTR_TEXT_BGCOLOR, color);
  if (JTAGDRIVERTYPE != CANELMB) dimmed = TRUE;
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_CSMIO, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_RESETCSM, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_SELECTSOFTTDO, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_SELECTHARDTDO, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_SETIOS, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_READIOS, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_RESETCSM, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_SELECTSOFTTDC, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_SELECTHARDTDC, ATTR_DIMMED, dimmed);
  if (CSMType != CSM) dimmed = TRUE;
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_PROGRAMFPGAVIAIO, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(CSMIOsHandle, P_CSMIO_PROGRAMFPGA, ATTR_DIMMED, dimmed);
  JTAGChainControl();
}


void AutoSetupJTAGChain(void) {
  int maxDivider, currDivider, divider;

  SetupJTAGChain();
  GetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_MAX_VALUE, &maxDivider);
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, &currDivider);
  divider = currDivider;
  if (gotUnknown) {
    printf("Got unknown JTAG device(s), JTAG clock maybe too fast, try to slow it down\n");
    while (divider < maxDivider) {
      divider++;
      printf("\nChange JTAG clock control to %d.\n", divider);
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, divider);
      SetJTAGRateDivisorButton();
      SetupJTAGChain();
      if (!gotUnknown) {
        currDivider = divider;
        break;
      }
    }
    if (gotUnknown)
      printf("Still got unknown JTAG device(s) at slowest JTAG clock, please check your hardware.\n");
  }
  SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, currDivider);
  SetJTAGRateDivisorButton();
  ResetTAP();
}


// Function for JTAG diagnostics
//
void GetAllDeviceID(void) {
  char path[256];
  unsigned int IDCode = 0xFFFFFFFF;
  int i, j;
  FILE *SVFFile;
  
  if (controlOption == NORM) {
    GetJTAGChainOnList();
    strcpy(path, "getIDCode.svf");
    if (SVFFile = fopen(path, "w")) {
      GetJTAGChainOnList();
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      JTAGdownload_instr(instrArray, TDC_IDCODE, HPTDCIDCODE, A3P250IDCODE, CSMIDCODE, TTCRXIDCODE, GOLDEVICEID, AX1000IDCODE, VERTEXIIIDCODE, PROMIDCODE);
      JTAGdownload_data(TDC_IDCODE, HPTDCIDCODE, A3P250IDCODE, CSMIDCODE, TTCRXIDCODE, GOLDEVICEID, AX1000IDCODE, VERTEXIIIDCODE, PROMIDCODE);   

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        if (mezzCardsOn) {
          for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
            if (CSMSetupArray[TDCENABLES+i] == 1) {
              if(i==NEWTDC_NUMBER){
                BinaryToInt(&IDCode, HPTDCOffset[i], 32, readbackArray);
                if (IDCode == TDC_ID) printf("Got right TDCV2 ID = 0x%08X for mezzanine card %d\n", IDCode, i);
                else printf("Unknown ID = 0x%08X, expected TDCV2 ID = 0x%08X for mezzanine card %d\n", IDCode, TDC_ID, i);
              }
              else{
                BinaryToInt(&IDCode, HPTDCOffset[i], 32, readbackArray);
                if (IDCode == HPTDCID) printf("Got right HPTDC ID = 0x%08X for mezzanine card %d\n", IDCode, i);
//              else if (IDCode == AMT3ID) printf("Got right AMT3 ID = 0x%08X for mezzanine card %d\n", IDCode, i);
                else printf("Unknown ID = 0x%08X, expected HPTDCID = 0x%08X for mezzanine card %d\n", IDCode, HPTDCID, i);
                BinaryToInt(&IDCode, A3P250Offset[i], 32, readbackArray); 
                if (IDCode == A3P250ID) printf("Got right A3P250 ID = 0x%08X for mezzanine card %d\n", IDCode, i);
//              else if (IDCode == AMT3ID) printf("Got right AMT3 ID = 0x%08X for mezzanine card %d\n", IDCode, i);
                else printf("Unknown ID = 0x%08X, expected A3P250ID = 0x%08X for mezzanine card %d\n", IDCode, A3P250ID, i);
              }          
			}
          }
        }
        if (CSMOn) {
          BinaryToInt(&IDCode, CSMOffset, 32, readbackArray);
          if (IDCode == CSMID) printf("Got right CSM ID = 0x%08X\n", IDCode);
          else if (IDCode == CSM1ID) printf("Got right CSM1 ID = 0x%08X\n", IDCode);
          else if (IDCode == CSM2ID) printf("Got right CSM2 ID = 0x%08X\n", IDCode);
          else if (IDCode == CSM3ID) printf("Got right CSM3 ID = 0x%08X\n", IDCode);
          else if (CSMType == CSM) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSMID);
          else if (CSMType == CSM1) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSM1ID);
          else if (CSMType == CSM2) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSM2ID);
          else if (CSMType == CSM3) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSM3ID);
          else printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSMID);
        }
        if (TTCrxOn) {
          BinaryToInt(&IDCode, TTCrxOffset, 32, readbackArray);
          if (IDCode == TTCRXID) printf("Got right TTCrx ID = 0x%08X\n", IDCode);
          else printf("Unknown ID = 0x%08X, expected TTCRXID = 0x%08X\n", IDCode, TTCRXID);
        }
        if (GOLOn) {
          BinaryToInt(&IDCode, GOLOffset, 32, readbackArray);
          if (IDCode == GOLID) printf("Got right GOL ID = 0x%08X\n", IDCode);
          else printf("Unknown ID = 0x%08X, expected GOLID = 0x%08X\n", IDCode, GOLID);
        }
        if (AX1000On) {
          BinaryToInt(&IDCode, AX1000Offset, 32, readbackArray);
          if (IDCode == AX1000ID) printf("Got right AX1000 ID = 0x%08X\n", IDCode);
          else printf("Unknown ID = 0x%08X, expected AX1000ID = 0x%08X\n", IDCode, AX1000ID);
        }
        if (XC2V1000On) {
          BinaryToInt(&IDCode, vertexIIOffset, 32, readbackArray);
          if ((IDCode&VERTEXIIIDMASK) == XC2V1000ID) printf("Got right XC2V1000 ID = 0x%08X\n", IDCode);
          else printf("Unknown ID = 0x%08X, expected XC2V1000 ID = 0x%08X\n", IDCode, XC2V1000ID);
        }
        if (XC2V2000On) {
          BinaryToInt(&IDCode, vertexIIOffset, 32, readbackArray);
          if ((IDCode&VERTEXIIIDMASK) == XC2V2000ID) printf("Got right XC2V2000 ID = 0x%08X\n", IDCode);
          else printf("Unknown ID = 0x%08X, expected XC2V2000 ID = 0x%08X\n", IDCode, XC2V2000ID);
        }
        if (PROMOn) {
          BinaryToInt(&IDCode, PROMOffset, 32, readbackArray);
          if ((IDCode&PROMIDMASK0) == XC18V04ID) printf("Got right PROM XC18V04 ID = 0x%08X\n", IDCode);
          else if ((IDCode&PROMIDMASK1) == XCF08PID) printf("Got right PROM XCF08P ID = 0x%08X\n", IDCode);
          else if ((IDCode&PROMIDMASK1) == XCF16PID) printf("Got right PROM XCF08P ID = 0x%16X\n", IDCode);
          else if ((IDCode&PROMIDMASK1) == XCF32PID) printf("Got right PROM XCF08P ID = 0x%32X\n", IDCode);
          else if ((IDCode&PROMIDMASK1) == XCF01SID) printf("Got right PROM XCF01S ID = 0x%08X\n", IDCode);
          else if ((IDCode&PROMIDMASK1) == XCF02SID) printf("Got right PROM XCF02S ID = 0x%08X\n", IDCode);
          else if ((IDCode&PROMIDMASK1) == XCF04SID) printf("Got right PROM XCF04S ID = 0x%08X\n", IDCode);
          else printf("Unknown ID = 0x%08X, expected PROMID = 0x%08X\n", IDCode, PROMID);
        }
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Get All JTAG Device ID",
                 "Get All JTAG Device ID through JTAG.");
    controlOption = NORM;
  }
}


void GetPROMID(void) {
  char path[256];
  unsigned int IDCode = 0xFFFFFFFF;
  int i;
  FILE *SVFFile;
  
  if (controlOption == NORM) {

    strcpy(path, "getIDCode.svf");
    if (SVFFile = fopen(path, "w")) {
      GetJTAGChainOnList();
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMIDCODE);     
      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMIDCODE);   

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        BinaryToInt(&IDCode, PROMOffset, 32, readbackArray);
        if ((IDCode&PROMIDMASK0) == XC18V04ID) printf("Got right PROM XC18V04 ID = 0x%08X\n", IDCode);
        else if ((IDCode&PROMIDMASK1) == XCF08PID) printf("Got right PROM XCF08P ID = 0x%08X\n", IDCode);
        else if ((IDCode&PROMIDMASK1) == XCF16PID) printf("Got right PROM XCF08P ID = 0x%16X\n", IDCode);
        else if ((IDCode&PROMIDMASK1) == XCF32PID) printf("Got right PROM XCF08P ID = 0x%32X\n", IDCode);
        else if ((IDCode&PROMIDMASK1) == XCF01SID) printf("Got right PROM XCF01S ID = 0x%08X\n", IDCode);
        else if ((IDCode&PROMIDMASK1) == XCF02SID) printf("Got right PROM XCF02S ID = 0x%08X\n", IDCode);
        else if ((IDCode&PROMIDMASK1) == XCF04SID) printf("Got right PROM XCF04S ID = 0x%08X\n", IDCode);
        else printf("Unknown ID = 0x%08X, expected PROMID = 0x%08X\n", IDCode, PROMID);
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Get PROM ID",
                 "Get PROM ID through JTAG.");
    controlOption = NORM;
  }
}


void GetCSMChipID(void) {
  char path[256];
  unsigned int IDCode = 0xFFFFFFFF;
  int i;
  FILE *SVFFile;
  
  if (controlOption == NORM) {
    GetJTAGChainOnList();
    strcpy(path, "getIDCode.svf");
    if (SVFFile = fopen(path, "w")) {
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000IDCODE, VERTEXIIIDCODE, PROMBYPASS);
      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000IDCODE, VERTEXIIIDCODE, PROMBYPASS);   

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        if (AX1000On) {
          BinaryToInt(&IDCode, AX1000Offset, 32, readbackArray);
          if (IDCode == AX1000ID) printf("Got right AX1000 ID = 0x%08X\n", IDCode);
          else printf("Unknown ID = 0x%08X, expected AX1000 ID = 0x%08X\n", IDCode, AX1000ID);
        }
        if (XC2V1000On) {
          BinaryToInt(&IDCode, vertexIIOffset, 32, readbackArray);
          if ((IDCode&VERTEXIIIDMASK) == XC2V1000ID) printf("Got right XC2V1000 ID = 0x%08X\n", IDCode);
          else printf("Unknown ID = 0x%08X, expected XC2V1000 ID = 0x%08X\n", IDCode, XC2V1000ID);
        }
        if (XC2V2000On) {
          BinaryToInt(&IDCode, vertexIIOffset, 32, readbackArray);
          if ((IDCode&VERTEXIIIDMASK) == XC2V2000ID) printf("Got right XC2V2000 ID = 0x%08X\n", IDCode);
          else printf("Unknown ID = 0x%08X, expected XC2V2000 ID = 0x%08X\n", IDCode, XC2V2000ID);
        }
        if (!(AX1000On || XC2V1000On || XC2V2000On)) {
          printf("CSM Chip type is unknown, unable to get its IDCode.\n");
        }
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    if (CSMChipID == XC2V1000ID) {
      MessagePopup("Help on Get CSM Chip (XC2V1000) ID",
                   "Get CSM Chip (XC2V1000) ID through JTAG.");
    }
    else if (CSMChipID == XC2V2000ID) {
      MessagePopup("Help on Get CSM Chip (XC2V2000) ID",
                   "Get CSM Chip (XC2V2000) ID through JTAG.");
    }
    else if (CSMChipID == AX1000ID) {
      MessagePopup("Help on Get CSM Chip (AX1000) ID",
                   "Get CSM Chip (XC2V1000) ID through JTAG.");
    }
    else {
      MessagePopup("Help on Get CSM Chip ID",
                   "Get CSM Chip ID through JTAG according to chip type.");
    }
    controlOption = NORM;
  }
}


void GetGOLID(void) {
  char path[256];
  unsigned int IDCode = 0xFFFFFFFF;
  int i;
  FILE *SVFFile;
  
  if (controlOption == NORM) {
    GetJTAGChainOnList();
    strcpy(path, "getIDCode.svf");
    if (SVFFile = fopen(path, "w")) {
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLDEVICEID, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
      
      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLDEVICEID, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);   

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        BinaryToInt(&IDCode, GOLOffset, 32, readbackArray);
        if (IDCode == GOLID) printf("Got right GOL ID = 0x%08X\n", IDCode);
        else printf("Unknown ID = 0x%08X, expected GOLID = 0x%08X\n", IDCode, GOLID);
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Get GOL ID", 
                 "Get GOL ID through JTAG.");
    controlOption = NORM;
  }
}


void GetTTCrxID(void) {
  char path[256];
  unsigned int IDCode = 0xFFFFFFFF;
  int i;
  FILE *SVFFile;
  
  if (controlOption == NORM) {
    GetJTAGChainOnList();
    strcpy(path, "getIDCode.svf");
    if (SVFFile = fopen(path, "w")) {
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXIDCODE, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
      
      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXIDCODE, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);   

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        BinaryToInt(&IDCode, TTCrxOffset, 32, readbackArray);
        if (IDCode == TTCRXID) printf("Got right TTCrx ID = 0x%08X\n", IDCode);
        else printf("Unknown ID = 0x%08X, expected TTCRXID = 0x%08X\n", IDCode, TTCRXID);
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Get TTCrx ID", 
                 "Get TTCrx ID through JTAG.");
    controlOption = NORM;
  }
}


void GetCSMID(void) {
  char path[256];
  unsigned int IDCode = 0xFFFFFFFF;
  int i;
  FILE *SVFFile;
  
  if (controlOption == NORM) {
    GetJTAGChainOnList();
    strcpy(path, "getIDCode.svf");
    if (SVFFile = fopen(path, "w")) {
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMIDCODE, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);      
      JTAGdownload_data(TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, CSMIDCODE, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);   

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        BinaryToInt(&IDCode, CSMOffset, 32, readbackArray);
        if (IDCode == CSMID) printf("Got right CSM ID = 0x%08X\n", IDCode);
        else if (IDCode == CSM1ID) printf("Got right CSM1 ID = 0x%08X\n", IDCode);
        else if (IDCode == CSM2ID) printf("Got right CSM2 ID = 0x%08X\n", IDCode);
        else if (IDCode == CSM3ID) printf("Got right CSM3 ID = 0x%08X\n", IDCode);
        else if (CSMType == CSM) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSMID);
        else if (CSMType == CSM1) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSM1ID);
        else if (CSMType == CSM2) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSM2ID);
        else if (CSMType == CSM3) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSM3ID);
        else printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSMID);
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Get CSM ID", 
                 "Get CSM ID through JTAG.");
    controlOption = NORM;
  }
}

// Modified by Xiangting


void GetAllAMTID(void) {
  char path[256];
  unsigned int IDCode = 0xFFFFFFFF;
  int i, j;
  FILE *SVFFile;
  
  if (controlOption == NORM) {
    GetJTAGChainOnList();
    strcpy(path, "getIDCode.svf");
    if (SVFFile = fopen(path, "w")) {
      AllJTAGDeviceInBYPASS(SVFFile, 0);
      JTAGdownload_instr(instrArray, TDC_IDCODE, HPTDCIDCODE, A3P250IDCODE, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);  
      JTAGdownload_data(TDC_IDCODE, HPTDCIDCODE, A3P250IDCODE, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);   

      if (action == DOWNLOAD) {
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        if (mezzCardsOn) {
          for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
            if (CSMSetupArray[TDCENABLES+i] == 1) {
              if(i==NEWTDC_NUMBER){
                BinaryToInt(&IDCode, HPTDCOffset[i], 32, readbackArray);
                if (IDCode == TDC_ID) printf("Got right TDCV2 ID = 0x%08X for mezzanine card %d\n", IDCode, i);
                else printf("Unknown ID = 0x%08X, expected TDCV2 ID = 0x%08X for mezzanine card %d\n", IDCode, TDC_ID, i);
              }
              else{
                BinaryToInt(&IDCode, HPTDCOffset[i], 32, readbackArray);
                if (IDCode == HPTDCID) printf("Got right HPTDC ID = 0x%08X for mezzanine card %d\n", IDCode, i);
                else printf("Unknown ID = 0x%08X, expected AMT3ID = 0x%08X for mezzanine card %d\n", IDCode, HPTDCID, i);     
                BinaryToInt(&IDCode, A3P250Offset[i], 32, readbackArray);
                if (IDCode == A3P250ID) printf("Got right A3P250 ID = 0x%08X for mezzanine card %d\n", IDCode, i);
                else printf("Unknown ID = 0x%08X, expected AMT3ID = 0x%08X for mezzanine card %d\n", IDCode, A3P250ID, i);
              }
            }			
          }
        }
      }
      AllJTAGDeviceInBYPASS(SVFFile, 1);
      fclose(SVFFile);
    }
    else {
      printf("Unable to open SVF file <%s>!\n", path);
      MessagePopup("Failed to Open SVF File",
                   "Unable to open SVF file, please check your disk and filename!");
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Get All AMT ID", 
                 "Get All AMT ID through JTAG.");
    controlOption = NORM;
  }
}

//End

// Function for varies JTAG device operations
//
int CSMJTAGOperation(int instr, int verify) {
  int status = 0, i, j, IDCode, oldDownloadCSMStatus, oldDownloadTTCrxStatus;
  
  LoadCSMSetupArray();
  LoadGOLSetupArray();
  LoadTTCrxSetupArray();
  GetJTAGChainOnList();

  JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCBYPASS, A3P250BYPASS, instr, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);  
  dataLength = 0;
  if (mezzCardsOn) {
    for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
      if (CSMSetupArray[TDCENABLES+i] == 1) {
//Modified by Xiangting
        if(i == NEWTDC_NUMBER){
          HPTDCOffset[i] = dataLength;
          dataArray[dataLength] = 1;
          maskArray[dataLength++] = 0;
          WriteJTAGDataToActionFile(1, HPTDCOffset[i]);
        }
        else{
          HPTDCOffset[i] = dataLength;
          dataArray[dataLength] = 1;
          maskArray[dataLength++] = 0;
          WriteJTAGDataToActionFile(1, HPTDCOffset[i]);
          A3P250Offset[i] = dataLength;
          dataArray[dataLength] = 1;
          maskArray[dataLength++] = 0;
          WriteJTAGDataToActionFile(1, A3P250Offset[i]);
        }
      }
    }
  }
  if (CSMOn) {
    CSMOffset = dataLength;
    if (instr == CSMIDCODE) {
      if (CSMType == CSM) IDCode = CSMID;
      else if (CSMType == CSM1) IDCode = CSM1ID;
      else if (CSMType == CSM2) IDCode = CSM2ID;
      else if (CSMType == CSM3) IDCode = CSM3ID;
      else IDCode = CSMID;
      IntToBinary(IDCode, dataLength, CSMIDCODELENGTH, dataArray, MAXJTAGARRAY);
      for (i = 0; i < CSMIDCODELENGTH; i++) maskArray[dataLength++] = 1;
    }
    else if (instr == CSMAMTPARITYERROR) {
      IntToBinary(AMTParityError, dataLength, CSMMEZZSTATUSLENGTH, dataArray, MAXJTAGARRAY);
      for (i = 0; i < CSMMEZZSTATUSLENGTH; i++) maskArray[dataLength++] = 1;
    }
    else if (instr == CSMAMTPHASEERROR) {
      IntToBinary(AMTPhaseError, dataLength, CSMMEZZSTATUSLENGTH, dataArray, MAXJTAGARRAY);
      for (i = 0; i < CSMMEZZSTATUSLENGTH; i++) maskArray[dataLength++] = 1;
    }
    else if (instr == CSMAMTFULLPHASE) {
      IntToBinary(AMTFullPhaseError, dataLength, CSMMEZZSTATUSLENGTH, dataArray, MAXJTAGARRAY);
      for (i = 0; i < CSMMEZZSTATUSLENGTH; i++) maskArray[dataLength++] = 1;
      for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
        IntToBinary(AMTPhaseSamples[j], dataLength, 4, dataArray, MAXJTAGARRAY);
        for (i = 0; i < 4; i++) maskArray[dataLength++] = 1;
      }
      dataArray[dataLength] = AMTFullPhaseErrorBit;
      maskArray[dataLength++] = 1;
    }
    else if (instr == CSMVERSIONDATE) {
      IntToBinary(CSMVersionDate, dataLength, CSMVERSIONDATELENGTH, dataArray, MAXJTAGARRAY);
      for (i = 0; i < CSMVERSIONDATELENGTH; i++) maskArray[dataLength++] = 1;
    }
    else if ((instr == CSMFULLRW) || (instr == CSMFULLRO)) {
      for (i = dataLength; i < dataLength+CSMSTATUSLENGTH; i++) {
        dataArray[i] = CSMStatusArray[i-dataLength];
        maskArray[i] = 0;
      }
      dataLength += CSMSTATUSLENGTH;
      for (i = dataLength; i < dataLength+CSMSETUPLENGTH; i++) {
        if (instr == CSMFULLRO) {
          if ((action == DOWNLOAD) && !downloadCSMSetupDone) {
            dataArray[i] = CSMSetupArray[i-dataLength];
            if (i == (dataLength+CSMCMD)) maskArray[i] = 0;
            else maskArray[i] = 1;
          }
          else {
            dataArray[i] = downloadedCSMSetupArray[i-dataLength];
            maskArray[i] = 1;
          }
        }
        else {
          dataArray[i] = CSMSetupArray[i-dataLength];
          maskArray[i] = 1;
          downloadedCSMSetupArray[i-dataLength] = dataArray[i];
        }
        if (CSMLastNextState != CSMNextState) {
          if (((i-dataLength) >= CSMNEXTSTATE) && ((i-dataLength) < (CSMNEXTSTATE+4))) maskArray[i] = 0;
        }
      }
      dataLength += CSMSETUPLENGTH;
      for (i = dataLength; i < dataLength+TTCRXSETUPLENGTH; i++) {
        dataArray[i] = TTCrxSetupArray[i-dataLength];
        maskArray[i] = 1;
      }
      dataLength += TTCRXSETUPLENGTH;
    }
    else if ((instr == CSMCONFIGRW) || (instr == CSMCONFIGRO)) {
      for (i = dataLength; i < dataLength+CSMSETUPLENGTH; i++) {
        if (instr == CSMCONFIGRO) {
          if ((action == DOWNLOAD) && !downloadCSMSetupDone) {
            dataArray[i] = CSMSetupArray[i-dataLength];
            if (i == (dataLength+CSMCMD)) maskArray[i] = 0;
            else maskArray[i] = 1;
          }
          else {
            dataArray[i] = downloadedCSMSetupArray[i-dataLength];
            maskArray[i] = 1;
          }
        }
        else {
          dataArray[i] = CSMSetupArray[i-dataLength];
          maskArray[i] = 1;
          downloadedCSMSetupArray[i-dataLength] = dataArray[i];
        }
        if (CSMLastNextState != CSMNextState) {
          if (((i-dataLength) >= CSMNEXTSTATE) && ((i-dataLength) < (CSMNEXTSTATE+4))) maskArray[i] = 0;
        }
      }
      dataLength += CSMSETUPLENGTH;
      for (i = dataLength; i < dataLength+TTCRXSETUPLENGTH; i++) {
        dataArray[i] = TTCrxSetupArray[i-dataLength];
        maskArray[i] = 1;
      }
      dataLength += TTCRXSETUPLENGTH;
	}
    else if ((instr == CSMCSMPARMRW) || (instr == CSMCSMPARMRO)) {
      for (i = dataLength; i < dataLength+CSMSETUPLENGTH; i++) {
        if (instr == CSMCSMPARMRO) {
          if ((action == DOWNLOAD) && !downloadCSMSetupDone) {
            dataArray[i] = CSMSetupArray[i-dataLength];
            if (i == (dataLength+CSMCMD)) maskArray[i] = 0;
            else maskArray[i] = 1;
          }
          else {
            dataArray[i] = downloadedCSMSetupArray[i-dataLength];
            maskArray[i] = 1;
          }
        }
        else {
          dataArray[i] = CSMSetupArray[i-dataLength];
          maskArray[i] = 1;
          downloadedCSMSetupArray[i-dataLength] = dataArray[i];
        }
        if (CSMLastNextState != CSMNextState) {
          if (((i-dataLength) >= CSMNEXTSTATE) && ((i-dataLength) < (CSMNEXTSTATE+4))) maskArray[i] = 0;
        }
      }
      dataLength += CSMSETUPLENGTH;
	}
    else if ((instr == CSMCSMSTATUS) || (instr == CSMTTCRXSTATUS) || (instr == CSMROBITS)) {
      if ((instr == CSMCSMSTATUS) || (instr == CSMROBITS)) {
        for (i = dataLength; i < dataLength+CSMCSMSTATUSLENGTH; i++) {
          j = i-dataLength;
          dataArray[i] = CSMStatusArray[j];
          if (j == I2COPERATIONBIT) maskArray[i] = 0;
          else if (j == TTCRXI2CCHECKERROR) maskArray[i] = 0;
          else maskArray[i] = 1;
        }
        dataLength += CSMCSMSTATUSLENGTH;
      }
      if (instr == CSMROBITS) {
        for (i = dataLength; i < dataLength+CSMMEZZSTATUSLENGTH; i++) {
          dataArray[i] = CSMStatusArray[CSMCSMSTATUSLENGTH+i-dataLength];
          maskArray[i] = 1;
        }
        dataLength += CSMMEZZSTATUSLENGTH;
      }
      if ((instr == CSMTTCRXSTATUS) || (instr == CSMROBITS)) {
        for (i = dataLength; i < dataLength+CSMTTCRXSTATUSLENGTH; i++) {
          j = CSMCSMSTATUSLENGTH+CSMMEZZSTATUSLENGTH+i-dataLength;
          dataArray[i] = CSMStatusArray[j];
          if ((j >= TTCRXSERRORCOUNT0) && (j < TTCRXID0)) maskArray[i] = 0;
          else if ((j >= TTCRXBUNCHCOUNTER00) && (j <= TTCRXREADBACKHIGH)) maskArray[i] = 0;
          else maskArray[i] = 1;
        }
        dataLength += CSMTTCRXSTATUSLENGTH;
      }
    }
	else {
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
	}
    if (!verify) for (i = CSMOffset; i < dataLength; i++) maskArray[i] = 0;
    WriteJTAGDataToActionFile(dataLength-CSMOffset, CSMOffset);
  }
  if (TTCrxOn) {
    TTCrxOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
    WriteJTAGDataToActionFile(1, TTCrxOffset);
  }
  if (GOLOn) {
    GOLOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
    WriteJTAGDataToActionFile(1, GOLOffset);
  }
  if (AX1000On) {
    AX1000Offset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
    WriteJTAGDataToActionFile(1, AX1000Offset);
  }
  if (XC2V1000On || XC2V2000On) {
    vertexIIOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
    WriteJTAGDataToActionFile(1, vertexIIOffset);
  }
  if (PROMOn) {
    PROMOffset = dataLength;
    dataArray[dataLength] = 1;
    maskArray[dataLength++] = 0;
    WriteJTAGDataToActionFile(1, PROMOffset);
  }
  if (action == DOWNLOAD) {
    JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
    JTAGScanAllData(dataLength, dataArray, readbackArray);
    if ((instr == CSMFULLRW) || (instr == CSMCONFIGRW) || (instr == CSMCSMPARMRW)) {
      if (CSMNextState == CMDRESETTTCRX) WaitInMillisecond(1000);       // Wait for 1 second
      else if (CSMNextState == CMDRESETGOL) WaitInMillisecond(1000);    // Wait for 1 second
      else if (CSMNextState == CMDRESETERROR) WaitInMillisecond(1000);  // Wait for 1 second
      else if (CSMNextState == CMDTTCRXLOAD) WaitInMillisecond(1000);   // Wait for 1 second
    }
    if (CSMOn) {
      oldDownloadCSMStatus = downloadCSMStatus;
      oldDownloadTTCrxStatus = downloadTTCrxStatus;
      if (verify) status = CheckFullJTAGDataArray();
	  printf("if (verify) status = CheckFullJTAGDataArray();  \n");
	  printf("if (verify) status = CheckFullJTAGDataArray();  \n");  	  
      CSMTestStatus = status;
      if (instr == CSMIDCODE) {
        CSMTestStatus = 0;
        BinaryToInt(&IDCode, CSMOffset, CSMIDCODELENGTH, readbackArray);
        if (IDCode == CSMID) printf("Got right CSM ID = 0x%08X\n", IDCode);
        else if (IDCode == CSM1ID) printf("Got right CSM1 ID = 0x%08X\n", IDCode);
        else if (IDCode == CSM2ID) printf("Got right CSM2 ID = 0x%08X\n", IDCode);
        else if (IDCode == CSM3ID) printf("Got right CSM3 ID = 0x%08X\n", IDCode);
        else if (CSMType == CSM) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSMID);
        else if (CSMType == CSM1) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSM1ID);
        else if (CSMType == CSM2) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSM2ID);
        else if (CSMType == CSM3) printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSM3ID);
        else {
          CSMTestStatus = 1;
          printf("Unknown ID = 0x%08X, expected CSMID = 0x%08X\n", IDCode, CSMID);
        }
      }
      else if (instr == CSMAMTPARITYERROR) {
        BinaryToInt(&AMTParityError, CSMOffset, CSMMEZZSTATUSLENGTH, readbackArray);
        if ((AMTParityError&mezzEnables) != 0) {
          printf("*** AMT Parity Error (ErrBits=0x%05X)\n", AMTParityError);
        }
        else {
          printf("No AMT Parity Error (ErrBits=0x%05X)\n", AMTParityError);
        }
      }
      else if (instr == CSMAMTPHASEERROR) {
        BinaryToInt(&AMTPhaseError, CSMOffset, CSMMEZZSTATUSLENGTH, readbackArray);
        if ((AMTPhaseError&mezzEnables) != 0) {
          printf("*** (JTAG Instr=0x%02x) AMT Phase Sample Error (ErrBits=0x%05X)\n", instr, AMTPhaseError);
        }
        else {
          printf("(JTAG Instr=0x%02x) No AMT Phase Sample Error (ErrBits=0x%05X)\n", instr, AMTPhaseError);
        }
      }
      else if (instr == CSMAMTFULLPHASE) {
        BinaryToInt(&AMTFullPhaseError, CSMOffset, CSMMEZZSTATUSLENGTH, readbackArray);
        for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
          BinaryToInt(&AMTPhaseSamples[j], CSMOffset+CSMMEZZSTATUSLENGTH+4*j, 4, readbackArray);
        }
        AMTFullPhaseErrorBit = readbackArray[CSMOffset+5*CSMMEZZSTATUSLENGTH];
        if ((AMTFullPhaseErrorBit != 0) || ((AMTFullPhaseError&mezzEnables) != 0)) {
          printf("*** (JTAG Instr=0x%02x) AMT Phase Sample Error (PhaseErr=%d ErrBits=0x%05X)\n", instr, AMTFullPhaseErrorBit, AMTFullPhaseError);
        }
        else {
          printf("(JTAG Instr=0x%02x) No AMT Phase Sample Error (PhaseErr=%d ErrBits=0x%05X)\n", instr, AMTFullPhaseErrorBit, AMTFullPhaseError);
        }
        printf("AMT Phase Samples (%d-0) : ", MAXNUMBERMEZZANINE-1);
        for (j = MAXNUMBERMEZZANINE-1; j >= 0; j--) printf("%01X", AMTPhaseSamples[j]);
        printf("\n");
      }
      else if (instr == CSMVERSIONDATE) {
        BinaryToInt(&CSMVersionDate, CSMOffset, CSMVERSIONDATELENGTH, readbackArray);
        printf("CSM Version Date (YYYYMMDDhh) = %010d\n", CSMVersionDate);
      }
      else if ((instr == CSMFULLRW) || (instr == CSMFULLRO)) {
        downloadCSMSetupDone = TRUE;
        if (verify) {
          if (downloadCSMStatus == 0) {
            for (i = 0; i < CSMSTATUSLENGTH; i++) CSMStatusArray[i] = readbackArray[CSMOffset+i];
            printf("CSM and TTCrx setup is verified successfully through JTAG.\n");
          }
          else {
            numberBadCSMDownload++;
            printf("Failed in verifying CSM and TTCrx setup through JTAG, downloadCSMStatus = %d\n", downloadCSMStatus);
          }
        }
        for (i = 0; i < CSMSTATUSLENGTH; i++) CSMStatusArray[i] = readbackArray[CSMOffset+i];
        if (instr == CSMFULLRW) {
          CSMLastNextState = CSMNextState;
          numberCSMDownload++;
        }
        else if (instr == CSMFULLRO) {
          downloadCSMStatus = oldDownloadCSMStatus;
          downloadTTCrxStatus = oldDownloadTTCrxStatus;
        }
        oldDownloadCSMStatus = downloadCSMStatus;
        oldDownloadTTCrxStatus = downloadTTCrxStatus;
      }
      else if ((instr == CSMCONFIGRW) || (instr == CSMCONFIGRO)) {
        downloadCSMSetupDone = TRUE;
        if (verify) {
          if (downloadCSMStatus == 0) {
            printf("CSM and TTCrx configuration is verified successfully through JTAG.\n");
          }
          else {
            numberBadCSMDownload++;
            printf("Failed in verifying CSM and TTCrx configuration through JTAG, downloadCSMStatus = %d\n", downloadCSMStatus);
          }
        }
        if (instr == CSMCONFIGRW) {
          CSMLastNextState = CSMNextState;
          numberCSMDownload++;
        }
        else if (instr == CSMCONFIGRO) {
          downloadCSMStatus = oldDownloadCSMStatus;
          downloadTTCrxStatus = oldDownloadTTCrxStatus;
        }
        oldDownloadCSMStatus = downloadCSMStatus;
        oldDownloadTTCrxStatus = downloadTTCrxStatus;
      }
      else if ((instr == CSMCSMPARMRW) || (instr == CSMCSMPARMRO)) {
        downloadCSMSetupDone = TRUE;
        if (verify) {
          if (downloadCSMStatus == 0) {
            printf("CSM setup parameter is verified successfully through JTAG.\n");
          }
          else {
            numberBadCSMDownload++;
            printf("Failed in verifying CSM setup parameter through JTAG, downloadCSMStatus = %d\n", downloadCSMStatus);
          }
          numberCSMDownload++;
        }
        downloadTTCrxStatus = oldDownloadTTCrxStatus;
        if (instr == CSMCSMPARMRW) {
          CSMLastNextState = CSMNextState;
          numberCSMDownload++;
        }
        else if (instr == CSMCSMPARMRO) {
          downloadCSMStatus = oldDownloadCSMStatus;
        }
        oldDownloadCSMStatus = downloadCSMStatus;
      }
      else if ((instr == CSMCSMSTATUS) || (instr == CSMTTCRXSTATUS) || (instr == CSMROBITS)) {
        if ((instr == CSMCSMSTATUS) || (instr == CSMROBITS)) {
          for (i = 0; i < CSMCSMSTATUSLENGTH; i++) CSMStatusArray[i] = readbackArray[CSMOffset+i];
        }
        if (instr == CSMROBITS) {
          j = CSMCSMSTATUSLENGTH;
          for (i = j; i < j+CSMMEZZSTATUSLENGTH; i++) CSMStatusArray[i] = readbackArray[CSMOffset+i];
        }
        if ((instr == CSMTTCRXSTATUS) || (instr == CSMROBITS)) {
          if (instr == CSMTTCRXSTATUS) j = 0;
          else j = CSMCSMSTATUSLENGTH + CSMMEZZSTATUSLENGTH;
          for (i = 0; i < CSMTTCRXSTATUSLENGTH; i++) 
            CSMStatusArray[CSMCSMSTATUSLENGTH+CSMMEZZSTATUSLENGTH+i] = readbackArray[CSMOffset+j+i];
        }
        if (verify) {
          if (downloadCSMStatus == 0) {
            if (instr == CSMCSMSTATUS) printf("CSM status bits have been verified successfully through JTAG.\n");
            if (instr == CSMTTCRXSTATUS) printf("TTCrx status bits have been verified successfully through JTAG.\n");
            if (instr == CSMROBITS) printf("CSM read only bits have been verified successfully through JTAG.\n");
          }
          else {
            if (instr == CSMCSMSTATUS) printf("Failed to verify CSM status bits through JTAG status = %d.\n", downloadCSMStatus);
            if (instr == CSMTTCRXSTATUS) printf("Failed to verify TTCrx status bits through JTAG status = %d.\n", downloadCSMStatus);
            if (instr == CSMROBITS) printf("Failed to verify CSM read only bits through JTAG status = %d.\n", downloadCSMStatus);
          }
        }
      }
      downloadCSMStatus = oldDownloadCSMStatus;
      downloadTTCrxStatus = oldDownloadTTCrxStatus;
      validCSMStatus = TRUE;
      UpdateCSMSetupPanel();
    }
    UpdateJTAGControlPanel();     // Update JTAG control panel
    HandleUserButtons(PanelDone);
  }

  return status;
}


int GOLJTAGOperation(int instr, int verify) {
  int status = 0;
  
  return status;
}


int TTCrxJTAGOperation(int instr, int verify) {
  int status = 0;
  
  return status;
}


int AMTJTAGOperation(int instr, int verify) {
  int status = 0;
  
  return status;
}


int ASDJTAGOperation(int instr, int verify) {
  int status = 0;
  
  return status;
}


// Routines to search mezzanine card in JTAG chain and downloading test
//
void UpMezzCardJTAGStatus(void)
{
  if (controlOption == NORM) {
    HidePanel(JTAGControlHandle);
    DisplayPanel(MezzJTAGStatusHandle);
    SetActivePanel(MezzJTAGStatusHandle);
    SearchMezzanineCardsAndSetupTest();
    HidePanel(JTAGControlHandle);
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Search Mezzanine Cards (AMTs)", 
                 "Bring up Mezzanine Card JTAG Status Panel\n"
                 "and search mezzanine card in JTAG chain and perform downloading test.");
  controlOption = NORM;
}


void SearchMezzanineCardsAndSetupTest(void) {
  int savedMezzEnables, savedEnable, mezz, color, button[MAXNUMBERMEZZANINE];
  int savedNumberCSMDownload, savedNumberBadCSMDownload; 

  button[0] = P_MEZZJTAG_MEZZ00;
  button[1] = P_MEZZJTAG_MEZZ01;
  button[2] = P_MEZZJTAG_MEZZ02;
  button[3] = P_MEZZJTAG_MEZZ03;
  button[4] = P_MEZZJTAG_MEZZ04;
  button[5] = P_MEZZJTAG_MEZZ05;
  button[6] = P_MEZZJTAG_MEZZ06;
  button[7] = P_MEZZJTAG_MEZZ07;
  button[8] = P_MEZZJTAG_MEZZ08;
  button[9] = P_MEZZJTAG_MEZZ09;
  button[10] = P_MEZZJTAG_MEZZ10;
  button[11] = P_MEZZJTAG_MEZZ11;
  button[12] = P_MEZZJTAG_MEZZ12;
  button[13] = P_MEZZJTAG_MEZZ13;
  button[14] = P_MEZZJTAG_MEZZ14;
  button[15] = P_MEZZJTAG_MEZZ15;
  button[16] = P_MEZZJTAG_MEZZ16;
  button[17] = P_MEZZJTAG_MEZZ17;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    SetCtrlAttribute(MezzJTAGStatusHandle, button[mezz], ATTR_TEXT_BGCOLOR, VAL_WHITE);
  }
  searchAMTStatus = 0;
  savedMezzEnables = mezzEnables;
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, &savedEnable);
  mezzCardSetupAll = TRUE;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    savedNumberCSMDownload = numberCSMDownload;
    savedNumberBadCSMDownload = numberBadCSMDownload;
    SetCtrlVal(MezzJTAGStatusHandle, P_MEZZJTAG_CHECKINGMEZZ, mezz);
    SetCtrlAttribute(MezzJTAGStatusHandle, P_MEZZJTAG_CHECKINGMEZZ, ATTR_TEXT_BGCOLOR, VAL_GREEN);
    mezzEnables = (1 << mezz);
    SaveParameters(PARAMEZZENABLES);
    SetMezzCardEnables();
    SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
    CSMSetupDone();
    SetupJTAGChain();
    if (gotAMT) {
      printf("Find mezzanine card %d in JTAG chain.\n", mezz);
      DownloadMezzCardSetup();
      if (MezzCardSetupStatus(mezz) == 0) color = VAL_GREEN;
      else {
        if (((1<<mezz)&savedMezzEnables) == 0) color = VAL_YELLOW;
        else {
          searchAMTStatus |= (1 << mezz);
          color = VAL_RED;
        }
      }
    }
    else {
      if (((1<<mezz)&savedMezzEnables) == 0) {
        color = VAL_YELLOW;
        numberCSMDownload = savedNumberCSMDownload;
        numberBadCSMDownload = savedNumberBadCSMDownload;
      }
      else {
        searchAMTStatus |= (1 << mezz);
        color = VAL_RED;
      }
    }
    SetCtrlAttribute(MezzJTAGStatusHandle, button[mezz], ATTR_TEXT_BGCOLOR, color);
    HandleUserButtons(PanelDone);
  }
  SetCtrlVal(MezzJTAGStatusHandle, P_MEZZJTAG_CHECKINGMEZZ, -1);
  SetCtrlAttribute(MezzJTAGStatusHandle, P_MEZZJTAG_CHECKINGMEZZ, ATTR_TEXT_BGCOLOR, VAL_MAGENTA);
  mezzEnables = savedMezzEnables;
  SaveParameters(PARAMEZZENABLES);
  SetMezzCardEnables();
  CSMSetupDone();
  SetupJTAGChain();
  DownloadMezzCardSetup();
}


void OpenActionFile(int csm) {
  if (strlen(actFileName) <= 0) return;
  actFile = fopen(actFileName, "w");
  if (actFile == NULL) return;
  fprintf(actFile, "# Action file for CSM %d:\n", csm); 
  fprintf(actFile, "# Generated by CSM JTAG Control on %s\n", WeekdayTimeString(time(NULL))); 
  fprintf(actFile, "# For the bit string (started with character I or A) is treated as binary number,\n"); 
  fprintf(actFile, "# i.e., the right most bit is the first bit\n"); 
  fprintf(actFile, "# Pay attention:\n"); 
  fprintf(actFile, "#   If bits are read in as string, the left most bit is the first character in the string!\n"); 
  fprintf(actFile, "#\n"); 
  fprintf(actFile, "# Action file has following commands\n"); 
  fprintf(actFile, "#   J Name-of-JTAG-Device\n"); 
  fprintf(actFile, "#     List of JTAG devices      PROM : chip XC18V04 or chip XCF08P\n");
  fprintf(actFile, "#                           XC2V1000 : Xilinx chip XC2V1000\n");
  fprintf(actFile, "#                           XC2V2000 : Xilinx chip XC2V2000\n");
  fprintf(actFile, "#                             AX1000 : Actel chip AX1000\n");
  fprintf(actFile, "#                                GOL : GOL\n");
  fprintf(actFile, "#                              TTCrx : TTCrx\n");
  fprintf(actFile, "#                                CSM : CSM\n");
  fprintf(actFile, "#                                AMT : AMT\n");
  fprintf(actFile, "#                                ASD : ASD\n");
  fprintf(actFile, "#   I Instruction-bit-string\n"); 
  fprintf(actFile, "#   D Data-bit-string\n"); 
  fprintf(actFile, "#   M Mask-bit-string\n"); 
  fprintf(actFile, "#     1 = check corresponding bit\n"); 
  fprintf(actFile, "#     0 = do not check corresponding bit\n"); 
  fprintf(actFile, "#   U ELMB-storage-number\n"); 
  fprintf(actFile, "#   W Waiting-time-in-millionsecond\n"); 
  fprintf(actFile, "#\n");
}


void CloseActionFile(void) {
  if (actFile == NULL) return;
  fprintf(actFile, "#\n");
  if (ELMBStorageNumber >= 0) fprintf(actFile, "U %d\n", ELMBStorageNumber);
  if (waitTimeInMillionSecond > 0) fprintf(actFile, "W %d\n", waitTimeInMillionSecond);
  fprintf(actFile, "#\n");
  fprintf(actFile, "# end of action list\n");
  fclose(actFile);
  actFile = NULL;
}


void WriteCommentToActionFile(char *comment)
{
  if (actFile == NULL) return;
  fprintf(actFile, "# %s\n", comment);
}

//Modified by Xiangting
void WriteJTAGInstructionToActionFile(int IDCode, int instruction, int mezzNumber)
{
  int length, i, PROMIDCode0, PROMIDCode1;
  char str[60], JStr[60], PROMStr[10];
  
  if (actFile == NULL) return;
  else if (newInstructionList) {
    newInstructionList = FALSE;
    fprintf(actFile, "#\n");
    fprintf(actFile, "# -------- JTAG Instruction List -------------\n");
    fprintf(actFile, "#\n");
  }
  strcpy(str, "");
  strcpy(JStr, "");
  strcpy(PROMStr, "");
  PROMIDCode0 = IDCode & PROMIDMASK0;
  PROMIDCode1 = IDCode & PROMIDMASK1;
  if ((IDCode == XC2V1000ID) && XC2V2000On) IDCode = XC2V2000ID;
  else if ((IDCode == XC2V2000ID) && XC2V1000On) IDCode = XC2V1000ID;
  if ((IDCode == PROMID) ||
      (PROMIDCode0 == XC18V04ID) ||
      (PROMIDCode1 == XCF08PID) || (PROMIDCode1 == XCF16PID) || (PROMIDCode1 == XCF32PID) ||
      (PROMIDCode1 == XCF01SID) || (PROMIDCode1 == XCF02SID) || (PROMIDCode1 == XCF04SID)) {
    if (PROMIDCode0 == XC18V04ID) strcpy(PROMStr, "XC18V04");
    else if (PROMIDCode1 == XCF08PID) strcpy(PROMStr, "XCF08P");
    else if (PROMIDCode1 == XCF16PID) strcpy(PROMStr, "XCF16P");
    else if (PROMIDCode1 == XCF32PID) strcpy(PROMStr, "XCF32P");
    else if (PROMIDCode1 == XCF01SID) strcpy(PROMStr, "XCF01S");
    else if (PROMIDCode1 == XCF02SID) strcpy(PROMStr, "XCF02S");
    else if (PROMIDCode1 == XCF04SID) strcpy(PROMStr, "XCF04S");
    length = PROMINSTLENGTH;
    PROMInstr = instruction;
    strcpy(JStr, "PROM");
    if (instruction == PROMEXTEST) sprintf(str, "# PROM %s Instruction 'EXTEST'", PROMStr);
    else if (instruction == PROMSAMPLEPRELOAD) sprintf(str, "# PROM %s Instruction 'SAMPLE/PRELOAD'", PROMStr);
    else if (instruction == PROMCONFIG) sprintf(str, "# PROM %s Instruction 'CONFIG'", PROMStr);
    else if (instruction == PROMCLAMP) sprintf(str, "# PROM %s Instruction 'CLAMP'", PROMStr);
    else if (instruction == PROMHIGHZ) sprintf(str, "# PROM %s Instruction 'HIGHZ'", PROMStr);
    else if (instruction == PROMUSERCODE) sprintf(str, "# PROM %s Instruction 'USERCODE'", PROMStr);
    else if (instruction == PROMIDCODE) sprintf(str, "# PROM %s Instruction 'IDCODE'", PROMStr);
    else if (instruction == PROMBYPASS) sprintf(str, "# PROM %s Instruction 'BYPASS'", PROMStr);
    else sprintf(str, "# PROM Unknown Instruction");
  }
  else if (IDCode == XC2V1000ID) {
    length = VERTEXIIINSTLENGTH;
    vertexIIInstr = instruction;
    strcpy(JStr, "XC2V1000");
    if (instruction == VERTEXIIEXTEST) strcpy(str, "# XC2V1000 Instruction 'EXTEST'");
    else if (instruction == VERTEXIISAMPLE) strcpy(str, "# XC2V1000 Instruction 'SAMPLE'");
    else if (instruction == VERTEXIIUSER1) strcpy(str, "# XC2V1000 Instruction 'USER1'");
    else if (instruction == VERTEXIIUSER2) strcpy(str, "# XC2V1000 Instruction 'USER2'");
    else if (instruction == VERTEXIICFGOUT) strcpy(str, "# XC2V1000 Instruction 'CFGOUT'");
    else if (instruction == VERTEXIICFGIN) strcpy(str, "# XC2V1000 Instruction 'CFGIN'");
    else if (instruction == VERTEXIIINTEST) strcpy(str, "# XC2V1000 Instruction 'INTEST'");
    else if (instruction == VERTEXIIUSERCODE) strcpy(str, "# XC2V1000 Instruction 'USERCODE'");
    else if (instruction == VERTEXIIIDCODE) strcpy(str, "# XC2V1000 Instruction 'IDCODE'");
    else if (instruction == VERTEXIIHIGHZ) strcpy(str, "# XC2V1000 Instruction 'HIGHZ'");
    else if (instruction == VERTEXIIJPROGB) strcpy(str, "# XC2V1000 Instruction 'JPROGB'");
    else if (instruction == VERTEXIIJSTART) strcpy(str, "# XC2V1000 Instruction 'START'");
    else if (instruction == VERTEXIIJSHUTDOWN) strcpy(str, "# XC2V1000 Instruction 'SHUTDOWN'");
    else if (instruction == VERTEXIIBYPASS) strcpy(str, "# XC2V1000 Instruction 'BYPASS'");
    else strcpy(str, "# XC2V1000 Unknown Instruction");
  }
  else if (IDCode == XC2V2000ID) {
    length = VERTEXIIINSTLENGTH;
    vertexIIInstr = instruction;
    strcpy(JStr, "XC2V2000");
    if (instruction == VERTEXIIEXTEST) strcpy(str, "# XC2V2000 Instruction 'EXTEST'");
    else if (instruction == VERTEXIISAMPLE) strcpy(str, "# XC2V2000 Instruction 'SAMPLE'");
    else if (instruction == VERTEXIIUSER1) strcpy(str, "# XC2V2000 Instruction 'USER1'");
    else if (instruction == VERTEXIIUSER2) strcpy(str, "# XC2V2000 Instruction 'USER2'");
    else if (instruction == VERTEXIICFGOUT) strcpy(str, "# XC2V2000 Instruction 'CFGOUT'");
    else if (instruction == VERTEXIICFGIN) strcpy(str, "# XC2V2000 Instruction 'CFGIN'");
    else if (instruction == VERTEXIIINTEST) strcpy(str, "# XC2V2000 Instruction 'INTEST'");
    else if (instruction == VERTEXIIUSERCODE) strcpy(str, "# XC2V2000 Instruction 'USERCODE'");
    else if (instruction == VERTEXIIIDCODE) strcpy(str, "# XC2V2000 Instruction 'IDCODE'");
    else if (instruction == VERTEXIIHIGHZ) strcpy(str, "# XC2V2000 Instruction 'HIGHZ'");
    else if (instruction == VERTEXIIJPROGB) strcpy(str, "# XC2V2000 Instruction 'JPROGB'");
    else if (instruction == VERTEXIIJSTART) strcpy(str, "# XC2V2000 Instruction 'START'");
    else if (instruction == VERTEXIIJSHUTDOWN) strcpy(str, "# XC2V2000 Instruction 'SHUTDOWN'");
    else if (instruction == VERTEXIIBYPASS) strcpy(str, "# XC2V2000 Instruction 'BYPASS'");
    else strcpy(str, "# XC2V2000 Unknown Instruction");
  }
  else if (IDCode == AX1000ID) {
    length = AX1000INSTLENGTH;
    AX1000Instr = instruction;
    strcpy(JStr, "AX1000");
    if (instruction == AX1000EXTEST) strcpy(str, "# AX1000 Instruction 'EXTEST'");
    else if (instruction == AX1000PRELOAD) strcpy(str, "# AX1000 Instruction 'PRELOAD'");
    else if (instruction == AX1000INTEST) strcpy(str, "# AX1000 Instruction 'INTEST'");
    else if (instruction == AX1000USERCODE) strcpy(str, "# AX1000 Instruction 'USERCODE'");
    else if (instruction == AX1000IDCODE) strcpy(str, "# AX1000 Instruction 'IDCODE'");
    else if (instruction == AX1000HIGHZ) strcpy(str, "# AX1000 Instruction 'HIGHZ'");
    else if (instruction == AX1000CLAMP) strcpy(str, "# AX1000 Instruction 'CLAMP'");
    else if (instruction == AX1000DIAGNOSTIC) strcpy(str, "# AX1000 Instruction 'DIAGNOSTIC'");
    else if (instruction == AX1000BYPASS) strcpy(str, "# AX1000 Instruction 'BYPASS'");
    else strcpy(str, "# AX1000 Unknown Instruction");
  }
  else if (IDCode == GOLID) {
    length = GOLINSTLENGTH;
    GOLInstr = instruction;
    strcpy(JStr, "GOL");
    if (instruction == GOLEXTEST) strcpy(str, "# GOL Instruction 'EXTEST'");
    else if (instruction == GOLDEVICEID) strcpy(str, "# GOL Instruction 'DEVICE_ID'");
    else if (instruction == GOLCONFRW) strcpy(str, "# GOL Instruction 'CONFRW'");
    else if (instruction == GOLCONFRO) strcpy(str, "# GOL Instruction 'CONFRO'");
    else if (instruction == GOLCORETEST) strcpy(str, "# GOL Instruction 'CORETEST'");
    else if (instruction == GOLBYPASS) strcpy(str, "# GOL Instruction 'BYPASS'");
    else strcpy(str, "# GOL Unknown Instruction");
  }
  else if (IDCode == TTCRXID) {
    length = TTCRXINSTLENGTH;
    TTCrxInstr = instruction;
    strcpy(JStr, "TTCrx");
    if (instruction == TTCRXEXTEST) strcpy(str, "# TTCrx Instruction 'EXTEST'");
    else if (instruction == TTCRXIDCODE) strcpy(str, "# TTCrx Instruction 'IDCODE'");
    else if (instruction == TTCRXINTEST) strcpy(str, "# TTCrx Instruction 'INTEST'");
    else if (instruction == TTCRXBYPASS) strcpy(str, "# TTCrx Instruction 'BYPASS'");
    else strcpy(str, "# TTCrx Unknown Instruction");
  }
  else if ((IDCode == CSMID) || (IDCode == CSM1ID) || (IDCode == CSM2ID) || (IDCode == CSM3ID)) {
    length = CSMINSTLENGTH;
    CSMInstr = instruction;
    strcpy(JStr, "CSM");
    if (instruction == CSMFULLRO) strcpy(str, "# CSM Instruction 'FULLRO'");
    else if (instruction == CSMFULLRW) strcpy(str, "# CSM Instruction 'FULLRW'");
    else if (instruction == CSMIDCODE) strcpy(str, "# CSM Instruction 'IDCODE'");
    else if (instruction == CSMANYUSER) strcpy(str, "# CSM Instruction 'ANYUSER'");
    else if (instruction == CSMTTCRXSTATUS) strcpy(str, "# CSM Instruction 'TTCRXSTATUS'");
    else if (instruction == CSMAMTPARITYERROR) strcpy(str, "# CSM Instruction 'AMTPARITYERROR'");
    else if (instruction == CSMCSMSTATUS) strcpy(str, "# CSM Instruction 'CSMSTATUS'");
    else if (instruction == CSMAMTPHASEERROR) strcpy(str, "# CSM Instruction 'AMTPHASEERROR'");
    else if (instruction == CSMCONFIGRO) strcpy(str, "# CSM Instruction 'CONFIGRO'");
    else if (instruction == CSMCONFIGRW) strcpy(str, "# CSM Instruction 'CONFIGRW'");
    else if (instruction == CSMCSMPARMRO) strcpy(str, "# CSM Instruction 'CSMCSMPARMRO'");
    else if (instruction == CSMCSMPARMRW) strcpy(str, "# CSM Instruction 'CSMCSMPARMRW'");
    else if (instruction == CSMVERSIONDATE) strcpy(str, "# CSM Instruction 'VERSIONDATE'");
    else if (instruction == CSMROBITS) strcpy(str, "# CSM Instruction 'ROBITS'");
    else if (instruction == CSMBYPASS) strcpy(str, "# CSM Instruction 'BYPASS'");
    else strcpy(str, "# CSM Unknown Instruction");
  }
/*  
  else if ((IDCode == AMT3ID) || (IDCode == AMT2ID)) {
    length = AMTINSTLENGTH;
    mezzInstr[mezzNumber] = instruction;
    if (instruction == AMTASDCONTROL) strcpy(JStr, "ASD");
    else strcpy(JStr, "AMT");
    if (instruction == AMTEXTEST)
      sprintf(str, "# AMT Instruction 'EXTEST' for mezz. card %d", mezzNumber);
    else if (instruction == AMTIDCODE)
      sprintf(str, "# AMT Instruction 'IDCODE' for mezz. card %d", mezzNumber);
    else if (instruction == AMTSAMPLE)
      sprintf(str, "# AMT Instruction 'SAMPLE' for mezz. card %d", mezzNumber);
    else if (instruction == AMTINTEST)
      sprintf(str, "# AMT Instruction 'INTEST' for mezz. card %d", mezzNumber);
    else if (instruction == AMTCONTROL)
      sprintf(str, "# AMT Instruction 'CONTROL' for mezz. card %d", mezzNumber);
    else if (instruction == AMTASDCONTROL)
      sprintf(str, "# AMT Instruction 'ASDCONTROL' for mezz. card %d", mezzNumber);
    else if (instruction == AMTSTATUS)
      sprintf(str, "# AMT Instruction 'STATUS' for mezz. card %d", mezzNumber);
    else if (instruction == AMTCORETEST)
      sprintf(str, "# AMT Instruction 'CORETEST' for mezz. card %d", mezzNumber);
    else if (instruction == AMTBIST)
      sprintf(str, "# AMT Instruction 'BIST' for mezz. card %d", mezzNumber);
    else if (instruction == AMTGOUTPORT)
      sprintf(str, "# AMT Instruction 'GOUTPORT' for mezz. card %d", mezzNumber);
    else if (instruction == AMTBYPASS)
      sprintf(str, "# AMT Instruction 'BYPASS' for mezz. card %d", mezzNumber);
    else
      sprintf(str, "# AMT Unknown Instruction for mezz. card %d", mezzNumber);
  }
*/
//Modified by Xiangting


   else if (IDCode == A3P250ID){
    length = A3P250INSTLENGTH;
    A3P250Instr[mezzNumber] = instruction;
    if (instruction == A3P250IDCODE)
      sprintf(str, "# A3P250 Instruction 'A3P250IDCODE' for mezz. card %d", mezzNumber);
    else if (instruction == A3P250ASD_WRITE)
      sprintf(str, "# A3P250 Instruction 'A3P250ASD_WRITE' for mezz. card %d", mezzNumber);
    else if (instruction == A3P250ASD_READ)
      sprintf(str, "# A3P250 Instruction 'A3P250ASD_READ' for mezz. card %d", mezzNumber);
    else if (instruction == A3P250FPGA_CW)
      sprintf(str, "# A3P250 Instruction 'A3P250FPGA_CW' for mezz. card %d", mezzNumber);
    else if (instruction == A3P250FPGA_CR)
      sprintf(str, "# A3P250 Instruction 'A3P250FPGA_CR' for mezz. card %d", mezzNumber);
    else if (instruction == A3P250FPGA_SR)
      sprintf(str, "# A3P250 Instruction 'A3P250FPGA_SR' for mezz. card %d", mezzNumber);
    else if (instruction == A3P250I2C_MS_W)
      sprintf(str, "# A3P250 Instruction 'A3P250I2C_MS_W' for mezz. card %d", mezzNumber);
    else if (instruction == A3P250I2C_MS_R)
      sprintf(str, "# A3P250 Instruction 'A3P250I2C_MS_R' for mezz. card %d", mezzNumber);
    else if (instruction == A3P250BYPASS)
      sprintf(str, "# A3P250 Instruction 'A3P250BYPASS' for mezz. card %d", mezzNumber);
    else
      sprintf(str, "# A3P250 Unknown Instruction for mezz. card %d", mezzNumber);
  }

  else if (IDCode == HPTDCID){
    length = HPTDCINSTLENGTH;
    HPTDCInstr[mezzNumber] = instruction;
    if (instruction == HPTDCEXTEST)
      sprintf(str, "# HPTDC Instruction 'HPTDCEXTEST' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCIDCODE)
      sprintf(str, "# HPTDC Instruction 'IDCODE' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCSAMPLE)
      sprintf(str, "# HPTDC Instruction 'SAMPLE' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCINTEST)
      sprintf(str, "# HPTDC Instruction 'INTEST' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCBIST)
      sprintf(str, "# HPTDC Instruction 'CONTROL' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCSCAN)
      sprintf(str, "# HPTDC Instruction 'ASDCONTROL' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCREADOUT)
      sprintf(str, "# HPTDC Instruction 'STATUS' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCSETUP)
      sprintf(str, "# HPTDC Instruction 'CORETEST' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCCONTROL)
      sprintf(str, "# HPTDC Instruction 'BIST' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCSTATUS)
      sprintf(str, "# HPTDC Instruction 'GOUTPORT' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCCORETEST)
      sprintf(str, "# HPTDC Instruction 'BYPASS' for mezz. card %d", mezzNumber);
    else if (instruction == HPTDCBYPASS)
      sprintf(str, "# HPTDC Instruction 'BYPASS' for mezz. card %d", mezzNumber);
    else
      sprintf(str, "# HPTDC Unknown Instruction for mezz. card %d", mezzNumber);
  }


//END  
  
  else {
    length = 1;
    strcpy(JStr, "UnknownDevice");
    printf("Unknown JTAG Device with IDCode = 0x%08X, something wrong!\n", IDCode);
  }
  newBitArrayList = TRUE;
  fprintf(actFile, "J %s", JStr);
  for (i = 30; i >= strlen(JStr); i--) fprintf(actFile, " ");
  fprintf(actFile, "# Instruction Length %d\n", length);
  fprintf(actFile, "I ");
  for (i = length-1; i >= 0; i--) fprintf(actFile, "%1d", (instruction>>i)&1);
  for (i = 30; i >= length; i--) fprintf(actFile, " ");
  fprintf(actFile, "%s\n", str);
}


void WriteJTAGDataToActionFile(int length, int offset)
{
  int i;
  char JStr[60];
  
  if (actFile == NULL) return;
  if (newBitArrayList) {
    newBitArrayList = FALSE;
    fprintf(actFile, "#\n");
    fprintf(actFile, "# ********* JTAG Bit Stream ***********\n");
  }
  newInstructionList = TRUE;
  if (offset == PROMOffset) strcpy(JStr, "PROM");
  else if (offset == vertexIIOffset) strcpy(JStr, "XC2V1000");
  else if (offset == AX1000Offset) strcpy(JStr, "AX1000");
  else if (offset == GOLOffset) strcpy(JStr, "GOL");
  else if (offset == TTCrxOffset) strcpy(JStr, "TTCrx");
  else if (offset == CSMOffset) strcpy(JStr, "CSM");
  else {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++) {

//Modified by Xiangting
/*	
      if (offset == mezzOffset[i]) {
        if (mezzInstr[i] == AMTASDCONTROL) strcpy(JStr, "ASD");
        else strcpy(JStr, "AMT");
        break;
      }
*/

	  if (offset == A3P250Offset[i]) strcpy(JStr, "A3P250");
	  else if (offset == HPTDCOffset[i]) strcpy(JStr, "HPTDC"); 
	  break;
//End
	  }
    if (i >= MAXNUMBERMEZZANINE) {
      strcpy(JStr, "UnknownDevice");
      printf("Unknown JTAG Device with length = %d offset = %d, something wrong!\n", length, offset);
    }
  }
  fprintf(actFile, "#\n");
  fprintf(actFile, "# Bit stream length is %d\n", length);
  fprintf(actFile, "#\n");
  fprintf(actFile, "J %s", JStr);
  for (i = 30; i >= strlen(JStr); i--) fprintf(actFile, " ");
  fprintf(actFile, "# Data Array Length %d\n", length);
  fprintf(actFile, "D ");
  for (i = offset+length-1; i >= offset; i--) fprintf(actFile, "%1d", dataArray[i]&1);
  fprintf(actFile, "\n");
  fprintf(actFile, "M ");
  for (i = offset+length-1; i >= offset; i--) {
    if (zeroAllMaskWord) fprintf(actFile, "%1d", 0);
    else fprintf(actFile, "%1d", maskArray[i]&1);
  }
  fprintf(actFile, "\n");
}


void OpenSequenceFile(char *filename)
{
  if (strlen(filename) <= 0) return;
  seqFile = fopen(filename, "w");
  if (seqFile == NULL) return;
  fprintf(seqFile, "# Sequence file\n"); 
  fprintf(seqFile, "# Generated by CSM JTAG Control on %s\n", WeekdayTimeString(time(NULL))); 
  fprintf(seqFile, "# List of actions. Each action refers to a file with name xxxxx.act\n"); 
  fprintf(seqFile, "# with command 'A xxxxx'\n"); 
  fprintf(actFile, "# Sequence file has following commands\n"); 
  fprintf(actFile, "#   S Server-number\n"); 
  fprintf(actFile, "#   C CSM-number ELMB-node-number\n"); 
  fprintf(actFile, "#   A action-file-name without file extension <.act>\n"); 
  fprintf(actFile, "#   O Digital-IO-bit-string (lowest bit = the right most bit\n"); 
  fprintf(actFile, "#   W Waiting-time-in-millionsecond\n"); 
  fprintf(seqFile, "#\n");
}


void CloseSequenceFile(void)
{
  if (seqFile == NULL) return;
  fprintf(seqFile, "#\n");
  fprintf(seqFile, "# end of sequence list\n");
  fclose(seqFile);
  seqFile = NULL;
}


void WriteCommentToSequenceFile(char *comment)
{
  if (seqFile == NULL) return;
  fprintf(seqFile, "# %s\n", comment);
}


void WriteActionToSequenceFile(char *actionFile)
{
  int csm, node, server;
  
  if (seqFile == NULL) return;
  fprintf(seqFile, "# -----------------------------------\n");
  for (csm = 0; csm <numberCSM; csm++) {
    server = 1;
    node = csm + 1;
    fprintf(seqFile, "#\n");
    fprintf(seqFile, "S %d\n", server);
    fprintf(seqFile, "C %d %d\n", csm, node);
    fprintf(seqFile, "A %s%04d\n", actionFile, csm);
    if (waitTimeInMillionSecond > 0) fprintf(seqFile, "W %d\n", waitTimeInMillionSecond);
  }
  fprintf(seqFile, "#\n");
}




void DownloadHPTDCControl(int control_number) {
  char path[256];
  int i, j,instr, length, l;
  FILE *SVFFile;
  FILE *file;
  printf("control_number=%d\n",control_number);
  if     (control_number == 1) {LoadHPTDCControlArray_step1();for(i=0;i<HPTDCCONTROL_LENGTH;i++) HPTDCControl_array[i]=basicControlArray_h1[i];}
  else if(control_number == 2) {LoadHPTDCControlArray_step2();for(i=0;i<HPTDCCONTROL_LENGTH;i++) HPTDCControl_array[i]=basicControlArray_h2[i];}
  else if(control_number == 3) {LoadHPTDCControlArray_step3();for(i=0;i<HPTDCCONTROL_LENGTH;i++) HPTDCControl_array[i]=basicControlArray_h3[i];}
  else if(control_number == 4) {LoadHPTDCControlArray_step4();for(i=0;i<HPTDCCONTROL_LENGTH;i++) HPTDCControl_array[i]=basicControlArray_h4[i];}
  else if(control_number == 5) {LoadHPTDCControlArray_step5();for(i=0;i<HPTDCCONTROL_LENGTH;i++) HPTDCControl_array[i]=basicControlArray_h5[i];}
  else if(control_number == 6) {LoadHPTDCControlArray_step6();for(i=0;i<HPTDCCONTROL_LENGTH;i++) HPTDCControl_array[i]=basicControlArray_h6[i];}

  if (file = fopen("ASD_setup_debug.txt", "a"));
  strcpy(path, "downloadHPTDCControl_step1.svf");
  if (SVFFile = fopen(path, "w")) {
    GetJTAGChainOnList();
    AllJTAGDeviceInBYPASS(SVFFile, 0);
    JTAGdownload_instr(instrArray, TDC_BYPASS, HPTDCCONTROL, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);  
    JTAGdownload_data(TDC_BYPASS, HPTDCCONTROL, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);

    if (action == DOWNLOAD) {
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
      CheckFullJTAGDataArray();
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if(i!=NEWTDC_NUMBER){
            if (mezzCardSetupAll) {
              if (downloadHPTDCStatus[i] == 0)
                printf("HPTDC control step%d is downloaded successfully through JTAG for mezzanine card %d.\n", control_number,i);
              else  
                printf("Failed in downloading HPTDC control step%d through JTAG for mezzanine card %d, downloadHPTDCStatus = %d\n", control_number,i, downloadHPTDCStatus[i]);
            }
            else if(mezzCardNb == i) {
              if (downloadHPTDCStatus[i] == 0)
                printf("HPTDC control step%d is downloaded successfully through JTAG for mezzanine card %d.\n",control_number,i);
              else  
                printf("Failed in downloading HPTDC control step%d through JTAG for mezzanine card %d, downloadHPTDCStatus = %d\n",control_number, i, downloadHPTDCStatus[i]);
            }
          }
        }
      }
    }
    AllJTAGDeviceInBYPASS(SVFFile, 1);
    fclose(SVFFile);
  }
  else {
    printf("Unable to open SVF file <%s>!\n", path);
    MessagePopup("Failed to Open SVF File",
                 "Unable to open SVF file, please check your disk and filename!");
  }
  fprintf(file,"Download HPTDCControl_%d finished!",control_number);
  //fprintSDR_TDI(file);
  //fprintSDR_TDO(file);
  //fprint_mask(file); 
  check_data(file);
  fclose(file);
}



void DownloadMDTTDCSetup(int instr, int step) {
  //char path[256];
  int i, j, length, l;
  if(instr == TDC_SETUP0_INSTR){
    length = LoadTDCSetup0Array();
    //length = TDC_SETUP0_LENGTH;
  }
  else if(instr == TDC_SETUP1_INSTR){
    length = LoadTDCSetup1Array();
    //length = TDC_SETUP1_LENGTH;
  }
  else if(instr == TDC_SETUP2_INSTR){
    length = LoadTDCSetup2Array();
    //length = TDC_SETUP2_LENGTH;
  }
  else if(instr == TDC_CONTROL0_INSTR){
	if(step == 1)	   LoadTDCControl0Array_step1();
	else if(step == 2) LoadTDCControl0Array_step2();
	else if(step == 3) LoadTDCControl0Array_step3();
    else length = LoadTDCControl0Array();
    length = TDC_CONTROL0_LENGTH;
  }
  else if(instr == TDC_CONTROL1_INSTR){
    length = LoadTDCControl1Array();
    //length = TDC_CONTROL1_LENGTH;
  }
  else if(instr == TDC_STATUS0_INSTR){
	IntToReversedBinary(0,0,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
	IntToReversedBinary(0,1,32,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
    length = TDC_STATUS0_LENGTH;
  }
  else if(instr == TDC_STATUS1_INSTR){
	IntToReversedBinary(0,0,25,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
    length = TDC_STATUS1_LENGTH;
  }
  else{
    printf("Invalid INSTR for TDC!\n");
    return;
  }
  TDC_setup_length = length;
  JTAGdownload_instr(instrArray, instr, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);  
  JTAGdownload_data(instr, HPTDCBYPASS, A3P250BYPASS, CSMBYPASS, TTCRXBYPASS, GOLBYPASS, AX1000BYPASS, VERTEXIIBYPASS, PROMBYPASS);
  
    if (action == DOWNLOAD) {
		
      JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      JTAGScanAllData(dataLength, dataArray, readbackArray);
	  if(instr != TDC_STATUS0_INSTR && instr != TDC_STATUS1_INSTR){
        JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
        JTAGScanAllData(dataLength, dataArray, readbackArray);
        CheckFullJTAGDataArray();  
	  }		
	  //for(i=0;i<dataLength;i++) dataArray[i] = 1;
	  //JTAGScanAllInstruction(instrLength, instrArray, readbackArray);
      //JTAGScanAllData(dataLength, dataArray, readbackArray);
    printf("Download TDC Setup is ongoing\n");
    printf("Download TDC Setup is ongoing\n"); 
      
      
      for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
        if (CSMSetupArray[TDCENABLES+i] == 1) {
          if(i==NEWTDC_NUMBER){
            if (mezzCardSetupAll) {
              if (downloadHPTDCStatus[i] == 0)
                printf("TDC setup is downloaded successfully through JTAG for mezzanine card %d.\n", i);
              else  
                printf("Failed in downloading TDC setup through JTAG for mezzanine card %d, downloadHPTDCStatus = %d\n", i, downloadHPTDCStatus[i]);
            }
            else if(mezzCardNb == i) {
              if (downloadHPTDCStatus[i] == 0)
                printf("TDC setup is downloaded successfully through JTAG for mezzanine card %d.\n", i);
              else  
                printf("Failed in downloading TDC setup through JTAG for mezzanine card %d, downloadHPTDCStatus = %d\n", i, downloadHPTDCStatus[i]);
            }
          }
        }
      }
    }
	//JTAGChainMezzCards();
	
	UpdateJTAGControlPanel();

    //AllJTAGDeviceInBYPASS(SVFFile, 1);
    //fclose(SVFFile);
}




void JTAGdownload_instr(int* array, int TDC_instr, int HPTDC_instr, int A3P250_instr, int CSM_instr, int TTCrx_instr, int GOL_instr, int AX1000_instr, int VERTEX_instr, int PROM_instr){
  int i;
  GetJTAGChainOnList();
  instrLength = 0;
  if (mezzCardsOn) {
    for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
      if (CSMSetupArray[TDCENABLES+i] == 1) {
        if(i==NEWTDC_NUMBER){
          IntToBinary(TDC_instr, instrLength, TDC_INSTR_LENGTH, array, JTAGINSTRLENGTH);
          instrLength += TDC_INSTR_LENGTH;
          //IntToBinary(A3P250BYPASS, instrLength, A3P250INSTLENGTH, array, JTAGINSTRLENGTH);
          //instrLength += A3P250INSTLENGTH;
        }
        else{
          IntToBinary(HPTDC_instr, instrLength, HPTDCINSTLENGTH, array, JTAGINSTRLENGTH);
          instrLength += HPTDCINSTLENGTH;
          IntToBinary(A3P250_instr, instrLength, A3P250INSTLENGTH, array, JTAGINSTRLENGTH);
          instrLength += A3P250INSTLENGTH;
        }
      }
    }
  }
  if (CSMOn) {
    IntToBinary(CSM_instr, instrLength, CSMINSTLENGTH, array, JTAGINSTRLENGTH);
    instrLength += CSMINSTLENGTH;  
  }
  if (TTCrxOn) {
    IntToBinary(TTCrx_instr, instrLength, TTCRXINSTLENGTH, array, JTAGINSTRLENGTH);
    instrLength += TTCRXINSTLENGTH;     
  }
  if (GOLOn) {
    IntToBinary(GOL_instr, instrLength, GOLINSTLENGTH, array, JTAGINSTRLENGTH);
    instrLength += GOLINSTLENGTH;   
  }
  if (AX1000On) {
    IntToBinary(AX1000_instr, instrLength, AX1000INSTLENGTH, array, JTAGINSTRLENGTH);
    instrLength += AX1000INSTLENGTH;    
  }
  if (XC2V1000On || XC2V2000On) {
    IntToBinary(VERTEX_instr, instrLength, VERTEXIIINSTLENGTH, array, JTAGINSTRLENGTH);
    instrLength += VERTEXIIINSTLENGTH;      
  }
  if (PROMOn) {
    IntToBinary(PROM_instr, instrLength, PROMINSTLENGTH, array, JTAGINSTRLENGTH);
    instrLength += PROMINSTLENGTH;     
  }
}


void JTAGdownload_data(int TDC_instr, 
                       int HPTDC_instr, 
                       int A3P250_instr, 
                       int CSM_instr, int TTCrx_instr, int GOL_instr, int AX1000_instr, int VERTEX_instr, int PROM_instr){
  dataLength = 0;

  JTAGdownload_data_Mezz(TDC_instr,
                         HPTDC_instr,
                         A3P250_instr);
  JTAGdownload_data_CSM(CSM_instr);
  JTAGdownload_data_TTCrx(TTCrx_instr);
  JTAGdownload_data_GOL(GOL_instr);
  JTAGdownload_data_CSM_chip(AX1000_instr, VERTEX_instr);
  JTAGdownload_data_PROM(PROM_instr);
}


//This is for the mezz card with TDCV1 and A3P250
// void JTAGdownload_data_Mezz(int TDC_instr, int HPTDC_instr, int A3P250_instr){
//   int length;
//   int i,j,l;
//   if (mezzCardsOn) {
//     if(HPTDC_instr==HPTDCBYPASS && A3P250_instr==A3P250BYPASS &&TDC_instr==TDC_BYPASS){
//       for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
//         if (CSMSetupArray[TDCENABLES+i] == 1){
//           HPTDCOffset[i] = dataLength;
//           dataArray[dataLength] = 0;
//           maskArray[dataLength++] = 0;
//           A3P250Offset[i] = dataLength; 
//           dataArray[dataLength] = 0;
//           maskArray[dataLength++] = 0;
//         }
//       }
//     }
//     else if(HPTDC_instr==HPTDCIDCODE && A3P250_instr==A3P250IDCODE){
//       for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
//         if (CSMSetupArray[TDCENABLES+i] == 1) {
//           HPTDCOffset[i] = dataLength;
//           IntToBinary(0, dataLength, 32, dataArray, MAXJTAGARRAY);
//           for (j = 0; j < 32; j++) maskArray[dataLength++] = 1;
//           A3P250Offset[i] = dataLength;
//           IntToBinary(0, dataLength, 32, dataArray, MAXJTAGARRAY);
//           for (j = 0; j < 32; j++) maskArray[dataLength++] = 1;
// 		}
// 	  }
//     }
//     else if(A3P250_instr==A3P250ASD_WRITE){    
//       for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
//         if (CSMSetupArray[TDCENABLES+i] == 1){
//           if(i==NEWASD_NUMBER)
//             length = LoadA3P250SetupArray_new();
//           else 
//             length = LoadA3P250SetupArray_old();
          
//           if(i==NEWTDC_NUMBER){
//             HPTDCOffset[i] = dataLength;
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//             for (l = length-1; l>=0; l--) {
//               dataArray[dataLength] = basicSetupArray_a3p250[l];
//               maskArray[dataLength++] = 1;
//             }
//             A3P250Offset[i] = dataLength; 
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//           }
//           else{
//             HPTDCOffset[i] = dataLength;
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//             A3P250Offset[i] = dataLength; 
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//             for (l = length-1; l>=0; l--) {
//               dataArray[dataLength] = basicSetupArray_a3p250[l];
//               maskArray[dataLength++] = 1;
              
//             }
//           }
//         }
//       }
//     }
// 	else if(HPTDC_instr==HPTDCBYPASS && A3P250_instr==A3P250BYPASS && TDC_instr!=TDC_BYPASS){
// 	  for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
//         if (CSMSetupArray[TDCENABLES+i] == 1){
//           if(i==NEWTDC_NUMBER){
//             length = TDC_setup_length; 
//             HPTDCOffset[i] = dataLength;        
//             for (l = 0; l < length; l++) {
//               if (length == TDC_setup_length) {
//               dataArray[dataLength] = TDC_setup_array[l];
//               maskArray[dataLength++] = 1;
//               }
//               else {
//               dataArray[dataLength] = 1;
//               maskArray[dataLength++] = 0;
//               }
//             }        
//             A3P250Offset[i] = dataLength;
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//           }
//           else{
//             HPTDCOffset[i] = dataLength;        
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//             A3P250Offset[i] = dataLength; 
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//           }
//         }
//       }
// 	}
//     else if(HPTDC_instr==HPTDCSETUP && A3P250_instr==A3P250BYPASS && TDC_instr==TDC_BYPASS){
//       for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
//         if (CSMSetupArray[TDCENABLES+i] == 1){
//           if(i==NEWTDC_NUMBER){
//             HPTDCOffset[i] = dataLength;
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//             A3P250Offset[i] = dataLength; 
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//           }
//           else{
//             length = HPTDCSETUP_LENGTH; 
//             HPTDCOffset[i] = dataLength;        
//             for (l = 0; l < length; l++) {
//               if (length == HPTDCSETUP_LENGTH) {
//               dataArray[dataLength] = basicSetupArray_h[l];
//               maskArray[dataLength++] = 1;
//               }
//               else {
//               dataArray[dataLength] = 1;
//               maskArray[dataLength++] = 0;
//               }
//             }        
//             A3P250Offset[i] = dataLength;
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//           }
//         }
//       }
//     }
//     else if(HPTDC_instr==HPTDCCONTROL && A3P250_instr==A3P250BYPASS&& TDC_instr==TDC_BYPASS){
//       for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
//         if (CSMSetupArray[TDCENABLES+i] == 1){
//           if(i==NEWTDC_NUMBER){
//             HPTDCOffset[i] = dataLength;
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//             A3P250Offset[i] = dataLength; 
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//           }
//           else{
//             length = HPTDCCONTROL_LENGTH; 
//             HPTDCOffset[i] = dataLength;        
//             for (l = 0; l < length; l++) {
//               if (length == HPTDCCONTROL_LENGTH) {
//               dataArray[dataLength] = HPTDCControl_array[l];
//               maskArray[dataLength++] = 1;
//               }
//               else {
//               dataArray[dataLength] = 1;
//               maskArray[dataLength++] = 0;
//               }
//             }
//             A3P250Offset[i] = dataLength;
//             dataArray[dataLength] = 0;
//             maskArray[dataLength++] = 0;
//           }
//         }
//       }
//     }
//   }
// }

//This is for the mezz card with TDCV2
void JTAGdownload_data_Mezz(int TDC_instr,int HPTDC_instr,int A3P250_instr){
  int i;
  if (mezzCardsOn) {
    for (i = MAXNUMBERMEZZANINE-1; i >= 0; i--) {
      if (CSMSetupArray[TDCENABLES+i] == 1){
        if(i==NEWTDC_NUMBER){
          HPTDCOffset[i] = dataLength;
          write_TDC_data_array(TDC_instr);
        }
        else{
          HPTDCOffset[i] = dataLength;
          write_HPTDC_data_array(HPTDC_instr);
          A3P250Offset[i] = dataLength;
          write_A3P250_data_array(A3P250_instr);
        }
      }
    }
  }
}

void write_TDC_data_array(int TDC_instr){
  int i;
  if(TDC_instr==TDC_BYPASS){    
    dataArray[dataLength] = 0;
    maskArray[dataLength++] = 0;
  }
  else if(TDC_instr==TDC_IDCODE){
    IntToBinary(0, dataLength, 32, dataArray, MAXJTAGARRAY);
    for (i = 0; i < 32; i++) maskArray[dataLength++] = 0;
  }
  else if(TDC_instr==TDC_ASD_CONFIG_INSTR){ 
    ASD_length = LoadA3P250SetupArray_new();
    dataArray[dataLength] = 0;  //Add 1 bit for the known ASD bug
    maskArray[dataLength++] = 0;
	dataArray[dataLength] = 0;  //Add 1 bit for the known ASD bug
    maskArray[dataLength++] = 0;
    for (i = ASD_length-1; i >=0; i--) { //note: A3P250 array reversed
      dataArray[dataLength] = basicSetupArray_a3p250[i];
      maskArray[dataLength++] = 1;
    }
  }
  else{  //do same thing for every known TDC instr
    switch(TDC_instr){
      case  TDC_SETUP0_INSTR      :;
      case  TDC_SETUP1_INSTR      :;  
      case  TDC_SETUP2_INSTR      :;
      case  TDC_CONTROL0_INSTR    :;
      case  TDC_CONTROL1_INSTR    :;
      case  TDC_STATUS0_INSTR     :;
      case  TDC_STATUS1_INSTR     :
        for (i = TDC_setup_length-1; i >=0; i--) {
		//  for (i = 0; i <TDC_setup_length; i++) {  
          dataArray[dataLength] = TDC_setup_array[i];  //note: TDC array reversed
          maskArray[dataLength++] = 1; 
        }
          break;
      default: printf("Warning! Unknown TDC JTAG INSTRA 0X%02X",TDC_instr);
    } //switch
  }//else
}


void write_HPTDC_data_array(int HPTDC_instr){
  int i;
  if(HPTDC_instr==HPTDCBYPASS){    
    dataArray[dataLength] = 0;
    maskArray[dataLength++] = 0;
  }
  else if(HPTDC_instr==HPTDCIDCODE){
    IntToBinary(0, dataLength, 32, dataArray, MAXJTAGARRAY);
    for (i = 0; i < 32; i++) maskArray[dataLength++] = 0;
  }
  else if(HPTDC_instr==HPTDCSETUP){      
    for (i = 0; i < HPTDCSETUP_LENGTH; i++) {
      dataArray[dataLength] = basicSetupArray_h[i];
      maskArray[dataLength++] = 1;
    }
  }
  else if(HPTDC_instr==HPTDCCONTROL){ 
    for (i = 0; i < HPTDCCONTROL_LENGTH; i++) {
      dataArray[dataLength] = HPTDCControl_array[i];
      maskArray[dataLength++] = 1;
    }
  }
  else{
    printf("Warning! Unknown HPTDC JTAG INSTRA 0X%02X",HPTDC_instr);
  }
}

void write_A3P250_data_array(int A3P250_instr){
  int i;
  if(A3P250_instr==A3P250BYPASS){    
    dataArray[dataLength] = 0;
    maskArray[dataLength++] = 0;
  }
  else if(A3P250_instr==A3P250IDCODE){
    IntToBinary(0, dataLength, 32, dataArray, MAXJTAGARRAY);
    for (i = 0; i < 32; i++) maskArray[dataLength++] = 0;
  }
  else if(A3P250_instr==A3P250ASD_WRITE||A3P250_instr==A3P250ASD_READ){    
    ASD_length = LoadA3P250SetupArray_old();  
    dataArray[dataLength] = 0; //Add 1 bit for the known ASD bug
    maskArray[dataLength++] = 0; 
    for (i = ASD_length-1; i >=0; i--) { //note: A3P250 array reversed
      dataArray[dataLength] = basicSetupArray_a3p250[i];
      maskArray[dataLength++] = 1;
    }
  }
  else{
    printf("Warning! Unknown A3P250 JTAG INSTRA 0X%02X",A3P250_instr);
  }
}
    



void JTAGdownload_data_CSM(int CSM_instr){
  int i;
  if (CSMOn) {
    if(CSM_instr==CSMBYPASS){
      CSMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
    }
    else if(CSM_instr==CSMFULLRW){
      CSMOffset = dataLength;
      for (i = dataLength; i < dataLength+CSMSTATUSLENGTH; i++) {
        dataArray[i] = CSMStatusArray[i-dataLength];
        maskArray[i] = 0;
      }
      dataLength += CSMSTATUSLENGTH;
      for (i = dataLength; i < dataLength+CSMSETUPLENGTH; i++) {
        if ((action == DOWNLOAD) && !downloadCSMSetupDone) {
          dataArray[i] = CSMSetupArray[i-dataLength];
          // maskArray[i] = 1;     //for GetCSMStatus();
          if (i == (dataLength+CSMCMD)) maskArray[i] = 0;
          else maskArray[i] = 1;
        }
        else {
          dataArray[i] = downloadedCSMSetupArray[i-dataLength];
          maskArray[i] = 1;
        }
      }
      dataLength += CSMSETUPLENGTH;
      for (i = dataLength; i < dataLength+TTCRXSETUPLENGTH; i++) {
        dataArray[i] = TTCrxSetupArray[i-dataLength];
        maskArray[i] = 1;
      }
      dataLength += TTCRXSETUPLENGTH;
    }
    else if(CSM_instr==CSMFULLRO){
      CSMOffset = dataLength;
        for (i = dataLength; i < dataLength+CSMSTATUSLENGTH; i++) {
          dataArray[i] = CSMStatusArray[i-dataLength];
          if (i == dataLength) {
            dataArray[i] = 1;
            maskArray[i] = 0;
          }
          else if (i < dataLength+12) {
            dataArray[i] = 0;
            maskArray[i] = 0;
          }
          else maskArray[i] = 0;
        }
        dataLength += CSMSTATUSLENGTH;
        for (i = dataLength; i < dataLength+CSMSETUPLENGTH; i++) {
          dataArray[i] = CSMSetupArray[i-dataLength];
          maskArray[i] = 1;
          downloadedCSMSetupArray[i-dataLength] = dataArray[i];
        }
        dataLength += CSMSETUPLENGTH;
        for (i = dataLength; i < dataLength+TTCRXSETUPLENGTH; i++) {
          dataArray[i] = TTCrxSetupArray[i-dataLength];
          maskArray[i] = 1;
        }
        dataLength += TTCRXSETUPLENGTH;

    }
    else if(CSM_instr==CSMIDCODE){
      CSMOffset = dataLength;
      IntToBinary(CSMID, dataLength, 32, dataArray, MAXJTAGARRAY);
      for (i = 0; i < 32; i++) maskArray[dataLength++] = 1;
    }
	//printf("CSMOffset = %d\n",CSMOffset);
  }
}


void JTAGdownload_data_TTCrx(int TTCrx_instr){
  int i; 
  if (TTCrxOn) {
    if(TTCrx_instr==TTCRXBYPASS){
      TTCrxOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
    }
    else if(TTCrx_instr==TTCRXIDCODE){
      TTCrxOffset = dataLength;
      IntToBinary(TTCRXID, dataLength, 32, dataArray, MAXJTAGARRAY);
      for (i = 0; i < 32; i++) maskArray[dataLength++] = 1;
    }
	//printf("TTCrxOffset = %d\n",TTCrxOffset);  
  }
}


void JTAGdownload_data_GOL(int GOL_instr){
  int i; 
  if (GOLOn) {
    if(GOL_instr==GOLBYPASS){
      GOLOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
    }
    else if(GOL_instr==GOLCONFRO){
      GOLOffset = dataLength;
      for (i = dataLength; i < dataLength+GOLSETUPLENGTH; i++) {
        dataArray[i] = GOLSetupArray[i-dataLength];
        //maskArray[i] = 1;
        if (i < dataLength+WAITTIMEFORREADY) maskArray[i] = 0;
        else if (i < dataLength+HAMMINGCHECKSUM) maskArray[i] = 1;
        else maskArray[i] = 0;
      }
      dataLength += GOLSETUPLENGTH;
    }
    else if(GOL_instr==GOLEXTEST){
      GOLOffset = dataLength;
      for (i = dataLength; i < dataLength+GOLBSCANLENGTH; i++) {
        dataArray[i] = GOLBScanArray[i-dataLength];
        maskArray[i] = 1;
      }
      dataLength += GOLBSCANLENGTH;
    }
    else if(GOL_instr==GOLDEVICEID){
      GOLOffset = dataLength;
      IntToBinary(GOLID, dataLength, 32, dataArray, MAXJTAGARRAY);
      for (i = 0; i < 32; i++) maskArray[dataLength++] = 1;
    }
	//printf("GOLOffset = %d\n",GOLOffset);
  }
}

void JTAGdownload_data_CSM_chip(int AX1000_instr, int VERTEX_instr){
  int i; 
  if (AX1000On) {
	if(AX1000_instr==AX1000BYPASS){
	  AX1000Offset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
	}
    else if(AX1000_instr==AX1000IDCODE){
      AX1000Offset = dataLength;
      IntToBinary(AX1000ID, dataLength, 32, dataArray, MAXJTAGARRAY);
      for (i = 0; i < 32; i++) maskArray[dataLength++] = 1;
    }
	//printf("AX1000Offset = %d\n",AX1000Offset);
  }
  if (XC2V1000On || XC2V2000On) {
	if(VERTEX_instr==VERTEXIIBYPASS){
	  vertexIIOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
	}
    else if(VERTEX_instr==VERTEXIIIDCODE){
      vertexIIOffset = dataLength;
      if (XC2V2000On) IntToBinary(XC2V2000ID, dataLength, 32, dataArray, MAXJTAGARRAY);
      else IntToBinary(XC2V1000ID, dataLength, 32, dataArray, MAXJTAGARRAY);
      for (i = 0; i < 32; i++) maskArray[dataLength++] = 1;
    }
	// printf("vertexIIOffset = %d\n",vertexIIOffset);
  }
}


void JTAGdownload_data_PROM(int PROM_instr){
  int i; 
  if (PROMOn) {
	if(PROM_instr==PROMBYPASS){
      PROMOffset = dataLength;
      dataArray[dataLength] = 1;
      maskArray[dataLength++] = 0;
	}
    else if(PROM_instr==PROMIDCODE){
      PROMOffset = dataLength;
      IntToBinary(PROMID, dataLength, 32, dataArray, MAXJTAGARRAY);
      for (i = 0; i < 32; i++) maskArray[dataLength++] = 1;
    }
	// printf("PROMOffset = %d\n",PROMOffset);
  }
}



