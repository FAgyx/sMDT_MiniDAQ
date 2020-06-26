// golSetup.c                         by T.S.Dai
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
#include "golSetup.h"

extern int Menu00Handle, JTAGControlHandle;
extern void UpTTCrxSetupControl(void);
extern void UpCSMSetupControl(void);
extern void GetGOLStatus(void);
extern void GetGOLBoundaryScan(void);
extern void DownloadGOLSetup(void);
extern void UpdateJTAGControlPanel(void);

// Bring up GOL Setup Control Panel
void UpGOLSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    PanelRecall(GOLSetupHandle);
    GetGOLStatus();
    GetGOLConfigure();
    if ((confClock < 0) || (confClock > 1)) {
      printf("Unknown GOL clock setting of confClock = %d!\n", confClock);
      MessagePopup("Unknown GOL clock",
                   "Unknown GOL clock setting, please check your program!");
    }
    else {
      HidePanel(panel);
      UpdateGOLSetupPanel();
      PanelSave(GOLSetupHandle);
      DisplayPanel(GOLSetupHandle);
    }
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup GOL",
                 "Bring up GOL Setup Control Panel.\n"
                 "Note: For GOL settings, consult the GOL manual.");
  }
  controlOption = NORM;
}


// Load GOL Setup Control Panel and define user buttons
int GOLSetupPanelAndButton(void) {
  // Define the GOL Setup Control panel handles
  if ((GOLSetupHandle = LoadPanel(0, "DAQGUI.uir", P_GOL)) < 0) return -1;
  SetPanelAttribute(GOLSetupHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);

  // GOL Setup Control Panel Buttons
  SetupUserButton(Menu00Handle, MENU00_JTAG_SETUPGOL, UpGOLSetupControl);
  SetupUserButton(GOLSetupHandle, P_GOL_DONE, GOLSetupDone);
  SetupUserButton(GOLSetupHandle, P_GOL_DEFAULT, PanelDefault);
  SetupUserButton(GOLSetupHandle, P_GOL_CANCEL, PanelCancel);
  SetupUserButton(GOLSetupHandle, P_GOL_SAVESETUP, SaveGOLSetup);
  SetupUserButton(GOLSetupHandle, P_GOL_TTCRXCONTROL, UpTTCrxSetupControl);
  SetupUserButton(GOLSetupHandle, P_GOL_CSMCONTROL, UpCSMSetupControl);
  SetupUserButton(GOLSetupHandle, P_GOL_CURRENTCONTROL, GOLCurrentControl);
  SetupUserButton(GOLSetupHandle, P_GOL_DRIVERSTRENGTH, GOLDriverStrength);
 
  return 0;
}


// GOL Setup Done
void GOLSetupDone(void) {
  PanelSave(GOLSetupHandle);
  LoadGOLSetupArray();
  HidePanel(GOLSetupHandle);
  DownloadGOLSetup();
  UpdateJTAGControlPanel();     // Update JTAG control panel
  DisplayPanel(JTAGControlHandle);
  SetActivePanel(JTAGControlHandle);
}


void SaveGOLSetup(void) {
  int value, bit, strength;
  float time;
  char setupFilename[281] = "GOLSetup.txt";
  FILE *setupFile;
  
  PromptPopup("Get GOL Setup File Name",
              "Please give the file name (Default: GOLSetup.txt) for saving GOL setup",
              setupFilename, 280);
  if (strlen(setupFilename) <= 5 ) strcpy(setupFilename, "GOLSetup.txt");
  if (setupFile = fopen(setupFilename, "w")) {
    LoadGOLSetupArray();
    fprintf(setupFile, "--------- GOL Setup File -------------\n");
    fprintf(setupFile, "Position  Settings  Description...\n");
    BinaryToInt(&value, WAITTIMEFORREADY, 5, GOLSetupArray);
    fprintf(setupFile, "%8d  %8d  ", WAITTIMEFORREADY, value);
    if (value < 20) time = ((1 << value) + 1) * GOLClockPeriod();
    else time = ((1 << 20) + 1) * GOLClockPeriod();
    if (time < 1000.0)
      fprintf(setupFile, "Wait %.2f ns from PLL Locked to Ready\n", time);
    else if (time < 1.0E6)
      fprintf(setupFile, "Wait %.2f us from PLL Locked to Ready\n", time/1000.);
    else if (time < 1.0E9)
      fprintf(setupFile, "Wait %.2f ms from PLL Locked to Ready\n", time/1000000.);

    BinaryToInt(&value, LOSSOFLOCKTIME, 3, GOLSetupArray);
    fprintf(setupFile, "%8d  %8d  ", LOSSOFLOCKTIME, value);
    if (value == 0) time = 0.0;
    else if (value < 6) time = (1 << value) * GOLClockPeriod();
    else time = -1.0;
    if (time < 0.0)
      fprintf(setupFile, "Unknown tolerable time for lose of lock\n");
    else if (time < 1000.0)
      fprintf(setupFile, "Allow %.2f ns for lose of lock\n", time);
    else if (time < 1.0E6)
      fprintf(setupFile, "Allow %.2f us for lose of lock\n", time/1000.);
    else if (time < 1.0E9)
      fprintf(setupFile, "Allow %.2f ms for lose of lock\n", time/1000000.);

    BinaryToInt(&value, PLLLOCKTIME, 4, GOLSetupArray);
    fprintf(setupFile, "%8d  %8d  ", PLLLOCKTIME, value);
    if (value < 10) time = (1 << value) * GOLClockPeriod();
    else time = (1 << 10) * GOLClockPeriod();
    if (time < 1000.0)
      fprintf(setupFile, "Wait %.2f ns from PLL unlocked to PLL locked\n", time);
    else if (time < 1.0E6)
      fprintf(setupFile, "Wait %.2f us from PLL unlocked to PLL locked\n", time/1000.);
    else if (time < 1.0E9)
      fprintf(setupFile, "Wait %.2f ms from PLL unlocked to PLL locked\n", time/1000000.);

    fprintf(setupFile, "%8d  %8d  ", LOSSOFLOCKCONTROL, GOLSetupArray[LOSSOFLOCKCONTROL]);
    if (GOLSetupArray[LOSSOFLOCKCONTROL] == 0) 
      fprintf(setupFile, "Reinitializing immediately GOL chip after lossing of PLL lock\n");
    else
      fprintf(setupFile, "Wait till loss_of_time passed to reinitialize GOL chip after PLL lock lost\n");
    
    fprintf(setupFile, "%8d  %8d  ", LOSSOFLOCKCNTCONTROL, GOLSetupArray[LOSSOFLOCKCNTCONTROL]); 
    if (GOLSetupArray[LOSSOFLOCKCNTCONTROL] == 0) 
      fprintf(setupFile, "No loss_of_lock counter transmission at event of lock lost\n");
    else
      fprintf(setupFile, "Automatically transmit loss_of_lock counter at event of lock lost\n");

    fprintf(setupFile, "%8d  %8d  ", LOCKCONTROL, GOLSetupArray[LOCKCONTROL]); 
    if (GOLSetupArray[LOCKCONTROL] == 0) 
      fprintf(setupFile, "Enable lock state machine for Normal Operation\n");
    else
      fprintf(setupFile, "Disable lock state machine (lock always set for test only)\n");

    fprintf(setupFile, "%8d  %8d  ", DATATRANSCONTROL, GOLSetupArray[DATATRANSCONTROL]); 
    if (GOLSetupArray[DATATRANSCONTROL] == 0) 
      fprintf(setupFile, "Normal GOL data transmission\n");
    else
      fprintf(setupFile, "Enable data transmission self-test\n");

    BinaryToInt(&value, CHARGEPUMPCURRENT, 5, GOLSetupArray);
    fprintf(setupFile, "%8d  %8d  ", CHARGEPUMPCURRENT, value);
    fprintf(setupFile, "PLL charge pump current %.2f uA\n", 1.25*value);

    BinaryToInt(&value, TESTSIGNALS, 2, GOLSetupArray);
    fprintf(setupFile, "%8d  %8d  ", TESTSIGNALS, value);
    fprintf(setupFile, "Select test signals to test pads\n");

    fprintf(setupFile, "%8d  %8d  ", FLAGBITSCONTROL, GOLSetupArray[FLAGBITSCONTROL]); 
    if (GOLSetupArray[DATATRANSCONTROL] == 0) 
      fprintf(setupFile, "Using internal flag bits in G-link\n");
    else
      fprintf(setupFile, "Using external flag bits in G-link\n");

    BinaryToInt(&value, DRIVERCURRSTRENGTH, 7, GOLSetupArray);
    fprintf(setupFile, "%8d  %8d  ", DRIVERCURRSTRENGTH, value);
    if (confLaser == 1)
      fprintf(setupFile, "Config. Register: Laser driver current is %.2f mA\n", 1.0 + 0.4*value);
    else {
      strength = 0;
      for (bit = 3; bit < 7; bit++) strength += (value >> bit) & 1;
      fprintf(setupFile, "Config. Register: Liner driver strength is %d\n", strength);
    }

    fprintf(setupFile, "%8d  %8d  ", CURRENTCONTROL, GOLSetupArray[CURRENTCONTROL]); 
    if (confLaser == 1) {
      if (GOLSetupArray[CURRENTCONTROL] == 0) 
        fprintf(setupFile, "Laser-diode bias current controled by pad\n");
      else
        fprintf(setupFile, "Laser-diode bias current controled by configuration register\n");
    }
    else {
      if (GOLSetupArray[CURRENTCONTROL] == 0) 
        fprintf(setupFile, "Line driver strength controled by pad\n");
      else
        fprintf(setupFile, "Line driver strength controled by configuration register\n");
    }

    fclose(setupFile);
    printf("GOL setup is saved into file <%s>.\n", setupFilename);
  }
  else {
    printf("Unable to open GOL setup file <%s>!\n", setupFilename);
    MessagePopup("Failed to Open GOL Setup File",
                 "Unable to open GOL setup file, please check your disk and filename!");
  }
}


void GOLCurrentControl(void) {
  int currentControl, dimmed;
  
  GetCtrlVal(GOLSetupHandle, P_GOL_CURRENTCONTROL, &currentControl);
  if (currentControl == 0) dimmed = TRUE;
  else dimmed = FALSE;
  SetCtrlAttribute(GOLSetupHandle, P_GOL_PLLCURRENT, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(GOLSetupHandle, P_GOL_DRIVERSTRENGTH, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(GOLSetupHandle, P_GOL_STRENGTHTEXT, ATTR_DIMMED, dimmed);
}


void GOLDriverStrength(void) {
  char str[16];
  static int prevConfLaser = -999, val, strength, bit;
  
  GetCtrlVal(GOLSetupHandle, P_GOL_DRIVER, &confLaser);
  if (prevConfLaser != confLaser) {
    if (confLaser == 1)
      SetCtrlAttribute(GOLSetupHandle, P_GOL_DRIVERSTRENGTH, ATTR_LABEL_TEXT, "LD Bias Current");
    else
      SetCtrlAttribute(GOLSetupHandle, P_GOL_DRIVERSTRENGTH, ATTR_LABEL_TEXT, "Driver Strength");
  }
  GetCtrlVal(GOLSetupHandle, P_GOL_DRIVERSTRENGTH, &val);
  if (confLaser == 1) sprintf(str, "%.1f mA", 1.0 + 0.4*val);
  else {
    strength = 0;
    for (bit = 3; bit < 7; bit++) strength += (val >> bit) & 1;
    sprintf(str, "%d", strength);
  }
  ReplaceTextBoxLine(GOLSetupHandle, P_GOL_STRENGTHTEXT, 0, str);
}


void GetGOLConfigure(void) {
  // confClock = 0     25MHz (40ns) GOL clock
  //             1     40MHz (25ns) GOL clock
  confClock = 0;
  confLinkMode = 0;
  confWordMode = 0;
  confLaser = -1;
  tx_er = -1;
  tx_en = -1;
  testShift = -1;
  GOLReady = -1;
  GOLReset = -1;
  GetGOLBoundaryScan();
  if (validGOLStatus) {
    confLaser = GOLBScanArray[GOLBSCANCONFLASER];
    tx_er = GOLBScanArray[GOLBSCANTXER];
    tx_en = GOLBScanArray[GOLBSCANTXEN];
    testShift = GOLBScanArray[GOLBSCANTESTSHIFT];
    GOLReady = GOLBScanArray[GOLBSCANREADY];
    GOLReset = !GOLBScanArray[GOLBSCANRESETB];
  }
}


void UpdateGOLSetupPanel(void) {
  int nlist, item, ctrlVal, mode, colorCode;
  
  GetNumListItems(GOLSetupHandle, P_GOL_CLOCK, &nlist);
  for (item = 0; item < nlist; item++) {
    GetValueFromIndex(GOLSetupHandle, P_GOL_CLOCK, item, &ctrlVal);
    if (ctrlVal == confClock) {
      SetCtrlIndex(GOLSetupHandle, P_GOL_CLOCK, item);
      break;
    }
  }
  CreatPLLLockTimeList();
  CreatWaitTimeList();
  CreatLossOfLockTimeList();
  
  // Set varies GOL status
  linkStatusA = -1;
  linkStatusB = -1;
  linkStatusC = -1;
  numberOfLockLost = -1;
  hammingChecksum = -1;
  if (validGOLStatus) {
    BinaryToInt(&linkStatusA, LINKCONTROLSTATEA, 2, GOLSetupArray);
    BinaryToInt(&linkStatusB, LINKCONTROLSTATEB, 2, GOLSetupArray);
    BinaryToInt(&linkStatusC, LINKCONTROLSTATEC, 2, GOLSetupArray);
    BinaryToInt(&numberOfLockLost, LOSSOFLOCKCOUNTER, 8, GOLSetupArray);
    BinaryToInt(&hammingChecksum, HAMMINGCHECKSUM, 7, GOLSetupArray);
    confLinkMode = GOLSetupArray[CONFIGLINKMODE];
    confWordMode = GOLSetupArray[CONFIGWORDMODE];
  }
  
  mode = 2*confLinkMode + confWordMode;
  GetNumListItems(GOLSetupHandle, P_GOL_LINKMODE, &nlist);
  for (item = 0; item < nlist; item++) {
    GetValueFromIndex(GOLSetupHandle, P_GOL_LINKMODE, item, &ctrlVal);
    if (ctrlVal == mode) {
      SetCtrlIndex(GOLSetupHandle, P_GOL_LINKMODE, item);
      break;
    }
  }

  GOLCurrentControl();
  if (confLaser < 0) colorCode = VAL_BLACK;
  else {
    colorCode = VAL_GREEN;
    SetCtrlVal(GOLSetupHandle, P_GOL_DRIVER, confLaser);
  }
  SetCtrlAttribute(GOLSetupHandle, P_GOL_DRIVER, ATTR_ON_COLOR, colorCode);
  SetCtrlAttribute(GOLSetupHandle, P_GOL_DRIVER, ATTR_OFF_COLOR, colorCode);
  GOLDriverStrength();

  if ((linkStatusA < 0) || (linkStatusA > 3)) colorCode = VAL_BLACK;
  else {
    GetNumListItems(GOLSetupHandle, P_GOL_LINKSTATEA, &nlist);
    for (item = 0; item < nlist; item++) {
      GetValueFromIndex(GOLSetupHandle, P_GOL_LINKSTATEA, item, &ctrlVal);
      if (ctrlVal == linkStatusA) {
        SetCtrlIndex(GOLSetupHandle, P_GOL_LINKSTATEA, item);
        break;
      }
    }
    if (ctrlVal == GOLREADY) colorCode = VAL_GREEN;
    else if (ctrlVal == GOLOUTOFLOCK) colorCode = VAL_RED;
    else if (ctrlVal == GOLLOCKED) colorCode = VAL_YELLOW;
    else colorCode = VAL_YELLOW;
  }
  SetCtrlAttribute(GOLSetupHandle, P_GOL_LINKSTATEA, ATTR_TEXT_BGCOLOR, colorCode);
  
  if ((linkStatusB < 0) || (linkStatusB > 3)) colorCode = VAL_BLACK;
  else {
    GetNumListItems(GOLSetupHandle, P_GOL_LINKSTATEB, &nlist);
    for (item = 0; item < nlist; item++) {
      GetValueFromIndex(GOLSetupHandle, P_GOL_LINKSTATEB, item, &ctrlVal);
      if (ctrlVal == linkStatusB) {
        SetCtrlIndex(GOLSetupHandle, P_GOL_LINKSTATEB, item);
        break;
      }
    }
    if (ctrlVal == GOLREADY) colorCode = VAL_GREEN;
    else if (ctrlVal == GOLOUTOFLOCK) colorCode = VAL_RED;
    else if (ctrlVal == GOLLOCKED) colorCode = VAL_YELLOW;
    else colorCode = VAL_YELLOW;
  }
  SetCtrlAttribute(GOLSetupHandle, P_GOL_LINKSTATEB, ATTR_TEXT_BGCOLOR, colorCode);
  
  if ((linkStatusC < 0) || (linkStatusC > 3)) colorCode = VAL_BLACK;
  else {
    GetNumListItems(GOLSetupHandle, P_GOL_LINKSTATEC, &nlist);
    for (item = 0; item < nlist; item++) {
      GetValueFromIndex(GOLSetupHandle, P_GOL_LINKSTATEC, item, &ctrlVal);
      if (ctrlVal == linkStatusC) {
        SetCtrlIndex(GOLSetupHandle, P_GOL_LINKSTATEC, item);
        break;
      }
    }
    if (ctrlVal == GOLREADY) colorCode = VAL_GREEN;
    else if (ctrlVal == GOLOUTOFLOCK) colorCode = VAL_RED;
    else if (ctrlVal == GOLLOCKED) colorCode = VAL_YELLOW;
    else colorCode = VAL_YELLOW;
  }
  SetCtrlAttribute(GOLSetupHandle, P_GOL_LINKSTATEC, ATTR_TEXT_BGCOLOR, colorCode);

  if ((numberOfLockLost < 0) || (numberOfLockLost > 255)) colorCode = VAL_BLACK;
  else {
    if (numberOfLockLost == 0) colorCode = VAL_GREEN;
    else colorCode = VAL_YELLOW;
    SetCtrlVal(GOLSetupHandle, P_GOL_NBLOSSOFLOCK, numberOfLockLost);
  }
  SetCtrlAttribute(GOLSetupHandle, P_GOL_NBLOSSOFLOCK, ATTR_TEXT_BGCOLOR, colorCode);

  if ((hammingChecksum < 0) || (hammingChecksum > 127)) colorCode = VAL_BLACK;
  else {
    colorCode = VAL_WHITE;
    SetCtrlVal(GOLSetupHandle, P_GOL_HAMMINGCHECKSUM, hammingChecksum);
  }
  SetCtrlAttribute(GOLSetupHandle, P_GOL_HAMMINGCHECKSUM, ATTR_TEXT_BGCOLOR, colorCode);

  if ((tx_er < 0) || (tx_er > 1)) colorCode = VAL_BLACK;
  else {
    colorCode = VAL_GREEN;
    SetCtrlVal(GOLSetupHandle, P_GOL_TXER, tx_er);
  }
  SetCtrlAttribute(GOLSetupHandle, P_GOL_TXER, ATTR_ON_COLOR, colorCode);
  SetCtrlAttribute(GOLSetupHandle, P_GOL_TXER, ATTR_OFF_COLOR, colorCode);

  if ((tx_en < 0) || (tx_en > 1)) colorCode = VAL_BLACK;
  else {
    colorCode = VAL_GREEN;
    SetCtrlVal(GOLSetupHandle, P_GOL_TXEN, tx_en);
  }
  SetCtrlAttribute(GOLSetupHandle, P_GOL_TXEN, ATTR_ON_COLOR, colorCode);
  SetCtrlAttribute(GOLSetupHandle, P_GOL_TXEN, ATTR_OFF_COLOR, colorCode);

  if ((testShift < 0) || (testShift > 1)) {
    SetCtrlAttribute(GOLSetupHandle, P_GOL_TESTSHIFT, ATTR_ON_COLOR, VAL_BLACK);
    SetCtrlAttribute(GOLSetupHandle, P_GOL_TESTSHIFT, ATTR_OFF_COLOR, VAL_BLACK);
  }
  else {
    SetCtrlAttribute(GOLSetupHandle, P_GOL_TESTSHIFT, ATTR_ON_COLOR, VAL_RED);
    SetCtrlAttribute(GOLSetupHandle, P_GOL_TESTSHIFT, ATTR_OFF_COLOR, VAL_GREEN);
    SetCtrlVal(GOLSetupHandle, P_GOL_TESTSHIFT, testShift);
  }

  if ((GOLReady < 0) || (GOLReady > 1)) {
    SetCtrlAttribute(GOLSetupHandle, P_GOL_READY, ATTR_ON_COLOR, VAL_BLACK);
    SetCtrlAttribute(GOLSetupHandle, P_GOL_READY, ATTR_OFF_COLOR, VAL_BLACK);
  }
  else {
    if (GOLReset == 0) {
      SetCtrlAttribute(GOLSetupHandle, P_GOL_READY, ATTR_ON_COLOR, VAL_GREEN);
      SetCtrlAttribute(GOLSetupHandle, P_GOL_READY, ATTR_OFF_COLOR, VAL_RED);
    }
    else {
      SetCtrlAttribute(GOLSetupHandle, P_GOL_READY, ATTR_ON_COLOR, VAL_RED);
      SetCtrlAttribute(GOLSetupHandle, P_GOL_READY, ATTR_OFF_COLOR, VAL_GREEN);
    }
    SetCtrlVal(GOLSetupHandle, P_GOL_READY, GOLReady);
  }

  if ((GOLReset < 0) || (GOLReset > 1)) {
    SetCtrlAttribute(GOLSetupHandle, P_GOL_RESET, ATTR_ON_COLOR, VAL_BLACK);
    SetCtrlAttribute(GOLSetupHandle, P_GOL_RESET, ATTR_OFF_COLOR, VAL_BLACK);
  }
  else {
    SetCtrlAttribute(GOLSetupHandle, P_GOL_RESET, ATTR_ON_COLOR, VAL_RED);
    SetCtrlAttribute(GOLSetupHandle, P_GOL_RESET, ATTR_OFF_COLOR, VAL_GREEN);
    SetCtrlVal(GOLSetupHandle, P_GOL_RESET, GOLReset);
  }
}


float GOLClockPeriod(void) {
  float clock;
  
  if (confClock == 0) {
    if (confWordMode == 0) clock = 20.0;
    else clock = 40.0;
  }
  else {
    if (confWordMode == 0) clock = 12.5;
    else clock = 25.0;
  }
  return clock;
}


void CreatPLLLockTimeList(void) {
  int nlist, item, list, ctrlVal, ctrlItem;
  float time;
  char str[16];
  
  GetCtrlIndex(GOLSetupHandle, P_GOL_PLLLOCKTIME, &ctrlItem);
  GetNumListItems(GOLSetupHandle, P_GOL_PLLLOCKTIME, &nlist);
  DeleteListItem(GOLSetupHandle, P_GOL_PLLLOCKTIME, 0, nlist);
  if ((nlist > 16) || (nlist <= 0)) nlist = 16;
  list = 0;
  for (item = nlist-1; item >= 0; item--) {
    if (item < 10) time = (1 << item) * GOLClockPeriod();
    else time = (1 << 10) * GOLClockPeriod();
    if (time < 1000.0)
      sprintf(str, "%d (%.2f ns)", item, time);
    else if (time < 1.0E6)
      sprintf(str, "%d (%.2f us)", item, time/1000.);
    else if (time < 1.0E9)
      sprintf(str, "%d (%.2f ms)", item, time/1000000.);
    if (ctrlItem == list) ctrlVal = item;
    InsertListItem(GOLSetupHandle, P_GOL_PLLLOCKTIME, list++, str, item);
  }
  SetCtrlIndex(GOLSetupHandle, P_GOL_PLLLOCKTIME, ctrlItem);
  SetCtrlVal(GOLSetupHandle, P_GOL_PLLLOCKTIME, ctrlVal);
}


void CreatWaitTimeList(void) {
  int nlist, item, list, ctrlVal, ctrlItem;
  float time;
  char str[16];
  
  GetCtrlIndex(GOLSetupHandle, P_GOL_WAITTIME, &ctrlItem);
  GetNumListItems(GOLSetupHandle, P_GOL_WAITTIME, &nlist);
  DeleteListItem(GOLSetupHandle, P_GOL_WAITTIME, 0, nlist);
  if ((nlist > 32) || (nlist <= 0)) nlist = 32;
  list = 0;
  for (item = nlist-1; item >= 0; item--) {
    if (item < 20) time = ((1 << item) + 1) * GOLClockPeriod();
    else time = ((1 << 20) + 1) * GOLClockPeriod();
    if (time < 1000.0)
      sprintf(str, "%d (%.2f ns)", item, time);
    else if (time < 1.0E6)
      sprintf(str, "%d (%.2f us)", item, time/1000.);
    else if (time < 1.0E9)
      sprintf(str, "%d (%.2f ms)", item, time/1000000.);
    if (ctrlItem == list) ctrlVal = item;
    InsertListItem(GOLSetupHandle, P_GOL_WAITTIME, list++, str, item);
  }
  SetCtrlIndex(GOLSetupHandle, P_GOL_WAITTIME, ctrlItem);
  SetCtrlVal(GOLSetupHandle, P_GOL_WAITTIME, ctrlVal);
}


void CreatLossOfLockTimeList(void) {
  int nlist, item, list, ctrlVal, ctrlItem;
  float time;
  char str[16];
  
  GetCtrlIndex(GOLSetupHandle, P_GOL_LOSSOFLOCKTIME, &ctrlItem);
  GetNumListItems(GOLSetupHandle, P_GOL_LOSSOFLOCKTIME, &nlist);
  DeleteListItem(GOLSetupHandle, P_GOL_LOSSOFLOCKTIME, 0, nlist);
  if ((nlist > 8) || (nlist <= 0)) nlist = 8;
  list = 0;
  for (item = nlist-1; item >= 0; item--) {
    if (item == 0) time = 0.0;
    else if (item < 6) time = (1 << item) * GOLClockPeriod();
    else time = -1.0;
    if (time < 0.0)
      sprintf(str, "%d (Undefined)", item);
    else if (time < 1000.0)
      sprintf(str, "%d (%.2f ns)", item, time);
    else if (time < 1.0E6)
      sprintf(str, "%d (%.2f us)", item, time/1000.);
    else if (time < 1.0E9)
      sprintf(str, "%d (%.2f ms)", item, time/1000000.);
    if (ctrlItem == list) ctrlVal = item;
    InsertListItem(GOLSetupHandle, P_GOL_LOSSOFLOCKTIME, list++, str, item);
  }
  SetCtrlIndex(GOLSetupHandle, P_GOL_LOSSOFLOCKTIME, ctrlItem);
  SetCtrlVal(GOLSetupHandle, P_GOL_LOSSOFLOCKTIME, ctrlVal);
}


void LoadGOLSetupArray(void) {
  int value;
  
  GetCtrlVal(GOLSetupHandle, P_GOL_WAITTIME, &value);
  IntToBinary(value, WAITTIMEFORREADY, 5, GOLSetupArray, GOLSETUPLENGTH);
  GetCtrlVal(GOLSetupHandle, P_GOL_LOSSOFLOCKTIME, &value);
  IntToBinary(value, LOSSOFLOCKTIME, 3, GOLSetupArray, GOLSETUPLENGTH);
  GetCtrlVal(GOLSetupHandle, P_GOL_PLLLOCKTIME, &value);
  IntToBinary(value, PLLLOCKTIME, 4, GOLSetupArray, GOLSETUPLENGTH);
  GetCtrlVal(GOLSetupHandle, P_GOL_LOCKLOSSCONTROL, &GOLSetupArray[LOSSOFLOCKCONTROL]); 
  GetCtrlVal(GOLSetupHandle, P_GOL_LOCKLOSSCOUNTERCTRL, &GOLSetupArray[LOSSOFLOCKCNTCONTROL]); 
  GetCtrlVal(GOLSetupHandle, P_GOL_FORCELOCK, &GOLSetupArray[LOCKCONTROL]); 
  GetCtrlVal(GOLSetupHandle, P_GOL_SELFTEST, &GOLSetupArray[DATATRANSCONTROL]); 
  GetCtrlVal(GOLSetupHandle, P_GOL_PLLCURRENT, &value);
  IntToBinary(value, CHARGEPUMPCURRENT, 5, GOLSetupArray, GOLSETUPLENGTH);
  GetCtrlVal(GOLSetupHandle, P_GOL_SELECTTESTSIGNALS, &value);
  IntToBinary(value, TESTSIGNALS, 2, GOLSetupArray, GOLSETUPLENGTH);
  GetCtrlVal(GOLSetupHandle, P_GOL_FLAGBITSINGLINK, &GOLSetupArray[FLAGBITSCONTROL]); 
  GetCtrlVal(GOLSetupHandle, P_GOL_DRIVERSTRENGTH, &value);
  IntToBinary(value, DRIVERCURRSTRENGTH, 7, GOLSetupArray, GOLSETUPLENGTH);
  GetCtrlVal(GOLSetupHandle, P_GOL_CURRENTCONTROL, &GOLSetupArray[CURRENTCONTROL]); 
}
