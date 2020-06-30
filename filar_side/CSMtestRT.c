/****************************************************************/
/*                                                              */
/*  file: filarscope.c                                          */
/*                                                              */
/* This program allows to access the resources of a FILAR       */
/* card in a user friendly way and includes some test routines  */
/*                                                              */
/*  Author: Markus Joos, CERN-EP                                */
/*                                                              */
/*  14. Jun. 02  MAJO  created                                  */
/*                                                              */
/****************C 2002 - A nickel program worth a dime**********/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include "rcc_error.h"
#include "io_rcc.h"
#include "cmem_rcc.h"
#include "tstamp.h"
#include "get_input.h"
#include "time.h"

#ifdef DEBUG
  #define debug(x) printf x
#else
  #define debug(x)
#endif

/*special prototypes*/
//extern ReturnCode DumpDesc(void);

/*constants*/
#define MAXBUF   33
#define CHANNELS 5              /*we will not use channel[0]*/
#define BUFSIZE  (256*1024)       /*bytes - changed from '16*1024' by jrhall*/
#define PREFILL  0xfeedbabe
#define RECEIVED_DATA   0			/* baddatas[RECEIVED_DATA][EXPECTED_DATA] array index */
#define EXPECTED_DATA 	1			/* baddatas[][] array index */
#define DATA   0			/* CSM Data array index */
#define CNT 	1			/* CSM Data array index */
#define FLAG	2			/* CSM Data array index */
#define INDEX 		2			/* baddatas[][] index of bad data in data buffer*/

/*types*/
typedef struct
{
unsigned int ocr;       /*0x000*/
unsigned int osr;       /*0x004*/
unsigned int imask;     /*0x008*/
unsigned int fifostat;  /*0x00c*/
unsigned int scw;       /*0x010*/
unsigned int ecw;       /*0x014*/
unsigned int testin;    /*0x018*/
unsigned int d1[57];    /*0x01c-0x0fc*/
unsigned int req1;      /*0x100*/
unsigned int ack1;      /*0x104*/
unsigned int d2[2];     /*0x108-0x10c*/
unsigned int req2;      /*0x110*/
unsigned int ack2;      /*0x114*/
unsigned int d3[2];     /*0x118-0x11c*/
unsigned int req3;      /*0x120*/
unsigned int ack3;      /*0x124*/
unsigned int d4[2];     /*0x128-0x12c*/
unsigned int req4;      /*0x130*/
unsigned int ack4;      /*0x134*/
unsigned int d5[2];     /*0x138-0x13c*/
}T_filar_regs;



/*globals*/
static unsigned int cont, first_event, shandle;
static unsigned int sreg, offset, prefill=0x00000000;
static T_filar_regs *filar;
static unsigned int pcidefault[16]=
  {
  0x001410dc, 0x00800000, 0x02800000, 0x0000ff00, 0xfffffc00, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000001ff
  };
static int bhandle[CHANNELS][MAXBUF],  paddr[CHANNELS][MAXBUF], uaddr[CHANNELS][MAXBUF];
static int active[CHANNELS];
static int bfree[CHANNELS] = {0, MAXBUF, MAXBUF, MAXBUF, MAXBUF};
static int nextbuf[CHANNELS] = {0, 0, 0, 0, 0};
static struct sigaction sa;
static struct tm current_time;


/*****************************/
void SigQuitHandler(int signum)
/*****************************/
{
  cont=0;
  debug(("SigQuitHandler: ctrl+// received\n"));
}


/********************/
int filar_map(int occ)
/********************/
{
  unsigned int eret, regbase, pciaddr;

  eret = IO_Open();
  if (eret != IO_RCC_SUCCESS)
  {
    rcc_error_print(stdout, eret);
    exit(-1);
  }

  eret = IO_PCIDeviceLink(0x10dc, 0x0014, occ, &shandle);
  if (eret != IO_RCC_SUCCESS)
  {
    rcc_error_print(stdout, eret);
    exit(-1);
  }

  eret = IO_PCIConfigReadUInt(shandle, 0x10, &pciaddr);
  if (eret != IO_RCC_SUCCESS)
  {
    rcc_error_print(stdout, eret);
    exit(-1);
  }

  offset = pciaddr & 0xfff;
  pciaddr &= 0xfffff000;
  eret = IO_PCIMemMap(pciaddr, 0x1000, &sreg);
  if (eret != IO_RCC_SUCCESS)
  {
    rcc_error_print(stdout, eret);
    exit(-1);
  }

  filar = (T_filar_regs *)(sreg + offset);
  return(0);
}


/*******************/
int filar_unmap(void)
/*******************/
{
  unsigned int eret;

  eret = IO_PCIMemUnmap(sreg, 0x1000);
  if (eret)
    rcc_error_print(stdout, eret);

  eret = IO_Close();
  if (eret)
    rcc_error_print(stdout, eret);
  return(0);
}


/**********/
int setlat()
/**********/
{
  unsigned int eret, latency, data;

  eret = IO_PCIConfigReadUInt(shandle, 0xC, &data);
  if (eret)
  {
    printf("ailed to read register\n");
    return(-1);
  }
  latency = (data >> 8) & 0xff;
  printf("Enter new value: ");
  latency = gethexd(latency);

  data &= 0xffff00ff;
  data |= (latency << 8);

  eret = IO_PCIConfigWriteUInt(shandle, 0xC, data);
  if (eret)
  {
    printf(" failed to write register\n");
    return(-1);
  }
  return(0);
}


/****************/
int dumpconf(void)
/****************/
{
  unsigned int loop, eret, data, value;

  printf("PCI configuration registers:\n\n");
  printf("Offset  |  content     |  Power-up default\n");
  for(loop = 0; loop < 0x40; loop += 4)
  {
    printf("   0x%02x |", loop);
    eret = IO_PCIConfigReadUInt(shandle, loop, &data);
    if (eret)
      printf(" failed to read register\n");
    else
    {
      printf("   0x%08x |", data);
      if (data == pcidefault[loop >> 2])
        printf(" Yes\n");
      else
        printf(" No (0x%08x)\n", pcidefault[loop >> 2]);
    }
  }
  return(0);
}


/***************/
int dumpmem(void)
/***************/
{
  unsigned int data, value;

  printf("\n=============================================================\n");
  data = filar->ocr;
  printf("Operation Control register (0x%08x)\n", data);
  printf("Channel 1:   Enabled: %s    Reset active: %s    URL0-3: 0x%01x\n", (data & 0x00000200)?"no":"yes", (data & 0x00000100)?"yes":"no ", (data >> 10) & 0xf);
  printf("Channel 2:   Enabled: %s    Reset active: %s    URL0-3: 0x%01x\n", (data & 0x00008000)?"no":"yes", (data & 0x00004000)?"yes":"no ", (data >> 16) & 0xf);
  printf("Channel 3:   Enabled: %s    Reset active: %s    URL0-3: 0x%01x\n", (data & 0x00200000)?"no":"yes", (data & 0x00100000)?"yes":"no ", (data >> 22) & 0xf);
  printf("Channel 4:   Enabled: %s    Reset active: %s    URL0-3: 0x%01x\n", (data & 0x08000000)?"no":"yes", (data & 0x04000000)?"yes":"no ", (data >> 28) & 0xf);
  value = (data >> 3) & 0x7;
  printf("Test mode:   %s         Page size:  ", (data & 0x00000040)?"on":"off");
  if (value == 0) printf("256 Bytes\n");
  if (value == 1) printf("1 KByte\n");
  if (value == 2) printf("2 KBytes\n");
  if (value == 3) printf("4 KBytes\n");
  if (value == 4) printf("16 KBytes\n");
  if (value == 5) printf("64 KBytes\n");
  if (value == 6) printf("256 KBytes\n");
  if (value == 7) printf("4 MBytes - 8 Bytes\n");
  printf("Swap words:  %s          Swap bytes: %s\n", (data & 0x00000004)?"yes":"no", (data &
0x00000002)?"yes":"no");
  printf("Board reset: %s\n", (data & 0x00000001)?"actrive":"not active");

  data = filar->osr;
  printf("\nOperation Status register (0x%08x)\n",data);
  printf("Channel 1: Present: %s    UXOFF: %s    Overflow: %s    Link Up: %s\n", (data &
0x00080000)?"no ":"yes", (data & 0x00040000)?"yes":"no ", (data & 0x00020000)?"yes":"no ", (data &
0x00010000)?"no ":"yes");
  printf("Channel 2: Present: %s    UXOFF: %s    Overflow: %s    Link Up: %s\n", (data &
0x00800000)?"no ":"yes", (data & 0x00400000)?"yes":"no ", (data & 0x00200000)?"yes":"no ", (data &
0x00100000)?"no ":"yes");
  printf("Channel 3: Present: %s    UXOFF: %s    Overflow: %s    Link Up: %s\n", (data &
0x08000000)?"no ":"yes", (data & 0x04000000)?"yes":"no ", (data & 0x02000000)?"yes":"no ", (data &
0x01000000)?"no ":"yes");
  printf("Channel 4: Present: %s    UXOFF: %s    Overflow: %s    Link Up: %s\n", (data &
0x80000000)?"no ":"yes", (data & 0x40000000)?"yes":"no ", (data & 0x20000000)?"yes":"no ", (data &
0x10000000)?"no ":"yes");
  printf("Card temperature: %d deg. C\n", (data >>8) & 0xff);
  printf("ACK available:    %s    REQ available: %s\n", (data & 0x00000002)?"yes":"no", (data &
0x00000001)?"yes":"no");


  data = filar->imask;
  printf("\nInterrupt register (0x%08x)\n",data);
  printf("Channel 1:  UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x00040000)?"enabled":"disabled", (data & 0x00020000)?"enabled ":"disabled", (data & 0x00010000)?"enabled ":"disabled");
  printf("Channel 2:  UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x00400000)?"enabled":"disabled", (data & 0x00200000)?"enabled ":"disabled", (data & 0x00100000)?"enabled ":"disabled");
  printf("Channel 3:  UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x04000000)?"enabled":"disabled", (data & 0x02000000)?"enabled ":"disabled", (data & 0x01000000)?"enabled ":"disabled");
  printf("Channel 4:  UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x40000000)?"enabled":"disabled", (data & 0x20000000)?"enabled ":"disabled", (data & 0x10000000)?"enabled ":"disabled");
  value = (data >> 8) & 0xff;
  printf("Temperature alarm: ");
  if (value)
    printf("at %d deg. C\n", value);
  else
    printf("disabled\n");
  printf("Interrupt if ACK FIFO not empty: %s\n", (data & 0x00000002)?"enabled ":"disabled");
  printf("Interrupt if REQ FIFO not full: %s\n", (data & 0x00000001)?"enabled ":"disabled");


  data = filar->fifostat;
  printf("\nFIFO status register (0x%08x)\n",data);
  printf("Channel 1: # of free slots in REQ FIFO: %2d     # of entries in ACK FIFO: %2d\n", (data >>

4) & 0xf, data & 0xf);
  printf("Channel 2: # of free slots in REQ FIFO: %2d     # of entries in ACK FIFO: %2d\n", (data >>

12) & 0xf, (data >> 8) & 0xf);
  printf("Channel 3: # of free slots in REQ FIFO: %2d     # of entries in ACK FIFO: %2d\n", (data >>

20) & 0xf, (data >> 16) & 0xf);
  printf("Channel 4: # of free slots in REQ FIFO: %2d     # of entries in ACK FIFO: %2d\n", (data >>

28) & 0xf, (data >> 24) & 0xf);

  data = filar->scw;
  printf("\n Start Control Word (0x%08x)\n",data);
  
  data = filar->ecw;
  printf("\n End Control Word (0x%08x)\n",data);

  printf("=============================================================\n");
  return(0);
}


/***************/
int setocrp(void)
/***************/
{
  int data;

  data = filar->ocr;
  printf("Enter new value for the Operation Control register ");
  data = gethexd(data);
  filar->ocr = data;
  return(0);
}


/*********************************/
int setreq(int channel, int number)
/*********************************/
{
  static int chan = 1, num = 1;
  int free, mode, bufnr, data, loop;

  /* If channel = 0 than assume this is an interactive call of 'setreq' and so prompt the user for input */
  if (!channel)
    mode = 1;
  else
    mode = 0;  
    
  if (!channel)
  {
    printf("Enter the channel to be filled (1..%d) ", CHANNELS - 1);
    chan = getdecd(chan);
  }
  else
    chan = channel;

  /* If number = 0 than assume this is an interactive call of 'setreq' and so prompt the user for input */
  if (!number)
  {
    printf("Enter the number of entries to be sent (1..15) ");
    num = getdecd(num);
  }
  else
    num = number;
    
  data = filar->fifostat;
  free = (data >> (chan * 8 - 4)) & 0xf;

  if (free < num)
  {
    printf("The Request FIFO of channel %d has only space for %d entries\n", chan, free);
    return(2);
  }

  for(loop = 0; loop < num; loop++)
  {
    bufnr = getbuf(chan, mode);
    if (bufnr < 0)
    {
      printf("All memory buffers are in use\n");
      return(1);
    }

    if (chan == 1) filar->req1 = paddr[chan][bufnr];
    if (chan == 2) filar->req2 = paddr[chan][bufnr];
    if (chan == 3) filar->req3 = paddr[chan][bufnr];
    if (chan == 4) filar->req4 = paddr[chan][bufnr];

    if(mode)
    {
    printf("FIFO of channel %d filled with PCI address=0x%08x\n", chan, filar->req1);
      printf("FIFO of channel %d filled with PCI address=0x%08x\n", chan, paddr[chan][bufnr]);
      printf("-->buffer number is %d\n", bufnr);
    }
  }
  return(0);
}


/*************/
int setti(void)
/*************/
{
  printf("To be done\n");
  return(0);
}


/**************/
int setpci(void)
/**************/
{
  unsigned int eret, offset=0, data;

  printf("Enter the offset of the register (4-byte alligned) ");
  offset = gethexd(offset);
  offset &= 0x3c;

  eret = IO_PCIConfigReadUInt(shandle, offset, &data);      
  if (eret)
  {
    printf(" failed to read register\n");
    return(1);
  }
  printf("Enter new value for this register ");
  data = gethexd(data);

  eret = IO_PCIConfigWriteUInt(shandle, offset, data);      
  if (eret)
  {
    printf(" failed to write register\n");
    return(2);
  }

  return(0);
}


/*************/
int setim(void)
/*************/
{
  int data;

  data = filar->imask;
  printf("Enter new value for the Interrupt Mask register ");
  data = gethexd(data);
  filar->imask = data;
  return(0);
}


/**************/
int setreg(void)
/**************/
{
  int fun = 1;

  printf("\n=========================================\n");
  while(fun != 0)
    {
    printf("\n");
    printf("Select an option:\n");
    printf("   1 Operation Control register   2 Request FIFO\n");
    printf("   3 Test Input register          4 PCI config. register\n");
    printf("   5 Interrupt Mask register\n");
    printf("   0 Exit\n");
    printf("Your choice ");
    fun=getdecd(fun);
    if (fun == 1) setocrp();
    if (fun == 2) setreq(0, 0);
    if (fun == 3) setti();
    if (fun == 4) setpci();
    if (fun == 5) setim();
    }
  printf("=========================================\n\n");
}


/****************/
int uio_init(void)
/****************/
{
  unsigned int chan, *ptr, loop2, loop, eret;

  eret = CMEM_Open();
  if (eret)
  {
    printf("Sorry. Failed to open the cmem_rcc library\n");
    rcc_error_print(stdout, eret);
    exit(6);
  }

  for(chan = 1; chan < CHANNELS; chan++)
  {
    for(loop = 0; loop < MAXBUF; loop++)
    {
      prefill = 0;    /*changed by jrhall */
      eret = CMEM_SegmentAllocate(BUFSIZE, "filar", &bhandle[chan][loop]);
      if (eret)
      {
        printf("Sorry. Failed to allocate buffer #%d for channel %d\n", loop + 1, chan);
        rcc_error_print(stdout, eret);
        exit(7);
      }

      eret = CMEM_SegmentVirtualAddress(bhandle[chan][loop], &uaddr[chan][loop]);
      if (eret)
      {
        printf("Sorry. Failed to get virtual address for buffer #%d for channel %d\n", loop + 1, 
chan);
        rcc_error_print(stdout, eret);
        exit(8);
      }

      eret = CMEM_SegmentPhysicalAddress(bhandle[chan][loop], &paddr[chan][loop]);
      if (eret)
      {
        printf("Sorry. Failed to get physical address for buffer #%d for channel %d\n", loop + 1,
chan);
        rcc_error_print(stdout, eret);
        exit(9);
      }

      /*initialise the buffer*/
      ptr = (unsigned int *)uaddr[chan][loop];
      for (loop2 = 0; loop2 < (BUFSIZE >> 2); loop2++)
        *ptr++ = PREFILL;
        /* *ptr++ = prefill++;		/* changed to variable by jrhall */
    }
  }
}


/****************/
int uio_exit(void)
/****************/
{
  unsigned int chan, loop, eret;
  
  for(chan = 1; chan < CHANNELS; chan++)
  {
    for(loop = 0; loop < MAXBUF; loop++)
    {
      eret = CMEM_SegmentFree(bhandle[chan][loop]);
      if (eret)
      {
        printf("Warning: Failed to free buffer #%d for channel %d\n", loop + 1, chan);
        rcc_error_print(stdout, eret);
      }
    }
  }
  
  eret = CMEM_Close();
  if (eret)
  {
    printf("Warning: Failed to close the CMEM_RCC library\n");
    rcc_error_print(stdout, eret);
  }
}


/*******************************/
int getbuf(int channel, int mode)
/*******************************/
{
  int bufnr;

  if (bfree[channel] == 0)
    return(-1);
  else
  {
    bfree[channel]--;
    bufnr = nextbuf[channel];
    nextbuf[channel]++;
    if (nextbuf[channel] > (MAXBUF - 1))
      nextbuf[channel] = 0;
    if (mode)
      printf("Buffer %d allocated\n", bufnr);
    return(bufnr);
  }
}


/*******************************/
int retbuf(int channel, int mode)
/*******************************/
{
  int bufnr;

  bufnr = nextbuf[channel] - MAXBUF + bfree[channel];
  if (bufnr < 0)
    bufnr += MAXBUF;
  bfree[channel]++;
  if (mode == 1)
    printf("Buffer %d returned\n", bufnr);
  return(bufnr);
}


/***************/
int readack(void)
/***************/
{
  unsigned int nacks, chan, loop, loop2, bnum, *ptr, data, data2, data3, value;

  printf("\n=========================================\n");
  for(chan = 1; chan < 5; chan++)
  {
    nacks = (filar->fifostat >> ((chan - 1) * 8)) & 0xf;
    if (!nacks)
      printf("The ACK FIFO of channel %d is empty\n", chan);
    else
    {    
      printf("The ACK FIFO of channel %d contains %d entries\n", chan, nacks);
      for(loop = 0; loop < nacks; loop++)
      { 
        printf("Decoding entry %d:\n", loop);     
        if (chan == 1) data = filar->ack1;
        if (chan == 2) data = filar->ack2;
        if (chan == 3) data = filar->ack3;
        if (chan == 4) data = filar->ack4;
        data2 = filar->scw;
        data3 = filar->ecw;
        printf("Raw data read from ACK register = 0x%08x\n", data);
        value = data & 0x80000000;
        if (value)
          printf("Start control word present:  no\n");
        else
        {
          printf("Start control word present:  yes\n");
          printf("Start control word wrong:    %s\n", (data & 0x40000000)?"yes":"no");
          printf("Start control :              0x%08x\n", data2);
        }

        value = data & 0x20000000;
        if (value)
          printf("End control word present:    no\n");
        else
        {
          printf("End control word present:    yes\n");
          printf("End control word wrong:      %s\n", (data & 0x10000000)?"yes":"no");
          printf("End control :                0x%08x\n", data3);
        }     
        printf("Block length (4-byte words): 0x%08x\n", data & 0x000fffff);
        
        bnum = retbuf(chan, 1);
        /*Print the first 10 words of the event*/
        ptr = (unsigned int *)uaddr[chan][bnum];
        for(loop2 = 0; loop2 < 10; loop2++)
          printf("Word %d = 0x%08x\n", loop2 + 1, *ptr++);
      }
    }
  }
  printf("=========================================\n\n");
  return(0);
}

/***************/
int readreq(void)
/***************/
{
  unsigned int nreqs, chan, loop, loop2, bnum, *ptr, data, data2, data3, value;

  printf("\n=========================================\n");

    nreqs = (filar->fifostat >> ((chan - 1) * 8 + 4)) & 0xf;
    if (!nreqs)
      printf("The REQ FIFO of channel %d is empty\n", chan);
    else
    {
      printf("The REQ FIFO of channel %d contains %d entries\n", chan, nreqs);
      for(loop = 0; loop < nreqs; loop++)
      { 
        printf("Decoding entry %d:\n", loop);     
        if (chan == 1) data = filar->req1;
        if (chan == 2) data = filar->req2;
        if (chan == 3) data = filar->req3;
        if (chan == 4) data = filar->req4;
        data2 = filar->scw;
        data3 = filar->ecw;
        printf("Raw data read from REQ register = 0x%08x\n", data);

        bnum = retbuf(chan, 1);
      }
  }
  printf("=========================================\n\n");
  return(0);
}


/****************/
int dumpbuff(void)
/****************/

/* modified by jrhall to automatically check dumped buffers to see if correct data inside. */
/* Data is assumed to start at 0x00000000 and increment by 0x1 */
{
  static int chan = 1, size = 0x10, bnum = 0;
  unsigned int loop, *ptr, testdata = 0x00000000, dataerror = 0;

  printf("\n=========================================\n");
  printf("Enter the number of the channel \n");
  chan = getdecd(chan);
  printf("Enter the number of the buffer \n");
  bnum = getdecd(bnum);
  printf("Enter the number of words to be dumped \n");
  size = getdecd(size);

  ptr = (unsigned int *)uaddr[chan][bnum];
  testdata = *ptr;		/* start jrhall addition */
  testdata++;
  ptr++;			/* end jrhall addition */

  for(loop = 1; loop < size; loop++)
    /* printf("Offset=0x%04x  data=0x%08x\n", loop * 4, *ptr++);  *** original code ***/
  {						/* start jrhall addition */
    printf("Offset=0x%04x  data=0x%08x\n", loop * 4, *ptr);
    if ((*ptr&0xFFFFFFDF) != (testdata&0xFFFFFFDF))
    {
      dataerror++;
      printf("Data Error at Offset=0x%04x! Data should be: 0x%08x\n", loop*4, testdata);
    }
    ptr++;
    testdata++;
  }						/* end jrhall addition */

  if (dataerror != 0)
  	printf("There were %d errors in the data\n", dataerror);
  else
  	printf("All the data looks good.\n\n");
  printf("=========================================\n\n");
  return(0);
}


/*****************/
int cardreset(void)
/*****************/
{
  unsigned int chan, data;

  data = filar->ocr;
  data |= 0x1;
  filar->ocr = data;
  sleep(1);
  data &= 0xfffffffe;
  filar->ocr = data;

  /*reset the buffers*/
  for(chan = 1; chan < CHANNELS; chan++)
  {
    bfree[chan] = MAXBUF;
    nextbuf[chan] = 0;
  }
  return(0);
}


/*****************/
int linkreset(void)
/*****************/
{
  unsigned int data;

  /*set the URESET bits*/
  filar->ocr |= 0x04104100;
  ts_delay(10); /*to be sure. 1 us should be enough*/

  /*now wait for LDOWN to come up again*/
  printf("Waiting for link to come up...\n");
  while(filar->osr & 0x11110000)
    printf("filar->osr = 0x%08x\n", filar->osr);

  /*reset the URESET bits*/
  filar->ocr &= 0xfbefbeff;
  printf("Link is up.\n");
  return(0);
}


/****************/
int mainhelp(void)
/****************/
{
  printf("Call Markus Joos, 72364, 160663 if you need help\n");
  return(0);
}


/********************************************************/
void SLIDASDataCheck1(unsigned int *bptr, int w, int chan)
/********************************************************/
{
  static int lastlength[CHANNELS] = {0, 0, 0, 0, 0};
  unsigned int *ptr, data;

  if (w != lastlength[chan] && !first_event)
    printf("Channel %d: Packet has %d words (previous had %d words)\n", chan, w, lastlength[chan]); 
  lastlength[chan] = w;

  data = 1;
  for(ptr = bptr; ptr < (bptr + w); ptr++)
  {
    if (*ptr != data)
    {
      printf("ERROR: Channel: %d  received: 0x%08x  expected 0x%08x  offset: 0x%08x\n",chan, *ptr, 
data, (unsigned int)ptr - (unsigned int)bptr);
      /* sleep(1); */
    }
    data = data << 1;
    if (data == 0)
      data = 1;
  }
}


/********************************************************/
void SLIDASDataCheck2(unsigned int *bptr, int w, int chan)
/********************************************************/
{
  static int lastlength[CHANNELS] = {0, 0, 0, 0, 0};
  unsigned int *ptr, data;

  if (w != lastlength[chan] && !first_event)
    printf("Channel %d: Packet has %d words (previous had %d words)\n", chan, w, lastlength[chan]);
  lastlength[chan] = w;

  data = 0xfffffffe;
  for(ptr = bptr; ptr < (bptr + w); ptr++)
  {
    if (*ptr != data)
      printf("ERROR: Channel: %d  received: 0x%08x  expected 0x%08x  offset: 0x%08x\n",chan, *ptr, 
data, (unsigned int)ptr - (unsigned int)bptr);
    data = (data << 1) | 0x1;
    if (data == 0xffffffff)
      data = 0xfffffffe;
  }   
}


/********************************************************/
void SLIDASDataCheck3(unsigned int *bptr, int w, int chan)
/********************************************************/
{
  static int lastlength[CHANNELS] = {0, 0, 0, 0, 0};
  unsigned int *ptr, data; 

  if (w != lastlength[chan] && !first_event)
    printf("Channel %d: Packet has %d words (previous had %d words)\n", chan, w, lastlength[chan]);
  lastlength[chan] = w;


  data = 0xffffffff;
  for(ptr = bptr; ptr < (bptr + w); ptr++)
  {
    if (*ptr != data)
      printf("ERROR: Channel: %d  received: 0x%08x  expected 0x%08x  offset: 0x%08x\n", chan, *ptr, data, (unsigned int)ptr - (unsigned int)bptr);
    if (data == 0xffffffff)
      data = 0;  
    else
      data = 0xffffffff;
  }
}  


/********************************************************/
void SLIDASDataCheck4(unsigned int *bptr, int w, int chan)
/********************************************************/
{
  static int lastlength[CHANNELS] = {0, 0, 0, 0, 0};
  unsigned int *ptr, data; 

  if (w != lastlength[chan] && !first_event)
    printf("Channel %d: Packet has %d words (previous had %d words)\n", chan, w, lastlength[chan]);
  lastlength[chan] = w;


  data = 0xaaaaaaaa;
  for(ptr = bptr; ptr < (bptr + w); ptr++)
  {
    if (*ptr != data)  
      printf("ERROR: Channel: %d  received: 0x%08x  expected 0x%08x  offset: 0x%08x\n",chan, *ptr, data, (unsigned int)ptr - (unsigned int)bptr);
    if (data == 0xaaaaaaaa)
      data = 0x55555555;
    else
      data = 0xaaaaaaaa;
  } 
}   


/********************************************************/
void SLIDASDataCheck8(unsigned int *bptr, int w, int chan)
/********************************************************/
{ 
  static unsigned int l1id[CHANNELS] = {0, 1, 1, 1, 1}, bcid[CHANNELS] = {0, 1, 1, 1, 1};
  static unsigned int hw6[CHANNELS] = {0, 2, 2, 2, 2};
  static int lastlength[CHANNELS] = {0, 0, 0, 0, 0};
  unsigned int slidas_hdr_wrd[] = {0xeeeeeeee, 0x00000020, 0x08000003, 0xa0000000, 0x1, 0x1, 0x2, 0x000000de};
  unsigned int slidas_stat_wrd[] = {0x0, 0x4, 0x0};
  unsigned int slidas_trl_wrd[] = {0x3, 0x0, 0x0};
  const int stat_size = 0x3, hdr_size = 0x8, trl_size = 0x3;
  unsigned int i,*ptr;
  int data_size; 

  if (w != lastlength[chan] && !first_event)
    printf("Channel %d: Packet has %d words (previous had %d words)\n",chan, w, lastlength[chan]);

  if (first_event)  
    l1id[chan] = bcid[chan] = 1;

  lastlength[chan] = w;

  slidas_hdr_wrd[4] = l1id[chan];
  slidas_hdr_wrd[5] = bcid[chan];
  slidas_hdr_wrd[6] = hw6[chan];


  /* Check the header */
  for(i = 0, ptr = bptr; ptr < (bptr + hdr_size); ptr++, i++) 
  {
    if (*ptr != slidas_hdr_wrd[i])
      printf("Channel %d: Header Word %d is %#x\texpected %#x (diff is %#x)\n", chan, i, *ptr, slidas_hdr_wrd[i], (slidas_hdr_wrd[i] - *ptr));
  }

  /* Check the status words */
  ptr = bptr + hdr_size;
  for(i = 0; ptr < (bptr + hdr_size + stat_size); ptr++, i++) 
  {
    if (*ptr != slidas_stat_wrd[i])
      printf("Channel %d: Status word %d is %#x\texpected %#x (diff is %#x)\n", chan, i, *ptr, slidas_stat_wrd[i], (slidas_stat_wrd[i] - *ptr));
  }

  /* Check the data */
  data_size = *(bptr + w - 2);
  ptr = bptr + hdr_size + stat_size;
  for(i = 0; ptr < (bptr + hdr_size + stat_size + data_size); ptr++, i++) 
  {
    if (*ptr != i)
      printf("Channel %d: Data word   %d is %#x\texpected %#x (diff is %#x)\n", chan, *ptr, i, (i - *ptr));
  }

  /* Check the trailer */
  i = 0x0; 
  ptr = bptr + hdr_size + stat_size + data_size;
  if (*ptr != slidas_trl_wrd[i])
    printf("Channel %d: Trailer word  %d is %#x\texpected %#x\n", chan, i, *ptr, slidas_trl_wrd[i]);

  i = 0x1;
  if (data_size != (w - hdr_size - stat_size - trl_size))
    printf("Channel %d: Trailer word %d is %#x instead of %#x\n", chan, i, data_size, (w - hdr_size - stat_size - trl_size));

  i = 0x2;
  if (*(bptr + w - 1) != slidas_trl_wrd[i])
    printf("Channel %d: Trailer word %d is %#x instead of %#x\n", chan, i, *(bptr+w-1), slidas_trl_wrd[i]);

  l1id[chan] = ++l1id[chan] & 0xffff;                 /*Slidas has a 16 bit L1ID counter*/
  bcid[chan] = (bcid[chan] + 3) & 0xfff;              
  hw6[chan] = (hw6[chan] + 3) & 0xfff;
  first_event = 0;
}


/******************/
int csmtest(void)
/******************/
{
	int numbuffersreq = 30, buffercol = 30, free, partialbuffer, bufnr = 0, numwords, pagesize, badentry = 1, remainder=0, checkdataloop, loopcounter = 0;  /* The number of data buffers requested for the specified test */
	int bufferprintloop, addwords, numloops, defaultnumwords, icol, irow;
	unsigned int fsize, bnum, dataprintloop, baddata = 0, index, testdata=0, eret, chan, baddatas[3][1000], loopcheck[8][10000];
	unsigned int  *dataptr, data, fsizedata, fifodata, evdata[65536*buffercol], notready = 1, printdata, enoughdata = 0;
	unsigned int WordID, WordIDEct = 0, WordPE, WordPEct = 0, TDCPE, TDCPEct = 0, maskflagct = 0, datact = 0, combdatact = 0, CurrentDataCt = 0, Tcombdatact = 0, tdcerrorct = 0, CSMMaskct = 0;
	unsigned int datacterror = 0, shift, bitct, HeadEVID, TrailEVID, TrailDatact = 0, headerct = 0, trailerct = 0, CommPEct = 0, totaldatact = 0, CSMMaskEct = 0, nodatact = 0;
	unsigned int TDCHead[3][18], TDCTrail[3][18], TDCData[3][18];
	int notdone, NoMatchHeadError = 0, NoMatchTrailError = 0, NoHeadError = 0, Header = 0, EVIDerror = 0, TDCChIndex = 0, dataindex = 0, offset = 0, firstflag = 0;


	FILE *fout;
	time_t aclock;
	struct tm *current_time;
	char file_name[100], hms[15];

	printf("start test \n");
	/* Make sure only channel one is active */
	filar->ocr = (filar->ocr | 0x08208200);
	filar->ocr = (filar->ocr & 0xfffffdff);
	data = filar->ocr;
	active[1] = (data & 0x00000200) ? 0 : 1;
	if (active[1]) printf("channel 1 is active\n");

		/* Find the page size so the user can enter how many words they wish to receive in blocks of page size */
	pagesize = (filar->ocr & 0x00000038) >> 3; /* the page size */

	switch(pagesize)
	{
		case 0:
			pagesize = 256;
			break;
		case 1:
			pagesize = 1024;
			break;
		case 2:
			pagesize = 2048;
			break;
		case 3:
			pagesize = 4096;
			break;
		case 4:
			pagesize = 16384;
			break;
		case 5:
			pagesize = 65536;
			break;
		case 6:
			pagesize = 262144;
			break;
		case 7:
			pagesize = 4194296;
			break;
		default:
			pagesize = 256;
			break;
	}  /* end switch */

	defaultnumwords = pagesize/4;

	/* Iniatilize variables */
	cont = 1;

	while(cont)
	{
		strcpy(file_name, "/home/data/CSMdata");
		time(&aclock);
		current_time = localtime(&aclock);
		sprintf(file_name, "%s:%d:%d:%d.bin", file_name, current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
		fout = fopen(file_name ,"wb");
		if (fout == NULL)
		{
			printf("Cannot open %s \n", file_name);
			exit(8);
		}
		else
			printf("Opened file %s \n", file_name);
		badentry = 1;
		while(badentry)
		{
			printf("How many data words to receive? (Enter in multiples of the pagesize: %d)\n", pagesize/4);
			numwords = getdecd(defaultnumwords);
			defaultnumwords = numwords;

			numloops = numwords/(pagesize/4);
			if (numloops < numbuffersreq)
				partialbuffer = numloops;
			else
				partialbuffer = numloops % numbuffersreq;
			printf ("partialbuffer: %d\n", partialbuffer);

			/* Check the number of data words to be sent to make sure it fills a whole number of pages*/
			remainder = numwords%(pagesize/4);
			if (remainder)
			{
				addwords = pagesize/4 - remainder;
				printf("You need to send: %d more words.\n", addwords);
				defaultnumwords = numwords + addwords;
				badentry = 1;
			}
			else
			{
				printf("numloops = %d\n", numloops);
				badentry = 0;
			}
		} /* end 'badentry' loop */

  	/* reset the comm link */
		/* Reset Card */
		cardreset();
		linkreset();

		/* Allocate numbuffersreq buffers for channel 1*/
		for (bufnr = 0; bufnr < numbuffersreq; bufnr++)
		{
			filar->req1 = paddr[1][bufnr];
		}

		/*initailize variables*/
		baddata = 0;
		bufnr = 0;
		loopcounter = 0;
		for (icol = 0; icol < 19; icol++)
			for (irow = 0; irow < 4; irow++)
			{
				TDCHead[irow][icol] = 0;
				TDCData[irow][icol] = 0;
				TDCTrail[irow][icol] = 0;
			}

		WordIDEct = 0;
		WordPEct = 0;
		TDCPEct = 0;
		maskflagct = 0;
		datact = 0;
		combdatact = 0;
		CurrentDataCt = 0;
		Tcombdatact = 0;
		tdcerrorct = 0;
		datacterror = 0;
		TrailDatact = 0;
		headerct = 0;
		trailerct = 0;
		CommPEct = 0;
		totaldatact = 0;
		CSMMaskEct = 0;
		nodatact = 0;
		NoMatchHeadError = 0;
		NoHeadError = 0;
		NoMatchTrailError = 0;
		EVIDerror = 0;
		TDCChIndex = 0;
		dataindex = 0;
		offset = 0;
		firstflag = 0;

		/* Grab the data */
		printf("Waiting for data...\n");

/********************************************************************************/
		/* Run this loop as fast as possible */
		while(numloops)
		{
		/*Wait until current buffer is full */
			while(notready) /* Look at CH1 ACK_Available until data becomes available */
			{
				fifodata = filar->fifostat;
				notready++;
				if (fifodata & 0x0000000f)
				{
					loopcheck[1][loopcounter] = notready; /* record how many loops are done before data is available */
					notready = 0;
				}
			}

			notready=1;
			/*Read the ACK FIFO to get the size of the data packet in the data buffer*/
			fsizedata = filar->ack1;
			fsize = fsizedata & 0xfffff; /* fsize in 32bit words. 1M word max */

			/*get a pointer to the data and copy it to 'evdata'*/
			dataptr = (unsigned int *)uaddr[1][bufnr];
			memcpy(&evdata[fsize*bufnr], dataptr, fsize * 4);

							/* sync up with data by finding first marker word 0xd0000000 */
			if (!(firstflag)) /* only sync up for the first buffer load of data */
			{
				while ((evdata[offset] & 0xf0000000) != 0xd0000000 && offset < 19)
					++offset;

				if (offset == 19)
				{
					printf("Cannot find marker word\n");
					++CSMMaskEct;
				}
				else ++CSMMaskct;
					++offset;
				firstflag = 1;
				dataindex = offset;
				printf("offset = %d \n", offset);
				printf("*************************************************************\n\n");
			}

			for (dataindex; dataindex < fsize*(bufnr+1); ++dataindex)
			{

				/* Check for TDC Parity error */
				if (evdata[dataindex] & 0x08000000)
				{
					++TDCPEct;
					printf ("TDC Parity Error! Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n\n", dataindex, TDCChIndex, evdata[dataindex]);
				}

				/* Check for transmission parity error*/
				bitct = 0;
				for (shift = 0; shift < 32; shift++)
					if ((evdata[dataindex] >> shift) & 0x00000001) bitct++; /* Strip off TDC Parity Error */
				if (!(bitct & 0x00000001)) /* strip off last bit to see if bitct is odd */
				{
					++CommPEct;
					printf ("CSM Transmission Parity Error! Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n\n", dataindex, TDCChIndex, evdata[dataindex]);
				}

					/* Make sure CSM Mask word is every 19th word */
				if (TDCChIndex == 18)
				{
					if ((evdata[dataindex] & 0xf0000000) != 0xd0000000)
					{
						printf ("Missing spacer/marker word error! dataindex = %5d, data = 0x%08x\n\n",dataindex, evdata[dataindex]);
						++CSMMaskEct;
						firstflag = 0;		/* resynch with spacer word */
					}
					TDCChIndex = 0;
				}

				WordID = evdata[dataindex] & 0xf0000000; /* Strip off TDC Word ID code */
				switch(WordID)
				{
					case 0x00000000:
						++nodatact;
						++TDCChIndex;
						break;
					case 0x20000000:
						++maskflagct;
						printf (" maskflag word. dataindex = %5d, data = 0x%08x\n\n",dataindex, evdata[dataindex]);
						++TDCChIndex;
						break;
					case 0x30000000:
					case 0x40000000:
						TDCData[DATA][TDCChIndex] = evdata[dataindex];
						++CurrentDataCt;
						++TDCData[CNT][TDCChIndex];
						if (TDCHead[CNT][TDCChIndex] < TDCData[CNT][TDCChIndex])
						{
							++NoHeadError;
							printf ("Data, no header Error! Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex, TDCChIndex, evdata[dataindex]);
							if ((dataindex - 19) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 19, TDCChIndex, evdata[dataindex - 19]);
							if ((dataindex - 38) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 38, TDCChIndex, evdata[dataindex - 38]);
							if ((dataindex - 57) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 57, TDCChIndex, evdata[dataindex - 57]);
							if ((dataindex - 76) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 76, TDCChIndex, evdata[dataindex - 76]);
							if ((dataindex - 95) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 95, TDCChIndex, evdata[dataindex - 95]);
							printf("\n");
						}
						++TDCChIndex;
						break;
					case 0x60000000:
						tdcerrorct++;
						++TDCChIndex;
						break;
					case 0x70000000:
						++TDCChIndex;
						break;
					case 0xa0000000:
						TDCHead[DATA][TDCChIndex] = evdata[dataindex];
						TDCData[CNT][TDCChIndex] = 0;
						++TDCHead[CNT][TDCChIndex];
						if (TDCHead[FLAG][TDCChIndex]) 
						{
							++NoMatchTrailError;
							printf ("Header, no matching trailer Error! Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex, TDCChIndex, evdata[dataindex]);
							if ((dataindex - 19) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 19, TDCChIndex, evdata[dataindex - 19]);
							if ((dataindex - 38) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 38, TDCChIndex, evdata[dataindex - 38]);
							if ((dataindex - 57) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 57, TDCChIndex, evdata[dataindex - 57]);
							if ((dataindex - 76) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 76, TDCChIndex, evdata[dataindex - 76]);
							if ((dataindex - 95) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 95, TDCChIndex, evdata[dataindex - 95]);
							printf("\n");
						}
						TDCHead[FLAG][TDCChIndex] = 1;
						++TDCChIndex;
						break;
					case 0xc0000000:
						TDCTrail[DATA][TDCChIndex] = evdata[dataindex];
						TDCTrail[CNT][TDCChIndex]++;
						if (!TDCHead[FLAG][TDCChIndex])
						{
							++NoMatchHeadError;
							printf ("Trailer, no matching header Error! Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex, TDCChIndex, evdata[dataindex]);
							if ((dataindex - 19) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 19, TDCChIndex, evdata[dataindex - 19]);
							if ((dataindex - 38) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 38, TDCChIndex, evdata[dataindex - 38]);
							if ((dataindex - 57) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 57, TDCChIndex, evdata[dataindex - 57]);
							if ((dataindex - 76) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 76, TDCChIndex, evdata[dataindex - 76]);
							if ((dataindex - 95) > -1)
								printf ("Previous Data. Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex - 95, TDCChIndex, evdata[dataindex - 95]);
							printf("\n");
						}
						TDCHead[FLAG][TDCChIndex] = 0;
						if ((int)(TDCTrail[DATA][TDCChIndex] & 0x00000fff) != (TDCData[CNT][TDCChIndex] + 2))
						{
							++datacterror;
							printf ("Data Count Error! Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex, TDCChIndex, evdata[dataindex]);
							printf ("Trailer Data count: %4d, Received Data count: %4d.\n\n", (int)(TDCTrail[DATA][TDCChIndex] & 0x00000fff), TDCData[CNT][TDCChIndex]);
						}
						if ((TDCHead[DATA][TDCChIndex] & 0x00fff000) != (TDCTrail[DATA][TDCChIndex] & 0x00fff000))
						{
							++EVIDerror;
							printf ("Event ID Error! Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n", dataindex, TDCChIndex, evdata[dataindex]);
							printf ("Header EVID: %4d, Trailer EVID: %4d.\n\n", (int)(TDCTrail[DATA][TDCChIndex] & 0x00000fff), CurrentDataCt);
						}
						++TDCChIndex;
						break;
					case 0xd0000000:
						++CSMMaskct;
						TDCChIndex = 0;
						break;
					default:
						++WordIDEct;
						printf ("Word ID Error! Data index: %6d, TDC Ch: %2d, Data: 0x%08x.\n\n", dataindex, TDCChIndex, evdata[dataindex]);
						++TDCChIndex;
						break;
				} /* end switch */

			} /* end scan data for loop */

			/*loopcheck[2][loopcounter] = fifodata;
			loopcheck[3][loopcounter] = fsize;
			loopcheck[4][loopcounter] = evdata[dataindex-1];
			loopcheck[5][loopcounter] = paddr[1][bufnr];
			loopcheck[6][loopcounter] = bufnr;
			loopcheck[7][loopcounter] = numloops; */
			printf ("\n%8d  0x%08x  %8d  0x%08x  0x%08x  %8d  %8d \n", notready, fifodata, fsize, evdata[dataindex-1], paddr[1][bufnr], bufnr, numloops);

			/* Save buffers into file
			if ((bufnr == (numbuffersreq - 1)) || (numloops == 1))
			{
				printf("\nopening data file. fsize*bufnr = 0x%08x\n", fsize*(bufnr+1));
				fwrite(&evdata[0], sizeof(int), fsize*(bufnr+1), fout);
			} */

			
			/* Fill the next address into the req fifo for the next loop */
			filar->req1 = paddr[1][bufnr];
			/* switch to the next buffer */
			++bufnr;
			if (bufnr == numbuffersreq)
			{
				bufnr = 0;
				dataindex = 0;
			}

			--numloops;
			++loopcounter;
			if (loopcounter > 9999) printf(" **** Overflow loopcheck array ***\n");
		} /* end 'grab data' while() */
/***************************************************************************************/
		/*fclose(fout);*/
		printf("****************************************************************\n");
		if (fclose(fout))
			printf("\nCannot close %s \n", file_name);
		else
			printf("\nClosed file %s \n", file_name);
			
		printf(" Channel    Header   Data     Trailer\n");
		for (icol = 0; icol < 18; ++icol)
		{
		printf(" TDC[%2d] %8d %8d %8d\n", icol, TDCHead[CNT][icol], TDCData[CNT][icol], TDCTrail[CNT][icol]);
			headerct = headerct + TDCHead[CNT][icol];
			datact = datact + TDCData[CNT][icol];
			trailerct = trailerct + TDCTrail[CNT][icol];
		}

		printf ("Data statistics\n");
		printf ("headerct, datact, trailerct, nodatct, CSMMaskct, WordIDEct\n");
		printf ("%5d %8d  %8d  %8d  %8d  %8d\n", headerct, CurrentDataCt, trailerct, nodatact, CSMMaskct, WordIDEct);
		printf ("datactEr, NoMatchHeadEr, NoMatchTrailEr, NoHeadEr, EVIDer, CSMMaskEct\n");
		printf ("%4d %14d %15d  %8d %8d  %8d\n", datacterror, NoMatchHeadError, NoMatchTrailError, NoHeadError, EVIDerror, CSMMaskEct);
		printf ("TDCPEct, CommPEct\n");
		printf ("%4d %9d \n", TDCPEct, CommPEct);

		printf ("\nWould you like to print out all the data? (Press 0 for No or 1 for Yes)\n");
		printdata = getdecd(0);
		icol = 0;
		if (printdata)
			for(dataprintloop = 0; dataprintloop < fsize*bufnr; dataprintloop++)
			{
				if ((evdata[dataprintloop] & 0xf0000000) != 0xd0000000)
				{
					printf ("index : %d. Chan : %2d. Data: 0x%08x \n", dataprintloop, icol, evdata[dataprintloop]);
					++icol;
					if (icol == 18) icol = 0;
				}
				else
					printf ("index : %d. Marker Word Data: 0x%08x \n", dataprintloop, evdata[dataprintloop]);
			}

		printf ("Do you want to run another test? (Press 0 for No, 1 for Yes)\n");
		cont = getdecd(cont);

	} /* End Main While Loop */
} /* End CSMTest

/******************/
int slidastest(void)
/******************/
{
  static unsigned int scw = 0xb0f00000, ecw = 0xe0f00000, sw1 = 1, nol = 1;
  unsigned int fsize, rmode, chan, isready, size[CHANNELS], ffrag, complete, ok=  0, loop, bnum, printdataloop;
  unsigned int  *ptr, data, data2, data3, eret, evdata[CHANNELS][0x10000];

  data = filar->ocr;
  active[1] = (data & 0x00000200) ? 0 : 1;
  active[2] = (data & 0x00008000) ? 0 : 1;
  active[3] = (data & 0x00200000) ? 0 : 1;
  active[4] = (data & 0x08000000) ? 0 : 1;

  printf("Enter the start control word: ");
  scw = gethexd(scw);

  printf("Enter the end control word: ");
  ecw = gethexd(ecw);

  printf("How many packets do you want to check? (0 = run forever)");
  nol = getdecd(nol);
  if (!nol)
    rmode = 1;
  else
    rmode = 0;

  while(!ok)
  {
    printf("Enter the value of SW1 on the SLIDAS ");
    sw1 = getdecd(sw1);
    if (sw1 != 1 && sw1 != 2  && sw1 != 3  && sw1 != 4 && sw1 != 8)
      printf("Sorry. Only positions 1,2,3,4 and 8 are supported\n");
    else
      ok = 1;
  }

  cardreset();
  linkreset();

  first_event = 1;
  loop = 0;
  cont = 1;
  if (rmode)
    printf("Running! Press <ctrl+\\> when finished\n");

while(cont)
  {
  if (loop == 0)
    printf("Waiting for data...\n");
    
  /*Receive a complete, potentially fragmented packet from all active channels*/
  complete = 0;
  ffrag = 1;
  for(chan = 1; chan < CHANNELS; chan++)
    size[chan] = 0;
   
  while(!complete)
  {      
    /*Fill the REQ FIFO of all active channels with one entry*/
    for(chan = 1; chan < CHANNELS; chan++)
    {
      if (active[chan])
      {
        /*Write one address to the request FIFO*/    
        eret = setreq(chan ,1);
        if (eret)
        {
          printf("Error %d received from setreq for channel %d\n", eret, chan);
          return(1);
        }
      }
    }

    /*Wait for a fragment to arrive on all active channels*/
    while(1)
    {
      data = filar->fifostat;
      isready = 1;
      if (active[1] && !(data & 0x0000000f))  isready = 0;
      if (active[2] && !(data & 0x00000f00))  isready = 0;
      if (active[3] && !(data & 0x000f0000))  isready = 0;
      if (active[4] && !(data & 0x0f000000))  isready = 0;
		/*printf("notready. data=0x%08x\n", data);*/
      if (isready)
        break;
    }

    /*Read the ACK FIFOs*/
    for(chan = 1; chan < CHANNELS; chan++)
    {
      if(active[chan])
      {
        if (chan == 1) data = filar->ack1;
        if (chan == 2) data = filar->ack2;
        if (chan == 3) data = filar->ack3;
        if (chan == 4) data = filar->ack4;
        data2 = filar->scw;
        data3 = filar->ecw;
        fsize = data & 0xfffff;

        if (loop == 0)
        {
          printf("Channel %d:\n", chan);
          printf("Start control word present:  %s\n", (data & 0x80000000)?"no":"yes");
          printf("Start control word wrong:    %s\n", (data & 0x40000000)?"yes":"no");
          printf("Start control word:          0x%08x\n", data2);
          printf("End control word present:    %s\n", (data & 0x20000000)?"no":"yes");
          printf("End control word wrong:      %s\n", (data & 0x10000000)?"yes":"no");
          printf("End control word:            0x%08x\n", data3);
          printf("Block length (4-byte words): 0x%08x\n", fsize);
        }

        /*Check the content of the ACK FIFO*/
        if (ffrag && (data & 0x80000000)) printf("ERROR: Packet #%d has no start control word\n", loop);
        if (ffrag && (data & 0x40000000)) printf("ERROR: Packet #%d has start control word 0x%08x\n", loop, data2);
        if (data2 & 0x3) printf("ERROR: Packet #%d has error %d in start control word\n", loop, data2 & 0x3);
        if (data3 & 0x3) printf("ERROR: Packet #%d has error %d in end control word\n", loop, data3 & 0x3);
        if (!(data & 0x20000000))
        {
          if((data & 0x10000000))
            printf("ERROR: Packet #%d has end control word 0x%08x\n", loop, data3);
          complete = 1;
        }
        ffrag = 0;

        bnum = retbuf(chan, 0); /*return the buffer and get a pointer to the data*/
        ptr = (unsigned int *)uaddr[chan][bnum];


        if (fsize) /*Copy the data into the array*/
        {
          if (loop == 0)
            printf("Copying 0x%08x bytes\n", fsize * 4);
          memcpy(&evdata[chan][size[chan]], ptr, fsize * 4);
	  for(printdataloop=0; printdataloop < 100; printdataloop++)
	  printf ("Data: 0x%08x\n", evdata[chan][printdataloop]);

          size[chan] += fsize;
        }
      }
    }
  }

  /*Check the data*/
  for(chan = 1; chan < CHANNELS; chan++)
  {
    if(active[chan])
    {
      if (sw1 == 1) SLIDASDataCheck1(evdata[chan], size[chan], chan);
      if (sw1 == 2) SLIDASDataCheck2(evdata[chan], size[chan], chan);
      if (sw1 == 3) SLIDASDataCheck3(evdata[chan], size[chan], chan);
      if (sw1 == 4) SLIDASDataCheck4(evdata[chan], size[chan], chan);
      if (sw1 == 8) SLIDASDataCheck8(evdata[chan], size[chan], chan);
    }
  }

  loop++;
  if (nol)
    {
    nol--;
    if (!nol)
      break;
    }
  }
}

/*****************/
int filarconf(void)
/*****************/
{
  static int bswap = 0, wswap = 0, psize = 2, active[CHANNELS] = {0, 1, 1, 1, 1};
  int chan, data;

  printf("=============================================================\n");
  data = 0;
  for(chan = 1; chan < CHANNELS; chan++)
  {  
    printf("Enable channel %d (1=yes 0=no) ", chan);
    active[chan] = getdecd(active[chan]);
    if (!active[chan])
      data += (1 << (6 * chan + 3));
  }
  printf("Select page size:\n");
  printf("0=256 Bytes  1=1 KB  2=2 KB  3=4 KB  4=16 KB  5=64KB  6=256KB  7=4MB\n");
  printf("Your choice ");
  psize = getdecd(psize);
  printf("Enable word swapping (1=yes 0=no) ");
  wswap = getdecd(wswap);
  printf("Enable byte swapping (1=yes 0=no) ");
  bswap = getdecd(bswap);

  data += (bswap << 1) + (wswap << 2) + (psize << 3);
  printf("Writing 0x%08x to the OPCTL register\n", data);
  filar->ocr = data;
  printf("\n=============================================================\n");
}


/**************************/
main(int argc, char *argv[])
/**************************/
{
  static int data, ret, fun = 1, occ = 1;

  if ((argc==2)&&(sscanf(argv[1],"%d",&occ)==1)) {argc--;} else {occ;}
  if (argc != 1)
  {
  printf("This is FILARSCOPE. \n\n");
  printf("Usage: filarscopr [FILAR occurrence]\n");
  exit(0);
  }


  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = SigQuitHandler;
  ret=sigaction(SIGQUIT, &sa, NULL);
  if (ret < 0)
  {
    printf("Cannot install signal handler (error=%d)\n", ret);
    exit(0);
  }

  ts_open(1, TS_DUMMY);
  uio_init();
  filar_map(occ);

  ret = IO_PCIConfigReadUInt(shandle, 0x8, &data);
  if (ret != IO_RCC_SUCCESS)
  {
    rcc_error_print(stdout, ret);
    exit(-1);
  }


  printf("\n\n\nThis is FILARSCOPE running on a card of revision %02x\n", data & 0xff);
  while(fun != 0)
  {
    printf("\n");
    printf("Select an option:\n");
    printf("  1 Print help           2 Dump PCI conf. space  3 Dump PCI MEM registers\n");
    printf("  4 Write to a register  5 Read ACK FIFO         6 Reset the FILAR\n");
    printf("  7 Reset the S-Link     8 Dump a buffer         9 SLIDAS test\n");
    printf(" 10 Configure FILAR     11 Set latency counter  12 CSM Test\n");
    printf("  0 Quit\n");
    printf("Your choice ");
    fun = getdecd(fun);
    if (fun == 1) mainhelp();
    if (fun == 2) dumpconf();
    if (fun == 3) dumpmem();
    if (fun == 4) setreg();
    if (fun == 5) readack();
    if (fun == 6) cardreset();
    if (fun == 7) linkreset();
    if (fun == 8) dumpbuff();
    if (fun == 9) slidastest();
    if (fun == 10) filarconf();
    if (fun == 11) setlat();
    if (fun == 12) csmtest();
  }

  filar_unmap();
  uio_exit();
  ts_close(TS_DUMMY);
  exit(0);
}

