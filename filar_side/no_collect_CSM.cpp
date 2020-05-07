/******************************************************************************
 *
 *  CSMMiniDAQ.cpp         Tiesheng Dai, Bob Ball 
 *                         Physics Department of University of Michigan
 *                         June 19, 2003.
 *
 *    MiniDAQ which is a TCP/IP client for CSM, where the DAQ controlled
 *    by a PC MiniDAQ which is a TCP/IP server.
 *    To run, type:
 *    CSMMiniDAQ.exe <servername> <CSM Number>
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <math.h>
#include "sockhelp.h"       // public domain file provided by Socket FAQ
#include "rcc_error/rcc_error.h"
#include "io_rcc/io_rcc.h"
#include "cmem_rcc/cmem_rcc.h"
#include "rcc_time_stamp/tstamp.h"
#include "filar_utils.h"
#include "filar_storage.h"
#include "csm_utils.h"
#include "CSMMiniDAQ.h"

// -------- Direct this build --------
//#define LINUX
//#define DAQDEBUG
#define CHECKEXPECTEDDATA
// -------- Direct this build --------

#ifdef LINUX
  #define SHIFTBITS  0  
#else
  #define SHIFTBITS 24  
#endif
#ifdef DAQDEBUG
  #define DAQDebug(x) printf x
#else
  #define DAQDebug(x)
#endif

double maxUintValue = 4294967295.0;
int pipe1[2], pipe2[2], clientID, CSMNumber, sock, osock, normalQuit, nbRequestedBuf, bufnr;
unsigned int buffer[4*BUFFER_MAX], prevBuffer[4*BUFFER_MAX];         // incoming data with 8bit valid data
unsigned int totalDAQTime;
char DAQStartedAt[30];
pid_t ppid, cpid;

int main(int argc, char *argv[]) {
  struct sigaction sa;
  DIR *dirdat;
  pid_t pid;
  int i, j, connected, nrecord = 0, flength, status, command, enables;
  unsigned int rawData[20*MAXEVTRECORD], builtData[MAXEVTRECORD], rawAddr, evtAddr;
  unsigned int TCPData[BUFFER_MAX] = {0}, dataWord, partDataWord, checksum;
  unsigned int savedData[BUFFER_MAX] = {0}, remainRawData, remainOneEvt, remainOneSeqEvt;
  time_t localTime;
  char c, fname[256], myDateTime[20], execStr[256], dirName[60], str[256];

  if ((argc < 2)) {
    printf("Usage:      CSMMiniDAQ.exe <servername> <client ID (optional)>\n");
    printf("servername: The machine running the tcpserver program.\n");
    printf("client ID : The client ID (optional).\n");
    exit(EXIT_FAILURE);
  }
  ignore_pipe();
  // opening socket, where port is fixed as 4000
  sock = make_connection("4000", SOCK_STREAM, argv[1]);
  if (sock == -1) {
    printf("make_connection failed.\n");
    return -1;
  }

  osock = make_connection("12345",SOCK_STREAM,"141.211.96.35");
  // osock = make_connection("12346",SOCK_STREAM,"127.0.0.1");
  // osock = make_connection("12345",SOCK_STREAM,"141.213.133.230");
  // osock = make_connection("14175",SOCK_STREAM,"umt3int03.physics.lsa.umich.edu");
  if (osock == -1) {
    printf("ERROR: Make connection for output socket failed.\n");
  }
  
  normalQuit = TRUE;
  clientID = 0;
  if (argc >= 3) sscanf(argv[2], "%d", &clientID);
  CSMNumber = clientID;
  CSMNumber &= ~DATATYPEBITMASK;
  printf("\n<%s> Start CSMMiniDAQ (sock=%d) with clientID = %d.\n", argv[0], sock, clientID);
  dataAnalysisControl = 0;
  CheckDataAnalysisPackageInstallations();
  pipe(pipe1);
  pipe(pipe2);
  pid = fork();
  if (pid == -1) {
    perror("fork");
    return -2;
  }
  else if (pid == 0) {
    singleEvInitWasDone = 0;
    strcpy(dataTopDirName, "dat");
    dirdat = opendir(dataTopDirName);
    if (dirdat == NULL) {
      strcpy(execStr, "mkdir ");
      strcat(execStr, dataTopDirName);
      system(execStr);
    }
    else closedir(dirdat);
    // child process
    ppid = getppid();
    status = OpenGOLACard();
    if (status != 0) printf("Unable to connect with GOLA card, have to stop!\n");
    else status = InitDAQ();
    if (status != 0) printf("Unable to perform DAQ initialization, have to stop!\n");
    else { 
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      sa.sa_handler = ChildSigTERMHandler;
      status = sigaction(SIGTERM, &sa, NULL);
      if (status < 0) {
        perror("signal");
        printf("Can not install SIGTERM for child process!\n");
      }
      else {
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = ChildSigUSR1Handler;
        status = sigaction(SIGUSR1, &sa, NULL);
        if (status < 0) {
          perror("signal");
          printf("Can not install SIGUSR1 for child process!\n");
        }
        else {
          sigemptyset(&sa.sa_mask);
          sa.sa_flags = 0;
          sa.sa_handler = ChildSigUSR2Handler;
          status = sigaction(SIGUSR2, &sa, NULL);
          if (status < 0) {
            perror("signal");
            printf("Can not install SIGUSR2 for child process!\n");
          }
        }
      }
      if (status > 0) status = 0;
    }
    if (status == 0) {
      while (TRUE) {
        command = -1;
        read(pipe1[0], buffer, 4);
        if ((buffer[0] > 0) && (buffer[0] < BUFFER_MAX-1)) read(pipe1[0], &buffer[1], 4*buffer[0]);
        status = 0;
        if (buffer[0] > (unsigned int) BUFFER_MAX) {
          // buffer from pipe1 corrupted, discard it
          printf("Previous buffer from pipe1 has a length %2u, buffer[1] = 0x%08x\n", prevBuffer[0], prevBuffer[1]); 
          printf("Current  buffer from pipe1 has a length %2u, buffer[1] = 0x%08x\n", buffer[0], buffer[1]); 
          printf("Corrupted buffer from pipe1, will discard it\n");
          buffer[0] = 0;
          status = 1;
        }
        else {
          for (i = 0; i < buffer[0]; i++) prevBuffer[i] = buffer[i];
        }
        if (buffer[0] != 0) {
          checksum = 0;
          for (i = 0; i < buffer[0]; i++) checksum += buffer[i];
          status = 0;
          i = buffer[0];
#ifdef DAQDEBUG
          printf("Buffer from pipe1 has a length %d, buffer[1] = 0x%08x\n", buffer[0], buffer[1]); 
          printf( "checksum=0x%08x and buffer[i]=0x%08x\n", checksum, buffer[i] );
#endif
          if (checksum != buffer[i]) {
             printf("Buffer from pipe1 has a length %d, buffer[1] = 0x%08x\n", buffer[0], buffer[1]); 
	           printf( "checksum=0x%08x and buffer[i]=0x%08x\n", checksum, buffer[i] );
             status = 2;
          }
          else {
            command = buffer[TCPCOMMAND] & 0xFFFF;
            if (buffer[0] > TCPFILENAME) {
              flength = buffer[TCPFILENAME]; 
              CSMVersion = buffer[TCPCSMVERSION];
              synchWordControl = buffer[TCPSYNCHWORDCONTROL];
              mezzEnables = buffer[TCPMEZZENABLES];
              GOLAPageSize = buffer[TCPGOLACARDPAGESIZE];
              reqEVIDMatch = TRUE;
              AMTType = buffer[TCPAMTTYPE];
              if (AMTType == TYPEHPTDC) {
                MYTDC_GHEADER = HPTDC_GHEADER;
                MYTDC_GTRAILER = HPTDC_GTRAILER;
                MYTDC_HEADER = HPTDC_HEADER;
                MYTDC_TRAILER = HPTDC_TRAILER;
                MYTDC_LEDGE = HPTDC_LEDGE;
                MYTDC_PAIR = HPTDC_PAIR;
                MYTDC_TEDGE = HPTDC_TEDGE;
                MYTDC_ERROR = HPTDC_ERROR;
                MYTDC_DEBUG = HPTDC_DEBUG;
                MYTDC_MASK = -1;
                MYTDC_EDGE = -1;
                HPTDC = TRUE;
                errWordMask = 0x7FFF;
                TDCDivider = 4;
                widthDivider = 1;
              }
              else {
                MYTDC_HEADER = TDC_HEADER;
                MYTDC_TRAILER = TDC_TRAILER;
                MYTDC_MASK = TDC_MASK;
                MYTDC_EDGE = TDC_EDGE;
                MYTDC_PAIR = TDC_PAIR;
                MYTDC_ERROR = TDC_ERROR;
                MYTDC_DEBUG = TDC_DEBUG;
                MYTDC_GHEADER = -1;
                MYTDC_GTRAILER = -1;
                MYTDC_LEDGE = -1;
                MYTDC_TEDGE = -1;
                HPTDC = FALSE;
                errWordMask = 0x3FFF;
                TDCDivider = 1;
                widthDivider = 1;
              }
              saveRawData = buffer[TCPSAVERAWDATA]&1;
              saveFlaggedEvent = (buffer[TCPSAVERAWDATA]>>1)&1;
              MDTChamberType = buffer[TCPCHAMBERTYPE];
              MDTChamberHVStatus = buffer[TCPCHAMBERHV];
              suppressCSMHeader = buffer[TCPSUPPRESSCSMHEADER];
              suppressCSMTrailer = buffer[TCPSUPPRESSCSMTRAILER];
              suppressTDCHeader = buffer[TCPSUPPRESSAMTHEADER];
              suppressTDCTrailer = buffer[TCPSUPPRESSAMTTRAILER];
              runNumber = buffer[TCPRUNNUMBER];
              numberOfEvent = buffer[TCPNUMBEROFEVENT];
              validBCIDPresetValue = FALSE;              
              if (numberOfEvent == 0xFFFFFFFF) numberOfEvent = 0xFFFFFFFE;
              
              if (buffer[TCPNOMINALTHRESHOLD] == 0) nominalThresholdUsed = FALSE;
              else {
                nominalThresholdUsed = TRUE;
                nominalThreshold = buffer[TCPNOMINALTHRESHOLD] - 1024;
              }
              if (buffer[TCPINTEGRATIONGATE] == 0) {
                integrationGate = -1;
                minWidth = 0;
              }
              else {
                integrationGate = buffer[TCPINTEGRATIONGATE] - 1024;
                if (integrationGate == 0) minWidth = 22;
                else if (integrationGate == 1) minWidth = 26;
                else if (integrationGate == 2) minWidth = 31;
                else if (integrationGate == 3) minWidth = 36;
                else if (integrationGate == 4) minWidth = 42;
                else if (integrationGate == 5) minWidth = 46;
                else if (integrationGate == 6) minWidth = 49;
                else if (integrationGate == 7) minWidth = 53;
                else if (integrationGate == 8) minWidth = 58;
                else if (integrationGate == 9) minWidth = 62;
                else if (integrationGate == 10) minWidth = 65;
                else if (integrationGate == 11) minWidth = 68;
                else if (integrationGate == 12) minWidth = 72;
                else if (integrationGate == 13) minWidth = 77;
                else if (integrationGate == 14) minWidth = 80;
                else if (integrationGate == 15) minWidth = 83;
                else minWidth = 0;
              }
              if (buffer[TCPRUNDOWNCURRENT] == 0) {
                rundownCurrent = -1;
                minWidth = 0;
              }
              else rundownCurrent = buffer[TCPRUNDOWNCURRENT] - 1024;
              widthSelection = buffer[TCPWIDTHSELECTION];
              CSMPairDebug = buffer[TCPPAIRDEBUG];
              checkSystemNoise = buffer[TCPCHECKSYSTEMNOISE];
              mappingMDTChamber = buffer[TCPMAPPINGMDTCHAMBER];
              selectedTrigger = buffer[TCPTRIGGERSELECTION];
              selectedTriggerRate = buffer[TCPTRIGGERRATESELECTION];
              expectedTriggerRate = buffer[TCPEXPECTEDTRIGGERRATE];
              dataAnalysisControl = buffer[TCPANALYSISCONTROL];
              startMezzCard = buffer[TCPSTARTMEZZCARD];
            }
#ifdef DAQDEBUG
            printf("buffer Length          : %10d\n", buffer[TCPLENGTH]);
            printf("DAQ Control Command    : 0x%08x\n", buffer[TCPCOMMAND]);
            if (buffer[0] > TCPFILENAME) {
              printf("CSM Number             : %10d\n", buffer[TCPCSMNUMBER]);
              printf("CSM Version Number     : %10d\n", buffer[TCPCSMVERSION]);
              printf("Synchword Control      : %10d\n", buffer[TCPSYNCHWORDCONTROL]);
              printf("Mezzanine Card Enables : 0x%08x\n", buffer[TCPMEZZENABLES]);
              printf("Edge and Pair Enables 0: 0x%08x\n", buffer[TCPEDGEPAIRENABLES0]);
              printf("Edge and Pair Enables 1: 0x%08x\n", buffer[TCPEDGEPAIRENABLES1]);
              printf("GOLA Card Page Size    : %10d\n", buffer[TCPGOLACARDPAGESIZE]);
              printf("AMT Type               : %10d\n", buffer[TCPAMTTYPE]);
              printf("Save Raw Data          : %10d\n", buffer[TCPSAVERAWDATA]);
              printf("MDT Chamber Type       : %10d\n", buffer[TCPCHAMBERTYPE]);
              printf("MDT Chamber HV Status  : %10d\n", buffer[TCPCHAMBERHV]);
              printf("Suppress CSM Header    : %10d\n", buffer[TCPSUPPRESSCSMHEADER]);
              printf("Suppress CSM Trailer   : %10d\n", buffer[TCPSUPPRESSCSMTRAILER]);
              printf("Suppress AMT Header    : %10d\n", buffer[TCPSUPPRESSAMTHEADER]);
              printf("Suppress AMT Trailer   : %10d\n", buffer[TCPSUPPRESSAMTTRAILER]);
              printf("Run Number             : %10d\n", buffer[TCPRUNNUMBER]);
              printf("Number of Event        : %10d\n", buffer[TCPNUMBEROFEVENT]);
              printf("Nominal Threshold      : %10d\n", buffer[TCPNOMINALTHRESHOLD]);
              printf("ASD W. Integration Gate: %10d\n", buffer[TCPINTEGRATIONGATE]);
              printf("ASD Rundown Current    : %10d\n", buffer[TCPRUNDOWNCURRENT]);
              printf("Width Selection        : %10d\n", buffer[TCPWIDTHSELECTION]);
              printf("Pair Debug Mode        : %10d\n", buffer[TCPPAIRDEBUG]);
              printf("Check System Noise     : %10d\n", buffer[TCPCHECKSYSTEMNOISE]);
              printf("Mapping MDT Chamber    : %10d\n", buffer[TCPMAPPINGMDTCHAMBER]);
              printf("Selected TTCvi Trigger : %10d\n", buffer[TCPTRIGGERSELECTION]);
              printf("Selected Trigger Rate  : %10d\n", buffer[TCPTRIGGERRATESELECTION]);
              printf("ExpectedTriggerRate(Hz): %10d\n", buffer[TCPEXPECTEDTRIGGERRATE]);
              printf("Max. Allowed MezzCards : %10d\n", buffer[TCPMAXALLOWEDMEZZS]);
              printf("Data Analysis Control  : %10d\n", buffer[TCPANALYSISCONTROL]);
              printf("Start MezzCard Number  : %10d\n", buffer[TCPSTARTMEZZCARD]);
              printf("Spare  7               : %10d\n", buffer[TCPSPARE07]);
              printf("Spare  6               : %10d\n", buffer[TCPSPARE06]);
              printf("Spare  5               : %10d\n", buffer[TCPSPARE05]);
              printf("Spare  4               : %10d\n", buffer[TCPSPARE04]);
              printf("Spare  3               : %10d\n", buffer[TCPSPARE03]);
              printf("Spare  2               : %10d\n", buffer[TCPSPARE02]);
              printf("Spare  1               : %10d\n", buffer[TCPSPARE01]);
            }
#endif
            if (buffer[0] > TCPFILENAME) {
              if (flength > 0) {
                j = 0;
                i = TCPFILENAME + 1;
                while (j < flength) {
                  fname[j++] = (char) (buffer[i] & 0xFF);
                  fname[j++] = (char) ((buffer[i] >> 8) & 0xFF);
                  fname[j++] = (char) ((buffer[i] >> 16) & 0xFF);
                  fname[j++] = (char) ((buffer[i++] >> 24) & 0xFF);
                }
                fname[flength] = '\0';
#ifdef DAQDEBUG
                printf("Filename length        : %10d\n", flength);
                printf("Filename               : %s\n", fname);
#endif
                j = 0;
                strcpy(dirName, "");
                for (i = 0; i < strlen(fname); i++) {
                  if (fname[i] != '/') str[j++] = fname[i];
                  else {
                    str[j] = '\0';
                    j = 0;
                    strcpy(dirName, str);
                  } 
                }
                str[j] = '\0';
                strcpy(fname, str);
                if (strlen(dirName) <= 0) strcpy(CSMDataFilename, fname);
                else if (strcmp(dirName, dataTopDirName) == 0) {
                  strcpy(dataDirName, dataTopDirName);
                  sprintf(CSMDataFilename, "%s/%s", dataDirName, fname);
                }
                else {
                  sprintf(dataDirName, "%s/%s", dataTopDirName, dirName);                  
                  sprintf(CSMDataFilename, "%s/%s", dataDirName, fname);
                  dirdat = opendir(dataDirName);
                  if (dirdat == NULL) {
                    strcpy(execStr, "mkdir ");
                    strcat(execStr, dataDirName);
                    system(execStr);
                  }
                  else closedir(dirdat);
                }
              }
              else strcpy(CSMDataFilename, "/dev/null");
              if (saveRawData) {
                if ((strlen(CSMDataFilename) > 4) && (strstr(CSMDataFilename, "/dev/null") == NULL)) {
                   strncpy(rawDataFilename, CSMDataFilename, strlen(CSMDataFilename)-4);
                   rawDataFilename[strlen(CSMDataFilename)-4] = '\0';
                   strcat(rawDataFilename, ".raw");
                }
                else {
                  localTime = time(NULL);
                  strftime(myDateTime, 20, "%Y%m%d", localtime(&localTime));
                  sprintf(rawDataFilename,"%s/run%08u_%8s.raw", dataDirName, runNumber, myDateTime);
                }
              }
              else strcpy(rawDataFilename, "");
            }
          }
          TCPData[0] = 3;
          TCPData[1] = DATATYPESTATUSONLY | CSMNumber;
          if (status != 0);   // Received data has problem, do nothing 
          else if (command == CMDINITDAQ) status = InitDAQ();
          else if (command == CMDSTARTRUN) status = InitToStartRun(TRUE);
          else if (command == CMDSTOPRUN) {
            status = 0;
            DAQState = State_Idle;
          }
          else if (command == CMDGETONEEVENTINIT) status = InitToStartRun(FALSE);
          else if (command == CMDREQALLDAQINFOR) {
            TCPData[0] = 4+NUMBERANALYSISCOUNTER;
            TCPData[1] = DATATYPEALLDAQINFOR | CSMNumber;
            TCPData[3] = 0;
            for (i = 0; i < NUMBERANALYSISCOUNTER; i++) TCPData[4+i] = analysisCounter[i];
          }
          else if (command == CMDREQRAWDATA) {
            if (remainRawData > 0) {
              j = singleEvInfo.bufSize - remainRawData;
              TCPData[0] = 4+remainRawData;
              TCPData[1] = DATATYPERAWDATA | CSMNumber;
              TCPData[3] = 0;
              if (TCPData[0] > BUFFER_MAX-2) {
                TCPData[0] = BUFFER_MAX - 2; 
                remainRawData = singleEvInfo.bufSize - (BUFFER_MAX - 6);  
              }
              else remainRawData = 0;
              for (i = 0; i < TCPData[0]-4; i++) TCPData[4+i] = rawData[j+i];
            }
            else if (CollectSeqEvent(rawData, 20*MAXEVTRECORD, builtData, MAXEVTRECORD)) {
              TCPData[0] = 4+singleEvInfo.bufSize;
              TCPData[1] = DATATYPERAWDATA | CSMNumber;
              TCPData[3] = 0;
              if (TCPData[0] > BUFFER_MAX-2) {
                printf("Raw data size (%d) is larger than allowed (%d).\n", TCPData[0], BUFFER_MAX-2);
                TCPData[0] = BUFFER_MAX - 2;
                remainRawData = singleEvInfo.bufSize - (BUFFER_MAX - 6);  
              }
              else remainRawData = 0;
              for (i = 0; i < TCPData[0]-4; i++) TCPData[4+i] = rawData[i];
            }
          }
          else if (command == CMDREQONEEVENT) {
            if (remainOneEvt > 0) {
              j = singleEvInfo.eventSize - remainOneEvt;
              TCPData[0] = 4+remainOneEvt;
              TCPData[1] = DATATYPEEVENT | CSMNumber;
              TCPData[3] = 0;
              if (TCPData[0] > BUFFER_MAX-2) {
                TCPData[0] = BUFFER_MAX - 2; 
                remainOneEvt = singleEvInfo.eventSize - (BUFFER_MAX - 6);  
              }
              else remainOneEvt = 0;
              for (i = 0; i < TCPData[0]-4; i++) TCPData[4+i] = builtData[j+i];
            }
            else if (CollectOneEvent(rawData, 20*MAXEVTRECORD, builtData, MAXEVTRECORD)) {
              TCPData[0] = 4+singleEvInfo.eventSize;
              TCPData[1] = DATATYPEEVENT | CSMNumber;
              TCPData[3] = 0;
              if (TCPData[0] > BUFFER_MAX-2) {
                printf("Event size (%d) is larger than allowed (%d).\n", TCPData[0], BUFFER_MAX-2);
                TCPData[0] = BUFFER_MAX - 2;
                remainOneEvt = singleEvInfo.eventSize - (BUFFER_MAX - 6);  
              }
              else remainOneEvt = 0;
              for (i = 0; i < TCPData[0]-4; i++) TCPData[4+i] = builtData[i];
            }
          }
          else if (command == CMDREQONESEQEVENT) {
            if (remainOneSeqEvt > 0) {
              j = singleEvInfo.eventSize - remainOneSeqEvt;
              TCPData[0] = 4+remainOneSeqEvt;
              TCPData[1] = DATATYPEEVENT | CSMNumber;
              TCPData[3] = 0;
              if (TCPData[0] > BUFFER_MAX-2) {
                TCPData[0] = BUFFER_MAX - 2; 
                remainOneSeqEvt = singleEvInfo.eventSize - (BUFFER_MAX - 6);  
              }
              else remainOneSeqEvt = 0;
              for (i = 0; i < TCPData[0]-4; i++) TCPData[4+i] = builtData[j+i];
            }
            else if (CollectSeqEvent(rawData, 20*MAXEVTRECORD, builtData, MAXEVTRECORD)) {
              TCPData[0] = 4+singleEvInfo.eventSize;
              TCPData[1] = DATATYPEEVENT | CSMNumber;
              TCPData[3] = 0;
              if (TCPData[0] > BUFFER_MAX-2) {
                printf("Event size (%d) is larger than allowed (%d).\n", TCPData[0], BUFFER_MAX-2);
                TCPData[0] = BUFFER_MAX - 2;
                remainOneSeqEvt = singleEvInfo.eventSize - (BUFFER_MAX - 6);  
              }
              else remainOneSeqEvt = 0;
              for (i = 0; i < TCPData[0]-4; i++) TCPData[4+i] = builtData[i];
            }
          }
          else if (command == CMDREQEDGES) {
            TCPData[0] = 4+49*MAXNUMBERMEZZANINE;
            TCPData[1] = DATATYPEEDGES | CSMNumber;
            TCPData[3] = 0;
            for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
              TCPData[4+i] = nEvents[i];
              for (j = 0; j < 24; j++) {
                TCPData[4+MAXNUMBERMEZZANINE+48*i+j] = nEdge[0][j][i];
                TCPData[28+MAXNUMBERMEZZANINE+48*i+j] = nEdge[1][j][i];
              }
            }
          }
          else if (command == CMDREQPAIR) {
            TCPData[0] = 4+49*MAXNUMBERMEZZANINE;
            TCPData[1] = DATATYPEPAIR | CSMNumber;
            TCPData[3] = 0;
            for (i = 0; i < MAXNUMBERMEZZANINE; i++) { 
              TCPData[4+i] = nEvents[i];
              for (j = 0; j < 24; j++) {
                TCPData[4+MAXNUMBERMEZZANINE+48*i+j] = nPair[j][i];
                TCPData[28+MAXNUMBERMEZZANINE+48*i+j] = nPair[j][i];  // Just repeat, could be replaced
              }
            }
          }
          else if (command == CMDREQHITS) {
            TCPData[0] = 4+49*MAXNUMBERMEZZANINE;
            TCPData[1] = DATATYPEHITS | CSMNumber;
            TCPData[3] = 0;
            for (i = 0; i < MAXNUMBERMEZZANINE; i++) { 
              TCPData[4+i] = nEvents[i];
              for (j = 0; j < 24; j++) {
                TCPData[4+MAXNUMBERMEZZANINE+48*i+j] = nGoodHit[j][i];
                TCPData[28+MAXNUMBERMEZZANINE+48*i+j] = nASDNoise[j][i];
              }
            }
          }
          else if (command == CMDREQAVERAGETDCTIME) {
            CalculateAverageTDCTimeAndSigma();
            TCPData[0] = 4+49*MAXNUMBERMEZZANINE;
            TCPData[1] = DATATYPEAVERAGETDCTIME | CSMNumber;
            TCPData[3] = 0;
            for (i = 0; i < MAXNUMBERMEZZANINE; i++) { 
              if (numberOfEvent > 0) TCPData[4+i] = nEvents[i];
              else TCPData[4+i] = analysisCounter[ANALYSEDEVENT];
              for (j = 0; j < 24; j++) {
                if (timeAverage[j][i] < 0.0) TCPData[4+MAXNUMBERMEZZANINE+48*i+j] = 0xFFFFFFFF;
                else TCPData[4+MAXNUMBERMEZZANINE+48*i+j] = (unsigned int) (1000.0 * timeAverage[j][i]);
                if (timeSigma[j][i] < 0.0) TCPData[28+MAXNUMBERMEZZANINE+48*i+j] = 0xFFFFFFFF;
                else TCPData[28+MAXNUMBERMEZZANINE+48*i+j] = (unsigned int) (1000.0 * timeSigma[j][i]);
              }
            }
          }
          else if (command == CMDREQFRACTIONUSED) {
            CalculateAverageTDCTimeAndSigma();
            CalculateAverageWidthAndSigma();
            TCPData[0] = 4+49*MAXNUMBERMEZZANINE;
            TCPData[1] = DATATYPEFRACTIONUSED | CSMNumber;
            TCPData[3] = 0;
            for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
              if (numberOfEvent > 0) TCPData[4+i] = nEvents[i];
              else TCPData[4+i] = analysisCounter[ANALYSEDEVENT];
              for (j = 0; j < 24; j++) {
                if (fracUsed[j][i] < 0.0) TCPData[4+MAXNUMBERMEZZANINE+48*i+j] = 0xFFFFFFFF;
                else TCPData[4+MAXNUMBERMEZZANINE+48*i+j] = (unsigned int) (1000.0 * fracUsed[j][i]);
                if (fracUsedInWidth[j][i] < 0.0) TCPData[28+MAXNUMBERMEZZANINE+48*i+j] = 0xFFFFFFFF;
                else TCPData[28+MAXNUMBERMEZZANINE+48*i+j] = (unsigned int) (1000.0 * fracUsedInWidth[j][i]);
              }
            }
          }
          else if (command == CMDREQAVERAGEWIDTH) {
            CalculateAverageWidthAndSigma();
            TCPData[0] = 4+49*MAXNUMBERMEZZANINE;
            TCPData[1] = DATATYPEAVERAGEWIDTH | CSMNumber;
            TCPData[3] = 0;
            for (i = 0; i < MAXNUMBERMEZZANINE; i++) { 
              if (numberOfEvent > 0) TCPData[4+i] = nEvents[i];
              else TCPData[4+i] = analysisCounter[ANALYSEDEVENT];
              for (j = 0; j < 24; j++) {
                if (widthAverage[j][i] < 0.0) TCPData[4+MAXNUMBERMEZZANINE+48*i+j] = 0xFFFFFFFF;
                else TCPData[4+MAXNUMBERMEZZANINE+48*i+j] = (unsigned int) (1000.0 * widthAverage[j][i]);
                if (widthSigma[j][i] < 0.0) TCPData[28+MAXNUMBERMEZZANINE+48*i+j] = 0xFFFFFFFF;
                else TCPData[28+MAXNUMBERMEZZANINE+48*i+j] = (unsigned int) (1000.0 * widthSigma[j][i]);
              }
            }
          }
          if (command != CMDREQRAWDATA) remainRawData = 0;
          if (command != CMDREQONEEVENT) remainOneEvt = 0;
          if (command != CMDREQONESEQEVENT) remainOneSeqEvt = 0;
          if (command == CMDREQDAQINFOR) ChildSigUSR1Handler(0);
          else {
            TCPData[2] = status;
            checksum = 0;
            for (i = 0; i < TCPData[0]; i++) checksum += TCPData[i];
            TCPData[TCPData[0]] = checksum;
            write(pipe2[1], TCPData, 4*(TCPData[0]+1));
            if (status == 0) {
              if (command == CMDQUIT) break;
              else if (command == CMDSTARTRUN) CollectCSMData();
            }
          }
        }
        DAQState = State_Idle;
      }            // end while(TRUE)
      CloseGOLACard();
    }
    else {
      printf("Unable to start child process due to error(s), quit now!.\n");
      kill(ppid, SIGTERM);
    }
    exit (0);
  }
  else {
    singleEvInitWasDone = 0;
    cpid = pid;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = ParentSigTERMHandler;
    status = sigaction(SIGTERM, &sa, NULL);
    if (status < 0) {
      perror("signal");
      printf("Can not install SIGTERM for parent process!\n");
    }
    else {
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      sa.sa_handler = ParentSigUSR1Handler;
      status = sigaction(SIGUSR1, &sa, NULL);
      if (status < 0) {
        perror("signal");
        printf("Can not install SIGUSR1 for Parent process!\n");
      }
      else {
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = ParentSigUSR2Handler;
        status = sigaction(SIGUSR2, &sa, NULL);
        if (status < 0) {
          perror("signal");
          printf("Can not install SIGUSR2 for Parent process!\n");
        }
      }
    }
    if (status >= 0) {
      // parent process
      // read first stream of data
      if (sock_read(sock, (char *) buffer, sizeof(buffer)) != -1) connected = 1; 
      while (connected) {
        nrecord++;
        command = -1;
        for (i = 0; i < BUFFER_MAX; i++) {
          dataWord = 0;
          for (j = 0; j < 4; j++) {
            partDataWord = (unsigned int) ((buffer[4*i+j] >> SHIFTBITS) & 0xFF);
            dataWord |= (partDataWord << (8*j));
          }
          TCPData[i] = dataWord;
        }
        status = 0;
        if (TCPData[0] > (unsigned int) BUFFER_MAX) {
          // TCP buffer corrupted, discard it
          TCPData[0] = 0;
          status = 1;
          printf("Corrupted TCP buffer, will discard it\n");
        }
        if (TCPData[0] != 0) {
#ifdef DAQDEBUG
          printf("\nTCPData has a length %d, nrecord = %d\n", TCPData[0], nrecord);
#endif
          checksum = 0;
          for (i = 0; i < TCPData[0]; i++) checksum += TCPData[i];
          status = 0;
          i = TCPData[0];
          if (checksum != TCPData[i]) status = 2;
          else {
            command = TCPData[TCPCOMMAND] & 0xFFFF;
            if (TCPData[0] >= TCPFILENAME) {
              flength = TCPData[TCPFILENAME]; 
              dataAnalysisControl = TCPData[TCPANALYSISCONTROL];
            }
#ifdef DAQDEBUG
            printf("TCPData Length         : %10d\n", TCPData[TCPLENGTH]);
            printf("DAQ Control Command    : 0x%08x\n", TCPData[TCPCOMMAND]);
            if (TCPData[0] >= TCPFILENAME) {
              printf("CSM Number             : %10d\n", TCPData[TCPCSMNUMBER]);
              printf("CSM Version Number     : %10d\n", TCPData[TCPCSMVERSION]);
              printf("Synchword Control      : %10d\n", TCPData[TCPSYNCHWORDCONTROL]);
              printf("Mezzanine Card Enables : 0x%08x\n", TCPData[TCPMEZZENABLES]);
              printf("Edge and Pair Enables 0: 0x%08x\n", TCPData[TCPEDGEPAIRENABLES0]);
              printf("Edge and Pair Enables 1: 0x%08x\n", TCPData[TCPEDGEPAIRENABLES1]);
              printf("GOLA Card Page Size    : %10d\n", TCPData[TCPGOLACARDPAGESIZE]);
              printf("AMT Type               : %10d\n", TCPData[TCPAMTTYPE]);
              printf("Save Raw Data          : %10d\n", TCPData[TCPSAVERAWDATA]);
              printf("MDT Chamber Type       : %10d\n", TCPData[TCPCHAMBERTYPE]);
              printf("MDT Chamber HV Status  : %10d\n", TCPData[TCPCHAMBERHV]);
              printf("Suppress CSM Header    : %10d\n", TCPData[TCPSUPPRESSCSMHEADER]);
              printf("Suppress CSM Trailer   : %10d\n", TCPData[TCPSUPPRESSCSMTRAILER]);
              printf("Suppress AMT Header    : %10d\n", TCPData[TCPSUPPRESSAMTHEADER]);
              printf("Suppress AMT Trailer   : %10d\n", TCPData[TCPSUPPRESSAMTTRAILER]);
              printf("Run Number             : %10d\n", TCPData[TCPRUNNUMBER]);
              printf("Number of Event        : %10d\n", TCPData[TCPNUMBEROFEVENT]);
              printf("Nominal Threshold      : %10d\n", TCPData[TCPNOMINALTHRESHOLD]);
              printf("ASD W. Integration Gate: %10d\n", TCPData[TCPINTEGRATIONGATE]);
              printf("ASD Rundown Current    : %10d\n", TCPData[TCPRUNDOWNCURRENT]);
              printf("Width Selection        : %10d\n", TCPData[TCPWIDTHSELECTION]);
              printf("Pair Debug Mode        : %10d\n", TCPData[TCPPAIRDEBUG]);
              printf("Check System Noise     : %10d\n", TCPData[TCPCHECKSYSTEMNOISE]);
              printf("Mapping MDT Chamber    : %10d\n", TCPData[TCPMAPPINGMDTCHAMBER]);
              printf("Selected TTCvi Trigger : %10d\n", TCPData[TCPTRIGGERSELECTION]);
              printf("Selected Trigger Rate  : %10d\n", TCPData[TCPTRIGGERRATESELECTION]);
              printf("ExpectedTriggerRate(Hz): %10d\n", TCPData[TCPEXPECTEDTRIGGERRATE]);
              printf("Max. Allowed MezzCards : %10d\n", TCPData[TCPMAXALLOWEDMEZZS]);
              printf("Data Analysis Control  : %10d\n", TCPData[TCPANALYSISCONTROL]);
              printf("Start MezzCard Number  : %10d\n", TCPData[TCPSTARTMEZZCARD]);
              printf("Spare  7               : %10d\n", TCPData[TCPSPARE07]);
              printf("Spare  6               : %10d\n", TCPData[TCPSPARE06]);
              printf("Spare  5               : %10d\n", TCPData[TCPSPARE05]);
              printf("Spare  4               : %10d\n", TCPData[TCPSPARE04]);
              printf("Spare  3               : %10d\n", TCPData[TCPSPARE03]);
              printf("Spare  2               : %10d\n", TCPData[TCPSPARE02]);
              printf("Spare  1               : %10d\n", TCPData[TCPSPARE01]);
            }
#endif
            if (TCPData[0] >= TCPFILENAME) {
              if (flength > 0) {
                j = 0;
                i = TCPFILENAME + 1;
                while (j < flength) {
                  fname[j++] = (char) (TCPData[i] & 0xFF);
                  fname[j++] = (char) ((TCPData[i] >> 8) & 0xFF);
                  fname[j++] = (char) ((TCPData[i] >> 16) & 0xFF);
                  fname[j++] = (char) ((TCPData[i++] >> 24) & 0xFF);
                }
                fname[flength] = '\0'; 
#ifdef DAQDEBUG
                printf("Filename length        : %10d\n", flength);
                printf("Filename               : %s\n", fname);
#endif
              }
            }
          }
          buffer[0] = 3;
          buffer[1] = CSMNumber;
          buffer[2] = status;
          checksum = 0;
          for (i = 0; i < buffer[0]; i++) checksum += buffer[i];
          buffer[buffer[0]] = checksum;
          if (status == 0) {
            if (command == CMDDETAILSTATUS) {
              CheckDataAnalysisPackageInstallations();
              CheckDataAnalysisProcessingState();
              buffer[0] = 8;
              buffer[1] = DATATYPEDETAILSTATUS | CSMNumber;
              buffer[2] = 0;
              buffer[3] = 0;
              buffer[4+CLIENTRUNSTATE] = DAQState;
              buffer[4+CLIENTANAINSTALLED] = anaInstalled;
              buffer[4+CLIENTCERNLIBINSTALLED] = CERNLibInstalled;
              buffer[4+CLIENTPROCESSINGDATA] = processingData;
              checksum = 0;
              for (i = 0; i < buffer[0]; i++) checksum += buffer[i];
              buffer[buffer[0]] = checksum;
            }
            else if (command == CMDPAUSERUN) DAQState = State_Paused;
            else if (command == CMDRESUMERUN) DAQState = State_Running;
            else if (command == CMDREREQDAQINFOR) {
              for (i = 0; i < savedData[0]+1; i++) TCPData[i] = savedData[i];
            }
            else {
              if (command == CMDINITDAQ) DAQState = State_Idle;
              else if (command == CMDSTARTRUN) DAQState = State_Running;
              else if (command == CMDSTOPRUN) {
                if (DAQState != State_Idle) kill(cpid, SIGUSR2);                
                DAQState = State_Idle;
              }
              i = TCPData[0];
              if ((command == CMDREQDAQINFOR) && (DAQState != State_Idle)) kill(cpid, SIGUSR1);
              else {
                write(pipe1[1], TCPData, 4*(TCPData[0]+1));
              }
              read(pipe2[0], buffer, 4);
              if ((buffer[0] > 0) && (buffer[0] < BUFFER_MAX-1)) read(pipe2[0], &buffer[1], 4*buffer[0]);
              else {
                buffer[0] = 3;
                buffer[1] = CSMNumber;
                buffer[2] = 1;
                buffer[3] = buffer[0] + buffer[1] + buffer[2];
              }
              for (i = 0; i < buffer[0]+1; i++) savedData[i] = buffer[i];
            }
          }
          for (i = 0; i < buffer[0]+1; i++) TCPData[i] = buffer[i];
          sock_write(sock, (const char *) TCPData, sizeof(TCPData));
        }
        if (command == CMDQUIT) break; 
        // read next stream
        if (sock_read(sock, (char *) buffer, sizeof(buffer)) == -1) {
          connected = 0;
          if (normalQuit) {
            printf("Disconnected by server due to one of following reasons:\n");
            if (nrecord <= 1) {
              printf("1) Multiple DAQ clients (most likely)\n");
              printf("2) Disconnected by TCP/IP Server (MiniDAQ at PC);\n");
              printf("3) TCP/IP Communatcation broken, check other TCP/IP node;\n");
              printf("4) TCP/IP Server (MiniDAQ at PC) crashed.\n"); 
            }
            else {
              printf("1) Disconnected by TCP/IP Server (MiniDAQ at PC);\n");
              printf("2) TCP/IP Communatcation broken;\n");
              printf("3) TCP/IP Server (MiniDAQ at PC) crashed.\n"); 
            }
          }
        }
      }
      if (connected && (command == CMDQUIT)) sleep(1);
      close(sock);
      close(osock);
    }
    else printf("Unable to start parent process due to error(s), quit now!\n");
    kill(cpid, SIGTERM);
    wait(NULL);
  }

  printf("CSMMiniDAQ Done.\n");
  return (0);
}


void CheckDataAnalysisPackageInstallations(void) {
  int i, j, strl;
  char filename[1024], outputName[1024], str[1024], cstr[60], *strp;
  FILE *pathFile, *tmpFile;

  strcpy(anaPath, "");
  strcpy(CERNLibPath, "");
  strcpy(filename, "dataAnalysisInstallationPath.txt");
  if (pathFile = fopen(filename, "r")) {
    while (feof(pathFile) == 0) {
      fgets(str, 1024, pathFile);
      i = 0;
      while(str[i] == ' ' || str[i] == '\t') i++;
      for (j = i; j < strlen(str)+1; j++) str[j-i] = str[j];
      if ((str[0] != '/') || (str[1] != '/')) {
        strcpy(cstr, "dataAnalysisPackagePath");
        strp = (char *) strstr(str, cstr);
        if (strp != NULL) {
          strcpy(str, strp);
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || (str[j] == '\t') || (str[j] == ':') || (str[j] == '=')) j++;
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%s", anaPath);
        }
        strcpy(cstr, "CERNLibraryPath");
        strp = (char *) strstr(str, cstr);
        if (strp != NULL) {
          strcpy(str, strp);
          strl = strlen(cstr);
          for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
          j = 0;
          while((str[j] == ' ') || (str[j] == '\t') || (str[j] == ':') || (str[j] == '=')) j++;
          for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
          sscanf(str, "%s", CERNLibPath);
        }
      }
      if (feof(pathFile) != 0) break;
    }
  }
  else {
    strcpy(anaPath, "~/chamber/bin");
    strcpy(CERNLibPath, "/cern");
    if (pathFile = fopen(filename, "w")) {
      fprintf(pathFile, "// This file defines the path for data analysis and CERN library (if they are installed)\n");
      fprintf(pathFile, "// A) Data analysis package installation path (Default = %s)\n", anaPath);
      fprintf(pathFile, "//    if it is installed not at default location, please modify following line\n");
      fprintf(pathFile, "dataAnalysisPackagePath = %s\n", anaPath);
      fprintf(pathFile, "// Note! Don't change key string \"dataAnalysisPackagePath\"\n");
      fprintf(pathFile, "// B) CERN library installation path (Default = %s)\n", CERNLibPath);
      fprintf(pathFile, "//    if it is installed not at default location, please modify following line\n");
      fprintf(pathFile, "CERNLibraryPath = %s\n", CERNLibPath);
      fprintf(pathFile, "// Note! Don't change key string \"CERNLibraryPath\"\n");
    }
    else {
      printf("Unable to open data analysis package installation path file <%s>\n", filename);
      printf("You may not have write privillage on current working directory or disk full, please check it!\n");
    }
  }
  if (pathFile != NULL) fclose(pathFile);

  if (strlen(anaPath) <= 0) strcpy(anaPath, "~/chamber/bin");
  if (strlen(CERNLibPath) <= 0) strcpy(CERNLibPath, "/cern");
  anaInstalled = FALSE;
  CERNLibInstalled = FALSE;
  i = 0;
  sprintf(filename, "/tmp/checkDataAnalysisPackageInstallationsTMP%03d.job", i++);
  while (tmpFile = fopen(filename, "r")) {
    fclose(tmpFile);
    sprintf(filename, "/tmp/checkDataAnalysisPackageInstallationsTMP%03d.job", i++);
    if (i >= 1000) break;
  }
  if (tmpFile = fopen(filename, "w")) {
    fprintf(tmpFile, "#!/bin/csh -f\n");
    fprintf(tmpFile, "#\n");
    fprintf(tmpFile, "set ANAPATH = %s\n", anaPath);
    fprintf(tmpFile, "set FILE0 = ${ANAPATH}/tdcSpectrum.job\n");
    fprintf(tmpFile, "set FILE1 = ${ANAPATH}/tdcSpectrum.exe\n");
    fprintf(tmpFile, "set FILE2 = ${ANAPATH}/readdata5.exe\n");
    fprintf(tmpFile, "set CERNPATH = %s\n", CERNLibPath);
    fprintf(tmpFile, "set ANAINSTALLED = Yes\n");
    fprintf(tmpFile, "set CERNLIBINSTALLED = Yes\n");
    fprintf(tmpFile, "\n");
    fprintf(tmpFile, "if (!(-d ${ANAPATH})) set ANAINSTALLED = No\n");
    fprintf(tmpFile, "if (!(-f ${FILE0})) set ANAINSTALLED = No\n");
    fprintf(tmpFile, "if (!(-f ${FILE1})) set ANAINSTALLED = No\n");
    fprintf(tmpFile, "if (!(-f ${FILE2})) set ANAINSTALLED = No\n");
    fprintf(tmpFile, "if (!(-d ${CERNPATH})) set CERNLIBINSTALLED = No\n");
    fprintf(tmpFile, "if (\"${ANAINSTALLED}\" == \"Yes\") echo \"Data analysis package is installed.\"\n");
    fprintf(tmpFile, "if (\"${CERNLIBINSTALLED}\" == \"Yes\") echo \"CERN library is installed.\"\n");
    fprintf(tmpFile, "\n");
    fprintf(tmpFile, "exit 0\n");
    fclose(tmpFile);
  }
  else {
    printf("Unable to open job file <%s>\n", filename);
    printf("You may not have write privillage on current working directory or disk full, please check it!\n");
    return;
  }
  sprintf(CMDString, "chmod 755 %s", filename);
  system(CMDString);
  sprintf(outputName, "%s.output", filename);
  strcpy(CMDString, filename);
  strcat(CMDString, " > ");
  strcat(CMDString, outputName);
  system(CMDString);
  tmpFile = fopen(outputName, "r");
  while (feof(tmpFile) == 0) {
    fgets(str, 1024, tmpFile);
    strp = (char *) strstr(str, "Data analysis package is installed");
    if (strp != NULL) anaInstalled = TRUE;
    strp = (char *) strstr(str, "CERN library is installed");
    if (strp != NULL) CERNLibInstalled = TRUE;
    if (feof(tmpFile) != 0) break;
  }
  if (tmpFile != NULL) fclose(tmpFile);
  sprintf(CMDString, "/bin/rm -f %s", filename);
  system(CMDString);
  sprintf(CMDString, "/bin/rm -f %s", outputName);
  system(CMDString);

  strcpy(filename, "dataAnalysisInstallationPath.txt");
  if ((dataAnalysisControl > 0) && (!anaInstalled)) {
    printf("* The data analysis package is not installed at this machine (Linux Box).\n");
    printf("  To process data after data taking, it is required to install the data analysis\n");
    printf("  package at Linux Box. Do as following to install data analysis package:\n");
    printf("  1) Obtain data analysis package for singile chamber (chamber.tar.gz) at\n");
    printf("     http://atlas.physics.lsa.umich.edu/docushare/dsweb/View/Collection-399;\n");
    printf("  2) It is recommended to install data analysis package at user's home directory\n");
    printf("     do as following at Linux box:\n");
    printf("     A) Copy chamber.tar.gz at user's home direcotory;\n");
    printf("     B) tar -zxvf chamber.tar.gz, where a directory \"chamber\" will be installed;\n");
    printf("  3) If data analysis package is not installed at user's home directory (for instance,\n");
    printf("     installed at another user's home directory), start the CSMMiniDAQ at Linux Box\n");
    printf("     and modify the text file \"chamberDataAnalysisInstallationPath.txt\" according to\n");
    printf("     its instruction.\n");
    if (!CERNLibInstalled) {
      printf("* The CERN library is not installed at this machine (Linux Box).\n");
      printf("  In case user wishes to view data analysis results (histograms), it is necessary to\n");
      printf("  install CERN library at Linux Box, otherwise ignore it. To install CERN library, ask\n");
      printf("  your computer adminstrator to help.\n");
    }
  }
}


void CheckDataAnalysisProcessingState(void) {
  int i, j, strl;
  char filename[1024], outputName[1024], str[1024], *strp;
  FILE *tmpFile;

  processingData = FALSE;
  i = 0;
  sprintf(filename, "/tmp/checkDataAnalysisProcessingStateTMP%03d.job", i++);
  while (tmpFile = fopen(filename, "r")) {
    fclose(tmpFile);
    sprintf(filename, "/tmp/checkDataAnalysisProcessingStateTMP%03d.job", i++);
    if (i >= 1000) break;
  }
  if (tmpFile = fopen(filename, "w")) {
    fprintf(tmpFile, "#!/bin/csh -f\n");
    fprintf(tmpFile, "#\n");
    fprintf(tmpFile, "set PROCESS0 = `ps -af | grep -v \"grep -c\" | grep -c tdcSpectrum`\n");
    fprintf(tmpFile, "set PROCESS1 = `ps -af | grep -v \"grep -c\" | grep -c readdata5.exe`\n");
    fprintf(tmpFile, "\n");
    fprintf(tmpFile, "if ((${PROCESS0} != 0) || (${PROCESS1} != 0)) echo \"Still processing data\"\n"); 
    fprintf(tmpFile, "\n");
    fprintf(tmpFile, "exit 0\n");
    fclose(tmpFile);
  }
  else {
    printf("Unable to open job file <%s>\n", filename);
    printf("You may not have write privillage on current working directory or disk full, please check it!\n");
    return;
  }
  sprintf(CMDString, "chmod 755 %s", filename);
  system(CMDString);
  sprintf(outputName, "%s.output", filename);
  strcpy(CMDString, filename);
  strcat(CMDString, " > ");
  strcat(CMDString, outputName);
  system(CMDString);
  tmpFile = fopen(outputName, "r");
  while (feof(tmpFile) == 0) {
    fgets(str, 1024, tmpFile);
    strp = (char *) strstr(str, "Still processing data");
    if (strp != NULL) processingData = TRUE;
  }
  if (tmpFile != NULL) fclose(tmpFile);
  sprintf(CMDString, "/bin/rm -f %s", filename);
  system(CMDString);
  sprintf(CMDString, "/bin/rm -f %s", outputName);
  system(CMDString);
}


int OpenGOLACard(void) {
  int status = 0;
  u_int data;

  ts_open(1, TS_DUMMY);
  status = uio_init();
  if (status == 0) status = filar_map(1);
  if (status == 0) {
    status = IO_PCIConfigReadUInt(shandle, 0x8, &data);
    if (status != IO_RCC_SUCCESS) rcc_error_print(stdout, status);
    else {
      dumpconf();
      printf("\n\n\nRunning on a GOLA card of revision %02x\n", data & 0xff);
    }
  }
  else ts_close(TS_DUMMY);

  return status;
}


void CloseGOLACard(void) {
  filar_unmap();
  uio_exit();
  ts_close(TS_DUMMY);
}


int InitDAQ(void) {
  int status = 0;
  if ((GOLAPageSize <= 0) || (GOLAPageSize > 6)) GOLAPageSize = 6;
  if (filarconf(0, NULL, GOLAPageSize) != 0) status++;  
  status += daqEvInit(0,0);
  return status;
}


int InitToStartRun(int openDataFile) {
  unsigned int data, uintSize, status;

  status = InitDAQ();
  if( status ) printf( "Status non-zero from InitDAQ() call in InitToStartRun = %d\n", status );
  bufnr = 0;
  uintSize = sizeof(unsigned int);
  nbRequestedBuf = 30;
  status = daqEvInit( nbRequestedBuf, 0 );
  singleEvMaxBuffers = nbRequestedBuf;

  if (openDataFile) {
    CSMDataFile = NULL;
    if (strcmp(CSMDataFilename, "/dev/null") != 0) {
      CSMDataFile = fopen(CSMDataFilename, "wb");
      if (CSMDataFile != NULL) printf("Data file <%s> is opened.\n", CSMDataFilename);
      else printf("Unable to open data file <%s>.\n", CSMDataFilename);
    }
    rawDataFile = NULL;
    if (strlen(rawDataFilename) > 4) {
      rawDataFile = fopen(rawDataFilename, "wb");
      if (rawDataFile != NULL) printf("Raw Data file <%s> is opened.\n", rawDataFilename);
      else printf("Unable to open raw data file <%s>.\n", rawDataFilename);
    }
    logFile = NULL;
    logFile = fopen("socket.log","w");
    if (logFile == NULL) printf("Unable to open log file.\n");
  }
  memset(singleCounter, 0, uintSize*NUMBERANALYSISCOUNTER);
  singleCounter[MINEVENTSIZE] = 1000000;
  memset(analysisCounter, 0, uintSize*NUMBERANALYSISCOUNTER);
  analysisCounter[MINEVENTSIZE] = 1000000;
  memset(nEdge, 0, uintSize*MAXNUMBERMEZZANINE*24*2);
  memset(nPair, 0, uintSize*MAXNUMBERMEZZANINE*24);
  memset(nGoodHit, 0, uintSize*MAXNUMBERMEZZANINE*24);
  memset(nASDNoise, 0, uintSize*MAXNUMBERMEZZANINE*24);
  memset(timeSpectrum, 0, uintSize*MAXNUMBERMEZZANINE*24*4000);
  memset(timeWidth, 0, uintSize*MAXNUMBERMEZZANINE*24*512);

  SingleEventInit();
  singleEvInitWasDone = 1;

  if( status ) printf( "Status non-zero in return from call to InitToStartRun = %d\n", status );
  return status;
}




void PrintAnalysisCounter(FILE *file, int cntIndex, char name[], int maxLength) {
  int length, high, low;
  double sum;
  char str[50];
  
  length = strlen(name);
  if (length > 0) {
    low = cntIndex;
    high = -1;
    if (cntIndex == ANALYSEDEVENT) high = ANALYSEDEVENTHIGH;
    else if (cntIndex == NTOTALWORD) high = NTOTALWORDHIGH;
    else if (cntIndex == PROCESSEDEVENT) high = PROCESSEDEVENTHIGH;
    else if (cntIndex == NDATAWORD) high = NDATAWORDHIGH;
    else if (cntIndex == NEVENTWORD) high = NEVENTWORDHIGH;
    else if (cntIndex == NGOODCYCLE) high = NGOODCYCLEHIGH;
    else if (cntIndex == NEMPTYCYCLE) high = NEMPTYCYCLEHIGH;
    if ((high >= 0) && (analysisCounter[high] > 0)) {
      sum = maxUintValue * ((double) analysisCounter[high]) + (double) analysisCounter[low];
      sprintf(str, " %%%ds%%%dc: %%%d.4e\n", length, maxLength-length+1, 12);
      if (file == NULL) printf(str, name, ' ', sum);
      else fprintf(file, str, name, ' ', sum);
    }
    else {
      sprintf(str, " %%%ds%%%dc: %%%du\n", length, maxLength-length+1, 12);
      if (file == NULL) printf(str, name, ' ', analysisCounter[cntIndex]);
      else fprintf(file, str, name, ' ', analysisCounter[cntIndex]);
    }
  }
}


void SaveErrorSummaryFile(void) {
  int i, maxLength, nslot;
  char sumFilename[281], name[NUMBERANALYSISCOUNTER][80];
  FILE *sumFile;

  if ((strlen(CSMDataFilename) > 4) && (strstr(CSMDataFilename, "/dev/null") == NULL)) {
    strncpy(sumFilename, CSMDataFilename, strlen(CSMDataFilename)-4);
    sumFilename[strlen(CSMDataFilename)-4] = '\0';
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
  if ((CSMVersion%2) == 1) {
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
  if ((CSMVersion%2) == 1) {
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
  if ((CSMVersion%2) == 1) {
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
  if ((CSMVersion%2) == 1) {
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
  if (HPTDC) {
    strcpy(name[TDCERRORLIST], "Number of hit lost in group 0 from read-out fifo overflow");
    strcpy(name[TDCERRORLIST+1], "Number of hit lost in group 0 from L1 buffer overflow");
    strcpy(name[TDCERRORLIST+2], "Number of hit error have been detected in group 0");
    strcpy(name[TDCERRORLIST+3], "Number of hit lost in group 1 from read-out fifo overflow");
    strcpy(name[TDCERRORLIST+4], "Number of hit lost in group 1 from L1 buffer overflow");
    strcpy(name[TDCERRORLIST+5], "Number of hit error have been detected in group 1");
    strcpy(name[TDCERRORLIST+6], "Number of hit lost in group 2 from read-out fifo overflow");
    strcpy(name[TDCERRORLIST+7], "Number of hit lost in group 2 from L1 buffer overflow");
    strcpy(name[TDCERRORLIST+8], "Number of hit error have been detected in group 2");
    strcpy(name[TDCERRORLIST+9], "Number of hit lost in group 3 from read-out fifo overflow");
    strcpy(name[TDCERRORLIST+10], "Number of hit lost in group 3 from L1 buffer overflow");
    strcpy(name[TDCERRORLIST+11], "Number of hit error have been detected in group 3");
    strcpy(name[TDCERRORLIST+12], "Number of hits rejected because of programmed event size limit");
    strcpy(name[TDCERRORLIST+13], "Number of event lost (trigger fifo overflow)");
  }
  else {
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
  }
  if ((CSMVersion%2) == 1) {
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
  if (HPTDC) {
    strcpy(name[HPTDCFATALCHIPERROR], "Number of internal fatal chip errrs");
    if ((CSMVersion%2) == 1) {
      for (i = 0; i < MAXNUMBERMEZZANINE; i++)
        sprintf(name[HPTDCFATALCHIPERRORINTDC+i], "Number of internal fatal chip errrs in TDC %d", i);
    }
    else {
      strcpy(name[HPTDCFATALCHIPERRORINTDC+0], "Number of internal fatal chip errrs in TDC 0/16");
      strcpy(name[HPTDCFATALCHIPERRORINTDC+1], "Number of internal fatal chip errrs in TDC 1/17");
      for (i = 2; i < 16; i++)
        sprintf(name[HPTDCFATALCHIPERRORINTDC+i], "Number of internal fatal chip errrs in TDC %d", i);
    }
  }
  else {
    strcpy(name[TDCMASKWORD], "Number of TDC mask words");
    if ((CSMVersion%2) == 1) {
      for (i = 0; i < MAXNUMBERMEZZANINE; i++)
        sprintf(name[TDCMASKWORDINTDC+i], "Number of TDC mask words in TDC %d", i);
    }
    else {
      strcpy(name[TDCMASKWORDINTDC+0], "Number of TDC mask words in TDC 0/16");
      strcpy(name[TDCMASKWORDINTDC+1], "Number of TDC mask words in TDC 1/17");
      for (i = 2; i < 16; i++)
        sprintf(name[TDCMASKWORDINTDC+i], "Number of TDC mask words in TDC %d", i);
    }
  }
  strcpy(name[TDCMASKEMPTY], "Number of Empty TDC mask");
  strcpy(name[TDCDEBUGWORD], "Number of TDC debug words");
  if ((CSMVersion%2) == 1) {
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
  if ((CSMVersion%2) == 1) nslot = MAXNUMBERMEZZANINE;
  else nslot = 16;
  strcpy(name[TDCTRAILERWRONGTDCID], "Number of wrong TDCID events in TDCTrailer");
  for (i = 0; i < nslot; i++) {
    sprintf(name[TDCHEADERMISSTDCID+i], "Number of missed TDCID=%d in TDCHeader", i);
    sprintf(name[TDCHEADEREXTRATDCID+i], "Number of extra TDCID=%d in TDCHeader", i);
    sprintf(name[TDCTRAILERMISSTDCID+i], "Number of missed TDCID=%d in TDCTrailer", i);
    sprintf(name[TDCTRAILEREXTRATDCID+i], "Number of extra TDCID=%d in TDCTrailer", i);
  }
  strcpy(name[TDCHEADERAFTERITSDATA], "Number of TDC header after its data");
  if ((CSMVersion%2) == 1) {
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
  if ((CSMVersion%2) == 1) {
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
  if ((CSMVersion%2) == 1) {
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
  strcpy(name[DISCARDEDTRAILINGONLY], "Number of Discarded TDC Trailing/Mask by CSM");
  strcpy(name[DISCARDEDLEADINGONLY], "Number of Discarded TDC Leading/Error by CSM");
  strcpy(name[DISCARDEDTDCWORDS], "Number of Discarded TDC Words by CSM");
  strcpy(name[EXPECTEDTDCWCERROR], "Number of Expected TDC WC Error");
  strcpy(name[UNEXPECTEDTDCWCERROR], "Number of TDC WC < # Received Words");
  strcpy(name[WRONGCSMFIFOOVERFLOW], "Number of Wrong CSM FIFO Overflows");
  strcpy(name[CSMFIFOOVERFLOW1], "Number of CSM FIFO Overflow 1");
  strcpy(name[CSMFIFOOVERFLOW2], "Number of CSM FIFO Overflow 2");
  strcpy(name[CSMFIFOOVERFLOWS], "Number of CSM FIFO Overflow 1&2");
  strcpy(name[CSMEVTPRESCALEERR], "Number of CSM Event Prescale Error");
  if (CSMPairDebug) strcpy(name[CSMPAIRERROR], "Number of Hits Paired Wrong by CSM");
  for (i = 0; i < MAXNUMBERMEZZANINE; i++) {
    sprintf(name[DISCARDEDTRAILINGONLYINTDC+i], "Number of Discarded TDC Trailing/Mask by CSM in TDC %d", i);
    sprintf(name[DISCARDEDLEADINGONLYINTDC+i], "Number of Discarded TDC Leading/Mask by CSM in TDC %d", i);
    sprintf(name[DISCARDEDTDCWORDSINTDC+i], "Number of Discarded TDC Words by CSM in TDC %d", i);
    sprintf(name[EXPECTEDTDCWCERRORINTDC+i], "Number of Expected TDC WC Error in TDC %d", i);
    sprintf(name[UNEXPECTEDTDCWCERRORINTDC+i], "Number of TDC WC < # Received Words in TDC %d", i);
    sprintf(name[WRONGCSMFIFOOVERFLOWINTDC+i], "Number of Wrong CSM FIFO Overflows in TDC %d", i);
    sprintf(name[CSMFIFOOVERFLOW1INTDC+i], "Number of CSM FIFO Overflow 1 in TDC %d", i);
    sprintf(name[CSMFIFOOVERFLOW2INTDC+i], "Number of CSM FIFO Overflow 2 in TDC %d", i);
    sprintf(name[CSMFIFOOVERFLOWSINTDC+i], "Number of CSM FIFO Overflow 1&2 in TDC %d", i);
    sprintf(name[CSMEVTPRESCALEERRINTDC+i], "Number of CSM Event Prescale Error in TDC %d", i);
    if (CSMPairDebug) sprintf(name[CSMPAIRERRORINTDC+i], "Number of Hits Paired Wrong By CSM in TDC %d", i);
  }
  strcpy(name[NREQUESTEDSTOP], "Number of Requested Stop Run");
  strcpy(name[NREQUESTEDPAUSE], "Number of Requested Pause Run");
  strcpy(name[NTOTALWORD], "Number of Total Received Words");
  strcpy(name[PROCESSEDEVENT], "Number of Processed Events");
  strcpy(name[NDATAWORD], "Number of Data Words");
  strcpy(name[NEVENTWORD], "Number of Data Words for Constructed Events");
  strcpy(name[NGOODCYCLE], "Number of Good Cycles");
  strcpy(name[NEMPTYCYCLE], "Number of Empty Cycles");
  strcpy(name[NBADCYCLE], "Number of Bad Cycles");
  strcpy(name[NBADCYCLEATBEGINNING], "Number of Bad Cycles at Beginning");
  strcpy(name[CURREVENTSIZE], "Current Event Size");
  strcpy(name[MINEVENTSIZE], "Minimum Event Size");
  strcpy(name[MAXEVENTSIZE], "Maximum Event Size");
  strcpy(name[NBUFFERPOINTERERROR], "Number Buffer Pointer Error");
  if ((CSMVersion%2) == 1) {
    strcpy(name[TXPARITYERROR], "Number of Parity Error (CSM to GOLACard)");
    for (i = 0; i < MAXNUMBERMEZZANINE; i++)
      sprintf(name[TXPARITYERRORINTDC+i], "Number of Parity Error (CSM to GOLACard) in TDC %d", i);
    if (synchWordControl == STATUSTRIGTIMEINSYNCHWORD) {
      strcpy(name[LHCCLOCKUNLOCKED], "Reported LHC Clock Unlocked in SynchWord");
      strcpy(name[XMT1CLOCKUNLOCKED], "Reported XMT1 Clock Unlocked in SynchWord");
      strcpy(name[XMT2CLOCKUNLOCKED], "Reported XMT2 Clock Unlocked in SynchWord");
      strcpy(name[CSMPHASEERROR], "Reported Phase Error in SynchWord");
      strcpy(name[I2COPERATIONFAILED], "Reported I2C operation failure in SynchWord");
      strcpy(name[UNEXPECTEDTTCRXSETUP], "Reported Wrong TTCrx Setup in SynchWord");
      strcpy(name[CSMHASERROR], "Reported CSM Error in SynchWord");
      strcpy(name[NUMBERTRIGGER], "Reported Matched Triggers in SynchWord");
      strcpy(name[NUMBERSAMETRIGGER], "Reported Same Triggers in SynchWord");
      strcpy(name[NUMBERTRIGGERLOW], "Reported Trigger Low 16 Bits in SynchWord");
      strcpy(name[NUMBERUNMATCHEDTRIGGERLOW], "Reported Unmatched Trigger Low 16 Bits in SynchWord");
      strcpy(name[NUMBERTRIGGERHIGH], "Reported Trigger High 16 Bits in SynchWord");
      strcpy(name[NUMBERUNMATCHEDTRIGGERHIGH], "Reported Unmatched Trigger High 16 Bits in SynchWord");
      strcpy(name[UNKNOWNSWORD], "Number of Unknown SynchWord");
    }
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
        PrintAnalysisCounter(sumFile, i, name[i], maxLength);
    }
    else {
      for (i = 0; i < NUMBERANALYSISCOUNTER; i++)
        if (analysisCounter[i] > 0) PrintAnalysisCounter(sumFile, i, name[i], maxLength);
      fprintf(sumFile, " No data error has been found from data monitor.\n");
    }
    fclose(sumFile);
    printf("Error summary has been saved into error summary file <%s>.\n", sumFilename);
  }
  else printf("Unable to open error summary file <%s> for the run!\n", sumFilename);
}


void SaveTDCTimeSpectrum(void) {
  int mezz, ch, i, nlist;
  char spectrumFilename[281], dataProcessedAt[30];
  time_t currentTime;
  FILE *spectrumFile;

  currentTime = time(NULL);
  strftime(dataProcessedAt, 30, "%Y:%m:%d %H:%M:%S", localtime(&currentTime));
  dataProcessedAt[20] = '\0';

  // If too many events, TDC time spectrum may be wrong, so do not save it
  if (analysisCounter[ANALYSEDEVENTHIGH] > 0) return;
  if (analysisCounter[ANALYSEDEVENT] == 0) return;

  if ((strlen(CSMDataFilename) > 4) && (strstr(CSMDataFilename, "/dev/null") == NULL)) {
    strncpy(spectrumFilename, CSMDataFilename, strlen(CSMDataFilename)-4);
    spectrumFilename[strlen(CSMDataFilename)-4] = '\0';
  }
  else strcpy(spectrumFilename, "sample_data");
  strcat(spectrumFilename, ".spectrum");
  if (spectrumFile = fopen(spectrumFilename, "w")) {
    fprintf(spectrumFile, "// -------- MiniDAQ TDC Time Spectrum File -------------\n");
    fprintf(spectrumFile, "// The maximum entry for TDC time distribution is 4000\n");
    fprintf(spectrumFile, "// The maximum entry for TDC time width distribution is 256\n");
    fprintf(spectrumFile, "\n");
    fprintf(spectrumFile, "Chamber Type and Number      :   0x%08x\n", MDTChamberType);
    fprintf(spectrumFile, "Data File Name               : %s\n", CSMDataFilename);
    if (strlen(DAQStartedAt) > 0)
      fprintf(spectrumFile, "DAQ Started Time             : %s\n", DAQStartedAt);
    if (totalDAQTime > 0)
      fprintf(spectrumFile, "Data Collection Time (s)     : %12u\n", totalDAQTime);
    fprintf(spectrumFile, "Data Processed at            : %s\n", dataProcessedAt);
    fprintf(spectrumFile, "Total Number Processed Event : %12u\n", analysisCounter[ANALYSEDEVENT]);
    fprintf(spectrumFile, "Total Number Rejected Event  : %12u\n", 0);
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      for (ch = 0; ch < 24; ch++) {
        nlist = 0;
        for (i = 4000; i > 0; i--) {
          if (timeSpectrum[i-1][ch][mezz] > 0) {
             nlist = i;
             break;
          }
        }
        if (nlist > 0) {
          fprintf(spectrumFile, "Mezzanine %d Channel %d TDC time distribution\n", mezz, ch);
          fprintf(spectrumFile, "Number of entry : %d\n", nlist);
          for (i = 0; i < nlist; i++) {
            fprintf(spectrumFile, "%11d", timeSpectrum[i][ch][mezz]);
            if (((i%8) == 7) || (i == (nlist-1))) fprintf(spectrumFile, "\n");
          }         
        }

        nlist = 0;
        for (i = 512; i > 0; i--) {
          if (timeWidth[i-1][ch][mezz] > 0) {
             nlist = i;
             break;
          }
        }
        if (nlist > 0) {
          fprintf(spectrumFile, "Mezzanine %d Channel %d TDC width distribution\n", mezz, ch);
          fprintf(spectrumFile, "Number of entry : %d\n", nlist);
          for (i = 0; i < nlist; i++) {
            fprintf(spectrumFile, "%11d", timeWidth[i][ch][mezz]);
            if (((i%8) == 7) || (i == (nlist-1))) fprintf(spectrumFile, "\n");
          }         
        }
      }
    }
    fclose(spectrumFile);
    printf("TDC time spectrum has been saved into file <%s>.\n", spectrumFilename);
  }
  else printf("Unable to open TDC time spectrum file <%s> for the run!\n", spectrumFilename);
}


void SaveTriggerTimeDistribution(unsigned int *trigTHist) {
  unsigned int i, j, allZero, sum;
  char trigFilename[281];
  FILE *trigFile;

  if (synchWordControl != STATUSTRIGTIMEINSYNCHWORD) return;
  allZero = TRUE;
  sum = 0;
  for (i = 0; i < 1000000; i++) {
    if (trigTHist[i] != 0) allZero = FALSE;
    if (sum > (sum+trigTHist[i])) sum = 0xFFFFFFFF;
    else sum += trigTHist[i];
  }
  if (allZero) return; 
  if ((strlen(CSMDataFilename) > 4) && (strstr(CSMDataFilename, "/dev/null") == NULL)) {
    strncpy(trigFilename, CSMDataFilename, strlen(CSMDataFilename)-4);
    trigFilename[strlen(CSMDataFilename)-4] = '\0';
  }
  else strcpy(trigFilename, "sample_data");
  strcat(trigFilename, ".trg");
  if (trigFile = fopen(trigFilename, "w")) {
    fprintf(trigFile, " -------- MiniDAQ Trigger Time Distribution File -------------\n");
    fprintf(trigFile, "Total number of trigger : %12u\n", sum);
    for (i = 0; i < 1000000; i += 10) {
      fprintf(trigFile, " %7d", i);
      for (j = 0; j < 10; j++) fprintf(trigFile, " %12u", trigTHist[i+j]);
      fprintf(trigFile, "\n");
    }
    fclose(trigFile);
    printf("Trigger time distribution has been saved into file <%s>.\n", trigFilename);
  }
  else printf("Unable to open trigger time distribution file <%s> for the run!\n", trigFilename);
}


void CalculateAverageTDCTimeAndSigma(void) {
  int mezz, ch, i, nEntry, i0, i1, entry, imax1, imax2, imax, nEntry1, nEntry2, ntry;
  double sum, average, sum1, sum2, totalEntry, subEntry;

  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    for (ch = 0; ch < 24; ch++) {
      timeAverage[ch][mezz] = 0.0;
      timeSigma[ch][mezz] = 0.0;
      fracUsed[ch][mezz] = 0.0;
      // first find highest peak
      imax1 = -1;
      nEntry = 0;
      for (i = 0; i < 4000; i++) {
        entry = timeSpectrum[i][ch][mezz];
        if (entry > nEntry) {
          imax1 = i;
          nEntry = entry;
        }
      }
      i0 = imax1;
      while (i0 > 0) {
        entry = timeSpectrum[i0][ch][mezz];
        if (timeSpectrum[i0-1][ch][mezz] < entry) i0--;
        else {
          if (i0 > 3) {
            if ((timeSpectrum[i0-2][ch][mezz] >= entry) && (timeSpectrum[i0-3][ch][mezz] >= entry)) break;
            else i0--;
          }
          else i0 = 0;
        }
      }
      i1 = imax1;
      while (i1 < 3999) {
        entry = timeSpectrum[i1][ch][mezz];
        if (timeSpectrum[i1+1][ch][mezz] < entry) i1++;
        else {
          if (i1 < 3996) {
            if ((timeSpectrum[i1+2][ch][mezz] >= entry) && (timeSpectrum[i1+3][ch][mezz] >= entry)) break;
            else i1++;
          }
          else i1 = 3999;
        }
      }
      // now try to find second peak if it exists
      imax2 = -1;
      nEntry = 0;
      for (i = 0; i < 4000; i++) {
        if ((i < i0) || (i > i1)) {
          entry = timeSpectrum[i][ch][mezz];
          if (entry > nEntry) {
            imax2 = i;
            nEntry = entry;
          }
        }
      }
      nEntry = 0;
      sum = 0.0;
      for (i = 0; i < 4000; i++) {
        entry = timeSpectrum[i][ch][mezz];
        nEntry += entry;
        sum += ((double) entry) * ((double) i);
      }
      totalEntry = (double) nEntry;
      subEntry = (double) nEntry;
      if (nEntry > 0) {
        timeAverage[ch][mezz] = sum / ((double) nEntry);
        sum = 0.0;
        for (i = 0; i < 4000; i++) {
          average = ((double) i) - timeAverage[ch][mezz];
          sum += ((double) timeSpectrum[i][ch][mezz]) * average * average;
        }
        average = sum / ((double) nEntry);
        timeSigma[ch][mezz] = sqrt(average);
        fracUsed[ch][mezz] = subEntry / totalEntry;
        ntry = 0;
        while ((timeSigma[ch][mezz] > 1.0) && (fracUsed[ch][mezz] > 0.80) && (ntry < 10)) {
          ntry++;
          if ((ntry == 1) && (imax2 >= 0)) {
            if (imax1 > imax2) {
              i0 = (imax1 + imax2) / 2;
              i1 = (3*imax1 - imax2) / 2;
            }
            else {
              i0 = (3*imax1 - imax2) / 2;
              i1 = (imax1 + imax2) / 2;
            }
          }
          else if (timeSigma[ch][mezz] < 3.0) {
            i0 = (int) (imax1 - 3.0*timeSigma[ch][mezz]);
            i1 = (int) (imax1 + 3.0*timeSigma[ch][mezz]);
          }
          else if (timeSigma[ch][mezz] < 5.0) {
            i0 = (int) (imax1 - 2.5*timeSigma[ch][mezz]);
            i1 = (int) (imax1 + 2.5*timeSigma[ch][mezz]);
          }
          else if (timeSigma[ch][mezz] < 10.0) {
            i0 = (int) (imax1 - 2.0*timeSigma[ch][mezz]);
            i1 = (int) (imax1 + 2.0*timeSigma[ch][mezz]);
          }
          else if (timeSigma[ch][mezz] < 20.0) {
            i0 = (int) (imax1 - 1.5*timeSigma[ch][mezz]);
            i1 = (int) (imax1 + 1.5*timeSigma[ch][mezz]);
          }
          else {
            i0 = (int) (imax1 - 30.);
            i1 = (int) (imax1 + 30.);
          }
          if (i0 < 0) i0 = 0;
          if (i1 > 4000) i1 = 4000;
          nEntry1 = 0;
          sum1 = 0.0;
          for (i = i0; i < i1; i++) {
            entry = timeSpectrum[i][ch][mezz];
            nEntry1 += entry;
            sum1 += ((double) entry) * ((double) i);
          }

          nEntry2 = 0;
          sum2 = 0.0;
          if (imax2 >= 0) {
            if (ntry == 1) {
              if (imax2 > imax1) {
                i0 = (imax2 + imax1) / 2;
                i1 = (3*imax2 - imax1) / 2;
              }
              else {
                i0 = (3*imax2 - imax1) / 2;
                i1 = (imax2 + imax1) / 2;
              }
            }
            else if (timeSigma[ch][mezz] < 3.0) {
              i0 = (int) (imax2 - 3.0*timeSigma[ch][mezz]);
              i1 = (int) (imax2 + 3.0*timeSigma[ch][mezz]);
            }
            else if (timeSigma[ch][mezz] < 5.0) {
              i0 = (int) (imax2 - 2.5*timeSigma[ch][mezz]);
              i1 = (int) (imax2 + 2.5*timeSigma[ch][mezz]);
            }
            else if (timeSigma[ch][mezz] < 10.0) {
              i0 = (int) (imax2 - 2.0*timeSigma[ch][mezz]);
              i1 = (int) (imax2 + 2.0*timeSigma[ch][mezz]);
            }
            else if (timeSigma[ch][mezz] < 20.0) {
              i0 = (int) (imax2 - 1.5*timeSigma[ch][mezz]);
              i1 = (int) (imax2 + 1.5*timeSigma[ch][mezz]);
            }
            else {
              i0 = (int) (imax2 - 30.);
              i1 = (int) (imax2 + 30.);
            }
            if (i0 < 0) i0 = 0;
            if (i1 > 4000) i1 = 4000;
            nEntry2 = 0;
            sum2 = 0.0;
            for (i = i0; i < i1; i++) {
              entry = timeSpectrum[i][ch][mezz];
              nEntry2 += entry;
              sum2 += ((double) entry) * ((double) i);
            }
          }
          if (nEntry1 >= nEntry2) {
            imax = imax1;
            nEntry = nEntry1;
            sum = sum1;
          }
          else {
            imax = imax2;
            nEntry = nEntry2;
            sum = sum2;
          }
          subEntry = (double) nEntry;

          if ((ntry == 1) && (imax2 >= 0)) {
            if (imax == imax1) {
              if (imax1 > imax2) {
                i0 = (imax1 + imax2) / 2;
                i1 = (3*imax1 - imax2) / 2;
              }
              else {
                i0 = (3*imax1 - imax2) / 2;
                i1 = (imax1 + imax2) / 2;
              }
            }
            else {
              if (imax2 > imax1) {
                i0 = (imax2 + imax1) / 2;
                i1 = (3*imax2 - imax1) / 2;
              }
              else {
                i0 = (3*imax2 - imax1) / 2;
                i1 = (imax2 + imax1) / 2;
              }
	    }
          }
          else if (timeSigma[ch][mezz] < 3.0) {
            i0 = (int) (imax - 3.0*timeSigma[ch][mezz]);
            i1 = (int) (imax + 3.0*timeSigma[ch][mezz]);
          }
          else if (timeSigma[ch][mezz] < 5.0) {
            i0 = (int) (imax - 2.5*timeSigma[ch][mezz]);
            i1 = (int) (imax + 2.5*timeSigma[ch][mezz]);
          }
          else if (timeSigma[ch][mezz] < 10.0) {
            i0 = (int) (imax - 2.0*timeSigma[ch][mezz]);
            i1 = (int) (imax + 2.0*timeSigma[ch][mezz]);
          }
          else if (timeSigma[ch][mezz] < 20.0) {
            i0 = (int) (imax - 1.5*timeSigma[ch][mezz]);
            i1 = (int) (imax + 1.5*timeSigma[ch][mezz]);
          }
          else {
            i0 = (int) (imax - 30.);
            i1 = (int) (imax + 30.);
          }
          if (i0 < 0) i0 = 0;
          if (i1 > 4000) i1 = 4000;
          if (nEntry > 0) {
            timeAverage[ch][mezz] = sum / ((double) nEntry);
            sum = 0.0;
            for (i = i0; i < i1; i++) {
              average = ((double) i) - timeAverage[ch][mezz];
              sum += ((double) timeSpectrum[i][ch][mezz]) * average * average;
            }
            average = sum / ((double) nEntry);
            timeSigma[ch][mezz] = sqrt(average);
          }
          fracUsed[ch][mezz] = subEntry / totalEntry;
        }
      }
    }
  }
}


void CalculateAverageWidthAndSigma(void) {
  int mezz, ch, i, nEntry, i0, i1, entry;
  double sum, average, totalEntry, subEntry;

  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    for (ch = 0; ch < 24; ch++) {
      widthAverage[ch][mezz] = 0.0;
      widthSigma[ch][mezz] = 0.0;
      fracUsedInWidth[ch][mezz] = 0.0;
      nEntry = 0;
      sum = 0.0;
      for (i = 0; i < 511; i++) {
        entry = timeWidth[i][ch][mezz];
        nEntry += entry;
        sum += ((double) entry) * ((double) i);
      }
      subEntry = (double) nEntry;
      totalEntry = (double) nEntry + (double) timeWidth[511][ch][mezz];
      if (nEntry > 0) {
        widthAverage[ch][mezz] = sum / ((double) nEntry);
        fracUsedInWidth[ch][mezz] = subEntry / totalEntry;
        sum = 0.0;
        for (i = 0; i < 511; i++) {
          average = ((double) i) - widthAverage[ch][mezz];
          sum += ((double) timeWidth[i][ch][mezz]) * average * average;
        }
        average = sum / ((double) nEntry);
        widthSigma[ch][mezz] = sqrt(average);
      }
    }
  }
}




// Signal routines
//
void ChildSigUSR1Handler(int sig) {
  unsigned int checksum, j, sum;

  buffer[0] = NUMBERANALYSISCOUNTER+4;
  buffer[1] = DATATYPEDAQINFOR | CSMNumber;
  buffer[2] = 0;
  buffer[3] = 0;
  if (requestForStop) {
    printf("Request to stop DAQ has been issued.\n");
    buffer[3] = (0xC0000 | CMDSTOPRUN);
    singleCounter[NREQUESTEDSTOP]++;
  }
  else if ((!requestForPause) && (numberFilledFIFOs >= 8)) {
    requestForPause = TRUE;
    buffer[3] = (0xC0000 | CMDPAUSERUN);
    singleCounter[NREQUESTEDPAUSE]++;
  }
  else if (requestForPause && (numberFilledFIFOs < 2)) {
    requestForPause = FALSE;
    buffer[3] = (0xC0000 | CMDRESUMERUN);
  }
  if (DAQState == State_Paused) printf("\nRun is paused!\n");
  checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3];
  for (j = 0; j <  NUMBERANALYSISCOUNTER; j++) {
    buffer[4+j] = singleCounter[j];
    checksum += singleCounter[j];
    if (j == ANALYSEDEVENT) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[ANALYSEDEVENTHIGH]++;
      analysisCounter[ANALYSEDEVENT] = sum;
    }
    else if (j == NTOTALWORD) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[NTOTALWORDHIGH]++;
      analysisCounter[NTOTALWORD] = sum;
    }
    else if (j == PROCESSEDEVENT) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[PROCESSEDEVENTHIGH]++;
      analysisCounter[PROCESSEDEVENT] = sum;
    }
    else if (j == NDATAWORD) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[NDATAWORDHIGH]++;
      analysisCounter[NDATAWORD] = sum;
    }
    else if (j == NEVENTWORD) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[NEVENTWORDHIGH]++;
      analysisCounter[NEVENTWORD] = sum;
    }
    else if (j == NGOODCYCLE) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[NGOODCYCLEHIGH]++;
      analysisCounter[NGOODCYCLE] = sum;
    }
    else if (j == NEMPTYCYCLE) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[NEMPTYCYCLEHIGH]++;
      analysisCounter[NEMPTYCYCLE] = sum;
    }
    else if ((j == CURREVENTSIZE) || (j == CURREVENTID)) {
      analysisCounter[j] = singleCounter[j];
    }
    else if (j == MINEVENTSIZE) {
      if ((singleCounter[ANALYSEDEVENT] > 0) && (analysisCounter[j] > singleCounter[j]))
        analysisCounter[j] = singleCounter[j];
    }
    else if (j == MAXEVENTSIZE) {
      if (analysisCounter[j] < singleCounter[j]) analysisCounter[j] = singleCounter[j];
    }
    else if ((j == LHCCLOCKUNLOCKED) || (j == XMT1CLOCKUNLOCKED) || (j == XMT2CLOCKUNLOCKED) ||
             (j == CSMPHASEERROR) || (j == I2COPERATIONFAILED) || (j == UNEXPECTEDTTCRXSETUP) ||
             (j == CSMHASERROR) || (j == UNKNOWNSWORD)) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[j] = 0xFFFFFFFF;
      else analysisCounter[j] += singleCounter[j];
    }
    else {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[j] = 0xFFFFFFFF;
      else analysisCounter[j] += singleCounter[j];
    }
  }
  for (j = 0; j <  NUMBERANALYSISCOUNTER; j++) singleCounter[j] = 0;
  singleCounter[MINEVENTSIZE] = 1000000;
  buffer[buffer[0]] = checksum;
  write(pipe2[1], buffer, 4*(buffer[0]+1));
  if (DAQState == State_Idle) buffer[0] = 0;
  DAQDebug(("ChildSigUSR1Handler: signal USR1 is received\n"));
}


void ChildSigUSR2Handler(int sig) {
  DAQState = State_Idle;
  DAQDebug(("ChildSigUSR2Handler: signal USR2 is received\n"));
}


void ChildSigTERMHandler(int sig) {
  DAQState = State_Idle;
  DAQDebug(("ChildSigTERMHandler: signal TERM is received\n"));
  if (CSMDataFile != NULL) {
    fclose(CSMDataFile);
    printf("\nData file <%s> is closed.\n", CSMDataFilename);
    CSMDataFile = NULL;
  }
  if (rawDataFile != NULL) {
    fclose(rawDataFile);
    printf("\nRaw Data file <%s> is closed.\n", rawDataFilename);
    rawDataFile = NULL;
  }
  if (logFile != NULL) {
    fclose(logFile);
    logFile = NULL;
    printf("\nLog file is closed.\n");
  }
  SaveErrorSummaryFile();
  SaveTDCTimeSpectrum();
  exit (0);
}


void ParentSigUSR1Handler(int sig) {
  DAQDebug(("ParnetSigUSR1Handler: signal USR1 is received\n"));
}


void ParentSigUSR2Handler(int sig) {
  DAQDebug(("ParentSigUSR2Handler: signal USR2 is received\n"));
}


void ParentSigTERMHandler(int sig) {
  normalQuit = FALSE;
  DAQState = State_Idle;
  DAQDebug(("ParentSigTERMHandler: signal TERM is received\n"));
  close(sock);
  close(osock);
  wait(NULL);
}


int CollectOneEvent(unsigned int *rawData, int rawDataSize, unsigned int *builtData, int builtDataSize) {
  oneEventInfo_T evInfo;
  int icol, eventReady = 0;
  int flowType;

  InitDAQ();

  evInfo.dataBuffer = rawData;        // Acquired data buffer
  evInfo.bufLen = rawDataSize;
  evInfo.builtEvent = builtData;      // Built event buffer
  evInfo.eventLen = builtDataSize;
  for( icol=0; icol<18; ++icol ) {
    evInfo.maskArray[icol] = mezzCardEnable[icol];
  }

  evInfo.checkSignals = 6;    // Break on wait of >2 seconds and on DAQState value
  evInfo.watchVariable = &DAQState;
  evInfo.watchValue = State_Idle;

  flowType = CSMVersion%2;
  DAQState = State_Running;
  eventReady = captureEvent( flowType, &evInfo );
  DAQState = State_Idle;

  printf( "Found event built with evid=%d of size %d\n\n", evInfo.evid, evInfo.bufSize );
  printf( "Raw data buffer:" );
  for( icol=0; icol<evInfo.bufSize; ++icol ) {
    if( icol%8 == 0 ) printf( "\n" );
    printf( "%8.8x ", rawData[icol] );
  }
  if( (evInfo.bufSize)%8 != 0 ) printf( "\n" );

  printf( "\nThe event built from this data contains %d words, which are", 
	  evInfo.eventSize );
  for( icol=0; icol<evInfo.eventSize; ++icol ) {
    if( icol%8 == 0 ) printf( "\n" );
    printf( "%8.8x ", builtData[icol] );
  }
  if( (evInfo.eventSize)%8 != 0 ) printf( "\n" );
  singleEvInfo.eventSize = evInfo.eventSize;
 
  return eventReady;
}


void SingleEventInit() {
// Mostly just a local interface to the csm_utils routine
// Call this once whenever a new set of buffers is to be searched for events
//   but DON'T call it between sequentially acquired buffers that will be 
//   searched.  That would not be fatal, it would just miss events spanning
//   buffers.

  buildInit();
  singleEvOffset = 0;
  singleEvBufnr = 0;
}


int GetNextSingleEvent( unsigned int *rawData, int rawDataSize, unsigned int *builtData, 
			int builtDataSize, unsigned int *dataptr, unsigned int fsize ) {
// Called to get the next built event out of the databuffer "dataptr" of size "fsize"
// The raw data input to the built event is placed in "rawData" of max size "rawDataSize"
// The built event is placed in "builtData" of size "builtDataSize".
// Remember, if the databuffer contains TDM data, then "rawData" will contain complete 
// 19-word cycles, which could have raw data from more than one building event.

// Return value is 1 for success, zero for no complete event found.  In this latter case,
// just call this routine again with the next acquired databuffer and building will take
// up where it left off.

  int icol;
  int flowType;
  int returnStat;

  singleEvInfo.dataBuffer = rawData;        // Acquired data buffer
  singleEvInfo.bufLen = rawDataSize;
  singleEvInfo.builtEvent = builtData;      // Built event buffer
  singleEvInfo.eventLen = builtDataSize;
  for( icol=0; icol<18; ++icol ) {
    singleEvInfo.maskArray[icol] = mezzCardEnable[icol];
  }

// No signal checking.  In fact, in this access method, these are not even used.
  singleEvInfo.checkSignals = 0;

  buildEvInit( &singleEvInfo );

  flowType = CSMVersion%2;
  returnStat = getNextEvent( &singleEvInfo, flowType, &singleEvOffset, dataptr, fsize );

// ------------------------------- NOTE: ----------------------------------
// -------------- Comment out this big "if" block later on.... ------------
  if( returnStat == 1 ) {   //success
    printf( "Found event built with evid=%d of size %d\n\n", singleEvInfo.evid, singleEvInfo.bufSize );
    printf( "Raw data buffer:" );
    for( icol=0; icol<singleEvInfo.bufSize; ++icol ) {
      if( icol%8 == 0 ) printf( "\n" );
      printf( "%8.8x ", *(rawData+icol) );
    }
    if( (singleEvInfo.bufSize)%8 != 0 ) printf( "\n" );

    printf( "\nThe event built from this data contains %d words, which are", 
      singleEvInfo.eventSize );
    for( icol=0; icol<singleEvInfo.eventSize; ++icol ) {
      if( icol%8 == 0 ) printf( "\n" );
      printf( "%8.8x ", *(builtData+icol) );
    }
    if( (singleEvInfo.eventSize)%8 != 0 ) printf( "\n" );
  }
  else {
    printf("No event successfully built this time\n" );
  }
// ------------------------------- END NOTE: ------------------------------

  return returnStat;
}


int CollectSeqEvent( unsigned int *rawData, int rawDataSize, unsigned int *builtData, 
			int builtDataSize ) {
// Called to get the next built event out of the databuffer "dataptr" of size "fsize"
// The raw data input to the built event is placed in "rawData" of max size "rawDataSize"
// The built event is placed in "builtData" of size "builtDataSize".
// Remember, if the databuffer contains TDM data, then "rawData" will contain complete 
// 19-word cycles, which could have raw data from more than one building event.

// Return value is 1 for success, zero for no complete event found.  In this latter case,
// just call this routine again with the next acquired databuffer and building will take
// up where it left off.

  static unsigned int *dataptr;
  static unsigned int fsize;

  int icol;
  int flowType;
  int returnStat;
  int loopCount;
  unsigned int fsizedata, fifodata;

  if( singleEvInitWasDone == 0 ) {
    SingleEventInit();
    singleEvInitWasDone = 1;
  }

  singleEvInfo.dataBuffer = rawData;        // Acquired data buffer
  singleEvInfo.bufLen = rawDataSize;
  singleEvInfo.builtEvent = builtData;      // Built event buffer
  singleEvInfo.eventLen = builtDataSize;
  for( icol=0; icol<18; ++icol ) {
    singleEvInfo.maskArray[icol] = mezzCardEnable[icol];
  }

// No signal checking.  That will be done right here via max loopCount value.
  singleEvInfo.checkSignals = 0;

  buildEvInit( &singleEvInfo );

  flowType = CSMVersion%2;
  cont = 1;
  returnStat = 0;

  while( cont ) {
    if( singleEvOffset == 0 ) {
      loopCount = 0;
      while( 1 ) {
        fifodata = filar->fifostat;
	      if (fifodata & 0x0000000f) {
          break;
        }
	      if( ++loopCount == 10000000 ) {  // No data buffers left to get, break out
          cont = 0;
          break;
        }
	    }
      if( cont == 0 )
        break;
      /* 
       * Read the ACK FIFO to get the size of the data packet in the data
       * buffer.  It had better always be the same for all that we grab!
       * fsize in 32bit words. 1M word max
       */
      fsizedata = filar->ack1;
      fsize = fsizedata & 0xfffff;

      /*
       * Get a pointer to the data and write it out to disk
       * after first doing a few checks (very few)
       */
      dataptr = (unsigned int *) uaddr[1][singleEvBufnr];
    }

    if( CSMVersion == 0 ) {
      flowType = 1;
      printf("\n ---> CSMVersion is zero!!!! Force to do TDM decoding. <---\n" );
    }
    returnStat = getNextEvent( &singleEvInfo, flowType, &singleEvOffset, dataptr, fsize );

// ------------------------------- NOTE: ----------------------------------
// -------------- Comment out this big "if" block later on.... ------------
    if( returnStat == 1 ) {  //success
      printf( "Found event built with evid=%d of size %d\n\n", singleEvInfo.evid, singleEvInfo.bufSize );
      printf( "Raw data buffer:" );
      for( icol=0; icol<singleEvInfo.bufSize; ++icol ) {
	      if( icol%8 == 0 ) printf( "\n" );
	      printf( "%8.8x ", *(rawData+icol) );
	    }
      if( (singleEvInfo.bufSize)%8 != 0 ) printf( "\n" );
      printf( "\nThe event built from this data contains %d words, which are", singleEvInfo.eventSize );
      for( icol=0; icol<singleEvInfo.eventSize; ++icol ) {
        if( icol%8 == 0 ) printf( "\n" );
        printf( "%8.8x ", *(builtData+icol) );
	    }
      if( (singleEvInfo.eventSize)%8 != 0 ) printf( "\n" );
    }
    else {
      printf("\n -----> No event successfully built this time, get next buffer\n" );
    }
// ------------------------------- END NOTE: ------------------------------

    if( returnStat == 1 ) {
      break;
    }

// Fill the next address into the req fifo for the next loop
    filar->req1 = paddr[1][singleEvBufnr];

// Switch to the next buffer
    if( ++singleEvBufnr == singleEvMaxBuffers ) {
      singleEvBufnr = 0;
    }
  }     // End of acquisition loop

  return returnStat;
}
