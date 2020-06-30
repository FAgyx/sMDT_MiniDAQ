#ifndef TCPServer_H
#define TCPServer_H
//
// Filename: TCPServer.h
// Author: T.S. Dai
//
// Description:
// This file contains TCPServer function prototypes for CSM1 DAQ Control.

// Defining constant TRUE and FALSE
#define TRUE                              1
#define FALSE                             0																					  

#define MAXTCPDATAWORDS               0x400
#define TCPTIMEOUT                     1000  // TCP Timeout in ms
#define WIN_PORT_NUM		           6000  // port # uniquely identifies each TCP server
#define UNIX_PORT_NUM		           4000

// Define Data Type, use bit 16 to 20 only
#define DATATYPESTATUSONLY       0x00000000
#define DATATYPERAWDATA          0x00010000
#define DATATYPEEVENT            0x00020000
#define DATATYPEDAQINFOR         0x00030000
#define DATATYPEALLDAQINFOR      0x00040000
#define DATATYPELEADINGEDGE      0x00050000
#define DATATYPETRAILINGEDGE     0x00060000
#define DATATYPEPAIR             0x00070000
#define DATATYPEBITMASK          0x000F0000

// TCP node list
struct tcp_node {
  unsigned int id;
  int port, GOLANumber, issuedCMD, haveData, status;
  struct tcp_node *next;
};
unsigned int TCPData[MAXTCPDATAWORDS];

// TCP server program function prototypes
int ActivateTCPServer(void);
int DeactivateTCPServer(void);
int WINServerCallback(unsigned handle, int event, int error, void *callbackData);
int UNIXServerCallback(unsigned handle, int event, int error, void *callbackData);
int SendTCPData(struct tcp_node *node);
int SendTCPDataToACard(int GOLANumber);
int SendToClients(void);
int ReadTCPData(struct tcp_node *node);
int DisconnectClient(unsigned int ID);
int DisconnectSockets(void);
int CheckClientsStatus(void);
int NumberConnectedTCPNodes(void);
void IntToChar(int *character, int integer, int *pointer);

#endif

