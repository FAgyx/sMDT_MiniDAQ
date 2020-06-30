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
/****************C 2003 - A nickel program worth a dime**********/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include "rcc_error/rcc_error.h"
#include "io_rcc/io_rcc.h"
#include "cmem_rcc/cmem_rcc.h"
#include "rcc_time_stamp/tstamp.h"
#include "ROSGetInput/get_input.h"

/*prototypes*/
void SigQuitHandler(int signum);
int dumpmem(void);
int filar_map(int occ);
int setlat();
int filar_unmap(void);
int dumpconf(void);
int setreq(int channel, int number);
int setocrp(void);
int setpci(void);
int setti(void);
int setim(void);
int setreg(void);
int setirq(void);
int uio_init(void);
int uio_exit(void);
int getbuf(int channel, int mode);
int retbuf(int channel, int mode);
int readack(void);
int dumpbuff(void);
int cardreset(void);
int linkreset(void);
int mainhelp(void);
void SLIDASDataCheck1(u_int *bptr, int w, int chan);
void SLIDASDataCheck2(u_int *bptr, int w, int chan);
void SLIDASDataCheck3(u_int *bptr, int w, int chan);
void SLIDASDataCheck4(u_int *bptr, int w, int chan);
void SLIDASDataCheck8(u_int *bptr, int w, int chan);
int slidastest(void);
int filarconf(void);
int dmaprot(void);
int dma_menu(void);
int dmaflush(void);
int recorddata(void);

/*constants*/
#define MAXBUF     33
#define CHANNELS   5              /*we will not use channel[0]*/
#define BUFSIZE    (16*1024)      /*bytes*/
#define PREFILL    0xfeedbabe
#define REQBUFSIZE 0x1000
#define ACKBUFSIZE 0x1000

/*types*/
typedef struct
{
  u_int ocr;       /*0x000*/
  u_int osr;       /*0x004*/
  u_int imask;     /*0x008*/
  u_int fifostat;  /*0x00c*/
  u_int scw;       /*0x010*/
  u_int ecw;       /*0x014*/
  u_int testin;    /*0x018*/
  u_int reqadd;    /*0x01c*/
  u_int blkctl;    /*0x020*/
  u_int ackadd;    /*0x024*/
  u_int efstatr;   /*0x028*/
  u_int efstata;   /*0x02c*/
  u_int d1[52];    /*0x030-0x0fc*/
  u_int req1;      /*0x100*/
  u_int ack1;      /*0x104*/
  u_int d2[2];     /*0x108-0x10c*/
  u_int req2;      /*0x110*/
  u_int ack2;      /*0x114*/
  u_int d3[2];     /*0x118-0x11c*/
  u_int req3;      /*0x120*/
  u_int ack3;      /*0x124*/
  u_int d4[2];     /*0x128-0x12c*/
  u_int req4;      /*0x130*/
  u_int ack4;      /*0x134*/
  u_int d5[2];     /*0x138-0x13c*/
} T_filar_regs;


/*globals*/
static u_int cont, first_event, shandle, offset;
static u_long sreg;
static volatile T_filar_regs *filar;
static u_int pcidefault[16] =
{
  0x001410dc, 0x00800000, 0x02800000, 0x0000ff00, 0xfffffc00, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000001ff
};
static u_long reqpaddr, paddr[CHANNELS][MAXBUF], uaddr[CHANNELS][MAXBUF], requaddr, ackpaddr, ackuaddr;
static int bhandle[CHANNELS][MAXBUF], ackbufhandle, reqbufhandle;
static int active[CHANNELS];
static int bfree[CHANNELS] = {0, MAXBUF, MAXBUF, MAXBUF, MAXBUF};
static int nextbuf[CHANNELS] = {0, 0, 0, 0, 0};
static struct sigaction sa;


/*****************************/
void SigQuitHandler(int signum)
/*****************************/
{
  cont=0;
}


/********************/
int filar_map(int occ)
/********************/
{
  u_int eret, pciaddr;

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
  u_int eret;

  eret = IO_PCIMemUnmap(sreg, 0x1000);
  if (eret)
    rcc_error_print(stdout, eret);

  eret = IO_PCIDeviceUnlink(shandle);
  if (eret != IO_RCC_SUCCESS)
  {
    rcc_error_print(stdout, eret);
    exit(-1);
  }  

  eret = IO_Close();
  if (eret)
    rcc_error_print(stdout, eret);
  return(0);
}


/**********/
int setlat()
/**********/
{
  u_int eret, latency, data;

  eret = IO_PCIConfigReadUInt(shandle, 0xC, &data);
  if (eret)
  {
    printf(" failed to read register\n");
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



/**************/
int setirq(void)
/**************/
{
  static int irqnum = 5;
  u_int data, eret;

  printf("Enter the IRQ number: ");
  irqnum = getdecd(irqnum);

  eret = IO_PCIConfigReadUInt(shandle, 0x3c, &data);      
  if (eret)
  {
    printf(" failed to read register\n");
    return(-1);
  }

  data &= 0xffffff00;
  data |= (irqnum & 0xff);

  eret = IO_PCIConfigWriteUInt(shandle, 0x3c, data);  
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
  u_int loop, eret, data;

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
  u_int data, value;
  
  printf("\n=============================================================\n");
  data = filar->ocr;
  printf("Operation Control register (0x%08x)\n", data);
  printf("Channel 1:   Enabled: %s    Reset active: %s    URL0-3: 0x%01x\n", (data & 0x00000200)?"no ":"yes", (data & 0x00000100)?"yes":"no ", (data >> 10) & 0xf);
  printf("Channel 2:   Enabled: %s    Reset active: %s    URL0-3: 0x%01x\n", (data & 0x00008000)?"no ":"yes", (data & 0x00004000)?"yes":"no ", (data >> 16) & 0xf);
  printf("Channel 3:   Enabled: %s    Reset active: %s    URL0-3: 0x%01x\n", (data & 0x00200000)?"no ":"yes", (data & 0x00100000)?"yes":"no ", (data >> 22) & 0xf);
  printf("Channel 4:   Enabled: %s    Reset active: %s    URL0-3: 0x%01x\n", (data & 0x08000000)?"no ":"yes", (data & 0x04000000)?"yes":"no ", (data >> 28) & 0xf);
  printf("Protocol mode (ACKREG_BLOCK):        %s\n", (data & 0x00000080)?"DMA":"Single cycles");
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
  printf("Swap words:  %s          Swap bytes: %s\n", (data & 0x00000004)?"yes":"no", (data & 0x00000002)?"yes":"no");
  printf("Board reset: %s\n", (data & 0x00000001)?"actrive":"not active");
  
  data = filar->osr;
  printf("\nOperation Status register (0x%08x)\n",data);
  printf("Channel 1: Present: %s    UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x00080000)?"no ":"yes", (data & 0x00040000)?"yes":"no ", (data & 0x00020000)?"yes":"no ", (data & 0x00010000)?"yes":"no ");
  printf("Channel 2: Present: %s    UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x00800000)?"no ":"yes", (data & 0x00400000)?"yes":"no ", (data & 0x00200000)?"yes":"no ", (data & 0x00100000)?"yes":"no ");
  printf("Channel 3: Present: %s    UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x08000000)?"no ":"yes", (data & 0x04000000)?"yes":"no ", (data & 0x02000000)?"yes":"no ", (data & 0x01000000)?"yes":"no ");
  printf("Channel 4: Present: %s    UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x80000000)?"no ":"yes", (data & 0x40000000)?"yes":"no ", (data & 0x20000000)?"yes":"no ", (data & 0x10000000)?"yes":"no ");
  printf("Card temperature: %d deg. C\n", (data >>8) & 0xff);
  printf("Protocol mode (ACKREG_BLOCK):            %s\n", (data & 0x00000080)?"DMA":"Single cycles");
  printf("ACK block done:   %s    REQ block done: %s\n", (data & 0x00000008)?"yes":"no ", (data & 0x00000004)?"yes":"no ");
  printf("ACK available:    %s    REQ available:  %s\n", (data & 0x00000002)?"yes":"no ", (data & 0x00000001)?"yes":"no ");
  
  data = filar->imask;
  printf("\nInterrupt register (0x%08x)\n",data);
  printf("Channel 1:  UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x00040000)?"enabled ":"disabled", (data & 0x00020000)?"enabled ":"disabled", (data & 0x00010000)?"enabled ":"disabled");
  printf("Channel 2:  UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x00400000)?"enabled ":"disabled", (data & 0x00200000)?"enabled ":"disabled", (data & 0x00100000)?"enabled ":"disabled");
  printf("Channel 3:  UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x04000000)?"enabled ":"disabled", (data & 0x02000000)?"enabled ":"disabled", (data & 0x01000000)?"enabled ":"disabled");
  printf("Channel 4:  UXOFF: %s    Overflow: %s    LDOWN: %s\n", (data & 0x40000000)?"enabled ":"disabled", (data & 0x20000000)?"enabled ":"disabled", (data & 0x10000000)?"enabled ":"disabled");
  value = (data >> 8) & 0xff;
  printf("Temperature alarm: ");
  if (value)
    printf("at %d deg. C\n", value);
  else
    printf("disabled\n");
  printf("Interrupt if ACK block written:  %s   Interrupt if REQ block read:     %s\n", (data & 0x00000008)?" enabled ":"disabled", (data & 0x00000004)?" enabled ":"disabled");
  printf("Interrupt if ACK FIFO not empty: %s   Interrupt if REQ FIFO not full:  %s\n", (data & 0x00000002)?" enabled ":"disabled",(data & 0x00000001)?" enabled ":"disabled");
  
  data = filar->fifostat;
  printf("\nFIFO status register (0x%08x)\n",data);
  printf("Channel 1: # of free slots in REQ FIFO: %2d     # of entries in ACK FIFO: %2d\n", (data >> 4) & 0xf, data & 0xf);
  printf("Channel 2: # of free slots in REQ FIFO: %2d     # of entries in ACK FIFO: %2d\n", (data >> 12) & 0xf, (data >> 8) & 0xf);
  printf("Channel 3: # of free slots in REQ FIFO: %2d     # of entries in ACK FIFO: %2d\n", (data >> 20) & 0xf, (data >> 16) & 0xf);
  printf("Channel 4: # of free slots in REQ FIFO: %2d     # of entries in ACK FIFO: %2d\n", (data >> 28) & 0xf, (data >> 24) & 0xf);

  data = filar->efstatr;
  printf("\nEFSTATR register (0x%08x)\n",data);
  printf("Channel 1: # of free slots in REQ FIFO: %2d   Channel 2: # of free slots in REQ FIFO: %2d\n", data & 0xff, (data >> 8) & 0xff);
  printf("Channel 3: # of free slots in REQ FIFO: %2d   Channel 4: # of free slots in REQ FIFO: %2d\n", (data >> 16) & 0xff, (data >> 24) & 0xff);

  data = filar->efstata;
  printf("\nEFSTATA register (0x%08x)\n",data);
  printf("Channel 1:# of entries in ACK FIFO: %2d       Channel 2:# of entries in ACK FIFO: %2d\n", data & 0xff, (data >> 8) & 0xff);
  printf("Channel 3:# of entries in ACK FIFO: %2d       Channel 4:# of entries in ACK FIFO: %2d\n", (data >> 16) & 0xff, (data >> 24) & 0xff);

  data = filar->reqadd;
  printf("\nREQ address register (0x%08x)\n",data);
  
  data = filar->blkctl;
  printf("REQ length register  (0x%08x)\n",data);  
  
  data = filar->ackadd;
  printf("ACK address register (0x%08x)\n",data);
  
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
      printf("FIFO of channel %d filled with PCI address=0x%08lx\n", chan, paddr[chan][bufnr]);
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
  u_int eret, offset=0, data;

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
    printf("   5 Interrupt Mask register      6 PCI IRQ\n");
    printf("   0 Exit\n");
    printf("Your choice ");
    fun=getdecd(fun);
    if (fun == 1) setocrp();
    if (fun == 2) setreq(0, 0);
    if (fun == 3) setti();
    if (fun == 4) setpci();
    if (fun == 5) setim();
    if (fun == 6) setirq();
    }
  printf("=========================================\n\n");
  return(0);
}


/****************/
int uio_init(void)
/****************/
{
  u_int chan, *ptr, loop2, loop, eret;

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
        printf("Sorry. Failed to get virtual address for buffer #%d for channel %d\n", loop + 1, chan);
        rcc_error_print(stdout, eret);
        exit(8);
      }

      eret = CMEM_SegmentPhysicalAddress(bhandle[chan][loop], &paddr[chan][loop]);
      if (eret)
      {
        printf("Sorry. Failed to get physical address for buffer #%d for channel %d\n", loop + 1, chan);
        rcc_error_print(stdout, eret);
        exit(9);
      }

      /*initialise the buffer*/
      ptr = (u_int *)uaddr[chan][loop];
      for (loop2 = 0; loop2 < (BUFSIZE >> 2); loop2++)
        *ptr++ = PREFILL;
    }
  }
  
  eret = CMEM_SegmentAllocate(REQBUFSIZE, "filar", &reqbufhandle);
  if (eret)
  {
    printf("Sorry. Failed to allocate REQ buffer\n");
    rcc_error_print(stdout, eret);
    exit(10);
  }

  eret = CMEM_SegmentVirtualAddress(reqbufhandle, &requaddr);
  if (eret)
  {
    printf("Sorry. Failed to get virtual address for REQ buffer\n");
    rcc_error_print(stdout, eret);
    exit(11);
  }

  eret = CMEM_SegmentPhysicalAddress(reqbufhandle, &reqpaddr);
  if (eret)
  {
    printf("Sorry. Failed to get physical address for REQ buffer\n");
    rcc_error_print(stdout, eret);
    exit(12);
  }
  
  eret = CMEM_SegmentAllocate(ACKBUFSIZE, "filar", &ackbufhandle);
  if (eret)
  {
    printf("Sorry. Failed to allocate ACK buffer\n");
    rcc_error_print(stdout, eret);
    exit(13);
  }

  eret = CMEM_SegmentVirtualAddress(ackbufhandle, &ackuaddr);
  if (eret)
  {
    printf("Sorry. Failed to get virtual address for ACK buffer\n");
    rcc_error_print(stdout, eret);
    exit(14);
  }

  eret = CMEM_SegmentPhysicalAddress(ackbufhandle, &ackpaddr);
  if (eret)
  {
    printf("Sorry. Failed to get physical address for ACK buffer\n");
    rcc_error_print(stdout, eret);
    exit(15);
  }
  
 return(0);
}


/****************/
int uio_exit(void)
/****************/
{
  u_int chan, loop, eret;
  
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
  
  eret = CMEM_SegmentFree(reqbufhandle);
  if (eret)
  {
    printf("Warning: Failed to free REQ buffer\n");
    rcc_error_print(stdout, eret);
  } 
  
  eret = CMEM_SegmentFree(ackbufhandle);
  if (eret)
  {
    printf("Warning: Failed to free ACK buffer\n");
    rcc_error_print(stdout, eret);
  }
  
  eret = CMEM_Close();
  if (eret)
  {
    printf("Warning: Failed to close the CMEM_RCC library\n");
    rcc_error_print(stdout, eret);
  }
  return(0);
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
  u_int nacks, chan, loop, loop2, bnum, *ptr, data, data2, data3, value;

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
        ptr = (u_int *)uaddr[chan][bnum];
        for(loop2 = 0; loop2 < 10; loop2++)
          printf("Word %d = 0x%08x\n", loop2 + 1, *ptr++);
      }
    }
  }
  printf("=========================================\n\n");
  return(0);
}


/****************/
int dumpbuff(void)
/****************/
{
  static int chan = 1, bnum = 0;
  static u_int size = 0x10;
  u_int loop, *ptr;
 
  printf("\n=========================================\n");
  printf("Enter the number of the channel \n");
  chan = getdecd(chan);
  printf("Enter the number of the buffer \n");
  bnum = getdecd(bnum);
  printf("Enter the number of words to be dumped \n");
  size = getdecd(size);

  ptr = (u_int *)uaddr[chan][bnum];
  for(loop = 0; loop < size; loop++)
    printf("Offset=0x%04x  data=0x%08x\n", loop * 4, *ptr++);

  printf("=========================================\n\n");
  return(0);
}


/*****************/
int cardreset(void)
/*****************/
{
  u_int chan, data;

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
  static int doit = 1;

  printf("Reset channel 0 (1=yes  0=no): ");
  doit = getdecd (doit);
  if (doit)
  {
    filar->ocr |= 0x00000100; /*set the URESET bits*/
    ts_delay(10);             /*to be sure. 1 us should be enough*/
    printf("Waiting for link to come up...\n");
    while(filar->osr & 0x00010000)
      printf("filar->osr = 0x%08x\n", filar->osr);
    filar->ocr &= 0xfffffeff; /*reset the URESET bits*/
  }

  printf("Reset channel 1 (1=yes  0=no): ");
  doit = getdecd (doit);
  if (doit)
  {
    filar->ocr |= 0x00004000; /*set the URESET bits*/
    ts_delay(10);             /*to be sure. 1 us should be enough*/
    printf("Waiting for link to come up...\n");
    while(filar->osr & 0x00100000)
      printf("filar->osr = 0x%08x\n", filar->osr);
    filar->ocr &= 0xffffbfff; /*reset the URESET bits*/
  }

  printf("Reset channel 2 (1=yes  0=no): ");
  doit = getdecd (doit);
  if (doit)
  {
    filar->ocr |= 0x00100000; /*set the URESET bits*/
    ts_delay(10);             /*to be sure. 1 us should be enough*/
    printf("Waiting for link to come up...\n");
    while(filar->osr & 0x01000000)
      printf("filar->osr = 0x%08x\n", filar->osr);
    filar->ocr &= 0xffefffff; /*reset the URESET bits*/
  }

  printf("Reset channel 3 (1=yes  0=no): ");
  doit = getdecd (doit);
  if (doit)
  {
    filar->ocr |= 0x04000000; /*set the URESET bits*/
    ts_delay(10);             /*to be sure. 1 us should be enough*/
    printf("Waiting for link to come up...\n");
    while(filar->osr & 0x10000000)
      printf("filar->osr = 0x%08x\n", filar->osr);
    filar->ocr &= 0xfbffffff; /*reset the URESET bits*/
  }

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
void SLIDASDataCheck1(u_int *bptr, int w, int chan)
/********************************************************/
{
  static int lastlength[CHANNELS] = {0, 0, 0, 0, 0};
  u_int *ptr, data;

  if (w != lastlength[chan] && !first_event)
    printf("Channel %d: Packet has %d words (previous had %d words)\n", chan, w, lastlength[chan]); 
  lastlength[chan] = w;

  data = 1;
  for(ptr = bptr; ptr < (bptr + w); ptr++)
  {
    if (*ptr != data)
    {
      printf("ERROR: Channel: %d  received: 0x%08x  expected 0x%08x  offset: 0x%08x\n",chan, *ptr, data, (u_int)ptr - (u_int)bptr); 
      sleep(1);
    }
    data = data << 1;
    if (data == 0)
      data = 1;
  }
}


/********************************************************/
void SLIDASDataCheck2(u_int *bptr, int w, int chan)
/********************************************************/
{
  static int lastlength[CHANNELS] = {0, 0, 0, 0, 0};
  u_int *ptr, data;

  if (w != lastlength[chan] && !first_event)
    printf("Channel %d: Packet has %d words (previous had %d words)\n", chan, w, lastlength[chan]);
  lastlength[chan] = w;

  data = 0xfffffffe;
  for(ptr = bptr; ptr < (bptr + w); ptr++)
  {
    if (*ptr != data)
      printf("ERROR: Channel: %d  received: 0x%08x  expected 0x%08x  offset: 0x%08x\n",chan, *ptr, data, (u_int)ptr - (u_int)bptr);
    data = (data << 1) | 0x1;
    if (data == 0xffffffff)
      data = 0xfffffffe;
  }   
}


/********************************************************/
void SLIDASDataCheck3(u_int *bptr, int w, int chan)
/********************************************************/
{
  static int lastlength[CHANNELS] = {0, 0, 0, 0, 0};
  u_int *ptr, data; 

  if (w != lastlength[chan] && !first_event)
    printf("Channel %d: Packet has %d words (previous had %d words)\n", chan, w, lastlength[chan]);
  lastlength[chan] = w;


  data = 0xffffffff;
  for(ptr = bptr; ptr < (bptr + w); ptr++)
  {
    if (*ptr != data)  
      printf("ERROR: Channel: %d  received: 0x%08x  expected 0x%08x  offset: 0x%08x\n", chan, *ptr, data, (u_int)ptr - (u_int)bptr);
    if (data == 0xffffffff)
      data = 0;  
    else
      data = 0xffffffff;
  } 
}  


/********************************************************/
void SLIDASDataCheck4(u_int *bptr, int w, int chan)
/********************************************************/
{
  static int lastlength[CHANNELS] = {0, 0, 0, 0, 0};
  u_int *ptr, data; 

  if (w != lastlength[chan] && !first_event)
    printf("Channel %d: Packet has %d words (previous had %d words)\n", chan, w, lastlength[chan]);
  lastlength[chan] = w;


  data = 0xaaaaaaaa;
  for(ptr = bptr; ptr < (bptr + w); ptr++)
  {
    if (*ptr != data)  
      printf("ERROR: Channel: %d  received: 0x%08x  expected 0x%08x  offset: 0x%08x\n",chan, *ptr, data, (u_int)ptr - (u_int)bptr);
    if (data == 0xaaaaaaaa)
      data = 0x55555555;
    else
      data = 0xaaaaaaaa;
  } 
}   


/********************************************************/
void SLIDASDataCheck8(u_int *bptr, int w, int chan)
/********************************************************/
{ 
  static u_int l1id[CHANNELS] = {0, 1, 1, 1, 1}, bcid[CHANNELS] = {0, 1, 1, 1, 1};
  static int lastlength[CHANNELS] = {0, 0, 0, 0, 0};
  u_int slidas_hdr_wrd[] = {0xee1234ee, 0x00000020, 0x02020000, 0x0, 0x1, 0x1, 0x0, 0x0};
  u_int slidas_stat_wrd = 0x0;
  u_int slidas_trl_wrd[] = {0x1, 0x0, 0x0};
  const int stat_size = 0x1, hdr_size = 0x8, trl_size = 0x3;
  u_int i,*ptr;
  int data_size; 

  if (w != lastlength[chan] && !first_event)
    printf("Channel %d: Packet has %d words (previous had %d words)\n",chan, w, lastlength[chan]);

  if (first_event)  
    l1id[chan] = bcid[chan] = 1;

  lastlength[chan] = w;

  slidas_hdr_wrd[4] = l1id[chan];
  slidas_hdr_wrd[5] = bcid[chan];

  /* Check the header */
  for(i = 0, ptr = bptr; ptr < (bptr + hdr_size); ptr++, i++) 
  {
    if (*ptr != slidas_hdr_wrd[i])
      printf("Channel %d: Header Word %d is %#x\texpected %#x (diff is %#x)\n", chan, i, *ptr, slidas_hdr_wrd[i], (slidas_hdr_wrd[i] - *ptr));
  }

  /* Check the status word */
  ptr = bptr + hdr_size;
  if (*ptr != slidas_stat_wrd)
    printf("Channel %d: Status word is %#x\texpected %#x (diff is %#x)\n", chan, *ptr, slidas_stat_wrd, (slidas_stat_wrd - *ptr));

  /* Check the data */
  data_size = *(bptr + w - 2);
  ptr = bptr + hdr_size + stat_size;
  for(i = 0; ptr < (bptr + hdr_size + stat_size + data_size); ptr++, i++) 
  {
    if (*ptr != i) 
      printf("Channel %d: Data word   %d is %#x\texpected %#x (diff is %#x)\n", chan, i, *ptr, i, (i - *ptr));
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

  l1id[chan] = l1id[chan]++; 
  bcid[chan] = (bcid[chan] + 3) & 0xfff;              
  first_event = 0;
}


/******************/
int slidastest(void)
/******************/
{
  static u_int scw = 0xb0f00000, ecw = 0xe0f00000, sw1 = 1, nol = 1;
  u_int dloop, fsize, rmode, chan, isready, size[CHANNELS], ffrag, complete, ok=  0, loop, bnum;
  u_int checked[CHANNELS], *ptr, data, data2, data3, eret, evdata[CHANNELS][0x10000];
  volatile int vdata; 

  data = filar->ocr;
  active[1] = (data & 0x00000200) ? 0 : 1;
  active[2] = (data & 0x00008000) ? 0 : 1;
  active[3] = (data & 0x00200000) ? 0 : 1;
  active[4] = (data & 0x08000000) ? 0 : 1;
  checked[1] = 0;
  checked[2] = 0;
  checked[3] = 0;
  checked[5] = 0;

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
	vdata = filar->fifostat;
	isready = 1;
	if (active[1] && !(vdata & 0x0000000f))  isready = 0;
	if (active[2] && !(vdata & 0x00000f00))  isready = 0;
	if (active[3] && !(vdata & 0x000f0000))  isready = 0;
	if (active[4] && !(vdata & 0x0f000000))  isready = 0;
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
          ptr = (u_int *)uaddr[chan][bnum];


          if (fsize) /*Copy the data into the array*/
          {
            if (loop == 0)
              printf("Copying 0x%08x bytes\n", fsize * 4);
            memcpy(&evdata[chan][size[chan]], ptr, fsize * 4);
            size[chan] += fsize;
          }

          if (loop == 0)
          {
	    printf("Dumping data of first fragment\n"); 
	    for(dloop = 0; dloop < fsize; dloop++)
	    {
	      printf("Word %d = 0x%08x\n", dloop, evdata[chan][dloop]);
	    }
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

	checked[chan]++;
	if (!(checked[chan] % 100000))
          printf("%d fragments checked OK on channel %d\n", checked[chan], chan);
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
  return(0);
}


/******************/
int recorddata(void)
/******************/
{    
  char filename[200] = {0};
  int loop, isready, eret, nevents, isok, outputFile;
  u_int sdump, *uptr, vdata, data, data2, data3, fsize, bnum;
  void *ptr;
  
  printf("Enter the path and name of the output file :");
  getstrd(filename, "/tmp/rod.data");

  printf("Dump data to screen: ");
  sdump = getdecd(0);

  outputFile = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (outputFile < 0)
  {
    printf("Failed to open %s\n", filename);
    return(-1);
  }

  cardreset();

  filar->ocr |= 0x00000100; /*set the URESET bits*/
  ts_delay(10);             /*to be sure. 1 us should be enough*/
  printf("Waiting for link to come up...\n");
  while(filar->osr & 0x00010000)
    printf("filar->osr = 0x%08x\n", filar->osr);
  filar->ocr &= 0xfffffeff; /*reset the URESET bits*/
  
  printf("Running! Press <ctrl+\\> to stop\n");
  nevents = 0;
  cont = 1;
  while(cont)
  {
    if (nevents == 0)
      printf("Waiting for data...\n"); 
    
    /*Fill the REQ FIFO with one entry*/       
    eret = setreq(1, 1);
    if (eret)
    {
      printf("Error %d received from setreq\n", eret);
      return(1);
    }

    /*Wait for a fragment to arrive*/
    while(1)
    {
      vdata = filar->fifostat;
      isready = 1;
      if (!(vdata & 0x0000000f))  isready = 0;
      if (isready)
        break;
    }

    /*Read the ACK FIFO*/
    data = filar->ack1;
    data2 = filar->scw;
    data3 = filar->ecw;
    fsize = data & 0xfffff;

    /*Check the content of the ACK FIFO*/
    if (data & 0x80000000) printf("ERROR: Packet #%d has no start control word\n", nevents);
    if (data & 0x40000000) printf("ERROR: Packet #%d has start control word 0x%08x\n", nevents, data2);
    if (data2 & 0x3) printf("ERROR: Packet #%d has error %d in start control word\n", nevents, data2 & 0x3);
    if (data3 & 0x3) printf("ERROR: Packet #%d has error %d in end control word\n", nevents, data3 & 0x3);      
    if (!(data & 0x20000000))
    {
      if((data & 0x10000000))
        printf("ERROR: Packet #%d has end control word 0x%08x\n", nevents, data3);
    }

    bnum = retbuf(1, 0); /*return the buffer and get a pointer to the data*/
    ptr = (void *)uaddr[1][bnum];
    uptr = (u_int *)uaddr[1][bnum];
   
    if (sdump)
    {
      printf("The last package had %d words\n", fsize);
      for(loop = 0; loop < fsize; loop++)
        printf("Word %d = 0x%08x\n", loop, *uptr++);
    }
 
    isok = write(outputFile, ptr, fsize * 4);
    if (isok < 0)
    {
      printf("Error in writing to file\n");
      return(-1);
    }
    nevents++;
    if (nevents < 100 || !(nevents % 1000))
      printf("%d events received\n", nevents);
  }
  
  close (outputFile);
  return(0);
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
  printf("0=256 Bytes  1=1 KB  2=2 KB  3=4 KB  4=16 KB\n");
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
  return(0);
}


/***************/
int dmaprot(void)
/***************/
{
  static int enab[5] = {0, 1, 1, 1, 0}, pagesize = 1;
  int bitpos, loop2, loop, data;
  u_int rlen, *reqptr, *ackptr;
  
  cardreset();
  printf("Enter encoded page size (0..7): ");
  pagesize = getdecd(pagesize);
  
  filar->ocr = 0x00000080 + (pagesize << 3);
  filar->imask = 0;
  
  rlen = 0;
  reqptr = (u_int *)requaddr;
  for (loop = 1; loop < 5; loop++)
  {
    printf("Enable channel %d (1=yes  0=no): ", loop);
    enab[loop] = getdecd (enab[loop]);
    if (!enab[loop])
    {
      bitpos = 1 << (9 + loop * 6);
      filar->ocr |= bitpos;
      *reqptr++ = 0;
      rlen++;
      printf("Writing 0 to REQ block for channel %d\n", loop);
    }
    else
    {
      *reqptr++ = 31;
      rlen++;
      printf("Writing 31 to REQ block for channel %d\n", loop);
      for (loop2 = 0; loop2 < 31; loop2++)
      {
        *reqptr++ = paddr[loop][loop2];
        rlen++;
        printf("Writing PCI address 0x%08x for channel %d\n", paddr[loop][loop2], loop);
      }
    }
  }
  
  printf("Writing 0x%08x to ACKADR\n", ackpaddr);
  printf("Writing 0x%08x to BLKCTL\n", rlen);
  printf("Writing 0x%08x to REQADR\n", reqpaddr);
  filar->ackadd = ackpaddr;
  filar->blkctl = rlen;
  filar->reqadd = reqpaddr;
  
  data = 0;
  while (data == 0)
    data = filar->osr & 0x8;

  ackptr = (u_int *)ackuaddr;
  printf("ACK block received. Dumping data:\n");
  for (loop = 0; loop < 4; loop++)
  {
    data = *ackptr++;
    printf("\n%d fragnemts received from channel %d\n", data, loop);
    for (loop2 = 0; loop2 < data; loop2++)
      printf("ACK word %d = 0x%08x\n", loop2, *ackptr++);
  }
  
  return(0);     
}


/****************/
int dmaflush(void)
/****************/
{
  static int pagesize = 1;
  int loop2, loop, data;
  u_int nevents, *reqptr, *ackptr;
  
  cardreset();
  printf("Enter encoded page size (0..7): ");
  pagesize = getdecd(pagesize);
  
  filar->ocr = 0x08208080 + (pagesize << 3);
  filar->imask = 0;
  nevents = 0;
  
  printf("Running! Press <ctrl+\\> to stop\n");
  cont = 1;
  while(cont)
  {
    reqptr = (u_int *)requaddr;

    *reqptr++ = 1;
    *reqptr++ = paddr[1][0];
    *reqptr++ = 0;
    *reqptr++ = 0;
    *reqptr++ = 0;
     
    if (nevents < 5)
    {
      printf("Writing 0x%08x to ACKADR\n", ackpaddr);
      printf("Writing 0x%08x to BLKCTL\n", 5);
      printf("Writing 0x%08x to REQADR\n", reqpaddr);
    }
    filar->ackadd = ackpaddr;
    filar->blkctl = 5;
    filar->reqadd = reqpaddr;
  
    data = 0;
    while (data == 0)
    {
      filar->blkctl |= 0x80000000;
      data = filar->osr & 0x8;
    }

    ackptr = (u_int *)ackuaddr;
    if (nevents < 5)
    {    
      printf("ACK block received. Dumping data:\n");
      for (loop = 0; loop < 4; loop++)
      {
        data = *ackptr++;
        printf("\n%d fragnemts received from channel %d\n", data, loop);
        for (loop2 = 0; loop2 < data; loop2++)
          printf("ACK word %d = 0x%08x\n", loop2, *ackptr++);
      }
    }
    nevents++;
    if (!(nevents % 10000))
      printf("%d events received\n", nevents);
  }
  
  printf("%d events received\n", nevents);
  return(0);     
}


/****************/
int dma_menu(void)
/****************/
{
  int fun = 1;
  
  while(fun != 0)
  {
    printf("\n");
    printf("Select an option:\n");
    printf("  1 Test ACKBLK_DONE bit in OSR     (1..4 active channels)\n");         
    printf("  2 Test ACKBLK_FLUSH bit in BLKCTL (channel 1 only)\n");         
    printf("  0 Quit\n");
    printf("Your choice ");
    fun = getdecd(fun);
    if (fun == 1) dmaprot();
    if (fun == 2) dmaflush();
  }
  return(0);
}



/******************************/
int main(int argc, char *argv[])
/******************************/
{
  static int ret, fun = 1, occ = 1;
  static u_int data;

  if ((argc == 2) && (sscanf(argv[1], "%d", &occ) == 1)) {argc--;} else {occ = 1;}
  if (argc != 1)
  {
    printf("This is FILARSCOPE. \n\n");
    printf("Usage: filarscope [FILAR occurrence]\n");
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
    printf(" 10 Configure FILAR     11 Set latency counter  12 Test DMA protocol\n");
    printf(" 13 Record data\n");
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
    if (fun == 12) dma_menu();
    if (fun == 13) recorddata();
  }

  filar_unmap();
  uio_exit();
  ts_close(TS_DUMMY);
  exit(0);
}

