// ttcrxSetup.c                         by T.S.Dai
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

extern char currSetupFile[30];
extern int Menu00Handle, JTAGControlHandle;
extern int PanelIndexInFile(int panelID, int setupType, int mezzNumber, int ASDNumber);
extern void UpCSMSetupControl(void);
extern void UpGOLSetupControl(void);
extern void GetTTCrxStatus(void);
extern void DownloadTTCrxSetup(void);
extern void JTAGChainCSM(void);
extern void JTAGChainTTCrx(void);

// Bring up TTCrx Setup Control Panel
void UpTTCrxSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    PanelRecall(TTCrxSetupHandle);
    PutClock1DelayValue();
    PutClock2DelayValue();
    PutTestOutputSelection();
    PutFrequencyCheckPeriod();
    PanelSave(TTCrxSetupHandle);
    DisplayPanel(TTCrxSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup TTCrx",
                 "Bring up TTCrx Setup Control Panel.\n"
                 "Note: For TTCrx settings, consult the TTCrx manual.");
  }
  controlOption = NORM;
}


// Load TTCrx Setup Control Panel and define user buttons
int TTCrxSetupPanelAndButton(void) {
  // Define the TTCrx Setup Control panel handles
  if ((TTCrxSetupHandle = LoadPanel(0, "DAQGUI.uir", P_TTCRX)) < 0) return -1;
  SetPanelAttribute(TTCrxSetupHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);

  // TTCrx Setup Control Panel Buttons
  SetupUserButton(Menu00Handle, MENU00_JTAG_SETUPTTCRX, UpTTCrxSetupControl);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_DONE, TTCrxSetupDone);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_DEFAULT, PanelDefault);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_CANCEL, PanelCancel);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_SAVESETUP, SaveTTCrxSetup);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_CSMCONTROL, UpCSMSetupControl);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_GOLCONTROL, UpGOLSetupControl);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_CLOCK1FINEDELAY, PutClock1DelayValue);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_CLOCK1COARSEDELAY, PutClock1DelayValue);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_CLOCK2FINEDELAY, PutClock2DelayValue);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_CLOCK2COARSEDELAY, PutClock2DelayValue);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_TESTOUTPUTS, PutTestOutputSelection);
  SetupUserButton(TTCrxSetupHandle, P_TTCRX_FREQCHECKPERIOD, PutFrequencyCheckPeriod);
  
  return 0;
}


// TTCrx Setup Done
void TTCrxSetupDone(void) {
  PanelSave(TTCrxSetupHandle);
  LoadTTCrxSetupArray();
  HidePanel(TTCrxSetupHandle);
  DownloadTTCrxSetup();
  // Update JTAG control panel
  JTAGChainCSM();
  JTAGChainTTCrx();
  DisplayPanel(JTAGControlHandle);
  SetActivePanel(JTAGControlHandle);
}


void SaveTTCrxSetup(void) {
  int val;
  char setupFilename[281] = "TTCrxSetup.txt";
  FILE *setupFile;
  
  PromptPopup("Get TTCrx Setup File Name",
              "Please give the file name (Default: TTCrxSetup.txt) for saving TTCrx setup",
              setupFilename, 280);
  if (strlen(setupFilename) <= 5 ) strcpy(setupFilename, "TTCrxSetup.txt");
  if (setupFile = fopen(setupFilename, "w")) {
    LoadTTCrxSetupArray();
    fprintf(setupFile, "--------- TTCrx Setup File -------------\n");
    fprintf(setupFile, "Position  Settings  Description...\n");
    BinaryToInt(&val, CLOCK1FINEDELAY, 8, TTCrxSetupArray);
    fprintf(setupFile, "%8d  %8d  ", CLOCK1FINEDELAY, val);
    fprintf(setupFile, "LHC deskewed clock 1 fine delay %.3f ns\n", TTCrxClockFineDelay(val));
    BinaryToInt(&val, CLOCK2FINEDELAY, 8, TTCrxSetupArray);
    fprintf(setupFile, "%8d  %8d  ", CLOCK2FINEDELAY, val);
    fprintf(setupFile, "LHC deskewed clock 2 fine delay %.3f ns\n", TTCrxClockFineDelay(val));
    BinaryToInt(&val, CLOCK1COARSEDELAY, 4, TTCrxSetupArray);
    fprintf(setupFile, "%8d  %8d  ", CLOCK1COARSEDELAY, val);
    fprintf(setupFile, "LHC deskewed clock 1 coarse delay %.3f ns\n", TTCrxClockCoarseDelay(val));
    BinaryToInt(&val, CLOCK2COARSEDELAY, 4, TTCrxSetupArray);
    fprintf(setupFile, "%8d  %8d  ", CLOCK2COARSEDELAY, val);
    fprintf(setupFile, "LHC deskewed clock 2 coarse delay %.3f ns\n", TTCrxClockCoarseDelay(val));

    fprintf(setupFile, "%8d  %8d  ", BUNCHCOUNTERCONTROL, TTCrxSetupArray[BUNCHCOUNTERCONTROL]);
    if (TTCrxSetupArray[BUNCHCOUNTERCONTROL] == 1)
      fprintf(setupFile, "Bunch Counter Operation enabled\n");
    else
      fprintf(setupFile, "Bunch Counter Operation disabled\n");

    fprintf(setupFile, "%8d  %8d  ", EVENTCOUNTERCONTROL, TTCrxSetupArray[EVENTCOUNTERCONTROL]);
    if (TTCrxSetupArray[EVENTCOUNTERCONTROL] == 1)
      fprintf(setupFile, "Event Counter Operation enabled\n");
    else
      fprintf(setupFile, "Event Counter Operation disabled\n");

    fprintf(setupFile, "%8d  %8d  ", SELECTSYNCHCLOCK, TTCrxSetupArray[SELECTSYNCHCLOCK]);
    if (TTCrxSetupArray[SELECTSYNCHCLOCK] == 1)
      fprintf(setupFile, "Select deskewed LHC clock 2 for synchronization of BrcstStr2 and Brcst[7:6]\n");
    else
      fprintf(setupFile, "Select deskewed LHC clock 1 for synchronization of BrcstStr2 and Brcst[7:6]\n");

    fprintf(setupFile, "%8d  %8d  ", CLOCK2OUTPUT, TTCrxSetupArray[CLOCK2OUTPUT]);
    if (TTCrxSetupArray[CLOCK2OUTPUT] == 1)
      fprintf(setupFile, "Deskewed LHC clock 2 output enabled\n");
    else
      fprintf(setupFile, "Deskewed LHC clock 2 output disabled\n");

    fprintf(setupFile, "%8d  %8d  ", CLOCKL1ACCEPTOUTPUT, TTCrxSetupArray[CLOCKL1ACCEPTOUTPUT]);
    if (TTCrxSetupArray[CLOCKL1ACCEPTOUTPUT] == 1)
      fprintf(setupFile, "L1 accept clock output enabled\n");
    else
      fprintf(setupFile, "L1 accept clock output disabled\n");

    fprintf(setupFile, "%8d  %8d  ", PARALLELOUTPUTBUS, TTCrxSetupArray[PARALLELOUTPUTBUS]);
    if (TTCrxSetupArray[PARALLELOUTPUTBUS] == 1)
      fprintf(setupFile, "Parallel output bus (Dout[7:0], DQ[3:0], SubAddr[7:0] and DoutStr) enabled\n");
    else
      fprintf(setupFile, "Parallel output bus (Dout[7:0], DQ[3:0], SubAddr[7:0] and DoutStr) disabled\n");

    fprintf(setupFile, "%8d  %8d  ", SERIALBOUTPUT, TTCrxSetupArray[SERIALBOUTPUT]);
    if (TTCrxSetupArray[SERIALBOUTPUT] == 1)
      fprintf(setupFile, "Serial B output enabled\n");
    else
      fprintf(setupFile, "Serial B output disabled\n");

    fprintf(setupFile, "%8d  %8d  ", CLOCKOUTPUT, TTCrxSetupArray[CLOCKOUTPUT]);
    if (TTCrxSetupArray[CLOCKOUTPUT] == 1)
      fprintf(setupFile, "Non-deskewed LHC clock output enabled\n");
    else
      fprintf(setupFile, "Non-deskewed LHC clock output disabled\n");

    BinaryToInt(&val, TTCRXADDRESS, 14, TTCrxSetupArray);
    fprintf(setupFile, "%8d  %8d  ", TTCRXADDRESS, val);
    fprintf(setupFile, "TTCrx Address\n");

    fprintf(setupFile, "%8d  %8d  ", SPCONVERTERA, TTCrxSetupArray[SPCONVERTERA]);
    if (TTCrxSetupArray[SPCONVERTERA] == 1)
      fprintf(setupFile, "Master Mode A: Serial to parallel converter disabled\n");
    else
      fprintf(setupFile, "Master Mode A: Serial to parallel converter enabled\n");

    fprintf(setupFile, "%8d  %8d  ", TESTMODEA, TTCrxSetupArray[TESTMODEA]);
    if (TTCrxSetupArray[TESTMODEA] == 1)
      fprintf(setupFile, "Master Mode A: Test mode enabled\n");
    else
      fprintf(setupFile, "Master Mode A: Test mode disabled\n");

    BinaryToInt(&val, I2CBASEADDRESS, 6, TTCrxSetupArray);
    fprintf(setupFile, "%8d  %8d  ", I2CBASEADDRESS, val);
    fprintf(setupFile, "I2C base address\n");

    fprintf(setupFile, "%8d  %8d  ", SPCONVERTERB, TTCrxSetupArray[SPCONVERTERB]);
    if (TTCrxSetupArray[SPCONVERTERB] == 1)
      fprintf(setupFile, "Master Mode B: Serial to parallel converter disabled\n");
    else
      fprintf(setupFile, "Master Mode B: Serial to parallel converter enabled\n");

    fprintf(setupFile, "%8d  %8d  ", TESTMODEB, TTCrxSetupArray[TESTMODEB]);
    if (TTCrxSetupArray[TESTMODEB] == 1)
      fprintf(setupFile, "Master Mode B: Test mode enabled\n");
    else
      fprintf(setupFile, "Master Mode B: Test mode disabled\n");

    BinaryToInt(&val, DLLCURRENT, 3, TTCrxSetupArray);
    fprintf(setupFile, "%8d  %8d  ", DLLCURRENT, val);
    fprintf(setupFile, "DLL current: The charge-pumps current for the delay-locked-loop\n");

    BinaryToInt(&val, PLLCURRENT, 3, TTCrxSetupArray);
    fprintf(setupFile, "%8d  %8d  ", PLLCURRENT, val);
    fprintf(setupFile, "PLL current: The charge-pumps current for the phase-locked-loop\n");

    fprintf(setupFile, "%8d  %8d  ", PSHF1TESTINPUT, TTCrxSetupArray[PSHF1TESTINPUT]);
    if (TTCrxSetupArray[PSHF1TESTINPUT] == 1)
      fprintf(setupFile, "Test input for phase shifter 1 seleted\n");
    else
      fprintf(setupFile, "Input for phase shifter 1 seleted for normal operation\n");

    fprintf(setupFile, "%8d  %8d  ", PSHF2TESTINPUT, TTCrxSetupArray[PSHF2TESTINPUT]);
    if (TTCrxSetupArray[PSHF2TESTINPUT] == 1)
      fprintf(setupFile, "Test input for phase shifter 2 seleted\n");
    else
      fprintf(setupFile, "Input for phase shifter 2 seleted for normal operation\n");

    BinaryToInt(&val, TESTOUTPUTS, 3, TTCrxSetupArray);
    fprintf(setupFile, "%8d  %8d  ", TESTOUTPUTS, val);
    fprintf(setupFile, "Test outputs selection\n");

    fprintf(setupFile, "%8d  %8d  ", PLLPHASEDETECTOR, TTCrxSetupArray[PLLPHASEDETECTOR]);
    if (TTCrxSetupArray[PLLPHASEDETECTOR] == 1)
      fprintf(setupFile, "External test signal selected for enabling the PLL phase detector\n");
    else
      fprintf(setupFile, "Normal operation signal selected for enabling the PLL phase detector\n");

    fprintf(setupFile, "%8d  %8d  ", SELECTINPUTS, TTCrxSetupArray[SELECTINPUTS]);
    if (TTCrxSetupArray[SELECTINPUTS] == 1)
      fprintf(setupFile, "Input from optical link seleted\n");
    else
      fprintf(setupFile, "Input from test_in[3:4] seleted\n");

    fprintf(setupFile, "%8d  %8d  ", ASSERTPLLTESTRESET, TTCrxSetupArray[ASSERTPLLTESTRESET]);
    if (TTCrxSetupArray[ASSERTPLLTESTRESET] == 1)
      fprintf(setupFile, "Assert PLL test reset line\n");
    else
      fprintf(setupFile, "Do nothing on PLL test reset line\n");

    fprintf(setupFile, "%8d  %8d  ", ASSERTDLLTESTRESET, TTCrxSetupArray[ASSERTDLLTESTRESET]);
    if (TTCrxSetupArray[ASSERTDLLTESTRESET] == 1)
      fprintf(setupFile, "Assert DLL test reset line\n");
    else
      fprintf(setupFile, "Do nothing on DLL test reset line\n");

    fprintf(setupFile, "%8d  %8d  ", HAMMINGCHECKA, TTCrxSetupArray[HAMMINGCHECKA]);
    if (TTCrxSetupArray[HAMMINGCHECKA] == 1)
      fprintf(setupFile, "A: The internal hamming check-machine enabled\n");
    else
      fprintf(setupFile, "A: The internal hamming check-machine disabled\n");

    BinaryToInt(&val, FREQCHECKPERIOD, 3, TTCrxSetupArray);
    fprintf(setupFile, "%8d  %8d  ", FREQCHECKPERIOD, val);
    fprintf(setupFile, "Stop frequency detection phase after %d Cycles if no `frequ_low` detected\n", 1 << (4+val));

    fprintf(setupFile, "%8d  %8d  ", AUTOFREQINCREASE, TTCrxSetupArray[AUTOFREQINCREASE]);
    if (TTCrxSetupArray[AUTOFREQINCREASE] == 1)
      fprintf(setupFile, "Automatic frequency increase aftger PLL reset disabled\n");
    else
      fprintf(setupFile, "Automatic frequency increase aftger PLL reset enabled\n");

    fprintf(setupFile, "%8d  %8d  ", WATCHDOGCIRCUIT, TTCrxSetupArray[WATCHDOGCIRCUIT]);
    if (TTCrxSetupArray[WATCHDOGCIRCUIT] == 1)
      fprintf(setupFile, "Watchdog circuit disabled\n");
    else
      fprintf(setupFile, "Watchdog circuit enabled\n");

    fprintf(setupFile, "%8d  %8d  ", HAMMINGERRORDETCORR, TTCrxSetupArray[HAMMINGERRORDETCORR]);
    if (TTCrxSetupArray[HAMMINGERRORDETCORR] == 1)
      fprintf(setupFile, "Hamming error detection and correction on incoming data stream enabled\n");
    else
      fprintf(setupFile, "Hamming error detection and correction on incoming data stream disabled\n");

    fprintf(setupFile, "%8d  %8d  ", TESTINOUTPUTS, TTCrxSetupArray[TESTINOUTPUTS]);
    if (TTCrxSetupArray[TESTINOUTPUTS] == 1)
      fprintf(setupFile, "Test inputs and outputs enabled\n");
    else
      fprintf(setupFile, "Test inputs and outputs disabled\n");

    fprintf(setupFile, "%8d  %8d  ", HAMMINGCHECKB, TTCrxSetupArray[HAMMINGCHECKB]);
    if (TTCrxSetupArray[HAMMINGCHECKB] == 1)
      fprintf(setupFile, "B: The internal hamming check-machine enabled\n");
    else
      fprintf(setupFile, "B: The internal hamming check-machine disabled\n");

    fclose(setupFile);
    printf("TTCrx setup is saved into file <%s>.\n", setupFilename);
  }
  else {
    printf("Unable to open TTCrx setup file <%s>!\n", setupFilename);
    MessagePopup("Failed to Open TTCrx Setup File",
                 "Unable to open TTCrx setup file, please check your disk and filename!");
  }
}


void PutClock1DelayValue(void) {
  int fine, coarse;
  char str[20];

  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCK1FINEDELAY, &fine);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCK1COARSEDELAY, &coarse);
  sprintf(str,"= %.3fns", TTCrxClockFineDelay(fine));
  ReplaceTextBoxLine(TTCrxSetupHandle, P_TTCRX_CLOCK1FINETEXT, 0, str);
  sprintf(str,"= %.2fns", TTCrxClockCoarseDelay(coarse));
  ReplaceTextBoxLine(TTCrxSetupHandle, P_TTCRX_CLOCK1COARSETEXT, 0, str);
  sprintf(str,"= %.3fns", TTCrxClockCoarseDelay(coarse)+TTCrxClockFineDelay(fine));
  ReplaceTextBoxLine(TTCrxSetupHandle, P_TTCRX_CLOCK1TOTALTEXT, 0, str);
}


void PutClock2DelayValue(void) {
  int fine, coarse;
  char str[20];

  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCK2FINEDELAY, &fine);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCK2COARSEDELAY, &coarse);
  sprintf(str,"= %.3fns", TTCrxClockFineDelay(fine));
  ReplaceTextBoxLine(TTCrxSetupHandle, P_TTCRX_CLOCK2FINETEXT, 0, str);
  sprintf(str,"= %.2fns", TTCrxClockCoarseDelay(coarse));
  ReplaceTextBoxLine(TTCrxSetupHandle, P_TTCRX_CLOCK2COARSETEXT, 0, str);
  sprintf(str,"= %.3fns", TTCrxClockCoarseDelay(coarse)+TTCrxClockFineDelay(fine));
  ReplaceTextBoxLine(TTCrxSetupHandle, P_TTCRX_CLOCK2TOTALTEXT, 0, str);
}


void PutTestOutputSelection(void) {
  int val, color;

  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_TESTOUTPUTS, &val);
  if (val == 5) color = VAL_WHITE;
  else color = VAL_RED;
  SetCtrlAttribute(TTCrxSetupHandle, P_TTCRX_TESTOUTPUTS, ATTR_TEXT_BGCOLOR, color);
}


void PutFrequencyCheckPeriod(void) {
  char str[256];
  int val, color;
  
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_FREQCHECKPERIOD, &val);
  sprintf(str, "Stop frequency detection phase after %d Cycles if no `frequ_low` detected", 1 << (4+val));
  ReplaceTextBoxLine(TTCrxSetupHandle, P_TTCRX_CYCLETEXT, 0, str);
  if (val == 7) color = VAL_WHITE;
  else color = VAL_RED;
  SetCtrlAttribute(TTCrxSetupHandle, P_TTCRX_FREQCHECKPERIOD, ATTR_TEXT_BGCOLOR, color);
  SetCtrlAttribute(TTCrxSetupHandle, P_TTCRX_CYCLETEXT, ATTR_TEXT_BGCOLOR, color);
}


void LoadTTCrxSetupArray(void) {
  int value;
  
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCK1FINEDELAY, &value);
  IntToBinary(value, CLOCK1FINEDELAY, 8, TTCrxSetupArray, TTCRXSETUPLENGTH);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCK2FINEDELAY, &value);
  IntToBinary(value, CLOCK2FINEDELAY, 8, TTCrxSetupArray, TTCRXSETUPLENGTH);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCK1COARSEDELAY, &value);
  IntToBinary(value, CLOCK1COARSEDELAY, 4, TTCrxSetupArray, TTCRXSETUPLENGTH);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCK2COARSEDELAY, &value);
  IntToBinary(value, CLOCK2COARSEDELAY, 4, TTCrxSetupArray, TTCRXSETUPLENGTH);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_BUNCHCOUNTER, &TTCrxSetupArray[BUNCHCOUNTERCONTROL]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_EVENTCOUNTER, &TTCrxSetupArray[EVENTCOUNTERCONTROL]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_SELECTCLOCK2, &TTCrxSetupArray[SELECTSYNCHCLOCK]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCK2OUTPUT, &TTCrxSetupArray[CLOCK2OUTPUT]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCKL1ACCEPTOUTPUT, &TTCrxSetupArray[CLOCKL1ACCEPTOUTPUT]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_PARALLELOUTPUTBUS, &TTCrxSetupArray[PARALLELOUTPUTBUS]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_SERIALBOUTPUT, &TTCrxSetupArray[SERIALBOUTPUT]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_CLOCKOUTPUT, &TTCrxSetupArray[CLOCKOUTPUT]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_TTCRXADDRESS, &value);
  IntToBinary(value, TTCRXADDRESS, 14, TTCrxSetupArray, TTCRXSETUPLENGTH);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_SPCONVERTERA, &TTCrxSetupArray[SPCONVERTERA]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_TESTMODEA, &TTCrxSetupArray[TESTMODEA]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_I2CBASEADDRESS, &value);
  IntToBinary(value, I2CBASEADDRESS, 6, TTCrxSetupArray, TTCRXSETUPLENGTH);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_SPCONVERTERB, &TTCrxSetupArray[SPCONVERTERB]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_TESTMODEB, &TTCrxSetupArray[TESTMODEB]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_DLLCURRENT, &value);
  IntToBinary(value, DLLCURRENT, 3, TTCrxSetupArray, TTCRXSETUPLENGTH);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_PLLCURRENT, &value);
  IntToBinary(value, PLLCURRENT, 3, TTCrxSetupArray, TTCRXSETUPLENGTH);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_TESTINPUTPSHFT1, &TTCrxSetupArray[PSHF1TESTINPUT]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_TESTINPUTPSHFT2, &TTCrxSetupArray[PSHF2TESTINPUT]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_TESTOUTPUTS, &value);
  IntToBinary(value, TESTOUTPUTS, 3, TTCrxSetupArray, TTCRXSETUPLENGTH);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_EXTSIGNALFORPLL, &TTCrxSetupArray[PLLPHASEDETECTOR]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_INPUT, &TTCrxSetupArray[SELECTINPUTS]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_PLLTESTRESETLINE, &TTCrxSetupArray[ASSERTPLLTESTRESET]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_DLLTESTRESETLINE, &TTCrxSetupArray[ASSERTDLLTESTRESET]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_HAMMINGCHECKA, &TTCrxSetupArray[HAMMINGCHECKA]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_FREQCHECKPERIOD, &value);
  IntToBinary(value, FREQCHECKPERIOD, 3, TTCrxSetupArray, TTCRXSETUPLENGTH);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_AUTOFREQINCREASE, &TTCrxSetupArray[AUTOFREQINCREASE]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_WATCHDOGCIRCUIT, &TTCrxSetupArray[WATCHDOGCIRCUIT]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_HAMMINGDATAERROR, &TTCrxSetupArray[HAMMINGERRORDETCORR]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_TESTINOUTPUTS, &TTCrxSetupArray[TESTINOUTPUTS]);
  GetCtrlVal(TTCrxSetupHandle, P_TTCRX_HAMMINGCHECKB, &TTCrxSetupArray[HAMMINGCHECKB]);
}


float TTCrxClockFineDelay(int fine) {
  float fineDelay, step = 0.10417;
  int n, m, k;

  n = (fine/16) % 15;
  m = fine % 16;
  k = (16*n + 15*m + 30) % 240;
  fineDelay = ((float) k) * step;
  return fineDelay;
}


float TTCrxClockCoarseDelay(int coarse) {
  float coarseDelay, step = 24.95;
  
  coarseDelay = ((float) coarse) * step;
  return coarseDelay;
}

