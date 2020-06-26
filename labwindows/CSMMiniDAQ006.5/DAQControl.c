// DAQControl.c version 5.x                         by T.S.Dai
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
#include <analysis.h>
#include "interface.h"
#include "DAQGUI.h"
#include "csmSetup.h"
#include "ttcrxSetup.h"
#include "golSetup.h"
#include "mezzSetup.h"
#include "jtagDriver.h"
#include "jtagControl.h"
#include "DAQControl.h"
#include "TCPServer.h"

#define MDTChamberOutput1(x) printf(x); if (MDTChamberSumFile) fprintf(MDTChamberSumFile, x); if (MDTChamberWarningFile) fprintf(MDTChamberWarningFile, x)
#define MDTChamberOutput2(x, y) printf(x, y); if (MDTChamberSumFile) fprintf(MDTChamberSumFile, x, y); if (MDTChamberWarningFile) fprintf(MDTChamberWarningFile, x, y)
#define MDTChamberFileOutput1(file, x) if (file != NULL) fprintf(file, x); if (MDTChamberSumFile) fprintf(MDTChamberSumFile, x)
#define MDTChamberFileOutput2(file, x, y) if (file != NULL) fprintf(file, x, y); if (MDTChamberSumFile) fprintf(MDTChamberSumFile, x, y)
// DAQ information controls and variables
#define INFORLENGTH         3600
#define RECENTINFORLENGTH   1200
int displayDAQInfor, DAQInforPrecision = 2;
unsigned int initDAQInfor = 0, DAQTime, inforIndex, inforTimeStep, recentInforIndex;
unsigned int numberEvent[INFORLENGTH], eventSizeHigh[INFORLENGTH], eventSizeLow[INFORLENGTH];
unsigned int recentNumberEvent[RECENTINFORLENGTH], recentEventSize[RECENTINFORLENGTH];
unsigned int numberTrigger[INFORLENGTH], recentNumberTrigger[RECENTINFORLENGTH];
unsigned int totalNumberTrigger, totalNumberTriggerHigh, ntrigger;
double currentDAQRate, averageDAQRate, minimumDAQRate, maximumDAQRate, totalTriggerTime;
double currentTriggerRate, averageTriggerRate, minimumTriggerRate, maximumTriggerRate;
double currentEventSize, averageEventSize, minimumEventSize, maximumEventSize;
double currentDataRate, averageDataRate, minimumDataRate, maximumDataRate;
double measuringTime = 2.0, plotFactorForEventSize = 1.0;
double plotMaxDAQRate, plotMaxEventSize, plotMaxDataRate;
unsigned int plotMinTime = 0, plotMaxTime = 600;
static time_t DAQStartTime, VFPStartTime;
double maxUintValue = 4294967295.0;

int autoRunFromControlFile = 0, prescaleControl = 0, scanOutputFormat = 0, triggerRate = -1;
int triggerSelect = -1, AMTCommandDelay = -1, calibrationTriggerDelay = -1, storageCtrl;
char currSetupFile[30] = "currentDAQProfiles.sav";
char parameterFile[30] = "DAQControlpara.txt";
int defaultProfileSaved = 0, canUseVME = TRUE, gotJTAGDriver = TRUE;
extern int TrigSelectHandle, BGOControlHandle, measureAllTriggerRate, TTCStatusAndControlHandle;
extern int TTCviPanelAndButton(void);
extern int InitTTCvi(void);
extern void TTCviEventCounterReset(void);
extern void TTCviBunchCounterReset(void);
extern void TTCviEventAndBunchCounterReset(void);
extern void TTCviReadEventNumber(void);
extern void TTCviSetUnusedTrigger(void);
extern void TTCviRestoreTrigger(void);
extern void TTCviSaveTriggerSettings(void);
extern void TTCviRestoreTriggerSettings(void);
extern void TTCviCalibrationRun(void);
extern void TTCviNormalDAQRun(void);
extern void MeasureVariesTriggerRate(void);
extern void SetTTCviTriggerAndRate(int triggerSelect, int *triggerRate);
extern int GetTTCviTriggerSelction(void);
extern int GetTTCviTriggerRateSelction(void);
extern float MeasuredTTCviTriggerRate(int *numberTrigger);
extern void UpTTCviTriggerSelect(void);
extern void TTCviOneShotTriggerIfSelected(void);
extern void SaveMeasuredTriggerRate(float rate);
extern float ExpectedTriggerRate(void);
extern float GetMeasuredTriggerRate(int trigSel0, int trigRate0);
extern int InternalSoftwareTrigger(void);
extern void WriteTTCviBGOControlRegisters(void);
extern int SendTTCviTCPCommand(int command, int addr, int data);

int main(int argc, char *argv[]) {
  int resp, mezz, triggerRate;
  
  gotTTCvi = FALSE;
  gotJTAGDriver = FALSE;
  useExternalTrigger = FALSE;
  measureTriggerRate = FALSE;
  initDAQDoneForRun = FALSE;
  rundownCurrent = -1;
  integrationGate = -1;
  widthSelection = 0;
  widthBin = (25.0/32.0) * (1 << widthSelection);
  requestStopByClient = FALSE;
  readNominalThresholdOnly = FALSE;
  verifyFPGAAndPROM = FALSE;
  requestStartOrStop = FALSE;
  restartRunAfterStop = FALSE;
  requestBackNormal = FALSE;
  checkAllDAQHardware = FALSE;
  checkCSM = 0;
  checkCSMJTAG = 0;
  checkCSMIO = 0;
  chamberIndex = -1;
  nChamberEntry = 0;
  MDTChamberControl = FALSE;
  MDTChamberNumberMezz = 0;
  MDTChamberNumberRow = 0;
  MDTChamberTest = FALSE;
  MDTChamberType = CTYPEUNKNOWN;
  strcpy(MDTChamberSumFilename, "");
  MDTChamberSumFile = NULL;
  strcpy(MDTChamberWarningFilename, "");
  MDTChamberWarningFile = NULL;
  thresholdScanMethod = 0;
  numberSubScans = 1;
  subScans = numberSubScans - 1;
  willDoThresholdScan = FALSE;
  runDiagnostics = FALSE;
  checkMezzOnOff = FALSE;
  verifyFPGATime =  60;
  verifyPROMTime = 600;
  radTestLogFile = NULL;
  numberCSMDownload = 0;
  numberBadCSMDownload = 0;
  numberMezzDownload = 0;
  numberBadMezzDownload = 0;
  numberInitDAQ = 0;
  numberBadInitDAQ = 0;
  numberReInitDAQ = 0;
  numberAutoRun = 0;
  numberBadAutoRun = 0;
  numberAutoRerun = 0;
  reqRerun = FALSE;
  numberRerun = 0;
  gotMezzDatabase = FALSE;
  largeVOffSpan = FALSE;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) getVOff[mezz] = FALSE;
  reqProcessData = FALSE;
  allowedFractionBadCycles = 0.0;
  strcpy(MDTChamberName, "");
  clientRunState = State_Idle;
  clientAnaInstalled = FALSE;
  clientCERNLibInstalled = FALSE;
  clientProcessingData = FALSE;
#ifdef TTCVIDRIVERVIATCP
  canUseVME = TRUE;
  if (JTAGDRIVERTYPE != CANELMB) {
    canUseVME = FALSE;
    MessagePopup("Not Supported", 
                 "JTAG driver is not ELMB via CAN\n"
                 "Control TTCvi via TCP/IP is supported only JTAG is drived by ELMB via CAN.\n");
    return -1;
  }
#else
  // Needed if linking in external compiler; harmless otherwise out of memory
  if (InitCVIRTE(0, argv, 0) == 0) return -1;
  if (InitVXIlibrary() < 0) {
    canUseVME = FALSE;
    resp = ConfirmPopup("VXILib. Init. Error", 
                        "VXI library initialization failed, no way to use VME right now.\n\n"
                        "Please check your hardwares.\n"
                        "1) Is power on?\n"
                        "2) Is VXI-MXI-2 installed in slot 0?\n"
                        "3) Have your run the RESMAN (Resource Manager)?\n"
                        "4) ...\n"
                        "\n\n"
                        "Do you wish to continue?\n"
                        "If yes, the program only can generate text files for JTAG.\n");
    CloseVXIlibrary();
    if (resp == 0) {
      CloseVXIlibrary();
      return -1;
    }
  }
#endif
  SetSleepPolicy(VAL_SLEEP_SOME);
  
  ActivateTCPServer();
  // OK, load the panels now.
  InitUserInterface();
  if (JTAGControlPanelAndButton() < 0) return -1;
  if (CSMIOsPanelAndButton() < 0) return -1;
  if (DAQControlButton() < 0) return -1;
  if (CSMSetupPanelAndButton() < 0) return -1;
  if (TTCrxSetupPanelAndButton() < 0) return -1;
  if (GOLSetupPanelAndButton() < 0) return -1;
  if (MezzCardSetupPanelAndButton() < 0) return -1;
  if (TTCviPanelAndButton() < 0) return -1;
  if (AutoRunSetupPanelAndButton() <0) return -1;
  if (RunConditionPanelAndButton() <0) return -1;
  
  controlOption = NORM;
  runState = State_Idle;
  validBCIDPresetValue = 0;
  enableAnalysis = 1;
  toTCPClients = TRUE;
  runState = State_Idle;
  autoRunOn = FALSE;
  DisplayPanel(TitleHandle);
  Delay(2);
  HidePanel(TitleHandle);
  GetIPConfiguration();
  Delay(1);
  if (JTAGDRIVERTYPE == XILNIXPCABLE) gotJTAGDriver = TRUE;
  else if (JTAGDRIVERTYPE == CANELMB) {
    CANNode = -1;
    ELMBNode = -1;
    gotJTAGDriver = TRUE;
  }
  else if (canUseVME) gotJTAGDriver = TRUE;
  ReadParameters(PARAALL);
  if (nominalThreshold == 0) {
    nominalThreshold = -40;
    SetCtrlVal(RunConditionHandle, P_RUNCOND_NTHRESHOLD, nominalThreshold);
    SaveParameters(PARANOMINALTHRESHOLD);
  }
  if (gotJTAGDriver) {
    printf("JTAG Server          : %10s\n", JTAGServerName);
    printf("CAN Node ID          : %10d\n", CANNode);
    printf("ELMB Node ID         : %10d\n", ELMBNode);
    HandleUserButtons(PanelDone);
    FindJTAGControllerBase();
    JTAGControlStartUp();
    InitJTAG();
    SetJTAGRateDivisorButton();
    AutoSetupJTAGChain();
    /*
    SetupJTAGChain();
    GetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, ATTR_MAX_VALUE, &maxDivider);
    GetCtrlVal(DAQControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, &currDivider);
    divider = currDivider;
    while (gotUnknown && (divider < maxDivider)) {
      divider++;
      SetCtrlVal(DAQControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, divider);
      SetJTAGRateDivisorButton();
      SetupJTAGChain();
      if (!gotUnknown) currDivider = divider;
    }
    SetCtrlVal(DAQControlHandle, P_JTAGCTRL_JTAGRATEDIVISOR, currDivider);
    SetJTAGRateDivisorButton();
    ResetTAP();
    */
  }
  if (canUseVME) {
    if (!InitTTCvi()) gotTTCvi = FALSE;
    else {
      gotTTCvi = TRUE;
      measureTriggerRate = TRUE;
      // set 5KHz random trigger as default trigger
      triggerRate = 5000;
      SetTTCviTriggerAndRate(-1, &triggerRate);
      TTCviSetUnusedTrigger();
    }
  }
  else {
    gotTTCvi = FALSE;
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, TRUE);
  }
  DisplayPanel(JTAGControlHandle);
  JTAGControlStartUp();
  if (gotCSM) CSMJTAGOperation(CSMVERSIONDATE, FALSE);
  DAQControlStartUp();
  LoadSavedProfile(defaultProfileSaved);
  LoopAndExecuteUserButtonAndFunction(Quit);
  if (runState != State_Idle) StopDAQ();  // Stop run if running/paused
  SendTTCviTCPCommand(CMDQUIT, 0, 0);
  SendTCPCommand(-1, CMDQUIT);
  SaveParameters(PARAALL);
  DeactivateTCPServer();
#ifndef TTCVIDRIVERVIATCP
  if (canUseVME) CloseVXIlibrary();
#endif
  
  return 0;
}


// User function
void UserFunction(void) {
  static int runStarted = FALSE, GOLANumber, status, numberTrigger, second, minute, hour, dimmed;
  static int nnodes, ntry, nEvent, checkingFPGAAndPROM = FALSE, nError, CSMError, dataError, dtime0;
  static int unknownTDCWords, oldTDCWords, unknownDataWords, oldDataWords, dataTime0;
  static float rate, oldRate = -1.0, time0, ttime0, triggerTime, dtime, FPGATime0, PROMTime0;
  static float dttime0, dtriggerTime, dtriggerDeadtime, fracDeadtime;
  static int oldWrongEvent = 0, PROMError, FPGAError, JTAGFailed, CSMSetupError, CSMStatusError;
  static unsigned int oldRunNumber = 0, drtime0, saveLogTime0, saveCSMVersion, checkJTAGCount;
  int version, j, checkMode;
  char string[20], str[256], *strp;
  
  UpdateJTAGControlPanelInfor();
#ifdef TTCVIDRIVERVIATCP
  if (!TTCviTCPConnected) {
    gotTTCvi = FALSE;
    measureTriggerRate = FALSE;
    dimmed = TRUE;
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_STATUSANDCONTROL, ATTR_DIMMED, dimmed);
  }
#endif
  if (!verifyFPGAAndPROM && !checkingFPGAAndPROM) {
    if (runState == State_Idle) {
#ifdef TTCVIDRIVERVIATCP
      if ((!gotTTCvi) && TTCviTCPConnected) {
        printf("TTCvi TCP client is connected\n");
        if (!InitTTCvi()) {
          printf("Unable to perform TTCvi initialization, will disconnect TTCvi TCP client.\n");
          gotTTCvi = FALSE;
          status = SendTTCviTCPCommand(CMDQUIT, 0, 0);
          if (status != 0) SendTTCviTCPCommand(CMDQUIT, 0, 0);
          if (status != 0) status = DisconnectTTCviClient();
        }
        else {
          gotTTCvi = TRUE;
          measureTriggerRate = TRUE;
        }
      }
#endif
      GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_MESSAGE, 0, str);
      strp = (char *) strstr(str, "Number of processed auto runs");
      if (!useMessageBox && (strp == NULL)) {
        sprintf(str, "IP Address = %s  (Host Name = %s.%s)\n", IPAddress, hostName, domainName);
        ResetTextBox(DAQControlHandle, P_JTAGCTRL_MESSAGE, str);
      }
    }
    else if (autoRunOn || (autoRunFromControlFile == 1)) {
      useMessageBox = TRUE;
      sprintf(str, "Number of processed auto runs = %d (# bad auto runs = %d)\n", numberAutoRun, numberBadAutoRun);
      ResetTextBox(DAQControlHandle, P_JTAGCTRL_MESSAGE, str);
    }
  }
  nnodes = NumberConnectedTCPNodes();
  dimmed = FALSE;
  if (runState != State_Idle) dimmed = TRUE;
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_AUTORUNSETUP, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_RUNCONDITION, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_CHECKALLHARDWARE, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RESETBOARD, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RESETL1AFIFO, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_SOFTWARETRIGGER, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_DISABLETRIGGER, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_TRIGSELECT, ATTR_DIMMED, dimmed);
  if (!DAQTCPConnected) {
    UpdateTCPStatus(FALSE);
    getOneEventInitDone = FALSE;
    getOneSequentialEventInitDone = FALSE;
    dimmed = TRUE;
    if ((runState != State_Idle) && (runStartedWithTCPClient > 0)) {
      if (verifyFPGAAndPROM) {
        requestStartOrStop = TRUE;
        restartRunAfterStop = FALSE;
      }
      else AutoRunControl(FALSE);
    }
  }
  SetCtrlVal(DAQControlHandle, P_JTAGCTRL_NBTCPNODES, nnodes);
  if (runDiagnostics) {
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, "Quit Diagnostics Run");
  }
  else if (checkMezzOnOff) {
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, "Stop Check Mezz. On/Off");
  }
  else if (verifyFPGAAndPROM) {
    GetCtrlVal(ConfirmVFPHandle, P_SELVFP_CHECKMODE, &checkMode);
    if ((checkMode < 2) && PROMError) {
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, FALSE);
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, "Program PROM");
    }
    else if ((checkMode < 2) && (FPGAError || JTAGFailed || (analysisCounter[WRONGEVENT] > 1000))) {
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, FALSE);
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, "Program FPGA from PROM");
    }
    else if ((checkMode < 1) && (JTAGFailed || (analysisCounter[WRONGEVENT] > 1000))) {
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, FALSE);
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, "Program FPGA from PROM");
    }
    else if ((checkMode < 1) && (CSMError || (dataError > 1))) {
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, FALSE);
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, "ResetCSM & INIT DAQ");
    }
    else if ((runState != State_Idle) && (!requestStartOrStop)) {
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, FALSE);
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, "Request to Stop Run");
    }
    else {
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, TRUE);
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, "Disconnect All TCP Clients");
    }
  }
  else if (autoRunOn || (autoRunFromControlFile == 1)) {
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, "Stop Auto Run");
  }
  else {
    if (nnodes <= 0) dimmed = TRUE;
    else dimmed = FALSE;
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, "Disconnect All TCP Clients");
  }
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_CHECKMEZZONOFF, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_RDIAG, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_RDIAG_GETRAWDATA, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_RDIAG_GETONEEVENT, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_RDIAG_GETONESEQEVENT, ATTR_DIMMED, dimmed);
  if (runState != State_Idle) {
    TTCviOneShotTriggerIfSelected();
    if (runNumber != oldRunNumber) runStarted = FALSE;
    oldRunNumber = runNumber;
    if (runStartedWithTCPClient > 0) {
      if (!runStarted) {
        status = 0;
        runStarted = TRUE;
        processAutoRun = FALSE;
        time0 = (float) Timer();
        ttime0 = time0;
        dtime0 = (int) time(NULL);
        drtime0 = (unsigned int) time(NULL);
        saveLogTime0 = (unsigned int) time(NULL);
        totalTriggerTime = 0.0;
        dttime0 = time0;
        dtriggerTime = 0.0;
        dtriggerDeadtime = 0.0;
      }
      if (requestStopByClient) {
        if ((((int) time(NULL)) - requestStopTime0) > 2) {
          if ((autoRunOn) || (autoRunFromControlFile == 1)) AutoRunControl(TRUE);
          else StopDAQ();
        }
        time0 = (float) Timer();
      }
      else {
        requestStopTime0 = (int) time(NULL);
        dtime = ((float) Timer()) - time0;
        TTCviOneShotTriggerIfSelected();
        if (((status == 0) && (dtime > 0.2)) || ((status < 0) && (dtime > 0.05))) {
          time0 = (float) Timer();
          GOLANumber = -1;
          TCPData[TCPCOMMAND] = 0xC0000 | CMDREQDAQINFOR;
          TCPData[TCPLENGTH] = 2;
          if (GOLANumber < 0) status = SendToClients(TRUE, UNIX_PORT_NUM);
          else status = 0;
          ntry = 0;
          TTCviOneShotTriggerIfSelected();
        }
        else if ((status > 0) && (dtime > 0.05)) {
          time0 = (float) Timer();
          GOLANumber = -1;
          TCPData[TCPCOMMAND] = 0xC0000 | CMDREREQDAQINFOR;
          if (GOLANumber < 0) {
            if (ntry == 1) status = SendToClients(FALSE, UNIX_PORT_NUM);
            else status = SendToClients(TRUE, UNIX_PORT_NUM);
          }
          else status = 0;
          ntry++;
          if (ntry > 3) status = 0;
          TTCviOneShotTriggerIfSelected();
        }
      }
      dtime = ((unsigned int) time(NULL)) - saveLogTime0;
      if (dtime > 120.) {
        SaveRunLogfile();             // Save run information to a log file
        saveLogTime0 = (unsigned int) time(NULL);
      }
    }
    TTCviOneShotTriggerIfSelected();
    if (!verifyFPGAAndPROM) {
      // Handle auto runs
      nError = analysisCounter[WRONGEVENT];
      if ((stopAfterNErrorOn == 1) && (nError >= stopAfterNError)) {
        stopAfterNErrorOn = 0;
        AutoRunControl(FALSE);
      }
      nEvent = analysisCounter[ANALYSEDEVENT];
      if ((stopRunAtEventOn == 1) && (totalEvent > stopRunAtEvent)) {
        totalRun = 0;
        totalEvent = 0;
        autoRunOn = FALSE;
        AutoRunControl(FALSE);
      }
      else if (((restartRunAtEventOn == 1) && (nEvent > restartRunAtEvent)) ||
               ((restartRunAfterSecOn == 1) && (DAQTime > restartRunAfterSec))) {
        totalRun++;
        if ((stopAfterNRunOn == 1) && (totalRun >= stopAfterNRun)) {
          totalRun = 0;
          totalEvent = 0;
          autoRunOn = FALSE;
          AutoRunControl(FALSE);
        }
        else AutoRunControl(TRUE);
      }
      if (currentDAQRate != 0) dtime0 = (int) time(NULL);
      else {
        dtime = ((int) time(NULL)) - dtime0;
        if ((restartRunAtHangTimeOn == 1) && (dtime >= restartRunAtHangTime)) {
          if (nEvent == 0) AutoRunControl(FALSE);
          else AutoRunControl(TRUE);
        }
      }
    }
  }
  else if (runState == State_Idle) runStarted = FALSE;
  // Handle FPGA and PROM verification
  if (verifyFPGAAndPROM && !checkingFPGAAndPROM) {
    GetCtrlVal(ConfirmVFPHandle, P_SELVFP_CHECKMODE, &checkMode);
    checkingFPGAAndPROM = TRUE;
    if (saveCSMVersion && (radTestLogFile != NULL)) {
      saveCSMVersion = FALSE;
      SetupJTAGChain();
      if (gotCSM) {
        CSMJTAGOperation(CSMAMTPHASEERROR, FALSE);
        if (CSMVersionDate > 2005021522) CSMJTAGOperation(CSMAMTFULLPHASE, FALSE);
        BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
        fprintf(radTestLogFile, "CSM Program Version    : %54c 0x%03X\n", ' ', version);
        fprintf(radTestLogFile, "CSMVerDate (YYYYMMDDhh): %49c %010d\n", ' ', CSMVersionDate);
        sprintf(str, "(JTAG Instr=0x%02x) ErrBits=0x%05X", CSMAMTPHASEERROR, AMTPhaseError);
        fprintf(radTestLogFile, "AMT Phase ErrorBitsOnly: %60s\n", str);
        if (CSMVersionDate > 2005021522) {
          sprintf(str, "(JTAG Instr=0x%02x) PhaseErr=%d ErrBits=0x%05X", CSMAMTFULLPHASE, AMTFullPhaseErrorBit, AMTFullPhaseError);
          fprintf(radTestLogFile, "AMT Full Phase Err Bits: %60s\n", str);
          sprintf(str, "(JTAG Instr=0x%02x) PhaseSamples (%d-0) = ", CSMAMTFULLPHASE, MAXNUMBERMEZZANINE-1);
          for (j = MAXNUMBERMEZZANINE-1; j >= 0; j--) sprintf(&str[strlen(str)], "%01X", AMTPhaseSamples[j]);
          fprintf(radTestLogFile, "AMT Full Phase Samples : %60s\n", str);
        }
      }
    }
    dtime = (float) Timer() - dataTime0;
    if ((runState != State_Idle) || (checkMode > 0) || (dtime >= 2)) {
      dataTime0 = (float) Timer();
      VFPCounter[VFPNUMBERVERIFYLOOP]++;
      if ((VFPCounter[VFPNUMBERVERIFYLOOP]%100) == 1) {
        sprintf(str, "Test loop %u with Run Number %u.\n", VFPCounter[VFPNUMBERVERIFYLOOP], runNumber);
        WriteToFPGAAndPROMLogFile(str);
      }
      dataError = analysisCounter[WRONGEVENT] - oldWrongEvent;
      if (runState == State_Idle) analysisCounter[WRONGEVENT] = 0;
      oldWrongEvent = analysisCounter[WRONGEVENT];
      if (dataError > 0) {
        VFPCounter[VFPDATAERROR] += dataError;
        if (dataError == 1) VFPCounter[VFPSINGLEDATAERROR]++;
        else VFPCounter[VFPMULTIPLEDATAERROR]++;
        sprintf(str, "Data Error (%d wrong events) at test loop %u.\n", dataError, VFPCounter[VFPNUMBERVERIFYLOOP]);
        WriteToFPGAAndPROMLogFile(str);
      }
      unknownTDCWords = analysisCounter[TDCUNEXPECTEDDATA] - oldTDCWords;
      if (runState == State_Idle) analysisCounter[TDCUNEXPECTEDDATA] = 0;
      oldTDCWords = analysisCounter[TDCUNEXPECTEDDATA];
      if (unknownTDCWords > 0) {
        VFPCounter[VFPUNKNOWNTDCWORDS] += unknownTDCWords;
        sprintf(str, "Unexpected %d TDC Data Words at test loop %u.\n", unknownTDCWords, VFPCounter[VFPNUMBERVERIFYLOOP]);
        WriteToFPGAAndPROMLogFile(str);
      }
      unknownDataWords = analysisCounter[EXTRAWORD] - oldDataWords;
      if (runState == State_Idle) analysisCounter[EXTRAWORD] = 0;
      oldDataWords = analysisCounter[EXTRAWORD];
      if (unknownDataWords > 0) {
        VFPCounter[VFPUNKNOWNDATAWORDS] += unknownDataWords;
        sprintf(str, "Unknown %d Data Words at test loop %u.\n", unknownDataWords, VFPCounter[VFPNUMBERVERIFYLOOP]);
        WriteToFPGAAndPROMLogFile(str);
      }
    }
    if (((checkMode == 1) && ((VFPCounter[VFPNUMBERVERIFYLOOP]%1023) == 1)) || (checkMode > 1) || requestVerifyJTAG) {
      VFPCounter[VFPJTAGVERIFY]++;
      SetupJTAGChain();
      if ((!gotPROM) || (!(gotXC2V1000 || gotXC2V2000))) SetupJTAGChain();
      if (gotPROM && (gotXC2V1000 || gotXC2V2000)) JTAGFailed = FALSE;
      else {
        JTAGFailed = TRUE;
        VFPCounter[VFPJTAGFAILURE]++;
        if (!gotPROM && (!(gotXC2V1000 || gotXC2V2000))) {
          sprintf(str, "Unable to find PROM and VertexII Chip at JTAG test loop %u.\n", VFPCounter[VFPJTAGVERIFY]);
          WriteToFPGAAndPROMLogFile(str);
        }
        else if (!gotPROM) {
          sprintf(str, "Unable to find PROM at JTAG test loop %u.\n", VFPCounter[VFPJTAGVERIFY]);
          WriteToFPGAAndPROMLogFile(str);
        }
        else if (!(gotXC2V1000 || gotXC2V2000)) {
          sprintf(str, "Unable to VertexII Chip at JTAG test loop %u.\n", VFPCounter[VFPJTAGVERIFY]);
          WriteToFPGAAndPROMLogFile(str);
        }
      }
      if (!JTAGFailed && gotCSM) {
        CSMSetupError = FALSE;
        CSMStatusError = FALSE;
        CSMError = FALSE;
        CheckCSMSetupAndStatus();
        if (downloadCSMStatus != 0) {
          CSMSetupError = TRUE;
          CSMError = TRUE;
          VFPCounter[VFPCSMSETUPERROR]++;
          if (downloadCSMStatus > 0) VFPCounter[VFPCSMSETUPNBITERROR] += downloadCSMStatus;
          sprintf(str, "CSM Setup Error (%d wrong bits) at JTAG test loop %u.\n", downloadCSMStatus, VFPCounter[VFPJTAGVERIFY]);
          WriteToFPGAAndPROMLogFile(str);
        }
        if (CSMStatusArray[CSMERRORBIT] == 1) {
          CSMStatusError = TRUE;
          CSMError = TRUE;
          VFPCounter[VFPCSMSTATUSERROR]++;
          sprintf(str, "CSM Error Bit is on at JTAG test loop %u.\n", VFPCounter[VFPJTAGVERIFY]);
          WriteToFPGAAndPROMLogFile(str);
        }
        if ((CSMStatusArray[XMTLOCK1BIT] == 0) || (CSMStatusArray[XMTLOCK2BIT] == 0)) {
          CSMStatusError = TRUE;
          CSMError = TRUE;
          VFPCounter[VFPCSMERROR]++;
          sprintf(str, "CSM XMT clock lock has error at JTAG test loop %u.\n", VFPCounter[VFPJTAGVERIFY]);
          WriteToFPGAAndPROMLogFile(str);
        }
        if (CSMStatusArray[PHASEERRORBIT] == 1) {
          CSMStatusError = TRUE;
          CSMError = TRUE;
          VFPCounter[VFPCSMERROR]++;
          sprintf(str, "CSM AMT Phase Lock error at JTAG test loop %u.\n", VFPCounter[VFPJTAGVERIFY]);
          WriteToFPGAAndPROMLogFile(str);
        }
        if (CSMStatusArray[TTCLOADSTATUSBIT] == 1) {
          CSMStatusError = TRUE;
          CSMError = TRUE;
          VFPCounter[VFPCSMERROR]++;
          sprintf(str, "CSM has error (TTCLoad Failed) at JTAG test loop %u.\n", VFPCounter[VFPJTAGVERIFY]);
          WriteToFPGAAndPROMLogFile(str);
        }
        if ((CSMStatusArray[TTCRXREADYBIT] == 0) || (CSMStatusArray[LHCLOCKBIT] == 0)) {
          CSMStatusError = TRUE;
          CSMError = TRUE;
          VFPCounter[VFPTTCRXERROR]++;
          sprintf(str, "TTCrx not ready or LHC Lock error at JTAG test loop %u.\n", VFPCounter[VFPJTAGVERIFY]);
          WriteToFPGAAndPROMLogFile(str);
        }
        if (CSMStatusArray[GOLREADYBIT] == 0) {
          CSMStatusError = TRUE;
          CSMError = TRUE;
          VFPCounter[VFPGOLERROR]++;
          sprintf(str, "GOL is not ready at JTAG test loop %u.\n", VFPCounter[VFPJTAGVERIFY]);
          WriteToFPGAAndPROMLogFile(str);
        }
      }
      if (!(JTAGFailed || CSMError)) {
        sprintf(str, "JTAG check loop %d at test loop %u.\n", VFPCounter[VFPJTAGVERIFY], VFPCounter[VFPNUMBERVERIFYLOOP]);
        WriteToFPGAAndPROMLogFile(str);
      }
      requestVerifyJTAG = FALSE;
    }
    if (requestStartOrStop) {
      if (runState == State_Idle) InitDAQ();
      StartOrStopDAQ();
      if (restartRunAfterStop && (runState != State_Idle)) {
        autoRunOn = TRUE;
        sprintf(str, "Run %d Started.\n", runNumber);
        WriteToFPGAAndPROMLogFile(str);
      }
      else if (runState == State_Idle) {
        sprintf(str, "Run %d Stopped.\n", runNumber);
        WriteToFPGAAndPROMLogFile(str);
      }
      if (!gotCSM) {      // If not find CSM, do not start another run
        autoRunOn = FALSE;
        restartRunAfterStop = FALSE;
      }
      if (!restartRunAfterStop) requestStartOrStop = FALSE;    
    }
    if (!JTAGFailed && ((checkMode > 1) || requestVerifyFPGA) || (VFPCounter[VFPNUMBERVERIFYLOOP] == 1)) {
      dtime = (float) Timer() - FPGATime0;
      if (requestVerifyFPGA || (VFPCounter[VFPNUMBERVERIFYLOOP] == 1)) dtime = verifyFPGATime;
      FPGAError = FALSE;
      nError = 0;
      if ((dtime >= verifyFPGATime) || CSMError || (dataError > 1)) {
        FPGATime0 = (float) Timer();
        VFPCounter[VFPNUMBERFPGAVERIFY]++;
        nError = VerifyVertexIIFPGA();
        if (nError > 0) {
          if (nError < 10000) {
            FPGAError = TRUE;
            VFPCounter[VFPFPGAERROR]++;
            VFPCounter[VFPFPGANBITERROR] += nError;
            sprintf(str, "FPGA error (%d wrong bits) at FPGA check loop %u.\n", nError, VFPCounter[VFPNUMBERFPGAVERIFY]);
          }
          else {
            VFPCounter[VFPFPGAWBIT10000]++;
            VFPCounter[VFPFPGANBITWBIT10000] += nError;
            sprintf(str, "(Fake?) FPGA error (%d wrong bits) at FPGA check loop %u.\n", nError, VFPCounter[VFPNUMBERFPGAVERIFY]);
          }
          WriteToFPGAAndPROMLogFile(str);
        }
        else {
          sprintf(str, "FPGA check loop %d at test loop %u.\n", VFPCounter[VFPNUMBERFPGAVERIFY], VFPCounter[VFPNUMBERVERIFYLOOP]);
          WriteToFPGAAndPROMLogFile(str);
        }
        requestVerifyFPGA = FALSE;
      }
      if (!FPGAError) {
        if (CSMSetupError) VFPCounter[VFPCSMSETUPERRORONLY]++;
        if (CSMStatusError) VFPCounter[VFPCSMSTATUSERRORONLY]++;
        VFPCounter[VFPDATAERRORONLY] += dataError;
      }
    }
    if ((checkMode > 1) || requestVerifyPROM) {
      dtime = (float) Timer() - PROMTime0;
      if (requestVerifyPROM) dtime = verifyPROMTime;
      PROMError = FALSE;
      if ((dtime >= verifyPROMTime) || (nError > 0)) {
        VFPCounter[VFPNUMBERPROMVERIFY]++;
        nError = VerifyPROM();
        if (nError > 0) {
          if (nError < 10000) {
            PROMError = TRUE;
            VFPCounter[VFPPROMERROR]++;
            VFPCounter[VFPPROMNBITERROR] += nError;
            sprintf(str, "PROM error (%d wrong bits) at PROM check loop %u.\n", nError, VFPCounter[VFPNUMBERPROMVERIFY]);
          }
          else {
            VFPCounter[VFPPROMWBIT10000]++;
            VFPCounter[VFPPROMNBITWBIT10000] += nError;
            sprintf(str, "(Fake?) PROM error (%d wrong bits) at PROM check loop %u.\n", nError, VFPCounter[VFPNUMBERPROMVERIFY]);
          }
          WriteToFPGAAndPROMLogFile(str);
        }
        else {
          sprintf(str, "PROM check loop %d at test loop %u.\n", VFPCounter[VFPNUMBERPROMVERIFY], VFPCounter[VFPNUMBERVERIFYLOOP]);
          WriteToFPGAAndPROMLogFile(str);
        }
        PROMTime0 = (float) Timer();
        requestVerifyPROM = FALSE;
      }
    }
    nError = 0;
    if (VFPCounter[VFPCSMSETUPERROR] != 0) nError++;
    if (VFPCounter[VFPCSMSTATUSERROR] != 0) nError++;
    if (VFPCounter[VFPDATAERROR] != 0) nError++;
    if (VFPCounter[VFPJTAGFAILURE] != 0) nError++;
    if (VFPCounter[VFPFPGAERROR] != 0) nError++;
    if (VFPCounter[VFPPROMERROR] != 0) nError++;
    if (nError == 0) sprintf(str, "No error is detected during verify FPGA and PROM so far.\n");
    else {
      sprintf(str, "Errors: CSMSetup=%d CSMStatus=%d Data=%d JTAG=%d FPGA=%d PROM=%d\n",
              VFPCounter[VFPCSMSETUPERROR], VFPCounter[VFPCSMSTATUSERROR], VFPCounter[VFPDATAERROR],
              VFPCounter[VFPJTAGFAILURE], VFPCounter[VFPFPGAERROR], VFPCounter[VFPPROMERROR]); 
    }
    ResetTextBox(DAQControlHandle, P_JTAGCTRL_MESSAGE, str);
    if ((checkMode > 0) && (JTAGFailed || (analysisCounter[WRONGEVENT] > 1000)) ||  
       ((checkMode > 1) && (PROMError || FPGAError)) || requestProgramPROM || requestProgramFPGA) {
      if (runState != State_Idle) StopDAQ();
      if (PROMError || requestProgramPROM) {
        ProgramPROM();
        requestProgramPROM = FALSE;
        VFPCounter[VFPREPROGRAMPROM]++;
        sprintf(str, "Reprogram PROM Done.\n");
        WriteToFPGAAndPROMLogFile(str);
      }
      else {
        sprintf(str, "Try to reprogram FPGA from PROM\n");
        WriteToFPGAAndPROMLogFile(str);
        ProgramVertexIIFPGAFromPROM();
        requestProgramFPGA = FALSE;
        VFPCounter[VFPREPROGRAMFPGA]++;
      }
      SetupJTAGChain();
      if ((!gotPROM) || (!(gotXC2V1000 || gotXC2V2000))) SetupJTAGChain();
      if ((!gotPROM) || (!(gotXC2V1000 || gotXC2V2000))) {
        VFPCounter[VFPJTAGFAILURE]++;
        VFPCounter[VFPUNRECOVERABLE]++;
        sprintf(str, "Unable to bring back JTAG chain at JTAG test loop %u.\n", VFPCounter[VFPJTAGVERIFY]);
        WriteToFPGAAndPROMLogFile(str);
        ResetTextBox(DAQControlHandle, P_JTAGCTRL_MESSAGE, str);
        verifyFPGAAndPROM = FALSE;
        restartRunAfterStop = FALSE;
        SetCtrlVal(DAQControlHandle, P_JTAGCTRL_RUNTYPE, NORMALALLCALIBCHANNELOFF);
        SetDAQRunType();
      }
      else {
        if (restartRunAfterStop) requestStartOrStop = FALSE;    
        InitDAQ();
        sprintf(str, "InitDAQ Done.\n");
        WriteToFPGAAndPROMLogFile(str);
      }
    }
    else if (((checkMode > 0) && (CSMError || (dataError > 1))) || requestResetINITDAQ) {
      CSMError = 0;
      dataError = 0;
      requestStartOrStop = FALSE;
      if (requestResetINITDAQ) ResetCSM();
      InitDAQ();
      requestResetINITDAQ = FALSE;
      sprintf(str, "InitDAQ Done.\n");
      WriteToFPGAAndPROMLogFile(str);
    }
    if (restartRunAfterStop && (runState == State_Idle)) {
      oldWrongEvent = 0;
      StartDAQ();
      if (runState != State_Idle) sprintf(str, "Run %d Started.\n", runNumber);
      else sprintf(str, "Start Run %d Failed.\n", runNumber);
      WriteToFPGAAndPROMLogFile(str);
    }
    if (requestBackNormal) {
      requestBackNormal = FALSE;
      SetDAQRunType();
    }
    VFPCounter[VFPTIME] = (unsigned int) (time(NULL) - VFPStartTime);
    if ((!verifyFPGAAndPROM) || quitProgram) {
      verifyFPGAAndPROM = FALSE;
      autoRunOn = FALSE;
      if (radTestLogFile != NULL) {
        fprintf(radTestLogFile, "//\n");
        if (checkMode > 1) {
          fprintf(radTestLogFile, "// Reprogram FPGA/PROM if any configuration error\n"
                                  "// --> no double count on FPGA/PROM errors.\n");
        }
        else {
          fprintf(radTestLogFile, "// No FPGA/PROM reprogramming if any configuration error\n"
                                  "// --> double count on FPGA/PROM errors is possible.\n"
                                  "// Detail error information saved in this logfile for each FPGA/PROM verification,\n"
                                  "// which could be used to figure out number bit upset for FPGA/PROM.\n");
        }
        fprintf(radTestLogFile, "//\n");
        fprintf(radTestLogFile, "Start Date and Time    : %60s\n", WeekdayTimeString(VFPStartTime));
        fprintf(radTestLogFile, "Stop Date and Time     : %60s\n", WeekdayTimeString(time(NULL)));
        second = VFPCounter[VFPTIME] % 60;
        minute = (VFPCounter[VFPTIME] / 60) % 60;
        hour = VFPCounter[VFPTIME] / 3600;
        sprintf(str, "%dH%2dM%2dS", hour, minute, second);
        fprintf(radTestLogFile, "Total Verifying Time   : %60s\n", str);
        fprintf(radTestLogFile, "# Verification Loop    : %60u\n", VFPCounter[VFPNUMBERVERIFYLOOP]);
        fprintf(radTestLogFile, "# Data Error           : %60u\n", VFPCounter[VFPDATAERROR]);
        fprintf(radTestLogFile, "# Single Data Error    : %60u\n", VFPCounter[VFPSINGLEDATAERROR]);
        fprintf(radTestLogFile, "# MultipleData Error   : %60u\n", VFPCounter[VFPMULTIPLEDATAERROR]);
        fprintf(radTestLogFile, "# Unexpected TDC Words : %60u\n", VFPCounter[VFPUNKNOWNTDCWORDS]);
        fprintf(radTestLogFile, "# Unknown data Words   : %60u\n", VFPCounter[VFPUNKNOWNDATAWORDS]);
        fprintf(radTestLogFile, "# JTAG/CSM Verification: %60u\n", VFPCounter[VFPJTAGVERIFY]);
        fprintf(radTestLogFile, "# CSM Setup Error      : %60u\n", VFPCounter[VFPCSMSETUPERROR]);
        fprintf(radTestLogFile, "# ErrBit in CSMSetup   : %60u\n", VFPCounter[VFPCSMSETUPNBITERROR]);
        fprintf(radTestLogFile, "# CSM Status Error     : %60u\n", VFPCounter[VFPCSMSTATUSERROR]);
        fprintf(radTestLogFile, "# CSM Error            : %60u\n", VFPCounter[VFPCSMERROR]);
        fprintf(radTestLogFile, "# TTCrx Error          : %60u\n", VFPCounter[VFPTTCRXERROR]);
        fprintf(radTestLogFile, "# GOL Error            : %60u\n", VFPCounter[VFPGOLERROR]);
        fprintf(radTestLogFile, "# CSM Setup Error Only : %60u\n", VFPCounter[VFPCSMSETUPERRORONLY]);
        fprintf(radTestLogFile, "# CSM Status Error Only: %60u\n", VFPCounter[VFPCSMSTATUSERRORONLY]);
        fprintf(radTestLogFile, "# Data Error Only      : %60u\n", VFPCounter[VFPDATAERRORONLY]);
        fprintf(radTestLogFile, "# JTAG Failure         : %60u\n", VFPCounter[VFPJTAGFAILURE]);
        fprintf(radTestLogFile, "# FPGA Verification    : %60u\n", VFPCounter[VFPNUMBERFPGAVERIFY]);
        fprintf(radTestLogFile, "# FPGA Error           : %60u\n", VFPCounter[VFPFPGAERROR]);
        fprintf(radTestLogFile, "# Wrong Bits in FPGA   : %60u\n", VFPCounter[VFPFPGANBITERROR]);
        fprintf(radTestLogFile, "# FPGA WBits >=10000   : %60u\n", VFPCounter[VFPFPGAWBIT10000]);
        fprintf(radTestLogFile, "# FPGA WBits aboveCase : %60u\n", VFPCounter[VFPFPGANBITWBIT10000]);
        fprintf(radTestLogFile, "# PROM Verification    : %60u\n", VFPCounter[VFPNUMBERPROMVERIFY]);
        fprintf(radTestLogFile, "# PROM Error           : %60u\n", VFPCounter[VFPPROMERROR]);
        fprintf(radTestLogFile, "# Wrong Bits in PROM   : %60u\n", VFPCounter[VFPPROMNBITERROR]);
        fprintf(radTestLogFile, "# PROM WBits >=10000   : %60u\n", VFPCounter[VFPPROMWBIT10000]);
        fprintf(radTestLogFile, "# PROM WBits aboveCase : %60u\n", VFPCounter[VFPPROMNBITWBIT10000]);
        fprintf(radTestLogFile, "# InitDAQ Due CSMError : %60u\n", VFPCounter[VFPINITDAQDUETOERROR]);
        fprintf(radTestLogFile, "# Reprogram FPGA       : %60u\n", VFPCounter[VFPREPROGRAMFPGA]);
        fprintf(radTestLogFile, "# Reprogram PROM       : %60u\n", VFPCounter[VFPREPROGRAMPROM]);
        fprintf(radTestLogFile, "# Unrecoverable        : %60u\n", VFPCounter[VFPUNRECOVERABLE]);
        fclose(radTestLogFile);
      }
      printf("//\n");
      if (checkMode > 1) {
        printf("// Reprogram FPGA/PROM if any configuration error\n"
               "// --> no double count on FPGA/PROM errors.\n");
      }
      else {
        printf("// No FPGA/PROM reprogramming if any configuration error\n"
               "// --> double count on FPGA/PROM errors is possible.\n");
      }
      printf("//\n");
      printf("Start Date and Time    : %60s\n", WeekdayTimeString(VFPStartTime));
      printf("Stop Date and Time     : %60s\n", WeekdayTimeString(time(NULL)));
      second = VFPCounter[VFPTIME] % 60;
      minute = (VFPCounter[VFPTIME] / 60) % 60;
      hour = VFPCounter[VFPTIME] / 3600;
      sprintf(str, "%dH%2dM%2dS", hour, minute, second);
      printf("Total Verifying Time   : %60s\n", str);
      printf("# Verification Loop    : %60u\n", VFPCounter[VFPNUMBERVERIFYLOOP]);
      printf("# Data Error           : %60u\n", VFPCounter[VFPDATAERROR]);
      printf("# Single Data Error    : %60u\n", VFPCounter[VFPSINGLEDATAERROR]);
      printf("# MultipleData Error   : %60u\n", VFPCounter[VFPMULTIPLEDATAERROR]);
      printf("# Unexpected TDC Words : %60u\n", VFPCounter[VFPUNKNOWNTDCWORDS]);
      printf("# Unknown data Words   : %60u\n", VFPCounter[VFPUNKNOWNDATAWORDS]);
      printf("# JTAG/CSM Verification: %60u\n", VFPCounter[VFPJTAGVERIFY]);
      printf("# CSM Setup Error      : %60u\n", VFPCounter[VFPCSMSETUPERROR]);
      printf("# ErrBit in CSMSetup   : %60u\n", VFPCounter[VFPCSMSETUPNBITERROR]);
      printf("# CSM Status Error     : %60u\n", VFPCounter[VFPCSMSTATUSERROR]);
      printf("# CSM Error            : %60u\n", VFPCounter[VFPCSMERROR]);
      printf("# TTCrx Error          : %60u\n", VFPCounter[VFPTTCRXERROR]);
      printf("# GOL Error            : %60u\n", VFPCounter[VFPGOLERROR]);
      printf("# CSM Setup Error Only : %60u\n", VFPCounter[VFPCSMSETUPERRORONLY]);
      printf("# CSM Status Error Only: %60u\n", VFPCounter[VFPCSMSTATUSERRORONLY]);
      printf("# Data Error Only      : %60u\n", VFPCounter[VFPDATAERRORONLY]);
      printf("# JTAG Failure         : %60u\n", VFPCounter[VFPJTAGFAILURE]);
      printf("# FPGA Verification    : %60u\n", VFPCounter[VFPNUMBERFPGAVERIFY]);
      printf("# FPGA Error           : %60u\n", VFPCounter[VFPFPGAERROR]);
      printf("# Wrong Bits in FPGA   : %60u\n", VFPCounter[VFPFPGANBITERROR]);
      printf("# FPGA WBits >=10000   : %60u\n", VFPCounter[VFPFPGAWBIT10000]);
      printf("# FPGA WBits aboveCase : %60u\n", VFPCounter[VFPFPGANBITWBIT10000]);
      printf("# PROM Verification    : %60u\n", VFPCounter[VFPNUMBERPROMVERIFY]);
      printf("# PROM Error           : %60u\n", VFPCounter[VFPPROMERROR]);
      printf("# Wrong Bits in PROM   : %60u\n", VFPCounter[VFPPROMNBITERROR]);
      printf("# PROM WBits >=10000   : %60u\n", VFPCounter[VFPPROMWBIT10000]);
      printf("# PROM WBits aboveCase : %60u\n", VFPCounter[VFPPROMNBITWBIT10000]);
      printf("# InitDAQ Due CSMError : %60u\n", VFPCounter[VFPINITDAQDUETOERROR]);
      printf("# Reprogram FPGA       : %60u\n", VFPCounter[VFPREPROGRAMFPGA]);
      printf("# Reprogram PROM       : %60u\n", VFPCounter[VFPREPROGRAMPROM]);
      printf("# Unrecoverable        : %60u\n", VFPCounter[VFPUNRECOVERABLE]);
    }
    checkingFPGAAndPROM = FALSE;
  }
  else if (!verifyFPGAAndPROM) {
    FPGATime0 = (float) Timer();
    PROMTime0 = (float) Timer();
    dataTime0 = (float) Timer();
    saveCSMVersion = TRUE;
    restartRunAfterStop = FALSE;
    requestBackNormal = FALSE;
    if ((!autoRunOn) && (autoRunFromControlFile == 0)) needCheckNumberMezzCard = TRUE;
  }
  if ((runState != State_Idle) || measureTriggerRate) {
    if (gotTTCvi) {
      triggerTime = (float) Timer() - dttime0;
      if (triggerDeadtime <= 0) {
        dtriggerTime = 0.0;
        dtriggerDeadtime = 0.0;
      }
      else if (dttime0 > 0.002) {
        dtriggerTime += triggerTime;
        if (TTCviTriggerDisabled) dtriggerDeadtime += triggerTime;
        if (dtriggerTime > 0.0) {
          fracDeadtime = dtriggerDeadtime / dtriggerTime;
          if (runState == State_Running) {
            if (TTCviTriggerDisabled) {
              if ((100.0*fracDeadtime) >= triggerDeadtime) TTCviRestoreTrigger();
            }
            else {
              if ((100.0*fracDeadtime) < triggerDeadtime) TTCviSetUnusedTrigger();
            }
          }
          if ((dtriggerTime > 5000.) || (dtriggerDeadtime > 500.)) {
            dtriggerTime = 0.0;
            dtriggerDeadtime = 0.0;
          }
        }
        dttime0 = (float) Timer();
      }
    }
    rate = MeasuredTTCviTriggerRate(&numberTrigger);
    if (rate < 0.0) rate = oldRate;
    else if (rate != oldRate) {
      if (runState == State_Running) {
        if (fracDeadtime < 0.80) rate /= (1.0 - fracDeadtime);
        else if (fracDeadtime < 1.0) {
          if ((rate*dtriggerTime) > 100.0) rate /= (1.0 - fracDeadtime);
        }
      }
      if (runState == State_Running) SaveMeasuredTriggerRate(rate);
      dtime = ((unsigned int) time(NULL)) - drtime0;
	  if (dtime > 2.0) {
	    drtime0 = (unsigned int) time(NULL);
        sprintf(string, "%.2fHz", rate);
        ResetTextBox(DAQControlHandle, P_JTAGCTRL_TRIGRATE, string);
        SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_TRIGRATE, ATTR_TEXT_BGCOLOR, VAL_WHITE);
      }
      currentTriggerRate = rate;
      if (runStarted) {
        triggerTime = (float) Timer() - ttime0;
        ntrigger += rate * triggerTime;
        ttime0 = (float) Timer();
        if (currentTriggerRate > maximumTriggerRate) maximumTriggerRate = currentTriggerRate;
        if (currentTriggerRate < minimumTriggerRate) minimumTriggerRate = currentTriggerRate;
        if ((totalNumberTrigger+ntrigger) < totalNumberTrigger) totalNumberTriggerHigh++;
        totalNumberTrigger += ntrigger;
        totalTriggerTime += triggerTime;
        ntrigger = 0;
      }
    }
    oldRate = rate;
  }
  else {
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_TRIGRATE, ATTR_TEXT_BGCOLOR, VAL_MAGENTA);
  }
}


// DAQ Control Start Up
void DAQControlStartUp(void) {
  int mezz, ch;
  
  if (!gotTTCvi) {
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, TRUE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, TRUE);
  }
  else SetDAQRunType();
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    lEdgeOn[mezz] = 0;
    tEdgeOn[mezz] = 0;
    pairOn[mezz] = 0;
    for (ch = 0; ch < 24; ch++) {
      channelStatusHVOn[ch][mezz] = RDOUTCHOK + RDOUTSTATUSOFFSETHVON;
      channelStatusHVOff[ch][mezz] = RDOUTCHOK + RDOUTSTATUSOFFSETHVOFF;
    }
  }
  initDAQDone = FALSE;
  getOneEventInitDone = FALSE;
  getOneSequentialEventInitDone = FALSE;
  GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, &storageCtrl);
}


// Define DAQ Control Buttons
int DAQControlButton(void) {
  // DAQ Information Panel and its Buttons
  if ((DAQInforHandle = LoadPanel(0, "DAQGUI.uir", P_DAQINFOR)) < 0) return -1;
  SetPanelAttribute(DAQInforHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  SetupUserButton(DAQInforHandle, P_DAQINFOR_CLOSE, CloseDAQInfor);
  SetupUserButton(DAQInforHandle, P_DAQINFOR_DISPLAYPLOTS, DAQInforControl);

  // Data Error Report Panel and Buttons
  if ((ErrorReportHandle = LoadPanel(0, "DAQGUI.uir", P_REPORT)) < 0) return -1;
  SetPanelAttribute(ErrorReportHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  SetupUserButton(ErrorReportHandle, P_REPORT_CLOSE, ErrorReportClose);
  
  // Define Serial Number Panel and its Buttons
  if ((SerialNumberHandle = LoadPanel(0, "DAQGUI.uir", P_SNUMBER)) < 0) return -1;
  SetPanelAttribute(SerialNumberHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  SetupUserButton(SerialNumberHandle, P_SNUMBER_LOADPREVIOUSSN, LoadPreviousSerialNumber);
  SetupUserButton(SerialNumberHandle, P_SNUMBER_USEDCSONCHAMBER, OnChamberDCSBoxUsage);
  SetupUserButton(SerialNumberHandle, P_SNUMBER_EXPECTEDELMBNODE, OnChamberDCSBoxUsage);
  SetupUserButton(SerialNumberHandle, P_SNUMBER_ALLCHECKED, CheckedAllMezzCardSerialNumber);
  SetupUserButton(SerialNumberHandle, P_SNUMBER_DONE, PanelDone);

  // Define check mezzanine card on/off status panel handles
  if ((CheckMezzOnOffHandle = LoadPanel(0, "DAQGUI.uir", P_MZONOFF)) < 0) return -1;
  SetPanelAttribute(CheckMezzOnOffHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  SetupUserButton(CheckMezzOnOffHandle, P_MZONOFF_REDO, CheckMezzCardOnOff);
  SetupUserButton(CheckMezzOnOffHandle, P_MZONOFF_CLOSE, PanelDone);

  newRunNumber = FALSE;
  DAQControlHandle = JTAGControlHandle;
  // DAQ Control Panel Buttons
  SetupUserButton(Menu00Handle, MENU00_FILE_QUIT, Quit);
  SetupUserButton(Menu00Handle, MENU00_EXE_INITDAQ, InitDAQ);
  SetupUserButton(Menu00Handle, MENU00_EXE_SEARCHJTAGDRIVER, SearchJTAGDriver);
  SetupUserButton(Menu00Handle, MENU00_EXE_CHECKMEZZONOFF, UpCheckMezzCardOnOff);
  SetupUserButton(Menu00Handle, MENU00_EXE_CHECKALLHARDWARE, CheckAllDAQHardware);
  SetupUserButton(Menu00Handle, MENU00_EXE_RDIAG_GETRAWDATA, GetRawData);
  SetupUserButton(Menu00Handle, MENU00_EXE_RDIAG_GETONEEVENT, GetOneEvent);
  SetupUserButton(Menu00Handle, MENU00_EXE_RDIAG_GETONESEQEVENT, GetOneSequentialEvent);
  SetupUserButton(Menu00Handle, MENU00_EXE_ERRORREPORT, UpErrorReport);
  SetupUserButton(Menu00Handle, MENU00_EXE_SAVEERRREPORT, SaveErrorSummaryFile);
  SetupUserButton(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, StartOrStopDAQ);
  SetupUserButton(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, PauseOrResumeDAQ);
  SetupUserButton(DAQControlHandle, P_JTAGCTRL_INITDAQ, InitDAQ);
  SetupUserButton(DAQControlHandle, P_JTAGCTRL_QUIT, Quit);
  SetupUserButton(DAQControlHandle, P_JTAGCTRL_RUNTYPE, SetDAQRunType);
  SetupUserButton(DAQControlHandle, P_JTAGCTRL_RUNNUMBER, SetRunNumberButton);
  SetupUserButton(DAQControlHandle, P_JTAGCTRL_DAQINFOR, DAQInforControl);
  SetupUserButton(DAQControlHandle, P_JTAGCTRL_NEWDOSE, NewDose);
  SetupUserButton(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, DisconnectAllTCPClients);
  SetupUserButton(SelectActionHandle, P_SELACT_INITDAQ, InitDAQ);
  SetupUserButton(SelectActionHandle, P_SELACT_DOWNLOAD, DownloadSetup);
  SetupUserButton(SelectActionHandle, P_SELACT_DONOTHING, PanelDone);
  
  return 0;
}


// Quits the DAQ Control program.
void Quit(void) {
  int response;
  
  if (controlOption == NORM) {
    response = ConfirmPopup("Confirm --> QUIT the CSM DAQ Control? <--",
                            "Confirm --> QUIT the CSM DAQ Control? <--");
    if (response) quitProgram = TRUE;
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Quit Button",
                 "Push this button to quit the CSM DAQ Control program.");
    controlOption = NORM;
  }
  if (!quitProgram) LoopAndExecuteUserButtonAndFunction(Quit);
}


void StartOrStopDAQ(void) {
  char str[128];

  if (controlOption == NORM) {
    useMessageBox = FALSE;
    if (!(autoRunOn || (autoRunFromControlFile == 1))) {
      sprintf(str, "IP Address = %s  (Host Name = %s.%s)\n", IPAddress, hostName, domainName);
      ResetTextBox(DAQControlHandle, P_JTAGCTRL_MESSAGE, str);
    }
    autoRunOn = FALSE;
    if (verifyFPGAAndPROM) {
      if (!requestStartOrStop) {
        SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, TRUE);
        requestStartOrStop = TRUE;
        if (runState != State_Idle) restartRunAfterStop = FALSE;
        return;
      }
    }
    requestStartOrStop = FALSE;
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, FALSE);
  }
  if (runState == State_Idle) StartDAQ();
  else {
    StopDAQ();
    if (controlOption == NORM) {
      if (autoRunOn || (autoRunFromControlFile == 1)) {
        MDTChamberOutput2("Total Number of Download CSM        = %30d\n", numberCSMDownload);
        MDTChamberOutput2("Total Number of Failed Download CSM = %30d\n", numberBadCSMDownload);
        MDTChamberOutput2("Total Number of Download MezzCards  = %30d\n", numberMezzDownload);
        MDTChamberOutput2("Total Number of Failed Download Mezz= %30d\n", numberBadMezzDownload);
        MDTChamberOutput2("Total Number of Init DAQ            = %30d\n", numberInitDAQ);
        MDTChamberOutput2("Total Number of Failed Init DAQ     = %30d\n", numberBadInitDAQ);
        MDTChamberOutput2("Total Number of Redone Init DAQ     = %30d\n", numberReInitDAQ);
        MDTChamberOutput2("Total Number of Auto Runs           = %30d\n", numberAutoRun);
        MDTChamberOutput2("Total Number of Bad Auto Runs       = %30d\n", numberBadAutoRun);
        MDTChamberOutput2("Total Number of Rerun in Auto Runs  = %30d\n", numberAutoRerun);
      }
      while (autoRunFromControlFile == 1) GetNewRunSetups(0);
      checkCSM = 0;
      checkCSMJTAG = 0;
      checkCSMIO = 0;
      MDTChamberControl = FALSE;
      autoRunOn = FALSE;
      useExternalTrigger = FALSE;
      thresholdScanMethod = 0;
      numberSubScans = 1;
      subScans = numberSubScans - 1;
      if (MDTChamberSumFile != NULL) {
        MDTChamberOutput1(">>>>>>>>>>  End MDT Chamber Tests <<<<<<<<<<\n\n");
        MDTChamberTest = FALSE;
        fclose(MDTChamberSumFile);
        MDTChamberSumFile = NULL;
      }
    }
  }
  if (controlOption == NORM) {
    numberCSMDownload = 0;
    numberBadCSMDownload = 0;
    numberMezzDownload = 0;
    numberBadMezzDownload = 0;
    numberInitDAQ = 0;
    numberBadInitDAQ = 0;
    numberReInitDAQ = 0;
    numberAutoRun = 0;
    numberBadAutoRun = 0;
    numberAutoRerun = 0;
    reqRerun = FALSE;
    numberRerun = 0;
    totalRun = 0;
    totalEvent = 0;
    autoRunOn = FALSE;
  }
}


void StartDAQ(void) {
  int toDisk, resp, getNewRun, i, stat, mezz, asd, thresh[3], value, ASDMask, calCap, time0, time1, scanRun;
  char fname[256], str[8192], dstr[80];
  
  if (controlOption == NORM) {
    if (autoRunOn || (autoRunFromControlFile == 1)) numberAutoRun++;
    reqProcessData = FALSE;
    requestStopByClient = FALSE;
    runStopStatus = NORMALSTOP;
    GetRunCondition();
    GetCtrlVal(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, &startMezzCard);
    TTCviRestoreTrigger();   // In case no trigger selected, this would do it
    TTCviSetUnusedTrigger();
    if (checkSystemNoise && (!InternalSoftwareTrigger())) {
      resp = ConfirmPopup("Confirm on Using External Trigger!",
                          "A random trigger is needed for checking system noise level!\n"
                          "Are you sure to use external trigger for the noise check?\n"
                          "  (No == User will select trigger for the DAQ run.)");
      if (resp == 0) UpTTCviTriggerSelect();
    }
    else if ((!autoRunOn) && (!checkSystemNoise) && InternalSoftwareTrigger()) {
      resp = ConfirmPopup("Confirm on Using Software Trigger!",
                          "Are you sure to use software trigger for the DAQ run?\n"
                          "  (No == User will select external trigger for the DAQ run.)");
      if (resp == 0) UpTTCviTriggerSelect();
    }
    NumberConnectedTCPNodes();
    if ((!autoRunOn) && (triggerDeadtime != 0) && DAQTCPConnected) {
      sprintf(str, "Are you sure to take data with %d %% trigger deadtime?\n"
                   "  (No == Will set trigger deadtime to zero)", triggerDeadtime);
      resp = ConfirmPopup("Confirm Trigger Deadtime", str);
      if (resp == 0) {
        triggerDeadtime = 0;
        SetCtrlVal(RunConditionHandle, P_RUNCOND_TRIGGERDEADTIME, triggerDeadtime);
      }
    }
    if (InternalSoftwareTrigger() || (MDTChamberHVStatus != MDTCHAMBERHVON)) {
      mappingMDTChamber = FALSE;
      SetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, mappingMDTChamber);
    }
    if (autoRunOn || (autoRunFromControlFile == 1)) stat = CheckClientInstallations(0);
    else if (stopAfterNRunOn || stopRunAtEventOn || restartRunAtEventOn || restartRunAfterSecOn || restartRunAtHangTimeOn) {
      stat = CheckClientInstallations(2);
      if (stat != 0) return;
    }
    else {
      stat = CheckClientInstallations(1);
      if (stat != 0) return; 
    }
    if (clientProcessingData) {
      if (autoRunOn) {
        sprintf(str, "Wait client machine (Linux Box) to finish its data processing at %s\n", WeekdayTimeString(time(NULL)));
        MDTChamberOutput2("%s", str);
        time0 = (int) time(NULL);
        time1 = (int) time(NULL);
        while (clientProcessingData && ((((int) time(NULL)) - time0) < 7200)) {
          if ((((int) time(NULL)) - time1) > 1) {
            stat = SendShortTCPCommand(-1, CMDDETAILSTATUS);
            if (stat != 0) SendShortTCPCommand(-1, CMDDETAILSTATUS);
            time1 = (int) time(NULL);
          }
          HandleUserButtons(Quit);
        }
        time1 = ((int) time(NULL)) - time0;
        strcpy(dstr, WeekdayTimeString(time(NULL)));
        if (clientProcessingData) {
          sprintf(str, "Linux Box is still processing data, terminate the waiting at %s (total wait time = %d (s))\n", dstr, time1);
        }
        else {
          sprintf(str, "Data processing at Linux Box is done at %s (total wait time = %d (s))\n", dstr, time1);
        }
        MDTChamberOutput2("%s", str);
      }
      else if (clientProcessingData) {
        sprintf(str, "Client machine (Linux Box) is still processing data, it is better to start run after\n"
                     "Linux Box finishes its data processing, otherwise data analysis may disturb data taking\n"
                     "with high data rate. In case trigger rate is not too high (< 1KHz) or event size is not\n"
                     "large (i.e., low data rate), run could be started without disturbing data taking.\n"
                     "Do you want to start data taking no matter data processing is done at Linux Box?\n");
        resp = ConfirmPopup("Confirm to Start DAQ", str);
        if (resp == 0) return;
      }
    }
    if (!initDAQDone) InitDAQ();
    if (!autoRunOn) {
      if (CheckAMTDataMode(TRUE)) return;
    }
    if (numberRerun > MAXIMUMRERUN) {
      sprintf(str, "Reaches maximum number of rerun %d, no more rerun", MAXIMUMRERUN);
      MDTChamberOutput2("%s", str);
      reqRerun = FALSE;
      numberRerun = 0;
    }
    if (autoRunOn && reqRerun) {
      scanRun = reqThresholdScan || reqInjectionScan || reqLinearityScan || reqGainScan;
      numberRerun++;
      numberAutoRerun++;
      numberAutoRun--;
      if (scanRun) numberBadScanRun--;
      numberBadAutoRun--;
      sprintf(str, "Run %08u is required for rerun, number try = %d\n", runNumber, numberRerun);
      MDTChamberOutput2("%s", str);
      InitDAQ();
    }
    else if (autoRunFromControlFile == 1) {
      getNewRun = FALSE;
      while (autoRunFromControlFile == 1) {
        if (GetNewRunSetups(1)) {
          getNewRun = TRUE;
          break;
        }
      }
      if (!getNewRun) {
        checkCSM = 0;
        checkCSMJTAG = 0;
        checkCSMIO = 0;
        MDTChamberControl = FALSE;
        autoRunOn = FALSE;
        useExternalTrigger = FALSE;
        thresholdScanMethod = 0;
        numberSubScans = 1;
        subScans = numberSubScans - 1;
        MDTChamberOutput2("Total Number of Download CSM        = %30d\n", numberCSMDownload);
        MDTChamberOutput2("Total Number of Failed Download CSM = %30d\n", numberBadCSMDownload);
        MDTChamberOutput2("Total Number of Download MezzCards  = %30d\n", numberMezzDownload);
        MDTChamberOutput2("Total Number of Failed Download Mezz= %30d\n", numberBadMezzDownload);
        MDTChamberOutput2("Total Number of Init DAQ            = %30d\n", numberInitDAQ);
        MDTChamberOutput2("Total Number of Failed Init DAQ     = %30d\n", numberBadInitDAQ);
        MDTChamberOutput2("Total Number of Redone Init DAQ     = %30d\n", numberReInitDAQ);
        MDTChamberOutput2("Total Number of Auto Runs           = %30d\n", numberAutoRun);
        MDTChamberOutput2("Total Number of Bad Auto Runs       = %30d\n", numberBadAutoRun);
        MDTChamberOutput2("Total Number of Rerun in Auto Runs  = %30d\n", numberAutoRerun);
        if (MDTChamberSumFile != NULL) {
          MDTChamberOutput1(">>>>>>>>>>  End MDT Chamber Tests <<<<<<<<<<\n\n");
          MDTChamberTest = FALSE;
          fclose(MDTChamberSumFile);
          MDTChamberSumFile = NULL;
        }
        return;
      }
    }
    reqRerun = FALSE;
    if (!newRunNumber) {
      runNumber++;
      SaveParameters(PARARUNNUMBER);
    }
    newRunNumber = FALSE;
    GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, &toDisk);
    strcpy(dataDirName, "dat");
    if ((toDisk == 1) || (toDisk == 0)) {
      if (strlen(MDTChamberName) > 5) strcpy(dataDirName, MDTChamberName);
      sprintf(fname, "%s/run%08u_%8s.dat", dataDirName, runNumber, DateStringNoSpace(time(NULL)));
      ReplaceTextBoxLine(DAQControlHandle, P_JTAGCTRL_FILENAME, 0, fname);
      if (SetDir(dataDirName) == 0) SetDir("..");
      else {
        strcpy(str, dataDirName);
        strcat(str, "_savedByDAQ");
        RenameFile(dataDirName, str);
        MakeDir(dataDirName);
      }
    }
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_RUNNUMBER, ATTR_DIMMED, TRUE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_LABEL_TEXT, "Stop Run");
    // Start run sequence
    // 1) Disable Trigger
    // 2) Start GOLA card
    // 3) Start CSM
    // 4) EVID Reset
    // 5) BCID Reset
    // 6) Global Reset
    // 7) Enable Trigger
    TTCviSetUnusedTrigger();
    if (SendTCPCommand(-1, CMDSTARTRUN) != 0) {
      if (runNumber > 0) runNumber--;
      SaveParameters(PARARUNNUMBER);
      runState = State_Idle;
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_RUNNUMBER, ATTR_DIMMED, FALSE);
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, TRUE);
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_LABEL_TEXT, "Start Run");
      printf("Failed to start a run %u!\n", runNumber);
    }
    else {
      SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Start CSM DAQ");
      StartOrStopCSMDAQ();
      TTCviEventCounterReset();
      TTCviBunchCounterReset();
      TTCviEventAndBunchCounterReset();
      GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DAQINFOR, &displayDAQInfor);
      initDAQInfor = 1;
      CollectAndDisplayDAQInfor();
      ResetTextBox(DAQControlHandle, P_JTAGCTRL_STARTTIME, DateTimeString(time(NULL)));
      ResetTextBox(DAQControlHandle, P_JTAGCTRL_STOPTIME, " ");
      sprintf(str, "%d", runNumber);
      ResetTextBox(DAQInforHandle, P_DAQINFOR_RUNNUMBER, str);
      ResetTextBox(DAQInforHandle, P_DAQINFOR_STARTTIME, DateTimeString(time(NULL)));
      ResetTextBox(DAQInforHandle, P_DAQINFOR_DATETIME, WeekdayTimeString(time(NULL)));
      ResetTextBox(DAQInforHandle, P_DAQINFOR_STOPTIME, " ");
      showErrorReport = FALSE;
      if ((!autoRunOn) && (autoRunFromControlFile == 0)) showErrorReport = displayDAQInfor;
      runStartedWithTCPClient = DAQTCPConnected;
      runState = State_Running;
      mezz = MAXNUMBERMEZZANINE;
      for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++) statusSetupCheck[i] = mezzSetupArray[i][mezz];
      if (reqThresholdScan) {
        for (asd = 0; asd < 3; asd++) {
          ReversedBinaryToInt(&value, asd*ASD_SETUP_LENGTH+ASD_MAIN_THRESH, 8, statusSetupCheck);
          thresh[asd] = 2*(value-127);
        }
        sprintf(str, "Run %08u with main thresholds %d %d %d (mV)\n", runNumber, thresh[0], thresh[1], thresh[2]);
      }
      else if (reqInjectionScan) {
        ASDMask = 0;
        for (asd = 0; asd < 3; asd++) {
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH0] << 0) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH1] << 1) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH2] << 2) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH3] << 3) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH4] << 4) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH5] << 5) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH6] << 6) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH7] << 7) << (8*asd);
        }
        sprintf(str, "Run %08u with ASD mask 0x%06X\n", runNumber, ASDMask);
      }
      else if (reqLinearityScan) {
        sprintf(str, "Run %08u with calibration trigger delay %u (ns)\n", runNumber, 25*calibrationTriggerDelay);
      }
      else if (reqGainScan) {
        ReversedBinaryToInt(&calCap, ASD_CAL_CAP, 3, statusSetupCheck);
        sprintf(str, "Run %08u with calibration capacitor %d (fF)\n", runNumber, 50*(calCap+1));
      }
      else if (checkSystemNoise || mappingMDTChamber) {
        MDTChamberOutput1("\n");
        sprintf(str, "Run %08u with checkSystemNoise/mappingMDTChamber flag on\n", runNumber);
	  }
      else {
        sprintf(str, "Data acquisition is started for run %08u and expected trigger rate is %.2f (Hz)\n", runNumber, ExpectedTriggerRate());
      }
      MDTChamberOutput2("%s", str);
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, FALSE);
      DAQControlInforColor();
      if (verifyFPGAAndPROM) restartRunAfterStop = TRUE;
      pauseRunAsRequested = FALSE;
      SetSleepPolicy(VAL_SLEEP_NONE);
      SaveRunLogfile();             // Save run information to a log file
      // Reset for trigger rate measurement
      measureTriggerRate = FALSE;
      MeasuredTTCviTriggerRate(&i);
      measureTriggerRate = TRUE;
      TTCviRestoreTrigger();
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Start Run Button",
                 "Push this button to start run for DAQ.");
    controlOption = NORM;
  }
}


void StopDAQ(void) {
  int GOLANumber, status, mezz, edgeMode, pairMode, i, runType, scanRun;
  float time0, dtime;
  
  if (controlOption == NORM) {
    if (runState == State_Paused) ResumeDAQ();  // Resume run if paused
    runState = State_Idle;
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_RUNNUMBER, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, TRUE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_LABEL_TEXT, "Start Run");
    // Stop run sequence
    // 1) Disable Trigger
    // 2) Stop CSM
    // 3) Stop GOLA card
    TTCviSetUnusedTrigger();
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Stop CSM DAQ");
    StartOrStopCSMDAQ();
    NumberConnectedTCPNodes();
    if (DAQTCPConnected) {
      edgeMode = FALSE;
      pairMode = FALSE;
      for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
        if (lEdgeOn[mezz] || tEdgeOn[mezz]) edgeMode = TRUE;
        if (pairOn[mezz]) pairMode = TRUE;
      }
      time0 = (float) Timer();
      dtime = 0.0;
      while (dtime < 0.5) {
        HandleUserButtons(Quit);
        dtime = ((float) Timer()) - time0;
      }
      toTCPClients = TRUE;
      GOLANumber = -1;
      status = SendTCPCommand(GOLANumber, CMDSTOPRUN);
      if (status != 0) status = SendTCPCommand(GOLANumber, CMDSTOPRUN);
      if (status != 0) {
        if ((autoRunOn || (autoRunFromControlFile == 1)) && (DAQTime < 2000)) {
          MDTChamberOutput2("Unable to stop DAQ for run %u, rerun is necessary\n", runNumber);
          reqRerun = TRUE;
        }
        else {
          MDTChamberOutput2("Unable to stop DAQ for run %u, DAQ at Linux side may be stucked.\n", runNumber);
        }
      }
      status = SendShortTCPCommand(GOLANumber, CMDREQALLDAQINFOR);
      if (status != 0) status = SendShortTCPCommand(GOLANumber, CMDREQALLDAQINFOR);
      if (status != 0) {
        MDTChamberOutput2("Unable to get full DAQ monitor information for run %u\n", runNumber);
      }
      status = SendShortTCPCommand(GOLANumber, CMDREQEDGES);
      if (status != 0) status = SendShortTCPCommand(GOLANumber, CMDREQEDGES);
      if ((status != 0) && edgeMode) {
        if (reqThresholdScan || reqInjectionScan) {
          MDTChamberOutput2("Unable to get edge counts for run %u, rerun is necessary\n", runNumber);
          reqRerun = TRUE;
        }
        else if (checkSystemNoise || mappingMDTChamber) {
          if (DAQTime < 2000) {
            MDTChamberOutput2("Unable to get edge counts for run %u, rerun is necessary\n", runNumber);
            reqRerun = TRUE;
          }
          else {
            MDTChamberOutput2("Unable to get edge counts for run %u, noise/efficiency measurement will be effected.\n", runNumber);
            MDTChamberOutput1("Using data analysis program (tdcSpectrum.job) to recover and check noise/effi. measurement.\n");
          }
        }
        else {
          MDTChamberOutput2("Unable to get edge counts for run %u\n", runNumber);
        }
      }
      status = SendShortTCPCommand(GOLANumber, CMDREQPAIR);
      if (status != 0) status = SendShortTCPCommand(GOLANumber, CMDREQPAIR);
      if ((status != 0) && pairMode) {
        if (reqThresholdScan || reqInjectionScan) {
          MDTChamberOutput2("Unable to get pair counts for run %u, rerun is necessary\n", runNumber);
          reqRerun = TRUE;
        }
        else if (checkSystemNoise || mappingMDTChamber) {
          if (DAQTime < 2000) {
            MDTChamberOutput2("Unable to get pair counts for run %u, rerun is necessary\n", runNumber);
            reqRerun = TRUE;
          }
          else {
            MDTChamberOutput2("Unable to get pair counts for run %u, noise/efficiency measurement will be effected.\n", runNumber);
            MDTChamberOutput1("Using data analysis program (tdcSpectrum.job) to recover and check noise/effi. measurement.\n");
          }
        }
        else {
          MDTChamberOutput2("Unable to get edge counts for run %u\n", runNumber);
        }
      }

      if (checkSystemNoise || mappingMDTChamber) {
        status = SendShortTCPCommand(GOLANumber, CMDREQHITS);
        if (status != 0) status = SendShortTCPCommand(GOLANumber, CMDREQHITS);
        if (status != 0) {
          if (DAQTime < 2000) {
            MDTChamberOutput2("Unable to get large width hit counts for run %u, rerun is necessary\n", runNumber);
            reqRerun = TRUE;
          }
          else {
            MDTChamberOutput2("Unable to get large width hit counts for run %u, noise/efficiency measurement will be effected.\n", runNumber);
            MDTChamberOutput1("Using data analysis program (tdcSpectrum.job) to recover and check noise/effi. measurement.\n");
          }
        }
      }

      if (reqLinearityScan) {
        status = SendShortTCPCommand(GOLANumber, CMDREQAVERAGETDCTIME);
        if (status != 0) status = SendShortTCPCommand(GOLANumber, CMDREQAVERAGETDCTIME);
        if (status != 0) {
          MDTChamberOutput2("Unable to get average TDC time and its resolution for run %u, rerun is necessary\n", runNumber);
          reqRerun = TRUE;
        }
      }

      if (reqGainScan) {
        status = SendShortTCPCommand(GOLANumber, CMDREQAVERAGEWIDTH);
        if (status != 0) status = SendShortTCPCommand(GOLANumber, CMDREQAVERAGEWIDTH);
        if (status != 0) {
          MDTChamberOutput2("Unable to get average width and its resolution for run %u, rerun is necessary\n", runNumber);
          reqRerun = TRUE;
        }
      }
    
      if (reqLinearityScan || reqGainScan) {
        status = SendShortTCPCommand(GOLANumber, CMDREQFRACTIONUSED);
        if (status != 0) status = SendShortTCPCommand(GOLANumber, CMDREQFRACTIONUSED);
        if (status != 0) {
          MDTChamberOutput2("Unable to get fraction used hits to compute resolutions for run %u\n", runNumber);
        }
      }
    }
    status = SendShortTCPCommand(GOLANumber, CMDPROBE);   // Send PROBE command at end

    if (SetDir("dat") == 0) SetDir("..");
    else MakeDir("dat");
    SaveRunLogfile();             // Save run information to a log file
    SaveErrorSummaryFile();
    if (totalNumberEvent != 0 || totalNumberEventHigh != 0) {
      if (DAQTime > 120) {
        UpErrorReport();
        SaveResultFile();
      }
      HidePanel(ErrorReportHandle);
      SaveScanResults();
      SaveEfficiencyAndNoiseRate();
      if (autoRunFromControlFile == 0) {
        scanOutputFormat = 0;
        triggerRate = -1;
      }
    }
    else if (autoRunOn || (autoRunFromControlFile == 1)) {
      reqRerun = TRUE;
      MDTChamberOutput2("Did not get any event from current run (%u), rerun is necessary.\n", runNumber); 
    }
    scanRun = reqThresholdScan || reqInjectionScan || reqLinearityScan || reqGainScan;
    if (reqRerun) {
      if (scanRun) numberBadScanRun++;
      if (autoRunOn || (autoRunFromControlFile == 1)) numberBadAutoRun++;
    }
    ResetTextBox(DAQControlHandle, P_JTAGCTRL_STOPTIME, DateTimeString(time(NULL)));
    ResetTextBox(DAQInforHandle, P_DAQINFOR_STOPTIME, DateTimeString(time(NULL)));
    for (i = 0; i < NUMBERANALYSISCOUNTER; i++) analysisCounter[i] = 0;
    analysisCounter[MINEVENTSIZE] = 1000000;
    initDAQDoneForRun = FALSE;
    printf("Data acquisition is stopped!\n");
    DAQControlInforColor();
    GetCtrlVal(DAQControlHandle, P_JTAGCTRL_RUNTYPE, &runType);
    if (runType != SPECIALCHECKFPGAANDPROM) SetSleepPolicy(VAL_SLEEP_SOME);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Stop Run Button",
                 "Push this button to stop run for DAQ.");
    controlOption = NORM;
  }
}


// Pause or resume run depending run status
void PauseOrResumeDAQ(void)
{
  if (runState == State_Running) PauseDAQ();
  else if (runState == State_Paused) ResumeDAQ();
}


void PauseDAQ(void) {
  if (controlOption == NORM) {
    if (runState == State_Running) {
      TTCviSetUnusedTrigger();
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_LABEL_TEXT, "Resume Run");
      SendTCPCommand(-1, CMDPAUSERUN);
      runState = State_Paused;
    }
    if (runState != State_Idle)
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, FALSE);
    DAQControlInforColor();
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Pause Run Button",
                 "Push this button to pause run for DAQ.");
    controlOption = NORM;
  }
}


void ResumeDAQ(void) {
  if (controlOption == NORM) {
    if (runState == State_Paused) {
      TTCviRestoreTrigger();
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_LABEL_TEXT, "Pause Run");
      SendTCPCommand(-1, CMDRESUMERUN);
      runState = State_Running;
    }
    if (runState != State_Idle)
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, FALSE);
    DAQControlInforColor();
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Resume Run Button",
                 "Push this button to resume run for DAQ.");
    controlOption = NORM;
  }
}


void InitDAQ(void) {
  static int panel, firstInitDAQ = TRUE;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    if (panel == SelectActionHandle) PanelDone();
    if (runState != State_Idle) StopDAQ();     // Stop run if running/paused
    if (firstInitDAQ) {
      if (CSMSetupStatus() != 0) GetCSMStatus();
      if (CSMSetupStatus() != 0) {
        printf("Unable to get CSM Status correctly, try again ...\n");
        GetCSMStatus();
      }
      if (CSMSetupStatus() == 0) {
        firstInitDAQ = FALSE;
        UpdateCSMControlBits();
        UpdateCSMSetupPanel();
        CreatGOLLDList();
        PutCSMNextState();
      }
    }
    GetRunCondition();
    // Init DAQ procedure
    // 1) Disable trigger;
    // 2) InitCSMDAQ
    // 3) Init GOLA Card
    // 4) EVID Reset
    // 5) BCID Reset
    // 6) Global Reset
    printf("Start to initialize DAQ.\n");
    if (gotTTCvi) {
      SetDAQRunType();
      TTCviSetUnusedTrigger();
    }
    InitCSMDAQ();
    CheckAMTDataMode(FALSE);
    SendTCPCommand(-1, CMDINITDAQ);
    if (gotTTCvi) {
      TTCviEventCounterReset();
      TTCviBunchCounterReset();
      TTCviEventAndBunchCounterReset();
    }
    initDAQDone = TRUE;
    initDAQDoneForRun = TRUE;
    numberInitDAQ++;
    if (CSMBoardStatus() != 0) {
      initDAQStatus = 1;
      numberBadInitDAQ++;
    }
    else initDAQStatus = 0;
    printf("DAQ initialization done.\n");
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on INITDAQ Button",
                 "Push this button to perform DAQ initialization.");
    controlOption = NORM;
  }
}


void SearchJTAGDriver(void) {
  int resp;
  
  resp = ConfirmPopup("Confirm to Search JTAG Driver", 
                      "Are you sure to search for JTAG driver?\n");
  if (resp == 0) return;
  gotJTAGDriver = FALSE;
  if (JTAGDRIVERTYPE == XILNIXPCABLE) gotJTAGDriver = TRUE;
  else if (JTAGDRIVERTYPE == CANELMB) {
    CANNode = -1;
    ELMBNode = -1;
    gotJTAGDriver = TRUE;
  }
  else if (canUseVME) gotJTAGDriver = TRUE;
  if (gotJTAGDriver) {
    RefindJTAGControllerBase();
    JTAGControlStartUp();
    InitJTAG();
    SetJTAGRateDivisorButton();
    AutoSetupJTAGChain();
  }
}


void SetDAQRunType(void) {
  static int i, runType = NORMALALLCALIBCHANNELOFF, newRunType;
  
  if (controlOption == NORM) {
    GetCtrlVal(DAQControlHandle, P_JTAGCTRL_RUNTYPE, &newRunType);
    if (requestBackNormal) {
      if (newRunType == SPECIALCHECKFPGAANDPROM) requestBackNormal = FALSE;
      runType = newRunType;
      return;
    }
    else if ((newRunType != runType) && (runType == SPECIALCHECKFPGAANDPROM)) {
      requestBackNormal = TRUE;
      runType = newRunType;
      return;
    }
    else if ((newRunType != runType) && (newRunType == SPECIALCHECKFPGAANDPROM)) {
      if ((verifyFPGAAndPROM = ConfirmToVerifyFPGAAndPROM())) {
        if (runState != State_Idle) StopDAQ();     // Stop run if running/paused
        VFPStartTime = time(NULL);
        for (i = 0; i < VFPNUMBERCOUNTER; i++) VFPCounter[i] = 0;
        GetCtrlVal(ConfirmVFPHandle, P_SELVFP_VERIFYFPGATIME, &verifyFPGATime);
        GetCtrlVal(ConfirmVFPHandle, P_SELVFP_VERIFYPROMTIME, &verifyPROMTime);
        GetCtrlVal(ConfirmVFPHandle, P_SELVFP_PAUSEDAQTOVERIFYFPGA, &pauseDAQToVerifyFPGA);
        GetCtrlVal(ConfirmVFPHandle, P_SELVFP_PAUSEDAQTOVERIFYPROM, &pauseDAQToVerifyPROM);
        if (verifyPROMTime < 300) verifyPROMTime = 300;
        SetSleepPolicy(VAL_SLEEP_NONE);
        runType = newRunType;
        SetupJTAGChain();
        if ((!gotPROM) || (!gotCSM)) SetupJTAGChain();
        if (gotPROM && gotCSM) {
          ResetCSM();
          InitDAQ();
        }
        SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_NEWDOSE, ATTR_DIMMED, FALSE);
        SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_NEWDOSE, ATTR_VISIBLE, TRUE);
      }
      else SetCtrlVal(DAQControlHandle, P_JTAGCTRL_RUNTYPE, runType);
    }
    else if (newRunType != SPECIALCHECKFPGAANDPROM) {
      verifyFPGAAndPROM = FALSE;
      SetSleepPolicy(VAL_SLEEP_SOME);
      runType = newRunType;
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_NEWDOSE, ATTR_DIMMED, TRUE);
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_NEWDOSE, ATTR_VISIBLE, FALSE);
    }
    SetupJTAGRelatedControlButtons();
    if (!gotTTCvi) return;
    if ((runType == CALIBALLCALIBCHANNELON) || (runType == CALIBKEEPCALIBCHANNEL)) TTCviCalibrationRun();
    else TTCviNormalDAQRun();
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Run Type",
                 "Use this button to select run type.");
    controlOption = NORM;
  }
}


void SetRunNumberButton(void) {
  int newRun;
  
  GetCtrlVal(DAQControlHandle, P_JTAGCTRL_RUNNUMBER, &newRun);
  if (newRun != runNumber) LoopAndCheckUserButtonAndFunction(SetRunNumberButton, SetRunNumber);
}


void SetRunNumber(void) {
  int newRun, resp;
  char str[256];
  
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_RUNNUMBER, &newRun);
  if (newRun != runNumber) {
    sprintf(str, "Are you sure to change run number from %d to %d?\n", runNumber, newRun);
    resp = ConfirmPopup("Confirm Changing Run Number", str);
    if (resp != 0) {
      newRunNumber = TRUE;
      GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_RUNNUMBER, &runNumber);
      SaveParameters(PARARUNNUMBER);
    }
  }
}


void NewDose(void) {
  char str[256];
  
  sprintf(str, "=== Add radiation dose at test loop %u ===\n", VFPCounter[VFPNUMBERVERIFYLOOP]);
  WriteToFPGAAndPROMLogFile(str);
}


void DisconnectAllTCPClients(void) {
  int resp, i;
  char str[40];
  
  if (controlOption == NORM) {
    if (runDiagnostics) {
      resp = ConfirmPopup("Confirm to Quit Diagnostics Run",
                          "Are you sure to quit diagnostics run?\n");
      if (resp != 0) runDiagnostics = FALSE;
    }
    else if (checkMezzOnOff) {
      resp = ConfirmPopup("Confirm to Stop Mezz On/Off Checking",
                          "Are you sure to stop mezzanine card on and off checking?\n");
      if (resp != 0) {
        checkMezzOnOff = FALSE;
        MDTChamberOutput2("Total Number of Download CSM        = %30d\n", numberCSMDownload);
        MDTChamberOutput2("Total Number of Failed Download CSM = %30d\n", numberBadCSMDownload);
        MDTChamberOutput2("Total Number of Download MezzCards  = %30d\n", numberMezzDownload);
        MDTChamberOutput2("Total Number of Failed Download Mezz= %30d\n", numberBadMezzDownload);
        MDTChamberOutput2("Total Number of Init DAQ            = %30d\n", numberInitDAQ);
        MDTChamberOutput2("Total Number of Failed Init DAQ     = %30d\n", numberBadInitDAQ);
        MDTChamberOutput2("Total Number of Redone Init DAQ     = %30d\n", numberReInitDAQ);
        MDTChamberOutput2("Total Number of Auto Runs           = %30d\n", numberAutoRun);
        MDTChamberOutput2("Total Number of Bad Auto Runs       = %30d\n", numberBadAutoRun);
        MDTChamberOutput2("Total Number of Rerun in Auto Runs  = %30d\n", numberAutoRerun);
        if (MDTChamberTest) {
          MDTChamberOutput1(">>>>>>>>>>  End MDT Chamber Tests <<<<<<<<<<\n\n");
          MDTChamberTest = FALSE;
        }
        if (MDTChamberSumFile != NULL) {
          fclose(MDTChamberSumFile);
          MDTChamberSumFile = NULL;
        }
      }
    }
    else if (verifyFPGAAndPROM) {
      GetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, str);
      for (i = 0; i <= strlen(str); i++) str[i] = tolower(str[i]);
      if (((char *) strstr(str, "program prom")) != NULL) {
        resp = ConfirmPopup("Confirm to Program PROM",
                            "Request to program PROM, are you sure?\n");
        if (resp != 0) requestProgramPROM = TRUE;
      }
      else if (((char *) strstr(str, "program fpga")) != NULL) {
        resp = ConfirmPopup("Confirm to Program FPGA from PROM",
                            "Request to program FPGA from PROM, are you sure?\n");
        if (resp != 0) requestProgramFPGA = TRUE;
      }
      else if (((char *) strstr(str, "resetcsm")) != NULL) {
        resp = ConfirmPopup("Confirm to Reset CSM & INIT DAQ",
                            "Request to reset CSM and perform DAQ initialization, are you sure?\n");
        if (resp != 0) requestResetINITDAQ = TRUE;
      }
      else if (((char *) strstr(str, "request to stop run")) != NULL) {
        resp = ConfirmPopup("Confirm to Stop Run",
                            "Request to stop run, are you sure?\n");
        if (resp != 0) {
          SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, TRUE);
          requestStartOrStop = TRUE;
          restartRunAfterStop = FALSE;
        }
      }
      else {
        // Do nothing
      }
    }
    else if (autoRunOn || (autoRunFromControlFile == 1)) {
      resp = ConfirmPopup("Confirm to Stop Auto Run",
                          "Are you sure to stop auto run?\n");
      if (resp != 0) {
        while (autoRunFromControlFile == 1) GetNewRunSetups(0);
        if (runState != State_Idle) StopDAQ();     // Stop run if running/paused
        checkCSM = 0;
        checkCSMJTAG = 0;
        checkCSMIO = 0;
        MDTChamberControl = FALSE;
        autoRunOn = FALSE;
        useExternalTrigger = FALSE;
        thresholdScanMethod = 0;
        numberSubScans = 1;
        subScans = numberSubScans - 1;
        MDTChamberOutput2("Total Number of Download CSM        = %30d\n", numberCSMDownload);
        MDTChamberOutput2("Total Number of Failed Download CSM = %30d\n", numberBadCSMDownload);
        MDTChamberOutput2("Total Number of Download MezzCards  = %30d\n", numberMezzDownload);
        MDTChamberOutput2("Total Number of Failed Download Mezz= %30d\n", numberBadMezzDownload);
        MDTChamberOutput2("Total Number of Init DAQ            = %30d\n", numberInitDAQ);
        MDTChamberOutput2("Total Number of Failed Init DAQ     = %30d\n", numberBadInitDAQ);
        MDTChamberOutput2("Total Number of Redone Init DAQ     = %30d\n", numberReInitDAQ);
        MDTChamberOutput2("Total Number of Auto Runs           = %30d\n", numberAutoRun);
        MDTChamberOutput2("Total Number of Bad Auto Runs       = %30d\n", numberBadAutoRun);
        MDTChamberOutput2("Total Number of Rerun in Auto Runs  = %30d\n", numberAutoRerun);
        if (MDTChamberTest) {
          MDTChamberOutput1(">>>>>>>>>>  End MDT Chamber Tests <<<<<<<<<<\n\n");
          MDTChamberTest = FALSE;
        }
        if (MDTChamberSumFile != NULL) {
          fclose(MDTChamberSumFile);
          MDTChamberSumFile = NULL;
        }
      }
    }
    else {
      resp = ConfirmPopup("Confirm to Disconnect All TCP Clients",
                          "Are you sure to disconnect all TCP clients?\n");
      if (resp != 0) {
        if (runState != State_Idle) StopDAQ();     // Stop run if running/paused
        WaitInMillisecond(200);
        SendTTCviTCPCommand(CMDQUIT, 0, 0);
        if (SendTCPCommand(-1, CMDQUIT) != 0) DisconnectSockets();
      }
    }
  }
  else if (controlOption == HELP) {
    if (runDiagnostics) {
      MessagePopup("Help on Quit Diagnostics Run",
                   "Use this button to quit from diagnostics run if confirmed.");
    }
    else if (checkMezzOnOff) {
      MessagePopup("Help on Stop Check Mezz On/Off",
                   "Use this button to stop check mezzanine card on/off if confirmed.");
    }
    else if (verifyFPGAAndPROM) {
      GetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_LABEL_TEXT, str);
      for (i = 0; i <= strlen(str); i++) str[i] = tolower(str[i]);
      if (((char *) strstr(str, "program prom")) != NULL) {
        MessagePopup("Help on Program PROM",
                     "Use this button to request program PROM if confirmed.");
      }
      else if (((char *) strstr(str, "program fpga")) != NULL) {
        MessagePopup("Help on Program FPGA from PROM",
                     "Use this button to request program FPGA from PROM if confirmed.");
      }
      else if (((char *) strstr(str, "resetcsm")) != NULL) {
        MessagePopup("Help on Reset CSM & INIT DAQ",
                     "Use this button to request reset CSM & perform DAQ initialization if confirmed.");
      }
      else if (((char *) strstr(str, "request to stop run")) != NULL) {
        MessagePopup("Help on Request to Stop Run",
                     "Use this button to request stop run if confirmed.");
      }
      else {
        MessagePopup("Help on Disconnect All TCP Clients",
                     "Use this button to disconnect all TCP clients if confirmed.");
      }
    }
    else if (autoRunOn || (autoRunFromControlFile == 1)) {
      MessagePopup("Help on Stop Auto Run",
                   "Use this button to quit from auto run if confirmed.");
    }
    else {
      MessagePopup("Help on Disconnect All TCP Clients",
                   "Use this button to disconnect all TCP clients if confirmed.");
    }
    controlOption = NORM;
  }
}


void GetRawData(void) {
  int GOLANumber, status, ntry, first;
  double time0, dtime;
  
  runDiagnostics = TRUE;
  eventReady = FALSE;
  GOLANumber = -1;
  if (!getOneEventInitDone) {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Stop CSM DAQ");
    StartOrStopCSMDAQ();
    if (gotTTCvi) {
      SetDAQRunType();
      TTCviSetUnusedTrigger();
    }
    if (SendTCPCommand(GOLANumber, CMDGETONEEVENTINIT) != 0) {
      printf("Unable to init GOLA Card, please check the system at UNIX\n");
      return;
    }
    getOneEventInitDone = TRUE;
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Start CSM DAQ");
    StartOrStopCSMDAQ();
    TTCviEventCounterReset();
    TTCviBunchCounterReset();
    TTCviRestoreTrigger();
    TTCviOneShotTriggerIfSelected();
  }
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, TRUE);
  time0 = (float) Timer();
  dtime = 0.0;
  while (dtime < 0.2) {
    HandleUserButtonAndFunction(PanelDone);
    TTCviOneShotTriggerIfSelected();
    dtime = ((float) Timer()) - time0;
  }
  TCPData[TCPCOMMAND] = 0xC0000 | CMDREQRAWDATA;
  TCPData[TCPLENGTH] = 2;
  if (GOLANumber < 0) status = SendToClients(TRUE, UNIX_PORT_NUM);
  else status = 0;
  ntry = 0;
  while (runDiagnostics && (status != 0) && (ntry < 3)) {
    time0 = (float) Timer();
    dtime = 0.0;
    while (dtime < 0.2) {
      HandleUserButtonAndFunction(PanelDone);
      TTCviOneShotTriggerIfSelected();
      dtime = ((float) Timer()) - time0;
    }
    GOLANumber = -1;
    TCPData[TCPCOMMAND] = 0xC0000 | CMDREREQRAWDATA;
    TCPData[TCPLENGTH] = 2;
    if (GOLANumber < 0) {
      if (ntry == 1) status = SendToClients(FALSE, UNIX_PORT_NUM);
      else status = SendToClients(TRUE, UNIX_PORT_NUM);
    }
    else status = 0;
    ntry++;
    HandleUserButtonAndFunction(PanelDone);
  }
  first = TRUE;
  while (runDiagnostics && (status == 0) && (!eventReady)) {
    if (first) {
      first = FALSE;
      TTCviRestoreTrigger();
    }
    TTCviOneShotTriggerIfSelected();
    GetRawData();
    HandleUserButtonAndFunction(PanelDone);
  }
  if (eventReady || (!runDiagnostics)) {
    TTCviSetUnusedTrigger();
    runDiagnostics = FALSE;
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, FALSE);
  }
  else TTCviRestoreTrigger();
  SetActivePanel(DAQControlHandle);
}


void GetOneEvent(void) {
  int GOLANumber, status, ntry, first;
  double time0, dtime;
  
  runDiagnostics = TRUE;
  eventReady = FALSE;
  GOLANumber = -1;
  if (!getOneEventInitDone) {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Stop CSM DAQ");
    StartOrStopCSMDAQ();
    if (gotTTCvi) {
      SetDAQRunType();
      TTCviSetUnusedTrigger();
    }
    if (SendTCPCommand(GOLANumber, CMDGETONEEVENTINIT) != 0) {
      printf("Unable to init GOLA Card, please check the system at UNIX\n");
      return;
    }
    getOneEventInitDone = TRUE;
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Start CSM DAQ");
    StartOrStopCSMDAQ();
    TTCviEventCounterReset();
    TTCviBunchCounterReset();
    TTCviRestoreTrigger();
    TTCviOneShotTriggerIfSelected();
  }
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, TRUE);
  time0 = (float) Timer();
  dtime = 0.0;
  while (dtime < 0.2) {
    HandleUserButtonAndFunction(PanelDone);
    TTCviOneShotTriggerIfSelected();
    dtime = ((float) Timer()) - time0;
  }
  TCPData[TCPCOMMAND] = 0xC0000 | CMDREQONEEVENT;
  TCPData[TCPLENGTH] = 2;
  if (GOLANumber < 0) status = SendToClients(TRUE, UNIX_PORT_NUM);
  else status = 0;
  ntry = 0;
  while (runDiagnostics && (status != 0) && (ntry < 3)) {
    time0 = (float) Timer();
    dtime = 0.0;
    while (dtime < 0.2) {
      HandleUserButtonAndFunction(PanelDone);
      TTCviOneShotTriggerIfSelected();
      dtime = ((float) Timer()) - time0;
    }
    GOLANumber = -1;
    TCPData[TCPCOMMAND] = 0xC0000 | CMDREREQONEEVENT;
    TCPData[TCPLENGTH] = 2;
    if (GOLANumber < 0) {
      if (ntry == 1) status = SendToClients(FALSE, UNIX_PORT_NUM);
      else status = SendToClients(TRUE, UNIX_PORT_NUM);
    }
    else status = 0;
    ntry++;
    HandleUserButtonAndFunction(PanelDone);
  }
  first = TRUE;
  while (runDiagnostics && (status == 0) && (!eventReady)) {
    if (first) {
      first = FALSE;
      TTCviRestoreTrigger();
    }
    TTCviOneShotTriggerIfSelected();
    GetOneEvent();
    HandleUserButtonAndFunction(PanelDone);
  }
  if (eventReady || (!runDiagnostics)) {
    TTCviSetUnusedTrigger();
    runDiagnostics = FALSE;
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, FALSE);
  }
  else TTCviRestoreTrigger();
  SetActivePanel(DAQControlHandle);
}


void GetOneSequentialEvent(void) {
  int GOLANumber, status, ntry, first;
  double time0, dtime;
  
  runDiagnostics = TRUE;
  eventReady = FALSE;
  GOLANumber = -1;
  if (!getOneSequentialEventInitDone) {
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Stop CSM DAQ");
    StartOrStopCSMDAQ();
    if (gotTTCvi) {
      SetDAQRunType();
      TTCviSetUnusedTrigger();
    }
    if (SendTCPCommand(GOLANumber, CMDGETONEEVENTINIT) != 0) {
      printf("Unable to init GOLA Card, please check the system at UNIX\n");
      return;
    }
    getOneSequentialEventInitDone = TRUE;
    SetCtrlAttribute(JTAGControlHandle, P_JTAGCTRL_STARTORSTOP, ATTR_LABEL_TEXT, "Start CSM DAQ");
    StartOrStopCSMDAQ();
    TTCviEventCounterReset();
    TTCviBunchCounterReset();
    TTCviRestoreTrigger();
    TTCviOneShotTriggerIfSelected();
  }
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, TRUE);
  time0 = (float) Timer();
  dtime = 0.0;
  while (dtime < 0.2) {
    HandleUserButtonAndFunction(PanelDone);
    TTCviOneShotTriggerIfSelected();
    dtime = ((float) Timer()) - time0;
  }
  TCPData[TCPCOMMAND] = 0xC0000 | CMDREQONESEQEVENT;
  TCPData[TCPLENGTH] = 2;
  if (GOLANumber < 0) status = SendToClients(TRUE, UNIX_PORT_NUM);
  else status = 0;
  ntry = 0;
  while (runDiagnostics && (status != 0) && (ntry < 3)) {
    time0 = (float) Timer();
    dtime = 0.0;
    while (dtime < 0.2) {
      HandleUserButtonAndFunction(PanelDone);
      TTCviOneShotTriggerIfSelected();
      dtime = ((float) Timer()) - time0;
    }
    GOLANumber = -1;
    TCPData[TCPCOMMAND] = 0xC0000 | CMDREREQONESEQEVENT;
    TCPData[TCPLENGTH] = 2;
    if (GOLANumber < 0) {
      if (ntry == 1) status = SendToClients(FALSE, UNIX_PORT_NUM);
      else status = SendToClients(TRUE, UNIX_PORT_NUM);
    }
    else status = 0;
    ntry++;
  }
  first = TRUE;
  while (runDiagnostics && (status == 0) && (!eventReady)) {
    if (first) {
      first = FALSE;
      TTCviRestoreTrigger();
    }
    TTCviOneShotTriggerIfSelected();
    GetOneSequentialEvent();
    HandleUserButtonAndFunction(PanelDone);
  }
  if (eventReady || (!runDiagnostics)) {
    TTCviSetUnusedTrigger();
    runDiagnostics = FALSE;
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, FALSE);
  }
  else TTCviRestoreTrigger();
  SetActivePanel(DAQControlHandle);
}


int CheckClientInstallations(int warningControl) {
  int stat, resp;
  char str[8192];

  NumberConnectedTCPNodes();
  if (!DAQTCPConnected) return 0;
  stat = SendShortTCPCommand(-1, CMDDETAILSTATUS);
  if (stat != 0) SendShortTCPCommand(-1, CMDDETAILSTATUS);
  if (warningControl <= 0) return 0;
  if ((analysisControl <= 0) && (!reqProcessData)) return 0;
  
  stat = 0;
  strcpy(str, "Following warning message are turned on since data analysis is requested\n");
  if (analysisControl > 0) {
    strcat(str, "at run control (to turn off it, please use Execute -> Setup Run Condition)");
    if (reqProcessData) strcat(str, "and");
    else strcat(str, ".");
    strcat(str, "\n");
  }
  if (reqProcessData) {
    strcat(str, "at auto run control file (to turn off it, please modify the auto run control file).\n");
  }
  strcat(str, "\n");
  if (!clientAnaInstalled) {
    strcat(str, "* The data analysis package is not installed at client machine (Linux Box).\n");
    strcat(str, "  To process data after data taking, it is required to install the data analysis\n");
    strcat(str, "  package at Linux Box. Do as following to install data analysis package:\n");
    strcat(str, "  1) Obtain data analysis package for singile chamber (chamber.tar.gz) at\n");
    strcat(str, "     http://atlas.physics.lsa.umich.edu/docushare/dsweb/View/Collection-399;\n");
    strcat(str, "  2) It is recommended to install data analysis package at user's home directory\n");
    strcat(str, "     do as following at Linux box:\n");
    strcat(str, "     A) Copy chamber.tar.gz at user's home direcotory;\n");
    strcat(str, "     B) tar -zxvf chamber.tar.gz, where a directory \"chamber\" will be installed;\n");
    strcat(str, "  3) If data analysis package is not installed at user's home directory (for instance,\n");
    strcat(str, "     installed at another user's home directory), start the CSMMiniDAQ at Linux Box\n");
    strcat(str, "     and modify the text file \"dataAnalysisInstallationPath.txt\" according to\n");
    strcat(str, "     its instruction.\n");
  }
  if (!clientCERNLibInstalled) {
    strcat(str, "* The CERN library is not installed at client machine (Linux Box).\n");
    strcat(str, "  In case user wishes to view data analysis results (histograms), it is necessary to\n");
    strcat(str, "  install CERN library at Linux Box, otherwise ignore it. To install CERN library, ask\n");
    strcat(str, "  your computer adminstrator to help.\n");
  }
  strcat(str, "\n");
  if (warningControl == 3) {
    strcat(str, "Are you sure to perform requested auto runs without data analysis package installed at Linux Box?\n\n");
  }
  else if (warningControl == 2) {
    strcat(str, "Are you sure to start auto runs without data analysis package installed at Linux Box?\n\n");
  }
  else {
    strcat(str, "Are you sure to start run without data analysis package installed at Linux Box?\n\n");
  }
  if (!clientAnaInstalled && ((analysisControl > 0) || reqProcessData)) {
    resp = ConfirmPopup("Confirm to Continue", str);
    if (resp == 1) stat = 0;
    else {
      stat = 1;
      if (warningControl == 3) autoRunFromControlFile = 0;
    }
  }
  
  return stat;
}


int SendTCPCommand(int GOLANumber, int command) {
  int version, nbEvent, toDisk, length, i, data, status, enables, on, INITDimmed;
  float trigRate;
  char fname[256];

  if (!toTCPClients) return 0;
  NumberConnectedTCPNodes();
  if (!DAQTCPConnected) return 0;
  if ((command != CMDREQONEEVENT) && (command != CMDREREQONEEVENT)) getOneEventInitDone = FALSE;
  if ((command != CMDREQONESEQEVENT) && (command != CMDREREQONESEQEVENT)) getOneSequentialEventInitDone = FALSE;
  GetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, &INITDimmed);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, TRUE);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, TRUE);
  TCPData[TCPCOMMAND] = 0xC0000 | (command & 0xFFFF);
  TCPData[TCPCSMNUMBER] = CSMNumber;
  BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
  TCPData[TCPCSMVERSION] = version;
  TCPData[TCPSYNCHWORDCONTROL] = CSMSetupArray[SYNCHWORDCONTROL];
  if (version%2 == 0) TCPData[TCPSYNCHWORDCONTROL] = 0;
  synchWordControl = CSMSetupArray[SYNCHWORDCONTROL];
  GetMezzCardEnables();
  TCPData[TCPMEZZENABLES] = mezzEnables;
  enables = 0;
  if (version < 10) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
      lEdgeOn[i] = 1;
      tEdgeOn[i] = 0;
      pairOn[i] = 0;
    }
  }
  for (i = 0; i < 10; i++) {
    enables |= lEdgeOn[i] << (3*i + LEADINGEDGEENABLE);
    enables |= tEdgeOn[i] << (3*i + TRAILINGEDGEENABLE);
    enables |= pairOn[i] << (3*i + PAIRENABLE);
  }
  TCPData[TCPEDGEPAIRENABLES0] = enables;
  enables = 0;
  for (i = 10; i < MAXNUMBERMEZZANINE; i++) {
    enables |= lEdgeOn[i] << (3*(i%10) + LEADINGEDGEENABLE);
    enables |= tEdgeOn[i] << (3*(i%10) + TRAILINGEDGEENABLE);
    enables |= pairOn[i] << (3*(i%10) + PAIRENABLE);
  }
  TCPData[TCPEDGEPAIRENABLES1] = enables;
  ReadParameters(PARARUNNUMBER);
  trigRate = ExpectedTriggerRate();
  if (command == CMDGETONEEVENTINIT) {
    TCPData[TCPGOLACARDPAGESIZE] = 4;
    if (trigRate >= 100.0) TCPData[TCPGOLACARDPAGESIZE] = 4;
    else if (trigRate > 0.0) TCPData[TCPGOLACARDPAGESIZE] = 3;
  }
  else {
    TCPData[TCPGOLACARDPAGESIZE] = 6;
    if (trigRate >= 1000.0) TCPData[TCPGOLACARDPAGESIZE] = 6;
    else if (trigRate >= 100.0) TCPData[TCPGOLACARDPAGESIZE] = 5;
    else if (trigRate > 0.0) TCPData[TCPGOLACARDPAGESIZE] = 4;
    if (GetTTCviTriggerSelction() == 5) TCPData[TCPGOLACARDPAGESIZE] = 6;
  }
  if (CSMSetupArray[EMPTYCYCLECONTROL] == 0) TCPData[TCPGOLACARDPAGESIZE] = 6;
//  TCPData[TCPAMTTYPE] = TYPEAMT3;

  TCPData[TCPAMTTYPE] = TYPEHPTDC;
  TCPData[TCPSAVERAWDATA] = saveRawData;
  TCPData[TCPSAVERAWDATA] |= (saveFlaggedEvt << 1);
  TCPData[TCPCHAMBERTYPE] = MDTChamberType;
  TCPData[TCPCHAMBERHV] = MDTChamberHVStatus;
  TCPData[TCPSUPPRESSCSMHEADER] = suppressCSMHeader;
  TCPData[TCPSUPPRESSCSMTRAILER] = suppressCSMTrailer;
  TCPData[TCPSUPPRESSAMTHEADER] = suppressTDCHeader;
  TCPData[TCPSUPPRESSAMTTRAILER] = suppressTDCTrailer;
  TCPData[TCPRUNNUMBER] = runNumber;
  nbEvent = 0;
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, &on);
  if (on) GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPATEVENT, &nbEvent);
  TCPData[TCPNUMBEROFEVENT] = nbEvent;
  if (!nominalThresholdUsed) TCPData[TCPNOMINALTHRESHOLD] = 0;
  else TCPData[TCPNOMINALTHRESHOLD] = nominalThreshold + 1024;
  if (integrationGate < 0) TCPData[TCPINTEGRATIONGATE] = 0; 
  else TCPData[TCPINTEGRATIONGATE] = integrationGate + 1024;
  if (rundownCurrent < 0) TCPData[TCPRUNDOWNCURRENT] = 0; 
  else TCPData[TCPRUNDOWNCURRENT] = rundownCurrent + 1024;
  TCPData[TCPWIDTHSELECTION] = 0;
  widthBin = 25.0/32.0;
  if (AMTPairOn) {
    TCPData[TCPWIDTHSELECTION] = widthSelection;
    widthBin *= (float) (1 << widthSelection);
  }
  else if (CSMPairMode || CSMPairDebug) {
    TCPData[TCPWIDTHSELECTION] = CSMPairWidthResolution;
    widthBin *= (float) (1 << CSMPairWidthResolution);
  }
  TCPData[TCPPAIRDEBUG] = CSMPairDebug;
  TCPData[TCPCHECKSYSTEMNOISE] = checkSystemNoise;
  TCPData[TCPMAPPINGMDTCHAMBER] = mappingMDTChamber;
  TCPData[TCPTRIGGERSELECTION] = GetTTCviTriggerSelction();
  TCPData[TCPTRIGGERRATESELECTION] = GetTTCviTriggerRateSelction();
  if ((trigRate > 0.0) && (trigRate < 1.0)) TCPData[TCPEXPECTEDTRIGGERRATE] = 1.0;
  else TCPData[TCPEXPECTEDTRIGGERRATE] = trigRate;
  TCPData[TCPMAXALLOWEDMEZZS] = CSMMaxAllowedMezzs;
  TCPData[TCPANALYSISCONTROL] = analysisControl;
  TCPData[TCPSTARTMEZZCARD] = startMezzCard;
  TCPData[TCPSPARE07] = 0;
  TCPData[TCPSPARE06] = 0;
  TCPData[TCPSPARE05] = 0;
  TCPData[TCPSPARE04] = 0;
  TCPData[TCPSPARE03] = 0;
  TCPData[TCPSPARE02] = 0;
  TCPData[TCPSPARE01] = 0;
  GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, &toDisk);
  length = TCPFILENAME + 1;
  if (toDisk == 0) TCPData[TCPFILENAME] = 0;
  else {
    GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_FILENAME, 0, fname);
    TCPData[TCPFILENAME] = strlen(fname);
    for (i = 0; i < strlen(fname);) {
      data = ((int) fname[i++]);
      data |= ((int) fname[i++]) << 8;
      data |= ((int) fname[i++]) << 16;
      data |= ((int) fname[i++]) << 24;
      TCPData[length++] = data;
    }
  }
  TCPData[TCPLENGTH] = length;
  if (GOLANumber < 0) status = SendToClients(TRUE, UNIX_PORT_NUM);
  else status = 0;
  if (status > 0) {
    printf("Error in sending out TCP command %d, try again\n", command);
    WaitInMillisecond(500);
    if (GOLANumber < 0) {
      status = SendToClients(TRUE, UNIX_PORT_NUM);
      if (status > 0) printf("Error in sending out TCP command %d again\n", command);
    }
    else status = 0;
  }
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, INITDimmed);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, FALSE);
  if (runState != State_Idle)
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, FALSE);
  if (status < 0) status = 0;
  return status;
}


int SendShortTCPCommand(int GOLANumber, int command) {
  int status, ntry;
  float time0, dtime;

  if (runState != State_Idle) {
    printf("DAQ is not stopped yet, not allowed to use SendShortTCPCommand to issue TCP command.\n"); 
  }
  else {
    time0 = (float) Timer();
    dtime = 0.0;
    while (dtime < 0.3) {
      HandleUserButtons(Quit);
      dtime = ((float) Timer()) - time0;
    }
    toTCPClients = TRUE;
    time0 = (float) Timer();
    TCPData[TCPCOMMAND] = 0xC0000 | command;
    TCPData[TCPLENGTH] = 2;
    if (GOLANumber < 0) status = SendToClients(TRUE, UNIX_PORT_NUM);
    else status = 0;
    ntry = 0;
    while ((status != 0) && (ntry < 3)) {
      time0 = (float) Timer();
      dtime = 0.0;
      while (dtime < 0.2) {
        HandleUserButtons(Quit);
        dtime = ((float) Timer()) - time0;
      }
      time0 = (float) Timer();
      TCPData[TCPCOMMAND] = 0xC0000 | command;
      TCPData[TCPLENGTH] = 2;
      if (GOLANumber < 0) status = SendToClients(TRUE, UNIX_PORT_NUM);
      else status = 0;
      ntry++;
    }
    if (status != 0) {   // Will try again by PROBE command
      time0 = (float) Timer();
      dtime = 0.0;
      while (dtime < 0.3) {
        HandleUserButtons(Quit);
        dtime = ((float) Timer()) - time0;
      }
      TCPData[TCPCOMMAND] = 0xC0000 | CMDPROBE;
      TCPData[TCPLENGTH] = 2;
      if (GOLANumber < 0) status = SendToClients(TRUE, UNIX_PORT_NUM);
      else status = 0;
       time0 = (float) Timer();
      dtime = 0.0;
      while (dtime < 0.3) {
        HandleUserButtons(Quit);
        dtime = ((float) Timer()) - time0;
      }
      time0 = (float) Timer();
      TCPData[TCPCOMMAND] = 0xC0000 | command;
      TCPData[TCPLENGTH] = 2;
      if (GOLANumber < 0) status = SendToClients(TRUE, UNIX_PORT_NUM);
      else status = 0;
      ntry = 0;
      while ((status != 0) && (ntry < 3)) {
        time0 = (float) Timer();
        dtime = 0.0;
        while (dtime < 0.2) {
          HandleUserButtons(Quit);
          dtime = ((float) Timer()) - time0;
        }
        time0 = (float) Timer();
        TCPData[TCPCOMMAND] = 0xC0000 | command;
        TCPData[TCPLENGTH] = 2;
        if (GOLANumber < 0) {
          if (ntry == 1) status = SendToClients(FALSE, UNIX_PORT_NUM);
          else status = SendToClients(TRUE, UNIX_PORT_NUM);
        }
        else status = 0;
        ntry++;
      }
    }
  }
  return status;
}


int WaitForTCPClientReply(int port) {
  int status, start;
  
//  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, FALSE);
//  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, FALSE);
  start = (int) time(NULL);
  status = CheckClientsStatus(port);
  while ((status == 11) && ((((int) time(NULL)) - start) < 10)) {
    WaitInMillisecond(1);   // Wait is necessary, otherwise may get fake TCP_DATAREADY signal
    HandleUserButtons(Quit);
    status = CheckClientsStatus(port); 
  }
  WaitInMillisecond(100);
  UpdateTCPStatus(FALSE);
//  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
//  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, TRUE);
  return status;
}


void UpdateTCPStatus(int newConnection) {
  int status, color;
  char str[15];
  
  if (newConnection) {
    color = VAL_YELLOW;
    strcpy(str, "newConnection");
  }
  else {
    status = CheckClientsStatus(UNIX_PORT_NUM);
#ifdef TTCVIDRIVERVIATCP
    if (status != 0) status = CheckClientsStatus(UNIX_PORT_NUM);
#endif
    if (status < 0) {
      color = VAL_MAGENTA;
      strcpy(str, "NoConnection");
    }
    else if (status > 0) {
      color = VAL_RED;
      if (status == 1) strcpy(str, "Timeout");
      else if (status == 2) strcpy(str, "TooManyDW");
      else if (status == 3) strcpy(str, "No DWords");
      else if (status == 4) strcpy(str, "Checksum Err");
      else if (status == 5) strcpy(str, "ClientSTSErr");
      else if (status == 11) strcpy(str, "No Reply");
      else strcpy(str, "TX Failed");
    }
    else {
      color = VAL_GREEN;
      strcpy(str, "TX Success");
    }
  }
  ReplaceTextBoxLine(DAQControlHandle, P_JTAGCTRL_TCPSTATUS, 0, str);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_TCPSTATUS, ATTR_TEXT_BGCOLOR, color);
  SetCtrlVal(DAQControlHandle, P_JTAGCTRL_NBTCPNODES, NumberConnectedTCPNodes());
}


void HandleVariesData(unsigned int data[]) {
  static int command, mezz, ch, pauseTime0, dtime;

  TTCviOneShotTriggerIfSelected();
  dataType = data[1] & DATATYPEBITMASK;
  command = data[3] & 0xFFFF;
  if (dataType == DATATYPEDETAILSTATUS) {
    clientRunState = data[4+CLIENTRUNSTATE];
    clientAnaInstalled = data[4+CLIENTANAINSTALLED];
    clientCERNLibInstalled = data[4+CLIENTCERNLIBINSTALLED];
    clientProcessingData = data[4+CLIENTPROCESSINGDATA];
  }
  else if (dataType == DATATYPEDAQINFOR) CollectDAQInfor(data);
  else if (dataType == DATATYPEALLDAQINFOR) CopyDAQInfor(data);
  else if (dataType == DATATYPERAWDATA) PrintOutData(data, NULL);
  else if (dataType == DATATYPERAWDATACONT) PrintOutData(data, NULL);
  else if (dataType == DATATYPEEVENT) PrintOutData(data, NULL);
  else if (dataType == DATATYPEEVENTCONT) PrintOutData(data, NULL);
  else if (dataType == DATATYPEEDGES) {
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      nEvents[mezz] = data[4+mezz];
      for (ch = 0; ch < 24; ch++) {
        nEdge[0][ch][mezz] = data[4+MAXNUMBERMEZZANINE+48*mezz+ch];
        nEdge[1][ch][mezz] = data[28+MAXNUMBERMEZZANINE+48*mezz+ch];
      }
    }
  }
  else if (dataType == DATATYPEPAIR) {
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      nEvents[mezz] = data[4+mezz];
      for (ch = 0; ch < 24; ch++) {
        nPair[ch][mezz] = data[4+MAXNUMBERMEZZANINE+48*mezz+ch];
      }
    }
  }
  else if (dataType == DATATYPEHITS) {
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      nEvents[mezz] = data[4+mezz];
      for (ch = 0; ch < 24; ch++) {
        nGoodHit[ch][mezz] = data[4+MAXNUMBERMEZZANINE+48*mezz+ch];
        nASDNoise[ch][mezz] = data[28+MAXNUMBERMEZZANINE+48*mezz+ch];
      }
    }
  }
  else if (dataType == DATATYPEAVERAGETDCTIME) {
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      for (ch = 0; ch < 24; ch++) {
        if (data[4+MAXNUMBERMEZZANINE+48*mezz+ch] == 0xFFFFFFFF) timeAverage[ch][mezz] = -1.0;
        else timeAverage[ch][mezz] = ((float) data[4+MAXNUMBERMEZZANINE+48*mezz+ch]) / 1000.;
        if (data[28+MAXNUMBERMEZZANINE+48*mezz+ch] == 0xFFFFFFFF) timeSigma[ch][mezz] = -1.0;
        else timeSigma[ch][mezz] = ((float) data[28+MAXNUMBERMEZZANINE+48*mezz+ch]) / 1000.;
      }
    }
  }
  else if (dataType == DATATYPEFRACTIONUSED) {
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      for (ch = 0; ch < 24; ch++) {
        if (data[4+MAXNUMBERMEZZANINE+48*mezz+ch] == 0xFFFFFFFF) fracUsed[ch][mezz] = -1.0;
        else fracUsed[ch][mezz] = ((float) data[4+MAXNUMBERMEZZANINE+48*mezz+ch]) / 1000.;
        if (data[28+MAXNUMBERMEZZANINE+48*mezz+ch] == 0xFFFFFFFF) fracUsedInWidth[ch][mezz] = -1.0;
        else fracUsedInWidth[ch][mezz] = ((float) data[28+MAXNUMBERMEZZANINE+48*mezz+ch]) / 1000.;
      }
    }
  }
  else if (dataType == DATATYPEAVERAGEWIDTH) {
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      for (ch = 0; ch < 24; ch++) {
        if (data[4+MAXNUMBERMEZZANINE+48*mezz+ch] == 0xFFFFFFFF) widthAverage[ch][mezz] = -1.0;
        else widthAverage[ch][mezz] = ((float) data[4+MAXNUMBERMEZZANINE+48*mezz+ch]) / 1000.;
        if (data[28+MAXNUMBERMEZZANINE+48*mezz+ch] == 0xFFFFFFFF) widthSigma[ch][mezz] = -1.0;
        else widthSigma[ch][mezz] = ((float) data[28+MAXNUMBERMEZZANINE+48*mezz+ch]) / 1000.;
      }
    }
  }
  if (command == CMDPAUSERUN) {
    toTCPClients = FALSE;
    if (runState == State_Running) {
      PauseDAQ();
      pauseTime0 = (int) time(NULL);
      pauseRunAsRequested = TRUE;
      printf("Run has been paused as requested by Client(s)!\n");
    }
  }
  else if (command == CMDRESUMERUN) {
    toTCPClients = FALSE;
    if (runState == State_Paused) ResumeDAQ();
    pauseRunAsRequested = FALSE;
  }
  else if ((command == CMDSTOPRUN) && (runState != State_Idle)) {
    printf("Stop run has been requested by Client(s)!\n");
    runStopStatus = ABNORMALSTOP;
    if (verifyFPGAAndPROM) {   // Request to stop DAQ without restart run
      requestStartOrStop = TRUE;
      restartRunAfterStop = FALSE;
    }
    else requestStopByClient = TRUE;
    requestStopTime0 = (int) time(NULL);
  }
  else if ((runState == State_Paused) && pauseRunAsRequested) {
    dtime = ((int) time(NULL)) - pauseTime0;
    if (dtime > 5) {
      toTCPClients = FALSE;
      ResumeDAQ();
      pauseRunAsRequested = FALSE;
    }
  }
  toTCPClients = TRUE;
  TTCviOneShotTriggerIfSelected();
}


void PrintOutData(unsigned int *data, FILE *file) {
  static int i, index, ndata;
  
  eventReady = TRUE;
  if ((dataType == DATATYPERAWDATA) || (dataType == DATATYPEEVENT)) {
    index = 0;
    if (file == NULL) printf("  Index   dataWord\n");
    else fprintf(file, "  Index   dataWord\n");
  }
  if ((dataType == DATATYPERAWDATA) || (dataType == DATATYPERAWDATACONT)) {
    if (dataType == DATATYPERAWDATA) {
      ndata = 0;
      for (i = 4; i < data[0]; i++) {
        if ((data[i]&SWORDHEADERMASK) == SWORDHEADER) break;
        ndata++;
        if (ndata > MAXNUMBERMEZZANINE) break;
      }
      if ((ndata > 0) && (ndata <= MAXNUMBERMEZZANINE)) {
        if (file == NULL) printf(" %6d 0x%08x  ", index, SWORDHEADER);
        else fprintf(file, " %6d 0x%08x  ", index, SWORDHEADER);
        RawDataInterpretation(SWORDHEADER, file);
        index++;
        for (i = ndata; i < MAXNUMBERMEZZANINE; i++) {
          if (file == NULL) printf(" %6d 0x%08x  ", index, CSMIDLEWORD);
          else fprintf(file, " %6d 0x%08x  ", index, CSMIDLEWORD);
          RawDataInterpretation(CSMIDLEWORD, file);
          index++;
        }
      }
    }
    for (i = 4; i < data[0]; i++) {
      if (file == NULL) printf(" %6d 0x%08x  ", index, data[i]);
      else fprintf(file, " %6d 0x%08x  ", index, data[i]);
      RawDataInterpretation(data[i], file);
      index++;
    }
  }
  else if ((dataType == DATATYPEEVENT) || (dataType == DATATYPEEVENTCONT)) {
    for (i = 4; i < data[0]; i++) {
      if (file == NULL) printf(" %6d 0x%08x  ", index, data[i]);
      else fprintf(file, " %6d 0x%08x  ", index, data[i]);
      DataInterpretation(data[i], file);
      index++;
    }
  }
}


void RawDataInterpretation(unsigned int data, FILE *file) {
  int wordID, subID, EVID, BCID, nword, Chan, Wid, Coarse, Fine, Type, Error;
  static int TDC = -1;
  static unsigned int count = 0, CSMData, CSMStatus, control;
  char CSMParityErr, TXParityErr, overflow1, overflow2;

  wordID = (data>>MAINIDBIT0LOCATION) & MAINIDBITS;
  if ((data&CSMPARITYERRORBIT) != 0) CSMParityErr = 'Y';
  else CSMParityErr = 'N';
  if (checkParity(data, &count) != 0) TXParityErr = 'Y';
  else TXParityErr = 'N';
  if (file == NULL) printf("CSMPErr=%c, TXPErr=%c, ", CSMParityErr, TXParityErr);
  else fprintf(file, "CSMPErr=%c, TXPErr=%c, ", CSMParityErr, TXParityErr);
  if (wordID == ID_SYNCHWORD) {
    count = 0;
    TDC = -1;
    if (synchWordControl) {
      CSMStatus = (data>>SWORDCSMSTATUSBIT0LOC) & SWORDCSMSTATUSBITS;
      control = (data>>SWORDCONTROLBIT0LOC) & SWORDCONTROLBITS;
      CSMData = (data>>SWORDTRIGGERTIMEBIT0LOC) & SWORDTRIGGERTIMEBITS;
      if (file == NULL) printf("SynchWord, status=0x%02x, control=%u, Bit15:0=0x%04x", CSMStatus, control, CSMData);
      else fprintf(file, "SynchWord, status=0x%02x, control=%u, Bit15:0=0x%04x", CSMStatus, control, CSMData);
    }
    else {
      CSMData = (data>>CSMDATABIT0LOCATION) & CSMDATABITS;
      if (file == NULL) printf("SynchWord, Bit25:0=0x%07x", CSMData);
      else fprintf(file, "SynchWord, Bit25:0=0x%07x", CSMData);
    }
  }
  else {
    TDC++;
    TDC %= MAXNUMBERMEZZANINE;
    if (wordID == ID_IDLEWORD) {
      CSMData = (data>>CSMDATABIT0LOCATION) & CSMDATABITS;
      if (file == NULL) printf("CSMIdleWord, Bit25:0=0x%07x", CSMData);
      else fprintf(file, "CSMIdleWord, Bit25:0=0x%07x", CSMData);
    }
    else if (wordID == TDC_HEADER) {
      BCID = (data>>BCIDBIT0LOCATION) & BCIDBITS;
      EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
      if (file == NULL)
        printf("TDC header, TDC=%d, EVID=0x%03x, BCID=0x%03x", TDC, EVID, BCID);
      else
        fprintf(file, "TDC header, TDC=%d, EVID=0x%03x, BCID=0x%03x", TDC, EVID, BCID);
    }
    else if (wordID == TDC_TRAILER) {
      nword = (data>>WORDCOUNTBIT0LOCATION) & WORDCOUNTBITS;
      EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
      if (CSMVersion > 0x20) {
        if (data&CSMFIFOOV1BITS) overflow1 = 'Y';
        else overflow1 = 'N';
        if (data&CSMFIFOOV2BITS) overflow2 = 'Y';
        else overflow2 = 'N';
        if (file == NULL)
          printf("TDC trailer, TDC=%d, EVID=0x%03x, WC=%d, OverF1=%c, OverF2=%c", TDC, EVID, nword, overflow1, overflow2);
        else
          fprintf(file, "TDC trailer, TDC=%d, EVID=0x%03x, WC=%d, OverF1=%c, OverF2=%c", TDC, EVID, nword, overflow1, overflow2);
      }
      else if (file == NULL)
        printf("TDC trailer, TDC=%d, EVID=0x%03x, WC=%d", TDC, EVID, nword);
      else
        fprintf(file, "TDC trailer, TDC=%d, EVID=0x%03x, WC=%d", TDC, EVID, nword);
    }
    else if (wordID == TDC_MASK) {
      if (file == NULL)
        printf("TDC mask, TDC=%d, MaskFlag=0x%06x", TDC, data&0xFFFFFF);
      else
        fprintf(file, "TDC mask, TDC=%d, MaskFlag=0x%06x", TDC, data&0xFFFFFF);
    }
    else if (wordID == TDC_DEBUG) {
      if (file == NULL)
        printf("TDC debug, TDC=%d, 24Bits=0x%06x", TDC, data&0xFFFFFF);
      else
        fprintf(file, "TDC debug, TDC=%d, 24Bits=0x%06x", TDC, data&0xFFFFFF);
    }
    else if (wordID == TDC_ERROR) {
      Error = data&0x3FFF;
      if (file == NULL)
        printf("TDC error, TDC=%d, Errors=0x%04x", TDC, Error);
      else
        fprintf(file, "TDC error, TDC=%d, Errors=0x%04x", TDC, Error);
    }
    else if (wordID == TDC_PAIR) {
      if (file == NULL)
        printf("TDC combined, TDC Chan Wid Coarse Fine =");
      else
        fprintf(file, "TDC combined, TDC Chan Wid Coarse Fine =");
      Chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
      Wid = (data>>PWIDTHBIT0LOCATION) & PWIDTHBITS;
      Coarse = (data>>PCOARSETIMEBIT0LOCATION) & PCOARSETIMEBITS;
      Fine = (data>>FINETIMEBIT0LOCATION) & FINETIMEBITS;
      if (Chan >= 24) {
        if (TDC < 3) {
          Chan -= 24;
          Chan += 8 * TDC;
          TDC = 16;
        }
        else if (TDC > 3 && TDC < 7) {
          Chan -= 24;
          Chan += 8 * (TDC - 4);
          TDC = 17;
        }
      }
      if (file == NULL)
        printf(" %d %d %d %d %d", TDC, Chan, Wid, Coarse, Fine);
      else
        fprintf(file, " %d %d %d %d %d", TDC, Chan, Wid, Coarse, Fine);
    }
    else if (wordID == TDC_EDGE) {
      if (file == NULL)
        printf("TDC edge, TDC Chan Type Error Coarse Fine =");
      else
        fprintf(file, "TDC edge, TDC Chan Type Error Coarse Fine =");
      Chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
      Coarse = (data>>SCOARSETIMEBIT0LOCATION) & SCOARSETIMEBITS;
      Fine = (data>>FINETIMEBIT0LOCATION) & FINETIMEBITS;
      Type = (data>>STYPEBIT0LOCATION) & STYPEBITS;
      Error = (data>>SERRORBIT0LOCATION) & SERRORBITS;
      if (Chan >= 24) {
        if (TDC < 3) {
          Chan -= 24;
          Chan += 8 * TDC;
          TDC = 16;
        }
        else if (TDC > 3 && TDC < 7) {
          Chan -= 24;
          Chan += 8 * (TDC - 4);
          TDC = 17;
        }
      }
      if (file == NULL) printf(" %d %d ", TDC, Chan);
      else fprintf(file, " %d %d ", TDC, Chan);
      if (Type) {
        if (file == NULL) printf("Leading ");
        else fprintf(file, "Leading ");
      }
      else {
        if (file == NULL) printf("Trailing ");
        else fprintf(file, "Trailing ");
      }
      if (Error) {
        if (file == NULL) printf("YES ");
        else fprintf(file, "YES ");
      }
      else {
        if (file == NULL) printf("NO ");
        else fprintf(file, "NO ");
      }
      if (file == NULL) printf("%d %d", Coarse, Fine);
      else fprintf(file, "%d %d", Coarse, Fine);
    }
    else if (wordID == CSM_WORD) {
      subID = (data>>SUBIDBIT0LOCATION) & SUBIDBITS;
      BCID = (data>>BCIDBIT0LOCATION) & BCIDBITS;
      EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
      nword = BCID;
      if (subID == CSM_HEAD) {
        if (file == NULL)
          printf("CSM header, EVID=0x%03x, BCID=0x%03x", EVID, BCID);
        else
          fprintf(file, "CSM header, EVID=0x%03x, BCID=0x%03x", EVID, BCID);
      }
      else if (subID == CSM_GOOD) {
        if (file == NULL)
          printf("CSM trailer, EVID=0x%03x, WC=%d", EVID, nword);
        else
          fprintf(file, "CSM trailer, EVID=0x%03x, WC=%d", EVID, nword);
      }
      else if (subID == CSM_ABORT) {
        if (file == NULL)
          printf("CSM abort, EVID=0x%03x, WC=%d", EVID, nword);
        else
          fprintf(file, "CSM abort, EVID=0x%03x, WC=%d", EVID, nword);
      }
      else if (subID == CSM_PARITY) {
        Error = (data<<18) & 0x3F;
        TDC = data & 0x3FFFF;
        if (file == NULL)
          printf("CSM error, Bit23:18=0x%02x, TDCWParityError=%05X", Error, TDC);
        else
          fprintf(file, "CSM error, Bit23:18=0x%02x, TDCWParityError=%05X", Error, TDC);
      }
      else if (subID == CSM_ERROR) {
        Error = (data<<18) & 0x3F;
        TDC = data & 0x3FFFF;
        if (file == NULL)
          printf("CSM error, Bit23:18=0x%03x, AbnormalTDC=%05X", Error, TDC);
        else
          fprintf(file, "CSM error, Bit23:18=0x%03x, AbnormalTDC=%05X", Error, TDC);
      }
      else {
        if (file == NULL)
          printf("Unknown CSM word");
        else
          fprintf(file, "Unknown CSM word");
      }
    }
    else {
      if (file == NULL)
        printf("Unknown word");
      else
        fprintf(file, "Unknown word");
    }
  }
  if (file == NULL) printf("\n");
  else fprintf(file, "\n");
}


void DataInterpretation(unsigned int data, FILE *file) {
  int wordID, subID, TDC, EVID, BCID, nword, Chan, Wid, Coarse, Fine, Type, Error;

  wordID = (data>>MAINIDBIT0LOCATION) & MAINIDBITS;
  TDC = (data>>TDCNUMBERBIT0LOCATION) & TDCNUMBERBITS;
  if (wordID == TDC_HEADER) {
    BCID = (data>>BCIDBIT0LOCATION) & BCIDBITS;
    EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
    if (file == NULL)
      printf("TDC header, TDC=%d, EVID=0x%03x, BCID=0x%03x", TDC, EVID, BCID);
    else
      fprintf(file, "TDC header, TDC=%d, EVID=0x%03x, BCID=0x%03x", TDC, EVID, BCID);
  }
  else if (wordID == TDC_TRAILER) {
    nword = (data>>WORDCOUNTBIT0LOCATION) & WORDCOUNTBITS;
    EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
    if (file == NULL)
      printf("TDC trailer, TDC=%d, EVID=0x%03x, WC=%d", TDC, EVID, nword);
    else
      fprintf(file, "TDC trailer, TDC=%d, EVID=0x%03x, WC=%d", TDC, EVID, nword);
  }
  else if (wordID == TDC_MASK) {
    if (file == NULL)
      printf("TDC mask, TDC=%d, MaskFlag=0x%06x", TDC, data&0xFFFFFF);
    else
      fprintf(file, "TDC mask, TDC=%d, MaskFlag=0x%06x", TDC, data&0xFFFFFF);
  }
  else if (wordID == TDC_DEBUG) {
    if (file == NULL)
      printf("TDC debug, TDC=%d, 24Bits=0x%06x", TDC, data&0xFFFFFF);
    else
      fprintf(file, "TDC debug, TDC=%d, 24Bits=0x%06x", TDC, data&0xFFFFFF);
  }
  else if (wordID == TDC_ERROR) {
    Error = data&0x3FFF;
    if (file == NULL)
      printf("TDC error, TDC=%d, Errors=0x%04x", TDC, Error);
    else
      fprintf(file, "TDC error, TDC=%d, Errors=0x%04x", TDC, Error);
  }
  else if (wordID == TDC_PAIR) {
    if (file == NULL)
      printf("TDC combined, TDC Chan Wid Coarse Fine =");
    else
      fprintf(file, "TDC combined, TDC Chan Wid Coarse Fine =");
    Chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
    Wid = (data>>PWIDTHBIT0LOCATION) & PWIDTHBITS;
    Coarse = (data>>PCOARSETIMEBIT0LOCATION) & PCOARSETIMEBITS;
    Fine = (data>>FINETIMEBIT0LOCATION) & FINETIMEBITS;
    if (Chan >= 24) {
      if (TDC < 3) {
        Chan -= 24;
        Chan += 8 * TDC;
        TDC = 16;
      }
      else if (TDC > 3 && TDC < 7) {
        Chan -= 24;
        Chan += 8 * (TDC - 4);
        TDC = 17;
      }
    }
    if (file == NULL)
      printf(" %d %d %d %d %d", TDC, Chan, Wid, Coarse, Fine);
    else
      fprintf(file, " %d %d %d %d %d", TDC, Chan, Wid, Coarse, Fine);
  }
  else if (wordID == TDC_EDGE) {
    if (file == NULL)
      printf("TDC edge, TDC Chan Type Error Coarse Fine =");
    else
      fprintf(file, "TDC edge, TDC Chan Type Error Coarse Fine =");
    Chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
    Coarse = (data>>SCOARSETIMEBIT0LOCATION) & SCOARSETIMEBITS;
    Fine = (data>>FINETIMEBIT0LOCATION) & FINETIMEBITS;
    Type = (data>>STYPEBIT0LOCATION) & STYPEBITS;
    Error = (data>>SERRORBIT0LOCATION) & SERRORBITS;
    if (Chan >= 24) {
      if (TDC < 3) {
        Chan -= 24;
        Chan += 8 * TDC;
        TDC = 16;
      }
      else if (TDC > 3 && TDC < 7) {
        Chan -= 24;
        Chan += 8 * (TDC - 4);
        TDC = 17;
      }
    }
    if (file == NULL) printf(" %d %d ", TDC, Chan);
    else fprintf(file, " %d %d ", TDC, Chan);
    if (Type) {
      if (file == NULL) printf("Leading ");
      else fprintf(file, "Leading ");
    }
    else {
      if (file == NULL) printf("Trailing ");
      else fprintf(file, "Trailing ");
    }
    if (Error) {
      if (file == NULL) printf("YES ");
      else fprintf(file, "YES ");
    }
    else {
      if (file == NULL) printf("NO ");
      else fprintf(file, "NO ");
    }
    if (file == NULL) printf("%d %d", Coarse, Fine);
    else fprintf(file, "%d %d", Coarse, Fine);
  }
  else if (wordID == CSM_WORD) {
    subID = (data>>SUBIDBIT0LOCATION) & SUBIDBITS;
    BCID = (data>>BCIDBIT0LOCATION) & BCIDBITS;
    EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
    nword = BCID;
    if (subID == CSM_HEAD) {
      if (file == NULL)
        printf("CSM header, EVID=0x%03x, BCID=0x%03x", EVID, BCID);
      else
        fprintf(file, "CSM header, EVID=0x%03x, BCID=0x%03x", EVID, BCID);
    }
    else if (subID == CSM_GOOD) {
      if (file == NULL)
        printf("CSM trailer, EVID=0x%03x, WC=%d", EVID, nword);
      else
        fprintf(file, "CSM trailer, EVID=0x%03x, WC=%d", EVID, nword);
    }
    else if (subID == CSM_ABORT) {
      if (file == NULL)
        printf("CSM abort, EVID=0x%03x, WC=%d", EVID, nword);
      else
        fprintf(file, "CSM abort, EVID=0x%03x, WC=%d", EVID, nword);
    }
    else if (subID == CSM_PARITY) {
      Error = (data<<18) & 0x3F;
      TDC = data & 0x3FFFF;
      if (file == NULL)
        printf("CSM error, Bit23:18=0x%02x, TDCWParityError=%05X", Error, TDC);
      else
        fprintf(file, "CSM error, Bit23:18=0x%02x, TDCWParityError=%05X", Error, TDC);
    }
    else if (subID == CSM_ERROR) {
      Error = (data<<18) & 0x3F;
      TDC = data & 0x3FFFF;
      if (file == NULL)
        printf("CSM error, Bit23:18=0x%03x, AbnormalTDC=%05X", Error, TDC);
      else
        fprintf(file, "CSM error, Bit23:18=0x%03x, AbnormalTDC=%05X", Error, TDC);
    }
    else {
      if (file == NULL)
        printf("Unknown CSM word");
      else
        fprintf(file, "Unknown CSM word");
    }
  }
  else {
    if (file == NULL)
      printf("Unknown word");
    else
      fprintf(file, "Unknown word");
  }
  if (file == NULL) printf("\n");
  else fprintf(file, "\n");
}


void SetupJTAGRelatedControlButtons(void) {
  int dimmed, checkMode;
  
  if (verifyFPGAAndPROM) {
    dimmed = TRUE;
    unlockJTAGChain = FALSE;
    SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_JTAGCHAINCONTROL, unlockJTAGChain);
    JTAGChainControl();
  }
  else {
    dimmed = FALSE;
    requestStartOrStop = FALSE;
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, FALSE);
  }
  JTAGChainCSM();
  GetCtrlVal(ConfirmVFPHandle, P_SELVFP_CHECKMODE, &checkMode);
  if (verifyFPGAAndPROM && (checkMode >= 2)) {
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, dimmed);
  }
  else {
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, FALSE);
  }
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_INITDAQ, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_PROGRAMFPGA, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_PROGRAMPROM, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_PROGRAMFPGAFROMPROM, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_VERIFYFPGA, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_VERIFYPROM, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_SHUTDOWNFPGA, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_EXE_FPGACONFSTATUS, ATTR_DIMMED, dimmed);
}


int CheckAMTDataMode(int giveWarning) {
  int mezz, status, resp;

  status = 0;
  AMTPairOn = FALSE;
  AMTEdgesOn = FALSE;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (lEdgeOn[mezz] && tEdgeOn[mezz]) AMTEdgesOn = TRUE;
    if (pairOn[mezz]) AMTPairOn = TRUE;
  }
  if (AMTPairOn && AMTEdgesOn) {
    resp = 1;
    if (giveWarning) {
      resp = ConfirmPopup("Confirm to Run with Mixed AMT Data Mode",
                          "AMTs are taking data with both edges mode and pair mode,\n"
                          "it is not a good way for normal data taking unless for testing AMT.\n"
                          "Please check AMT settings for all mezzanine cards to correct them.\n"
                          "\nAre you sure to run with mixed AMT Data Mode?\n");
      if (resp == 0) status = 1;
    }
    if ((resp != 0) && (CSMPairMode || CSMPairDebug) && (CSMPairWidthResolution != widthSelection)) {
      printf("Change CSM pair width resolution to AMT pair width resolution.\n");
      CSMPairWidthResolution = widthSelection;
      SetCtrlVal(CSMSetupHandle, P_CSM_PWIDTHRESOLUTION, CSMPairWidthResolution);
      PanelSave(CSMSetupHandle);
      if (CSMOn) DownloadCSMSetup();
    }
  }
  
  return status;
}


// Routines for Serial Number
//
// Bring up the Serial Number Panel
void UpSerialNumber(void) {
  int mezz, checkButton[MAXNUMBERMEZZANINE], sameMezz, checksum, expChecksum, oldChamberType, strl;
  int dimmed, mezzSNDone, needDoAgain, done, i, j, k, ndot, nblank, wrongMezzSN, gotChamberName, nmezz;
  int savedMeasureTriggerRate;
  char str[1024], *strp, expSNumber[20], cstr[60], fstr[1024];
  FILE *snFile;
  
  if (controlOption == NORM) {
    ReadMDTChamberDatabase();
    OnChamberDCSBoxUsage();
    dimmed = TRUE;
    if (JTAGDRIVERTYPE == CANELMB) dimmed = FALSE;
    SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_USEDCSONCHAMBER, ATTR_DIMMED, dimmed);
    if (willDoThresholdScan) SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_ALLCHECKED, ATTR_DIMMED, TRUE);
    else SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_ALLCHECKED, ATTR_DIMMED, FALSE);
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      snButton[mezz] = -1;
      checkButton[mezz] = -1;
    }
    snButton[0] = P_SNUMBER_MEZZ00SN;
    snButton[1] = P_SNUMBER_MEZZ01SN;
    snButton[2] = P_SNUMBER_MEZZ02SN;
    snButton[3] = P_SNUMBER_MEZZ03SN;
    snButton[4] = P_SNUMBER_MEZZ04SN;
    snButton[5] = P_SNUMBER_MEZZ05SN;
    snButton[6] = P_SNUMBER_MEZZ06SN;
    snButton[7] = P_SNUMBER_MEZZ07SN;
    snButton[8] = P_SNUMBER_MEZZ08SN;
    snButton[9] = P_SNUMBER_MEZZ09SN;
    snButton[10] = P_SNUMBER_MEZZ10SN;
    snButton[11] = P_SNUMBER_MEZZ11SN;
    snButton[12] = P_SNUMBER_MEZZ12SN;
    snButton[13] = P_SNUMBER_MEZZ13SN;
    snButton[14] = P_SNUMBER_MEZZ14SN;
    snButton[15] = P_SNUMBER_MEZZ15SN;
    snButton[16] = P_SNUMBER_MEZZ16SN;
    snButton[17] = P_SNUMBER_MEZZ17SN;
    checkButton[0] = P_SNUMBER_MEZZ00SNSTS;
    checkButton[1] = P_SNUMBER_MEZZ01SNSTS;
    checkButton[2] = P_SNUMBER_MEZZ02SNSTS;
    checkButton[3] = P_SNUMBER_MEZZ03SNSTS;
    checkButton[4] = P_SNUMBER_MEZZ04SNSTS;
    checkButton[5] = P_SNUMBER_MEZZ05SNSTS;
    checkButton[6] = P_SNUMBER_MEZZ06SNSTS;
    checkButton[7] = P_SNUMBER_MEZZ07SNSTS;
    checkButton[8] = P_SNUMBER_MEZZ08SNSTS;
    checkButton[9] = P_SNUMBER_MEZZ09SNSTS;
    checkButton[10] = P_SNUMBER_MEZZ10SNSTS;
    checkButton[11] = P_SNUMBER_MEZZ11SNSTS;
    checkButton[12] = P_SNUMBER_MEZZ12SNSTS;
    checkButton[13] = P_SNUMBER_MEZZ13SNSTS;
    checkButton[14] = P_SNUMBER_MEZZ14SNSTS;
    checkButton[15] = P_SNUMBER_MEZZ15SNSTS;
    checkButton[16] = P_SNUMBER_MEZZ16SNSTS;
    checkButton[17] = P_SNUMBER_MEZZ17SNSTS;
    SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, ATTR_MIN_VALUE, 0);
    SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, ATTR_MAX_VALUE, MAXNUMBERMEZZANINE);
    SetCtrlVal(SerialNumberHandle, P_SNUMBER_EXPECTEDMEZZCARD, nbMezzCard);
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      if (snButton[mezz] >= 0) {
        if (((mezzEnables>>mezz)&1) == 1) dimmed = FALSE;
        else dimmed = TRUE;
        SetCtrlAttribute(SerialNumberHandle, snButton[mezz], ATTR_DIMMED, dimmed);
        SetCtrlAttribute(SerialNumberHandle, checkButton[mezz], ATTR_DIMMED, dimmed);
      }
    }
    SavePanelState(SerialNumberHandle, "SerialNumber_temp", 0);
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      if (snButton[mezz] >= 0) {
        SetCtrlVal(SerialNumberHandle, checkButton[mezz], FALSE);
        SetCtrlVal(SerialNumberHandle, snButton[mezz], "");
      }
    }
    SetCtrlVal(SerialNumberHandle, P_SNUMBER_EXPECTEDELMBNODE, -1);
    OnChamberDCSBoxUsage();
    SetCtrlVal(SerialNumberHandle, P_SNUMBER_NIKHEFID, "");
    SetCtrlVal(SerialNumberHandle, P_SNUMBER_MDTDCSSN, "");
    SetCtrlVal(SerialNumberHandle, P_SNUMBER_CSMSN, "");
    SetCtrlVal(SerialNumberHandle, P_SNUMBER_MBSN, "");
    SetCtrlVal(SerialNumberHandle, P_SNUMBER_CHAMBERSN, "");
    SetCtrlVal(SerialNumberHandle, P_SNUMBER_OPERATOR, "");
    DisplayPanel(SerialNumberHandle);
    SetActivePanel(SerialNumberHandle);
    savedMeasureTriggerRate = measureTriggerRate;
    measureTriggerRate = FALSE;
    oldChamberType = MDTChamberType;
    mezzSNDone = FALSE;
    while (!mezzSNDone) {
      gotChamberName = FALSE;
      mezzSNDone = HandleUserButtonAndFunction(PanelDone);
#ifdef TTCVIDRIVERVIATCP
      dimmed = FALSE;
      if (measureTriggerRate) {
        savedMeasureTriggerRate = measureTriggerRate;
        measureTriggerRate = FALSE;
      }
      else if (TTCviTCPConnected && (!gotTTCvi)) dimmed = TRUE;
      SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_DONE, ATTR_DIMMED, dimmed);
#endif
      GetCtrlVal(SerialNumberHandle, P_SNUMBER_MDTDCSSN, MDTDCSSN);
      GetCtrlVal(SerialNumberHandle, P_SNUMBER_NIKHEFID, MDTDCSNIKHEFID);
      GetCtrlVal(SerialNumberHandle, P_SNUMBER_EXPECTEDELMBNODE, &expectedELMBNode);
      GetCtrlVal(SerialNumberHandle, P_SNUMBER_CSMSN, CSMSN);
      GetCtrlVal(SerialNumberHandle, P_SNUMBER_MBSN, motherboardSN);
      GetCtrlVal(SerialNumberHandle, P_SNUMBER_CHAMBERSN, MDTChamberSN);
      GetCtrlVal(SerialNumberHandle, P_SNUMBER_OPERATOR, operatorName);
	  nblank = 0;
	  ndot = 0;
	  for (i = 0; i < strlen(MDTChamberSN)+1; i++) {
	    if (MDTChamberSN[i] == '.') {
	      ndot++;
          if (ndot == 2) j = i+1;
	    }
	    else if (MDTChamberSN[i] == ' ') nblank++;
	    cstr[i] = tolower(MDTChamberSN[i]);
	  }
      MDTChamberName[0] = '\0';
	  if (ndot == 3) {
	    k = 0;
        for (i = j; i < strlen(MDTChamberSN); i++) {
          if (isalnum(MDTChamberSN[i])) MDTChamberName[k++] = tolower(MDTChamberSN[i]);
          else if (MDTChamberSN[i] != '.') MDTChamberName[k++] = '_';
        }
        MDTChamberName[k] = '\0';
	  }
	  else {
        j = 0;
        for (i = 0; i < strlen(MDTChamberSN); i++) {
          if ((MDTChamberSN[i] == '-') || (MDTChamberSN[i] == '_'));
          else if (!isalnum(MDTChamberSN[i])) j = i+1;
        }
        for (i = j; i < strlen(MDTChamberSN); i++) {
          if (!isalnum(MDTChamberSN[i])) MDTChamberName[i-j] = '_';
          else MDTChamberName[i-j] = tolower(MDTChamberSN[i]);
        }
        MDTChamberName[i-j] = '\0';
        chamberIndex = -1;
        for (i = 0; i < nChamberEntry; i++) {
          strl = strlen(MDTChamberName);
          if (strl < 5) strl = 5;
          if (strncmp(chamberName[i], MDTChamberName, strl) == 0) chamberIndex = i;
          strl = strlen(cstr);
          if (strl < 14) strl = 14;
          if (strncmp(chamberSNumber[i], cstr, strl) == 0) {
            chamberIndex = i;
            strcpy(MDTChamberName, chamberName[i]);
          }
        }
      }
      if (strncmp(MDTChamberName, "eml1a", 5) == 0) MDTChamberType = CTYPEEML1A;
      else if (strncmp(MDTChamberName, "eml1c", 5) == 0) MDTChamberType = CTYPEEML1C;
      else if (strncmp(MDTChamberName, "eml2a", 5) == 0) MDTChamberType = CTYPEEML2A;
      else if (strncmp(MDTChamberName, "eml2c", 5) == 0) MDTChamberType = CTYPEEML2C;
      else if (strncmp(MDTChamberName, "eml3a", 5) == 0) MDTChamberType = CTYPEEML3A;
      else if (strncmp(MDTChamberName, "eml3c", 5) == 0) MDTChamberType = CTYPEEML3C;
      else if (strncmp(MDTChamberName, "eml4a", 5) == 0) MDTChamberType = CTYPEEML4A;
      else if (strncmp(MDTChamberName, "eml4c", 5) == 0) MDTChamberType = CTYPEEML4C;
      else if (strncmp(MDTChamberName, "eml5a", 5) == 0) MDTChamberType = CTYPEEML5A;
      else if (strncmp(MDTChamberName, "eml5c", 5) == 0) MDTChamberType = CTYPEEML5C;
      else if (strncmp(MDTChamberName, "ems1a", 5) == 0) MDTChamberType = CTYPEEMS1A;
      else if (strncmp(MDTChamberName, "ems1c", 5) == 0) MDTChamberType = CTYPEEMS1C;
      else if (strncmp(MDTChamberName, "ems2a", 5) == 0) MDTChamberType = CTYPEEMS2A;
      else if (strncmp(MDTChamberName, "ems2c", 5) == 0) MDTChamberType = CTYPEEMS2C;
      else if (strncmp(MDTChamberName, "ems3a", 5) == 0) MDTChamberType = CTYPEEMS3A;
      else if (strncmp(MDTChamberName, "ems3c", 5) == 0) MDTChamberType = CTYPEEMS3C;
      else if (strncmp(MDTChamberName, "ems4a", 5) == 0) MDTChamberType = CTYPEEMS4A;
      else if (strncmp(MDTChamberName, "ems4c", 5) == 0) MDTChamberType = CTYPEEMS4C;
      else if (strncmp(MDTChamberName, "ems5a", 5) == 0) MDTChamberType = CTYPEEMS5A;
      else if (strncmp(MDTChamberName, "ems5c", 5) == 0) MDTChamberType = CTYPEEMS5C;
      else if (strncmp(MDTChamberName, "eil1a", 5) == 0) MDTChamberType = CTYPEEIL1A;
      else if (strncmp(MDTChamberName, "eil1c", 5) == 0) MDTChamberType = CTYPEEIL1C;
      else if (strncmp(MDTChamberName, "eil2a", 5) == 0) MDTChamberType = CTYPEEIL2A;
      else if (strncmp(MDTChamberName, "eil2c", 5) == 0) MDTChamberType = CTYPEEIL2C;
      else if (strncmp(MDTChamberName, "eil3a", 5) == 0) MDTChamberType = CTYPEEIL3A;
      else if (strncmp(MDTChamberName, "eil3c", 5) == 0) MDTChamberType = CTYPEEIL3C;
      else if (strncmp(MDTChamberName, "eil23a", 5) == 0) MDTChamberType = CTYPEEIL23A;
      else if (strncmp(MDTChamberName, "eil23c", 5) == 0) MDTChamberType = CTYPEEIL23C;
      else if (strncmp(MDTChamberName, "eil2_3a", 5) == 0) MDTChamberType = CTYPEEIL23A;
      else if (strncmp(MDTChamberName, "eil2_3c", 5) == 0) MDTChamberType = CTYPEEIL23C;
      else if (strncmp(MDTChamberName, "eil4a", 5) == 0) MDTChamberType = CTYPEEIL4A;
      else if (strncmp(MDTChamberName, "eil4c", 5) == 0) MDTChamberType = CTYPEEIL4C;
      else if (strncmp(MDTChamberName, "eil5a", 5) == 0) MDTChamberType = CTYPEEIL5A;
      else if (strncmp(MDTChamberName, "eil5c", 5) == 0) MDTChamberType = CTYPEEIL5C;
      else if (strncmp(MDTChamberName, "eis1a", 5) == 0) MDTChamberType = CTYPEEIS1A;
      else if (strncmp(MDTChamberName, "eis1c", 5) == 0) MDTChamberType = CTYPEEIS1C;
      else if (strncmp(MDTChamberName, "eis2a", 5) == 0) MDTChamberType = CTYPEEIS2A;
      else if (strncmp(MDTChamberName, "eis2c", 5) == 0) MDTChamberType = CTYPEEIS2C;
      else if (strncmp(MDTChamberName, "eis3a", 5) == 0) MDTChamberType = CTYPEEIS3A;
      else if (strncmp(MDTChamberName, "eis3c", 5) == 0) MDTChamberType = CTYPEEIS3C;
      else if (strncmp(MDTChamberName, "eis4a", 5) == 0) MDTChamberType = CTYPEEIS4A;
      else if (strncmp(MDTChamberName, "eis4c", 5) == 0) MDTChamberType = CTYPEEIS4C;
      else if (strncmp(MDTChamberName, "eis5a", 5) == 0) MDTChamberType = CTYPEEIS5A;
      else if (strncmp(MDTChamberName, "eis5c", 5) == 0) MDTChamberType = CTYPEEIS5C;
      else if (strncmp(MDTChamberName, "eol1a", 5) == 0) MDTChamberType = CTYPEEOL1A;
      else if (strncmp(MDTChamberName, "eol1c", 5) == 0) MDTChamberType = CTYPEEOL1C;
      else if (strncmp(MDTChamberName, "eol2a", 5) == 0) MDTChamberType = CTYPEEOL2A;
      else if (strncmp(MDTChamberName, "eol2c", 5) == 0) MDTChamberType = CTYPEEOL2C;
      else if (strncmp(MDTChamberName, "eol3a", 5) == 0) MDTChamberType = CTYPEEOL3A;
      else if (strncmp(MDTChamberName, "eol3c", 5) == 0) MDTChamberType = CTYPEEOL3C;
      else if (strncmp(MDTChamberName, "eol4a", 5) == 0) MDTChamberType = CTYPEEOL4A;
      else if (strncmp(MDTChamberName, "eol4c", 5) == 0) MDTChamberType = CTYPEEOL4C;
      else if (strncmp(MDTChamberName, "eol5a", 5) == 0) MDTChamberType = CTYPEEOL5A;
      else if (strncmp(MDTChamberName, "eol5c", 5) == 0) MDTChamberType = CTYPEEOL5C;
      else if (strncmp(MDTChamberName, "eol6a", 5) == 0) MDTChamberType = CTYPEEOL6A;
      else if (strncmp(MDTChamberName, "eol6c", 5) == 0) MDTChamberType = CTYPEEOL6C;
      else if (strncmp(MDTChamberName, "eos1a", 5) == 0) MDTChamberType = CTYPEEOS1A;
      else if (strncmp(MDTChamberName, "eos1c", 5) == 0) MDTChamberType = CTYPEEOS1C;
      else if (strncmp(MDTChamberName, "eos2a", 5) == 0) MDTChamberType = CTYPEEOS2A;
      else if (strncmp(MDTChamberName, "eos2c", 5) == 0) MDTChamberType = CTYPEEOS2C;
      else if (strncmp(MDTChamberName, "eos3a", 5) == 0) MDTChamberType = CTYPEEOS3A;
      else if (strncmp(MDTChamberName, "eos3c", 5) == 0) MDTChamberType = CTYPEEOS3C;
      else if (strncmp(MDTChamberName, "eos4a", 5) == 0) MDTChamberType = CTYPEEOS4A;
      else if (strncmp(MDTChamberName, "eos4c", 5) == 0) MDTChamberType = CTYPEEOS4C;
      else if (strncmp(MDTChamberName, "eos5a", 5) == 0) MDTChamberType = CTYPEEOS5A;
      else if (strncmp(MDTChamberName, "eos5c", 5) == 0) MDTChamberType = CTYPEEOS5C;
      else if (strncmp(MDTChamberName, "eos6a", 5) == 0) MDTChamberType = CTYPEEOS6A;
      else if (strncmp(MDTChamberName, "eos6c", 5) == 0) MDTChamberType = CTYPEEOS6C;
      else if (strncmp(MDTChamberName, "eel1a", 5) == 0) MDTChamberType = CTYPEEEL1A;
      else if (strncmp(MDTChamberName, "eel1c", 5) == 0) MDTChamberType = CTYPEEEL1C;
      else if (strncmp(MDTChamberName, "eel2a", 5) == 0) MDTChamberType = CTYPEEEL2A;
      else if (strncmp(MDTChamberName, "eel2c", 5) == 0) MDTChamberType = CTYPEEEL2C;
      else if (strncmp(MDTChamberName, "eel3a", 5) == 0) MDTChamberType = CTYPEEEL3A;
      else if (strncmp(MDTChamberName, "eel3c", 5) == 0) MDTChamberType = CTYPEEEL3C;
      else if (strncmp(MDTChamberName, "eel4a", 5) == 0) MDTChamberType = CTYPEEEL4A;
      else if (strncmp(MDTChamberName, "eel4c", 5) == 0) MDTChamberType = CTYPEEEL4C;
      else if (strncmp(MDTChamberName, "eel5a", 5) == 0) MDTChamberType = CTYPEEEL5A;
      else if (strncmp(MDTChamberName, "eel5c", 5) == 0) MDTChamberType = CTYPEEEL5C;
      else if (strncmp(MDTChamberName, "ees1a", 5) == 0) MDTChamberType = CTYPEEES1A;
      else if (strncmp(MDTChamberName, "ees1c", 5) == 0) MDTChamberType = CTYPEEES1C;
      else if (strncmp(MDTChamberName, "ees2a", 5) == 0) MDTChamberType = CTYPEEES2A;
      else if (strncmp(MDTChamberName, "ees2c", 5) == 0) MDTChamberType = CTYPEEES2C;
      else if (strncmp(MDTChamberName, "ees3a", 5) == 0) MDTChamberType = CTYPEEES3A;
      else if (strncmp(MDTChamberName, "ees3c", 5) == 0) MDTChamberType = CTYPEEES3C;
      else if (strncmp(MDTChamberName, "ees4a", 5) == 0) MDTChamberType = CTYPEEES4A;
      else if (strncmp(MDTChamberName, "ees4c", 5) == 0) MDTChamberType = CTYPEEES4C;
      else if (strncmp(MDTChamberName, "ees5a", 5) == 0) MDTChamberType = CTYPEEES5A;
      else if (strncmp(MDTChamberName, "ees5c", 5) == 0) MDTChamberType = CTYPEEES5C;
      else if (strncmp(MDTChamberName, "bil1a", 5) == 0) MDTChamberType = CTYPEBIL1A;
      else if (strncmp(MDTChamberName, "bil1c", 5) == 0) MDTChamberType = CTYPEBIL1C;
      else if (strncmp(MDTChamberName, "bil2a", 5) == 0) MDTChamberType = CTYPEBIL2A;
      else if (strncmp(MDTChamberName, "bil2c", 5) == 0) MDTChamberType = CTYPEBIL2C;
      else if (strncmp(MDTChamberName, "bil3a", 5) == 0) MDTChamberType = CTYPEBIL3A;
      else if (strncmp(MDTChamberName, "bil3c", 5) == 0) MDTChamberType = CTYPEBIL3C;
      else if (strncmp(MDTChamberName, "bil4a", 5) == 0) MDTChamberType = CTYPEBIL4A;
      else if (strncmp(MDTChamberName, "bil4c", 5) == 0) MDTChamberType = CTYPEBIL4C;
      else if (strncmp(MDTChamberName, "bil5a", 5) == 0) MDTChamberType = CTYPEBIL5A;
      else if (strncmp(MDTChamberName, "bil5c", 5) == 0) MDTChamberType = CTYPEBIL5C;
      else if (strncmp(MDTChamberName, "bil6a", 5) == 0) MDTChamberType = CTYPEBIL6A;
      else if (strncmp(MDTChamberName, "bil6c", 5) == 0) MDTChamberType = CTYPEBIL6C;
      else if (strncmp(MDTChamberName, "bil7a", 5) == 0) MDTChamberType = CTYPEBIL7A;
      else if (strncmp(MDTChamberName, "bil7c", 5) == 0) MDTChamberType = CTYPEBIL7C;
      else if (strncmp(MDTChamberName, "bim1a", 5) == 0) MDTChamberType = CTYPEBIM1A;
      else if (strncmp(MDTChamberName, "bim1c", 5) == 0) MDTChamberType = CTYPEBIM1C;
      else if (strncmp(MDTChamberName, "bim2a", 5) == 0) MDTChamberType = CTYPEBIM2A;
      else if (strncmp(MDTChamberName, "bim2c", 5) == 0) MDTChamberType = CTYPEBIM2C;
      else if (strncmp(MDTChamberName, "bim3a", 5) == 0) MDTChamberType = CTYPEBIM3A;
      else if (strncmp(MDTChamberName, "bim3c", 5) == 0) MDTChamberType = CTYPEBIM3C;
      else if (strncmp(MDTChamberName, "bim4a", 5) == 0) MDTChamberType = CTYPEBIM4A;
      else if (strncmp(MDTChamberName, "bim4c", 5) == 0) MDTChamberType = CTYPEBIM4C;
      else if (strncmp(MDTChamberName, "bim5a", 5) == 0) MDTChamberType = CTYPEBIM5A;
      else if (strncmp(MDTChamberName, "bim5c", 5) == 0) MDTChamberType = CTYPEBIM5C;
      else if (strncmp(MDTChamberName, "bim6a", 5) == 0) MDTChamberType = CTYPEBIM6A;
      else if (strncmp(MDTChamberName, "bim6c", 5) == 0) MDTChamberType = CTYPEBIM6C;
      else if (strncmp(MDTChamberName, "bis1a", 5) == 0) MDTChamberType = CTYPEBIS1A;
      else if (strncmp(MDTChamberName, "bis1c", 5) == 0) MDTChamberType = CTYPEBIS1C;
      else if (strncmp(MDTChamberName, "bis2a", 5) == 0) MDTChamberType = CTYPEBIS2A;
      else if (strncmp(MDTChamberName, "bis2c", 5) == 0) MDTChamberType = CTYPEBIS2C;
      else if (strncmp(MDTChamberName, "bis3a", 5) == 0) MDTChamberType = CTYPEBIS3A;
      else if (strncmp(MDTChamberName, "bis3c", 5) == 0) MDTChamberType = CTYPEBIS3C;
      else if (strncmp(MDTChamberName, "bis4a", 5) == 0) MDTChamberType = CTYPEBIS4A;
      else if (strncmp(MDTChamberName, "bis4c", 5) == 0) MDTChamberType = CTYPEBIS4C;
      else if (strncmp(MDTChamberName, "bis5a", 5) == 0) MDTChamberType = CTYPEBIS5A;
      else if (strncmp(MDTChamberName, "bis5c", 5) == 0) MDTChamberType = CTYPEBIS5C;
      else if (strncmp(MDTChamberName, "bis6a", 5) == 0) MDTChamberType = CTYPEBIS6A;
      else if (strncmp(MDTChamberName, "bis6c", 5) == 0) MDTChamberType = CTYPEBIS6C;
      else if (strncmp(MDTChamberName, "bis7a", 5) == 0) MDTChamberType = CTYPEBIS7A;
      else if (strncmp(MDTChamberName, "bis7c", 5) == 0) MDTChamberType = CTYPEBIS7C;
      else if (strncmp(MDTChamberName, "bis8a", 5) == 0) MDTChamberType = CTYPEBIS8A;
      else if (strncmp(MDTChamberName, "bis8c", 5) == 0) MDTChamberType = CTYPEBIS8C;
      else if (strncmp(MDTChamberName, "bir1a", 5) == 0) MDTChamberType = CTYPEBIR1A;
      else if (strncmp(MDTChamberName, "bir1c", 5) == 0) MDTChamberType = CTYPEBIR1C;
      else if (strncmp(MDTChamberName, "bir2a", 5) == 0) MDTChamberType = CTYPEBIR2A;
      else if (strncmp(MDTChamberName, "bir2c", 5) == 0) MDTChamberType = CTYPEBIR2C;
      else if (strncmp(MDTChamberName, "bir3a", 5) == 0) MDTChamberType = CTYPEBIR3A;
      else if (strncmp(MDTChamberName, "bir3c", 5) == 0) MDTChamberType = CTYPEBIR3C;
      else if (strncmp(MDTChamberName, "bir4a", 5) == 0) MDTChamberType = CTYPEBIR4A;
      else if (strncmp(MDTChamberName, "bir4c", 5) == 0) MDTChamberType = CTYPEBIR4C;
      else if (strncmp(MDTChamberName, "bir5a", 5) == 0) MDTChamberType = CTYPEBIR5A;
      else if (strncmp(MDTChamberName, "bir5c", 5) == 0) MDTChamberType = CTYPEBIR5C;
      else if (strncmp(MDTChamberName, "bir6a", 5) == 0) MDTChamberType = CTYPEBIR6A;
      else if (strncmp(MDTChamberName, "bir6c", 5) == 0) MDTChamberType = CTYPEBIR6C;
      else if (strncmp(MDTChamberName, "bir7a", 5) == 0) MDTChamberType = CTYPEBIR7A;
      else if (strncmp(MDTChamberName, "bir7c", 5) == 0) MDTChamberType = CTYPEBIR7C;
      else if (strncmp(MDTChamberName, "bmf1a", 5) == 0) MDTChamberType = CTYPEBMF1A;
      else if (strncmp(MDTChamberName, "bmf1c", 5) == 0) MDTChamberType = CTYPEBMF1C;
      else if (strncmp(MDTChamberName, "bmf2a", 5) == 0) MDTChamberType = CTYPEBMF2A;
      else if (strncmp(MDTChamberName, "bmf2c", 5) == 0) MDTChamberType = CTYPEBMF2C;
      else if (strncmp(MDTChamberName, "bmf3a", 5) == 0) MDTChamberType = CTYPEBMF3A;
      else if (strncmp(MDTChamberName, "bmf3c", 5) == 0) MDTChamberType = CTYPEBMF3C;
      else if (strncmp(MDTChamberName, "bmf4a", 5) == 0) MDTChamberType = CTYPEBMF4A;
      else if (strncmp(MDTChamberName, "bmf4c", 5) == 0) MDTChamberType = CTYPEBMF4C;
      else if (strncmp(MDTChamberName, "bmf5a", 5) == 0) MDTChamberType = CTYPEBMF5A;
      else if (strncmp(MDTChamberName, "bmf5c", 5) == 0) MDTChamberType = CTYPEBMF5C;
      else if (strncmp(MDTChamberName, "bml1a", 5) == 0) MDTChamberType = CTYPEBML1A;
      else if (strncmp(MDTChamberName, "bml1c", 5) == 0) MDTChamberType = CTYPEBML1C;
      else if (strncmp(MDTChamberName, "bml2a", 5) == 0) MDTChamberType = CTYPEBML2A;
      else if (strncmp(MDTChamberName, "bml2c", 5) == 0) MDTChamberType = CTYPEBML2C;
      else if (strncmp(MDTChamberName, "bml3a", 5) == 0) MDTChamberType = CTYPEBML3A;
      else if (strncmp(MDTChamberName, "bml3c", 5) == 0) MDTChamberType = CTYPEBML3C;
      else if (strncmp(MDTChamberName, "bml4a", 5) == 0) MDTChamberType = CTYPEBML4A;
      else if (strncmp(MDTChamberName, "bml4c", 5) == 0) MDTChamberType = CTYPEBML4C;
      else if (strncmp(MDTChamberName, "bml5a", 5) == 0) MDTChamberType = CTYPEBML5A;
      else if (strncmp(MDTChamberName, "bml5c", 5) == 0) MDTChamberType = CTYPEBML5C;
      else if (strncmp(MDTChamberName, "bml6a", 5) == 0) MDTChamberType = CTYPEBML6A;
      else if (strncmp(MDTChamberName, "bml6c", 5) == 0) MDTChamberType = CTYPEBML6C;
      else if (strncmp(MDTChamberName, "bml7a", 5) == 0) MDTChamberType = CTYPEBML7A;
      else if (strncmp(MDTChamberName, "bml7c", 5) == 0) MDTChamberType = CTYPEBML7C;
      else if (strncmp(MDTChamberName, "bms1a", 5) == 0) MDTChamberType = CTYPEBMS1A;
      else if (strncmp(MDTChamberName, "bms1c", 5) == 0) MDTChamberType = CTYPEBMS1C;
      else if (strncmp(MDTChamberName, "bms2a", 5) == 0) MDTChamberType = CTYPEBMS2A;
      else if (strncmp(MDTChamberName, "bms2c", 5) == 0) MDTChamberType = CTYPEBMS2C;
      else if (strncmp(MDTChamberName, "bms3a", 5) == 0) MDTChamberType = CTYPEBMS3A;
      else if (strncmp(MDTChamberName, "bms3c", 5) == 0) MDTChamberType = CTYPEBMS3C;
      else if (strncmp(MDTChamberName, "bms4a", 5) == 0) MDTChamberType = CTYPEBMS4A;
      else if (strncmp(MDTChamberName, "bms4c", 5) == 0) MDTChamberType = CTYPEBMS4C;
      else if (strncmp(MDTChamberName, "bms5a", 5) == 0) MDTChamberType = CTYPEBMS5A;
      else if (strncmp(MDTChamberName, "bms5c", 5) == 0) MDTChamberType = CTYPEBMS5C;
      else if (strncmp(MDTChamberName, "bms6a", 5) == 0) MDTChamberType = CTYPEBMS6A;
      else if (strncmp(MDTChamberName, "bms6c", 5) == 0) MDTChamberType = CTYPEBMS6C;
      else if (strncmp(MDTChamberName, "bol1a", 5) == 0) MDTChamberType = CTYPEBOL1A;
      else if (strncmp(MDTChamberName, "bol1c", 5) == 0) MDTChamberType = CTYPEBOL1C;
      else if (strncmp(MDTChamberName, "bol2a", 5) == 0) MDTChamberType = CTYPEBOL2A;
      else if (strncmp(MDTChamberName, "bol2c", 5) == 0) MDTChamberType = CTYPEBOL2C;
      else if (strncmp(MDTChamberName, "bol3a", 5) == 0) MDTChamberType = CTYPEBOL3A;
      else if (strncmp(MDTChamberName, "bol3c", 5) == 0) MDTChamberType = CTYPEBOL3C;
      else if (strncmp(MDTChamberName, "bol4a", 5) == 0) MDTChamberType = CTYPEBOL4A;
      else if (strncmp(MDTChamberName, "bol4c", 5) == 0) MDTChamberType = CTYPEBOL4C;
      else if (strncmp(MDTChamberName, "bol5a", 5) == 0) MDTChamberType = CTYPEBOL5A;
      else if (strncmp(MDTChamberName, "bol5c", 5) == 0) MDTChamberType = CTYPEBOL5C;
      else if (strncmp(MDTChamberName, "bol6a", 5) == 0) MDTChamberType = CTYPEBOL6A;
      else if (strncmp(MDTChamberName, "bol6c", 5) == 0) MDTChamberType = CTYPEBOL6C;
      else if (strncmp(MDTChamberName, "bol7a", 5) == 0) MDTChamberType = CTYPEBOL7A;
      else if (strncmp(MDTChamberName, "bol7c", 5) == 0) MDTChamberType = CTYPEBOL7C;
      else if (strncmp(MDTChamberName, "bos1a", 5) == 0) MDTChamberType = CTYPEBOS1A;
      else if (strncmp(MDTChamberName, "bos1c", 5) == 0) MDTChamberType = CTYPEBOS1C;
      else if (strncmp(MDTChamberName, "bos2a", 5) == 0) MDTChamberType = CTYPEBOS2A;
      else if (strncmp(MDTChamberName, "bos2c", 5) == 0) MDTChamberType = CTYPEBOS2C;
      else if (strncmp(MDTChamberName, "bos3a", 5) == 0) MDTChamberType = CTYPEBOS3A;
      else if (strncmp(MDTChamberName, "bos3c", 5) == 0) MDTChamberType = CTYPEBOS3C;
      else if (strncmp(MDTChamberName, "bos4a", 5) == 0) MDTChamberType = CTYPEBOS4A;
      else if (strncmp(MDTChamberName, "bos4c", 5) == 0) MDTChamberType = CTYPEBOS4C;
      else if (strncmp(MDTChamberName, "bos5a", 5) == 0) MDTChamberType = CTYPEBOS5A;
      else if (strncmp(MDTChamberName, "bos5c", 5) == 0) MDTChamberType = CTYPEBOS5C;
      else if (strncmp(MDTChamberName, "bos6a", 5) == 0) MDTChamberType = CTYPEBOS6A;
      else if (strncmp(MDTChamberName, "bos6c", 5) == 0) MDTChamberType = CTYPEBOS6C;
      else if (strncmp(MDTChamberName, "bosxa", 5) == 0) MDTChamberType = CTYPEBOSXA;
      else if (strncmp(MDTChamberName, "bosxc", 5) == 0) MDTChamberType = CTYPEBOSXC;
      else if (strncmp(MDTChamberName, "bog0a", 5) == 0) MDTChamberType = CTYPEBOG0A;
      else if (strncmp(MDTChamberName, "bog0b", 5) == 0) MDTChamberType = CTYPEBOG0B;
      else if (strncmp(MDTChamberName, "bog0c", 5) == 0) MDTChamberType = CTYPEBOG0C;
      else if (strncmp(MDTChamberName, "bog2a", 5) == 0) MDTChamberType = CTYPEBOG2A;
      else if (strncmp(MDTChamberName, "bog2b", 5) == 0) MDTChamberType = CTYPEBOG2B;
      else if (strncmp(MDTChamberName, "bog2c", 5) == 0) MDTChamberType = CTYPEBOG2C;
      else if (strncmp(MDTChamberName, "bog4a", 5) == 0) MDTChamberType = CTYPEBOG4A;
      else if (strncmp(MDTChamberName, "bog4b", 5) == 0) MDTChamberType = CTYPEBOG4B;
      else if (strncmp(MDTChamberName, "bog4c", 5) == 0) MDTChamberType = CTYPEBOG4C;
      else if (strncmp(MDTChamberName, "bog6a", 5) == 0) MDTChamberType = CTYPEBOG6A;
      else if (strncmp(MDTChamberName, "bog6b", 5) == 0) MDTChamberType = CTYPEBOG6B;
      else if (strncmp(MDTChamberName, "bog6c", 5) == 0) MDTChamberType = CTYPEBOG6C;
      else if (strncmp(MDTChamberName, "bog8a", 5) == 0) MDTChamberType = CTYPEBOG8A;
      else if (strncmp(MDTChamberName, "bog8b", 5) == 0) MDTChamberType = CTYPEBOG8B;
      else if (strncmp(MDTChamberName, "bog8c", 5) == 0) MDTChamberType = CTYPEBOG8C;
      else if (strncmp(MDTChamberName, "bof1a", 5) == 0) MDTChamberType = CTYPEBOF1A;
      else if (strncmp(MDTChamberName, "bof1c", 5) == 0) MDTChamberType = CTYPEBOF1C;
      else if (strncmp(MDTChamberName, "bof3a", 5) == 0) MDTChamberType = CTYPEBOF3A;
      else if (strncmp(MDTChamberName, "bof3c", 5) == 0) MDTChamberType = CTYPEBOF3C;
      else if (strncmp(MDTChamberName, "bof5a", 5) == 0) MDTChamberType = CTYPEBOF5A;
      else if (strncmp(MDTChamberName, "bof5c", 5) == 0) MDTChamberType = CTYPEBOF5C;
      else if (strncmp(MDTChamberName, "bof7a", 5) == 0) MDTChamberType = CTYPEBOF7A;
      else if (strncmp(MDTChamberName, "bof7c", 5) == 0) MDTChamberType = CTYPEBOF7C;
      else if (strncmp(MDTChamberName, "bee1a", 5) == 0) MDTChamberType = CTYPEBEE1A;
      else if (strncmp(MDTChamberName, "bee1c", 5) == 0) MDTChamberType = CTYPEBEE1C;
      else if (strncmp(MDTChamberName, "bee2a", 5) == 0) MDTChamberType = CTYPEBEE2A;
      else if (strncmp(MDTChamberName, "bee2c", 5) == 0) MDTChamberType = CTYPEBEE2C;
      else if (chamberIndex >= 0) MDTChamberType = CTYPEINDB;
      else MDTChamberType = CTYPEUNKNOWN;
      i = 0xFF;
      if (MDTChamberType != CTYPEUNKNOWN) {
        j = strlen(MDTChamberName);
        if (isdigit(MDTChamberName[j-2]) && isdigit(MDTChamberName[j-1])) {
          gotChamberName = TRUE;
          str[0] = MDTChamberName[j-2];
          str[1] = MDTChamberName[j-1];
          str[2] = '\0';
          sscanf(str, "%d", &i);
        }
      }
      MDTChamberType |= (i & CNUMBERMASK);
      if (gotChamberName && ((MDTChamberType != oldChamberType) || (nbMezzCard != MDTChamberNumberMezz))) {
        GetNumberMezzAndRow(MDTChamberType, &MDTChamberNumberMezz, &MDTChamberNumberRow);
        if ((MDTChamberNumberMezz > 0) && (MDTChamberNumberMezz <= MAXNUMBERMEZZANINE)) {
          k = MAXNUMBERMEZZANINE - MDTChamberNumberMezz;
        }
        else k = MAXNUMBERMEZZANINE;
        GetCtrlVal(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, &startMezzCard);
        if (startMezzCard > k) {
          startMezzCard = 0;
          SetCtrlVal(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, startMezzCard);
        }
        SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, ATTR_MIN_VALUE, 0);
        SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, ATTR_MAX_VALUE, k);
        if (MDTChamberControl) {
          if (SetDir("database") == 0) SetDir("..");
          else MakeDir("database");
          strcpy(fstr, "database/");
          strcat(fstr, MDTChamberName);
          strcat(fstr, "_SN.txt");
          startMezzCard = 0;
          if (snFile = fopen(fstr, "r")) {
            nmezz = 0;
            startMezzCard = MAXNUMBERMEZZANINE;
            while (feof(snFile) == 0) {
              fgets(str, 256, snFile);
              strcpy(cstr, "MDT Chamber Start MezzCard");
              strp = strstr(str, cstr);
              if (strp == NULL) {
                strcpy(cstr, "Chamber Start MezzCard");
                strp = strstr(str, cstr);
              }
              if (strp == NULL) {
                strcpy(cstr, "Start Mezzanine Card");
                strp = strstr(str, cstr);
              }
              i = 0;
              if ((strp != NULL) && (str[0] != '/') && (str[1] != '/')) {
                strl = strlen(cstr);
                for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
                j = 0;
                while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
                for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
                sscanf(str, "%d", &k);
                if ((k >= 0) && ((k+MDTChamberNumberMezz) < MAXNUMBERMEZZANINE)) {
                  i = 1;
                  startMezzCard = k;
                  break;
                }
                else i = 0;
              }
              if (i == 1) break;
              for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
                sprintf(cstr, "Mezzanine Card %2d Serial Number", mezz);
                strp = strstr(str, cstr);
                if (strp != NULL) {
                  strl = strlen(cstr);
                  for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
                  j = 0;
                  while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
                  for (i = j; i < strlen(str)+1; i++) {
                    if (str[i] == '\n') str[i] = '\0';
                    str[i-j] = str[i];
                  }
                  if (strlen(str) > 6) {
                    nmezz++;
                    if (mezz < startMezzCard) startMezzCard = mezz;
                  }
                }
              }
            }
            fclose(snFile);
            if (nmezz <= (MDTChamberNumberMezz/2)) startMezzCard = 0;
          }
          k = FALSE;
          if (startMezzCard > 0) {
            sprintf(str, "First mezzanine card of MDT chamber %s is started from %d instead of 0.\n\n"
                         "Are you sure it is right starting point?\n", MDTChamberName, startMezzCard);
            i = ConfirmPopup("Confirm Chamber MezzCard Starting Point", str);
            if (i != 1) {
              sprintf(str, "Please verify MDT chamber %s serial number file : \n\n"
                           "%s\n", MDTChamberName, fstr);
              MessagePopup("Check Chamber Serial Number File", str);
              while (i != 1) {
                j = MAXNUMBERMEZZANINE - MDTChamberNumberMezz;
                sprintf(str, "Please give starting mezzanine card number (0 - %d) for chamber %s.\n", j, MDTChamberName);
                PromptPopup("Chamber MezzCard Start Point", str, cstr, 2);
                if (cstr[0] != '\0') startMezzCard = atoi(cstr);
                if ((startMezzCard+MDTChamberNumberMezz) > MAXNUMBERMEZZANINE) i = 0;
                else if (startMezzCard < 0) i = 0;
                else i = 1;
              }
            }
            k = TRUE;
          }
          EnableMezzCardsAccordingMDTChamber();
          for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
            if (snButton[mezz] >= 0) {
              if (((mezzEnables>>mezz)&1) == 1) dimmed = FALSE;
              else dimmed = TRUE;
              SetCtrlAttribute(SerialNumberHandle, snButton[mezz], ATTR_DIMMED, dimmed);
              SetCtrlAttribute(SerialNumberHandle, checkButton[mezz], ATTR_DIMMED, dimmed);
            }
          }
          SavePanelState(SerialNumberHandle, "SerialNumber_temp", 0);
          for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
            if (snButton[mezz] >= 0) {
              SetCtrlVal(SerialNumberHandle, checkButton[mezz], FALSE);
              SetCtrlVal(SerialNumberHandle, snButton[mezz], "");
            }
          }
          SetCtrlVal(SerialNumberHandle, P_SNUMBER_EXPECTEDMEZZCARD, nbMezzCard);
          if (k == TRUE) LoadPreviousSerialNumber();
        }
        SetCtrlVal(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, startMezzCard);
        oldChamberType = MDTChamberType;
      }
      GetCtrlVal(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, &startMezzCard);
      if (startMezzCard == 0) {
        SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, ATTR_TEXT_BGCOLOR, VAL_GREEN);
      }
      else {
        SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, ATTR_TEXT_BGCOLOR, VAL_MAGENTA);
      }
    }
    measureTriggerRate = savedMeasureTriggerRate;
    if (!MDTChamberControl && (MDTChamberType != CTYPEUNKNOWN) && (startMezzCard > 0)) {
      sprintf(str, "First mezzanine card of MDT chamber %s is started from %d instead of 0.\n\n"
                   "Are you sure it is right starting point?\n", MDTChamberName, startMezzCard);
      i = ConfirmPopup("Confirm Chamber MezzCard Starting Point", str);
      if (i != 1) UpSerialNumber();
    }
    sameMezz = FALSE;
    wrongMezzSN = FALSE;
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      if ((snButton[mezz] >= 0) && (((mezzEnables>>mezz)&1) == 1)) {
        GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
        for (i = mezz+1; i < MAXNUMBERMEZZANINE; i++) {
          if ((snButton[i] >= 0) && (((mezzEnables>>i)&1) == 1)) {
            strcpy(str, "");
            GetCtrlVal(SerialNumberHandle, snButton[i], str);
            strp = (char *) strstr(str, serialNumber);
            if (strp != NULL) {
              printf("Identical Serial Number <%s> for Mezzanine Card %2d and %2d\n", serialNumber, mezz, i);
              sameMezz = TRUE;
              SetCtrlVal(SerialNumberHandle, checkButton[mezz], FALSE);
            }
          }
        }
        strcpy(str, serialNumber);
        for (i = 0; i < strlen(str); i++) {
          if (!isdigit(str[i])) str[i] = ' ';
        }
        sscanf(str, "%d", &expChecksum);
        for (i = 0; i < strlen(str); i++) {
          if (str[i] != ' ') str[i] = ' ';
          else break;
        }
        checksum = 0;
        for (i = 0; i < strlen(str); i++) {
          if (isdigit(str[i])) {
            sscanf(&str[i], "%1d", &j);
            checksum += j;
          }
        }
        if (checksum != expChecksum) {
          printf("Wrong Checksum! Expected Checksum = %d  Actual Checksum = %d\n", expChecksum, checksum);
          printf("Wrong checksum of Serial Number <%s> for Mezzanine Card %2d\n", serialNumber, mezz);
          strcpy(str, serialNumber);
          for (i = 0; i < strlen(str); i++) {
            if (isdigit(str[i])) str[i] = ' ';
            else break;
          }
          j = 0;
          for (i = 0; i < strlen(str)+1; i++) {
            if (str[i] != ' ') str[j++] = str[i];
          }
          sprintf(expSNumber, "%d", checksum);
          strcat(expSNumber, str);
          printf("Please Check!  Is Serial Number <%s> for Mezzanine Card %2d?\n", expSNumber, mezz);
          wrongMezzSN = TRUE;
          SetCtrlVal(SerialNumberHandle, checkButton[mezz], FALSE);
        }
      }
    }
    if (wrongMezzSN) {
      sprintf(str, "Found mezzanine card serial number with wrong checksum.\n"
                   "Please check Mezzanine Card Serial Number and make sure they are right!\n");
      MessagePopup("Wrong Checksum for Mezzanine Card Serail Number", str);
      UpSerialNumber();
    }
    else if (sameMezz) {
      sprintf(str, "Different mezzanine cards have same serial number.\n"
                   "Please check Mezzanine Card Serial Number and make sure they are right!\n");
      MessagePopup("Identical Mezzanine Card Serail Number", str);
      UpSerialNumber();
    }
    if (MDTChamberTest) {
      needDoAgain = FALSE;
      done = 1;
      if (strlen(operatorName) <= 0) needDoAgain = TRUE;
      printf("Operator Name                   : %30s\n", operatorName);
      if (strlen(MDTChamberSN) <= 0) needDoAgain = TRUE;
      printf("MDT Chamber Serial Number       : %30s\n", MDTChamberSN);
      printf("MDT Chamber Name                : %30s\n", MDTChamberName);
      if (strlen(motherboardSN) <= 0) needDoAgain = TRUE;
      printf("Motherboard Serial Number       : %30s\n", motherboardSN);
      if (strlen(CSMSN) <= 0) needDoAgain = TRUE;
      printf("CSM Serial Number               : %30s\n", CSMSN);
      if (strlen(MDTDCSSN) <= 0) needDoAgain = TRUE;
      if (strlen(MDTDCSNIKHEFID) <= 0) needDoAgain = TRUE;
      printf("MDT-DCS Serial Number           : %30s\n", MDTDCSSN);
      printf("MDT-DCS NIKHEF ID               : %30s\n", MDTDCSNIKHEFID);
      printf("MDT-DCS Expected ELMB Node      : %30d\n", expectedELMBNode);
      if (useOnChamberDCS) {
        printf("MDT-DCS Actual ELMB Node        : %30d\n", ELMBNode);
        printf("MDT-DCS ELMB Version            : %30s\n", ELMBVersion);
      }
      printf("MDT Chamber Start MezzCard      : %d\n", startMezzCard);
      for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
        if ((snButton[mezz] >= 0) && (((mezzEnables>>mezz)&1) == 1)) {
          GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
          if (strlen(serialNumber) <= 0) needDoAgain = TRUE;
          printf("Mezzanine Card %2d Serial Number : %30s\n", mezz, serialNumber);
          GetCtrlVal(SerialNumberHandle, checkButton[mezz], &done);
          if (!done) needDoAgain = TRUE;
        }
      }
      if (needDoAgain) {
        MessagePopup("Uncompleted Serial Number",
                     "User must provide all the serial number and check mezzanine card serial number.");
        UpSerialNumber();
      }
      else if (MDTChamberType == CTYPEUNKNOWN) {
        sprintf(str, "Unknown MDT Chamber name <%s>\n"
                     "Please check MDT Chamber Serial Number and make sure it is right!\n"
                     "If your chamber is not EM or EI, please contact \"Tiesheng Dai <daits@umich.edu>\".\n"
                     "To exit Serial Number Panel, give MDT Chamber Serial Number as ATLM.M.EML.5C99.\n",
                     MDTChamberName);
        MessagePopup("Unknown MDT Chamber", str);
        UpSerialNumber();
      }
      else if ((expectedELMBNode < 0) || (expectedELMBNode > 127)) {
        sprintf(str, "The ELMB Node on MDT chamber %s is %d out of allowed range of 0 to 127.\n"
                     "  If you know the ELMB Node, please put it in <MDT-DCS ELMB Node : Expected>.\n"
                     "  If MDT-DCS box is installed on chamber and you do not know ELMB Node, contact expert!\n"
                     "  If MDT-DCS box is not installed on chamber, please put zero in <MDT-DCS ELMB Node : Expected>.\n",
                     MDTChamberName, expectedELMBNode);
        MessagePopup("Illegal ELMB Node", str);
        UpSerialNumber();
      }
    }
    else {
      needDoAgain = FALSE;
      for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
        if ((snButton[mezz] >= 0) && (((mezzEnables>>mezz)&1) == 1)) {
          GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
          if (strlen(serialNumber) <= 0) needDoAgain = TRUE;
        }
      }
      if (needDoAgain) {
        MessagePopup("Uncompleted Serial Number",
                     "User must provide all the mezzanine card serial number.");
        UpSerialNumber();
      }
    }
    if (MDTChamberType != CTYPEUNKNOWN) {
      if (chamberIndex < 0) {
        // Check Chamber Serial Number, make sure it is right
        nblank = 0;
        ndot = 0;
        for (i = 0; i < strlen(MDTChamberSN)+1; i++) {
          if (MDTChamberSN[i] == '.') ndot++;
          else if (MDTChamberSN[i] == ' ') nblank++;
        }
        if ((ndot < 2) || (ndot > 3) || (nblank > 0)) {
          for (i = 0; i < strlen(MDTChamberName)+1; i++) serialNumber[i] = toupper(MDTChamberName[i]);
          sprintf(str, "MDT Chamber Serial Number is not as expected.\n"
                       "It should like as ATLM.M.%s.\n"
                       "Please provide correct MDT Chamber Serial Number.", serialNumber);
          printf("%s\n", str);
          MessagePopup("Wrong MDT Chamber Serial Number", str);
          UpSerialNumber();
        }
      }
      // Check Motherboard Serial Number, make sure it is right
	  strp = (char *) strstr(motherboardSN, "-MB-");
	  nblank = 0;
	  ndot = 0;
	  for (i = 0; i < strlen(motherboardSN); i++) {
	    if (motherboardSN[i] == '-') ndot++;
	    else if (motherboardSN[i] == ' ') nblank++;
	  }
	  if ((ndot != 4) || (nblank > 0) || (strp == NULL)) {
	    sprintf(str, "Motherboard Serial Number is not as expected.\n"
	                 "It should contain key word '-MB-' and at least 4 '-' without blank space.\n"
	                 "Please provide correct motherboard Chamber Serial Number.");
		printf("%s\n", str);
        MessagePopup("Wrong MDT Chamber Serial Number", str);
        UpSerialNumber();
	  }

      if (SetDir("database") == 0) SetDir("..");
      else MakeDir("database");
      strcpy(str, "database/");
      strcat(str, MDTChamberName);
      strcat(str, "_SN.txt");
      if (snFile = fopen(str, "w")) {
        fprintf(snFile, "Operator Name                   : %s\n", operatorName);
        fprintf(snFile, "MDT Chamber Serial Number       : %s\n", MDTChamberSN);
        fprintf(snFile, "MDT Chamber Name                : %s\n", MDTChamberName);
        fprintf(snFile, "Motherboard Serial Number       : %s\n", motherboardSN);
        fprintf(snFile, "CSM Serial Number               : %s\n", CSMSN);
        fprintf(snFile, "MDT-DCS Serial Number           : %s\n", MDTDCSSN);
        fprintf(snFile, "MDT-DCS NIKHEF ID               : %s\n", MDTDCSNIKHEFID);
        fprintf(snFile, "MDT-DCS Expected ELMB Node      : %d\n", expectedELMBNode);
        if (JTAGDRIVERTYPE == CANELMB) {
          if (useOnChamberDCS) {
            fprintf(snFile, "MDT-DCS Actual ELMB Node        : %d\n", ELMBNode);
            fprintf(snFile, "MDT-DCS ELMB Version            : %s\n", ELMBVersion);
          }
          else {
            if (CANNode >= 0) fprintf(snFile, "CAN Node                        : %d\n", CANNode);
            if (ELMBNode > 0) {
              fprintf(snFile, "ELMB Node                       : %d\n", ELMBNode);
              fprintf(snFile, "ELMB Version                    : %s\n", ELMBVersion);
            }
          }
        }
        fprintf(snFile, "MDT Chamber Start MezzCard      : %d\n", startMezzCard);
        for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
          if ((snButton[mezz] >= 0) && (((mezzEnables>>mezz)&1) == 1)) {
            GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
            fprintf(snFile, "Mezzanine Card %2d Serial Number : %s\n", mezz, serialNumber);
          }
        }
        fclose(snFile);
      }
    }
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Serial Number Panel", 
                 "Bring up Serial Number Controls.");
  controlOption = NORM;
}


void LoadPreviousSerialNumber(void) {
  int i, j, strl, mezz, checkButton[MAXNUMBERMEZZANINE];
  char str[256], *strp, cstr[80];
  FILE *snFile;

  checkButton[0] = P_SNUMBER_MEZZ00SNSTS;
  checkButton[1] = P_SNUMBER_MEZZ01SNSTS;
  checkButton[2] = P_SNUMBER_MEZZ02SNSTS;
  checkButton[3] = P_SNUMBER_MEZZ03SNSTS;
  checkButton[4] = P_SNUMBER_MEZZ04SNSTS;
  checkButton[5] = P_SNUMBER_MEZZ05SNSTS;
  checkButton[6] = P_SNUMBER_MEZZ06SNSTS;
  checkButton[7] = P_SNUMBER_MEZZ07SNSTS;
  checkButton[8] = P_SNUMBER_MEZZ08SNSTS;
  checkButton[9] = P_SNUMBER_MEZZ09SNSTS;
  checkButton[10] = P_SNUMBER_MEZZ10SNSTS;
  checkButton[11] = P_SNUMBER_MEZZ11SNSTS;
  checkButton[12] = P_SNUMBER_MEZZ12SNSTS;
  checkButton[13] = P_SNUMBER_MEZZ13SNSTS;
  checkButton[14] = P_SNUMBER_MEZZ14SNSTS;
  checkButton[15] = P_SNUMBER_MEZZ15SNSTS;
  checkButton[16] = P_SNUMBER_MEZZ16SNSTS;
  checkButton[17] = P_SNUMBER_MEZZ17SNSTS;
  snFile = NULL;
  if (MDTChamberType != CTYPEUNKNOWN) {
    strcpy(str, "database/");
    strcat(str, MDTChamberName);
    strcat(str, "_SN.txt");
    if (snFile = fopen(str, "r")) {
      while (feof(snFile) == 0) {
        fgets(str, 256, snFile);
        strcpy(cstr, "Operator Name");
        strp = strstr(str, cstr);
        if ((strp != NULL) && (strlen(operatorName) <= 0)) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          SetCtrlVal(SerialNumberHandle, P_SNUMBER_OPERATOR, str);
        }

        strcpy(cstr, "MDT Chamber Serial Number");
        strp = strstr(str, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          if (strlen(str) > strlen(MDTChamberSN)) SetCtrlVal(SerialNumberHandle, P_SNUMBER_CHAMBERSN, str);
        }

        strcpy(cstr, "Motherboard Serial Number");
        strp = strstr(str, cstr);
        if ((strp != NULL) && (strlen(motherboardSN) <= 0)) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          SetCtrlVal(SerialNumberHandle, P_SNUMBER_MBSN, str);
        }

        strcpy(cstr, "CSM Serial Number");
        strp = strstr(str, cstr);
        if ((strp != NULL) && (strlen(CSMSN) <= 0)) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          SetCtrlVal(SerialNumberHandle, P_SNUMBER_CSMSN, str);
        }

        strcpy(cstr, "MDT-DCS Serial Number");
        strp = strstr(str, cstr);
        if (strp == NULL) {
          strcpy(cstr, "DCS barcode");
          strp = strstr(str, cstr);
        }
        if (strp == NULL) {
          strcpy(cstr, "DCS Barcode");
          strp = strstr(str, cstr);
        }
        if ((strp != NULL) && (strlen(MDTDCSSN) <= 0)) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          SetCtrlVal(SerialNumberHandle, P_SNUMBER_MDTDCSSN, str);
        }

        strcpy(cstr, "MDT-DCS NIKHEF ID");
        strp = strstr(str, cstr);
        if (strp == NULL) {
          strcpy(cstr, "DCS NIKHEF ID");
          strp = strstr(str, cstr);
        }
        if (strp == NULL) {
          strcpy(cstr, "DCS Nikhef ID");
          strp = strstr(str, cstr);
        }
        if ((strp != NULL) && (strlen(MDTDCSNIKHEFID) <= 0)) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          SetCtrlVal(SerialNumberHandle, P_SNUMBER_NIKHEFID, str);
        }

        strcpy(cstr, "MDT-DCS Expected ELMB Node");
        strp = strstr(str, cstr);
        if (strp == NULL) {
          strcpy(cstr, "DCS node");
          strp = strstr(str, cstr);
        }
        if (strp == NULL) {
          strcpy(cstr, "DCS Node");
          strp = strstr(str, cstr);
        }
        if ((strp != NULL) && ((expectedELMBNode <= 0) || (expectedELMBNode > 127))) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &expectedELMBNode);
          SetCtrlVal(SerialNumberHandle, P_SNUMBER_EXPECTEDELMBNODE, expectedELMBNode);
        }

        strcpy(cstr, "MDT Chamber Start MezzCard");
        strp = strstr(str, cstr);
        if (strp == NULL) {
          strcpy(cstr, "Chamber Start MezzCard");
          strp = strstr(str, cstr);
        }
        if (strp == NULL) {
          strcpy(cstr, "Start Mezzanine Card");
          strp = strstr(str, cstr);
        }
        if ((strp != NULL) && (str[0] != '/') && (str[1] != '/')) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &i);
          if ((i >= 0) && ((i+MDTChamberNumberMezz) < MAXNUMBERMEZZANINE)) {
            startMezzCard = i;
            SetCtrlVal(SerialNumberHandle, P_SNUMBER_STARTMEZZCARD, startMezzCard);
          }
        }

        for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
          if ((snButton[mezz] >= 0) && (((mezzEnables>>mezz)&1) == 1)) {
            GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
            sprintf(cstr, "Mezzanine Card %2d Serial Number", mezz);
            strp = strstr(str, cstr);
            if ((strp != NULL) && (strlen(serialNumber) <= 0)) {
              strl = strlen(cstr);
              for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
              j = 0;
              while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
              for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
              SetCtrlVal(SerialNumberHandle, snButton[mezz], str);
            }
          }
        }
      }
      fclose(snFile);
    }
  } 
  if (snFile == NULL) RecallPanelState(SerialNumberHandle, "SerialNumber_temp", 0);
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (snButton[mezz] >= 0) {
      SetCtrlVal(SerialNumberHandle, checkButton[mezz], FALSE);
    }
  }
  OnChamberDCSBoxUsage();
}


void OnChamberDCSBoxUsage(void) {
  int color1, color2, node;
  
  if (JTAGDRIVERTYPE != CANELMB) {
    SetCtrlVal(SerialNumberHandle, P_SNUMBER_USEDCSONCHAMBER, FALSE);
  }
  GetCtrlVal(SerialNumberHandle, P_SNUMBER_USEDCSONCHAMBER, &useOnChamberDCS);
  GetCtrlVal(SerialNumberHandle, P_SNUMBER_EXPECTEDELMBNODE, &expectedELMBNode);
  if ((ELMBNode < 0) || (ELMBNode > 127)) {
    node = -1;
    color1 = VAL_RED;
  }
  else {
    node = ELMBNode;
    color1 = VAL_WHITE;
  }
  SetCtrlVal(SerialNumberHandle, P_SNUMBER_ACTUALELMBNODE, node);
  if ((expectedELMBNode < 0) || (expectedELMBNode > 127)) {
    expectedELMBNode = -1;
    color2 = VAL_RED;
  }
  else color2 = VAL_GREEN;
  if (useOnChamberDCS) {
    if (ELMBNode != expectedELMBNode) {
      color1 = VAL_RED;
      color2 = VAL_RED;
    }
  }
  else color1 = VAL_BLACK;
  SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_ACTUALELMBNODE, ATTR_TEXT_BGCOLOR, color1);
  SetCtrlVal(SerialNumberHandle, P_SNUMBER_EXPECTEDELMBNODE, expectedELMBNode);
  SetCtrlAttribute(SerialNumberHandle, P_SNUMBER_EXPECTEDELMBNODE, ATTR_TEXT_BGCOLOR, color2);
}


void CheckedAllMezzCardSerialNumber(void) {
  int mezz, checkButton[MAXNUMBERMEZZANINE];

  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    snButton[mezz] = -1;
    checkButton[mezz] = -1;
  }
  snButton[0] = P_SNUMBER_MEZZ00SN;
  snButton[1] = P_SNUMBER_MEZZ01SN;
  snButton[2] = P_SNUMBER_MEZZ02SN;
  snButton[3] = P_SNUMBER_MEZZ03SN;
  snButton[4] = P_SNUMBER_MEZZ04SN;
  snButton[5] = P_SNUMBER_MEZZ05SN;
  snButton[6] = P_SNUMBER_MEZZ06SN;
  snButton[7] = P_SNUMBER_MEZZ07SN;
  snButton[8] = P_SNUMBER_MEZZ08SN;
  snButton[9] = P_SNUMBER_MEZZ09SN;
  snButton[10] = P_SNUMBER_MEZZ10SN;
  snButton[11] = P_SNUMBER_MEZZ11SN;
  snButton[12] = P_SNUMBER_MEZZ12SN;
  snButton[13] = P_SNUMBER_MEZZ13SN;
  snButton[14] = P_SNUMBER_MEZZ14SN;
  snButton[15] = P_SNUMBER_MEZZ15SN;
  snButton[16] = P_SNUMBER_MEZZ16SN;
  snButton[17] = P_SNUMBER_MEZZ17SN;
  checkButton[0] = P_SNUMBER_MEZZ00SNSTS;
  checkButton[1] = P_SNUMBER_MEZZ01SNSTS;
  checkButton[2] = P_SNUMBER_MEZZ02SNSTS;
  checkButton[3] = P_SNUMBER_MEZZ03SNSTS;
  checkButton[4] = P_SNUMBER_MEZZ04SNSTS;
  checkButton[5] = P_SNUMBER_MEZZ05SNSTS;
  checkButton[6] = P_SNUMBER_MEZZ06SNSTS;
  checkButton[7] = P_SNUMBER_MEZZ07SNSTS;
  checkButton[8] = P_SNUMBER_MEZZ08SNSTS;
  checkButton[9] = P_SNUMBER_MEZZ09SNSTS;
  checkButton[10] = P_SNUMBER_MEZZ10SNSTS;
  checkButton[11] = P_SNUMBER_MEZZ11SNSTS;
  checkButton[12] = P_SNUMBER_MEZZ12SNSTS;
  checkButton[13] = P_SNUMBER_MEZZ13SNSTS;
  checkButton[14] = P_SNUMBER_MEZZ14SNSTS;
  checkButton[15] = P_SNUMBER_MEZZ15SNSTS;
  checkButton[16] = P_SNUMBER_MEZZ16SNSTS;
  checkButton[17] = P_SNUMBER_MEZZ17SNSTS;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if ((snButton[mezz] >= 0) && (((mezzEnables>>mezz)&1) == 1)) {
      GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
      if (strlen(serialNumber) > 0) SetCtrlVal(SerialNumberHandle, checkButton[mezz], TRUE);
    }
  }
}


void ReadMDTChamberDatabase(void) {
  int i, j, k, exist, stat, goodSNName, goodNumber;
  char filename[256], str[512], *strp, cstr[40];
  FILE *snFile;

  if (SetDir("database") == 0) SetDir("..");
  else MakeDir("database");
  strcpy(filename, "database/MDTChamberDatabase.txt");
  exist = GetFileInfo(filename, &stat);
  if (!exist) {
    if (snFile = fopen(filename, "w")) {
      fprintf(snFile, "//\n");
      fprintf(snFile, "// This is a MiniDAQ MDT chamber database file\n");
      fprintf(snFile, "//\n"); 
      fprintf(snFile, "// ChamberSerialNumber : MDT Chamber Serial Number\n");
      fprintf(snFile, "// ChamberName         : MDT Chamber Name, must be something\n");
      fprintf(snFile, "//                       like EMS4C01, EOS3A02, BOL4C11 ...\n");
      fprintf(snFile, "// NMLayer             : (optional)\n");
      fprintf(snFile, "//                       Number multilayer\n");
      fprintf(snFile, "// NMezzMLayer         : (Optional)\n");
      fprintf(snFile, "//                       Number mezzanine cards per multilayer\n");
      fprintf(snFile, "// NLayer              : (Optional)\n");
      fprintf(snFile, "//                       Number Layer per multilayer\n");
      fprintf(snFile, "// MezzEnables         : (Optional)\n");
      fprintf(snFile, "//                       Mezzanine Card Enables in Hexdecimal, where\n");
      fprintf(snFile, "//                       bit i is mezzanine card i enable (1/0=on/off).\n");
      fprintf(snFile, "//                       This is optional, if it is not provided or zero,\n");
      fprintf(snFile, "//                       it will be decided by MiniDAQ.\n");
      fprintf(snFile, "// Note: a) Comments are started with \"//\" or \"*\"\n");
      fprintf(snFile, "//       b) Different field is seperated by space, tab, comma or double quota.\n");
      fprintf(snFile, "//\n");
      fprintf(snFile, "ChamberSerialNumber  ChamberName  NMLayer NMezzMLayer NLayer MezzEnables\n");
      fprintf(snFile, "// 20MMALUC000001,BOG2C12,2,5,4,0x003FF  // 3FF mezz 0 to 9 are enabled\n");
      fprintf(snFile, "// 20MMHEPC000001 EOL3A01                // Will take EOL default\n");
      fclose(snFile);
    }
    return;
  }
  chamberIndex = -1;
  nChamberEntry = 0;
  if (snFile = fopen(filename, "r")) {
    while ((feof(snFile) == 0) && (nChamberEntry < 5000)) {
      fgets(str, 512, snFile);
      j = 0;
      while((str[j] == ' ') || str[j] == '\t') j++; 
      for (i = j; i < strlen(str)+1; i++) {
        if ((str[i] == ',') || (str[i] == '"') || (str[i] == '\t') || (str[i] == '\n')) str[i] = ' ';
        str[i-j] = tolower(str[i]);
      }
      if ((str[0] == '*') || ((str[0] == '/') && (str[1] == '/')) || (strlen(str) < 17)) {
        // comments, do nothing
      }
      else {
        strcpy(cstr, "");
        strcpy(cstr, "chamberserialnumber");
        strp = strstr(str, cstr);
        if (strp == NULL) {
          goodSNName = TRUE;
          j = 0;
          while((str[j] == ' ') || str[j] == '\t') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          j = 0;
          k = 0;
          while((str[j] != ' ') && (str[j] != '\t') && (str[j] != ',')) {
            if (str[j] != '"') chamberSNumber[nChamberEntry][k++] = str[j];
            j++;
          }
          chamberSNumber[nChamberEntry][k] = '\0';
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          if (k < 10) goodSNName = FALSE;
          
          if (goodSNName) {
            j = 0;
            while((str[j] == ' ') || str[j] == '\t') j++; 
            for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
            j = 0;
            k = 0;
            while((str[j] != ' ') && (str[j] != '\t') && (str[j] != ',')) {
              if (str[j] != '"') chamberName[nChamberEntry][k++] = str[j];
              j++;
            }
            chamberName[nChamberEntry][k] = '\0';
            for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
            if (k < 6) goodSNName = FALSE;
          }

          nMLayer[nChamberEntry] = 0;
          nMezzMLayer[nChamberEntry] = 0;
          nLayer[nChamberEntry] = 0;
          mezzControl[nChamberEntry] = 0;
          if (goodSNName) {
            goodNumber = TRUE;
            j = 0;
            while((str[j] == ' ') || str[j] == '\t') j++; 
            for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
            j = 0;
            k = 0;
            while((str[j] != ' ') && (str[j] != '\t') && (str[j] != ',')) {
              if (k > 30) k = 30;
              if (str[j] != '"') {
                cstr[k++] = str[j];
                if (!isdigit(str[j])) goodNumber = FALSE;
              }
              j++;
            }
            cstr[k] = '\0';
            for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
            i = 0;
            if (goodNumber) {
              sscanf(cstr, "%d", &i);
              if ((i <= 0) || (i > 2)) goodNumber = FALSE;
              else nMLayer[nChamberEntry] = i;

              j = 0;
              while((str[j] == ' ') || str[j] == '\t') j++; 
              for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
              j = 0;
              k = 0;
              while((str[j] != ' ') && (str[j] != '\t') && (str[j] != ',')) {
                if (k > 30) k = 30;
                if (str[j] != '"') {
                  cstr[k++] = str[j];
                  if (!isdigit(str[j])) goodNumber = FALSE;
                }
                j++;
              }
              cstr[k] = '\0';
              i = 0;
              for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
              if (goodNumber) {
                sscanf(cstr, "%d", &i);
                if (nMLayer[nChamberEntry] == 1) {
                  if ((i <= 0) || (i > MAXNUMBERMEZZANINE/2)) goodNumber = FALSE;
                  else {
                    nMezzMLayer[nChamberEntry] = i;
                    mezzControl[nChamberEntry] = 0;
                    for (j = 0; j < i; j++) mezzControl[nChamberEntry] |= (1 << j);
                  }
                }
                else if (nMLayer[nChamberEntry] == 2) {
                  if ((i <= 0) || (i > MAXNUMBERMEZZANINE/2)) goodNumber = FALSE;
                  else {
                    nMezzMLayer[nChamberEntry] = i;
                    mezzControl[nChamberEntry] = 0;
                    for (j = 0; j < i; j++) mezzControl[nChamberEntry] |= (3 << (2*j));
                  }
                }
              }
            }

            if (goodNumber) {
              j = 0;
              while((str[j] == ' ') || str[j] == '\t') j++; 
              for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
              j = 0;
              k = 0;
              while((str[j] != ' ') && (str[j] != '\t') && (str[j] != ',')) {
                if (k > 30) k = 30;
                if (str[j] != '"') {
                  cstr[k++] = str[j];
                  if (!isdigit(str[j])) goodNumber = FALSE;
                }
                j++;
              }
              cstr[k] = '\0';
              for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
              i = 0;
              if (goodNumber) {
                sscanf(cstr, "%d", &i);
                if ((i < 3) || (i > 4)) goodNumber = FALSE;
                else nLayer[nChamberEntry] = i;
              }
            }

            if (goodNumber) {
              j = 0;
              while((str[j] == ' ') || str[j] == '\t') j++; 
              for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
              j = 0;
              k = 0;
              while((str[j] != ' ') && (str[j] != '\t') && (str[j] != ',')) {
                if (k > 30) k = 30;
                if (str[j] != '"') {
                  cstr[k++] = str[j];
                  if ((!isdigit(str[j])) && (str[j] != 'x') && (str[j] != 'a') && (str[j] != 'b')
                       && (str[j] != 'c') && (str[j] != 'd') && (str[j] != 'e') && (str[j] != 'f')) goodNumber = FALSE;
                }
                j++;
              }
              cstr[k] = '\0';
              i = 0;
              if (goodNumber) {
                sscanf(cstr, "%x", &i);
                k = 0;
                for (j = MAXNUMBERMEZZANINE; j < 32; j++) k |= (1 << j);
                if ((i & k) != 0) goodNumber = FALSE;
                else mezzControl[nChamberEntry] = i;
              }
            }
            strcpy(str, "");

            nChamberEntry++;
          }
        }
      }
    }
    fclose(snFile);
  }
  else {
    sprintf(str, "(WARNING> Unable to open chamber database file <%s>.\n", filename);
    MDTChamberOutput2("%s", str);
    sprintf(str, "Default ASD main threshold (%dmV) will be used for all mezzanine cards.\n", nominalThreshold); 
    MDTChamberOutput2("%s", str);
  }
}


// Routines for Error Report of Data Monitor
//
void UpErrorReport(void)
{
  double sum;
  char str[30];
  
  if (controlOption == NORM) {
    if (runState == State_Idle) {
      DisplayPanel(ErrorReportHandle);
      SetActivePanel(ErrorReportHandle);
      return;
    }
    showErrorReport = TRUE;
    sum = maxUintValue * ((double) analysisCounter[ANALYSEDEVENTHIGH]) + (double) analysisCounter[ANALYSEDEVENT];
    if ((enableAnalysis == 1) && (sum > 0)) {
      SetCtrlAttribute(ErrorReportHandle, P_REPORT_RUNNUMBER, ATTR_TEXT_BGCOLOR, VAL_GREEN);
      SetCtrlAttribute(ErrorReportHandle, P_REPORT_ANALYSEDEVENT, ATTR_TEXT_BGCOLOR, VAL_GREEN);
    }
    else {
      SetCtrlAttribute(ErrorReportHandle, P_REPORT_RUNNUMBER, ATTR_TEXT_BGCOLOR, VAL_MAGENTA);
      SetCtrlAttribute(ErrorReportHandle, P_REPORT_ANALYSEDEVENT, ATTR_TEXT_BGCOLOR, VAL_MAGENTA);
    }
    SetCtrlVal(ErrorReportHandle, P_REPORT_RUNNUMBER, runNumber);
    if (analysisCounter[ANALYSEDEVENTHIGH] > 0) sprintf(str, "%.4e", sum);
    else sprintf(str, "%u", analysisCounter[ANALYSEDEVENT]);
    ResetTextBox(ErrorReportHandle, P_REPORT_ANALYSEDEVENT, str);
    SetErrorReportCounter(P_REPORT_WRONGEVENT, WRONGEVENT);
    SetErrorReportCounter(P_REPORT_WCERROR, WORDCOUNTERERROR);
    SetErrorReportCounter(P_REPORT_EVIDMISMATCH, EVIDMISMATCH);
    SetErrorReportCounter(P_REPORT_BCIDMISMATCH, BCIDMISMATCH);
    SetErrorReportCounter(P_REPORT_EMPTYEVENT, EMPTYEVENT);
    SetErrorReportCounter(P_REPORT_EXTRAWORD, EXTRAWORD);
    SetErrorReportCounter(P_REPORT_CSMHDREVIDERROR, CSMHEADEREVIDMISMATCH);
    SetErrorReportCounter(P_REPORT_CSMTRLEVIDERROR, CSMTRAILEREVIDMISMATCH);
    SetErrorReportCounter(P_REPORT_CSMABTEVIDERROR, CSMABORTEVIDMISMATCH);
    SetErrorReportCounter(P_REPORT_CSMBCIDERROR, CSMBCIDMISMATCH);
    SetErrorReportCounter(P_REPORT_CSMWCERROR, CSMWORDCOUNTERERR);
    SetErrorReportCounter(P_REPORT_CSMERROR, CSMERROR);
    SetErrorReportCounter(P_REPORT_CSMABORT, CSMABORT);
    SetErrorReportCounter(P_REPORT_MISSCSMHEADER, MISSCSMHEADER);
    SetErrorReportCounter(P_REPORT_EXTRACSMHEADER, EXTRACSMHEADER);
    SetErrorReportCounter(P_REPORT_MISSCSMTRAILER, MISSCSMTRAILER);
    SetErrorReportCounter(P_REPORT_EXTRACSMTRAILER, EXTRACSMTRAILER);
    SetErrorReportCounter(P_REPORT_TDCSOFTERROR, TDCSOFTERROR);
    SetErrorReportCounter(P_REPORT_TDCHARDERROR, TDCHARDERROR);
    SetErrorReportCounter(P_REPORT_TDCERROREMPTY, TDCERROREMPTY);
    SetErrorReportCounter(P_REPORT_TDCMASKWORD, TDCMASKWORD);
    SetErrorReportCounter(P_REPORT_TDCMASKEMPTY, TDCMASKEMPTY);
    SetErrorReportCounter(P_REPORT_TDCDEBUGWORD, TDCDEBUGWORD);
    SetErrorReportCounter(P_REPORT_TDCBCIDERROR, TDCBCIDMISMATCH);
    SetErrorReportCounter(P_REPORT_TDCHDREVIDERROR, TDCHEADEREVIDMISMATCH);
    SetErrorReportCounter(P_REPORT_TDCTRLEVIDERROR, TDCTRAILEREVIDMISMATCH);
    SetErrorReportCounter(P_REPORT_MISSTDCHEADER, MISSTDCHEADER);
    SetErrorReportCounter(P_REPORT_EXTRATDCHEADER, EXTRATDCHEADER);
    SetErrorReportCounter(P_REPORT_MISSTDCTRAILER, MISSTDCTRAILER);
    SetErrorReportCounter(P_REPORT_EXTRATDCTRAILER, EXTRATDCTRAILER);
    SetErrorReportCounter(P_REPORT_CSMPARITYERROR, CSMPARITYERROR);
    SetErrorReportCounter(P_REPORT_TDCWCMISMATCH, TDCWCMISMATCH);
    SetErrorReportCounter(P_REPORT_TDCHDAFTERITSDATA, TDCHEADERAFTERITSDATA);
    SetErrorReportCounter(P_REPORT_TDCTRAHEADITSDATA, TDCTRAILERAHEADDATA);
    SetErrorReportCounter(P_REPORT_TDCIDERRINTDCHD, TDCHEADERWRONGTDCID);
    SetErrorReportCounter(P_REPORT_TDCIDERRINTDCTR, TDCTRAILERWRONGTDCID);
    SetErrorReportCounter(P_REPORT_TDCCONTSAMEEDGE, TDCCONTSAMEEDGE);
    SetErrorReportCounter(P_REPORT_TDCUNEXPECTEDDATA, TDCUNEXPECTEDDATA);
    SetErrorReportCounter(P_REPORT_WARNINGEVENT, WARNINGEVENT);
    SetErrorReportCounter(P_REPORT_TDCTIMEWRONGORDER, TDCTIMEWRONGORDER);
    SetErrorReportCounter(P_REPORT_TDCEDGEERROR, TDCEDGEERROR);
    SetErrorReportCounter(P_REPORT_TDCEDGEERRW20, TDCEDGEERRW20);
    SetErrorReportCounter(P_REPORT_EVENTNOTDCHITS, EVENTNOTDCHITS);
    SetErrorReportCounter(P_REPORT_EVENTONLYTDCHT, EVENTONLYTDCHDANDTR);
    SetErrorReportCounter(P_REPORT_WRONGCSMOVERFLOW, WRONGCSMFIFOOVERFLOW);
    SetErrorReportCounter(P_REPORT_UNEXPWRONGTDCWC, UNEXPECTEDTDCWCERROR);
    SetErrorReportCounter(P_REPORT_DISCARDEDTDCWORDS, DISCARDEDTDCWORDS);
    SetErrorReportCounter(P_REPORT_EXPWRONGTDCWC, EXPECTEDTDCWCERROR);
    SetErrorReportCounter(P_REPORT_CSMFIFOOVERFLOW1, CSMFIFOOVERFLOW1);
    SetErrorReportCounter(P_REPORT_CSMFIFOOVERFLOW2, CSMFIFOOVERFLOW2);
    SetErrorReportCounter(P_REPORT_PRESCALEERROR, CSMEVTPRESCALEERR);
    SetErrorReportCounter(P_REPORT_PAIRERROR, CSMPAIRERROR);
    SetErrorReportCounter(P_REPORT_NUMBERHUGEEVENT, NUMBERHUGEEVENT);
    SetErrorReportCounter(P_REPORT_NREQUESTEDPAUSE, NREQUESTEDPAUSE);
    DisplayPanel(ErrorReportHandle);
    SetActivePanel(ErrorReportHandle);
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Error Report", 
                 "Bring up Error Report Panel of Data Monitor.");
  controlOption = NORM;
}


void ErrorReportClose(void)
{
  if (controlOption == NORM) {
    showErrorReport = FALSE;
    HidePanel(ErrorReportHandle);
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Error Report", 
                 "Close Error Report Panel of Data Monitor.");
  controlOption = NORM;
}


void SetErrorReportCounter(int panelIndex, int counterIndex)
{
  int color = VAL_RED, count, version, val, setColor;
  double sum;
  char str[30];
  
  BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
  count = analysisCounter[counterIndex];
  if (counterIndex == ANALYSEDEVENT) {
    sum = maxUintValue * ((double) analysisCounter[ANALYSEDEVENTHIGH]) + (double) count;
    if (analysisCounter[ANALYSEDEVENTHIGH] > 0) sprintf(str, "%.4e", sum);
    else sprintf(str, "%u", count);
    ResetTextBox(ErrorReportHandle, panelIndex, str);
  }
  else SetCtrlVal(ErrorReportHandle, panelIndex, count);
  if (count == 0) color = VAL_GREEN;
  else if (counterIndex == WARNINGEVENT) color = VAL_YELLOW;
  else if (counterIndex == TDCSOFTERROR) color = VAL_YELLOW;
  else if (counterIndex == TDCTIMEWRONGORDER) color = VAL_YELLOW;
  else if (counterIndex == TDCCONTSAMEEDGE) color = VAL_YELLOW;
  else if (counterIndex == TDCEDGEERROR) color = VAL_YELLOW;
  else if (counterIndex == TDCEDGEERRW20) color = VAL_YELLOW;
  else if (counterIndex == EVENTNOTDCHITS) color = VAL_GREEN;
  else if (counterIndex == EVENTONLYTDCHDANDTR) color = VAL_GREEN;
  else if (counterIndex == TDCMASKWORD) color = VAL_GREEN;
  else if (counterIndex == TDCMASKEMPTY) color = VAL_GREEN;
  else if (counterIndex == TDCDEBUGWORD) color = VAL_GREEN;
  else if (counterIndex == NUMBERHUGEEVENT) color = VAL_YELLOW;
  else if (counterIndex == NREQUESTEDPAUSE) color = VAL_WHITE;
  else if (counterIndex == DISCARDEDTDCWORDS) color = VAL_WHITE;
  else if (counterIndex == EXPECTEDTDCWCERROR) color = VAL_WHITE;
  else if (counterIndex == CSMFIFOOVERFLOW1) color = VAL_WHITE;
  else if (counterIndex == CSMFIFOOVERFLOW2) color = VAL_WHITE;
  else color = VAL_RED;
  if (counterIndex == BCIDMISMATCH) {
    if (nbMezzCard == 1 && validBCIDPresetValue == 0) color = VAL_BLACK;
    else if (nbMezzCard <= 0) color = VAL_BLACK;
  }
  if (counterIndex == TDCBCIDMISMATCH && nbMezzCard <= 1) color = VAL_BLACK;
  else if (counterIndex == CSMBCIDMISMATCH && validBCIDPresetValue == 0) color = VAL_BLACK;
  setColor = TRUE;
  if ((version%2) == 1) {
    if ((counterIndex == CSMWORDCOUNTERERR) || (counterIndex == TXPARITYERROR)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "CSM1 TX Parity Error");
      counterIndex = TXPARITYERROR;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == CSMHEADEREVIDMISMATCH) || (counterIndex == NBUFFERPOINTERERROR)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# Buffer PointerError");
      counterIndex = NBUFFERPOINTERERROR;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == CSMERROR) || (counterIndex == NEMPTYCYCLE)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# Empty Cycle");
      counterIndex = NEMPTYCYCLE;
      count = analysisCounter[counterIndex];
      if (analysisCounter[NEMPTYCYCLEHIGH] > 0) {
        SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# Empty Cycle >");
        SetCtrlVal(ErrorReportHandle, panelIndex, 0xFFFFFFFF);
      }
      else SetCtrlVal(ErrorReportHandle, panelIndex, count);
      if (CSMSetupArray[EMPTYCYCLECONTROL] == 0) color = VAL_WHITE;
      else if (count != 0) color = VAL_RED;
      else color = VAL_GREEN;
    }
    else if ((counterIndex == CSMTRAILEREVIDMISMATCH) ||(counterIndex == NBADCYCLE)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# Bad Cycle");
      counterIndex = NBADCYCLE;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_LEFT, 401);
    }
    else if ((counterIndex == CSMBCIDMISMATCH) || (counterIndex == DISCARDEDEVENT)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# DiscardedEvents");
      counterIndex = DISCARDEDEVENT;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
	  if (count != 0) color = VAL_YELLOW;
      else color = VAL_GREEN;
    }
    else if ((counterIndex == CSMABORTEVIDMISMATCH) || (counterIndex == UNKNOWNSWORD)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# Unknow SynchWord");
      counterIndex = UNKNOWNSWORD;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
      if (synchWordControl != STATUSTRIGTIMEINSYNCHWORD) color = VAL_BLACK;
    }
    else if ((counterIndex == CSMABORT) || (counterIndex == CSMHASERROR)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "ReportedCSMError");
      counterIndex = CSMHASERROR;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
      if (synchWordControl != STATUSTRIGTIMEINSYNCHWORD) color = VAL_BLACK;
    }
    else if ((counterIndex == MISSCSMHEADER) || (counterIndex == XMT1CLOCKUNLOCKED) || (counterIndex == XMT2CLOCKUNLOCKED)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "XMT Clock Unlocked");
      counterIndex = XMT1CLOCKUNLOCKED;
      count = analysisCounter[XMT1CLOCKUNLOCKED] + analysisCounter[XMT2CLOCKUNLOCKED];
      if (count < analysisCounter[XMT1CLOCKUNLOCKED]) count = 0xFFFFFFFF;
      if (count < analysisCounter[XMT2CLOCKUNLOCKED]) count = 0xFFFFFFFF;
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
      if (synchWordControl != STATUSTRIGTIMEINSYNCHWORD) color = VAL_BLACK;
    }
    else if ((counterIndex == EXTRACSMHEADER) || (counterIndex == LHCCLOCKUNLOCKED)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "UnlockedLHCClock");
      counterIndex = LHCCLOCKUNLOCKED;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
      if (synchWordControl != STATUSTRIGTIMEINSYNCHWORD) color = VAL_BLACK;
    }
    else if ((counterIndex == MISSCSMTRAILER) || (counterIndex == I2COPERATIONFAILED)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "I2C Operation Failure");
      counterIndex = I2COPERATIONFAILED;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
      if (synchWordControl != STATUSTRIGTIMEINSYNCHWORD) color = VAL_BLACK;
	  else if (count != 0) color = VAL_YELLOW;
      else color = VAL_GREEN;
    }
    else if ((counterIndex == EXTRACSMTRAILER) || (counterIndex == CSMPHASEERROR)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "CSM Phase Error");
      counterIndex = CSMPHASEERROR;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
      if (synchWordControl != STATUSTRIGTIMEINSYNCHWORD) color = VAL_BLACK;
    }
    else setColor = FALSE;
    if (setColor) {
      if ((color == VAL_GREEN) && (count != 0)) color = VAL_RED;
    }
  }
  else {
    if ((counterIndex == CSMWORDCOUNTERERR) || (counterIndex == TXPARITYERROR)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "CSM WordCount Error");
      counterIndex = CSMWORDCOUNTERERR;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == CSMHEADEREVIDMISMATCH) || (counterIndex == NBUFFERPOINTERERROR)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# CSMhdr EVID Error");
      counterIndex = CSMHEADEREVIDMISMATCH;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == CSMERROR) || (counterIndex == NEMPTYCYCLE)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# CSM Error");
      counterIndex = CSMERROR;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == CSMTRAILEREVIDMISMATCH) || (counterIndex == NBADCYCLE)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# CSMtrl EVID Error");
      counterIndex = CSMTRAILEREVIDMISMATCH;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == CSMBCIDMISMATCH) || (counterIndex == DISCARDEDEVENT)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# CSM BCID Error");
      counterIndex = CSMBCIDMISMATCH;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == CSMABORTEVIDMISMATCH) || (counterIndex == UNKNOWNSWORD)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# CSMabt EVID Error");
      counterIndex = CSMABORTEVIDMISMATCH;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == CSMABORT) || (counterIndex == CSMHASERROR)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# CSM Abort");
      counterIndex = CSMABORT;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == MISSCSMHEADER) || (counterIndex == XMT1CLOCKUNLOCKED) || (counterIndex == XMT2CLOCKUNLOCKED)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# Missing CSM Header");
      counterIndex = MISSCSMHEADER;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == EXTRACSMHEADER) || (counterIndex == LHCCLOCKUNLOCKED)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# Extra CSM Header");
      counterIndex = EXTRACSMHEADER;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == MISSCSMTRAILER) || (counterIndex == I2COPERATIONFAILED)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# Missing CSM Trailer");
      counterIndex = MISSCSMTRAILER;
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
    }
    else if ((counterIndex == EXTRACSMTRAILER) || (counterIndex == CSMPHASEERROR)) {
      SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# Extra CSM Trailer");
      count = analysisCounter[counterIndex];
      SetCtrlVal(ErrorReportHandle, panelIndex, count);
      counterIndex = EXTRACSMTRAILER;
    }
    else setColor = FALSE;
    if (setColor) {
      if ((color == VAL_GREEN) && (count != 0)) color = VAL_RED;
    }
  }
  SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_TEXT_BGCOLOR, color);
}


void SaveErrorSummaryFile(void)
{
  int i, maxLength, version, nslot;
  char dataFilename[281], sumFilename[281], name[NUMBERANALYSISCOUNTER][80];
  FILE *sumFile;

  if (controlOption == HELP) {
    MessagePopup("Help on Save Error Report", 
                 "Save Data Error Report into a file.");
    controlOption = NORM;
    return;
  }
  BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
  GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_FILENAME, 0, dataFilename);
  if (strlen(dataFilename) > 4) {
    strncpy(sumFilename, dataFilename, strlen(dataFilename)-4);
    sumFilename[strlen(dataFilename)-4] = '\0';
  }
  else strcpy(sumFilename, "sample_data");
  strcat(sumFilename, ".err");
  for (i = 0; i < NUMBERANALYSISCOUNTER; i++) strcpy(name[i], "");
  strcpy(name[ANALYSEDEVENT], "Number of analyzed events");
  strcpy(name[WRONGEVENT], "Number of events with error");
  strcpy(name[WARNINGEVENT], "Number of events with warning");
  strcpy(name[DISCARDEDEVENT], "Number of discarded events");
  strcpy(name[NUMBERHUGEEVENT], "Number of huge event (>4000 words)");
  strcpy(name[WORDCOUNTERERROR], "Number of word counter error");
  strcpy(name[CSMWORDCOUNTERERR], "Number of CSM word counter error");
  strcpy(name[EVIDMISMATCH], "Number of mismatched EVID");
  strcpy(name[BCIDMISMATCH], "Number of mismatched BCID");
  strcpy(name[EMPTYEVENT], "Number of events with zero data word");
  strcpy(name[EXTRAWORD], "Number of unexpected data words");
  if ((version%2) == 1) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[EXTRAWORDINTDC+i], "Number of unexpected data words for TDC %d", i);
  }
  strcpy(name[CSMHEADEREVIDMISMATCH], "Number of mismatched EVID in CSM header");
  strcpy(name[CSMTRAILEREVIDMISMATCH], "Number of mismatched EVID in CSM trailer");
  strcpy(name[CSMABORTEVIDMISMATCH], "Number of mismatched EVID in CSM abort");
  strcpy(name[CSMBCIDMISMATCH], "Number of mismatched CSM BCID");
  strcpy(name[CSMERROR], "Number of CSM error words");
  for (i = 0; i < MAXNUMBERMEZZANINE; i++)
    sprintf(name[CSMERRORINTDC+i], "Number of CSM errors due to TDC %d", i);
  strcpy(name[CSMABORT], "Number of CSM abort words");
  strcpy(name[MISSCSMHEADER], "Number of events without CSM header");
  strcpy(name[EXTRACSMHEADER], "Number of events with more than 1 CSM header");
  strcpy(name[MISSCSMTRAILER], "Number of events without CSM trailer");
  strcpy(name[EXTRACSMTRAILER], "Number of events with more than 1 CSM trailer");
  strcpy(name[CSMPARITYERROR], "Number of CSM parity error words");
  for (i = 0; i < MAXNUMBERMEZZANINE; i++)
    sprintf(name[PARITYERRORINTDC+i], "Number of parity errors in TDC %d", i);
  strcpy(name[TDCBCIDMISMATCH], "Number of mismatched TDC BCID");
  if ((version%2) == 1) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TDCBCIDMISMATCHINTDC+i], "Number of mismatched TDC BCID for TDC %d", i);
  }
  else {
    strcpy(name[TDCBCIDMISMATCHINTDC+0], "Number of mismatched TDC BCID for TDC 0/16");
    strcpy(name[TDCBCIDMISMATCHINTDC+1], "Number of mismatched TDC BCID for TDC 1/17");
    for (i = 2; i < 16; i++)
      sprintf(name[TDCBCIDMISMATCHINTDC+i], "Number of mismatched TDC BCID for TDC %d", i);
  }
  strcpy(name[TDCHEADEREVIDMISMATCH], "Number of mismatched EVID in TDC header");
  if ((version%2) == 1) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TDCHEADEREVIDMISMATCHINTDC+i], "Number of mismatched EVID in TDC header for TDC %d", i);
  }
  else {
    strcpy(name[TDCHEADEREVIDMISMATCHINTDC+0], "Number of mismatched EVID in TDC header for TDC 0/16");
    strcpy(name[TDCHEADEREVIDMISMATCHINTDC+1], "Number of mismatched EVID in TDC header for TDC 1/17");
    for (i = 2; i < 16; i++)
      sprintf(name[TDCHEADEREVIDMISMATCHINTDC+i], "Number of mismatched EVID in TDC header for TDC %d", i);
  }
  strcpy(name[TDCTRAILEREVIDMISMATCH], "Number of mismatched EVID in TDC trailer");
  if ((version%2) == 1) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TDCTRAILEREVIDMISMATCHINTDC+i], "Number of mismatched EVID in TDC trailer for TDC %d", i);
  }
  else {
    strcpy(name[TDCTRAILEREVIDMISMATCHINTDC+0], "Number of mismatched EVID in TDC trailer for TDC 0/16");
    strcpy(name[TDCTRAILEREVIDMISMATCHINTDC+1], "Number of mismatched EVID in TDC trailer for TDC 1/17");
    for (i = 2; i < 16; i++)
      sprintf(name[TDCTRAILEREVIDMISMATCHINTDC+i], "Number of mismatched EVID in TDC trailer for TDC %d", i);
  }
  strcpy(name[TDCEDGEERROR], "Number of reported errors in TDC edge word");
  for (i = 0; i < MAXNUMBERMEZZANINE; i++)
    sprintf(name[TDCEDGEERRORINTDC+i], "Number of reported errors in TDC %d edge word", i);
  strcpy(name[TDCEDGEERRW20], "Number of TDC edge error (width<20bin)");
  for (i = 0; i < MAXNUMBERMEZZANINE; i++)
    sprintf(name[TDCEDGEERRW20INTDC+i], "Number of TDC %d edge error (width<20bin)", i);
  strcpy(name[MISSTDCHEADER], "Number of events with less TDC headers than expected");
  strcpy(name[EXTRATDCHEADER], "Number of events with more TDC headers than expected");
  strcpy(name[MISSTDCTRAILER], "Number of events with less TDC trailers than expected");
  strcpy(name[EXTRATDCTRAILER], "Number of events with more TDC trailers than expected");
  strcpy(name[TDCSOFTERROR], "Number of TDC soft-error words");
  strcpy(name[TDCHARDERROR], "Number of TDC hard-error words");
  strcpy(name[TDCERROR], "Number of TDC error words");
  strcpy(name[TDCERRORLIST], "Number of TDC coarse count errors");
  strcpy(name[TDCERRORLIST+1], "Number of TDC channel select errors");
  strcpy(name[TDCERRORLIST+2], "Number of TDC level 1 buffer errors");
  strcpy(name[TDCERRORLIST+3], "Number of TDC trigger FIFO errors");
  strcpy(name[TDCERRORLIST+4], "Number of TDC matching state errors");
  strcpy(name[TDCERRORLIST+5], "Number of TDC readout FIFO errors");
  strcpy(name[TDCERRORLIST+6], "Number of TDC readout state errors");
  strcpy(name[TDCERRORLIST+7], "Number of TDC control parity errors");
  strcpy(name[TDCERRORLIST+8], "Number of TDC JTAG parity errors");
  strcpy(name[TDCERRORLIST+9], "Number of TDC level 1 buffer overflow");
  strcpy(name[TDCERRORLIST+10], "Number of TDC trigger FIFO overflow");
  strcpy(name[TDCERRORLIST+11], "Number of TDC readout FIFO overflow");
  strcpy(name[TDCERRORLIST+12], "Number of TDC hit errors");
  strcpy(name[TDCERRORLIST+13], "Number of TDC channel buffer overflow");
  if ((version%2) == 1) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TDCERRORINTDC+i], "Number of TDC error words in TDC %d", i);
  }
  else {
    strcpy(name[TDCERRORINTDC+0], "Number of TDC error words in TDC 0/16");
    strcpy(name[TDCERRORINTDC+1], "Number of TDC error words in TDC 1/17");
    for (i = 2; i < 16; i++)
      sprintf(name[TDCERRORINTDC+i], "Number of TDC error words in TDC %d", i);
  }
  strcpy(name[TDCERROREMPTY], "Number of Empty TDC error");
  strcpy(name[TDCMASKWORD], "Number of TDC mask words");
  if ((version%2) == 1) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TDCMASKWORDINTDC+i], "Number of TDC mask words in TDC %d", i);
  }
  else {
    strcpy(name[TDCMASKWORDINTDC+0], "Number of TDC mask words in TDC 0/16");
    strcpy(name[TDCMASKWORDINTDC+1], "Number of TDC mask words in TDC 1/17");
    for (i = 2; i < 16; i++)
      sprintf(name[TDCMASKWORDINTDC+i], "Number of TDC mask words in TDC %d", i);
  }
  strcpy(name[TDCMASKEMPTY], "Number of Empty TDC mask");
  strcpy(name[TDCDEBUGWORD], "Number of TDC debug words");
  if ((version%2) == 1) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TDCDEBUGWORDINTDC+i], "Number of TDC debug words in TDC %d", i);
  }
  else {
    strcpy(name[TDCDEBUGWORDINTDC+0], "Number of TDC debug words in TDC 0/16");
    strcpy(name[TDCDEBUGWORDINTDC+1], "Number of TDC debug words in TDC 1/17");
    for (i = 2; i < 16; i++)
      sprintf(name[TDCDEBUGWORDINTDC+i], "Number of TDC debug words in TDC %d", i);
  }
  strcpy(name[TDCHEADERWRONGTDCID], "Number of wrong TDCID events in TDCHeader");
  if ((version%2) == 1) nslot = MAXNUMBERMEZZANINE;
  else nslot = 16;
  strcpy(name[TDCTRAILERWRONGTDCID], "Number of wrong TDCID events in TDCTrailer");
  for (i = 0; i < nslot; i++) {
    sprintf(name[TDCHEADERMISSTDCID+i], "Number of missed TDCID=%d in TDCHeader", i);
    sprintf(name[TDCHEADEREXTRATDCID+i], "Number of extra TDCID=%d in TDCHeader", i);
    sprintf(name[TDCTRAILERMISSTDCID+i], "Number of missed TDCID=%d in TDCTrailer", i);
    sprintf(name[TDCTRAILEREXTRATDCID+i], "Number of extra TDCID=%d in TDCTrailer", i);
  }
  strcpy(name[TDCHEADERAFTERITSDATA], "Number of TDC header after its data");
  if ((version%2) == 1) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TDCHEADERAFTERITSDATALIST+i], "Number of TDC header after its data in TDC %d", i);
  }
  else {
    strcpy(name[TDCHEADERAFTERITSDATALIST+0], "Number of TDC header after its data in TDC 0/16");
    strcpy(name[TDCHEADERAFTERITSDATALIST+1], "Number of TDC header after its data in TDC 1/17");
    for (i = 2; i < 16; i++)
      sprintf(name[TDCHEADERAFTERITSDATALIST+i], "Number of TDC header after its data in TDC %d", i);
  }
  strcpy(name[TDCTRAILERAHEADDATA], "Number of TDC trailer ahead its data");
  if ((version%2) == 1) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TDCTRAILERAHEADDATALIST+i], "Number of TDC trailer ahead its data in TDC %d", i);
  }
  else {
    strcpy(name[TDCTRAILERAHEADDATALIST+0], "Number of TDC trailer ahead its data in TDC 0/16");
    strcpy(name[TDCTRAILERAHEADDATALIST+1], "Number of TDC trailer ahead its data in TDC 1/17");
    for (i = 2; i < 16; i++)
      sprintf(name[TDCTRAILERAHEADDATALIST+i], "Number of TDC trailer ahead its data in TDC %d", i);
  }
  strcpy(name[TDCWCMISMATCH], "Number of TDC word counter mismatch");
  if ((version%2) == 1) {
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TDCWCMISMATCHLIST+i], "Number of TDC %d word counter mismatch", i);
  }
  else {
    strcpy(name[TDCWCMISMATCHLIST+0], "Number of TDC 0/16 word counter mismatch");
    strcpy(name[TDCWCMISMATCHLIST+1], "Number of TDC 1/17 word counter mismatch");
    for (i = 2; i < 16; i++)
      sprintf(name[TDCWCMISMATCHLIST+i], "Number of TDC %d word counter mismatch", i);
  }
  strcpy(name[TDCCONTSAMEEDGE], "Number of continued same edge");
  strcpy(name[TDCCONTTRAILINGEDGE], "Number of continued trailing edge");
  strcpy(name[TDCUNEXPECTEDDATA], "Number of unexpected TDC data word");
  for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
    sprintf(name[TDCCONTSAMEEDGEINTDC+i], "Number of continued same edge in TDC %d", i);
    sprintf(name[TDCCONTTRAILINGEDGEINTDC+i], "Number of continued trailing edge in TDC %d", i);
    sprintf(name[TDCUNEXPECTEDDATAINTDC+i], "Number of unexpected TDC data word in TDC %d", i);
  }
  strcpy(name[TDCTIMEWRONGORDER], "Number of TDCtime in wrong order");
  strcpy(name[EVENTNOTDCHITS], "Number of event without TDC hits");
  strcpy(name[EVENTONLYTDCHDANDTR], "Number of event only with TDC Header & Trailer");
  for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
    sprintf(name[TDCTIMEWRONGORDERINTDC+i], "Number of TDCtime in wrong order in TDC %d", i);
    sprintf(name[EVENTNOTDCHITSLIST+i], "Number of event without hits in TDC %d", i);
    sprintf(name[EVENTONLYTDCHDANDTRLIST+i], "Number of event only with Hds & Trls in TDC %d", i);
  }
  strcpy(name[NBCIDERRDUETRIGFIFOOV], "Number of BCID Mismatch due to TDC TrigFIFO Overflow");
  strcpy(name[DISCARDEDTRAILINGONLY], "Number of Discarded TDC Trailing/Mask by CSM");
  strcpy(name[DISCARDEDLEADINGONLY], "Number of Discarded TDC Leading/Error by CSM");
  strcpy(name[DISCARDEDTDCWORDS], "Number of Discarded TDC Words by CSM");
  strcpy(name[EXPECTEDTDCWCERROR], "Number of Expected TDC WC Error");
  strcpy(name[UNEXPECTEDTDCWCERROR], "Number of TDC WC < # Received Words");
  strcpy(name[WRONGCSMFIFOOVERFLOW], "Number of Wrong CSM FIFO Overflows");
  strcpy(name[CSMFIFOOVERFLOW1], "Number of CSM FIFO Overflow 1");
  strcpy(name[CSMFIFOOVERFLOW2], "Number of CSM FIFO Overflow 2");
  strcpy(name[CSMFIFOOVERFLOWS], "Number of CSM FIFO Overflow 1&2");
  strcpy(name[CSMEVTPRESCALEERR], "Number of CSM Event Prescale Error");
  if (CSMPairDebug) strcpy(name[CSMPAIRERROR], "Number of Hits Paired Wrong by CSM");
  for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
    sprintf(name[DISCARDEDTRAILINGONLYINTDC+i], "Number of Discarded TDC Trailing/Mask by CSM in TDC %d", i);
    sprintf(name[DISCARDEDLEADINGONLYINTDC+i], "Number of Discarded TDC Leading/Mask by CSM in TDC %d", i);
    sprintf(name[DISCARDEDTDCWORDSINTDC+i], "Number of Discarded TDC Words by CSM in TDC %d", i);
    sprintf(name[EXPECTEDTDCWCERRORINTDC+i], "Number of Expected TDC WC Error in TDC %d", i);
    sprintf(name[UNEXPECTEDTDCWCERRORINTDC+i], "Number of TDC WC < # Received Words in TDC %d", i);
    sprintf(name[WRONGCSMFIFOOVERFLOWINTDC+i], "Number of Wrong CSM FIFO Overflows in TDC %d", i);
    sprintf(name[CSMFIFOOVERFLOW1INTDC+i], "Number of CSM FIFO Overflow 1 in TDC %d", i);
    sprintf(name[CSMFIFOOVERFLOW2INTDC+i], "Number of CSM FIFO Overflow 2 in TDC %d", i);
    sprintf(name[CSMFIFOOVERFLOWSINTDC+i], "Number of CSM FIFO Overflow 1&2 in TDC %d", i);
    sprintf(name[CSMEVTPRESCALEERRINTDC+i], "Number of CSM Event Prescale Error in TDC %d", i);
    if (CSMPairDebug) sprintf(name[CSMPAIRERRORINTDC+i], "Number of Hits Paired Wrong By CSM in TDC %d", i);
  }
  strcpy(name[NREQUESTEDSTOP], "Number of Requested Stop Run");
  strcpy(name[NREQUESTEDPAUSE], "Number of Requested Pause Run");
  strcpy(name[NTOTALWORD], "Number of Total Received Words");
  strcpy(name[PROCESSEDEVENT], "Number of Processed Events");
  strcpy(name[NDATAWORD], "Number of Data Words");
  strcpy(name[NEVENTWORD], "Number of Data Words for Constructed Events");
  strcpy(name[NGOODCYCLE], "Number of Good Cycles");
  strcpy(name[NEMPTYCYCLE], "Number of Empty Cycles");
  strcpy(name[NBADCYCLE], "Number of Bad Cycles");
  strcpy(name[NBADCYCLEATBEGINNING], "Number of Bad Cycles at Beginning");
  strcpy(name[CURREVENTSIZE], "Current Event Size");
  strcpy(name[MINEVENTSIZE], "Minimum Event Size");
  strcpy(name[MAXEVENTSIZE], "Maximum Event Size");
  strcpy(name[NBUFFERPOINTERERROR], "Number Buffer Pointer Error");
  if ((version%2) == 1) {
    strcpy(name[TXPARITYERROR], "Number of Parity Error (CSM to GOLACard)");
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TXPARITYERRORINTDC+i], "Number of Parity Error (CSM to GOLACard) in TDC %d", i);
    if (synchWordControl == STATUSTRIGTIMEINSYNCHWORD) {
      strcpy(name[LHCCLOCKUNLOCKED], "Reported LHC Clock Unlocked in SynchWord");
      strcpy(name[XMT1CLOCKUNLOCKED], "Reported XMT1 Clock Unlocked in SynchWord");
      strcpy(name[XMT2CLOCKUNLOCKED], "Reported XMT2 Clock Unlocked in SynchWord");
      strcpy(name[CSMPHASEERROR], "Reported Phase Error in SynchWord");
      strcpy(name[I2COPERATIONFAILED], "Reported I2C operation failure in SynchWord");
      strcpy(name[UNEXPECTEDTTCRXSETUP], "Reported Wrong TTCrx Setup in SynchWord");
      strcpy(name[CSMHASERROR], "Reported CSM Error in SynchWord");
      strcpy(name[NUMBERTRIGGER], "Reported Matched Triggers in SynchWord");
      strcpy(name[NUMBERSAMETRIGGER], "Reported Same Triggers in SynchWord");
      strcpy(name[NUMBERTRIGGERLOW], "Reported Trigger Low 16 Bits in SynchWord");
      strcpy(name[NUMBERUNMATCHEDTRIGGERLOW], "Reported Unmatched Trigger Low 16 Bits in SynchWord");
      strcpy(name[NUMBERTRIGGERHIGH], "Reported Trigger High 16 Bits in SynchWord");
      strcpy(name[NUMBERUNMATCHEDTRIGGERHIGH], "Reported Unmatched Trigger High 16 Bits in SynchWord");
      strcpy(name[UNKNOWNSWORD], "Number of Unknown SynchWord");
    }
  }
  if (nbMezzCard <= 1) {
    if (validBCIDPresetValue == 0 || nbMezzCard <= 0) {
      strcpy(name[BCIDMISMATCH], "");
      strcpy(name[CSMBCIDMISMATCH], "");
    }
    strcpy(name[TDCBCIDMISMATCH], "");
    for (i = 0; i < MAXNUMBERMEZZANINE; i++) strcpy(name[TDCBCIDMISMATCHINTDC+i], "");
  }
  maxLength = 0;
  for (i = 0; i < NUMBERANALYSISCOUNTER; i++) 
    if (strlen(name[i]) > maxLength) maxLength = strlen(name[i]);
  if (sumFile = fopen(sumFilename, "w")) {
    if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
      fprintf(sumFile, " --- MiniDAQ Error Summary File for MDT chamber %s ---\n", MDTChamberName);
    }
    else {
      fprintf(sumFile, " -------- MiniDAQ Error Summary File -------------\n");
    }
    if ((analysisCounter[WRONGEVENT] > 0 || analysisCounter[WARNINGEVENT] > 0)) {
      for (i = 0; i < NUMBERANALYSISCOUNTER; i++) 
        ErrorSummaryPrint(sumFile, i, name[i], maxLength);
    }
    else {
      for (i = 0; i < NUMBERANALYSISCOUNTER; i++)
        if (analysisCounter[i] > 0) ErrorSummaryPrint(sumFile, i, name[i], maxLength);
      fprintf(sumFile, " No data error has been found from data monitor.\n");
    }
    fclose(sumFile);
    printf("Error summary has been saved into error summary file <%s>.\n", sumFilename);
  }
  else printf("Unable to open error summary file <%s> for the run!\n", sumFilename);
}


void ErrorSummaryPrint(FILE *sumFile, int i, char name[], int maxLength) {
  int length, high, low;
  double sum;
  char str[50];
  
  length = strlen(name);
  if (length > 0) {
    low = i;
    high = -1;
    if (i == ANALYSEDEVENT) high = ANALYSEDEVENTHIGH;
    else if (i == NTOTALWORD) high = NTOTALWORDHIGH;
    else if (i == PROCESSEDEVENT) high = PROCESSEDEVENTHIGH;
    else if (i == NDATAWORD) high = NDATAWORDHIGH;
    else if (i == NEVENTWORD) high = NEVENTWORDHIGH;
    else if (i == NGOODCYCLE) high = NGOODCYCLEHIGH;
    else if (i == NEMPTYCYCLE) high = NEMPTYCYCLEHIGH;
    if ((high >= 0) && (analysisCounter[high] > 0)) {
      sum = maxUintValue * ((double) analysisCounter[high]) + (double) analysisCounter[low];
      sprintf(str, " %%%ds%%%dc: %%%d.4e\n", length, maxLength-length+1, 12);
      fprintf(sumFile, str, name, ' ', sum);
    }
    else {
      sprintf(str, " %%%ds%%%dc: %%%du\n", length, maxLength-length+1, 12);
      fprintf(sumFile, str, name, ' ', analysisCounter[i]);
    }
  }
}


void SaveRunLogfile(void) {
  double ratio, evtSum, nevtSum, minimumValue;
  int i, j, k, nlist, value, asd, version, enable, numberOfLockLost, linkA, linkB, linkC;
  int ASDMask, gate[3], curr[3], wthre[3], deadtime[3], ccap[3], mode[3], TTCviEvtNB, trig;
  char dataFilename[281], logFilename[281], str[80];
  FILE *logFile;
  
//  CheckCSMAndTTCrxSetup();
//  CheckGOLSetup();
//  CheckAMTSetup();
//  CheckASDSetup();

  for (asd = 0; asd < 3; asd++) {
    gate[asd] = -1;
    curr[asd] = -1;
    wthre[asd] = -1;
    deadtime[asd] = -1;
    ccap[asd] = -1;
    mode[asd] = -1;
  }
  BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
  GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_FILENAME, 0, dataFilename);
  if (strlen(dataFilename) > 4) {
    strncpy(logFilename, dataFilename, strlen(dataFilename)-4);
    logFilename[strlen(dataFilename)-4] = '\0';
  }
  else strcpy(logFilename, "sample_data");
  strcat(logFilename, ".log");
  if (logFile = fopen(logFilename, "w")) {
    fprintf(logFile, " MiniDAQ Run Logfile\n");
    if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
      fprintf(logFile, " MDT Chamber Name           : %60s\n", MDTChamberName);
      fprintf(logFile, " MDT Chamber HV Status      : %60d\n", MDTChamberHVStatus);
      fprintf(logFile, " MDT Chamber Start MezzCard : %60d\n", startMezzCard);
    }
    fprintf(logFile, " CSM Program Version Number : %54c 0x%03X\n", ' ', version);
    fprintf(logFile, " CSMVersionDate (YYYYMMDDhh): %49c %010d\n", ' ', CSMVersionDate);
    GetLabelFromIndex(DAQControlHandle, P_JTAGCTRL_CSMBOARDSTATUS, 0, str);
    fprintf(logFile, " CSM Board Status           : %60s\n", str);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, &enable);
    if (enable) {
      sprintf(str, "(JTAG Instr=0x%02x) ErrBits=0x%05X", CSMAMTPHASEERROR, AMTPhaseError);
      fprintf(logFile, " AMT Phase Error Bits Only  : %60s\n", str);
      if (CSMVersionDate > 2005021522) {
        sprintf(str, "(JTAG Instr=0x%02x) PhaseErr=%d ErrBits=0x%05X", CSMAMTFULLPHASE, AMTFullPhaseErrorBit, AMTFullPhaseError);
        fprintf(logFile, " AMT Full Phase Error Bits  : %60s\n", str);
        sprintf(str, "(JTAG Instr=0x%02x) PhaseSamples (%d-0) = ", CSMAMTFULLPHASE, MAXNUMBERMEZZANINE-1);
        for (j = MAXNUMBERMEZZANINE-1; j >= 0; j--) sprintf(&str[strlen(str)], "%01X", AMTPhaseSamples[j]);
        fprintf(logFile, " AMT Full Phase Samples     : %60s\n", str);
      }
    }
    fprintf(logFile, " Run Number                 : %60u\n", runNumber);
    if (initDAQDoneForRun) strcpy(str, "Done");
    else strcpy(str, "Not Done");
    fprintf(logFile, " Full DAQ Initialization    : %60s\n", str);
    if (initDAQDoneForRun) {
      if (initDAQStatus != 0) strcpy(str, "Failed");
      else strcpy(str, "Success");
      fprintf(logFile, " DAQ Initialization Status  : %60s\n", str);
    }
    fprintf(logFile, " Run Started Time           : %60s\n", WeekdayTimeString(DAQStartTime));
    fprintf(logFile, " Data File Name             : %60s\n", dataFilename);
    if (totalNumberEventHigh == 0) 
      fprintf(logFile, " # Event                    : %60u\n", totalNumberEvent);
    else {
      nevtSum = maxUintValue * ((double) totalNumberEventHigh) + (double) totalNumberEvent;
      fprintf(logFile, " # Event                    : %60.8e\n", nevtSum);
    }
    fprintf(logFile, " # Monitored Event          : %60u\n", analysisCounter[ANALYSEDEVENT]);
    fprintf(logFile, " # Event With Error         : %60u\n", analysisCounter[WRONGEVENT]);
    fprintf(logFile, " # Requested Pause          : %60u\n", analysisCounter[NREQUESTEDPAUSE]);
    if (nominalThresholdUsed) {
      strcpy(str, "From Database (Nominal Setting)");
      fprintf(logFile, " Threshold Settings         : %60s\n", str);
      fprintf(logFile, " Effective Nominal Threshold: %55d (mV)\n", nominalThreshold);
    }
    else {
      strcpy(str, "From User");
      fprintf(logFile, " Threshold Settings         : %60s\n", str);
    }
    fprintf(logFile, " # Mezzanine Cards          : %60d\n", nbMezzCard);
    k = MAXNUMBERMEZZANINE + (MAXNUMBERMEZZANINE-1) / 4;
    str[k--] = '\0';
    for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
      if (((mezzEnables>>j)&1) == 1) str[k--] = '1';
      else str[k--] = '0';
      if (k%5 == 2) str[k--] = ' ';
    }
    fprintf(logFile, " MezzCard Enable Mask(%2d-0) : %60s\n", MAXNUMBERMEZZANINE-1, str);
    GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_MEZZSETUPSTATUS, 0, str);
    fprintf(logFile, " Mezz.Card Init. Status     : %60s\n", str);
    nlist = 0;
    for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
      if ((mezzInitStatus[i]&MEZZINITTYPEMASK) == MEZZINITBYSETUPALL) {
        nlist++;
        str[5*((MAXNUMBERMEZZANINE-1-4*(i/4))/4)+(MAXNUMBERMEZZANINE-1)%4-(i-4*(i/4))] = '1';
      }
      else str[5*((MAXNUMBERMEZZANINE-1-4*(i/4))/4)+(MAXNUMBERMEZZANINE-1)%4-(i-4*(i/4))] = '0';
      str[5*((MAXNUMBERMEZZANINE-i)/4)+MAXNUMBERMEZZANINE%4] = ' ';
    }
    str[5*(MAXNUMBERMEZZANINE/4)+MAXNUMBERMEZZANINE%4] = '\0';
    fprintf(logFile, " # Init. With Same Setting  : %60d\n", nlist);
    fprintf(logFile, " Common Init. Mask(%2d-0)    : %60s\n", MAXNUMBERMEZZANINE-1, str);
    nlist = 0;
    for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
      if ((mezzInitStatus[i]&MEZZINITTYPEMASK) == MEZZINITBYSETUPIND) {
        nlist++;
        str[5*((MAXNUMBERMEZZANINE-1-4*(i/4))/4)+(MAXNUMBERMEZZANINE-1)%4-(i-4*(i/4))] = '1';
      }
      else str[5*((MAXNUMBERMEZZANINE-1-4*(i/4))/4)+(MAXNUMBERMEZZANINE-1)%4-(i-4*(i/4))] = '0';
      str[5*((MAXNUMBERMEZZANINE-i)/4)+MAXNUMBERMEZZANINE%4] = ' ';
    }
    str[5*(MAXNUMBERMEZZANINE/4)+MAXNUMBERMEZZANINE%4] = '\0';
    fprintf(logFile, " # Init. Individually       : %60d\n", nlist);
    fprintf(logFile, " Individual Init. Mask(%2d-0): %60s\n", MAXNUMBERMEZZANINE-1, str);
    for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
      if (mezzInitStatus[i] != MEZZNOINIT) {
        sprintf(str, "0x%08X", mezzInitStatus[i]);
        if ((mezzInitStatus[i]&MEZZINITERROR) == 0) strcat(str, " (Success ");
        else strcat(str, " (Failed ");
        if ((mezzInitStatus[i]&MEZZINITTYPEMASK) == MEZZINITBYSETUPALL) 
          strcat(str, "Init. by SetupAll)");
        else if ((mezzInitStatus[i]&MEZZINITTYPEMASK) == MEZZINITBYSETUPIND)
          strcat(str, "Init. Individually)");
        fprintf(logFile, " Mezz.Card %2d Init. Status  : %60s\n", i, str);
        for (j = 0; j < AMTS_SETUP+AMTSETUP_LENGTH; j++) statusSetupCheck[j] = mezzSetupArray[j][i];
        for (asd = 1; asd < 4; asd++) {
          ReversedBinaryToInt(&value, (asd-1)*ASD_SETUP_LENGTH+ASD_MAIN_THRESH, 8, statusSetupCheck);
          sprintf(str, "%d (DAC Setting Value = %d)", 2*(value-127), value);
          fprintf(logFile, "   ASD%d Main Threshold (mV) : %60s\n", asd, str);
          ReversedBinaryToInt(&value, (asd-1)*ASD_SETUP_LENGTH+ASD_HYSTERESIS, 4, statusSetupCheck);
          sprintf(str, "%.2f (Setting Value = %d)", 1.25*((float) value), value);
          fprintf(logFile, "   ASD%d Hysteresis (mV)     : %60s\n", asd, str);
        }
        for (asd = 1; asd < 4; asd++) {
          ReversedBinaryToInt(&value, (asd-1)*ASD_SETUP_LENGTH+ASD_CAL_CAP, 3, statusSetupCheck);
          if (value != ccap[asd-1]) {
            sprintf(str, "%d (Calib.Cap. Value = %d)", 50*(value+1), value);
            fprintf(logFile, "   ASD%d Calib.Capacitor (fF): %60s\n", asd, str);
          }
          ccap[asd-1] = value;
          ReversedBinaryToInt(&value, (asd-1)*ASD_SETUP_LENGTH+ASD_WIL_INT, 4, statusSetupCheck);
          if (value != gate[asd-1]) {
            sprintf(str, "%.1f (W.Integration Gate Value = %d)", 11.0+1.5*((float) value), value);
            fprintf(logFile, "   ASD%d Wil.Integ.Gate (ns) : %60s\n", asd, str);
          }
          gate[asd-1] = value;
          ReversedBinaryToInt(&value, (asd-1)*ASD_SETUP_LENGTH+ASD_WIL_CUR, 3, statusSetupCheck);
          if (value != curr[asd-1]) {
            sprintf(str, "%.1f (W.Rundown Current Value = %d)", 7.3-0.7*((float) value), value);
            fprintf(logFile, "   ASD%d Wil.RundownCurr.(uA): %60s\n", asd, str);
          }
          curr[asd-1] = value;
          ReversedBinaryToInt(&value, (asd-1)*ASD_SETUP_LENGTH+ASD_WIL_THRESH, 3, statusSetupCheck);
          if (value != wthre[asd-1]) {
            sprintf(str, "%d (Wilkinson Threshold Value = %d)", 126-16*value, value);
            fprintf(logFile, "   ASD%d Wil. Threshold (mV) : %60s\n", asd, str);
          }
          wthre[asd-1] = value;
          ReversedBinaryToInt(&value, (asd-1)*ASD_SETUP_LENGTH+ASD_DEADTIME, 3, statusSetupCheck);
          if (value != deadtime[asd-1]) {
            sprintf(str, "%.1f (Deadtime Value = %d)", 18.3+74.0*((float) value), value);
            fprintf(logFile, "   ASD%d Deadtime (ns)       : %60s\n", asd, str);
          }
          deadtime[asd-1] = value;
          value = statusSetupCheck[(asd-1)*ASD_SETUP_LENGTH+ASD_CHIP_MODE];
          if (value != mode[asd-1]) {
            if (value == 1) sprintf(str, "TOT (Mode Value = 1)");
            else sprintf(str, "ADC (Mode Value = 1)");
            fprintf(logFile, "   ASD%d Chip Mode           : %60s\n", asd, str);
          }
          mode[asd-1] = value;
        }
        ASDMask = 0;
        for (asd = 0; asd < 3; asd++) {
          mode[asd] = statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CHIP_MODE];
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH0] << 0) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH1] << 1) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH2] << 2) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH3] << 3) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH4] << 4) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH5] << 5) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH6] << 6) << (8*asd);
          ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH7] << 7) << (8*asd);
        }
        BinaryToInt(&value, AMTS_SETUP+AMTS_MTCH_WDW, 12, statusSetupCheck);
        sprintf(str, "%.3f (Match Window Value = %d)", ((float) (value+1)*25)/1000., value);
        fprintf(logFile, "   AMT Match Window (us)    : %60s\n", str);
        BinaryToInt(&value, AMTS_SETUP+AMTS_CH0, 24, statusSetupCheck);
        fprintf(logFile, "   AMT Channel Enable Mask  : %49c 0x%08X\n", ' ', value);
        fprintf(logFile, "   ASD Channel Enable Mask  : %49c 0x%08X\n", ' ', ASDMask);
        for (j = (AMTSETUP_LENGTH-1)/60; j >= 0; j--) {
          value = (j+1)*60 - 1;
          if (value > AMTSETUP_LENGTH-1) value = AMTSETUP_LENGTH - 1;
          fprintf(logFile, "   AMT Setup Array (%3d-%3d): ", value, j*60);
          for (k = (j+1)*60-1; k > value; k--) fprintf(logFile, " ");
          for (k = value; k >= j*60; k--) fprintf(logFile, "%1d", mezzSetupArray[AMTS_SETUP+k][i]);
          fprintf(logFile, "\n");
        }
        for (asd = 3; asd > 0; asd--) {
          for (j = (ASD_SETUP_LENGTH-1)/60; j >= 0; j--) {
            value = (j+1)*60 - 1;
            if (value > ASD_SETUP_LENGTH-1) value = ASD_SETUP_LENGTH - 1;
            fprintf(logFile, "   ASD%d Setup Array(%3d-%3d): ", asd, value, j*60);
            for (k = (j+1)*60-1; k > value; k--) fprintf(logFile, " ");
            for (k = value; k >= j*60; k--) fprintf(logFile, "%1d", mezzSetupArray[(asd-1)*ASD_SETUP_LENGTH+k][i]);
            fprintf(logFile, "\n");
          }
        }
      }
    }
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, &enable);
    fprintf(logFile, " CSM JTAG Device Enable     : %60d\n", enable);
    if (enable) {
      GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_CSMSETUPSTATUS, 0, str);
      fprintf(logFile, " CSM (VertexII) Init. Status: %60s\n", str);
      BinaryToInt(&value, AMTCOMMANDDELAY, 7, CSMSetupArray);
      fprintf(logFile, "   AMT Command Delay        : %60d\n", value);
      for (j = (CSMSETUPLENGTH-1)/60; j >= 0; j--) {
        value = (j+1)*60 - 1;
        if (value > CSMSETUPLENGTH-1) value = CSMSETUPLENGTH - 1;
        fprintf(logFile, "   CSM Setup Array (%3d-%3d): ", value, j*60);
        for (k = (j+1)*60-1; k > value; k--) fprintf(logFile, " ");
        for (k = value; k >= j*60; k--) fprintf(logFile, "%1d", CSMSetupArray[k]);
        fprintf(logFile, "\n");
      }
      if (CSMStatusArray[CSMERRORBIT] != 0) strcpy(str, "Has Error");
      else strcpy(str, "Ready");
      fprintf(logFile, "   CSM Internal Status      : %60s\n", str);
      for (j = (CSMSTATUSLENGTH-1)/60; j >= 0; j--) {
        value = (j+1)*60 - 1;
        if (value > CSMSTATUSLENGTH-1) value = CSMSTATUSLENGTH - 1;
        fprintf(logFile, "   CSM StatusArray (%3d-%3d): ", value, j*60);
        for (k = (j+1)*60-1; k > value; k--) fprintf(logFile, " ");
        for (k = value; k >= j*60; k--) fprintf(logFile, "%1d", CSMStatusArray[k]);
        fprintf(logFile, "\n");
      }
    }
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, &enable);
    fprintf(logFile, " TTCrx Enable               : %60d\n", enable);
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSM, &enable);
    if (enable) {
      GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_TTCRXSETUPSTATUS, 0, str);
      fprintf(logFile, " TTCrx Init. Status         : %60s\n", str);
      for (j = (TTCRXSETUPLENGTH-1)/60; j >= 0; j--) {
        value = (j+1)*60 - 1;
        if (value > TTCRXSETUPLENGTH-1) value = TTCRXSETUPLENGTH - 1;
        fprintf(logFile, "   TTCrxSetupArray (%3d-%3d): ", value, j*60);
        for (k = (j+1)*60-1; k > value; k--) fprintf(logFile, " ");
        for (k = value; k >= j*60; k--) fprintf(logFile, "%1d", TTCrxSetupArray[k]);
        fprintf(logFile, "\n");
      }
    }
    GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, &enable);
    fprintf(logFile, " GOL Enable                 : %60d\n", enable);
    if (enable) {
      GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_GOLSETUPSTATUS, 0, str);
      fprintf(logFile, " GOL Init. Status           : %60s\n", str);
      BinaryToInt(&linkA, LINKCONTROLSTATEA, 2, GOLSetupArray);
      BinaryToInt(&linkB, LINKCONTROLSTATEB, 2, GOLSetupArray);
      BinaryToInt(&linkC, LINKCONTROLSTATEC, 2, GOLSetupArray);
      BinaryToInt(&numberOfLockLost, LOSSOFLOCKCOUNTER, 8, GOLSetupArray);
      if (linkA == GOLREADY) strcpy(str, "Ready");
      else if (linkA == GOLOUTOFLOCK) strcpy(str, "PLL is unlocked");
      else if (linkA == GOLLOCKED) strcpy(str, "PLL has acquired lock");
      else strcpy(str, "Transmit loss_of_lock counter");
      fprintf(logFile, "   GOL Link A State         : %60s\n", str);
      if (linkB == GOLREADY) strcpy(str, "Ready");
      else if (linkB == GOLOUTOFLOCK) strcpy(str, "PLL is unlocked");
      else if (linkB == GOLLOCKED) strcpy(str, "PLL has acquired lock");
      else strcpy(str, "Transmit loss_of_lock counter");
      fprintf(logFile, "   GOL Link B State         : %60s\n", str);
      if (linkC == GOLREADY) strcpy(str, "Ready");
      else if (linkC == GOLOUTOFLOCK) strcpy(str, "PLL is unlocked");
      else if (linkC == GOLLOCKED) strcpy(str, "PLL has acquired lock");
      else strcpy(str, "Transmit loss_of_lock counter");
      fprintf(logFile, "   GOL Link C State         : %60s\n", str);
      fprintf(logFile, "   Number of Lock Lost      : %60d\n", numberOfLockLost);
      for (j = (GOLSETUPLENGTH-1)/60; j >= 0; j--) {
        value = (j+1)*60 - 1;
        if (value > GOLSETUPLENGTH-1) value = GOLSETUPLENGTH - 1;
        fprintf(logFile, "   GOL Setup Array (%3d-%3d): ", value, j*60);
        for (k = (j+1)*60-1; k > value; k--) fprintf(logFile, " ");
        for (k = value; k >= j*60; k--) fprintf(logFile, "%1d", GOLSetupArray[k]);
        fprintf(logFile, "\n");
      }
    }
    fprintf(logFile, " MDT Chamber HV Status      : %60d\n", MDTChamberHVStatus);
    fprintf(logFile, " Suppress CSM Header        : %60d\n", suppressCSMHeader);
    fprintf(logFile, " Suppress CSM Trailer       : %60d\n", suppressCSMTrailer);
    fprintf(logFile, " Suppress TDC Header        : %60d\n", suppressTDCHeader);
    fprintf(logFile, " Suppress TDC Trailer       : %60d\n", suppressTDCTrailer);
    fprintf(logFile, " Setted Trigger Deadtime (%%): %60d\n", triggerDeadtime);
    if (gotTTCvi) {
      TTCviReadEventNumber();
      GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_CURRENTEVTNB, &TTCviEvtNB);
      fprintf(logFile, " TTCvi Event Number         : %60d\n", TTCviEvtNB);
    }
    if (calibrationTriggerDelay >= 0)
      fprintf(logFile, " Calibration Trigger Delay  : %60d\n", calibrationTriggerDelay);
    trig = GetTTCviTriggerSelction();
    if (trig >= 0) triggerSelect = trig;
    if (triggerSelect >= 0) {
      fprintf(logFile, " TTCvi Trigger Selection    : %60d\n", triggerSelect);
      if (triggerSelect == 5) { 
        if (triggerRate < 0) triggerRate = (int) ExpectedTriggerRate();
        fprintf(logFile, " TTCvi Trigger Rate (Hz)    : %60d\n", triggerRate);
      }
    }
    minimumValue = minimumTriggerRate;
    if (maximumTriggerRate <= 0.0) {
      averageTriggerRate = 0.0;
      maximumTriggerRate = 0.0;
      minimumValue = 0.0;
    }
    else if ((averageTriggerRate < minimumTriggerRate) || (averageTriggerRate > maximumTriggerRate)) {
      nevtSum = maxUintValue * ((double) totalNumberTriggerHigh) + (double) totalNumberTrigger;
      if (DAQTime > 0) averageTriggerRate = nevtSum / ((double) DAQTime);
    }
    sprintf(str, "%14.4f %14.4f %14.4f", averageTriggerRate, minimumValue, maximumTriggerRate);
    fprintf(logFile, " TrigRate (Hz) (Ave Min Max): %60s\n", str);
    minimumValue = minimumDAQRate;
    if (maximumDAQRate <= 0.0) {
      averageDAQRate = 0.0;
      maximumDAQRate = 0.0;
      minimumValue = 0.0;
    }
    else if ((averageDAQRate < minimumDAQRate) || (averageDAQRate > maximumDAQRate)) {
      nevtSum = maxUintValue * ((double) totalNumberEventHigh) + (double) totalNumberEvent;
      if (DAQTime > 0) averageDAQRate = nevtSum / ((double) DAQTime);
    }
    sprintf(str, "%14.4f %14.4f %14.4f", averageDAQRate, minimumValue, maximumDAQRate);
    fprintf(logFile, " DAQ Rate (Hz) (Ave Min Max): %60s\n", str);
    minimumValue = minimumEventSize;
    if (maximumEventSize <= 0.0) {
      averageEventSize = 0.0;
      maximumEventSize = 0.0;
      minimumValue = 0.0;
    }
    sprintf(str, "%14.4f %14.0f %14.0f", averageEventSize, minimumValue, maximumEventSize);
    fprintf(logFile, " Event Size    (Ave Min Max): %60s\n", str);
    GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_DAQTIME, 0, str);
    fprintf(logFile, " DAQ Time                   : %60s\n", str);
    if (runState == State_Idle) {
      fprintf(logFile, " Run Stopped Time           : %60s\n", WeekdayTimeString(time(NULL)));
      printf("Run information has been saved into logfile <%s>.\n", logFilename);
    }
    else {
      fprintf(logFile, " Run Logfile Saved At       : %60s\n", WeekdayTimeString(time(NULL)));
    }
    fclose(logFile);
  }
  else printf("Unable to open logfile <%s> for the run!\n", logFilename);
}


void SaveResultFile(void) {
  char dataFilename[281], resultFilename[281];
  int toDisk;
  
  GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_FILENAME, 0, dataFilename);
  if (strlen(dataFilename) > 4) {
    strncpy(resultFilename, dataFilename, strlen(dataFilename)-4);
    resultFilename[strlen(dataFilename)-4] = '\0';
  }
  else strcpy(resultFilename, "sample_data");
  strcat(resultFilename, ".prf");
  GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, &toDisk);
  if (toDisk != 0) SaveAllPanelState(resultFilename, 0);
}


void SaveScanResults(void) {
  unsigned int nhit, scanFileOpened, asd, numberEvents, sameSettings, enoughEvents;
  int i, mezz, ch, length, type, thresh[3], ccap[3], AMTMask, ASDMask, exist, value;
  float ratio, fracBadCycles, totalCycles, maxUintValue = 4294967296.0;
  char dataFilename[281], scanFilename[281], ctype[10], initStatus[10], str[512];
  FILE *scanFile;

  if (nbMezzCard <= 0) return;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) snButton[mezz] = -1;
  snButton[0] = P_SNUMBER_MEZZ00SN;
  snButton[1] = P_SNUMBER_MEZZ01SN;
  snButton[2] = P_SNUMBER_MEZZ02SN;
  snButton[3] = P_SNUMBER_MEZZ03SN;
  snButton[4] = P_SNUMBER_MEZZ04SN;
  snButton[5] = P_SNUMBER_MEZZ05SN;
  snButton[6] = P_SNUMBER_MEZZ06SN;
  snButton[7] = P_SNUMBER_MEZZ07SN;
  snButton[8] = P_SNUMBER_MEZZ08SN;
  snButton[9] = P_SNUMBER_MEZZ09SN;
  snButton[10] = P_SNUMBER_MEZZ10SN;
  snButton[11] = P_SNUMBER_MEZZ11SN;
  snButton[12] = P_SNUMBER_MEZZ12SN;
  snButton[13] = P_SNUMBER_MEZZ13SN;
  snButton[14] = P_SNUMBER_MEZZ14SN;
  snButton[15] = P_SNUMBER_MEZZ15SN;
  snButton[16] = P_SNUMBER_MEZZ16SN;
  snButton[17] = P_SNUMBER_MEZZ17SN;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    nhit = 0;
    for (ch = 0; ch < 24; ch++) nhit |= nEdge[1][ch][mezz];
    for (ch = 0; ch < 24; ch++) nhit |= nEdge[0][ch][mezz];
    for (ch = 0; ch < 24; ch++) nhit |= nPair[ch][mezz];
    if ((((mezzEnables>>mezz)&1) == 1) || (nhit != 0)) {
      length = 0;
      if (scanOutputFormat != 0) {
        strcpy(scanFilename, dataDirName);
        strcat(scanFilename, "/scan_");
        length = strlen(scanFilename);
        GetCtrlVal(SerialNumberHandle, snButton[mezz], &scanFilename[length]);
        length = strlen(scanFilename);
      }
      if (length > 9) {
        for (i = 9; i < length; i++) {
          if (!isalnum(scanFilename[i])) 
            if (scanFilename[i] != '.') scanFilename[i] = '_';
        }
      }
      else {
        GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_FILENAME, 0, dataFilename);
        if (strlen(dataFilename) > 4) {
          strncpy(scanFilename, dataFilename, strlen(dataFilename)-4);
          scanFilename[strlen(dataFilename)-4] = '\0';
        }
        else strcpy(scanFilename, "sample_data");
      }
      if (scanOutputFormat == 2) strcat(scanFilename, ".csv");
      else strcat(scanFilename, ".sum");
      if ((mezzInitStatus[mezz]&MEZZINITERROR) == 0) strcpy(initStatus, "  OK");
      else strcpy(initStatus, " Bad");
      scanFileOpened = FALSE;
      exist = GetFileInfo(scanFilename, &i);
      if (!exist) {
        if (scanFile = fopen(scanFilename, "w")) {
          scanFileOpened = TRUE;
          if (scanOutputFormat == 2) {
            fprintf(scanFile, "TDC,");
            fprintf(scanFile, " RunNumber,");
            fprintf(scanFile, " #OfEvents,");
            fprintf(scanFile, " Init,");
            fprintf(scanFile, " AMTCMDDelay,");
            fprintf(scanFile, " CTrigDelay,");
            fprintf(scanFile, " CCap1,");
            fprintf(scanFile, " CCap2,");
            fprintf(scanFile, " CCap3,");
            fprintf(scanFile, " Thresh1,");
            fprintf(scanFile, " Thresh2,");
            fprintf(scanFile, " Thresh3,");
            fprintf(scanFile, " AMTChE,");
            fprintf(scanFile, " ASDChE,");
            fprintf(scanFile, "  Type,");
            fprintf(scanFile, " #HitsCh00,");
            fprintf(scanFile, " #HitsCh01,");
            fprintf(scanFile, " #HitsCh02,");
            fprintf(scanFile, " #HitsCh03,");
            fprintf(scanFile, " #HitsCh04,");
            fprintf(scanFile, " #HitsCh05,");
            fprintf(scanFile, " #HitsCh06,");
            fprintf(scanFile, " #HitsCh07,");
            fprintf(scanFile, " #HitsCh08,");
            fprintf(scanFile, " #HitsCh09,");
            fprintf(scanFile, " #HitsCh10,");
            fprintf(scanFile, " #HitsCh11,");
            fprintf(scanFile, " #HitsCh12,");
            fprintf(scanFile, " #HitsCh13,");
            fprintf(scanFile, " #HitsCh14,");
            fprintf(scanFile, " #HitsCh15,");
            fprintf(scanFile, " #HitsCh16,");
            fprintf(scanFile, " #HitsCh17,");
            fprintf(scanFile, " #HitsCh18,");
            fprintf(scanFile, " #HitsCh19,");
            fprintf(scanFile, " #HitsCh20,");
            fprintf(scanFile, " #HitsCh21,");
            fprintf(scanFile, " #HitsCh22,");
            fprintf(scanFile, " #HitsCh23,");
            fprintf(scanFile, "          DateAndTime");
            fprintf(scanFile, "\n");
          }
          else {
            if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
              fprintf(scanFile, "//--- MiniDAQ Hit Summary File for MDT Chamber %s ---\n", MDTChamberName);
            }
            else {
              fprintf(scanFile, "//---------- MiniDAQ Hit Summary File --------------\n");
            }
            fprintf(scanFile, "//\n");
            fprintf(scanFile, "// TDC            : TDC Number (= Mezzanine card number)\n");
            fprintf(scanFile, "// RunNumber      : Run Number\n");
            fprintf(scanFile, "// #OfEvents      : Number of Events\n");
            fprintf(scanFile, "// Init           : Individual Mezzanine Card Initialization Status\n");
            fprintf(scanFile, "// AMTCMDDelay(ns): AMT Command Delay, unit in ns \n");
            fprintf(scanFile, "// CTrigDelay(ns) : Calibration Trigger Delay, unit in ns \n");
            fprintf(scanFile, "// ASDsCalCap(fF) : ASD1 ASD2 ASD3 calibration capacitor, unit in fF\n");
            fprintf(scanFile, "// ASDsThresh(mV) : ASD1 ASD2 ASD3 thresholds, unit in mV\n");
            fprintf(scanFile, "// AMTChE         : 24 AMT Channel Enable/Disable Mask (Hexdecimal)\n");
            fprintf(scanFile, "// ASDChE         : 24 ASD Calibration Channel Enable/Disable Mask (Hexdecimal)\n");
            fprintf(scanFile, "// Type           : Hit Type  LEdge = Leading Edge\n");
            fprintf(scanFile, "//                            TEdge = Trailing Edge\n");
            fprintf(scanFile, "//                             Pair = Pair\n");
            fprintf(scanFile, "//                             GHit = Good Hits = hits with large width\n");
            fprintf(scanFile, "//                             None = Unknown Type, something wrong!!!\n");
            fprintf(scanFile, "// #HitsChxx      : Number of hits for channel xx, xx from 0 to 23\n");
            fprintf(scanFile, "// DateTime       : DAQ end date and time\n");
            fprintf(scanFile, "//\n");
            fprintf(scanFile, "TDC");
            fprintf(scanFile, " RunNumber");
            fprintf(scanFile, " #OfEvents");
            fprintf(scanFile, " Init");
            fprintf(scanFile, " AMTCMDDelay(ns)");
            fprintf(scanFile, " CTrigDelay(ns)");
            fprintf(scanFile, " ASDsCalCap(fF)");
            fprintf(scanFile, " ASDsThresh(mV)");
            fprintf(scanFile, " AMTChE");
            fprintf(scanFile, " ASDChE");
            fprintf(scanFile, "  Type");
            fprintf(scanFile, " #HitsCh00");
            fprintf(scanFile, " #HitsCh01");
            fprintf(scanFile, " #HitsCh02");
            fprintf(scanFile, " #HitsCh03");
            fprintf(scanFile, " #HitsCh04");
            fprintf(scanFile, " #HitsCh05");
            fprintf(scanFile, " #HitsCh06");
            fprintf(scanFile, " #HitsCh07");
            fprintf(scanFile, " #HitsCh08");
            fprintf(scanFile, " #HitsCh09");
            fprintf(scanFile, " #HitsCh10");
            fprintf(scanFile, " #HitsCh11");
            fprintf(scanFile, " #HitsCh12");
            fprintf(scanFile, " #HitsCh13");
            fprintf(scanFile, " #HitsCh14");
            fprintf(scanFile, " #HitsCh15");
            fprintf(scanFile, " #HitsCh16");
            fprintf(scanFile, " #HitsCh17");
            fprintf(scanFile, " #HitsCh18");
            fprintf(scanFile, " #HitsCh19");
            fprintf(scanFile, " #HitsCh20");
            fprintf(scanFile, " #HitsCh21");
            fprintf(scanFile, " #HitsCh22");
            fprintf(scanFile, " #HitsCh23");
            fprintf(scanFile, "          DateAndTime");
            fprintf(scanFile, "\n");
          }
        }
        else printf("Unable to open hit summary file <%s> for the run!\n", scanFilename);
      }
      else {
        if (scanFile = fopen(scanFilename, "a")) scanFileOpened = TRUE;
        else printf("Unable to open scan summary file <%s> for the run!\n", scanFilename);
      }
      if (lEdgeOn[mezz]) {
        type = 1;
        strcpy(ctype, "LEdge");
      }
      else if (tEdgeOn[mezz]) {
        type = 0;
        strcpy(ctype, "TEdge");
      }
      else if (pairOn[mezz]) {
        type = 2;
        strcpy(ctype, " Pair");
      }
      else {
        type = -1;
        strcpy(ctype, " None");
      }
      if (scanFileOpened) {
        thresh[0] = -999;
        thresh[1] = -999;
        thresh[2] = -999;
        AMTMask = 0;
        ASDMask = 0;
        if (mezzInitStatus[mezz] != MEZZNOINIT) {
          for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++) statusSetupCheck[i] = mezzSetupArray[i][mezz];
          for (asd = 0; asd < 3; asd++) {
            ReversedBinaryToInt(&value, asd*ASD_SETUP_LENGTH+ASD_MAIN_THRESH, 8, statusSetupCheck);
            thresh[asd] = 2*(value-127);
            ReversedBinaryToInt(&value, asd*ASD_SETUP_LENGTH+ASD_CAL_CAP, 3, statusSetupCheck);
            ccap[asd] = 50*(value+1);
            ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH0] << 0) << (8*asd);
            ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH1] << 1) << (8*asd);
            ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH2] << 2) << (8*asd);
            ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH3] << 3) << (8*asd);
            ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH4] << 4) << (8*asd);
            ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH5] << 5) << (8*asd);
            ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH6] << 6) << (8*asd);
            ASDMask |= (statusSetupCheck[asd*ASD_SETUP_LENGTH+ASD_CH7] << 7) << (8*asd);
          }
          BinaryToInt(&AMTMask, AMTS_SETUP+AMTS_CH0, 24, statusSetupCheck);
        }
        numberEvents = nEvents[mezz];
        if (scanOutputFormat == 2) {
          fprintf(scanFile, "%3d,", mezz);
          fprintf(scanFile, " %9u,", runNumber);
          fprintf(scanFile, " %9u,", numberEvents);
          fprintf(scanFile, " %4s,", initStatus);
          fprintf(scanFile, " %11d,", 25*AMTCommandDelay);
          if (calibrationTriggerDelay >= 0) fprintf(scanFile, " %10d,", 25*calibrationTriggerDelay);
          else fprintf(scanFile, " %10d,", -1);
          fprintf(scanFile, " %5d, %5d, %5d,", ccap[0], ccap[1], ccap[2]);
          fprintf(scanFile, " %7d, %7d, %7d,", thresh[0], thresh[1], thresh[2]);
          fprintf(scanFile, " %06X,", AMTMask);
          fprintf(scanFile, " %06X,", ASDMask);
          fprintf(scanFile, " %5s,", ctype);
          for (ch = 0; ch < 24; ch++) {
            if (type == 0) nhit = nEdge[0][ch][mezz];
            else if (type == 1) nhit = nEdge[1][ch][mezz];
            else if (type == 2) nhit = nPair[ch][mezz];
            else nhit = nEdge[1][ch][mezz] + nEdge[0][ch][mezz] + nPair[ch][mezz];
            fprintf(scanFile, " %9u,", nhit);
          }
          fprintf(scanFile, " %20s\n", Date_TimeString(time(NULL)));
        }
        else {
          fprintf(scanFile, "%3d", mezz);
          fprintf(scanFile, " %9u", runNumber);
          fprintf(scanFile, " %9u", numberEvents);
          fprintf(scanFile, " %4s", initStatus);
          fprintf(scanFile, " %15d", 25*AMTCommandDelay);
          if (calibrationTriggerDelay >= 0) fprintf(scanFile, " %14d", 25*calibrationTriggerDelay);
          else fprintf(scanFile, " %14d", -1);
          fprintf(scanFile, " %4d %4d %4d", ccap[0], ccap[1], ccap[2]);
          fprintf(scanFile, " %4d %4d %4d", thresh[0], thresh[1], thresh[2]);
          fprintf(scanFile, " %06X", AMTMask);
          fprintf(scanFile, " %06X", ASDMask);
          fprintf(scanFile, " %5s", ctype);
          for (ch = 0; ch < 24; ch++) {
            if (type == 0) nhit = nEdge[0][ch][mezz];
            else if (type == 1) nhit = nEdge[1][ch][mezz];
            else if (type == 2) nhit = nPair[ch][mezz];
            else nhit = nEdge[1][ch][mezz] + nEdge[0][ch][mezz] + nPair[ch][mezz];
            fprintf(scanFile, " %9u", nhit);
          }
          fprintf(scanFile, " %20s\n", Date_TimeString(time(NULL)));
        }
        fclose(scanFile);
        printf("Scan summary results for TDC %d have been saved into file <%s>\n", mezz, scanFilename);
      }
    }
  }

  scanRun = reqThresholdScan || reqInjectionScan || reqLinearityScan || reqGainScan;
  sameSettings = FALSE;
  enoughEvents = TRUE;
  ratio = 1.0;
  fracBadCycles = 1.0;
  totalCycles = maxUintValue * ((float) analysisCounter[NGOODCYCLEHIGH]) + (float) analysisCounter[NGOODCYCLE];
  if (totalCycles > 0.0) fracBadCycles = ((float) analysisCounter[NBADCYCLE]) / totalCycles;
  if (restartRunAtEventOn && (restartRunAtEvent > 0)) {
    ratio = ((float) numberEvents) / ((float) restartRunAtEvent);
    if (ratio < 0.90) enoughEvents = FALSE;
  }
  else if (stopRunAtEventOn && (stopRunAtEvent > 0)) {
    ratio = ((float) numberEvents) / ((float) stopRunAtEvent);
    if (ratio < 0.90) enoughEvents = FALSE;
  }
  if (numberScanRun == 200) {
    MDTChamberOutput2("More than 200 runs for a scan run, current run (%u) results will not be used.\n", runNumber); 
  }
  else if (runStopStatus != NORMALSTOP) {
    reqRerun = TRUE;
    MDTChamberOutput2("Run is stopped abnormally, current run (%u) results will not be used, request rerun.\n", runNumber); 
  }
  else if (!enoughEvents) {
    reqRerun = TRUE;
    MDTChamberOutput2(">> Ignored Run << Not enough events (#evt = %d), request rerun\n", numberEvents); 
  }
  else if (scanRun && (fracBadCycles > allowedFractionBadCycles)) {
    reqRerun = TRUE;
    MDTChamberOutput2(">> Ignored Run << Run has bad cycle (#badCycle = %d), request rerun\n", analysisCounter[NBADCYCLE]); 
  }
  else if (reqThresholdScan || reqInjectionScan) {
    ReversedBinaryToInt(&value, ASD_MAIN_THRESH, 8, basicSetupArray);
    threshs[0][numberScanRun] = 2*(value-127);
    ReversedBinaryToInt(&value, ASD_SETUP_LENGTH+ASD_MAIN_THRESH, 8, basicSetupArray);
    threshs[1][numberScanRun] = 2*(value-127);
    ReversedBinaryToInt(&value, 2*ASD_SETUP_LENGTH+ASD_MAIN_THRESH, 8, basicSetupArray);
    threshs[2][numberScanRun] = 2*(value-127);
    if (reqThresholdScan && (subScans == (numberSubScans-1))) {
      for (i = 0; i < numberScanRun; i++) {
        for (asd = 0; asd < 3; asd++) {
          if ((threshs[asd][i] == threshs[asd][numberScanRun]) && (abs(threshs[asd][i]) < 250)) {
            sameSettings = TRUE;
            sprintf(str, ">> Ignored Scan Run << with asd %d threshold %d (mV)\n", asd, threshs[asd][numberScanRun]);
            MDTChamberOutput2("%s", str);
            break;
          }
          if (sameSettings) break;
        }
        if (sameSettings) break;
      }
    }
    ASDMask = 0;
    for (asd = 0; asd < 3; asd++) {
      ASDMask |= (basicSetupArray[asd*ASD_SETUP_LENGTH+ASD_CH0] << 0) << (8*asd);
      ASDMask |= (basicSetupArray[asd*ASD_SETUP_LENGTH+ASD_CH1] << 1) << (8*asd);
      ASDMask |= (basicSetupArray[asd*ASD_SETUP_LENGTH+ASD_CH2] << 2) << (8*asd);
      ASDMask |= (basicSetupArray[asd*ASD_SETUP_LENGTH+ASD_CH3] << 3) << (8*asd);
      ASDMask |= (basicSetupArray[asd*ASD_SETUP_LENGTH+ASD_CH4] << 4) << (8*asd);
      ASDMask |= (basicSetupArray[asd*ASD_SETUP_LENGTH+ASD_CH5] << 5) << (8*asd);
      ASDMask |= (basicSetupArray[asd*ASD_SETUP_LENGTH+ASD_CH6] << 6) << (8*asd);
      ASDMask |= (basicSetupArray[asd*ASD_SETUP_LENGTH+ASD_CH7] << 7) << (8*asd);
    }
    injMasks[numberScanRun] = ASDMask;
    trigCount[0][numberScanRun] = numberEvents;
    trigCount[1][numberScanRun] = numberEvents;
    trigCount[2][numberScanRun] = numberEvents;
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      errCount[mezz][0][numberScanRun] = analysisCounter[TDCERRORINTDC+mezz];
      errCount[mezz][1][numberScanRun] = analysisCounter[TDCERRORINTDC+mezz];
      errCount[mezz][2][numberScanRun] = analysisCounter[TDCERRORINTDC+mezz];
      for (ch = 0; ch < 24; ch++) {
        if (((AMTMask & (1 << ch)) != 0) || (!reqThresholdScan) || (thresholdScanMethod == 0)) {
          if (lEdgeOn[mezz]) hitCount[mezz][ch][numberScanRun] = nEdge[1][ch][mezz];
          else if (tEdgeOn[mezz]) hitCount[mezz][ch][numberScanRun] = nEdge[0][ch][mezz];
          else if (pairOn[mezz]) hitCount[mezz][ch][numberScanRun] = nPair[ch][mezz];
          else hitCount[mezz][ch][numberScanRun] = nEdge[1][ch][mezz] + nEdge[0][ch][mezz] + nPair[ch][mezz];
        }
      }
    }
    if (reqInjectionScan) {
      for (i = 0; i < numberScanRun; i++) {
        if (injMasks[i] == injMasks[numberScanRun]) {
          sameSettings = TRUE;
          MDTChamberOutput2(">> Ignored Scan Run << with ASDMask 0x%06X\n", injMasks[numberScanRun]); 
        }
      }
    }
    if (!sameSettings) {
      if ((reqThresholdScan && (subScans == 0)) || (thresholdScanMethod == 0)) numberScanRun++;
      else numberScanRun++;
    }
    else numberIgnoredScanRun++;
  }
  else if (reqLinearityScan) {
    calibTrigDelay[numberScanRun] = calibrationTriggerDelay;
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      errCount[mezz][0][numberScanRun] = analysisCounter[TDCERRORINTDC+mezz];
      errCount[mezz][1][numberScanRun] = analysisCounter[TDCERRORINTDC+mezz];
      errCount[mezz][2][numberScanRun] = analysisCounter[TDCERRORINTDC+mezz];
      for (ch = 0; ch < 24; ch++) {
        measuredTDCTime[numberScanRun][ch][mezz] = timeAverage[ch][mezz];
        measuredTDCTimeResolution[numberScanRun][ch][mezz] = timeSigma[ch][mezz];
        fracUsedInMeasureTDCTime[numberScanRun][ch][mezz] = fracUsed[ch][mezz];
      }
    }
    for (i = 0; i < numberScanRun; i++) {
      if (calibTrigDelay[i] == calibTrigDelay[numberScanRun]) {
        sameSettings = TRUE;
        MDTChamberOutput2(">> Ignored Scan Run << with calibration trigger delay %d\n", calibTrigDelay[numberScanRun]); 
      }
    }
    if (!sameSettings) numberScanRun++;
    else numberIgnoredScanRun++;
  }
  else if (reqGainScan) {
    ReversedBinaryToInt(&value, ASD_CAL_CAP, 3, statusSetupCheck);
    calibCapacitor[numberScanRun] = 50*(value+1);
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      errCount[mezz][0][numberScanRun] = analysisCounter[TDCERRORINTDC+mezz];
      errCount[mezz][1][numberScanRun] = analysisCounter[TDCERRORINTDC+mezz];
      errCount[mezz][2][numberScanRun] = analysisCounter[TDCERRORINTDC+mezz];
      for (ch = 0; ch < 24; ch++) {
        measuredAverageWidth[numberScanRun][ch][mezz] = widthAverage[ch][mezz];
        measuredWidthResolution[numberScanRun][ch][mezz] = widthSigma[ch][mezz];
        fracUsedInMeasureWidth[numberScanRun][ch][mezz] = fracUsedInWidth[ch][mezz];
      }
    }
    for (i = 0; i < numberScanRun; i++) {
      if (calibCapacitor[i] == calibCapacitor[numberScanRun]) {
        sameSettings = TRUE;
        MDTChamberOutput2(">> Ignored Scan Run << with calibration capacitor %d\n", calibCapacitor[numberScanRun]); 
      }
    }
    if (!sameSettings) numberScanRun++;
    else numberIgnoredScanRun++;
  }
}


void SaveEfficiencyAndNoiseRate(void) {
  unsigned int nhit, numberEvents, offset, resp, exist, asd, nNoise, nDead, nLowEffi, nHot, nVeryHot;
  int noiseStatus[24][MAXNUMBERMEZZANINE], tubeStatus[24][MAXNUMBERMEZZANINE];
  int mezz, ch, mappedCh, row, tube, value, i, ndata, order, on, checkStatus, status, oldStatus;
  char dataFilename[281], sumFilename[281], wlogFilename[281], str[512], sstr[80], inforStr[10];
  double nevtSum, thre, effThre[3], gate, x[24], y[24], z[24], coef[3], mse, effi, rate1, rate2, cut;
  double aver, expectedCRRate, expectedCRHits;
  FILE *sumFile, *wlogFile;

  if (nbMezzCard <= 0) return;
  if (!checkSystemNoise && !mappingMDTChamber) return;
  if (reqRerun) {
    if (numberRerun == MAXIMUMRERUN) {
      sprintf(str, "<WARNING> Efficiency and Noise Check Run Failed, unable to check effi. and noise level\n"); 
      MDTChamberOutput2("%s", str);
      if (scanControl == 1) {
        if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
          sprintf(str, "Unable to obtain Results from Efficiency and Noise Check Run\n"
                       "(MDT Chamber HV Status = %d). Possible sources:\n"
                       "  1) Unstable communication between PC and Linux Machine (need reboot PC and Linux Machine);\n"
                       "  2) Trigger rate is too high;\n"
                       "  3) Hardware problems such as BAD cables, BAD boards (MotherBoard, CSM & MezzCards).\n",
                       MDTChamberHVStatus);
        }
        else {
          sprintf(str, "Unable to obtain Results from Efficiency and Noise Check Run\n"
                       "Possible sources:\n"
                       "  1) Unstable communication between PC and Linux Machine (need reboot PC and Linux Machine);\n"
                       "  2) Trigger rate is too high;\n"
                       "  3) Hardware problems such as BAD cables, BAD boards (MotherBoard, CSM & MezzCards).\n");
        }
        if (autoRunOn || (autoRunFromControlFile == 1)) {
          if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
            strcat(str, "\nAre you sure to continue without checking MDT Chamber Efficiency and Noise Level?\n");
          }
          else {
            strcat(str, "\nAre you sure to continue without checking Efficiency and Noise Level?\n");
          }
          resp = ConfirmPopup("Unable to obtain Results from Efficiency and Noise Check Run", str);
          if (resp == 0) {
            while (autoRunFromControlFile == 1) GetNewRunSetups(0);
            autoRunFromControlFile = 0;
            checkCSM = 0;
            checkCSMJTAG = 0;
            checkCSMIO = 0;
            MDTChamberControl = FALSE;
            autoRunOn = FALSE;
            useExternalTrigger = FALSE;
            thresholdScanMethod = 0;
            numberSubScans = 1;
            subScans = numberSubScans - 1;
            MDTChamberOutput2("Total Number of Download CSM        = %30d\n", numberCSMDownload);
            MDTChamberOutput2("Total Number of Failed Download CSM = %30d\n", numberBadCSMDownload);
            MDTChamberOutput2("Total Number of Download MezzCards  = %30d\n", numberMezzDownload);
            MDTChamberOutput2("Total Number of Failed Download Mezz= %30d\n", numberBadMezzDownload);
            MDTChamberOutput2("Total Number of Init DAQ            = %30d\n", numberInitDAQ);
            MDTChamberOutput2("Total Number of Failed Init DAQ     = %30d\n", numberBadInitDAQ);
            MDTChamberOutput2("Total Number of Redone Init DAQ     = %30d\n", numberReInitDAQ);
            MDTChamberOutput2("Total Number of Auto Runs           = %30d\n", numberAutoRun);
            MDTChamberOutput2("Total Number of Bad Auto Runs       = %30d\n", numberBadAutoRun);
            MDTChamberOutput2("Total Number of Rerun in Auto Runs  = %30d\n", numberAutoRerun);
            if (MDTChamberSumFile != NULL) {
              MDTChamberOutput1(">>>>>>>>>>  End MDT Chamber Tests <<<<<<<<<<\n\n");
              MDTChamberTest = FALSE;
              fclose(MDTChamberSumFile);
              MDTChamberSumFile = NULL;
            }
          }
          else {
            MDTChamberOutput1("Confirm to continue ..\n");
          }
        }
        else MessagePopup("Unable to obtain Results from Efficiency and Noise Check Run", str);
      }
      else if (scanControl == 0) {
        if (autoRunOn) {
          sprintf(str, "*********!!! Unable to obtain Results from Efficiency and Noise Check Run !!!********\n"
                       "Continue auto run without warning to user\n");
        }
        else {
          sprintf(str, "*********!!! Unable to obtain Results from Efficiency and Noise Check Run !!!********\n"
                       "Please check corresponding summary file for details.\n");
        }
        MDTChamberOutput2("%s", str);
      }
    }
    return;
  }
  GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_FILENAME, 0, dataFilename);
  if (strlen(dataFilename) > 4) {
    strncpy(sumFilename, dataFilename, strlen(dataFilename)-4);
    sumFilename[strlen(dataFilename)-4] = '\0';
  }
  else strcpy(sumFilename, "sample_data");
  strcat(sumFilename, ".sum");
  numberEvents = 0;
  snButton[17] = P_SNUMBER_MEZZ17SN;
  gate = 0.0;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if ((((mezzEnables>>mezz)&1) == 1)) {
      MapChamberFromMezzCard(MDTChamberType, mezz, ch, &mappedCh, &row, &tube);
      BinaryToInt(&value, AMTS_SETUP+AMTS_MTCH_WDW, 12, statusSetupCheck);
      gate = (double) (value+1)*25;
      if (nEvents[mezz] > numberEvents) numberEvents = nEvents[mezz];
    }
  }
  if (numberEvents <= 0) return;
  ASDNoiseWarningCut = 40.0;
  ASDNoiseCut = 5.0;
  noiseCut0 = 2.0;
  noiseCut1 = 5.0;
  effiCut0 = 0.10;
  effiCut1 = 0.80;
  effiCut2 = 1.20;
  effiCut3 = 2.00;
  fracCRRateCut0 = 0.2;
  fracCRRateCut1 = 0.8;
  exist = GetFileInfo(sumFilename, &i);
  if (!exist) sumFile = fopen(sumFilename, "w");
  else {
    if (sumFile = fopen(sumFilename, "a")) {
      fprintf(sumFile, "//\n");
      fprintf(sumFile, "//\n");
      fprintf(sumFile, "//\n");
    }
  }
  if (!sumFile) {
    sprintf(str, "Unable to efficiency and various noise rate summary file <%s> for the run!\n", sumFilename);
    MDTChamberOutput2("%s", str);
  }
  else {
    fprintf(sumFile, "//---------- MiniDAQ Summary File for Efficiency and Various Noise Rate --------------\n");
    if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
      fprintf(sumFile, "//----------              for MDT Chamber %s                    --------------\n", MDTChamberName);
    }
    MDTChamberFileOutput1(sumFile, "//\n");
    sprintf(str, "// Basic Run Information\n");
    MDTChamberFileOutput2(sumFile, "%s", str);
    sprintf(str, " Run Number                 : %60u\n", runNumber);
    MDTChamberFileOutput2(sumFile, "%s", str);
    if (initDAQDoneForRun) strcpy(sstr, "Done");
    else strcpy(sstr, "Not Done");
    sprintf(str, " Full DAQ Initialization    : %60s\n", sstr);
    MDTChamberFileOutput2(sumFile, "%s", str);
    if (initDAQDoneForRun) {
      if (initDAQStatus != 0) strcpy(sstr, "Failed");
      else strcpy(sstr, "Success");
      sprintf(str, " DAQ Initialization Status  : %60s\n", sstr);
      MDTChamberFileOutput2(sumFile, "%s", str);
    }
    sprintf(str, " Run Started Time           : %60s\n", WeekdayTimeString(DAQStartTime));
    MDTChamberFileOutput2(sumFile, "%s", str);
    sprintf(str, " Data File Name             : %60s\n", dataFilename);
    MDTChamberFileOutput2(sumFile, "%s", str);
    GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, &on);
    if (on) {
      nevtSum = (double) numberEvents;
      sprintf(str, " # Event                    : %60u\n", numberEvents);
      MDTChamberFileOutput2(sumFile, "%s", str);
    }
    else if (totalNumberEventHigh == 0) { 
      nevtSum = (double) numberEvents;
      sprintf(str, " # Event                    : %60u\n", totalNumberEvent);
      MDTChamberFileOutput2(sumFile, "%s", str);
    }
    else {
      nevtSum = maxUintValue * ((double) totalNumberEventHigh) + (double) numberEvents;
      sprintf(str, " # Event                    : %60.8e\n", nevtSum);
      MDTChamberFileOutput2(sumFile, "%s", str);
    }
    sprintf(str, " # Monitored Event          : %60u\n", analysisCounter[ANALYSEDEVENT]);
    MDTChamberFileOutput2(sumFile, "%s", str);
    sprintf(str, " # Event With Error         : %60u\n", analysisCounter[WRONGEVENT]);
    MDTChamberFileOutput2(sumFile, "%s", str);
    sprintf(str, " # Requested Pause          : %60u\n", analysisCounter[NREQUESTEDPAUSE]);
    MDTChamberFileOutput2(sumFile, "%s", str);
    if (nominalThresholdUsed) {
      strcpy(sstr, "From Database (Nominal Setting)");
      sprintf(str, " Threshold Settings         : %60s\n", sstr);
      MDTChamberFileOutput2(sumFile, "%s", str);
      sprintf(str, " Effective Nominal Threshold: %55d (mV)\n", nominalThreshold);
      MDTChamberFileOutput2(sumFile, "%s", str);
    }
    else {
      strcpy(sstr, "From User");
      sprintf(str, " Threshold Settings         : %60s\n", sstr);
      MDTChamberFileOutput2(sumFile, "%s", str);
    }
    sprintf(str, " # Mezzanine Cards          : %60d\n", nbMezzCard);
    MDTChamberFileOutput2(sumFile, "%s", str);
    sprintf(sstr, "%14.4f %14.4f %14.4f", averageTriggerRate, minimumTriggerRate, maximumTriggerRate);
    sprintf(str, " TrigRate (Hz) (Ave Min Max): %60s\n", sstr);
    MDTChamberFileOutput2(sumFile, "%s", str);
    sprintf(sstr, "%14.4f %14.4f %14.4f", averageDAQRate, minimumDAQRate, maximumDAQRate);
    sprintf(str, " DAQ Rate (Hz) (Ave Min Max): %60s\n", sstr);
    MDTChamberFileOutput2(sumFile, "%s", str);
    sprintf(sstr, "%14.4f %14.0f %14.0f", averageEventSize, minimumEventSize, maximumEventSize);
    sprintf(str, " Event Size    (Ave Min Max): %60s\n", sstr);
    MDTChamberFileOutput2(sumFile, "%s", str);
    GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_DAQTIME, 0, sstr);
    sprintf(str, " DAQ Time                   : %60s\n", sstr);
    MDTChamberFileOutput2(sumFile, "%s", str);
    if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
      sprintf(str, " MDT Chamber Name           : %60s\n", MDTChamberName);
      MDTChamberFileOutput2(sumFile, "%s", str);
      sprintf(str, " MDT Chamber HV Status      : %60d\n", MDTChamberHVStatus);
      MDTChamberFileOutput2(sumFile, "%s", str);
      sprintf(str, " MDT Chamber Start MezzCard : %60d\n", startMezzCard);
      MDTChamberFileOutput2(sumFile, "%s", str);
    }
  }
  MDTChamberFileOutput1(sumFile, "//\n");
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    if (MDTChamberHVStatus == MDTCHAMBERHVON) strcpy(sstr, "ON");
    else strcpy(sstr, "OFF");
    sprintf(str, "Results of Efficiency and Various Noise Rate with MDT Chamber HV %s\n", sstr);
    MDTChamberFileOutput2(sumFile, "%s", str);
  }
  else {
    MDTChamberFileOutput1(sumFile, "Results of Efficiency and Various Noise Rate\n");
  }
  MDTChamberFileOutput1(sumFile, "//\n");
  if ((MDTChamberHVStatus == MDTCHAMBERHVON) && mappingMDTChamber) {
    MDTChamberFileOutput1(sumFile, "// --- Description for Efficiency and ASD Noise Level Check Test ---\n");
    MDTChamberFileOutput1(sumFile, "//     The external tracking trigger (typically cosmic ray trigger) is used to\n"); 
    MDTChamberFileOutput1(sumFile, "//   check channel (= MDT chamber tube) efficiency and ASD noise level, where\n"); 
    MDTChamberFileOutput1(sumFile, "//   the ASD noise is the intrinsic noise in the front end electronics (typically\n");
    MDTChamberFileOutput1(sumFile, "//   from mezzanine card); 2) Tube noise which is mainly related with tube,\n"); 
    MDTChamberFileOutput1(sumFile, "//   HV Hedgehog card and Signal Hedgehog card. The mapping of MDT chamber could\n"); 
    MDTChamberFileOutput1(sumFile, "//   be checked by using same data.\n"); 
  }
  else if (MDTChamberHVStatus == MDTCHAMBERHVON) {
    MDTChamberFileOutput1(sumFile, "// --- Description for Various Noise Level Check Test ---\n");
    MDTChamberFileOutput1(sumFile, "//     The random trigger is used to check various noise levels and MDT chamber tube\n");
    MDTChamberFileOutput1(sumFile, "//   efficiency, where two kind noise rates are measured: 1) ASD noise which is the\n"); 
    MDTChamberFileOutput1(sumFile, "//   intrinsic noise in the front end electronics (typically from mezzanine card);\n"); 
    MDTChamberFileOutput1(sumFile, "//   2) Tube noise which is mainly related with tube, HV Hedgehog card and Signal\n");
    MDTChamberFileOutput1(sumFile, "//   Hedgehog card.\n");
  }
  else {
    MDTChamberFileOutput1(sumFile, "// --- Description for Various Noise Level Check Test ---\n");
    MDTChamberFileOutput1(sumFile, "//     The random trigger is used to check various noise levels, where two kind\n"); 
    MDTChamberFileOutput1(sumFile, "//   noise rates are measured: 1) ASD noise which is the intrinsic noise in the\n"); 
    MDTChamberFileOutput1(sumFile, "//   front end electronics (typically from mezzanine card); 2) Tube noise which is\n"); 
    MDTChamberFileOutput1(sumFile, "//   mainly related with tube, HV Hedgehog card and Signal Hedgehog card.\n"); 
  }
  MDTChamberFileOutput1(sumFile, "//\n");
  MDTChamberFileOutput1(sumFile, "// Mezz           : Mezzanine Card Number\n");
  MDTChamberFileOutput1(sumFile, "// Ch             : Channel Number\n");
  MDTChamberFileOutput1(sumFile, "// NumberHits     : Number of good hits = large width\n");
  if (MDTChamberHVStatus == MDTCHAMBERHVON) {
    MDTChamberFileOutput1(sumFile, "// Efficiency     : Channel Relative Efficiency\n");
  }
  if (!mappingMDTChamber) {
    MDTChamberFileOutput1(sumFile, "// NoiseRate(KHz) : None ASD noise rate (i.e., measured with large width hits)\n");
  }
  MDTChamberFileOutput1(sumFile, "// ASDNoise(KHz)  : ASD noise rate (i.e., measured with small width hits)\n");
  MDTChamberFileOutput1(sumFile, "// Row            : Row Number\n");
  MDTChamberFileOutput1(sumFile, "//                  = 0 -> do not know corresponding row number\n");
  MDTChamberFileOutput1(sumFile, "// Tube           : Tube Number\n");
  MDTChamberFileOutput1(sumFile, "//                  = 0 -> do not know corresponding tube number\n");
  MDTChamberFileOutput1(sumFile, "//\n");
  MDTChamberFileOutput1(sumFile, "Mezz");
  MDTChamberFileOutput1(sumFile, " Ch");
  MDTChamberFileOutput1(sumFile, " NumberHits");
  if (MDTChamberHVStatus == MDTCHAMBERHVON) {
    MDTChamberFileOutput1(sumFile, " Efficiency");
  }
  if (!mappingMDTChamber) {
    MDTChamberFileOutput1(sumFile, " NoiseRate(KHz)");
  }
  MDTChamberFileOutput1(sumFile, " ASDNoise(KHz)");
  MDTChamberFileOutput1(sumFile, " Row");
  MDTChamberFileOutput1(sumFile, " Tube");
  MDTChamberFileOutput1(sumFile, "\n");
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++) statusSetupCheck[i] = mezzSetupArray[i][mezz];
    nhit = 0;
    for (ch = 0; ch < 24; ch++) {
      nhit |= nGoodHit[ch][mezz];
      nhit |= nASDNoise[ch][mezz];
    }
    if ((((mezzEnables>>mezz)&1) == 1) || (nhit != 0)) {
      ndata = 0;
      for (ch = 0; ch < 24; ch++) {
        x[ch] = ch;
        y[ch] = nGoodHit[ch][mezz];
        ndata++;
      }
      order = 2;
      status = PolyFit(x, y, ndata, order, z, coef, &mse);
      aver = AverageWithLimit(ndata, y, 0.10);
      for (ch = 0; ch < 24; ch++) {
        MapChamberFromMezzCard(MDTChamberType, mezz, ch, &mappedCh, &row, &tube);
        if (((mezzEnables>>mezz)&1) == 1) {
          BinaryToInt(&value, AMTS_SETUP+AMTS_MTCH_WDW, 12, statusSetupCheck);
          gate = (double) (value+1)*25;
        }
        MDTChamberFileOutput2(sumFile, "%4d", mezz);
        MDTChamberFileOutput2(sumFile, " %2d", ch);
        MDTChamberFileOutput2(sumFile, " %10u", nGoodHit[ch][mezz]);
        if (MDTChamberHVStatus == MDTCHAMBERHVON) {
          if ((row <= 0) || (tube <= 0)) relativeEfficiency[tube-1][row-1] = 1.0;
          else {
            if (aver <= 0.0) {
              if (nGoodHit[ch][mezz] <= 0) relativeEfficiency[tube-1][row-1] = 0.0;
              else relativeEfficiency[tube-1][row-1] = 1.0;
            }
            else relativeEfficiency[tube-1][row-1] = ((double) nGoodHit[ch][mezz]) / aver;
          }
          MDTChamberFileOutput2(sumFile, " %10.3f", relativeEfficiency[tube-1][row-1]);
        }
        if (!mappingMDTChamber) {
          noiseRate[ch][mezz] = ((double) nGoodHit[ch][mezz]) / (gate * 1.0e-6 * nevtSum);
          MDTChamberFileOutput2(sumFile, " %14.2f", noiseRate[ch][mezz]);
        }
        ASDNoiseRate[ch][mezz] = ((double) nASDNoise[ch][mezz]) / (gate * 1.0e-6 * nevtSum);  
        MDTChamberFileOutput2(sumFile, " %13.2f", ASDNoiseRate[ch][mezz]);
        MDTChamberFileOutput2(sumFile, " %3d", row);
        MDTChamberFileOutput2(sumFile, " %4d", tube);
        MDTChamberFileOutput1(sumFile, "\n");
      }
    }
  }
  if (sumFile) {
    fclose(sumFile);
    MDTChamberOutput1("Relative efficiency and various noise rate have been saved into the summary file.\n");
    sprintf(str, "The summary file name is <%s>.\n", sumFilename);
    MDTChamberOutput2("%s", str);
  }

  if (strlen(MDTChamberWarningFilename) <= 5) {
    if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
      strcpy(MDTChamberWarningFilename, dataDirName);
      strcat(MDTChamberWarningFilename, "/");
      strcat(MDTChamberWarningFilename, MDTChamberName);
      strcat(MDTChamberWarningFilename, "_WARNING.log");
      status = GetFileInfo(MDTChamberWarningFilename, &value);
      if (status == 0) {
        MDTChamberWarningFile = fopen(MDTChamberWarningFilename, "w");
        if (MDTChamberWarningFile != NULL) {
          MDTChamberOutput2("WARNING/INFOR message log file for MDT Chamber ECTests is opened at %s\n", WeekdayTimeString(time(NULL)));
          MDTChamberOutput1("\n");
          fclose(MDTChamberWarningFile);
          MDTChamberWarningFile = NULL;
        }
        else {
          MDTChamberOutput2("Unable to open log file <%s> to save WARNING/INFOR message\n", MDTChamberWarningFilename);
          MDTChamberOutput1("WARNING/INFOR messages will only be saved into ECTests Summary File (i.e, this file)\n");
          MDTChamberOutput1("\n");
          strcpy(MDTChamberWarningFilename, "");
        }
      }
    }
  }
  if (strlen(MDTChamberWarningFilename) > 0) {
    status = GetFileInfo(MDTChamberWarningFilename, &value);
    if (status > 0) {
      MDTChamberWarningFile = fopen(MDTChamberWarningFilename, "a");
      if (MDTChamberWarningFile != NULL) {
        fprintf(MDTChamberWarningFile, "\n");
        if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
          if (MDTChamberHVStatus == MDTCHAMBERHVON) strcpy(sstr, "ON");
          else strcpy(sstr, "OFF");
          fprintf(MDTChamberWarningFile, "Checking Results of Efficiency and Various Noise Rate with MDT Chamber HV %s at %s\n", sstr, WeekdayTimeString(time(NULL)));
        }
        else {
          fprintf(MDTChamberWarningFile, "Checking Results of Efficiency and Various Noise Rate at %s\n", WeekdayTimeString(time(NULL)));
        }
        if (nominalThresholdUsed) {
          strcpy(sstr, "From Database (Nominal Setting)");
          fprintf(MDTChamberWarningFile, " Threshold Settings         : %60s\n", sstr);
          fprintf(MDTChamberWarningFile, " Effective Nominal Threshold: %55d (mV)\n", nominalThreshold);
        }
        else {
          strcpy(sstr, "From User");
          fprintf(MDTChamberWarningFile, " Threshold Settings         : %60s\n", sstr);
        }
      }
    }
  }
  MDTChamberOutput1("// Message will be issued at either of following conditions:\n");
  if (nominalThresholdUsed) {
    if (nominalThreshold < -48) strcpy(inforStr, "<Warning>");
    else {
      strcpy(inforStr, "<Warning>");
      sprintf(str, "// * %s High ASD noise rate  : ASD noise rate is equal or larger than %.2fKHz\n", inforStr, ASDNoiseWarningCut);
      strcpy(inforStr, "<-Infor->");
    }
    sprintf(str, "// * %s High ASD noise rate  : ASD noise rate is equal or larger than %.2fKHz\n", inforStr, ASDNoiseCut);
    MDTChamberOutput2("%s", str);
  }
  else {
    strcpy(inforStr, "<Warning>");
    sprintf(str, "// * %s High ASD noise rate  : ASD noise rate is equal or larger than %.2fKHz\n", inforStr, ASDNoiseWarningCut);
    strcpy(inforStr, "<Xxxxxxx>");
    sprintf(str, "// * %s High ASD noise rate  : ASD noise rate is equal or larger than %.2fKHz\n", inforStr, ASDNoiseCut);
    MDTChamberOutput2("%s", str);
    MDTChamberOutput1("//    Xxxxxxx either Warning or -Infor- depending on individual effective ASD threshold.\n"); 
  }
  if ((MDTChamberHVStatus == MDTCHAMBERHVON) && mappingMDTChamber) {
    sprintf(str, "// * <Warning> Dead tube            : efficiency is equal or less than %.3f;\n", effiCut0);
    MDTChamberOutput2("%s", str);
    sprintf(str, "// * <-Infor-> Low efficiency tube  : %.3f < Efficiency <= %.3f;\n", effiCut0, effiCut1);
    MDTChamberOutput2("%s", str);
    sprintf(str, "// * <-Infor-> Hot tube             : %.3f <= Efficiency < %.3f;\n", effiCut2, effiCut3);
    MDTChamberOutput2("%s", str);
    sprintf(str, "// * <Warning> Very hot tube        : efficiency is equal or larger than %.3f;\n", effiCut3);
    MDTChamberOutput2("%s", str);
  }
  else if ((MDTChamberHVStatus != MDTCHAMBERHVON) && checkSystemNoise) {
    sprintf(str, "// * <-Infor-> High tube noise      : %.1fKHz <= tube noise rate < %.1fKHz;\n", noiseCut0, noiseCut1);
    MDTChamberOutput2("%s", str);
    sprintf(str, "// * <Warning> Very high tube noise : tube nose rate is equal or larger than %.1fKHz;\n", noiseCut1);
    MDTChamberOutput2("%s", str);
  }
  else if ((MDTChamberHVStatus == MDTCHAMBERHVON) && checkSystemNoise) {
    sprintf(str, "// * <Warning> Dead tube            : efficiency is equal or less than %.3f;\n", effiCut0);
    MDTChamberOutput2("%s", str);
    sprintf(str, "// * <-Infor-> High tube noise      : %.1fKHz <= tube noise rate < %.1fKHz;\n", noiseCut0, noiseCut1);
    MDTChamberOutput2("%s", str);
    sprintf(str, "// * <Warning> Very high tube noise : tube nose rate is equal or larger than %.1fKHz;\n", noiseCut1);
    MDTChamberOutput2("%s", str);
    sprintf(str, "// * <Warning> Dead tube            : tube noise rate less than  %.3f of expected cosmic ray hit rate;\n", fracCRRateCut0);
    MDTChamberOutput2("%s", str);
    sprintf(str, "// * <-Infor-> Low efficiency tube  : tube noise rate between %.3f and %.3f of expected cosmic ray hit rate;\n", fracCRRateCut0, fracCRRateCut1);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput1("//\n"); 
  nNoise = 0;
  nDead = 0;
  nLowEffi = 0;
  nHot = 0;
  nVeryHot = 0;
  checkStatus = 0;
  if (MDTChamberHVStatus == MDTCHAMBERHVON) offset = RDOUTSTATUSOFFSETHVON;
  else  offset = RDOUTSTATUSOFFSETHVOFF;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    for (ch = 0; ch < 24; ch++) {
      noiseStatus[ch][mezz] = RDOUTCHOK;
      tubeStatus[ch][mezz] = RDOUTCHOK;
    }
    if (((mezzEnables>>mezz)&1) == 1) {
      for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++) statusSetupCheck[i] = mezzSetupArray[i][mezz];
      for (asd = 1; asd < 4; asd++) {
        ReversedBinaryToInt(&value, (asd-1)*ASD_SETUP_LENGTH+ASD_MAIN_THRESH, 8, statusSetupCheck);
        thre = 2.0 * (value - 127);
        ReversedBinaryToInt(&value, (asd-1)*ASD_SETUP_LENGTH+ASD_HYSTERESIS, 4, statusSetupCheck);
        effThre[asd-1] = thre - 1.25*((float) value);
      }
      BinaryToInt(&value, AMTS_SETUP+AMTS_MTCH_WDW, 12, statusSetupCheck);
      gate = (double) (value+1)*25;
      for (ch = 0; ch < 24; ch++) {
        if (nominalThresholdUsed) {
          if (nominalThreshold < -48) {
            strcpy(inforStr, "WARNING");
            cut = ASDNoiseCut;
          }
          else {
            strcpy(inforStr, "-INFOR-");
            cut = ASDNoiseWarningCut;
          }
        }
        else {
          if (effThre[ch/8] < -48.0) {
            strcpy(inforStr, "WARNING");
            cut = ASDNoiseCut;
          }
          else {
            strcpy(inforStr, "-INFOR-");
            cut = ASDNoiseWarningCut;
          }
        }
        status = RDOUTCHOK;
        rate1 = noiseRate[ch][mezz];
        rate2 = ASDNoiseRate[ch][mezz];
        if (rate2 >= ASDNoiseWarningCut) {
          status = RDOUTCHASDNOISE;
          sprintf(str, "<WARNING> High ASD noise rate (= %8.2f KHz) Readout channel (mezz = %2d channel = %2d)\n", rate2, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (rate2 >= ASDNoiseCut) {
          status = RDOUTCHASDNOISE;
          sprintf(str, "<%s> High ASD noise rate (= %8.2f KHz) Readout channel (mezz = %2d channel = %2d)\n", inforStr, rate2, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        if (rate2 >= cut) {
          nNoise++;
          noiseStatus[ch][mezz] = RDOUTCHASDNOISE;
        }
        MapChamberFromMezzCard(MDTChamberType, mezz, ch, &mappedCh, &row, &tube);
        if (mappedCh >= 0) {
          effi = relativeEfficiency[tube-1][row-1];
          if ((MDTChamberHVStatus == MDTCHAMBERHVON) && mappingMDTChamber) {
            if (effi <= effiCut0) {
              status = RDOUTCHDEAD;
              nDead++;
              sprintf(str, "<WARNING> Dead Readout channel (effi = %.2f mezz = %2d channel = %2d)\n", effi, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
            else if (effi <= effiCut1) {
              status = RDOUTCHLOWEFFICIENCY;
              nLowEffi++;
              sprintf(str, "<-INFOR-> Low Efficiency Readout channel (effi = %.2f mezz = %2d channel = %2d)\n", effi, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
            else if (effi >= effiCut3) {
              status = RDOUTCHVERYHOT;
              nVeryHot++;
              sprintf(str, "<WARNING> Very Hot Readout channel (effi = %6.2f mezz = %2d channel = %2d)\n", effi, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
            else if (effi >= effiCut2) {
              status = RDOUTCHHOT;
              nHot++;
              sprintf(str, "<-INFOR-> Hot Readout channel (effi = %6.2f mezz = %2d channel = %2d)\n", effi, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
          }
          else if ((MDTChamberHVStatus != MDTCHAMBERHVON) && checkSystemNoise) {
            if (rate1 >= noiseCut1) {
              status = RDOUTCHVERYHOT;
              nVeryHot++;
              sprintf(str, "<WARNING> Very Hot Readout channel (noiseRate = %8.2f KHz mezz = %2d channel = %2d)\n", rate1, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
            else if (rate1 >= noiseCut0) {
              status = RDOUTCHHOT;
              nHot++;
              sprintf(str, "<-INFOR-> Hot Readout channel (noiseRate = %8.2f KHz mezz = %2d channel = %2d)\n", rate1, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
          }
          else if ((MDTChamberHVStatus == MDTCHAMBERHVON) && checkSystemNoise) {
            expectedCRRate = (1.0/60.0) * 3.0 * 0.1 * TubeLength(MDTChamberName, row, tube);
            expectedCRHits = expectedCRRate * gate * 1.0e-9 * nevtSum;
            expectedCRRate *= 0.001;
            if (effi <= effiCut0) {
              status = RDOUTCHDEAD;
              nDead++;
              sprintf(str, "<WARNING> Dead Readout channel (effi = %.2f mezz = %2d channel = %2d)\n", effi, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
            else if (rate1 >= noiseCut1) {
              status = RDOUTCHVERYHOT;
              nVeryHot++;
              sprintf(str, "<WARNING> Very Hot Readout channel (noiseRate = %8.2f KHz mezz = %2d channel = %2d)\n", rate1, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
            else if ((expectedCRHits > 20.0) && (rate1 < fracCRRateCut0*expectedCRRate)) {
              status = RDOUTCHDEAD;
              nDead++;
              sprintf(str, "<WARNING> Dead Readout channel (noiseRate = %8.4f KHz mezz = %2d channel = %2d)\n", rate1, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
            else if ((expectedCRHits > 20.0) && (rate1 < fracCRRateCut1*expectedCRRate)) {
              status = RDOUTCHLOWEFFICIENCY;
              nLowEffi++;
              sprintf(str, "<-INFOR-> Low Efficiency Readout channel (noiseRate = %8.4f KHz mezz = %2d channel = %2d)\n", rate1, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
            else if (rate1 >= noiseCut0) {
              status = RDOUTCHHOT;
              nHot++;
              sprintf(str, "<-INFOR-> Hot Readout channel (noiseRate = %8.2f KHz mezz = %2d channel = %2d)\n", rate1, mezz, ch);
              MDTChamberOutput2("%s", str);
            }
          }
          tubeStatus[ch][mezz] = status;
          if (status != RDOUTCHOK) checkStatus = status;
          if (MDTChamberHVStatus == MDTCHAMBERHVON) {
            oldStatus = channelStatusHVOn[ch][mezz] - offset;
            if (oldStatus == RDOUTCHOK) channelStatusHVOn[ch][mezz] = status + offset;
          }
          else {
            oldStatus = channelStatusHVOff[ch][mezz] - offset;
            if (oldStatus == RDOUTCHOK) channelStatusHVOff[ch][mezz] = status + offset;
          }
        }
      }
    }
  }
  wlogFile = NULL;
  if (strlen(MDTChamberName) > 5) {
    if (nominalThresholdUsed) sprintf(sstr, "%2dmV", abs(nominalThreshold));
    else strcpy(sstr, "XXmV");
    if (mappingMDTChamber) strcat(sstr, "CosmicRay");
    else strcat(sstr, "Noise");
    if (MDTChamberHVStatus == MDTCHAMBERHVON) strcat(sstr, "_ON");
    else strcat(sstr, "_OFF");
    strcpy(wlogFilename, dataDirName);
    strcat(wlogFilename, "/");
    strcat(wlogFilename, MDTChamberName);
    strcat(wlogFilename, "_WARNING_");
    strcat(wlogFilename, sstr);
    strcat(wlogFilename, ".log");
    status = GetFileInfo(wlogFilename, &value);
    if (status == 0) {
      wlogFile = fopen(wlogFilename, "w");
      if (wlogFile != NULL) {
        if ((MDTChamberHVStatus == MDTCHAMBERHVON) && mappingMDTChamber) {
          sprintf(str, "MDT Chamber check file <%s> for efficiency and ASD noise is opend at %s\n", wlogFilename, WeekdayTimeString(time(NULL)));
          MDTChamberFileOutput2(wlogFile, "%s", str);
          fprintf(wlogFile, "// --- Description for Efficiency and ASD Noise Level Check Test ---\n");
          fprintf(wlogFile, "//     The external tracking trigger (typically cosmic ray trigger) is used to\n"); 
          fprintf(wlogFile, "//   check channel (= MDT chamber tube) efficiency and ASD noise level, where\n"); 
          fprintf(wlogFile, "//   the ASD noise is the intrinsic noise in the front end electronics (typically\n");
          fprintf(wlogFile, "//   from mezzanine card); 2) Tube noise which is mainly related with tube,\n"); 
          fprintf(wlogFile, "//   HV Hedgehog card and Signal Hedgehog card. The mapping of MDT chamber could\n"); 
          fprintf(wlogFile, "//   be checked by using same data.\n"); 
        }
        else if (MDTChamberHVStatus == MDTCHAMBERHVON) {
          sprintf(str, "MDT Chamber check file <%s> for various noise is opend at %s\n", wlogFilename, WeekdayTimeString(time(NULL)));
          MDTChamberFileOutput2(wlogFile, "%s", str);
          fprintf(wlogFile, "// --- Description for Various Noise Level Check Test ---\n");
          fprintf(wlogFile, "//     The random trigger is used to check various noise levels and MDT chamber tube\n");
          fprintf(wlogFile, "//   efficiency, where two kind noise rates are measured: 1) ASD noise which is the\n"); 
          fprintf(wlogFile, "//   intrinsic noise in the front end electronics (typically from mezzanine card);\n"); 
          fprintf(wlogFile, "//   2) Tube noise which is mainly related with tube, HV Hedgehog card and Signal\n");
          fprintf(wlogFile, "//   Hedgehog card.\n");
        }
        else {
          sprintf(str, "MDT Chamber check file <%s> for various noise is opend at %s\n", wlogFilename, WeekdayTimeString(time(NULL)));
          MDTChamberFileOutput2(wlogFile, "%s", str);
          fprintf(wlogFile, "// --- Description for Various Noise Level Check Test ---\n");
          fprintf(wlogFile, "//     The random trigger is used to check various noise levels, where two kind\n"); 
          fprintf(wlogFile, "//   noise rates are measured: 1) ASD noise which is the intrinsic noise in the\n"); 
          fprintf(wlogFile, "//   front end electronics (typically from mezzanine card); 2) Tube noise which is\n"); 
          fprintf(wlogFile, "//   mainly related with tube, HV Hedgehog card and Signal Hedgehog card.\n"); 
        }
        fprintf(wlogFile, "// Note! All results from tests done at different time will be saved, please ALLWAYS use the latest test results.\n"); 
        fprintf(wlogFile, "//\n"); 
        fprintf(wlogFile, "// Mezz           : Mezzanine Card Number\n");
        fprintf(wlogFile, "// Ch             : Channel Number\n");
        fprintf(wlogFile, "// NumberHits     : Number of good hits = large width\n");
        if (MDTChamberHVStatus == MDTCHAMBERHVON) {
          fprintf(wlogFile, "// Efficiency     : Channel Relative Efficiency\n");
        }
        if (!mappingMDTChamber) {
          fprintf(wlogFile, "// NoiseRate(KHz) : None ASD noise rate (i.e., measured with large width hits)\n");
        }
        fprintf(wlogFile, "// ASDNoise(KHz)  : ASD noise rate (i.e., measured with small width hits)\n");
        fprintf(wlogFile, "// Row            : Row Number\n");
        fprintf(wlogFile, "//                  = 0 -> do not know corresponding row number\n");
        fprintf(wlogFile, "// Tube           : Tube Number\n");
        fprintf(wlogFile, "//                  = 0 -> do not know corresponding tube number\n");
        fprintf(wlogFile, "//\n");
        MDTChamberFileOutput1(wlogFile, "\n");
      }
      else {
        MDTChamberFileOutput2(wlogFile, "Unable to open new MDT chamber check file <%s>\n", wlogFilename);
        MDTChamberFileOutput1(wlogFile, "WARNING/INFOR messages will only be saved into ECTests Summary File (i.e, this file)\n");
        MDTChamberFileOutput1(wlogFile, "\n");
      }
    }
    else {
      wlogFile = fopen(wlogFilename, "a");
      if (wlogFile == NULL) {
        MDTChamberFileOutput2(wlogFile, "Unable to open old MDT chamber check file <%s>\n", wlogFilename);
        MDTChamberFileOutput1(wlogFile, "WARNING/INFOR messages will only be saved into ECTests Summary File (i.e, this file)\n");
        MDTChamberFileOutput1(wlogFile, "\n");
      }
    }
  }
  
  if (wlogFile != NULL) {
    fprintf(wlogFile, "//\n"); 
    if ((MDTChamberHVStatus == MDTCHAMBERHVON) && mappingMDTChamber) {
      fprintf(wlogFile, "Check efficiency and ASD noise level at %s\n", WeekdayTimeString(time(NULL)));
    }
    else {
      fprintf(wlogFile, "Check various noise level at %s\n", WeekdayTimeString(time(NULL)));
    }
    fprintf(wlogFile, "// Basic Run Information\n");
    fprintf(wlogFile, " Run Number                 : %60u\n", runNumber);
    if (initDAQDoneForRun) strcpy(sstr, "Done");
    else strcpy(sstr, "Not Done");
    fprintf(wlogFile, " Full DAQ Initialization    : %60s\n", sstr);
    if (initDAQDoneForRun) {
      if (initDAQStatus != 0) strcpy(sstr, "Failed");
      else strcpy(sstr, "Success");
      fprintf(wlogFile, " DAQ Initialization Status  : %60s\n", sstr);
    }
    fprintf(wlogFile, " Run Started Time           : %60s\n", WeekdayTimeString(DAQStartTime));
    fprintf(wlogFile, " Data File Name             : %60s\n", dataFilename);
    GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, &on);
    if (on) {
      nevtSum = (double) numberEvents;
      fprintf(wlogFile, " # Event                    : %60u\n", numberEvents);
    }
    else if (totalNumberEventHigh == 0) { 
      nevtSum = (double) numberEvents;
      fprintf(wlogFile, " # Event                    : %60u\n", totalNumberEvent);
    }
    else {
      nevtSum = maxUintValue * ((double) totalNumberEventHigh) + (double) numberEvents;
      fprintf(wlogFile, " # Event                    : %60.8e\n", nevtSum);
    }
    fprintf(wlogFile, " # Monitored Event          : %60u\n", analysisCounter[ANALYSEDEVENT]);
    fprintf(wlogFile, " # Event With Error         : %60u\n", analysisCounter[WRONGEVENT]);
    fprintf(wlogFile, " # Requested Pause          : %60u\n", analysisCounter[NREQUESTEDPAUSE]);
    if (nominalThresholdUsed) {
      strcpy(sstr, "From Database (Nominal Setting)");
      fprintf(wlogFile, " Threshold Settings         : %60s\n", sstr);
      fprintf(wlogFile, " Effective Nominal Threshold: %55d (mV)\n", nominalThreshold);
    }
    else {
      strcpy(sstr, "From User");
      fprintf(wlogFile, " Threshold Settings         : %60s\n", sstr);
    }
    fprintf(wlogFile, " # Mezzanine Cards          : %60d\n", nbMezzCard);
    sprintf(sstr, "%14.4f %14.4f %14.4f", averageTriggerRate, minimumTriggerRate, maximumTriggerRate);
    fprintf(wlogFile, " TrigRate (Hz) (Ave Min Max): %60s\n", sstr);
    sprintf(sstr, "%14.4f %14.4f %14.4f", averageDAQRate, minimumDAQRate, maximumDAQRate);
    fprintf(wlogFile, " DAQ Rate (Hz) (Ave Min Max): %60s\n", sstr);
    sprintf(sstr, "%14.4f %14.0f %14.0f", averageEventSize, minimumEventSize, maximumEventSize);
    fprintf(wlogFile, " Event Size    (Ave Min Max): %60s\n", sstr);
    GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_DAQTIME, 0, sstr);
    fprintf(wlogFile, " DAQ Time                   : %60s\n", sstr);
    if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
      fprintf(wlogFile, " MDT Chamber Name           : %60s\n", MDTChamberName);
      fprintf(wlogFile, " MDT Chamber HV Status      : %60d\n", MDTChamberHVStatus);
      fprintf(wlogFile, " MDT Chamber Start MezzCard : %60d\n", startMezzCard);
    }
    
    if (nominalThresholdUsed) {
      fprintf(wlogFile, "//\n");
      if (nominalThreshold < -48) {
        fprintf(wlogFile, "// List channel/tube with ASD noise rate >= %.2fKHz\n", ASDNoiseCut);
      }
      else {
        fprintf(wlogFile, "// List channel/tube with ASD noise rate >= %.2fKHz\n", ASDNoiseWarningCut);
      }
    }
    else {
      fprintf(wlogFile, "// List channel/tube with ASD noise rate >= %.2fKHz if its effective threshold >  48mV\n", ASDNoiseCut);
      fprintf(wlogFile, "//                   or with ASD noise rate >= %.2fKHz if its effective threshold <= 48mV\n", ASDNoiseWarningCut);
    }
    if ((nominalThresholdUsed) && (nominalThreshold <= -40)) {
      if ((nNoise > 0) && (nNoise <= 12)) {
        fprintf(wlogFile, "// Mezzanine card(s) with high ASD noise may need to be replaced.\n");
      }
      else if (nNoise > 12) {
        fprintf(wlogFile, "// Too many channels/tubes with high ASD noise, there may be grouding problem.\n");
      }
    }
    fprintf(wlogFile, "Number of High ASD Noise Channels/Tubes : %6d\n", nNoise);
    if (nNoise > 0) {
      fprintf(wlogFile, "Mezz Ch ASDNoise(KHz) Row Tube\n");
      for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
        for (ch = 0; ch < 24; ch++) {
          if (noiseStatus[ch][mezz] == RDOUTCHASDNOISE) {
            MapChamberFromMezzCard(MDTChamberType, mezz, ch, &mappedCh, &row, &tube);
            fprintf(wlogFile, "%4d %2d %13.2f %3d %4d\n", mezz, ch, ASDNoiseRate[ch][mezz], row, tube);
          }
        }
      }
    }
    
    if (MDTChamberHVStatus == MDTCHAMBERHVON) {
      if (mappingMDTChamber) {
        fprintf(wlogFile, "// List dead channel/tube with efficiency <= %.3f\n", effiCut0);
      }
      else {
        fprintf(wlogFile, "// List dead channel/tube with efficiency <= %.3f\n", effiCut0);
        fprintf(wlogFile, "//                          or noise rate < %.3f of expected cosmic ray hit rate\n", fracCRRateCut0);
      }
      if (nDead > 0) {
        fprintf(wlogFile, "// Problem must be found out and resolved, possible reasons:\n");
        if (nDead > 12) fprintf(wlogFile, "//   0) Gas or HV problem (too many dead channels);\n");
        fprintf(wlogFile, "//   1) Dead tube (such as lose wire, blocked tubelet etc.);\n");
        fprintf(wlogFile, "//   2) Bad connection between mezzanine card and signal hedgehog card;\n");
        fprintf(wlogFile, "//   3) Bad signal hedgehog card;\n");
        fprintf(wlogFile, "//   4) Bad HV hedgehog card;\n");
        fprintf(wlogFile, "//   5) Bad mezzanine card;\n");
      }
      fprintf(wlogFile, "Number of Dead Channels/Tubes           : %6d\n", nDead);
      if (nDead > 0) {
        if (mappingMDTChamber) {
          fprintf(wlogFile, "Mezz Ch NumberHits Efficiency Row Tube\n");
        }
        else {
          fprintf(wlogFile, "Mezz Ch NumberHits Efficiency NoiseRate(KHz) Row Tube\n");
        }
        for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
          for (ch = 0; ch < 24; ch++) {
            if (tubeStatus[ch][mezz] == RDOUTCHDEAD) {
              MapChamberFromMezzCard(MDTChamberType, mezz, ch, &mappedCh, &row, &tube);
              fprintf(wlogFile, "%4d %2d %10u %10.3f ", mezz, ch, nGoodHit[ch][mezz], relativeEfficiency[tube-1][row-1]);
              if (!mappingMDTChamber) fprintf(wlogFile, "%14.2f ", noiseRate[ch][mezz]);
              fprintf(wlogFile, "%3d %4d\n", row, tube);
            }
          }
        }
      }

      if (mappingMDTChamber) {
        fprintf(wlogFile, "// List low efficiency channel/tube with efficiency between %.3f and %3f\n", effiCut0, effiCut1);
      }
      else {
        fprintf(wlogFile, "// List low efficiency channel/tube with noise rate between %.3f and %.3f of expected cosmic ray hit rate\n", fracCRRateCut0, fracCRRateCut1);
      }
      if (nLowEffi > 0) {
        fprintf(wlogFile, "// Low efficiency channels/tubes should be understood, possible reasons:\n");
        if (nLowEffi > 24) fprintf(wlogFile, "//   0) General problem (such as GAS, HV too low and grounding);\n");
        fprintf(wlogFile, "//   1) Tube (such as lose wire, partially blocked tubelet etc.);\n");
        fprintf(wlogFile, "//   2) Wrong measurements (such as due to high noise rate in some channels/tubes);\n");
        fprintf(wlogFile, "//   3) Bad cards (such as mezzanine, signal hedgehog and HV hedgehog card);\n");
      }
      fprintf(wlogFile, "Number of Low Efficiency Channels/Tubes : %6d\n", nLowEffi);
      if (nLowEffi > 0) {
        if (mappingMDTChamber) {
          fprintf(wlogFile, "Mezz Ch NumberHits Efficiency Row Tube\n");
        }
        else {
          fprintf(wlogFile, "Mezz Ch NumberHits Efficiency NoiseRate(KHz) Row Tube\n");
        }
        for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
          for (ch = 0; ch < 24; ch++) {
            if (tubeStatus[ch][mezz] == RDOUTCHLOWEFFICIENCY) {
              MapChamberFromMezzCard(MDTChamberType, mezz, ch, &mappedCh, &row, &tube);
              fprintf(wlogFile, "%4d %2d %10u %10.3f ", mezz, ch, nGoodHit[ch][mezz], relativeEfficiency[tube-1][row-1]);
              if (!mappingMDTChamber) fprintf(wlogFile, "%14.2f ", noiseRate[ch][mezz]);
              fprintf(wlogFile, "%3d %4d\n", row, tube);
            }
          }
        }
      }
    }

    if ((MDTChamberHVStatus == MDTCHAMBERHVON) && mappingMDTChamber) {
      fprintf(wlogFile, "// List hot channel/tube with efficiency between %.3f and %3f\n", effiCut2, effiCut3);
    }
    else {
      fprintf(wlogFile, "// List hot channel/tube with noise rate between %.1fKHz and %.1fKHz\n", noiseCut0, noiseCut1);
    }
    if (nHot > 0) {
      if (MDTChamberHVStatus != MDTCHAMBERHVON) {
        fprintf(wlogFile, "// Hot channels must be understood with chamber HV OFF, possible reasons:\n");
        if (nHot > 24) fprintf(wlogFile, "//   0) General problem (such as grounding);\n");
        fprintf(wlogFile, "//   1) Bad connections of mezzanine card, signal hedgehog and HV hedghog cards;\n");
        fprintf(wlogFile, "//   2) Bad cards (such as mezzanine, signal hedgehog and HV hedgehog card);\n");
      }
      else {
        fprintf(wlogFile, "// Hot channels/tubes should be understood, possible reasons:\n");
        if (nHot > 24) fprintf(wlogFile, "//   0) General problem (such as GAS, HV too high and grounding);\n");
        fprintf(wlogFile, "//   1) Tube (treat tube with negative HV may help);\n");
        fprintf(wlogFile, "//   2) Wrong measurements (such as caused by high noise rate in some channels/tubes);\n");
        fprintf(wlogFile, "//   3) Bad cards (such as mezzanine, signal hedgehog and HV hedgehog card);\n");
      }
    }
    fprintf(wlogFile, "Number of Hot Channels/Tubes            : %6d\n", nHot);
    if (nHot > 0) {
      if ((MDTChamberHVStatus == MDTCHAMBERHVON) && mappingMDTChamber) {
        fprintf(wlogFile, "Mezz Ch NumberHits Efficiency Row Tube\n");
      }
      else if (MDTChamberHVStatus == MDTCHAMBERHVON) {
        fprintf(wlogFile, "Mezz Ch NumberHits Efficiency NoiseRate(KHz) Row Tube\n");
      }
      else {
        fprintf(wlogFile, "Mezz Ch NumberHits NoiseRate(KHz) Row Tube\n");
      }
      for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
        for (ch = 0; ch < 24; ch++) {
          if (tubeStatus[ch][mezz] == RDOUTCHHOT) {
            MapChamberFromMezzCard(MDTChamberType, mezz, ch, &mappedCh, &row, &tube);
            fprintf(wlogFile, "%4d %2d %10u ", mezz, ch, nGoodHit[ch][mezz]);
            if (MDTChamberHVStatus == MDTCHAMBERHVON) fprintf(wlogFile, "%10.3f ", relativeEfficiency[tube-1][row-1]);
            if (!mappingMDTChamber) fprintf(wlogFile, "%14.2f ", noiseRate[ch][mezz]);
            fprintf(wlogFile, "%3d %4d\n", row, tube);
          }
        }
      }
    }

    if ((MDTChamberHVStatus == MDTCHAMBERHVON) && mappingMDTChamber) {
      fprintf(wlogFile, "// List very hot channel/tube with efficiency >= %.3f\n", effiCut3);
    }
    else {
      fprintf(wlogFile, "// List very hot channel/tube with noise rate >= %.1fKHz\n", noiseCut1);
    }
    if (nVeryHot > 0) {
      if (MDTChamberHVStatus != MDTCHAMBERHVON) {
        fprintf(wlogFile, "// Very hot channels must be understood with chamber HV OFF, possible reasons:\n");
        if (nVeryHot > 24) fprintf(wlogFile, "//   0) General problem (such as grounding);\n");
        fprintf(wlogFile, "//   1) Bad connections of mezzanine card, signal hedgehog and HV hedghog cards;\n");
        fprintf(wlogFile, "//   2) Bad cards (such as mezzanine, signal hedgehog and HV hedgehog card);\n");
      }
      else {
        fprintf(wlogFile, "// Very hot channels/tubes must be understood, possible reasons:\n");
        if (nVeryHot > 24) fprintf(wlogFile, "//   0) General problem (such as GAS, HV too high and grounding);\n");
        fprintf(wlogFile, "//   1) Tube (clean tubelet and treat tube with negative HV may help);\n");
        fprintf(wlogFile, "//   2) Wrong measurements (such as caused by high noise rate in some channels/tubes);\n");
        fprintf(wlogFile, "//   3) Bad cards (such as mezzanine, signal hedgehog and HV hedgehog card);\n");
      }
    }
    fprintf(wlogFile, "Number of Very Hot Channels/Tubes       : %6d\n", nVeryHot);
    if (nVeryHot > 0) {
      if ((MDTChamberHVStatus == MDTCHAMBERHVON) && mappingMDTChamber) {
        fprintf(wlogFile, "Mezz Ch NumberHits Efficiency Row Tube\n");
      }
      else if (MDTChamberHVStatus == MDTCHAMBERHVON) {
        fprintf(wlogFile, "Mezz Ch NumberHits Efficiency NoiseRate(KHz) Row Tube\n");
      }
      else {
        fprintf(wlogFile, "Mezz Ch NumberHits NoiseRate(KHz) Row Tube\n");
      }
      for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
        for (ch = 0; ch < 24; ch++) {
          if (tubeStatus[ch][mezz] == RDOUTCHVERYHOT) {
            MapChamberFromMezzCard(MDTChamberType, mezz, ch, &mappedCh, &row, &tube);
            fprintf(wlogFile, "%4d %2d %10u ", mezz, ch, nGoodHit[ch][mezz]);
            if (MDTChamberHVStatus == MDTCHAMBERHVON) fprintf(wlogFile, "%10.3f ", relativeEfficiency[tube-1][row-1]);
            if (!mappingMDTChamber) fprintf(wlogFile, "%14.2f ", noiseRate[ch][mezz]);
            fprintf(wlogFile, "%3d %4d\n", row, tube);
          }
        }
      }
    }
    
    fclose(wlogFile);
  }

  if (MDTChamberWarningFile != NULL) {
    if ((scanControl == 1) && (checkStatus != 0)) {
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
      strcpy(str, "");
      sprintf(str, "notepad %s", MDTChamberWarningFilename);
      status = LaunchExecutable(str);
      if (status != 0) status = LaunchExecutable(str);
    }
    else {
      if (checkStatus == 0) {
        fprintf(MDTChamberWarningFile, "All readout channels are normal in regarding its Efficiency and Various Noise Rate\n");
      }
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
    }
  }
  if (wlogFile != NULL) {
    if ((MDTChamberHVStatus == MDTCHAMBERHVON) && mappingMDTChamber) {
      sprintf(str, "MDT Chamber check file <%s> for efficiency and ASD noise is saved.\n", wlogFilename);
    }
    else {
      sprintf(str, "MDT Chamber check file <%s> for various noise is saved.\n", wlogFilename);
    }
    MDTChamberOutput2("%s", str);
    if ((nNoise >0) || (nDead > 0) || (nLowEffi > 0) || (nHot > 0) || (nVeryHot > 0)) {
      strcpy(str, "");
      sprintf(str, "notepad %s", wlogFilename);
      status = LaunchExecutable(str);
      if (status != 0) status = LaunchExecutable(str);
    }
  }
  if ((scanControl == 1) && (checkStatus != 0)) {
    if (MDTChamberHVStatus == MDTCHAMBERHVON) {
      sprintf(str, "Dead or abnormal readout channels have been detected from Efficiency and Noise Check Run\n"
                   "(MDT Chamber HV ON). Possible sources:\n"
                   "  1) Bad Mezzanine Card Card;\n"
                   "  2) Signal or HV Hedghog Card;\n"
                   "  3) Very noise chamber tube;\n");
    }
    else if (MDTChamberTest) {
      sprintf(str, "Dead or abnormal readout channels have been detected from Efficiency and Noise Check Run\n"
                   "(MDT Chamber HV OFF). Most likely the corresponding mezzanine card has problem.\n");
    }
    else {
      sprintf(str, "Dead or abnormal readout channels have been detected from Efficiency and Noise Check Run\n"
                   "Most likely the corresponding mezzanine card has problem.\n");
    }
    if (autoRunOn || (autoRunFromControlFile == 1)) {
      strcat(str, "\nAre you sure to continue with Abnormal Readout Channel from Efficiency and Noise Check Run?\n");
      resp = ConfirmPopup("Dead or Abnormal Readout Channel", str);
      if (resp == 0) {
        while (autoRunFromControlFile == 1) GetNewRunSetups(0);
        autoRunFromControlFile = 0;
        checkCSM = 0;
        checkCSMJTAG = 0;
        checkCSMIO = 0;
        MDTChamberControl = FALSE;
        autoRunOn = FALSE;
        useExternalTrigger = FALSE;
        thresholdScanMethod = 0;
        numberSubScans = 1;
        subScans = numberSubScans - 1;
        MDTChamberOutput2("Total Number of Download CSM        = %30d\n", numberCSMDownload);
        MDTChamberOutput2("Total Number of Failed Download CSM = %30d\n", numberBadCSMDownload);
        MDTChamberOutput2("Total Number of Download MezzCards  = %30d\n", numberMezzDownload);
        MDTChamberOutput2("Total Number of Failed Download Mezz= %30d\n", numberBadMezzDownload);
        MDTChamberOutput2("Total Number of Init DAQ            = %30d\n", numberInitDAQ);
        MDTChamberOutput2("Total Number of Failed Init DAQ     = %30d\n", numberBadInitDAQ);
        MDTChamberOutput2("Total Number of Redone Init DAQ     = %30d\n", numberReInitDAQ);
        MDTChamberOutput2("Total Number of Auto Runs           = %30d\n", numberAutoRun);
        MDTChamberOutput2("Total Number of Bad Auto Runs       = %30d\n", numberBadAutoRun);
        MDTChamberOutput2("Total Number of Rerun in Auto Runs  = %30d\n", numberAutoRerun);
        if (MDTChamberSumFile != NULL) {
          MDTChamberOutput1(">>>>>>>>>>  End MDT Chamber Tests <<<<<<<<<<\n\n");
          MDTChamberTest = FALSE;
          fclose(MDTChamberSumFile);
          MDTChamberSumFile = NULL;
        }
      }
      else {
        MDTChamberOutput1("Confirm to continue ..\n");
      }
    }
    else MessagePopup("Dead or Abnormal Readout Channel", str);
  }
  else if ((scanControl == 0) && (checkStatus != 0)) {
    if (autoRunOn) {
      sprintf(str, "*********!!! Abnormal Results in Efficiency and Noise Check Run !!!********\n"
                   "Continue auto run without warning to user\n");
    }
    else {
      sprintf(str, "*********!!! Abnormal Results in Efficiency and Noise Check Run !!!********\n"
                   "Please check corresponding summary file for details.\n");
    }
    MDTChamberOutput2("%s", str);
  }
}


// Routines for DAQ information panel
//
void DAQControlInforColor(void) {
  int colorCode, dimmed;

  dimmed = TRUE;
  if (runState == State_Idle) {
    colorCode = VAL_MAGENTA;
    dimmed = FALSE;
  }
  else if (runState == State_Running)
    colorCode = VAL_GREEN;
  else
    colorCode = VAL_YELLOW;
  
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_DATAOUT, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_FILENAME, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_RUNTYPE, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_RUNNUMBER, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTTIME, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STOPTIME, ATTR_TEXT_BGCOLOR, colorCode);
  NumberConnectedTCPNodes();
  if (!DAQTCPConnected) colorCode = VAL_MAGENTA;
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_NUMBEREVENT, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_DAQRATE, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_DAQTIME, ATTR_TEXT_BGCOLOR, colorCode);
  if (displayDAQInfor != 1) colorCode = VAL_MAGENTA;
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_RUNNUMBER, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTID, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_NUMBEREVENT, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQTIME, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_CURR, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_AVER, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_MIN, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_MAX, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_CURR, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_AVER, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_MIN, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_MAX, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_CURR, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_AVER, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MIN, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MAX, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_CURR, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_AVER, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_MIN, ATTR_TEXT_BGCOLOR, colorCode);
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_MAX, ATTR_TEXT_BGCOLOR, colorCode);
}


// Routines for DAQ Information
//
void CloseDAQInfor(void) {
  if (runState != State_Idle) {
    displayDAQInfor = FALSE;
    SetCtrlVal(DAQControlHandle, P_JTAGCTRL_DAQINFOR, displayDAQInfor);
  }
  HidePanel(DAQInforHandle);
}


void DAQInforControl(void) {
  int partDisplay, hour, minute, second;
  unsigned int nevent, totalSize, step, i, j, index;
  double myDAQRate, myEventSize, myDataRate, size;
  double x1, y1, x2, y2;
  static unsigned int partDisplayMaxTime = 900, allDisplayMaxTime = 600;
  char string[9];
  
  GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DAQINFOR, &displayDAQInfor);
  if (controlOption == HELP) {
    MessagePopup("Help on DAQInfor button",
                 "If enabled, basic DAQ information will be displayed.");
    controlOption = NORM;
    return;
  }
  if (runState == State_Idle) {
    ResetTextBox(DAQInforHandle, P_DAQINFOR_DATETIME, WeekdayTimeString(time(NULL)));
    if (displayDAQInfor == 1) {
      DisplayPanel(DAQInforHandle);
      SetActivePanel(DAQInforHandle);
    }
    return;
  }
  if (DAQTime < 20 && (totalNumberEvent < 200 || DAQTime < 2)) return;
  ResetTextBox(DAQInforHandle, P_DAQINFOR_DATETIME, WeekdayTimeString(time(NULL)));
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DISPLAYPLOTS, ATTR_DIMMED, TRUE);
  if (displayDAQInfor == 1) {
    if (DAQTime < 900) {
      partDisplayMaxTime = 900;
      allDisplayMaxTime = 600;
    }
    GetCtrlVal(DAQInforHandle, P_DAQINFOR_DISPLAYPLOTS, &partDisplay);
    if (partDisplay == 1) {
      if (DAQTime >= partDisplayMaxTime) {
        partDisplayMaxTime = DAQTime - (DAQTime % 300);
        if (((int) (DAQTime - partDisplayMaxTime)) > 200) partDisplayMaxTime += 300;
      }
      plotMaxTime = partDisplayMaxTime;
      plotMaxTime += 300;
      plotMinTime = plotMaxTime - 1200;
      if (maximumDAQRate > 0) {
        plotMaxDAQRate = 0.0;
        plotMaxEventSize = 0.0;
        plotMaxDataRate = 0.0;
      }
      step = (int) measuringTime;
      for (i = plotMinTime; i < (DAQTime - step); i += step) {
        nevent = 0;
        totalSize = 0;
        for (j = i; j < i+step; j++) {
          index = j % RECENTINFORLENGTH;
          nevent += recentNumberEvent[index];
          totalSize += recentEventSize[index];
          if (j == recentInforIndex) break;
        }
        if (j == recentInforIndex) break;
        myDAQRate = ((double) nevent) / measuringTime;
        myEventSize = 0;0;
        if (nevent > 0) myEventSize = ((double) totalSize) / ((double) nevent);
        myDataRate = myDAQRate * myEventSize / 1000.0;
        if (1.2*myDAQRate > plotMaxDAQRate) plotMaxDAQRate = 1.2 * myDAQRate;
        if (2.0*myEventSize > plotMaxEventSize) plotMaxEventSize = 2.0 * myEventSize;
        if (1.2*myDataRate > plotMaxDataRate) plotMaxDataRate = 1.2 * myDataRate;
      }
      if (1.2*currentDAQRate > plotMaxDAQRate) plotMaxDAQRate = 1.2 * currentDAQRate;
      if (2.0*currentEventSize > plotMaxEventSize) plotMaxEventSize = 2.0 * currentEventSize;
      if (1.2*currentDataRate > plotMaxDataRate) plotMaxDataRate = 1.2 * currentDataRate;
    }
    else {
      while (DAQTime >= allDisplayMaxTime) {
        i = DAQTime / 1800;
        if (i > 11) i = 11;
        allDisplayMaxTime += 300 * (i + 1);
      }
      plotMaxTime = allDisplayMaxTime;
      plotMinTime = 0.0;
      if (maximumDAQRate > 0) {
        plotMaxDAQRate = 1.2 * maximumDAQRate;
        plotMaxEventSize = 2.0 * maximumEventSize;
        plotMaxDataRate = 1.2 * maximumDataRate;
      }
    }
    if (plotMaxDAQRate > 100)
      plotMaxDAQRate = ((int) plotMaxDAQRate) + 10 - ((int) plotMaxDAQRate) % 10;
    if (plotMaxEventSize > 100)
      plotMaxEventSize = ((int) plotMaxEventSize) + 10 - ((int) plotMaxEventSize) % 10;
    if (plotMaxDataRate > 100)
      plotMaxDataRate = ((int) plotMaxDataRate) + 10 - ((int) plotMaxDataRate) % 10;
    plotFactorForEventSize = plotMaxDAQRate / plotMaxEventSize;
    DeleteGraphPlot(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, -1, VAL_IMMEDIATE_DRAW);
    SetAxisScalingMode(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, VAL_XAXIS, 
                       VAL_MANUAL, (double) plotMinTime, (double) plotMaxTime);
    SetAxisScalingMode(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, VAL_LEFT_YAXIS, 
                       VAL_MANUAL, 0.0, plotMaxDAQRate);
    SetAxisScalingMode(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, VAL_RIGHT_YAXIS, 
                       VAL_MANUAL, 0.0, plotMaxEventSize);
    DeleteGraphPlot(DAQInforHandle, P_DAQINFOR_GRAPH_DATARATE, -1, VAL_IMMEDIATE_DRAW);
    SetAxisScalingMode(DAQInforHandle, P_DAQINFOR_GRAPH_DATARATE, VAL_XAXIS, 
                       VAL_MANUAL, (double) plotMinTime, (double) plotMaxTime);
    SetAxisScalingMode(DAQInforHandle, P_DAQINFOR_GRAPH_DATARATE, VAL_LEFT_YAXIS, 
                       VAL_MANUAL, 0.0, plotMaxDataRate);
    hour = plotMinTime / 3600;
    minute = (plotMinTime / 60) % 60;
    sprintf(string, "%dh%02d", hour, minute);
    ReplaceAxisItem(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, VAL_XAXIS,
                    0, string, (double) plotMinTime);
    ReplaceAxisItem(DAQInforHandle, P_DAQINFOR_GRAPH_DATARATE, VAL_XAXIS,
                    0, string, (double) plotMinTime);
    hour = (plotMinTime + plotMaxTime) / 7200;
    minute = ((plotMinTime + plotMaxTime) / 120) % 60;
    second = ((plotMinTime + plotMaxTime) / 2) % 60;
    sprintf(string, "%dh%02dm%02d", hour, minute, second);
    ReplaceAxisItem(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, VAL_XAXIS,
                    1, string, (double) (plotMinTime+plotMaxTime)/2.0);
    ReplaceAxisItem(DAQInforHandle, P_DAQINFOR_GRAPH_DATARATE, VAL_XAXIS,
                    1, string, (double) (plotMinTime+plotMaxTime)/2.0);
    hour = plotMaxTime / 3600;
    minute = (plotMaxTime / 60) % 60;
    sprintf(string, "%dh%02d", hour, minute);
    ReplaceAxisItem(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, VAL_XAXIS,
                    2, string, (double) plotMaxTime);
    ReplaceAxisItem(DAQInforHandle, P_DAQINFOR_GRAPH_DATARATE, VAL_XAXIS,
                    2, string, (double) plotMaxTime);
    x1 = (double) plotMinTime + ((double) (plotMaxTime - plotMinTime)) / 20.0;
    y1 = 0.95*plotMaxDAQRate;
    x2 = x1 + ((double) (plotMaxTime - plotMinTime)) / 10.0; 
    y2 = y1;
    i = PlotLine(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, x1, y1, x2, y2, VAL_RED);
    SetPlotAttribute(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, i, ATTR_PLOT_THICKNESS, 3);
    x2 += ((double) (plotMaxTime - plotMinTime)) / 30.0;
    y2 *= 0.95;
    i = PlotText(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, x2, y2, "DAQRate", VAL_MENU_META_FONT,
                 VAL_RED, VAL_WHITE);
    x1 = 0.5 * (double) (plotMinTime + plotMaxTime) + ((double) (plotMaxTime - plotMinTime)) / 20.0;
    y1 = 0.95*plotMaxDAQRate;
    x2 = x1 + ((double) (plotMaxTime - plotMinTime)) / 10.0; 
    y2 = y1;
    i = PlotLine(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, x1, y1, x2, y2, VAL_BLACK);
    SetPlotAttribute(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, i, ATTR_PLOT_THICKNESS, 3);
    x2 += ((double) (plotMaxTime - plotMinTime)) / 30.0; 
    y2 *= 0.95;
    i = PlotText(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, x2, y2, "EventSize", VAL_MENU_META_FONT,
                 VAL_BLACK, VAL_WHITE);
    DisplayPanel(DAQInforHandle);
    if (partDisplay == 1) step = (int) 2*measuringTime;
    else step = 4*inforTimeStep;
    if (DAQTime > 2*step) {
      for (i = plotMinTime; i < (DAQTime - 2*step); i += step) {
        if (partDisplay == 1) {
          nevent = 0;
          totalSize = 0;
          for (j = i; j < i+step; j++) {
            index = j % RECENTINFORLENGTH;
            nevent += recentNumberEvent[index];
            totalSize += recentEventSize[index];
            if (j == recentInforIndex) break;
          }
          if (j == recentInforIndex) break;
          size = (double) totalSize;
        }
        else {
          index = i / step;
          nevent = numberEvent[index] + numberEvent[index+1] + 
                   numberEvent[index+2] + numberEvent[index+3];
          size = ((double) eventSizeHigh[index]) * maxUintValue + eventSizeLow[index] +
                 ((double) eventSizeHigh[index+1]) * maxUintValue + eventSizeLow[index+1] +
                 ((double) eventSizeHigh[index+2]) * maxUintValue + eventSizeLow[index+2] +
                 ((double) eventSizeHigh[index+3]) * maxUintValue + eventSizeLow[index+3];
        }
        myDAQRate = ((double) nevent) / ((double) step);
        myEventSize = 0.0;
        if (nevent > 0) myEventSize = size / ((double) nevent);
        myDataRate = myDAQRate * myEventSize / 1000.0 * 4.0;
        if (myDAQRate >= minimumDAQRate) {
          PlotPoint(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, ((double) i) + ((double) step / 2.0), 
                    myDAQRate, 1, VAL_RED);
          PlotPoint(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, ((double) i) + ((double) step / 2.0), 
                    myEventSize*plotFactorForEventSize, 1, VAL_BLACK);
          PlotPoint(DAQInforHandle, P_DAQINFOR_GRAPH_DATARATE, ((double) i) + ((double) step / 2.0), 
                    myDataRate, 1, VAL_RED);
        }
      }
    }
  }
  SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DISPLAYPLOTS, ATTR_DIMMED, FALSE);
}


static unsigned int eventSizeSum = 0;
static double startTime = 0.0, nevent, dtime, average, currMaxEventSize;
void CollectAndDisplayDAQInfor(void)
{
  unsigned int i, sum, WCOnly, hour, minute, second, index;
  static int lastEventID, tdc, cha, suppressAll;
  static double dsum, nevtSum, maxUintValue = 4294967296.0;
  static time_t savedTimeForDisplay;
  static char string[20], sprecision[10];
  
  if (initDAQInfor == 1) {  // Initialization
    initDAQInfor = 0;
    for (i = 0; i < NUMBERANALYSISCOUNTER; i++) analysisCounter[i] = 0;
    analysisCounter[MINEVENTSIZE] = 1000000;
    ResetTextBox(DAQControlHandle, P_JTAGCTRL_NUMBERERROR, "0");
    if (enableAnalysis != 0)
      SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_NUMBERERROR, ATTR_TEXT_BGCOLOR, VAL_GREEN);
    inforTimeStep = 1;
    for (i = 0; i < INFORLENGTH; i++) {
      numberEvent[i] = 0;
      eventSizeLow[i] = 0;
      eventSizeHigh[i] = 0;
      numberTrigger[i] = 0;
    }
    for (i = 0; i < RECENTINFORLENGTH; i++) {
      recentNumberEvent[i] = 0;
      recentEventSize[i] = 0;
      recentNumberTrigger[i] = 0;
    }
    currentDAQRate = 0.0;
    averageDAQRate = 0.0;
    minimumDAQRate = 10000000.;
    maximumDAQRate = 0.0;
    currentTriggerRate = 0.0;
    averageTriggerRate = 0.0;
    minimumTriggerRate = 10000000.;
    maximumTriggerRate = 0.0;
    currentEventSize = 0.0;
    averageEventSize = 0.0;
    minimumEventSize = 10000000.;
    maximumEventSize = 0.0;
    currMaxEventSize = 0.0;
    currentDataRate = 0.0;
    averageDataRate = 0.0;
    minimumDataRate = 10000000.;
    maximumDataRate = 0.0;
    plotMaxDAQRate = -1.0;
    plotMaxEventSize = -1.0;
    plotMaxDataRate = -1.0;

    totalNumberEvent = 0;
    totalNumberEventHigh = 0;
    totalNumberTrigger = 0;
    totalNumberTriggerHigh = 0;
    eventSizeSum = 0;
    lastEventID = -1;
    nevent = 0.0;
    ntrigger = 0;
    startTime = Timer();
    time(&DAQStartTime);
    DAQTime = 0;
    time(&savedTimeForDisplay);
    inforIndex = DAQTime / inforTimeStep;
    recentInforIndex = DAQTime % RECENTINFORLENGTH;
    DAQInforPrecision = ReadParameters(PARAINFORPRECISION);
    if (DAQInforPrecision < 2) DAQInforPrecision = 2;
    sprintf(sprecision, " %%.%df", DAQInforPrecision);
    for (tdc = 0; tdc < MAXNUMBERMEZZANINE; tdc++) {
      for (cha = 0; cha < 24; cha++) {
        nPair[cha][tdc] = 0;
        nEdge[0][cha][tdc] = 0;
        nEdge[1][cha][tdc] = 0;
      }
    }
  }
  else {                        // Now let's collect DAQ Information
    DAQTime = (unsigned int) (time(NULL) - DAQStartTime);
    dtime = Timer() - startTime;
    if (dtime >= measuringTime) {
      currentDAQRate = nevent / dtime;
      if (currentDAQRate < 1.0) measuringTime = 10.0;
      else if (currentDAQRate < 10.0) measuringTime = 5.0;
      else measuringTime = 2.0;
      if (currentDAQRate > maximumDAQRate) maximumDAQRate = currentDAQRate;
      if (currentDAQRate < minimumDAQRate) minimumDAQRate = currentDAQRate;
      nevtSum = maxUintValue * ((double) totalNumberEventHigh) + (double) totalNumberEvent;
      averageDAQRate = nevtSum / ((double) DAQTime);
      // printf("nevtSum %.f DAQTime %d aDaqRate %.f\n", nevtSum, DAQTime, averageDAQRate);
      average = 0.0;
      if (nevent > 0) average = (double) eventSizeSum / nevent;
      currentDataRate = (4.0 * average * currentDAQRate / 1000.);
      if (currentDataRate > maximumDataRate) maximumDataRate = currentDataRate;
      if (currentDataRate < minimumDataRate) minimumDataRate = currentDataRate;
      if (displayDAQInfor == 1) {
        if (currMaxEventSize >= 0.9*plotMaxEventSize) DAQInforControl();
        if ((DAQTime >= plotMaxTime) || (currentDAQRate >= 0.9*plotMaxDAQRate) ||
            (currentDataRate >= 0.9*plotMaxDataRate)) DAQInforControl();
        PlotPoint(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, (double) DAQTime, 
                  currentDAQRate, 1, VAL_RED);
        PlotPoint(DAQInforHandle, P_DAQINFOR_GRAPH_DAQRATE, (double) DAQTime, 
                  currentEventSize*plotFactorForEventSize, 1, VAL_BLACK);
        PlotPoint(DAQInforHandle, P_DAQINFOR_GRAPH_DATARATE, (double) DAQTime, 
                  currentDataRate, 1, VAL_RED);
      }
      nevtSum = maxUintValue * ((double) totalNumberTriggerHigh) + (double) totalNumberTrigger;
      if (DAQTime > 100) averageTriggerRate = nevtSum / ((double) DAQTime);
      else if (totalTriggerTime > 0.0) averageTriggerRate = nevtSum / totalTriggerTime;
      nevent = 0.0;
      eventSizeSum = 0.0;
      currMaxEventSize = 0.0;
      startTime = Timer();
    }

    // Display DAQ information
    if (enableAnalysis == 1) {
      if (analysisCounter[WRONGEVENT] > 0) {
        sprintf(string, "%u", analysisCounter[WRONGEVENT]);
        ResetTextBox(DAQControlHandle, P_JTAGCTRL_NUMBERERROR, string);
        SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_NUMBERERROR, ATTR_TEXT_BGCOLOR, VAL_RED);
      }
    }
    dtime = (double) (time(NULL) - savedTimeForDisplay);
    if (dtime >= 1.0) {
      suppressAll = suppressCSMHeader && suppressCSMTrailer && suppressTDCHeader && suppressTDCTrailer;
      sum = 0;
      dsum = 0.0;
      for (i = 0; i <= inforIndex; i++) {
        sum += eventSizeLow[i];
        if (sum < eventSizeLow[i]) dsum += maxUintValue;
        dsum += (eventSizeHigh[i] * maxUintValue);
      }
      dsum += (double) sum;
      nevtSum = maxUintValue * ((double) totalNumberEventHigh) + (double) totalNumberEvent;
      if (nevtSum > 0.0) averageEventSize = dsum / nevtSum;
      averageDataRate = (4.0 * averageEventSize * averageDAQRate / 1000.);
      ResetTextBox(DAQInforHandle, P_DAQINFOR_DATETIME, WeekdayTimeString(time(NULL)));
      time(&savedTimeForDisplay);
      sprintf(string, "%.2fHz", currentDAQRate);
      ResetTextBox(DAQControlHandle, P_JTAGCTRL_DAQRATE, string);
      if ((currentDAQRate <= 0.0) && (DAQTime > measuringTime)) {
        SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_DAQRATE, ATTR_TEXT_BGCOLOR, VAL_RED);
      }  
      else if (runState == State_Running) {
        SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_DAQRATE, ATTR_TEXT_BGCOLOR, VAL_GREEN);
      }
      second = DAQTime % 60;
      minute = (DAQTime / 60) % 60;
      hour = DAQTime / 3600;
      sprintf(string, "%dH%2dM%2dS", hour, minute, second);
      ResetTextBox(DAQControlHandle, P_JTAGCTRL_DAQTIME, string);
      if (displayDAQInfor == 1) {
        ResetTextBox(DAQInforHandle, P_DAQINFOR_DAQTIME, string);
        sprintf(string, "%d", runNumber);
        ResetTextBox(DAQInforHandle, P_DAQINFOR_RUNNUMBER, string);
        sprintf(string, "%X", eventID);
        ResetTextBox(DAQInforHandle, P_DAQINFOR_EVENTID, string);
        if (totalNumberEventHigh == 0)
          sprintf(string, "%u", totalNumberEvent);
        else {
          nevtSum = maxUintValue * ((double) totalNumberEventHigh) + (double) totalNumberEvent;
          sprintf(string, "%.4e", nevtSum);
        }
        ResetTextBox(DAQInforHandle, P_DAQINFOR_NUMBEREVENT, string);
        ResetTextBox(DAQControlHandle, P_JTAGCTRL_NUMBEREVENT, string);
        if (DAQTime >= measuringTime) {
          sprintf(string, "%.2f", currentDAQRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_DAQRATE_CURR, string);
          if (currentDAQRate <= 0.0)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_CURR, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_CURR, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, "%.2f", averageDAQRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_DAQRATE_AVER, string);
          if (averageDAQRate <= 0.0)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_AVER, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_AVER, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, "%.2f", minimumDAQRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_DAQRATE_MIN, string);
          if (minimumDAQRate <= 0.0)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_MIN, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_MIN, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, "%.2f", maximumDAQRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_DAQRATE_MAX, string);
          if (maximumDAQRate <= 0.0)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_MAX, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DAQRATE_MAX, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, "%.2f", currentTriggerRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_TRIGRATE_CURR, string);
          if (currentTriggerRate <= 0.0)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_CURR, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_CURR, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, "%.2f", averageTriggerRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_TRIGRATE_AVER, string);
          if (averageTriggerRate <= 0.0)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_AVER, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_AVER, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, "%.2f", minimumTriggerRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_TRIGRATE_MIN, string);
          if (minimumTriggerRate <= 0.0)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_MIN, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_MIN, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, "%.2f", maximumTriggerRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_TRIGRATE_MAX, string);
          if (maximumTriggerRate <= 0.0)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_MAX, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_TRIGRATE_MAX, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, "%.0f", currentEventSize);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_EVENTSIZE_CURR, string);
          if ((currentEventSize <= 0.0) && (!suppressAll))
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_CURR, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_CURR, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, sprecision, averageEventSize);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_EVENTSIZE_AVER, string);
          if ((averageEventSize <= 0.0) && (!suppressAll))
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_AVER, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_AVER, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          if (averageEventSize > 0) 
            sprintf(string, "%.0f", minimumEventSize);
          else
            sprintf(string, "%.0f", 0.0);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MIN, string);
          if (((minimumEventSize <= 0.0) && (!suppressAll)) || (minimumEventSize > maximumEventSize))
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MIN, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MIN, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, "%.0f", maximumEventSize);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MAX, string);
          if ((maximumEventSize <= 0.0) && (!suppressAll))
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MAX, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MAX, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, sprecision, currentDataRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_DATARATE_CURR, string);
          if ((currentDataRate <= 0.0) && (!suppressAll))
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_CURR, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_CURR, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, sprecision, averageDataRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_DATARATE_AVER, string);
          if ((averageDataRate <= 0.0) && (!suppressAll))
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_AVER, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_AVER, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, sprecision, minimumDataRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_DATARATE_MIN, string);
          if ((minimumDataRate <= 0.0) && (!suppressAll))
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_MIN, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_MIN, ATTR_TEXT_BGCOLOR, VAL_GREEN);
          sprintf(string, sprecision, maximumDataRate);
          ResetTextBox(DAQInforHandle, P_DAQINFOR_DATARATE_MAX, string);
          if ((maximumDataRate <= 0.0) && (!suppressAll))
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_MAX, ATTR_TEXT_BGCOLOR, VAL_RED);
          else if (runState == State_Running)
            SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_MAX, ATTR_TEXT_BGCOLOR, VAL_GREEN);
        }
      }
    }
  }
}


void CollectDAQInfor(int counter[]) {
  static float collectTime, startCollect, time0;
  static unsigned int i, bigOR, index, sum, nbEvents, nWords;
  
  if (initDAQInfor) {
    collectTime = 0.0;
    time0 = Timer();
  }
  bigOR = 0;
  for (i = 0; i < NUMBERANALYSISCOUNTER; i++) {
    bigOR |= counter[4+i];
    if (i == ANALYSEDEVENT) {
      sum = analysisCounter[i] + counter[4+i];
      if (sum < analysisCounter[i]) analysisCounter[ANALYSEDEVENTHIGH]++;
      analysisCounter[ANALYSEDEVENT] = sum;
      totalEvent += counter[4+i];
    }
    else if (i == NTOTALWORD) {
      sum = analysisCounter[i] + counter[4+i];
      if (sum < analysisCounter[i]) analysisCounter[NTOTALWORDHIGH]++;
      analysisCounter[NTOTALWORD] = sum;
    }
    else if (i == PROCESSEDEVENT) {
      sum = analysisCounter[i] + counter[4+i];
      if (sum < analysisCounter[i]) analysisCounter[PROCESSEDEVENTHIGH]++;
      analysisCounter[PROCESSEDEVENT] = sum;
    }
    else if (i == NDATAWORD) {
      sum = analysisCounter[i] + counter[4+i];
      if (sum < analysisCounter[i]) analysisCounter[NDATAWORDHIGH]++;
      analysisCounter[NDATAWORD] = sum;
    }
    else if (i == NEVENTWORD) {
      sum = analysisCounter[i] + counter[4+i];
      if (sum < analysisCounter[i]) analysisCounter[NEVENTWORDHIGH]++;
      analysisCounter[NEVENTWORD] = sum;
    }
    else if (i == NGOODCYCLE) {
      sum = analysisCounter[i] + counter[4+i];
      if (sum < analysisCounter[i]) analysisCounter[NGOODCYCLEHIGH]++;
      analysisCounter[NGOODCYCLE] = sum;
    }
    else if (i == NEMPTYCYCLE) {
      sum = analysisCounter[i] + counter[4+i];
      if (sum < analysisCounter[i]) analysisCounter[NEMPTYCYCLEHIGH]++;
      analysisCounter[NEMPTYCYCLE] = sum;
    }
    else if ((i == CURREVENTSIZE) || (i == CURREVENTID)) {
      analysisCounter[i] = counter[4+i];
    }
    else if (i == MINEVENTSIZE) {
      if ((counter[4+ANALYSEDEVENT] > 0) && (analysisCounter[i] > counter[4+i]))
        analysisCounter[i] = counter[4+i];
    }
    else if (i == MAXEVENTSIZE) {
      if (analysisCounter[i] < counter[4+i]) analysisCounter[i] = counter[4+i];
    }
    else if ((i == LHCCLOCKUNLOCKED) || (i == XMT1CLOCKUNLOCKED) || (i == XMT2CLOCKUNLOCKED) ||
             (i == CSMPHASEERROR) || (i == I2COPERATIONFAILED) || (i == UNEXPECTEDTTCRXSETUP) ||
             (i == CSMHASERROR) || (i == UNKNOWNSWORD)) {
      sum = analysisCounter[i] + counter[4+i];
      if (sum < analysisCounter[i]) analysisCounter[i] = 0xFFFFFFFF;
      else analysisCounter[i] += counter[4+i];
    }
    else {
      sum = analysisCounter[i] + counter[4+i];
      if (sum < analysisCounter[i]) analysisCounter[i] = 0xFFFFFFFF;
      else analysisCounter[i] += counter[4+i];
    }
  }
  if (bigOR == 0) {
    collectTime = Timer() - time0;
    if ((currentTriggerRate <= 0.0) && (collectTime < 60.0)) return;
    else if ((currentTriggerRate <= 5.0) && (collectTime < 20.0)) return;
    else if ((currentTriggerRate <= 10.0) && (collectTime < 5.0)) return;
    else if (collectTime < 3.0) return;
  }
  time0 = Timer();
  nbEvents = counter[4+PROCESSEDEVENT];
  nWords = counter[4+NEVENTWORD];
  nevent += (float) nbEvents;
  if ((totalNumberEvent+nbEvents) < totalNumberEvent) totalNumberEventHigh++;
  totalNumberEvent += nbEvents;
  eventID = analysisCounter[CURREVENTID];
  if (counter[4+ANALYSEDEVENT] > 0) currentEventSize = analysisCounter[CURREVENTSIZE];
  minimumEventSize = analysisCounter[MINEVENTSIZE];
  maximumEventSize = analysisCounter[MAXEVENTSIZE];
  if (currMaxEventSize < currentEventSize)  currMaxEventSize = currentEventSize;
  eventSizeSum += (unsigned int) nWords;
  inforIndex = DAQTime / inforTimeStep;
  if (inforIndex >= INFORLENGTH) {
    for (i = 0; i < INFORLENGTH/2; i++) {
      numberEvent[i] = numberEvent[2*i] + numberEvent[2*i+1];
      eventSizeHigh[i] = eventSizeHigh[2*i] + eventSizeHigh[2*i+1];
      eventSizeLow[i] = eventSizeLow[2*i] + eventSizeLow[2*i+1];
      if (eventSizeLow[i] < eventSizeLow[2*i]) eventSizeHigh[i]++;
    }
    for (i = INFORLENGTH/2; i < INFORLENGTH; i++) {
      numberEvent[i] = 0;
      eventSizeHigh[i] = 0;
      eventSizeLow[i] = 0;
    }
    inforTimeStep *= 2;
    inforIndex = DAQTime / inforTimeStep;
  }
  numberEvent[inforIndex] += nbEvents;
  sum = eventSizeLow[inforIndex] + nWords;
  if (sum < eventSizeLow[inforIndex]) eventSizeHigh[inforIndex]++;
  eventSizeLow[inforIndex] = sum;
  index = DAQTime % RECENTINFORLENGTH;
  if (index != recentInforIndex) {
    recentNumberEvent[index] = 1;
    recentEventSize[index] = currentEventSize;
    recentInforIndex = index;
  }
  else {
    recentNumberEvent[index] += nbEvents;
    recentEventSize[index] += nWords;
  }
  CollectAndDisplayDAQInfor();
  if (showErrorReport) UpErrorReport();
}


void CopyDAQInfor(int counter[]) {
  unsigned int i;
  
  for (i = 0; i < NUMBERANALYSISCOUNTER; i++) {
    analysisCounter[i] = counter[4+i];
  }
  totalNumberEvent = analysisCounter[ANALYSEDEVENT];
  totalNumberEventHigh = analysisCounter[ANALYSEDEVENTHIGH];
  if (showErrorReport) UpErrorReport();
}


// Routines to check mezzanine card on/off status
//
void UpCheckMezzCardOnOff(void)
{
  if (controlOption == NORM) {
    HidePanel(DAQControlHandle);
    DisplayPanel(CheckMezzOnOffHandle);
    SetActivePanel(CheckMezzOnOffHandle);
    CheckMezzCardOnOff();
    HidePanel(DAQControlHandle);
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Check Mezzanine Card On/Off Status", 
                 "Bring up Check Mezzanine Card On/Off Status Panel\n"
                 "and test whether the mezzanine card will receive data.");
  controlOption = NORM;
}


void CheckMezzCardOnOff(void) {
  int savedMezzEnables, mezz, color, button[MAXNUMBERMEZZANINE], trig, rate, analysis;
  int storage, rawData, time0, dtime, nevt, savedNumberCSMDownload, savedNumberBadCSMDownload;

  NumberConnectedTCPNodes();
  if (!DAQTCPConnected) {
    MessagePopup("No DAQ TCP Node Connected", 
                 "No DAQ TCP Node from Linux Box is connected which controls GOLA card to collect data\n"
                 "Please start the DAQ program at Linux Box to check mezzanine card on/off status.");
    return;
  }
  button[0] = P_MZONOFF_MEZZ00;
  button[1] = P_MZONOFF_MEZZ01;
  button[2] = P_MZONOFF_MEZZ02;
  button[3] = P_MZONOFF_MEZZ03;
  button[4] = P_MZONOFF_MEZZ04;
  button[5] = P_MZONOFF_MEZZ05;
  button[6] = P_MZONOFF_MEZZ06;
  button[7] = P_MZONOFF_MEZZ07;
  button[8] = P_MZONOFF_MEZZ08;
  button[9] = P_MZONOFF_MEZZ09;
  button[10] = P_MZONOFF_MEZZ10;
  button[11] = P_MZONOFF_MEZZ11;
  button[12] = P_MZONOFF_MEZZ12;
  button[13] = P_MZONOFF_MEZZ13;
  button[14] = P_MZONOFF_MEZZ14;
  button[15] = P_MZONOFF_MEZZ15;
  button[16] = P_MZONOFF_MEZZ16;
  button[17] = P_MZONOFF_MEZZ17;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    SetCtrlAttribute(CheckMezzOnOffHandle, button[mezz], ATTR_TEXT_BGCOLOR, VAL_WHITE);
  }
  SetCtrlVal(DAQControlHandle, P_JTAGCTRL_RUNTYPE, NORMALALLCALIBCHANNELOFF);
  SetDAQRunType();
  TTCviSaveTriggerSettings();
  if (gotTTCvi) {
    trig = GetTTCviTriggerSelction();
    rate = (int) ExpectedTriggerRate();
    triggerRate = 5000;
    SetTTCviTriggerAndRate(-1, &triggerRate);
    TTCviSetUnusedTrigger();
  }
  GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, &storage);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_SAVERAWDATA, &rawData);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_ANALYSIS, &analysis);
  SetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, 0);
  SetCtrlVal(RunConditionHandle, P_RUNCOND_SAVERAWDATA, FALSE);
  SetCtrlVal(RunConditionHandle, P_RUNCOND_ANALYSIS, 0);
  AutoRunSetupAllOff();
  GetAutoRunControls();
  checkSystemNoise = FALSE;
  mappingMDTChamber = FALSE;
  SetCtrlVal(RunConditionHandle, P_RUNCOND_CHECKSYSTEMNOISE, checkSystemNoise);
  SetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, mappingMDTChamber);
  mezzOnOffStatus = 0;
  savedMezzEnables = mezzEnables;
  mezzCardSetupAll = TRUE;
  checkMezzOnOff = TRUE;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    savedNumberCSMDownload = numberCSMDownload;
    savedNumberBadCSMDownload = numberBadCSMDownload;
    SetCtrlVal(CheckMezzOnOffHandle, P_MZONOFF_CHECKINGMEZZ, mezz);
    SetCtrlAttribute(CheckMezzOnOffHandle, P_MZONOFF_CHECKINGMEZZ, ATTR_TEXT_BGCOLOR, VAL_GREEN);
    JTAGControlInitialization();
    mezzEnables = (1 << mezz);
    SaveParameters(PARAMEZZENABLES);
    SetMezzCardEnables();
    SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
    CSMSetupDone();
    if (CSMSetupStatus() == 0) {
      InitDAQ();
      autoRunOn = TRUE;
      StartDAQ();
      time0 = (int) time(NULL);
      dtime = 0;
      nevt = 0;
      while (dtime < 10) {
        HandleUserButtonAndFunction(PanelDone);
        dtime = ((int) time(NULL)) - time0;
        nevt = analysisCounter[PROCESSEDEVENT];
        if (nevt > 1000) break;
        if (!checkMezzOnOff) break;
      }
      StopDAQ();
      autoRunOn = FALSE;
      if ((nevt <= 0) && checkMezzOnOff) {
        // Try again
        InitDAQ();
        autoRunOn = TRUE;
        StartDAQ();
        time0 = (int) time(NULL);
        dtime = 0;
        nevt = 0;
        while (dtime < 10) {
          HandleUserButtonAndFunction(PanelDone);
          dtime = ((int) time(NULL)) - time0;
          nevt = analysisCounter[PROCESSEDEVENT];
          if (nevt > 1000) break;
          if (!checkMezzOnOff) break;
        }
        StopDAQ();
        autoRunOn = FALSE;
      }
      if (nevt > 0) color = VAL_GREEN;
      else {
        if (((1<<mezz)&savedMezzEnables) == 0) color = VAL_YELLOW;
        else {
          mezzOnOffStatus |= (1 << mezz);
          color = VAL_RED;
        }
      }
    }
    else if (((1<<mezz)&savedMezzEnables) == 0) {
      color = VAL_YELLOW;
      numberCSMDownload = savedNumberCSMDownload;
      numberBadCSMDownload = savedNumberBadCSMDownload;
    }
    else {
      mezzOnOffStatus |= (1 << mezz);
      color = VAL_RED;
    }
    if (!checkMezzOnOff) break;
    SetCtrlAttribute(CheckMezzOnOffHandle, button[mezz], ATTR_TEXT_BGCOLOR, color);
    HandleUserButtons(PanelDone);
  }
  autoRunOn = FALSE;
  checkMezzOnOff = FALSE;
  SetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, storage);
  SetCtrlVal(RunConditionHandle, P_RUNCOND_SAVERAWDATA, rawData);
  SetCtrlVal(RunConditionHandle, P_RUNCOND_ANALYSIS, analysis);
  SetCtrlVal(CheckMezzOnOffHandle, P_MEZZJTAG_CHECKINGMEZZ, -1);
  SetCtrlAttribute(CheckMezzOnOffHandle, P_MEZZJTAG_CHECKINGMEZZ, ATTR_TEXT_BGCOLOR, VAL_MAGENTA);
  HandleUserButtons(PanelDone);
  TTCviRestoreTriggerSettings();
  mezzEnables = savedMezzEnables;
  SaveParameters(PARAMEZZENABLES);
  SetMezzCardEnables();
  if (gotTTCvi) {
    SetTTCviTriggerAndRate(trig, &rate);
    triggerRate = rate;
    TTCviSetUnusedTrigger();
  }
  InitDAQ();
}


void CheckAllDAQHardware(void) {
  int noExternalTrig = TRUE, abnormalExternalTrig = FALSE, resp, i, val1, val2;

  allDAQHardwareStatus = 0;
  MDTChamberOutput1("Start to check all DAQ hardware, take a while!\n"); 
  if (!gotTTCvi) {
    MessagePopup("No TTCvi", 
                 "No TTCvi module is found and it is impossible to collect data with CSM.\n\n"
                 "Possible reasons:\n"
                 "1) TTCvi module is not firmly installed inside VME crate;\n"
                 "2) TTCvi module is broken.\n");
    allDAQHardwareStatus = 1;
    MDTChamberOutput1("No TTCvi\n");
  }
  if (allDAQHardwareStatus == 0) {
    printf("Measure TTCvi various trigger rate, takes while please be patient\n"); 
    HidePanel(DAQControlHandle);
    DisplayPanel(TrigSelectHandle);
    SetActivePanel(TrigSelectHandle);
    measureAllTriggerRate = FALSE;
    MeasureVariesTriggerRate();
    measureAllTriggerRate = TRUE;
    if (MDTChamberTest || checkAllDAQHardware) {
      if (GetMeasuredTriggerRate(0, 0) > 0) {
        if (GetMeasuredTriggerRate(0, 0) > 100000.0) abnormalExternalTrig = TRUE;
        else noExternalTrig = FALSE;
      }
      if (GetMeasuredTriggerRate(1, 0) > 0) {
        if (GetMeasuredTriggerRate(1, 0) > 100000.0) abnormalExternalTrig = TRUE;
        else noExternalTrig = FALSE;
      }
      if (GetMeasuredTriggerRate(2, 0) > 0) {
        if (GetMeasuredTriggerRate(2, 0) > 100000.0) abnormalExternalTrig = TRUE;
        else noExternalTrig = FALSE;
      }
      if (GetMeasuredTriggerRate(3, 0) > 0) {
        if (GetMeasuredTriggerRate(3, 0) > 100000.0) abnormalExternalTrig = TRUE;
        else noExternalTrig = FALSE;
      }
      if (MDTChamberTest && (MDTChamberHVStatus == MDTCHAMBERHVON)) {
        if (noExternalTrig && useExternalTrigger) {
          MessagePopup("No Valid External Trigger", 
                       "No valid external trigger is found, will unable to perform cosmic ray test.\n\n"
                       "Possible reasons:\n"
                       "1) External trigger cable to TTCvi is not connected or bad cable;\n"
                       "2) Abnormal external trigger (trigger rate > 100KHz, corresponding channel is broken);\n"
                       "3) External trigger system has problem;\n"
                       "4) TTCvi module has problem.\n");
          allDAQHardwareStatus = 2;
          MDTChamberOutput1("No Valid External Trigger\n");
	    }
        if (abnormalExternalTrig && (allDAQHardwareStatus == 0)) {
          resp = ConfirmPopup("Abnormal External Trigger", 
                              "Abnormal external trigger is found (i.e. trigger rate > 100KHz).\n"
                              "Abnormal external trigger should not be used for DAQ.\n\n"
                              "Possible reasons:\n"
                              "1) Corresponding trigger channel is broken at TTCvi);\n"
                              "2) External trigger system has problem;\n\n"
                              "Are you sure that abnormal external trigger is not used and want to continue?");
          if (resp == 0) allDAQHardwareStatus = 3;
          MDTChamberOutput1("Abnormal External Trigger\n");
	    }
	  }
	  if (MDTChamberTest || checkCSM) {
        if ((allDAQHardwareStatus == 0) && (GetMeasuredTriggerRate(6, 0) <= 0.0)) {
          resp = ConfirmPopup("No Calibration Trigger", 
                              "Calibration trigger is not available.\n"
                              "One would not be able to perform calibration run.\n\n"
                              "Possible reasons:\n"
                              "1) No external NIM signal is connected to TTCvi B-Go channel 2;\n"
                              "2) NIM cable to TTCvi B-Go channel 2 is loose or bad;\n"
                              "3) External NIM signal to TTCvi B-Go channel is not good;\n"
                              "4) TTCvi module has problem;\n\n"
                              "Are you sure no calibration run and want to continue?");
          if (resp == 0) allDAQHardwareStatus = 4;
          MDTChamberOutput1("No Calibration Trigger\n");
	    }
        if ((allDAQHardwareStatus == 0) && (GetMeasuredTriggerRate(6, 0) > 100000.0)) {
          resp = ConfirmPopup("Abnormal Calibration Trigger", 
                              "Calibration trigger is too high (> 100Khz).\n"
                              "One would not be able to perform calibration run.\n\n"
                              "Possible reasons:\n"
                              "1) NIM cable to TTCvi B-Go channel 2 is loose or bad;\n"
                              "2) External NIM signal to TTCvi B-Go channel is not good;\n"
                              "3) TTCvi module has problem;\n\n"
                              "Are you sure no calibration run and want to continue?");
          if (resp == 0) allDAQHardwareStatus = 5;
          MDTChamberOutput1("Abnormal Calibration Trigger\n");
	    }
	  }
    }
    HandleUserButtons(PanelDone);
    WaitInMillisecond(1000);
    HidePanel(TrigSelectHandle);
    HandleUserButtons(PanelDone);
    WaitInMillisecond(100);
    DisplayPanel(DAQControlHandle);
    SetActivePanel(DAQControlHandle);
  }
  if (allDAQHardwareStatus == 0) {
    printf("Search CSM board.\n"); 
    SetupJTAGChain();
    if (!gotCSM) {
      MessagePopup("No CSM Found", 
                   "No CSM is found in the JTAG chain, DAQ system will not work.\n\n"
                   "Possible reasons:\n"
                   "1) No power on CSM Motherboard or JTAG adaptor;\n"
                   "2) Related JTAG cables are not connected properly, such as:\n"
                   "   a) JTAG OUT is connected to JTAG IN and JTAG IN is connected to JTAG OUT;\n"
                   "   b) Flat ribbon cable from JTAG adaptor is not connected to HS1 connector on Motherboard;\n"
                   "   c) JTAG cables are loose or broken;\n"
                   "3) CSM1 is not firmly installed on Motherboard;\n"
                   "4) CSM1 or Motherboard is broken;\n"
                   "5) Problem related with JTAG Driver (Hardware or Software).\n");
      allDAQHardwareStatus = 10;
      MDTChamberOutput1("No Found CSM\n");
    }
  }
  if (allDAQHardwareStatus == 0) {
    printf("Check downloading on CSM.\n"); 
    DownloadCSMSetup();
    if (CSMSetupStatus() != 0) DownloadCSMSetup();     // Try again if failed 
    if (CSMSetupStatus() != 0) DownloadCSMSetup();     // Try again if failed 
    if (CSMSetupStatus() != 0) {
      MessagePopup("Failure in Setup CSM", 
                   "Downloading CSM setttings are failed, DAQ system will not work.\n\n"
                   "Possible reasons:\n"
                   "1) No power on CSM Motherboard or JTAG adaptor;\n"
                   "2) Related JTAG cables are not connected properly, such as:\n"
                   "   a) JTAG OUT is connected to JTAG IN and JTAG IN is connected to JTAG OUT;\n"
                   "   b) Flat ribbon cable from JTAG adaptor is not connected to HS1 connector on Motherboard;\n"
                   "   c) JTAG cables are loose or broken;\n"
                   "3) CSM1 is not firmly installed on Motherboard;\n"
                   "4) CSM1 or Motherboard is broken;\n"
                   "5) Problem related with JTAG Driver (Hardware or Software).\n");
      allDAQHardwareStatus = 11;
      MDTChamberOutput1("Setup CSM Failed\n");
    }
  }
  if (allDAQHardwareStatus == 0) {
    printf("Check DAQ initialization.\n");
    InitDAQ();
    if (CSMBoardStatus() != 0) InitDAQ();             // Try again if failed
    if (CSMBoardStatus() != 0) InitDAQ();             // Try again if failed
    if (CSMBoardStatus() != 0) {
      MessagePopup("Failure in DAQ Initialization", 
                   "DAQ initialization is failed, DAQ system will not work.\n\n"
                   "Possible reasons:\n"
                   "1) Cable connecting mezzanine card and Motherboard:\n"
                   "   a) Not firmly connected at Motherboard;\n"
                   "   b) bad cable;\n"
                   "   c) Not properly connected at mezzanine card;\n"
                   "2) Bad chip on CSM1 board, such as GOL or TTCrx;\n"
                   "3) CSM1 or Motherboard is broken;\n"
                   "4) Problem related with JTAG Driver (Hardware or Software).\n");
      allDAQHardwareStatus = 12;
      MDTChamberOutput1("DAQ Initialization Failed\n");
    }
    if (allDAQHardwareStatus == 0) {
      printf("Check TTC system (TTCvi/TTCrx).\n"); 
      for (i = 0; i < 100; i++) {
        TTCviEventCounterReset();
        TTCviBunchCounterReset();
        TTCviEventAndBunchCounterReset();
        if ((i%10) == 9) printf("Check TTC system step %d is done\n", i);
      }
      printf("Need to wait 8 seconds for CSM to read TTCrx status registers.\n"); 
      WaitInMillisecond(8000);
      GetCSMStatus();
      if (CSMSetupStatus() == 0) {
        BinaryToInt(&val1, TTCRXSERRORCOUNT0, 16, CSMStatusArray);
        BinaryToInt(&val2, TTCRXDERRORCOUNT, 8, CSMStatusArray);
        MDTChamberOutput2("Single Error Count in CSM TTCrx is %d\n", val1);
        MDTChamberOutput2("Double Error Count in CSM TTCrx is %d\n", val2);
        if ((val1 != 0) || (val2 != 0)) {
          resp = ConfirmPopup("Found Problem in TTC system", 
                              "Single/Double errors have been seen in CSM TTCrx chip.\n"
                              "With this problem (TTCvi and TTCrx clock may not sychronized),\n"
                              "the data still can be collected with following effects:\n"
                              "A) CSM may not be able to catch a trigger (probability small);\n"
                              "B) CSM may not be able to catch a various commands from TTCvi, such as\n"
                              "   Bunch/Event Count Reset, Calibration Command\n"
                              "C) Efficiency from injection tests will be smaller than 1 (around 0.97);\n"
                              "D) Only less than 70% hits will be used to compute time resolution in linearity scan test;\n\n"
                              "Possible reasons (assuming TTCvi and TTCrx are used):\n"
                              "1) <TTCrx Clock Out> is not connected to <TTCvi Clock In bc/ecl).\n"
                              "   Note: Do not connect to <TTCvi Clock Spare I/O>;\n"
                              "2) Bad cable between <TTCrx Clock Out> and <TTCvi Clock In bc/ecl);\n"
                              "3) improper <BC delay> in TTCvi, adjust it to see if it helps;\n"
                              "4) Problem in TTCvi or TTCrx;\n"
                              "5) Problem in fiber cable between CSM and TTCvi;\n\n"
                              "Are you sure to continue with non-perfect TTC system?");
          if (resp == 0) allDAQHardwareStatus = 3;
          MDTChamberOutput1("CSM TTCrx has Single/Double Error from TTC system.\n");
        }
      }
    }
  }
  if (allDAQHardwareStatus == 0) {
    printf("Check TCP/IP connection.\n");
    NumberConnectedTCPNodes();
    if (!DAQTCPConnected) {
      resp = ConfirmPopup("No DAQ from Linux", 
                          "CSMMiniDAQ at Linux is not connected, will unable to collect data.\n\n"
                          "To start CSMMiniDAQ at Linux machine to control GOLA card, do following:\n\n"
                          "1) Login to the Linux machine;\n"
                          "2) Go to the directory where CSMMiniDAQ.exe is stored;\n"
                          "3) Type: ./CSMMiniDAQ.exe MiniDAQ-Node-Name;\n\n"
                          "Are you sure to continue without CSMMiniDAQ at Linux?");
      if (resp == 0) allDAQHardwareStatus = 100;
      MDTChamberOutput1("No DAQ on Linux Box to Control GOLA Card\n");
    }
    else {
      SendTCPCommand(-1, CMDINITDAQ);
      if (CheckClientsStatus(UNIX_PORT_NUM) != 0) {
        MessagePopup("Failure to Initialize GOLA card", 
                     "GOLA card at Linux initialization is failed, DAQ system will not work.\n\n"
                     "Possible reasons:\n"
                     "1) Communication between PC and Linux is broken:\n"
                     "     i) Quit the MiniDAQ program at PC and CSMMiniDAQ program at Linux\n"
                     "    ii) Start MiniDAQ program at PC (this computer);\n"
                     "   iii) Start CSMMiniDAQ at Linux;\n"
                     "2) GOLA card has problem;\n");
        allDAQHardwareStatus = 101;
        MDTChamberOutput1("GOLA Card Initialization Failed\n");
      }
    }
  }
  if (allDAQHardwareStatus == 0) {
    printf("Check Mezzanine Card JTAG connection.\n");
    UpMezzCardJTAGStatus();
    if (searchAMTStatus != 0) {
      MessagePopup("JTAG Failure for Mezzanine Card", 
                   "Enabled mezzanine card(s) is not in JTAG chain or downloading its settings is failed.\n"
                   "DAQ system will not work properly.\n\n"
                   "Possible reasons:\n"
                   "1) CSM1 is not firmly installed on Motherboard;\n"
                   "2) Cable connecting mezzanine card and Motherboard:\n"
                   "   a) Not firmly connected at Motherboard;\n"
                   "   b) bad cable;\n"
                   "   c) Not properly connected at mezzanine card;\n"
                   "2) Mezzanine card has problem;\n"
                   "3) CSM board has problem;\n");
      allDAQHardwareStatus = 1000;
      HidePanel(DAQControlHandle);
      DisplayPanel(MezzJTAGStatusHandle);
      SetActivePanel(MezzJTAGStatusHandle);
      MDTChamberOutput1("JTAG Failed for Enabled Mezzanine Card(s)\n");
    }
    else {
      HidePanel(MezzJTAGStatusHandle);
      DisplayPanel(DAQControlHandle);
      SetActivePanel(DAQControlHandle);
    }
  }
  if (allDAQHardwareStatus == 0) {
    printf("Check Mezzanine Card On/Off Status.\n");
    UpCheckMezzCardOnOff();
    if (mezzOnOffStatus != 0) {
      MessagePopup("Mezzanine Card Off", 
                   "Unable to collect data from enabled mezzanine card(s).\n"
                   "DAQ system will not work properly.\n\n"
                   "Possible reasons:\n"
                   "1) Check if PC and Linux Box communication is fine;\n"
                   "2) Check if fiber from CSM board to GOLA connect is connected or broken;\n"
                   "3) GOLA card has problem (dismount it from Linux Box and reinstall it\n"
                   "   normal fixed the problem. If none mezzanine card can collect data,\n"
                   "   the most likely trouble is 1 or 2);\n"
                   "4) CSM1 is not firmly installed on Motherboard;\n"
                   "5) Cable connecting mezzanine card and Motherboard:\n"
                   "   a) Not firmly connected at Motherboard;\n"
                   "   b) bad cable;\n"
                   "   c) Not properly connected at mezzanine card;\n"
                   "6) Mezzanine card has problem;\n"
                   "7) CSM board has problem;\n");
      allDAQHardwareStatus = 1000;
      HidePanel(DAQControlHandle);
      DisplayPanel(CheckMezzOnOffHandle);
      SetActivePanel(CheckMezzOnOffHandle);
      MDTChamberOutput1("Unable to Collect Data from Enabled Mezzanine Card(s)\n");
    }
    else {
      HidePanel(CheckMezzOnOffHandle);
      DisplayPanel(DAQControlHandle);
      SetActivePanel(DAQControlHandle);
    }
  }
  MDTChamberOutput2("End of checking all DAQ hardware with Status = %d\n", allDAQHardwareStatus); 
}


// Run Condition Routines
//
// Load Run Condition Panel and define user buttons
int RunConditionPanelAndButton(void) {
  if ((RunConditionHandle = LoadPanel(0, "DAQGUI.uir", P_RUNCOND)) < 0) return -1;
  SetPanelAttribute(AutoRunSetupHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);

  // Auto Run Setup Panel Buttons
  SetupUserButton(Menu00Handle, MENU00_EXE_RUNCONDITION, UpRunCondition);
  SetupUserButton(RunConditionHandle, P_RUNCOND_DONE, RunConditionDone);
  SetupUserButton(RunConditionHandle, P_RUNCOND_ALLON, RunConditionAllOn);
  SetupUserButton(RunConditionHandle, P_RUNCOND_ALLOFF, RunConditionAllOff);
  SetupUserButton(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, SetMDTChamberHVStatus);
  
  return 0;
}


// Bring up Run Condition Panel
void UpRunCondition(void) {
  if (controlOption == NORM) {
    DisplayPanel(RunConditionHandle);
    SetActivePanel(RunConditionHandle);
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Execute Menu", 
                 "Bring up Run Condition controls.");
  controlOption = NORM;
}


void RunConditionDone(void) {
  int val;
  
  if (controlOption == NORM) {
    HidePanel(RunConditionHandle);
    GetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, &mappingMDTChamber);
    GetCtrlVal(RunConditionHandle, P_RUNCOND_CHECKSYSTEMNOISE, &checkSystemNoise);
    if ((mappingMDTChamber || checkSystemNoise) && (MDTChamberType == CTYPEUNKNOWN)) {
      UpSerialNumber();
      if (MDTChamberType == CTYPEUNKNOWN) {
        mappingMDTChamber = FALSE;
        SetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, mappingMDTChamber);
      }
    }
    GetRunCondition();
    GetCtrlVal(RunConditionHandle, P_RUNCOND_NTHRESHOLD, &val);
    if (val != nominalThreshold) {
      nominalThreshold = val;
      SaveParameters(PARANOMINALTHRESHOLD);
      LoadNominalASDMainThreshold();
    }
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Run Condition Panel Buttons", 
                 "Exit from Run Condition Panel.");
  controlOption = NORM;
}


void RunConditionAllOn(void)
{
  if (controlOption == NORM) {
    SetCtrlVal(RunConditionHandle, P_RUNCOND_CSMHEADER, TRUE);
    SetCtrlVal(RunConditionHandle, P_RUNCOND_CSMTRAILER, TRUE);
    SetCtrlVal(RunConditionHandle, P_RUNCOND_TDCHEADER, TRUE);
    SetCtrlVal(RunConditionHandle, P_RUNCOND_TDCTRAILER, TRUE);
   }
  else if (controlOption == HELP)
    MessagePopup("Help on Run Condition Panel Buttons", 
                 "Turn on all controls on Run Condition Panel.");
  controlOption = NORM;
}


void RunConditionAllOff(void)
{
  if (controlOption == NORM) {
    SetCtrlVal(RunConditionHandle, P_RUNCOND_CSMHEADER, FALSE);
    SetCtrlVal(RunConditionHandle, P_RUNCOND_CSMTRAILER, FALSE);
    SetCtrlVal(RunConditionHandle, P_RUNCOND_TDCHEADER, FALSE);
    SetCtrlVal(RunConditionHandle, P_RUNCOND_TDCTRAILER, FALSE);
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Run Condition Panel Buttons", 
                 "Turn off all controls on Run Condition Panel.");
  controlOption = NORM;
}


void SetMDTChamberHVStatus(void) {
  GetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, &mappingMDTChamber);
  if (mappingMDTChamber) {
    checkSystemNoise = FALSE;
    SetCtrlVal(RunConditionHandle, P_RUNCOND_CHECKSYSTEMNOISE, checkSystemNoise);
    MDTChamberHVStatus = MDTCHAMBERHVON;
    SetCtrlVal(RunConditionHandle, P_RUNCOND_CHAMBERHV, MDTChamberHVStatus);
  }
}


void GetRunCondition(void) {
  GetCtrlVal(RunConditionHandle, P_RUNCOND_CSMHEADER, &suppressCSMHeader);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_CSMTRAILER, &suppressCSMTrailer);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_TDCHEADER, &suppressTDCHeader);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_TDCTRAILER, &suppressTDCTrailer);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_CHAMBERHV, &MDTChamberHVStatus);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_ANALYSIS, &analysisControl);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_SAVERAWDATA, &saveRawData);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_CHECKSYSTEMNOISE, &checkSystemNoise);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, &mappingMDTChamber);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_SAVEFLAGGEDEVT, &saveFlaggedEvt);
  GetCtrlVal(RunConditionHandle, P_RUNCOND_TRIGGERDEADTIME, &triggerDeadtime);
}


// Auto Run Routines
//
// Load Auto Run Setup Panel and define user buttons
int AutoRunSetupPanelAndButton(void) {
  if ((AutoRunSetupHandle = LoadPanel(0, "DAQGUI.uir", P_AUTORUN)) < 0) return -1;
  SetPanelAttribute(AutoRunSetupHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);

  // Auto Run Setup Panel Buttons
  SetupUserButton(Menu00Handle, MENU00_EXE_AUTORUNSETUP, UpAutoRunSetup);
  SetupUserButton(AutoRunSetupHandle, P_AUTORUN_DONE, AutoRunSetupDone);
  SetupUserButton(AutoRunSetupHandle, P_AUTORUN_ALLON, AutoRunSetupAllOn);
  SetupUserButton(AutoRunSetupHandle, P_AUTORUN_ALLOFF, AutoRunSetupAllOff);
  SetupUserButton(AutoRunSetupHandle, P_AUTORUN_RUNCONTROLFILE, AutoRunSelectControlFile);
  
  return 0;
}


// Bring up Auto Run Setup Panel
void UpAutoRunSetup(void) {
  if (controlOption == NORM) {
    DisplayPanel(AutoRunSetupHandle);
    SetActivePanel(AutoRunSetupHandle);
    autoRunSetupDone = 0;
    while (autoRunSetupDone == 0) {
      GetAutoRunControls();
      HandleUserButtonAndFunction(Quit);
    }
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Execute Menu", 
                 "Bring up Auto Run Setup controls.");
  controlOption = NORM;
}


void AutoRunSetupDone(void) {
  if (controlOption == NORM) {
    GetAutoRunControls();
    HidePanel(AutoRunSetupHandle);
    autoRunSetupDone = 1;
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Auto Run Setup Panel Buttons", 
                 "Exit from Auto Run Setup Panel.");
  controlOption = NORM;
}


void AutoRunSetupAllOn(void)
{
  if (controlOption == NORM) {
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, TRUE);
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, TRUE);
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTEVENTON, TRUE);
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, TRUE);
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTHANGON, TRUE);
   }
  else if (controlOption == HELP)
    MessagePopup("Help on Auto Run Setup Panel Buttons", 
                 "Turn on all controls on Auto Run Setup Panel.");
  controlOption = NORM;
}


void AutoRunSetupAllOff(void)
{
  if (controlOption == NORM) {
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, FALSE);
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, FALSE);
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTEVENTON, FALSE);
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, FALSE);
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTHANGON, FALSE);
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Auto Run Setup Panel Buttons", 
                 "Turn off all controls on Auto Run Setup Panel.");
  controlOption = NORM;
}


void GetAutoRunControls(void)
{
  static unsigned int toDisk, savedNTotalEvt = 0, savedNEvt = 0, savedHangTime = 0;
  static unsigned int savedStopNErr = 0, savedNTotalRun = 0, savedRestartTime = 0;
  
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_SCANCONTROL, &scanControl);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_INITDAQCONTROL, &initDAQControl);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_AUTOSTARTRUN, &autoStartRunControl);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, &stopAfterNRunOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, &stopRunAtEventOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTEVENTON, &restartRunAtEventOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, &restartRunAfterSecOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTHANGON, &restartRunAtHangTimeOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERNERRON, &stopAfterNErrorOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNS, &stopAfterNRun);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPATEVENT, &stopRunAtEvent);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTATEVENT, &restartRunAtEvent);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSEC, &restartRunAfterSec);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTHANGTIME, &restartRunAtHangTime);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERNERR, &stopAfterNError);
  if (savedNTotalRun == 0) savedNTotalRun = stopAfterNRun;
  if (savedNTotalEvt == 0) savedNTotalEvt = stopRunAtEvent;
  if (savedNEvt == 0) savedNEvt = restartRunAtEvent;
  if (savedRestartTime == 0) savedRestartTime = restartRunAfterSec;
  if (savedHangTime == 0) savedHangTime = restartRunAtHangTime;
  if (savedStopNErr == 0) savedStopNErr = stopAfterNError;
  if (savedNTotalRun != stopAfterNRun) 
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, TRUE);
  if (savedNTotalEvt != stopRunAtEvent) {
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, TRUE);
    if (stopRunAtEvent < restartRunAtEvent) {
      SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTATEVENT, stopRunAtEvent);
      GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTATEVENT, &restartRunAtEvent);
      savedNEvt = restartRunAtEvent;
    }
  }
  if (savedNEvt != restartRunAtEvent) {
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTEVENTON, TRUE);
    if (stopRunAtEvent < 2*restartRunAtEvent) {
      SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPATEVENT, 2*restartRunAtEvent);
      GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPATEVENT, &stopRunAtEvent);
      savedNTotalEvt = stopRunAtEvent;
    }
  }
  if (savedRestartTime != restartRunAfterSec) 
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, TRUE);
  if (savedHangTime != restartRunAtHangTime)
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTHANGON, TRUE);
  if (savedStopNErr != stopAfterNError)
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERNERRON, TRUE);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, &stopAfterNRunOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, &stopRunAtEventOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTEVENTON, &restartRunAtEventOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, &restartRunAfterSecOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTHANGON, &restartRunAtHangTimeOn);
  GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERNERRON, &stopAfterNErrorOn);
  savedNTotalRun = stopAfterNRun;
  savedNTotalEvt = stopRunAtEvent;
  savedNEvt = restartRunAtEvent;
  savedRestartTime = restartRunAfterSec;
  savedHangTime = restartRunAtHangTime;
  savedStopNErr = stopAfterNError;
  if ((restartRunAtEventOn == 1) || (restartRunAfterSecOn == 1) ||(restartRunAtHangTimeOn == 1)) {
    GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, &toDisk);
    if (toDisk != 0) SetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, 1);
  }
}


char controlFilePath[5192];
// Select Automatic Run Control File
void AutoRunSelectControlFile(void)
{
  int numberValidRun = 0, mezzNotInDB, stat, resp, mezz, ch, val, valid;
  char str[8192];
  
  snButton[0] = P_SNUMBER_MEZZ00SN;
  snButton[1] = P_SNUMBER_MEZZ01SN;
  snButton[2] = P_SNUMBER_MEZZ02SN;
  snButton[3] = P_SNUMBER_MEZZ03SN;
  snButton[4] = P_SNUMBER_MEZZ04SN;
  snButton[5] = P_SNUMBER_MEZZ05SN;
  snButton[6] = P_SNUMBER_MEZZ06SN;
  snButton[7] = P_SNUMBER_MEZZ07SN;
  snButton[8] = P_SNUMBER_MEZZ08SN;
  snButton[9] = P_SNUMBER_MEZZ09SN;
  snButton[10] = P_SNUMBER_MEZZ10SN;
  snButton[11] = P_SNUMBER_MEZZ11SN;
  snButton[12] = P_SNUMBER_MEZZ12SN;
  snButton[13] = P_SNUMBER_MEZZ13SN;
  snButton[14] = P_SNUMBER_MEZZ14SN;
  snButton[15] = P_SNUMBER_MEZZ15SN;
  snButton[16] = P_SNUMBER_MEZZ16SN;
  snButton[17] = P_SNUMBER_MEZZ17SN;
  if (controlOption == NORM) {
    if (FileSelectPopup("", "*.ctrl", "", "Select Automatic Run Control File",
                        VAL_LOAD_BUTTON, 0, 1, 1, 0, controlFilePath)) {
      sprintf(str, "Control File -> <%s>\n"
                   "Are you sure it is the right control file?\n", controlFilePath);
      resp = ConfirmPopup("Confirm Selected Control File", str);
      if (resp == 0) {
        autoRunFromControlFile = 0;
        return;
      }
      if (autoStartRunControl != 0) {
        TTCviRestoreTrigger();   // In case no trigger selected, this would do it
        TTCviSetUnusedTrigger();
	  }
      reqProcessData = FALSE;
	  allowedFractionBadCycles = 0.0;
      checkCSM = 0;
      checkCSMJTAG = 0;
      checkCSMIO = 0;
      MDTChamberControl = FALSE;
      autoRunFromControlFile = 1;
      scanOutputFormat = 0;
      needLoadNominalThreshold = FALSE;
      thresholdScanTrigWarning = FALSE;
      thresholdScanMethod = 0;
      numberSubScans = 1;
      subScans = numberSubScans - 1;
      willDoThresholdScan = FALSE;
      reqThresholdScan = FALSE;
      thresholdScanPaired = TRUE;
      reqInjectionScan = FALSE;
      injectionScanPaired = TRUE;
      reqLinearityScan = FALSE;
      linearityScanPaired = TRUE;
      reqGainScan = FALSE;
      gainScanPaired = TRUE;
      differentScanMixed = FALSE;
      useExternalTrigger = FALSE;
      while (autoRunFromControlFile == 1) if (GetNewRunSetups(0) == 1) numberValidRun++;
      if (CheckClientInstallations(3) != 0) {
        autoRunFromControlFile = 0;
        return;
      }
      if (thresholdScanTrigWarning) {
        sprintf(str, "For threshold scan test, it is recommended to use software trigger.\n"
                     "Right now, external trigger is selected to do threshold scan test\n"
                     "in control file <%s>.\n"
                     "To change trigger selection, please modify the control file first.\n\n"
                     "Are you sure to use external trigger for threshold scan test?\n", controlFilePath);
        resp = ConfirmPopup("Confirm External Trigger Usage", str);
        if (resp == 0) {
          autoRunFromControlFile = 0;
          return;
        }
      }
      if (numberValidRun == 0) {
        MessagePopup("No Valid Run",
                     "No valid run was found in selected Run Control File!\n"
                     "Please check Run Control File to make sure it is OK!");
        autoRunFromControlFile = 0;
      }
      else if (!thresholdScanPaired) {
        MessagePopup("Unpaired Threshold Scan Command(s)",
                     "Threshold Scan Command is not in pair, check the auto run control file\n\n"
                     "Possible reasons\n"
                     "1) No Command <Start Threshold Scan>;\n"
                     "2) No Command <End Threshold Scan>;\n"
                     "3) Threshold Scan Commands in reversed order.");
        autoRunFromControlFile = 0;
      }
      else if (!injectionScanPaired) {
        MessagePopup("Unpaired Injection Scan Command(s)",
                     "Injection Scan Command is not in pair, check the auto run control file\n\n"
                     "Possible reasons\n"
                     "1) No Command <Start Injection Scan>;\n"
                     "2) No Command <End Injection Scan>;\n"
                     "3) Injection Scan Commands in reversed order.");
        autoRunFromControlFile = 0;
      }
      else if (!linearityScanPaired) {
        MessagePopup("Unpaired Linearity Scan Command(s)",
                     "Linearity Scan Command is not in pair, check the auto run control file\n\n"
                     "Possible reasons\n"
                     "1) No Command <Start Linearity Scan>;\n"
                     "2) No Command <End Linearity Scan>;\n"
                     "3) Linearity Scan Commands in reversed order.");
        autoRunFromControlFile = 0;
      }
      else if (!gainScanPaired) {
        MessagePopup("Unpaired Gain Scan Command(s)",
                     "Gain Scan Command is not in pair, check the auto run control file\n\n"
                     "Possible reasons\n"
                     "1) No Command <Start Gain Scan>;\n"
                     "2) No Command <End Gain Scan>;\n"
                     "3) Gain Scan Commands in reversed order.");
        autoRunFromControlFile = 0;
      }
      else if (differentScanMixed) {
        MessagePopup("Different Scan Mixed ",
                     "Mix different scan is not allowed!\n"
                     "For example, <Start Threshold Scan> must be finished by <End Threshold Scan>\n"
                     "before one could start another different scan.\n"
                     "Please check the auto control file");
        autoRunFromControlFile = 0;
      }
      else {
        strcpy(MDTChamberWarningFilename, "");
        MDTChamberWarningFile = NULL;
        numberCSMDownload = 0;
        numberBadCSMDownload = 0;
        numberMezzDownload = 0;
        numberBadMezzDownload = 0;
        numberInitDAQ = 0;
        numberBadInitDAQ = 0;
        numberReInitDAQ = 0;
        numberScanRun = 0;
        numberIgnoredScanRun = 0;
        numberBadScanRun = 0;
        AutoRunSetupDone();
        for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
          for (ch = 0; ch < 24; ch++) {
            channelStatusHVOn[ch][mezz] = RDOUTCHOK + RDOUTSTATUSOFFSETHVON;
            channelStatusHVOff[ch][mezz] = RDOUTCHOK + RDOUTSTATUSOFFSETHVOFF;
          }
        }
        if (needLoadNominalThreshold || scanSerialNumber || MDTChamberTest) {
          UpSerialNumber();
          if (needLoadNominalThreshold) needLoadNominalThreshold = FALSE;
        }
        if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
          if (MDTChamberSumFile != NULL) {
            fclose(MDTChamberSumFile);
            MDTChamberSumFile = NULL;
          }
          if (strlen(MDTChamberName) > 5) strcpy(dataDirName, MDTChamberName);
          else strcpy(dataDirName, "dat");
          if (SetDir(dataDirName) == 0) SetDir("..");
          else {
            strcpy(str, dataDirName);
            strcat(str, "_savedByDAQ");
            RenameFile(dataDirName, str);
            MakeDir(dataDirName);
          }
          strcpy(MDTChamberSumFilename, dataDirName);
          strcat(MDTChamberSumFilename, "/");
          strcat(MDTChamberSumFilename, MDTChamberName);
          strcat(MDTChamberSumFilename, "_ECTest.sum");
          stat = GetFileInfo(MDTChamberSumFilename, &val);
          if ((stat > 0) && (val > 5000)) {
            MDTChamberSumFile = fopen(MDTChamberSumFilename, "a");
          }
          else if (stat < 0) {
            strcpy(str, MDTChamberSumFilename);
            strcat(str, "_savedByDAQ");
            RenameFile(MDTChamberSumFilename, str);
            stat = 0;
          }
          else stat = 0;
          if (stat == 0) {
            if (MDTChamberHVStatus == MDTCHAMBERHVON) {
              sprintf(str, "Did not find MDT chamber electronics and cosmic ray test summary file\n"
                           "        <%s>\n\n"
                           "===> MDT chamber electronics tests with HV OFF may not be done yet.\n"
                           "Do you still wish to do MDT chamber electronics tests with HV ON?\n",
                           MDTChamberSumFilename);
              resp = ConfirmPopup("Confirm HV ON MDT Chamber Tests", str);
              if (resp == 0) {
                autoRunFromControlFile = 0;
                willDoThresholdScan = FALSE;
                UpAutoRunSetup();
                return;
              }
            }
            MDTChamberSumFile = fopen(MDTChamberSumFilename, "w");
            if (MDTChamberSumFile != NULL) {
              MDTChamberOutput1("--- MDT Chamber Summary File of Electronics and Cosmic Ray Tests ---\n");
              MDTChamberOutput1("//\n");
              MDTChamberOutput1("// It is necessary that user pay attention to two type messages\n");
              MDTChamberOutput1("// Message marked as <WARNING>, Waring Message for User\n");
              MDTChamberOutput1("//   In such case, following actions may be necessary:\n");
              MDTChamberOutput1("//   A) Replacing Mezzanine Card(s);\n");
              MDTChamberOutput1("//   B) Replacing Signal or HV Hedgehog Cards in case hot/dead tube;\n");
              MDTChamberOutput1("//   C) Repairing tube in case hot/dead tube;\n");
              MDTChamberOutput1("//   D) General problem, such as a lot of noise channels/tubes.\n");
              MDTChamberOutput1("//      Many possibilites -> grounding, cable connections, replacing motherboard...;\n");
              MDTChamberOutput1("//   E) Ignore it, such as a known dead tube;\n");
              MDTChamberOutput1("// Message marked as <-INFOR->, Information for User\n");
              MDTChamberOutput1("//   In such case, user is required to pay more attention on channel(s) or\n");;
              MDTChamberOutput1("//   mezzanine card(s) in cosmic ray data. In case abnormal behaviour is\n");
              MDTChamberOutput1("//   observed in cosmic ray data analysis, following actions may be necessary:\n");
              MDTChamberOutput1("//   A) Replacing Mezzanine Card(s);\n");
              MDTChamberOutput1("//   B) Replacing Signal or HV Hedgehog Cards in case hot/dead tube;\n");
              MDTChamberOutput1("//   C) Repairing tube in case hot/dead tube;\n");
              MDTChamberOutput1("// ==> User's judgement is necessary for both cases\n");
              MDTChamberOutput1("//\n");
            }
          }
          if (MDTChamberSumFile == NULL) {
            sprintf(str, "Unable to open MDT chamber summary file <%s>\n"
                         "for electronics and cosmic ray tests.\n\n"
                         "Do you wish to continue without saving MDT chamber test results?\n",
                         MDTChamberSumFilename);
            resp = ConfirmPopup("Unable to Open MDT Chamber Summary File", str);
            if (resp == 0) {
              autoRunFromControlFile = 0;
              willDoThresholdScan = FALSE;
              return;
            }
          }
          else {
            MDTChamberOutput1("\n");
            MDTChamberOutput2("Electronics and Cosmic Ray Tests for MDT Chamber on %s\n", WeekdayTimeString(time(NULL)));
            MDTChamberOutput2("Operator Name                   = %30s\n", operatorName);
            MDTChamberOutput2("MDT Chamber Serial Number       = %30s\n", MDTChamberSN);
            MDTChamberOutput2("MDT Chamber Name                = %30s\n", MDTChamberName);
            MDTChamberOutput2("Motherboard Serial Number       = %30s\n", motherboardSN);
            MDTChamberOutput2("CSM Serial Number               = %30s\n", CSMSN);
            MDTChamberOutput2("MDT-DCS Serial Number           = %30s\n", MDTDCSSN);
            MDTChamberOutput2("MDT-DCS NIKHEF ID               = %30s\n", MDTDCSNIKHEFID);
            MDTChamberOutput2("MDT-DCS Expected ELMB Node      = %30d\n", expectedELMBNode);
            if (JTAGDRIVERTYPE == CANELMB) {
              if (useOnChamberDCS) {
                MDTChamberOutput2("MDT-DCS Actual ELMB Node        = %30d\n", ELMBNode);
                MDTChamberOutput2("MDT-DCS ELMB Version            = %30s\n", ELMBVersion);
              }
              else {
                if (CANNode >= 0) MDTChamberOutput2("CAN Node                        : %30d\n", CANNode);
                if (ELMBNode > 0) {
                  MDTChamberOutput2("ELMB Node                       : %30d\n", ELMBNode);
                  MDTChamberOutput2("ELMB Version                    : %30s\n", ELMBVersion);
                }
              }
            }
            MDTChamberOutput2("MDT Chamber Start MezzCard       = %30d\n", startMezzCard);
            for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
              if (((mezzEnables>>mezz)%2) == 1) {
                GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
                sprintf(str, "Mezzanine Card %02d Serial Number = %30s\n", mezz, serialNumber);
                MDTChamberOutput2("%s", str);
              }
            }
            MDTChamberOutput2("MDT Chamber HV Status           = %30d\n", MDTChamberHVStatus);
          }

          MDTChamberOutput1("\n");
          strcpy(MDTChamberWarningFilename, dataDirName);
          strcat(MDTChamberWarningFilename, "/");
          strcat(MDTChamberWarningFilename, MDTChamberName);
          strcat(MDTChamberWarningFilename, "_WARNING.log");
          stat = GetFileInfo(MDTChamberWarningFilename, &val);
          if (stat == 0) {
            MDTChamberWarningFile = fopen(MDTChamberWarningFilename, "w");
            if (MDTChamberWarningFile != NULL) {
              MDTChamberOutput2("WARNING/INFOR message log file for MDT Chamber ECTests is opened at %s\n", WeekdayTimeString(time(NULL)));
              fclose(MDTChamberWarningFile);
              MDTChamberWarningFile = NULL;
            }
            else {
              MDTChamberOutput2("Unable to open log file <%s> to save WARNING/INFOR message\n", MDTChamberWarningFilename);
              MDTChamberOutput1("WARNING/INFOR messages will only be into ECTests Summary File (i.e, this file)\n");
              strcpy(MDTChamberWarningFilename, "");
            }
            MDTChamberOutput1("\n");
          }
        }
        
        if (needLoadNominalThreshold || scanSerialNumber || MDTChamberTest) {
          readNominalThresholdOnly = TRUE;
          LoadNominalASDMainThreshold();
          readNominalThresholdOnly = FALSE;
        }
        if (MDTChamberTest) {
          resp = 1;
          if ((resp == 1) && (!gotMezzDatabase)) {
            sprintf(str, "Unable to get mezzanine card database file <mezzanineCardDatabase.txt>\n"
                         "under directory <database> for electronics and cosmic ray tests.\n\n"
                         "Do you wish to continue without mezzanine card database?\n");
            resp = ConfirmPopup("Unable to Get Mezzanine Card Database File", str);
          }
          if ((resp == 1) && largeVOffSpan) {
            sprintf(str, "Found mezzanine card(s) with large V offset span (> 16mV).\n"
                         "See screen for more details.\n\n"
                         "If V offset is from ASD database,\n"
                         "it is REQUIRED to replace the mezzanine card(s) with large V offset span.\n\n"
                         "Do you wish to continue without replacing mezzanine card(s)?\n");
            resp = ConfirmPopup("Large V Offset Span", str);
          }
          mezzNotInDB = FALSE;
          for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
            if (((mezzEnables>>mezz)%2) == 1) {
              if (!getVOff[mezz]) mezzNotInDB = TRUE;
            }
          }
          if ((resp == 1) && mezzNotInDB) {
            sprintf(str, "Unable to find mezzanine card(s) in database, see screen for more details.\n\n"
                         "If used mezzanine card database is updated and V offset from ASD database,\n"
                         "it is REQUIRED to replace those mezzanine card(s).\n\n"
                         "Do you wish to continue without replacing mezzanine card(s)?\n");
            resp = ConfirmPopup("Mezzanine Card Not In Database", str);
          }
          if (resp == 0) {
            if (MDTChamberSumFile != NULL) {
              MDTChamberOutput1(">>>>>>>>>>  End MDT Chamber Tests <<<<<<<<<<\n\n");
              MDTChamberTest = FALSE;
              fclose(MDTChamberSumFile);
              MDTChamberSumFile = NULL;
            }
            autoRunFromControlFile = 0;
            willDoThresholdScan = FALSE;
            return;
          }
        }

        if (checkCSMJTAG) {
          InitDAQ();
          GetCSMStatus();
          for (val = 1; val < 11; val++) {
            printf("CSM JTAG check loop %d\n", val);
            CSMJTAGOperation(CSMIDCODE, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMIDCODE\n", val);
              break;
            }
            CSMJTAGOperation(CSMVERSIONDATE, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMVERSIONDATE\n", val);
              break;
            }
            CSMJTAGOperation(CSMAMTPARITYERROR, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMAMTPARITYERROR\n", val);
              break;
            }
            CSMJTAGOperation(CSMAMTPHASEERROR, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMAMTPHASEERROR\n", val);
              break;
            }
            CSMJTAGOperation(CSMAMTFULLPHASE, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMAMTFULLPHASE\n", val);
              break;
            }
            CSMJTAGOperation(CSMCSMSTATUS, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMCSMSTATUS\n", val);
              break;
            }
            CSMJTAGOperation(CSMTTCRXSTATUS, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMTTCRXSTATUS\n", val);
              break;
            }
            CSMJTAGOperation(CSMROBITS, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMROBITS\n", val);
              break;
            }
            CSMJTAGOperation(CSMFULLRW, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMFULLRW\n", val);
              break;
            }
            CSMJTAGOperation(CSMFULLRO, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMFULLRO\n", val);
              break;
            }
            CSMJTAGOperation(CSMCONFIGRW, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMCONFIGRW\n", val);
              break;
            }
            CSMJTAGOperation(CSMCONFIGRO, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMCONFIGRO\n", val);
              break;
            }
            CSMJTAGOperation(CSMCSMPARMRW, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMCSMPARMRW\n", val);
              break;
            }
            CSMJTAGOperation(CSMCSMPARMRO, TRUE);
            if (CSMTestStatus != 0) {
              MDTChamberOutput2("CSM JTAG failed at loop %d with JTAG instruction CSMCSMPARMRO\n", val);
              break;
            }
          }
          if (CSMTestStatus != 0) {
            MDTChamberOutput1("CSM JTAG Tests are failed.\n");
            if (MDTChamberSumFile != NULL) {
              MDTChamberOutput1(">>>>>>>>>>  End MDT Chamber Tests <<<<<<<<<<\n\n");
              MDTChamberTest = FALSE;
              fclose(MDTChamberSumFile);
              MDTChamberSumFile = NULL;
            }
            else {
              MDTChamberOutput1(">>>>>>>>>>  End CSM Tests <<<<<<<<<<\n\n");
            }
            autoRunFromControlFile = 0;
            willDoThresholdScan = FALSE;
            return;
          }
          if (checkCSMIO) {
            printf("Start CSM I/O tests\n");
            if (!initDAQDone) InitDAQ();
            GetCSMStatus();
            UpCSMIOsControl();
            if (CSMTestStatus != 0) {
              MDTChamberOutput1("CSM I/O error\n");
            }
            else {
              CSMIOsSelectHardwareTDO();
              if ((gotPROM && gotCSMChip) || gotAX1000) CSMTestStatus = 0;
              else {
                CSMTestStatus = 1;
                MDTChamberOutput1("Did not get expected JTAG device with HardwareTDO\n");
              }
              if (gotGOL) {
                CSMTestStatus = 2;
                MDTChamberOutput1("Got unexpected JTAG device (GOL) with HardwareTDO\n");
              }
              if (gotTTCrx) {
                CSMTestStatus = 3;
                MDTChamberOutput1("Got unexpected JTAG device (TTCrx) with HardwareTDO\n");
              }
              if (gotCSM) {
                CSMTestStatus = 4;
                MDTChamberOutput1("Got unexpected JTAG device (Virtual CSM) with HardwareTDO\n");
              }
              if (gotAMT) {
                CSMTestStatus = 5;
                MDTChamberOutput1("Got unexpected JTAG device (AMT) with HardwareTDO\n");
              }
              if (gotUnknown) {
                CSMTestStatus = 6;
                MDTChamberOutput1("Got unexpected JTAG device (Unknown) with HardwareTDO\n");
              }
            }
            if (CSMTestStatus == 0) {
              CSMIOsSelectSoftwareTDO();
              if ((gotPROM && gotCSMChip) || gotAX1000) CSMTestStatus = 0;
              else {
                CSMTestStatus = 1;
                MDTChamberOutput1("Did not get expected JTAG device with HardwareTDO\n");
              }
              if (!gotGOL) {
                CSMTestStatus = 2;
                MDTChamberOutput1("Did not get expected JTAG device (GOL) with HardwareTDO\n");
              }
              if (!gotTTCrx) {
                CSMTestStatus = 3;
                MDTChamberOutput1("Did not get expected JTAG device (TTCrx) with HardwareTDO\n");
              }
              if (!gotCSM) {
                CSMTestStatus = 4;
                MDTChamberOutput1("Did not get expected JTAG device (Virtual CSM) with HardwareTDO\n");
              }
              if (!gotAMT && (nbMezzCard > 0)) {
                CSMTestStatus = 5;
                MDTChamberOutput1("Did not get expected JTAG device (AMT) with HardwareTDO\n");
              }
              if (gotUnknown) {
                CSMTestStatus = 6;
                MDTChamberOutput1("Got unexpected JTAG device (Unknown) with HardwareTDO\n");
              }
            }
            if (CSMTestStatus == 0) {
              CSMIOsResetCSMBoard();
              if ((gotPROM && gotCSMChip) || gotAX1000) CSMTestStatus = 0;
              else {
                CSMTestStatus = 1;
                MDTChamberOutput1("Did not get expected JTAG device with HardwareTDO\n");
              }
              if (!gotGOL) {
                CSMTestStatus = 2;
                MDTChamberOutput1("Did not get expected JTAG device (GOL) with HardwareTDO\n");
              }
              if (!gotTTCrx) {
                CSMTestStatus = 3;
                MDTChamberOutput1("Did not get expected JTAG device (TTCrx) with HardwareTDO\n");
              }
              if (!gotCSM) {
                CSMTestStatus = 4;
                MDTChamberOutput1("Did not get expected JTAG device (Virtual CSM) with HardwareTDO\n");
              }
              if (gotAMT) {
                CSMTestStatus = 5;
                MDTChamberOutput1("Got unexpected JTAG device (AMT) with HardwareTDO\n");
              }
              if (gotUnknown) {
                CSMTestStatus = 6;
                MDTChamberOutput1("Got unexpected JTAG device (Unknown) with HardwareTDO\n");
              }
            }
            if (CSMTestStatus == 0) {
              if (nbMezzCard > 0) {
                SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, 1);
                JTAGChainMezzCards();
              }
              CSMIOsProgramFPGAFromPROM();
              if ((gotPROM && gotCSMChip) || gotAX1000) CSMTestStatus = 0;
              else {
                CSMTestStatus = 1;
                MDTChamberOutput1("Did not get expected JTAG device with HardwareTDO\n");
              }
              if (!gotGOL) {
                CSMTestStatus = 2;
                MDTChamberOutput1("Did not get expected JTAG device (GOL) with HardwareTDO\n");
              }
              if (!gotTTCrx) {
                CSMTestStatus = 3;
                MDTChamberOutput1("Did not get expected JTAG device (TTCrx) with HardwareTDO\n");
              }
              if (!gotCSM) {
                CSMTestStatus = 4;
                MDTChamberOutput1("Did not get expected JTAG device (Virtual CSM) with HardwareTDO\n");
              }
              if (gotAMT) {
                CSMTestStatus = 5;
                MDTChamberOutput1("Got unexpected JTAG device (AMT) with HardwareTDO\n");
              }
              if (gotUnknown) {
                CSMTestStatus = 6;
                MDTChamberOutput1("Got unexpected JTAG device (Unknown) with HardwareTDO\n");
              }
            }
            val = CSMTestStatus;
            ResetCSM();
            InitDAQ();
            CSMIOsDone();
            CSMTestStatus = val;
            printf("End CSM I/O tests\n");
          }
          if (CSMTestStatus != 0) {
            MDTChamberOutput1("CSM I/O Tests are failed.\n");
            if (MDTChamberSumFile != NULL) {
              MDTChamberOutput1(">>>>>>>>>>  End MDT Chamber Tests <<<<<<<<<<\n\n");
              MDTChamberTest = FALSE;
              fclose(MDTChamberSumFile);
              MDTChamberSumFile = NULL;
            }
            else {
              MDTChamberOutput1(">>>>>>>>>>  End CSM Tests <<<<<<<<<<\n\n");
            }
            autoRunFromControlFile = 0;
            willDoThresholdScan = FALSE;
            return;
          }
        }
        if (checkAllDAQHardware || MDTChamberTest) CheckAllDAQHardware();
        if (allDAQHardwareStatus != 0) {
          MDTChamberOutput2("Total Number of Download CSM        = %30d\n", numberCSMDownload);
          MDTChamberOutput2("Total Number of Failed Download CSM = %30d\n", numberBadCSMDownload);
          MDTChamberOutput2("Total Number of Download MezzCards  = %30d\n", numberMezzDownload);
          MDTChamberOutput2("Total Number of Failed Download Mezz= %30d\n", numberBadMezzDownload);
          MDTChamberOutput2("Total Number of Init DAQ            = %30d\n", numberInitDAQ);
          MDTChamberOutput2("Total Number of Failed Init DAQ     = %30d\n", numberBadInitDAQ);
          MDTChamberOutput2("Total Number of Redone Init DAQ     = %30d\n", numberReInitDAQ);
          MDTChamberOutput2("Total Number of Auto Runs           = %30d\n", numberAutoRun);
          MDTChamberOutput2("Total Number of Bad Auto Runs       = %30d\n", numberBadAutoRun);
          MDTChamberOutput2("Total Number of Rerun in Auto Runs  = %30d\n", numberAutoRerun);
          if (MDTChamberSumFile != NULL) {
            MDTChamberOutput1(">>>>>>>>>>  End MDT Chamber Tests <<<<<<<<<<\n\n");
            MDTChamberTest = FALSE;
            fclose(MDTChamberSumFile);
            MDTChamberSumFile = NULL;
          }
          autoRunFromControlFile = 0;
        }
        else {
          if (MDTChamberTest) {
            newRunNumber = TRUE;
            if (MDTChamberHVStatus == MDTCHAMBERHVON) {
              val = runNumber%1000;
              runNumber += (1500 -val);
            }
            else {
              val = runNumber%1000;
              runNumber += (1000 -val);
            }
            SaveParameters(PARARUNNUMBER);
          }
          numberAutoRun = 0;
          numberBadAutoRun = 0;
          numberAutoRerun = 0;
          reqRerun = FALSE;
          numberRerun = 0;
          autoRunFromControlFile = 1;
          AutoRunSetupAllOff();
          GetAutoRunControls();
          if (autoStartRunControl != 0) {
			autoRunOn = TRUE;
            totalRun = 0;
            totalEvent = 0;
            StartDAQ();
          }
        }
      }
    }
    willDoThresholdScan = FALSE;
  }
  else if (controlOption == HELP)
    MessagePopup("Help on Auto Run Setup Panel Buttons", 
                 "Select automatic run control file.");
  controlOption = NORM;
}


int GetNewRunSetups(int downloadSetups)
{
  static unsigned int myNumberEvent = 0, DAQTimeInSeconds = 0, trigLimit = 14,
                      evtPFactorCtrl, DAQInforCtrl, setControlParaOnly;
  static int validProfile, fileOpened = 0, stopAtEventOn, stopAtTimeOn, preFlag = 0;
  static char str[256];
  static FILE *ctrlFile;
  char profileName[256], str1[256], cstr[40], *strp;
  int runType, TDCEnableMask, val, version, isINT;
  int i, j, readString, strl, gotRunPara[100], paraIndex, gotTheRun, firstStop, initMezz;
  int mezz, matchWindow, AMTChannel, ASDChannel, ASDThreshold, ASD1Threshold, ASD2Threshold,
      ASD3Threshold, mezzCopyMask, AMTChannelCopyMask, ASDChannelCopyMask, thresholdCopyMask,
      ASDHysteresis, ASD1Hysteresis, ASD2Hysteresis, ASD3Hysteresis, ASDCCap, ASD1CCap,
      ASD2CCap, ASD3CCap, ASDDeadtime, ASD1Deadtime, ASD2Deadtime, ASD3Deadtime, ASDChipMode,
      ASDWThreshold, ASDWGate, ASDWCurrent, ASDChannelMode[3], wantSetupASD, newProfile;
  extern unsigned int stopRunAtEvent, restartRunAfterSec;
  extern int AutoRunSetupHandle;

  if (downloadSetups) useExternalTrigger = FALSE;
  readString = 0;
  strcpy(profileName, "");
  initMezz = FALSE;
  newProfile = FALSE;
  matchWindow = -99999;
  AMTChannel = -99999;
  ASDChannel = -99999;
  for (i = 0; i < 3; i++) ASDChannelMode[i] = -99999;
  ASDThreshold = -99999;
  ASD1Threshold = -99999;
  ASD2Threshold = -99999;
  ASD3Threshold = -99999;
  ASDHysteresis = -99999;
  ASD1Hysteresis = -99999;
  ASD2Hysteresis = -99999;
  ASD3Hysteresis = -99999;
  ASDCCap = -99999;
  ASD1CCap = -99999;
  ASD2CCap = -99999;
  ASD3CCap = -99999;
  ASDDeadtime = -99999;
  ASD1Deadtime = -99999;
  ASD2Deadtime = -99999;
  ASD3Deadtime = -99999;
  ASDChipMode = -99999;
  ASDWThreshold = -99999;
  ASDWGate = -99999;
  ASDWCurrent = -99999;
  wantSetupASD = FALSE;
  runType = -1;
  mezzCopyMask = 0;
  AMTChannelCopyMask = 0;
  ASDChannelCopyMask = 0;
  thresholdCopyMask = 0;
  if (fileOpened && (preFlag != downloadSetups)) {
    fclose(ctrlFile);
    fileOpened = 0;
  }
  preFlag = downloadSetups;
  if ((autoRunFromControlFile == 1) && (fileOpened == 0)) {
    setControlParaOnly = FALSE;
    validProfile = 1;
    // Set default Values
    evtPFactorCtrl = prescaleControl;
    GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, &storageCtrl);
    GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DAQINFOR, &DAQInforCtrl);
    GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, &stopAtEventOn);
    GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, &stopAtTimeOn);
    if (stopAtEventOn) myNumberEvent = stopRunAtEvent;
    if (stopAtTimeOn) DAQTimeInSeconds = restartRunAfterSec;
    if (!stopAtEventOn && !stopAtTimeOn) {
      DAQTimeInSeconds = 3600;
      stopAtTimeOn = 1;
    }
    if (ctrlFile = fopen(controlFilePath, "r")) {
      fileOpened = 1;
      readString = 1;
    }
    else {
      printf("Unable to open automatic run control file <%s>\n", controlFilePath);
      if (ctrlFile != NULL) {
        fclose(ctrlFile);
        fileOpened = 0;
      }
      autoRunFromControlFile = 0;
      return FALSE;
    }
  }
  if (fileOpened == 1) {
    firstStop = TRUE;
    for (i = 0; i < 100; i++) gotRunPara[i] = 0;
    gotTheRun = 0;
    strcpy(profileName, "");
    if (reqThresholdScan && (thresholdScanMethod > 0) && (subScans > 0)) {
      subScans--;
      gotTheRun = 1;
    }
    while (TRUE && (gotTheRun == 0)) {
      if (readString == 1) {
        while (TRUE) {
          if ((str[0] = fgetc(ctrlFile)) == EOF) {
            autoRunFromControlFile = 0;
            fclose(ctrlFile);
            fileOpened = 0;
            for (i = 0; i < 100; i++) if (gotRunPara[i] == 1) gotTheRun = 1;
            if (gotTheRun == 0) validProfile = 0;
            break;
          }
          else if ((str[0] != ' ') && (str[0] != '\t') && (str[0] != '\n')) {
            fgets(&str[1], 255, ctrlFile);
            break;
          }
        }
      }
      if (fileOpened == 0) break;
      
      if (((str[0] == '/') && (str[1] == '/')) ||
          ((str[0] == '/') && (str[1] == '*')) || (str[0] == '\0') ||
           (str[0] == '*') || (str[0] == '\n'));   // Ignore comments and new line
      else if (str[0] != EOF) {
        for (i = 0; i <= strlen(str); i++) str1[i] = tolower(str[i]);
        // Global Command
        strcpy(cstr, "check all daq hardware");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "check daq hardware");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "check all hardware");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "check hardware");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &checkAllDAQHardware);
          if (checkAllDAQHardware < 0) checkAllDAQHardware = 0;
          else checkAllDAQHardware = 1;
        }

        strcpy(cstr, "mdt chamber test");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "chamber test");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &MDTChamberTest);
          if (MDTChamberTest <= 0) MDTChamberTest = 0;
          else MDTChamberTest = 1;
        }

        strcpy(cstr, "mdt chamber hv status");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "chamber hv status");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "hv status");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &MDTChamberHVStatus);
          if (MDTChamberHVStatus <= 0) MDTChamberHVStatus = 0;
          else if (MDTChamberHVStatus > 3) MDTChamberHVStatus = MDTCHAMBERHVUNKNOWN;
          SetCtrlVal(RunConditionHandle, P_RUNCOND_CHAMBERHV, MDTChamberHVStatus);
        }

        strcpy(cstr, "scan serial number");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "scan serialnumber");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "serial number");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "serialnumber");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &scanSerialNumber);
          if (scanSerialNumber <= 0) scanSerialNumber = 0;
          else scanSerialNumber = 1;
        }

        strcpy(cstr, "check csm");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "test csm");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &checkCSM);
          if (checkCSM <= 0) checkCSM = 0;
          else if (checkCSM > 3) checkCSM = 3;
          checkCSMJTAG = checkCSM & 1;
          if (JTAGDRIVERTYPE == CANELMB) checkCSMIO = checkCSM & 2;
          checkCSM &= 1;
        }

        // Local Command (Type A)
        paraIndex = 0;
		// If MDT chamber test, ignore command to set run number, where the run number will
		// be managed by program itself.
        if (!MDTChamberTest) {
          strcpy(cstr, "run number");
          if (strstr(str1, cstr) == NULL) strcpy(cstr, "runnumber");
          strp = strstr(str1, cstr);
          if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
            gotRunPara[paraIndex] = 1;
            strl = strlen(cstr);
            for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
            j = 0;
            while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
            for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
            sscanf(str, "%u", &runNumber);
            if (downloadSetups) SaveParameters(PARARUNNUMBER);
          }
          else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        }

        paraIndex++;
        strcpy(cstr, "run type");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "runtype");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &runType);
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "number of event");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "number event");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%u", &myNumberEvent);
          stopAtEventOn = 1;
          if (firstStop) {
            firstStop = FALSE;
            stopAtTimeOn = 0;
          }
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "daq time (s)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "daq time(s)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "daq time in seconds");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "daq time");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%u", &DAQTimeInSeconds);
          stopAtTimeOn = 1;
          if (firstStop) {
            firstStop = FALSE;
            stopAtEventOn = 0;
          }
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "trigger limit");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%u", &trigLimit);
          if (trigLimit > 14) trigLimit = 14;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "event prescaled factor");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "prescaled factor");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%u", &evtPFactorCtrl);
          if (evtPFactorCtrl <= 50) evtPFactorCtrl = 0;          // Prescaled Factor 1      
          else if (evtPFactorCtrl <= 500) evtPFactorCtrl = 1;    // Prescaled Factor 101
          else if (evtPFactorCtrl <= 5000) evtPFactorCtrl = 2;   // Prescaled Factor 1009
          else evtPFactorCtrl = 3;                               // Prescaled Factor 10007
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "storage selection");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "storage");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%u", &storageCtrl);
          if (storageCtrl <= 0) storageCtrl = 0;
          else if (storageCtrl > 1) storageCtrl = 1;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "daq information");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "daq infor");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%u", &DAQInforCtrl);
          if (DAQInforCtrl > 0) DAQInforCtrl = 1;
          else DAQInforCtrl = 0;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "profile name");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "profile");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%s", profileName);
          // Check if profile exists
          validProfile = GetFileInfo(profileName, &i);
          if (!downloadSetups && !validProfile) {
            sprintf(str, "Profile <%s> does not exist!\n"
                         "Please check Run Control File to make sure it is OK!\n"
                         "Otherwise corresponding runs using invalid profile will be skipped.",
                         profileName);
            MessagePopup("Invalid Profile", str);
            printf("%s\n", str);
          }
          else if (downloadSetups && validProfile) {
            newProfile = TRUE;
            RecallAllPanelState(profileName);
            LoadCSMSetupArray();
            GetMezzCardEnables();
            JTAGControlInitialization();
          }
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "tdc enable mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "tdc enable mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "tdc enable mask");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%x", &TDCEnableMask);
          mezzEnables = TDCEnableMask;
          SaveParameters(PARAMEZZENABLES);
          SetMezzCardEnables();
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "matching window");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "match window");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &matchWindow);
          if (matchWindow <= 0) matchWindow = 0;
          else if (matchWindow > 4095) matchWindow = 4095;
          initMezz = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "amt channel enable (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel enable(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel enable");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel control (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel control(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel control");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel mask");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%x", &AMTChannel);
          initMezz = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd channel enable (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel enable(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel enable");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel control (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel control(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel control");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel mask");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%x", &ASDChannel);
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd threshold (mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd threshold(mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd threshold");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asds threshold (mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asds threshold(mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asds threshold");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD1Threshold);
          if (ASD1Threshold < -254) ASD1Threshold = -254;
          else if (ASD1Threshold > 256) ASD1Threshold = 256;
          j = 0;
          while(isspace(str[j]) == 0) j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          j = 0;
          while(isspace(str[j]) != 0) j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          if (strlen(str) > 0) {
            isINT = FALSE;
            for (i = 0; i < strlen(str); i++) {
              if (isdigit(str[i]) != 0) isINT = TRUE;
              else if (isspace(str[i]) != 0) break;
              else if (str[i] != '+' && str[i] != '-') {
                isINT = FALSE;
                break;
              }
            }
            if (isINT) {
              sscanf(str, "%d", &ASD2Threshold);
              if (ASD2Threshold < -254) ASD2Threshold = -254;
              else if (ASD2Threshold > 256) ASD2Threshold = 256;
            }
            else {
              ASD2Threshold = ASD1Threshold;
              ASD3Threshold = ASD1Threshold;
            }
            j = 0;
            while(isspace(str[j]) == 0) j++; 
            for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
            j = 0;
            while(isspace(str[j]) != 0) j++; 
            for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
            if (strlen(str) > 0) {
              isINT = FALSE;
              for (i = 0; i < strlen(str); i++) {
                if (isdigit(str[i]) != 0) isINT = TRUE;
                else if (isspace(str[i]) != 0) break;
                else if (str[i] != '+' && str[i] != '-') {
                  isINT = FALSE;
                  break;
                }
              }
              if (isINT) {
                sscanf(str, "%d", &ASD3Threshold);
                if (ASD3Threshold < -254) ASD3Threshold = -254;
                else if (ASD3Threshold > 256) ASD3Threshold = 256;
              }
              else {
                ASD3Threshold = ASD2Threshold;
              }
            }
            else {
              ASD3Threshold = ASD2Threshold;
            }
          }
          else {
            ASD2Threshold = ASD1Threshold;
            ASD3Threshold = ASD1Threshold;
          }
          wantSetupASD = TRUE;
          if (thresholdScanMethod == 2) numberSubScans = 24;
          else if (thresholdScanMethod == 1) numberSubScans = 3;
          else numberSubScans = 1;
          subScans = numberSubScans - 1;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd1 threshold (mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd1 threshold(mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd1 threshold");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD1Threshold);
          if (ASD1Threshold < -254) ASD1Threshold = -254;
          else if (ASD1Threshold > 256) ASD1Threshold = 256;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd2 threshold (mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd2 threshold(mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd2 threshold");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD2Threshold);
          if (ASD2Threshold < -254) ASD2Threshold = -254;
          else if (ASD2Threshold > 256) ASD2Threshold = 256;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd3 threshold (mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd3 threshold(mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd3 threshold");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD3Threshold);
          if (ASD3Threshold < -254) ASD3Threshold = -254;
          else if (ASD3Threshold > 256) ASD3Threshold = 256;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd hysteresis");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd hysterisis");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASDHysteresis);
          if (ASDHysteresis < 0) ASDHysteresis = 0;
          else if (ASDHysteresis > 15) ASDHysteresis = 15;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd1 hysteresis");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd1 hysterisis");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD1Hysteresis);
          if (ASD1Hysteresis < 0) ASDHysteresis = 0;
          else if (ASD1Hysteresis > 15) ASD1Hysteresis = 15;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd2 hysteresis");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd2 hysterisis");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD2Hysteresis);
          if (ASD2Hysteresis < 0) ASD2Hysteresis = 0;
          else if (ASD2Hysteresis > 15) ASD2Hysteresis = 15;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd3 hysteresis");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd3 hysterisis");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD3Hysteresis);
          if (ASD3Hysteresis < 0) ASD3Hysteresis = 0;
          else if (ASD3Hysteresis > 15) ASD3Hysteresis = 15;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd calibration capacitor");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd calibrationcapacitor");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASDCCap);
          if (ASDCCap < 0) ASDCCap = 0;
          else if (ASDCCap > 7) ASDCCap = 7;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd1 calibration capacitor");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd1 calibrationcapacitor");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD1CCap);
          if (ASD1CCap < 0) ASD1CCap = 0;
          else if (ASD1CCap > 7) ASD1CCap = 7;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd2 calibration capacitor");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd2 calibrationcapacitor");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD2CCap);
          if (ASD2CCap < 0) ASD2CCap = 0;
          else if (ASD2CCap > 7) ASD2CCap = 7;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd3 calibration capacitor");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd3 calibrationcapacitor");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD3CCap);
          if (ASD3CCap < 0) ASD3CCap = 0;
          else if (ASD3CCap > 7) ASD3CCap = 7;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd deadtime");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd dead-time");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd dead time");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASDDeadtime);
          if (ASDDeadtime < 0) ASDDeadtime = 0;
          else if (ASDDeadtime > 7) ASDDeadtime = 7;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd1 deadtime");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd1 dead-time");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd1 dead time");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD1Deadtime);
          if (ASD1Deadtime < 0) ASD1Deadtime = 0;
          else if (ASD1Deadtime > 7) ASD1Deadtime = 7;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd2 deadtime");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd2 dead-time");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd2 dead time");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD2Deadtime);
          if (ASD2Deadtime < 0) ASD2Deadtime = 0;
          else if (ASD2Deadtime > 7) ASD2Deadtime = 7;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd3 deadtime");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd3 dead-time");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd3 dead time");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASD3Deadtime);
          if (ASD3Deadtime < 0) ASD3Deadtime = 0;
          else if (ASD3Deadtime > 7) ASD3Deadtime = 7;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd chip mode");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd chipmode");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd mode");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASDChipMode);
          if (ASDChipMode < 0) ASDChipMode = 0;
          else if (ASDChipMode > 1) ASDChipMode = 1;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd wilkinson adc threshold");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. adc threshold");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson threshold");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. threshold");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASDWThreshold);
          if (ASDWThreshold < 0) ASDWThreshold = 0;
          else if (ASDWThreshold > 7) ASDWThreshold = 7;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd wilkinson integrationgate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. integrationgate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson adc integrationgate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. adc integrationgate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson integration gate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. integration gate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson adc integration gate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. adc integration gate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson adc gate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. adc gate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson gate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. gate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd integration gate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd integrationgate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd gate");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASDWGate);
          if (ASDWGate < 0) ASDWGate = 0;
          else if (ASDWGate > 15) ASDWGate = 15;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "asd wilkinson rundown current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. rundown current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson adc rundown current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. adc rundown current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson adc rundowncurrent");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. adc rundowncurrent");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson rundowncurrent");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. rundowncurrent");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson adc current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. adc current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd rundown current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd rundowncurrent");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd discharge current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd dischargecurrent");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson adc discharge current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. adc discharge current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wilkinson discharge current");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd wil. discharge current");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &ASDWCurrent);
          if (ASDWCurrent < 0) ASDWCurrent = 0;
          else if (ASDWCurrent > 15) ASDWCurrent = 15;
          wantSetupASD = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "mezz. setup copy mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz. setup copy mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz. setup copy mask");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz. card setup copy mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz. card setup copy mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz. card setup copy mask");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz.card setup copy mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz.card setup copy mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz.card setup copy mask");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz setup copy mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz setup copy mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz setup copy mask");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezzcard setup copy mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezzcard setup copy mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezzcard setup copy mask");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezzanine setup copy mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezzanine setup copy mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezzanine setup copy mask");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%x", &mezzCopyMask);
          if (mezzCopyMask != 0) initMezz = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "amt channel copy mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel copy mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel copy mask");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%x", &AMTChannelCopyMask);
          if (AMTChannelCopyMask != 0) initMezz = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "asd channel copy mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel copy mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel copy mask");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%x", &ASDChannelCopyMask);
          if (ASDChannelCopyMask != 0) initMezz = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "threshold copy mask (hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "threshold copy mask(hex)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "threshold copy mask");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%x", &thresholdCopyMask);
          if (thresholdCopyMask != 0) initMezz = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

        paraIndex++;
        strcpy(cstr, "ttcvi trigger rate (hz)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "ttcvi trigger rate(hz)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "ttcvi trigger rate");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "trigger rate (hz)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "trigger rate(hz)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "trigger rate");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            triggerRate = val;
            if (triggerRate <= 0) triggerRate = 0;
            if (!gotTTCvi) triggerRate = -1;
            else triggerSelect = 5;
          }
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "ttcvi trigger selection");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "ttcvi trigger select");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "trigger selection");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "trigger select");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            triggerSelect = val;
            if (triggerSelect < 0) triggerSelect = -1;
            if (!gotTTCvi) triggerSelect = -1;
          }
          if (reqThresholdScan && (val < 4)) thresholdScanTrigWarning = TRUE;
          if ((val >= 0) && (val < 4)) useExternalTrigger = TRUE;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "calibration trigger delay");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "calib. trigger delay");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "calibration delay");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "calib. delay");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &calibrationTriggerDelay);
          if (calibrationTriggerDelay < 0) calibrationTriggerDelay = -1;
          if (!gotTTCvi) calibrationTriggerDelay = -1;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
        paraIndex++;
        strcpy(cstr, "amt command delay");
        strp = strstr(str1, cstr);
        if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
          gotRunPara[paraIndex] = 1;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &AMTCommandDelay);
          if (AMTCommandDelay < 0) AMTCommandDelay = -1;
        }
        else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;
        
		// Local Command (Type B)
        strcpy(cstr, "data analysis control");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "data analysis");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "dataanalysis");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if ((val <= 0) || (val > 2)) val = 0;
          else reqProcessData = TRUE;
          if (downloadSetups) {
            analysisControl = val;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_ANALYSIS, analysisControl);
          }
        }

        strcpy(cstr, "warning control on scan test");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "control on scan test");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            if (val <= 0) scanControl = 0;
            else scanControl = 1;
            SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_SCANCONTROL, scanControl);
          }
        }

        strcpy(cstr, "auto run init daq control");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            if (val <= 0) initDAQControl = 0;
            else initDAQControl = 1;
            SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_INITDAQCONTROL, initDAQControl);
          }
        }

        strcpy(cstr, "mezzanine card scan output format");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezzanine cards scan output format");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz. card scan output format");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz. cards scan output format");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz.card scan output format");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mezz.cards scan output format");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) scanOutputFormat = val;
        }

        strcpy(cstr, "disable individual settings");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "disable individual setting");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            if (val > 0) {
              DisableAllIndividualMezzSettings();
              initMezz = TRUE;
            }
          }
        }

        strcpy(cstr, "suppress csm header");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "suppress csmheader");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            if (val <= 0) suppressCSMHeader = FALSE;
            else suppressCSMHeader = TRUE;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_CSMHEADER, suppressCSMHeader);
          }
        }

        strcpy(cstr, "suppress csm trailer");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "suppress csmtrailer");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            if (val <= 0) suppressCSMTrailer = FALSE;
            else suppressCSMTrailer = TRUE;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_CSMTRAILER, suppressCSMTrailer);
          }
        }

        strcpy(cstr, "suppress tdc header");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "suppress tdcheader");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            if (val <= 0) suppressTDCHeader = FALSE;
            else suppressTDCHeader = TRUE;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_TDCHEADER, suppressTDCHeader);
          }
        }

        strcpy(cstr, "suppress tdc trailer");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "suppress tdctrailer");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            if (val <= 0) suppressTDCTrailer = FALSE;
            else suppressTDCTrailer = TRUE;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_TDCTRAILER, suppressTDCTrailer);
          }
        }

        strcpy(cstr, "trigger deadtime (%)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "trigger dead time (%)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "trigger deadtime");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "trigger dead time");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            if (val <= 0) triggerDeadtime = 0;
            else if (val >= 99) triggerDeadtime = 99;
            else triggerDeadtime = val;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_TRIGGERDEADTIME, triggerDeadtime);
          }
        }

        strcpy(cstr, "check system noise level");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "check system noise");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "system noise level");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "system noise");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            if (val <= 0) checkSystemNoise = FALSE;
            else checkSystemNoise = TRUE;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_CHECKSYSTEMNOISE, checkSystemNoise);
          }
        }

        strcpy(cstr, "mapping mdt chamber");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "map mdt chamber");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mapping chamber");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "map chamber");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (val > 0) scanSerialNumber = TRUE;
          if (downloadSetups) {
            if (MDTChamberType == CTYPEUNKNOWN) mappingMDTChamber = FALSE;
            else if (val <= 0) mappingMDTChamber = FALSE;
            else mappingMDTChamber = TRUE;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, mappingMDTChamber);
            SetMDTChamberHVStatus();
          }
        }

        strcpy(cstr, "effective threshold (mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "effective threshold(mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "effective nominal threshold (mv)");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "effective nominal threshold(mv)");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          needLoadNominalThreshold = TRUE;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups) {
            if (val < -256) val = -256;
            else if (val > 256) val = 256;
            nominalThreshold = val;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_NTHRESHOLD, nominalThreshold);
            SaveParameters(PARANOMINALTHRESHOLD);
            LoadNominalASDMainThreshold();
            initMezz = TRUE;
          }
        }

        strcpy(cstr, "load nominal threshold");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "nominal threshold");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          needLoadNominalThreshold = TRUE;
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (downloadSetups && (val > 0)) {
            LoadNominalASDMainThreshold();
            initMezz = TRUE;
          }
        }

        strcpy(cstr, "threshold scan method");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (val < 0) val = 0;
          else if (val > 2) val = 2;
          if ((numberScanRun <= 0) || (!reqThresholdScan)) thresholdScanMethod = val;
        }

        strcpy(cstr, "start threshold scan");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "start threshold");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          numberScanRun = 0;
          numberIgnoredScanRun = 0;
          numberBadScanRun = 0;
          willDoThresholdScan = TRUE;
          reqThresholdScan = TRUE;
          needLoadNominalThreshold = TRUE;
          thresholdScanPaired = FALSE;
          if (reqInjectionScan || reqLinearityScan || reqGainScan) differentScanMixed = TRUE;
          if (downloadSetups) {
            runType = NORMALALLCALIBCHANNELOFF;
            MDTChamberOutput2("Threshold scan test start on %s\n", WeekdayTimeString(time(NULL)));
            thresholdScanStartTime = (int) time(NULL);
            checkSystemNoise = FALSE;
            mappingMDTChamber = FALSE;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_CHECKSYSTEMNOISE, checkSystemNoise);
            SetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, mappingMDTChamber);
            DisableAllIndividualMezzSettings();
            if (ASDHysteresis == -99999) ASDHysteresis = 0;
            if (ASDCCap == -99999) ASDCCap = 0;
            if (AMTChannel == -99999) AMTChannel = 0xFFFFFF;
            for (i = 0; i < 3; i++) ASDChannelMode[i] = 0;
            initMezz = TRUE;
            wantSetupASD = TRUE;
            if (MDTChamberTest && (!newRunNumber)) {
              newRunNumber = TRUE;
              val = (runNumber % 100);
              runNumber += (100 -val);
              SaveParameters(PARARUNNUMBER);
            }
            if (gotTTCvi) triggerSelect = 4;   // Default trigger for threshold scan
          }
        }

        strcpy(cstr, "end threshold scan");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "end threshold");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          if (reqThresholdScan) thresholdScanPaired = TRUE;
          else thresholdScanPaired = FALSE;
          reqThresholdScan = FALSE;
          if (downloadSetups) {
            if (ASDHysteresis == -99999) ASDHysteresis = 2;
            if (AMTChannel == -99999) AMTChannel = 0xFFFFFF;
            for (i = 0; i < 3; i++) ASDChannelMode[i] = 0;
            initMezz = TRUE;
            wantSetupASD = TRUE;
            if (FitThresholdScanResults() != 0) {
              autoRunFromControlFile = 0;
              fclose(ctrlFile);
              fileOpened = 0;
              return FALSE;
            }
            if (MDTChamberTest) {
              newRunNumber = TRUE;
              val = (runNumber % 100);
              runNumber += (100 -val);
              SaveParameters(PARARUNNUMBER);
            }
          }
          numberSubScans = 1;
          subScans = numberSubScans - 1;
        }

        strcpy(cstr, "start injection scan");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "start injection");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          numberScanRun = 0;
          numberIgnoredScanRun = 0;
          numberBadScanRun = 0;
          reqInjectionScan = TRUE;
          injectionScanPaired = FALSE;
          if (reqThresholdScan || reqLinearityScan || reqGainScan) differentScanMixed = TRUE;
          if (downloadSetups) {
            runType = CALIBKEEPCALIBCHANNEL;
            MDTChamberOutput2("Injection scan test start on %s\n", WeekdayTimeString(time(NULL)));
            injectionScanStartTime = (int) time(NULL);
            checkSystemNoise = FALSE;
            mappingMDTChamber = FALSE;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_CHECKSYSTEMNOISE, checkSystemNoise);
            SetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, mappingMDTChamber);
            DisableAllIndividualMezzSettings();
            if (ASDThreshold == -99999) ASDThreshold = -50;
            if (ASDHysteresis == -99999) ASDHysteresis = 2;
            if ((ASD1CCap == -99999) || (ASD1CCap < 4)) ASD1CCap = 4;
            if ((ASD2CCap == -99999) || (ASD2CCap < 4)) ASD2CCap = 4;
            if ((ASD3CCap == -99999) || (ASD3CCap < 4)) ASD3CCap = 4;
            initMezz = TRUE;
            wantSetupASD = TRUE;
            if (MDTChamberTest && (!newRunNumber)) {
              newRunNumber = TRUE;
              val = (runNumber % 100);
              runNumber += (100 -val);
              SaveParameters(PARARUNNUMBER);
            }
          }
        }

        strcpy(cstr, "end injection scan");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "end injection");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          if (reqInjectionScan) injectionScanPaired = TRUE;
          else injectionScanPaired = FALSE;
          reqInjectionScan = FALSE;
          if (downloadSetups) {
            if (ASDCCap == -99999) ASDCCap = 0;
            initMezz = TRUE;
            wantSetupASD = TRUE;
            if (FitInjectionScanResults() != 0) {
              autoRunFromControlFile = 0;
              fclose(ctrlFile);
              fileOpened = 0;
              return FALSE;
            }
            if (MDTChamberTest) {
              newRunNumber = TRUE;
              val = (runNumber % 100);
              runNumber += (100 -val);
              SaveParameters(PARARUNNUMBER);
            }
          }
        }

        strcpy(cstr, "start linearity scan");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "start linearity");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          numberScanRun = 0;
          numberIgnoredScanRun = 0;
          numberBadScanRun = 0;
          reqLinearityScan = TRUE;
          linearityScanPaired = FALSE;
          if (reqInjectionScan || reqThresholdScan || reqGainScan) differentScanMixed = TRUE;
          if (downloadSetups) {
            runType = CALIBALLCALIBCHANNELON;
            MDTChamberOutput2("Linearity scan test start on %s\n", WeekdayTimeString(time(NULL)));
            linearityScanStartTime = (int) time(NULL);
            checkSystemNoise = FALSE;
            mappingMDTChamber = FALSE;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_CHECKSYSTEMNOISE, checkSystemNoise);
            SetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, mappingMDTChamber);
            if (ASDThreshold == -99999) ASDThreshold = -50;
            if (ASDHysteresis == -99999) ASDHysteresis = 2;
            if ((ASD1CCap == -99999) || (ASD1CCap < 4)) ASD1CCap = 4;
            if ((ASD2CCap == -99999) || (ASD2CCap < 4)) ASD2CCap = 4;
            if ((ASD3CCap == -99999) || (ASD3CCap < 4)) ASD3CCap = 4;
            initMezz = TRUE;
            wantSetupASD = TRUE;
            if (MDTChamberTest && (!newRunNumber)) {
              newRunNumber = TRUE;
              val = (runNumber % 100);
              runNumber += (100 -val);
              SaveParameters(PARARUNNUMBER);
            }
          }
        }

        strcpy(cstr, "end linearity scan");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "end linearity");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          if (reqLinearityScan) linearityScanPaired = TRUE;
          else linearityScanPaired = FALSE;
          reqLinearityScan = FALSE;
          if (downloadSetups) {
            if (ASDCCap == -99999) ASDCCap = 0;
            initMezz = TRUE;
            wantSetupASD = TRUE;
            if (FitLinearityScanResults() != 0) {
              autoRunFromControlFile = 0;
              fclose(ctrlFile);
              fileOpened = 0;
              return FALSE;
            }
            if (MDTChamberTest) {
              newRunNumber = TRUE;
              val = (runNumber % 100);
              runNumber += (100 -val);
              SaveParameters(PARARUNNUMBER);
            }
          }
        }

        strcpy(cstr, "start gain scan");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "start gain");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          numberScanRun = 0;
          numberIgnoredScanRun = 0;
          numberBadScanRun = 0;
          reqGainScan = TRUE;
          gainScanPaired = FALSE;
          if (reqInjectionScan || reqThresholdScan || reqLinearityScan) differentScanMixed = TRUE;
          if (downloadSetups) {
            runType = CALIBALLCALIBCHANNELON;
            MDTChamberOutput2("Gain scan test start on %s\n", WeekdayTimeString(time(NULL)));
            gainScanStartTime = (int) time(NULL);
            checkSystemNoise = FALSE;
            mappingMDTChamber = FALSE;
            SetCtrlVal(RunConditionHandle, P_RUNCOND_CHECKSYSTEMNOISE, checkSystemNoise);
            SetCtrlVal(RunConditionHandle, P_RUNCOND_MAPPINGMDTCHAMBER, mappingMDTChamber);
            DisableAllIndividualMezzSettings();
            if (ASDThreshold == -99999) ASDThreshold = -50;
            if (ASDHysteresis == -99999) ASDHysteresis = 2;
            ASDChipMode = 0;             // Must be ADC mode for gain test
            initMezz = TRUE;
            wantSetupASD = TRUE;
            if (MDTChamberTest && (!newRunNumber)) {
              newRunNumber = TRUE;
              val = (runNumber % 100);
              runNumber += (100 -val);
              SaveParameters(PARARUNNUMBER);
            }
          }
        }

        strcpy(cstr, "end gain scan");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "end gain");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          if (reqGainScan) gainScanPaired = TRUE;
          else gainScanPaired = FALSE;
          reqGainScan = FALSE;
          if (downloadSetups) {
            if (ASDCCap == -99999) ASDCCap = 0;
            initMezz = TRUE;
            wantSetupASD = TRUE;
            if (FitGainScanResults() != 0) {
              autoRunFromControlFile = 0;
              fclose(ctrlFile);
              fileOpened = 0;
              return FALSE;
            }
            if (MDTChamberTest) {
              newRunNumber = TRUE;
              val = (runNumber % 100);
              runNumber += (100 -val);
              SaveParameters(PARARUNNUMBER);
            }
          }
        }

        strcpy(cstr, "allowed fraction bad cycles");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "allowed fraction badcycles");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "allowed fraction bad cycle");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "allowed fraction badcycles");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "fraction bad cycles");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "fraction badcycles");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "fraction bad cycle");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "fraction badcycle");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%f", &allowedFractionBadCycles);
          if (downloadSetups) {
            if (allowedFractionBadCycles < 0.0) allowedFractionBadCycles = 0.0;
            else if (allowedFractionBadCycles > 0.1) allowedFractionBadCycles = 0.1;
          }
          else allowedFractionBadCycles = 0.0;
        }

        strcpy(cstr, "mdt chamber control");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mdtchamber control");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "mdtchambercontrol");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "chamber control");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "chambercontrol");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (val > 0) MDTChamberControl = TRUE;
          else MDTChamberControl = FALSE;
          if (downloadSetups && MDTChamberControl) {
            EnableMezzCardsAccordingMDTChamber();
          }
        }

        strcpy(cstr, "set control parameter only");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "set control parameter");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "set parameter only");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%d", &val);
          if (val > 0) setControlParaOnly = TRUE;
          else setControlParaOnly = FALSE;
        }

		// End of Run List command
        strcpy(cstr, "end of run list");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "end run list");
        if (strstr(str1, cstr) == NULL) strcpy(cstr, "end of run");
        strp = strstr(str1, cstr);
        if (strp != NULL) {
          strcpy(str, "");     // Command has been processed, reset string
          gotTheRun = 1;
        }
      }
      if (gotTheRun == 1) {
        readString = 0;
        break;
      }
      else readString = 1;
    }
  }
  else return FALSE;
  if (gotTheRun <= 0) return FALSE;
  
  if (reqThresholdScan && (thresholdScanMethod > 0) && (subScans >= 0)) {
    if (thresholdScanMethod == 2) {
      initMezz = TRUE;
      wantSetupASD = TRUE;
      AMTChannel = 1 << (23 - subScans);
      for (i = 0; i < 3; i++) ASDChannelMode[i] = 0x5555;
      j = (23 - subScans) / 8;
      i = 2 * ((23 - subScans) - 8*j);
      ASDChannelMode[(23-subScans)/8] &= ~(3 << i);
    }
    else if (thresholdScanMethod == 1) {
      initMezz = TRUE;
      wantSetupASD = TRUE;
      AMTChannel = 0xff << (8*(2 - subScans));
      for (i = 0; i < 3; i++) ASDChannelMode[i] = 0x5555;
      ASDChannelMode[2-subScans] = 0;
    }
  }
  if (downloadSetups) {
    if ((runType >= 0) && (runType <= 4)) {
      SetCtrlVal(DAQControlHandle, P_JTAGCTRL_RUNTYPE, runType);
      SetDAQRunType();
    }
    GetCtrlVal(DAQControlHandle, P_JTAGCTRL_RUNTYPE, &runType);
    if ((triggerRate > 0) || (triggerSelect >= 0)) {
      SetTTCviTriggerAndRate(triggerSelect, &triggerRate);
      TTCviSetUnusedTrigger();
    }
    if (AMTCommandDelay >= 0) {
      if (AMTCommandDelay > 127) AMTCommandDelay = 127;
      SetCtrlVal(CSMSetupHandle, P_CSM_AMTCOMMANDDELAY, AMTCommandDelay);
      CSMSetupDone();
    }
    if (calibrationTriggerDelay >= 0) {
      if ((runType == CALIBALLCALIBCHANNELON) || (runType == CALIBKEEPCALIBCHANNEL)) {
        if (calibrationTriggerDelay > 255) calibrationTriggerDelay = 255;
        SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DELAY, calibrationTriggerDelay);
        WriteTTCviBGOControlRegisters();
        printf("calibration trigger delay = %d\n", calibrationTriggerDelay);
      }
    }
  }
  if (validProfile && downloadSetups) {    // Setup DAQ parameters
    prescaleControl = evtPFactorCtrl;
    if (storageCtrl <= 0) SetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, 0);
    else SetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, 1);
    SetCtrlVal(DAQControlHandle, P_JTAGCTRL_DAQINFOR, DAQInforCtrl);
    if (gotRunPara[0] == 1) {
      newRunNumber = TRUE;
      SaveParameters(PARARUNNUMBER);
    }
    if (initMezz || wantSetupASD) {
      if (initDAQControl != 1) {
        BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
        if ((version < 0) || (version > 10)) {
          SetCtrlVal(DAQControlHandle, P_JTAGCTRL_MEZZCARDS, 1);
          SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 1);
          UpdateCSMControlBits();
          GetJTAGChainOnList();
          DownloadCSMSetup();
        }
      }
      mezzCardSetupAll = TRUE;
      RecallMezzanineSetup();
      if (matchWindow != -99999) {
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_MATCH_WINDOW, matchWindow);
        if (matchWindow+25 < 4095) 
          SetCtrlVal(AMTSetupHandle, P_AMTSETUP_SEARCH_WINDOW, matchWindow+25);
        else SetCtrlVal(AMTSetupHandle, P_AMTSETUP_SEARCH_WINDOW, 4095);
      }
      if (AMTChannel != -99999) {
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL0, (AMTChannel>>0)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL1, (AMTChannel>>1)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL2, (AMTChannel>>2)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL3, (AMTChannel>>3)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL4, (AMTChannel>>4)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL5, (AMTChannel>>5)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL6, (AMTChannel>>6)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL7, (AMTChannel>>7)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL8, (AMTChannel>>8)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL9, (AMTChannel>>9)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL10, (AMTChannel>>10)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL11, (AMTChannel>>11)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL12, (AMTChannel>>12)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL13, (AMTChannel>>13)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL14, (AMTChannel>>14)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL15, (AMTChannel>>15)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL16, (AMTChannel>>16)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL17, (AMTChannel>>17)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL18, (AMTChannel>>18)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL19, (AMTChannel>>19)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL20, (AMTChannel>>20)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL21, (AMTChannel>>21)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL22, (AMTChannel>>22)&0x1);
        SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL23, (AMTChannel>>23)&0x1);
      }
      if (ASD1Threshold == -99999) ASD1Threshold = ASDThreshold;
      if (ASD2Threshold == -99999) ASD2Threshold = ASDThreshold;
      if (ASD3Threshold == -99999) ASD3Threshold = ASDThreshold;
      if (ASD1Hysteresis == -99999) ASD1Hysteresis = ASDHysteresis;
      if (ASD2Hysteresis == -99999) ASD2Hysteresis = ASDHysteresis;
      if (ASD3Hysteresis == -99999) ASD3Hysteresis = ASDHysteresis;
      if (ASD1CCap == -99999) ASD1CCap = ASDCCap;
      if (ASD2CCap == -99999) ASD2CCap = ASDCCap;
      if (ASD3CCap == -99999) ASD3CCap = ASDCCap;
      if (ASD1Deadtime == -99999) ASD1Deadtime = ASDDeadtime;
      if (ASD2Deadtime == -99999) ASD2Deadtime = ASDDeadtime;
      if (ASD3Deadtime == -99999) ASD3Deadtime = ASDDeadtime;
      if (wantSetupASD) {
        WaitInMillisecond(10);
        ASD1Control();
        if (ASDChannel != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0, (ASDChannel>>0)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1, (ASDChannel>>1)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2, (ASDChannel>>2)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3, (ASDChannel>>3)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4, (ASDChannel>>4)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5, (ASDChannel>>5)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6, (ASDChannel>>6)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7, (ASDChannel>>7)&0x1);
        }
        if (ASDChannelMode[0] != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, (ASDChannelMode[0]>>0)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1MODE, (ASDChannelMode[0]>>2)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2MODE, (ASDChannelMode[0]>>4)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3MODE, (ASDChannelMode[0]>>6)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4MODE, (ASDChannelMode[0]>>8)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5MODE, (ASDChannelMode[0]>>10)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6MODE, (ASDChannelMode[0]>>12)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7MODE, (ASDChannelMode[0]>>14)&0x1);
        }
        if (ASD1Threshold != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, ASD1Threshold);
        }
        if (ASD1Hysteresis != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_HYSTERESIS, ASD1Hysteresis);
        }
        if (ASD1CCap != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CALIBCAPACITOR, ASD1CCap);
        }
        if (ASD1Deadtime != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_DEADTIME, ASD1Deadtime);
        }
        if (ASDChipMode != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHIPMODE, ASDChipMode);
        }
        if (ASDWThreshold != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_THRESH, ASDWThreshold);
        }
        if (ASDWGate != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_INT, ASDWGate);
        }
        if (ASDWCurrent != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_CUR, ASDWCurrent);
        }
        ASDDone();
        WaitInMillisecond(10);
        ASD2Control();
        if (ASDChannel != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0, (ASDChannel>>8)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1, (ASDChannel>>9)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2, (ASDChannel>>10)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3, (ASDChannel>>11)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4, (ASDChannel>>12)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5, (ASDChannel>>13)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6, (ASDChannel>>14)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7, (ASDChannel>>15)&0x1);
        }
        if (ASDChannelMode[1] != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, (ASDChannelMode[1]>>0)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1MODE, (ASDChannelMode[1]>>2)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2MODE, (ASDChannelMode[1]>>4)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3MODE, (ASDChannelMode[1]>>6)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4MODE, (ASDChannelMode[1]>>8)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5MODE, (ASDChannelMode[1]>>10)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6MODE, (ASDChannelMode[1]>>12)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7MODE, (ASDChannelMode[1]>>14)&0x1);
        }
        if (ASD2Threshold != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, ASD2Threshold);
        }
        if (ASD2Hysteresis != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_HYSTERESIS, ASD2Hysteresis);
        }
        if (ASD2CCap != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CALIBCAPACITOR, ASD2CCap);
        }
        if (ASD2Deadtime != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_DEADTIME, ASD2Deadtime);
        }
        if (ASDChipMode != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHIPMODE, ASDChipMode);
        }
        if (ASDWThreshold != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_THRESH, ASDWThreshold);
        }
        if (ASDWGate != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_INT, ASDWGate);
        }
        if (ASDWCurrent != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_CUR, ASDWCurrent);
        }
        ASDDone();
        WaitInMillisecond(10);
        ASD3Control();
        if (ASDChannel != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0, (ASDChannel>>16)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1, (ASDChannel>>17)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2, (ASDChannel>>18)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3, (ASDChannel>>19)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4, (ASDChannel>>20)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5, (ASDChannel>>21)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6, (ASDChannel>>22)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7, (ASDChannel>>23)&0x1);
        }
        if (ASDChannelMode[2] != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, (ASDChannelMode[2]>>0)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1MODE, (ASDChannelMode[2]>>2)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2MODE, (ASDChannelMode[2]>>4)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3MODE, (ASDChannelMode[2]>>6)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4MODE, (ASDChannelMode[2]>>8)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5MODE, (ASDChannelMode[2]>>10)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6MODE, (ASDChannelMode[2]>>12)&0x1);
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7MODE, (ASDChannelMode[2]>>14)&0x1);
        }
        if (ASD3Threshold != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, ASD3Threshold);
        }
        if (ASD3Hysteresis != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_HYSTERESIS, ASD3Hysteresis);
        }
        if (ASD3CCap != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CALIBCAPACITOR, ASD3CCap);
        }
        if (ASD3Deadtime != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_DEADTIME, ASD3Deadtime);
        }
        if (ASDChipMode != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHIPMODE, ASDChipMode);
        }
        if (ASDWThreshold != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_THRESH, ASDWThreshold);
        }
        if (ASDWGate != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_INT, ASDWGate);
        }
        if (ASDWCurrent != -99999) {
          SetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_CUR, ASDWCurrent);
        }
        ASDDone();
      }
      if (mezzCopyMask != 0) {
        WaitInMillisecond(100);
        CopyMezzSetupControl();
        for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
          if (((mezzCopyMask>>mezz)&0x1) != 0) {
            SetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], TRUE);
            SetCtrlVal(CopyMezzSetupHandle, P_COPYMEZZ_AMTCHANNEL, (AMTChannelCopyMask>>mezz)&0x1);
            SetCtrlVal(CopyMezzSetupHandle, P_COPYMEZZ_ASDCHANNEL, (ASDChannelCopyMask>>mezz)&0x1);
            SetCtrlVal(CopyMezzSetupHandle, P_COPYMEZZ_MAINTHRESHOLD, (thresholdCopyMask>>mezz)&0x1);
            WaitInMillisecond(100);
            DisplayPanel(CopyMezzSetupHandle);
            CopyMezzSetup();
          }
          SetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], FALSE);
        }
        CopyMezzSetupDone();
      }
      WaitInMillisecond(100);
      downloadMezzSetup = FALSE;
      AMTDone();                          // Here save mezzanine card setting
      DownloadAllMezzCardSetup();
    }
    if ((initDAQControl == 1) || initMezz || newProfile) {
      InitDAQ();
      if (mezzCopyMask != 0) InitDAQ();   // Init DAQ again to avoid any potential threshold problem
    }
    if (CSMBoardStatus() != 0) {          // If any problem, perform DAQ initialization
      numberReInitDAQ++;
      InitDAQ();
    }

    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, FALSE);
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, FALSE);
    SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, FALSE);
    if (stopAtEventOn) {
      if (myNumberEvent > 0) {
        SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, TRUE);
        SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPATEVENT, myNumberEvent);
      }
      else SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, FALSE);
    }
    if (stopAtTimeOn) {
      if (DAQTimeInSeconds > 0) {
        SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, TRUE);
        SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNS, 1);
        SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, TRUE);
        SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSEC, DAQTimeInSeconds);
      }
      else {
        SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, FALSE);
        SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, FALSE);
      }
    }
    // printf("runNumber = %u\n", runNumber);
    // printf("number of event = %u\n", myNumberEvent);
    // printf("DAQTimeInSeconds = %u\n", DAQTimeInSeconds);
    // printf("evtPFactorCtrl = %u\n", evtPFactorCtrl);
    // printf("storageCtrl = %u\n", storageCtrl);
    // printf("trigLimit = %u\n", trigLimit);
    // printf("initMezz = %d\n", initMezz);
    // printf("match window = %d\n", matchWindow);
    // printf("AMTChannel = 0x%06x\n", AMTChannel);
    // printf("mezzCopyMask = 0x%06x\n", mezzCopyMask);
    // printf("AMTChannelCopyMask = 0x%06x\n", AMTChannelCopyMask);
    // printf("ASDChannelCopyMask = 0x%06x\n", ASDChannelCopyMask);
    // printf("thresholdCopyMask = 0x%06x\n", thresholdCopyMask);
    AutoRunSetupDone();
  }
  
  if (setControlParaOnly && downloadSetups) return FALSE;
  else return validProfile;
}


void AutoRunControl(int restartRun) {
  processAutoRun = TRUE;
  WaitInMillisecond(100);
  StopDAQ();           // Stop DAQ first           
  WaitInMillisecond(100);
  if ((restartRun > 0) || (autoRunFromControlFile == 1)) {
    autoRunOn = TRUE;
    StartDAQ();        // Start DAQ           
  }
  else {
    numberCSMDownload = 0;
    numberBadCSMDownload = 0;
    numberMezzDownload = 0;
    numberBadMezzDownload = 0;
    numberInitDAQ = 0;
    numberBadInitDAQ = 0;
    numberReInitDAQ = 0;
    numberAutoRun = 0;
    numberBadAutoRun = 0;
    numberAutoRerun = 0;
    reqRerun = FALSE;
    numberRerun = 0;
  }
  return;
}


// Load ASD nominal main threshold from database file to correct V offset
// In case no database file exists, V offset correction will not be applied
void LoadNominalASDMainThreshold(void) {
  int oldMezzCardSetupAll, oldMezzCardNb, mezz, threshold[3][MAXNUMBERMEZZANINE], readStr;
  int exist, stat, i, j, k, strl, ch, enableLNThreshold, IDMezz, NDF, val, thre, type;
  int first[MAXNUMBERMEZZANINE], nchannel, channel0, mezzNotInDB;
  float offset[24], vOff, sigma, chisq, norm, eff, xtalk, vmin, vmax;
  char filename[256], mezzSNumber[30], str[512], *strp, str1[512], cstr[40];
  FILE *threFile, *snFile;
  
  ASDVOffSpanCut = 16.0;
  oldMezzCardSetupAll = mezzCardSetupAll;
  oldMezzCardNb = mezzCardNb;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) snButton[mezz] = -1;
  snButton[0] = P_SNUMBER_MEZZ00SN;
  snButton[1] = P_SNUMBER_MEZZ01SN;
  snButton[2] = P_SNUMBER_MEZZ02SN;
  snButton[3] = P_SNUMBER_MEZZ03SN;
  snButton[4] = P_SNUMBER_MEZZ04SN;
  snButton[5] = P_SNUMBER_MEZZ05SN;
  snButton[6] = P_SNUMBER_MEZZ06SN;
  snButton[7] = P_SNUMBER_MEZZ07SN;
  snButton[8] = P_SNUMBER_MEZZ08SN;
  snButton[9] = P_SNUMBER_MEZZ09SN;
  snButton[10] = P_SNUMBER_MEZZ10SN;
  snButton[11] = P_SNUMBER_MEZZ11SN;
  snButton[12] = P_SNUMBER_MEZZ12SN;
  snButton[13] = P_SNUMBER_MEZZ13SN;
  snButton[14] = P_SNUMBER_MEZZ14SN;
  snButton[15] = P_SNUMBER_MEZZ15SN;
  snButton[16] = P_SNUMBER_MEZZ16SN;
  snButton[17] = P_SNUMBER_MEZZ17SN;
  MDTChamberOutput2("Read Mezzanine Card Database at %s\n", WeekdayTimeString(time(NULL)));
  //
  // First check to see mezzanine card information has been read from database
  //
  if (strlen(MDTChamberName) > 5) strcpy(dataDirName, MDTChamberName);
  else strcpy(dataDirName, "dat");
  strcpy(str, dataDirName);
  strcat(str, "/");
  strcat(str, MDTChamberName);
  strcat(str, "_SN.txt");
  val = 0;
  if (snFile = fopen(str, "r")) {
    while (feof(snFile) == 0) {
      fgets(str, 256, snFile);
      for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
        if ((snButton[mezz] >= 0) && (((mezzEnables>>mezz)&1) == 1)) {
          GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
          sprintf(cstr, "Mezzanine Card %2d Serial Number", mezz);
          strp = strstr(str, cstr);
          if ((strp != NULL) && (strlen(serialNumber) > 0)) {
            strl = strlen(cstr);
            for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
            j = 0;
            while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
            for (i = j; i < strlen(str)+1; i++) {
              if (str[i] == '\n') str[i] = '\0';
              str[i-j] = str[i];
            }
            if ((strlen(serialNumber) > 0) && (strcmp(serialNumber, str) == 0)) {
              if (getVOff[mezz]) val |= (1 << mezz);
            }
          }
        }
      }
    }
    fclose(snFile);
  }
  if (val != mezzEnables) {
    largeVOffSpan = FALSE;
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      threshold[0][mezz] = nominalThreshold;
      threshold[1][mezz] = nominalThreshold;
      threshold[2][mezz] = nominalThreshold;
      getVOff[mezz] = FALSE;
      first[mezz] = TRUE;
    }
    if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
      if (strlen(MDTChamberWarningFilename) > 0) {
        stat = GetFileInfo(MDTChamberWarningFilename, &val);
        if (stat > 0) {
          MDTChamberWarningFile = fopen(MDTChamberWarningFilename, "a");
          if (MDTChamberWarningFile != NULL) {
            fprintf(MDTChamberWarningFile, "\n");
            fprintf(MDTChamberWarningFile, "Checking mezzanine card from mezzanine card database at %s\n", WeekdayTimeString(time(NULL)));
          }
        }
      }
    }
    MDTChamberOutput1("//\n");
    MDTChamberOutput1("// Message will be issued at either of following conditions:\n");
    MDTChamberOutput1("// * <Warning> Unable to find or open mezzanine card database file;\n");
    MDTChamberOutput1("// * <Warning> Unable to find mezzanine card in mezzanine card database;\n");
    sprintf(str, "// * <Warning> Any ASD threshold offset span is larger than %.1f mV.\n", ASDVOffSpanCut);
    MDTChamberOutput2("%s", str);
    MDTChamberOutput1("//\n");
    strcpy(filename, "database/mezzanineCardDatabase.txt");
    exist = GetFileInfo(filename, &stat);
    if (!exist) {
      strcpy(filename, "database/thresholdOffset_");
      GetCtrlVal(SerialNumberHandle, P_SNUMBER_CHAMBERSN, &filename[25]);
      strcat(filename, ".txt");
      exist = GetFileInfo(filename, &stat);
    }
    if (exist) {
      if (threFile = fopen(filename, "r")) {
        gotMezzDatabase = TRUE;
        fgets(str, 512, threFile);
        strcpy(cstr, "");
        strcpy(cstr, "Barcode IdMezzBoard IdChannel");
        if (strncmp(str, cstr, strlen(cstr)) == 0) {
          type = 0;
          readStr = TRUE;
        }
        else {
          type = 1;
          readStr = FALSE;
        }
        while (feof(threFile) == 0) {
          if (readStr) fgets(str, 512, threFile);
          readStr = TRUE;
          j = 0;
          while((str[j] == ' ') || str[j] == '\t') j++; 
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          j = 0;
          k = 0;
          while((str[j] != ' ') && (str[j] != '\t') && (str[j] != ',')) {
            if (str[j] != '"') mezzSNumber[k++] = str[j];
            j++;
          }
          mezzSNumber[k] = '\0';
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
            if (((mezzEnables>>mezz)%2) == 1) {
              GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
              if ((strlen(serialNumber) > 0) && (strcmp(serialNumber, mezzSNumber) == 0)) {
                if (type == 0) {
                  sscanf(str, "%d %d %f %f %f %d %f %f %f", &IDMezz, &i, &vOff, &sigma, &chisq, &NDF, &norm, &eff, &xtalk);
                  // printf("mezz %02d i-channel %02d Offset = %.4f\n", mezz, i, vOff);
                  if (first[mezz]) {
                    for (ch = 0; ch < 24; ch++) offset[ch] = 0;
                    channel0 = i;
                    nchannel = 0;
                    first[mezz] = FALSE;
                  }
                  if ((i-channel0) >= 0) {
                    nchannel++;
                    offset[i-channel0] = vOff;
                    benchThresholdScan[0][i-channel0][mezz] = vOff;
                    benchThresholdScan[1][i-channel0][mezz] = sigma;
                    benchThresholdScan[2][i-channel0][mezz] = norm;
                    benchThresholdScan[3][i-channel0][mezz] = chisq;
                    benchThresholdScan[4][i-channel0][mezz] = NDF;
                    benchInjectionScan[0][i-channel0][mezz] = eff;
                    benchInjectionScan[1][i-channel0][mezz] = xtalk;
                  }
                  if (nchannel == 24) getVOff[mezz] = TRUE;
                }
                else if (type == 1) {
                  for (i = 0; i < strlen(str); i++) {
                    if (str[i] == ',') str[i] = ' ';
                  }
                  sscanf(str, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                         &offset[0], &offset[1], &offset[2], &offset[3], &offset[4], &offset[5], &offset[6], &offset[7],
                         &offset[8], &offset[9], &offset[10], &offset[11], &offset[12], &offset[13], &offset[14], &offset[15],
                         &offset[16], &offset[17], &offset[18], &offset[19], &offset[20], &offset[21], &offset[22], &offset[23]);
                  for (i = 0; i < 24; i++) {
                    // printf("mezz %02d channel %02d Offset = %.4f\n", mezz, i, offset[i]);
                    benchThresholdScan[0][i][mezz] = offset[i];
                    benchThresholdScan[1][i][mezz] = 0.0;
                    benchThresholdScan[2][i][mezz] = 0.0;
                    benchThresholdScan[3][i][mezz] = 0.0;
                    benchThresholdScan[4][i][mezz] = 0.0;
                    benchInjectionScan[0][i][mezz] = 0.0;
                    benchInjectionScan[1][i][mezz] = 0.0;
                  }
                  getVOff[mezz] = TRUE;
                }
              }
            }
          }
        }
        fclose(threFile);
        if (strlen(MDTChamberName) > 5) strcpy(dataDirName, MDTChamberName);
        else strcpy(dataDirName, "dat");
        if (SetDir(dataDirName) == 0) SetDir("..");
        else MakeDir(dataDirName);
        strcpy(str, dataDirName);
        strcat(str, "/");
        strcat(str, MDTChamberName);
        strcat(str, "_SN.txt");
        if (snFile = fopen(str, "w")) {
          fprintf(snFile, "Operator Name                   : %s\n", operatorName);
          fprintf(snFile, "MDT Chamber Serial Number       : %s\n", MDTChamberSN);
          fprintf(snFile, "MDT Chamber Name                : %s\n", MDTChamberName);
          fprintf(snFile, "Motherboard Serial Number       : %s\n", motherboardSN);
          fprintf(snFile, "CSM Serial Number               : %s\n", CSMSN);
          fprintf(snFile, "MDT-DCS Serial Number           : %s\n", MDTDCSSN);
          fprintf(snFile, "MDT-DCS NIKHEF ID               : %s\n", MDTDCSNIKHEFID);
          fprintf(snFile, "MDT-DCS Expected ELMB Node      : %d\n", expectedELMBNode);
          if (JTAGDRIVERTYPE == CANELMB) {
            if (useOnChamberDCS) {
              fprintf(snFile, "MDT-DCS Actual ELMB Node        : %d\n", ELMBNode);
              fprintf(snFile, "MDT-DCS ELMB Version            : %s\n", ELMBVersion);
            }
            else {
              if (CANNode >= 0) fprintf(snFile, "CAN Node                        : %d\n", CANNode);
              if (ELMBNode > 0) {
                fprintf(snFile, "ELMB Node                       : %d\n", ELMBNode);
                fprintf(snFile, "ELMB Version                    : %s\n", ELMBVersion);
              }
            }
          }
          fprintf(snFile, "MDT Chamber Start MezzCard      : %d\n", startMezzCard);
          val = 0;
          for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
            if ((snButton[mezz] >= 0) && (((mezzEnables>>mezz)&1) == 1)) {
              GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
              fprintf(snFile, "Mezzanine Card %2d Serial Number : %s\n", mezz, serialNumber);
              if (getVOff[mezz]) val |= (1 << mezz);
            }
          }
          fprintf(snFile, "Mezzanine Card Found Bits in DB : 0x%08X\n", val);
          fclose(snFile);
        }
      }
      else {
        gotMezzDatabase = FALSE;
        sprintf(str, "(WARNING> Unable to open mezzanine card threshold offset file <%s>.\n", filename);
        MDTChamberOutput2("%s", str);
        sprintf(str, "Default ASD main threshold (%dmV) will be used for all mezzanine cards.\n", nominalThreshold); 
        MDTChamberOutput2("%s", str);
      }
    }
    else {
      strcpy(filename, "mezzanineCardDatabase.txt");
      gotMezzDatabase = FALSE;
      sprintf(str, "<WARNING> No database file <%s> for mezzanine card threshold offset.\n", filename);
      MDTChamberOutput2("%s", str);
      sprintf(str, "Default ASD main threshold (%dmV) will be used for all mezzanine cards.\n", nominalThreshold); 
      MDTChamberOutput2("%s", str);
    }
  }
  mezzNotInDB = FALSE;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    GetCtrlVal(SerialNumberHandle, snButton[mezz], mezzSNumber);
    if (getVOff[mezz]) {
      printf("Found Mezzanine Card %02d (SN = %s) in Database file\n", mezz, mezzSNumber);
      for (i = 0; i < 24; i++) offset[i] = benchThresholdScan[0][i][mezz];
      vmin = 1.0e+31;
      vmax = -1.0e+31;
      for (i = 0; i < 8; i++) {
        if (offset[i] < vmin) vmin = offset[i];
        if (offset[i] > vmax) vmax = offset[i];
      }
      threshold[0][mezz] = nominalThreshold + 0.5*(vmin + vmax);
      if ((thresholdSettingMethod != 0) && ((vmax-vmin) > 12.)) {
        if ((vmax-vmin) < 14.) threshold[0][mezz] -= 2;
        else if ((vmax-vmin) < 16.) threshold[0][mezz] -= 4;
      }
      if ((vmax-vmin) > ASDVOffSpanCut) {
        largeVOffSpan = TRUE;
        sprintf(str, "<WARNING> ASD1 of Mezzanine card %2d <SN=%s> has large V offset span (=%.1f).\n", mezz, mezzSNumber);
        MDTChamberOutput2("%s", str);
      }
      vmin = 1.0e+31;
      vmax = -1.0e+31;
      for (i = 8; i < 16; i++) {
        if (offset[i] < vmin) vmin = offset[i];
        if (offset[i] > vmax) vmax = offset[i];
      }
      threshold[1][mezz] = nominalThreshold + 0.5*(vmin + vmax);
      if ((thresholdSettingMethod != 0) && ((vmax-vmin) > 12.)) {
        if ((vmax-vmin) < 14.) threshold[1][mezz] -= 2;
        else if ((vmax-vmin) < 16.) threshold[1][mezz] -= 4;
      }
      if ((vmax-vmin) > ASDVOffSpanCut) {
        largeVOffSpan = TRUE;
        sprintf(str, "<WARNING> ASD2 of Mezzanine card %2d <SN=%s> has large V offset span (=%.1f).\n", mezz, mezzSNumber);
        MDTChamberOutput2("%s", str);
      }
      vmin = 1.0e+31;
      vmax = -1.0e+31;
      for (i = 16; i < 24; i++) {
        if (offset[i] < vmin) vmin = offset[i];
        if (offset[i] > vmax) vmax = offset[i];
      }
      threshold[2][mezz] = nominalThreshold + 0.5*(vmin + vmax);
      if ((thresholdSettingMethod != 0) && ((vmax-vmin) > 12.)) {
        if ((vmax-vmin) < 14.) threshold[2][mezz] -= 2;
        else if ((vmax-vmin) < 16.) threshold[2][mezz] -= 4;
      }
      if ((vmax-vmin) > ASDVOffSpanCut) {
        largeVOffSpan = TRUE;
        sprintf(str, "<WARNING> ASD3 of Mezzanine card %2d <SN=%s> has large V offset span (=%.1f).\n", mezz, mezzSNumber);
        MDTChamberOutput2("%s", str);
      }
    }
    else {
      threshold[0][mezz] = nominalThreshold;
      threshold[1][mezz] = nominalThreshold;
      threshold[2][mezz] = nominalThreshold;
      if ((((mezzEnables>>mezz)%2) == 1) && (strlen(mezzSNumber) >= 15)) {
        mezzNotInDB = TRUE;
        sprintf(str, "<WARNING> Unable to find mezz %d (SN = %s) in database file!\n", mezz, mezzSNumber);
        MDTChamberOutput2("%s", str);
      }
    }
  }
  if (MDTChamberWarningFile != NULL) {
    if ((!gotMezzDatabase) || mezzNotInDB || (largeVOffSpan)) {
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
      strcpy(str, "");
      sprintf(str, "notepad %s", MDTChamberWarningFilename);
      stat = LaunchExecutable(str);
      if (stat != 0) stat = LaunchExecutable(str);
    }
    else {
      fprintf(MDTChamberWarningFile, "All Mezzanine Cards are fine from mezzanine card database\n");
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
    }
  }
  if (readNominalThresholdOnly) return;
  mezzCardSetupAll = TRUE;
  RecallMezzanineSetup();
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_LNTCONTROL, &enableLNThreshold);
  if (enableLNThreshold) {
    ASD1Control();
    ReversedBinaryToInt(&val, ASD_HYSTERESIS, 4, basicSetupArray);
    thre = (int) (nominalThreshold + 1.25*((float) val));
    if ((thre%2) != 0) {
      if (thre < 0) thre -= 1;
      else if (thre > 0) thre += 1;
    }
    SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, thre);
    ASD2Control();
    ReversedBinaryToInt(&val, ASD_SETUP_LENGTH+ASD_HYSTERESIS, 4, basicSetupArray);
    thre = (int) (nominalThreshold + 1.25*((float) val));
    if ((thre%2) != 0) {
      if (thre < 0) thre -= 1;
      else if (thre > 0) thre += 1;
    }
    SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, thre);
    ASD3Control();
    ReversedBinaryToInt(&val, 2*ASD_SETUP_LENGTH+ASD_HYSTERESIS, 4, basicSetupArray);
    thre = (int) (nominalThreshold + 1.25*((float) val));
    if ((thre%2) != 0) {
      if (thre < 0) thre -= 1;
      else if (thre > 0) thre += 1;
    }
    SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, thre);
    ASDDone();
    WaitInMillisecond(100);
    AMTErrorControlDone();
    WaitInMillisecond(100);
    PanelSave(AMTSetupHandle);
    LoadAMTSetupArray(); 
  }
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)%2) == 1) {
      mezzCardSetupAll = FALSE;
      mezzCardNb = mezz;
      RecallMezzanineSetup();
      GetCtrlVal(AMTSetupHandle, P_AMTSETUP_LNTCONTROL, &enableLNThreshold);
      if (enableLNThreshold) {
        ASD1Control();
        ReversedBinaryToInt(&val, ASD_HYSTERESIS, 4, basicSetupArray);
        NTHyst[0][mezz] = val;
        thre = (int) (threshold[0][mezz] + 1.25*((float) val));
        if ((thre%2) != 0) {
          if (thre < 0) thre -= 1;
          else if (thre > 0) thre += 1;
        }
        NTThre[0][mezz] = 127 + (thre/2);
        SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, thre);
        ASD2Control();
        ReversedBinaryToInt(&val, ASD_SETUP_LENGTH+ASD_HYSTERESIS, 4, basicSetupArray);
        NTHyst[1][mezz] = val;
        thre = (int) (threshold[1][mezz] + 1.25*((float) val));
        if ((thre%2) != 0) {
          if (thre < 0) thre -= 1;
          else if (thre > 0) thre += 1;
        }
        NTThre[1][mezz] = 127 + (thre/2);
        SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, thre);
        ASD3Control();
        ReversedBinaryToInt(&val, 2*ASD_SETUP_LENGTH+ASD_HYSTERESIS, 4, basicSetupArray);
        NTHyst[2][mezz] = val;
        thre = (int) (threshold[2][mezz] + 1.25*((float) val));
        if ((thre%2) != 0) {
          if (thre < 0) thre -= 1;
          else if (thre > 0) thre += 1;
        }
        NTThre[2][mezz] = 127 + (thre/2);
        SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, thre);
        ASDDone();
        WaitInMillisecond(100);
        AMTErrorControlDone();
        WaitInMillisecond(100);
        PanelSave(AMTSetupHandle);
        LoadAMTSetupArray();
      }
      SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezz], ATTR_CHECKED, 0);
      SetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezz], TRUE);
    }
  }
  mezzCardSetupAll = oldMezzCardSetupAll;
  mezzCardNb = oldMezzCardNb;
  RecallMezzanineSetup();
  PanelSave(AMTSetupHandle);
  LoadAMTSetupArray(); 
}


void UpSerialNumberAndLoadNominalASDMainThreshold(void) {
  UpSerialNumber();
  LoadNominalASDMainThreshold();
  InitDAQ();
}


void UpSerialNumberAndReadNominalASDMainThreshold(void) {
  UpSerialNumber();
  readNominalThresholdOnly = TRUE;
  LoadNominalASDMainThreshold();
  readNominalThresholdOnly = FALSE;
}


//*****************************************************************************
//
// Chamber related routines
//
//*****************************************************************************
//
// void GetNumberMezzAndRow(int chamberID, int *numberMezz, int *numberRow);
// Obtain number of mezzanine card and number of row according to chamber type
// --- INPUTs --- 
//    chamberID : int, MDT chamber ID (including its type and number)
// --- OUTPUTs ---
//   numberMezz : int, total number of mezzanine cards
//                0 = Invalid Number of Mezzanine Cards 
//    numberRow : int, total number of row for the chamber
//                0 = Invalid Number of Row 
//
void GetNumberMezzAndRow(int chamberID, int *numberMezz, int *numberRow) {
  int ctype, cnumber, n;
  
  *numberMezz = 0;
  *numberRow = 0;
  ctype = chamberID & CTYPEMASK; 
  cnumber = chamberID & CNUMBERMASK; 
  // printf("chamberID = 0x%08x, ctype = 0x%04x, cnumber = %d\n", chamberID, ctype, cnumber);
  if ((ctype == CTYPEEML1A) || (ctype == CTYPEEML1C)) {
    *numberMezz = 14;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEEML2A) || (ctype == CTYPEEML2C) ||
           (ctype == CTYPEEML3A) || (ctype == CTYPEEML3C) || 
           (ctype == CTYPEEML4A) || (ctype == CTYPEEML4C) || 
           (ctype == CTYPEEML5A) || (ctype == CTYPEEML5C) || 
           (ctype == CTYPEEMS1A) || (ctype == CTYPEEMS1C) || 
           (ctype == CTYPEEMS2A) || (ctype == CTYPEEMS2C) || 
           (ctype == CTYPEEMS3A) || (ctype == CTYPEEMS3C) || 
           (ctype == CTYPEEMS4A) || (ctype == CTYPEEMS4C) || 
           (ctype == CTYPEEMS5A) || (ctype == CTYPEEMS5C)) {
    *numberMezz = 16;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEEIL1A) || (ctype == CTYPEEIL1C) ||
           (ctype == CTYPEEIL2A) || (ctype == CTYPEEIL2C)) {
    *numberMezz = 12;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEEIL3A) || (ctype == CTYPEEIL3C)) {
    *numberMezz = 4;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEEIL23A) || (ctype == CTYPEEIL23C)) {
    *numberMezz = 16;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEEIL4A) || (ctype == CTYPEEIL4C)) {
    if ((cnumber == 1) || (cnumber == 9) || (cnumber == 11) || (cnumber == 15)) {
      *numberMezz = 14;
    }
    else *numberMezz = 18;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEEIS1A) || (ctype == CTYPEEIS1C)) {
    *numberMezz = 14;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEEIS2A) || (ctype == CTYPEEIS2C)) {
    *numberMezz = 12;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEEOL1A) || (ctype == CTYPEEOL1C) ||
           (ctype == CTYPEEOL2A) || (ctype == CTYPEEOL2C)) {
    *numberMezz = 14;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEEOL3A) || (ctype == CTYPEEOL3C) ||
           (ctype == CTYPEEOL4A) || (ctype == CTYPEEOL4C) ||
           (ctype == CTYPEEOL5A) || (ctype == CTYPEEOL5C) ||
           (ctype == CTYPEEOL6A) || (ctype == CTYPEEOL6C)) {
    *numberMezz = 12;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEEOS1A) || (ctype == CTYPEEOS1C) ||
           (ctype == CTYPEEOS2A) || (ctype == CTYPEEOS2C) ||
           (ctype == CTYPEEOS3A) || (ctype == CTYPEEOS3C)) {
    *numberMezz = 14;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEEOS4A) || (ctype == CTYPEEOS4C) ||
           (ctype == CTYPEEOS5A) || (ctype == CTYPEEOS5C) ||
           (ctype == CTYPEEOS6A) || (ctype == CTYPEEOS6C)) {
    *numberMezz = 12;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEEEL1A) || (ctype == CTYPEEEL1C) ||
           (ctype == CTYPEEEL2A) || (ctype == CTYPEEEL2C)) {
    *numberMezz = 10;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEEES1A) || (ctype == CTYPEEES1C)) {
    *numberMezz = 12;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEEES2A) || (ctype == CTYPEEES2C)) {
    *numberMezz = 10;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBIL1A) || (ctype == CTYPEBIL1C)) {
    // BIL1 chamber number of mezzanine card from 8 to 12
    *numberMezz = 12;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIL2A) || (ctype == CTYPEBIL2C)) {
    *numberMezz = 10;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIL3A) || (ctype == CTYPEBIL3C)) {
    // BIL3 chamber number of mezzanine card from 10 to 12
    *numberMezz = 12;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIL4A) || (ctype == CTYPEBIL4C)) {
    // BIL4 chamber number of mezzanine card from 10 to 12
    *numberMezz = 12;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIL5A) || (ctype == CTYPEBIL5C)) {
    *numberMezz = 10;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIL6A) || (ctype == CTYPEBIL6C)) {
    // BIL6 chamber number of mezzanine card from 10 to 12
    *numberMezz = 12;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIM1A) || (ctype == CTYPEBIM1C) ||
           (ctype == CTYPEBIM2A) || (ctype == CTYPEBIM2C) ||
           (ctype == CTYPEBIM3A) || (ctype == CTYPEBIM3C) ||
           (ctype == CTYPEBIM4A) || (ctype == CTYPEBIM4C) ||
           (ctype == CTYPEBIM5A) || (ctype == CTYPEBIM5C)) {
    *numberMezz = 12;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIS1A) || (ctype == CTYPEBIS1C)) {
    *numberMezz = 12;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIS2A) || (ctype == CTYPEBIS2C) ||
           (ctype == CTYPEBIS3A) || (ctype == CTYPEBIS3C) ||
           (ctype == CTYPEBIS4A) || (ctype == CTYPEBIS4C) ||
           (ctype == CTYPEBIS5A) || (ctype == CTYPEBIS5C) ||
           (ctype == CTYPEBIS6A) || (ctype == CTYPEBIS6C) ||
           (ctype == CTYPEBIS7A) || (ctype == CTYPEBIS7C)) {
    *numberMezz = 10;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIS8A) || (ctype == CTYPEBIS8C)) {
    *numberMezz = 2;
    *numberRow = 3;
  }
  else if ((ctype == CTYPEBIR1A) || (ctype == CTYPEBIR1C)) {
    *numberMezz = 10;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIR2A) || (ctype == CTYPEBIR2C)) {
    *numberMezz = 10;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIR3A) || (ctype == CTYPEBIR3C)) {
    *numberMezz = 12;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIR4A) || (ctype == CTYPEBIR4C)) {
    *numberMezz = 10;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIR5A) || (ctype == CTYPEBIR5C)) {
    *numberMezz = 8;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBIR6A) || (ctype == CTYPEBIR6C)) {
    *numberMezz = 12;
    *numberRow = 8;
  }
  else if ((ctype == CTYPEBMF1A) || (ctype == CTYPEBMF1C)) {
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBMF2A) || (ctype == CTYPEBMF3C)) {
    *numberMezz = 16;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBMF3A) || (ctype == CTYPEBMF3C)) {
    *numberMezz = 12;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBML1A) || (ctype == CTYPEBML1C)) {
    // BML1 number of mezzanine cards from 8 to 14
    *numberMezz = 14;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBML2A) || (ctype == CTYPEBML2C)) {
    *numberMezz = 14;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBML3A) || (ctype == CTYPEBML3C)) {
    *numberMezz = 14;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBML4A) || (ctype == CTYPEBML4C)) {
    *numberMezz = 10;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBML5A) || (ctype == CTYPEBML5C)) {
    *numberMezz = 10;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBML6A) || (ctype == CTYPEBML6C)) {
    *numberMezz = 12;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBMS1A) || (ctype == CTYPEBMS1C)) {
    *numberMezz = 14;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBMS2A) || (ctype == CTYPEBMS2C)) {
    *numberMezz = 12;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBMS3A) || (ctype == CTYPEBMS3C)) {
    *numberMezz = 12;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBMS4A) || (ctype == CTYPEBMS4C)) {
    *numberMezz = 12;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBMS5A) || (ctype == CTYPEBMS5C)) {
    *numberMezz = 8;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBMS6A) || (ctype == CTYPEBMS6C)) {
    *numberMezz = 12;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOL1A) || (ctype == CTYPEBOL1C)) {
    // BOL1 number of mezzanine cards from 12 to 18
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOL2A) || (ctype == CTYPEBOL2C)) {
    // BOL2 number of mezzanine cards from 12 or 18
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOL3A) || (ctype == CTYPEBOL3C)) {
    // BOL3 number of mezzanine cards from 12 or 14
    *numberMezz = 14;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOL4A) || (ctype == CTYPEBOL4C)) {
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOL5A) || (ctype == CTYPEBOL5C)) {
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOL6A) || (ctype == CTYPEBOL6C)) {
    *numberMezz = 14;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOS1A) || (ctype == CTYPEBOS1C)) {
    // BOS1 number of mezzanine cards from 12 or 18
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOS2A) || (ctype == CTYPEBOS2C)) {
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOS3A) || (ctype == CTYPEBOS3C)) {
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOS4A) || (ctype == CTYPEBOS4C)) {
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOS5A) || (ctype == CTYPEBOS5C)) {
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOS6A) || (ctype == CTYPEBOS6C)) {
    *numberMezz = 16;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOSXA) || (ctype == CTYPEBOSXC)) {
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOG0A) || (ctype == CTYPEBOG0B) || (ctype == CTYPEBOG0C)) {
    *numberMezz = 10;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOG2A) || (ctype == CTYPEBOG2B) || (ctype == CTYPEBOG2C)) {
    *numberMezz = 10;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOG4A) || (ctype == CTYPEBOG4B) || (ctype == CTYPEBOG4C)) {
    *numberMezz = 10;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOG6A) || (ctype == CTYPEBOG6B) || (ctype == CTYPEBOG6C)) {
    *numberMezz = 10;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOG8A) || (ctype == CTYPEBOG8B) || (ctype == CTYPEBOG8C)) {
    *numberMezz = 10;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOF1A) || (ctype == CTYPEBOF1C)) {
    *numberMezz = 18;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOF3A) || (ctype == CTYPEBOF3C)) {
    *numberMezz = 16;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOF5A) || (ctype == CTYPEBOF5C)) {
    *numberMezz = 12;
    *numberRow = 6;
  }
  else if ((ctype == CTYPEBOF7A) || (ctype == CTYPEBOF7C)) {
    *numberMezz = 10;
    *numberRow = 6;
  }

  // Overwrite default vaule from chamber database
  //
  if (chamberIndex >= 0) {
    n = nMLayer[chamberIndex] * nMezzMLayer[chamberIndex];
    if ((n > 0) && (n <= MAXNUMBERMEZZANINE)) *numberMezz = n;
    n = nMLayer[chamberIndex] * nLayer[chamberIndex];
    if ((n > 0) && (n < 9)) *numberRow = n;
  }
}


//
// void MapChamberFromMezzCard(int ctype, int mezz, int ch, int *mappedCh, int *row, int *tube);
// Convert mezzanine card number and its channel number to MDT chamber row number
// and tube number according to chamber type
// --- INPUTs --- 
//        ctype : int, MDT chamber type
//         mezz : int, mezzanine card number
//           ch : int, Mezzanine card channel number
// --- OUTPUTs ---
//     mappedCh : int*, Mapped Mezzanine card channber number as following
//                -1 = Invalid Mapped Channel Number
//                 0 = Mapped Channel Number 0
//                ...
//                23 = Mapped Channel Number 23
//                 Mezzanine Card look as following
//                -----------------------------------
//                |                                 |
//                |     ASD1Chip          AMTChip   |
//                | o                               |
//                | O    ASD2Chip              O    |
//                | o                 ConnectToMB   |
//                |     ASD3Chip                    |
//                |                                 |
//                -----------------------------------
//                >>>> For 3x8 mezzanine card <<<<
//                TUBE      T  T  T  T  T  T  T  T
//                ASD1 ==>  0  1  2  3  4  5  6  7  (For A Chamber TOP/Bottom row 1)
//                ASD2 ==>  8  9 10 11 12 13 14 15  (For A Chamber TOP/Bottom row 2)
//                ASD3 ==> 16 17 18 19 20 21 22 23  (For A Chamber TOP/Bottom row 3)
//
//                TUBE      T  T  T  T  T  T  T  T
//                ASD1 ==>  0  1  2  3  4  5  6  7  (For C Chamber TOP/Bottom row 3)
//                ASD2 ==>  8  9 10 11 12 13 14 15  (For C Chamber TOP/Bottom row 2)
//                ASD3 ==> 16 17 18 19 20 21 22 23  (For C Chamber TOP/Bottom row 1)
//
//                >>>> For 4x6 mezzanine card <<<<
//                TUBE           T  T  T  T  T  T
//                ASD1      ==>  0  1  2  3  4  5  (For A Chamber TOP/Bottom row 1)
//                ASD1/ASD2 ==>  6  7  8  9 10 11  (For A Chamber TOP/Bottom row 2)
//                ASD2/ASD3 ==> 12 13 14 15 16 17  (For A Chamber TOP/Bottom row 3)
//                ASD3      ==> 18 19 20 21 22 23  (For A Chamber TOP/Bottom row 4)
//
//                TUBE           T  T  T  T  T  T
//                ASD1      ==>  0  1  2  3  4  5  (For C Chamber TOP/Bottom row 4)
//                ASD1/ASD2 ==>  6  7  8  9 10 11  (For C Chamber TOP/Bottom row 3)
//                ASD2/ASD3 ==> 12 13 14 15 16 17  (For C Chamber TOP/Bottom row 2
//                ASD3      ==> 18 19 20 21 22 23  (For C Chamber TOP/Bottom row 1)
//          row : int*, Chamber Row Number
//                >>>> For Chamber EM/EO/EE Type A and C <<<<
//                  0 = Invalid Row Number 
//                  1 = TOP row 1
//                  2 = TOP row 2
//                  3 = TOP row 3
//                  4 = Bottom row 1
//                  5 = Bottom row 2
//                  6 = Bottom row 3
//
//                >>>> For Chamber EI Type A and C <<<<
//                  0 = Invalid Row Number 
//                  1 = TOP row 1
//                  2 = TOP row 2
//                  3 = TOP row 3
//                  4 = TOP row 4
//                  5 = Bottom row 1
//                  6 = Bottom row 2
//                  7 = Bottom row 3
//                  8 = Bottom row 3
//         tube : int*, Tube Number 1 to maximum number of tubes from short side to long Side,
//                where for EM type chamber, the maximum number of tubes is 54 (EML1) or 64,
//                      for EI type chamber, the maximum number of tubes is 12, 36, 42 and 56
//                   0 = Invalid Tube Number
//                   1 = Shortest Tube Number
//                   ...
//                  64 = Longest Tube Number (for most EM type chamber)
//                Short-Tube                                                    Long-Tube
//                12345678 90123456 78901234 56789012 34567890 12345678 90123456 78901234
//   
void MapChamberFromMezzCard(int ctype, int mezz, int ch, int *mappedCh, int *row, int *tube) {
  int i, nRow, nRowChannel;

  if ((mezz - startMezzCard) >= 0) mezz -= startMezzCard;
  if (MDTChamberNumberMezz <= 0) GetNumberMezzAndRow(ctype, &MDTChamberNumberMezz, &MDTChamberNumberRow);
  if ((MDTChamberNumberRow >= 3) && (MDTChamberNumberRow <= 4)) {
    nRow = 2 * MDTChamberNumberRow;
    nRowChannel = 24 / MDTChamberNumberRow;
  }
  else {
    nRow = 0;
    nRowChannel = 0;
  }
  ctype &= CTYPEMASK; 
  if (ctype == CTYPEEML1A) ctype = CTYPEEMA;
  else if (ctype == CTYPEEML2A) ctype = CTYPEEMA; 
  else if (ctype == CTYPEEML3A) ctype = CTYPEEMA; 
  else if (ctype == CTYPEEML4A) ctype = CTYPEEMA; 
  else if (ctype == CTYPEEML5A) ctype = CTYPEEMA; 
  else if (ctype == CTYPEEML1C) ctype = CTYPEEMC;
  else if (ctype == CTYPEEML2C) ctype = CTYPEEMC;
  else if (ctype == CTYPEEML3C) ctype = CTYPEEMC; 
  else if (ctype == CTYPEEML4C) ctype = CTYPEEMC; 
  else if (ctype == CTYPEEML5C) ctype = CTYPEEMC; 
  else if (ctype == CTYPEEMS1A) ctype = CTYPEEMA;
  else if (ctype == CTYPEEMS2A) ctype = CTYPEEMA; 
  else if (ctype == CTYPEEMS3A) ctype = CTYPEEMA; 
  else if (ctype == CTYPEEMS4A) ctype = CTYPEEMA; 
  else if (ctype == CTYPEEMS5A) ctype = CTYPEEMA;
  else if (ctype == CTYPEEMS1C) ctype = CTYPEEMC;
  else if (ctype == CTYPEEMS2C) ctype = CTYPEEMC; 
  else if (ctype == CTYPEEMS3C) ctype = CTYPEEMC; 
  else if (ctype == CTYPEEMS4C) ctype = CTYPEEMC; 
  else if (ctype == CTYPEEMS5C) ctype = CTYPEEMC;
  else if (ctype == CTYPEEIL1A) ctype = CTYPEEIA;
  else if (ctype == CTYPEEIL2A) ctype = CTYPEEIA; 
  else if (ctype == CTYPEEIL3A) ctype = CTYPEEIA; 
  else if (ctype == CTYPEEIL4A) ctype = CTYPEEIA; 
  else if (ctype == CTYPEEIL5A) ctype = CTYPEEIA; 
  else if (ctype == CTYPEEIL23A) ctype = CTYPEEIA; 
  else if (ctype == CTYPEEIL1C) ctype = CTYPEEIC;
  else if (ctype == CTYPEEIL2C) ctype = CTYPEEIC;
  else if (ctype == CTYPEEIL3C) ctype = CTYPEEIC; 
  else if (ctype == CTYPEEIL4C) ctype = CTYPEEIC; 
  else if (ctype == CTYPEEIL5C) ctype = CTYPEEIC; 
  else if (ctype == CTYPEEIL23C) ctype = CTYPEEIC;
  else if (ctype == CTYPEEIS1A) ctype = CTYPEEIA;
  else if (ctype == CTYPEEIS2A) ctype = CTYPEEIA; 
  else if (ctype == CTYPEEIS3A) ctype = CTYPEEIA; 
  else if (ctype == CTYPEEIS4A) ctype = CTYPEEIA; 
  else if (ctype == CTYPEEIS5A) ctype = CTYPEEIA; 
  else if (ctype == CTYPEEIS1C) ctype = CTYPEEIC;
  else if (ctype == CTYPEEIS2C) ctype = CTYPEEIC;
  else if (ctype == CTYPEEIS3C) ctype = CTYPEEIC; 
  else if (ctype == CTYPEEIS4C) ctype = CTYPEEIC; 
  else if (ctype == CTYPEEIS5C) ctype = CTYPEEIC; 
  else if (ctype == CTYPEEOL1A) ctype = CTYPEEOA;
  else if (ctype == CTYPEEOL2A) ctype = CTYPEEOA; 
  else if (ctype == CTYPEEOL3A) ctype = CTYPEEOA; 
  else if (ctype == CTYPEEOL4A) ctype = CTYPEEOA; 
  else if (ctype == CTYPEEOL5A) ctype = CTYPEEOA; 
  else if (ctype == CTYPEEOL6A) ctype = CTYPEEOA; 
  else if (ctype == CTYPEEOL1C) ctype = CTYPEEOC;
  else if (ctype == CTYPEEOL2C) ctype = CTYPEEOC; 
  else if (ctype == CTYPEEOL3C) ctype = CTYPEEOC; 
  else if (ctype == CTYPEEOL4C) ctype = CTYPEEOC; 
  else if (ctype == CTYPEEOL5C) ctype = CTYPEEOC; 
  else if (ctype == CTYPEEOL6C) ctype = CTYPEEOC; 
  else if (ctype == CTYPEEOS1A) ctype = CTYPEEOA;
  else if (ctype == CTYPEEOS2A) ctype = CTYPEEOA; 
  else if (ctype == CTYPEEOS3A) ctype = CTYPEEOA; 
  else if (ctype == CTYPEEOS4A) ctype = CTYPEEOA; 
  else if (ctype == CTYPEEOS5A) ctype = CTYPEEOA; 
  else if (ctype == CTYPEEOS6A) ctype = CTYPEEOA; 
  else if (ctype == CTYPEEOS1C) ctype = CTYPEEOC;
  else if (ctype == CTYPEEOS2C) ctype = CTYPEEOC; 
  else if (ctype == CTYPEEOS3C) ctype = CTYPEEOC; 
  else if (ctype == CTYPEEOS4C) ctype = CTYPEEOC; 
  else if (ctype == CTYPEEOS5C) ctype = CTYPEEOC; 
  else if (ctype == CTYPEEOS6C) ctype = CTYPEEOC; 
  else if (ctype == CTYPEEEL1A) ctype = CTYPEEEA;
  else if (ctype == CTYPEEEL2A) ctype = CTYPEEEA; 
  else if (ctype == CTYPEEEL3A) ctype = CTYPEEEA; 
  else if (ctype == CTYPEEEL4A) ctype = CTYPEEEA; 
  else if (ctype == CTYPEEEL5A) ctype = CTYPEEEA; 
  else if (ctype == CTYPEEEL1C) ctype = CTYPEEEC;
  else if (ctype == CTYPEEEL2C) ctype = CTYPEEEC; 
  else if (ctype == CTYPEEEL3C) ctype = CTYPEEEC; 
  else if (ctype == CTYPEEEL4C) ctype = CTYPEEEC; 
  else if (ctype == CTYPEEEL5C) ctype = CTYPEEEC; 
  else if (ctype == CTYPEEES1A) ctype = CTYPEEEA;
  else if (ctype == CTYPEEES2A) ctype = CTYPEEEA; 
  else if (ctype == CTYPEEES3A) ctype = CTYPEEEA; 
  else if (ctype == CTYPEEES4A) ctype = CTYPEEEA; 
  else if (ctype == CTYPEEES5A) ctype = CTYPEEEA; 
  else if (ctype == CTYPEEES1C) ctype = CTYPEEEC;
  else if (ctype == CTYPEEES2C) ctype = CTYPEEEC; 
  else if (ctype == CTYPEEES3C) ctype = CTYPEEEC; 
  else if (ctype == CTYPEEES4C) ctype = CTYPEEEC; 
  else if (ctype == CTYPEEES5C) ctype = CTYPEEEC; 

  *mappedCh = -1;
  *row = 0;
  *tube = 0;
  if ((ch < 0) || (ch >= 24)) return;
  if ((mezz < 0) || (mezz >= MAXNUMBERMEZZANINE)) return;
  if (mezz >= MDTChamberNumberMezz) return;

  if ((ctype == CTYPEEMA) || (ctype == CTYPEEMC) ||
      (ctype == CTYPEEOA) || (ctype == CTYPEEOC) ||
      (ctype == CTYPEEEA) || (ctype == CTYPEEEC)) {
    if ((nRow <= 0) || (nRowChannel <= 0)) {
      nRow = 6;
      nRowChannel = 8;
    }
    if ((mezz%2) == 0) {  // Map Signal Hedgehog type I with AMT3 mezz.card
      if (ch == 0) *mappedCh = 5;
      else if (ch == 1) *mappedCh = 0;
      else if (ch == 2) *mappedCh = 7;
      else if (ch == 3) *mappedCh = 1;
      else if (ch == 4) *mappedCh = 6;
      else if (ch == 5) *mappedCh = 2;
      else if (ch == 6) *mappedCh = 4;
      else if (ch == 7) *mappedCh = 3;
      else if (ch == 8) *mappedCh = 13;
      else if (ch == 9) *mappedCh = 8;
      else if (ch == 10) *mappedCh = 15;
      else if (ch == 11) *mappedCh = 9;
      else if (ch == 12) *mappedCh = 14;
      else if (ch == 13) *mappedCh = 10;
      else if (ch == 14) *mappedCh = 12;
      else if (ch == 15) *mappedCh = 11;
      else if (ch == 16) *mappedCh = 21;
      else if (ch == 17) *mappedCh = 16;
      else if (ch == 18) *mappedCh = 23;
      else if (ch == 19) *mappedCh = 17;
      else if (ch == 20) *mappedCh = 22;
      else if (ch == 21) *mappedCh = 18;
      else if (ch == 22) *mappedCh = 20;
      else if (ch == 23) *mappedCh = 19;
      i = mezz / 2;
      if ((ctype == CTYPEEMA) || (ctype == CTYPEEOA) || (ctype == CTYPEEEA))
        *row = 1 + 24/nRowChannel + *mappedCh/nRowChannel;
      else if ((ctype == CTYPEEMC) || (ctype == CTYPEEOC) || (ctype == CTYPEEEC))
        *row = 24/nRowChannel - *mappedCh/nRowChannel;
    }
    else if ((mezz%2) == 1) {  // Map Signal Hedgehog type II with AMT3 mezz.card
      if (ch == 0) *mappedCh = 7;
      else if (ch == 1) *mappedCh = 0;
      else if (ch == 2) *mappedCh = 6;
      else if (ch == 3) *mappedCh = 1;
      else if (ch == 4) *mappedCh = 5;
      else if (ch == 5) *mappedCh = 2;
      else if (ch == 6) *mappedCh = 4;
      else if (ch == 7) *mappedCh = 3;
      else if (ch == 8) *mappedCh = 15;
      else if (ch == 9) *mappedCh = 8;
      else if (ch == 10) *mappedCh = 14;
      else if (ch == 11) *mappedCh = 9;
      else if (ch == 12) *mappedCh = 13;
      else if (ch == 13) *mappedCh = 10;
      else if (ch == 14) *mappedCh = 12;
      else if (ch == 15) *mappedCh = 11;
      else if (ch == 16) *mappedCh = 23;
      else if (ch == 17) *mappedCh = 16;
      else if (ch == 18) *mappedCh = 22;
      else if (ch == 19) *mappedCh = 17;
      else if (ch == 20) *mappedCh = 21;
      else if (ch == 21) *mappedCh = 18;
      else if (ch == 22) *mappedCh = 20;
      else if (ch == 23) *mappedCh = 19;
      i = mezz / 2;
      if ((ctype == CTYPEEMA) || (ctype == CTYPEEOA) || (ctype == CTYPEEEA))
        *row = 1 + *mappedCh/nRowChannel;
      else if ((ctype == CTYPEEMC) || (ctype == CTYPEEOC) || (ctype == CTYPEEEC))
        *row = 48/nRowChannel - *mappedCh/nRowChannel;
    }
    if (*mappedCh < nRowChannel) *tube = nRowChannel*i + *mappedCh + 1;
    else if (*mappedCh < 2*nRowChannel) *tube = nRowChannel*i + *mappedCh + 1 - nRowChannel;
    else if (*mappedCh < 3*nRowChannel) *tube = nRowChannel*i + *mappedCh + 1 - nRowChannel*2;
    else if (*mappedCh < 4*nRowChannel) *tube = nRowChannel*i + *mappedCh + 1 - nRowChannel*3;
  }
  else if ((ctype == CTYPEEIA) || (ctype == CTYPEEIC)) {
    if ((nRow <= 0) || (nRowChannel <= 0)) {
      nRow = 8;
      nRowChannel = 6;
    }
    if ((mezz%2) == 0) {  // Map Signal Hedgehog type IV with AMT3 mezz.card
      if (ch == 0) *mappedCh = 3;
      else if (ch == 1) *mappedCh = 1;
      else if (ch == 2) *mappedCh = 4;
      else if (ch == 3) *mappedCh = 0;
      else if (ch == 4) *mappedCh = 5;
      else if (ch == 5) *mappedCh = 2;
      else if (ch == 6) *mappedCh = 9;
      else if (ch == 7) *mappedCh = 7;
      else if (ch == 8) *mappedCh = 10;
      else if (ch == 9) *mappedCh = 6;
      else if (ch == 10) *mappedCh = 11;
      else if (ch == 11) *mappedCh = 8;
      else if (ch == 12) *mappedCh = 15;
      else if (ch == 13) *mappedCh = 13;
      else if (ch == 14) *mappedCh = 16;
      else if (ch == 15) *mappedCh = 12;
      else if (ch == 16) *mappedCh = 17;
      else if (ch == 17) *mappedCh = 14;
      else if (ch == 18) *mappedCh = 21;
      else if (ch == 19) *mappedCh = 18;
      else if (ch == 20) *mappedCh = 22;
      else if (ch == 21) *mappedCh = 19;
      else if (ch == 22) *mappedCh = 23;
      else if (ch == 23) *mappedCh = 20;
      i = mezz / 2;
      if (ctype == CTYPEEIA) *row = 1 + 24/nRowChannel + *mappedCh/nRowChannel;
      else if (ctype == CTYPEEIC) *row = 24/nRowChannel - *mappedCh/nRowChannel;
    }
    else if ((mezz%2) == 1) {  // Map Signal Hedgehog type III with AMT3 mezz.card
      if (ch == 0) *mappedCh = 4;
      else if (ch == 1) *mappedCh = 5;
      else if (ch == 2) *mappedCh = 3;
      else if (ch == 3) *mappedCh = 1;
      else if (ch == 4) *mappedCh = 2;
      else if (ch == 5) *mappedCh = 0;
      else if (ch == 6) *mappedCh = 10;
      else if (ch == 7) *mappedCh = 11;
      else if (ch == 8) *mappedCh = 9;
      else if (ch == 9) *mappedCh = 7;
      else if (ch == 10) *mappedCh = 8;
      else if (ch == 11) *mappedCh = 6;
      else if (ch == 12) *mappedCh = 16;
      else if (ch == 13) *mappedCh = 17;
      else if (ch == 14) *mappedCh = 15;
      else if (ch == 15) *mappedCh = 13;
      else if (ch == 16) *mappedCh = 14;
      else if (ch == 17) *mappedCh = 12;
      else if (ch == 18) *mappedCh = 22;
      else if (ch == 19) *mappedCh = 23;
      else if (ch == 20) *mappedCh = 21;
      else if (ch == 21) *mappedCh = 19;
      else if (ch == 22) *mappedCh = 20;
      else if (ch == 23) *mappedCh = 18;
      i = mezz / 2;
      if (ctype == CTYPEEIA) *row = 1 + *mappedCh/nRowChannel;
      else if (ctype == CTYPEEIC) *row = 48/nRowChannel - *mappedCh/nRowChannel;
    }
    if (*mappedCh < nRowChannel) *tube = nRowChannel*i + *mappedCh + 1;
    else if (*mappedCh < 2*nRowChannel) *tube = nRowChannel*i + *mappedCh + 1 - nRowChannel;
    else if (*mappedCh < 3*nRowChannel) *tube = nRowChannel*i + *mappedCh + 1 - nRowChannel*2;
    else if (*mappedCh < 4*nRowChannel) *tube = nRowChannel*i + *mappedCh + 1 - nRowChannel*3;
  }
}



//
// void MapMezzCardFromChamber(int ctype, int row, int tube, int *mezz, int *ch, int *mappedCh);
// Convert MDT chamber row number and tube number to mezzanine card number
// and its channel number according to chamber type
// --- INPUTs --- 
//        ctype : int, MDT chamber type
//          row : int, Chamber Row Number
//                >>>> For Chamber EM/EO/EE Type A and C <<<<
//                  0 = Invalid Row Number 
//                  1 = TOP row 1
//                  2 = TOP row 2
//                  3 = TOP row 3
//                  4 = Bottom row 1
//                  5 = Bottom row 2
//                  6 = Bottom row 3
//
//                >>>> For Chamber EI Type A and C <<<<
//                  0 = Invalid Row Number 
//                  1 = TOP row 1
//                  2 = TOP row 2
//                  3 = TOP row 3
//                  4 = TOP row 4
//                  5 = Bottom row 1
//                  6 = Bottom row 2
//                  7 = Bottom row 3
//                  8 = Bottom row 3
//         tube : int, Tube Number 1 to maximum number of tubes from short side to long Side,
//                where for EM type chamber, the maximum number of tubes is 54 (EML1) or 64,
//                      for EI type chamber, the maximum number of tubes is 12, 36, 42 and 56
//                   0 = Invalid Tube Number
//                   1 = Shortest Tube Number
//                   ...
//                  64 = Longest Tube Number (for most EM type chamber)
//                Short-Tube                                                    Long-Tube
//                12345678 90123456 78901234 56789012 34567890 12345678 90123456 78901234
// --- OUTPUTs ---
//         mezz : int*, mezzanine card number
//                -1 = Invalid mezzanine card number
//                 0 = Mezzanine card number 0
//                ...
//                17 = Mezzanine card number 17
//           ch : int*, Mezzanine card channel number
//                -1 = Invalid Channel Number
//                 0 = Channel Number 0
//                ...
//                23 = Channel Number 23
//     mappedCh : int*, Mapped Mezzanine card channber number as following
//                -1 = Invalid Mapped Channel Number
//                 0 = Mapped Channel Number 0
//                ...
//                23 = Mapped Channel Number 23
//                 Mezzanine Card look as following
//                -----------------------------------
//                |                                 |
//                |     ASD1Chip          AMTChip   |
//                | o                               |
//                | O    ASD2Chip              O    |
//                | o                 ConnectToMB   |
//                |     ASD3Chip                    |
//                |                                 |
//                -----------------------------------
//                >>>> For 3x8 mezzanine card <<<<
//                TUBE      T  T  T  T  T  T  T  T
//                ASD1 ==>  0  1  2  3  4  5  6  7  (For A Chamber TOP/Bottom row 1)
//                ASD2 ==>  8  9 10 11 12 13 14 15  (For A Chamber TOP/Bottom row 2)
//                ASD3 ==> 16 17 18 19 20 21 22 23  (For A Chamber TOP/Bottom row 3)
//
//                TUBE      T  T  T  T  T  T  T  T
//                ASD1 ==>  0  1  2  3  4  5  6  7  (For C Chamber TOP/Bottom row 3)
//                ASD2 ==>  8  9 10 11 12 13 14 15  (For C Chamber TOP/Bottom row 2)
//                ASD3 ==> 16 17 18 19 20 21 22 23  (For C Chamber TOP/Bottom row 1)
//
//                >>>> For 4x6 mezzanine card <<<<
//                TUBE           T  T  T  T  T  T
//                ASD1      ==>  0  1  2  3  4  5  (For A Chamber TOP/Bottom row 1)
//                ASD1/ASD2 ==>  6  7  8  9 10 11  (For A Chamber TOP/Bottom row 2)
//                ASD2/ASD3 ==> 12 13 14 15 16 17  (For A Chamber TOP/Bottom row 3)
//                ASD3      ==> 18 19 20 21 22 23  (For A Chamber TOP/Bottom row 4)
//
//                TUBE           T  T  T  T  T  T
//                ASD1      ==>  0  1  2  3  4  5  (For C Chamber TOP/Bottom row 4)
//                ASD1/ASD2 ==>  6  7  8  9 10 11  (For C Chamber TOP/Bottom row 3)
//                ASD2/ASD3 ==> 12 13 14 15 16 17  (For C Chamber TOP/Bottom row 2
//                ASD3      ==> 18 19 20 21 22 23  (For C Chamber TOP/Bottom row 1)
//   
void MapMezzCardFromChamber(int ctype, int row, int tube, int *mezz, int *ch, int *mappedCh) {
  int row1, tube1, mezz1, ch1, mappedCh1, mapDone;

  mapDone = FALSE;
  *mezz = -1;
  *ch = -1;
  *mappedCh = -1;
  if ((row <= 0) || (tube <= 0)) return;
  for (mezz1 = 0; mezz1 < MAXNUMBERMEZZANINE; mezz1++) {
    for (ch1 = 0; ch1 < 24; ch1++) {
      MapChamberFromMezzCard(ctype, mezz1, ch1, &mappedCh1, &row1, &tube1);
      if ((row1 == row) && (tube1 == tube)) {
        mapDone = TRUE;
        *mezz = mezz1;
        *ch = ch1;
        *mappedCh = mappedCh1;
        break;
      }
    }
    if (mapDone) break;
  }
}



//
// void int MultilayerNumber(char *chamberName, int row);
// Convert to Standard ATLAS Multilayer and Layer number
// from MDT row number according to chamber name
// --- INPUTs --- 
//  chamberName : char*, MDT chamber name
//          row : int, Chamber Row Number
//                >>>> For Chamber EM/EO/EE Type A and C <<<<
//                  0 = Invalid Row Number 
//                  1 = TOP row 1
//                  2 = TOP row 2
//                  3 = TOP row 3
//                  4 = Bottom row 1
//                  5 = Bottom row 2
//                  6 = Bottom row 3
//
//                >>>> For Chamber EI Type A and C <<<<
//                  0 = Invalid Row Number 
//                  1 = TOP row 1
//                  2 = TOP row 2
//                  3 = TOP row 3
//                  4 = TOP row 4
//                  5 = Bottom row 1
//                  6 = Bottom row 2
//                  7 = Bottom row 3
//                  8 = Bottom row 3
// return value : chamber Multilayer-layer number
//                = MultilayerNumber*10 + LayerNumber
//
int MultilayerNumber(char *chamberName, int row) {
  int ML = 0;

  if (strncmp(chamberName, "eml", 3) == 0) {
    if (row == 1) ML = 11;
    else if (row == 2) ML = 12;
    else if (row == 3) ML = 13;
    else if (row == 4) ML = 21;
    else if (row == 5) ML = 22;
    else if (row == 6) ML = 23;
  }
  else if (strncmp(chamberName, "ems", 3) == 0) {
    if (row == 1) ML = 23;
    else if (row == 2) ML = 22;
    else if (row == 3) ML = 21;
    else if (row == 4) ML = 13;
    else if (row == 5) ML = 12;
    else if (row == 6) ML = 11;
  }
  else if (strncmp(chamberName, "eil", 3) == 0) {
    if (row == 1) ML = 11;
    else if (row == 2) ML = 12;
    else if (row == 3) ML = 13;
    else if (row == 4) ML = 14;
    else if (row == 5) ML = 21;
    else if (row == 6) ML = 22;
  }
  else if (strncmp(chamberName, "eis", 3) == 0) {
    if (row == 1) ML = 24;
    else if (row == 2) ML = 23;
    else if (row == 3) ML = 22;
    else if (row == 4) ML = 21;
    else if (row == 5) ML = 14;
    else if (row == 6) ML = 13;
    else if (row == 7) ML = 12;
    else if (row == 8) ML = 11;
  }
  else if (strncmp(chamberName, "eol", 3) == 0) {
    if (row == 1) ML = 23;
    else if (row == 2) ML = 22;
    else if (row == 3) ML = 21;
    else if (row == 4) ML = 13;
    else if (row == 5) ML = 12;
    else if (row == 6) ML = 11;
  }
  else if (strncmp(chamberName, "eos", 3) == 0) {
    if (row == 1) ML = 11;
    else if (row == 2) ML = 12;
    else if (row == 3) ML = 13;
    else if (row == 4) ML = 21;
    else if (row == 5) ML = 22;
    else if (row == 6) ML = 23;
  }

  return ML;
}



//
// double TubeLength(char *chamberName, int row, int tube);
// obtain the tube length in mm for a given MDT chamber
// --- INPUTs --- 
//  chamberName : char*, MDT chamber name
//          row : int, Chamber Row Number
//                >>>> For Chamber EM/EO/EE Type A and C <<<<
//                  0 = Invalid Row Number 
//                  1 = TOP row 1
//                  2 = TOP row 2
//                  3 = TOP row 3
//                  4 = Bottom row 1
//                  5 = Bottom row 2
//                  6 = Bottom row 3
//
//                >>>> For Chamber EI Type A and C <<<<
//                  0 = Invalid Row Number 
//                  1 = TOP row 1
//                  2 = TOP row 2
//                  3 = TOP row 3
//                  4 = TOP row 4
//                  5 = Bottom row 1
//                  6 = Bottom row 2
//                  7 = Bottom row 3
//                  8 = Bottom row 3
//         tube : int, Tube Number 1 to maximum number of tubes from short side to long Side,
//                where for EM type chamber, the maximum number of tubes is 54 (EML1) or 64,
//                      for EI type chamber, the maximum number of tubes is 12, 36, 42 and 56
//                   0 = Invalid Tube Number
//                   1 = Shortest Tube Number
//                   ...
//                  64 = Longest Tube Number (for most EM type chamber)
//                Short-Tube                                                    Long-Tube
//                12345678 90123456 78901234 56789012 34567890 12345678 90123456 78901234
// return value : 0.0 = invalid tube length (for instance for unknow chamber)
//                Otherwise:
//                  tube length (in mm) if a valid tube number is given
//                  average tube length if the given tube number is 0
//               
double TubeLength(char *chamberName, int row, int tube) {
  double tubeLength, minLength, maxLength, step;
  int ntubes, unknownChamber;
  
  minLength = 0.0;
  maxLength = 0.0;
  ntubes = 0;
  unknownChamber = FALSE;
  if (strncmp(chamberName, "eml1", 4) == 0) {
    minLength = 1163.0;
    maxLength = 1883.0;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "eml2", 4) == 0) {
    minLength = 2003.0;
    maxLength = 2843.0;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "eml3", 4) == 0) {
    minLength = 2963.0;
    maxLength = 3803.0;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "eml4", 4) == 0) {
    minLength = 3923.0;
    maxLength = 4763.0;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "eml5", 4) == 0) {
    minLength = 4883.0;
    maxLength = 5723.0;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "ems1", 4) == 0) {
    minLength = 812.0;
    maxLength = 1316.0;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "ems2", 4) == 0) {
    minLength = 1388.0;
    maxLength = 1892.0;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "ems3", 4) == 0) {
    minLength = 1964.0;
    maxLength = 2468.0;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "ems4", 4) == 0) {
    minLength = 2540.0;
    maxLength = 3044.0;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "ems5", 4) == 0) {
    minLength = 3116.0;
    maxLength = 3620.0;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "eil1", 4) == 0) {
    minLength = 1298.0;
    maxLength = 1748.0;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "eil2", 4) == 0) {
    minLength = 1838.0;
    maxLength = 2288.0;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "eil3", 4) == 0) {
    // eil3 chambers have 3 different lengthes, pickup the shortest
    minLength = 1718.0;
    maxLength = 1718.0;
    ntubes = 12;
  }
  else if (strncmp(chamberName, "eil4", 4) == 0) {
    // eil4 chambers have 4 different lengthes, pickup the shortest
    minLength = 1258.0;
    maxLength = 1798.0;
    ntubes = 54;
  }
  else if (strncmp(chamberName, "eis1", 4) == 0) {
    minLength = 875.0;
    maxLength = 1199.0;
    ntubes = 42;
  }
  else if (strncmp(chamberName, "eis2", 4) == 0) {
    minLength = 1253.0;
    maxLength = 1523.0;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "eol1", 4) == 0) {
    minLength = 1681.5;
    maxLength = 2401.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "eol2", 4) == 0) {
    minLength = 2641.5;
    maxLength = 3361.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "eol3", 4) == 0) {
    minLength = 3481.5;
    maxLength = 4081.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "eol4", 4) == 0) {
    minLength = 4201.5;
    maxLength = 4801.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "eol5", 4) == 0) {
    minLength = 4921.5;
    maxLength = 5521.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "eol6", 4) == 0) {
    minLength = 5641.5;
    maxLength = 6241.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "eos1", 4) == 0) {
    minLength = 1249.5;
    maxLength = 1681.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "eos2", 4) == 0) {
    minLength = 1753.5;
    maxLength = 2185.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "eos3", 4) == 0) {
    minLength = 2257.5;
    maxLength = 2689.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "eos4", 4) == 0) {
    minLength = 2761.5;
    maxLength = 3121.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "eos5", 4) == 0) {
    minLength = 3193.5;
    maxLength = 3553.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "eos6", 4) == 0) {
    minLength = 3625.5;
    maxLength = 3985.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "eel1", 4) == 0) {
    minLength = 3361.5;
    maxLength = 3841.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "eel2", 4) == 0) {
    minLength = 3961.5;
    maxLength = 4441.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "ees1", 4) == 0) {
    minLength = 2014.5;
    maxLength = 2374.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "ees2", 4) == 0) {
    minLength = 2446.5;
    maxLength = 2734.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "bil1", 4) == 0) {
    minLength = 2671.5;
    maxLength = 2671.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bil2", 4) == 0) {
    minLength = 2671.5;
    maxLength = 2671.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bil3", 4) == 0) {
    minLength = 2671.5;
    maxLength = 2671.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bil4", 4) == 0) {
    minLength = 2671.5;
    maxLength = 2671.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bil5", 4) == 0) {
    minLength = 2671.5;
    maxLength = 2671.5;
    ntubes = 30;
  }
  else if (strncmp(chamberName, "bil6", 4) == 0) {
    minLength = 2671.5;
    maxLength = 2671.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bim1", 4) == 0) {
    minLength = 1536.5;
    maxLength = 1536.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bim2", 4) == 0) {
    minLength = 1536.5;
    maxLength = 1536.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bim3", 4) == 0) {
    minLength = 1536.5;
    maxLength = 1536.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bim4", 4) == 0) {
    minLength = 1536.5;
    maxLength = 1536.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bim5", 4) == 0) {
    minLength = 1536.5;
    maxLength = 1536.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bis1", 4) == 0) {
    minLength = 1671.5;
    maxLength = 1671.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bis2", 4) == 0) {
    minLength = 1671.5;
    maxLength = 1671.5;
    ntubes = 30;
  }
  else if (strncmp(chamberName, "bis3", 4) == 0) {
    minLength = 1671.5;
    maxLength = 1671.5;
    ntubes = 30;
  }
  else if (strncmp(chamberName, "bis4", 4) == 0) {
    minLength = 1671.5;
    maxLength = 1671.5;
    ntubes = 30;
  }
  else if (strncmp(chamberName, "bis5", 4) == 0) {
    minLength = 1671.5;
    maxLength = 1671.5;
    ntubes = 30;
  }
  else if (strncmp(chamberName, "bis6", 4) == 0) {
    minLength = 1671.5;
    maxLength = 1671.5;
    ntubes = 30;
  }
  else if (strncmp(chamberName, "bis7", 4) == 0) {
    minLength = 1671.5;
    maxLength = 1671.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bis8", 4) == 0) {
    minLength = 851.5;
    maxLength = 851.5;
    ntubes = 16;
  }
  else if (strncmp(chamberName, "bir1", 4) == 0) {
    minLength = 2671.5;
    maxLength = 2671.5;
    ntubes = 30;
  }
  else if (strncmp(chamberName, "bir2", 4) == 0) {
    minLength = 1536.5;
    maxLength = 1536.5;
    ntubes = 30;
  }
  else if (strncmp(chamberName, "bir3", 4) == 0) {
    minLength = 1105.5;
    maxLength = 1105.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bir4", 4) == 0) {
    minLength = 1536.5;
    maxLength = 1536.5;
    ntubes = 30;
  }
  else if (strncmp(chamberName, "bir5", 4) == 0) {
    minLength = 1536.5;
    maxLength = 1536.5;
    ntubes = 24;
  }
  else if (strncmp(chamberName, "bir6", 4) == 0) {
    minLength = 1105.5;
    maxLength = 1105.5;
    ntubes = 36;
  }
  else if (strncmp(chamberName, "bmf1", 4) == 0) {
    minLength = 3071.5;
    maxLength = 3071.5;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bmf2", 4) == 0) {
    minLength = 3071.5;
    maxLength = 3071.5;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "bmf3", 4) == 0) {
    minLength = 3071.5;
    maxLength = 3071.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "bml1", 4) == 0) {
    minLength = 3551.5;
    maxLength = 3551.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "bml2", 4) == 0) {
    minLength = 3551.5;
    maxLength = 3551.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "bml3", 4) == 0) {
    minLength = 3551.5;
    maxLength = 3551.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "bml4", 4) == 0) {
    minLength = 3551.5;
    maxLength = 3551.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "bml5", 4) == 0) {
    minLength = 3551.5;
    maxLength = 3551.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "bml6", 4) == 0) {
    minLength = 3551.5;
    maxLength = 3551.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "bms1", 4) == 0) {
    minLength = 3071.5;
    maxLength = 3071.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "bms2", 4) == 0) {
    minLength = 3071.5;
    maxLength = 3071.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "bms3", 4) == 0) {
    minLength = 3071.5;
    maxLength = 3071.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "bms4", 4) == 0) {
    minLength = 3071.5;
    maxLength = 3071.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "bms5", 4) == 0) {
    minLength = 3071.5;
    maxLength = 3071.5;
    ntubes = 32;
  }
  else if (strncmp(chamberName, "bms6", 4) == 0) {
    minLength = 3071.5;
    maxLength = 3071.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "bol1", 4) == 0) {
    minLength = 4961.5;
    maxLength = 4961.5;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bol2", 4) == 0) {
    minLength = 4961.5;
    maxLength = 4961.5;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bol3", 4) == 0) {
    minLength = 4961.5;
    maxLength = 4961.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "bol4", 4) == 0) {
    minLength = 4961.5;
    maxLength = 4961.5;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bol5", 4) == 0) {
    minLength = 4961.5;
    maxLength = 4961.5;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bol6", 4) == 0) {
    minLength = 4961.5;
    maxLength = 4961.5;
    ntubes = 56;
  }
  else if (strncmp(chamberName, "bos1", 4) == 0) {
    minLength = 3773.3;
    maxLength = 3773.3;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bos2", 4) == 0) {
    minLength = 3773.3;
    maxLength = 3773.3;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bos3", 4) == 0) {
    minLength = 3773.3;
    maxLength = 3773.3;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bos4", 4) == 0) {
    minLength = 3773.3;
    maxLength = 3773.3;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bos5", 4) == 0) {
    minLength = 3773.3;
    maxLength = 3773.3;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bos6", 4) == 0) {
    minLength = 3773.3;
    maxLength = 3773.3;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "bosx", 4) == 0) {
    minLength = 3773.3;
    maxLength = 3773.3;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bog0", 4) == 0) {
    minLength = 3771.5;
    maxLength = 3771.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "bog2", 4) == 0) {
    minLength = 3771.5;
    maxLength = 3771.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "bog4", 4) == 0) {
    minLength = 3771.5;
    maxLength = 3771.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "bog6", 4) == 0) {
    minLength = 3771.5;
    maxLength = 3771.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "bog8", 4) == 0) {
    minLength = 3771.5;
    maxLength = 3771.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "bof1", 4) == 0) {
    minLength = 3773.5;
    maxLength = 3773.5;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bof2", 4) == 0) {
    minLength = 3773.5;
    maxLength = 3773.5;
    ntubes = 72;
  }
  else if (strncmp(chamberName, "bof3", 4) == 0) {
    minLength = 3773.5;
    maxLength = 3773.5;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "bof4", 4) == 0) {
    minLength = 3773.5;
    maxLength = 3773.5;
    ntubes = 64;
  }
  else if (strncmp(chamberName, "bof5", 4) == 0) {
    minLength = 3773.5;
    maxLength = 3773.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "bof6", 4) == 0) {
    minLength = 3773.5;
    maxLength = 3773.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "bof7", 4) == 0) {
    minLength = 3773.5;
    maxLength = 3773.5;
    ntubes = 40;
  }
  else if (strncmp(chamberName, "bee1", 4) == 0) {
    minLength = 911.5;
    maxLength = 911.5;
    ntubes = 48;
  }
  else if (strncmp(chamberName, "bee2", 4) == 0) {
    minLength = 911.5;
    maxLength = 911.5;
    ntubes = 48;
  }
  else unknownChamber = TRUE;
  
  if (unknownChamber) tubeLength = 0.0;
  else if ((tube > 0) && (tube <= ntubes)) {
    step = (maxLength - minLength) / ((double) ntubes);
    tubeLength = minLength + ((double) (tube - 1)) * step;
  }
  else tubeLength = 0.5 * (minLength+maxLength);
  
  return tubeLength;
}



void EnableMezzCardsAccordingMDTChamber(void) {
  int mezz;
  
  mezzEnables = 0;
  nbMezzCard = 0;
  if ((chamberIndex >= 0) && (mezzControl[chamberIndex] != 0)) {
    mezzEnables = mezzControl[chamberIndex];
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      if (((mezzEnables >> mezz) % 2) == 1) nbMezzCard++;
    }
  }
  else {
    GetNumberMezzAndRow(MDTChamberType, &MDTChamberNumberMezz, &MDTChamberNumberRow);
    if (MDTChamberNumberMezz <= 0) return;
    if ((startMezzCard+MDTChamberNumberMezz) > MAXNUMBERMEZZANINE) startMezzCard = 0;
    for (mezz = startMezzCard; mezz < MDTChamberNumberMezz+startMezzCard; mezz++) {
      nbMezzCard++;
      mezzEnables |= (1 << mezz);
    }
  }
  SaveParameters(PARAMEZZENABLES);
  SetMezzCardEnables();
}



//*****************************************************************************
//
// Fit routines
//
//*****************************************************************************

// Subroutine of GaussFit
/*
void GaussFit(int crd, int chn, int* NDF, float* chisq, float* x0, float* sigma, float* logN0) {
  // Do a Gaussian fit to the noise hits from a threshold sweep
  // Exclude points with lt 10 hits or more than 20% occupancy 
  int npoint, npoint1, npoint2, nmaxpoint, nhitmin = 10, nhitmax;
  int nRun, nhit, i, j, maxhits, nhitsave;
  float X, Y, A[3], sigma2;
  double U[3][3], Uinv[3][3], V[3];
  double W = 0.;

  *NDF = -3;
  for (i = 0; i < 3; i++) {
    V[i] = 0.;
    for(j = 0; j < 3; j++) {
      U[i][j] = 0.;
    }
  }
  nmaxpoint = 3;
  npoint1 = 0;
  nhitsave = 0;
  for (nRun = 0; nRun < (numberScanRun-1)/2; nRun++) {
    nhitmax = 0.15*trigCount[chn/8][nRun];
    nhit = hitCount[crd][chn][nRun];
    if (abs(threshs[chn/8][nRun]) >= 250) break;
    if (nhit > nhitmax) break;
    if (nhit > nhitmin && nhit > nhitsave) {
      npoint1++;
      nhitsave = nhit;
    }
  }
  npoint2 = 0;
  nhitsave = 0;
  for (nRun = numberScanRun-1; nRun >= (numberScanRun-1)/2; nRun--) {
    nhitmax = 0.15*trigCount[chn/8][nRun];
    nhit = hitCount[crd][chn][nRun];
    if (abs(threshs[chn/8][nRun]) >= 250) break;
    if (nhit > nhitmax) break;
    if (nhit > nhitmin && nhit > nhitsave) {
      npoint2++;
      nhitsave = nhit;
    }
  }
  npoint = npoint1;
  if (npoint2 < npoint1) npoint = npoint2;
  if (npoint > nmaxpoint) nmaxpoint = npoint;
  // Sweep over runs from the left side computing sums in arrays U and V
  // Don't record data past the point where nhit>nhitmax or thresh=0
  // Also eliminate points where nhit<nhitsave (previous value)
  maxhits = 0;
  npoint = 0;
  nhitsave = 0;
  for (nRun = 0; nRun < (numberScanRun-1)/2; nRun++) {
    if (npoint >= nmaxpoint) break;
    nhitmax = 0.15*trigCount[chn/8][nRun];
    nhit = hitCount[crd][chn][nRun];
    if (nhit > nhitmax) maxhits = 1;
    if (nhit > nhitmin && maxhits == 0 && nhit > nhitsave && abs(threshs[chn/8][nRun]) < 250) {
      npoint++;
      nhitsave = nhit;
      Y = log(nhit);
      X = threshs[chn/8][nRun];
      U[0][0] = U[0][0] + nhit;
      U[1][0] = U[1][0] + nhit*X;
      U[1][1] = U[1][1] + nhit*X*X;
      U[2][1] = U[2][1] + nhit*X*X*X;
      U[2][2] = U[2][2] + nhit*X*X*X*X;
      V[0] = V[0] + nhit*Y;
      V[1] = V[1] + nhit*Y*X;
      V[2] = V[2] + nhit*Y*X*X;
      W = W + nhit*Y*Y;
      *NDF += 1;
    }
  }
  // Next sweep up the right side with the same conditions
  maxhits = 0;
  npoint = 0;
  nhitsave = 0;
  for (nRun = numberScanRun-1; nRun >= (numberScanRun-1)/2; nRun--) {
    if (npoint >= nmaxpoint) break;
    nhitmax = 0.15*trigCount[chn/8][nRun];
    nhit = hitCount[crd][chn][nRun];
    if (nhit > nhitmax) maxhits = 1;
    if (nhit > nhitmin && maxhits == 0 && nhit > nhitsave && abs(threshs[chn/8][nRun]) < 250) {
      npoint++;
      nhitsave = nhit;
      Y = log(nhit);
      X = threshs[chn/8][nRun];
      U[0][0] = U[0][0] + nhit;
      U[1][0] = U[1][0] + nhit*X;
      U[1][1] = U[1][1] + nhit*X*X;
      U[2][1] = U[2][1] + nhit*X*X*X;
      U[2][2] = U[2][2] + nhit*X*X*X*X;
      V[0] = V[0] + nhit*Y;
      V[1] = V[1] + nhit*Y*X;
      V[2] = V[2] + nhit*Y*X*X;
      W = W + nhit*Y*Y;
      *NDF += 1;
    }
  }
  // Invert and multiply the fit matrices to extract the fit parameters
  U[0][1] = U[1][0];
  U[1][2] = U[2][1];
  U[0][2] = U[1][1];
  U[2][0] = U[1][1];
  InvMatrix(U, 3, Uinv);
  for (i = 0; i < 3; i++) {
    A[i] = 0.;
    for (j = 0; j < 3; j++) {
      A[i] = A[i] + Uinv[i][j]*V[j];
    }
  }
  *sigma = 0.;
  *x0 = 0.;
  *logN0 = 0.;
  if (*NDF >= 0) {
    sigma2 = -0.5/A[2];
    *sigma = sqrt(sigma2);
    *x0 = A[1]*sigma2;
    *logN0 = A[0] + 0.5 * (*x0) * (*x0)/sigma2;
  }
  *chisq = 0.;
  if (*NDF > 0) {
    *chisq = W;
    for (i = 0; i < 3; i++) {
      *chisq = *chisq - 2*A[i]*V[i];
      for (j = 0; j < 3; j++) {
        *chisq = *chisq + A[i]*U[i][j]*A[j];
      }
    }
    *chisq = *chisq / *NDF;
  }
}


*/

// Subroutine of InjEff
void InjEff(int crd, int chn, float* eff, float* xtalk) {
  // Find the injection efficiency for a channel and
  // find the xtalk in the other channels in the mezzanine card
  int nRun, ichn, khit, mhit, nhit;

  *eff = 0.;
  *xtalk = 0.;
  for (nRun = 0; nRun < numberScanRun; nRun++) {
    if ((injMasks[nRun] >> chn) & 0x01) {
      khit = 0;
      mhit = 0;
      for (ichn = 0; ichn < 24; ichn++) {
       	nhit = hitCount[crd][ichn][nRun];
        khit = khit + nhit;
        if (ichn == chn) mhit = nhit;
      }
      *eff = mhit/(float)trigCount[chn/8][nRun];
      *xtalk = (khit-mhit)/(float)trigCount[chn/8][nRun];
    }
  }
}


int FitThresholdScanResults(void) {
  int checkStatus, i, j, scanTime, mezz, ch, NDF, offset, status, oldStatus, cstatus, resp;
  int thre, nhit, nerr, ntrig;
  float chisq, x0, sigma, logN0;
  double voff1[24], voff2[24], ceffi0, ceffi1, ceffi2, ceffi3;
  char str[5120];
  
  mezzCoeffiCut = 0.70;
  ASDCoeffiCut = 0.70;
  threScanNDFCut = 0;
  VOffsetCut = 16.0;
  threScanSigmaCut = 10.0;
  checkStatus = 0;
  if (numberScanRun < 6) {
    sprintf(str, "*********!!! Abnormal Scan Run !!!********\n"
                 "Not enough threshold scan fit points (= %d), no fit will be performed\n", numberScanRun);
    MDTChamberOutput2("%s", str);
    return 0;
  }
  // Order the hitcount according to threshold
  for (i = 0; i < numberScanRun-1; i++) {
    for (j = i+1; j < numberScanRun; j++) {
      if (threshs[0][j] < threshs[0][i]) {
        thre = threshs[0][i];
        threshs[0][i] = threshs[0][j];
        threshs[0][j] = thre;
        for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
          for (ch = 0; ch < 8; ch++) {
            nhit = hitCount[mezz][ch][i];
            hitCount[mezz][ch][i] = hitCount[mezz][ch][j];
            hitCount[mezz][ch][j] = nhit;
          }
          nerr = errCount[mezz][0][i];
          errCount[mezz][0][i] = errCount[mezz][0][j];
          errCount[mezz][0][j] = nerr;
        }
        ntrig = trigCount[0][i];
        trigCount[0][i] = trigCount[0][j];
        trigCount[0][j] = ntrig;
      }
      if (threshs[1][j] < threshs[1][i]) {
        thre = threshs[1][i];
        threshs[1][i] = threshs[1][j];
        threshs[1][j] = thre;
        for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
          for (ch = 8; ch < 16; ch++) {
            nhit = hitCount[mezz][ch][i];
            hitCount[mezz][ch][i] = hitCount[mezz][ch][j];
            hitCount[mezz][ch][j] = nhit;
          }
          nerr = errCount[mezz][1][i];
          errCount[mezz][1][i] = errCount[mezz][1][j];
          errCount[mezz][1][j] = nerr;
        }
        ntrig = trigCount[1][i];
        trigCount[1][i] = trigCount[1][j];
        trigCount[1][j] = ntrig;
      }
      if (threshs[2][j] < threshs[2][i]) {
        thre = threshs[2][i];
        threshs[2][i] = threshs[2][j];
        threshs[2][j] = thre;
        for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
          for (ch = 16; ch < 24; ch++) {
            nhit = hitCount[mezz][ch][i];
            hitCount[mezz][ch][i] = hitCount[mezz][ch][j];
            hitCount[mezz][ch][j] = nhit;
          }
          nerr = errCount[mezz][2][i];
          errCount[mezz][2][i] = errCount[mezz][2][j];
          errCount[mezz][2][j] = nerr;
        }
        ntrig = trigCount[2][i];
        trigCount[2][i] = trigCount[2][j];
        trigCount[2][j] = ntrig;
      }
    }
  }

  // Try to fit scanned results
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    sprintf(str, "MDT Chamber HV Status               = %30d\n", MDTChamberHVStatus);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput2("Threshold scan test end on %s\n", WeekdayTimeString(time(NULL)));
  scanTime = ((int) time(NULL)) - thresholdScanStartTime;
  MDTChamberOutput2("Mezzanine Card Threshold Scan Results (total scan time = %d (s))\n", scanTime);
  MDTChamberOutput1("//\n");
  MDTChamberOutput1("// --- Description for Threshold Scan Test ---\n");
  MDTChamberOutput1("//     The intrinsic noise of each channel of ASD chip is threshold dependent,\n"); 
  MDTChamberOutput1("//   which is called ASD noise in MiniDAQ and is Gaussian distribution as a\n"); 
  MDTChamberOutput1("//   function of its threshold:\n"); 
  MDTChamberOutput1("//         ASDNoiseRate = R0 * EXP(-(V-VOff)**2/(2.0*Sigma**2))\n"); 
  MDTChamberOutput1("//   where R0 is the maximum ASD noise rate (typical value 20MHz);\n"); 
  MDTChamberOutput1("//         V is the ASD threshold;\n"); 
  MDTChamberOutput1("//         VOff is the threshold offset, it is refered as V Offset;\n"); 
  MDTChamberOutput1("//         Sigma is the Gaussion width of ASD noise distribution.\n"); 
  MDTChamberOutput1("//   Threshold scan test measures mezzanine card noise with different threshold\n"); 
  MDTChamberOutput1("//   and measured results are fitted to a Gaussion distribution to obtain V Offset.\n"); 
  MDTChamberOutput1("//   Note: due to readout limitation of the AMT chip (0.3MHz/channel), only the tail\n"); 
  MDTChamberOutput1("//         from threshold scan test are used for the fit.\n"); 
  MDTChamberOutput1("//   24 channels V offset from a mezzanine card are the finger-print of the mezzanine\n"); 
  MDTChamberOutput1("//   card, which could be used to identify the mezzanine card. To install mezzanine cards\n"); 
  MDTChamberOutput1("//   to MDT chamber, this test is essential to insure that the recorded mezzanine card IDs\n"); 
  MDTChamberOutput1("//   are correct for their location on a MDT chamber by comparing V offsets from this test\n"); 
  MDTChamberOutput1("//   with Harvard mezzanine card/ASD database.\n"); 
  MDTChamberOutput1("//   Note: large noise not due to mezzanine card (such as from MDT chamber) could spoil the\n"); 
  MDTChamberOutput1("//         V offset measurement, typically with large CH2 and abnorml sigam.\n"); 
  MDTChamberOutput1("//\n"); 
  MDTChamberOutput1("// Board              : Mezzanine card number\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("// Mezz.Serial-Number : Mezzanine card serial number (ID)\n");
  }
  MDTChamberOutput1("// Chan               : Channel number\n"); 
  MDTChamberOutput1("// NTrigs             : Number of triggers\n"); 
  MDTChamberOutput1("// NDF                : Number of degree freedom\n"); 
  MDTChamberOutput1("// Xsq/DF             : CH2 per degree freedom\n"); 
  MDTChamberOutput1("// Voffset            : V offset\n"); 
  MDTChamberOutput1("// Sigma              : Sigma of the noise distribution\n"); 
  MDTChamberOutput1("// logN0              : log(N0)\n"); 
  MDTChamberOutput1("//\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("Board   Mezz.Serial-Number Chan  NTrigs  NDF    Xsq/DF   VOffset     Sigma     logN0\n");
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      if (((mezzEnables>>mezz)&1) == 1) {
        GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
        for (ch = 0; ch < 24; ch++) {
//          GaussFit(mezz, ch, &NDF,  &chisq, &x0, &sigma, &logN0);
          sprintf(str, "%5d %20s %4d %7d %4d %9.5f %9.5f %9.5f %9.5f\n",
                  mezz, serialNumber, ch, trigCount[0][0], NDF, chisq, x0, sigma, logN0);
          MDTChamberOutput2("%s", str);
          thresholdScanResults[0][ch][mezz] = x0;
          thresholdScanResults[1][ch][mezz] = sigma;
          thresholdScanResults[2][ch][mezz] = logN0;
          thresholdScanResults[3][ch][mezz] = chisq;
          thresholdScanResults[4][ch][mezz] = NDF;
        }
      }
    }
  }
  else {
    MDTChamberOutput1("Board Chan  NTrigs  NDF    Xsq/DF   VOffset     Sigma     logN0\n");
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      if (((mezzEnables>>mezz)&1) == 1) {
        for (ch = 0; ch < 24; ch++) {
//          GaussFit(mezz, ch, &NDF, &chisq, &x0, &sigma, &logN0);
          sprintf(str, "%5d %4d %7d %4d %9.5f %9.5f %9.5f %9.5f\n",
                  mezz, ch, trigCount[0][0], NDF, chisq, x0, sigma, logN0);
          MDTChamberOutput2("%s", str);
          thresholdScanResults[0][ch][mezz] = x0;
          thresholdScanResults[1][ch][mezz] = sigma;
          thresholdScanResults[2][ch][mezz] = logN0;
          thresholdScanResults[3][ch][mezz] = chisq;
          thresholdScanResults[4][ch][mezz] = NDF;
        }
      }
    }
  }
  
  MDTChamberOutput1("//\n"); 
  MDTChamberOutput1("// Board              : Mezzanine card number\n"); 
  MDTChamberOutput1("// Mezz.Serial-Number : Mezzanine card serial number (ID)\n");
  MDTChamberOutput1("// MezzCEffi          : Mezzanine card coefficiency (all readout channels)\n");
  MDTChamberOutput1("// ASD1CEffi          : ASD1 coefficiency (readout channel  0 to  7)\n");
  MDTChamberOutput1("// ASD2CEffi          : ASD2 coefficiency (readout channel  8 to 15)\n");
  MDTChamberOutput1("// ASD3CEffi          : ASD3 coefficiency (readout channel 16 to 23)\n");
  MDTChamberOutput1("//\n"); 
  MDTChamberOutput1("Board   Mezz.Serial-Number MezzCEffi ASD1CEffi ASD2CEffi ASD3CEffi\n");
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 1) {
      for (ch = 0; ch < 24; ch++) {
        voff1[ch] = thresholdScanResults[0][ch][mezz];
        voff2[ch] = benchThresholdScan[0][ch][mezz];
      }
      coeffiFromThresholdScan[0][mezz] = 0.0;
      coeffiFromThresholdScan[1][mezz] = 0.0;
      coeffiFromThresholdScan[2][mezz] = 0.0;
      coeffiFromThresholdScan[3][mezz] = 0.0;
	  if (getVOff[mezz]) {
        coeffiFromThresholdScan[0][mezz] = CorrelationEffi(voff1, voff2, 24);
        coeffiFromThresholdScan[1][mezz] = CorrelationEffi(&voff1[0], &voff2[0], 8);
        coeffiFromThresholdScan[2][mezz] = CorrelationEffi(&voff1[8], &voff2[8], 8);
        coeffiFromThresholdScan[3][mezz] = CorrelationEffi(&voff1[16], &voff2[16], 8);
	  }
      GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
      sprintf(str, "%5d %20s %9.3f %9.3f %9.3f %9.3f\n", mezz, serialNumber,
              coeffiFromThresholdScan[0][mezz], coeffiFromThresholdScan[1][mezz],
              coeffiFromThresholdScan[2][mezz], coeffiFromThresholdScan[3][mezz]);
      MDTChamberOutput2("%s", str);
    }
  }
  
  if (strlen(MDTChamberWarningFilename) > 0) {
    status = GetFileInfo(MDTChamberWarningFilename, &i);
    if (status > 0) {
      MDTChamberWarningFile = fopen(MDTChamberWarningFilename, "a");
      if (MDTChamberWarningFile != NULL) {
        fprintf(MDTChamberWarningFile, "\n");
        fprintf(MDTChamberWarningFile, "Checking threshold scan test results at %s\n", WeekdayTimeString(time(NULL)));
      }
    }
  }
  MDTChamberOutput1("// Message will be issued at either of following conditions:\n");
  MDTChamberOutput1("// * <Warning> Unable to find mezzanine card in mezzanine card database;\n");
  sprintf(str, "// * <Warning> Mezzanine coefficiency is less than %.3f\n", mezzCoeffiCut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <Warning> Any ASD coefficiency is less than %.3f;\n", ASDCoeffiCut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <Warning> Number degree freedom of gaussion fit is less than %.0f;\n", threScanNDFCut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <-Infor-> Absolute value of V offset is large than %.0f;\n", VOffsetCut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <-Infor-> Sigma from the Gaussion fit is large than %.0f.\n", threScanSigmaCut);
  MDTChamberOutput2("%s", str);
  MDTChamberOutput1("//\n"); 
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 1) {
      ceffi0 = coeffiFromThresholdScan[0][mezz];
      ceffi1 = coeffiFromThresholdScan[1][mezz];
      ceffi2 = coeffiFromThresholdScan[2][mezz];
      ceffi3 = coeffiFromThresholdScan[3][mezz];
      if (!getVOff[mezz]) {
        cstatus = MEZZNOTINDATABASE;
        sprintf(str, "<WARNING> Mezzanine Card (mezz = %2d) is not found in database file.\n", mezz, ch);
        MDTChamberOutput2("%s", str);
      }
      else if (ceffi0 < mezzCoeffiCut) {
        cstatus = MEZZLOWCOEFFI;
        sprintf(str, "<WARNING> Low coefficiency %.3f for mezzanine card %2d.\n", ceffi0, mezz);
        MDTChamberOutput2("%s", str);
      }
      else if (ceffi1 < ASDCoeffiCut) {
        cstatus = MEZZASD1LOWCOEFFI;
        sprintf(str, "<WARNING> Low coefficiency %.3f for ASD1 of mezzanine card %2d.\n", ceffi1, mezz);
        MDTChamberOutput2("%s", str);
      }
      else if (ceffi2 < ASDCoeffiCut) {
        cstatus = MEZZASD2LOWCOEFFI;
        sprintf(str, "<WARNING> Low coefficiency %.3f for ASD2 of mezzanine card %2d.\n", ceffi2, mezz);
        MDTChamberOutput2("%s", str);
      }
      else if (ceffi3 < ASDCoeffiCut) {
        cstatus = MEZZASD3LOWCOEFFI;
        sprintf(str, "<WARNING> Low coefficiency %.3f for ASD3 of mezzanine card %2d.\n", ceffi3, mezz);
        MDTChamberOutput2("%s", str);
      }
      else cstatus = MEZZOK;
      if (cstatus != MEZZOK) checkStatus = cstatus;
      for (ch = 0; ch < 24; ch++) {
        x0 = thresholdScanResults[0][ch][mezz];
        sigma = thresholdScanResults[1][ch][mezz];
        chisq = thresholdScanResults[3][ch][mezz];
        NDF = thresholdScanResults[4][ch][mezz];
        if (NDF < threScanNDFCut) {
          status = RDOUTCHDEAD;
          sprintf(str, "<WARNING> Dead or Very Noise Readout channel (mezz = %2d channel = %2d)\n", mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (ABS(x0) > VOffsetCut) {
          status = RDOUTCHABNORMALOFFSET;
          sprintf(str, "<-INFOR-> Abnormal threshold offset value %9.2f (mezz = %2d channel = %2d)\n", x0, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (sigma >= threScanSigmaCut) {
          status = RDOUTCHABNORMALSIGMA;
          sprintf(str, "<-INFOR-> Abnormal sigma %9.2f (mezz = %2d channel = %2d)\n", sigma, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else status = RDOUTCHOK;
        if (status != RDOUTCHOK) checkStatus = status;
        if (MDTChamberHVStatus == MDTCHAMBERHVON) {
          oldStatus = channelStatusHVOn[ch][mezz] - offset;
          if (oldStatus == RDOUTCHOK) channelStatusHVOn[ch][mezz] = status + offset;
        }
        else {
          oldStatus = channelStatusHVOff[ch][mezz] - offset;
          if (oldStatus == RDOUTCHOK) channelStatusHVOff[ch][mezz] = status + offset;
        }
      }
    }
  }
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    sprintf(str, "MDT Chamber HV Status               = %30d\n", MDTChamberHVStatus);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput2("Total Number of Threshold Scan Runs = %30d\n", numberScanRun*numberSubScans);
  MDTChamberOutput2("Number of Threshold Scan Fit Points = %30d\n", numberScanRun);
  MDTChamberOutput2("Number Ignored Threshold Scan Runs  = %30d\n", numberIgnoredScanRun);
  MDTChamberOutput2("Number Bad Threshold Scan Runs      = %30d\n", numberBadScanRun);
  if (numberBadScanRun > 0) {
    if (scanControl == 1) {
      sprintf(str, "There are two many bad runs (= %u) during threshold scan.\n"
                   "Please check the scan file <%s> for details.\n"
                   "Possible reasons:\n"
                   "  1) Problem in Linux Box => Need reboot Linux Box and redo scan tests;\n"
                   "  2) Hardware problems => Need find out by you and redo scan tests;\n"
                   "  3) Unknown reasons => You have to make a judgement.\n"
                   "\nAre you sure to continue?\n",
                   numberBadScanRun, MDTChamberSumFilename);
      resp = ConfirmPopup("Too Many Unusable Runs in Threshold Scan", str);
      if (resp == 1) {
        MDTChamberOutput1("Confirm to continue ..\n");
      }
      else {
        checkStatus = 1;
        return checkStatus;
      }
    }
    else {
      sprintf(str, "*********!!! Too many bad runs (=%u) during threshold scan !!!********.\n"
                   "Possible reasons:\n"
                   "  1) Problem in Linux Box => Need reboot Linux Box and redo scan tests;\n"
                   "  2) Hardware problems => Need find out by you and redo scan tests;\n"
                   "  3) Unknown reasons => You have to make a judgement.\n"
                   "Continue auto run without warning to user\n",
                   numberBadScanRun);
      MDTChamberOutput2("%s", str);
    }
  }
  if (MDTChamberWarningFile != NULL) {
    if ((scanControl == 1) && (checkStatus != 0)) {
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
      strcpy(str, "");
      sprintf(str, "notepad %s", MDTChamberWarningFilename);
      status = LaunchExecutable(str);
      if (status != 0) status = LaunchExecutable(str);
    }
    else {
      if (checkStatus == 0) {
        fprintf(MDTChamberWarningFile, "All readout channels are normal in threshold scan test\n");
      }
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
    }
  }
  if ((scanControl == 1) && (checkStatus != 0)) {
    if (MDTChamberHVStatus == MDTCHAMBERHVON) {
      sprintf(str, "Dead or abnormal readout channels have been detected from threshold scan,\n"
                   "where chamber HV is on, possible sources:\n"
                   "  1) Bad Mezzanine Card Card;\n"
                   "  2) Signal or HV Hedghog Card;\n"
                   "  3) Very noise chamber tube;\n"
                   "Please look corresponding mezzanine card(s) in cosmic ray data,;\n"
                   "if abnormal behaviour is observed in cosmic ray data (such as very hot channel);\n"
                   "replace corresponding mezzanine card(s) and re-do all necessary tests.;\n"
                   "\nAre you sure to continue?\n");
    }
    else {
      sprintf(str, "Dead or abnormal readout channels have been detected from threshold scan,\n"
                   "where chamber HV is off. Most likely the corresponding mezzanine card has problem.\n"
                   "If bad correlation observed, please make sure corresponding mezzanine card\n"
                   "barcode is right, otherwise replace corresponding mezzanine card(s) and re-do tests.\n"
                   "\nAre you sure to continue?\n");
    }
    resp = ConfirmPopup("Dead or Abnormal Readout Channel", str);
    if (resp == 1) {
      checkStatus = 0;
      MDTChamberOutput1("Confirm to continue ...\n");
    }
    else checkStatus = 1;
  }
  else if ((scanControl == 0) && (checkStatus != 0)) {
    sprintf(str, "*********!!! Abnormal Results in Scan Run !!!********\n"
                 "Dead or abnormal readout channels have been detected from threshold scan.\n"
                 "Most likely the corresponding mezzanine card has problem.\n"
                 "Continue auto run without warning to user\n");
    MDTChamberOutput2("%s", str);
    checkStatus = 0;
  }
  
  return checkStatus;
}


int FitInjectionScanResults(void) {
  int checkStatus, scanTime, mezz, ch, offset, status, oldStatus, resp;
  float eff, xtalk;
  char str[5120];
  
  injEffiCut0 = 0.05;
  injEffiCut1 = 0.95;
  injEffiCut2 = 1.05;
  injEffiCut3 = 2.00;
  injXTalkCut0 = 0.20;
  injXTalkCut1 = 0.05;
  checkStatus = 0;
  if (numberScanRun < 3) {
    sprintf(str, "*********!!! Abnormal Scan Run !!!********\n"
                 "Not enough injection scan runs (= %d), no fit will be performed\n", numberScanRun);
    MDTChamberOutput2("%s", str);
    return 0;
  }
  if (MDTChamberHVStatus == MDTCHAMBERHVON) offset = RDOUTSTATUSOFFSETHVON;
  else  offset = RDOUTSTATUSOFFSETHVOFF;
  // Try to fit scanned results
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    sprintf(str, "MDT Chamber HV Status               = %30d\n", MDTChamberHVStatus);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput2("Injection scan test end on %s\n", WeekdayTimeString(time(NULL)));
  scanTime = ((int) time(NULL)) - injectionScanStartTime;
  MDTChamberOutput2("Mezzanine Card Injection Scan Results (total scan time = %d (s))\n", scanTime);
  MDTChamberOutput1("//\n");
  MDTChamberOutput1("// --- Description for Injection Scan Test ---\n");
  MDTChamberOutput1("//     The calibration signal is injected to mezzanine card channel by channel,\n"); 
  MDTChamberOutput1("//   where hit are expected only in injected channel. The channel efficiency is\n"); 
  MDTChamberOutput1("//   defined as number actual hits of injected channel over number of injections,\n"); 
  MDTChamberOutput1("//   and the channel crosstalk is defined as number actual hits in all other channels\n"); 
  MDTChamberOutput1("//   where R0 is the maximum ASD noise rate (typical value 20MHz);\n"); 
  MDTChamberOutput1("//   over number of injections.\n"); 
  MDTChamberOutput1("//   Note: noise could give fake crosstalk.\n"); 
  MDTChamberOutput1("//\n"); 
  MDTChamberOutput1("// Board              : Mezzanine card number\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("// Mezz.Serial-Number : Mezzanine card serial number (ID)\n");
  }
  MDTChamberOutput1("// Chan               : Channel number\n"); 
  MDTChamberOutput1("// NTrigs             : Number of triggers/injections\n"); 
  MDTChamberOutput1("// InjEff             : Channel efficiency obtained from injection test\n"); 
  MDTChamberOutput1("// X-Talk             : Channel crosstalk\n"); 
  MDTChamberOutput1("//\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("Board   Mezz.Serial-Number Chan  NTrigs    InjEff    X-Talk\n");
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      if (((mezzEnables>>mezz)&1) == 1) {
        GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
        for (ch = 0; ch < 24; ch++) {
          InjEff(mezz, ch, &eff, &xtalk);
          sprintf(str, "%5d %20s %4d %7d %9.5f %9.5f\n",
                  mezz, serialNumber, ch, trigCount[0][0], eff, xtalk);
          MDTChamberOutput2("%s", str);
          injectionScanResults[0][ch][mezz] = eff;
          injectionScanResults[1][ch][mezz] = xtalk;
        }
      }
    }
   }
  else {
    MDTChamberOutput1("Board Chan  NTrigs    InjEff    X-Talk\n");
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      if (((mezzEnables>>mezz)&1) == 1) {
        for (ch = 0; ch < 24; ch++) {
          InjEff(mezz, ch, &eff, &xtalk);
          sprintf(str, "%5d %4d %7d %9.5f %9.5f\n",
                  mezz, ch, trigCount[0][0], eff, xtalk);
          MDTChamberOutput2("%s", str);
          injectionScanResults[0][ch][mezz] = eff;
          injectionScanResults[1][ch][mezz] = xtalk;
        }
      }
    }
  }
  
  if (strlen(MDTChamberWarningFilename) > 0) {
    status = GetFileInfo(MDTChamberWarningFilename, &resp);
    if (status > 0) {
      MDTChamberWarningFile = fopen(MDTChamberWarningFilename, "a");
      if (MDTChamberWarningFile != NULL) {
        fprintf(MDTChamberWarningFile, "\n");
        fprintf(MDTChamberWarningFile, "Checking injection scan test results at %s\n", WeekdayTimeString(time(NULL)));
      }
    }
  }
  MDTChamberOutput1("// Message will be issued at either of following conditions:\n");
  sprintf(str, "// * <Warning> Dead readout channel            : efficiency is equal or less than %.3f;\n", injEffiCut0);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <Warning> Low efficiency readout channel  : %.3f < Efficiency <= %.3f;\n", injEffiCut0, injEffiCut1);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <Warning> Hot readout channel             : %.3f <= Efficiency < %.3f;\n", injEffiCut2, injEffiCut3);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <Warning> Very hot readout channel        : efficiency is equal or larger than %.3f;\n", injEffiCut3);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <Warning> Large Crosstalk readout channel : xtalk >= %.3f for channel 0, 8 and 16. Other channels >= %.3f.\n", injXTalkCut0, injXTalkCut1);
  MDTChamberOutput2("%s", str);
  MDTChamberOutput1("//\n"); 
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 1) {
      for (ch = 0; ch < 24; ch++) {
        eff = injectionScanResults[0][ch][mezz];
        xtalk = injectionScanResults[1][ch][mezz];
        if (eff <= injEffiCut0) {
          status = RDOUTCHDEAD;
          sprintf(str, "<WARNING> Dead Readout channel (effi = %.2f) (mezz = %2d channel = %2d)\n", eff, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (eff <= injEffiCut1) {
          status = RDOUTCHLOWEFFICIENCY;
          sprintf(str, "<WARNING> Low efficiency (%.2f) channel (mezz = %2d channel = %2d)\n", eff, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (eff >= injEffiCut3) {
          status = RDOUTCHHOT;
          sprintf(str, "<WARNING> Very Hot channel (effi = %6.2f) (mezz = %2d channel = %2d)\n", eff, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (eff >= injEffiCut2) {
          status = RDOUTCHHOT;
          sprintf(str, "<WARNING> Hot channel (effi = %6.2f) (mezz = %2d channel = %2d)\n", eff, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (((ch%8) == 0) && (xtalk >= injXTalkCut0)) {
          status = RDOUTCHHASCROSSTALK;
          sprintf(str, "<WARNING> Cross talk (xtalk = %5.2f) channel (mezz = %2d channel = %2d)\n", xtalk, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (((ch%8) != 0) && (xtalk >= injXTalkCut1)) {
          status = RDOUTCHHASCROSSTALK;
          sprintf(str, "<WARNING> Cross talk (xtalk = %5.2f) channel (mezz = %2d channel = %2d)\n", xtalk, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else status = RDOUTCHOK;
        if (status != RDOUTCHOK) checkStatus = status;
        if (MDTChamberHVStatus == MDTCHAMBERHVON) {
          oldStatus = channelStatusHVOn[ch][mezz] - offset;
          if (oldStatus == RDOUTCHOK) channelStatusHVOn[ch][mezz] = status + offset;
        }
        else {
          oldStatus = channelStatusHVOff[ch][mezz] - offset;
          if (oldStatus == RDOUTCHOK) channelStatusHVOff[ch][mezz] = status + offset;
        }
      }
    }
  }
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    sprintf(str, "MDT Chamber HV Status               = %30d\n", MDTChamberHVStatus);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput2("Total Number of Injection Scan Runs = %30d\n", numberScanRun);
  MDTChamberOutput2("Number Ignored Injection Scan Runs  = %30d\n", numberIgnoredScanRun);
  MDTChamberOutput2("Number Bad Injection Scan Runs      = %30d\n", numberBadScanRun);
  if (numberBadScanRun > 0) {
    if (scanControl == 1) {
      sprintf(str, "There are two many bad runs (= %u) during injection scan.\n"
                   "Please check the scan file <%s> for details.\n"
                   "Possible reasons:\n"
                   "  1) Problem in Linux Box => Need reboot Linux Box and redo scan tests;\n"
                   "  2) Hardware problems => Need find out by you and redo scan tests;\n"
                   "  3) Unknown reasons => You have to make a judgement.\n"
                   "\nAre you sure to continue?\n",
                   numberBadScanRun, MDTChamberSumFilename);
      resp = ConfirmPopup("Too Many Unusable Runs in Threshold Scan", str);
      if (resp == 1) {
        MDTChamberOutput1("Confirm to continue ..\n");
      }
      else {
        checkStatus = 1;
        return checkStatus;
      }
    }
    else {
      sprintf(str, "*********!!! Too many bad runs (=%u) during injection scan !!!********.\n"
                   "Possible reasons:\n"
                   "  1) Problem in Linux Box => Need reboot Linux Box and redo scan tests;\n"
                   "  2) Hardware problems => Need find out by you and redo scan tests;\n"
                   "  3) Unknown reasons => You have to make a judgement.\n"
                   "Continue auto run without warning to user\n",
                   numberBadScanRun);
      MDTChamberOutput2("%s", str);
    }
  }
  if (MDTChamberWarningFile != NULL) {
    if ((scanControl == 1) && (checkStatus != 0)) {
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
      strcpy(str, "");
      sprintf(str, "notepad %s", MDTChamberWarningFilename);
      status = LaunchExecutable(str);
      if (status != 0) status = LaunchExecutable(str);
    }
    else {
      if (checkStatus == 0) {
        fprintf(MDTChamberWarningFile, "All readout channels are normal in injection scan test\n");
      }
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
    }
  }
  if ((scanControl == 1) && (checkStatus != 0)) {
    if (MDTChamberHVStatus == MDTCHAMBERHVON) {
      sprintf(str, "Dead or abnormal readout channels have been detected from injection scan\n"
                   "Possible sources:\n"
                   "  1) Bad Mezzanine Card Card;\n"
                   "  2) Signal or HV Hedghog Card;\n"
                   "  3) Very noise chamber tube;\n"
                   "\nAre you sure to continue?\n");
    }
    else {
      sprintf(str, "Dead or abnormal readout channels have been detected from injection scan\n"
                   "Most likely the corresponding mezzanine card has problem.\n"
                   "\nAre you sure to continue?\n");
    }
    resp = ConfirmPopup("Dead or Abnormal Readout Channel", str);
    if (resp == 1) {
      checkStatus = 0;
      MDTChamberOutput1("Confirm to continue ...\n");
    }
    else checkStatus = 1;
  }
  else if ((scanControl == 0) && (checkStatus != 0)) {
    sprintf(str, "*********!!! Abnormal Results in Scan Run !!!********\n"
                 "Dead or abnormal readout channels have been detected from injection scan.\n"
                 "Most likely the corresponding mezzanine card has problem.\n"
                 "Continue auto run without warning to user\n");
    MDTChamberOutput2("%s", str);
    checkStatus = 0;
  }
  
  return checkStatus;
}


int FitLinearityScanResults(void) {
  int checkStatus, scanTime, mezz, ch, offset, i, status, oldStatus, resp;
  double sigma, maxSigma, minSigma, frac, maxFrac, minFrac;
  double x[200], y[200], z[200], slope, intercept, mse;
  char str[5120];
  
  linMSErrorCut = 0.0;
  minTimeSigmaCut = 0.0;
  timeSigmaCut = 3.0;
  linSlopeCut = 0.01;
  checkStatus = 0;
  if (numberScanRun < 3) {
    sprintf(str, "*********!!! Abnormal Scan Run !!!********\n"
                 "Not enough linearity scan runs (= %d), no fit will be performed\n", numberScanRun);
    MDTChamberOutput2("%s", str);
    return 0;
  }
  if (MDTChamberHVStatus == MDTCHAMBERHVON) offset = RDOUTSTATUSOFFSETHVON;
  else  offset = RDOUTSTATUSOFFSETHVOFF;
  // Try to fit scanned results
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    sprintf(str, "MDT Chamber HV Status               = %30d\n", MDTChamberHVStatus);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput2("Linearity scan test end on %s\n", WeekdayTimeString(time(NULL)));
  scanTime = ((int) time(NULL)) - linearityScanStartTime;
  MDTChamberOutput2("Mezzanine Card Linearity Scan Results (total scan time = %d (s))\n", scanTime);
  MDTChamberOutput1("//\n");
  MDTChamberOutput1("// --- Description for Linearity Scan Test ---\n");
  MDTChamberOutput1("//     The calibration signals are injected at different time with respect to\n"); 
  MDTChamberOutput1("//   calibration trigger. The average TDC time and TDC time resolution are\n"); 
  MDTChamberOutput1("//   obtained at each injection, and the measured TDC times are fitted against\n"); 
  MDTChamberOutput1("//   expected by using a lineary function where the absolute value of fitted slope\n");
  MDTChamberOutput1("//   should be 1. This test measures each readout channel's intrinsic time resolution\n"); 
  MDTChamberOutput1("//   and the time linearity of a mezzanine card, where typically intrinsic time resolution\n"); 
  MDTChamberOutput1("//   is below 1 count and linearity (|(|slope| - 1.)|) is better than 10 to -4.\n"); 
  MDTChamberOutput1("//   Note: Huge noise could spoil the measurements.\n"); 
  MDTChamberOutput1("//\n"); 
  MDTChamberOutput1("// Board              : Mezzanine card number\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("// Mezz.Serial-Number : Mezzanine card serial number (ID)\n");
  }
  MDTChamberOutput1("// Chan               : Channel number\n"); 
  MDTChamberOutput1("// calTrigDelay       : Calibration trigger delay in CSM clock tick\n"); 
  MDTChamberOutput2("// tdcTime            : Measured (average) TDC time in count (1 count = %.5f ns)\n", widthBin); 
  MDTChamberOutput2("// resolution         : TDC time resolution in count (1 count = %.5f ns)\n", widthBin); 
  MDTChamberOutput1("// fracUsed           : Fraction of hits are used to compute TDC time resolution\n"); 
  MDTChamberOutput1("//\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("Board   Mezz.Serial-Number Chan calTrigDelay   tdcTime resolution fracUsed\n");
  }
  else {
    MDTChamberOutput1("Board Chan calTrigDelay   tdcTime resolution fracUsed\n");
  }
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 1) {
      GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
      for (ch = 0; ch < 24; ch++) {
        for (i = 0; i < numberScanRun; i++) {
          x[i] = measuredTDCTime[i][ch][mezz];
          sigma = measuredTDCTimeResolution[i][ch][mezz];
          frac = fracUsedInMeasureTDCTime[i][ch][mezz];
          if (scanSerialNumber || MDTChamberTest) {
            sprintf(str, "%5d %20s %4d %12d %9.2f %10.3f %8.3f\n",
                    mezz, serialNumber, ch, calibTrigDelay[i], x[i], sigma, frac);
          }
          else {
            sprintf(str, "%5d %4d %12d %9.2f %10.3f %8.3f\n",
                    mezz, ch, calibTrigDelay[i], x[i], sigma, frac);
          }
          MDTChamberOutput2("%s", str);
        }
      }
    }
  }
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    sprintf(str, "MDT Chamber HV Status               = %30d\n", MDTChamberHVStatus);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput1("//\n"); 
  MDTChamberOutput1("// Board              : Mezzanine card number\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("// Mezz.Serial-Number : Mezzanine card serial number (ID)\n");
  }
  MDTChamberOutput1("// Chan               : Channel number\n"); 
  MDTChamberOutput1("// slope              : Slope of fitted lineary function\n"); 
  MDTChamberOutput1("// intercept          : Intercept of fitted lineary function\n"); 
  MDTChamberOutput1("// MSError            : Mean square error\n"); 
  MDTChamberOutput2("// minRes             : Minimum TDC time resolution in count (1 count = %.5f ns)\n", widthBin); 
  MDTChamberOutput2("// maxRes             : Maximum TDC time resolution in count (1 count = %.5f ns)\n", widthBin); 
  MDTChamberOutput1("// minFrac            : Minimum fraction of used hits to compute TDC time resolution\n"); 
  MDTChamberOutput1("// maxFrac            : Maximum fraction of used hits to compute TDC time resolution\n"); 
  MDTChamberOutput1("//\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("Board   Mezz.Serial-Number Chan     slope  intercept   MSError  minRes  maxRes minFrac maxFrac\n");
  }
  else {
    MDTChamberOutput1("Board Chan     slope  intercept   MSError  minRes  maxRes minFrac maxFrac\n");
  }
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 1) {
      GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
      for (ch = 0; ch < 24; ch++) {
        minSigma = 1.0E6;
        maxSigma = -1.0;
        minFrac = 1.0E6;
        maxFrac = -1.0;
        for (i = 0; i < numberScanRun; i++) {
          y[i] = 32.0 * calibTrigDelay[i];
          x[i] = measuredTDCTime[i][ch][mezz];
          sigma = measuredTDCTimeResolution[i][ch][mezz];
          if (sigma < minSigma) minSigma = sigma;
          if (sigma > maxSigma) maxSigma = sigma;
          frac = fracUsedInMeasureTDCTime[i][ch][mezz];
          if (frac < minFrac) minFrac = frac;
          if (frac > maxFrac) maxFrac = frac;
        }
        status = LinFit(x, y, numberScanRun, z, &slope, &intercept, &mse);
        if (status != 0) {
          slope = -1.0;
          intercept = -1.0;
          mse = -1.0;
        }
        if (scanSerialNumber || MDTChamberTest) {
          sprintf(str, "%5d %20s %4d %9.5f %10.5f %9.5f %7.3f %7.3f %7.3f %7.3f\n",
                  mezz, serialNumber, ch, slope, intercept, mse, minSigma, maxSigma, minFrac, maxFrac);
        }
        else {
          sprintf(str, "%5d %4d %9.5f %10.5f %9.5f %7.3f %7.3f %7.3f %7.3f\n",
                  mezz, ch, slope, intercept, mse, minSigma, maxSigma, minFrac, maxFrac);
        }
        MDTChamberOutput2("%s", str);
        linearityScanResults[0][ch][mezz] = minSigma;
        linearityScanResults[1][ch][mezz] = maxSigma;
        linearityScanResults[2][ch][mezz] = slope;
        linearityScanResults[3][ch][mezz] = intercept;
        linearityScanResults[4][ch][mezz] = mse;
        linearityScanResults[5][ch][mezz] = minFrac;
        linearityScanResults[6][ch][mezz] = maxFrac;
      }
    }
  }
  
  if (strlen(MDTChamberWarningFilename) > 0) {
    status = GetFileInfo(MDTChamberWarningFilename, &resp);
    if (status > 0) {
      MDTChamberWarningFile = fopen(MDTChamberWarningFilename, "a");
      if (MDTChamberWarningFile != NULL) {
        fprintf(MDTChamberWarningFile, "\n");
        fprintf(MDTChamberWarningFile, "Checking linearity scan test results at %s\n", WeekdayTimeString(time(NULL)));
      }
    }
  }
  MDTChamberOutput1("// Message will be issued at either of following conditions:\n");
  sprintf(str, "// * <Warning> Bad linearity fit     : Mean square error is less than %.3f;\n", linMSErrorCut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <Warning> Possible dead channel : Minimum TDC time resolution <= %.3f;\n", minTimeSigmaCut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <Warning> Bad time resolution   : Maximum TDC time resolution >= %.3f;\n", timeSigmaCut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <Warning> Bad slope (linearity) : |(1.0 - |slope|)| > %.4f;\n", linSlopeCut);
  MDTChamberOutput2("%s", str);
  MDTChamberOutput1("//\n"); 
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 1) {
      for (ch = 0; ch < 24; ch++) {
        minSigma = linearityScanResults[0][ch][mezz];
        maxSigma = linearityScanResults[1][ch][mezz];
        slope = linearityScanResults[2][ch][mezz];
        mse = linearityScanResults[4][ch][mezz];
        if (mse < linMSErrorCut) {
          status = RDOUTCHBADLINEARITY;
          sprintf(str, "<WARNING> Bad linearity fit (mezz = %2d channel = %2d)\n", mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (minSigma <= minTimeSigmaCut) {
          status = RDOUTCHBADRESOLUTION;
          sprintf(str, "<WARNING> Possible dead channel (mezz = %2d channel = %2d)\n", mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (maxSigma >= timeSigmaCut) {
          status = RDOUTCHBADRESOLUTION;
          sprintf(str, "<WARNING> Channel has bad resolution (%6.1f) (mezz = %2d channel = %2d)\n", maxSigma, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (ABS(1.0+slope) > linSlopeCut) {
          status = RDOUTCHBADSLOPE;
          sprintf(str, "<WARNING> Bad slope (%6.3f) channel (mezz = %2d channel = %2d)\n", slope, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else status = RDOUTCHOK;
        if (status != RDOUTCHOK) checkStatus = status;
        if (MDTChamberHVStatus == MDTCHAMBERHVON) {
          oldStatus = channelStatusHVOn[ch][mezz] - offset;
          if (oldStatus == RDOUTCHOK) channelStatusHVOn[ch][mezz] = status + offset;
        }
        else {
          oldStatus = channelStatusHVOff[ch][mezz] - offset;
          if (oldStatus == RDOUTCHOK) channelStatusHVOff[ch][mezz] = status + offset;
        }
      }
    }
  }
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    sprintf(str, "MDT Chamber HV Status               = %30d\n", MDTChamberHVStatus);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput2("Total Number of Linearity Scan Runs = %30d\n", numberScanRun);
  MDTChamberOutput2("Number Ignored Linearity Scan Runs  = %30d\n", numberIgnoredScanRun);
  MDTChamberOutput2("Number Bad Linearity Scan Runs      = %30d\n", numberBadScanRun);
  if (numberBadScanRun > 3) {
    if (scanControl == 1) {
      sprintf(str, "There are two many bad runs (= %u) during linearity scan.\n"
                   "Please check the scan file <%s> for details.\n"
                   "Possible reasons:\n"
                   "  1) Problem in Linux Box => Need reboot Linux Box and redo scan tests;\n"
                   "  2) Hardware problems => Need find out by you and redo scan tests;\n"
                   "  3) Unknown reasons => You have to make a judgement.\n"
                   "\nAre you sure to continue?\n",
                   numberBadScanRun, MDTChamberSumFilename);
      resp = ConfirmPopup("Too Many Unusable Runs in Threshold Scan", str);
      if (resp == 1) {
        MDTChamberOutput1("Confirm to continue ..\n");
      }
      else {
        checkStatus = 1;
        return checkStatus;
      }
    }
    else {
      sprintf(str, "*********!!! Too many bad runs (=%u) during linearity scan !!!********.\n"
                   "Possible reasons:\n"
                   "  1) Problem in Linux Box => Need reboot Linux Box and redo scan tests;\n"
                   "  2) Hardware problems => Need find out by you and redo scan tests;\n"
                   "  3) Unknown reasons => You have to make a judgement.\n"
                   "Continue auto run without warning to user\n",
                   numberBadScanRun);
      MDTChamberOutput2("%s", str);
    }
  }
  if (MDTChamberWarningFile != NULL) {
    if ((scanControl == 1) && (checkStatus != 0)) {
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
      strcpy(str, "");
      sprintf(str, "notepad %s", MDTChamberWarningFilename);
      status = LaunchExecutable(str);
      if (status != 0) status = LaunchExecutable(str);
    }
    else {
      if (checkStatus == 0) {
        fprintf(MDTChamberWarningFile, "All readout channels are normal in linearity scan test\n");
      }
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
    }
  }
  if ((scanControl == 1) && (checkStatus != 0)) {
    if (MDTChamberHVStatus == MDTCHAMBERHVON) {
      sprintf(str, "Dead or abnormal readout channels have been detected from linearity scan\n"
                   "Possible sources:\n"
                   "  1) Signal or HV Hedghog Card;\n"
                   "  2) Very noise chamber tube;\n"
                   "  3) Bad Mezzanine Card Card;\n"
                   "\nAre you sure to continue?\n");
    }
    else {
      sprintf(str, "Dead or abnormal readout channels have been detected from linearity scan\n"
                   "Most likely the corresponding mezzanine card has problem.\n"
                   "\nAre you sure to continue?\n");
    }
    resp = ConfirmPopup("Dead or Abnormal Readout Channel", str);
    if (resp == 1) {
      checkStatus = 0;
      MDTChamberOutput1("Confirm to continue ..\n");
    }
    else checkStatus = 1;
  }
  else if ((scanControl == 0) && (checkStatus != 0)) {
    sprintf(str, "*********!!! Abnormal Results in Scan Run !!!********\n"
                 "Dead or abnormal readout channels have been detected from linearity scan.\n"
                 "Most likely the corresponding mezzanine card has problem.\n"
                 "Continue auto run without warning to user\n");
    MDTChamberOutput2("%s", str);
    checkStatus = 0;
  }
  
  return checkStatus;
}


double WidthFunc(double x, double a[], int npara);

int FitGainScanResults(void) {
  int checkStatus, scanTime, mezz, ch, offset, i, status, oldStatus, resp, npara, np;
  double sigma, maxSigma, minSigma, frac, maxFrac, minFrac, gainRatio, minGainR, maxGainR;
  double x[200], y[200], z[200], err[200], slope, intercept, mse, para[3], mse2, chi2;
  char str[5120];
  
  gainMSErrorCut = 0.0;
  gainCHI2Cut = 1.0;
  widthSigmaCut = 10.0;
  gainRatioCut = 0.10;
  minIntTimeCut = 10.0;
  maxIntTimeCut = 100.0; 
  checkStatus = 0;
  if (numberScanRun < 3) {
    sprintf(str, "*********!!! Abnormal Scan Run !!!********\n"
                 "Not enough gain scan runs (= %d), no fit will be performed\n", numberScanRun);
    MDTChamberOutput2("%s", str);
    return 0;
  }
  if (MDTChamberHVStatus == MDTCHAMBERHVON) offset = RDOUTSTATUSOFFSETHVON;
  else  offset = RDOUTSTATUSOFFSETHVOFF;
  // Try to fit scanned results
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    sprintf(str, "MDT Chamber HV Status               = %30d\n", MDTChamberHVStatus);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput2("Gain scan test end on %s\n", WeekdayTimeString(time(NULL)));
  scanTime = ((int) time(NULL)) - gainScanStartTime;
  MDTChamberOutput2("Mezzanine Card Gain Scan Results (total scan time = %d (s))\n", scanTime);
  //
  // Perform fit first
  //
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 1) {
      for (ch = 0; ch < 24; ch++) {
        minSigma = 1.0E6;
        maxSigma = -1.0;
        minFrac = 1.0E6;
        maxFrac = -1.0;
        np = 0;
        for (i = 0; i < numberScanRun; i++) {
          frac = fracUsedInMeasureWidth[i][ch][mezz];
          if ((calibCapacitor[i] > 50) && (frac > 0.80)) {
            x[np] = calibCapacitor[i];
            y[np] = measuredAverageWidth[i][ch][mezz];
            err[np] = measuredWidthResolution[i][ch][mezz];
            np++;
          }
          sigma = measuredWidthResolution[i][ch][mezz];
          if (sigma < minSigma) minSigma = sigma;
          if (sigma > maxSigma) maxSigma = sigma;
          frac = fracUsedInMeasureWidth[i][ch][mezz];
          if (frac < minFrac) minFrac = frac;
          if (frac > maxFrac) maxFrac = frac;
        }
        status = 1;
        if (np > 2) status = LinFit(x, y, np, z, &slope, &intercept, &mse);
        else {
          sprintf(str, "Not enough points (=%d) to perform a fit for mezz %2d ch %2d\n", np, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        if (status != 0) {
          slope = -1.0;
          intercept = -1.0;
          mse = -1.0;
          para[0] = -1.0;
          para[1] = -1.0;
          para[2] = -1.0;
          mse2 = -1.0;
          chi2 = -1.0;
        }
        else {
          npara = 3;
          para[0] = intercept;
          para[1] = slope;
          para[2] = 1000.;
          status = NonLinearFit(x, y, z, np, WidthFunc, para, npara, &mse2);
          if (status != 0) {
            mse = -1.0;
            para[0] = -1.0;
            para[1] = -1.0;
            para[2] = -1.0;
            mse2 = -1.0;
            chi2 = -1.0;
          }
          else chi2 = CHISquare(y, z, err, np, npara);
        }
        gainScanResults[0][ch][mezz] = minSigma;
        gainScanResults[1][ch][mezz] = maxSigma;
        gainScanResults[2][ch][mezz] = slope;
        gainScanResults[3][ch][mezz] = intercept;
        gainScanResults[4][ch][mezz] = mse;
        gainScanResults[5][ch][mezz] = minFrac;
        gainScanResults[6][ch][mezz] = maxFrac;
        gainScanResults[7][ch][mezz] = para[0];
        gainScanResults[8][ch][mezz] = para[1];
        gainScanResults[9][ch][mezz] = para[2];
        gainScanResults[10][ch][mezz] = mse2;
        gainScanResults[11][ch][mezz] = chi2;
      }
    }
  }
  MDTChamberOutput1("//\n");
  MDTChamberOutput1("// --- Description for Gain Scan Test ---\n");
  MDTChamberOutput1("//     To understand gain scan test, it is necessary to know how ASD functional.\n"); 
  MDTChamberOutput1("//   There are two possible input sources for a channel of ASD chip, 1) from ouside\n"); 
  MDTChamberOutput1("//   (for instance from MDT chamber); 2) from calibration signal generated by\n"); 
  MDTChamberOutput1("//   ASD calibration circuit. The calibration signal is generated by a 4us square pulse\n"); 
  MDTChamberOutput1("//   as delta charge at beginning/end of the 4us square pulse, where only signal at\n"); 
  MDTChamberOutput1("//   beginning is used. In first order, the calibration is proportional to calibration\n"); 
  MDTChamberOutput1("//   capacitor's capacitance.\n"); 
  MDTChamberOutput1("//     When an input signal is over ASD threshold, ASD start to charge the charge capacitor\n"); 
  MDTChamberOutput1("//   with programmable time window (so called ASD charge integration time), and the moment of\n"); 
  MDTChamberOutput1("//   the input signal over ASD threshold is the leading edge, i.e., TDC time. After the charge\n"); 
  MDTChamberOutput1("//   integration, ASD start to discharge with a constant rundown current (again programmable)\n"); 
  MDTChamberOutput1("//   and stop at certain threshold (Wilkinson ADC threshold) which determined the trailing edge\n"); 
  MDTChamberOutput1("//   of the input signal at ASD ADC mode, therefore the width, it is:\n"); 
  MDTChamberOutput1("//              MinWidth + Q/I\n"); 
  MDTChamberOutput1("//   where Q is total integrated charge and I is rundown current, and MinWidth is roughly\n"); 
  MDTChamberOutput1("//   corresponding to ASD chaarge integration time. For calibration signal, total charge is\n"); 
  MDTChamberOutput1("//   predictable, which is:\n"); 
  MDTChamberOutput1("//              V0*ccal*(1.0 - exp(-C2/ccal))\n"); 
  MDTChamberOutput1("//   where V0 is the product of maximum voltage on calibration capacitor and the gain, ccal\n"); 
  MDTChamberOutput1("//   is the calibration capacitor's capacitance and C2 is constant (charge integration time\n"); 
  MDTChamberOutput1("//   over calibration circuit resistance, i.e., T/R). In calibration run, the width is:\n"); 
  MDTChamberOutput1("//              MinWidth + C1*ccal*(1.0 - exp(-C2/ccal))\n"); 
  MDTChamberOutput1("//   where C1 = V0/I and C2 = T/R.\n"); 
  MDTChamberOutput1("//   It is recommemded to measure the charge width with 7 calibration capacitor settings\n"); 
  MDTChamberOutput1("//   (100, 150, 200, 250, 300, 350 and 400 fF), and the measured width is fitted with above\n"); 
  MDTChamberOutput1("//   width distribution. In gain scan test, the ratio of the actual gain over expected gain is\n"); 
  MDTChamberOutput1("//   measured which should be near 1 for all calibration capacitor settings (i.e., constant gain\n"); 
  MDTChamberOutput1("//   in measured range), and the intrinsic width resolution is measured also with different\n"); 
  MDTChamberOutput1("//   calibration capacitor settings.\n"); 
  MDTChamberOutput1("//\n"); 
  MDTChamberOutput1("// Board              : Mezzanine card number\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("// Mezz.Serial-Number : Mezzanine card serial number (ID)\n");
  }
  MDTChamberOutput1("// Chan               : Channel number\n"); 
  MDTChamberOutput1("// calCap (fF)        : Calibration capacitor's capacitance in fF\n"); 
  MDTChamberOutput2("// averageADC         : Average ADC (width) in count (1 count = %.5f ns)\n", widthBin); 
  MDTChamberOutput2("// resolution         : Width resolution in count (1 count = %.5f ns)\n", widthBin); 
  MDTChamberOutput1("// fracUsed           : Fraction of hits are used to compute width resolution\n"); 
  MDTChamberOutput1("// gainRatio          : Ratio of actual gain over expected gain\n"); 
  MDTChamberOutput1("//                      Note! A value of 1.0 will be assigned if no gain measured due to bad fit\n"); 
  MDTChamberOutput1("//\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("Board   Mezz.Serial-Number Chan calCap averageADC resolution fracUsed gainRatio\n");
  }
  else {
    MDTChamberOutput1("Board Chan calCap averageADC resolution fracUsed gainRatio\n");
  }
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 1) {
      GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
      for (ch = 0; ch < 24; ch++) {
        minGainR = 99.0;
        maxGainR = -1.0;
        mse2 = gainScanResults[10][ch][mezz];
        for (i = 0; i < numberScanRun; i++) {
          x[i] = measuredAverageWidth[i][ch][mezz];
          sigma = measuredWidthResolution[i][ch][mezz];
          frac = fracUsedInMeasureWidth[i][ch][mezz];
          gainRatio = 1.0;
          if (mse2 > 0.0) {
            para[0] = gainScanResults[7][ch][mezz];
            para[1] = gainScanResults[8][ch][mezz];
            para[2] = gainScanResults[9][ch][mezz];
            y[0] = calibCapacitor[i];
            z[0] = WidthFunc(y[0], para, 3) - para[0];
            if (z[0] != 0.0) gainRatio = (x[i] - para[0]) / z[0];
          }
          if (gainRatio < minGainR) minGainR = gainRatio;
          if (gainRatio > maxGainR) maxGainR = gainRatio;
          if (scanSerialNumber || MDTChamberTest) {
            sprintf(str, "%5d %20s %4d %6d %10.2f %10.3f %8.3f %9.5f\n",
                    mezz, serialNumber, ch, calibCapacitor[i], x[i], sigma, frac, gainRatio);
          }
          else {
            sprintf(str, "%5d %4d %6d %10.2f %10.3f %8.3f %9.5f\n",
                    mezz, ch, calibCapacitor[i], x[i], sigma, frac, gainRatio);
          }
          MDTChamberOutput2("%s", str);
        }
        gainScanResults[12][ch][mezz] = minGainR;
        gainScanResults[13][ch][mezz] = maxGainR;
      }
    }
  }
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    sprintf(str, "MDT Chamber HV Status               = %30d\n", MDTChamberHVStatus);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput1("//\n"); 
  MDTChamberOutput1("// Board              : Mezzanine card number\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("// Mezz.Serial-Number : Mezzanine card serial number (ID)\n");
  }
  MDTChamberOutput1("// Chan               : Channel number\n"); 
  MDTChamberOutput2("// minWidth           : Minimum width in count (1 count = %.5f ns)\n", widthBin); 
  MDTChamberOutput1("// V0OverI            : Width fit constant C1, see gain scan test description for details\n"); 
  MDTChamberOutput1("// IntTOverR          : Width fit constant C2, see gain scan test description for details\n"); 
  MDTChamberOutput1("// MSError            : Mean square error of width fit\n"); 
  MDTChamberOutput1("// CHI2               : CHI2 per number degree freedom of width fit\n"); 
  MDTChamberOutput2("// minRes             : Minimum width resolution in count (1 count = %.5f ns)\n", widthBin); 
  MDTChamberOutput2("// maxRes             : Maximum width resolution in count (1 count = %.5f ns)\n", widthBin); 
  MDTChamberOutput1("// minFrac            : Minimum fraction of hits are used to compute width resolution\n"); 
  MDTChamberOutput1("// maxFrac            : Maximum fraction of hits are used to compute width resolution\n"); 
  MDTChamberOutput1("// minGainR           : Minumum ratio of actual gain over expected gain\n"); 
  MDTChamberOutput1("// maxGainR           : Maxumum ratio of actual gain over expected gain\n"); 
  MDTChamberOutput1("//\n"); 
  if (scanSerialNumber || MDTChamberTest) {
    MDTChamberOutput1("Board   Mezz.Serial-Number Chan  minWidth   V0OverI IntTOverR   MSError      CHI2  minRes  maxRes minFrac maxFrac  minGainR  maxGainR\n");
  }
  else {
    MDTChamberOutput1("Board Chan  minWidth   V0OverI IntTOverR   MSError      CHI2  minRes  maxRes minFrac maxFrac\n");
  }
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 1) {
      GetCtrlVal(SerialNumberHandle, snButton[mezz], serialNumber);
      for (ch = 0; ch < 24; ch++) {
        minSigma = gainScanResults[0][ch][mezz];
        maxSigma = gainScanResults[1][ch][mezz];
        slope = gainScanResults[2][ch][mezz];
        intercept = gainScanResults[3][ch][mezz];
        mse = gainScanResults[4][ch][mezz];
        minFrac = gainScanResults[5][ch][mezz];
        maxFrac = gainScanResults[6][ch][mezz];
        para[0] = gainScanResults[7][ch][mezz];
        para[1] = gainScanResults[8][ch][mezz];
        para[2] = gainScanResults[9][ch][mezz];
        mse2 = gainScanResults[10][ch][mezz];
        chi2 = gainScanResults[11][ch][mezz];
        minGainR = gainScanResults[12][ch][mezz];
        maxGainR = gainScanResults[13][ch][mezz];
        if (scanSerialNumber || MDTChamberTest) {
          sprintf(str, "%5d %20s %4d %9.2f %9.3f %9.1f %9.5f %9.4f %7.3f %7.3f %7.3f %7.3f %9.5f %9.5f\n",
                  mezz, serialNumber, ch, para[0], para[1], para[2], mse2, chi2, minSigma, maxSigma, minFrac, maxFrac, minGainR, maxGainR);
        }
        else {
          sprintf(str, "%5d %4d %9.2f %9.3f %9.1f %9.5f %9.4f %7.3f %7.3f %7.3f %7.3f %9.5f %9.5f\n",
                  mezz, ch, para[0], para[1], para[2], mse2, chi2, minSigma, maxSigma, minFrac, maxFrac, minGainR, maxGainR);
        }
        MDTChamberOutput2("%s", str);
      }
    }
  }
  
  if (strlen(MDTChamberWarningFilename) > 0) {
    status = GetFileInfo(MDTChamberWarningFilename, &resp);
    if (status > 0) {
      MDTChamberWarningFile = fopen(MDTChamberWarningFilename, "a");
      if (MDTChamberWarningFile != NULL) {
        fprintf(MDTChamberWarningFile, "\n");
        fprintf(MDTChamberWarningFile, "Checking gain scan test results at %s\n", WeekdayTimeString(time(NULL)));
      }
    }
  }
  MDTChamberOutput1("// Message will be issued at either of following conditions:\n");
  sprintf(str, "// * <Warning> Bad width fit        : Mean square error is less than %.3f;\n", gainMSErrorCut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <Warning> Bad gain linearity   : CH2 > %.2f;\n", gainCHI2Cut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <-Infor-> None linear gain     : |1.0 - gainRatio| > %.3f;\n", gainRatioCut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <-Infor-> Bad minimum width    : minWidth < %.1f OR minWidth > %.1f;\n", minIntTimeCut, maxIntTimeCut);
  MDTChamberOutput2("%s", str);
  sprintf(str, "// * <-Infor-> Bad width resolution : width resolution > %.2f;\n", widthSigmaCut);
  MDTChamberOutput2("%s", str);
  MDTChamberOutput1("//\n"); 
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 1) {
      for (ch = 0; ch < 24; ch++) {
        chi2 = gainScanResults[11][ch][mezz];
        mse2 = gainScanResults[10][ch][mezz];
        intercept = gainScanResults[7][ch][mezz];
        maxSigma = gainScanResults[1][ch][mezz];
        minGainR = gainScanResults[12][ch][mezz];
        maxGainR = gainScanResults[13][ch][mezz];
        if (mse2 < gainMSErrorCut) {
          status = RDOUTCHBADGAINFIT;
          sprintf(str, "<WARNING> Bad gain fit (mezz = %2d channel = %2d)\n", mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (chi2 > gainCHI2Cut) {
          status = RDOUTCHBADGAINLINEARITY;
          sprintf(str, "<WARNING> Bad gain linearity (chi2 = %6.1f) channel (mezz = %2d channel = %2d)\n", chi2, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if ((ABS(1.0 - minGainR) > gainRatioCut) || (ABS(1.0 - maxGainR) > gainRatioCut)) {
          status = RDOUTCHBADGAINLINEARITY;
          sprintf(str, "<-INFOR-> None linear gain (minGainR = %5.2f, maxGainR = %5.2f) channel (mezz = %2d channel = %2d)\n", minGainR, maxGainR, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (intercept < minIntTimeCut || intercept > maxIntTimeCut) {
          status = RDOUTCHBADGAINLINEARITY;
          sprintf(str, "<-INFOR-> Abnormal minWidth ( = %10.1f) channel (mezz = %2d channel = %2d)\n", intercept, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else if (maxSigma > widthSigmaCut) {
          status = RDOUTCHBADWIDTHRESOLUTION;
          sprintf(str, "<-INFOR-> Bad width resolution ( = %6.1f) channel (mezz = %2d channel = %2d)\n", maxSigma, mezz, ch);
          MDTChamberOutput2("%s", str);
        }
        else status = RDOUTCHOK;
        if (status != RDOUTCHOK) checkStatus = status;
        if (MDTChamberHVStatus == MDTCHAMBERHVON) {
          oldStatus = channelStatusHVOn[ch][mezz] - offset;
          if (oldStatus == RDOUTCHOK) channelStatusHVOn[ch][mezz] = status + offset;
        }
        else {
          oldStatus = channelStatusHVOff[ch][mezz] - offset;
          if (oldStatus == RDOUTCHOK) channelStatusHVOff[ch][mezz] = status + offset;
        }
      }
    }
  }
  if (MDTChamberTest || (MDTChamberHVStatus == MDTCHAMBERHVON)) {
    sprintf(str, "MDT Chamber HV Status               = %30d\n", MDTChamberHVStatus);
    MDTChamberOutput2("%s", str);
  }
  MDTChamberOutput2("Total Number of Gain Scan Runs      = %30d\n", numberScanRun);
  MDTChamberOutput2("Number Ignored Gain Scan Runs       = %30d\n", numberIgnoredScanRun);
  MDTChamberOutput2("Number Bad Gain Scan Runs           = %30d\n", numberBadScanRun);
  if (numberBadScanRun > 0) {
    if (scanControl == 1) {
      sprintf(str, "There are two many bad runs (= %u) during gain scan.\n"
                   "Please check the scan file <%s> for details.\n"
                   "Possible reasons:\n"
                   "  1) Problem in Linux Box => Need reboot Linux Box and redo scan tests;\n"
                   "  2) Hardware problems => Need find out by you and redo scan tests;\n"
                   "  3) Unknown reasons => You have to make a judgement.\n"
                   "\nAre you sure to continue?\n",
                   numberBadScanRun, MDTChamberSumFilename);
      resp = ConfirmPopup("Too Many Unusable Runs in Threshold Scan", str);
      if (resp == 1) {
        MDTChamberOutput1("Confirm to continue ..\n");
      }
      else {
        checkStatus = 1;
        return checkStatus;
      }
    }
    else {
      sprintf(str, "*********!!! Too many bad runs (=%u) during gain scan !!!********.\n"
                   "Possible reasons:\n"
                   "  1) Problem in Linux Box => Need reboot Linux Box and redo scan tests;\n"
                   "  2) Hardware problems => Need find out by you and redo scan tests;\n"
                   "  3) Unknown reasons => You have to make a judgement.\n"
                   "Continue auto run without warning to user\n",
                   numberBadScanRun);
      MDTChamberOutput2("%s", str);
    }
  }
  if (MDTChamberWarningFile != NULL) {
    if ((scanControl == 1) && (checkStatus != 0)) {
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
      strcpy(str, "");
      sprintf(str, "notepad %s", MDTChamberWarningFilename);
      status = LaunchExecutable(str);
      if (status != 0) status = LaunchExecutable(str);
    }
    else {
      if (checkStatus == 0) {
        fprintf(MDTChamberWarningFile, "All readout channels are normal in gain scan test\n");
      }
      fclose(MDTChamberWarningFile);
      MDTChamberWarningFile = NULL;
    }
  }
  if ((scanControl == 1) && (checkStatus != 0)) {
    if (MDTChamberHVStatus == MDTCHAMBERHVON) {
      sprintf(str, "Dead or abnormal readout channels have been detected from gain scan\n"
                   "Possible sources:\n"
                   "  1) Signal or HV Hedghog Card;\n"
                   "  2) Very noise chamber tube;\n"
                   "  3) Bad Mezzanine Card Card;\n"
                   "\nAre you sure to continue?\n");
    }
    else {
      sprintf(str, "Dead or abnormal readout channels have been detected from gain scan\n"
                   "Most likely the corresponding mezzanine card has problem.\n"
                   "\nAre you sure to continue?\n");
    }
    resp = ConfirmPopup("Dead or Abnormal Readout Channel", str);
    if (resp == 1) {
      checkStatus = 0;
      MDTChamberOutput1("Confirm to continue ..\n");
    }
    else checkStatus = 1;
  }
  else if ((scanControl == 0) && (checkStatus != 0)) {
    sprintf(str, "*********!!! Abnormal Results in Scan Run !!!********\n"
                 "Dead or abnormal readout channels have been detected from gain scan.\n"
                 "Most likely the corresponding mezzanine card has problem.\n"
                 "Continue auto run without warning to user\n");
    MDTChamberOutput2("%s", str);
    checkStatus = 0;
  }
  
  return checkStatus;
}


double WidthFunc(double x, double a[], int npara) {
  double width;
  
  width = a[0] + a[1]*x*(1.0 - exp(-a[2]/x));
  
  return width;
}
