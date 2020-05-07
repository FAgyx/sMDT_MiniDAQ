//****************************************************************************
//
// TCP server control functions. Allows up to 255 simultaneous TCP
// server-client connections (limited by Windows). TCP server control
// functions are called by ServClnt.c
//  
//****************************************************************************

#include <tcpsupp.h>
#include <ansi_c.h>
#include <utility.h>
#include "interface.h"
#include "TCPServer.h"
	   
#define C_ID(node)          (node)->id	
#define C_PORT(node)        (node)->port	
#define C_ISSUEDCMD(node)   (node)->issuedCMD	
#define C_CMDTIME(node)     (node)->CMDTime	
#define C_HAVEDATA(node)    (node)->haveData	
#define C_STATUS(node)      (node)->status	

extern int WaitForTCPClientReply(int port);
extern void UpdateTCPStatus(int newConnection);
extern void HandleVariesData(unsigned int data[]);
extern void HandleTTCviData(unsigned int data[]);
unsigned long numberTCPError;
char nConnected = 0;
struct tcp_node *tcp_node_list = NULL;

// Registers a valid TCP server
int ActivateTCPServer(void)
{
  int retval, status = 0;
  
  retval = RegisterTCPServer(WIN_PORT_NUM, WINServerCallback, NULL);
  if (retval < 0) {
    status++;
    printf("TCP error: Window TCP Server registration failed, retval = %d\n", retval);
  }
  retval = RegisterTCPServer(UNIX_PORT_NUM, UNIXServerCallback, NULL);
  if (retval < 0) {
    status++;
    printf("TCP error: UNIX TCP Server registration failed, retval = %d\n", retval);
  }
#ifdef TTCVIDRIVERVIATCP
  retval = RegisterTCPServer(WIN_TTCVI_PORT, WINTTCviServerCallback, NULL);
  if (retval < 0) {
    status++;
    printf("TCP error: WIN TCP TTCvi Server registration failed, retval = %d\n", retval);
  }
  retval = RegisterTCPServer(UNIX_TTCVI_PORT, UNIXTTCviServerCallback, NULL);
  if (retval < 0) {
    status++;
    printf("TCP error: UNIX TCP TTCvi Server registration failed, retval = %d\n", retval);
  }
#endif
  return status;
}

//
// Server termination routine.  Disconnects all existing TCP client conversations, and 
// de-registers itself as a valid TCP server.
//
int DeactivateTCPServer(void)
{
  int retval, status = 0;
  
  retval = DisconnectSockets();
  if (retval != 0) status++;
  retval = UnregisterTCPServer(WIN_PORT_NUM);
  if (retval < 0) {
    status++;
    printf("TCP error: Window TCP Server de-registration failed, retval = %d\n", retval);
  }
  retval = UnregisterTCPServer(UNIX_PORT_NUM);
  if (retval < 0) {
    status++;
    printf("TCP error: UNIX TCP Server de-registration failed, retval = %d\n", retval);
  }
#ifdef TTCVIDRIVERVIATCP
  retval = UnregisterTCPServer(WIN_TTCVI_PORT);
  if (retval < 0) {
    status++;
    printf("TCP error: WIN TCP TTCvi Server de-registration failed, retval = %d\n", retval);
  }
  retval = UnregisterTCPServer(UNIX_TTCVI_PORT);
  if (retval < 0) {
    status++;
    printf("TCP error: UNIX TCP TTCvi Server de-registration failed, retval = %d\n", retval);
  }
#endif
  return status;
}	

//
// Recieves and processes all messages sent by clients. TCP_CONNECT is recieved when a 
// client is requesting a new connection. TCP_DATAREADY is recieved when a client has sent 
// data to the server. TCP_DISCONNECT is recieved when a client requests termination
// of a connection, or when one is terminated due to an error. A linked list, tcp_node_list, 
// is used to identify up to 255 simultaneous TCP client conversations.
//
int WINServerCallback(unsigned handle, int event, int error, void *callbackData)
{
  struct tcp_node *new_tcp_node, *c, *c2;
  
  switch(event) {
  case TCP_CONNECT:  // client requests connection, place node in tcp_node_list
    if (nConnected < 255) {
      nConnected++;
      new_tcp_node = (struct tcp_node *) malloc(sizeof(struct tcp_node));
      C_ID(new_tcp_node) = handle;
      C_PORT(new_tcp_node) = WIN_PORT_NUM;
      C_ISSUEDCMD(new_tcp_node) = FALSE;
      C_HAVEDATA(new_tcp_node) = FALSE;
      new_tcp_node->next = NULL;
      if (tcp_node_list) {
        c = tcp_node_list;
        while(c->next) c = c->next;
        c->next = new_tcp_node;
      }
      else tcp_node_list = new_tcp_node;
      UpdateTCPStatus(TRUE);
      printf("A Window TCP client connected (handle = %d, event = %d)\n", handle, event);
      c = new_tcp_node;
      for (c2 = tcp_node_list; c2; c2 = c2 -> next) {
        if ((c2 != c) && ((C_PORT(c2) == C_PORT(c)) || (C_PORT(c2) == UNIX_PORT_NUM))) {
          printf("New TCPNode (ID = %d) has same port number (= %d) as TCPNode (ID = %d).\n"
                 "It is not allowed and the new TCPNode will be disconnected,\n"
                 "please use different port number.\n",
                 C_ID(c), C_PORT(c), C_ID(c2));
          DisconnectClient(C_ID(c));
          break;
        }
      }
      NumberConnectedTCPNodes();
    }
    else
      printf("TCP warning: Max number (255) of connections reached\n"
             "No further connections can be added at this time\n");
    break;
  case TCP_DATAREADY:         //recieve tcp data from a client
    for (c = tcp_node_list; c; c2 = c, c = c->next) {
      if (C_ID(c) == handle) {
        ReadTCPData(c);
        break;
      }
    }
    break;
  case TCP_DISCONNECT:       // disconnect client, free memory in tcp_node_list
    for (c = tcp_node_list; c; c2 = c, c = c->next) {
      if (C_ID(c) == handle) break;
    }
    if(c == tcp_node_list) tcp_node_list = c->next;
    else c2->next = c->next;
    free(c);
    nConnected--;
    NumberConnectedTCPNodes();
    UpdateTCPStatus(FALSE);
    printf("A Window TCP client disconnected (handle = %d, event = %d)\n", handle, event);
    break;
  default:
    break;
  }
  return 0;
}

int WINTTCviServerCallback(unsigned handle, int event, int error, void *callbackData)
{
  struct tcp_node *new_tcp_node, *c, *c2;
  
  switch(event) {
  case TCP_CONNECT:  // client requests connection, place node in tcp_node_list
    if (nConnected < 255) {
      nConnected++;
      new_tcp_node = (struct tcp_node *) malloc(sizeof(struct tcp_node));
      C_ID(new_tcp_node) = handle;
      C_PORT(new_tcp_node) = WIN_TTCVI_PORT;
      C_ISSUEDCMD(new_tcp_node) = FALSE;
      C_HAVEDATA(new_tcp_node) = FALSE;
      new_tcp_node->next = NULL;
      if (tcp_node_list) {
        c = tcp_node_list;
        while(c->next) c = c->next;
        c->next = new_tcp_node;
      }
      else tcp_node_list = new_tcp_node;
      UpdateTCPStatus(TRUE);
      printf("A Window TCP client connected (handle = %d, event = %d)\n", handle, event);
      c = new_tcp_node;
      for (c2 = tcp_node_list; c2; c2 = c2 -> next) {
        if ((c2 != c) && ((C_PORT(c2) == C_PORT(c)) || (C_PORT(c2) == UNIX_TTCVI_PORT))) {
          printf("New TCPNode (ID = %d) has same port number (= %d) as TCPNode (ID = %d).\n"
                 "It is not allowed and the new TCPNode will be disconnected,\n"
                 "please use different port number.\n",
                 C_ID(c), C_PORT(c), C_ID(c2));
          DisconnectClient(C_ID(c));
          break;
        }
      }
      NumberConnectedTCPNodes();
    }
    else
      printf("TCP warning: Max number (255) of connections reached\n"
             "No further connections can be added at this time\n");
    break;
  case TCP_DATAREADY:         //recieve tcp data from a client
    for (c = tcp_node_list; c; c2 = c, c = c->next) {
      if (C_ID(c) == handle) {
        ReadTCPData(c);
        break;
      }
    }
    break;
  case TCP_DISCONNECT:       // disconnect client, free memory in tcp_node_list
    for (c = tcp_node_list; c; c2 = c, c = c->next) {
      if (C_ID(c) == handle) break;
    }
    if(c == tcp_node_list) tcp_node_list = c->next;
    else c2->next = c->next;
    free(c);
    nConnected--;
    NumberConnectedTCPNodes();
    UpdateTCPStatus(FALSE);
    printf("A Window TCP client disconnected (handle = %d, event = %d)\n", handle, event);
    break;
  default:
    break;
  }
  return 0;
}

int UNIXServerCallback(unsigned handle, int event, int error, void *callbackData)
{
  struct tcp_node *new_tcp_node, *c, *c2;
  
  switch(event) {
  case TCP_CONNECT:  // client requests connection, place node in tcp_node_list
    if (nConnected < 255) {
      nConnected++;
      new_tcp_node = (struct tcp_node *) malloc(sizeof(struct tcp_node));
      C_ID(new_tcp_node) = handle;
      C_PORT(new_tcp_node) = UNIX_PORT_NUM;
      C_ISSUEDCMD(new_tcp_node) = FALSE;
      C_HAVEDATA(new_tcp_node) = FALSE;
      new_tcp_node->next = NULL;
      if (tcp_node_list) {
        c = tcp_node_list;
        while(c->next) c = c->next;
        c->next = new_tcp_node;
      }
      else tcp_node_list = new_tcp_node;
      UpdateTCPStatus(TRUE);
      printf("An UNIX TCP client connected (handle = %d, event = %d)\n", handle, event);
      c = new_tcp_node;
      for (c2 = tcp_node_list; c2; c2 = c2 -> next) {
        if ((c2 != c) && ((C_PORT(c2) == C_PORT(c)) || (C_PORT(c2) == WIN_PORT_NUM))) {
          printf("New TCPNode (ID = %d) has same port number (= %d) as TCPNode (ID = %d).\n"
                 "It is not allowed and the new TCPNode will be disconnected,\n"
                 "please use different port number.\n",
                 C_ID(c), C_PORT(c), C_ID(c2));
          DisconnectClient(C_ID(c));
          break;
        }
      }
      NumberConnectedTCPNodes();
    }
    else
      printf("TCP warning: Max number (255) of connections reached\n"
             "No further connections can be added at this time\n");
    break;
  case TCP_DATAREADY:         //recieve tcp data from a client
    for (c = tcp_node_list; c; c2 = c, c = c->next) {
      if (C_ID(c) == handle) {
        ReadTCPData(c);
        break;
      }
    }
    break;
  case TCP_DISCONNECT:       // disconnect client, free memory in tcp_node_list
    for(c = tcp_node_list; c; c2 = c, c = c->next) {
      if (C_ID(c) == handle) break;
    }
    if(c == tcp_node_list) tcp_node_list = c->next;
    else c2->next = c->next;
    free(c);
    nConnected--;
    NumberConnectedTCPNodes();
    UpdateTCPStatus(FALSE);
    printf("An UNIX TCP client disconnected (handle = %d, event = %d)\n", handle, event);
    break;
  default:
    break;
  }
  return 0;
}

int UNIXTTCviServerCallback(unsigned handle, int event, int error, void *callbackData)
{
  struct tcp_node *new_tcp_node, *c, *c2;
  
  switch(event) {
  case TCP_CONNECT:  // client requests connection, place node in tcp_node_list
    if (nConnected < 255) {
      nConnected++;
      new_tcp_node = (struct tcp_node *) malloc(sizeof(struct tcp_node));
      C_ID(new_tcp_node) = handle;
      C_PORT(new_tcp_node) = UNIX_TTCVI_PORT;
      C_ISSUEDCMD(new_tcp_node) = FALSE;
      C_HAVEDATA(new_tcp_node) = FALSE;
      new_tcp_node->next = NULL;
      if (tcp_node_list) {
        c = tcp_node_list;
        while(c->next) c = c->next;
        c->next = new_tcp_node;
      }
      else tcp_node_list = new_tcp_node;
      UpdateTCPStatus(TRUE);
      printf("An UNIX TCP client connected (handle = %d, event = %d)\n", handle, event);
      c = new_tcp_node;
      for (c2 = tcp_node_list; c2; c2 = c2 -> next) {
        if ((c2 != c) && ((C_PORT(c2) == C_PORT(c)) || (C_PORT(c2) == WIN_TTCVI_PORT))) {
          printf("New TCPNode (ID = %d) has same port number (= %d) as TCPNode (ID = %d).\n"
                 "It is not allowed and the new TCPNode will be disconnected,\n"
                 "please use different port number.\n",
                 C_ID(c), C_PORT(c), C_ID(c2));
          DisconnectClient(C_ID(c));
          break;
        }
      }
      NumberConnectedTCPNodes();
    }
    else
      printf("TCP warning: Max number (255) of connections reached\n"
             "No further connections can be added at this time\n");
    break;
  case TCP_DATAREADY:         //recieve tcp data from a client
    for (c = tcp_node_list; c; c2 = c, c = c->next) {
      if (C_ID(c) == handle) {
        ReadTCPData(c);
        break;
      }
    }
    break;
  case TCP_DISCONNECT:       // disconnect client, free memory in tcp_node_list
    for(c = tcp_node_list; c; c2 = c, c = c->next) {
      if (C_ID(c) == handle) break;
    }
    if(c == tcp_node_list) tcp_node_list = c->next;
    else c2->next = c->next;
    free(c);
    nConnected--;
    NumberConnectedTCPNodes();
    UpdateTCPStatus(FALSE);
    printf("An UNIX TCP client disconnected (handle = %d, event = %d)\n", handle, event);
    break;
  default:
    break;
  }
  return 0;
}

// Writes data to a single specified client
int SendTCPData(int wait, struct tcp_node *node)
{
  int success, numberTry, data[4*MAXTCPDATAWORDS], nbytes, i, pointer, checksum;
  float time0;
  char *err, message[256];
  
  if (C_ID(node) < -1) return 0;
  time0 = (float) Timer();
  while ((((float) Timer()) - time0) < 0.100) HandleUserButtons(PanelDone);
  while (C_ISSUEDCMD(node) && (!C_HAVEDATA(node))) {
    if (wait) {
      HandleUserButtonAndFunction(PanelDone);
      if ((((int) time(NULL)) - C_CMDTIME(node)) >= 4) {
        C_STATUS(node) = -1;
        return C_STATUS(node);
      }
    }
    else return -2;
  }
  checksum = 0;
  if ((C_PORT(node) == UNIX_TTCVI_PORT) || (C_PORT(node) == WIN_TTCVI_PORT)) {
    if (TCPTTCviData[0] >= MAXTCPTTCVIWORDS) TCPTTCviData[0] = MAXTCPTTCVIWORDS - 1;
    for (i = 0; i < TCPTTCviData[0]; i++) checksum += TCPTTCviData[i];
    TCPTTCviData[TCPTTCviData[0]] = checksum;
  }
  else {
    if (TCPData[0] >= MAXTCPDATAWORDS) TCPData[0] = MAXTCPDATAWORDS - 1;
    for (i = 0; i < TCPData[0]; i++) checksum += TCPData[i];
    TCPData[TCPData[0]] = checksum;
  }
  pointer = 0;
  numberTry = 0;
  if ((node->port) == UNIX_TTCVI_PORT) {
    nbytes = 16*MAXTCPTTCVIWORDS;
    for (i = 0; i < MAXTCPTTCVIWORDS; i++) IntToChar(data, TCPTTCviData[i], &pointer);
  }
  else if ((node->port) == UNIX_PORT_NUM) {
    nbytes = 16*MAXTCPDATAWORDS;
    for (i = 0; i < MAXTCPDATAWORDS; i++) IntToChar(data, TCPData[i], &pointer);
  }
  else if ((node->port) == WIN_TTCVI_PORT) {
    nbytes = 4*MAXTCPTTCVIWORDS;
    for (i = 0; i < MAXTCPDATAWORDS; i++) data[i] = TCPTTCviData[i];
  }
  else {
    nbytes = 4*MAXTCPDATAWORDS;
    for (i = 0; i < MAXTCPDATAWORDS; i++) data[i] = TCPData[i];
  }
  C_ISSUEDCMD(node) = TRUE;
  C_CMDTIME(node) = time(NULL);
  C_HAVEDATA(node) = FALSE;
  success = ServerTCPWrite(C_ID(node), data, nbytes, TCPTIMEOUT);
  while ((success < 0) && (numberTry < 5)) {; 
    numberTCPError++;
    numberTry++;
    err = GetTCPErrorString(success);
    sprintf(message, "Write failed %d times in roll(%s).", numberTry, err);
    printf("TCP error: %s\n", message);
    success = ServerTCPWrite(C_ID(node), data, nbytes, TCPTIMEOUT);
  }
  if (success < 0) {
    C_STATUS(node) = 1;
    return 1;
  }
  else {
    C_STATUS(node) = 0;
    if (wait) if (WaitForTCPClientReply(C_PORT(node)) != 0) return 2;
  }
  return 0;
}	

// Writes data to a single specified port
int SendTCPDataToAPort(int wait, int port)
{
  struct tcp_node *c;
  
  if (nConnected <= 0) return -1;
  for (c = tcp_node_list; c; c = c -> next) {
    if (C_PORT(c) == port) return SendTCPData(wait, c);
  }
  return -1;
}	

// Writes data to all connected clients 
int SendToClients(int wait, int port)
{
  struct tcp_node *a;
  int success, numberTry, status;
  unsigned int data[4*MAXTCPDATAWORDS], nbytes, i, pointer, checksum;
  char *err, message[256];
  float time0;
  
  if (nConnected <= 0) return -1;
  if ((port != UNIX_TTCVI_PORT) && (port != WIN_TTCVI_PORT)) {
    time0 = (float) Timer();
    while ((((float) Timer()) - time0) < 0.100) HandleUserButtons(PanelDone);
  }
  for (a = tcp_node_list; a;) {
    while (C_ISSUEDCMD(a) && (!C_HAVEDATA(a))) {
      if (wait) {
        HandleUserButtons(PanelDone);
        if ((((int) time(NULL)) - C_CMDTIME(a)) >= 4) {
          C_STATUS(a) = -1;
          C_ISSUEDCMD(a) = FALSE;
          return C_STATUS(a);
        }
      }
      else return -2;
    }
    a = a->next;
  }
  status = 0;
  checksum = 0;
  if ((port == UNIX_TTCVI_PORT) || (port == WIN_TTCVI_PORT)) {
    if (TCPTTCviData[0] >= MAXTCPTTCVIWORDS) TCPTTCviData[0] = MAXTCPTTCVIWORDS - 1;
    for (i = 0; i < TCPTTCviData[0]; i++) checksum += TCPTTCviData[i];
    TCPTTCviData[TCPTTCviData[0]] = checksum;
  }
  else {
    if (TCPData[0] >= MAXTCPDATAWORDS) TCPData[0] = MAXTCPDATAWORDS - 1;
    for (i = 0; i < TCPData[0]; i++) checksum += TCPData[i];
    TCPData[TCPData[0]] = checksum;
  }
  for (a = tcp_node_list; a;) {
    if (C_PORT(a) == port) {
      pointer = 0;
      numberTry = 0;
      if (C_PORT(a) == UNIX_TTCVI_PORT) {
        nbytes = 16*MAXTCPTTCVIWORDS;
        for (i = 0; i < MAXTCPTTCVIWORDS; i++) IntToChar(data, TCPTTCviData[i], &pointer);
      }
      else if (C_PORT(a) == UNIX_PORT_NUM) {
        nbytes = 16*MAXTCPDATAWORDS;
        for (i = 0; i < MAXTCPDATAWORDS; i++) IntToChar(data, TCPData[i], &pointer);
      }
      else if (C_PORT(a) == WIN_TTCVI_PORT) {
        nbytes = 16*MAXTCPTTCVIWORDS;
        for (i = 0; i < MAXTCPDATAWORDS; i++) data[i] = TCPTTCviData[i];
      }
      else {
        nbytes = 4*MAXTCPDATAWORDS;
        for (i = 0; i < MAXTCPDATAWORDS; i++) data[i] = TCPData[i];
      }
      C_STATUS(a) = 0;
      C_ISSUEDCMD(a) = TRUE;
      C_CMDTIME(a) = (int) time(NULL);
      C_HAVEDATA(a) = FALSE;
      success = ServerTCPWrite(C_ID(a), data, nbytes, TCPTIMEOUT);
      while ((success < 0) && (numberTry < 5)) {; 
        numberTCPError++;
        numberTry++;
        err = GetTCPErrorString(success);
        sprintf(message, "Write failed %d times in roll(%s) (nConnected = %d).", numberTry, err, nConnected);
        printf("TCP error: %s\n", message);
        success = ServerTCPWrite(C_ID(a), data, nbytes, TCPTIMEOUT);
      }
      if (success < 0) C_STATUS(a) = 1;
      a = a->next;
      if (success < 0) status++;
      else if (wait) {
        i = WaitForTCPClientReply(port);
        if (i != 0) {
          if ((port == UNIX_TTCVI_PORT) || (port == WIN_TTCVI_PORT)) {
            if (i == 1) status++;
          }
          else status++;
        }
      }
    }
    else a = a->next;
  }
  return status;
}


int ReadTCPData(struct tcp_node *node) {
  struct tcp_node *c;
  unsigned int data[2*MAXTCPDATAWORDS], checksum, status, i, dataType;
  float time0;
  
  WaitInMillisecond(2);
  status = 0;
  if (ServerTCPRead(C_ID(node), data, sizeof(data), TCPTIMEOUT) < 0) {
    status = 1;
    numberTCPError++;
    printf("TCP Error: TCP read error (TCPnode ID = %d)\n", C_ID(node));
    C_STATUS(node) = status;
    UpdateTCPStatus(FALSE);
  }
  else if (!C_HAVEDATA(node)) {
    C_HAVEDATA(node) = TRUE;
    checksum = 0;
    if (data[0] >= MAXTCPDATAWORDS) status = 2;
    else if (data[0] == 0) status = 3;
    else {
      for (i = 0; i < data[0]; i++) checksum += data[i];
      if (checksum != data[data[0]]) {
        status = 4;
//        printf("TCP error: checksum error Expected = 0x%08x, Gotten = 0x%08x\n", checksum, data[data[0]]);
//        printf("           data[0] = %d, data[1] = 0x%08x, data[2] = 0x%08x, data[3] = 0x%08x\n", data[0], data[1], data[2], data[3]);
      }
    }
    if (data[2] != 0) status = 5;
    C_STATUS(node) = status;
    if ((data[0] > 0) && (data[0] <= 3)) {
      if (status == 5) printf("Status error from TCPnode (ID = %d) with status = 0x%08X\n", C_ID(node), data[2]);
      else if (status != 0) printf("TCP error: TCP data transfer error (TCPnode ID = %d status = %d)\n", C_ID(node), status);
    }
    if (status != 0) {
      time0 = (float) Timer();
      while ((((float) Timer()) - time0) < 0.200) {
        UpdateTCPStatus(FALSE);
        HandleUserButtons(PanelDone);
      }
    }
    if ((C_PORT(node) == UNIX_TTCVI_PORT) || (C_PORT(node) == WIN_TTCVI_PORT)) HandleTTCviData(data);
    else if ((status == 0) && (data[0] > 3)) {
      dataType = data[1] & DATATYPEBITMASK;
      if (dataType != DATATYPESTATUSONLY) HandleVariesData(data);
    }
    UpdateTCPStatus(FALSE);
  }
  return status;
}


// Terminates a connection with a single specified client and free the memory.
int DisconnectClient(unsigned int ID)
{
  struct tcp_node *c, *c2;
  int status;
  
  printf("Disconnect TCP with ID = %d\n", ID);
  status = DisconnectTCPClient(ID);
  for (c = tcp_node_list; c; c2 = c, c = c->next) {
    if (C_ID(c) == ID) break;
  }
  if(c == tcp_node_list) tcp_node_list = c->next;
  else c2->next = c->next;
  free(c);
  nConnected--;
  UpdateTCPStatus(FALSE);
  if (status >= 0) status = 0;
  else printf("TCP error", "TCP Disconnect failed (connectID = %d)\n", ID);
  return status;
}

int DisconnectTTCviClient(void)
{
  struct tcp_node *c, *c2;
  int status = 0, foundTTCviClient;
  
  printf("Sart to Disconnect TCP TTCvi client(s)\n");
  foundTTCviClient = TRUE;
  while (foundTTCviClient) {
    foundTTCviClient = FALSE;
    for (c = tcp_node_list; c; c2 = c, c = c->next) {
      if ((C_PORT(c) == UNIX_TTCVI_PORT) || (C_PORT(c) == WIN_TTCVI_PORT)) {
        foundTTCviClient = TRUE;
        status = DisconnectClient(C_ID(c));
        break;
      }
    }
    if (status != 0) break;
  }
  printf("Disconnect TCP TTCvi client(s) Done\n");
  return status;
}

// Disconnect all TCP client connections, free the memory used by tcp_node_list.
int DisconnectSockets(void)
{
  int status = 0;
  struct tcp_node *i, *i2;
  
  if (nConnected <= 0) return 0;
  for (i = tcp_node_list; i; i = i2) {
    i2 = i->next;
    status |= DisconnectClient(C_ID(i));
  }
  free(tcp_node_list);
  nConnected = 0;
  return status;
}


int CheckClientsStatus(int port) {
  struct tcp_node *c;

  if (nConnected <= 0) return -1;
  for (c = tcp_node_list; c; c = c -> next) {
    if (C_PORT(c) == port) {
      if (C_STATUS(c) == 1) return C_STATUS(c);
      if (C_ISSUEDCMD(c)) {
        if (!C_HAVEDATA(c)) return 11;
      }
      if (C_STATUS(c) != 0) return C_STATUS(c);
    }
  }
  return 0;
}


int NumberConnectedTCPNodes(void) {
  struct tcp_node *a;

  DAQTCPConnected = FALSE;
  TTCviTCPConnected = FALSE;
  for (a = tcp_node_list; a;) {
    if (C_PORT(a) == UNIX_PORT_NUM) DAQTCPConnected = TRUE;
    if (C_PORT(a) == UNIX_TTCVI_PORT) TTCviTCPConnected = TRUE;
    if (C_PORT(a) == WIN_TTCVI_PORT) TTCviTCPConnected = TRUE;
    a = a->next;
  }
  return nConnected;
}


void IntToChar(int *character, int integer, int *pointer)
{
  int mask = 0xFF, i, temp;
  
  for (i = 0; i < 32; i += 8) {
    temp = (integer >> i) & mask;
    character[*pointer] = temp;
    *pointer += 1;
  }
}
