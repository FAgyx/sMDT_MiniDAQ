// jtagDriver.h
//
#ifndef JTAGDRIVER_H
#define JTAGDRIVER_H

#define XILNIXPCABLE                    0
#define CSM0                            1
#define VMIC2510                        2
#define CORELIS11491                    3
#define CANELMB                         4
#define JTAGDRIVERTYPE            CANELMB

#ifdef __cplusplus
void HandleRequest(const char cmd);
extern "C" {
__declspec(dllexport) unsigned int CSM0Base, Co11491Base, VMIC2510Base, instrumentID;
__declspec(dllexport) int globalEnableSerialControl, CANNode, ELMBNode;
__declspec(dllexport) char JTAGServerName[15], ELMBVersion[15];

__declspec(dllexport) void __stdcall GlobalEnableSerial(void);
__declspec(dllexport) void __stdcall GlobalDisableSerial(void);
__declspec(dllexport) void __stdcall FindJTAGControllerBase(void);
__declspec(dllexport) void __stdcall RefindJTAGControllerBase(void);
__declspec(dllexport) void __stdcall InitJTAG(void);
__declspec(dllexport) void __stdcall ResetTAP(void);
__declspec(dllexport) int __stdcall RunTCKAtTAPIdle(int numberTCK);
__declspec(dllexport) int __stdcall SetJTAGRateDivider(int divider);
__declspec(dllexport) int __stdcall JTAGScanAllInstruction(int bitCount, int dataArrayIn[], int dataArrayOut[]);
__declspec(dllexport) int __stdcall JTAGScanAllData(int bitCount, int dataArrayIn[], int dataArrayOut[]);
__declspec(dllexport) int __stdcall JTAGScanData(int bitCount, int dataArrayIn[], int dataArrayOut[], int endScan);
__declspec(dllexport) int __stdcall WriteCSMIOs(int dataArrayIn[]);
__declspec(dllexport) int __stdcall ReadCSMIOs(int dataArrayOut[]);
}
#else
extern unsigned int DLLIMPORT CSM0Base, Co11491Base, VMIC2510Base, instrumentID;
extern int DLLIMPORT globalEnableSerialControl, CANNode, ELMBNode;
extern char DLLIMPORT JTAGServerName[], ELMBVersion[];

extern void __stdcall DLLIMPORT GlobalEnableSerial(void);
extern void __stdcall DLLIMPORT GlobalDisableSerial(void);
extern void __stdcall DLLIMPORT FindJTAGControllerBase(void);
extern void __stdcall DLLIMPORT RefindJTAGControllerBase(void);
extern void __stdcall DLLIMPORT InitJTAG(void);
extern void __stdcall DLLIMPORT ResetTAP(void);
extern int __stdcall DLLIMPORT RunTCKAtTAPIdle(int numberTCK);
extern int __stdcall DLLIMPORT SetJTAGRateDivider(int divider);
extern int __stdcall DLLIMPORT JTAGScanAllInstruction(int bitCount, int dataArrayIn[], int dataArrayOut[]);
extern int __stdcall DLLIMPORT JTAGScanAllData(int bitCount, int dataArrayIn[], int dataArrayOut[]);
extern int __stdcall DLLIMPORT JTAGScanData(int bitCount, int dataArrayIn[], int dataArrayOut[], int endScan);
extern int __stdcall DLLIMPORT WriteCSMIOs(int dataArrayIn[]);
extern int __stdcall DLLIMPORT ReadCSMIOs(int dataArrayOut[]);
#endif

/*
#ifdef __cplusplus
extern "C" {
#endif
    void __declspec (naked) __cdecl _chkesp (void)
    {
	_asm {	jz	exit_chkesp	};
	_asm {	int	3		};
    exit_chkesp:
	_asm {	ret			};
    }
#ifdef __cplusplus
}
#endif
*/
#endif
