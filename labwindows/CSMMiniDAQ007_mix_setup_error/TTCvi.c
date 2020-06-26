// TTCvi.c
//
#ifdef _MSC_VER           // Defined for Miscrsoft Visual C++
  #include "MVC_Stuff.h"
  #include <nivxi.h>
#elif defined _CVI_       // Otherwise compile in CVI environment
  #include <userint.h>
  #include <utility.h>
  #include <ansi_c.h>
  #include <nivxi.h>
#endif

#include <formatio.h>
#include "interface.h"
#include "DAQGUI.h"
#include "TCPServer.h"
#include "TTCvi.h"

static int needTTCviInit = TRUE;
extern int Menu00Handle, JTAGControlHandle, TTCviTriggerDisabled;
extern void Quit(void);

//-------------------------------------------------------------
//
// TTCvi panel control routines
//
//-------------------------------------------------------------
// Load TTCvi Panels and define user buttons
int TTCviPanelAndButton(void)
{
  int dimmed;
  
  // Define the TTCvi Status and Control panel handles
  if ((TTCStatusAndControlHandle = LoadPanel(0, "DAQGUI.uir", TTCCTRL_P)) < 0) return -1;
  SetPanelAttribute(TTCStatusAndControlHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  if ((TTCBaseMessageHandle = LoadPanel(0, "DAQGUI.uir", TTCMSG_P)) < 0) return -1;
  SetPanelAttribute(TTCBaseMessageHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  if ((BGOControlHandle = LoadPanel(0, "DAQGUI.uir", BGOCTRL_P)) < 0) return -1;
  SetPanelAttribute(BGOControlHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);
  if ((TrigSelectHandle = LoadPanel(0, "DAQGUI.uir", TRIGSEL_P)) < 0) return -1;
  SetPanelAttribute(TrigSelectHandle, ATTR_CLOSE_ITEM_VISIBLE, 0);

  // TTCvi Status and Control Panel Buttons
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_DONE, TTCviDone);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_BASE, TTCviSetBase);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_BOARDID, TTCviBoardID);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_BOARDRESET, TTCviBoardReset);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_ECRESET, TTCviEventCounterReset);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_BCRESET, TTCviBunchCounterReset);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_ECBCRESET, TTCviEventAndBunchCounterReset);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_SOFTWARETRIGGER, TTCviSoftwareTrigger);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_READEVENTNB, TTCviReadEventNumber);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_ORBITSIGNAL, TTCviOrbitSignalSelection);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, TTCviTriggerSelection);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, TTCviRandomTriggerRate);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RESETL1AFIFO, TTCviResetL1AFIFO);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RESETBGOFIFO0, TTCviResetBGOFIFO0);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RESETBGOFIFO1, TTCviResetBGOFIFO1);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RESETBGOFIFO2, TTCviResetBGOFIFO2);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RESETBGOFIFO3, TTCviResetBGOFIFO3);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RESETBGOFIFO4, TTCviResetBGOFIFOAll);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO0, TTCviRetransmitBGOFIFOControl);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO1, TTCviRetransmitBGOFIFOControl);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO2, TTCviRetransmitBGOFIFOControl);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO3, TTCviRetransmitBGOFIFOControl);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITENABLEALL, TTCviRetransmitBGOFIFOEnableAll);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITDISABLEALL, TTCviRetransmitBGOFIFODisableAll);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, TTCviRunType);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_SELECTCOUNT, TTCviSelectCount);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_RESETEVTORBITCOUNT, TTCviResetTTCviEventOrbitCounter);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_FORMAT, TTCviTriggerWord);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXACCESS, TTCviTriggerWord);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXADDRESS, TTCviTriggerWord);
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_SUBADDRESS, TTCviTriggerWord);

  // TTCvi B-Go Control Panel Buttons
  SetupUserButton(TTCStatusAndControlHandle, TTCCTRL_P_BGOCONTROL, UpTTCviBGOControl);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_LOADBGOSETUP, TTCviLoadBGOSetup);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_SAVEBGOSETUP, TTCviSaveBGOSetup);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_DONE, TTCviBGOControlDone);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_DEFAULT, TTCviBGODefault);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_EXECUTEBCHANNEL, TTCviExecuteBChannelCycle);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_FPENABLEALL, TTCviEnableAllFrontPanel);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_FPDISABLEALL, TTCviDisableAllFrontPanel);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_ALLSINGLE, TTCviAllSingleMode);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_ALLREPETIVE, TTCviAllRepetiveMode);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_ALLSYNCHRONOUS, TTCviAllSynchronous);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_ALLASYNCHRONOUS, TTCviAllAsynchronous);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_ALLYES, TTCviAllStartIfFIFONotEmpty);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_ALLNO, TTCviAllIgnoreFIFOStatus);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO0, TTCviWriteBGOFIFO0);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO1, TTCviWriteBGOFIFO1);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO2, TTCviWriteBGOFIFO2);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO3, TTCviWriteBGOFIFO3);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFOALL, TTCviWriteBGOFIFOAll);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_GENERATEBGO0SIGNAL, TTCviGenerateBGO0Signal);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_GENERATEBGO1SIGNAL, TTCviGenerateBGO1Signal);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_GENERATEBGO2SIGNAL, TTCviGenerateBGO2Signal);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_GENERATEBGO3SIGNAL, TTCviGenerateBGO3Signal);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_GENERATEALLBGOSIGNAL, TTCviGenerateAllBGOSignal);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RESETBGOFIFO0, TTCviResetBGOFIFO0);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RESETBGOFIFO1, TTCviResetBGOFIFO1);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RESETBGOFIFO2, TTCviResetBGOFIFO2);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RESETBGOFIFO3, TTCviResetBGOFIFO3);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RESETBGOFIFOALL, TTCviResetBGOFIFOAll);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO0, TTCviRetransmitBGOFIFOControl);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO1, TTCviRetransmitBGOFIFOControl);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO2, TTCviRetransmitBGOFIFOControl);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO3, TTCviRetransmitBGOFIFOControl);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RETRANSMITENABLEALL, TTCviRetransmitBGOFIFOEnableAll);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_RETRANSMITDISABLEALL, TTCviRetransmitBGOFIFODisableAll);
  SetupUserButton(BGOControlHandle, BGOCTRL_P_BOARDRESET, TTCviBoardReset);
  
  SetupUserButton(Menu00Handle, MENU00_TTCVI_STATUSANDCONTROL, UpTTCviStatusAndControl);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_BGOCONTROL, UpTTCviBGOControl);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_RESETBOARD, TTCviBoardReset);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_RESETL1AFIFO, TTCviResetL1AFIFO);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_ECRESET, TTCviEventCounterReset);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_BCRESET, TTCviBunchCounterReset);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_ECBCRESET, TTCviEventAndBunchCounterReset);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_SOFTWARETRIGGER, TTCviSoftwareTrigger);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_1HZ, TTCviSetTrigger1Hz);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_100HZ, TTCviSetTrigger100Hz);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_1KHZ, TTCviSetTrigger1KHz);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_5KHZ, TTCviSetTrigger5KHz);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_10KHZ, TTCviSetTrigger10KHz);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_25KHZ, TTCviSetTrigger25KHz);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_50KHZ, TTCviSetTrigger50KHz);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_100KHZ, TTCviSetTrigger100KHz);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH0, TTCviSetExternalTriggerChannel0);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH1, TTCviSetExternalTriggerChannel1);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH2, TTCviSetExternalTriggerChannel2);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH3, TTCviSetExternalTriggerChannel3);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_DISABLETRIGGER, TTCviSetUnusedTrigger);
  SetupUserButton(Menu00Handle, MENU00_TTCVI_TRIGSELECT, UpTTCviTriggerSelect);
  dimmed = FALSE;
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_STATUSANDCONTROL, ATTR_DIMMED, dimmed);
  dimmed = TRUE;
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_BGOCONTROL, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RESETBOARD, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RESETL1AFIFO, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_ECRESET, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_BCRESET, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_ECBCRESET, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_SOFTWARETRIGGER, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_1HZ, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_100HZ, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_1KHZ, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_5KHZ, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_10KHZ, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_25KHZ, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_50KHZ, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_100KHZ, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH0, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH1, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH2, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH3, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_DISABLETRIGGER, ATTR_DIMMED, dimmed);
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_TRIGSELECT, ATTR_DIMMED, dimmed);

  // TTCvi Trigger Selection Panel Buttons
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_DONE, TTCviTriggerSelectDone);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_MEASURETRIGGERRATE, MeasureVariesTriggerRate);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_L1ACHANNEL0, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_L1ACHANNEL1, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_L1ACHANNEL2, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_L1ACHANNEL3, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_VMEONESHOT, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_SOFTTRIG1HZ, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_SOFTTRIG100HZ, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_SOFTTRIG1KHZ, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_SOFTTRIG5KHZ, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_SOFTTRIG10KHZ, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_SOFTTRIG25KHZ, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_SOFTTRIG50KHZ, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_SOFTTRIG100KHZ, TTCviTriggerSelect);
  SetupUserButton(TrigSelectHandle, TRIGSEL_P_CALIBTRIGGER, TTCviTriggerSelect);

  return 0;
}


void UpTTCviStatusAndControl(void)
{
  int dimmed;
  
  if (controlOption == HELP) {
    MessagePopup("Help on TTCvi Status & Control",
                 "Bring up TTCvi status and control panel\n");
    controlOption = NORM;
    return;
  }
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, TRUE);
  gotTTCvi = FALSE;
  measureTriggerRate = FALSE;
  if (needTTCviInit) {
#ifdef TTCVIDRIVERVIATCP
    if (!TTCviTCPConnected) {
      printf("TTCvi TCP/IP client is not connected, unable to control TTCvi.\n");
      printf("If wish to control TTCvi by MiniDAQ, please connect TTCvi TCP/IP client.\n");
      dimmed = TRUE;
      SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, dimmed);
      SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_STATUSANDCONTROL, ATTR_DIMMED, dimmed);
      return;      
    }
#else
    if (InitVXIlibrary() < 0) { 
      MessagePopup("TTCvi Error", 
                   "VXI library initialization failed, no way to use TTCvi right now.\n\n"
                   "If you wish to use TTCvi, please follow the steps as listed below:\n"
                   "1) Quit from current program (i.e., DAQ Controller);\n"
                   "2) Please make sure NI-VXI is properly installed and that your NI-VXI\n"
                   "   board (or crate) is connected with the computer;\n"
                   "3) Perform the RESMAN (Resource Manager) and make sure it runs succesfully;\n"
                   "4) Restart JTAG Controller to include TTCvi.\n");
      CloseVXIlibrary();
      dimmed = TRUE;
      SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, dimmed);
      SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_STATUSANDCONTROL, ATTR_DIMMED, dimmed);
      return;      
    }
#endif
    InitTTCvi();
    needTTCviInit = FALSE;
  }
  if (!GetTTCviBase()) return;
  gotTTCvi = TRUE;
  if (setTTCviDefault) {
    TTCviBoardReset();
    TTCviBGODefault();
    setTTCviDefault = FALSE;
  }
  TTCviReadEventNumber();
  UpdateTTCviStatusAndControlPanel();
  UpdateTTCviBGOControlPanel();
  // Now bring the panel up
  DisplayPanel(TTCStatusAndControlHandle); 
  SetActivePanel(TTCStatusAndControlHandle);
  HidePanel(JTAGControlHandle);
  TTCviControlsDone = 0;
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, FALSE);
  while (TTCviControlsDone == 0) HandleUserButtonAndFunction(TTCviDone); // Handle buttons and user function
  measureTriggerRate = TRUE;
}


void TTCviDone(void)
{
  TTCviControlsDone = 1;
  TTCviBGOControlDone();
  DisplayPanel(JTAGControlHandle); 
  SetActivePanel(JTAGControlHandle);
  HidePanel(TTCStatusAndControlHandle);
}


void TTCviSetBase(void)
{
  unsigned int newBase;
  
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_BASE, &newBase);
  if (newBase != TTCviBase) TTCviResetBGOFIFOAll();
  if (SetTTCviBase(newBase, EEPROMLOW) != 0) {
    TTCviBase |= 0xFF000000;
    SaveParameters(PARATTCVIBASE);
  }
  UpdateTTCviStatusAndControlPanel();
}


void TTCviBoardID(void)
{
  GetTTCviBoardID(TRUE);
}


void TTCviBoardReset(void)
{
  if (controlOption == HELP) {
    MessagePopup("Help on TTCvi Reset Board",
                 "Reset TTCvi Board (Note: No Reset on BGO FIFOs)\n");
    controlOption = NORM;
    return;
  }
  // First reset event offset to 0
  WriteTTCviReg(EVENTCOUNTLSW, 0);
  WriteTTCviReg(EVENTCOUNTMSW, 0);
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_EVTNBOFFSET, 0);
  WriteTTCviReg(SOFTWARERESET, 0xFFFF);
  UpdateTTCviStatusAndControlPanel();
  UpdateTTCviBGOControlPanel();
  TTCviReadEventNumber();
}


void TTCviEventCounterReset(void) 
{
  unsigned int readAddr, writeAddr, addr, keptData[256], duration;
  unsigned int format, access, address, subaddress, dataComm, data;
  
  if (controlOption == HELP) {
    MessagePopup("Help on TTCvi Event Counter Reset",
                 "Issue an event counter reset broadcast command from TTCvi\n");
    controlOption = NORM;
    return;
  }
  readAddr = BGOFIFOReadAddress[0];
  writeAddr = BGOFIFOWriteAddress[0];
  for (addr = 0; addr < 256; addr++) keptData[addr] = BGOFIFOData[addr][0];
  ReadTTCviReg(INHIBIT0DURATION, &duration);
  duration &= TTCVIDURATIONMASK;
  WriteTTCviReg(INHIBIT0DURATION, 1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, 1);
  format = 0;
  dataComm = 2;
  TTCviResetBGOFIFO0();
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, format);
  CheckTTCviBGOSettings();
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, dataComm);
  TTCviWriteBGOFIFO0();
  TTCviGenerateBGO0Signal();
  WaitInMillisecond(3);
  TTCviResetBGOFIFO0();
  TTCviWriteBGOFIFO0();
  TTCviGenerateBGO0Signal();
  WriteTTCviReg(INHIBIT0DURATION, duration);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, duration);

  // Deal with B-Go FIFO 0
  TTCviResetBGOFIFO0();
  for (addr = 0; addr < writeAddr; addr++) {
    data = keptData[addr];
    format = (data >> 31) & 1;
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, format);
    if (format == 0)             // Short format
      dataComm = (data >> 23) & 0xFF;
    else {                       // Long format
      access = (data >> 16) & 1;
      address = (data >> 17) & 0x3FFF;
      subaddress = (data >> 8) & 0xFF;
      dataComm = (data >> 0) & 0xFF;
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, access);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, address);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, subaddress);
    }
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, dataComm);
    TTCviWriteBGOFIFO0();
  }
  BGOFIFOReadAddress[0] = readAddr;
}


void TTCviBunchCounterReset(void)
{
  unsigned int readAddr, writeAddr, addr, keptData[256], duration;
  unsigned int format, access, address, subaddress, dataComm, data;
  
  if (controlOption == HELP) {
    MessagePopup("Help on TTCvi Bunch Counter Reset",
                 "Issue a bunch counter reset broadcast command from TTCvi\n");
    controlOption = NORM;
    return;
  }
  readAddr = BGOFIFOReadAddress[0];
  writeAddr = BGOFIFOWriteAddress[0];
  for (addr = 0; addr < 256; addr++) keptData[addr] = BGOFIFOData[addr][0];
  ReadTTCviReg(INHIBIT0DURATION, &duration);
  duration &= TTCVIDURATIONMASK;
  WriteTTCviReg(INHIBIT0DURATION, 1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, 1);
  format = 0;
  dataComm = 1;
  TTCviResetBGOFIFO0();
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, format);
  CheckTTCviBGOSettings();
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, dataComm);
  TTCviWriteBGOFIFO0();
  TTCviGenerateBGO0Signal();
  WaitInMillisecond(3);
  TTCviResetBGOFIFO0();
  TTCviWriteBGOFIFO0();
  TTCviGenerateBGO0Signal();
  WriteTTCviReg(INHIBIT0DURATION, duration);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, duration);
  
  // Deal with B-Go FIFO 0
  TTCviResetBGOFIFO0();
  for (addr = 0; addr < writeAddr; addr++) {
    data = keptData[addr];
    format = (data >> 31) & 1;
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, format);
    if (format == 0)             // Short format
      dataComm = (data >> 23) & 0xFF;
    else {                       // Long format
      access = (data >> 16) & 1;
      address = (data >> 17) & 0x3FFF;
      subaddress = (data >> 8) & 0xFF;
      dataComm = (data >> 0) & 0xFF;
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, access);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, address);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, subaddress);
    }
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, dataComm);
    TTCviWriteBGOFIFO0();
  }
  BGOFIFOReadAddress[0] = readAddr;
}


void TTCviEventAndBunchCounterReset(void)
{
  unsigned int readAddr, writeAddr, addr, keptData[256], duration;
  unsigned int format, access, address, subaddress, dataComm, data;
  
  if (controlOption == HELP) {
    MessagePopup("Help on TTCvi Event and Bunch Counter Reset",
                 "Issue an event and bunch counter reset broadcast command from TTCvi\n");
    controlOption = NORM;
    return;
  }
  readAddr = BGOFIFOReadAddress[0];
  writeAddr = BGOFIFOWriteAddress[0];
  for (addr = 0; addr < 256; addr++) keptData[addr] = BGOFIFOData[addr][0];
  ReadTTCviReg(INHIBIT0DURATION, &duration);
  duration &= TTCVIDURATIONMASK;
  WriteTTCviReg(INHIBIT0DURATION, 1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, 1);
  format = 0;
  dataComm = 3;
  TTCviResetBGOFIFO0();
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, format);
  CheckTTCviBGOSettings();
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, dataComm);
  TTCviWriteBGOFIFO0();
  TTCviGenerateBGO0Signal();
  WaitInMillisecond(3);
  TTCviResetBGOFIFO0();
  TTCviWriteBGOFIFO0();
  TTCviGenerateBGO0Signal();
  WriteTTCviReg(INHIBIT0DURATION, duration);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, duration);

  // Deal with B-Go FIFO 0
  TTCviResetBGOFIFO0();
  for (addr = 0; addr < writeAddr; addr++) {
    data = keptData[addr];
    format = (data >> 31) & 1;
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, format);
    if (format == 0)             // Short format
      dataComm = (data >> 23) & 0xFF;
    else {                       // Long format
      access = (data >> 16) & 1;
      address = (data >> 17) & 0x3FFF;
      subaddress = (data >> 8) & 0xFF;
      dataComm = (data >> 0) & 0xFF;
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, access);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, address);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, subaddress);
    }
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, dataComm);
    TTCviWriteBGOFIFO0();
  }
  BGOFIFOReadAddress[0] = readAddr;
}


void TTCviSoftwareTrigger(void)
{
  int val;

  if (controlOption == HELP) {
    MessagePopup("Help on TTCvi One Shot Software Trigger",
                 "Generate one shot software trigger from TTCvi\n");
    controlOption = NORM;
    return;
  }
  savedTrigger = L1AFROMVME;
  ReadTTCviReg(CSR1, &val);
  val &= ~(1 << L1AFIFORESET);
  val &= ~L1ATRIGGERSELECTBITS;
  val |= (L1AFROMVME << L1ATRIGGERSELECT);
  WriteTTCviReg(CSR1, val);			       // Enable VME (software) trigger
  WriteTTCviReg(SOFTWARETRIGGER, 0xFFFF);  // VME (software) trigger
  UpdateTTCviStatusAndControlPanel();
  TTCviReadEventNumber();
}


void TTCviReadEventNumber(void)
{
  int val, eventNumber;
  
  ReadTTCviReg(EVENTCOUNTLSW, &val);
  // printf("   0x%02x       Read      D16\n", EVENTCOUNTLSW);
  eventNumber = val&0xFFFF;
  ReadTTCviReg(EVENTCOUNTMSW, &val);
  // printf("   0x%02x       Read      D16\n", EVENTCOUNTMSW);
  eventNumber |= (val&0xFF) << 16;
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_CURRENTEVTNB, eventNumber);
}


void TTCviOrbitSignalSelection(void)
{
  int orbit, val;

  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_ORBITSIGNAL, &orbit);
  ReadTTCviReg(CSR1, &val);
  val &= ~(1 << L1AFIFORESET);
  val &= ~(1 << ORBITSIGNALSELECT);
  val |= (orbit << ORBITSIGNALSELECT);
  WriteTTCviReg(CSR1, val);
  UpdateTTCviStatusAndControlPanel();
}


void TTCviTriggerSelection(void)
{
  int trig, val, runType, format, access, address, subaddress, dataComm, data, addr;
  
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &trig);
  if (runType == TTCVICALIBRATION) {
    data = BGOFIFOData[0][2];
    format = (data >> 31) & 1;
    if (format == 0) {           // Short format
      dataComm = (data >> 23) & 0xFF;
      subaddress = 0;
    }
    else {                       // Long format
      access = (data >> 16) & 1;
      address = (data >> 17) & 0x3FFF;
      subaddress = (data >> 8) & 0xFF;
      dataComm = (data >> 0) & 0xFF;
    }
    if (subaddress == 4) {
      trig = L1ANOSELECTION7;
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, trig);
    }
    else {
      if (trig == L1ACALIBRATION)
        SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO2SIGNAL, ATTR_LABEL_TEXT, "StopCTrig");
      else if (trig == L1ANOSELECTION7)
        SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO2SIGNAL, ATTR_LABEL_TEXT, "StartCTrig");
    }
  }
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &trig);
  if ((trig != L1ANOSELECTION7) && (trig != L1ANOSELECTION6)) savedTrigger = trig;
  ReadTTCviReg(CSR1, &val);
  val &= ~(1 << L1AFIFORESET);
  val &= ~L1ATRIGGERSELECTBITS;
  val |= (trig << L1ATRIGGERSELECT);
  WriteTTCviReg(CSR1, val);                // Enable selected trigger
  if (trig == L1ANOSELECTION7) TTCviTriggerDisabled = TRUE;
  else if (TTCviType == TTCVITYPE01) {
    if (trig == L1ANOSELECTION6) TTCviTriggerDisabled = TRUE;
    else TTCviTriggerDisabled = FALSE;
  }
  else TTCviTriggerDisabled = FALSE;
  UpdateTTCviStatusAndControlPanel();
}


void TTCviRandomTriggerRate(void)
{
  int rate, val;

  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, &rate);
  ReadTTCviReg(CSR1, &val);
  val &= ~(1 << L1AFIFORESET);
  val &= ~RANDOMTRIGGERRATEBITS;
  val |= (rate << RANDOMTRIGGERRATE);
  WriteTTCviReg(CSR1, val);
  UpdateTTCviStatusAndControlPanel();
}


void TTCviResetL1AFIFO(void)
{
  int val;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Reset TTCvi L1A FIFO",
                 "Reset TTCvi L1A FIFO.\n");
    controlOption = NORM;
    return;
  }
  ReadTTCviReg(CSR1, &val);
  val |= (1 << L1AFIFORESET);
  WriteTTCviReg(CSR1, val);         // Write 1 to L1AFIFORESET bit
  val &= ~(1 << L1AFIFORESET);
  WriteTTCviReg(CSR1, val);			// Write 0 to L1AFIFORESET bit
  UpdateTTCviStatusAndControlPanel();
}


void TTCviResetBGOFIFO0(void)
{
  int val;
  
  ReadTTCviReg(CSR2, &val);
  val &= ~(1 << RESETBGOFIFO0);
  val &= ~(1 << RESETBGOFIFO1);
  val &= ~(1 << RESETBGOFIFO2);
  val &= ~(1 << RESETBGOFIFO3);
  val |= (1 << RESETBGOFIFO0);
  WriteTTCviReg(CSR2, val);
  BGOFIFOReadAddress[0] = 0;
  BGOFIFOWriteAddress[0] = 0;
  TTCviRetransmitBGOFIFOControl();
  UpdateTTCviStatusAndControlPanel();
}


void TTCviResetBGOFIFO1(void)
{
  int val;
  
  ReadTTCviReg(CSR2, &val);
  val &= ~(1 << RESETBGOFIFO0);
  val &= ~(1 << RESETBGOFIFO1);
  val &= ~(1 << RESETBGOFIFO2);
  val &= ~(1 << RESETBGOFIFO3);
  val |= (1 << RESETBGOFIFO1);
  WriteTTCviReg(CSR2, val);
  BGOFIFOReadAddress[1] = 0;
  BGOFIFOWriteAddress[1] = 0;
  TTCviRetransmitBGOFIFOControl();
  UpdateTTCviStatusAndControlPanel();
}


void TTCviResetBGOFIFO2(void)
{
  int val;
  
  ReadTTCviReg(CSR2, &val);
  val &= ~(1 << RESETBGOFIFO0);
  val &= ~(1 << RESETBGOFIFO1);
  val &= ~(1 << RESETBGOFIFO2);
  val &= ~(1 << RESETBGOFIFO3);
  val |= (1 << RESETBGOFIFO2);
  WriteTTCviReg(CSR2, val);
  BGOFIFOReadAddress[2] = 0;
  BGOFIFOWriteAddress[2] = 0;
  TTCviRetransmitBGOFIFOControl();
  UpdateTTCviStatusAndControlPanel();
}


void TTCviResetBGOFIFO3(void)
{
  int val;
  
  ReadTTCviReg(CSR2, &val);
  val &= ~(1 << RESETBGOFIFO0);
  val &= ~(1 << RESETBGOFIFO1);
  val &= ~(1 << RESETBGOFIFO2);
  val &= ~(1 << RESETBGOFIFO3);
  val |= (1 << RESETBGOFIFO3);
  WriteTTCviReg(CSR2, val);
  BGOFIFOReadAddress[3] = 0;
  BGOFIFOWriteAddress[3] = 0;
  TTCviRetransmitBGOFIFOControl();
  UpdateTTCviStatusAndControlPanel();
}


void TTCviResetBGOFIFOAll(void)
{
  int val;
  
  ReadTTCviReg(CSR2, &val);
  val |= (1 << RESETBGOFIFO0);
  val |= (1 << RESETBGOFIFO1);
  val |= (1 << RESETBGOFIFO2);
  val |= (1 << RESETBGOFIFO3);
  WriteTTCviReg(CSR2, val);
  BGOFIFOReadAddress[0] = 0;
  BGOFIFOWriteAddress[0] = 0;
  BGOFIFOReadAddress[1] = 0;
  BGOFIFOWriteAddress[1] = 0;
  BGOFIFOReadAddress[2] = 0;
  BGOFIFOWriteAddress[2] = 0;
  BGOFIFOReadAddress[3] = 0;
  BGOFIFOWriteAddress[3] = 0;
  TTCviRetransmitBGOFIFOControl();
  UpdateTTCviStatusAndControlPanel();
}


void TTCviRetransmitBGOFIFOControl(void)
{
  int panel, val, control = 0;

  panel = GetActivePanel();
  if (panel == TTCStatusAndControlHandle) {
    GetCtrlVal(panel, TTCCTRL_P_RETRANSMITBGOFIFO0, &val);
    control |= (val&1) << RETRANSMITBGOFIFO0;
    GetCtrlVal(panel, TTCCTRL_P_RETRANSMITBGOFIFO1, &val);
    control |= (val&1) << RETRANSMITBGOFIFO1;
    GetCtrlVal(panel, TTCCTRL_P_RETRANSMITBGOFIFO2, &val);
    control |= (val&1) << RETRANSMITBGOFIFO2;
    GetCtrlVal(panel, TTCCTRL_P_RETRANSMITBGOFIFO3, &val);
    control |= (val&1) << RETRANSMITBGOFIFO3;
  }
  else {
    panel = BGOControlHandle;
    GetCtrlVal(panel, BGOCTRL_P_RETRANSMITBGOFIFO0, &val);
    control |= (val&1) << RETRANSMITBGOFIFO0;
    GetCtrlVal(panel, BGOCTRL_P_RETRANSMITBGOFIFO1, &val);
    control |= (val&1) << RETRANSMITBGOFIFO1;
    GetCtrlVal(panel, BGOCTRL_P_RETRANSMITBGOFIFO2, &val);
    control |= (val&1) << RETRANSMITBGOFIFO2;
    GetCtrlVal(panel, BGOCTRL_P_RETRANSMITBGOFIFO3, &val);
    control |= (val&1) << RETRANSMITBGOFIFO3;
  }
  WriteTTCviReg(CSR2, control);
  val = (control >> RETRANSMITBGOFIFO0) & 1;
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO0, val);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO0, val);
  val = (control >> RETRANSMITBGOFIFO1) & 1;
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO1, val);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO1, val);
  val = (control >> RETRANSMITBGOFIFO2) & 1;
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO2, val);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO2, val);
  val = (control >> RETRANSMITBGOFIFO3) & 1;
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO3, val);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO3, val);
  UpdateTTCviStatusAndControlPanel();
}


void TTCviRetransmitBGOFIFOEnableAll(void) {
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO0, 0);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO0, 0);
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO1, 0);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO1, 0);
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO2, 0);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO2, 0);
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO3, 0);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO3, 0);
  TTCviRetransmitBGOFIFOControl();
}


void TTCviRetransmitBGOFIFODisableAll(void) {
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO0, 1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO0, 1);
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO1, 1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO1, 1);
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO2, 1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO2, 1);
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO3, 1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO3, 1);
  TTCviRetransmitBGOFIFOControl();
}


void TTCviRunType(void) {
  static int saved = FALSE, normalTrig = -1, runType, val, oldRunType = -1, panel;
  
  panel = GetActivePanel();
  if (TTCviType == TTCVITYPE01) SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, TTCVINORMAL);
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  if (runType == TTCVINORMAL) {
    if (savedTrigger >= 0) normalTrig = savedTrigger;
    else if (selectedTrigger >= 0) normalTrig = selectedTrigger;
  }
  if (runType == oldRunType) return;
  oldRunType = runType;
  if (runType == TTCVINORMAL) {
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_BGOCONTROL, ATTR_ITEM_NAME, "B-Go Control");
    SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOCONTROL, ATTR_LABEL_TEXT, "B-Go Control");
    if (saved) {
      LoadTTCviBGOSetup("SavedNormalRun.bgo");
      saved = FALSE;
      if (normalTrig >= 0) {
        SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, normalTrig);
        TTCviTriggerSelection();
      }
    }
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_INHIBIT2DELAY, ATTR_LABEL_TEXT, "Inhibit 2 Delay");
    ResetTextBox(BGOControlHandle, BGOCTRL_P_BGO2TITLE, "B-Go 2");
  }
  else if (runType == TTCVICALIBRATION) {
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &normalTrig);
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, TTCVINORMAL);
    SaveTTCviBGOSetup("SavedNormalRun.bgo");
    saved = TRUE;
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, TTCVICALIBRATION);
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, L1ANOSELECTION7);
    TTCviTriggerSelection();
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_BGOCONTROL, ATTR_ITEM_NAME, "B-Go and Calib. Control");
    SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOCONTROL, ATTR_LABEL_TEXT, "B-Go/Calib. Control");
    TTCviResetBGOFIFOAll();
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO2SIGNAL, ATTR_LABEL_TEXT, "StartCTrig");
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_INHIBIT2DELAY, ATTR_LABEL_TEXT, "CalibTrigDelay");
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DURATION, &val);
    if (val <= 0) SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DURATION, 1);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DELAY, 65);
    ResetTextBox(BGOControlHandle, BGOCTRL_P_BGO2TITLE, "Calib.Ctrl");
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE2, FALSE);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE2, FALSE);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE2, TRUE);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY2, FALSE);
    WriteTTCviBGOControlRegisters();
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO2, FALSE);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO2, FALSE);
    TTCviRetransmitBGOFIFOControl();
    CheckTTCviBGOSettings();
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT2, 1);           // Long format
    CheckTTCviBGOSettings();
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS2, 1);      // External 
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS2, 0);     // Broadcast mode
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS2, 3);       // Set pulse width and generate it
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, 160);  // Pulse width 4us
    TTCviWriteBGOFIFO2();
    if (panel == TTCStatusAndControlHandle) UpTTCviBGOControl();
  }
  UpdateTTCviStatusAndControlPanel();
}


void TTCviCalibrationRun(void) {
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, TTCVICALIBRATION);
  TTCviRunType();
}


void TTCviNormalDAQRun(void) {
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, TTCVINORMAL);
  TTCviRunType();
}


void TTCviSelectCount(void) {
  int trig, sel, val;

  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &trig);
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_SELECTCOUNT, &sel);
  ReadTTCviReg(CSR1, &val);
  val &= ~(1 << L1AFIFORESET);
  val &= ~L1ATRIGGERSELECTBITS;
  val &= ~(1 << SELECTCOUNT);
  val |= (L1ANOSELECTION7 << L1ATRIGGERSELECT);   // Disable trigger first
  val |= (sel << SELECTCOUNT);
  WriteTTCviReg(CSR1, val);
  WriteTTCviReg(RESETEVTORBITCOUNT, 0xFFFF);      // Reset Counter
  val &= ~(1 << L1AFIFORESET);
  val &= ~L1ATRIGGERSELECTBITS;
  val &= ~(1 << SELECTCOUNT);
  val |= (trig << L1ATRIGGERSELECT);              // Restore trigger selection
  val |= (sel << SELECTCOUNT);
  WriteTTCviReg(CSR1, val);
  TTCviReadEventNumber();
  UpdateTTCviStatusAndControlPanel();
}


void TTCviResetTTCviEventOrbitCounter(void) {
  WriteTTCviReg(RESETEVTORBITCOUNT, 0xFFFF);
  TTCviReadEventNumber();
  UpdateTTCviStatusAndControlPanel();
}


void TTCviTriggerWord(void) {
  int gotFunction, timeout, format, access, address, subaddr, val, dimmed;
  double time0, dtime;

  gotFunction = 0;
  timeout = FALSE;
  time0 = Timer();
  while ((gotFunction <= 1) && (!timeout)) {
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_FORMAT, &format);
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXACCESS, &access);
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXADDRESS, &address);
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_SUBADDRESS, &subaddr);
    gotFunction = CheckUserButtons(TTCviTriggerWord);   // Handle buttons
    if (gotFunction > 1) {
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_FORMAT, format);
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXACCESS, access);
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXADDRESS, address);
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_SUBADDRESS, subaddr);
    }
    dimmed = FALSE;
    if (format == 0) dimmed = TRUE;
    SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXACCESS, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXADDRESS, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_SUBADDRESS, ATTR_DIMMED, dimmed);
    if (gotFunction == 1) time0 = Timer();
    else dtime = Timer() - time0;
    if (dtime > 6.0) timeout = TRUE;
    if (dimmed) break;
  }
  val = address << TRIGWORDADDRESS;
  WriteTTCviReg(TRIGGERWORDMSW, val);
  val = 0;
  val |= subaddr << TRIGWORDSUBADDR;
  val |= access << TRIGWORDACCESS;
  val |= format << TRIGWORDCONTROL;
  WriteTTCviReg(TRIGGERWORDLSW, val);
}


//-------------------------------------------------------------
//
// TTCvi controls for menu
//
//-------------------------------------------------------------
void TTCviSetTrigger1Hz(void)
{
  int triggerRate;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Set Random Trigger Rate",
                 "Set random software trigger to 1 Hz\n");
    controlOption = NORM;
    return;
  }
  triggerRate = 1;
  SetTTCviTriggerAndRate(-1, &triggerRate);
}


void TTCviSetTrigger100Hz(void)
{
  int triggerRate;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Set Random Trigger Rate",
                 "Set random software trigger to 100 Hz\n");
    controlOption = NORM;
    return;
  }
  triggerRate = 100;
  SetTTCviTriggerAndRate(-1, &triggerRate);
}


void TTCviSetTrigger1KHz(void)
{
  int triggerRate;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Set Random Trigger Rate",
                 "Set random software trigger to 1 KHz\n");
    controlOption = NORM;
    return;
  }
  triggerRate = 1000;
  SetTTCviTriggerAndRate(-1, &triggerRate);
}


void TTCviSetTrigger5KHz(void)
{
  int triggerRate;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Set Random Trigger Rate",
                 "Set random software trigger to 5 KHz\n");
    controlOption = NORM;
    return;
  }
  triggerRate = 5000;
  SetTTCviTriggerAndRate(-1, &triggerRate);
}


void TTCviSetTrigger10KHz(void)
{
  int triggerRate;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Set Random Trigger Rate",
                 "Set random software trigger to 10 KHz\n");
    controlOption = NORM;
    return;
  }
  triggerRate = 10000;
  SetTTCviTriggerAndRate(-1, &triggerRate);
}


void TTCviSetTrigger25KHz(void)
{
  int triggerRate;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Set Random Trigger Rate",
                 "Set random software trigger to 25 KHz\n");
    controlOption = NORM;
    return;
  }
  triggerRate = 25000;
  SetTTCviTriggerAndRate(-1, &triggerRate);
}


void TTCviSetTrigger50KHz(void)
{
  int triggerRate;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Set Random Trigger Rate",
                 "Set random software trigger to 50 KHz\n");
    controlOption = NORM;
    return;
  }
  triggerRate = 50000;
  SetTTCviTriggerAndRate(-1, &triggerRate);
}


void TTCviSetTrigger100KHz(void)
{
  int triggerRate;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Set Random Trigger Rate",
                 "Set random software trigger to 100 KHz\n");
    controlOption = NORM;
    return;
  }
  triggerRate = 100000;
  SetTTCviTriggerAndRate(-1, &triggerRate);
}


void TTCviSetExternalTriggerChannel0(void)
{
  int val, trig, nlist, i;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Select External Trigger",
                 "Select external trigger from channel 0\n");
    controlOption = NORM;
    return;
  }
  trig = L1AFROMCHANNEL0;
  GetNumListItems(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &nlist);
  for (i = 0; i < nlist; i++) {
    SetCtrlIndex(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, i);
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &val);
    if (val == trig) {
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, val);
      break;
    }
  }
  TTCviTriggerSelection();
}


void TTCviSetExternalTriggerChannel1(void)
{
  int val, trig, nlist, i;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Select External Trigger",
                 "Select external trigger from channel 1\n");
    controlOption = NORM;
    return;
  }
  trig = L1AFROMCHANNEL1;
  GetNumListItems(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &nlist);
  for (i = 0; i < nlist; i++) {
    SetCtrlIndex(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, i);
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &val);
    if (val == trig) {
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, val);
      break;
    }
  }
  TTCviTriggerSelection();
}


void TTCviSetExternalTriggerChannel2(void)
{
  int val, trig, nlist, i;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Select External Trigger",
                 "Select external trigger from channel 2\n");
    controlOption = NORM;
    return;
  }
  trig = L1AFROMCHANNEL2;
  GetNumListItems(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &nlist);
  for (i = 0; i < nlist; i++) {
    SetCtrlIndex(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, i);
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &val);
    if (val == trig) {
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, val);
      break;
    }
  }
  TTCviTriggerSelection();
}


void TTCviSetExternalTriggerChannel3(void)
{
  int val, trig, nlist, i;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Select External Trigger",
                 "Select external trigger from channel 3\n");
    controlOption = NORM;
    return;
  }
  trig = L1AFROMCHANNEL3;
  GetNumListItems(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &nlist);
  for (i = 0; i < nlist; i++) {
    SetCtrlIndex(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, i);
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &val);
    if (val == trig) {
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, val);
      break;
    }
  }
  TTCviTriggerSelection();
}


void TTCviSetUnusedTrigger(void)
{
  static int val, trig, nlist, i;
  
  if (controlOption == HELP) {
    MessagePopup("Help on Disable TTCvi Trigger",
                 "Set TTCvi trigger to unused selection == Disable TTCvi Trigger\n");
    controlOption = NORM;
    return;
  }
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &val);
  if (TTCviType == TTCVITYPE01) {
    if (trig == L1ANOSELECTION6) trig = L1ANOSELECTION7; 
    else trig = L1ANOSELECTION6;
  }
  else trig = L1ANOSELECTION7;
  if ((val != L1ANOSELECTION7) && (val != L1ANOSELECTION6)) savedTrigger = val;
  GetNumListItems(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &nlist);
  for (i = 0; i < nlist; i++) {
    SetCtrlIndex(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, i);
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &val);
    if (val == trig) {
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, val);
      break;
    }
  }
  TTCviTriggerSelection();
}


void TTCviRestoreTrigger(void) {
  int val, nlist, i, runType, selectTrigger = FALSE;
  
  if (savedTrigger < 0) {
    ReadTTCviReg(CSR1, &val);
    savedTrigger = (val & L1ATRIGGERSELECTBITS) >> L1ATRIGGERSELECT;
  }
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  if (savedTrigger == L1ANOSELECTION7) selectTrigger = TRUE;
  if (savedTrigger == L1ANOSELECTION6) selectTrigger = TRUE;
  if ((TTCviType != TTCVITYPE01) && (runType == TTCVICALIBRATION)) selectTrigger = FALSE;
  if (selectTrigger) {
    MessagePopup("TTCvi Trigger Selection",
                 "Trigger is disabled, please select a trigger!");
    UpTTCviTriggerSelect();
    savedTrigger = selectedTrigger;
  }
  GetNumListItems(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &nlist);
  for (i = 0; i < nlist; i++) {
    SetCtrlIndex(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, i);
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &val);
    if (runType == TTCVICALIBRATION) {
      if (val == L1ACALIBRATION) {
        SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, val);
        break;
      }
    }
    else {
      if (val == savedTrigger) {
        SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, val);
        break;
      }
    }
  }
  TTCviTriggerSelection();
}


void TTCviSaveTriggerSettings(void) {
  saveSavedTrigger = savedTrigger;
  saveSelectedTrigger = selectedTrigger;
  saveSelectedTriggerRate = selectedTriggerRate;
  triggerSettingSaved = TRUE;
}


void TTCviRestoreTriggerSettings(void) {
  if (triggerSettingSaved) {
    savedTrigger = saveSavedTrigger;
    selectedTrigger = saveSelectedTrigger;
    selectedTriggerRate = saveSelectedTriggerRate;
  }
  triggerSettingSaved = FALSE;
}


//-------------------------------------------------------------
//
// TTCvi B-Go control panel and buttons
//
//-------------------------------------------------------------
void UpTTCviBGOControl(void)
{
  if (controlOption == HELP) {
    MessagePopup("Help on BGO Control",
                 "Bring up TTCvi BGO control panel\n");
    controlOption = NORM;
    return;
  }
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, TRUE);
  if (!GetTTCviBase()) return;
  if (setTTCviDefault) {
    TTCviBoardReset();
    TTCviBGODefault();
    setTTCviDefault = FALSE;
  }
  TTCviReadEventNumber();
  UpdateTTCviStatusAndControlPanel();
  UpdateTTCviBGOControlPanel();
  // Now bring the panel up
  DisplayPanel(BGOControlHandle); 
  SetActivePanel(BGOControlHandle);
  TTCviControlsDone = 0;
  TTCviBGODone = 0;
  SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, FALSE);
  while ((TTCviControlsDone == 0) && (TTCviBGODone == 0)) {
    CheckTTCviBGOSettings();
    HandleUserButtonAndFunction(TTCviBGOControlDone);   // Handle buttons and user function
  }
}


void TTCviLoadBGOSetup(void)
{
  char filename[5192];

  if (FileSelectPopup("", "*.bgo", "", "Retrieve TTCvi B-Go Settings",
                      VAL_LOAD_BUTTON, 0, 1, 1, 0, filename)) {
    if (!LoadTTCviBGOSetup(filename)) {
      printf("Unable to open file <%s> to load TTCvi B-Go settings.\n", filename);
      MessagePopup("Unable to load B-Go Setup", "Unable to open file to retrieve TTCvi B-Go settings!");
    }
  }
}


void TTCviSaveBGOSetup(void)
{
  char filename[5192];
  FILE *BGOFile;
  unsigned int format, access, address, subaddress, dataComm, data, addr;
  int i, val, val0[4], val1[4];
  
  if (FileSelectPopup("", "*.bgo", "", "Save TTCvi B-Go Settings",
                      VAL_SAVE_BUTTON, 0, 1, 1, 0, filename)) {
    if (!SaveTTCviBGOSetup(filename))
      MessagePopup("Unable to Save B-Go Setup", "Unable to open file to save TTCvi B-Go settings!");
  }
}


void TTCviBGOControlDone(void)
{
  TTCviBGODone = 1;
  WriteTTCviBGOControlRegisters();
  HidePanel(BGOControlHandle); 
}


void TTCviBGODefault(void)
{
  int runType;
  char filename[80];
  
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  if (runType == TTCVICALIBRATION) strcpy(filename, "CalibrationRunDefault.bgo");
  else strcpy(filename, "default.bgo");
  if (!LoadTTCviBGOSetup(filename)) {
    DefaultPanel(BGOControlHandle);
    WriteTTCviBGOControlRegisters();
    TTCviResetBGOFIFOAll();
  }
}


void TTCviExecuteBChannelCycle(void)
{
  int format, access, address, subaddress, data, command;
  
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT, &format);
  if (format == 0) {                  // Short format
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND, &command);
    WriteTTCviReg(SHORTVMECYCLE, command);
  }
  else {                              // Long format
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS, &access);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS, &address);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS, &subaddress);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND, &data);
    access &= 1;
    address &= 0x3FFF;
    subaddress &= 0xFF;
    data &= 0xFF;
    data = (1 << 31) | (address << 17) | (access << 16) | (subaddress << 8);
    WriteTTCviReg4(LONGVMECYCLE, data);
  }
}


void TTCviEnableAllFrontPanel(void)
{
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE0, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE1, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE2, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE3, FALSE);
}


void TTCviDisableAllFrontPanel(void)
{
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE0, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE1, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE2, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE3, TRUE);
}


void TTCviAllSynchronous(void)
{
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE0, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE1, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE2, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE3, FALSE);
}


void TTCviAllAsynchronous(void)
{
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE0, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE1, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE2, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE3, TRUE);
}


void TTCviAllSingleMode(void)
{
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE0, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE1, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE2, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE3, FALSE);
}


void TTCviAllRepetiveMode(void)
{
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE0, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE1, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE2, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE3, TRUE);
}


void TTCviAllStartIfFIFONotEmpty(void)
{
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY0, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY1, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY2, FALSE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY3, FALSE);
}


void TTCviAllIgnoreFIFOStatus(void)
{
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY0, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY1, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY2, TRUE);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY3, TRUE);
}


void TTCviWriteBGOFIFO0(void)
{
  unsigned int format, access, address, subaddress, dataComm, data;

  if (BGOFIFOWriteAddress[0] > 255) return;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, &format);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, &access);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, &address);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, &subaddress);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, &dataComm);
  format &= 1;
  access &= 1;
  address &= 0x3FFF;
  subaddress &= 0xFF;
  dataComm &= 0xFF;
  data = 0;
  if (format == 0)      // Short format
    data = (0 << 31) | (dataComm << 23);
  else                  // Long format 
    data = (1 << 31) | (address << 17) | (access << 16) | (subaddress << 8) | dataComm;
  if (WriteTTCviReg4(BGO0PARAMETER, data) == 0) BGOFIFOData[BGOFIFOWriteAddress[0]++][0] = data;
  UpdateTTCviStatusAndControlPanel();
}


void TTCviWriteBGOFIFO1(void)
{
  unsigned int format, access, address, subaddress, dataComm, data;

  if (BGOFIFOWriteAddress[1] > 255) return;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT1, &format);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS1, &access);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS1, &address);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS1, &subaddress);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, &dataComm);
  format &= 1;
  access &= 1;
  address &= 0x3FFF;
  subaddress &= 0xFF;
  dataComm &= 0xFF;
  data = 0;
  if (format == 0)      // Short format
    data = (0 << 31) | (dataComm << 23);
  else                  // Long format 
    data = (1 << 31) | (address << 17) | (access << 16) | (subaddress << 8) | dataComm;
  if (WriteTTCviReg4(BGO1PARAMETER, data) == 0) BGOFIFOData[BGOFIFOWriteAddress[1]++][1] = data;
  UpdateTTCviStatusAndControlPanel();
}


void TTCviWriteBGOFIFO2(void)
{
  unsigned int format, access, address, subaddress, dataComm, data;

  if (BGOFIFOWriteAddress[2] > 255) return;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT2, &format);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS2, &access);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS2, &address);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS2, &subaddress);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, &dataComm);
  format &= 1;
  access &= 1;
  address &= 0x3FFF;
  subaddress &= 0xFF;
  dataComm &= 0xFF;
  data = 0;
  if (format == 0)      // Short format
    data = (0 << 31) | (dataComm << 23);
  else                  // Long format 
    data = (1 << 31) | (address << 17) | (access << 16) | (subaddress << 8) | dataComm;
  if (WriteTTCviReg4(BGO2PARAMETER, data) == 0) BGOFIFOData[BGOFIFOWriteAddress[2]++][2] = data;
  UpdateTTCviStatusAndControlPanel();
}


void TTCviWriteBGOFIFO3(void)
{
  unsigned int format, access, address, subaddress, dataComm, data;

  if (BGOFIFOWriteAddress[3] > 255) return;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT3, &format);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS3, &access);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS3, &address);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS3, &subaddress);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND3, &dataComm);
  format &= 1;
  access &= 1;
  address &= 0x3FFF;
  subaddress &= 0xFF;
  dataComm &= 0xFF;
  data = 0;
  if (format == 0)      // Short format
    data = (0 << 31) | (dataComm << 23);
  else                  // Long format 
    data = (1 << 31) | (address << 17) | (access << 16) | (subaddress << 8) | dataComm;
  if (WriteTTCviReg4(BGO3PARAMETER, data) == 0) BGOFIFOData[BGOFIFOWriteAddress[3]++][3] = data;
  UpdateTTCviStatusAndControlPanel();
}


void TTCviWriteBGOFIFOAll(void)
{
  TTCviWriteBGOFIFO0();
  TTCviWriteBGOFIFO1();
  TTCviWriteBGOFIFO2();
  TTCviWriteBGOFIFO3();
}


void TTCviGenerateBGO0Signal(void)
{
  int duration;
  char number[4] = {'\0', '\0', '\0', '\0'};
  
  // Check Inhibit 0 duration and update corresponding panel value
  ReadTTCviReg(INHIBIT0DURATION, &duration);
  duration &= 0xFF;
  if (duration == 0) {
    PromptPopup("Inhibit 0 Duration",
                "Inhibit 0 duration is zero where the BGO signal is disabled\n"
                "Please give a duration value for inhibit 0 (from 1 to 255, DEFAULT = 1).",
                number, 3);
    if (number[0] != '\0') duration = atoi(number);
    else duration = 1;
    if (duration > 255) duration = 255;
    WriteTTCviReg(INHIBIT0DURATION, duration);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, duration);
  }
  WriteTTCviBGOControlRegisters();
  if (BGOFIFOWriteAddress[0] == 0) {
    TTCviResetBGOFIFO0();
    TTCviWriteBGOFIFO0();
  }
  WriteTTCviReg(BGO0SWGO, 0);
  UpdateTTCviStatusAndControlPanel();
  TTCviHandleBGOFIFO(0);
}


void TTCviGenerateBGO1Signal(void)
{
  int duration;
  char number[4] = {'\0', '\0', '\0', '\0'};
  
  // Check Inhibit 1 duration and update corresponding panel value
  ReadTTCviReg(INHIBIT1DURATION, &duration);
  duration &= 0xFF;
  if (duration == 0) {
    PromptPopup("Inhibit 1 Duration",
                "Inhibit 1 duration is zero where the BGO signal is disabled\n"
                "Please give a duration value for inhibit 1 (from 1 to 255, DEFAULT = 1).",
                number, 3);
    if (number[0] != '\0') duration = atoi(number);
    else duration = 1;
    if (duration > 255) duration = 255;
    WriteTTCviReg(INHIBIT1DURATION, duration);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT1DURATION, duration);
  }
  WriteTTCviBGOControlRegisters();
  if (BGOFIFOWriteAddress[1] == 0) {
    TTCviResetBGOFIFO1();
    TTCviWriteBGOFIFO1();
  }
  WriteTTCviReg(BGO1SWGO, 0);
  UpdateTTCviStatusAndControlPanel();
  TTCviHandleBGOFIFO(1);
}


void TTCviGenerateBGO2Signal(void)
{
  int duration, runType, i;
  char number[4] = {'\0', '\0', '\0', '\0'}, str[20], *strp;
  
  // Check Inhibit 2 duration and update corresponding panel value
  ReadTTCviReg(INHIBIT2DURATION, &duration);
  duration &= 0xFF;
  if (duration == 0) {
    PromptPopup("Inhibit 2 Duration",
                "Inhibit 2 duration is zero where the BGO signal is disabled\n"
                "Please give a duration value for inhibit 2 (from 1 to 255, DEFAULT = 1).",
                number, 3);
    if (number[0] != '\0') duration = atoi(number);
    else duration = 1;
    if (duration > 255) duration = 255;
    WriteTTCviReg(INHIBIT2DURATION, duration);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DURATION, duration);
  }
  WriteTTCviBGOControlRegisters();
  if (BGOFIFOWriteAddress[2] == 0) {
    TTCviResetBGOFIFO2();
    TTCviWriteBGOFIFO2();
  }
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  if (runType == TTCVICALIBRATION) {
    GetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO2SIGNAL, ATTR_LABEL_TEXT, str);
    for (i = 0; i <= strlen(str); i++) str[i] = tolower(str[i]);
    strp = strstr(str, "start");
    if (strp == NULL) {
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, L1ANOSELECTION7);
      TTCviTriggerSelection();
      printf("Calibration Trigger Stopped!\n");
    }
    else {
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, L1ACALIBRATION);
      TTCviTriggerSelection();
      printf("Calibration Trigger Started!\n");
    }
  }
  else WriteTTCviReg(BGO2SWGO, 0);
  UpdateTTCviStatusAndControlPanel();
  TTCviHandleBGOFIFO(2);
}


void TTCviGenerateBGO3Signal(void)
{
  int duration;
  char number[4] = {'\0', '\0', '\0', '\0'};
  
  // Check Inhibit 3 duration and update corresponding panel value
  ReadTTCviReg(INHIBIT3DURATION, &duration);
  duration &= 0xFF;
  if (duration == 0) {
    PromptPopup("Inhibit 3 Duration",
                "Inhibit 3 duration is zero where the BGO signal is disabled\n"
                "Please give a duration value for inhibit 3 (from 1 to 255, DEFAULT = 1).",
                number, 3);
    if (number[0] != '\0') duration = atoi(number);
    else duration = 1;
    if (duration > 255) duration = 255;
    WriteTTCviReg(INHIBIT3DURATION, duration);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT3DURATION, duration);
  }
  WriteTTCviBGOControlRegisters();
  if (BGOFIFOWriteAddress[3] == 0) {
    TTCviResetBGOFIFO3();
    TTCviWriteBGOFIFO3();
  }
  WriteTTCviReg(BGO3SWGO, 0);
  UpdateTTCviStatusAndControlPanel();
  TTCviHandleBGOFIFO(3);
}


void TTCviGenerateAllBGOSignal(void)
{
  int runType;
  
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  TTCviGenerateBGO0Signal();
  TTCviGenerateBGO1Signal();
  if (runType != TTCVICALIBRATION) TTCviGenerateBGO2Signal();
  TTCviGenerateBGO3Signal();
  UpdateTTCviStatusAndControlPanel();
}


void TTCviHandleBGOFIFO(int BGONumber)
{
  int i, csr2, BGOMode, retx[4], mode[4], data0;
  
  retx[0] = RETRANSMITBGOFIFO0;
  retx[1] = RETRANSMITBGOFIFO1;
  retx[2] = RETRANSMITBGOFIFO2;
  retx[3] = RETRANSMITBGOFIFO3;
  mode[0] = BGO0MODE;
  mode[1] = BGO1MODE;
  mode[2] = BGO2MODE;
  mode[3] = BGO3MODE;
  if ((BGONumber < 0) || (BGONumber > 3)) return;
  if (ReadTTCviReg(CSR2, &csr2) != 0) return;
  if (ReadTTCviReg(mode[BGONumber], &BGOMode) != 0) return;
  if (BGOFIFOWriteAddress[BGONumber] <= 0) return;
  if (((BGOMode>>BGOFRONTPANEL)&1) == 0) return;
  if (((BGOMode>>BGOTRANSFERTYPE)&1) == 1) return;	      
  if (((BGOMode>>BGOFIFOUSAGE)&1) == 0) return;
  data0 = BGOFIFOData[0][BGONumber];
  for (i = 1; i < BGOFIFOWriteAddress[BGONumber]; i++) 
    BGOFIFOData[i-1][BGONumber] = BGOFIFOData[i][BGONumber];
  if (((csr2>>retx[BGONumber])&1) == 1) {
    BGOFIFOReadAddress[BGONumber]--;
    BGOFIFOWriteAddress[BGONumber]--;
  }
  else BGOFIFOData[BGOFIFOWriteAddress[BGONumber]-1][BGONumber] = data0;
  UpdateTTCviStatusAndControlPanel();
}


//--------------------------------------------------------------
//
// TTCvi trigger selection routines
//
//--------------------------------------------------------------
//
// Bring up TTCvi trigger selection panel
void UpTTCviTriggerSelect(void) {
  int panel;
  
  panel = GetActivePanel();
  HidePanel(panel);
  // Bring the panel up
  DisplayPanel(TrigSelectHandle); 
  SetActivePanel(TrigSelectHandle);
  measureAllTriggerRate = FALSE;
  MeasureVariesTriggerRate();
  measureAllTriggerRate = TRUE;
  triggerSelectDone = FALSE;
  while (!triggerSelectDone) HandleUserButtonAndFunction(TTCviTriggerSelectDone);   // Handle buttons and user function
  DisplayPanel(panel); 
  SetActivePanel(panel);
  if (selectedTrigger >= 0) savedTrigger = selectedTrigger;
}


void TTCviTriggerSelectDone(void) {
  int resp;
  
  if (selectedTrigger < 0) {
    resp = ConfirmPopup("No Selected Trigger Confirmation",
                        "No trigger is selected, are you sure?");
    if (resp == 0) return;
  }
  HidePanel(TrigSelectHandle);
  if (selectedTrigger == L1ARANDOM) {
    if (selectedTriggerRate < 0) selectedTriggerRate = 0;
    else if (selectedTriggerRate > 7) selectedTriggerRate = 7;
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, selectedTriggerRate);
    TTCviRandomTriggerRate();
  }
  if (selectedTrigger >= 0) {
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, selectedTrigger);
    TTCviTriggerSelection();
  }
  triggerSelectDone = TRUE;
}


void TTCviTriggerSelect(void) {
  int i, on, onIndex, trig, trigRate;
  
  onIndex = -1;
  for (i = 0; i < nButton; i++) {
    GetCtrlVal(TrigSelectHandle, trigButton[i], &on);
    if (on != 0) {
      trig = trigSele[i] & 0xF;
      if (trig != selectedTrigger) {
        onIndex = i;
        selectedTrigger = trig;
      }
      if (trig == L1ARANDOM) {
        trigRate = ((trigSele[i] & 0xF0) >> 4);
        if (trigRate != selectedTriggerRate) {
          onIndex = i;
          selectedTriggerRate = trigRate;
        }
      }
    }
    if (onIndex >= 0) break;
  }
  for (i = 0; i < nButton; i++) {
    if (onIndex < 0) {
      GetCtrlVal(TrigSelectHandle, trigButton[i], &on);
      if (on) onIndex = i;
    }
    else if (i != onIndex) SetCtrlVal(TrigSelectHandle, trigButton[i], 0);
  }
  if (onIndex < 0) selectedTrigger = -1;
}


void MeasureVariesTriggerRate(void) {
  int selectedCount, val, eventNumber, oldEventNumber, numberTrigger, measureIt, color;
  int i, on, onIndex, trig, trigRate, trig0, trigRate0, runType, dimmed, oldSavedTrigger;
  float rate, startTime, dtime;

  ReadTTCviReg(CSR1, &val);
  trig0 = ((val & L1ATRIGGERSELECTBITS) >> L1ATRIGGERSELECT);
  trigRate0 = ((val & RANDOMTRIGGERRATEBITS) >> RANDOMTRIGGERRATE);
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  if (TTCviType == TTCVITYPE01) runType = TTCVINORMAL;
  ResetTextBox(TrigSelectHandle, TRIGSEL_P_MESSAGE, "Patient! Take a while to measure trigger rate.");
  SetCtrlAttribute(TrigSelectHandle, TRIGSEL_P_DONE, ATTR_DIMMED, TRUE);
  SetCtrlAttribute(TrigSelectHandle, TRIGSEL_P_MEASURETRIGGERRATE, ATTR_DIMMED, TRUE);
  onIndex = -1;
  for (i = 0; i < nButton; i++) {
    GetCtrlVal(TrigSelectHandle, trigButton[i], &on);
    SetCtrlAttribute(TrigSelectHandle, trigButton[i], ATTR_DIMMED, TRUE);
    SetCtrlAttribute(TrigSelectHandle, rateButton[i], ATTR_TEXT_BGCOLOR, VAL_MAGENTA);   
    if (on != 0) onIndex = i;
  }
  oldSavedTrigger = savedTrigger;
  TTCviSetUnusedTrigger();
  if (onIndex < 0) {
    if (savedTrigger >= 0) trig0 = savedTrigger;
    for (i = 0; i < nButton; i++) {
      trig = trigSele[i] & 0xF;
      if (trig == trig0) {
        if (trig == L1ARANDOM) {
          trigRate = ((trigSele[i] & 0xF0) >> 4);
          if (trigRate == trigRate0) onIndex = i;
        }
        else onIndex = i;
      }
    }
  }
  DisplayPanel(TrigSelectHandle);
  // Now try to measure varies trigger rate
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_SELECTCOUNT, &selectedCount);
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_SELECTCOUNT, 0);
  for (i = 0; i < nButton; i++) {
    trig = trigSele[i] & 0xF;
    color = VAL_GREEN;
    GetCtrlVal(TrigSelectHandle, rateButton[i], &rate);
    if (measureAllTriggerRate || (rate <= 0.0)) measureIt = TRUE;
    else if ((trig == L1ARANDOM) || (trig == L1AFROMVME)) measureIt = FALSE;
    else measureIt = TRUE;
    if (measureIt) {
      if (trig == L1ARANDOM) {
        trigRate = ((trigSele[i] & 0xF0) >> 4);
        if (trigRate < 0) trigRate = 0;
        else if (trigRate > 7) trigRate = 7;
        SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, trigRate);
        TTCviRandomTriggerRate();
      }
      if (trig >= 0) {
        if (trig == L1ACALIBRATION) TTCviCalibrationRun();
        SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, trig);
        TTCviTriggerSelection();
      }
      TTCviSelectCount();
      oldEventNumber = 0;
      eventNumber = 0;
      numberTrigger = 0;
      startTime = (float) Timer();
      dtime = 0.0;
      while (TRUE) {
        if (trig == L1AFROMVME) {
          savedTrigger = L1AFROMVME;
          TTCviOneShotTriggerIfSelected();
        }
        if ((eventNumber > 5) && (dtime >= 0.5)) break;
        else if ((eventNumber == 0) && (dtime >= 3.0)) break;
        else if (dtime > 10.0) break;
        dtime = ((float) Timer()) - startTime;
        ReadTTCviReg(EVENTCOUNTLSW, &val);
        eventNumber = val&0xFFFF;
        ReadTTCviReg(EVENTCOUNTMSW, &val);
        eventNumber |= (val&0xFF) << 16;
      }
      dtime = ((float) Timer()) - startTime;
      numberTrigger += eventNumber - oldEventNumber;
      if ((eventNumber&0xFF0000) < (oldEventNumber&0xFF0000)) numberTrigger += 0xFFFFFF;
      rate = ((float) numberTrigger) / dtime;
      if (rate <= 0.0) {
        rate = 0.0;
        color = VAL_RED;
      }
    }
    else {
      if (rate <= 0.0) {
        rate = 0.0;
        color = VAL_MAGENTA;
      }
    }
    SetCtrlAttribute(TrigSelectHandle, rateButton[i], ATTR_TEXT_BGCOLOR, color);   
    SetCtrlVal(TrigSelectHandle, rateButton[i], rate);
    DisplayPanel(TrigSelectHandle);
    SetActivePanel(TrigSelectHandle);
  }
  if (runType != TTCVICALIBRATION) TTCviNormalDAQRun();
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_SELECTCOUNT, &selectedCount);
  TTCviSelectCount();
  SetCtrlAttribute(TrigSelectHandle, TRIGSEL_P_DONE, ATTR_DIMMED, FALSE);
  SetCtrlAttribute(TrigSelectHandle, TRIGSEL_P_MEASURETRIGGERRATE, ATTR_DIMMED, FALSE);
  if (onIndex >= 0) {
    selectedTrigger = trigSele[onIndex] & 0xF;
    if (selectedTrigger == L1ARANDOM) selectedTriggerRate = ((trigSele[onIndex] & 0xF0) >> 4);
    if (selectedTriggerRate >= 0) {
      if (selectedTriggerRate > 7) selectedTriggerRate = 7;
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, selectedTriggerRate);
      TTCviRandomTriggerRate();
    }
    if (selectedTrigger >= 0) {
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, selectedTrigger);
      TTCviTriggerSelection();
    }
    SetCtrlVal(TrigSelectHandle, trigButton[onIndex], 1);
  }
  for (i = 0; i < nButton; i++) {
    trig = trigSele[i] & 0xF;
    if (i != onIndex) SetCtrlVal(TrigSelectHandle, trigButton[i], 0);
    if (runType == TTCVICALIBRATION) {
      if (trig == L1ACALIBRATION) {
        dimmed = FALSE;
        SetCtrlVal(TrigSelectHandle, trigButton[i], 1);
        TTCviTriggerSelect();
      }
      else {
        dimmed = TRUE;
        SetCtrlVal(TrigSelectHandle, trigButton[i], 0);
      }
    }
    else {
      if (trig != L1ACALIBRATION) {
        dimmed = FALSE;
        if (i == onIndex) SetCtrlVal(TrigSelectHandle, trigButton[i], 1);
      }
      else {
        dimmed = TRUE;
        SetCtrlVal(TrigSelectHandle, trigButton[i], 0);
      }
    }
    SetCtrlAttribute(TrigSelectHandle, trigButton[i], ATTR_DIMMED, dimmed);
  }
  ResetTextBox(TrigSelectHandle, TRIGSEL_P_MESSAGE, "Notice! Only one trigger can be selected.");
  TTCviSetUnusedTrigger();
  savedTrigger = oldSavedTrigger;
}


void TTCviOneShotTriggerIfSelected(void) {
  static int first = TRUE, i, ntrig;
  
  if (savedTrigger == L1AFROMVME) {
    if (first) {
      first = FALSE;
      srand((int) time(NULL));
    }
    ntrig = rand() / (RAND_MAX / 128) + 1;
    for (i = 0; i < ntrig; i++) WriteTTCviReg(SOFTWARETRIGGER, 0xFFFF);  // VME (software) trigger
  }
  else first = TRUE;
}


void SaveMeasuredTriggerRate(float rate) {
  int val, trig0, trigRate0, i, trig, trigRate, match, color;

  if (rate < 0.0) return;
  ReadTTCviReg(CSR1, &val);
  trig0 = ((val & L1ATRIGGERSELECTBITS) >> L1ATRIGGERSELECT);
  trigRate0 = ((val & RANDOMTRIGGERRATEBITS) >> RANDOMTRIGGERRATE);
  for (i = 0; i < nButton; i++) {
    trig = trigSele[i] & 0xF;
    if (trig == trig0) {
      match = FALSE;
      if (trig == L1ARANDOM) {
        trigRate = ((trigSele[i] & 0xF0) >> 4);
        if (trigRate == trigRate0) match = TRUE;
      }
      else match = TRUE;
      if (match) {
        if (rate <= 0.0) color = VAL_RED;
        else color = VAL_GREEN;
        SetCtrlAttribute(TrigSelectHandle, rateButton[i], ATTR_TEXT_BGCOLOR, color);   
        SetCtrlVal(TrigSelectHandle, rateButton[i], rate);
      }
    }
  }
}


float ExpectedTriggerRate(void) {
  int runType, val, i, trig0, trigRate0, trig, trigRate, match;
  float rate = -1.0, rate1;
  
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  if (TTCviType == TTCVITYPE01) runType = TTCVINORMAL;
  ReadTTCviReg(CSR1, &val);
  trigRate0 = ((val & RANDOMTRIGGERRATEBITS) >> RANDOMTRIGGERRATE);
  if (TTCviType == TTCVITYPE01) runType = TTCVINORMAL;
  if (runType == TTCVICALIBRATION) trig0 = L1ACALIBRATION;
  else trig0 = savedTrigger;
  for (i = 0; i < nButton; i++) {
    trig = trigSele[i] & 0xF;
    if (trig == trig0) {
      match = FALSE;
      if (trig == L1ARANDOM) {
        trigRate = ((trigSele[i] & 0xF0) >> 4);
        if (trigRate == trigRate0) match = TRUE;
      }
      else match = TRUE;
      if (match) {
        GetCtrlVal(TrigSelectHandle, rateButton[i], &rate);
        if (trig == L1ARANDOM) {
          if (trigRate == L1ARANDOM1HZ) rate1 = 1.0;
          else if (trigRate == L1ARANDOM100HZ) rate1 = 100.0;
          else if (trigRate == L1ARANDOM1KHZ) rate1 = 1000.0;
          else if (trigRate == L1ARANDOM5KHZ) rate1 = 5000.0;
          else if (trigRate == L1ARANDOM10KHZ) rate1 = 10000.0;
          else if (trigRate == L1ARANDOM25KHZ) rate1 = 25000.0;
          else if (trigRate == L1ARANDOM50KHZ) rate1 = 50000.0;
          else if (trigRate == L1ARANDOM100KHZ) rate1 = 100000.0;
          if (rate1 > rate) rate = rate1;
        }
      }
    }
  }
  
  return rate;
}


float GetMeasuredTriggerRate(int trigSel0, int trigRate0) {
  int i, trig, trigRate, match;
  float rate = -1.0;
  
  for (i = 0; i < nButton; i++) {
    trig = trigSele[i] & 0xF;
    if (trig == trigSel0) {
      match = FALSE;
      if (trig == L1ARANDOM) {
        trigRate = ((trigSele[i] & 0xF0) >> 4);
        if (trigRate == trigRate0) match = TRUE;
      }
      else match = TRUE;
      if (match) {
        GetCtrlVal(TrigSelectHandle, rateButton[i], &rate);
        if ((rate <= 0.0) && (trig == L1ARANDOM)) {
          if (trigRate == L1ARANDOM1HZ) rate = 1.0;
          else if (trigRate == L1ARANDOM100HZ) rate = 100.0;
          else if (trigRate == L1ARANDOM1KHZ) rate = 1000.0;
          else if (trigRate == L1ARANDOM5KHZ) rate = 5000.0;
          else if (trigRate == L1ARANDOM10KHZ) rate = 10000.0;
          else if (trigRate == L1ARANDOM25KHZ) rate = 25000.0;
          else if (trigRate == L1ARANDOM50KHZ) rate = 50000.0;
          else if (trigRate == L1ARANDOM100KHZ) rate = 100000.0;
        }
      }
    }
  }
  
  return rate;
}


int InternalSoftwareTrigger(void) {
  int softTrigger = FALSE, runType;
  
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  if (TTCviType == TTCVITYPE01) runType = TTCVINORMAL;
  if (runType == TTCVICALIBRATION) softTrigger = FALSE;
  else if (savedTrigger == L1AFROMVME) softTrigger = TRUE;
  else if (savedTrigger == L1ARANDOM) softTrigger = TRUE;
  
  return softTrigger;
}


//--------------------------------------------------------------
//
// TTCvi utility routines
//
//--------------------------------------------------------------
//
// Set the base address for following TTCvi module
// and return VME read status at VME address baseAddress+address
// No range checking performed. You're on your own.
int SetTTCviBase(unsigned int baseAddress, int address) {
  int data, status;
  
  TTCviBase = baseAddress & 0xFFFF00;
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_BASE, TTCviBase);
#ifdef TTCVIDRIVERVIATCP
  status = SendTTCviTCPCommand(CMDTTCVIBASEADDRESS, baseAddress, 0);
  if (status == 0) status = ReadTTCviReg(address, &data);
#else
  status = VXIin(0x02, TTCviBase+address, 0x02, &data);
#endif
  if (status == 0) {
    GetTTCviBoardID(FALSE);
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_BASE, TTCviBase);
    SaveParameters(PARATTCVIBASE);
  }
  return status;
}


int GetTTCviBase(void)
{
  unsigned int i, value, status, newBase, numberBase, baseList[32], dimmed;
  char string[256], strBase[20];
  
  numberBase = 0;
#ifdef TTCVIDRIVERVIATCP
  if (!TTCviTCPConnected) return FALSE;
  if (SendTTCviTCPCommand(CMDGETTTCVIBASEADDRESS, 0, 0) == 0) {
    if (TTCviTCPVMEStatus == 0) {
      TTCviBase = TTCviTCPData;
      if (ReadTTCviReg(EEPROMLOW, &value) == 0) {
        baseList[numberBase++] = TTCviBase;
        SaveParameters(PARATTCVIBASE);
      }
    }
  }
#endif
  if (numberBase == 0) {
    ReadParameters(PARATTCVIBASE);
    TTCviBase &= 0x00FFFF00;
#ifdef TTCVIDRIVERVIATCP
    if (SendTTCviTCPCommand(CMDTTCVIBASEADDRESS, TTCviBase, 0) != 0) return FALSE;
#endif
    if (TTCviBase != 0)
      if (ReadTTCviReg(EEPROMLOW, &value) == 0) baseList[numberBase++] = TTCviBase;
  }
  if (numberBase == 0) {
    setTTCviDefault = TRUE;
    // Now bring the panel up
    DisplayPanel(TTCBaseMessageHandle); 
    SetActivePanel(TTCBaseMessageHandle);
    HidePanel(JTAGControlHandle);
#ifdef TTCVIDRIVERVIATCP
    i = 0;
    newBase = 0x100;
    while ((newBase <= 0xFFFF00) && TTCviTCPConnected) {
      status = (unsigned int) SendTTCviTCPCommand(CMDTTCVIBASEREAD16, newBase, EEPROMLOW);
      if (status == 0) {
        if (TTCviTCPData == 0) {
          if (i != 0) {
            i = 0;
            newBase += 0x100;
          }
          i++;
        }
        else {
          newBase = TTCviTCPData;
          if (TTCviTCPVMEStatus == 0) {
            status = SetTTCviBase(newBase, EEPROMLOW);
            if (status != 0) status = SetTTCviBase(newBase, EEPROMLOW);
            if (status == 0) {
              baseList[numberBase++] = newBase;
              printf("Find a good TTCvi base address = 0x%06X\n", newBase);
              if (numberBase >= 32) break;
            }
          }
          i = 0;
          newBase += 0x100;
        }
      }
      else if (i != 0) {
        i = 0;
        newBase += 0x100;
      }
      else i++;
      SetCtrlVal(TTCBaseMessageHandle, TTCMSG_P_BASE, newBase);
      HandleUserButtons(TTCviDone);            // Handle buttons and user function
    }
#else
    for (i = 0x1; i < 0xFFFF; i++) {
      if ((i < 0xFE00) || (i > 0xFE3F)) {
        newBase = i << 8;
        if (i%256 == 1) SetCtrlVal(TTCBaseMessageHandle, TTCMSG_P_BASE, newBase);
        if (SetTTCviBase(newBase, EEPROMLOW) == 0) {
          baseList[numberBase++] = newBase;
          printf("Find a good TTCvi base address = 0x%06X\n", newBase);
          if (numberBase >= 32) break;
        }
      }
      HandleUserButtons(TTCviDone);            // Handle buttons and user function
    }
#endif
    DisplayPanel(JTAGControlHandle); 
    SetActivePanel(JTAGControlHandle);
    HidePanel(TTCBaseMessageHandle);
  }
  if (numberBase == 0) {
    sprintf(string, "No Valid TTCvi Base was found! (i.e., no VME access to TTCvi board)\n"
                    "Therefore TTCvi can not be used for MiniDAQ right now.\n");
    printf("%s\n", string);
    MessagePopup("No Valid TTCvi Base", string);
    return FALSE;
  }
  else if (numberBase > 1) {
    sprintf(strBase, " ");
    sprintf(string, "Multiple TTCvi Base Was Found! Please give wanted BASE in HEXDecimal (Default=0x%06X)", baseList[0]);
    while (strlen(strBase) <= 6) {
      PromptPopup("New TTCvi Base", string, strBase, 12);
      if (strlen(strBase) <= 0) sprintf(strBase, "%06X", baseList[0]);
    }
    sscanf(strBase, "%x", &baseList[0]);
  }
  if (SetTTCviBase(baseList[0], EEPROMLOW) == 0) return TRUE;
  else {
    TTCviBase |= 0xFF000000;
    SaveParameters(PARATTCVIBASE);
    return FALSE;
  }
}


void GetTTCviBoardID(int printFlag)
{
  unsigned int manufactureID = 0, boardID = 0, boardRev = 0, val, addr, dimmed;

  for (addr = MANUFACTURERID+8; addr >= MANUFACTURERID; addr -= 4) {
    ReadTTCviReg(addr, &val);
    manufactureID |= (val&0xFF) << 2*(MANUFACTURERID+8-addr);
  }
  for (addr = BOARDIDANDSN+12; addr >= BOARDIDANDSN; addr -= 4) { 
    ReadTTCviReg(addr, &val);
    boardID |= (val&0xFF) << 2*(BOARDIDANDSN+12-addr);
  }
  for (addr = BOARDREVISIONNO+12; addr >= BOARDREVISIONNO; addr -= 4) { 
    ReadTTCviReg(addr, &val);
    boardRev |= (val&0xFF) << 2*(BOARDREVISIONNO+12-addr);
  }
  
  dimmed = FALSE;
  TTCviType = MARKII;                 // Default TTCvi Mark-II
  if ((boardRev >> 16) == 1998) {
    dimmed = TRUE;
    TTCviType = TTCVITYPE01;
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, TTCVINORMAL);
  }
  CreatTriggerSelectList();
  SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_SELECTCOUNT, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_RESETEVTORBITCOUNT, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_FORMAT, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXACCESS, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXADDRESS, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_SUBADDRESS, ATTR_DIMMED, dimmed);
  if (printFlag) {
    printf("\n");
    if (TTCviType == TTCVITYPE01) printf("Old type TTCvi (Frist Version)!\n");
    printf("Manufacture ID             =   0x%06X\n", manufactureID);
    printf("Board ID and Serial Number = 0x%08X\n", boardID);
    printf("Board Revision Number      = 0x%08X\n", boardRev);
    printf("\n");
  }
}


void CreatTriggerSelectList(void) {
  int nlist, item, list, ctrlVal, ctrlItem;
  char str[50];

  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &ctrlVal);
  GetCtrlIndex(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &ctrlItem);
  GetNumListItems(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &nlist);
  DeleteListItem(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, 0, nlist);
  nlist = 8;
  list = 0;
  for (item = 0; item < nlist; item++) {
    if (item == L1AFROMCHANNEL0) sprintf(str, "L1A Channel0 (Ext.)");
    else if (item == L1AFROMCHANNEL1) sprintf(str, "L1A Channel1 (Ext.)");
    else if (item == L1AFROMCHANNEL2) sprintf(str, "L1A Channel2 (Ext.)");
    else if (item == L1AFROMCHANNEL3) sprintf(str, "L1A Channel3 (Ext.)");
    else if (item == L1AFROMVME) sprintf(str, "One Shot SoftTrig.");
    else if (item == L1ARANDOM) sprintf(str, "Random SoftTrig.");
    else if (TTCviType == TTCVITYPE01) sprintf(str, "No Trigger Selected(%d)", item);
    else {
      if (item == L1ACALIBRATION) sprintf(str, "L1A Calib. Trigger");
      else if (item == L1ANOSELECTION7) sprintf(str, "Disable Trigger");
      else sprintf(str, "Unknow Selection(%d)", item);
    }
    InsertListItem(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, list++, str, item);
  }
  SetCtrlIndex(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, ctrlItem);
  SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, ctrlVal);
}


void UpdateTTCviStatusAndControlPanel(void)
{
  int val, trig, nlist, val1, i, dimmed;
  char str[80];
  
  if (ReadTTCviReg(EEPROMLOW, &val) == 0) {
    SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_VMESTATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
    ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_VMESTATUS, 0, "OK");
    ReadTTCviReg(CSR1, &val);                                 // Read CSR1
    trig = (val & L1ATRIGGERSELECTBITS) >> L1ATRIGGERSELECT;  // Trigger selection
    GetNumListItems(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &nlist);
    for (i = 0; i < nlist; i++) {
      SetCtrlIndex(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, i);
      GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TRIGGERSELECTION, &val1);
      if (val1 == trig) break;
    }
    if (trig == L1ARANDOM)
      SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, ATTR_DIMMED, FALSE);
    else
      SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, ATTR_DIMMED, TRUE);
    val1 = (val >> ORBITSIGNALSELECT) & 1;                   // Orbit signal selection  
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_ORBITSIGNAL, val1);
    val1 = (val >> L1AFIFOFULL) & 1;                         // L1A FIFO status
    if (val1 == 0) {
      val1 = (val >> L1AFIFOEMPTY) & 1;
      if (val1 == 1) {       // L1A FIFO empty
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_L1AFIFOSTATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_L1AFIFOSTATUS, 0, "Empty");
      }
      else {				 // L1A FIFO not empty
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_L1AFIFOSTATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_L1AFIFOSTATUS, 0, "NotEmpty");
      }
    }
    else {
      val1 = (val >> L1AFIFOEMPTY) & 1;
      if (val1 == 1) {       // Wrong L1A FIFO status
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_L1AFIFOSTATUS, ATTR_TEXT_BGCOLOR, VAL_RED);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_L1AFIFOSTATUS, 0, "Error");
      }
      else {				 // L1A FIFO full
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_L1AFIFOSTATUS, ATTR_TEXT_BGCOLOR, VAL_YELLOW);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_L1AFIFOSTATUS, 0, "Full");
      }
    }
    val1 = (val >> VMETRANSFERPENDING) & 1;                  // VME Transfer status  
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_VMETRANSFER, val1);
    val1 = (val & BCDELAYBITS) >> BCDELAY;                   // BC Delay
    sprintf(str, "%d ( = %d ns)", val1, 2*(15-val1));
    ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BCDELAY, 0, str);
    trig = (val & RANDOMTRIGGERRATEBITS) >> RANDOMTRIGGERRATE;  // Random trigger rate
    GetNumListItems(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, &nlist);
    for (i = 0; i < nlist; i++) {
      SetCtrlIndex(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, i);
      GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, &val1);
      if (val1 == trig) break;
    }
    val1 = (val >> SELECTCOUNT) & 1;                       // Counter selection  
    if (TTCviType == TTCVITYPE01) val1 = 0;
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_SELECTCOUNT, val1);
    if (val1 == 0) {   // Select Event Count
      SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_EVTNBOFFSET, ATTR_LABEL_TEXT, "Event # Offset");
      SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_READEVENTNB, ATTR_LABEL_TEXT, "Read Current Event #");
      SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_RESETEVTORBITCOUNT, ATTR_LABEL_TEXT, "Reset TTCvi Event Number");
    }
    else {             // Select Orbit Count
      SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_EVTNBOFFSET, ATTR_LABEL_TEXT, "Orbit # Offset");
      SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_READEVENTNB, ATTR_LABEL_TEXT, "Read Current Orbit #");
      SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_RESETEVTORBITCOUNT, ATTR_LABEL_TEXT, "Reset TTCvi Orbit Number");
    }
    ReadTTCviReg(CSR2, &val);                                 // Read CSR2
    val1 = (val >> BGOFIFO0FULL) & 1;                         // B-Go FIFO 0 status
    if (val1 == 0) {
      val1 = (val >> BGOFIFO0EMPTY) & 1;
      if (val1 == 1) {       // B-Go FIFO 0 empty
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO0STATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO0STATUS, 0, "Empty");
        BGOFIFOReadAddress[0] = 0;
        BGOFIFOWriteAddress[0] = 0;
      }
      else {				 // B-Go FIFO 0 not empty
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO0STATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO0STATUS, 0, "NotEmpty");
      }
    }
    else {
      val1 = (val >> BGOFIFO0EMPTY) & 1;
      if (val1 == 1) {       // Wrong B-Go FIFO 0 status
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO0STATUS, ATTR_TEXT_BGCOLOR, VAL_RED);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO0STATUS, 0, "Error");
      }
      else {				 // B-Go FIFO 0 full
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO0STATUS, ATTR_TEXT_BGCOLOR, VAL_YELLOW);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO0STATUS, 0, "Full");
      }
    }
    val1 = (val >> BGOFIFO1FULL) & 1;                         // B-Go FIFO 1 status
    if (val1 == 0) {
      val1 = (val >> BGOFIFO1EMPTY) & 1;
      if (val1 == 1) {       // B-Go FIFO 1 empty
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO1STATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO1STATUS, 0, "Empty");
        BGOFIFOReadAddress[1] = 0;
        BGOFIFOWriteAddress[1] = 0;
      }
      else {				 // B-Go FIFO 1 not empty
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO1STATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO1STATUS, 0, "NotEmpty");
      }
    }
    else {
      val1 = (val >> BGOFIFO1EMPTY) & 1;
      if (val1 == 1) {       // Wrong B-Go FIFO 1 status
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO1STATUS, ATTR_TEXT_BGCOLOR, VAL_RED);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO1STATUS, 0, "Error");
      }
      else {				 // B-Go FIFO 1 full
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO1STATUS, ATTR_TEXT_BGCOLOR, VAL_YELLOW);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO1STATUS, 0, "Full");
      }
    }
    val1 = (val >> BGOFIFO2FULL) & 1;                         // B-Go FIFO 2 status
    if (val1 == 0) {
      val1 = (val >> BGOFIFO2EMPTY) & 1;
      if (val1 == 1) {       // B-Go FIFO 2 empty
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO2STATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO2STATUS, 0, "Empty");
        BGOFIFOReadAddress[2] = 0;
        BGOFIFOWriteAddress[2] = 0;
      }
      else {				 // B-Go FIFO 2 not empty
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO2STATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO2STATUS, 0, "NotEmpty");
      }
    }
    else {
      val1 = (val >> BGOFIFO2EMPTY) & 1;
      if (val1 == 1) {       // Wrong B-Go FIFO 2 status
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO2STATUS, ATTR_TEXT_BGCOLOR, VAL_RED);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO2STATUS, 0, "Error");
      }
      else {				 // B-Go FIFO 2 full
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO2STATUS, ATTR_TEXT_BGCOLOR, VAL_YELLOW);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO2STATUS, 0, "Full");
      }
    }
    val1 = (val >> BGOFIFO3FULL) & 1;                         // B-Go FIFO 3 status
    if (val1 == 0) {
      val1 = (val >> BGOFIFO3EMPTY) & 1;
      if (val1 == 1) {       // B-Go FIFO 3 empty
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO3STATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO3STATUS, 0, "Empty");
        BGOFIFOReadAddress[3] = 0;
        BGOFIFOWriteAddress[3] = 0;
      }
      else {				 // B-Go FIFO 3 not empty
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO3STATUS, ATTR_TEXT_BGCOLOR, VAL_GREEN);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO3STATUS, 0, "NotEmpty");
      }
    }
    else {
      val1 = (val >> BGOFIFO3EMPTY) & 1;
      if (val1 == 1) {       // Wrong B-Go FIFO 3 status
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO3STATUS, ATTR_TEXT_BGCOLOR, VAL_RED);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO3STATUS, 0, "Error");
      }
      else {				 // B-Go FIFO 3 full
        SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO3STATUS, ATTR_TEXT_BGCOLOR, VAL_YELLOW);   
        ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_BGOFIFO3STATUS, 0, "Full");
      }
    }
    ReadTTCviReg(TRIGGERWORDMSW, &val);
    val1 = (val&TRIGWORDADDRESSBITS) >> TRIGWORDADDRESS;
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXADDRESS, val1);
    ReadTTCviReg(TRIGGERWORDLSW, &val);
    val1 = (val >> TRIGWORDCONTROL) & 1;
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_FORMAT, val1);
    dimmed = FALSE;
    if (val1 == 0) dimmed = TRUE;
    val1 = (val >> TRIGWORDACCESS) & 1;
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXACCESS, val1);
    val1 = (val&TRIGWORDSUBADDRBITS) >> TRIGWORDSUBADDR;
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_SUBADDRESS, val1);
    SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXACCESS, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_TTCRXADDRESS, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_SUBADDRESS, ATTR_DIMMED, dimmed);
  }
  else {
    SetCtrlAttribute(TTCStatusAndControlHandle, TTCCTRL_P_VMESTATUS, ATTR_TEXT_BGCOLOR, VAL_RED);   
    ReplaceTextBoxLine(TTCStatusAndControlHandle, TTCCTRL_P_VMESTATUS, 0, "Error");
  }
}


void UpdateTTCviBGOControlPanel(void)
{
  int delay, duration, val;
  
  // Read Inhibit 0 delay and duration and update corresponding panel value
  ReadTTCviReg(INHIBIT0DELAY, &delay); 
  ReadTTCviReg(INHIBIT0DURATION, &duration);
  delay &= 0xFFF;
  duration &= 0xFF;
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, duration);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DELAY, delay);
  // Read Inhibit 1 delay and duration and update corresponding panel value
  ReadTTCviReg(INHIBIT1DELAY, &delay); 
  ReadTTCviReg(INHIBIT1DURATION, &duration);
  delay &= 0xFFF;
  duration &= 0xFF;
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT1DURATION, duration);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT1DELAY, delay);
  // Read Inhibit 2 delay and duration and update corresponding panel value
  ReadTTCviReg(INHIBIT2DELAY, &delay); 
  ReadTTCviReg(INHIBIT2DURATION, &duration);
  delay &= 0xFFF;
  duration &= 0xFF;
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DURATION, duration);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DELAY, delay);
  // Read Inhibit 3 delay and duration and update corresponding panel value
  ReadTTCviReg(INHIBIT3DELAY, &delay); 
  ReadTTCviReg(INHIBIT3DURATION, &duration);
  delay &= 0xFFF;
  duration &= 0xFF;
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT3DURATION, duration);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT3DELAY, delay);
  // Read B-Go 0 Modes
  ReadTTCviReg(BGO0MODE, &val);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE0, (val>>BGOFRONTPANEL)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE0, (val>>BGOCYCLEMODE)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE0, (val>>BGOTRANSFERTYPE)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY0, (val>>BGOFIFOUSAGE)&1);
  // Read B-Go 1 Modes
  ReadTTCviReg(BGO1MODE, &val);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE1, (val>>BGOFRONTPANEL)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE1, (val>>BGOCYCLEMODE)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE1, (val>>BGOTRANSFERTYPE)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY1, (val>>BGOFIFOUSAGE)&1);
  // Read B-Go 2 Modes
  ReadTTCviReg(BGO2MODE, &val);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE2, (val>>BGOFRONTPANEL)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE2, (val>>BGOCYCLEMODE)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE2, (val>>BGOTRANSFERTYPE)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY2, (val>>BGOFIFOUSAGE)&1);
  if (TTCviType != TTCVITYPE01) {
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, (val>>BGOSELECTMODE)&1);
    TTCviRunType();
  }
  // Read B-Go 3 Modes
  ReadTTCviReg(BGO3MODE, &val);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE3, (val>>BGOFRONTPANEL)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE3, (val>>BGOCYCLEMODE)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE3, (val>>BGOTRANSFERTYPE)&1);
  SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY3, (val>>BGOFIFOUSAGE)&1);
}


void WriteTTCviBGOControlRegisters(void)
{
  int delay, duration, control, val;
  static int oldDelay0 = -1, oldDuration0 = -1, oldControl0 = -1,
             oldDelay1 = -1, oldDuration1 = -1, oldControl1 = -1,
             oldDelay2 = -1, oldDuration2 = -1, oldControl2 = -1,
             oldDelay3 = -1, oldDuration3 = -1, oldControl3 = -1;

  // Check Inhibit 0 delay and duration and write to corresponding TTCvi registers if setting changed
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DELAY, &delay);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, &duration);
  if (delay != oldDelay0) {
    WriteTTCviReg(INHIBIT0DELAY, delay);
    oldDelay0 = delay;
  }
  if (duration != oldDuration0) {
    WriteTTCviReg(INHIBIT0DURATION, duration);
    oldDuration0 = duration;
  }
  // Check Inhibit 1 delay and duration and write to corresponding TTCvi registers if setting changed
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT1DELAY, &delay);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT1DURATION, &duration);
  if (delay != oldDelay1) {
    WriteTTCviReg(INHIBIT1DELAY, delay);
    oldDelay1 = delay;
  }
  if (duration != oldDuration1) {
    WriteTTCviReg(INHIBIT1DURATION, duration);
    oldDuration1 = duration;
  }
  // Check Inhibit 2 delay and duration and write to corresponding TTCvi registers if setting changed
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DELAY, &delay);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DURATION, &duration);
  if (delay != oldDelay2) {
    WriteTTCviReg(INHIBIT2DELAY, delay);
    oldDelay2 = delay;
  }
  if (duration != oldDuration2) {
    WriteTTCviReg(INHIBIT2DURATION, duration);
    oldDuration2 = duration;
  }
  // Check Inhibit 3 delay and duration and write to corresponding TTCvi registers if setting changed
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT3DELAY, &delay);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT3DURATION, &duration);
  if (delay != oldDelay3) {
    WriteTTCviReg(INHIBIT3DELAY, delay);
    oldDelay3 = delay;
  }
  if (duration != oldDuration3) {
    WriteTTCviReg(INHIBIT3DURATION, duration);
    oldDuration3 = duration;
  }
  // Check B-Go 0 Modes and write to corresponding TTCvi register if changed
  control = 0;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE0, &val);
  control |= val << BGOFRONTPANEL;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE0, &val);
  control |= val << BGOCYCLEMODE;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE0, &val);
  control |= val << BGOTRANSFERTYPE;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY0, &val);
  control |= val << BGOFIFOUSAGE;
  if (control != oldControl0) {
    WriteTTCviReg(BGO0MODE, control);
    oldControl0 = control;
  }
  // Check B-Go 1 Modes and write to corresponding TTCvi register if changed
  control = 0;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE1, &val);
  control |= val << BGOFRONTPANEL;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE1, &val);
  control |= val << BGOCYCLEMODE;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE1, &val);
  control |= val << BGOTRANSFERTYPE;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY1, &val);
  control |= val << BGOFIFOUSAGE;
  if (control != oldControl1) {
    WriteTTCviReg(BGO1MODE, control);
    oldControl1 = control;
  }
  // Check B-Go 2 Modes and write to corresponding TTCvi register if changed
  control = 0;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE2, &val);
  control |= val << BGOFRONTPANEL;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE2, &val);
  control |= val << BGOCYCLEMODE;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE2, &val);
  control |= val << BGOTRANSFERTYPE;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY2, &val);
  control |= val << BGOFIFOUSAGE;
  if (TTCviType != TTCVITYPE01) {
    GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &val);
    control |= val << BGOSELECTMODE;
  }
  if (control != oldControl2) {
    WriteTTCviReg(BGO2MODE, control);
    oldControl2 = control;
  }
  // Check B-Go 3 Modes and write to corresponding TTCvi register if changed
  control = 0;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE3, &val);
  control |= val << BGOFRONTPANEL;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE3, &val);
  control |= val << BGOCYCLEMODE;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE3, &val);
  control |= val << BGOTRANSFERTYPE;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY3, &val);
  control |= val << BGOFIFOUSAGE;
  if (control != oldControl3) {
    WriteTTCviReg(BGO3MODE, control);
    oldControl3 = control;
  }
}


void CheckTTCviBGOSettings(void)
{
  static int shortCommand = 0, longData = 0, BAccessFormat, oldAccessFormat = -1, dimmed,
             command[4], data[4], oldFormat[4], disableFP, nDisabled, same, fill, val;
  int i, runType, controlMode0, controlMode1, address[4], addrLimit[4];
  
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  // Check B Channel data
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT, &BAccessFormat);
  if (BAccessFormat != oldAccessFormat) {
    if (BAccessFormat == 0) {            // Short format
      dimmed = TRUE;
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND, ATTR_LABEL_TEXT, "Command");
      GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND, &longData);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND, shortCommand);
    }
    else {                               // Long format
      dimmed = FALSE;
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND, ATTR_LABEL_TEXT, "Data");
      GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND, &shortCommand);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND, longData);
    }
    oldAccessFormat = BAccessFormat;
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND, ATTR_LABEL_JUSTIFY, VAL_RIGHT_JUSTIFIED);
  }

  // Check B-Go FIFO 0 data
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, &BAccessFormat);
  if (BAccessFormat != oldFormat[0]) {
    if (BAccessFormat == 0) {            // Short format
      dimmed = TRUE;
      GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, &data[0]);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, command[0]);
    }
    else {                               // Long format
      dimmed = FALSE;
      GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, &command[0]);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, data[0]);
    }
    oldFormat[0] = BAccessFormat;
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, ATTR_LABEL_JUSTIFY, VAL_RIGHT_JUSTIFIED);
  }

  // Check B-Go FIFO 1 data
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT1, &BAccessFormat);
  if (BAccessFormat != oldFormat[1]) {
    if (BAccessFormat == 0) {            // Short format
      dimmed = TRUE;
      GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, &data[1]);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, command[1]);
    }
    else {                               // Long format
      dimmed = FALSE;
      GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, &command[1]);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, data[1]);
    }
    oldFormat[1] = BAccessFormat;
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS1, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS1, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS1, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, ATTR_LABEL_JUSTIFY, VAL_RIGHT_JUSTIFIED);
  }

  // Check B-Go FIFO 2 data
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT2, &BAccessFormat);
  if (BAccessFormat != oldFormat[2]) {
    if (BAccessFormat == 0) {            // Short format
      dimmed = TRUE;
      GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, &data[2]);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, command[2]);
    }
    else {                               // Long format
      dimmed = FALSE;
      GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, &command[2]);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, data[2]);
    }
    oldFormat[2] = BAccessFormat;
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS2, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS2, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS2, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, ATTR_LABEL_JUSTIFY, VAL_RIGHT_JUSTIFIED);
  }

  // Check B-Go FIFO 3 data
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT3, &BAccessFormat);
  if (BAccessFormat != oldFormat[3]) {
    if (BAccessFormat == 0) {            // Short format
      dimmed = TRUE;
      GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND3, &data[3]);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND3, command[3]);
    }
    else {                               // Long format
      dimmed = FALSE;
      GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND3, &command[3]);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND3, data[3]);
    }
    oldFormat[3] = BAccessFormat;
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS3, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS3, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS3, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND3, ATTR_LABEL_JUSTIFY, VAL_RIGHT_JUSTIFIED);
  }

  // Check front panel enable/disable and set Generate B-GoSignal button correspondingly
  nDisabled = 0;
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE0, &disableFP);
  nDisabled += disableFP;
  if (disableFP == 1)
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO0SIGNAL, ATTR_DIMMED, FALSE);
  else
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO0SIGNAL, ATTR_DIMMED, TRUE);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE1, &disableFP);
  nDisabled += disableFP;
  if (disableFP == 1)
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO1SIGNAL, ATTR_DIMMED, FALSE);
  else
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO1SIGNAL, ATTR_DIMMED, TRUE);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE2, &disableFP);
  nDisabled += disableFP;
  if (disableFP == 1)
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO2SIGNAL, ATTR_DIMMED, FALSE);
  else
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO2SIGNAL, ATTR_DIMMED, TRUE);
  if (runType == TTCVICALIBRATION)
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO2SIGNAL, ATTR_DIMMED, FALSE);
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE3, &disableFP);
  nDisabled += disableFP;
  if (disableFP == 1)
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO3SIGNAL, ATTR_DIMMED, FALSE);
  else
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEBGO3SIGNAL, ATTR_DIMMED, TRUE);
  if (nDisabled > 1)
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEALLBGOSIGNAL, ATTR_DIMMED, FALSE);
  else
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_GENERATEALLBGOSIGNAL, ATTR_DIMMED, TRUE);

  // Check if B-Go FIFO input would be same
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMATINPUT, &same);
  if (same == 1) {
    dimmed = TRUE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT3, &val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT1, val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT2, val);
  }
  else dimmed = FALSE;
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_FORMAT0, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_FORMAT1, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_FORMAT2, ATTR_DIMMED, dimmed);
  // TTCrx register selection
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXREGINPUT, &same);
  if (same == 1) {
    dimmed = TRUE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS3, &val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS1, val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS2, val);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS1, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS2, ATTR_DIMMED, dimmed);
  }
  else {
    dimmed = FALSE;
    if (oldFormat[0] == 0)
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, ATTR_DIMMED, TRUE);
    else
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, ATTR_DIMMED, dimmed);
    if (oldFormat[1] == 0)
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS1, ATTR_DIMMED, TRUE);
    else
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS1, ATTR_DIMMED, dimmed);
    if (oldFormat[2] == 0)
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS2, ATTR_DIMMED, TRUE);
    else
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS2, ATTR_DIMMED, dimmed);
  }
  // TTCrx address
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRINPUT, &same);
  if (same == 1) {
    dimmed = TRUE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS3, &val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS1, val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS2, val);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS1, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS2, ATTR_DIMMED, dimmed);
  }
  else {
    dimmed = FALSE;
    if (oldFormat[0] == 0)
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, ATTR_DIMMED, TRUE);
    else
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, ATTR_DIMMED, dimmed);
    if (oldFormat[1] == 0)
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS1, ATTR_DIMMED, TRUE);
    else
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS1, ATTR_DIMMED, dimmed);
    if (oldFormat[2] == 0)
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS2, ATTR_DIMMED, TRUE);
    else
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS2, ATTR_DIMMED, dimmed);
  }
  // Subaddress
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRINPUT, &same);
  if (same == 1) {
    dimmed = TRUE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS3, &val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS1, val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS2, val);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS1, ATTR_DIMMED, dimmed);
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS2, ATTR_DIMMED, dimmed);
  }
  else {
    dimmed = FALSE;
    if (oldFormat[0] == 0)
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, ATTR_DIMMED, TRUE);
    else
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, ATTR_DIMMED, dimmed);
    if (oldFormat[1] == 0)
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS1, ATTR_DIMMED, TRUE);
    else
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS1, ATTR_DIMMED, dimmed);
    if (oldFormat[2] == 0)
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS2, ATTR_DIMMED, TRUE);
    else
      SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS2, ATTR_DIMMED, dimmed);
  }
  // Data (for long format) or command (for short format) 
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMANDINPUT, &same);
  if (same == 1) {
    dimmed = TRUE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND3, &val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, val);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, val);
  }
  else dimmed = FALSE;
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, ATTR_DIMMED, dimmed);
  
  // Check if filling B-Go FIFO
  GetCtrlVal(BGOControlHandle, BGOCTRL_P_FILLBGOFIFOS, &fill);
  if (fill == 0) {
    dimmed = TRUE;
    controlMode0 = VAL_INDICATOR;
    controlMode1 = VAL_HOT;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO0ADDRESS, &address[0]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO1ADDRESS, &address[1]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO2ADDRESS, &address[2]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO3ADDRESS, &address[3]);
    for (i = 0; i < 4; i++) if (address[i] > 256) address[i] = 256;
    if (address[0] > BGOFIFOWriteAddress[0]) {
      address[0] = BGOFIFOWriteAddress[0];
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO0ADDRESS, address[0]);
    }
    if (address[1] > BGOFIFOWriteAddress[1]) {
      address[1] = BGOFIFOWriteAddress[1];
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO1ADDRESS, address[1]);
    }
    if (address[2] > BGOFIFOWriteAddress[2]) {
      address[2] = BGOFIFOWriteAddress[2];
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO2ADDRESS, address[2]);
    }
    if (address[3] > BGOFIFOWriteAddress[3]) {
      address[3] = BGOFIFOWriteAddress[3];
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO3ADDRESS, address[3]);
    }
    for (i = 0; i < 4; i++) {
      BGOFIFOReadAddress[i] = address[i];
      addrLimit[i] = BGOFIFOWriteAddress[i];
      if (addrLimit[i] > 256) addrLimit[i] = 256;
    }
    TTCviViewBGOFIFOs();
  }
  else {
    dimmed = FALSE;
    controlMode0 = VAL_HOT;
    controlMode1 = VAL_INDICATOR;
    for (i = 0; i < 4; i++) {
      address[i] = BGOFIFOWriteAddress[i]; 
      addrLimit[i] = 256;
      if (address[i] > 256) address[i] = 256;
    }
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO0ADDRESS, address[0]);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO1ADDRESS, address[1]);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO2ADDRESS, address[2]);
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_BGOFIFO3ADDRESS, address[3]);
  }
  nDisabled = 0;
  if (BGOFIFOWriteAddress[0] > 255) {
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO0, ATTR_DIMMED, TRUE);
    nDisabled++;
  }
  else
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO0, ATTR_DIMMED, dimmed);
  if (BGOFIFOWriteAddress[1] > 255) {
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO1, ATTR_DIMMED, TRUE);
    nDisabled++;
  }
  else
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO1, ATTR_DIMMED, dimmed);
  if (BGOFIFOWriteAddress[2] > 255) {
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO2, ATTR_DIMMED, TRUE);
    nDisabled++;
  }
  else
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO2, ATTR_DIMMED, dimmed);
  if (BGOFIFOWriteAddress[3] > 255) {
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO3, ATTR_DIMMED, TRUE);
    nDisabled++;
  }
  else
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFO3, ATTR_DIMMED, dimmed);
  if (nDisabled > 2)
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFOALL, ATTR_DIMMED, TRUE);
  else
    SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_WRITEBGOFIFOALL, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_FORMATINPUT, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXREGINPUT, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRINPUT, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRINPUT, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMANDINPUT, ATTR_DIMMED, dimmed);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_FORMAT0, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_FORMAT1, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_FORMAT2, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_FORMAT3, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS1, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS2, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXACCESS3, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS1, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS2, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS3, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS1, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS2, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_SUBADDRESS3, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND3, ATTR_CTRL_MODE, controlMode0);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_BGOFIFO0ADDRESS, ATTR_CTRL_MODE, controlMode1);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_BGOFIFO1ADDRESS, ATTR_CTRL_MODE, controlMode1);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_BGOFIFO2ADDRESS, ATTR_CTRL_MODE, controlMode1);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_BGOFIFO3ADDRESS, ATTR_CTRL_MODE, controlMode1);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_BGOFIFO0ADDRESS, ATTR_MAX_VALUE, addrLimit[0]);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_BGOFIFO1ADDRESS, ATTR_MAX_VALUE, addrLimit[1]);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_BGOFIFO2ADDRESS, ATTR_MAX_VALUE, addrLimit[2]);
  SetCtrlAttribute(BGOControlHandle, BGOCTRL_P_BGOFIFO3ADDRESS, ATTR_MAX_VALUE, addrLimit[3]);
  WriteTTCviBGOControlRegisters();
}


void TTCviViewBGOFIFOs(void)
{
  unsigned int format, access, address, subaddress, dataComm, data, addr;

  addr = BGOFIFOReadAddress[0];
  data = BGOFIFOData[addr][0];
  if ( addr <= 255) {
    format = (data >> 31) & 1;
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, format);
    if (format == 0)             // Short format
      dataComm = (data >> 23) & 0xFF;
    else {                       // Long format
      access = (data >> 16) & 1;
      address = (data >> 17) & 0x3FFF;
      subaddress = (data >> 8) & 0xFF;
      dataComm = (data >> 0) & 0xFF;
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, access);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, address);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, subaddress);
    }
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, dataComm);
  }

  addr = BGOFIFOReadAddress[1];
  data = BGOFIFOData[addr][1];
  if ( addr <= 255) {
    format = (data >> 31) & 1;
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT1, format);
    if (format == 0)             // Short format
      dataComm = (data >> 23) & 0xFF;
    else {                       // Long format
      access = (data >> 16) & 1;
      address = (data >> 17) & 0x3FFF;
      subaddress = (data >> 8) & 0xFF;
      dataComm = (data >> 0) & 0xFF;
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS1, access);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS1, address);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS1, subaddress);
    }
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, dataComm);
  }

  addr = BGOFIFOReadAddress[2];
  data = BGOFIFOData[addr][2];
  if ( addr <= 255) {
    format = (data >> 31) & 1;
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT2, format);
    if (format == 0)             // Short format
      dataComm = (data >> 23) & 0xFF;
    else {                       // Long format
      access = (data >> 16) & 1;
      address = (data >> 17) & 0x3FFF;
      subaddress = (data >> 8) & 0xFF;
      dataComm = (data >> 0) & 0xFF;
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS2, access);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS2, address);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS2, subaddress);
    }
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, dataComm);
  }

  addr = BGOFIFOReadAddress[3];
  data = BGOFIFOData[addr][3];
  if ( addr <= 255) {
    format = (data >> 31) & 1;
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT3, format);
    if (format == 0)             // Short format
      dataComm = (data >> 23) & 0xFF;
    else {                       // Long format
      access = (data >> 16) & 1;
      address = (data >> 17) & 0x3FFF;
      subaddress = (data >> 8) & 0xFF;
      dataComm = (data >> 0) & 0xFF;
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS3, access);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS3, address);
      SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS3, subaddress);
    }
    SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND3, dataComm);
  }
}


int LoadTTCviBGOSetup(char *filename)
{
  char str[256], cstr[40], *strp;
  FILE *BGOFile;
  unsigned int format, access, address, subaddress, dataComm, data, addr;
  int i, val, val0[4], val1[4], nwords;

  if (BGOFile = fopen(filename, "r")) {
    // Reset B-Go FIFO first
    TTCviResetBGOFIFOAll();
    printf("Loading TTCvi B-Go settings to file <%s>.\n", filename);
    while (TRUE) {
      if ((str[0] = fgetc(BGOFile)) == EOF) {
        fclose(BGOFile);
        break;
      }
      else if (str[0] != ' ' && str[0] != '\t' && str[0] != '\n') {
        fgets(&str[1], 255, BGOFile);
        if ((str[0] != '/') && (str[1] != '/') && strlen(str) > 0) {
          for (i = 0; i <= strlen(str); i++) str[i] = tolower(str[i]);
          // Deal with Inhit 0, 1, 2 and 3 Delay and Duration
          strcpy(cstr, "inh0delay duration");
          strp = strstr(str, cstr);
          if (strp == NULL) {
            strcpy(cstr, "inh1delay duration");
            strp = strstr(str, cstr);
          }
          if (strp == NULL) {
            strcpy(cstr, "inh2delay duration");
            strp = strstr(str, cstr);
          }
          if (strp == NULL) {
            strcpy(cstr, "inh3delay duration");
            strp = strstr(str, cstr);
          }
          if (strp != NULL) {
            fscanf(BGOFile, "%d %d %d %d %d %d %d %d\n", &val0[0], &val1[0], &val0[1], &val1[1],
                                                         &val0[2], &val1[2], &val0[3], &val1[3]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DELAY, val0[0]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, val1[0]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT1DELAY, val0[1]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT1DURATION, val1[1]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DELAY, val0[2]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DURATION, val1[2]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT3DELAY, val0[3]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT3DURATION, val1[3]);
            WriteTTCviBGOControlRegisters();
          }
          // Deal with B-Go Modes
          strcpy(cstr, "b-go0mode   b-go1mode");
          strp = strstr(str, cstr);
          if (strp == NULL) {
            strcpy(cstr, "b-go0mode  b-go1mode");
            strp = strstr(str, cstr);
          }
          if (strp == NULL) {
            strcpy(cstr, "b-go0mode b-go1mode");
            strp = strstr(str, cstr);
          }
          if (strp == NULL) {
            strcpy(cstr, "b-go2mode   b-go3mode");
            strp = strstr(str, cstr);
          }
          if (strp == NULL) {
            strcpy(cstr, "b-go2mode  b-go3mode");
            strp = strstr(str, cstr);
          }
          if (strp == NULL) {
            strcpy(cstr, "b-go2mode b-go3mode");
            strp = strstr(str, cstr);
          }
          if (strp != NULL) {
            fscanf(BGOFile, "%d %d %d %d\n", &val0[0], &val0[1], &val0[2], &val0[3]);
            val = val0[0];
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE0, (val>>BGOFRONTPANEL)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE0, (val>>BGOCYCLEMODE)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE0, (val>>BGOTRANSFERTYPE)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY0, (val>>BGOFIFOUSAGE)&1);
            val = val0[1];
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE1, (val>>BGOFRONTPANEL)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE1, (val>>BGOCYCLEMODE)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE1, (val>>BGOTRANSFERTYPE)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY1, (val>>BGOFIFOUSAGE)&1);
            val = val0[2];
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE2, (val>>BGOFRONTPANEL)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE2, (val>>BGOCYCLEMODE)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE2, (val>>BGOTRANSFERTYPE)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY2, (val>>BGOFIFOUSAGE)&1);
            if (TTCviType != TTCVITYPE01) {
              SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, (val>>BGOSELECTMODE)&1);
              TTCviRunType();
            }
            val = val0[3];
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE3, (val>>BGOFRONTPANEL)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE3, (val>>BGOCYCLEMODE)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE3, (val>>BGOTRANSFERTYPE)&1);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY3, (val>>BGOFIFOUSAGE)&1);
            WriteTTCviBGOControlRegisters();
          }
          // Deal with B-Go FIFO Retransmission Control
          strcpy(cstr, "b-go0retx   b-go1retx");
          strp = strstr(str, cstr);
          if (strp == NULL) {
            strcpy(cstr, "b-go0retx  b-go1retx");
            strp = strstr(str, cstr);
          }
          if (strp == NULL) {
            strcpy(cstr, "b-go0retx b-go1retx");
            strp = strstr(str, cstr);
          }
          if (strp == NULL) {
            strcpy(cstr, "b-go2retx   b-go3retx");
            strp = strstr(str, cstr);
          }
          if (strp == NULL) {
            strcpy(cstr, "b-go2retx  b-go3retx");
            strp = strstr(str, cstr);
          }
          if (strp == NULL) {
            strcpy(cstr, "b-go2retx b-go3retx");
            strp = strstr(str, cstr);
          }
          if (strp != NULL) {
            fscanf(BGOFile, "%d %d %d %d\n", &val0[0], &val0[1], &val0[2], &val0[3]);
            for (i = 0; i < 4; i++) if (val0[i] != 0) val0[i] = 1;
            SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO0, val0[0]);
            SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO1, val0[1]);
            SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO2, val0[2]);
            SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RETRANSMITBGOFIFO3, val0[3]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO0, val0[0]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO1, val0[1]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO2, val0[2]);
            SetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO3, val0[3]);
            TTCviRetransmitBGOFIFOControl();
          }
          // Deal with B-Go FIFO 0
          strcpy(cstr, "b-go fifo 0 data list");
          strp = strstr(str, cstr);
          if (strp != NULL) {
            fscanf(BGOFile, "%d", &nwords);
            fgets(str, 256, BGOFile);
            for (addr = 0; addr < nwords; addr++) {
              fscanf(BGOFile, "%x", &data);
              fgets(str, 256, BGOFile);
              format = (data >> 31) & 1;
              SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT0, format);
              if (format == 0)             // Short format
                dataComm = (data >> 23) & 0xFF;
              else {                       // Long format
                access = (data >> 16) & 1;
                address = (data >> 17) & 0x3FFF;
                subaddress = (data >> 8) & 0xFF;
                dataComm = (data >> 0) & 0xFF;
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS0, access);
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS0, address);
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS0, subaddress);
              }
              SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND0, dataComm);
              TTCviWriteBGOFIFO0();
            }
          }
          // Deal with B-Go FIFO 1
          strcpy(cstr, "b-go fifo 1 data list");
          strp = strstr(str, cstr);
          if (strp != NULL) {
            fscanf(BGOFile, "%d", &nwords);
            fgets(str, 256, BGOFile);
            for (addr = 0; addr < nwords; addr++) {
              fscanf(BGOFile, "%x", &data);
              fgets(str, 256, BGOFile);
              format = (data >> 31) & 1;
              SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT1, format);
              if (format == 0)             // Short format
                dataComm = (data >> 23) & 0xFF;
              else {                       // Long format
                access = (data >> 16) & 1;
                address = (data >> 17) & 0x3FFF;
                subaddress = (data >> 8) & 0xFF;
                dataComm = (data >> 0) & 0xFF;
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS1, access);
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS1, address);
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS1, subaddress);
              }
              SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND1, dataComm);
              TTCviWriteBGOFIFO1();
            }
          }
          // Deal with B-Go FIFO 2
          strcpy(cstr, "b-go fifo 2 data list");
          strp = strstr(str, cstr);
          if (strp != NULL) {
            fscanf(BGOFile, "%d", &nwords);
            fgets(str, 256, BGOFile);
            for (addr = 0; addr < nwords; addr++) {
              fscanf(BGOFile, "%x", &data);
              fgets(str, 256, BGOFile);
              format = (data >> 31) & 1;
              SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT2, format);
              if (format == 0)             // Short format
                dataComm = (data >> 23) & 0xFF;
              else {                       // Long format
                access = (data >> 16) & 1;
                address = (data >> 17) & 0x3FFF;
                subaddress = (data >> 8) & 0xFF;
                dataComm = (data >> 0) & 0xFF;
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS2, access);
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS2, address);
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS2, subaddress);
              }
              SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND2, dataComm);
              TTCviWriteBGOFIFO2();
            }
          }
          // Deal with B-Go FIFO 3
          strcpy(cstr, "b-go fifo 3 data list");
          strp = strstr(str, cstr);
          if (strp != NULL) {
            fscanf(BGOFile, "%d", &nwords);
            fgets(str, 256, BGOFile);
            for (addr = 0; addr < nwords; addr++) {
              fscanf(BGOFile, "%x", &data);
              fgets(str, 256, BGOFile);
              format = (data >> 31) & 1;
              SetCtrlVal(BGOControlHandle, BGOCTRL_P_FORMAT3, format);
              if (format == 0)             // Short format
                dataComm = (data >> 23) & 0xFF;
              else {                       // Long format
                access = (data >> 16) & 1;
                address = (data >> 17) & 0x3FFF;
                subaddress = (data >> 8) & 0xFF;
                dataComm = (data >> 0) & 0xFF;
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXACCESS3, access);
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_TTCRXADDRESS3, address);
                SetCtrlVal(BGOControlHandle, BGOCTRL_P_SUBADDRESS3, subaddress);
              }
              SetCtrlVal(BGOControlHandle, BGOCTRL_P_DATAORCOMMAND3, dataComm);
              TTCviWriteBGOFIFO3();
            }
          }
        }
      }
    }
    return TRUE;                    // Done return TRUE as success
  }
  else return FALSE;                // Can not open file, return FALSE
}


int SaveTTCviBGOSetup(char *filename) {
  FILE *BGOFile;
  unsigned int format, access, address, subaddress, dataComm, data, addr;
  int i, val, val0[4], val1[4];
  
  if (BGOFile = fopen(filename, "w")) {
    printf("Saving TTCvi B-Go settings to file <%s>.\n", filename);
    fprintf(BGOFile, "// ----------------- File for TTCvi B-Go settings -----------------------\n");
    fprintf(BGOFile, "//\n");
    fprintf(BGOFile, "// Please be careful in changing the TTCvi B-Go setting file\n");
    fprintf(BGOFile, "// And do not change/modify following key words!!!\n");
    fprintf(BGOFile, "//   Inh0Delay Duration   Inh1Delay Duration   Inh2Delay Duration   Inh3Delay Duration\n");
    fprintf(BGOFile, "//   B-Go0Mode   B-Go1Mode   B-Go2Mode   B-Go3Mode\n");
    fprintf(BGOFile, "//   B-Go FIFO 0 Data List\n");
    fprintf(BGOFile, "//   B-Go FIFO 1 Data List\n");
    fprintf(BGOFile, "//   B-Go FIFO 2 Data List\n");
    fprintf(BGOFile, "//   B-Go FIFO 3 Data List\n");
    fprintf(BGOFile, "// Comments are started with `//'\n");
    fprintf(BGOFile, "//\n");
    fprintf(BGOFile, "  Inh0Delay Duration   Inh1Delay Duration   Inh2Delay Duration   Inh3Delay Duration\n");
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DELAY, &val0[0]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT0DURATION, &val1[0]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT1DELAY, &val0[1]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT1DURATION, &val1[1]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DELAY, &val0[2]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT2DURATION, &val1[2]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT3DELAY, &val0[3]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_INHIBIT3DURATION, &val1[3]);
    fprintf(BGOFile, "%11d %8d %11d %8d %11d %8d %11d %8d\n", val0[0], val1[0], val0[1], val1[1],
                                                              val0[2], val1[2], val0[3], val1[3]);
    fprintf(BGOFile, "//\n");
    fprintf(BGOFile, "// B-Go Mode Bit Map\n");
    fprintf(BGOFile, "//   Bit %d : Front Panel Enable/Disable\n", BGOFRONTPANEL);
    fprintf(BGOFile, "//            0 = Front Panel Enabled\n");
    fprintf(BGOFile, "//            1 = Front Panel Disabled\n");
    fprintf(BGOFile, "//   Bit %d : Cycle Mode (Synchronous/Asynchronous Cycle)\n", BGOCYCLEMODE);
    fprintf(BGOFile, "//            0 = Sychronous Cycle\n");
    fprintf(BGOFile, "//            1 = Asynchronous Cycle\n");
    fprintf(BGOFile, "//   Bit %d : Transfermission Type (Single/Repetive Mode)\n", BGOTRANSFERTYPE);
    fprintf(BGOFile, "//            0 = Single Mode\n");
    fprintf(BGOFile, "//            1 = Repetive Mode\n");
    fprintf(BGOFile, "//   Bit %d : Start Cycle by looking FIFO Status\n", BGOFIFOUSAGE);
    fprintf(BGOFile, "//            0 = Start Cycle as soon as FIFO not Empty\n");
    fprintf(BGOFile, "//            1 = Do not look at FIFO Status\n");
    if (TTCviType != TTCVITYPE01) {
      fprintf(BGOFile, "//   Bit %d : Select Operation Mode (Should be 0 for B-Go 0, 1 & 3)\n", BGOSELECTMODE);
      fprintf(BGOFile, "//            0 = Basic operation mode\n");
      fprintf(BGOFile, "//            1 = Calibration mode (B-Go 2 Only)\n");
    }
    fprintf(BGOFile, "//\n");
    fprintf(BGOFile, "  B-Go0Mode   B-Go1Mode   B-Go2Mode   B-Go3Mode\n");
    for (i = 0; i < 4; i++) val0[i] = 0;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE0, &val);
    val0[0] |= val << BGOFRONTPANEL;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE0, &val);
    val0[0] |= val << BGOCYCLEMODE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE0, &val);
    val0[0] |= val << BGOTRANSFERTYPE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY0, &val);
    val0[0] |= val << BGOFIFOUSAGE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE1, &val);
    val0[1] |= val << BGOFRONTPANEL;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE1, &val);
    val0[1] |= val << BGOCYCLEMODE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE1, &val);
    val0[1] |= val << BGOTRANSFERTYPE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY1, &val);
    val0[1] |= val << BGOFIFOUSAGE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE2, &val);
    val0[2] |= val << BGOFRONTPANEL;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE2, &val);
    val0[2] |= val << BGOCYCLEMODE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE2, &val);
    val0[2] |= val << BGOTRANSFERTYPE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY2, &val);
    val0[2] |= val << BGOFIFOUSAGE;
    if (TTCviType != TTCVITYPE01) {
      GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &val);
      val0[2] |= val << BGOSELECTMODE;
    }
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_FPENABLE3, &val);
    val0[3] |= val << BGOFRONTPANEL;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_CYCLE3, &val);
    val0[3] |= val << BGOCYCLEMODE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_MODE3, &val);
    val0[3] |= val << BGOTRANSFERTYPE;
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_STARTNOTEMPTY3, &val);
    val0[3] |= val << BGOFIFOUSAGE;
    fprintf(BGOFile, "%11d %11d %11d %11d\n", val0[0], val0[1], val0[2], val0[3]);
    fprintf(BGOFile, "//\n");
    fprintf(BGOFile, "// B-Go FIFO Retransmission Control\n");
    fprintf(BGOFile, "//   0 = Enable B-Go FIFO Retransmission\n");
    fprintf(BGOFile, "//   1 = Disable B-Go FIFO Retransmission\n");
    fprintf(BGOFile, "//\n");
    fprintf(BGOFile, "  B-Go0ReTX   B-Go1ReTX   B-Go2ReTX   B-Go3ReTX\n");
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO0, &val0[0]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO1, &val0[1]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO2, &val0[2]);
    GetCtrlVal(BGOControlHandle, BGOCTRL_P_RETRANSMITBGOFIFO3, &val0[3]);
    fprintf(BGOFile, "%11d %11d %11d %11d\n", val0[0], val0[1], val0[2], val0[3]);
    fprintf(BGOFile, "//\n");
    fprintf(BGOFile, "// B-Go FIFO Data\n");
    fprintf(BGOFile, "//   Bit 31    : Data Format, Long/Short = 1/0\n");
    fprintf(BGOFile, "//   Long Format (bit 31 set)\n");
    fprintf(BGOFile, "//   Bit 30-17 : 14 bits TTCrx Address)\n");
    fprintf(BGOFile, "//   Bit 16    : TTCrx register selection, External/Internal = 1/0\n");
    fprintf(BGOFile, "//   Bit 15-08 : 8 bits subaddress)\n");
    fprintf(BGOFile, "//   Bit 07-00 : 8 bits data)\n");
    fprintf(BGOFile, "//   Short Format (bit 31 cleared)\n");
    fprintf(BGOFile, "//   Bit 30-23 : 8 bits command)\n");
    fprintf(BGOFile, "//   Bit 22-00 : Do not care)\n");
    fprintf(BGOFile, "//\n");
    fprintf(BGOFile, "  B-Go FIFO 0 Data List\n");
    fprintf(BGOFile, "  %10d (Number of B-Go FIFO data words)\n", BGOFIFOWriteAddress[0]);
    for (addr = 0; addr < BGOFIFOWriteAddress[0]; addr++) {
      data = BGOFIFOData[addr][0];
      fprintf(BGOFile, "  0x%08x", data);
      format = (data >> 31) & 1;
      if (format == 0) {           // Short format
        dataComm = (data >> 23) & 0xFF;
        fprintf(BGOFile, " (Format 0=Short, Command %3d)", dataComm);
      }
      else {                       // Long format
        access = (data >> 16) & 1;
        address = (data >> 17) & 0x3FFF;
        subaddress = (data >> 8) & 0xFF;
        dataComm = (data >> 0) & 0xFF;
        if (access == 1)
          fprintf(BGOFile, " (Format 1=Long, TTCrxReg 1=External,");
        else
          fprintf(BGOFile, " (Format 1=Long, TTCrxReg 0=Internal,");
        fprintf(BGOFile, " TTCrxAddr 0x%05x, SubAddr %3d, Data %3d)", address, subaddress, dataComm);
      }
      fprintf(BGOFile, "\n");
    }
    fprintf(BGOFile, "  B-Go FIFO 1 Data List\n");
    fprintf(BGOFile, "  %10d (Number of B-Go FIFO data words)\n", BGOFIFOWriteAddress[1]);
    for (addr = 0; addr < BGOFIFOWriteAddress[1]; addr++) {
      data = BGOFIFOData[addr][1];
      fprintf(BGOFile, "  0x%08x", data);
      format = (data >> 31) & 1;
      if (format == 0) {           // Short format
        dataComm = (data >> 23) & 0xFF;
        fprintf(BGOFile, " (Format 0=Short, Command %3d)", dataComm);
      }
      else {                       // Long format
        access = (data >> 16) & 1;
        address = (data >> 17) & 0x3FFF;
        subaddress = (data >> 8) & 0xFF;
        dataComm = (data >> 0) & 0xFF;
        if (access == 1)
          fprintf(BGOFile, " (Format 1=Long, TTCrxReg 1=External,");
        else
          fprintf(BGOFile, " (Format 1=Long, TTCrxReg 0=Internal,");
        fprintf(BGOFile, " TTCrxAddr 0x%05x, SubAddr %3d, Data %3d)", address, subaddress, dataComm);
      }
      fprintf(BGOFile, "\n");
    }
    fprintf(BGOFile, "  B-Go FIFO 2 Data List\n");
    fprintf(BGOFile, "  %10d (Number of B-Go FIFO data words)\n", BGOFIFOWriteAddress[2]);
    for (addr = 0; addr < BGOFIFOWriteAddress[2]; addr++) {
      data = BGOFIFOData[addr][2];
      fprintf(BGOFile, "  0x%08x", data);
      format = (data >> 31) & 1;
      if (format == 0) {           // Short format
        dataComm = (data >> 23) & 0xFF;
        fprintf(BGOFile, " (Format 0=Short, Command %3d)", dataComm);
      }
      else {                       // Long format
        access = (data >> 16) & 1;
        address = (data >> 17) & 0x3FFF;
        subaddress = (data >> 8) & 0xFF;
        dataComm = (data >> 0) & 0xFF;
        if (access == 1)
          fprintf(BGOFile, " (Format 1=Long, TTCrxReg 1=External,");
        else
          fprintf(BGOFile, " (Format 1=Long, TTCrxReg 0=Internal,");
        fprintf(BGOFile, " TTCrxAddr 0x%05x, SubAddr %3d, Data %3d)", address, subaddress, dataComm);
      }
      fprintf(BGOFile, "\n");
    }
    fprintf(BGOFile, "  B-Go FIFO 3 Data List\n");
    fprintf(BGOFile, "  %10d (Number of B-Go FIFO data words)\n", BGOFIFOWriteAddress[3]);
    for (addr = 0; addr < BGOFIFOWriteAddress[3]; addr++) {
      data = BGOFIFOData[addr][3];
      fprintf(BGOFile, "  0x%08x", data);
      format = (data >> 31) & 1;
      if (format == 0) {           // Short format
        dataComm = (data >> 23) & 0xFF;
        fprintf(BGOFile, " (Format 0=Short, Command %3d)", dataComm);
      }
      else {                       // Long format
        access = (data >> 16) & 1;
        address = (data >> 17) & 0x3FFF;
        subaddress = (data >> 8) & 0xFF;
        dataComm = (data >> 0) & 0xFF;
        if (access == 1)
          fprintf(BGOFile, " (Format 1=Long, TTCrxReg 1=External,");
        else
          fprintf(BGOFile, " (Format 1=Long, TTCrxReg 0=Internal,");
        fprintf(BGOFile, " TTCrxAddr 0x%05x, SubAddr %3d, Data %3d)", address, subaddress, dataComm);
      }
      fprintf(BGOFile, "\n");
    }
    fclose(BGOFile);
    return TRUE;                   // Done return TRUE as success 
  }
  else {
    printf("Unable to open file <%s> to save TTCvi B-Go settings.\n", filename);
    return FALSE;                  // Can not open file, return FALSE
  }
}


int InitTTCvi(void)
{
  int dimmed;
  
  nButton = 0;
  trigButton[nButton] = TRIGSEL_P_L1ACHANNEL0;
  rateButton[nButton] = TRIGSEL_P_L1ACHANNEL0RATE;
  trigSele[nButton++] = L1AFROMCHANNEL0;
  trigButton[nButton] = TRIGSEL_P_L1ACHANNEL1;
  rateButton[nButton] = TRIGSEL_P_L1ACHANNEL1RATE;
  trigSele[nButton++] = L1AFROMCHANNEL1;
  trigButton[nButton] = TRIGSEL_P_L1ACHANNEL2;
  rateButton[nButton] = TRIGSEL_P_L1ACHANNEL2RATE;
  trigSele[nButton++] = L1AFROMCHANNEL2;
  trigButton[nButton] = TRIGSEL_P_L1ACHANNEL3;
  rateButton[nButton] = TRIGSEL_P_L1ACHANNEL3RATE;
  trigSele[nButton++] = L1AFROMCHANNEL3;
  trigButton[nButton] = TRIGSEL_P_VMEONESHOT;
  rateButton[nButton] = TRIGSEL_P_VMEONESHOTRATE;
  trigSele[nButton++] = L1AFROMVME;
  trigButton[nButton] = TRIGSEL_P_SOFTTRIG1HZ;
  rateButton[nButton] = TRIGSEL_P_SOFTTRIG1HZRATE;
  trigSele[nButton++] = L1ARANDOM | 0x00;
  trigButton[nButton] = TRIGSEL_P_SOFTTRIG100HZ;
  rateButton[nButton] = TRIGSEL_P_SOFTTRIG100HZRATE;
  trigSele[nButton++] = L1ARANDOM | 0x10;
  trigButton[nButton] = TRIGSEL_P_SOFTTRIG1KHZ;
  rateButton[nButton] = TRIGSEL_P_SOFTTRIG1KHZRATE;
  trigSele[nButton++] = L1ARANDOM | 0x20;
  trigButton[nButton] = TRIGSEL_P_SOFTTRIG5KHZ;
  rateButton[nButton] = TRIGSEL_P_SOFTTRIG5KHZRATE;
  trigSele[nButton++] = L1ARANDOM | 0x30;
  trigButton[nButton] = TRIGSEL_P_SOFTTRIG10KHZ;
  rateButton[nButton] = TRIGSEL_P_SOFTTRIG10KHZRATE;
  trigSele[nButton++] = L1ARANDOM | 0x40;
  trigButton[nButton] = TRIGSEL_P_SOFTTRIG25KHZ;
  rateButton[nButton] = TRIGSEL_P_SOFTTRIG25KHZRATE;
  trigSele[nButton++] = L1ARANDOM | 0x50;
  trigButton[nButton] = TRIGSEL_P_SOFTTRIG50KHZ;
  rateButton[nButton] = TRIGSEL_P_SOFTTRIG50KHZRATE;
  trigSele[nButton++] = L1ARANDOM | 0x60;
  trigButton[nButton] = TRIGSEL_P_SOFTTRIG100KHZ;
  rateButton[nButton] = TRIGSEL_P_SOFTTRIG100KHZRATE;
  trigSele[nButton++] = L1ARANDOM | 0x70;
  trigButton[nButton] = TRIGSEL_P_CALIBTRIGGER;
  rateButton[nButton] = TRIGSEL_P_CALIBTRIGGERRATE;
  trigSele[nButton++] = L1ACALIBRATION | 0x70;
  if (GetTTCviBase()) {
    TTCviBoardReset();
    TTCviBGODefault();
    setTTCviDefault = FALSE;
    needTTCviInit = FALSE;
    dimmed = FALSE;
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_STATUSANDCONTROL, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_BGOCONTROL, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RESETBOARD, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RESETL1AFIFO, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_ECRESET, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_BCRESET, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_ECBCRESET, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_SOFTWARETRIGGER, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_1HZ, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_100HZ, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_1KHZ, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_5KHZ, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_10KHZ, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_25KHZ, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_50KHZ, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_RANDOMTRIG_100KHZ, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH0, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH1, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH2, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_EXTERNALTRIG_CH3, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_DISABLETRIGGER, ATTR_DIMMED, dimmed);
    SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI_TRIGSELECT, ATTR_DIMMED, dimmed);
    return TRUE;
  }
  else return FALSE;
}


void SetTTCviTriggerAndRate(int triggerSelect, int *triggerRate)
{
  int regCSR1, rate;

  if (((triggerSelect < 0) || (triggerSelect > 7)) && (*triggerRate <= 0)) return;
  ReadTTCviReg(CSR1, &regCSR1);
  regCSR1 |= ~(1 << L1AFIFORESET);              // Want Reset trigger FIFO
  regCSR1 &= ~L1ATRIGGERSELECTBITS;
  if ((triggerSelect < 0) || (triggerSelect > 7) || (triggerSelect == 5)) {
    if (*triggerRate <= 0) regCSR1 |= (L1AFROMVME << L1ATRIGGERSELECT);  // Select VME trigger
    else {
      regCSR1 |= (L1ARANDOM << L1ATRIGGERSELECT);                        // Select random trigger
      if (*triggerRate < 50) {
        rate = L1ARANDOM1HZ;
        *triggerRate = 1;
      }
      else if (*triggerRate < 550) {
        rate = L1ARANDOM100HZ;
        *triggerRate = 100;
      }
      else if (*triggerRate < 3000) {
        rate = L1ARANDOM1KHZ;
        *triggerRate = 1000;
      }
      else if (*triggerRate < 7500) {
        rate = L1ARANDOM5KHZ;
        *triggerRate = 5000;
      }
      else if (*triggerRate < 15000) {
        rate = L1ARANDOM10KHZ;
        *triggerRate = 10000;
      }
      else if (*triggerRate < 37500) {
        rate = L1ARANDOM25KHZ;
        *triggerRate = 25000;
      }
      else if (*triggerRate < 75000) {
        rate = L1ARANDOM50KHZ;
        *triggerRate = 50000;
      }
      else {
        rate = L1ARANDOM100KHZ;
        *triggerRate = 100000;
      }
      SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RATESELECTION, rate);
      regCSR1 &= ~RANDOMTRIGGERRATEBITS;
      regCSR1 |= (rate << RANDOMTRIGGERRATE);
    }
  }
  else regCSR1 |= (triggerSelect << L1ATRIGGERSELECT);
  savedTrigger = (regCSR1 & L1ATRIGGERSELECTBITS) >> L1ATRIGGERSELECT;
  WriteTTCviReg(CSR1, regCSR1);                 // Write CSR1
  UpdateTTCviStatusAndControlPanel();
}


int GetTTCviTriggerRate(void)
{
  int regCSR1, trig, rate, triggerRate = -1;
  
  if ((TTCviBase>>24) == 0) {
    if (ReadTTCviReg(CSR1, &regCSR1) == 0) {
      trig = (regCSR1&L1ATRIGGERSELECTBITS) >> L1ATRIGGERSELECT;
      rate = (regCSR1&RANDOMTRIGGERRATEBITS) >> RANDOMTRIGGERRATE;
      if (trig == L1AFROMVME) triggerRate = 0;
      else if (trig == L1ARANDOM) {
        if (rate = L1ARANDOM1HZ) triggerRate = 1;
        else if (rate = L1ARANDOM100HZ) triggerRate = 100;
        else if (rate = L1ARANDOM1KHZ) triggerRate = 1000;
        else if (rate = L1ARANDOM5KHZ) triggerRate = 5000;
        else if (rate = L1ARANDOM10KHZ) triggerRate = 10000;
        else if (rate = L1ARANDOM25KHZ) triggerRate = 25000;
        else if (rate = L1ARANDOM50KHZ) triggerRate = 50000;
        else if (rate = L1ARANDOM100KHZ) triggerRate = 100000;
      }
    }
    else triggerRate = -1;
  }
  return triggerRate;
}


int GetTTCviTriggerSelction(void) {
  int runType, trig = -1, val;
  
  GetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_RUNTYPE, &runType);
  if (runType == TTCVICALIBRATION) {
    trig = L1ACALIBRATION;
    return trig;
  }
  if (!gotTTCvi) return trig;
  if (savedTrigger >= 0) trig = savedTrigger;
  else if (selectedTrigger >= 0) trig = selectedTrigger;
  else {
    ReadTTCviReg(CSR1, &val);
    trig = (val & L1ATRIGGERSELECTBITS) >> L1ATRIGGERSELECT;
  }
  return trig;
}


int GetTTCviTriggerRateSelction(void) {
  int rate = -1, val;

  if (!gotTTCvi) return rate;
  if (selectedTriggerRate >= 0) rate = selectedTriggerRate;
  else {
    ReadTTCviReg(CSR1, &val);
    rate = ((val & RANDOMTRIGGERRATEBITS) >> RANDOMTRIGGERRATE);
  }
  return rate;
}


// There could be wrong trigger rate measurement due to the fact that event
// number could change during reading event number, i.e., event number is not
// 100% reliable if there is trigger
//
float MeasuredTTCviTriggerRate(int *numberTrigger) {
  static int val, eventNumber, oldEventNumber, newMeasurement = TRUE;
  static float rate = -1.0, startTime, checkTime = -1.0, dtime, measuringTime = 0.0;
  
  if (!measureTriggerRate) {
    *numberTrigger = 0;
    rate = -1.0;
    checkTime = (float) Timer();
    newMeasurement = TRUE;
    return rate;
  }
  dtime = ((float) Timer()) - checkTime;
  if (dtime < 0.5) return rate;
  checkTime = (float) Timer();
  if (TTCviType == TTCVITYPE01) val = 0;
  else ReadTTCviReg(CSR1, &val);
  if (((val>>SELECTCOUNT)&1) == 1) {
    eventNumber = 0;
    *numberTrigger = 0;
    SetCtrlVal(TTCStatusAndControlHandle, TTCCTRL_P_SELECTCOUNT, 0);
    TTCviSelectCount();
    measuringTime = 0.0;
  }
  else {
    oldEventNumber = eventNumber;
    ReadTTCviReg(EVENTCOUNTLSW, &val);
    eventNumber = val&0xFFFF;
    ReadTTCviReg(EVENTCOUNTMSW, &val);
    eventNumber |= (val&0xFF) << 16;
    if (newMeasurement) {
      measuringTime = 0.0;
      oldEventNumber = eventNumber;
      newMeasurement = FALSE;
    }
  }
  if (measuringTime <= 0.0) {
    *numberTrigger = 0;
    startTime = (float) Timer();
    measuringTime = 2.0;
    return rate;
  }
  dtime = ((float) Timer()) - startTime;
  *numberTrigger += eventNumber - oldEventNumber;
  if ((eventNumber&0xFF0000) < (oldEventNumber&0xFF0000)) {
    TTCviSetUnusedTrigger();
    ReadTTCviReg(EVENTCOUNTLSW, &val);
    eventNumber = val&0xFFFF;
    ReadTTCviReg(EVENTCOUNTMSW, &val);
    eventNumber |= (val&0xFF) << 16;
    TTCviRestoreTrigger();
  }
  if ((eventNumber&0xFF0000) < (oldEventNumber&0xFF0000)) *numberTrigger += 0xFFFFFF;
  if (dtime >= measuringTime) {
    rate = ((float) *numberTrigger) / dtime;
    if (rate < 1.0) measuringTime = 10.0;
    else if (rate < 10.0) measuringTime = 5.0;
    else measuringTime = 2.0;
    *numberTrigger = 0;
    startTime = (float) Timer();
  }
  oldEventNumber = eventNumber;
  return rate;
}


int SendTTCviTCPCommand(int command, int addr, int data) {
  int status = 1001;
  float time0, dtime;

  if (!TTCviTCPConnected) return status;
  time0 = (float) Timer();
  dtime = 0.0;
  gotTTCviTCPData = FALSE;
  TCPTTCviData[TCPCOMMAND] = 0xC0000 | command;
  TCPTTCviData[TCPCSMNUMBER] = TCPTTCVICSMNUMBER;
  TCPTTCviData[TCPTTCVIADDR] = addr;
  TCPTTCviData[TCPTTCVIDATA] = data;
  TCPTTCviData[TCPLENGTH] = 5;
  status = SendToClients(TRUE, UNIX_TTCVI_PORT);
  while ((status == 0) && (!gotTTCviTCPData) && (dtime < 1.0)) {
    HandleUserButtons(Quit);
    dtime = ((float) Timer()) - time0;
  }
  if ((!gotTTCviTCPData) && (status == 0)) status = 101;

  return status;
}


void HandleTTCviData(unsigned int data[]) {
  unsigned int checksum, i;

  gotTTCviTCPData = TRUE;
  if ((data[0] < 5) || (data[0] > 10)) {
    TTCviTCPVMEStatus = 1101;
    TTCviTCPData = 0;
    return;
  }
  else if (data[1] != TCPTTCVICSMNUMBER) {
    TTCviTCPVMEStatus = 1102;
    TTCviTCPData = 0;
    return;
  }  
  checksum = 0;
  for (i = 0; i < data[0]; i++) checksum += data[i];
  if (checksum != data[data[0]]) {
    TTCviTCPVMEStatus = 1103;
    TTCviTCPData = 0;
    return;
  }

  TTCviTCPVMEStatus = data[2];
  TTCviTCPData = data[4];
}


#ifdef TTCVIDRIVERVIATCP
// TTCvi TCP Driver
//
int ReadTTCviReg(int address, int *data) {
  int status = 1001;

  status = SendTTCviTCPCommand(CMDTTCVIREAD16, address, *data);
  if (status == 0) {
    status = TTCviTCPVMEStatus;
    *data = TTCviTCPData;
  }

  return status;
}


int WriteTTCviReg(int address, int data) {
  int status = 1001;

  status = SendTTCviTCPCommand(CMDTTCVIWRITE16, address, data);
  if (status == 0) {
    status = TTCviTCPVMEStatus;
    data = TTCviTCPData;
  }

  return status;
}

  
int ReadTTCviReg4(int address, int *data) {
  int status = 1001;

  status = SendTTCviTCPCommand(CMDTTCVIREAD32, address, *data);
  if (status == 0) {
    status = TTCviTCPVMEStatus;
    *data = TTCviTCPData;
  }

  return status;
}


int WriteTTCviReg4(int address, int data) {
  int status = 1001;

  status = SendTTCviTCPCommand(CMDTTCVIWRITE32, address, data);
  if (status == 0) {
    status = TTCviTCPVMEStatus;
    data = TTCviTCPData;
  }

  return status;
}
#else
// Read 16 bit value "data" with A24 mode
int ReadTTCviReg(int address, int *data) {
  return VXIin(0x02, TTCviBase+address, 0x02, data);
}


// Write 16 bit value "data" with A24 mode
int WriteTTCviReg(int address, int data) {
  // printf("   0x%02x     0x%04x      D16\n", address, data&0xFFFF);
  return VXIout(0x02, TTCviBase+address, 0x02, data);
}


// Read 32 bit value "data" with A24 mode
int ReadTTCviReg4(int address, int *data) {
  return VXIin(0x02, TTCviBase+address, 0x04, data);
}


// Write 32 bit value "data" with A24 mode
int WriteTTCviReg4(int address, int data) {
  // printf("   0x%02x 0x%08x      D32\n", address, data);
  return VXIout(0x02, TTCviBase+address, 0x04, data);
}
#endif
