#ifndef FILAR_STORAGE_H

#define FILAR_STORAGE_H

#ifdef FILAR_DEFINES    // These are the definitions, else use external declarations

unsigned int pcidefault[16] =
{
 0x001410dc, 0x00800000, 0x02800000, 0x0000ff00, 0xfffffc00, 0x00000000, 0x00000000, 0x00000000,
 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000001ff
};

u_int paddr[CHANNELS][MAXBUF], uaddr[CHANNELS][MAXBUF];
u_int ackpaddr, reqpaddr, ackuaddr, requaddr;
int bhandle[CHANNELS][MAXBUF], ackbufhandle, reqbufhandle;
int active[CHANNELS], shandle, sreg;
int bfree[CHANNELS] = {0, MAXBUF, MAXBUF, MAXBUF, MAXBUF};
int nextbuf[CHANNELS] = {0, 0, 0, 0, 0};
unsigned int parityBits[32] =
{
 0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 0x00000080,
 0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000, 0x00008000,
 0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 0x00800000,
 0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000
};

T_filar_regs *filar;

#else       // These are the external declarations

extern unsigned int pcidefault[16];

extern u_int paddr[CHANNELS][MAXBUF], uaddr[CHANNELS][MAXBUF];
extern u_int ackpaddr, reqpaddr, ackuaddr, requaddr;
extern int bhandle[CHANNELS][MAXBUF], ackbufhandle, reqbufhandle;
extern int active[CHANNELS], shandle, sreg;
extern int bfree[CHANNELS];
extern int nextbuf[CHANNELS];
extern unsigned int parityBits[32];

extern T_filar_regs *filar;

#endif

#endif
