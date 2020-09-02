#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
//#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "rcc_error/rcc_error.h"
#include "io_rcc/io_rcc.h"
#include "cmem_rcc/cmem_rcc.h"

#define FILAR_DEFINES 1   // Define the storage here

#include "filar_utils.h"
#include "filar_storage.h"

#ifdef BIGPHYSAREA
#define ALLOCATE_SEGMENT CMEM_BPASegmentAllocate
#define FREE_SEGMENT     CMEM_BPASegmentFree
#else
#define ALLOCATE_SEGMENT CMEM_SegmentAllocate
#define FREE_SEGMENT     CMEM_SegmentFree
#endif


int filar_map( int occ ) 
{
  unsigned int eret, regbase, pciaddr, offset;

  eret = IO_Open();
  if (eret != IO_RCC_SUCCESS) {
    rcc_error_print(stdout, eret);
    return 1;
  }

  eret = IO_PCIDeviceLink(0x10dc, 0x0014, occ, &shandle);
  if (eret != IO_RCC_SUCCESS) {
    IO_Close();
    rcc_error_print(stdout, eret);
    return 2;
  }

  eret = IO_PCIConfigReadUInt(shandle, 0x10, &pciaddr);
  if (eret != IO_RCC_SUCCESS) {
    IO_Close();
    rcc_error_print(stdout, eret);
    return 3;
  }

  offset = pciaddr & 0xfff;
  pciaddr &= 0xfffff000;
  eret = IO_PCIMemMap(pciaddr, 0x1000, &sreg);
  if (eret != IO_RCC_SUCCESS) {
    IO_Close();
    rcc_error_print(stdout, eret);
    return 4;
  }

  filar = (T_filar_regs *)(sreg + offset);
  return 0;
}


int filar_unmap(void)
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


int filarconf( int prompt, int *flowType, int takePsize )
{
//#ifdef ONE_CHANNEL_CARD
  static int bswap = 0, wswap = 0, psize = 6, active[CHANNELS] = {0, 1, 1, 1, 1};
//#else
//  static int bswap = 0, wswap = 0, psize = 4, active[CHANNELS] = {0, 1, 1, 1, 1};
//#endif
  int chan, data;

  if( prompt ) 
    {
      printf("=============================================================\n");
    }
  else
    {
      if( takePsize < 0 )
	takePsize = 0;
//#ifdef ONE_CHANNEL_CARD
      if( takePsize > 6 )
	takePsize = 6;
//#else
//      if( takePsize > 4 )
//	takePsize = 4;
//#endif
      psize = takePsize;
    }
  data = 0;
  for(chan = 1; chan < CHANNELS; chan++)
  {
    if( prompt )
      {
	printf("Enable channel %d (1=yes 0=no) ", chan);
	active[chan] = getdecd(active[chan]);
      }
    else
      {
	active[chan] = 1;
      }

    if (!active[chan])
      data += (1 << (6 * chan + 3));
  }

  if( prompt )
    {
      printf("Select page size:\n");
//#ifdef ONE_CHANNEL_CARD
      printf("0=256 Bytes  1=1 KB  2=2 KB  3=4 KB  4=16 KB  5=64KB  6=256KB  \n");
//#else
//      printf("0=256 Bytes  1=1 KB  2=2 KB  3=4 KB  4=16 KB  \n");
//#endif
      printf("Your choice ");
      psize = getdecd(psize);
      printf("Enable word swapping (1=yes 0=no) ");
      wswap = getdecd(wswap);
      printf("Enable byte swapping (1=yes 0=no) ");
      bswap = getdecd(bswap);
    }

  data += (bswap << 1) + (wswap << 2) + (psize << 3);
//  printf("Writing 0x%08x to the OPCTL register\n", data);
  filar->ocr = data;

  if( prompt && flowType )
    {
      printf("TDMux(1) or EBuild(0) data  ");
      *flowType = getdecd( *flowType );
    }
  printf("===== Configure FILAR Done ================\n");
  if( prompt )
    printf("\n=============================================================\n");
  return 0;
}


int dumpconf(void)
{
  unsigned int loop, eret, data;

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


int uio_init(void) {
  unsigned int chan, *ptr, loop2, loop, eret;

  eret = CMEM_Open();
  if (eret) {
    printf("Sorry. Failed to open the cmem_rcc library\n");
    rcc_error_print(stdout, eret);
    return 6;
  }

  memset( (void *) bhandle, 0, CHANNELS*MAXBUF*sizeof(int) );

  for (chan = 1; chan < CHANNELS; chan++) 
  {
    for (loop = 0; loop < MAXBUF; loop++) 
    {
      eret = ALLOCATE_SEGMENT(BUFSIZE, "filar", &bhandle[chan][loop]);
      if (eret) 
      {
        uio_exit();
        printf("Sorry. Failed to allocate buffer #%d for channel %d\n", loop + 1, chan);
        rcc_error_print(stdout, eret);
        return 7;
      }

      eret = CMEM_SegmentVirtualAddress(bhandle[chan][loop], &uaddr[chan][loop]);
      if (eret) 
      {
        uio_exit();
        printf("Sorry. Failed to get virtual address for buffer #%d for channel %d\n", loop + 1, chan);
        rcc_error_print(stdout, eret);
        return 8;
      }

      eret = CMEM_SegmentPhysicalAddress(bhandle[chan][loop], &paddr[chan][loop]);
      // printf("chan %1d loop %2d paddr 0x%08x\n",chan,loop,paddr[chan][loop]);
      if (eret) 
      {
        uio_exit();
        printf("Sorry. Failed to get physical address for buffer #%d for channel %d\n", loop + 1, chan);
        rcc_error_print(stdout, eret);
        return 9;
      }

      /*initialise the buffer*/
      ptr = (unsigned int *)uaddr[chan][loop];
      for (loop2 = 0; loop2 < (BUFSIZE >> 2); loop2++) *ptr++ = PREFILL;
    }
  }
  
  eret = ALLOCATE_SEGMENT(REQBUFSIZE, "filar", &reqbufhandle);
  if (eret)
  {
    uio_exit();
    printf("Sorry. Failed to allocate REQ buffer\n");
    rcc_error_print(stdout, eret);
    return(10);
  }

  eret = CMEM_SegmentVirtualAddress(reqbufhandle, &requaddr);
  if (eret)
  {
    uio_exit();
    printf("Sorry. Failed to get virtual address for REQ buffer\n");
    rcc_error_print(stdout, eret);
    return(11);
  }

  eret = CMEM_SegmentPhysicalAddress(reqbufhandle, &reqpaddr);
  if (eret)
  {
    uio_exit();
    printf("Sorry. Failed to get physical address for REQ buffer\n");
    rcc_error_print(stdout, eret);
    return(12);
  }
  
  eret = ALLOCATE_SEGMENT(ACKBUFSIZE, "filar", &ackbufhandle);
  if (eret)
  {
    uio_exit();
    printf("Sorry. Failed to allocate ACK buffer\n");
    rcc_error_print(stdout, eret);
    return(13);
  }

  eret = CMEM_SegmentVirtualAddress(ackbufhandle, &ackuaddr);
  if (eret)
  {
    uio_exit();
    printf("Sorry. Failed to get virtual address for ACK buffer\n");
    rcc_error_print(stdout, eret);
    return(14);
  }

  eret = CMEM_SegmentPhysicalAddress(ackbufhandle, &ackpaddr);
  if (eret)
  {
    uio_exit();
    printf("Sorry. Failed to get physical address for ACK buffer\n");
    rcc_error_print(stdout, eret);
    return(15);
  }

  return 0;
}


int uio_exit(void)
{
  unsigned int chan, loop, eret;
  
  for(chan = 1; chan < CHANNELS; chan++)
  {
    for(loop = 0; loop < MAXBUF; loop++)
    {
      eret = FREE_SEGMENT(bhandle[chan][loop]);
      if (eret)
      {
        printf("Warning: Failed to free buffer #%d for channel %d\n", loop + 1, chan);
        rcc_error_print(stdout, eret);
      }
    }
  }
  
  eret = FREE_SEGMENT(reqbufhandle);
  if (eret)
  {
    printf("Warning: Failed to free REQ buffer\n");
    rcc_error_print(stdout, eret);
  } 
  
  eret = FREE_SEGMENT(ackbufhandle);
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
  return 0;
}


int retbuf(int channel, int mode)
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


int readack(void)
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


int readreq(void)
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


int cardreset(void)
{
  unsigned int chan, data;

  //printf("cardreset: initial OPCTL = 0x%08x\n",filar->ocr);
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
  //printf("cardreset: final OPCTL = 0x%08x\n",filar->ocr);
  return(0);
}


int linkreset(int linkno)
{
  //printf("initial OPCTL = 0x%08x with linkno %d\n",filar->ocr,linkno);
    if(linkno == 0) linkno = 0x1111; //reset all channels
    printf("init filar_osr = 0x%08x\n",filar->osr);
    unsigned int data = 0x00000000;
    if(linkno & 0x0001) data |= 0x00000100;
    if(linkno & 0x0010) data |= 0x00004000;
    if(linkno & 0x0100) data |= 0x00100000;
    if(linkno & 0x1000) data |= 0x04000000;
    filar->ocr |= data; //set the URESET bits
    ts_delay(10);  //to be sure. 1 us should be enough
    while(filar->osr & (linkno<<16))  //wait until requested channel up
        printf("filar->osr = 0x%08x\n", filar->osr);
    filar->ocr &= ~data;  //reset the URESET bits
    printf("final OPCTL = 0x%08x\n",filar->ocr);

  return(0);
}


int mainhelp(void)
{
  printf("Call Markus Joos, 72364, 160663 if you need help\n");
  return(0);
}


// No error checking on indices below

int get_bhandle( int chan, int buf )
{
  return bhandle[chan][buf];
}


int get_paddr( int chan, int buf )
{
  return paddr[chan][buf];
}


int get_uaddr( int chan, int buf )
{
  return uaddr[chan][buf];
}
