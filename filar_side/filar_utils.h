#ifndef FILAR_UTILS_H

#define FILAR_UTILS_H

#define MAXBUF                      33
#define CHANNELS                     5   // we will not use channel[0]
//#ifdef ONE_CHANNEL_CARD
#define BUFSIZE             (256*1024)   // bytes
//#else
//#define BUFSIZE             (16*1024)
//#endif
#define PREFILL             0xfeedbabe
#define REQBUFSIZE          0x1000
#define ACKBUFSIZE          0x1000

// types
//
typedef struct {
  unsigned int ocr;       /*0x000*/
  unsigned int osr;       /*0x004*/
  unsigned int imask;     /*0x008*/
  unsigned int fifostat;  /*0x00c*/
  unsigned int scw;       /*0x010*/
  unsigned int ecw;       /*0x014*/
  unsigned int testin;    /*0x018*/
  unsigned int reqadd;    /*0x01c*/
  unsigned int blkctl;    /*0x020*/
  unsigned int ackadd;    /*0x024*/
  unsigned int efstatr;   /*0x028*/
  unsigned int efstata;   /*0x02c*/
  unsigned int d1[52];    /*0x030-0x0fc*/
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
} T_filar_regs;

//
// Function prototypes

  #ifdef __cplusplus
    extern "C" {
  #endif

int filar_map( int occ );
int filar_unmap(void);
int filarconf( int prompt, int *flowType, int takePsize );
int dumpconf(void);
int uio_init(void);
int uio_exit(void);
int retbuf( int channel, int mode );
int readack(void);
int readreq(void);
int cardreset(void);
int linkreset(int linkno);
int mainhelp(void);
int get_bhandle( int chan, int buf );
int get_paddr( int chan, int buf );
int get_uaddr( int chan, int buf );

  #ifdef __cplusplus
    }
  #endif
#endif
