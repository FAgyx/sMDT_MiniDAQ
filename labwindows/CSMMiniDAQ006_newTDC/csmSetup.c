// csmSetup.c                         by T.S.Dai
//
#ifdef _MSC_VER           // Defined for Miscrsoft Visual C++
  #include "MVC_Stuff.h"
#elif defined _CVI_       // Otherwise compile in CVI environment
  #include <userint.h>
  #include <utility.h>
  #include <ansi_c.h>
#endif
#include <cvirte.h>       // Needed if linking in external compiler; harmless otherwise
#include "interface.h"
#include "DAQGUI.h"
#include "ttcrxSetup.h"
#include "csmSetup.h"

extern int Menu00Handle, JTAGControlHandle, action, AMTCommandDelay, GOLOn, TTCrxOn;
extern void UpGOLSetupControl(void);
extern void GetCSMStatus(void);
extern void DownloadCSMSetup(void);
extern void TurnOnOffGOLAndTTCrxFromJTAGChain(void);
extern void UpdateJTAGControlPanel(void);
extern void GetMezzCardEnables(void);
extern void SetupJTAGChain(void);
extern void ClearSetupStatus(void);

// Bring up CSM Setup Control Panel
void UpCSMSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    PanelRecall(CSMSetupHandle);
    GetCSMStatus();
	
	printf("GetCSMStatus() is in UpCSMSetupControl()\n");
	
    UpdateCSMControlBits();
    UpdateCSMSetupPanel();
    CreatGOLLDList();
    PutCSMNextState();
    PanelSave(CSMSetupHandle);
    DisplayPanel(CSMSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup CSM",
                 "Bring up CSM Setup Control Panel.\n"
                 "Note: For CSM settings, consult the CSM manual.");
  }
  controlOption = NORM;
}


// Bring up TTCrx Internal Register Information Panel
void UpTTCrxInfoControl(void) {
  int val, rval, color, okColor, warningColor, errColor;

  HidePanel(CSMSetupHandle);
  readbackTTCrxRegisterDone = TRUE;
  LoadTTCrxSetupArray();
  okColor = VAL_GREEN;
  if (readbackTTCrxRegisterDone) warningColor = VAL_YELLOW;
  else warningColor = VAL_WHITE;
  if (readbackTTCrxRegisterDone) errColor = VAL_RED;
  else errColor = VAL_WHITE;
  BinaryToInt(&val, CLOCK1FINEDELAY, 8, TTCrxSetupArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_FINEDELAY1, val);
  BinaryToInt(&rval, TTCRXFINEDELAY1, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_RBFINEDELAY1, rval);
  if (val != rval) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBFINEDELAY1, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBFINEDELAY1, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&val, CLOCK2FINEDELAY, 8, TTCrxSetupArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_FINEDELAY2, val);
  BinaryToInt(&rval, TTCRXFINEDELAY2, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_RBFINEDELAY2, rval);
  if (val != rval) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBFINEDELAY2, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBFINEDELAY2, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&val, CLOCK1COARSEDELAY, 8, TTCrxSetupArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_COARSEDELAY, val);
  BinaryToInt(&rval, TTCRXCOARSEDELAY, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_RBCOARSEDELAY, rval);
  if (val != rval) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBCOARSEDELAY, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBCOARSEDELAY, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&val, BUNCHCOUNTERCONTROL, 8, TTCrxSetupArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_CONTROL, val);
  BinaryToInt(&rval, TTCRXCONTROL, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_RBCONTROL, rval);
  if (val != rval) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBCONTROL, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBCONTROL, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&val, TTCRXADDRESS, 8, TTCrxSetupArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_ID0007, val);
  BinaryToInt(&rval, TTCRXID0, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_RBID0007, rval);
  if (val != rval) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBID0007, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBID0007, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&val, TTCRXADDRESS+8, 8, TTCrxSetupArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_ID0813, val);
  BinaryToInt(&rval, TTCRXMASTERMODEAID8, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_RBID0813, rval);
  if (val != rval) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBID0813, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBID0813, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&val, I2CBASEADDRESS, 8, TTCrxSetupArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_I2CID0005, val);
  BinaryToInt(&rval, TTCRXMASTERMODEBI2CID, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_RBI2CID0005, rval);
  if (val != rval) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBI2CID0005, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBI2CID0005, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&val, DLLCURRENT, 8, TTCrxSetupArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_CONFIGURATION1, val);
  BinaryToInt(&rval, TTCRXCONFIG1, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_RBCONFIGURATION1, rval);
  if (val != rval) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBCONFIGURATION1, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBCONFIGURATION1, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&val, TESTOUTPUTS, 8, TTCrxSetupArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_CONFIGURATION2, val);
  BinaryToInt(&rval, TTCRXCONFIG2, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_RBCONFIGURATION2, rval);
  if (val != rval) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBCONFIGURATION2, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBCONFIGURATION2, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&val, FREQCHECKPERIOD, 8, TTCrxSetupArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_CONFIGURATION3, val);
  BinaryToInt(&rval, TTCRXCONFIG3, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_RBCONFIGURATION3, rval);
  if (val != rval) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBCONFIGURATION3, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_RBCONFIGURATION3, ATTR_TEXT_BGCOLOR, okColor);

  BinaryToInt(&rval, TTCRXSERRORCOUNT0, 16, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_SINGLEERROR, rval);
  if (rval != 0) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_SINGLEERROR, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_SINGLEERROR, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&rval, TTCRXDERRORCOUNT, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_DOUBLEERROR, rval);
  if (rval != 0) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_DOUBLEERROR, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_DOUBLEERROR, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&rval, TTCRXSEUERRORCOUNT, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_SEUERROR, rval);
  if (rval != 0) 
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_SEUERROR, ATTR_TEXT_BGCOLOR, warningColor);
  else
    SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_SEUERROR, ATTR_TEXT_BGCOLOR, okColor);
  BinaryToInt(&rval, TTCRXBUNCHCOUNTER00, 16, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_BUNCHNUMBER, rval);
  BinaryToInt(&rval, TTCRXEVENTCOUNTER00, 24, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_EVENTNUMBER, rval);
  BinaryToInt(&rval, TTCRXSTATUSREGISTER, 8, CSMStatusArray);
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_STATUS, rval);
  color = okColor;
  val = rval & TTCRXSTSALWAYS0BITS;
  if (val != 0) {
    val = 1;
    color = warningColor;
  }
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_UNUSEDBITS, val);
  val = (rval >> TTCRXSTSAUTORESETFLAG) & 1;
  if (val == 1) color = warningColor;
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_AUTORESET, val);
  val = (rval >> TTCRXSTSFRAMESYNCH) & 1;
  if (val == 0) color = errColor;
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_FRAMESYNCH, val);
  val = (rval >> TTCRXSTSDLLREADY) & 1;
  if (val == 0) color = errColor;
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_DLLREADY, val);
  val = (rval >> TTCRXSTSPLLREADY) & 1;
  if (val == 0) color = errColor;
  SetCtrlVal(TTCrxInfoHandle, P_TTCREG_PLLREADY, val);
  SetCtrlAttribute(TTCrxInfoHandle, P_TTCREG_STATUS, ATTR_TEXT_BGCOLOR, color);
  DisplayPanel(TTCrxInfoHandle);
  SetActivePanel(TTCrxInfoHandle);
}


// Load CSM Setup Control Panel and define user buttons
int CSMSetupPanelAndButton(void) {
  // Define the CSM Setup Control panel handles
  if ((CSMSetupHandle = LoadPanel(0, "DAQGUI.uir", P_CSM)) < 0) return -1;
  SetPanelAttribute(CSMSetupHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  if ((TTCrxInfoHandle = LoadPanel(0, "DAQGUI.uir", P_TTCREG)) < 0) return -1;
  SetPanelAttribute(TTCrxInfoHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);

  // CSM Setup Control Panel Buttons
  SetupUserButton(Menu00Handle, MENU00_JTAG_SETUPCSM, UpCSMSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_RESETCSM, ResetCSM);
  SetupUserButton(CSMSetupHandle, P_CSM_DONE, CSMSetupDone);
  SetupUserButton(CSMSetupHandle, P_CSM_DEFAULT, PanelDefault);
  SetupUserButton(CSMSetupHandle, P_CSM_CANCEL, PanelCancel);
  SetupUserButton(CSMSetupHandle, P_CSM_SAVESETUP, SaveCSMSetup);
  SetupUserButton(CSMSetupHandle, P_CSM_TTCRXCONTROL, UpTTCrxSetupControl);
  SetupUserButton(CSMSetupHandle, P_CSM_GOLCONTROL, UpGOLSetupControl);
  SetupUserButton(CSMSetupHandle, P_CSM_SETALLMEZZ, SetAllMezzanineCard);
  SetupUserButton(CSMSetupHandle, P_CSM_CLEARALLMEZZ, ClearAllMezzanineCard);
  SetupUserButton(CSMSetupHandle, P_CSM_CSMDAQCONTROL, CSMDAQControl);
  SetupUserButton(CSMSetupHandle, P_CSM_GOLLASER, CreatGOLLDList);
  SetupUserButton(CSMSetupHandle, P_CSM_TTCRXUSEPROM, TTCrxPROMUsage);
  SetupUserButton(CSMSetupHandle, P_CSM_CSMNEXTSTATE, PutCSMNextState);
  SetupUserButton(CSMSetupHandle, P_CSM_MAXALLOWEDMEZZS, TurnOffUnwantedMezzCards);
  SetupUserButton(CSMSetupHandle, P_CSM_CSMMEZZLINK, SetCSMModeAccordingToLinkRate);
  SetupUserButton(CSMSetupHandle, P_CSM_DETAIL, UpTTCrxInfoControl);
  SetupUserButton(TTCrxInfoHandle, P_TTCREG_CLOSE, TTCrxInfoClose);

  return 0;
}


// Close TTCrx Internal Register Information Panel
void TTCrxInfoClose(void) {
  HidePanel(TTCrxInfoHandle);
  DisplayPanel(CSMSetupHandle);
  SetActivePanel(CSMSetupHandle);
}


// CSM Setup Done
void CSMSetupDone(void) {
  int panel, GOLEnable, TTCrxEnable;
  
  panel = GetActivePanel();
  PanelSave(CSMSetupHandle);
  LoadCSMSetupArray();
  SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, CSMSetupArray[MEZZJTAGENABLE]);
  HidePanel(CSMSetupHandle);
  if (panel == CSMSetupHandle) {
    GetCtrlVal(CSMSetupHandle, P_CSM_GOLTDI, &GOLEnable);
    GetCtrlVal(CSMSetupHandle, P_CSM_TTCRXTDI, &TTCrxEnable);
    if ((GOLOn != GOLEnable) || (TTCrxOn != TTCrxEnable)) {
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, GOLEnable);
      SetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, TTCrxEnable);
      PanelSave(CSMSetupHandle);
      TurnOnOffGOLAndTTCrxFromJTAGChain();
    }
  }
  DownloadCSMSetup();
  UpdateJTAGControlPanel();     // Update JTAG control panel
  DisplayPanel(JTAGControlHandle);
  SetActivePanel(JTAGControlHandle);
}


void SaveCSMSetup(void) {
  unsigned int val, mezz, index;
  char setupFilename[281] = "CSMSetup.txt", str[80];
  FILE *setupFile;
  
  PromptPopup("Get CSM Setup File Name",
              "Please give the file name (Default: CSMSetup.txt) for saving CSM setup",
              setupFilename, 280);
  if (strlen(setupFilename) <= 5 ) strcpy(setupFilename, "CSMSetup.txt");
  if (setupFile = fopen(setupFilename, "w")) {
    LoadCSMSetupArray();
    fprintf(setupFile, "--------- CSM Setup File -------------\n");
    fprintf(setupFile, "Position  Settings  Description...\n");
    for (mezz = 0; mezz < 18; mezz++) {
      fprintf(setupFile, "%8d  %8d  ", TDCENABLES+mezz, CSMSetupArray[TDCENABLES+mezz]);
      if (CSMSetupArray[TDCENABLES+mezz] == 1)
        fprintf(setupFile, "Mezzanine Card %02d enabled\n", mezz);
      else
        fprintf(setupFile, "Mezzanine Card %02d disabled\n", mezz);
    }
    
    fprintf(setupFile, "%8d  %8d  ", MEZZJTAGENABLE, CSMSetupArray[MEZZJTAGENABLE]);
    if (CSMSetupArray[MEZZJTAGENABLE] == 1)
      fprintf(setupFile, "JTAG signals for Mezzanine Card enabled\n");
    else
      fprintf(setupFile, "JTAG signals for Mezzanine Card disabled\n");
    
    fprintf(setupFile, "%8d  %8d  ", CSMDAQCONTROL, CSMSetupArray[CSMDAQCONTROL]);
    if (CSMSetupArray[CSMDAQCONTROL] == 1)
      fprintf(setupFile, "DAQ enabled at CSM\n");
    else
      fprintf(setupFile, "DAQ disabled at CSM\n");
    
    BinaryToInt(&val, AMTCOMMANDDELAY, 7, CSMSetupArray);
    fprintf(setupFile, "%8d  %8d  ", AMTCOMMANDDELAY, val);
    fprintf(setupFile, "Delay vale (= %d ns) for AMT Command\n", 25*(val+1));

    fprintf(setupFile, "%8d  %8d  ", MEZZ80MHZOPERATION, CSMSetupArray[MEZZ80MHZOPERATION]);
    if (CSMSetupArray[MEZZ80MHZOPERATION] == 1)
      fprintf(setupFile, "Use 80MHz Link between CSM and Mezzanine Cards\n");
    else
      fprintf(setupFile, "Use 40MHz Link between CSM and Mezzanine Cards\n");
    
    BinaryToInt(&val, MEZZPAIRWIDTHRESOLUTION, 3, CSMSetupArray);
    fprintf(setupFile, "%8d  %8d  ", MEZZPAIRWIDTHRESOLUTION, val);
    if ((CSMSetupArray[MEZZCSMPAIRMODE]|CSMSetupArray[MEZZCSMPAIRDEBUGMODE]) == 1) {
      if (val < 8) fprintf(setupFile, "Width Resolution %f ns\n", 0.78125*(1<<val));
      else fprintf(setupFile, "Unknown Width Resolution setting %d\n", val);
    }
    else {
      if (val < 8) fprintf(setupFile, "UNUSED! Width Resolution %f ns\n", 0.78125*(1<<val));
      else fprintf(setupFile, "UNUSED! Unknown Width Resolution setting %d\n", val);
    }

    fprintf(setupFile, "%8d  %8d  ", MEZZCSMPAIRMODE, CSMSetupArray[MEZZCSMPAIRMODE]);
    if (CSMSetupArray[MEZZCSMPAIRMODE] == 1)
      fprintf(setupFile, "CSM in Pair Mode\n");
    else
      fprintf(setupFile, "CSM not in Pair Mode\n");
    
    fprintf(setupFile, "%8d  %8d  ", MEZZCSMPAIRDEBUGMODE, CSMSetupArray[MEZZCSMPAIRDEBUGMODE]);
    if (CSMSetupArray[MEZZCSMPAIRDEBUGMODE] == 1)
      fprintf(setupFile, "CSM in Pair Debug Mode (Only for test!)\n");
    else
      fprintf(setupFile, "CSM in Normal Mode\n");
    
    fprintf(setupFile, "%8d  %8d  ", CSMSPAREBIT00, CSMSetupArray[CSMSPAREBIT00]);
    fprintf(setupFile, "Spare Bit\n");
    
    fprintf(setupFile, "%8d  %8d  ", EMPTYCYCLECONTROL, CSMSetupArray[EMPTYCYCLECONTROL]);
    if (CSMSetupArray[EMPTYCYCLECONTROL] == 1)
      fprintf(setupFile, "Suppress CSM Empty Cycle (TDM only)\n");
    else
      fprintf(setupFile, "Include CSM Empty Cycle (TDM only)\n");

    fprintf(setupFile, "%8d  %8d  ", SYNCHWORDCONTROL, CSMSetupArray[SYNCHWORDCONTROL]);
    if (CSMSetupArray[SYNCHWORDCONTROL] == 1)
      fprintf(setupFile, "Include CSM Status and Trigger Time in SynchWord (TDM only)\n");
    else
      fprintf(setupFile, "Exclude CSM Status and Trigger Time in SynchWord (TDM only)\n");

    fprintf(setupFile, "%8d  %8d  ", GOLDIFF, CSMSetupArray[GOLDIFF]);
    if (CSMSetupArray[GOLDIFF] == 1)
      fprintf(setupFile, "GOL differential ended clock in use\n");
    else
      fprintf(setupFile, "GOL single ended clock in use\n");

    BinaryToInt(&val, GOLLD0, 2, CSMSetupArray);
    fprintf(setupFile, "%8d  %8d  ", GOLLD0, val);
    if (CSMSetupArray[GOLLASER] == 1) {
      if (val == 0) fprintf(setupFile, "GOL Laser-diode bias current 5.8 mA\n");
      else if (val == 1) fprintf(setupFile, "GOL Laser-diode bias current 7.4 mA\n");
      else if (val == 2) fprintf(setupFile, "GOL Laser-diode bias current 11.4 mA\n");
      else if (val == 3) fprintf(setupFile, "GOL Laser-diode bias current 16.2 mA\n");
      else fprintf(setupFile, "Unknown GOL laser-diode bias current setting\n");
    }
    else {
      if (val <= 3) fprintf(setupFile, "GOL Line-driver strength %d\n", val+1);
      else fprintf(setupFile, "Unknown GOL line-driver strength setting\n");
    }
    
    fprintf(setupFile, "%8d  %8d  ", GOLPLL, CSMSetupArray[GOLPLL]);
    if (CSMSetupArray[GOLPLL] == 1)
      fprintf(setupFile, "GOL PLL current 15 uA\n");
    else
      fprintf(setupFile, "GOL PLL current 10 uA\n");

    fprintf(setupFile, "%8d  %8d  ", GOLLASER, CSMSetupArray[GOLLASER]);
    if (CSMSetupArray[GOLLASER] == 1)
      fprintf(setupFile, "GOL laser-diode driver selected\n");
    else
      fprintf(setupFile, "GOL line-driver selected\n");

    fprintf(setupFile, "%8d  %8d  ", GOLNEG, CSMSetupArray[GOLNEG]);
    if (CSMSetupArray[GOLNEG] == 1)
      fprintf(setupFile, "GOL failing edge selected to sample data\n");
    else
      fprintf(setupFile, "GOL leading edge selected to sample data\n");

    fprintf(setupFile, "%8d  %8d  ", GOLMODE, CSMSetupArray[GOLMODE]);
    if (CSMSetupArray[GOLMODE] == 1)
      fprintf(setupFile, "GOL Slow Mode (16 bits data transmission)\n");
    else
      fprintf(setupFile, "GOL Fast Mode (32 bits data transmission)\n");

    fprintf(setupFile, "%8d  %8d  ", ENABLEGOLTDI, CSMSetupArray[ENABLEGOLTDI]);
    if (CSMSetupArray[ENABLEGOLTDI] == 1)
      fprintf(setupFile, "Include GOL in the JTAG Chain\n");
    else
      fprintf(setupFile, "Exclude GOL in the JTAG Chain)\n");

    BinaryToInt(&val, MAXNUMBERMEZZ, 5, CSMSetupArray);
    fprintf(setupFile, "%8d  %8d  ", MAXNUMBERMEZZ, val);
    fprintf(setupFile, "Maximum Number of Allowed Mezzanine Cards\n");

    fprintf(setupFile, "%8d  %8d  ", CSMSPAREBIT01, CSMSetupArray[CSMSPAREBIT01]);
    fprintf(setupFile, "Spare Bit\n");
    
    fprintf(setupFile, "%8d  %8d  ", ENABLETTCRXTDI, CSMSetupArray[ENABLETTCRXTDI]);
    if (CSMSetupArray[ENABLETTCRXTDI] == 1)
      fprintf(setupFile, "Include TTCrx in the JTAG Chain\n");
    else
      fprintf(setupFile, "Exclude TTCrx in the JTAG Chain)\n");

    fprintf(setupFile, "%8d  %8d  ", TTCRXUSEPROM, CSMSetupArray[TTCRXUSEPROM]);
    if (CSMSetupArray[TTCRXUSEPROM] == 1)
      fprintf(setupFile, "TTCrx use PROM\n");
    else
      fprintf(setupFile, "TTCrx does not use PROM\n");

    BinaryToInt(&val, CSMNEXTSTATE, 4, CSMSetupArray);
    fprintf(setupFile, "%8d  %8d  ", CSMNEXTSTATE, val);
    strcpy(CSMCMDText[CMDIDLE], "Next CSM State: Idle State");
    strcpy(CSMCMDText[CMDRESETFPGA], "Next CSM State: Reset CSM FPGA Chip");
    strcpy(CSMCMDText[CMDRESETTTCRX], "Next CSM State: Reset TTCrx Chip");
    strcpy(CSMCMDText[CMDRESETGOL], "Next CSM State: Reset GOL Chip");
    strcpy(CSMCMDText[CMDWAITCLKLOCK], "Next CSM State: Wait Clock Lock");
    strcpy(CSMCMDText[CMDRESETANDWAITCLK25], "Next CSM State: Reset and Wait Clock25");
    strcpy(CSMCMDText[CMDSAMPLEAMTPHASE], "Next CSM State: Sample AMT Phase");
    strcpy(CSMCMDText[CMDJTAGRESET], "Next CSM State: JTAG Reset for TTCrx and GOL");
    strcpy(CSMCMDText[CMDTTCRXLOAD], "Next CSM State: Load TTCrx");
    strcpy(CSMCMDText[CMDWAITPROM], "Next CSM State: Wait Load TTCrx (PROM)");
    strcpy(CSMCMDText[CMDRESETAMT], "Next CSM State: Reset AMT");
    strcpy(CSMCMDText[CMDRESETERROR], "Next CSM State: Reset Error");
    strcpy(CSMCMDText[CMD12UNKNOWN], "Next CSM State: Undefined CSM State");
    strcpy(CSMCMDText[CMD13UNKNOWN], "Next CSM State: Undefined CSM State");
    strcpy(CSMCMDText[CMD14UNKNOWN], "Next CSM State: Undefined CSM State");
    strcpy(CSMCMDText[CMD15UNKNOWN], "Next CSM State: Undefined CSM State");
    fprintf(setupFile, "%s\n", CSMCMDText[val]);

    fprintf(setupFile, "%8d  %8d  ", CSMCMD, CSMSetupArray[CSMCMD]);
    if (CSMSetupArray[CSMCMD] == 1)
      fprintf(setupFile, "Execute next CSM state command\n");
    else
      fprintf(setupFile, "Ignore next CSM state command\n");

    fprintf(setupFile, "%8d  %8d  ", DISABLEBCIDMATCH, CSMSetupArray[DISABLEBCIDMATCH]);
    if (CSMSetupArray[DISABLEBCIDMATCH] == 1)
      fprintf(setupFile, "BCID Match is not required to build event\n");
    else
      fprintf(setupFile, "BCID Match is required to build event\n");

    fprintf(setupFile, "%8d  %8d  ", DISABLEEVIDMATCH, CSMSetupArray[DISABLEEVIDMATCH]);
    if (CSMSetupArray[DISABLEEVIDMATCH] == 1)
      fprintf(setupFile, "EVID Match is not required to build event\n");
    else
      fprintf(setupFile, "EVID Match is required to build event\n");

    fprintf(setupFile, "%8d  %8d  ", PASSALLAMTDATA, CSMSetupArray[PASSALLAMTDATA]);
    if (CSMSetupArray[PASSALLAMTDATA] == 1)
      fprintf(setupFile, "Accept all received AMT data word in building event\n");
    else
      fprintf(setupFile, "Drop unknown AMT data word in building event\n");

    fprintf(setupFile, "%8d  %8d  ", DROPAMTHDANDTR, CSMSetupArray[DROPAMTHDANDTR]);
    if (CSMSetupArray[DROPAMTHDANDTR] == 1)
      fprintf(setupFile, "Drop AMT header and trailer word in building event\n");
    else
      fprintf(setupFile, "Keep AMT header and trailer word in building event\n");

    BinaryToInt(&val, MEZZPAIRPRESCALE, 3, CSMSetupArray);
    fprintf(setupFile, "%8d  %8d  ", MEZZPAIRPRESCALE, val);
    if ((CSMSetupArray[MEZZCSMPAIRMODE]|CSMSetupArray[MEZZCSMPAIRDEBUGMODE]) == 1) {
      if (val < 8) {
        GetIndexFromValue(CSMSetupHandle, P_CSM_PAIRCONTROL, &index, val);
        GetLabelFromIndex(CSMSetupHandle, P_CSM_PAIRCONTROL, index, str);
        fprintf(setupFile, "%s\n", str);
      }
      else fprintf(setupFile, "Unknown Event Prescale setting (CSM Pair Mode Only)%d\n", val);
    }
    else {
      if (val < 8) {
        GetIndexFromValue(CSMSetupHandle, P_CSM_PAIRCONTROL, &index, val);
        GetLabelFromIndex(CSMSetupHandle, P_CSM_PAIRCONTROL, index, str);
        fprintf(setupFile, "UNUSED! %s\n", str);
      }
      else fprintf(setupFile, "UNUSED! Unknown Event Prescale setting (CSM Pair Mode Only)%d\n", val);
    }

    BinaryToInt(&val, MEZZBUFFTHRESHOLD1, 4, CSMSetupArray);
    fprintf(setupFile, "%8d  %8d  ", MEZZBUFFTHRESHOLD1, val);
    if (val < 16) fprintf(setupFile, "CSM FIFO Threshold 1 (discard Trailing Edge & TDC Mask Word)\n");
    else fprintf(setupFile, "Unknown CSM FIFO Threshold 1 value %d\n", val);

    BinaryToInt(&val, MEZZBUFFTHRESHOLD2, 4, CSMSetupArray);
    fprintf(setupFile, "%8d  %8d  ", MEZZBUFFTHRESHOLD2, val);
    if (val < 16) fprintf(setupFile, "CSM FIFO Threshold 2 (discard Leading Edge & TDC Error Word)\n");
    else fprintf(setupFile, "Unknown CSM FIFO Threshold 2 value %d\n", val);

    fclose(setupFile);
    printf("CSM setup is saved into file <%s>.\n", setupFilename);
  }
  else {
    printf("Unable to open CSM setup file <%s>!\n", setupFilename);
    MessagePopup("Failed to Open CSM Setup File",
                 "Unable to open CSM setup file, please check your disk and filename!");
  }
}


void SetAllMezzanineCard(void) {
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ00, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ01, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ02, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ03, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ04, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ05, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ06, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ07, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ08, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ09, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ10, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ11, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ12, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ13, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ14, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ15, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ16, TRUE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ17, TRUE);
}


void ClearAllMezzanineCard(void) {
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ00, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ01, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ02, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ03, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ04, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ05, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ06, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ07, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ08, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ09, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ10, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ11, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ12, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ13, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ14, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ15, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ16, FALSE);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZ17, FALSE);
}


void CSMDAQControl(void) {
  int enable;
  
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, &enable);
  if (enable) {
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDIDLE);
    SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 0);
    PanelSave(CSMSetupHandle);
  }
}


void CreatGOLLDList(void) {
  int golLaser, nlist, item, list, ctrlVal, ctrlItem;
  float time;
  char str[50];
  
  GetCtrlIndex(CSMSetupHandle, P_CSM_GOLLD, &ctrlItem);
  GetNumListItems(CSMSetupHandle, P_CSM_GOLLD, &nlist);
  DeleteListItem(CSMSetupHandle, P_CSM_GOLLD, 0, nlist);
  if ((nlist > 4) || (nlist <= 0)) nlist = 4;
  list = 0;
  GetCtrlVal(CSMSetupHandle, P_CSM_GOLLASER, &golLaser);
  for (item = nlist-1; item >= 0; item--) {
    if (golLaser == 1) {
      if (item == 0) sprintf(str, "%d (Laser-diode bias current 5.8 mA)", item);
      else if (item == 1) sprintf(str, "%d (Laser-diode bias current 7.4 mA)", item);
      else if (item == 2) sprintf(str, "%d (Laser-diode bias current 11.4 mA)", item);
      else if (item == 3) sprintf(str, "%d (Laser-diode bias current 16.2 mA)", item);
    }
    else sprintf(str, "%d (Line-driver strength %d)", item, item+1);
    if (item == ctrlItem) ctrlVal = nlist - item - 1;
    InsertListItem(CSMSetupHandle, P_CSM_GOLLD, list++, str, item);
  }
  SetCtrlIndex(CSMSetupHandle, P_CSM_GOLLD, ctrlItem);
  SetCtrlVal(CSMSetupHandle, P_CSM_GOLLD, ctrlVal);
}


void TTCrxPROMUsage(void) {
  int usePROM;
  
  GetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, &usePROM);
  if (usePROM) {
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDRESETTTCRX);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMNEXTSTATE, ATTR_TEXT_BGCOLOR, VAL_YELLOW);
  }
}


void PutCSMNextState(void) {
  int val, color = VAL_WHITE;
  
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, &val);
  if ((val != CMDIDLE) && (val != CMDRESETERROR))
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
  if (val == CMDTTCRXLOAD) SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, 1);
  if (val == CMD12UNKNOWN) color = VAL_RED;
  else if (val == CMD13UNKNOWN) color = VAL_RED;
  else if (val == CMD14UNKNOWN) color = VAL_RED;
  else if (val == CMD15UNKNOWN) color = VAL_RED;
  SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMNEXTSTATE, ATTR_TEXT_BGCOLOR, color);
}


void TurnOffUnwantedMezzCards(void) {
  int val, buttons[18], mezz;

  for (mezz = 0; mezz < 18; mezz++) buttons[mezz] = -1;
  buttons[0] = P_CSM_MEZZ00;
  buttons[1] = P_CSM_MEZZ01;
  buttons[2] = P_CSM_MEZZ02;
  buttons[3] = P_CSM_MEZZ03;
  buttons[4] = P_CSM_MEZZ04;
  buttons[5] = P_CSM_MEZZ05;
  buttons[6] = P_CSM_MEZZ06;
  buttons[7] = P_CSM_MEZZ07;
  buttons[8] = P_CSM_MEZZ08;
  buttons[9] = P_CSM_MEZZ09;
  buttons[10] = P_CSM_MEZZ10;
  buttons[11] = P_CSM_MEZZ11;
  buttons[12] = P_CSM_MEZZ12;
  buttons[13] = P_CSM_MEZZ13;
  buttons[14] = P_CSM_MEZZ14;
  buttons[15] = P_CSM_MEZZ15;
  buttons[16] = P_CSM_MEZZ16;
  buttons[17] = P_CSM_MEZZ17;
  GetCtrlVal(CSMSetupHandle, P_CSM_MAXALLOWEDMEZZS, &val);
  if (val < 1) val = 1;
  if (val > 18) val = 18;
  for (mezz = val; mezz < 18; mezz++) {
    SetCtrlVal(CSMSetupHandle, buttons[mezz], FALSE);
    SetCtrlAttribute(CSMSetupHandle, buttons[mezz], ATTR_DIMMED, TRUE);
  }
  for (mezz = 0; mezz < val; mezz++) {
    SetCtrlAttribute(CSMSetupHandle, buttons[mezz], ATTR_DIMMED, FALSE);
  }
}


void SetCSMModeAccordingToLinkRate(void) {
  int val, okColor = VAL_GREEN, warningColor = VAL_YELLOW;

  GetCtrlVal(CSMSetupHandle, P_CSM_CSMMEZZLINK, &val);
  if (val == 0) {
    SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMMODE, ATTR_ON_COLOR, warningColor);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMMODE, ATTR_OFF_COLOR, okColor);
  }
  else {
    SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMMODE, ATTR_ON_COLOR, okColor);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMMODE, ATTR_OFF_COLOR, warningColor);
  }
}


// Reset CSM
void ResetCSM(void) {
  int execCSMCMD, oldCMD;
  if (controlOption == NORM) {
    UpdateCSMControlBits();
    GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &execCSMCMD);
    GetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, &oldCMD);
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDRESETFPGA);
    CSMSetupDone();
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, execCSMCMD);
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, oldCMD);
    PanelSave(CSMSetupHandle);
    if (action == DOWNLOAD) {
      LoadCSMSetupArray();
      SetupJTAGChain();
      printf("Reset CSM FPGA (Vertex-II chip) Done.\n");
      GetCSMStatus();
	  printf("GetCSMStatus() is in Void ResetCSM()\n");
      ClearSetupStatus();
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Reset CSM",
                 "Reset CSM FPGA (Vertex-II chip), which is done with two steps:\n"
                 "1) Reset any CSM Error;\n"
                 "2) Reset CSM FPGA Chip.");
  }
  controlOption = NORM;
}


// Reset CSM Error
void ResetCSMError(void) {
  int execCSMCMD, oldCMD;
  
  UpdateCSMControlBits();
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &execCSMCMD);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, &oldCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDRESETERROR);
  CSMSetupDone();
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, execCSMCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, oldCMD);
  PanelSave(CSMSetupHandle);
  if (action == DOWNLOAD) {
    WaitInMillisecond(1000);     // Wait for 1 seconds
    printf("Reset CSM Error Done.\n");
    GetCSMStatus();
	printf("GetCSMStatus() is in Void ResetCSMError()\n");
  }
}


// Reset GOL
void ResetGOL(void) {
  int execCSMCMD, oldCMD;
  
  UpdateCSMControlBits();
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &execCSMCMD);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, &oldCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDRESETGOL);
  CSMSetupDone();
  if (action == DOWNLOAD) {
    WaitInMillisecond(1000);     // Wait for 1 seconds
    printf("Reset GOL Done.\n");
    if (!doingInitCSMDAQ) GetCSMStatus();
	printf("GetCSMStatus() is in void ResetGOL()\n");
  }
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, execCSMCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, oldCMD);
  PanelSave(CSMSetupHandle);
}


// Reset TTCrx
void ResetTTCrx(void) {
  int execCSMCMD, oldCMD, PROMUsage;
  
  UpdateCSMControlBits();
  GetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, &PROMUsage);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &execCSMCMD);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, &oldCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
  SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, 0);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDRESETTTCRX);
  CSMSetupDone();
  if (action == DOWNLOAD) {
    WaitInMillisecond(2);     // Wait for 2ms
    printf("Reset TTCrx Done.\n");
    if (!doingInitCSMDAQ) GetCSMStatus();
	printf("GetCSMStatus() is in void ResetTTCrx()\n");
  }
  SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, PROMUsage);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, execCSMCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, oldCMD);
  PanelSave(CSMSetupHandle);
}


// Reset and Download TTCrx, method 1
// Turn on TTCrx-use-PROM bit and use Reset TTCrx Command
// GOL will be reset after first power up, one needs to wait 0.8 seconds at least
void ResetAndDownloadTTCrxMethod1(void) {
  int execCSMCMD, oldCMD, PROMUsage;
  
  UpdateCSMControlBits();
  GetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, &PROMUsage);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &execCSMCMD);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, &oldCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
  SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDRESETTTCRX);
  CSMSetupDone();
  if (action == DOWNLOAD) {
    WaitInMillisecond(2);     // Wait for 2ms
    printf("Reset TTCrx and Download TTCrx/CSM (Reset TTCrx Command) Done.\n");
    if (!doingInitCSMDAQ) GetCSMStatus();
	printf("GetCSMStatus() is in void ResetAndDownloadTTCrxMethod1()\n");
  }
  SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, PROMUsage);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, execCSMCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, oldCMD);
  PanelSave(CSMSetupHandle);
}


// Reset and Download TTCrx, method 2
// Use the Load TTCrx command, where GOL will not be affected.
void ResetAndDownloadTTCrxMethod2(void) {
  int execCSMCMD, oldCMD, PROMUsage;
  
  UpdateCSMControlBits();
  GetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, &PROMUsage);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &execCSMCMD);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, &oldCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
  SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDTTCRXLOAD);
  CSMSetupDone();
  if (action == DOWNLOAD) {
    WaitInMillisecond(2);     // Wait for 2ms
    printf("Reset TTCrx and Download TTCrx/CSM (Load TTCrx Command) Done.\n");
    if (!doingInitCSMDAQ) GetCSMStatus();
  }
  SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, PROMUsage);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, execCSMCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, oldCMD);
  PanelSave(CSMSetupHandle);
}


// Reset GOL
void ResetMezzanineCards(void) {
  int execCSMCMD, oldCMD;
  
  UpdateCSMControlBits();
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &execCSMCMD);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, &oldCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDRESETAMT);
  CSMSetupDone();
  if (action == DOWNLOAD) {
    printf("Reset Mezzanine Cards Done.\n");
    if (!doingInitCSMDAQ) GetCSMStatus();
	printf("GetCSMStatus() is in void ResetMezzanineCards()\n");
  }
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, execCSMCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, oldCMD);
  PanelSave(CSMSetupHandle);
}


void StartCSMDAQ(void) {
  UpdateCSMControlBits();
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDIDLE);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 1);
  CSMSetupDone();
  if (action == DOWNLOAD) GetCSMStatus();
  printf("GetCSMStatus() is in StartCSMDAQ()\n");
}


void StopCSMDAQ(void) {
  UpdateCSMControlBits();
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDIDLE);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
  CSMSetupDone();
  if (action == DOWNLOAD) GetCSMStatus();
  printf("GetCSMStatus() is in StopCSMDAQ()\n");   
}


void StopCSMDAQAndTurnOffMezzJTAGSignals(void) {
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDIDLE);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, 0);
  CSMSetupDone();
  if (action == DOWNLOAD) GetCSMStatus();
  printf("GetCSMStatus() is in void StopCSMDAQAndTurnOffMezzJTAGSignals(void)\n");   
}


// Sample AMT Phase
void CSMSampleAMTPhase(void) {
  int execCSMCMD, oldCMD;
  
  UpdateCSMControlBits();
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &execCSMCMD);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, &oldCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, 1);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, 0);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, CMDSAMPLEAMTPHASE);
  CSMSetupDone();
  if (action == DOWNLOAD) {
    WaitInMillisecond(1);
    printf("Sample AMT Phase Command Issued.\n");
    if (!doingInitCSMDAQ) GetCSMStatus();
  	printf("GetCSMStatus() is in void CSMSampleAMTPhase(void)\n"); 	
	
  }
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, execCSMCMD);
  SetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, oldCMD);
  PanelSave(CSMSetupHandle);
}


int GetAMTPhaseLockStatus(void) {
  int status = 0;
  
  return status;
}


void UpdateCSMControlBits(void) {
  int enable;
  
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_MEZZCARDS, &enable);
  SetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, enable);
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_GOL, &enable);
  SetCtrlVal(CSMSetupHandle, P_CSM_GOLTDI, enable);
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_TTCRX, &enable);
  SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXTDI, enable);
  PanelSave(CSMSetupHandle);
}


void UpdateCSMSetupPanel(void) {
  static int type, oldType = -1, laser;
  int color, okColor, errColor, warningColor, val, dimmed, warning;
  char str[80], strval[6];
  
  if (validCSMStatus) {
    color = VAL_WHITE;
    okColor = VAL_GREEN;
    errColor = VAL_RED;
    warningColor = VAL_YELLOW;
    SetCtrlVal(CSMSetupHandle, P_CSM_VERSIONDATE, CSMVersionDate);
    if (CSMVersionDate > 2004000000) SetCtrlAttribute(CSMSetupHandle, P_CSM_VERSIONDATE, ATTR_TEXT_BGCOLOR, color);
    else SetCtrlAttribute(CSMSetupHandle, P_CSM_VERSIONDATE, ATTR_TEXT_BGCOLOR, errColor);
    BinaryToInt(&CSMVersion, CSMVERSION, 12, CSMStatusArray);
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMVERSION, CSMVersion);
    if (CSMVersion > 0 && CSMVersion < 64) {
      GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMTYPE, &type);
      if (type == CSM1) {
        laser = 0;
        SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLLASER, ATTR_ON_COLOR, errColor);
        SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLLASER, ATTR_OFF_COLOR, okColor);
      }
      else {
        laser = 1;
        SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLLASER, ATTR_ON_COLOR, okColor);
        SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLLASER, ATTR_OFF_COLOR, errColor);
      }
      if (type != oldType) SetCtrlVal(CSMSetupHandle, P_CSM_GOLLASER, laser);
      oldType = type;
      SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMVERSION, ATTR_TEXT_BGCOLOR, color);
      SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLTDI, ATTR_ON_COLOR, VAL_GREEN);
      SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLTDI, ATTR_OFF_COLOR, VAL_CYAN);
      SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLTDI, ATTR_ON_TEXT, "Enable GOL TDI");
      SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLTDI, ATTR_OFF_TEXT, "Disable GOL TDI");
      SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXTDI, ATTR_ON_COLOR, VAL_GREEN);
      SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXTDI, ATTR_OFF_COLOR, VAL_CYAN);
      SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXTDI, ATTR_ON_TEXT, "Enable TTCrx TDI");
      SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXTDI, ATTR_OFF_TEXT, "Disable TTCrx TDI");
    }
    else SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMVERSION, ATTR_TEXT_BGCOLOR, errColor);
    if ((CSMVersion%2) == 0) dimmed = FALSE;
    else dimmed = TRUE;
    SetCtrlAttribute(CSMSetupHandle, P_CSM_EVIDMATCH, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_BCIDMATCH, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_PASSALLAMTDATA, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_DROPAMTHDTR, ATTR_DIMMED, dimmed);
    if ((CSMVersion%2) == 0) dimmed = TRUE;
    else dimmed = FALSE;
    SetCtrlAttribute(CSMSetupHandle, P_CSM_EMPTYCYCLECONTROL, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_SYNCHWORDCONTROL, ATTR_DIMMED, dimmed);
    SetCtrlVal(CSMSetupHandle, P_CSM_GOLREADY, CSMStatusArray[GOLREADYBIT]);
    SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXREADY, CSMStatusArray[TTCRXREADYBIT]);
    SetCtrlVal(CSMSetupHandle, P_CSM_LHCLOCK, CSMStatusArray[LHCLOCKBIT]);
    SetCtrlVal(CSMSetupHandle, P_CSM_XMTLOCK1, CSMStatusArray[XMTLOCK1BIT]);
    SetCtrlVal(CSMSetupHandle, P_CSM_XMTLOCK2, CSMStatusArray[XMTLOCK2BIT]);
    SetCtrlVal(CSMSetupHandle, P_CSM_ORUNUSED, CSMStatusArray[ORUNUSEDBIT]);
    SetCtrlVal(CSMSetupHandle, P_CSM_TTCLOADSTATUS, CSMStatusArray[TTCLOADSTATUSBIT]);
    BinaryToInt(&val, CSMSTATE, 4, CSMStatusArray);
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMSTATE, val);
    if (val == CMDIDLE) SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMSTATE, ATTR_TEXT_BGCOLOR, color);
    else SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMSTATE, ATTR_TEXT_BGCOLOR, errColor);
    SetCtrlVal(CSMSetupHandle, P_CSM_PHASEERROR, CSMStatusArray[PHASEERRORBIT]);
    SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXSETUPERRI2C, CSMStatusArray[TTCRXI2CCHECKERROR]);
    SetCtrlVal(CSMSetupHandle, P_CSM_TTCRXSETUPERRDUMP, CSMStatusArray[TTCRXDUMPCHECKERROR]);
    SetCtrlVal(CSMSetupHandle, P_CSM_I2COPERATION, CSMStatusArray[I2COPERATIONBIT]);
    SetCtrlVal(CSMSetupHandle, P_CSM_CSMERROR, CSMStatusArray[CSMERRORBIT]);
    BinaryToInt(&AMTParityError, PARITYERROR, 18, CSMStatusArray);
    SetCtrlVal(CSMSetupHandle, P_CSM_PARITYERROR, AMTParityError);
    if (AMTParityError == 0) SetCtrlAttribute(CSMSetupHandle, P_CSM_PARITYERROR, ATTR_TEXT_BGCOLOR, color);
    else SetCtrlAttribute(CSMSetupHandle, P_CSM_PARITYERROR, ATTR_TEXT_BGCOLOR, errColor);
    SetCtrlVal(CSMSetupHandle, P_CSM_AMTPHASEERROR, AMTPhaseError);
    if (AMTPhaseError == 0) SetCtrlAttribute(CSMSetupHandle, P_CSM_AMTPHASEERROR, ATTR_TEXT_BGCOLOR, color);
    else SetCtrlAttribute(CSMSetupHandle, P_CSM_AMTPHASEERROR, ATTR_TEXT_BGCOLOR, errColor);
	warning = FALSE;
    strcpy(str, "");
    BinaryToInt(&val, TTCRXCONFIG3, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXCONFIG2, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXCONFIG1, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXMASTERMODEBI2CID, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXMASTERMODEAID8, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXID0, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXCONTROL, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXCOARSEDELAY, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXFINEDELAY2, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXFINEDELAY1, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    if (CSMStatusArray[TTCRXI2CCHECKERROR] != 0) warning = TRUE;
    if (CSMStatusArray[TTCRXDUMPCHECKERROR] != 0) warning = TRUE;
    ReplaceTextBoxLine(CSMSetupHandle, P_CSM_TTCRXSETUPBITS, 0, str);
    if (!warning) SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXSETUPBITS, ATTR_TEXT_BGCOLOR, color);
    else SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXSETUPBITS, ATTR_TEXT_BGCOLOR, warningColor);

	warning = FALSE;
    strcpy(str, "");
    BinaryToInt(&val, TTCRXEVENTCOUNTER16, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXEVENTCOUNTER08, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXEVENTCOUNTER00, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXBUNCHCOUNTER08, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXBUNCHCOUNTER00, 8, CSMStatusArray);
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXSTATUSREGISTER, 8, CSMStatusArray);
    if ((val & TTCRXSTSALWAYS0BITS) != 0) warning = TRUE;
    if (((val >> TTCRXSTSAUTORESETFLAG) & 1) != 0) warning = TRUE;
    if (((val >> TTCRXSTSFRAMESYNCH) & 1) != 1) warning = TRUE;
    if (((val >> TTCRXSTSDLLREADY) & 1) != 1) warning = TRUE;
    if (((val >> TTCRXSTSDLLREADY) & 1) != 1) warning = TRUE;
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXSEUERRORCOUNT, 8, CSMStatusArray);
    if (val != 0) warning = TRUE;
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXDERRORCOUNT, 8, CSMStatusArray);
    if (val != 0) warning = TRUE;
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXSERRORCOUNT8, 8, CSMStatusArray);
    if (val != 0) warning = TRUE;
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    BinaryToInt(&val, TTCRXSERRORCOUNT0, 8, CSMStatusArray);
    if (val != 0) warning = TRUE;
    sprintf(strval, " %02X", val);
    strcat(str, strval);
    ReplaceTextBoxLine(CSMSetupHandle, P_CSM_TTCRXSTATUSBITS, 0, str);
    if (!warning) SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXSTATUSBITS, ATTR_TEXT_BGCOLOR, color);
    else SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXSTATUSBITS, ATTR_TEXT_BGCOLOR, warningColor);
  }
  else {
    color = VAL_BLACK;
    okColor = VAL_BLACK;
    errColor = VAL_BLACK;
    warningColor = VAL_BLACK;
    SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMVERSION, ATTR_TEXT_BGCOLOR, color);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_VERSIONDATE, ATTR_TEXT_BGCOLOR, color);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMSTATE, ATTR_TEXT_BGCOLOR, color);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_PARITYERROR, ATTR_TEXT_BGCOLOR, color);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_AMTPHASEERROR, ATTR_TEXT_BGCOLOR, color);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXSETUPBITS, ATTR_TEXT_BGCOLOR, color);
    SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXSTATUSBITS, ATTR_TEXT_BGCOLOR, color);
  }
  SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMERROR, ATTR_ON_COLOR, errColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_CSMERROR, ATTR_OFF_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLREADY, ATTR_ON_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_GOLREADY, ATTR_OFF_COLOR, errColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXREADY, ATTR_ON_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXREADY, ATTR_OFF_COLOR, errColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_LHCLOCK, ATTR_ON_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_LHCLOCK, ATTR_OFF_COLOR, errColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_XMTLOCK1, ATTR_ON_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_XMTLOCK1, ATTR_OFF_COLOR, errColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_XMTLOCK2, ATTR_ON_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_XMTLOCK2, ATTR_OFF_COLOR, errColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCLOADSTATUS, ATTR_ON_COLOR, errColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCLOADSTATUS, ATTR_OFF_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_PHASEERROR, ATTR_ON_COLOR, errColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_PHASEERROR, ATTR_OFF_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXSETUPERRI2C, ATTR_ON_COLOR, warningColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXSETUPERRI2C, ATTR_OFF_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXSETUPERRDUMP, ATTR_ON_COLOR, warningColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_TTCRXSETUPERRDUMP, ATTR_OFF_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_I2COPERATION, ATTR_ON_COLOR, warningColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_I2COPERATION, ATTR_OFF_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_ORUNUSED, ATTR_ON_COLOR, okColor);
  SetCtrlAttribute(CSMSetupHandle, P_CSM_ORUNUSED, ATTR_OFF_COLOR, okColor);
}


void LoadCSMSetupArray(void) {
  static int type, oldType = -1, laser, val;
  
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_CSMTYPE, &type);
  if (type == CSM1) laser = 0;
  else laser = 1;
  if (type != oldType) SetCtrlVal(CSMSetupHandle, P_CSM_GOLLASER, laser);
  oldType = type;
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ00, &CSMSetupArray[TDCENABLES+0]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ01, &CSMSetupArray[TDCENABLES+1]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ02, &CSMSetupArray[TDCENABLES+2]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ03, &CSMSetupArray[TDCENABLES+3]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ04, &CSMSetupArray[TDCENABLES+4]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ05, &CSMSetupArray[TDCENABLES+5]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ06, &CSMSetupArray[TDCENABLES+6]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ07, &CSMSetupArray[TDCENABLES+7]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ08, &CSMSetupArray[TDCENABLES+8]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ09, &CSMSetupArray[TDCENABLES+9]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ10, &CSMSetupArray[TDCENABLES+10]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ11, &CSMSetupArray[TDCENABLES+11]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ12, &CSMSetupArray[TDCENABLES+12]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ13, &CSMSetupArray[TDCENABLES+13]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ14, &CSMSetupArray[TDCENABLES+14]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ15, &CSMSetupArray[TDCENABLES+15]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ16, &CSMSetupArray[TDCENABLES+16]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZ17, &CSMSetupArray[TDCENABLES+17]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MEZZJTAGENABLE, &CSMSetupArray[MEZZJTAGENABLE]);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMDAQCONTROL, &CSMSetupArray[CSMDAQCONTROL]);
  GetCtrlVal(CSMSetupHandle, P_CSM_AMTCOMMANDDELAY, &val);
  IntToBinary(val, AMTCOMMANDDELAY, 7, CSMSetupArray, CSMSETUPLENGTH);
  AMTCommandDelay = val;
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMMEZZLINK, &CSMSetupArray[MEZZ80MHZOPERATION]);
  GetCtrlVal(CSMSetupHandle, P_CSM_PWIDTHRESOLUTION, &val);
  IntToBinary(val, MEZZPAIRWIDTHRESOLUTION, 3, CSMSetupArray, CSMSETUPLENGTH);
  CSMPairWidthResolution = val;
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMMODE, &CSMSetupArray[MEZZCSMPAIRMODE]);
  CSMPairMode = CSMSetupArray[MEZZCSMPAIRMODE];
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMPAIRDEBUG, &CSMSetupArray[MEZZCSMPAIRDEBUGMODE]);
  CSMPairDebug = CSMSetupArray[MEZZCSMPAIRDEBUGMODE];
  GetCtrlVal(CSMSetupHandle, P_CSM_EMPTYCYCLECONTROL, &CSMSetupArray[EMPTYCYCLECONTROL]);
  GetCtrlVal(CSMSetupHandle, P_CSM_SYNCHWORDCONTROL, &CSMSetupArray[SYNCHWORDCONTROL]);
  GetCtrlVal(CSMSetupHandle, P_CSM_GOLDIFF, &CSMSetupArray[GOLDIFF]);
  GetCtrlVal(CSMSetupHandle, P_CSM_GOLLD, &val);
  IntToBinary(val, GOLLD0, 2, CSMSetupArray, CSMSETUPLENGTH);
  GetCtrlVal(CSMSetupHandle, P_CSM_GOLPLL, &CSMSetupArray[GOLPLL]);
  GetCtrlVal(CSMSetupHandle, P_CSM_GOLLASER, &CSMSetupArray[GOLLASER]);
  GetCtrlVal(CSMSetupHandle, P_CSM_GOLNEG, &CSMSetupArray[GOLNEG]);
  GetCtrlVal(CSMSetupHandle, P_CSM_GOLMODE, &CSMSetupArray[GOLMODE]);
  GetCtrlVal(CSMSetupHandle, P_CSM_GOLTDI, &CSMSetupArray[ENABLEGOLTDI]);
  GetCtrlVal(CSMSetupHandle, P_CSM_MAXALLOWEDMEZZS, &val);
  IntToBinary(val, MAXNUMBERMEZZ, 5, CSMSetupArray, CSMSETUPLENGTH);
  CSMMaxAllowedMezzs = val;
  GetCtrlVal(CSMSetupHandle, P_CSM_TTCRXTDI, &CSMSetupArray[ENABLETTCRXTDI]);
  GetCtrlVal(CSMSetupHandle, P_CSM_TTCRXUSEPROM, &CSMSetupArray[TTCRXUSEPROM]);
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMNEXTSTATE, &val);
  IntToBinary(val, CSMNEXTSTATE, 4, CSMSetupArray, CSMSETUPLENGTH);
  CSMNextState = val;
  GetCtrlVal(CSMSetupHandle, P_CSM_CSMCMD, &CSMSetupArray[CSMCMD]);
  GetCtrlVal(CSMSetupHandle, P_CSM_BCIDMATCH, &CSMSetupArray[DISABLEBCIDMATCH]);
  GetCtrlVal(CSMSetupHandle, P_CSM_EVIDMATCH, &CSMSetupArray[DISABLEEVIDMATCH]);
  GetCtrlVal(CSMSetupHandle, P_CSM_PASSALLAMTDATA, &CSMSetupArray[PASSALLAMTDATA]);
  GetCtrlVal(CSMSetupHandle, P_CSM_DROPAMTHDTR, &CSMSetupArray[DROPAMTHDANDTR]);
  GetCtrlVal(CSMSetupHandle, P_CSM_PAIRCONTROL, &val);
  IntToBinary(val, MEZZPAIRPRESCALE, 3, CSMSetupArray, CSMSETUPLENGTH);
  GetCtrlVal(CSMSetupHandle, P_CSM_FIFOTHRESHOLD1, &val);
  IntToBinary(val, MEZZBUFFTHRESHOLD1, 4, CSMSetupArray, CSMSETUPLENGTH);
  GetCtrlVal(CSMSetupHandle, P_CSM_FIFOTHRESHOLD2, &val);
  IntToBinary(val, MEZZBUFFTHRESHOLD2, 4, CSMSetupArray, CSMSETUPLENGTH);
  GetCtrlVal(CSMSetupHandle, P_CSM_SPARE, &val);
  CSMSetupArray[CSMSPAREBIT00] = (val >> 0) & 1;
  CSMSetupArray[CSMSPAREBIT01] = (val >> 2) & 1;
  GetMezzCardEnables();
}

