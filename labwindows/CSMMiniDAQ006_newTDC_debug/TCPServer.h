#ifndef TCPServer_H
#define TCPServer_H
//
// Filename: TCPServer.h
// Author: T.S. Dai
//
// Description:
// This file contains TCPServer function prototypes for CSM1 DAQ Control.

// Turn on following flag if TTCvi driver is via TCP/IP
// #define TTCVIDRIVERVIATCP

// Defining constant TRUE and FALSE
#define TRUE                              1
#define FALSE                             0																					  

#define MAXTCPTTCVIWORDS              0x020
#define MAXTCPDATAWORDS               0x400
#define TCPTIMEOUT                     1000  // TCP Timeout in ms
#define WIN_PORT_NUM		           6000  // port # uniquely identifies each TCP server
#define WIN_TTCVI_PORT		           7200  // Win TTCvi port #
#define UNIX_PORT_NUM		           4000
#define UNIX_TTCVI_PORT		           5200

// Define Data Type, use bit 16 to 20 only
#define DATATYPESTATUSONLY       0x00000000
#define DATATYPERAWDATA          0x00010000
#define DATATYPERAWDATACONT      0x00020000
#define DATATYPEEVENT            0x00030000
#define DATATYPEEVENTCONT        0x00040000
#define DATATYPEDAQINFOR         0x00050000
#define DATATYPEALLDAQINFOR      0x00060000
#define DATATYPEEDGES            0x00070000
#define DATATYPEPAIR             0x00080000
#define DATATYPEHITS             0x00090000
#define DATATYPEAVERAGETDCTIME   0x000A0000
#define DATATYPEFRACTIONUSED     0x000B0000
#define DATATYPEAVERAGEWIDTH     0x000C0000
#define DATATYPEDETAILSTATUS     0x000F0000
#define DATATYPEBITMASK          0x000F0000

// TCP node list
struct tcp_node {
  unsigned int id;
  int port, issuedCMD, CMDTime, haveData, status;
  struct tcp_node *next;
};
unsigned int TCPData[MAXTCPDATAWORDS], TCPTTCviData[MAXTCPTTCVIWORDS];
int DAQTCPConnected, TTCviTCPConnected; 

// TCP server program function prototypes
int ActivateTCPServer(void);
int DeactivateTCPServer(void);
int WINServerCallback(unsigned handle, int event, int error, void *callbackData);
int WINTTCviServerCallback(unsigned handle, int event, int error, void *callbackData);
int UNIXServerCallback(unsigned handle, int event, int error, void *callbackData);
int UNIXTTCviServerCallback(unsigned handle, int event, int error, void *callbackData);
int SendTCPData(int wait, struct tcp_node *node);
int SendTCPDataToAPort(int wait, int port);
int SendToClients(int wait, int port);
int ReadTCPData(struct tcp_node *node);
int DisconnectClient(unsigned int ID);
int DisconnectTTCviClient(void);
int DisconnectSockets(void);
int CheckClientsStatus(int port);
int NumberConnectedTCPNodes(void);
void IntToChar(int *character, int integer, int *pointer);

#endif

