#ifndef CSMTEST_H

#define CSMTEST_H

/*special prototypes*/
//extern ReturnCode DumpDesc(void);

/*constants*/
#define MAX_EV_DATA           25000000   // size of evdata array
#define RECEIVED_DATA   0		/* baddatas[RECEIVED_DATA][EXPECTED_DATA] array index */
#define EXPECTED_DATA 	1		/* baddatas[][] array index */
#define DATA   	        0		/* CSM Data array index */
#define CNT 	        1		/* CSM Data array index */
#define FLAG	        2		/* CSM Data array index */
#define INDEX 		2		/* baddatas[][] index of bad data in data buffer*/


/*globals*/
unsigned int first_event;
unsigned int prefill=0x00000000;
int flowType = 1;
struct tm current_time;

unsigned int evdata[MAX_EV_DATA];

char csmFileName[120];

typedef enum 
  {
    D_SRC_FIBER = 0,
    D_SRC_DISK
  }   D_SRC_TYPE;


// Function prototypes

void SigQuitHandler(int signum);
void SigIntHandler(int signum);
D_SRC_TYPE getDataSource( FILE **fIn );

int csmtest(void);
int csmAcquire(void);
int captureOneEvent(void);

#endif
