// DAQControl.c version 4.1                         by T.S.Dai
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
#include "jtagDriver.h"
#include "jtagControl.h"
#include "DAQControl.h"
#include "TCPServer.h"

// DAQ information controls and variables
#define INFORLENGTH         3600
#define RECENTINFORLENGTH   1200
int displayDAQInfor, DAQInforPrecision = 2;
unsigned int initDAQInfor = 0, DAQTime, inforIndex, inforTimeStep, recentInforIndex;
unsigned int numberEvent[INFORLENGTH], eventSizeHigh[INFORLENGTH], eventSizeLow[INFORLENGTH];
unsigned int recentNumberEvent[RECENTINFORLENGTH], recentEventSize[RECENTINFORLENGTH];
unsigned int numberTrigger[INFORLENGTH], recentNumberTrigger[RECENTINFORLENGTH];
unsigned int totalNumberTrigger, totalNumberTriggerHigh, ntrigger;
double currentDAQRate, averageDAQRate, minimumDAQRate, maximumDAQRate;
double currentTriggerRate, averageTriggerRate, minimumTriggerRate, maximumTriggerRate;
double currentEventSize, averageEventSize, minimumEventSize, maximumEventSize;
double currentDataRate, averageDataRate, minimumDataRate, maximumDataRate;
double measuringTime = 2.0, plotFactorForEventSize = 1.0;
double plotMaxDAQRate, plotMaxEventSize, plotMaxDataRate;
unsigned int plotMinTime = 0, plotMaxTime = 600;
static time_t DAQStartTime;
double maxUintValue = 4294967295.0;

int autoRunFromControlFile = 0, prescaleControl = 0, scanOutputFormat = 0, triggerRate = -1;
char currSetupFile[30] = "currentDAQProfiles.sav";
char parameterFile[30] = "DAQControlpara.txt";
int defaultProfileSaved = 0, canUseVME = TRUE;
extern int TTCviPanelAndButton(void);
extern int InitTTCvi(void);
extern void TTCviEventCounterReset(void);
extern void TTCviBunchCounterReset(void);
extern void TTCviEventAndBunchCounterReset(void);
extern void TTCviSetUnusedTrigger(void);
extern void TTCviRestoreTrigger(void);
extern void TTCviCalibrationRun(void);
extern void TTCviNormalDAQRun(void);
extern void SetTTCviTriggerRate(int *triggerRate);
extern float MeasuredTTCviTriggerRate(int *numberTrigger);
extern void UpTTCviTriggerSelect(void);
extern void TTCviOneShotTriggerIfSelected(void);
extern void SaveMeasuredTriggerRate(float rate);
extern float expectedTriggerRate(void);
extern int InternalSoftwareTrigger(void);

int main(int argc, char *argv[]) {
	int resp;

	gotTTCvi = FALSE;
	measureTriggerRate = FALSE;
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
		if (resp == 0) return -1;
	}
	SetSleepPolicy(VAL_SLEEP_SOME);

	ActivateTCPServer();
	// OK, load the panels now.
	InitUserInterface();
	if (JTAGControlPanelAndButton() < 0) return -1;
	if (DAQControlButton() < 0) return -1;
	if (CSMSetupPanelAndButton() < 0) return -1;
	if (TTCrxSetupPanelAndButton() < 0) return -1;
	if (GOLSetupPanelAndButton() < 0) return -1;
	if (MezzCardSetupPanelAndButton() < 0) return -1;
	if (TTCviPanelAndButton() < 0) return -1;
	if (AutoRunSetupPanelAndButton() <0) return -1;

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
	if (canUseVME) {
		FindJTAGControllerBase();
		InitJTAG();
		SetJTAGRateDivisorButton();
		SetupJTAGChain();
		ResetTAP();
		if (!InitTTCvi()) gotTTCvi = FALSE;
		else {
			gotTTCvi = TRUE;
			measureTriggerRate = TRUE;
		}
	}
	else {
		gotTTCvi = FALSE;
		SetMenuBarAttribute(Menu00Handle, MENU00_TTCVI, ATTR_DIMMED, TRUE);
	}
	DisplayPanel(JTAGControlHandle);
	JTAGControlStartUp();
	DAQControlStartUp();
	LoadSavedProfile(defaultProfileSaved);
	LoopAndExecuteUserButtonAndFunction(Quit);
	SaveParameters(PARAALL);
	DeactivateTCPServer();

	return 0;
}


// User function
void UserFunction(void) {
	static int runStarted = FALSE, stoppingRun, GOLANumber, status, numberTrigger, dimmed, nnodes, ntry;
	static float rate, oldRate = -1.0, time0, ttime0, triggerTime, dtime;
	char string[20];

	UpdateJTAGControlPanelInfor();
	nnodes = NumberConnectedTCPNodes();
	dimmed = FALSE;
	if (runState != State_Idle) dimmed = TRUE;
	SetMenuBarAttribute(Menu00Handle, MENU00_EXE_AUTORUNSETUP, ATTR_DIMMED, dimmed);
	if (nnodes <= 0) {
		getOneEventInitDone = FALSE;
		getOneSequentialEventInitDone = FALSE;
		dimmed = TRUE;
		if ((runState != State_Idle) && (runStartedWithTCPClient > 0)) StopDAQ();
	}
	SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_KILLALLTCPCLIENTS, ATTR_DIMMED, dimmed);
	SetMenuBarAttribute(Menu00Handle, MENU00_EXE_RDIAG, ATTR_DIMMED, dimmed);
	SetMenuBarAttribute(Menu00Handle, MENU00_EXE_RDIAG_GETRAWDATA, ATTR_DIMMED, dimmed);
	SetMenuBarAttribute(Menu00Handle, MENU00_EXE_RDIAG_GETONEEVENT, ATTR_DIMMED, dimmed);
	SetMenuBarAttribute(Menu00Handle, MENU00_EXE_RDIAG_GETONESEQEVENT, ATTR_DIMMED, dimmed);
	if (runState != State_Idle) {
		TTCviOneShotTriggerIfSelected();
		if (runStartedWithTCPClient > 0) {
			if (!runStarted) {
				status = 0;
				runStarted = TRUE;
				stoppingRun = FALSE;
				time0 = (float) Timer();
				ttime0 = (float) Timer();
			}
			// status = CheckClientsStatus();
			dtime = ((float) Timer()) - time0;
			if ((status == 0) && (dtime > 0.2)) {
				time0 = (float) Timer();
				GOLANumber = -1;
				TCPData[TCPCOMMAND] = 0xC0000 | CMDREQDAQINFOR;
				TCPData[TCPLENGTH] = 2;
				if (GOLANumber < 0) status = SendToClients();
				else status = 0;
				ntry = 0;
			}
			else if ((status != 0) && (dtime > 0.05)) {
				time0 = (float) Timer();
				GOLANumber = -1;
				TCPData[TCPCOMMAND] = 0xC0000 | CMDREREQDAQINFOR;
				if (GOLANumber < 0) status = SendToClients();
				else status = 0;
				ntry++;
				if (ntry > 3) status = 0;
			}
			if ((restartRunAfterSecOn == 1) && (DAQTime > restartRunAfterSec)) {
				WaitInMillisecond(100);
				if (!stoppingRun) {
					stoppingRun = TRUE;
					StopDAQ();
				}
			}
		}
	}
	else if (runState == State_Idle) runStarted = FALSE;
	if ((runState != State_Idle) || measureTriggerRate) {
		rate = MeasuredTTCviTriggerRate(&numberTrigger);
		if (rate < 0.0) rate = oldRate;
		if (rate != oldRate) {
			if (runState == State_Running) SaveMeasuredTriggerRate(rate);
			sprintf(string, "%.2fHz", rate);
			ResetTextBox(DAQControlHandle, P_JTAGCTRL_TRIGRATE, string);
			SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_TRIGRATE, ATTR_TEXT_BGCOLOR, VAL_WHITE);
			currentTriggerRate = rate;
			triggerTime = (float) Timer() - ttime0;
			ntrigger += rate * triggerTime;
			ttime0 = (float) Timer();
			if (currentTriggerRate > maximumTriggerRate) maximumTriggerRate = currentTriggerRate;
			if (currentTriggerRate < minimumTriggerRate) minimumTriggerRate = currentTriggerRate;
			if ((totalNumberTrigger+ntrigger) < totalNumberTrigger) totalNumberTriggerHigh++;
			totalNumberTrigger += ntrigger;
			ntrigger = 0;
		}
		oldRate = rate;
	}
	else {
		SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_TRIGRATE, ATTR_TEXT_BGCOLOR, VAL_MAGENTA);
	}
}


// DAQ Control Start Up
void DAQControlStartUp(void) {
	int mezz;

	if (!gotTTCvi) {
		SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, TRUE);
		SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, TRUE);
	}
	else SetDAQRunType();
	for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
		lEdgeOn[mezz] = 0;
		tEdgeOn[mezz] = 0;
		pairOn[mezz] = 0;
	}
	initDAQDone = FALSE;
	getOneEventInitDone = FALSE;
	getOneSequentialEventInitDone = FALSE;
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
	SetupUserButton(SerialNumberHandle, P_SNUMBER_DONE, PanelDone);

	newRunNumber = FALSE;
	DAQControlHandle = JTAGControlHandle;
	// DAQ Control Panel Buttons
	SetupUserButton(Menu00Handle, MENU00_FILE_QUIT, Quit);
	SetupUserButton(Menu00Handle, MENU00_EXE_INITDAQ, InitDAQ);
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
		if (response) {
			if (runState != State_Idle) StopDAQ();  // Stop run if running/paused
			SendTCPCommand(-1, CMDQUIT);
			quitProgram = TRUE;
		}
	}
	else if (controlOption == HELP) {
		MessagePopup("Help on Quit Button",
				"Push this button to quit the CSM DAQ Control program.");
		controlOption = NORM;
	}
	if (!quitProgram) LoopAndExecuteUserButtonAndFunction(Quit);
}


void StartOrStopDAQ(void) {
	if (controlOption == NORM) autoRunOn = FALSE;
	if (runState == State_Idle) StartDAQ();
	else StopDAQ();
}


void StartDAQ(void) {
	int toDisk, resp;
	char fname[256], str[80];

	if (controlOption == NORM) {
		TTCviSetUnusedTrigger();
		if ((!autoRunOn) && InternalSoftwareTrigger()) {
			resp = ConfirmPopup("Confirm on Using Software Trigger!",
					"Are you sure to use software trigger for the DAQ run?\n"
					"  (No == User will select external trigger for the DAQ run.)");
			if (resp == 0) UpTTCviTriggerSelect();
		}
		if (!initDAQDone) InitDAQ();
		while (autoRunFromControlFile == 1) if (GetNewRunSetups(1)) break;
		if (!newRunNumber) {
			runNumber++;
			SaveParameters(PARARUNNUMBER);
		}
		newRunNumber = FALSE;
		GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, &toDisk);
		if (toDisk == 1) {
			if (SetDir("dat") == 0) SetDir("..");
			else MakeDir("dat");
			sprintf(fname, "dat/run%08u_%8s.dat", runNumber, DateStringNoSpace(time(NULL)));
			ReplaceTextBoxLine(DAQControlHandle, P_JTAGCTRL_FILENAME, 0, fname);
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
			printf("Failed to start a run!\n");
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
			TTCviRestoreTrigger();
			ResetTextBox(DAQControlHandle, P_JTAGCTRL_STARTTIME, DateTimeString(time(NULL)));
			ResetTextBox(DAQControlHandle, P_JTAGCTRL_STOPTIME, " ");
			sprintf(str, "%d", runNumber);
			ResetTextBox(DAQInforHandle, P_DAQINFOR_RUNNUMBER, str);
			ResetTextBox(DAQInforHandle, P_DAQINFOR_STARTTIME, DateTimeString(time(NULL)));
			ResetTextBox(DAQInforHandle, P_DAQINFOR_DATETIME, WeekdayTimeString(time(NULL)));
			ResetTextBox(DAQInforHandle, P_DAQINFOR_STOPTIME, " ");
			showErrorReport = FALSE;
			if ((!autoRunOn) && (autoRunFromControlFile == 0)) showErrorReport = displayDAQInfor;
			runStartedWithTCPClient = NumberConnectedTCPNodes();
			runState = State_Running;
			printf("Data acquisition is started and expected trigger rate is %.2f (Hz)\n", expectedTriggerRate());
			SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, FALSE);
			DAQControlInforColor();
		}
	}
	else if (controlOption == HELP) {
		MessagePopup("Help on Start Run Button",
				"Push this button to start run for DAQ.");
		controlOption = NORM;
	}
}


void StopDAQ(void) {
	int GOLANumber, status, ntry;
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
		SendTCPCommand(-1, CMDSTOPRUN);
		GOLANumber = -1;
		time0 = (float) Timer();
		dtime = 0.0;
		while (dtime < 0.2) dtime = ((float) Timer()) - time0;
		time0 = (float) Timer();
		TCPData[TCPCOMMAND] = 0xC0000 | CMDREQALLDAQINFOR;
		TCPData[TCPLENGTH] = 2;
		if (GOLANumber < 0) status = SendToClients();
		else status = 0;
		ntry = 0;
		while ((status != 0) && (ntry < 3)) {
			time0 = (float) Timer();
			dtime = 0.0;
			while (dtime < 0.1) dtime = ((float) Timer()) - time0;
			time0 = (float) Timer();
			GOLANumber = -1;
			TCPData[TCPCOMMAND] = 0xC0000 | CMDREQALLDAQINFOR;
			TCPData[TCPLENGTH] = 2;
			if (GOLANumber < 0) status = SendToClients();
			else status = 0;
			ntry++;
		}

		time0 = (float) Timer();
		dtime = 0.0;
		while (dtime < 0.2) dtime = ((float) Timer()) - time0;
		time0 = (float) Timer();
		TCPData[TCPCOMMAND] = 0xC0000 | CMDREQLEADINGEDGE;
		TCPData[TCPLENGTH] = 2;
		if (GOLANumber < 0) status = SendToClients();
		else status = 0;
		ntry = 0;
		while ((status != 0) && (ntry < 3)) {
			time0 = (float) Timer();
			dtime = 0.0;
			while (dtime < 0.1) dtime = ((float) Timer()) - time0;
			time0 = (float) Timer();
			GOLANumber = -1;
			TCPData[TCPCOMMAND] = 0xC0000 | CMDREQLEADINGEDGE;
			TCPData[TCPLENGTH] = 2;
			if (GOLANumber < 0) status = SendToClients();
			else status = 0;
			ntry++;
		}

		time0 = (float) Timer();
		dtime = 0.0;
		while (dtime < 0.2) dtime = ((float) Timer()) - time0;
		time0 = (float) Timer();
		TCPData[TCPCOMMAND] = 0xC0000 | CMDREQTRAILINGEDGE;
		TCPData[TCPLENGTH] = 2;
		if (GOLANumber < 0) status = SendToClients();
		else status = 0;
		ntry = 0;
		while ((status != 0) && (ntry < 3)) {
			time0 = (float) Timer();
			dtime = 0.0;
			while (dtime < 0.1) dtime = ((float) Timer()) - time0;
			time0 = (float) Timer();
			GOLANumber = -1;
			TCPData[TCPCOMMAND] = 0xC0000 | CMDREQTRAILINGEDGE;
			TCPData[TCPLENGTH] = 2;
			if (GOLANumber < 0) status = SendToClients();
			else status = 0;
			ntry++;
		}

		time0 = (float) Timer();
		dtime = 0.0;
		while (dtime < 0.2) dtime = ((float) Timer()) - time0;
		time0 = (float) Timer();
		TCPData[TCPCOMMAND] = 0xC0000 | CMDREQPAIR;
		TCPData[TCPLENGTH] = 2;
		if (GOLANumber < 0) status = SendToClients();
		else status = 0;
		ntry = 0;
		while ((status != 0) && (ntry < 3)) {
			time0 = (float) Timer();
			dtime = 0.0;
			while (dtime < 0.1) dtime = ((float) Timer()) - time0;
			time0 = (float) Timer();
			GOLANumber = -1;
			TCPData[TCPCOMMAND] = 0xC0000 | CMDREQPAIR;
			TCPData[TCPLENGTH] = 2;
			if (GOLANumber < 0) status = SendToClients();
			else status = 0;
			ntry++;
		}

		if (totalNumberEvent != 0 || totalNumberEventHigh != 0) {
			if (SetDir("dat") == 0) SetDir("..");
			else MakeDir("dat");
			SaveRunLogfile();             // Save run information to a log file
			SaveErrorSummaryFile();
			if (DAQTime > 120) UpErrorReport();
			SaveResultFile();
			HidePanel(ErrorReportHandle);
			SaveScanResults();
			if (autoRunFromControlFile == 0) {
				scanOutputFormat = 0;
				triggerRate = -1;
			}
		}
		ResetTextBox(DAQControlHandle, P_JTAGCTRL_STOPTIME, DateTimeString(time(NULL)));
		ResetTextBox(DAQInforHandle, P_DAQINFOR_STOPTIME, DateTimeString(time(NULL)));
		if (!autoRunOn) autoRunFromControlFile = 0;
		printf("Data acquisition is stopped!\n");
		DAQControlInforColor();
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
	int panel;

	if (controlOption == NORM) {
		panel = GetActivePanel();
		if (panel == SelectActionHandle) PanelDone();
		if (runState != State_Idle) StopDAQ();     // Stop run if running/paused
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
		SendTCPCommand(-1, CMDINITDAQ);
		if (gotTTCvi) {
			TTCviEventCounterReset();
			TTCviBunchCounterReset();
			TTCviEventAndBunchCounterReset();
		}
		initDAQDone = TRUE;
		printf("DAQ initialization done.\n");
	}
	else if (controlOption == HELP) {
		MessagePopup("Help on INITDAQ Button",
				"Push this button to perform DAQ initialization.");
		controlOption = NORM;
	}
}


void SetDAQRunType(void) {
	int runType;

	if (controlOption == NORM) {
		if (!gotTTCvi) return;
		GetCtrlVal(DAQControlHandle, P_JTAGCTRL_RUNTYPE, &runType);
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


void DisconnectAllTCPClients(void) {
	int resp;

	if (controlOption == NORM) {
		resp = ConfirmPopup("Confirm to Disconnect All TCP Clients",
				"Are you sure to disconnect all TCP clients?\n");
		if (resp != 0) {
			if (runState != State_Idle) StopDAQ();     // Stop run if running/paused
			WaitInMillisecond(200);
			if (SendTCPCommand(-1, CMDQUIT) != 0) DisconnectSockets();
		}
	}
	else if (controlOption == HELP) {
		MessagePopup("Help on Disconnect All TCP Clients",
				"Use this button to disconnect all TCP clients if confirmed.");
		controlOption = NORM;
	}
}


void GetRawData(void) {
}


void GetOneEvent(void) {
	int GOLANumber, status, ntry, first;
	double time0, dtime;

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
	}
	time0 = (float) Timer();
	dtime = 0.0;
	while (dtime < 0.2) dtime = ((float) Timer()) - time0;
	TCPData[TCPCOMMAND] = 0xC0000 | CMDREQONEEVENT;
	TCPData[TCPLENGTH] = 2;
	if (GOLANumber < 0) status = SendToClients();
	else status = 0;
	ntry = 0;
	while ((status != 0) && (ntry < 3)) {
		time0 = (float) Timer();
		dtime = 0.0;
		while (dtime < 0.1) dtime = ((float) Timer()) - time0;
		time0 = (float) Timer();
		GOLANumber = -1;
		TCPData[TCPCOMMAND] = 0xC0000 | CMDREREQONEEVENT;
		TCPData[TCPLENGTH] = 2;
		if (GOLANumber < 0) status = SendToClients();
		else status = 0;
		ntry++;
	}
	first = TRUE;
	while ((status == 0) && (!eventReady)) {
		if (first) {
			first = FALSE;
			TTCviRestoreTrigger();
		}
		GetOneEvent();
	}
	if (eventReady) TTCviSetUnusedTrigger();
	else TTCviRestoreTrigger();
	SetActivePanel(DAQControlHandle);
}


void GetOneSequentialEvent(void) {
	int GOLANumber, status, ntry, first;
	double time0, dtime;

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
	}
	time0 = (float) Timer();
	dtime = 0.0;
	while (dtime < 0.2) dtime = ((float) Timer()) - time0;
	TCPData[TCPCOMMAND] = 0xC0000 | CMDREQONESEQEVENT;
	TCPData[TCPLENGTH] = 2;
	if (GOLANumber < 0) status = SendToClients();
	else status = 0;
	ntry = 0;
	while ((status != 0) && (ntry < 3)) {
		time0 = (float) Timer();
		dtime = 0.0;
		while (dtime < 0.1) dtime = ((float) Timer()) - time0;
		time0 = (float) Timer();
		GOLANumber = -1;
		TCPData[TCPCOMMAND] = 0xC0000 | CMDREREQONESEQEVENT;
		TCPData[TCPLENGTH] = 2;
		if (GOLANumber < 0) status = SendToClients();
		else status = 0;
		ntry++;
	}
	first = TRUE;
	while ((status == 0) && (!eventReady)) {
		if (first) {
			first = FALSE;
			TTCviRestoreTrigger();
		}
		GetOneSequentialEvent();
	}
	if (eventReady) TTCviSetUnusedTrigger();
	else TTCviRestoreTrigger();
	SetActivePanel(DAQControlHandle);
}


int SendTCPCommand(int GOLANumber, int command) {
	int version, nbEvent, toDisk, length, i, data, status, enables, on;
	float trigRate;
	char fname[256];

	if (!toTCPClients) return 0;
	if ((command != CMDREQONEEVENT) && (command != CMDREREQONEEVENT)) getOneEventInitDone = FALSE;
	if ((command != CMDREQONESEQEVENT) && (command != CMDREREQONESEQEVENT)) getOneSequentialEventInitDone = FALSE;
	SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, TRUE);
	SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, TRUE);
	SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, TRUE);
	TCPData[TCPCOMMAND] = 0xC0000 | (command & 0xFFFF);
	TCPData[TCPNUMBERTCPNODES] = NumberConnectedTCPNodes();
	BinaryToInt(&version, CSMVERSION, 12, CSMStatusArray);
	TCPData[TCPCSMVERSION] = version;
	GetMezzCardEnables();
	TCPData[TCPMEZZENABLES] = mezzEnables;
	enables = 0;
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
	trigRate = expectedTriggerRate();
	if (command == CMDGETONEEVENTINIT) {
		TCPData[TCPGOLACARDPAGESIZE] = 4;
		if (trigRate >= 100.0) TCPData[TCPGOLACARDPAGESIZE] = 4;
		else if (trigRate > 0.0) TCPData[TCPGOLACARDPAGESIZE] = 3;
	}
	else {
		TCPData[TCPGOLACARDPAGESIZE] = 6;
		if (trigRate >= 200.0) TCPData[TCPGOLACARDPAGESIZE] = 6;
		else if (trigRate >= 50.0) TCPData[TCPGOLACARDPAGESIZE] = 5;
		else if (trigRate > 0.0) TCPData[TCPGOLACARDPAGESIZE] = 4;
	}
	TCPData[TCPAMTTYPE] = TYPEAMT3;
	TCPData[TCPRUNNUMBER] = runNumber;
	nbEvent = 0;
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, &on);
	if (on) GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPATEVENT, &nbEvent);
	TCPData[TCPNUMBEROFEVENT] = nbEvent;
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
	if (GOLANumber < 0) status = SendToClients();
	else status = 0;
	if (status > 0) {
		printf("Error in sending out TCP command, try again\n");
		if (GOLANumber < 0) status = SendToClients();
		else status = 0;
	}
	SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, FALSE);
	SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, FALSE);
	if (runState != State_Idle)
		SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_PAUSEORRESUMEDAQ, ATTR_DIMMED, FALSE);
	if (status < 0) status = 0;
	return status;
}


int WaitForTCPClientReply(void) {
	int status, start;

	//  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_INITDAQ, ATTR_DIMMED, FALSE);
	//  SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_STARTORSTOPDAQ, ATTR_DIMMED, FALSE);
	start = (int) time(NULL);
	status = CheckClientsStatus();
	while ((status == 11) && ((((int) time(NULL)) - start) < 4)) {
		WaitInMillisecond(1);   // Wait is necessary, otherwise may get fake TCP_DATAREADY signal
		HandleUserButtons(Quit);
		status = CheckClientsStatus();
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
		status = CheckClientsStatus();
		color = VAL_GREEN;
		strcpy(str, "TX Success");
		if (status < 0) {
			color = VAL_MAGENTA;
			strcpy(str, "NoConnection");
		}
		else if (status > 0) {
			color = VAL_RED;
			if (status == 1) strcpy(str, "Timeout");
			else if (status == 11) strcpy(str, "No Reply");
			else if (status < 5) strcpy(str, "TX Failed");
			else strcpy(str, "ClientSTSErr");
		}
	}
	ReplaceTextBoxLine(DAQControlHandle, P_JTAGCTRL_TCPSTATUS, 0, str);
	SetCtrlAttribute(DAQControlHandle, P_JTAGCTRL_TCPSTATUS, ATTR_TEXT_BGCOLOR, color);
	SetCtrlVal(DAQControlHandle, P_JTAGCTRL_NBTCPNODES, NumberConnectedTCPNodes());
}


void HandleVariesData(unsigned int data[]) {
	int command, mezz, ch;

	dataType = data[1] & DATATYPEBITMASK;
	command = data[3] & 0xFFFF;
	if (dataType == DATATYPEDAQINFOR) CollectDAQInfor(data);
	else if (dataType == DATATYPEALLDAQINFOR) CopyDAQInfor(data);
	else if (dataType == DATATYPERAWDATA) PrintOutData(data, NULL);
	else if (dataType == DATATYPERAWDATACONT) PrintOutData(data, NULL);
	else if (dataType == DATATYPEEVENT) PrintOutData(data, NULL);
	else if (dataType == DATATYPEEVENTCONT) PrintOutData(data, NULL);
	else if (dataType == DATATYPELEADINGEDGE) {
		for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
			nEvents[mezz] = data[4+mezz];
			for (ch = 0; ch < 24; ch++) nEdge[0][ch][mezz] = data[4+MAXNUMBERMEZZANINE+24*mezz+ch];
		}
	}
	else if (dataType == DATATYPETRAILINGEDGE) {
		for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
			nEvents[mezz] = data[4+mezz];
			for (ch = 0; ch < 24; ch++) nEdge[1][ch][mezz] = data[4+MAXNUMBERMEZZANINE+24*mezz+ch];
		}
	}
	else if (dataType == DATATYPEPAIR) {
		for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
			nEvents[mezz] = data[4+mezz];
			for (ch = 0; ch < 24; ch++) nPair[ch][mezz] = data[4+MAXNUMBERMEZZANINE+24*mezz+ch];
		}
	}
	toTCPClients = FALSE;
	if (command == CMDPAUSERUN) PauseDAQ();
	else if (command == CMDRESUMERUN) ResumeDAQ();
	else if ((command == CMDSTOPRUN) && (runState != State_Idle)) {
		printf("Stop run has been requested by Client(s)!");
		StopDAQ();
	}
	toTCPClients = TRUE;
}


void PrintOutData(unsigned int *data, FILE *file) {
	static int i, index;

	eventReady = TRUE;
	if ((dataType == DATATYPERAWDATA) || (dataType == DATATYPEEVENT)) {
		index = 0;
		if (file == NULL) printf("  Index   dataWord\n");
		else fprintf(file, "  Index   dataWord\n");
	}
	for (i = 4; i < data[0]; i++) {
		if (file == NULL) printf(" %6d 0x%08x  ", index, data[i]);
		else fprintf(file, " %6d 0x%08x  ", index, data[i]);
		DataInterpretation(data[i], file);
		index++;
	}
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


// Routines for Serial Number
//
// Bring up the Serial Number Panel
void UpSerialNumber(void) {
	int mezz, snButton[MAXNUMBERMEZZANINE], dimmed, mezzSNDone;

	if (controlOption == NORM) {
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
		SetCtrlVal(SerialNumberHandle, P_SNUMBER_EXPECTEDMEZZCARD, nbMezzCard);
		for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
			if (snButton[mezz] >= 0) {
				if (((mezzEnables>>mezz)&1) == 1) dimmed = FALSE;
				else dimmed = TRUE;
				SetCtrlAttribute(SerialNumberHandle, snButton[mezz], ATTR_DIMMED, dimmed);
			}
		}
		DisplayPanel(SerialNumberHandle);
		SetActivePanel(SerialNumberHandle);
		mezzSNDone = FALSE;
		while (!mezzSNDone) mezzSNDone = HandleUserButtons(PanelDone);
	}
	else if (controlOption == HELP)
		MessagePopup("Help on Serial Number Panel",
				"Bring up Serial Number Controls.");
	controlOption = NORM;
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
	int color = VAL_RED, count, version, val;
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
	else if (counterIndex == NREQUESTEDPAUSE) color = VAL_GREEN;
	else color = VAL_RED;
	if (counterIndex == BCIDMISMATCH) {
		if (nbMezzCard == 1 && validBCIDPresetValue == 0) color = VAL_BLACK;
		else if (nbMezzCard <= 0) color = VAL_BLACK;
	}
	if (counterIndex == TDCBCIDMISMATCH && nbMezzCard <= 1) color = VAL_BLACK;
	else if (counterIndex == CSMBCIDMISMATCH && validBCIDPresetValue == 0) color = VAL_BLACK;
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
			color = VAL_GREEN;
		}
		else if ((counterIndex == CSMTRAILEREVIDMISMATCH) ||(counterIndex == NBADCYCLE)) {
			SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# Bad Cycle");
			counterIndex = NBADCYCLE;
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
		else if (counterIndex == CSMABORTEVIDMISMATCH) color = VAL_BLACK;
		else if (counterIndex == CSMABORT) color = VAL_BLACK;
		else if (counterIndex == MISSCSMHEADER) color = VAL_BLACK;
		else if (counterIndex == EXTRACSMHEADER) color = VAL_BLACK;
		else if (counterIndex == MISSCSMTRAILER) color = VAL_BLACK;
		else if (counterIndex == EXTRACSMTRAILER) color = VAL_BLACK;
	}
	else {
		if ((counterIndex == CSMWORDCOUNTERERR) || (counterIndex == TXPARITYERROR)) {
			SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "CSM WordCount Error");
			counterIndex = CSMWORDCOUNTERERR;
			SetCtrlVal(ErrorReportHandle, panelIndex, count);
		}
		else if ((counterIndex == CSMHEADEREVIDMISMATCH) || (counterIndex == NBUFFERPOINTERERROR)) {
			SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# CSMhdr EVID Error");
			counterIndex = CSMHEADEREVIDMISMATCH;
			SetCtrlVal(ErrorReportHandle, panelIndex, count);
		}
		else if ((counterIndex == CSMERROR) || (counterIndex == NEMPTYCYCLE)) {
			SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# CSM Error");
			counterIndex = CSMERROR;
			SetCtrlVal(ErrorReportHandle, panelIndex, count);
		}
		else if ((counterIndex == CSMTRAILEREVIDMISMATCH) ||(counterIndex == NBADCYCLE)) {
			SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# CSMtrl EVID Error");
			counterIndex = CSMTRAILEREVIDMISMATCH;
			SetCtrlVal(ErrorReportHandle, panelIndex, count);
		}
		else if ((counterIndex == CSMBCIDMISMATCH) || (counterIndex == DISCARDEDEVENT)) {
			SetCtrlAttribute(ErrorReportHandle, panelIndex, ATTR_LABEL_TEXT, "# CSM BCID Error");
			counterIndex = CSMBCIDMISMATCH;
			SetCtrlVal(ErrorReportHandle, panelIndex, count);
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
	strcpy(name[TDCUNEXPECTEDDATA], "Number of unexpected TDC data word");
	for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
		sprintf(name[TDCCONTSAMEEDGEINTDC+i], "Number of continued same edge in TDC %d", i);
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
	strcpy(name[NREQUESTEDPAUSE], "Number of Requested Pause Run");
	strcpy(name[NTOTALWORD], "Number of Total Received Words");
	strcpy(name[PROCESSEDEVENT], "Number of Processed Events");
	strcpy(name[NDATAWORD], "Number of Data Words");
	strcpy(name[NEVENTWORD], "Number of Data Words for Constructed Events");
	strcpy(name[NGOODCYCLE], "Number of Good Cycles");
	strcpy(name[NEMPTYCYCLE], "Number of Empty Cycles");
	strcpy(name[NBADCYCLE], "Number of Bad Cycles");
	strcpy(name[CURREVENTSIZE], "Current Event Size");
	strcpy(name[MINEVENTSIZE], "Minimum Event Size");
	strcpy(name[MAXEVENTSIZE], "Maximum Event Size");
	strcpy(name[NBUFFERPOINTERERROR], "Number Buffer Pointer Error");
	if ((version%2) == 1) {
		strcpy(name[TXPARITYERROR], "Number of Parity Error (CSM1 to GOLACard)");
		for (i = 0; i < MAXNUMBERMEZZANINE; i++)
			sprintf(name[TXPARITYERRORINTDC+i], "Number of Parity Error (CSM1 to GOLACard) in TDC %d", i);
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
		fprintf(sumFile, " -------- MiniDAQ Error Summary File -------------\n");
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
	double ratio, evtSum, nevtSum;
	int i, j, k, nlist, value, asd, version, enable, numberOfLockLost, linkA, linkB, linkC;
	char dataFilename[281], logFilename[281], str[80];
	FILE *logFile;

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
		fprintf(logFile, " CSM1 Program Version Number: %54c 0x%03X\n", ' ', version);
		fprintf(logFile, " Run Number                 : %60u\n", runNumber);
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
					sprintf(str, "%d (DAC Value = %d)", 2*(value-127), value);
					fprintf(logFile, "   ASD%d Main Threshold (mV) : %60s\n", asd, str);
				}
				BinaryToInt(&value, AMTS_SETUP+AMTS_CH0, 24, statusSetupCheck);
				fprintf(logFile, "   AMT Channel Enable Mask  : %49c 0x%08X\n", ' ', value);
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
		GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_VERTEXII, &enable);
		fprintf(logFile, " CSM (VertexII) Enable      : %60d\n", enable);
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
		GetCtrlVal(JTAGControlHandle, P_JTAGCTRL_VERTEXII, &enable);
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
		if (triggerRate >= 0)
			fprintf(logFile, " TTCvi Trigger Rate (Hz)    : %60d\n", triggerRate);
		sprintf(str, "%14.4f %14.4f %14.4f", averageTriggerRate, minimumTriggerRate, maximumTriggerRate);
		fprintf(logFile, " TrigRate (Hz) (Ave Min Max): %60s\n", str);
		sprintf(str, "%14.4f %14.4f %14.4f", averageDAQRate, minimumDAQRate, maximumDAQRate);
		fprintf(logFile, " DAQ Rate (Hz) (Ave Min Max): %60s\n", str);
		sprintf(str, "%14.4f %14.0f %14.0f", averageEventSize, minimumEventSize, maximumEventSize);
		fprintf(logFile, " Event Size    (Ave Min Max): %60s\n", str);
		GetTextBoxLine(DAQControlHandle, P_JTAGCTRL_DAQTIME, 0, str);
		fprintf(logFile, " DAQ Time                   : %60s\n", str);
		fprintf(logFile, " Run Stopped Time           : %60s\n", WeekdayTimeString(time(NULL)));
		fclose(logFile);
		printf("Run information has been saved into logfile <%s>.\n", logFilename);
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
	unsigned int nhit, scanFileOpened, asd, numberEvents, snButton[MAXNUMBERMEZZANINE];
	int i, mezz, ch, length, type, thresh[3], AMTMask, ASDMask, exist, value;
	char dataFilename[281], scanFilename[281], ctype[10], initStatus[10];
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
				strcpy(scanFilename, "dat/scan_");
				GetTextBoxLine(SerialNumberHandle, snButton[mezz], 0, &scanFilename[9]);
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
						fprintf(scanFile, "//---------- MiniDAQ Scan Summary File --------------\n");
						fprintf(scanFile, "//\n");
						fprintf(scanFile, "// TDC            : TDC Number (= CSM0 input channel number)\n");
						fprintf(scanFile, "// RunNumber      : Run Number\n");
						fprintf(scanFile, "// #OfEvents      : Number of Events\n");
						fprintf(scanFile, "// Init           : Individual Mezzanine Card Initialization Status\n");
						fprintf(scanFile, "// ASDsThresh(mV) : ASD1 ASD2 ASD3 thresholds, unit in mV\n");
						fprintf(scanFile, "// AMTChE         : 24 AMT Channel Enable/Disable Mask (Hexdecimal)\n");
						fprintf(scanFile, "// ASDChE         : 24 ASD Calibration Channel Enable/Disable Mask (Hexdecimal)\n");
						fprintf(scanFile, "// Type           : Hit Type  LEdge = Leading Edge\n");
						fprintf(scanFile, "//                            TEdge = Trailing Edge\n");
						fprintf(scanFile, "//                             Pair = Pair\n");
						fprintf(scanFile, "//                             None = Unknown Type, something wrong!!!\n");
						fprintf(scanFile, "// #HitsChxx      : Number of hits for channel xx, xx from 0 to 23\n");
						fprintf(scanFile, "// DateTime       : DAQ end date and time\n");
						fprintf(scanFile, "//\n");
						fprintf(scanFile, "TDC");
						fprintf(scanFile, " RunNumber");
						fprintf(scanFile, " #OfEvents");
						fprintf(scanFile, " Init");
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
				else printf("Unable to open scan summary file <%s> for the run!\n", scanFilename);
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
					fprintf(scanFile, " %7d, %7d, %7d,", thresh[0], thresh[1], thresh[2]);
					fprintf(scanFile, " %06X,", AMTMask);
					fprintf(scanFile, " %06X,", ASDMask);
					fprintf(scanFile, " %5s,", ctype);
					for (ch = 0; ch < 24; ch++) {
						if (type == 0) nhit = nEdge[1][ch][mezz];
						else if (type == 1) nhit = nEdge[0][ch][mezz];
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
					fprintf(scanFile, " %4d %4d %4d", thresh[0], thresh[1], thresh[2]);
					fprintf(scanFile, " %06X", AMTMask);
					fprintf(scanFile, " %06X", ASDMask);
					fprintf(scanFile, " %5s", ctype);
					for (ch = 0; ch < 24; ch++) {
						if (type == 0) nhit = nEdge[1][ch][mezz];
						else if (type == 1) nhit = nEdge[0][ch][mezz];
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
	if (NumberConnectedTCPNodes() <= 0) colorCode = VAL_MAGENTA;
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
				"If enabled, basical DAQ information will be displayed.");
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
	static int lastEventID, tdc, cha;
	static double dsum, nevtSum, maxUintValue = 4294967296.0;
	static time_t savedTimeForDisplay;
	static char string[20], sprecision[10];

	if (initDAQInfor == 1) {  // Initialization
		initDAQInfor = 0;
		for (i = 0; i < NUMBERANALYSISCOUNTER; i++) analysisCounter[i] = 0;
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
			averageTriggerRate = nevtSum / ((double) DAQTime);
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
					if (currentEventSize <= 0.0)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_CURR, ATTR_TEXT_BGCOLOR, VAL_RED);
					else if (runState == State_Running)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_CURR, ATTR_TEXT_BGCOLOR, VAL_GREEN);
					sprintf(string, sprecision, averageEventSize);
					ResetTextBox(DAQInforHandle, P_DAQINFOR_EVENTSIZE_AVER, string);
					if (averageEventSize <= 0.0)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_AVER, ATTR_TEXT_BGCOLOR, VAL_RED);
					else if (runState == State_Running)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_AVER, ATTR_TEXT_BGCOLOR, VAL_GREEN);
					if (averageEventSize > 0)
						sprintf(string, "%.0f", minimumEventSize);
					else
						sprintf(string, "%.0f", 0.0);
					ResetTextBox(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MIN, string);
					if (minimumEventSize <= 0.0 || minimumEventSize > maximumEventSize)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MIN, ATTR_TEXT_BGCOLOR, VAL_RED);
					else if (runState == State_Running)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MIN, ATTR_TEXT_BGCOLOR, VAL_GREEN);
					sprintf(string, "%.0f", maximumEventSize);
					ResetTextBox(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MAX, string);
					if (maximumEventSize <= 0.0)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MAX, ATTR_TEXT_BGCOLOR, VAL_RED);
					else if (runState == State_Running)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_EVENTSIZE_MAX, ATTR_TEXT_BGCOLOR, VAL_GREEN);
					sprintf(string, sprecision, currentDataRate);
					ResetTextBox(DAQInforHandle, P_DAQINFOR_DATARATE_CURR, string);
					if (currentDataRate <= 0.0)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_CURR, ATTR_TEXT_BGCOLOR, VAL_RED);
					else if (runState == State_Running)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_CURR, ATTR_TEXT_BGCOLOR, VAL_GREEN);
					sprintf(string, sprecision, averageDataRate);
					ResetTextBox(DAQInforHandle, P_DAQINFOR_DATARATE_AVER, string);
					if (averageDataRate <= 0.0)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_AVER, ATTR_TEXT_BGCOLOR, VAL_RED);
					else if (runState == State_Running)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_AVER, ATTR_TEXT_BGCOLOR, VAL_GREEN);
					sprintf(string, sprecision, minimumDataRate);
					ResetTextBox(DAQInforHandle, P_DAQINFOR_DATARATE_MIN, string);
					if (minimumDataRate <= 0.0)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_MIN, ATTR_TEXT_BGCOLOR, VAL_RED);
					else if (runState == State_Running)
						SetCtrlAttribute(DAQInforHandle, P_DAQINFOR_DATARATE_MIN, ATTR_TEXT_BGCOLOR, VAL_GREEN);
					sprintf(string, sprecision, maximumDataRate);
					ResetTextBox(DAQInforHandle, P_DAQINFOR_DATARATE_MAX, string);
					if (maximumDataRate <= 0.0)
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
			if ((counter[4+i] > 0) && ((analysisCounter[i] == 0) || (analysisCounter[i] > counter[4+i])))
				analysisCounter[i] = counter[4+i];
		}
		else if (i == MAXEVENTSIZE) {
			if (analysisCounter[i] < counter[4+i]) analysisCounter[i] = counter[4+i];
		}
		else {
			analysisCounter[i] += counter[4+i];
		}
	}
	if (bigOR == 0) return;
	nbEvents = counter[4+PROCESSEDEVENT];
	nWords = counter[4+NEVENTWORD];
	nevent += (float) nbEvents;
	if ((totalNumberEvent+nbEvents) < totalNumberEvent) totalNumberEventHigh++;
	totalNumberEvent += nbEvents;
	eventID = analysisCounter[CURREVENTID];
	currentEventSize = analysisCounter[CURREVENTSIZE];
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


// Auto Run Routines
//
// Load Auto Run Setup Panel and define user buttons
int AutoRunSetupPanelAndButton(void) {
	// Define the JTAG diagnostics panel handles
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
			HandleUserButtons(Quit);
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
		SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_EVTERRRESETDAQON, TRUE);
		SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_SAVEALLREGISTER, TRUE);
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
		SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_EVTERRRESETDAQON, FALSE);
		SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_SAVEALLREGISTER, FALSE);
	}
	else if (controlOption == HELP)
		MessagePopup("Help on Auto Run Setup Panel Buttons",
				"Turn off all controls on Auto Run Setup Panel.");
	controlOption = NORM;
}


void GetAutoRunControls(void)
{
	static unsigned int toDisk, savedNTotalEvt = 0, savedNEvt = 0, savedHangTime = 0;
	static unsigned int savedResetErrEvt = 0, savedStopNErr = 0;
	static unsigned int savedNTotalRun = 0, savedRestartTime = 0;

	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, &stopAfterNRunOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, &stopRunAtEventOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTEVENTON, &restartRunAtEventOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, &restartRunAfterSecOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTHANGON, &restartRunAtHangTimeOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_EVTERRRESETDAQON, &resetDAQAtEventErrorOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_SAVEALLREGISTER, &autoSaveRegisterOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERNERRON, &stopAfterNErrorOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNS, &stopAfterNRun);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPATEVENT, &stopRunAtEvent);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTATEVENT, &restartRunAtEvent);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSEC, &restartRunAfterSec);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTHANGTIME, &restartRunAtHangTime);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_EVTERRRESETDAQ, &resetDAQAtEventError);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERNERR, &stopAfterNError);
	if (savedNTotalRun == 0) savedNTotalRun = stopAfterNRun;
	if (savedNTotalEvt == 0) savedNTotalEvt = stopRunAtEvent;
	if (savedNEvt == 0) savedNEvt = restartRunAtEvent;
	if (savedRestartTime == 0) savedRestartTime = restartRunAfterSec;
	if (savedHangTime == 0) savedHangTime = restartRunAtHangTime;
	if (savedResetErrEvt == 0) savedResetErrEvt = resetDAQAtEventError;
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
	if (savedResetErrEvt != resetDAQAtEventError)
		SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_EVTERRRESETDAQON, TRUE);
	if (savedStopNErr != stopAfterNError)
		SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERNERRON, TRUE);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, &stopAfterNRunOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, &stopRunAtEventOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTEVENTON, &restartRunAtEventOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, &restartRunAfterSecOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTHANGON, &restartRunAtHangTimeOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_EVTERRRESETDAQON, &resetDAQAtEventErrorOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_SAVEALLREGISTER, &autoSaveRegisterOn);
	GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERNERRON, &stopAfterNErrorOn);
	if (stopAfterNErrorOn == 1)
		SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_EVTERRRESETDAQON, FALSE);
	savedNTotalRun = stopAfterNRun;
	savedNTotalEvt = stopRunAtEvent;
	savedNEvt = restartRunAtEvent;
	savedRestartTime = restartRunAfterSec;
	savedHangTime = restartRunAtHangTime;
	savedResetErrEvt = resetDAQAtEventError;
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
	int numberValidRun = 0;

	if (controlOption == NORM) {
		if (FileSelectPopup("", "*.ctrl", "", "Select Automatic Run Control File",
					VAL_LOAD_BUTTON, 0, 1, 1, 0, controlFilePath)) {
			autoRunFromControlFile = 1;
			scanOutputFormat = 0;
			while (autoRunFromControlFile == 1) if (GetNewRunSetups(0) == 1) numberValidRun++;
			if (numberValidRun == 0)
				MessagePopup("No Valid Run",
						"No valid run was found in selected Run Control File!\n"
						"Please check Run Control File to make sure it is OK!");
			else {
				autoRunFromControlFile = 1;
			}
		}
	}
	else if (controlOption == HELP)
		MessagePopup("Help on Auto Run Setup Panel Buttons",
				"Select automatic run control file.");
	controlOption = NORM;
}


int GetNewRunSetups(int downloadSetups)
{
	static unsigned int numberEvent, DAQTimeInSeconds, trigLimit, evtPFactorCtrl,
						storageCtrl, analysisCtrl, DAQInforCtrl, TDCEnableMask;
	static int validProfile, fileOpened = 0, stopAtEventOn, stopAtTimeOn, preFlag = 0;
	static char str[256];
	static FILE *ctrlFile;
	char profileName[256], str1[256], cstr[40], *strp;
	int i, j, readString, strl, gotRunPara[20], paraIndex, gotTheRun, firstStop, initMezz;
	int mezz, matchWindow, AMTChannel, ASDChannel, ASDThreshold, ASD1Threshold, ASD2Threshold,
		ASD3Threshold, mezzCopyMask, AMTChannelCopyMask, ASDChannelCopyMask, thresholdCopyMask;
	extern unsigned int stopRunAtEvent, restartRunAfterSec;
	extern int AutoRunSetupHandle;

	readString = 0;
	strcpy(profileName, "");
	initMezz = FALSE;
	matchWindow = -99999;
	AMTChannel = -99999;
	ASDChannel = -99999;
	ASDThreshold = -99999;
	ASD1Threshold = -99999;
	ASD2Threshold = -99999;
	ASD3Threshold = -99999;
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
		validProfile = 1;
		//Set default Values
		evtPFactorCtrl = prescaleControl;
		GetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, &storageCtrl);
		GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, &stopAtEventOn);
		GetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, &stopAtTimeOn);
		if (stopAtEventOn) numberEvent = stopRunAtEvent;
		if (stopAtTimeOn) DAQTimeInSeconds = restartRunAfterSec;
		if (!stopAtEventOn && !stopAtTimeOn) {
			DAQTimeInSeconds = 3600;
			stopAtTimeOn = 1;
		}
		if (ctrlFile = fopen(controlFilePath, "r")) {
			fileOpened = 1;
			readString = 1;
		}
		else printf("Unable to open automatic run control file <%s>\n", controlFilePath);
	}
	if (fileOpened == 1) {
		firstStop = TRUE;
		for (i = 0; i < 20; i++) gotRunPara[i] = 0;
		gotTheRun = 0;
		strcpy(profileName, "");
		while (TRUE) {
			if (readString == 1) {
				while (TRUE) {
					if ((str[0] = fgetc(ctrlFile)) == EOF) {
						autoRunFromControlFile = 0;
						fclose(ctrlFile);
						fileOpened = 0;
						for (i = 0; i < 10; i++) if (gotRunPara[i] == 1) gotTheRun = 1;
						if (gotTheRun == 0) validProfile = 0;
						break;
					}
					else if (str[0] != ' ' && str[0] != '\n') {
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
				paraIndex = 0;
				strcpy(cstr, "run number");
				strp = strstr(str1, cstr);
				if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
					gotRunPara[paraIndex] = 1;
					strl = strlen(cstr);
					for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
					j = 0;
					while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++;
					for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
					sscanf(str, "%u", &runNumber);
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 1;
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
					sscanf(str, "%u", &numberEvent);
					stopAtEventOn = 1;
					if (firstStop) {
						firstStop = FALSE;
						stopAtTimeOn = 0;
					}
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 2;
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
					if (DAQTimeInSeconds < 30) DAQTimeInSeconds = 30;
					stopAtTimeOn = 1;
					if (firstStop) {
						firstStop = FALSE;
						stopAtEventOn = 0;
					}
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 3;
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

				paraIndex = 4;
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

				paraIndex = 5;
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
					if (storageCtrl > 0) storageCtrl = 1;
					else storageCtrl = 0;
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 6;
				strcpy(cstr, "data analysis");
				if (strstr(str1, cstr) == NULL) strcpy(cstr, "data monitor");
				strp = strstr(str1, cstr);
				if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
					gotRunPara[paraIndex] = 1;
					strl = strlen(cstr);
					for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
					j = 0;
					while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++;
					for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
					sscanf(str, "%u", &analysisCtrl);
					if (analysisCtrl > 0) analysisCtrl = 1;
					else analysisCtrl = 0;
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 7;
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

				paraIndex = 8;
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
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 9;
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

				paraIndex = 10;
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
					if (matchWindow < 0) matchWindow = 0;
					else if (matchWindow > 4095) matchWindow = 4095;
					initMezz = TRUE;
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 11;
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

				paraIndex = 12;
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
					initMezz = TRUE;
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 13;
				strcpy(cstr, "asd threshold (mv)");
				if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd threshold(mv)");
				if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd threshold");
				strp = strstr(str1, cstr);
				if ((strp != NULL) && (gotRunPara[paraIndex] == 0)) {
					gotRunPara[paraIndex] = 1;
					strl = strlen(cstr);
					for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
					j = 0;
					while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++;
					for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
					sscanf(str, "%d", &ASDThreshold);
					initMezz = TRUE;
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 14;
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
					initMezz = TRUE;
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 15;
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
					initMezz = TRUE;
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

				paraIndex = 16;
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
					initMezz = TRUE;
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

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
					strl = strlen(cstr);
					for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
					j = 0;
					while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++;
					for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
					sscanf(str, "%x", &mezzCopyMask);
					if (mezzCopyMask != 0) initMezz = TRUE;
				}

				strcpy(cstr, "amt channel copy mask (hex)");
				if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel copy mask(hex)");
				if (strstr(str1, cstr) == NULL) strcpy(cstr, "amt channel copy mask");
				strp = strstr(str1, cstr);
				if (strp != NULL) {
					strl = strlen(cstr);
					for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
					j = 0;
					while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++;
					for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
					sscanf(str, "%x", &AMTChannelCopyMask);
					if (AMTChannelCopyMask != 0) initMezz = TRUE;
				}

				strcpy(cstr, "asd channel copy mask (hex)");
				if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel copy mask(hex)");
				if (strstr(str1, cstr) == NULL) strcpy(cstr, "asd channel copy mask");
				strp = strstr(str1, cstr);
				if (strp != NULL) {
					strl = strlen(cstr);
					for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
					j = 0;
					while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++;
					for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
					sscanf(str, "%x", &ASDChannelCopyMask);
					if (ASDChannelCopyMask != 0) initMezz = TRUE;
				}

				strcpy(cstr, "threshold copy mask (hex)");
				if (strstr(str1, cstr) == NULL) strcpy(cstr, "threshold copy mask(hex)");
				if (strstr(str1, cstr) == NULL) strcpy(cstr, "threshold copy mask");
				strp = strstr(str1, cstr);
				if (strp != NULL) {
					strl = strlen(cstr);
					for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
					j = 0;
					while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++;
					for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
					sscanf(str, "%x", &thresholdCopyMask);
					if (thresholdCopyMask != 0) initMezz = TRUE;
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
					sscanf(str, "%d", &scanOutputFormat);
				}

				paraIndex = 17;
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
					sscanf(str, "%d", &triggerRate);
					if (triggerRate < 0) triggerRate = 0;
					if (!gotTTCvi) triggerRate = -1;
				}
				else if ((strp != NULL) && (gotRunPara[paraIndex] == 1)) gotTheRun = 1;

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

	if (downloadSetups && triggerRate >= 0) SetTTCviTriggerRate(&triggerRate);
	if (validProfile && downloadSetups) {    // Setup DAQ parameters
		if (strlen(profileName) > 0) InitDAQ();
		prescaleControl = evtPFactorCtrl;
		SetCtrlVal(DAQControlHandle, P_JTAGCTRL_DATAOUT, storageCtrl);
		if (gotRunPara[0] == 1) {
			newRunNumber = TRUE;
			SaveParameters(PARARUNNUMBER);
		}
		if (initMezz) {
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
			if (ASDChannel != -99999) {
				WaitInMillisecond(100);
				ASD1Control();
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0, (ASDChannel>>0)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1, (ASDChannel>>1)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2, (ASDChannel>>2)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3, (ASDChannel>>3)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4, (ASDChannel>>4)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5, (ASDChannel>>5)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6, (ASDChannel>>6)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7, (ASDChannel>>7)&0x1);
				WaitInMillisecond(100);
				ASD2Control();
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0, (ASDChannel>>8)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1, (ASDChannel>>9)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2, (ASDChannel>>10)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3, (ASDChannel>>11)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4, (ASDChannel>>12)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5, (ASDChannel>>13)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6, (ASDChannel>>14)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7, (ASDChannel>>15)&0x1);
				WaitInMillisecond(100);
				ASD3Control();
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL0, (ASDChannel>>16)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL1, (ASDChannel>>17)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL2, (ASDChannel>>18)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL3, (ASDChannel>>19)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL4, (ASDChannel>>20)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL5, (ASDChannel>>21)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL6, (ASDChannel>>22)&0x1);
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_CHANNEL7, (ASDChannel>>23)&0x1);
			}
			if (ASD1Threshold == -99999) ASD1Threshold = ASDThreshold;
			if (ASD2Threshold == -99999) ASD2Threshold = ASDThreshold;
			if (ASD3Threshold == -99999) ASD3Threshold = ASDThreshold;
			if (ASD1Threshold != -99999) {
				WaitInMillisecond(100);
				ASD1Control();
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, ASD1Threshold);
			}
			if (ASD2Threshold != -99999) {
				WaitInMillisecond(100);
				ASD2Control();
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, ASD2Threshold);
			}
			if (ASD3Threshold != -99999) {
				WaitInMillisecond(100);
				ASD3Control();
				SetCtrlVal(ASDSetupHandle, P_ASDSETUP_MAINTHRESHOLD, ASD3Threshold);
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
			WaitInMillisecond(200);
			AMTDone();
			InitDAQ();
			if (mezzCopyMask != 0) InitDAQ();  // Init DAQ again to avoid any potential threshold problem
		}

		SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, FALSE);
		SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, FALSE);
		SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, FALSE);
		if (stopAtEventOn) {
			SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPEVENTON, TRUE);
			SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPATEVENT, numberEvent);
		}
		if (stopAtTimeOn) {
			SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNSON, TRUE);
			SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_STOPAFTERRUNS, 1);
			SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSECON, TRUE);
			SetCtrlVal(AutoRunSetupHandle, P_AUTORUN_RESTARTAFTERSEC, DAQTimeInSeconds);
		}
		// printf("runNumber = %u\n", runNumber);
		// printf("number of event = %u\n", numberEvent);
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

	return validProfile;
}


void AutoRunControl(int restartRun) {
	if (runState == State_Running) {
		StopDAQ;           // Stop DAQ first
		if ((restartRun > 0) || (autoRunFromControlFile == 1)) {
			autoRunOn = TRUE;
			StartDAQ;        // Start DAQ
		}
	}
	return;
}


