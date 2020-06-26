//
// Filename: interface.c
// Author: T.S. Dai
//
// Description:
// This file contains all of the user interface and utility functions. To use
// it, add #include "ATLASinterface.h" in your code.
//

#include <utility.h>
#include <formatio.h>
#include "interface.h"
extern void UserFunction(void);

// This function initializes all user buttons.
void InitUserInterface(void) {
  int i;
  
  numberButtons = 0;
  for (i = 0; i < MAXBUTTON; i++) {
    userButtons[i].panel_id = 0;
    userButtons[i].id = 0;
    userButtons[i].func = NULL;
  }
}


//
// Function: SetupUserButton
//
// This function allows the user to create a button acessible from the
// LoopAndExecuteUserButton(void (*func)(void)).
// Required parameters:
//   panel_id : Identification number for the PANEL in which the control resides
//   id       : identification number for the control to be assigned to the button,
//              i.e., PANEL_QUIT
//   *func    : a function pointer indicating the function which is executed when
//              the button is clicked on
//
// The return value is the id of the button created, an integer from 0 to MAXBUTTON.
// If there are no slots available for additional buttons, the function returns -1.
//
int SetupUserButton(int panel_id, int id, void (*func)(void)) {
  int i;
  
  for (i = 0; i < MAXBUTTON; i++) if (userButtons[i].id == 0) break;
  if (i == MAXBUTTON) {
    printf(" User button on panel %2d with id %3d can not be installed, exceed button limit %d\n",
           panel_id, id, MAXBUTTON);
    return -1;
  }
  userButtons[i].panel_id = panel_id;
  userButtons[i].id = id;
  userButtons[i].para1 = 0;
  userButtons[i].para2 = 0;
  userButtons[i].func = func;
  userButtons[i].func1 = NULL;
  userButtons[i].func2 = NULL;
  numberButtons++;
  return i;
}


int SetupUserButton1(int panel_id, int id, int (*func)(int), int para1) {
  int i;
  
  for (i = 0; i < MAXBUTTON; i++) if (userButtons[i].id == 0) break;
  if (i == MAXBUTTON) {
    printf(" User button on panel %2d with id %3d can not be installed, exceed button limit %d\n",
           panel_id, id, MAXBUTTON);
    return -1;
  }
  userButtons[i].panel_id = panel_id;
  userButtons[i].id = id;
  userButtons[i].para1 = para1;
  userButtons[i].para2 = 0;
  userButtons[i].func = NULL;
  userButtons[i].func1 = func;
  userButtons[i].func2 = NULL;
  numberButtons++;
  return i;
}


int SetupUserButton2(int panel_id, int id, int (*func)(int, int), int para1, int para2) {
  int i;
  
  for (i = 0; i < MAXBUTTON; i++) if (userButtons[i].id == 0) break;
  if (i == MAXBUTTON) {
    printf(" User button on panel %2d with id %3d can not be installed, exceed button limit %d\n",
           panel_id, id, MAXBUTTON);
    return -1;
  }
  userButtons[i].panel_id = panel_id;
  userButtons[i].id = id;
  userButtons[i].para1 = para1;
  userButtons[i].para2 = para2;
  userButtons[i].func = NULL;
  userButtons[i].func1 = NULL;
  userButtons[i].func2 = func;
  numberButtons++;
  return i;
}


void LoopAndExecuteUserButtonAndFunction(void (*func)(void)) {
  int i, ctrl, handle, exitLoop = FALSE;
  
  quitProgram = FALSE;
  if ((numberButtons <= 0) || (func == NULL)) exitLoop = TRUE;
  while (!exitLoop) {
    UserFunction();
    if (GetUserEvent(0, &handle, &ctrl) == 1) {
      for (i = 0; i < numberButtons; i++) {
        if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
          if (userButtons[i].func) {
            if (userButtons[i].func == func) exitLoop = TRUE; 
            (*userButtons[i].func)();
          }
          else if (userButtons[i].func1) {
            (*userButtons[i].func1)(userButtons[i].para1);
          }
          else if (userButtons[i].func2) {
            (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
          }
        }
      }
    }
    if (quitProgram) break;
  }
}


void LoopAndCheckUserButtonAndFunction(void (*func)(void), void (*exeF)(void)) {
  int i, ctrl, handle, exitLoop = FALSE;
  
  if ((numberButtons <= 0) || (func == NULL)) exitLoop = TRUE;
  while (!exitLoop) {
    UserFunction();
    if (GetUserEvent(0, &handle, &ctrl) == 1) {
      for (i = 0; i < numberButtons; i++) {
        if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
          if (userButtons[i].func) {
            if (userButtons[i].func != func) {
              exitLoop = TRUE;
              (*exeF)();
              (*userButtons[i].func)();
            }
          }
          else if (userButtons[i].func1) {
            (*exeF)();
            (*userButtons[i].func1)(userButtons[i].para1);
          }
          else if (userButtons[i].func2) {
            (*exeF)();
            (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
          }
        }
      }
    }
  }
}


int CheckUserButtons(void (*Func)(void)) {
  int i, ctrl, handle, gotFunc = 0;
  
  UserFunction();
  if (GetUserEvent(0, &handle, &ctrl) == 1) {
    for (i = 0; i < numberButtons; i++) {
      if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
        if (userButtons[i].func) {
          if (userButtons[i].func == Func) gotFunc = 1;
          else {
            (*userButtons[i].func)();
            if (gotFunc == 0) gotFunc = 2;
          }
        }
        else if (userButtons[i].func1) {
          (*userButtons[i].func1)(userButtons[i].para1);
          if (gotFunc == 0) gotFunc = 2;
        }
        else if (userButtons[i].func2) {
          (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
          if (gotFunc == 0) gotFunc = 2;
        }
      }
    }
  }
  return gotFunc;
}


int HandleUserButtons(void (*Func)(void)) {
  int i, ctrl, handle, gotFunc = 0;
  
  if (GetUserEvent(0, &handle, &ctrl) == 1) {
    for (i = 0; i < numberButtons; i++) {
      if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
        if (userButtons[i].func) {
          if (userButtons[i].func == Func) gotFunc = 1;
          (*userButtons[i].func)();
        }
        else if (userButtons[i].func1) {
          (*userButtons[i].func1)(userButtons[i].para1);
        }
        else if (userButtons[i].func2) {
          (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
        }
      }
    }
  }
  return gotFunc;
}


int HandleUserButtonAndFunction(void (*Func)(void)) {
  int i, ctrl, handle, gotFunc = 0;
  
  UserFunction();
  if (GetUserEvent(0, &handle, &ctrl) == 1) {
    for (i = 0; i < numberButtons; i++) {
      if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
        if (userButtons[i].func) {
          if (userButtons[i].func == Func) gotFunc = 1;
          (*userButtons[i].func)();
        }
        else if (userButtons[i].func1) {
          (*userButtons[i].func1)(userButtons[i].para1);
        }
        else if (userButtons[i].func2) {
          (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
        }
      }
    }
  }
  return gotFunc;
}


int HandleUserButtons2(void (*Func1)(void), void (*Func2)(void)) {
  int i, ctrl, handle, gotFunc = 0;
  
  if (GetUserEvent(0, &handle, &ctrl) == 1) {
    for (i = 0; i < numberButtons; i++) {
      if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
        if (userButtons[i].func) {
          if (userButtons[i].func == Func1) gotFunc = 1;
          if (userButtons[i].func == Func2) gotFunc = 2;
          (*userButtons[i].func)();
        }
        else if (userButtons[i].func1) {
          (*userButtons[i].func1)(userButtons[i].para1);
        }
        else if (userButtons[i].func2) {
          (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
        }
      }
    }
  }
  return gotFunc;
}


int HandleUserButtonAndFunction2(void (*Func1)(void), void (*Func2)(void)) {
  int i, ctrl, handle, gotFunc = 0;
  
  UserFunction();
  if (GetUserEvent(0, &handle, &ctrl) == 1) {
    for (i = 0; i < numberButtons; i++) {
      if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
        if (userButtons[i].func) {
          if (userButtons[i].func == Func1) gotFunc = 1;
          if (userButtons[i].func == Func2) gotFunc = 2;
          (*userButtons[i].func)();
        }
        else if (userButtons[i].func1) {
          (*userButtons[i].func1)(userButtons[i].para1);
        }
        else if (userButtons[i].func2) {
          (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
        }
      }
    }
  }
  return gotFunc;
}


int HandleUserButtons3(void (*Func1)(void), void (*Func2)(void), void (*Func3)(void)) {
  int i, ctrl, handle, gotFunc = 0;
  
  if (GetUserEvent(0, &handle, &ctrl) == 1) {
    for (i = 0; i < numberButtons; i++) {
      if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
        if (userButtons[i].func) {
          if (userButtons[i].func == Func1) gotFunc = 1;
          if (userButtons[i].func == Func2) gotFunc = 2;
          if (userButtons[i].func == Func3) gotFunc = 3;
          (*userButtons[i].func)();
        }
        else if (userButtons[i].func1) {
          (*userButtons[i].func1)(userButtons[i].para1);
        }
        else if (userButtons[i].func2) {
          (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
        }
      }
    }
  }
  return gotFunc;
}


int HandleUserButtonAndFunction3(void (*Func1)(void), void (*Func2)(void), void (*Func3)(void)) {
  int i, ctrl, handle, gotFunc = 0;
  
  UserFunction();
  if (GetUserEvent(0, &handle, &ctrl) == 1) {
    for (i = 0; i < numberButtons; i++) {
      if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
        if (userButtons[i].func) {
          if (userButtons[i].func == Func1) gotFunc = 1;
          if (userButtons[i].func == Func2) gotFunc = 2;
          if (userButtons[i].func == Func3) gotFunc = 3;
          (*userButtons[i].func)();
        }
        else if (userButtons[i].func1) {
          (*userButtons[i].func1)(userButtons[i].para1);
        }
        else if (userButtons[i].func2) {
          (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
        }
      }
    }
  }
  return gotFunc;
}


int HandleUserButtons4(void (*Func1)(void), void (*Func2)(void), void (*Func3)(void), void (*Func4)(void)) {
  int i, ctrl, handle, gotFunc = 0;
  
  if (GetUserEvent(0, &handle, &ctrl) == 1) {
    for (i = 0; i < numberButtons; i++) {
      if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
        if (userButtons[i].func) {
          if (userButtons[i].func == Func1) gotFunc = 1;
          if (userButtons[i].func == Func2) gotFunc = 2;
          if (userButtons[i].func == Func3) gotFunc = 3;
          if (userButtons[i].func == Func4) gotFunc = 4;
          (*userButtons[i].func)();
        }
        else if (userButtons[i].func1) {
          (*userButtons[i].func1)(userButtons[i].para1);
        }
        else if (userButtons[i].func2) {
          (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
        }
      }
    }
  }
  return gotFunc;
}


int HandleUserButtonAndFunction4(void (*Func1)(void), void (*Func2)(void), void (*Func3)(void), void (*Func4)(void)) {
  int i, ctrl, handle, gotFunc = 0;
  
  UserFunction();
  if (GetUserEvent(0, &handle, &ctrl) == 1) {
    for (i = 0; i < numberButtons; i++) {
      if ((ctrl == userButtons[i].id) && (handle == userButtons[i].panel_id)) {
        if (userButtons[i].func) {
          if (userButtons[i].func == Func1) gotFunc = 1;
          if (userButtons[i].func == Func2) gotFunc = 2;
          if (userButtons[i].func == Func3) gotFunc = 3;
          if (userButtons[i].func == Func4) gotFunc = 4;
          (*userButtons[i].func)();
        }
        else if (userButtons[i].func1) {
          (*userButtons[i].func1)(userButtons[i].para1);
        }
        else if (userButtons[i].func2) {
          (*userButtons[i].func2)(userButtons[i].para1, userButtons[i].para2);
        }
      }
    }
  }
  return gotFunc;
}


//
// Utility: Data processing functions
//
// Converts part of a binary string with starting position "offset" and ending 
// position "width" to an integer (32 bits) which is then returned to the
// calling function.
void BinaryToInt(int *integer, int offset, int width, int *array) {
  unsigned int mask = 1;
  int nbit, temp;
  
  *integer = 0;
  for (nbit = 0; nbit < width; nbit++) {
    temp = (int) (array[nbit+offset] & mask);
    *integer = *integer | (temp <<nbit);
    if (nbit >= 31) break;
  }
}


// Converts part of a binary string with starting position "offset+width" 
// and ending position "offset" and to an integer.
void ReversedBinaryToInt(int *integer, int offset, int width, int *array) {
  unsigned int mask = 1;
  int nbit = 0, temp;
  
  *integer = 0;
  for (nbit = width-1; nbit >= 0; nbit--) {
    temp = (int) (array[nbit+offset] & mask);
    *integer = *integer | (temp << (width-1-nbit));
    if ((width-1-nbit) >= 31) break;
  }
}


// Converts an integer to binary, storing the bits in an array of length "arrayLength."
// The array must be defined as global for this to work.
void IntToBinary(int integer, int offset, int width, int *array, int arrayLength) {
  unsigned int mask = 1;
  int nbit, temp, i = 0;
  
  for (nbit = 0; nbit < width; nbit++) {
    temp = (int) ((integer>>nbit) & mask);
    if (i+offset < arrayLength) {
      array[i+offset] = temp;
      i++;
    }
  }
}


// Converts an integer to binary, storing the bits in an array with bit0 at starting position
// "offset+width" and ending position "offset". The array must be defined as global for this to work.
void IntToReversedBinary(int integer, int offset, int width, int *array, int arrayLength) {
  unsigned int mask = 1;
  int nbit, temp, i = 0;
  
  for (nbit = width-1; nbit >= 0; nbit--) {
    temp = (int) ((integer>>nbit) & mask);
    if (i+offset < arrayLength) {
      array[i+offset] = temp;
      i++;
    }
  }
}


// Takes the unsigned int 32-bit wide "wordOut" array passed by the calling function and
// converts it to binary, filling the binary "array" sent to it. 
void WordToArray(unsigned int *wordOut, int wordCount, int *array, int arrayLength) {
  unsigned int mask = 1;
  int nw = 0, nbit = 0, temp, bitcount, i = 0, j;
 
  for (j = 0; j < arrayLength; j++) array[j] = 0;
  for (nw = 0; nw < wordCount; nw++)
    for (nbit = 0; nbit < 32; nbit++) {
      temp = (int) ((wordOut[nw]>>nbit) & mask);
      if (i < arrayLength) array[i++] = temp;
    }
}


// Takes the int array passed by the calling function and converts it to 32-bit wide 
// unsigned integers by filling the wordIn array sent to it. 
void ArrayToWord(unsigned int *wordIn, int *wordCount, int *array, int arrayLength) {
  unsigned int mask = 1;
  int nw = 0, nbit = 0, temp, i = 0, j;
  
  nw = arrayLength / 32;
  if ((arrayLength-32*nw) > 0) nw = nw + 1; 
  *wordCount = nw;
  for (j = 0; j < *wordCount; j++) wordIn[j] = 0;
  for (nw = 0; nw < *wordCount; nw++)
    for (nbit = 0; nbit < 32; nbit++)
      if ( i < arrayLength) {
        temp = (int) (array[i++] & mask);
        wordIn[nw] = wordIn[nw] |(temp << nbit);
      }
}


void ReverseArray(int *array, int start, int bits) {
  int k, move;
  
  for(k = 0; k < (bits/2); k++) {
    move = array[start+k];
    array[start+k] = array[start+bits-1-k];
    array[start+bits-1-k] = move;
  }
}


// Utility function for setting a delay
void WaitInMillisecond(int waitTime) {
  int startTime = (int) clock();

  while ((((int) clock()) - startTime) < waitTime);
}


void GetIPConfiguration(void) {
  int size, status, ntried, gotIP, gotHost, gotDomain, gotAll, i, j, strl, localArea;
  char str[256], cstr[80];
  FILE *logFile;
  
  strcpy(IPAddress, "Unknown");
  strcpy(netIPAddress, "Unknown");
  strcpy(hostName, "Unknown");
  strcpy(domainName, "Unknown");
  gotIP = FALSE;
  gotHost = FALSE;
  gotDomain = FALSE;
  localArea = TRUE;
  if (GetFileInfo("myIPConfig.cmd", &size)) {
    if (GetFileInfo("myIPConfig.log", &size)) DeleteFile("myIPConfig.log");
    status = LaunchExecutable("myIPConfig.cmd");
    if (status != 0) status = LaunchExecutable("myIPConfig.cmd");
    ntried = 0;
    while (!GetFileInfo("myIPConfig.log", &size)) { 
      if ((ntried%3) == 2) printf("Still waiting to get  ...\n"); 
      WaitInMillisecond(500);
      ntried++;
    }   
    WaitInMillisecond(1000);
    if (status == 0) {
      ntried = 0;
      gotAll = gotIP & gotHost & gotDomain;
      while ((!gotAll) && (ntried < 10000)) {
        if (logFile = fopen("myIPConfig.log", "r")) {
          while (feof(logFile) == 0) {
            fgets(str, 256, logFile);
            j = 0;
            while((str[j] == ' ') || str[j] == '\t' || str[j] == ':') j++; 
            for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
            strcpy(cstr, "Host Name");
            if (strstr(str, cstr) != NULL) {
              strl = strlen(cstr);
              for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
              j = 0;
              while((str[j] == ' ') || str[j] == '\t' || str[j] == '.' || str[j] == ':') j++; 
              for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
              sscanf(str, "%s", hostName);
              gotHost = TRUE;
            }
            strcpy(cstr, "Connection-specific DNS Suffix");
            if (strstr(str, cstr) != NULL) {
              strl = strlen(cstr);
              for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
              j = 0;
              while((str[j] == ' ') || str[j] == '\t' || str[j] == '.' || str[j] == ':') j++; 
              for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
              sscanf(str, "%s", domainName);
              gotDomain = TRUE;
            }
            strcpy(cstr, "Ethernet adapter Local Area Connection");
            if (strstr(str, cstr) != NULL) localArea = TRUE;
            else {
              strcpy(cstr, "Ethernet adapter");
              if (strstr(str, cstr) != NULL) {
                strcpy(cstr, "Network Connection");
                if (strstr(str, cstr) != NULL) localArea = FALSE;
              }
            }
            if (localArea) {
              strcpy(cstr, "IP Address");
              if (strstr(str, cstr) != NULL) {
                strl = strlen(cstr);
                for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
                j = 0;
                while((str[j] == ' ') || str[j] == '\t' || str[j] == '.' || str[j] == ':') j++; 
                for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
                sscanf(str, "%s", IPAddress);
                gotIP = TRUE;
              }
            }
            else {
              strcpy(cstr, "Autoconfiguration IP Address");
              if (strstr(str, cstr) != NULL) {
                strl = strlen(cstr);
                for (i = strl; i < strlen(str)+1; i++) str[i-strl] = str[i];
                j = 0;
                while((str[j] == ' ') || str[j] == '\t' || str[j] == '.' || str[j] == ':') j++; 
                for (i = j; i < strlen(str)+1; i++) str[i-j] = str[i];
                sscanf(str, "%s", netIPAddress);
              }
            }
            ntried++;
            gotAll = gotIP & gotHost & gotDomain;
          }
          fclose(logFile);
        }
        else {
          if ((ntried%100) == 0) WaitInMillisecond(20);
          ntried++;
          if (ntried == 10000) printf("Unable to open <myIPConfig.log>\n");
        }
      }
      if (gotHost)   printf("Host Name. . . : %s\n", hostName);
      if (gotDomain) printf("Domain Name. . : %s\n", domainName);
      if (gotIP)     printf("IP Address . . : %s\n", IPAddress);
    }
  }
}


//
// Utility functions, date and time
//
struct tm *localDateTime;

char *TimeString(time_t dateTime) {
  static char myDateTime[30];
  static char *myDateTimePointer = myDateTime;

  localDateTime = localtime(&dateTime);
  strftime(myDateTime, 30, "%H:%M:%S", localDateTime);
  return myDateTimePointer;
}


char *DateString(time_t dateTime) {
  static char myDateTime[30];
  static char *myDateTimePointer = myDateTime;

  localDateTime = localtime(&dateTime);
  strftime(myDateTime, 30, "%b %d %Y", localDateTime);
  return myDateTimePointer;
}


char *DateStringNoSpace(time_t dateTime) {
  static char myDateTime[30];
  static char *myDateTimePointer = myDateTime;

  localDateTime = localtime(&dateTime);
  strftime(myDateTime, 30, "%Y%m%d", localDateTime);
  return myDateTimePointer;
}


char *DateTimeString(time_t dateTime) {
  static char myDateTime[30];
  static char *myDateTimePointer = myDateTime;

  localDateTime = localtime(&dateTime);
  strftime(myDateTime, 30, "%b %d %Y %H:%M:%S", localDateTime);
  return myDateTimePointer;
}


char *Date_TimeString(time_t dateTime) {
  static char myDateTime[30];
  static char *myDateTimePointer = myDateTime;

  localDateTime = localtime(&dateTime);
  strftime(myDateTime, 30, "%b-%d-%Y %H:%M:%S", localDateTime);
  return myDateTimePointer;
}


char *WeekdayString(time_t dateTime) {
  static char myDateTime[30];
  static char *myDateTimePointer = myDateTime;

  localDateTime = localtime(&dateTime);
  strftime(myDateTime, 30, "%a %b %d %Y", localDateTime);
  return myDateTimePointer;
}


char *WeekdayTimeString(time_t dateTime) {
  static char myDateTime[30];
  static char *myDateTimePointer = myDateTime;

  localDateTime = localtime(&dateTime);
  strftime(myDateTime, 30, "%a %b %d %Y %H:%M:%S", localDateTime);
  return myDateTimePointer;
}


//
// Utility functions
//
//
// double ABS(double x);
// Return an absolute value
//
double ABS(double x) {
  if (x > 0.0) return x;
  else return -x;
}



//
// int checkParity(unsigned int dataWord, unsigned int *CommPEct);
// Check for fiber transmission parity error
//
int checkParity(unsigned int dataWord, unsigned int *CommPEct) {
  int bitct, shift;

  bitct = 0;
  for (shift = 0; shift < 32; ++shift) {
    if( (dataWord & (1<<shift)) != 0 ) bitct++;
  }

  if (!(bitct & 0x00000001)) {
    ++(*CommPEct);
    return 1;
  }
  else {
    return 0;
  }
}



//
// double CorrelationEffi(double x[], double y[], int ndata);
// Compute the correlation efficiency of two measurements 
//        x : array of double
//            data from first measurements
//        y : arrray of double
//            data from second measurements
//    ndata : int
//            number of data point
//  Return value is correlation efficiency of two measurements
//
double CorrelationEffi(double x[], double y[], int ndata) {
  double ceffi, cov, xsum, ysum, xx, xy, yy, xerr, yerr, nd;
  int i;

  ceffi = 0.0;
  if (ndata > 0) {
    nd = (double) ndata;
    xsum = 0.0;
    ysum = 0.0;
    xx = 0.0;
    yy = 0.0;
    xy = 0.0;
    for (i = 0; i < ndata; i++) {
      xsum += x[i];
      ysum += y[i];
      xx += x[i]*x[i];
      yy += y[i]*y[i];
      xy += x[i]*y[i];
    }
  }
  xerr = sqrt(xx/nd - (xsum/nd)*(xsum/nd));
  yerr = sqrt(yy/nd - (ysum/nd)*(ysum/nd));
  cov = xy/nd - (xsum/nd)*(ysum/nd);
  if ((xerr == 0.0) && (yerr == 0.0)) ceffi = 1.0;
  else if ((xerr == 0.0) || (yerr == 0.0)) ceffi = 0.0;
  else ceffi = cov / (xerr*yerr);
   
  return ceffi;
}



//
// double CHISquare(double cval[], double mval[], double err[], int ndata, int npara);
// Compute CHI2 per degree freedom
//     cval : array of double
//            Calculated data value
//     mval : arrray of double
//            Measured data value
//      err : array of double
//            Error of the measurement
//    ndata : int
//            number of data point
//    npara : int
//            number parameters for the fit/calculation 
// Return value is CHI2 per degree freedom
//   
double CHISquare(double cval[], double mval[], double err[], int ndata, int npara) {
  double chi2, y;
  int i, ndf;

  ndf = ndata - npara;
  chi2 = 0.0;
  for (i = 0; i < ndata; i++) {
    if (err[i] <= 0.0) ndf--;
    else {
      y = (cval[i] - mval[i]) / err[i];
      chi2 += y*y;
    }
  }
  if (ndf <= 0) chi2 = 0.0;
  else chi2 /= ((double) ndf);

  return chi2;
}



//
// double CHISquareFromDiff(double diff[], double err[], int ndata, int npara);
// Compute CHI2 per degree freedom
//     diff : array of double
//            (Calculated data value) - (Measured data value)
//      err : array of double
//            Error of the measurement
//    ndata : int
//            number of data point
//    npara : int
//            number parameters for the fit/calculation 
// Return value is CHI2 per degree freedom
//   
double CHISquareFromDiff(double diff[], double err[], int ndata, int npara) {
  double chi2, y;
  int i, ndf;

  ndf = ndata - npara;
  chi2 = 0.0;
  for (i = 0; i < ndata; i++) {
    if (err[i] <= 0.0) ndf--;
    else {
      y = diff[i] / err[i];
      chi2 += y*y;
    }
  }
  if (ndf <= 0) chi2 = 0.0;
  else chi2 /= ((double) ndf);

  return chi2;
}



//
// void LineFromLeastSquareMethod(double x[], double y[], int ndata, double para[], double cy[]);
// Using the least square method to fit a line y = y0 + a*x or
//                                             x = x0 + b*y
//   *** Inputs ****
//        x : array of double
//     xerr : array of double
//            corresponding error of x
//        y : arrray of double
//     yerr : array of double
//            corresponding error of y
//    ndata : int
//            number of data point
//   *** Outputs ***
//     para : array of double
//            0 = line status
//                0 -> no valid line fit
//                1 -> line y = y0 + a*x fit is OK
//                2 -> line x = x0 + b*y fit is OK
//                3 -> both line y = y0 + a*x and x = x0 + b*y fits are OK
//            1 = line parameter y0  (intercept)
//            2 = line parameter  a  (slope of the line)
//            3 = chi2 per degree freedom of the fit (0.0 if all error of y are zero)
//            4 = line parameter x0  (intercept)
//            5 = line parameter  b  (slope of the line)
//            6 = chi2 per degree freedom of the fit (0.0 if all error of x are zero)
//            7 = line correlation efficiency
//
void LineFromLeastSquareMethod(double x[], double xerr[], double y[], double yerr[], int ndata, double para[]) {
  double cov, xsum, ysum, xx, xy, yy, xerr2, yerr2, nd, chi2, ndf;
  int i;

  for (i = 0; i < 7; i++) para[i] = 0.0;
  if (ndata > 0) {
    nd = (double) ndata;
    xsum = 0.0;
    ysum = 0.0;
    xx = 0.0;
    yy = 0.0;
    xy = 0.0;
    for (i = 0; i < ndata; i++) {
      xsum += x[i];
      ysum += y[i];
      xx += x[i]*x[i];
      yy += y[i]*y[i];
      xy += x[i]*y[i];
    }
  }
  xerr2 = xx/nd - (xsum/nd)*(xsum/nd);
  yerr2 = yy/nd - (ysum/nd)*(ysum/nd);
  cov = xy/nd - (xsum/nd)*(ysum/nd);
  if ((xerr2 == 0.0) && (yerr2 == 0.0)) {
    // no valid line fit
    para[0] = 0.0;
    para[7] = 0.0;
  }
  else if (xx == 0.0) {
    para[0] = 2.0;
    para[5] = xy/yy;
    para[4] = xsum/nd - para[5]*(ysum/nd);
    para[6] = 0.0;
    para[7] = 1.0;
  }
  else if (yy == 0.0) {
    para[0] = 1.0;
    para[2] = xy/xx;
    para[1] = ysum/nd - para[2]*(xsum/nd);
    para[3] = 0.0;
    para[7] = 1.0;    
  }
  else {
    para[0] = 3.0;
    para[2] = xy/xx;
    para[1] = ysum/nd - para[2]*(xsum/nd);
    for (i = 0; i < ndata; i++) y[i] -= para[1] + para[2]*x[i];
    para[3] = CHISquareFromDiff(y, yerr, ndata, 2);
    para[5] = xy/yy;
    para[4] = xsum/nd - para[5]*(ysum/nd);
    for (i = 0; i < ndata; i++) x[i] -= para[4] + para[5]*y[i];
    para[6] = CHISquareFromDiff(x, xerr, ndata, 2);
    para[7] = cov/sqrt(xerr2*yerr2);    
  }
}



//
// double AverageWithLimit(int ndata, double data[], double dlimit);
// Compute Average by using the data within the limits, the limits are:
//     abs(1.0 - data/average) <= dlimit
// i.e., only data within above limit are used to compute average
// --- INPUTs ---
//    ndata : int
//            number of data point
//     data : array of double
//            data value
//   dlimit : double
//            Delta limit value
// Return value is the computed average
//   
double AverageWithLimit(int ndata, double data[], double dlimit) {
  double average, r, np, sum, minData, maxData, maxDiff;
  int i, imin, imax, nLow, nHigh, j;

  average = 0.0;
  if (ndata <= 0) return average;
  np = 0.0;
  sum = 0.0;
  minData = 1.0e+31;
  maxData = -1.0e+31;
  for (i = 0; i < ndata; i++) {
    if (data[i] <= minData) {
      minData = data[i];
      imin = i;
    }
    if (data[i] >= maxData) {
      maxData = data[i];
      imax = i;
    }
    np += 1.0;
    sum += data[i];
  }
  average = sum / np;
  if (average == 0.0) return average;
  maxDiff = 0.0;
  nLow = 0;
  nHigh = 0;
  for (i = 0; i < ndata; i++) {
    r = data[i] / average;
    if ((1.0-r) > maxDiff) maxDiff = 1.0-r;
    else if ((r-1.0) > maxDiff) maxDiff = r-1.0;
    if (data[i] < average) nLow++;
    else if (data[i] > average) nHigh++;
  }

  while (maxDiff > dlimit) {
    if (nLow < nHigh) {
      j = 0;
      for (i = 0; i < ndata; i++) {
        if (imin != i) data[j++] = data[i];
      }
      ndata--;
    }
    else if (nLow > nHigh) {
      j = 0;
      for (i = 0; i < ndata; i++) {
        if (imax != i) data[j++] = data[i];
      }
      ndata--;
    }
    else {
      j = 0;
      for (i = 0; i < ndata; i++) {
        if ((imin != i) && (imax != i)) data[j++] = data[i];
      }
      ndata -= 2;
    }
    if (ndata <= 0) return average;

    np = 0.0;
    sum = 0.0;
    minData = 1.0e+31;
    maxData = -1.0e+31;
    for (i = 0; i < ndata; i++) {
      if (data[i] <= minData) {
        minData = data[i];
        imin = i;
      }
      if (data[i] >= maxData) {
        maxData = data[i];
        imax = i;
      }
      np += 1.0;
      sum += data[i];
    }
    average = sum / np;
    if (average == 0.0) return average;
    maxDiff = 0.0;
    nLow = 0;
    nHigh = 0;
    for (i = 0; i < ndata; i++) {
      r = data[i] / average;
      if ((1.0-r) > maxDiff) maxDiff = 1.0-r;
      else if ((r-1.0) > maxDiff) maxDiff = r-1.0;
      if (data[i] < average) nLow++;
      else if (data[i] > average) nHigh++;
    }
  }

  return average;
}
