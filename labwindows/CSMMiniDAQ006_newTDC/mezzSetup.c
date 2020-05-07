// mezzSetup.c                         by T.S.Dai
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
#include "TCPServer.h"
#include "mezzSetup.h"

char ASDTmpSetupFile[30]  = "ASDTmpSetup.sav";

extern char currSetupFile[30];
extern int Menu00Handle, JTAGControlHandle, CSMSetupHandle, TTCrxSetupHandle, GOLSetupHandle, action;
extern int PanelIndexInFile(int panelID, int setupType, int mezzNumber, int ASDNumber);
extern void GetAllAMTID(void);
extern void DownloadMezzCardSetup(void);
extern void UpdateJTAGControlPanel(void);
extern int MezzCardSetupStatus(int mezzCardNumber);
extern void UpCSMSetupControl(void);
extern void CSMSetupDone(void);
extern void TTCrxSetupDone(void);
extern void GOLSetupDone(void);

void MezzCardParameterInit(void) {
  int i;
  
  for (i = 0; i < MAXNUMBERMEZZANINE; i++) mezzInitStatus[i] = MEZZNOINIT;
  downloadMezzSetupDone = FALSE;
  downloadMezzSetup = TRUE;
  // Get Mezzanine Enable/Disble Button IDs
  mezzButtonID[0] = P_INDMEZZS_MEZZ00;
  mezzButtonID[1] = P_INDMEZZS_MEZZ01;
  mezzButtonID[2] = P_INDMEZZS_MEZZ02;
  mezzButtonID[3] = P_INDMEZZS_MEZZ03;
  mezzButtonID[4] = P_INDMEZZS_MEZZ04;
  mezzButtonID[5] = P_INDMEZZS_MEZZ05;
  mezzButtonID[6] = P_INDMEZZS_MEZZ06;
  mezzButtonID[7] = P_INDMEZZS_MEZZ07;
  mezzButtonID[8] = P_INDMEZZS_MEZZ08;
  mezzButtonID[9] = P_INDMEZZS_MEZZ09;
  mezzButtonID[10] = P_INDMEZZS_MEZZ10;
  mezzButtonID[11] = P_INDMEZZS_MEZZ11;
  mezzButtonID[12] = P_INDMEZZS_MEZZ12;
  mezzButtonID[13] = P_INDMEZZS_MEZZ13;
  mezzButtonID[14] = P_INDMEZZS_MEZZ14;
  mezzButtonID[15] = P_INDMEZZS_MEZZ15;
  mezzButtonID[16] = P_INDMEZZS_MEZZ16;
  mezzButtonID[17] = P_INDMEZZS_MEZZ17;
  
    // Get Mezzanine Card Menu Buttons 
  mezzButtonForMenu[0] = MENU00_JTAG_MEZZSETUPIND_M00;
  mezzButtonForMenu[1] = MENU00_JTAG_MEZZSETUPIND_M01;
  mezzButtonForMenu[2] = MENU00_JTAG_MEZZSETUPIND_M02;
  mezzButtonForMenu[3] = MENU00_JTAG_MEZZSETUPIND_M03;
  mezzButtonForMenu[4] = MENU00_JTAG_MEZZSETUPIND_M04;
  mezzButtonForMenu[5] = MENU00_JTAG_MEZZSETUPIND_M05;
  mezzButtonForMenu[6] = MENU00_JTAG_MEZZSETUPIND_M06;
  mezzButtonForMenu[7] = MENU00_JTAG_MEZZSETUPIND_M07;
  mezzButtonForMenu[8] = MENU00_JTAG_MEZZSETUPIND_M08;
  mezzButtonForMenu[9] = MENU00_JTAG_MEZZSETUPIND_M09;
  mezzButtonForMenu[10] = MENU00_JTAG_MEZZSETUPIND_M10;
  mezzButtonForMenu[11] = MENU00_JTAG_MEZZSETUPIND_M11;
  mezzButtonForMenu[12] = MENU00_JTAG_MEZZSETUPIND_M12;
  mezzButtonForMenu[13] = MENU00_JTAG_MEZZSETUPIND_M13;
  mezzButtonForMenu[14] = MENU00_JTAG_MEZZSETUPIND_M14;
  mezzButtonForMenu[15] = MENU00_JTAG_MEZZSETUPIND_M15;
  mezzButtonForMenu[16] = MENU00_JTAG_MEZZSETUPIND_M16;
  mezzButtonForMenu[17] = MENU00_JTAG_MEZZSETUPIND_M17;
  
  // Get Buttons for copying mezzanine card setup
  mezzButtonForCopy[0] = P_COPYMEZZ_MEZZ00;
  mezzButtonForCopy[1] = P_COPYMEZZ_MEZZ01;
  mezzButtonForCopy[2] = P_COPYMEZZ_MEZZ02;
  mezzButtonForCopy[3] = P_COPYMEZZ_MEZZ03;
  mezzButtonForCopy[4] = P_COPYMEZZ_MEZZ04;
  mezzButtonForCopy[5] = P_COPYMEZZ_MEZZ05;
  mezzButtonForCopy[6] = P_COPYMEZZ_MEZZ06;
  mezzButtonForCopy[7] = P_COPYMEZZ_MEZZ07;
  mezzButtonForCopy[8] = P_COPYMEZZ_MEZZ08;
  mezzButtonForCopy[9] = P_COPYMEZZ_MEZZ09;
  mezzButtonForCopy[10] = P_COPYMEZZ_MEZZ10;
  mezzButtonForCopy[11] = P_COPYMEZZ_MEZZ11;
  mezzButtonForCopy[12] = P_COPYMEZZ_MEZZ12;
  mezzButtonForCopy[13] = P_COPYMEZZ_MEZZ13;
  mezzButtonForCopy[14] = P_COPYMEZZ_MEZZ14;
  mezzButtonForCopy[15] = P_COPYMEZZ_MEZZ15;
  mezzButtonForCopy[16] = P_COPYMEZZ_MEZZ16;
  mezzButtonForCopy[17] = P_COPYMEZZ_MEZZ17;
  mezzButtonForCopy[MAXNUMBERMEZZANINE] = P_COPYMEZZ_ALLSETUP;
}


void CheckNumberMezzCard(int panel) {
  int mezz, resp, gotIt, dimmed, savedMeasureTriggerRate;

  mezzEnables = ReadParameters(PARAMEZZENABLES);
  nbMezzCard = 0;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) if (((mezzEnables>>mezz)&1) == 1) nbMezzCard++;
  if (nbMezzCard == 0) {
    printf("No mezzanine card is enabled for the JTAG. Need to setup CSM first!\n");
    printf("If No CONFIRMATION POPUP window, click on PC window then click on JTAG Control Window.\n");
    resp = ConfirmPopup("Need Setup CSM First",
                        "No mezzanine card is enabled for the JTAG\n"
                        "Need to setup CSM first, do you wish to setup CSM?");
    if (resp == 1) {
      UpCSMSetupControl();
      savedMeasureTriggerRate = measureTriggerRate;
      measureTriggerRate = FALSE;
      gotIt = 0;
      while (gotIt == 0) {
        gotIt = HandleUserButtons4(CSMSetupDone, TTCrxSetupDone, GOLSetupDone, PanelCancel);
#ifdef TTCVIDRIVERVIATCP
        dimmed = FALSE;
        if (measureTriggerRate) {
          savedMeasureTriggerRate = measureTriggerRate;
          measureTriggerRate = FALSE;
        }
        else if (TTCviTCPConnected && (!gotTTCvi)) dimmed = TRUE;
        SetCtrlAttribute(CSMSetupHandle, P_CSM_CANCEL, ATTR_DIMMED, dimmed);
        SetCtrlAttribute(CSMSetupHandle, P_CSM_DONE, ATTR_DIMMED, dimmed);
        SetCtrlAttribute(TTCrxSetupHandle, P_TTCRX_CANCEL, ATTR_DIMMED, dimmed);
        SetCtrlAttribute(TTCrxSetupHandle, P_TTCRX_DONE, ATTR_DIMMED, dimmed);
        SetCtrlAttribute(GOLSetupHandle, P_GOL_CANCEL, ATTR_DIMMED, dimmed);
        SetCtrlAttribute(GOLSetupHandle, P_GOL_DONE, ATTR_DIMMED, dimmed);
        if (dimmed) HandleUserButtonAndFunction(PanelCancel);
#endif
      }
      printf("Out it\n");
      measureTriggerRate = savedMeasureTriggerRate;
    }
    else if (panel > 0) DisplayPanel(panel);
  }
  else if (mezzCardSetupAll) SetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, nbMezzCard);
  if (nbMezzCard > 1) 
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_COPYSETUP, ATTR_DIMMED, FALSE);
  else
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_COPYSETUP, ATTR_DIMMED, TRUE);
}


// Bring up Common AMT Setup Control Panel
void UpAMTSetupAllControl(void) {
  static int panel, warningOn = TRUE;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = TRUE;
    RecallMezzanineSetup();
    CheckNumberMezzCard(panel);
    if (warningOn == 1 && nbMezzCard > 1)
      warningOn = ConfirmPopup("Warning on Mezz Card Setup All",
                  "WARNING! Mezzanine Card Setup All will override individual Mezzanine Card settings\n"
                  "Do you wish to keep this warning message in the future?");
    if (nbMezzCard > 0) DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card (Common Setting)",
                 "Bring up Mezzanine Card Common Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


// Bring up Individual AMT Setup Control Panel from mezzanine 0 to mezzanine 17
void UpMezz00AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 0;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 0",
                 "Bring up Mezzanine Card 0 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz01AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 1;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 1",
                 "Bring up Mezzanine Card 1 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz02AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 2;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 2",
                 "Bring up Mezzanine Card 2 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz03AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 3;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 3",
                 "Bring up Mezzanine Card 3 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz04AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 4;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 4",
                 "Bring up Mezzanine Card 4 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz05AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 5;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 5",
                 "Bring up Mezzanine Card 5 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz06AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 6;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 6",
                 "Bring up Mezzanine Card 6 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz07AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 7;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 7",
                 "Bring up Mezzanine Card 7 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz08AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 8;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 8",
                 "Bring up Mezzanine Card 8 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz09AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 9;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 9",
                 "Bring up Mezzanine Card 9 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz10AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 10;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 10",
                 "Bring up Mezzanine Card 10 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz11AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 11;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 11",
                 "Bring up Mezzanine Card 11 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz12AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 12;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 12",
                 "Bring up Mezzanine Card 12 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz13AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 13;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 13",
                 "Bring up Mezzanine Card 13 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz14AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 14;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 14",
                 "Bring up Mezzanine Card 14 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz15AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 15;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 15",
                 "Bring up Mezzanine Card 15 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz16AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 16;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 16",
                 "Bring up Mezzanine Card 16 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void UpMezz17AMTSetupControl(void) {
  int panel;
  
  if (controlOption == NORM) {
    panel = GetActivePanel();
    HidePanel(panel);
    mezzCardSetupAll = FALSE;
    mezzCardNb = 17;
    RecallMezzanineSetup();
    DisplayPanel(AMTSetupHandle);
  }
  else if (controlOption == HELP) {
    MessagePopup("Help on Setup Mezzanine Card 17",
                 "Bring up Mezzanine Card 17 Setup Control Panel.\n"
                 "Note: For AMT and ASD settings, consult the AMT and ASD manuals.");
  }
  controlOption = NORM;
}


void DisableAllIndividualMezzSettings(void) {
  int mezz;

  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezz], ATTR_CHECKED, 0);
    SetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezz], FALSE);
  }
}


// Load Mezzanine Card Setup Control Panel and define user buttons
int MezzCardSetupPanelAndButton(void) {
  // Define Individual Mezzanine Card Enable/Disable Control panel handles
  if ((IndMezzEnableHandle = LoadPanel(0, "DAQGUI.uir", P_INDMEZZS)) < 0) return -1;
  SetPanelAttribute(IndMezzEnableHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  // Define the AMT Setup Control panel handles
  if ((AMTSetupHandle = LoadPanel(0, "DAQGUI.uir", P_AMTSETUP)) < 0) return -1;
  SetPanelAttribute(AMTSetupHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  // Define the AMT Error Control panel handles
  if ((AMTErrorHandle = LoadPanel(0, "DAQGUI.uir", P_AMTERROR)) < 0) return -1;
  SetPanelAttribute(AMTErrorHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  // Define the ASD Setup Control panel handles
  if ((ASDSetupHandle = LoadPanel(0, "DAQGUI.uir", P_ASDSETUP)) < 0) return -1;
  SetPanelAttribute(ASDSetupHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  // Define the ASD Setup Control panel handles
  if ((CopyMezzSetupHandle = LoadPanel(0, "DAQGUI.uir", P_COPYMEZZ)) < 0) return -1;
  SetPanelAttribute(CopyMezzSetupHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);

  // AMT Setup Control Panel Buttons
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPALL, UpAMTSetupAllControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_NOMINALTHRESHOLD, UpSerialNumberAndLoadNominalASDMainThreshold);
  SetupUserButton(Menu00Handle, MENU00_JTAG_GETALLAMTID, GetAllAMTID);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M00, UpMezz00AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M01, UpMezz01AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M02, UpMezz02AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M03, UpMezz03AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M04, UpMezz04AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M05, UpMezz05AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M06, UpMezz06AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M07, UpMezz07AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M08, UpMezz08AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M09, UpMezz09AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M10, UpMezz10AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M11, UpMezz11AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M12, UpMezz12AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M13, UpMezz13AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M14, UpMezz14AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M15, UpMezz15AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M16, UpMezz16AMTSetupControl);
  SetupUserButton(Menu00Handle, MENU00_JTAG_MEZZSETUPIND_M17, UpMezz17AMTSetupControl);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_DONE, AMTDone);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_DEFAULT, PanelDefault);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_CANCEL, AMTCancel);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_EN_SERIAL, AMTStrobeSelection);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_MEZZCARD, AMTMezzCard);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_ALLCHANNELON, AMTChannelAllOn);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_ALLCHANNELOFF, AMTChannelAllOff);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_CHANNEL0TO7ON, AMTChannel0To7On);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_CHANNEL0TO7OFF, AMTChannel0To7Off);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_CHANNEL8TO15ON, AMTChannel8To15On);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_CHANNEL8TO15OFF, AMTChannel8To15Off);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_CHANNEL16TO23ON, AMTChannel16To23On);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_CHANNEL16TO23OFF, AMTChannel16To23Off);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_ASD1CONTROL, ASD1Control);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_ASD2CONTROL, ASD2Control);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_ASD3CONTROL, ASD3Control);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_SAVESETUP, SaveMezzCardSetup);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_ERRORCONTROL, AMTErrorControl);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_COPYSETUP, CopyMezzSetupControl);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_LEADING, DisablePair);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_TRAILING, DisablePair);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_PAIR, DisableLeadingAndTrailing);
  SetupUserButton(AMTSetupHandle, P_AMTSETUP_LOADTHRESHOLD, LoadNominalASDMainThreshold);
  
  // AMT Error Control Panel Buttons
  SetupUserButton(AMTErrorHandle, P_AMTERROR_DONE, AMTErrorControlDone);
  SetupUserButton(AMTErrorHandle, P_AMTERROR_CANCEL, AMTErrorControlCancel);
  
  // ASD Control Panel Buttons
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_DEFAULT, PanelDefault);
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_DONE, ASDDone);
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_CANCEL, ASDCancel);
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_ALLCHANNELON, ASDAllChannelOn);
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_ALLCHANNELOFF, ASDAllChannelOff);
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_CHANNELSMODEACTIVE, ASDChannelsModeActive);
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_CHANNELSMODEHIGH, ASDChannelsModeHigh);
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_CHANNELSMODELOW, ASDChannelsModeLow);
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_APPLYTOALLASD, ApplyToAllASD);
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_APPLYTO1STNEXTASD, ApplyTo1stNextASD);
  SetupUserButton(ASDSetupHandle, P_ASDSETUP_APPLYTO2RDNEXTASD, ApplyTo2rdNextASD);
   
  // Copy Mezzanine Card Setup Panel Buttons
  SetupUserButton(CopyMezzSetupHandle, P_COPYMEZZ_STARTCOPY, CopyMezzSetup);
  SetupUserButton(CopyMezzSetupHandle, P_COPYMEZZ_DONE, CopyMezzSetupDone);
  SetupUserButton(CopyMezzSetupHandle, P_COPYMEZZ_FROM, CopyMezzSetupFrom);
  SetupUserButton(CopyMezzSetupHandle, P_COPYMEZZ_SELECTALL, SelectAllForCopy);
  SetupUserButton(CopyMezzSetupHandle, P_COPYMEZZ_UNSELECTALL, UnselectAllForCopy);

  return 0;
}


void DownloadAllMezzCardSetup(void) {
  int mezz, on, off;
  
  // Setup all mezzanine cards with common settings
  mezzCardSetupAll = 1;
  downloadMezzSetup = FALSE;
  RecallMezzanineSetup();
  AMTDone();
  // Setup individual mezzanine card
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (((mezzEnables>>mezz)&1) == 0) off = TRUE;
    else off = FALSE;
    if (off) {
      SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezz], ATTR_CHECKED, 0);
      SetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezz], FALSE);
    }
    else
      SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezz], ATTR_DIMMED, 0);
    GetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezz], &on);
    if (on == 1) {
      mezzCardSetupAll = 0;
      mezzCardNb = mezz;
      RecallMezzanineSetup();
      AMTDone();
    }
  }
  downloadMezzSetup = TRUE;
  mezzCardSetupAll = 1;
  DownloadMezzCardSetup();
}


// Does a global reset of the AMT via JTAG by toggling the global reset bit in AMT
void AMTGlobalReset(void) {
  int savedGlobalReset;
  
  // Using Setup All to perform AMT global reset
  printf("Preparing for AMT Global Reset via JTAG\n");
  mezzCardSetupAll = 1;
  RecallMezzanineSetup();
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_GLOBALRESET, &savedGlobalReset);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_GLOBALRESET, 1);
  AMTDone();
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_GLOBALRESET, savedGlobalReset);
  PanelSave(AMTSetupHandle);
  LoadAMTSetupArray(); 
  printf("AMT Global Reset via JTAG is done\n");
}


int MezzCardsClockMode(void) {
  int mode = AMTUNDEFINEDCLKMODE, i, mezz, clkMode;
  
  if (!downloadMezzSetupDone) return mode;
  mezz = MAXNUMBERMEZZANINE;
  for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++) statusSetupCheck[i] = mezzSetupArray[i][mezz];
  if (statusSetupCheck[AMTS_SETUP+AMTS_ENB_SER] == 0) return mode;
  
  BinaryToInt(&mode, AMTS_SETUP+AMTS_STROBE_SEL, 2, statusSetupCheck);
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (mezzInitStatus[mezz] != MEZZNOINIT) {
      for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++) statusSetupCheck[i] = mezzSetupArray[i][mezz];
      BinaryToInt(&clkMode, AMTS_SETUP+AMTS_STROBE_SEL, 2, statusSetupCheck);
      if (statusSetupCheck[AMTS_SETUP+AMTS_ENB_SER] == 0) mode = AMTUNDEFINEDCLKMODE;
      if (mode != clkMode) mode = AMTUNDEFINEDCLKMODE;
    }
  }
  
  return mode;
}


//**************************************************************
//
// AMT Panel Routines Here
//
//**************************************************************
//
// AMT Setup Done
void AMTDone(void) {
  int i, same, mezz, off;

  if (mezzCardSetupAll) GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &nbMezzCard);
  else {
    GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &mezzCardNb);
    if (((mezzEnables>>mezzCardNb)&1) == 0) off = TRUE;
    else off = FALSE;
    if (off) {
      SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezzCardNb], ATTR_CHECKED, 0);
      SetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezzCardNb], FALSE);
      MessagePopup("WARNING On MEZZ CARD ID", "Unable to download Selected Mezzanine Card Setup");
      printf("Unable to download Selected Mezzanine Card (=%d) setup.\n", mezzCardNb);
      return;
    }
    SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezzCardNb], ATTR_CHECKED, 1);
    SetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezzCardNb], TRUE);
  }
  if (downloadMezzSetup) {
    ASDDone();
    WaitInMillisecond(5);
    AMTErrorControlDone();
  }
  WaitInMillisecond(5);
  PanelSave(AMTSetupHandle);
  LoadAMTSetupArray(); 
  HidePanel(AMTSetupHandle);
  DisplayPanel(JTAGControlHandle);
  SetActivePanel(JTAGControlHandle);
  if (mezzCardSetupAll) {
    for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++)
      mezzSetupArray[i][MAXNUMBERMEZZANINE] = basicSetupArray[i];
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++)
      for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++)
        mezzSetupArray[i][mezz] = basicSetupArray[i];
    ReversedBinaryToInt(&same, ASD_MAIN_THRESH, 8, basicSetupArray);
  }
  else {
    mezz = mezzCardNb;
    for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++) mezzSetupArray[i][mezz] = basicSetupArray[i];
    ReversedBinaryToInt(&same, ASD_MAIN_THRESH, 8, basicSetupArray);
  }
  if (downloadMezzSetup) DownloadMezzCardSetup();
  UpdateJTAGControlPanel();     // Update JTAG control panel
  if (mezzCardSetupAll) {
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_MEZZSETUPIND, ATTR_DIMMED, 0);
    for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++)
      mezzSetupArray[i][MAXNUMBERMEZZANINE] = basicSetupArray[i];
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      IntToBinary(mezz, AMTS_SETUP+AMTS_TDC_ID, 4, basicSetupArray, SETUP_LENGTH); 
      if (MezzCardSetupStatus(mezz) <= -1) mezzInitStatus[mezz] = MEZZNOINIT;
      else {
        mezzInitStatus[mezz] = MEZZINITBYSETUPALL;
        if (MezzCardSetupStatus(mezz) != 0) mezzInitStatus[mezz] |= MEZZAMTINITERROR;
      }
      if (((mezzEnables>>mezz)&1) == 0) off = TRUE;
      else off = FALSE;
      if (off) {
        SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezz], ATTR_DIMMED, 1);
        lEdgeOn[mezz] = 0;
        tEdgeOn[mezz] = 0;
        pairOn[mezz] = 0;
      }
      else {
        SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezz], ATTR_DIMMED, 0);
        for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++)
          mezzSetupArray[i][mezz] = basicSetupArray[i];
        lEdgeOn[mezz] = mezzSetupArray[AMTS_SETUP+AMTS_ENB_LEAD][MAXNUMBERMEZZANINE];
        tEdgeOn[mezz] = mezzSetupArray[AMTS_SETUP+AMTS_ENB_TRL][MAXNUMBERMEZZANINE];
        pairOn[mezz] = mezzSetupArray[AMTS_SETUP+AMTS_ENB_PAIR][MAXNUMBERMEZZANINE];
        if (pairOn[mezz]) {
          BinaryToInt(&widthSelection, AMTS_SETUP+AMTS_SEL_WID, 3, basicSetupArray);
        }
        else widthSelection = 0;
        ReversedBinaryToInt(&integrationGate, ASD_WIL_INT, 4, basicSetupArray);
        ReversedBinaryToInt(&rundownCurrent, ASD_WIL_CUR, 3, basicSetupArray);
      }
      SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezz], ATTR_CHECKED, 0);
    }
    for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++)
      basicSetupArray[i] = mezzSetupArray[i][MAXNUMBERMEZZANINE];
  }
  else {
    mezz = mezzCardNb;
    for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++) mezzSetupArray[i][mezz] = basicSetupArray[i];
    if (MezzCardSetupStatus(mezz) <= -1) mezzInitStatus[mezz] = MEZZNOINIT;
    else {
      mezzInitStatus[mezz] = MEZZINITBYSETUPIND;
      if (MezzCardSetupStatus(mezz) != 0) mezzInitStatus[mezz] |= MEZZAMTINITERROR;
    }
    same = TRUE;
    for (i = 0; i < AMTS_SETUP+AMTS_TDC_ID; i++)
      if (mezzSetupArray[i][MAXNUMBERMEZZANINE] != mezzSetupArray[i][mezz]) same = FALSE;
    for (i = AMTS_SETUP+AMTS_TDC_ID+4; i < AMTS_SETUP+AMTSETUP_LENGTH; i++)
      if (mezzSetupArray[i][MAXNUMBERMEZZANINE] != mezzSetupArray[i][mezz]) same = FALSE;
    if (same) {  // Mezz. Card Settings are same as common settings
      SetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezz], FALSE);
      SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezz], ATTR_CHECKED, 0);
    }
    SetMenuBarAttribute(Menu00Handle, MENU00_JTAG_MEZZSETUPIND, ATTR_DIMMED, 0);
    lEdgeOn[mezz] = mezzSetupArray[AMTS_SETUP+AMTS_ENB_LEAD][mezz];
    tEdgeOn[mezz] = mezzSetupArray[AMTS_SETUP+AMTS_ENB_TRL][mezz];
    pairOn[mezz] = mezzSetupArray[AMTS_SETUP+AMTS_ENB_PAIR][mezz];
    if (pairOn[mezz]) {
      BinaryToInt(&widthSelection, AMTS_SETUP+AMTS_SEL_WID, 3, basicSetupArray);
    }
    else widthSelection = 0;
    ReversedBinaryToInt(&integrationGate, ASD_WIL_INT, 4, basicSetupArray);
    ReversedBinaryToInt(&rundownCurrent, ASD_WIL_CUR, 3, basicSetupArray);
    // SetMezzCardSetupStatus();
  }
}


// Cancel changes made to the AMT JTAG Setup panel
void AMTCancel(void) {
  int response;
  
  response = ConfirmPopup("AMT Setup Cancellation Confirmation",
                          "Are you sure to discard all changes for AMT & ASD setup?");
  if (response == 1) {
    selectedASD = 0;
    applyToASD1 = 0;
    applyToASD2 = 0;
    applyToASD3 = 0;
    PanelRecall(AMTSetupHandle);        // Recall the panel states
    AMTErrorControlCancel();
    HidePanel(ASDSetupHandle);
    HidePanel(AMTSetupHandle);
    DisplayPanel(JTAGControlHandle);
    SetActivePanel(JTAGControlHandle);
  }
}


// Set strobe selection according to serial control bit
void AMTStrobeSelection(void) {
  static int serialEnabled = 1, nlist, item, mezz, val = 3, sval = 0, cval = 3;
  
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, &val);
  if (serialEnabled == 0) sval = val;
  else cval = val;
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_EN_SERIAL, &serialEnabled);
  GetNumListItems(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, &nlist);
  DeleteListItem(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, 0, nlist);
  item = 0;
  if (serialEnabled == 0) {
    InsertListItem(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, item++, "Continuous", 0);
    InsertListItem(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, item++, "Handshaked", 1);
    SetCtrlVal(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, sval);
  }
  else {
    if (mezzCardSetupAll) mezz = MAXNUMBERMEZZANINE;
    else mezz = mezzCardNb;
    InsertListItem(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, item++, "DS Strobe", 0);
    InsertListItem(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, item++, "DS STR Cont", 1);
    if ((AMTIDCode[mezz] == AMT2ID) || (AMTIDCode[mezz] == AMT1ID)) 
      InsertListItem(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, item++, "Lead Edge", 2);
    else
      InsertListItem(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, item++, "Clock Off", 2);
    InsertListItem(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, item++, "Edge Cont", 3);
    SetCtrlVal(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, cval);
  }
}


// Change Mezzanine Card ID
void AMTMezzCard(void) {
  static time_t start = NULL;
  int i, ctrl, handle, response, value, breakAndExecute, mezz, on, numberOn = 0;
  char str[512];
  
  if (start != NULL) {
    time(&start);
    return;
  }
  time(&start);
  breakAndExecute = 0;
  while (((int) (time(NULL)-start)) < 2) {
    // Handle buttons
    if (GetUserEvent(0, &handle, &ctrl) == 1) {
      for (i = 0; i < MAXBUTTON; i++) {
        if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
          if (userButtons[i].func) {
            if (userButtons[i].func != AMTMezzCard) {
              breakAndExecute = 1;
              break;
            }
            else (*userButtons[i].func)();
          }
        }
      }
    }
    if (breakAndExecute == 1) break;
  }
  start = NULL;
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &value);
  if (mezzCardSetupAll)
    sprintf(str, "Change Number of Mezzanine Card from %d to %d, are you sure?", nbMezzCard, value);
  else
    sprintf(str, "Change Mezzanine Card ID from %d to %d, are you sure?", mezzCardNb, value);
  response = ConfirmPopup("Change Confirmation for Mezzanine Card", str);
  if (response == 1) {
    if (mezzCardSetupAll) {
      for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
        GetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezz], &on);
        if (on == 1) numberOn++;
      }
      if (numberOn > 0) {
        sprintf(str, "Number of Mezzanine Cards has changed from %d to %d.\n"
                     "Mezzanine card individual settings may not correct any more.\n"
                     "Do you still want to keep individual settings of mezzanine cards?", 
                     nbMezzCard, value);
        response = ConfirmPopup("WARNING on Mezzanine Card Individual Settings", str);
        if (response == 0) {
          for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
            SetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezz], FALSE);
            SetMenuBarAttribute(Menu00Handle, mezzButtonForMenu[mezz], ATTR_CHECKED, 0);
          }
        }
      }
      nbMezzCard = value;
    }
    else mezzCardNb = value;
  }
  else {
    if (mezzCardSetupAll) value = nbMezzCard;
    else value = mezzCardNb;
    SetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, value);
  }
  if (breakAndExecute == 1) (*userButtons[i].func)();
}


// Turn on all AMT Channels
void AMTChannelAllOn(void) {
  AMTChannel0To7On();
  AMTChannel8To15On();
  AMTChannel16To23On();
}


// Turn off all AMT Channels
void AMTChannelAllOff(void) {
  AMTChannel0To7Off();
  AMTChannel8To15Off();
  AMTChannel16To23Off();
}


// Turn on AMT Channel 0 to 7
void AMTChannel0To7On(void) {
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL0, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL1, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL2, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL3, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL4, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL5, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL6, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL7, 1);
}


// Turn off AMT Channel 0 to 7
void AMTChannel0To7Off(void) {
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL0, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL1, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL2, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL3, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL4, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL5, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL6, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL7, 0);
}


// Turn on AMT Channel 8 to 15
void AMTChannel8To15On(void) {
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL8, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL9, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL10, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL11, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL12, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL13, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL14, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL15, 1);
}


// Turn off AMT Channel 8 to 15
void AMTChannel8To15Off(void) {
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL8, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL9, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL10, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL11, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL12, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL13, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL14, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL15, 0);
}


// Turn on AMT Channel 16 to 23
void AMTChannel16To23On(void) {
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL16, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL17, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL18, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL19, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL20, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL21, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL22, 1);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL23, 1);
}


// Turn off AMT Channel 16 to 23
void AMTChannel16To23Off(void) {
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL16, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL17, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL18, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL19, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL20, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL21, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL22, 0);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL23, 0);
}


void SaveASDSetupChanges(int saveOption) {
  int index, ASDNumber;
  
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &mezzCardNb);
  if ((selectedASD > 0) && (selectedASD <= MAXNUMBERASD)) {
    SavePanelState(ASDSetupHandle, ASDTmpSetupFile, selectedASD);
    index = PanelIndexInFile(ASDSetupHandle, mezzCardSetupAll, mezzCardNb, selectedASD);
    if (saveOption == 1) SavePanelState(ASDSetupHandle, currSetupFile, index);
    if (applyToASD1 == 1) {
      RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, 1);
      index = PanelIndexInFile(ASDSetupHandle, mezzCardSetupAll, mezzCardNb, 1);
      if (saveOption == 1) SavePanelState(ASDSetupHandle, currSetupFile, index);
    }
    if (applyToASD2 == 1) {
      RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, 2);
      index = PanelIndexInFile(ASDSetupHandle, mezzCardSetupAll, mezzCardNb, 2);
      if (saveOption == 1) SavePanelState(ASDSetupHandle, currSetupFile, index);
    }
    if (applyToASD3 == 1) {
      RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, 3);
      index = PanelIndexInFile(ASDSetupHandle, mezzCardSetupAll, mezzCardNb, 3);
      if (saveOption == 1) SavePanelState(ASDSetupHandle, currSetupFile, index);
    }
    if (saveOption == 1) {
      for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
        RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, ASDNumber);
        index = PanelIndexInFile(ASDSetupHandle, mezzCardSetupAll, mezzCardNb, ASDNumber);
        SavePanelState(ASDSetupHandle, currSetupFile, index);
        LoadASDSetupArray(ASDNumber);
      }
    }
  }
  selectedASD = 0;
  applyToASD1 = 0;
  applyToASD2 = 0;
  applyToASD3 = 0;
}


void ASD1Control(void) {
  SaveASDSetupChanges(0);
  selectedASD = 1;
  RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, selectedASD);
  SetPanelAttribute(ASDSetupHandle, ATTR_TITLE, "ASD1 Control");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL0, ATTR_LABEL_TEXT, "Channel 0");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL1, ATTR_LABEL_TEXT, "Channel 1");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL2, ATTR_LABEL_TEXT, "Channel 2");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL3, ATTR_LABEL_TEXT, "Channel 3");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL4, ATTR_LABEL_TEXT, "Channel 4");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL5, ATTR_LABEL_TEXT, "Channel 5");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL6, ATTR_LABEL_TEXT, "Channel 6");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL7, ATTR_LABEL_TEXT, "Channel 7");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, ATTR_LABEL_TEXT, "Channel 0");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL1MODE, ATTR_LABEL_TEXT, "Channel 1");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL2MODE, ATTR_LABEL_TEXT, "Channel 2");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL3MODE, ATTR_LABEL_TEXT, "Channel 3");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL4MODE, ATTR_LABEL_TEXT, "Channel 4");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL5MODE, ATTR_LABEL_TEXT, "Channel 5");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL6MODE, ATTR_LABEL_TEXT, "Channel 6");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL7MODE, ATTR_LABEL_TEXT, "Channel 7");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_APPLYTO1STNEXTASD, ATTR_LABEL_TEXT, "Apply to ASD2");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_APPLYTO2RDNEXTASD, ATTR_LABEL_TEXT, "Apply to ASD3");
  DisplayPanel(ASDSetupHandle);
  SetActivePanel(ASDSetupHandle);
}


void ASD2Control(void) {
  SaveASDSetupChanges(0);
  selectedASD = 2;
  RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, selectedASD);
  SetPanelAttribute(ASDSetupHandle, ATTR_TITLE, "ASD2 Control");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL0, ATTR_LABEL_TEXT, "Channel 8");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL1, ATTR_LABEL_TEXT, "Channel 9");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL2, ATTR_LABEL_TEXT, "Channel 10");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL3, ATTR_LABEL_TEXT, "Channel 11");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL4, ATTR_LABEL_TEXT, "Channel 12");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL5, ATTR_LABEL_TEXT, "Channel 13");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL6, ATTR_LABEL_TEXT, "Channel 14");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL7, ATTR_LABEL_TEXT, "Channel 15");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, ATTR_LABEL_TEXT, "Channel 8");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL1MODE, ATTR_LABEL_TEXT, "Channel 9");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL2MODE, ATTR_LABEL_TEXT, "Channel 10");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL3MODE, ATTR_LABEL_TEXT, "Channel 11");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL4MODE, ATTR_LABEL_TEXT, "Channel 12");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL5MODE, ATTR_LABEL_TEXT, "Channel 13");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL6MODE, ATTR_LABEL_TEXT, "Channel 14");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL7MODE, ATTR_LABEL_TEXT, "Channel 15");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_APPLYTO1STNEXTASD, ATTR_LABEL_TEXT, "Apply to ASD1");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_APPLYTO2RDNEXTASD, ATTR_LABEL_TEXT, "Apply to ASD3");
  DisplayPanel(ASDSetupHandle);
  SetActivePanel(ASDSetupHandle);
}


void ASD3Control(void) {
  SaveASDSetupChanges(0);
  selectedASD = 3;
  RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, selectedASD);
  SetPanelAttribute(ASDSetupHandle, ATTR_TITLE, "ASD3 Control");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL0, ATTR_LABEL_TEXT, "Channel 16");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL1, ATTR_LABEL_TEXT, "Channel 17");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL2, ATTR_LABEL_TEXT, "Channel 18");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL3, ATTR_LABEL_TEXT, "Channel 19");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL4, ATTR_LABEL_TEXT, "Channel 20");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL5, ATTR_LABEL_TEXT, "Channel 21");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL6, ATTR_LABEL_TEXT, "Channel 22");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL7, ATTR_LABEL_TEXT, "Channel 23");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, ATTR_LABEL_TEXT, "Channel 16");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL1MODE, ATTR_LABEL_TEXT, "Channel 17");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL2MODE, ATTR_LABEL_TEXT, "Channel 18");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL3MODE, ATTR_LABEL_TEXT, "Channel 19");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL4MODE, ATTR_LABEL_TEXT, "Channel 20");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL5MODE, ATTR_LABEL_TEXT, "Channel 21");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL6MODE, ATTR_LABEL_TEXT, "Channel 22");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL7MODE, ATTR_LABEL_TEXT, "Channel 23");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, ATTR_LABEL_TEXT, "CH 16 Mode");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_APPLYTO1STNEXTASD, ATTR_LABEL_TEXT, "Apply to ASD1");
  SetCtrlAttribute(ASDSetupHandle, P_ASDSETUP_APPLYTO2RDNEXTASD, ATTR_LABEL_TEXT, "Apply to ASD2");
  DisplayPanel(ASDSetupHandle);
  SetActivePanel(ASDSetupHandle);
}


// Save Mezz. Card setup into a file
void SaveMezzCardSetup(void) {
  int ASDNumber;
  
  PromptPopup("Get Setup File Name",
              "Please give the file name for saving mezzanine card setup",
              mezzSetupFileName, 280);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &mezzCardNb);
  if ((selectedASD > 0) && (selectedASD <= MAXNUMBERASD))
    SavePanelState(ASDSetupHandle, ASDTmpSetupFile, selectedASD);
  for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
    RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, ASDNumber);
    LoadASDSetupArray(ASDNumber);
  }
  if ((selectedASD > 0) && (selectedASD <= MAXNUMBERASD)) {
    RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, selectedASD);
    LoadASDSetupArray(ASDNumber);
  }
  LoadAMTSetupArray(); 
  WriteSetupFile();
}


int WriteSetupFile(void) {
  int i, temp;
  FILE *setupWrite;
  
  if (strlen(mezzSetupFileName) <= 5 ) strcpy(mezzSetupFileName, "AMTandASDSetup.txt");
  printf("\nSave AMT and ASD setup string to the file <%s>.\n", mezzSetupFileName);
  setupWrite = fopen(mezzSetupFileName, "w");
  // ASD1 setup stuff
  fprintf(setupWrite, "%d\t", ASD_CH0);
  for (i = ASD_CH0; i <= ASD_CH7; i++)
    fprintf(setupWrite, "%d ", basicSetupArray[i]);
  fprintf(setupWrite, "    asd 1 channel mask\n");
   
  ReversedBinaryToInt(&temp, ASD_CAL_CAP, 3, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_CAL_CAP); 
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 calibration capacitors\n");

  ReversedBinaryToInt(&temp, ASD_MAIN_THRESH, 8, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_MAIN_THRESH);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 main threshold\n");
   
  ReversedBinaryToInt(&temp, ASD_WIL_THRESH, 3, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_WIL_THRESH);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 wilkinson threshold\n");
   
  ReversedBinaryToInt(&temp, ASD_HYSTERESIS, 4, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_HYSTERESIS);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 hysteresis\n");   
   
  ReversedBinaryToInt(&temp, ASD_WIL_INT, 4, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_WIL_INT);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 wilkinson integration length\n");
   
  ReversedBinaryToInt(&temp, ASD_WIL_CUR, 3, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_WIL_CUR);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 wilkinson current\n");   	
   
  ReversedBinaryToInt(&temp, ASD_DEADTIME, 3, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_DEADTIME);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 deadtime\n");
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_0, 2, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_CHANNEL_MODE_0);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 Channel 0 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_1, 2, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_CHANNEL_MODE_1);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 Channel 1 mode\n");   	
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_2, 2, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_CHANNEL_MODE_2);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 Channel 2 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_3, 2, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_CHANNEL_MODE_3);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 Channel 3 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_4, 2, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_CHANNEL_MODE_4);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 Channel 4 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_5, 2, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_CHANNEL_MODE_5);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 Channel 5 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_6, 2, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_CHANNEL_MODE_6);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 Channel 6 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_7, 2, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_CHANNEL_MODE_7);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 Channel 7 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHIP_MODE, 1, &basicSetupArray[0]);
  fprintf(setupWrite, "%d\t", ASD_CHIP_MODE);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 1 Chip mode\n");    

  // ASD 2 setup stuff 
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CH0);
  for (i = ASD_CH0; i <= ASD_CH7; i++)
    fprintf(setupWrite, "%d ", basicSetupArray[ASD_SETUP_LENGTH+i]);
  fprintf(setupWrite, "    asd 2 channel mask\n");
   
  ReversedBinaryToInt(&temp, ASD_CAL_CAP, 3, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CAL_CAP); 
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 calibration capacitors\n");

  ReversedBinaryToInt(&temp, ASD_MAIN_THRESH, 8, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_MAIN_THRESH);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 main threshold\n");
   
  ReversedBinaryToInt(&temp, ASD_WIL_THRESH, 3, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_WIL_THRESH);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 wilkinson threshold\n");
   
  ReversedBinaryToInt(&temp, ASD_HYSTERESIS, 4, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_HYSTERESIS);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 hysteresis\n");   
   
  ReversedBinaryToInt(&temp, ASD_WIL_INT, 4, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_WIL_INT);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 wilkinson integration length\n");   	
   
  ReversedBinaryToInt(&temp, ASD_WIL_CUR, 3, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_WIL_CUR);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 wilkinson current\n");   	
   
  ReversedBinaryToInt(&temp, ASD_DEADTIME, 3, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_DEADTIME);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 deadtime\n");
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_0, 2, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CHANNEL_MODE_0);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 Channel 0 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_1, 2, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CHANNEL_MODE_1);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 Channel 1 mode\n");   	
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_2, 2, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CHANNEL_MODE_2);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 Channel 2 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_3, 2, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CHANNEL_MODE_3);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 Channel 3 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_4, 2, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CHANNEL_MODE_4);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 Channel 4 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_5, 2, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CHANNEL_MODE_5);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 Channel 5 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_6, 2, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CHANNEL_MODE_6);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 Channel 6 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_7, 2, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CHANNEL_MODE_7);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 Channel 7 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHIP_MODE, 1, &basicSetupArray[ASD_SETUP_LENGTH]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH+ASD_CHIP_MODE);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 2 Chip mode\n");   

  // ASD 3 setup stuff
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CH0);
  for (i = ASD_CH0; i <= ASD_CH7; i++)
    fprintf(setupWrite, "%d ", basicSetupArray[ASD_SETUP_LENGTH*2+i]);
  fprintf(setupWrite, "    asd 3 channel mask\n");
   
  ReversedBinaryToInt(&temp, ASD_CAL_CAP, 3, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CAL_CAP); 
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 calibration capacitors\n");

  ReversedBinaryToInt(&temp, ASD_MAIN_THRESH, 8, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_MAIN_THRESH);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 main threshold\n");
   
  ReversedBinaryToInt(&temp, ASD_WIL_THRESH, 3, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_WIL_THRESH);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 wilkinson threshold\n");
   
  ReversedBinaryToInt(&temp, ASD_HYSTERESIS, 4, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_HYSTERESIS);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 hysteresis\n");   
   
  ReversedBinaryToInt(&temp, ASD_WIL_INT, 4, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_WIL_INT);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 wilkinson integration length\n");   	
   
  ReversedBinaryToInt(&temp, ASD_WIL_CUR, 3, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_WIL_CUR);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 wilkinson current\n");   	
   
  ReversedBinaryToInt(&temp, ASD_DEADTIME, 3, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_DEADTIME);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 deadtime\n");
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_0, 2, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CHANNEL_MODE_0);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 Channel 0 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_1, 2, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CHANNEL_MODE_1);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 Channel 1 mode\n");   	
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_2, 2, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CHANNEL_MODE_2);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 Channel 2 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_3, 2, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CHANNEL_MODE_3);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 Channel 3 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_4, 2, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CHANNEL_MODE_4);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 Channel 4 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_5, 2, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CHANNEL_MODE_5);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 Channel 5 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_6, 2, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CHANNEL_MODE_6);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 Channel 6 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHANNEL_MODE_7, 2, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CHANNEL_MODE_7);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 Channel 7 mode\n"); 
   
  ReversedBinaryToInt(&temp, ASD_CHIP_MODE, 1, &basicSetupArray[ASD_SETUP_LENGTH*2]);
  fprintf(setupWrite, "%d\t", ASD_SETUP_LENGTH*2+ASD_CHIP_MODE);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   asd 3 Chip mode\n");  

  // AMT's setup stuff
   
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_PLL_MULT);
  BinaryToInt(&temp, AMTS_PLL_MULT, 2, &basicSetupArray[AMTS_SETUP]);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   phase lock loop multiplier\n");

  BinaryToInt(&temp, AMTS_SETUP+AMTS_CLK_MODE, 2, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CLK_MODE);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   clock mode\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_DIS_RING);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_DIS_RING]);
  fprintf(setupWrite, "%s", "   disable ring osc\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_DIR_CNTL);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_DIR_CNTL]);
  fprintf(setupWrite, "%s", "   direct control\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_TST_INV);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_TST_INV]);
  fprintf(setupWrite, "%s", "   test invert mode\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_TST_MOD);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_TST_MOD]);
  fprintf(setupWrite, "%s", "   test mode\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_ERST_BCECRST);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_ERST_BCECRST]);
  fprintf(setupWrite, "%s", "   reset err on bcrst and ecrst\n");
   
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_DIS_ENCODE);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_DIS_ENCODE]);
  fprintf(setupWrite, "%s", "   disable encode\n");
   
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_GL_RST);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_GL_RST]);
  fprintf(setupWrite, "%s", "   global reset line\n");
   
  BinaryToInt(&temp, AMTS_SETUP+AMTS_MSK_WDW, 12, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_MSK_WDW);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   mask window\n");

  BinaryToInt(&temp, AMTS_SETUP+AMTS_SRCH_WDW, 12, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_SRCH_WDW);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   search window\n");

  BinaryToInt(&temp, AMTS_SETUP+AMTS_MTCH_WDW, 12, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_MTCH_WDW);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   match window\n");
   
  BinaryToInt(&temp, AMTS_SETUP+AMTS_RC_OFFSET, 12, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_RC_OFFSET);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   reject count offset\n");
   
  BinaryToInt(&temp, AMTS_SETUP+AMTS_EC_OFFSET, 12, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_EC_OFFSET);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   event count offset\n");

  BinaryToInt(&temp, AMTS_SETUP+AMTS_BC_OFFSET, 12, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_BC_OFFSET);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   bunch count offset\n");

  BinaryToInt(&temp, AMTS_SETUP+AMTS_COARSE_OFFSET, 12, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_COARSE_OFFSET);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   coarse time offset\n");
   
  BinaryToInt(&temp, AMTS_SETUP+AMTS_CNT_RO, 12, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CNT_RO);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   count rollover\n");

  BinaryToInt(&temp, AMTS_SETUP+AMTS_TDC_ID, 4, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_TDC_ID);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   tdc id\n");
  
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_ERRORFLAGTEST);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_ERRORFLAGTEST]);
  fprintf(setupWrite, "%s", "   error flag test\n");

  BinaryToInt(&temp, AMTS_SETUP+AMTS_SEL_WID, 3, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_SEL_WID);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   pulse width\n");

  BinaryToInt(&temp, AMTS_SETUP+AMTS_RD_SPEED, 2, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_RD_SPEED);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   serial readout speed\n");

  BinaryToInt(&temp, AMTS_SETUP+AMTS_STROBE_SEL, 2, basicSetupArray);
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_STROBE_SEL);
  fprintf(setupWrite, "%d", temp);
  fprintf(setupWrite, "%s", "   strobe select\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_ENB_LEAD);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_ENB_LEAD]);
  fprintf(setupWrite, "%s", "   amt leading\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_ENB_TRL);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_ENB_TRL]);
  fprintf(setupWrite, "%s", "   amt trailing\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_ENB_PAIR);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_ENB_PAIR]);
  fprintf(setupWrite, "%s", "   amt pair\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_ENABLE_REJECTED);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_ENABLE_REJECTED]);
  fprintf(setupWrite, "%s", "   enable rejected\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_TRAILER);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_TRAILER]);
  fprintf(setupWrite, "%s", "   AMT trailer\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_HEADER);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_HEADER]);
  fprintf(setupWrite, "%s", "   AMT header\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_ENB_SER);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_ENB_SER]);
  fprintf(setupWrite, "%s", "   amt enable serial\n");
   
  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_RELATIVE);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_RELATIVE]);
  fprintf(setupWrite, "%s", "   relative\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_MASK);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_MASK]);
  fprintf(setupWrite, "%s", "   amt mask flags\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_MATCH);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_MATCH]);
  fprintf(setupWrite, "%s", "   amt trigger match\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_L1_OCC);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_L1_OCC]);
  fprintf(setupWrite, "%s", "   L1 buff. occ. ro\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_AUTO_REJ);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_AUTO_REJ]);
  fprintf(setupWrite, "%s", "   auto reject\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_SET_ON_BR);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_SET_ON_BR]);
  fprintf(setupWrite, "%s", "   set counters on bunch reset\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_MR_ON_ER);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_MR_ON_ER]);
  fprintf(setupWrite, "%s", "   master reset on event reset\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_RCH_BUF);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_RCH_BUF]);
  fprintf(setupWrite, "%s", "   reset channel buf. on sep.\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_MR_RST);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_MR_RST]);
  fprintf(setupWrite, "%s", "   enc. master reset\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_BUF_OVR);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_BUF_OVR]);
  fprintf(setupWrite, "%s", "   buffer overflow detect\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_MRK_OVR);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_MRK_OVR]);
  fprintf(setupWrite, "%s", "   error mark overflow\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_EMRK_REJ);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_EMRK_REJ]);
  fprintf(setupWrite, "%s", "   error mark rejected\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_INCLKBOOST);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_INCLKBOOST]);
  fprintf(setupWrite, "%s", "   double the internal clock frequency\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_MARK);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_MARK]);
  fprintf(setupWrite, "%s", "   error mark\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_TRG_FULL);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_TRG_FULL]);
  fprintf(setupWrite, "%s", "   trigger full reject\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_L1_REJ);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_L1_REJ]);
  fprintf(setupWrite, "%s", "   L1 full reject\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_FUL_REJ);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_FUL_REJ]);
  fprintf(setupWrite, "%s", "   ro full reject\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_COARSE);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_COARSE]);
  fprintf(setupWrite, "%s", "   coarse error\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH_SEL);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH_SEL]);
  fprintf(setupWrite, "%s", "   channel select error\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH_SEL);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_L1_PAR]);
  fprintf(setupWrite, "%s", "   L1 buffer parity\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_FIFO_PAR);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_FIFO_PAR]);
  fprintf(setupWrite, "%s", "   trigger fifo parity\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_TRG_MATCH);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_TRG_MATCH]);
  fprintf(setupWrite, "%s", "   trigger match\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_RO_FIFO);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_RO_FIFO]);
  fprintf(setupWrite, "%s", "   readout fifo parity\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_RO_FIFO);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_RO_FIFO]);
  fprintf(setupWrite, "%s", "   readout state error\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_SETUP_PAR);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_SETUP_PAR]);
  fprintf(setupWrite, "%s", "   setup parity error\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_JTAG_INS);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_JTAG_INS]);
  fprintf(setupWrite, "%s", "   JTAG instruction error\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_SEP_ON_ER);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_SEP_ON_ER]);
  fprintf(setupWrite, "%s", "   sep. on event reset\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_SEP_ON_BR);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_SEP_ON_BR]);
  fprintf(setupWrite, "%s", "   amt seperator bc reset\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_SEP_RO);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_SEP_RO]);
  fprintf(setupWrite, "%s", "   amt seperator on readout\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH0);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH0]);
  fprintf(setupWrite, "%s", "   Channel 0 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH1);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH1]);
  fprintf(setupWrite, "%s", "   Channel 1 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH2);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH2]);
  fprintf(setupWrite, "%s", "   Channel 2 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH3);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH3]);
  fprintf(setupWrite, "%s", "   Channel 3 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH4);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH4]);
  fprintf(setupWrite, "%s", "   Channel 4 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH5);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH5]);
  fprintf(setupWrite, "%s", "   Channel 5 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH6);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH6]);
  fprintf(setupWrite, "%s", "   Channel 6 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH7);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH7]);
  fprintf(setupWrite, "%s", "   Channel 7 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH8);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH8]);
  fprintf(setupWrite, "%s", "   Channel 8 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH9);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH9]);
  fprintf(setupWrite, "%s", "   Channel 9 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH10);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH10]);
  fprintf(setupWrite, "%s", "   Channel 10 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH11);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH11]);
  fprintf(setupWrite, "%s", "   Channel 11 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH12);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH12]);
  fprintf(setupWrite, "%s", "   Channel 12 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH13);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH13]);
  fprintf(setupWrite, "%s", "   Channel 13 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH14);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH14]);
  fprintf(setupWrite, "%s", "   Channel 14 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH15);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH15]);
  fprintf(setupWrite, "%s", "   Channel 15 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH16);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH16]);
  fprintf(setupWrite, "%s", "   Channel 16 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH17);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH17]);
  fprintf(setupWrite, "%s", "   Channel 17 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH18);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH18]);
  fprintf(setupWrite, "%s", "   Channel 18 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH19);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH19]);
  fprintf(setupWrite, "%s", "   Channel 19 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH20);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH20]);
  fprintf(setupWrite, "%s", "   Channel 20 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH21);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH21]);
  fprintf(setupWrite, "%s", "   Channel 21 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH22);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH22]);
  fprintf(setupWrite, "%s", "   Channel 22 enable\n");

  fprintf(setupWrite, "%d\t", AMTS_SETUP+AMTS_CH23);
  fprintf(setupWrite, "%d", basicSetupArray[AMTS_SETUP+AMTS_CH23]);
  fprintf(setupWrite, "%s", "   Channel 23 enable\n");

  fclose(setupWrite);
  printf("\n");
  return(0);
}


// Brings up the AMT Error Setup Panel
void AMTErrorControl(void) {
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &mezzCardNb);
  PanelRecall(AMTErrorHandle);        // Recall the panel states
  DisplayPanel(AMTErrorHandle);
  SetActivePanel(AMTErrorHandle);
}

// Hide the AMT Error Setup Panel
void AMTErrorControlDone(void) {
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &mezzCardNb);
  PanelSave(AMTErrorHandle);         // Save the panel states for future use
  HidePanel(AMTErrorHandle);
}


// Cancel the AMT Error Setup
void AMTErrorControlCancel(void) {
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &mezzCardNb);
  PanelRecall(AMTErrorHandle);        // Recall the panel states
  HidePanel(AMTErrorHandle);
}


// Disable Pair if leading or trailing edge is on
void DisablePair(void) {
  int on, setting;
  
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_PAIR, &setting);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_LEADING, &on);
  if (on == 1) setting = 0;
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_TRAILING, &on);
  if (on == 1) setting = 0;
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_PAIR, setting);
}


// Disable Leading and Trailing Edge if pair is on
void DisableLeadingAndTrailing(void) {
  int on, setting;
  
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_PAIR, &on);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_LEADING, &setting);
  if (on == 1) setting = 0;
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_LEADING, setting);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_TRAILING, &setting);
  if (on == 1) setting = 0;
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_TRAILING, setting);
}


// Loads the AMT setup into setup array (basicSetupArray)
void LoadAMTSetupArray(void)
{
  int tdc_id, coarse_offset, bc_offset, event_offset, rc_offset, mask_window,
      search_window, match_window, pulse_width, count_rollover, strobe_select,
      serial_ro_speed, single_cycle_ro, pll_mult, clkout_mode, i;

  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_PLL_MULT, &pll_mult);
  IntToBinary(pll_mult, AMTS_SETUP+AMTS_PLL_MULT, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CLKOUT_MODE, &clkout_mode);
  IntToBinary(clkout_mode, AMTS_SETUP+AMTS_CLK_MODE, 2, basicSetupArray, SETUP_LENGTH);   
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_DIS_RING, &basicSetupArray[AMTS_SETUP+AMTS_DIS_RING]); 
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_DIR_CNTL, &basicSetupArray[AMTS_SETUP+AMTS_DIR_CNTL]); 
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_TEST_INVERT_MODE, &basicSetupArray[AMTS_SETUP+AMTS_TST_INV]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_TEST_MODE, &basicSetupArray[AMTS_SETUP+AMTS_TST_MOD]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_ERST_BCECRST, &basicSetupArray[AMTS_SETUP+AMTS_ERST_BCECRST]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_DIS_ENCODE, &basicSetupArray[AMTS_SETUP+AMTS_DIS_ENCODE]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_GLOBALRESET, &basicSetupArray[AMTS_SETUP+AMTS_GL_RST]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MASK_WINDOW, &mask_window);
  IntToBinary(mask_window, AMTS_SETUP+AMTS_MSK_WDW, 12, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_SEARCH_WINDOW, &search_window);
  IntToBinary(search_window, AMTS_SETUP+AMTS_SRCH_WDW, 12, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MATCH_WINDOW, &match_window);
  IntToBinary(match_window, AMTS_SETUP+AMTS_MTCH_WDW, 12, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_RC_OFFSET, &rc_offset);
  IntToBinary(rc_offset, AMTS_SETUP+AMTS_RC_OFFSET, 12, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_EC_OFFSET, &event_offset);
  IntToBinary(event_offset, AMTS_SETUP+AMTS_EC_OFFSET, 12, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_BC_OFFSET, &bc_offset);
  IntToBinary(bc_offset, AMTS_SETUP+AMTS_BC_OFFSET, 12, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_COARSE_OFFSET, &coarse_offset);
  IntToBinary(coarse_offset, AMTS_SETUP+AMTS_COARSE_OFFSET, 12, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CNT_ROLLOVER, &count_rollover);
  IntToBinary(count_rollover, AMTS_SETUP+AMTS_CNT_RO, 12, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &tdc_id);
  IntToBinary(tdc_id, AMTS_SETUP+AMTS_TDC_ID, 4, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_PULSE_WIDTH, &pulse_width);
  IntToBinary(pulse_width, AMTS_SETUP+AMTS_SEL_WID, 3, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_RD_SPEED, &serial_ro_speed);
  IntToBinary(serial_ro_speed, AMTS_SETUP+AMTS_RD_SPEED, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_STROBE_SEL, &strobe_select);
  IntToBinary(strobe_select, AMTS_SETUP+AMTS_STROBE_SEL, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_LEADING, &basicSetupArray[AMTS_SETUP+AMTS_ENB_LEAD]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_TRAILING, &basicSetupArray[AMTS_SETUP+AMTS_ENB_TRL]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_PAIR, &basicSetupArray[AMTS_SETUP+AMTS_ENB_PAIR]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_ENABLE_REJECTED, &basicSetupArray[AMTS_SETUP+AMTS_ENABLE_REJECTED]);   
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_TRAILER, &basicSetupArray[AMTS_SETUP+AMTS_TRAILER]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_HEADER, &basicSetupArray[AMTS_SETUP+AMTS_HEADER]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_EN_SERIAL, &basicSetupArray[AMTS_SETUP+AMTS_ENB_SER]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_RELATIVE, &basicSetupArray[AMTS_SETUP+AMTS_RELATIVE]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MASK_FLAGS, &basicSetupArray[AMTS_SETUP+AMTS_MASK]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_TRIG_MATCH, &basicSetupArray[AMTS_SETUP+AMTS_MATCH]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_L1_BUFF_OCC_RO, &basicSetupArray[AMTS_SETUP+AMTS_L1_OCC]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_AUTO_REJ, &basicSetupArray[AMTS_SETUP+AMTS_AUTO_REJ]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_SET_CTRS_BNCH_RST, &basicSetupArray[AMTS_SETUP+AMTS_SET_ON_BR]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MASTER_RESET_EVT, &basicSetupArray[AMTS_SETUP+AMTS_MR_ON_ER]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_RST_CHAN_BUFF_SEP, &basicSetupArray[AMTS_SETUP+AMTS_RCH_BUF]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_ENC_MASTER_RESET, &basicSetupArray[AMTS_SETUP+AMTS_MR_RST]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_BUFF_OVERFLOW_DET, &basicSetupArray[AMTS_SETUP+AMTS_BUF_OVR]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_ERR_MARK_OVERFLOW, &basicSetupArray[AMTS_SETUP+AMTS_MRK_OVR]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_ERR_MARK_REJ, &basicSetupArray[AMTS_SETUP+AMTS_EMRK_REJ]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_INCLKBOOST, &basicSetupArray[AMTS_SETUP+AMTS_INCLKBOOST]);  
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_TRIG_FULL_REJ, &basicSetupArray[AMTS_SETUP+AMTS_TRG_FULL]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_L1_FULL_REJ, &basicSetupArray[AMTS_SETUP+AMTS_L1_REJ]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_RO_FULL_REJ, &basicSetupArray[AMTS_SETUP+AMTS_FUL_REJ]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_SEP_EVT_RESET, &basicSetupArray[AMTS_SETUP+AMTS_SEP_ON_ER]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_SEP_BNCH_RESET, &basicSetupArray[AMTS_SETUP+AMTS_SEP_ON_BR]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_SEPARATOR_RO, &basicSetupArray[AMTS_SETUP+AMTS_SEP_RO]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL0, &basicSetupArray[AMTS_SETUP+AMTS_CH0]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL1, &basicSetupArray[AMTS_SETUP+AMTS_CH1]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL2, &basicSetupArray[AMTS_SETUP+AMTS_CH2]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL3, &basicSetupArray[AMTS_SETUP+AMTS_CH3]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL4, &basicSetupArray[AMTS_SETUP+AMTS_CH4]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL5, &basicSetupArray[AMTS_SETUP+AMTS_CH5]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL6, &basicSetupArray[AMTS_SETUP+AMTS_CH6]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL7, &basicSetupArray[AMTS_SETUP+AMTS_CH7]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL8, &basicSetupArray[AMTS_SETUP+AMTS_CH8]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL9, &basicSetupArray[AMTS_SETUP+AMTS_CH9]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL10, &basicSetupArray[AMTS_SETUP+AMTS_CH10]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL11, &basicSetupArray[AMTS_SETUP+AMTS_CH11]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL12, &basicSetupArray[AMTS_SETUP+AMTS_CH12]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL13, &basicSetupArray[AMTS_SETUP+AMTS_CH13]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL14, &basicSetupArray[AMTS_SETUP+AMTS_CH14]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL15, &basicSetupArray[AMTS_SETUP+AMTS_CH15]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL16, &basicSetupArray[AMTS_SETUP+AMTS_CH16]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL17, &basicSetupArray[AMTS_SETUP+AMTS_CH17]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL18, &basicSetupArray[AMTS_SETUP+AMTS_CH18]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL19, &basicSetupArray[AMTS_SETUP+AMTS_CH19]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL20, &basicSetupArray[AMTS_SETUP+AMTS_CH20]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL21, &basicSetupArray[AMTS_SETUP+AMTS_CH21]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL22, &basicSetupArray[AMTS_SETUP+AMTS_CH22]);
  GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL23, &basicSetupArray[AMTS_SETUP+AMTS_CH23]);
 
  // AMT Error Controls
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_AMT_ERR_MARK,          &basicSetupArray[AMTS_SETUP+AMTS_MARK]);    
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_AMT_ERR_COARSE,        &basicSetupArray[AMTS_SETUP+AMTS_COARSE]);
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_AMT_ERR_CH_SEL,        &basicSetupArray[AMTS_SETUP+AMTS_CH_SEL]);
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_AMT_ERR_L1_BUFFER_PAR, &basicSetupArray[AMTS_SETUP+AMTS_L1_PAR]);
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_AMT_TRIG_FIFO_PAR,     &basicSetupArray[AMTS_SETUP+AMTS_FIFO_PAR]);
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_AMT_TRIG_MATCH,        &basicSetupArray[AMTS_SETUP+AMTS_TRG_MATCH]);
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_AMT_ERR_RO_FIFO_PAR,   &basicSetupArray[AMTS_SETUP+AMTS_RO_FIFO]);
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_AMT_ERR_RO_STATE,      &basicSetupArray[AMTS_SETUP+AMTS_RO_STATE]);
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_AMT_ERR_SETUP_PAR,     &basicSetupArray[AMTS_SETUP+AMTS_SETUP_PAR]);
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_AMT_JTAG_INSTR,        &basicSetupArray[AMTS_SETUP+AMTS_JTAG_INS]);
  GetCtrlVal(AMTErrorHandle, P_AMTERROR_ERRORFLAGTEST,         &basicSetupArray[AMTS_SETUP+AMTS_ERRORFLAGTEST]);
  
  if (mezzCardSetupAll) {
    // Check if user has changed the total no. of TDC during setup and update indicator
    GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &nbMezzCard);
  }
  else {
    // Check if user has changed the TDC number during setup
    GetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, &mezzCardNb);
  }
  for (i = 0; i < AMTSETUP_LENGTH; i++)
    statusSetupCheck[AMTS_SETUP+i] = basicSetupArray[AMTS_SETUP+i];
}


// Modified by Xiangting

//Old ASD chip on HPTDC based mezzanine card.

/*

void LoadA3P250SetupArray(void)
{

	basicSetupArray_a3p250[ASD0_CH0]=asd0_ch0;
	basicSetupArray_a3p250[ASD0_CH1]=asd0_ch1;
	basicSetupArray_a3p250[ASD0_CH2]=asd0_ch2;
	basicSetupArray_a3p250[ASD0_CH3]=asd0_ch3;
	basicSetupArray_a3p250[ASD0_CH4]=asd0_ch4;
	basicSetupArray_a3p250[ASD0_CH5]=asd0_ch5;
	basicSetupArray_a3p250[ASD0_CH6]=asd0_ch6;
	basicSetupArray_a3p250[ASD0_CH7]=asd0_ch7;
	IntToReversedBinary(asd0_cal_cap,ASD0_CAL_CAP,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_main_thresh,ASD0_MAIN_THRESH,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_wil_thresh,ASD0_WIL_THRESH,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd0_hysteresis,ASD0_HYSTERESIS,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd0_wil_int,ASD0_WIL_INT,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd0_wil_cur,ASD0_WIL_CUR,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToReversedBinary(asd0_deadtime,ASD0_DEADTIME,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToReversedBinary(asd0_chmod_0,ASD0_CHANNEL_MODE_0,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd0_chmod_1,ASD0_CHANNEL_MODE_1,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_chmod_2,ASD0_CHANNEL_MODE_2,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd0_chmod_3,ASD0_CHANNEL_MODE_3,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd0_chmod_4,ASD0_CHANNEL_MODE_4,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd0_chmod_5,ASD0_CHANNEL_MODE_5,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_chmod_6,ASD0_CHANNEL_MODE_6,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd0_chmod_7,ASD0_CHANNEL_MODE_7,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	basicSetupArray_a3p250[ASD0_CHIP_MODE]=asd0_chip_mode;	

	
	
	basicSetupArray_a3p250[ASD1_CH0]=asd1_ch0;
	basicSetupArray_a3p250[ASD1_CH1]=asd1_ch1;
	basicSetupArray_a3p250[ASD1_CH2]=asd1_ch2;
	basicSetupArray_a3p250[ASD1_CH3]=asd1_ch3;
	basicSetupArray_a3p250[ASD1_CH4]=asd1_ch4;
	basicSetupArray_a3p250[ASD1_CH5]=asd1_ch5;
	basicSetupArray_a3p250[ASD1_CH6]=asd1_ch6;
	basicSetupArray_a3p250[ASD1_CH7]=asd1_ch7;
	IntToReversedBinary(asd1_cal_cap,ASD1_CAL_CAP,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_main_thresh,ASD1_MAIN_THRESH,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_wil_thresh,ASD1_WIL_THRESH,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd1_hysteresis,ASD1_HYSTERESIS,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd1_wil_int,ASD1_WIL_INT,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd1_wil_cur,ASD1_WIL_CUR,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToReversedBinary(asd1_deadtime,ASD1_DEADTIME,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToReversedBinary(asd1_chmod_0,ASD1_CHANNEL_MODE_0,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd1_chmod_1,ASD1_CHANNEL_MODE_1,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_chmod_2,ASD1_CHANNEL_MODE_2,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd1_chmod_3,ASD1_CHANNEL_MODE_3,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd1_chmod_4,ASD1_CHANNEL_MODE_4,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd1_chmod_5,ASD1_CHANNEL_MODE_5,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_chmod_6,ASD1_CHANNEL_MODE_6,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd1_chmod_7,ASD1_CHANNEL_MODE_7,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	basicSetupArray_a3p250[ASD1_CHIP_MODE]=asd1_chip_mode;	

 	
	
	basicSetupArray_a3p250[ASD2_CH0]=asd2_ch0;
	basicSetupArray_a3p250[ASD2_CH1]=asd2_ch1;
	basicSetupArray_a3p250[ASD2_CH2]=asd2_ch2;
	basicSetupArray_a3p250[ASD2_CH3]=asd2_ch3;
	basicSetupArray_a3p250[ASD2_CH4]=asd2_ch4;
	basicSetupArray_a3p250[ASD2_CH5]=asd2_ch5;
	basicSetupArray_a3p250[ASD2_CH6]=asd2_ch6;
	basicSetupArray_a3p250[ASD2_CH7]=asd2_ch7;
	IntToReversedBinary(asd2_cal_cap,ASD2_CAL_CAP,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_main_thresh,ASD2_MAIN_THRESH,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_wil_thresh,ASD2_WIL_THRESH,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd2_hysteresis,ASD2_HYSTERESIS,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd2_wil_int,ASD2_WIL_INT,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd2_wil_cur,ASD2_WIL_CUR,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToReversedBinary(asd2_deadtime,ASD2_DEADTIME,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToReversedBinary(asd2_chmod_0,ASD2_CHANNEL_MODE_0,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd2_chmod_1,ASD2_CHANNEL_MODE_1,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_chmod_2,ASD2_CHANNEL_MODE_2,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd2_chmod_3,ASD2_CHANNEL_MODE_3,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd2_chmod_4,ASD2_CHANNEL_MODE_4,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd2_chmod_5,ASD2_CHANNEL_MODE_5,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_chmod_6,ASD2_CHANNEL_MODE_6,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd2_chmod_7,ASD2_CHANNEL_MODE_7,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	basicSetupArray_a3p250[ASD2_CHIP_MODE]=asd2_chip_mode;		
}

*/ 
  
/*
	  
void LoadA3P250SetupArray(void)
{

	basicSetupArray_a3p250[ASD0_CH0]=asd0_ch0;
	basicSetupArray_a3p250[ASD0_CH1]=asd0_ch1;
	basicSetupArray_a3p250[ASD0_CH2]=asd0_ch2;
	basicSetupArray_a3p250[ASD0_CH3]=asd0_ch3;
	basicSetupArray_a3p250[ASD0_CH4]=asd0_ch4;
	basicSetupArray_a3p250[ASD0_CH5]=asd0_ch5;
	basicSetupArray_a3p250[ASD0_CH6]=asd0_ch6;
	basicSetupArray_a3p250[ASD0_CH7]=asd0_ch7;
	IntToBinary(asd0_cal_cap,ASD0_CAL_CAP,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd0_main_thresh,ASD0_MAIN_THRESH,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd0_wil_thresh,ASD0_WIL_THRESH,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd0_hysteresis,ASD0_HYSTERESIS,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd0_wil_int,ASD0_WIL_INT,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd0_wil_cur,ASD0_WIL_CUR,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToBinary(asd0_deadtime,ASD0_DEADTIME,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToBinary(asd0_chmod_0,ASD0_CHANNEL_MODE_0,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd0_chmod_1,ASD0_CHANNEL_MODE_1,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd0_chmod_2,ASD0_CHANNEL_MODE_2,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd0_chmod_3,ASD0_CHANNEL_MODE_3,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd0_chmod_4,ASD0_CHANNEL_MODE_4,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd0_chmod_5,ASD0_CHANNEL_MODE_5,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd0_chmod_6,ASD0_CHANNEL_MODE_6,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd0_chmod_7,ASD0_CHANNEL_MODE_7,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	basicSetupArray_a3p250[ASD0_CHIP_MODE]=asd0_chip_mode;	

	
	
	basicSetupArray_a3p250[ASD1_CH0]=asd1_ch0;
	basicSetupArray_a3p250[ASD1_CH1]=asd1_ch1;
	basicSetupArray_a3p250[ASD1_CH2]=asd1_ch2;
	basicSetupArray_a3p250[ASD1_CH3]=asd1_ch3;
	basicSetupArray_a3p250[ASD1_CH4]=asd1_ch4;
	basicSetupArray_a3p250[ASD1_CH5]=asd1_ch5;
	basicSetupArray_a3p250[ASD1_CH6]=asd1_ch6;
	basicSetupArray_a3p250[ASD1_CH7]=asd1_ch7;
	IntToBinary(asd1_cal_cap,ASD1_CAL_CAP,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd1_main_thresh,ASD1_MAIN_THRESH,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd1_wil_thresh,ASD1_WIL_THRESH,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd1_hysteresis,ASD1_HYSTERESIS,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd1_wil_int,ASD1_WIL_INT,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd1_wil_cur,ASD1_WIL_CUR,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToBinary(asd1_deadtime,ASD1_DEADTIME,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToBinary(asd1_chmod_0,ASD1_CHANNEL_MODE_0,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd1_chmod_1,ASD1_CHANNEL_MODE_1,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd1_chmod_2,ASD1_CHANNEL_MODE_2,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd1_chmod_3,ASD1_CHANNEL_MODE_3,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd1_chmod_4,ASD1_CHANNEL_MODE_4,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd1_chmod_5,ASD1_CHANNEL_MODE_5,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd1_chmod_6,ASD1_CHANNEL_MODE_6,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd1_chmod_7,ASD1_CHANNEL_MODE_7,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	basicSetupArray_a3p250[ASD1_CHIP_MODE]=asd1_chip_mode;	

 	
	
	basicSetupArray_a3p250[ASD2_CH0]=asd2_ch0;
	basicSetupArray_a3p250[ASD2_CH1]=asd2_ch1;
	basicSetupArray_a3p250[ASD2_CH2]=asd2_ch2;
	basicSetupArray_a3p250[ASD2_CH3]=asd2_ch3;
	basicSetupArray_a3p250[ASD2_CH4]=asd2_ch4;
	basicSetupArray_a3p250[ASD2_CH5]=asd2_ch5;
	basicSetupArray_a3p250[ASD2_CH6]=asd2_ch6;
	basicSetupArray_a3p250[ASD2_CH7]=asd2_ch7;
	IntToBinary(asd2_cal_cap,ASD2_CAL_CAP,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd2_main_thresh,ASD2_MAIN_THRESH,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd2_wil_thresh,ASD2_WIL_THRESH,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd2_hysteresis,ASD2_HYSTERESIS,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd2_wil_int,ASD2_WIL_INT,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd2_wil_cur,ASD2_WIL_CUR,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToBinary(asd2_deadtime,ASD2_DEADTIME,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);		
	IntToBinary(asd2_chmod_0,ASD2_CHANNEL_MODE_0,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd2_chmod_1,ASD2_CHANNEL_MODE_1,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd2_chmod_2,ASD2_CHANNEL_MODE_2,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd2_chmod_3,ASD2_CHANNEL_MODE_3,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd2_chmod_4,ASD2_CHANNEL_MODE_4,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd2_chmod_5,ASD2_CHANNEL_MODE_5,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd2_chmod_6,ASD2_CHANNEL_MODE_6,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToBinary(asd2_chmod_7,ASD2_CHANNEL_MODE_7,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	basicSetupArray_a3p250[ASD2_CHIP_MODE]=asd2_chip_mode;		
}

*/

//Load Setup registers



//New ASD chip on mezzanine card.
 /*
void LoadA3P250SetupArray(void)
{
	IntToReversedBinary(asd0_vmon,ASD0_VMON,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_ch_mask_tst_ctrl,ASD0_CH_MASK_TST_CTRL,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_cap_select,ASD0_CAP_SELECT,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_main_thr_dac,ASD0_MAIN_THR_DAC,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_wilk_thr_dac,ASD0_WILK_THR_DAC,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd0_hyst_dac,ASD0_HYST_DAC,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_rund_curr,ASD0_RUND_CURR,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_int_gate,ASD0_INT_GATE,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_deadtime,ASD0_DEADTIME,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	basicSetupArray_a3p250[ASD0_CHIP_MODE]=asd0_chip_mode;
	IntToReversedBinary(asd0_channel_7_mode,ASD0_CHANNEL_7_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_channel_6_mode,ASD0_CHANNEL_6_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_channel_5_mode,ASD0_CHANNEL_5_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_channel_4_mode,ASD0_CHANNEL_4_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd0_channel_3_mode,ASD0_CHANNEL_3_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_channel_2_mode,ASD0_CHANNEL_2_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_channel_1_mode,ASD0_CHANNEL_1_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd0_channel_0_mode,ASD0_CHANNEL_1_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	
	
	
	IntToReversedBinary(asd1_vmon,ASD1_VMON,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_ch_mask_tst_ctrl,ASD1_CH_MASK_TST_CTRL,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_cap_select,ASD1_CAP_SELECT,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_main_thr_dac,ASD1_MAIN_THR_DAC,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_wilk_thr_dac,ASD1_WILK_THR_DAC,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd1_hyst_dac,ASD1_HYST_DAC,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_rund_curr,ASD1_RUND_CURR,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_int_gate,ASD1_INT_GATE,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_deadtime,ASD1_DEADTIME,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	basicSetupArray_a3p250[ASD1_CHIP_MODE]=asd1_chip_mode;
	IntToReversedBinary(asd1_channel_7_mode,ASD1_CHANNEL_7_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_channel_6_mode,ASD1_CHANNEL_6_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_channel_5_mode,ASD1_CHANNEL_5_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_channel_4_mode,ASD1_CHANNEL_4_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd1_channel_3_mode,ASD1_CHANNEL_3_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_channel_2_mode,ASD1_CHANNEL_2_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_channel_1_mode,ASD1_CHANNEL_1_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd1_channel_0_mode,ASD1_CHANNEL_1_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	
	
	IntToReversedBinary(asd2_vmon,ASD2_VMON,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_ch_mask_tst_ctrl,ASD2_CH_MASK_TST_CTRL,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_cap_select,ASD2_CAP_SELECT,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_main_thr_dac,ASD2_MAIN_THR_DAC,8,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_wilk_thr_dac,ASD2_WILK_THR_DAC,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToBinary(asd2_hyst_dac,ASD2_HYST_DAC,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_rund_curr,ASD2_RUND_CURR,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_int_gate,ASD2_INT_GATE,4,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_deadtime,ASD2_DEADTIME,3,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	basicSetupArray_a3p250[ASD2_CHIP_MODE]=asd2_chip_mode;
	IntToReversedBinary(asd2_channel_7_mode,ASD2_CHANNEL_7_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_channel_6_mode,ASD2_CHANNEL_6_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_channel_5_mode,ASD2_CHANNEL_5_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_channel_4_mode,ASD2_CHANNEL_4_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	
	IntToReversedBinary(asd2_channel_3_mode,ASD2_CHANNEL_3_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_channel_2_mode,ASD2_CHANNEL_2_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_channel_1_mode,ASD2_CHANNEL_1_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);
	IntToReversedBinary(asd2_channel_0_mode,ASD2_CHANNEL_1_MODE,2,basicSetupArray_a3p250, A3P250SETUP_LENGTH);	

}

 */
 

 
void LoadTDCSetup0Array(void){
  IntToReversedBinary(TDC_ENABLE_NEW_TTC,TDC_ENABLE_NEW_TTC_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_MASTER_RESET_CODE,TDC_ENABLE_MASTER_RESET_CODE_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_DIRECT_BUNCH_RESET,TDC_ENABLE_DIRECT_BUNCH_RESET_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_DIRECT_EVENT_RESET,TDC_ENABLE_DIRECT_EVENT_RESET_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_DIRECT_TRIGGER,TDC_ENABLE_DIRECT_TRIGGER_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_AUTO_ROLL_OVER,TDC_ENABLE_AUTO_ROLL_OVER_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_BYPASS_BCR_DISTRIBUTION,TDC_BYPASS_BCR_DISTRIBUTION_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_TRIGGER,TDC_ENABLE_TRIGGER_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_CHANNEL_DATA_DEBUG,TDC_CHANNEL_DATA_DEBUG_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_LEADING,TDC_ENABLE_LEADING_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_PAIR,TDC_ENABLE_PAIR_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_FAKE_HIT,TDC_ENABLE_FAKE_HIT_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_RISING_IS_LEADING,TDC_RISING_IS_LEADING_INDEX,24,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_CHANNEL_ENABLE_R,TDC_CHANNEL_ENABLE_R_INDEX,24,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_CHANNEL_ENABLE_F,TDC_CHANNEL_ENABLE_F_INDEX,24,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_INNER_ID,TDC_INNER_ID_INDEX,19,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_TRIGGER_TIMEOUT,TDC_ENABLE_TRIGGER_TIMEOUT_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_HIGH_SPEED,TDC_ENABLE_HIGH_SPEED_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_LEGACY,TDC_ENABLE_LEGACY_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_FULL_WIDTH_RES,TDC_FULL_WIDTH_RES_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_WIDTH_SELECT,TDC_WIDTH_SELECT_INDEX,3,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_8B10B,TDC_ENABLE_8B10B_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_INSERT,TDC_ENABLE_INSERT_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_ERROR_PACKET,TDC_ENABLE_ERROR_PACKET_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_ID,TDC_ENABLE_ID_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ENABLE_ERROR_NOTIFY,TDC_ENABLE_ERROR_NOTIFY_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
} 
void LoadTDCSetup1Array(void){
  IntToReversedBinary(TDC_COMBINE_TIME_OUT_CONFIG,TDC_COMBINE_TIME_OUT_CONFIG_INDEX,10,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_FAKE_HIT_TIME_INTERVAL,TDC_FAKE_HIT_TIME_INTERVAL_INDEX,12,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_SYN_PACKET_NUMBER,TDC_SYN_PACKET_NUMBER_INDEX,12,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_ROLL_OVER,TDC_ROLL_OVER_INDEX,12,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_COARSE_COUNT_OFFSET,TDC_COARSE_COUNT_OFFSET_INDEX,12,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_BUCH_OFFSET,TDC_BUCH_OFFSET_INDEX,12,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EVENT_OFFSET,TDC_EVENT_OFFSET_INDEX,12,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_MATCH_WINDOW,TDC_MATCH_WINDOW_INDEX,12,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
}
void LoadTDCSetup2Array(void){
  IntToReversedBinary(TDC_FINE_SEL,TDC_FINE_SEL_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_LUT_INDEX,TDC_LUT_INDEX,32,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
}
void LoadTDCControl0Array(void){
  IntToReversedBinary(TDC_RESET_EPLL,TDC_RESET_EPLL_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_RESET_JTAG_IN,TDC_RESET_JTAG_IN_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EVENT_RESET_JTAG_IN,TDC_EVENT_RESET_JTAG_IN_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_CHNL_FIFO_OVERFLOW_CLEAR,TDC_CHNL_FIFO_OVERFLOW_CLEAR_INDEX,1,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_DEBUG_PORT_SELECT,TDC_DEBUG_PORT_SELECT_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
}
void LoadTDCControl1Array(void){
  IntToReversedBinary(TDC_PHASE_CLK160,TDC_PHASE_CLK160_INDEX,5,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_PHASE_CLK320_0,TDC_PHASE_CLK320_0_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_PHASE_CLK320_1,TDC_PHASE_CLK320_1_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_PHASE_CLK320_2,TDC_PHASE_CLK320_2_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EPLL_RES_A,TDC_EPLL_RES_A_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EPLL_ICP_A,TDC_EPLL_ICP_A_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EPLL_CAP_A,TDC_EPLL_CAP_A_INDEX,2,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EPLL_RES_B,TDC_EPLL_RES_B_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EPLL_ICP_B,TDC_EPLL_ICP_B_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EPLL_CAP_B,TDC_EPLL_CAP_B_INDEX,2,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EPLL_RES_C,TDC_EPLL_RES_C_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EPLL_ICP_C,TDC_EPLL_ICP_C_INDEX,4,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(TDC_EPLL_CAP_C,TDC_EPLL_CAP_C_INDEX,2,TDC_setup_array,TDC_SETUP_MAX_LENGTH);
}

void LoadNewASDSetupArray(void){
  IntToReversedBinary(asd0_vmon,ASD0_VMON,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_ch_mask_tst_ctrl,ASD0_CH_MASK_TST_CTRL,8,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_cap_select,ASD0_CAP_SELECT,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_main_thr_dac,ASD0_MAIN_THR_DAC,8,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_wilk_thr_dac,ASD0_WILK_THR_DAC,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToBinary(asd0_hyst_dac,ASD0_HYST_DAC,4,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_rund_curr,ASD0_RUND_CURR,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_int_gate,ASD0_INT_GATE,4,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_deadtime,ASD0_DEADTIME,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  TDC_setup_array[ASD0_CHIP_MODE]=asd0_chip_mode;
  IntToReversedBinary(asd0_channel_7_mode,ASD0_CHANNEL_7_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_channel_6_mode,ASD0_CHANNEL_6_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_channel_5_mode,ASD0_CHANNEL_5_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_channel_4_mode,ASD0_CHANNEL_4_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);  
  IntToReversedBinary(asd0_channel_3_mode,ASD0_CHANNEL_3_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_channel_2_mode,ASD0_CHANNEL_2_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_channel_1_mode,ASD0_CHANNEL_1_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd0_channel_0_mode,ASD0_CHANNEL_1_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);   
  
  IntToReversedBinary(asd1_vmon,ASD1_VMON,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_ch_mask_tst_ctrl,ASD1_CH_MASK_TST_CTRL,8,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_cap_select,ASD1_CAP_SELECT,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_main_thr_dac,ASD1_MAIN_THR_DAC,8,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_wilk_thr_dac,ASD1_WILK_THR_DAC,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToBinary(asd1_hyst_dac,ASD1_HYST_DAC,4,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_rund_curr,ASD1_RUND_CURR,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_int_gate,ASD1_INT_GATE,4,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_deadtime,ASD1_DEADTIME,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  TDC_setup_array[ASD1_CHIP_MODE]=asd1_chip_mode;
  IntToReversedBinary(asd1_channel_7_mode,ASD1_CHANNEL_7_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_channel_6_mode,ASD1_CHANNEL_6_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_channel_5_mode,ASD1_CHANNEL_5_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_channel_4_mode,ASD1_CHANNEL_4_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);  
  IntToReversedBinary(asd1_channel_3_mode,ASD1_CHANNEL_3_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_channel_2_mode,ASD1_CHANNEL_2_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_channel_1_mode,ASD1_CHANNEL_1_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd1_channel_0_mode,ASD1_CHANNEL_1_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);  
   
  IntToReversedBinary(asd2_vmon,ASD2_VMON,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_ch_mask_tst_ctrl,ASD2_CH_MASK_TST_CTRL,8,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_cap_select,ASD2_CAP_SELECT,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_main_thr_dac,ASD2_MAIN_THR_DAC,8,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_wilk_thr_dac,ASD2_WILK_THR_DAC,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToBinary(asd2_hyst_dac,ASD2_HYST_DAC,4,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_rund_curr,ASD2_RUND_CURR,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_int_gate,ASD2_INT_GATE,4,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_deadtime,ASD2_DEADTIME,3,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  TDC_setup_array[ASD2_CHIP_MODE]=asd2_chip_mode;
  IntToReversedBinary(asd2_channel_7_mode,ASD2_CHANNEL_7_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_channel_6_mode,ASD2_CHANNEL_6_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_channel_5_mode,ASD2_CHANNEL_5_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_channel_4_mode,ASD2_CHANNEL_4_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);  
  IntToReversedBinary(asd2_channel_3_mode,ASD2_CHANNEL_3_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_channel_2_mode,ASD2_CHANNEL_2_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_channel_1_mode,ASD2_CHANNEL_1_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
  IntToReversedBinary(asd2_channel_0_mode,ASD2_CHANNEL_1_MODE,2,TDC_setup_array, TDC_SETUP_MAX_LENGTH);
}










 /*

void LoadHPTDCSetupArray(void)
{
	IntToBinary(hptdc_test_select,TEST_SELECT,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	basicSetupArray_h[ENABLE_ERROR_MARK]=hptdc_enable_error_mark;
	basicSetupArray_h[ENABLE_ERROR_BYPASS]=hptdc_enable_error_bypass;
	IntToBinary(hptdc_enable_error,ENABLE_ERROR,11,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_readout_sincyspeed,READOUT_SINGLE_CYCLE_SPEED,3,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_serial_delay,SERIAL_DELAY,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_strobe_select,STROBE_SELECT,2,basicSetupArray_h, HPTDCSETUP_LENGTH);
	basicSetupArray_h[READOUT_SPEED_SELECT]=hptdc_readout_speed_select;
	IntToBinary(hptdc_token_delay,TOKEN_DELAY,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	basicSetupArray_h[ENABLE_LOCAL_TRAILER]=hptdc_enable_local_trailer;
	basicSetupArray_h[ENABLE_LOCAL_HEADER]=hptdc_enable_local_header;
	basicSetupArray_h[ENABLE_GLOBAL_TRAILER]=hptdc_enable_global_trailer;
	basicSetupArray_h[ENABLE_GLOBAL_HEADER]=hptdc_enable_global_header;
	basicSetupArray_h[KEEP_TOKEN]=hptdc_keep_token;
	basicSetupArray_h[MASTER]=hptdc_master;
	basicSetupArray_h[ENABLE_BYTEWISE]=hptdc_enable_bytewise;
	basicSetupArray_h[ENABLE_SERIAL]=hptdc_enable_serial;
	basicSetupArray_h[ENABLE_JTAG_READOUT]=hptdc_enable_jtag_readout;
	IntToBinary(hptdc_tdc_id ,TDC_ID,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	basicSetupArray_h[SELECT_BYPASS_INPUTS]=hptdc_select_bypass_inputs;
	IntToBinary(hptdc_readout_fifo_size,READOUT_FIFO_SIZE,3,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_reject_count_offset,REJECT_COUNT_OFFSET,12,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_search_window ,SEARCH_WINDOW,12,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_match_window,MATCH_WINDOW,12,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_leading_resolution,LEADING_RESOLUTION,3,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_fixed_pattern,FIXED_PATTERN,28,basicSetupArray_h, HPTDCSETUP_LENGTH);
	basicSetupArray_h[ENABLE_FIXED_PATTERN]=hptdc_enable_fixed_pattern;
	IntToBinary(hptdc_max_event_size,MAX_EVENT_SIZE,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	basicSetupArray_h[REJECT_READOUT_FIFO_FULL]=hptdc_reject_readout_fifo_full;
	basicSetupArray_h[ENABLE_READOUT_OCCUPANCY]=hptdc_enable_readout_occupancy;
	basicSetupArray_h[ENABLE_READOUT_SEPARATOR]=hptdc_enable_readout_separator;
	basicSetupArray_h[ENABLE_OVERFLOW_DETECT]=hptdc_enable_overflow_detect;
	basicSetupArray_h[ENABLE_RELATIVE]=hptdc_enable_relative;
	basicSetupArray_h[ENABLE_AUTOMATIC_REJECT]=hptdc_enable_automatic_reject;
	IntToBinary(hptdc_event_count_offset,EVENT_COUNT_OFFSET,12,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_trigger_count_offset,TRIGGER_COUNT_OFFSET,12,basicSetupArray_h, HPTDCSETUP_LENGTH);
	basicSetupArray_h[ENABLE_SET_COUNTERS_ON_BUNCH_RESET]=hptdc_enable_set_counbunres;
	basicSetupArray_h[ENABLE_MASTER_RESET_CODE]=hptdc_enable_master_reset_code;
	basicSetupArray_h[ENABLE_MASTER_RESET_ON_EVENT_RESET]=hptdc_enable_mast_res_evnt_res;
	basicSetupArray_h[ENABLE_RESET_CHANNEL_BUFFER_WHENSEPARATOR]=hptdc_enable_res_chn_buf_sepa;
	basicSetupArray_h[ENABLE_SEPARATOR_ON_EVENT_RESET]=hptdc_enable_sepa_on_evnt_res;
	basicSetupArray_h[ENABLE_SEPARATOR_ON_BUNCH_RESET]=hptdc_enable_sepa_on_bnch_res;
	basicSetupArray_h[ENABLE_DIRECT_EVENT_RESET]=hptdc_enable_direct_evnt_res;
	basicSetupArray_h[ENABLE_DIRECT_BUNCH_RESET]=hptdc_enable_direct_bnch_res;
	basicSetupArray_h[ENABLE_DIRECT_TRIGGER]=hptdc_enable_direct_trigger;
	IntToBinary(hptdc_offset31,OFFSET31,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset30,OFFSET30,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset29,OFFSET29,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset28,OFFSET28,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset27,OFFSET27,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset26,OFFSET26,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset25,OFFSET25,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset24,OFFSET24,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset23,OFFSET23,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset22,OFFSET22,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset21,OFFSET21,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset20,OFFSET20,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset19,OFFSET19,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset18,OFFSET18,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset17,OFFSET17,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset16,OFFSET16,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset15,OFFSET15,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset14,OFFSET14,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset13,OFFSET13,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset12,OFFSET12,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset11,OFFSET11,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset10,OFFSET10,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset9,OFFSET9,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset8,OFFSET8,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset7,OFFSET7,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset6,OFFSET6,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset5,OFFSET5,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset4,OFFSET4,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset3,OFFSET3,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset2,OFFSET2,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset1,OFFSET1,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_offset0,OFFSET0,9,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_coarse_count_offset,COARSE_COUNT_OFFSET,12,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_dll_tap_adjust,DLL_TAP_ADJUST,96,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_rc_adjust,RC_ADJUST,12,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_not_used_567,NOT_USED_567,12,basicSetupArray_h, HPTDCSETUP_LENGTH);
	basicSetupArray_h[LOW_POWER_MODE]=hptdc_low_power_mode;
	IntToBinary(hptdc_width_select,WIDTH_SELECT,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_vernier_offset,VERNIER_OFFSET,5,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_dll_control,DLL_CONTROL,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_dead_time,DEAD_TIME,2,basicSetupArray_h, HPTDCSETUP_LENGTH);
	basicSetupArray_h[TEST_INVERT]=hptdc_test_invert;
	basicSetupArray_h[TEST_MODE]=hptdc_test_mode;
	basicSetupArray_h[ENABLE_TRAILING]=hptdc_enable_trailing;
	basicSetupArray_h[ENABLE_LEADING]=hptdc_enable_leading;
	basicSetupArray_h[MODE_RC_COMPRESSION]=hptdc_mode_rc_compression;
	basicSetupArray_h[MODE_RC]=hptdc_mode_rc;
	IntToBinary(hptdc_dll_mode,DLL_MODE,2,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_pll_control,PLL_CONTROL,8,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_serial_clock_delay,SERIAL_CLOCK_DELAY,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_io_clock_delay,IO_CLOCK_DELAY,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_core_clock_delay,CORE_CLOCK_DELAY,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_dll_clock_delay,DLL_CLOCK_DELAY,4,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_serial_clock_source,SERIAL_CLOCK_SOURCE,2,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_io_clock_source,IO_CLOCK_SOURCE,2,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_core_clock_source,CORE_CLOCK_SOURCE,2,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_dll_clock_source,DLL_CLOCK_SOURCE,3,basicSetupArray_h, HPTDCSETUP_LENGTH);
	IntToBinary(hptdc_roll_over,ROLL_OVER,12,basicSetupArray_h, HPTDCSETUP_LENGTH);
	basicSetupArray_h[ENABLE_MATCHING]=hptdc_enable_matching;
	basicSetupArray_h[ENABLE_PAIR]=hptdc_enable_pair;
	basicSetupArray_h[ENABLE_TTL_SERIAL]=hptdc_enable_ttl_serial;
	basicSetupArray_h[ENABLE_TTL_CONTROL]=hptdc_enable_ttl_control;
	basicSetupArray_h[ENABLE_TTL_RESET]=hptdc_enable_ttl_reset;
	basicSetupArray_h[ENABLE_TTL_CLOCK]=hptdc_enable_ttl_clock;
	basicSetupArray_h[ENABLE_TTL_HIT]=hptdc_enable_ttl_hit;
	basicSetupArray_h[SETUP_PARITY]=hptdc_setup_parity;
}

void LoadHPTDCControlArray_step1(void)
{
	IntToBinary(4,ENABLE_PATTERN,4,basicControlArray_h1, HPTDCCONTROL_LENGTH);
	basicControlArray_h1[GLOBLE_RESET]=0;
	IntToBinary(hptdc_enable_channel,ENABLE_CHANNEL,32,basicControlArray_h1, HPTDCCONTROL_LENGTH);
	basicControlArray_h1[DLL_RESET]=1;
	basicControlArray_h1[PLL_RESET]=1;
	basicControlArray_h1[CONTROL_PARITY]=1;
}

void LoadHPTDCControlArray_step2(void)
{
	IntToBinary(4,ENABLE_PATTERN,4,basicControlArray_h2, HPTDCCONTROL_LENGTH);
	basicControlArray_h2[GLOBLE_RESET]=0;
	IntToBinary(hptdc_enable_channel,ENABLE_CHANNEL,32,basicControlArray_h2, HPTDCCONTROL_LENGTH);
	basicControlArray_h2[DLL_RESET]=1;
	basicControlArray_h2[PLL_RESET]=0;
	basicControlArray_h2[CONTROL_PARITY]=0;
}

void LoadHPTDCControlArray_step3(void)
{
	IntToBinary(4,ENABLE_PATTERN,4,basicControlArray_h3, HPTDCCONTROL_LENGTH);
	basicControlArray_h3[GLOBLE_RESET]=0;
	IntToBinary(hptdc_enable_channel,ENABLE_CHANNEL,32,basicControlArray_h3, HPTDCCONTROL_LENGTH);
	basicControlArray_h3[DLL_RESET]=0;
	basicControlArray_h3[PLL_RESET]=0;
	basicControlArray_h3[CONTROL_PARITY]=1;
}
void LoadHPTDCControlArray_step4(void)
{
	IntToBinary(4,ENABLE_PATTERN,4,basicControlArray_h4, HPTDCCONTROL_LENGTH);
	basicControlArray_h4[GLOBLE_RESET]=1;
	IntToBinary(hptdc_enable_channel,ENABLE_CHANNEL,32,basicControlArray_h4, HPTDCCONTROL_LENGTH);
	basicControlArray_h4[DLL_RESET]=0;
	basicControlArray_h4[PLL_RESET]=0;
	basicControlArray_h4[CONTROL_PARITY]=0;
}

void LoadHPTDCControlArray_step5(void)
{
	IntToBinary(4,ENABLE_PATTERN,4,basicControlArray_h5, HPTDCCONTROL_LENGTH);
	basicControlArray_h5[GLOBLE_RESET]=0;
	IntToBinary(hptdc_enable_channel,ENABLE_CHANNEL,32,basicControlArray_h5, HPTDCCONTROL_LENGTH);
	basicControlArray_h5[DLL_RESET]=0;
	basicControlArray_h5[PLL_RESET]=0;
	basicControlArray_h5[CONTROL_PARITY]=1;
}

void LoadHPTDCControlArray_step6(void)
{
	IntToBinary(5,ENABLE_PATTERN,4,basicControlArray_h6, HPTDCCONTROL_LENGTH);
	basicControlArray_h6[GLOBLE_RESET]=0;
	IntToBinary(hptdc_enable_channel,ENABLE_CHANNEL,32,basicControlArray_h6, HPTDCCONTROL_LENGTH);
	basicControlArray_h6[DLL_RESET]=0;
	basicControlArray_h6[PLL_RESET]=0;
	basicControlArray_h6[CONTROL_PARITY]=0;
}

//End
 */

void RecallMezzanineSetup(void) {
  int runType, ASDNumber, savedSelectedASD;
  
  GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_RUNTYPE, &runType);
  savedSelectedASD = selectedASD;
  PanelRecall(AMTSetupHandle);
  PanelRecall(AMTErrorHandle);
  for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
    selectedASD = ASDNumber;
    PanelRecall(ASDSetupHandle);
    if (runType == NORMALALLCALIBCHANNELOFF) ASDAllChannelOff();
    else if (runType == CALIBALLCALIBCHANNELON) ASDAllChannelOn();
    SavePanelState(ASDSetupHandle, ASDTmpSetupFile, ASDNumber);
    LoadASDSetupArray(ASDNumber);
  }
  selectedASD = savedSelectedASD;
  if (mezzCardSetupAll == 1) {
    printf("Recall common mezzanine card setttings.\n");
    SetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, nbMezzCard);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MIN_VALUE, 1);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MAX_VALUE, MAXNUMBERMEZZANINE);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_TEXT, "# of MezzCard");
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_LEFT, VAL_AUTO_CENTER);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_CTRL_MODE, VAL_INDICATOR);
  }
  else {
    printf("Recall mezzanine card %d setttings.\n", mezzCardNb);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MIN_VALUE, 0);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MAX_VALUE, MAXNUMBERMEZZANINE-1);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_TEXT, "MezzCard ID");
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_LEFT, VAL_AUTO_CENTER);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_CTRL_MODE, VAL_HOT);
    SetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, mezzCardNb);
  }
  if (nbMezzCard > 1) 
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_COPYSETUP, ATTR_DIMMED, FALSE);
  else
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_COPYSETUP, ATTR_DIMMED, TRUE);
  DisablePair();
  AMTStrobeSelection();
  LoadAMTSetupArray();
}


// Bring up copy mezzanine card setup panel
void CopyMezzSetupControl(void) {
  int mezz, dimmed, item, nlist;
  char str[20];
  
  GetNumListItems(CopyMezzSetupHandle, P_COPYMEZZ_FROM, &nlist);
  DeleteListItem(CopyMezzSetupHandle, P_COPYMEZZ_FROM, 0, nlist);
  item = 0;
  strcpy(str, "Current Setup");
  InsertListItem(CopyMezzSetupHandle, P_COPYMEZZ_FROM, item++, str, -1);
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if ((mezzEnables&(1<<mezz)) != 0) {
      dimmed = FALSE;
      sprintf(str, "Mezz %d", mezz);
      InsertListItem(CopyMezzSetupHandle, P_COPYMEZZ_FROM, item++, str, mezz);
    }
    else dimmed = TRUE;
    if (mezzCardSetupAll == 0 && mezzCardNb == mezz) {
      dimmed = TRUE;
      item--;
      DeleteListItem(CopyMezzSetupHandle, P_COPYMEZZ_FROM, item, 1);
    }
    SetCtrlAttribute(CopyMezzSetupHandle, mezzButtonForCopy[mezz], ATTR_DIMMED, dimmed);
    if (dimmed == TRUE) SetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], FALSE);
  }
  if (mezzCardSetupAll == 0) {
    strcpy(str, "All Setup");
    InsertListItem(CopyMezzSetupHandle, P_COPYMEZZ_FROM, item++, str, MAXNUMBERMEZZANINE);
    SetCtrlAttribute(CopyMezzSetupHandle, mezzButtonForCopy[MAXNUMBERMEZZANINE], ATTR_DIMMED, FALSE);
  }
  else {  // Copy to itself is not allowed
    mezz = MAXNUMBERMEZZANINE;
    SetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], FALSE);
    SetCtrlAttribute(CopyMezzSetupHandle, mezzButtonForCopy[mezz], ATTR_DIMMED, TRUE);
  }
  SetCtrlIndex(CopyMezzSetupHandle, P_COPYMEZZ_FROM, 0);
  HidePanel(AMTSetupHandle);
  DisplayPanel(CopyMezzSetupHandle);
  SetActivePanel(CopyMezzSetupHandle);
}


// Copy Mezzanine card setup to selected mezzanine cards
void CopyMezzSetup(void) {
  int mezz, numberToBeCopied = 0, yes, index, ASDNumber, beCopiedMezz, keepOldValue;
  int i, savedSetupAll, savedTDCNo, copyThreshold, copyAMTChannel, copyASDChannel;
  
  // First save current setup panels into disk
  SavePanelState(ASDSetupHandle, "MezzSetup_temp", 0);
  for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
    if (ASDNumber == selectedASD) RecallPanelState(ASDSetupHandle, "MezzSetup_temp", 0);
    else RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, ASDNumber);
    SavePanelState(ASDSetupHandle, "MezzSetup_temp", ASDNumber);
    RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, ASDNumber);
    SavePanelState(ASDSetupHandle, "ASDSetup_temp", ASDNumber);
    LoadASDSetupArray(ASDNumber);
  }
  SavePanelState(AMTSetupHandle, "MezzSetup_temp", MAXNUMBERASD+1);
  SavePanelState(AMTErrorHandle, "MezzSetup_temp", MAXNUMBERASD+2);
  LoadAMTSetupArray();
  GetCtrlVal(CopyMezzSetupHandle, P_COPYMEZZ_FROM, &beCopiedMezz);
  if (beCopiedMezz < 0) {
    if (mezzCardSetupAll == 1) mezz = MAXNUMBERMEZZANINE;
    else mezz = mezzCardNb;
  }
  else {
    mezz = beCopiedMezz;
    if (mezzCardSetupAll == 0)
      GetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezzCardNb], &yes);
    else
      GetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[MAXNUMBERMEZZANINE], &yes);
    if (yes == 1)
      MessagePopup("WARNING",
                   "Will copy selected mezz.card setup as DEFAULT setting of current mezz.card.\n"
                   "The settings value of current setup panel will not be effected!\n"
                   "If want copied the value, exit from current setup panel and redo it.\n");
  }
  for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++)
    mezzSetupArray[i][mezz] = basicSetupArray[i];
  // Disable command buttons
  SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_DONE, ATTR_DIMMED, TRUE);
  SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_STARTCOPY, ATTR_DIMMED, TRUE);
  SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_SELECTALL, ATTR_DIMMED, TRUE);
  SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_UNSELECTALL, ATTR_DIMMED, TRUE);
  // Save parameters
  savedSetupAll = mezzCardSetupAll;
  savedTDCNo = mezzCardNb;
  GetCtrlVal(CopyMezzSetupHandle, P_COPYMEZZ_MAINTHRESHOLD, &copyThreshold);
  GetCtrlVal(CopyMezzSetupHandle, P_COPYMEZZ_AMTCHANNEL, &copyAMTChannel);
  GetCtrlVal(CopyMezzSetupHandle, P_COPYMEZZ_ASDCHANNEL, &copyASDChannel);
  keepOldValue = copyThreshold & copyAMTChannel & copyASDChannel;
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (mezzButtonForCopy[mezz] >= 0)
      GetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], &yes);
    if (yes == 1) {
      if (keepOldValue == 0) {
        printf("Recall mezzanine card %d setup for future usage.\n", mezz);
        mezzCardSetupAll = 0;
        mezzCardNb = mezz;
        RecallMezzanineSetup();
        for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++)
          mezzSetupArray[i][mezz] = basicSetupArray[i];
      }
      numberToBeCopied++;
    }
  }
  mezz = MAXNUMBERMEZZANINE;
  GetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], &yes);
  if (yes == 1) {
    if (keepOldValue == 0) {
      printf("Recall mezzanine card common settings for future usage.\n");
      mezzCardSetupAll = 1;
      mezzCardNb = mezz;
      RecallMezzanineSetup();
      for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++)
        mezzSetupArray[i][mezz] = basicSetupArray[i];
    }
    numberToBeCopied++;
  }
  if (numberToBeCopied <= 0) {   // Nothing need to be copied
    RecallPanelState(ASDSetupHandle, "MezzSetup_temp", 0);
    RecallPanelState(AMTSetupHandle, "MezzSetup_temp", MAXNUMBERASD+1);
    RecallPanelState(AMTErrorHandle, "MezzSetup_temp", MAXNUMBERASD+2);
    // Enable command buttons
    SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_DONE, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_STARTCOPY, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_SELECTALL, ATTR_DIMMED, FALSE);
    SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_UNSELECTALL, ATTR_DIMMED, FALSE);
    return;
  }
  // Now let's start to copy mezzanine card setup
  printf("Copy mezzanine card setup to selected destinations, it takes a while ...\n");
  //needSetupSend = 1;    // Definitly need to do SetupSend
  if (beCopiedMezz >= 0) {
    if (beCopiedMezz == MAXNUMBERMEZZANINE) mezzCardSetupAll = 1;
    else mezzCardSetupAll = 0;
    mezzCardNb = beCopiedMezz;
    RecallMezzanineSetup();
    for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
      RecallPanelState(ASDSetupHandle, ASDTmpSetupFile, ASDNumber);
      SavePanelState(ASDSetupHandle, "ASDSetup_temp", ASDNumber);
    }
  }
  else {
    RecallPanelState(ASDSetupHandle, "MezzSetup_temp", 0);
    RecallPanelState(AMTSetupHandle, "MezzSetup_temp", MAXNUMBERASD+1);
    RecallPanelState(AMTErrorHandle, "MezzSetup_temp", MAXNUMBERASD+2);
  }
  // Now let's start to copy mezzanine card setup
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (mezzButtonForCopy[mezz] < 0) yes = 0;
    else GetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], &yes);
    if (yes == 1) {
      printf("Copy mezzanine card setup to mezzanine card %d!\n", mezz);
      SetCtrlVal(IndMezzEnableHandle, mezzButtonID[mezz], TRUE);
      SetCtrlVal(CopyMezzSetupHandle, P_COPYMEZZ_COPYINGTOMEZZ, mezz);
      SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_COPYINGTOMEZZ, ATTR_TEXT_BGCOLOR, VAL_GREEN);
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MIN_VALUE, 0);
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MAX_VALUE, MAXNUMBERMEZZANINE-1);
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_TEXT, "MezzCard ID");
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_LEFT, VAL_AUTO_CENTER);
      SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_CTRL_MODE, VAL_HOT);
      SetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, mezz);
      if (copyAMTChannel == 0) SetMezzanineAMTChannel(mezz);
      index = PanelIndexInFile(AMTSetupHandle, 0, mezz, 0);
      SavePanelState(AMTSetupHandle, currSetupFile, index);
      index = PanelIndexInFile(AMTErrorHandle, 0, mezz, 0);
      SavePanelState(AMTErrorHandle, currSetupFile, index);
      for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
        RecallPanelState(ASDSetupHandle, "MezzSetup_temp", ASDNumber);
        if (copyASDChannel == 0) SetMezzanineASDChannel(mezz, ASDNumber);
        if (copyThreshold == 0) SetMezzanineThreshold(mezz, ASDNumber);
        index = PanelIndexInFile(ASDSetupHandle, 0, mezz, ASDNumber);
        SavePanelState(ASDSetupHandle, currSetupFile, index);
      }
    }
  }
  SetCtrlVal(CopyMezzSetupHandle, P_COPYMEZZ_COPYINGTOMEZZ, -1);
  SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_COPYINGTOMEZZ, ATTR_TEXT_BGCOLOR, VAL_MAGENTA);
  mezz = MAXNUMBERMEZZANINE;
  GetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], &yes);
  if (yes == 1) {
    printf("Copy mezzanine card setup to common setup array.\n");
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MIN_VALUE, 1);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_MAX_VALUE, MAXNUMBERMEZZANINE);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_TEXT, "# of MezzCard");
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_LABEL_LEFT, VAL_AUTO_CENTER);
    SetCtrlAttribute(AMTSetupHandle, P_AMTSETUP_MEZZCARD, ATTR_CTRL_MODE, VAL_INDICATOR);
    SetCtrlVal(AMTSetupHandle, P_AMTSETUP_MEZZCARD, nbMezzCard); 
    GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL0, &mezzSetupArray[AMTS_SETUP+AMTS_CH0][mezzCardNb]);
    GetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL7, &mezzSetupArray[AMTS_SETUP+AMTS_CH7][mezzCardNb]);
    if (copyAMTChannel == 0) SetMezzanineAMTChannel(mezz);
    index = PanelIndexInFile(AMTSetupHandle, 1, 0, 0);
    SavePanelState(AMTSetupHandle, currSetupFile, index);
    index = PanelIndexInFile(AMTErrorHandle, 1, 0, 0);
    SavePanelState(AMTErrorHandle, currSetupFile, index);
    for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
      RecallPanelState(ASDSetupHandle, "MezzSetup_temp", ASDNumber);
      if (copyASDChannel == 0) SetMezzanineASDChannel(mezz, ASDNumber);
      if (copyThreshold ==0) SetMezzanineThreshold(mezz, ASDNumber);
      index = PanelIndexInFile(ASDSetupHandle, 1, 0, ASDNumber);
      SavePanelState(ASDSetupHandle, currSetupFile, index);
    }
  }
  // Recall saved setup panels
  for (ASDNumber = 1; ASDNumber < MAXNUMBERASD+1; ASDNumber++) {
    RecallPanelState(ASDSetupHandle, "ASDSetup_temp", ASDNumber);
    SavePanelState(ASDSetupHandle, ASDTmpSetupFile, ASDNumber);
  }
  RecallPanelState(ASDSetupHandle, "MezzSetup_temp", 0);
  RecallPanelState(AMTSetupHandle, "MezzSetup_temp", MAXNUMBERASD+1);
  RecallPanelState(AMTErrorHandle, "MezzSetup_temp", MAXNUMBERASD+2);
  // Enable command buttons
  SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_DONE, ATTR_DIMMED, FALSE);
  SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_STARTCOPY, ATTR_DIMMED, FALSE);
  SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_SELECTALL, ATTR_DIMMED, FALSE);
  SetCtrlAttribute(CopyMezzSetupHandle, P_COPYMEZZ_UNSELECTALL, ATTR_DIMMED, FALSE);
  mezzCardSetupAll = savedSetupAll;
  mezzCardNb = savedTDCNo;
  printf("Copy mezzanine card setup done!\n\n");
}


// Hide copy mezzanine card setup panel
void CopyMezzSetupDone(void) {
  HidePanel(CopyMezzSetupHandle);
  DisplayPanel(AMTSetupHandle);
  SetActivePanel(AMTSetupHandle);
}


// Select which setup should be used to be copied
void CopyMezzSetupFrom(void) {
  int mezz, dimmed, beCopiedMezz;
  
  GetCtrlVal(CopyMezzSetupHandle, P_COPYMEZZ_FROM, &beCopiedMezz);
  if (beCopiedMezz == -1) {
    if (mezzCardSetupAll == 0) beCopiedMezz = mezzCardNb;
    else beCopiedMezz = MAXNUMBERMEZZANINE;
  }
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    if (mezz < nbMezzCard) dimmed = FALSE;
    else dimmed = TRUE;
    if (beCopiedMezz == mezz) dimmed = TRUE;
    SetCtrlAttribute(CopyMezzSetupHandle, mezzButtonForCopy[mezz], ATTR_DIMMED, dimmed);
    if (dimmed == TRUE) SetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], FALSE);
  }
  mezz = MAXNUMBERMEZZANINE;
  if (beCopiedMezz == mezz) dimmed = TRUE;
  else dimmed = FALSE;
  SetCtrlAttribute(CopyMezzSetupHandle, mezzButtonForCopy[mezz], ATTR_DIMMED, dimmed);
  if (dimmed == TRUE) SetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], FALSE);
}


// Select all available mezzanine cards for copying setup
void SelectAllForCopy(void) {
  int mezz, dimmed;
  
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    GetCtrlAttribute(CopyMezzSetupHandle, mezzButtonForCopy[mezz], ATTR_DIMMED, &dimmed);
    if (dimmed == FALSE) SetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], TRUE);
  }
  mezz = MAXNUMBERMEZZANINE;
  GetCtrlAttribute(CopyMezzSetupHandle, mezzButtonForCopy[mezz], ATTR_DIMMED, &dimmed);
  if (dimmed == FALSE) SetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], TRUE);
}


// Unselect all mezzanine cards for copying setup
void UnselectAllForCopy(void) {
  int mezz;
  
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++)
    SetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[mezz], FALSE);
  SetCtrlVal(CopyMezzSetupHandle, mezzButtonForCopy[MAXNUMBERMEZZANINE], FALSE);
}


void SetMezzanineAMTChannel(int mezz) {
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL0, mezzSetupArray[AMTS_SETUP+AMTS_CH0][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL1, mezzSetupArray[AMTS_SETUP+AMTS_CH1][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL2, mezzSetupArray[AMTS_SETUP+AMTS_CH2][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL3, mezzSetupArray[AMTS_SETUP+AMTS_CH3][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL4, mezzSetupArray[AMTS_SETUP+AMTS_CH4][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL5, mezzSetupArray[AMTS_SETUP+AMTS_CH5][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL6, mezzSetupArray[AMTS_SETUP+AMTS_CH6][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL7, mezzSetupArray[AMTS_SETUP+AMTS_CH7][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL8, mezzSetupArray[AMTS_SETUP+AMTS_CH8][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL9, mezzSetupArray[AMTS_SETUP+AMTS_CH9][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL10, mezzSetupArray[AMTS_SETUP+AMTS_CH10][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL11, mezzSetupArray[AMTS_SETUP+AMTS_CH11][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL12, mezzSetupArray[AMTS_SETUP+AMTS_CH12][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL13, mezzSetupArray[AMTS_SETUP+AMTS_CH13][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL14, mezzSetupArray[AMTS_SETUP+AMTS_CH14][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL15, mezzSetupArray[AMTS_SETUP+AMTS_CH15][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL16, mezzSetupArray[AMTS_SETUP+AMTS_CH16][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL17, mezzSetupArray[AMTS_SETUP+AMTS_CH17][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL18, mezzSetupArray[AMTS_SETUP+AMTS_CH18][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL19, mezzSetupArray[AMTS_SETUP+AMTS_CH19][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL20, mezzSetupArray[AMTS_SETUP+AMTS_CH20][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL21, mezzSetupArray[AMTS_SETUP+AMTS_CH21][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL22, mezzSetupArray[AMTS_SETUP+AMTS_CH22][mezz]);
  SetCtrlVal(AMTSetupHandle, P_AMTSETUP_CHANNEL23, mezzSetupArray[AMTS_SETUP+AMTS_CH23][mezz]);
}


void SetMezzanineASDChannel(int mezz, int ASDNumber) {
  int ASDOffset;
  
  ASDOffset = ASD_SETUP_LENGTH * (ASDNumber - 1);
  //set ASD channel mask flags
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0, mezzSetupArray[ASDOffset+ASD_CH0][mezz]);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1, mezzSetupArray[ASDOffset+ASD_CH1][mezz]);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2, mezzSetupArray[ASDOffset+ASD_CH2][mezz]);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3, mezzSetupArray[ASDOffset+ASD_CH3][mezz]);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4, mezzSetupArray[ASDOffset+ASD_CH4][mezz]);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5, mezzSetupArray[ASDOffset+ASD_CH5][mezz]);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6, mezzSetupArray[ASDOffset+ASD_CH6][mezz]);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7, mezzSetupArray[ASDOffset+ASD_CH7][mezz]);
}


void SetMezzanineThreshold(int mezz, int ASDNumber) {
  int i, ASDOffset, main_thresh;
  
  ASDOffset = ASD_SETUP_LENGTH * (ASDNumber - 1);

  for (i = 0; i < AMTS_SETUP+AMTSETUP_LENGTH; i++) statusSetupCheck[i] = mezzSetupArray[i][mezz];
  ReversedBinaryToInt(&main_thresh, ASDOffset+ASD_MAIN_THRESH, 8, statusSetupCheck);
  main_thresh = 2*main_thresh - 254;
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, main_thresh);
}


//************************************************************************
//
// ASD Panel Routines Here
//
//************************************************************************

//check that all ASDs have data
void ASDDone(void) {
  int main_thresh;
  
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, &main_thresh);
  main_thresh /= 2; main_thresh *= 2;
  if (main_thresh < -254) main_thresh = -254;
  if (main_thresh > 256) main_thresh = 256;
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, main_thresh);
  SaveASDSetupChanges(1);
  HidePanel(ASDSetupHandle);
}


// Discard all changes for ASD setup if confirmed
// The current ASD setup is stored in the file "currSetupFile" which is not
// touched, therefor it is not recall previous ASD setup. 
void ASDCancel(void) {
  int response;
  
  response = ConfirmPopup("ASD Setup Cancellation Confirmation",
                          "Are you sure to discard all changes for current ASD setup?");
  if (response == 1) {
    HidePanel(ASDSetupHandle);
    PanelRecall(ASDSetupHandle);
    SavePanelState(ASDSetupHandle, ASDTmpSetupFile, selectedASD);
  }
}


void ASDAllChannelOn(void) {
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0, TRUE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1, TRUE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2, TRUE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3, TRUE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4, TRUE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5, TRUE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6, TRUE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7, TRUE);
}


void ASDAllChannelOff(void) {
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0, FALSE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1, FALSE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2, FALSE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3, FALSE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4, FALSE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5, FALSE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6, FALSE);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7, FALSE);
}


void ASDChannelsModeActive(void) {
  static int active = 0;
  
  if (active == 0) active = 2;
  else active = 0;
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, active);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1MODE, active);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2MODE, active);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3MODE, active);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4MODE, active);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5MODE, active);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6MODE, active);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7MODE, active);
}


void ASDChannelsModeLow(void) {
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, 1);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1MODE, 1);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2MODE, 1);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3MODE, 1);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4MODE, 1);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5MODE, 1);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6MODE, 1);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7MODE, 1);
}


void ASDChannelsModeHigh(void) {
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, 3);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1MODE, 3);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2MODE, 3);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3MODE, 3);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4MODE, 3);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5MODE, 3);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6MODE, 3);
  SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7MODE, 3);
}


void ApplyToAllASD(void) {
  applyToASD1 = 1;
  applyToASD2 = 1;
  applyToASD3 = 1;
  SetPanelAttribute(ASDSetupHandle, ATTR_TITLE, "ASD1 Control");
  SavePanelState(ASDSetupHandle, ASDTmpSetupFile, 1);
  SetPanelAttribute(ASDSetupHandle, ATTR_TITLE, "ASD2 Control");
  SavePanelState(ASDSetupHandle, ASDTmpSetupFile, 2);
  SetPanelAttribute(ASDSetupHandle, ATTR_TITLE, "ASD3 Control");
  SavePanelState(ASDSetupHandle, ASDTmpSetupFile, 3);
}


void ApplyTo1stNextASD(void) {
  if (selectedASD == 1) {
    applyToASD2 = 1;
    SetPanelAttribute(ASDSetupHandle, ATTR_TITLE, "ASD2 Control");
    SavePanelState(ASDSetupHandle, ASDTmpSetupFile, 2);
  }
  else {
    applyToASD1 = 1; 
    SetPanelAttribute(ASDSetupHandle, ATTR_TITLE, "ASD1 Control");
    SavePanelState(ASDSetupHandle, ASDTmpSetupFile, 1);
  }
}


void ApplyTo2rdNextASD(void) {
  if (selectedASD == 3) {
    applyToASD2 = 1;
    SetPanelAttribute(ASDSetupHandle, ATTR_TITLE, "ASD2 Control");
    SavePanelState(ASDSetupHandle, ASDTmpSetupFile, 2);
  }
  else {
    applyToASD3 = 1; 
    SetPanelAttribute(ASDSetupHandle, ATTR_TITLE, "ASD3 Control");
    SavePanelState(ASDSetupHandle, ASDTmpSetupFile, 3);
  }
}


void LoadASDSetupArray(int ASDNumber) {
  //temporary variables for control values, to be converted to binary.
  int cal_cap, main_thresh, wil_thresh, hysteresis, wil_int, wil_cur, deadtime,
      channel0Mode, channel1Mode, channel2Mode, channel3Mode, channel4Mode,
      channel5Mode, channel6Mode, channel7Mode, chipMode, ASDOffset, i;
  
  ASDOffset = ASD_SETUP_LENGTH * (ASDNumber - 1);
  //set ASD channel mask flags
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0, &basicSetupArray[ASDOffset+ASD_CH0]);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1, &basicSetupArray[ASDOffset+ASD_CH1]);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2, &basicSetupArray[ASDOffset+ASD_CH2]);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3, &basicSetupArray[ASDOffset+ASD_CH3]);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4, &basicSetupArray[ASDOffset+ASD_CH4]);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5, &basicSetupArray[ASDOffset+ASD_CH5]);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6, &basicSetupArray[ASDOffset+ASD_CH6]);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7, &basicSetupArray[ASDOffset+ASD_CH7]);

  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CALIBCAPACITOR, &cal_cap);
  IntToReversedBinary(cal_cap, ASDOffset+ASD_CAL_CAP, 3, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, &main_thresh);
  main_thresh = (main_thresh + 254) / 2;
  IntToReversedBinary(main_thresh, ASDOffset+ASD_MAIN_THRESH, 8, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_THRESH, &wil_thresh);
  IntToReversedBinary(wil_thresh, ASDOffset+ASD_WIL_THRESH, 3, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_HYSTERESIS, &hysteresis);
  IntToReversedBinary(hysteresis, ASDOffset+ASD_HYSTERESIS, 4, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_INT, &wil_int);
  IntToReversedBinary(wil_int, ASDOffset+ASD_WIL_INT, 4, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_WIL_CUR, &wil_cur);
  IntToReversedBinary(wil_cur, ASDOffset+ASD_WIL_CUR, 3, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_DEADTIME, &deadtime);
  IntToReversedBinary(deadtime, ASDOffset+ASD_DEADTIME, 3, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0MODE, &channel0Mode);
  IntToReversedBinary(channel0Mode, ASDOffset+ASD_CHANNEL_MODE_0, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1MODE, &channel1Mode);
  IntToReversedBinary(channel1Mode, ASDOffset+ASD_CHANNEL_MODE_1, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2MODE, &channel2Mode);
  IntToReversedBinary(channel2Mode, ASDOffset+ASD_CHANNEL_MODE_2, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3MODE, &channel3Mode);
  IntToReversedBinary(channel3Mode, ASDOffset+ASD_CHANNEL_MODE_3, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4MODE, &channel4Mode);
  IntToReversedBinary(channel4Mode, ASDOffset+ASD_CHANNEL_MODE_4, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5MODE, &channel5Mode);
  IntToReversedBinary(channel5Mode, ASDOffset+ASD_CHANNEL_MODE_5, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6MODE, &channel6Mode);
  IntToReversedBinary(channel6Mode, ASDOffset+ASD_CHANNEL_MODE_6, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7MODE, &channel7Mode);
  IntToReversedBinary(channel7Mode, ASDOffset+ASD_CHANNEL_MODE_7, 2, basicSetupArray, SETUP_LENGTH);
  GetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHIPMODE, &chipMode);
  IntToReversedBinary(chipMode, ASDOffset+ASD_CHIP_MODE, 1, basicSetupArray, SETUP_LENGTH);
  for (i = 0; i < ASD_SETUP_LENGTH; i++)
    statusSetupCheck[ASDOffset+i] = basicSetupArray[ASDOffset+i];
}

