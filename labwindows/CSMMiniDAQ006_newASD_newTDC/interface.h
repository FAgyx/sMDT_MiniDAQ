#ifndef interface_H
#define interface_H
//
// Filename: interface.h
// Author: T.S. Dai
//
// Description:
// This file contains prototypes for all of the user interface functions and
// general utility routines.

#include <ansi_c.h>				
#include <userint.h>

#define MAXBUTTON                      400     // Maximum number of buttons 

// Defining constant TRUE and FALSE
#define TRUE                             1
#define FALSE                            0																					  

// Define constant of NORM and HELP 
#define NORM                             0
#define HELP                             1

#define DOWNLOAD                         0
#define TEXTFILES                        1
#define ACTSEQFILES                      2

#define State_Idle                       0
#define State_Starting                   1
#define State_Running                    2
#define State_Paused                     3

#define NORMALSTOP                       0
#define ABNORMALSTOP                     1

// Define CSM Type
//
#define UNKNOWN                         -1
#define CSM                              0
#define CSM1                             1
#define CSM2                             2
#define CSM3                             3

//
// Define Run Type
//
#define NORMALALLCALIBCHANNELOFF         0 // Normal run, all calibration channel off
#define CALIBALLCALIBCHANNELON           1 // Calibration run, all calibration channel on
#define NORMALKEEPCALIBCHANNEL           2 // Normal run, keep calibration channel enable
#define CALIBKEEPCALIBCHANNEL            3 // Calibration run, keep calibration channel enable
#define SPECIALCHECKFPGAANDPROM          4 // Special run, check FPGA and PROM for radiation test

int runState, runNumber, autoRunOn, measureTriggerRate, verifyFPGAAndPROM, doingInitCSMDAQ;
int runStopStatus, nominalThreshold;
int controlOption, numberButtons, quitProgram, gotTTCvi, MDTChamberTest, MDTChamberType;
char IPAddress[80], netIPAddress[80], hostName[80], domainName[80];
//
// user_button_data holds the various parameters which define how a user button
// operates in the eventLoop environment.
//
struct userButtonData {
  int panel_id, id, para1, para2;
  void (*func)(void);
  int (*func1)(int para);
  int (*func2)(int para1, int para2);
};
// static array for buttons
struct userButtonData userButtons[MAXBUTTON];

//
// Public (user) function prototypes
//
// Interface setup functions
//
void InitUserInterface(void);
int SetupUserButton(int panel_id, int id, void (*func) (void));
int SetupUserButton1(int panel_id, int id, int (*func)(int), int para1);
int SetupUserButton2(int panel_id, int id, int (*func)(int, int), int para1, int para2);
void LoopAndExecuteUserButtonAndFunction(void (*func)(void));
void LoopAndCheckUserButtonAndFunction(void (*func)(void), void (*exeF)(void));
int CheckUserButtons(void (*Func)(void));
int HandleUserButtons(void (*Func)(void));
int HandleUserButtonAndFunction(void (*Func)(void));
int HandleUserButtons2(void (*Func1)(void), void (*Func2)(void));
int HandleUserButtonAndFunction2(void (*Func1)(void), void (*Func2)(void));
int HandleUserButtons3(void (*Func1)(void), void (*Func2)(void), void (*Func3)(void));
int HandleUserButtonAndFunction3(void (*Func1)(void), void (*Func2)(void), void (*Func3)(void));
int HandleUserButtons4(void (*Func1)(void), void (*Func2)(void), void (*Func3)(void), void (*Func4)(void));
int HandleUserButtonAndFunction4(void (*Func1)(void), void (*Func2)(void), void (*Func3)(void), void (*Func4)(void));

//
// Utility: Data processing functions
//
void BinaryToInt(int *integer, int offset, int width, int *array);
void ReversedBinaryToInt(int *integer, int offset, int width, int *array);
void IntToBinary(int integer, int offset, int width, int *array, int arrayLength);
void IntToReversedBinary(int integer, int offset, int width, int *array, int arrayLength);
void WordToArray(unsigned int *wordOut, int wordCount, int *array, int arrayLength);
void ArrayToWord(unsigned int *wordIn, int *wordCount, int *array, int arrayLength);
void ReverseArray(int *array, int start, int bits);
void WaitInMillisecond(int waitTime);
void GetIPConfiguration(void);

// Utility functions, date and time
char *TimeString(time_t dateTime);
char *DateString(time_t dateTime);
char *DateStringNoSpace(time_t dateTime);
char *DateTimeString(time_t dateTime);
char *Date_TimeString(time_t dateTime);
char *WeekdayString(time_t dateTime);
char *WeekdayTimeString(time_t dateTime);

// Utility functions
double ABS(double x);
int checkParity(unsigned int dataWord, unsigned int *CommPEct);
double CorrelationEffi(double x[], double y[], int ndata);
double CHISquare(double cval[], double mval[], double err[], int ndata, int npara);
double CHISquareFromDiff(double diff[], double err[], int ndata, int npara);
void LineFromLeastSquareMethod(double x[], double xerr[], double y[], double yerr[], int ndata, double para[]);
double AverageWithLimit(int ndata, double data[], double dlimit);

//----------------------------------------------------
// Following Utility functions are in jtagControl.c
//----------------------------------------------------
//
// Utility functions related with panel handling
void PanelDone(void);
void PanelDefault(void);
void PanelCancel(void);
void PanelSave(int panel);
void PanelRecall(int panel);

// Save and read Control Parameter 
#define PARAALL                      0
#define PARARUNNUMBER                1
#define PARANORMALJTAG               2
#define PARADEFAULTSAVED             3
#define PARANUMBERCSM                4
#define PARACURRENTCSMNUMBER         5
#define PARAMEZZENABLES              6
#define PARANOMINALTHRESHOLD         7
#define PARAINFORPRECISION           8
#define PARACSM0BASE                 9
#define PARACO11491BASE             10
#define PARAVMIC2510BASE            11
#define PARATTCVIBASE               12
#define PARABITSTREAMLENGTH         13
#define PARABITSTREAMFILE           14

void SaveParameters(int paraIndex);
int ReadParameters(int paraIndex);

#endif
