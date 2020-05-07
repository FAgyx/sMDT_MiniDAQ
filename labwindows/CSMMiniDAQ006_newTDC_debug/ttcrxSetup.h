#ifndef ttcrxSetup_H
#define ttcrxSetup_H
//
// Filename: ttcrxSetup.h
// Author: T.S. Dai
//
// Description:
// This file contains prototypes for all of the TTCrx JTAG Setup functions.

#define TTCRXSETUPLENGTH          80

// Define TTCrx setup bits
//
#define CLOCK1FINEDELAY            0
#define CLOCK2FINEDELAY	           8
#define CLOCK1COARSEDELAY         16
#define CLOCK2COARSEDELAY         20
#define BUNCHCOUNTERCONTROL       24
#define EVENTCOUNTERCONTROL       25
#define SELECTSYNCHCLOCK          26
#define CLOCK2OUTPUT              27
#define CLOCKL1ACCEPTOUTPUT       28
#define PARALLELOUTPUTBUS         29
#define SERIALBOUTPUT             30
#define CLOCKOUTPUT               31
#define TTCRXADDRESS              32
#define MASTERMODEA               46
#define SPCONVERTERA              46
#define TESTMODEA                 47
#define I2CBASEADDRESS            48
#define MASTERMODEB               54
#define SPCONVERTERB              54
#define TESTMODEB                 55
#define DLLCURRENT                56
#define PLLCURRENT                59
#define PSHF1TESTINPUT            62
#define PSHF2TESTINPUT            63
#define TESTOUTPUTS               64
#define PLLPHASEDETECTOR          67
#define SELECTINPUTS              68
#define ASSERTPLLTESTRESET        69
#define ASSERTDLLTESTRESET        70
#define HAMMINGCHECKA             71
#define FREQCHECKPERIOD           72
#define AUTOFREQINCREASE          75
#define WATCHDOGCIRCUIT           76
#define HAMMINGERRORDETCORR       77
#define TESTINOUTPUTS             78
#define HAMMINGCHECKB             79

int TTCrxSetupHandle;
int TTCrxSetupArray[TTCRXSETUPLENGTH];

//
// Function prototypes
//
void UpTTCrxSetupControl(void);
int TTCrxSetupPanelAndButton(void);
void TTCrxSetupDone(void);
void SaveTTCrxSetup(void);
void PutClock1DelayValue(void);
void PutClock2DelayValue(void);
void PutTestOutputSelection(void);
void PutFrequencyCheckPeriod(void);

void LoadTTCrxSetupArray(void);
float TTCrxClockFineDelay(int fine);
float TTCrxClockCoarseDelay(int coarse);

#endif
