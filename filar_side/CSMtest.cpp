/****************************************************************/
/*                                                              */
/*  file: CSMtest.c                                             */
/*  based upon original file: filarscope.c                      */
/*                                                              */
/* This program allows to access the resources of a FILAR       */
/* card in a user friendly way and includes some test routines  */
/*                                                              */
/*  Author: Markus Joos, CERN-EP                                */
/*                                                              */
/*  14. Jun. 02  MAJO  created                                  */
/*                                                              */
/*    Mods by J.Hall and B.Ball, University of Michigan         */
/*                                                              */
/****************C 2002 - A nickel program worth a dime**********/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include "rcc_error/rcc_error.h"
#include "io_rcc/io_rcc.h"
#include "cmem_rcc/cmem_rcc.h"
#include "rcc_time_stamp/tstamp.h"
#include "ROSGetInput/get_input.h"
#include "time.h"

#include "filar_utils.h"
#include "filar_storage.h"
#include "csm_utils.h"
#include "CSMtest.h"

//--------------- OPTIONS WHICH MODIFY THE CODE COMPILATION ------------------

#define WRITE_OPTION12_FILE 1
// #define KEEP_BAD_DATA_CYCLES 1
// #define WRITE_DISK_WHEN_READ_DISK 1

//--------------- end OPTIONS WHICH MODIFY THE CODE COMPILATION --------------

#ifdef DEBUG
  #define debug(x) printf x
#else
  #define debug(x)
#endif


/*****************************/
void SigQuitHandler(int signum)
/*****************************/
{
  cont=0;
  debug(("SigQuitHandler: ctrl+// received\n"));
}

/*****************************/
void SigIntHandler(int signum)
/*****************************/
{
  cont=0;
  debug(("SigIntHandler: ctrl+// received\n"));
  longjmp( env, 1 );
}


/*****************************************************************/
D_SRC_TYPE getDataSource( FILE **fIn )
/*****************************************************************/
{
/*
 *  Determine if fiber or a disk file will be used as the data source
 */
  int getType;

  while( 1 )
  {
      printf( "Is the data source fiber(0) or disk file(1) :" );
      getType = 0;
      getType = getdecd( getType );
      if( getType )
      {
          printf( "Read data from disk file named: " );
          csmFileName[0] = '\0';
          getstrd( csmFileName, csmFileName );
	  *fIn = fopen( csmFileName, "r" );
	  if( *fIn == NULL )
	  {
	      printf( "ERROR: --> Cannot open file \"%s\" for input.\n", csmFileName );
	      continue;
	  }
	  printf( "   File \"%s\" opened for input\n\n", csmFileName );
	  getType = (int) D_SRC_DISK;    // Force it to be a good enum
	  break;
      }
      else
      {
	  break;
      }
  }
  return ((D_SRC_TYPE) getType);
}


/******************/
int csmtest(void)
/******************/
{
	int numbuffersreq = 30, buffercol = 30, free, partialbuffer, bufnr = 0;
	int numwords, pagesize, badentry = 1, remainder=0, checkdataloop, loopcounter = 0;  
/* The number of data buffers requested for the specified test */
	int bufferprintloop, addwords, numloops, numReadLoops, defaultnumwords, icol, irow;
	unsigned int fsize, bnum, dataprintloop, baddata = 0, index, testdata=0, eret, chan;
	unsigned int baddatas[3][1000], loopcheck[8][10000];
	unsigned int  *dataptr, data, fsizedata, fifodata; 
/* old -> evdata[65536*buffercol]; NOW IS GLOBAL */
	unsigned int notready = 1, printdata, enoughdata = 0;
	unsigned int WordID, WordIDEct = 0, BadCycleCount = 0, WordPE, WordPEct = 0;
	unsigned int TDCPE, TDCPEct = 0, maskflagct = 0, datact = 0, combdatact = 0;
	unsigned int Tdatact = 0, Tcombdatact = 0, tdcerrorct = 0, CSMMaskct = 0;
	unsigned int datacterror = 0, shift, bitct, HeadEVID, TrailEVID, TrailDatact = 0;
	unsigned int headerct = 0, trailerct = 0, CommPEct = 0, totaldatact = 0, CSMMaskEct = 0, nodatact = 0;
	unsigned int TDCHead[3][18], TDCTrail[3][18], TDCData[3][18], TDCErr[3][18];
	unsigned int TDCMark[3][18], TDCOther[3][18];
	unsigned int evdataOffset;
	int notdone, NoMatchHeadError = 0, NoMatchTrailError = 0, NoHeadError = 0, Header = 0;
	int EVIDerror = 0, TDCChIndex = 0, dataindex = 0, lastDataIndex = 0, offset = 0, firstflag = 0;

	int cycleOK, goodDataOffset;
	int CSMHeaderCnt, CSMTrailerCnt, CSMETrailerCnt, builtEvents, CSMErrorCnt;
	int errReason1, errReason2, errReason3, errReason4;
	int fileIsNull;
	int jumpBack;
	int tstat;

	FILE *fout;
	time_t aclock;
	struct tm *current_time;
	char file_name[100], hms[15];

	int dontCare;

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

	/* Initialize variables */
	cont = 1;

#ifdef WRITE_OPTION12_FILE
	printf( "Output data to what directory? (- for no output) " ); fflush(stdout);
	strcpy( csmFileName, "/home/data" );
	getstrd( csmFileName, csmFileName );
        if( strcmp(csmFileName, "-") == 0 )
	{
	    fileIsNull = 1;
        }
        else
	{
	    fileIsNull = 0;
	    if( csmFileName[strlen(csmFileName)-1] != '/' ) strcat( csmFileName, "/" );
	    strcat( csmFileName, "CSMdata" );
	    time(&aclock);
	    current_time = localtime(&aclock);
	    sprintf(file_name, "%s:%d:%d:%d.bin", csmFileName, current_time->tm_hour, 
		   current_time->tm_min, current_time->tm_sec);
	}
#else
	fileIsNull = 1;
#endif
		 

	while(cont)
	{
#ifdef WRITE_OPTION12_FILE
	        if( !fileIsNull )
		{
		    strcpy(file_name, "/home/data/CSMdata");
		    time(&aclock);
		    current_time = localtime(&aclock);
		    sprintf(file_name, "%s:%d:%d:%d.bin", csmFileName, current_time->tm_hour, 
			current_time->tm_min, current_time->tm_sec);
		    fout = fopen(file_name ,"wb");
		    if (fout == NULL)
		    {
			printf("Cannot open %s \n  --> NO DATA FILE WILL BE WRITTEN! <--\n", 
			       file_name);
			fileIsNull = 1;
		    }
		    else
		    {
			printf("Opened file %s \n", file_name);
		    }
		}
#endif
		badentry = 1;
		fsize = pagesize/4;    // Just as an initial value, but not likely to change
		while(badentry)
		{
			printf("How many data words to receive? (Enter in multiples of the pagesize: %d)\n", 
			       pagesize/4);
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
			if( numwords > MAX_EV_DATA )
			{
			        numwords = MAX_EV_DATA/(pagesize/4);
				numwords = numwords*(pagesize/4);
			        printf( "Number of data words may not exceed %d\n", numwords );
				defaultnumwords = numwords;
				badentry = 1;
			}
			else
			{
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
			}
		} /* end 'badentry' loop */

	     /* initialize variables */
		baddata = 0;
		bufnr = 0;
		loopcounter = 0;
		for (icol = 0; icol < 19; icol++)
		{
			for (irow = 0; irow < 4; irow++)
			{
				TDCHead[irow][icol] = 0;
				TDCData[irow][icol] = 0;
				TDCTrail[irow][icol] = 0;
				TDCErr[irow][icol] = 0;
				TDCMark[irow][icol] = 0;
				TDCOther[irow][icol] = 0;
			}
		}

		WordIDEct = 0;
		BadCycleCount = 0;
		WordPEct = 0;
		TDCPEct = 0;
		maskflagct = 0;
		datact = 0;
		combdatact = 0;
		Tdatact = 0;
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
		lastDataIndex = 0;
		offset = 0;
		firstflag = 0;
		CSMHeaderCnt = 0; 
		CSMTrailerCnt = 0; 
		CSMETrailerCnt = 0; 
		builtEvents = 0;
		CSMErrorCnt = 0;
		goodDataOffset = -1;
		errReason1 = 0;
		errReason2 = 0;
		errReason3 = 0;
		errReason4 = 0;

		tstat = daqEvInit( numbuffersreq, 0 );
		bufnr = 0;

	/* Grab the data */
/*		printf("Waiting for data...\n");   */

/********************************************************************************/
		/* Run this loop as fast as possible 
		   This means splitting the read from the analysis.
		   Avoid any printf during the read section.
		   NOTE -- as structured now, this could create problems for MANY buffers
		           read during the course of a long run.
		*/

		numReadLoops = numloops;
		evdataOffset = 0;
		while(numReadLoops)
		{
		        jumpBack = sigsetjmp( env, 1 );
		        if( jumpBack )          // Interrupt, get out
		        {
			    numloops -= numReadLoops;
		            break;
		        }

		   /* Wait until current buffer is full */
			while(notready) /* Look at CH1 ACK_Available until data becomes available */
			{
				fifodata = filar->fifostat;
			    //  notready++;
				if (fifodata & 0x0000000f)
				{
					loopcheck[1][loopcounter] = notready; /* record how many loops are done before data is available */
					notready = 0;
				}
			}

			notready=1;
		      /* Read the ACK FIFO to get the size of the data packet in the data buffer
			 It had better always be the same for all that we grab!
		      */
			fsizedata = filar->ack1;
			fsize = fsizedata & 0xfffff; /* fsize in 32bit words. 1M word max */

		      /* get a pointer to the data and copy it to 'evdata'*/
			dataptr = (unsigned int *)uaddr[1][bufnr];
			memcpy(&evdata[evdataOffset], dataptr, fsize * 4);
			evdataOffset += fsize;

		      /* Fill the next address into the req fifo for the next loop */
			filar->req1 = paddr[1][bufnr];
		      /* switch to the next buffer */
			if (++bufnr == numbuffersreq)
			{
				bufnr = 0;
			}

			--numReadLoops;
			++loopcounter;
			if (loopcounter > 9999) loopcounter=9999;
		} /* end 'grab data' while() */

/********************************************************************************/
	      /* Analyze grabbed data loop */
		bufnr = 0;
		loopcounter = 0;
		numReadLoops = numloops;
		cycleOK = 0;
		evdataOffset = 1;

		if( !numloops )    // Could have been an interrupt
		{
		    break;
		}
		while(numloops)
		{
		    /* sync up with data by finding first marker word 0xd0000000 */
			if( flowType )
			{
				if (!(firstflag)) /* only sync up for the first buffer load of data */
				{
					while ( ((evdata[offset] & 0xf0000000) != 0xd0000000) && (offset < 19))
						++offset;

					if (offset == 19)
					{
/*						printf("Cannot find marker word\n");  */
						++CSMMaskEct;
					}
/*
					else ++CSMMaskct;
					printf("First marker word found at offset = %d \n", offset);
*/
					++offset;
					firstflag = 1;
				}

				for (dataindex=lastDataIndex; dataindex < evdataOffset*fsize; ++dataindex)
				{
					WordID = evdata[dataindex] & 0xf0000000; /* Strip off TDC Word ID code */
					if( WordID == 0xd0000000 )
					{
					     if( (dataindex+19) > fsize*numReadLoops )
					     {
						  cycleOK = 0;
						  break;
					     }
					     else if( (evdata[dataindex+19] & 0xf0000000) == 0xd0000000 )
					     {
					          if( cycleOK == 0 )
						  {
						    goodDataOffset = dataindex;
						  }
						  cycleOK = 1;
					     }
					     else
					     {
					          cycleOK = 0;
						  ++BadCycleCount;
					     }

					     if( cycleOK )
					     {
						  ++CSMMaskct;
					     }
					     TDCChIndex = 0;
					}
					if( cycleOK )
					{
					  /* Check for TDC Parity error */
					     if (evdata[dataindex] & 0x08000000)
						++TDCPEct;

					     switch(WordID)
					     {
					        case 0x00000000:
							++nodatact;
							if( evdata[dataindex] != 0x04000000 ) ++CommPEct;
							++TDCChIndex;
							break;
						case 0x20000000:
						        dontCare = checkParity( evdata[dataindex], &CommPEct );
							++maskflagct;
							if( TDCChIndex < 18 ) ++TDCMark[CNT][TDCChIndex];
							++TDCChIndex;
							break;
						case 0x30000000:
						case 0x40000000:
						        dontCare = checkParity( evdata[dataindex], &CommPEct );
							++Tdatact;
							if( TDCChIndex < 18 )
							{
							    ++TDCData[CNT][TDCChIndex];
							    if( !TDCHead[FLAG][TDCChIndex] ) ++NoHeadError;
//							      if (TDCHead[CNT][TDCChIndex] < TDCData[CNT][TDCChIndex]) ++NoHeadError;
							}
							++TDCChIndex;
							break;
						case 0x60000000:
						        dontCare = checkParity( evdata[dataindex], &CommPEct );
							tdcerrorct++;
							if( TDCChIndex < 18 ) ++TDCErr[CNT][TDCChIndex];
							++TDCChIndex;
							break;
						case 0x70000000:
						        dontCare = checkParity( evdata[dataindex], &CommPEct );
							if( TDCChIndex < 18 ) ++TDCOther[CNT][TDCChIndex];
							++TDCChIndex;
							break;
						case 0xa0000000:
						        dontCare = checkParity( evdata[dataindex], &CommPEct );
							if( TDCChIndex < 18 )
							{
							    TDCTrail[DATA][TDCChIndex] = 0;
							    ++TDCHead[CNT][TDCChIndex];
							    if (TDCHead[FLAG][TDCChIndex]) ++NoMatchTrailError;
							    TDCHead[FLAG][TDCChIndex] = 1;
							    TDCHead[DATA][TDCChIndex] = evdata[dataindex];
							}
							++TDCChIndex;
							break;
						case 0xc0000000:
						        dontCare = checkParity( evdata[dataindex], &CommPEct );
							if( TDCChIndex < 18 )
							{
							    TDCTrail[DATA][TDCChIndex] = evdata[dataindex];
						 	    TDCTrail[CNT][TDCChIndex]++;
							    if (!TDCHead[FLAG][TDCChIndex]) ++NoMatchHeadError;
							    TDCHead[FLAG][TDCChIndex] = 0;
//							    if (TDCTrail[DATA][TDCChIndex] & 0x00000fff != totaldatact) ++datacterror;
							    if (TDCHead[DATA][TDCChIndex] && (TDCHead[DATA][TDCChIndex] & 0x00fff000) != (TDCTrail[DATA][TDCChIndex] & 0x00fff000)) ++EVIDerror;
							    TDCHead[DATA][TDCChIndex] = 0;
							}
							++TDCChIndex;
							break;
						case 0xd0000000:
						       dontCare =  checkParity( evdata[dataindex], &CommPEct );
//						        if( evdata[dataindex] != 0xd0000000 ) ++CommPEct;
							break;
						default:
						        dontCare = checkParity( evdata[dataindex], &CommPEct );
							if( TDCChIndex < 18 ) ++TDCOther[CNT][TDCChIndex];
							++WordIDEct;
							++TDCChIndex;
							break;
					     } /* end switch */
					}
					/* printf(" TDC[%3d] %8d %8d %8d %8d data = 0x%08x\n", TDCChIndex, TDCHead[CNT][TDCChIndex], TDCData[CNT][TDCChIndex], TDCTrail[CNT][TDCChIndex], dataindex, evdata[dataindex]); */

				} /* end scan data for loop */
				lastDataIndex = dataindex;

			} /* end look at TDM-style data */

			else
			{
			     for( dataindex=lastDataIndex; dataindex<fsize*evdataOffset; ++dataindex )
			     {
				  if( !cycleOK )
				  {
				       WordID = evdata[dataindex] & 0xFF000000;
				       if( WordID == 0x59000000 )
				       {
					    cycleOK = 1;
					    if( goodDataOffset == -1 ) goodDataOffset=dataindex;
				       }
				  }
				  if( cycleOK )
				  {
				       WordID = evdata[dataindex] & 0xF0000000;
				     /* compute tdcno from encoded values */
				       TDCChIndex = evdata[dataindex]>>24 & 0xF;
					switch(WordID)
					{
						case 0x00000000:
							++nodatact;
							break;
						case 0x20000000:
							++maskflagct;
							++TDCMark[CNT][TDCChIndex];
							++totaldatact;
							break;
						case 0x30000000:
						case 0x40000000:
						  /* re-compute tdcno from encoded values if needed */
						        if( (evdata[dataindex]&0x00c00000) == 0x00c00000 )
							{
							    TDCChIndex = 16 + ((evdata[dataindex]>>26) & 0x1);
							}
							++Tdatact;
							++TDCData[CNT][TDCChIndex];
							++totaldatact;
							break;
						case 0x60000000:
							tdcerrorct++;
							++TDCErr[CNT][TDCChIndex];
							++totaldatact;
							break;
						case 0x70000000:
							++TDCOther[CNT][TDCChIndex];
							++totaldatact;
							break;
						case 0xa0000000:
							TDCTrail[DATA][TDCChIndex] = 0;
							++TDCHead[CNT][TDCChIndex];
							if (TDCHead[FLAG][TDCChIndex]) ++NoMatchTrailError;
							TDCHead[FLAG][TDCChIndex] = 1;
							TDCHead[DATA][TDCChIndex] = evdata[dataindex];
							++totaldatact;
							break;
						case 0xc0000000:
							TDCTrail[DATA][TDCChIndex] = evdata[dataindex];
							TDCTrail[CNT][TDCChIndex]++;
							if (!TDCHead[FLAG][TDCChIndex]) ++NoMatchHeadError;
							TDCHead[FLAG][TDCChIndex] = 0;
//							  if (TDCTrail[DATA][TDCChIndex] & 0x00000fff != totaldatact) ++datacterror;
							if (TDCHead[DATA][TDCChIndex] && (TDCHead[DATA][TDCChIndex] & 0x00fff000) != (TDCTrail[DATA][TDCChIndex] & 0x00fff000)) ++EVIDerror;
							TDCHead[DATA][TDCChIndex] = 0;
							++totaldatact;
							break;
						case 0x50000000:
						        WordID = evdata[dataindex] & 0x0F000000;
							if( WordID == 0x09000000 )
							{
							    CSMHeaderCnt++;
							    totaldatact = 1;
							}
							else if( WordID == 0x0 ) /* CSM Error word */
							{
							    CSMErrorCnt++;
							    if( evdata[dataindex] & 0x00800000 ) errReason1++;
							    if( evdata[dataindex] & 0x00400000 ) errReason2++;
							    if( evdata[dataindex] & 0x00200000 ) errReason3++;
							    if( evdata[dataindex] & 0x00100000 ) errReason4++;
							    ++totaldatact;
							}
							else if( WordID == 0x0d000000 )  /* Error trailer */
							{
							    CSMETrailerCnt++;
							    builtEvents++;
							    cycleOK = 0;
							    ++totaldatact;
							    if( (evdata[dataindex] & 0x00000FFF) != totaldatact ) ++datacterror;
							    totaldatact = 0;
							}
							else if( WordID == 0x0b000000 )  /* Normal trailer */
							{
							    CSMTrailerCnt++;
							    builtEvents++;
							    cycleOK = 0;
							    ++totaldatact;
							    if( (evdata[dataindex] & 0x00000FFF) != totaldatact ) ++datacterror;
							    totaldatact = 0;
							}
							break;
					        default:
							++TDCOther[CNT][TDCChIndex];
							++WordIDEct;
							++totaldatact;
							break;
					} /* end switch */
				  }
			     } /* end scan data for loop */
			     lastDataIndex = dataindex;

			} /* end look at event-built data */

			loopcheck[2][loopcounter] = fifodata;
			loopcheck[3][loopcounter] = fsize;
			loopcheck[4][loopcounter] = evdata[dataindex-1];
			loopcheck[5][loopcounter] = paddr[1][bufnr];
			loopcheck[6][loopcounter] = bufnr;
			loopcheck[7][loopcounter] = numloops;
			printf ("%8d  0x%08x  %8d  0x%08x  0x%08x  %8d  %8d \n", loopcheck[1][loopcounter],
				loopcheck[2][loopcounter], loopcheck[3][loopcounter], 
				loopcheck[4][loopcounter], loopcheck[5][loopcounter], 
				loopcheck[6][loopcounter], loopcheck[7][loopcounter]);

		      /* switch to the next buffer */
			if (++bufnr == numbuffersreq)
			{
				bufnr = 0;
			}

			--numloops;
			++loopcounter;
			evdataOffset++;
			if (loopcounter > 9999) loopcounter=9999;
		} /* end 'analyze grabbed data' while() */
/***************************************************************************************/

#ifdef WRITE_OPTION12_FILE
		if( !fileIsNull )
		{
	          /* Save buffers into file */
		    fwrite( evdata, sizeof(unsigned int), numReadLoops*fsize, fout );

	          /* fclose(fout); */
		    if (fclose(fout))
			printf("Cannot close %s \n", file_name);
		    else
			printf("Closed file %s \n", file_name);
		}
#endif
			
		if( flowType )
		{
		    printf( "Fully good data begins at index=%d\n\n", goodDataOffset );
		}
		else
		{
		    printf( "First fully built event begins at index=%d\n\n", goodDataOffset );
		}

		printf(" Channel    Header     Data  Trailer    Error    Mark     Other\n");
		for (icol = 0; icol < 18; ++icol)
		{
		        printf(" TDC[%2d] %8d %8d %8d %8d %8d %8d\n", icol, TDCHead[CNT][icol], TDCData[CNT][icol], 
			      TDCTrail[CNT][icol], TDCErr[CNT][icol], TDCMark[CNT][icol], TDCOther[CNT][icol]);
			headerct = headerct + TDCHead[CNT][icol];
			datact = datact + TDCData[CNT][icol];
			trailerct = trailerct + TDCTrail[CNT][icol];
		}

		printf ("\nData statistics\n");
		printf ("headerct, datact, trailerct, nodatct, CSMMaskct, WordIDEct, BadCycleCnt\n");
		printf ("%5d %8d  %8d  %8d  %8d  %8d  %8d\n", headerct, datact, trailerct, nodatact, 
			CSMMaskct, WordIDEct, BadCycleCount);
		printf ("datactEr, NoMatchHeadEr, NoMatchTrailEr, NoHeadEr, EVIDer, CSMMaskEct\n");
		printf ("%4d %14d %15d  %8d %8d  %8d\n", datacterror, NoMatchHeadError, NoMatchTrailError, 
			NoHeadError, EVIDerror, CSMMaskEct);
		printf ("TDCPEct, CommPEct\n");
		printf ("%4d %9d \n", TDCPEct, CommPEct);
		if( !flowType )
		{
		    printf( "CSM HeaderCnt, TrailerCnt, ETrailerCnt, ErrorCnt, builtEvents\n" );
		    printf( "    %9d   %9d    %9d  %8d    %9d\n", CSMHeaderCnt, CSMTrailerCnt, CSMETrailerCnt,
			    CSMErrorCnt, builtEvents );
		    if( CSMErrorCnt )
		    {
		        printf( "CSM Error summary:\n  Missing trailers %d, Missing headers %d"
			      ", RAM full %d, data wait timeout %d\n", errReason1, errReason2, errReason3,
			      errReason4 );
		    }
		}

		printf ("\nWould you like to print out all the data? (Press 0 for No or 1 for Yes)\n");
		printdata = getdecd(0);
		icol = 0;
		if (printdata)
			for(dataprintloop = 0; dataprintloop < fsize*bufnr; dataprintloop++)
			{
				if ((evdata[dataprintloop] & 0xf0000000) != 0xd0000000)
				{
				        if( flowType )
					{
					   bitct = 0;
					   for (shift = 0; shift < 32; shift++)
					      if ((evdata[dataprintloop] >> shift) & 0x00000001) bitct++; /* Strip off TDC Parity Error */
					}
					printf ("index : %d. Chan : %2d. Data: 0x%08x ", dataprintloop, 
						icol++, evdata[dataprintloop]);
				        if (flowType && !(bitct & 0x00000001)) 
					{
					    printf( "*\n" );
					}
					else
					{
					    if( !flowType )
					    {
						if( (evdata[dataprintloop] & 0xf0000000) == 0x50000000 )
						{
						    WordID = evdata[dataprintloop] & 0x0f000000;
						    if( WordID == 0x09000000 )
							printf( "  CSM Header" );
						    else if( WordID == 0x0b000000 )
							printf( "  CSM Trailer" );
						    else if( WordID == 0x0d000000 )
							printf( "  CSM Error Trailer" );
						}
					    }
					    printf( "\n" );
					}
				}
				else
				{
					printf ("index : %d. Marker Word Data: 0x%08x \n", dataprintloop, 
						evdata[dataprintloop]);
					icol = 0;
				}
			}

		printf ("Do you want to run another test? (Press 0 for No, 1 for Yes)\n");
		cont = getdecd(cont);

	} /* End Main While Loop */
} /* End CSMTest

/******************/
int captureRaw(void)
/******************/

/*
 * Acquire raw data to a disk file. 
 */
{
	unsigned int gsaveLoc;
	int validDataInCycle;
	int buildID;
	int icol, irowIcol;
	unsigned int WordIDEct, maskflagct, tdcerrorct, NoMatchHeadError, NoHeadError;
	unsigned int NoMatchTrailError, EVIDerror;
	unsigned int CommPEct, tdcPEct;
	unsigned int inWords, outWords, overInWords;
	char bigPrint[20];
	int headerct, datact, trailerct;
	int dupHeadTrail;

	int CSMHeaderCnt, CSMTrailerCnt, CSMETrailerCnt, builtEvents, CSMErrorCnt;
	int cycleOK;
	unsigned int totaldatact, datacterror;
	int errReason1, errReason2, errReason3, errReason4;

	int numbuffersreq = 30, buffercol = 30, bufnr;
  
	unsigned int fsize;
	unsigned int *dataptr, data, fsizedata, fifodata, *evLocData; 
	unsigned int notready;
	unsigned int WordID;
	int TDCChIndex = -1, dataindex = 0, offset = 0, firstflag = 0, firstValidOffset = -1;
        unsigned int badCycleCount = 0;
	unsigned int emptyCycleCount = 0;
        unsigned int cycleBadAtEvent;
        int newlyBadCycle = 0;
        unsigned int fileSize = 0;
        int fileIsNull = 0;
	int jumpBack;
	int tstat;
	int jbm;

	char inChar[20];
	FILE *fout, *fIn;
	D_SRC_TYPE dSource;
	unsigned int inputBuffer[4096];
	int tabIndex, j;
	unsigned int eventCount, eventNotify;
	pid_t myPid;

	int dontCare;

	myPid = getpid();

	dSource = getDataSource( &fIn );

	
#ifndef WRITE_DISK_WHEN_READ_DISK
	if( dSource == D_SRC_FIBER )
	{
#endif
	    while( 1 )
	    {
	         printf( "Acquire data to disk file named: " ); fflush(stdout);
                 csmFileName[0] = '\0';
	         getstrd( csmFileName, csmFileName );
                 if( strcmp(csmFileName, "/dev/null") == 0 )
	         {
	            fileIsNull = 1;
                 }
                 else
	         {
	            if( (fout=fopen( csmFileName, "rb" )) == NULL )
	            {
	                 printf( "    Write to file named \"%s\"\n", csmFileName ); fflush(stdout);
	            }
	            else
	            {
		         fclose( fout );
		         printf( "File \"%s\" exists: over-write it? (Y/N): ", csmFileName ); fflush(stdout);
                         inChar[0] = getfstchar();
		         if( toupper(inChar[0]) == 'N' )
		              continue;
	            }
                 }

	         if( !fileIsNull && ((fout=fopen( csmFileName, "wb" )) == NULL) )
	         {
		      printf("--> Cannot open %s for writing!\n", csmFileName);
	         }
	         else
	         {
                      eventNotify = 1000;
		      if( !fileIsNull ) printf("    Opened file ""%s"" \n", csmFileName);
		      printf("\n At what event count interval do you want to be notified? " );
                      eventNotify = getdecd( eventNotify );
		      printf("\n *** From another shell, send a \"kill"
                          " -QUIT %d\" command to end acquisition\n"
			     "     or just type a ^C\n\n", myPid );
		      break;
	         }
	    }
#ifdef WRITE_DISK_WHEN_READ_DISK
	if( dSource == D_SRC_FIBER )
	{
#endif
	    tstat = daqEvInit( numbuffersreq, 0 );
	    bufnr = 0;

      /* Grab the data */
	    printf("Waiting for data...\n"); fflush(stdout);
	}
        else
	{
#ifndef WRITE_DISK_WHEN_READ_DISK
	    fileIsNull = 1;
#endif
	    bufnr = 0;
	    eventNotify = 100000;
	}

/********************************************************************************/
      /* Initialize acquire control variables */
	cont = 1;
	notready = 1;
	tabIndex = -1;
	eventCount = 0;

	inWords = 0;
	overInWords = 0;
	outWords = 0;

	validDataInCycle = 0;
	gsaveLoc = 0;

	// Do this for a maximum of 60 buffers
	while( cont )
	  //	for( jbm=0; jbm<60; ++jbm )
	{
	    jumpBack = sigsetjmp( env, 1 );
	    if( jumpBack )
	    {
		break;
	    }

            if( dSource == D_SRC_FIBER ) 
	    {
	  /* Wait until current buffer is full */
          /* Look at CH1 ACK_Available until data becomes available */

	        while( notready ) 
	        {
		    fifodata = filar->fifostat;
		    if (fifodata & 0x0000000f)
		    {
		        break;
		    }
	        }

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
	        dataptr = (unsigned int *) uaddr[1][bufnr];
	    }
	    else
	    {
		if( (fsize = fread( inputBuffer, sizeof(unsigned int), 4096, fIn )) == 0 )
		    break;    //EOF encountered, don't bother checking for error.
		dataptr = inputBuffer;
	    }

	    offset = 0;
	    incBigValue( &inWords, &overInWords, fsize );


	  /* OK, now write the buffer to disk */
            if( !fileIsNull )
	    {
		 //		   if( gsaveLoc )
		 //		   {
		     //		       fwrite( gsave, sizeof( unsigned int ), gsaveLoc, fout );
		     //		       fileSize += gsaveLoc*sizeof( unsigned int );
		     //		       outWords += gsaveLoc;
		       fwrite( dataptr, sizeof( unsigned int ), fsize, fout );
		       fileSize += fsize*sizeof( unsigned int );
		       outWords += fsize;
		       //		   }
		       //		   gsaveLoc = 0;
               if( fileSize >= 2000000000 )
	       {
	          printf( "\n\n-----> QUIT: file size exceeds 2GB, the file system limit! <-----\n" );
                  break;
               }
            }

	 /* Fill the next address into the req fifo for the next loop */
	    filar->req1 = paddr[1][bufnr];
         /* switch to the next buffer */
	    if( ++bufnr == numbuffersreq )
	    {
		bufnr = 0;
	    }
	}     // End of acquisition loop

/********************************************************************************/

      /* fclose(fout); */
        if( !fileIsNull )
	{
	   if (fclose(fout))
	       printf("\n\nCannot close %s \n", csmFileName);
	   else
	       printf("\n\nClosed file %s \n", csmFileName);
        }
        else
	{
	   printf( "\n\n" );
        }

        printBigValue( inWords, overInWords, bigPrint );
	printf( "/n     Incoming words on fiber: %s\n     Words saved to disk: %d\n",
		bigPrint, outWords );
} /* End csmAcquire */

/******************/
int csmAcquire(void)
/******************/

/*
 * Acquire data to a disk file. Lots of tests, and try to build events
 * in the case of TDM.  Also, completely empty data cycles are suppressed
 * from output to disk for both TDM and EB.  EB tests are minimal.
 */
{
        unsigned int gsave[65560];
	unsigned int gsaveLoc;
	int validDataInCycle;
	unsigned int TDCHead[3][18], TDCTrail[3][18], TDCData[3][18], TDCErr[3][18];
	unsigned int TDCMark[3][18], TDCOther[3][18];
	unsigned int dupHead[18], dupTrail[18];
	int buildID;
	int icol, irowIcol;
	unsigned int WordIDEct, maskflagct, tdcerrorct, NoMatchHeadError, NoHeadError;
	unsigned int NoMatchTrailError, EVIDerror;
	unsigned int CommPEct, tdcPEct;
	unsigned int inWords, outWords, overInWords;
	char bigPrint[20];
	int headerct, datact, trailerct;
	int dupHeadTrail;

	int CSMHeaderCnt, CSMTrailerCnt, CSMETrailerCnt, builtEvents, CSMErrorCnt;
	int cycleOK;
	unsigned int totaldatact, datacterror;
	int errReason1, errReason2, errReason3, errReason4;

	int numbuffersreq = 30, buffercol = 30, bufnr;
  
	unsigned int fsize;
	unsigned int *dataptr, data, fsizedata, fifodata, *evLocData; 
	unsigned int notready;
	unsigned int WordID;
	int TDCChIndex = -1, dataindex = 0, offset = 0, firstflag = 0, firstValidOffset = -1;
        unsigned int badCycleCount = 0;
	unsigned int emptyCycleCount = 0;
        unsigned int cycleBadAtEvent;
        int newlyBadCycle = 0;
        unsigned int fileSize = 0;
        int fileIsNull = 0;
	int jumpBack;
	int tstat;

	char inChar[20];
	FILE *fout, *fIn;
	D_SRC_TYPE dSource;
	unsigned int inputBuffer[4096];
	int tabIndex, j;
	unsigned int eventCount, eventNotify;
	pid_t myPid;

	int dontCare;

	myPid = getpid();

	WordIDEct = 0;
	maskflagct = 0;
	tdcerrorct = 0;
	headerct = 0;
	datact = 0;
	trailerct = 0;
	NoMatchHeadError = 0;
	NoHeadError = 0;
	NoMatchTrailError = 0;
	EVIDerror = 0;
	irowIcol = 3*18*sizeof(unsigned int);
	memset( TDCHead, 0, irowIcol );
	memset( TDCData, 0, irowIcol );
	memset( TDCTrail, 0, irowIcol );
	memset( TDCErr, 0, irowIcol );
	memset( TDCMark, 0, irowIcol );
	memset( TDCOther, 0, irowIcol );
	memset( noHead,  0, 18*sizeof(unsigned int) );
	memset( noTrail, 0, 18*sizeof(unsigned int) );
	memset( dupHead,  0, 18*sizeof(unsigned int) );
	memset( dupTrail, 0, 18*sizeof(unsigned int) );

	dSource = getDataSource( &fIn );

	if( flowType )
	{
	     CommPEct = 0;
	     tdcPEct = 0;
	     dupHeadTrail = 0;

	     newSeqNo = 0;
	     incompleteBuild = 0;
	     goodBuild = 0;
	     forcedFlush = 0;
	     activeTriggers = 0;
	     for( j=0; j<BUILD_SIZE; ++j )
	     {
	         seqNo[j] = -1;
		 activeEvid[j] = -1;
	     }

	     activeChanCount = 0;
	     printf( "     Specify valid data channels for built event checking\n" );
	     for( j=0; j<18; ++j )
	     {
	         printf( "Is channel %d enabled (1=yes)? ", j );
	         buildChannels[j] = getdecd( buildChannels[j] );
		 if( buildChannels[j] ) activeChanCount++;
	     }
	     printf( "\n" );
	}
	else
	{
	     CSMHeaderCnt = 0;
	     CSMTrailerCnt = 0;
	     CSMETrailerCnt = 0;
	     builtEvents = 0;
	     CSMErrorCnt = 0;
	     totaldatact = 0;
	     datacterror = 0;
	     cycleOK = 0;
	     errReason1 = 0;
	     errReason2 = 0;
	     errReason3 = 0;
	     errReason4 = 0;
	}
	
#ifndef WRITE_DISK_WHEN_READ_DISK
	if( dSource == D_SRC_FIBER )
	{
#endif
	    while( 1 )
	    {
	         printf( "Acquire data to disk file named: " ); fflush(stdout);
                 csmFileName[0] = '\0';
	         getstrd( csmFileName, csmFileName );
                 if( strcmp(csmFileName, "/dev/null") == 0 )
	         {
	            fileIsNull = 1;
                 }
                 else
	         {
	            if( (fout=fopen( csmFileName, "rb" )) == NULL )
	            {
	                 printf( "    Write to file named \"%s\"\n", csmFileName ); fflush(stdout);
	            }
	            else
	            {
		         fclose( fout );
		         printf( "File \"%s\" exists: over-write it? (Y/N): ", csmFileName ); fflush(stdout);
                         inChar[0] = getfstchar();
		         if( toupper(inChar[0]) == 'N' )
		              continue;
	            }
                 }

	         if( !fileIsNull && ((fout=fopen( csmFileName, "wb" )) == NULL) )
	         {
		      printf("--> Cannot open %s for writing!\n", csmFileName);
	         }
	         else
	         {
                      eventNotify = 1000;
		      if( !fileIsNull ) printf("    Opened file ""%s"" \n", csmFileName);
		      printf("\n At what event count interval do you want to be notified? " );
                      eventNotify = getdecd( eventNotify );
		      printf("\n *** From another shell, send a \"kill"
                          " -QUIT %d\" command to end acquisition\n"
			     "     or just type a ^C\n\n", myPid );
		      break;
	         }
	    }
#ifdef WRITE_DISK_WHEN_READ_DISK
	if( dSource == D_SRC_FIBER )
	{
#endif
	    tstat = daqEvInit( numbuffersreq, 0 );
	    bufnr = 0;

      /* Grab the data */
	    printf("Waiting for data...\n"); fflush(stdout);
	}
        else
	{
#ifndef WRITE_DISK_WHEN_READ_DISK
	    fileIsNull = 1;
#endif
	    bufnr = 0;
	    eventNotify = 100000;
	}

/********************************************************************************/
      /* Initialize acquire control variables */
	cont = 1;
	notready = 1;
	tabIndex = -1;
	eventCount = 0;

	inWords = 0;
	overInWords = 0;
	outWords = 0;

	validDataInCycle = 0;
	gsaveLoc = 0;

	while( cont )
	{
	    jumpBack = sigsetjmp( env, 1 );
	    if( jumpBack )
	    {
		break;
	    }

            if( dSource == D_SRC_FIBER ) 
	    {
	  /* Wait until current buffer is full */
          /* Look at CH1 ACK_Available until data becomes available */

	        while( notready ) 
	        {
		    fifodata = filar->fifostat;
		    if (fifodata & 0x0000000f)
		    {
		        break;
		    }
	        }

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
	        dataptr = (unsigned int *) uaddr[1][bufnr];
	    }
	    else
	    {
		if( (fsize = fread( inputBuffer, sizeof(unsigned int), 4096, fIn )) == 0 )
		    break;    //EOF encountered, don't bother checking for error.
		dataptr = inputBuffer;
	    }

	    offset = 0;
	    incBigValue( &inWords, &overInWords, fsize );

            if( flowType )   // TDM data
	    {
		if( !firstflag )  // sync up.  Find an active channel
		{
		    TDCChIndex = -1;
		    for( j=0; j<fsize; ++j )
		    {
			if( (*(dataptr+j) & 0xf0000000) == 0xd0000000 )
			{
			    offset = j;
                            firstValidOffset = j;
			    TDCChIndex = 18;
			    break;
			}
		    }
		    if( firstValidOffset != -1 )
			 firstflag = 1;
		}
		if( firstflag )
		{
		    for( dataindex=offset; dataindex<fsize; ++dataindex )
		    {
		        evLocData = dataptr+dataindex;
			WordID = *evLocData & 0xf0000000;

		/* Check for TDC Parity error */
			if( (*evLocData & 0x08000000) != 0 )
			     ++tdcPEct;

			switch( WordID )
			{
			case 0x00000000:
			     if( *evLocData != 0x04000000 ) CommPEct++;
			     ++TDCChIndex;
			     break;
			case 0x20000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     ++maskflagct;
			     if( TDCChIndex < 18 ) ++TDCMark[CNT][TDCChIndex];
			     ++TDCChIndex;
			     validDataInCycle = 1;
			     break;
			case 0x30000000:
			case 0x40000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     if( TDCChIndex < 18 ) 
			     {
			         ++TDCData[CNT][TDCChIndex];
			         if( !TDCHead[FLAG][TDCChIndex] ) ++NoHeadError;
			     }
			     ++TDCChIndex;
			     validDataInCycle = 1;
			     break;
			case 0x60000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     tdcerrorct++;
			     if( TDCChIndex < 18 ) ++TDCErr[CNT][TDCChIndex];
			     ++TDCChIndex;
			     validDataInCycle = 1;
			     break;
			case 0x70000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     if( TDCChIndex < 18 ) ++TDCOther[CNT][TDCChIndex];
			     ++TDCChIndex;
			     validDataInCycle = 1;
			     break;
			case 0xa0000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     if( TDCChIndex < 18 ) 
			     {
			         TDCTrail[DATA][TDCChIndex] = 0;
			         ++TDCHead[CNT][TDCChIndex];
			         if (TDCHead[FLAG][TDCChIndex]) ++NoMatchTrailError;
			         TDCHead[FLAG][TDCChIndex] = 1;
			         TDCHead[DATA][TDCChIndex] = *evLocData;
			         if( buildChannels[TDCChIndex] == 1 )
			         {
//			             printf( "Header, chan=%d data=0x%8x ", TDCChIndex, *evLocData );
			             buildID = whichId( *evLocData, 0 );
			             checkFlush( buildID, TDCChIndex );
			             if( haveHeader[buildID][TDCChIndex] == 1 ) 
			             {
			                 dupHead[TDCChIndex]++;
			                 dupHeadTrail = 1;
			             }
			             haveHeader[buildID][TDCChIndex] = 1;
			         }
			     }
			     ++TDCChIndex; 
			     if( tabIndex<0 && TDCChIndex<18 )
			     {
				 tabIndex = TDCChIndex;
			     }
			     if( tabIndex == TDCChIndex )
			     {
				 eventCount++;
				 if( eventCount%eventNotify == 0 )
				 {
				     printf( "\rAcquire event %10d", eventCount );
                                     if( newlyBadCycle )
				     {
				        printf(" -- Bad cycle on event %10d", cycleBadAtEvent );
                                        newlyBadCycle = 0;
                                     }
				     fflush(stdout);
				 }
			     }
			     validDataInCycle = 1;
			     break;
			case 0xc0000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     if( TDCChIndex < 18 )
			     {
			         TDCTrail[DATA][TDCChIndex] = *evLocData;
			         TDCTrail[CNT][TDCChIndex]++;
			         if (!TDCHead[FLAG][TDCChIndex]) ++NoMatchHeadError;
			         TDCHead[FLAG][TDCChIndex] = 0;
			         if (TDCHead[DATA][TDCChIndex] && (TDCHead[DATA][TDCChIndex] & 0x00fff000) != (TDCTrail[DATA][TDCChIndex] & 0x00fff000)) ++EVIDerror;
			         if( buildChannels[TDCChIndex] == 1 )
			         {
//			             printf( "Trailer, chan=%d data=0x%8x ", TDCChIndex, *evLocData );
			             buildID = whichId( *evLocData, 0 );
			             checkFlush( buildID, TDCChIndex );
			             if( haveTrailer[buildID][TDCChIndex] == 1 ) 
			             {
			                 dupTrail[TDCChIndex]++;
			                 dupHeadTrail = 1;
			             }
			             else
			             {
			                 haveTrailer[buildID][TDCChIndex] = 1;
				         if( haveHeader[buildID][TDCChIndex] == 1 ) matchedCount[buildID]++;
				         if( matchedCount[buildID] == activeChanCount )
				         {
				             if( eFlush[buildID] )
				               incompleteBuild++;
				             else
				               goodBuild++;
				             activeEvid[buildID] = -1;
				         }
			             }
			         }
			         TDCHead[DATA][TDCChIndex] = 0;
			     }
			     ++TDCChIndex;
			     validDataInCycle = 1;
			     break;
			case 0xd0000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
//			     if( *evLocData != 0xd0000000 ) CommPEct++;
			     if( TDCChIndex != 18 ) 
                             {
                                badCycleCount++;
                                cycleBadAtEvent = eventCount;
                                newlyBadCycle = 1;
#ifndef KEEP_BAD_DATA_CYCLES
			        validDataInCycle = 0;
#endif
                             }
			     else if( validDataInCycle == 0 )
			     {
				 ++emptyCycleCount;
			     }
			     if( validDataInCycle )
			     {
			         gsaveLoc += (TDCChIndex+1);
				 validDataInCycle = 0;
			     }
			     TDCChIndex = 0;
			     break;
			default:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     if( TDCChIndex < 18 ) ++TDCOther[CNT][TDCChIndex];
			     ++WordIDEct;
			     ++TDCChIndex;
			     validDataInCycle = 1;
			     break;
			}   // End of switch.....

			gsave[gsaveLoc+TDCChIndex] = *evLocData;
		    }
		}   // end data check for TDM-style data
	    }       // end TDM-style data

	    else    // begin Event-Built (EB-style) data
	    {
		for( dataindex=offset; dataindex<fsize; ++dataindex )
		{
		    evLocData = dataptr+dataindex;
		    if( !cycleOK )
		    {
		        WordID = *evLocData & 0xff000000;
		        switch( WordID )
		        {
		        case 0x59000000:
		             if( firstValidOffset == -1 )
			        firstValidOffset = dataindex;
			     cycleOK = 1;
			     eventCount++;
			     if( eventCount%eventNotify == 0 )
			     {
			         printf( "\rAcquire event %10d", eventCount );
			         fflush(stdout);
			     }
			     break;
		        default:
			     break;
		        }
		    }

		    if( cycleOK )
		    {
			 WordID = *evLocData & 0xF0000000;
		      // compute tdcno from encoded values
			 TDCChIndex = (*evLocData)>>24 & 0xF;
			 switch(WordID)
			 {
			 case 0x00000000:
			      break;
			 case 0x20000000:
			      ++maskflagct;
			      ++TDCMark[CNT][TDCChIndex];
			      ++totaldatact;
			      break;
			 case 0x30000000:
			 case 0x40000000:
			   // re-compute tdcno from encoded values if needed
			      if( (*evLocData & 0x00c00000) == 0x00c00000 )
			      {
				  TDCChIndex = 16 + (((*evLocData)>>26) & 0x1);
			      }
			      ++TDCData[CNT][TDCChIndex];
			      ++totaldatact;
			      break;
			 case 0x60000000:
			      tdcerrorct++;
			      ++TDCErr[CNT][TDCChIndex];
			      ++totaldatact;
			      break;
			 case 0x70000000:
			      ++TDCOther[CNT][TDCChIndex];
			      ++totaldatact;
			      break;
			 case 0xa0000000:
			      TDCTrail[DATA][TDCChIndex] = 0;
			      ++TDCHead[CNT][TDCChIndex];
			      if (TDCHead[FLAG][TDCChIndex]) ++NoMatchTrailError;
			      TDCHead[FLAG][TDCChIndex] = 1;
			      TDCHead[DATA][TDCChIndex] = *evLocData;
			      ++totaldatact;
			      break;
			 case 0xc0000000:
			      TDCTrail[DATA][TDCChIndex] = *evLocData;
			      TDCTrail[CNT][TDCChIndex]++;
			      if (!TDCHead[FLAG][TDCChIndex]) ++NoMatchHeadError;
			      TDCHead[FLAG][TDCChIndex] = 0;
//			      if (TDCTrail[DATA][TDCChIndex] & 0x00000fff != totaldatact) ++datacterror;
			      if (TDCHead[DATA][TDCChIndex] && (TDCHead[DATA][TDCChIndex] & 0x00fff000) != (TDCTrail[DATA][TDCChIndex] & 0x00fff000)) ++EVIDerror;
			      TDCHead[DATA][TDCChIndex] = 0;
			      ++totaldatact;
			      break;
			 case 0x50000000:
			      WordID = *evLocData & 0x0F000000;
			      if( WordID == 0x09000000 )
			      {
				  CSMHeaderCnt++;
				  totaldatact = 1;
			      }
			      else if( WordID == 0x0 ) /* CSM Error word */
			      {
				  CSMErrorCnt++;
				  if( *evLocData & 0x00800000 ) errReason1++;
				  if( *evLocData & 0x00400000 ) errReason2++;
				  if( *evLocData & 0x00200000 ) errReason3++;
				  if( *evLocData & 0x00100000 ) errReason4++;
				  ++totaldatact;
			      }
			      else if( WordID == 0x0d000000 )  /* Error trailer */
			      {
				  CSMETrailerCnt++;
				  builtEvents++;
				  cycleOK = 0;
				  ++totaldatact;
				  if( (*evLocData & 0x00000FFF) != totaldatact ) ++datacterror;
				  totaldatact = 0;
			      }
			      else if( WordID == 0x0b000000 )  /* Normal trailer */
			      {
				  CSMTrailerCnt++;
				  builtEvents++;
				  cycleOK = 0;
				  ++totaldatact;
				  if( (*evLocData & 0x00000FFF) != totaldatact ) ++datacterror;
				  totaldatact = 0;
			      }
			      break;
			 default:
			      ++TDCOther[CNT][TDCChIndex];
			      ++WordIDEct;
			      ++totaldatact;
			      break;
			 } /* end switch */
		    }
		}
	    }

	  /* OK, now write the buffer to disk */
            if( !fileIsNull )
	    {
	       if( flowType )
	       {
		   if( gsaveLoc )
		   {
		       fwrite( gsave, sizeof( unsigned int ), gsaveLoc, fout );
		       fileSize += gsaveLoc*sizeof( unsigned int );
		       outWords += gsaveLoc;
		       if( TDCChIndex > 0 )
		       {
		           for( j=0; j<(TDCChIndex+1); ++j )
		           {
		               gsave[j] = gsave[gsaveLoc+j];
		           }
		       }
		   }
		   gsaveLoc = 0;
	       }
	       else
	       {
                   fwrite( dataptr, sizeof(unsigned int), fsize, fout );
                   fileSize += fsize*sizeof(unsigned int);
		   outWords += fsize;
	       }
               if( fileSize >= 2000000000 )
	       {
	          printf( "\n\n-----> QUIT: file size exceeds 2GB, the file system limit! <-----\n" );
                  break;
               }
            }
	    else if( flowType )
	    {
	       gsaveLoc = 0;
	    }

	 /* Fill the next address into the req fifo for the next loop */
	    filar->req1 = paddr[1][bufnr];
         /* switch to the next buffer */
	    if( ++bufnr == numbuffersreq )
	    {
		bufnr = 0;
	    }
	}     // End of acquisition loop

/********************************************************************************/

      /* fclose(fout); */
        if( !fileIsNull )
	{
	   if( flowType )
	   {
#ifdef KEEP_BAD_DATA_CYCLES
	       if( TDCChIndex!=0 && validDataInCycle && gsaveLoc==0 )
#else
	       if( TDCChIndex==18 && validDataInCycle && gsaveLoc==0 )
#endif
	       {
		   fwrite( gsave, sizeof( unsigned int ), (TDCChIndex+1), fout );
		   fileSize += (TDCChIndex+1)*sizeof( unsigned int );
		   outWords += (TDCChIndex+1);
	       }
	   }
	   if (fclose(fout))
	       printf("\n\nCannot close %s \n", csmFileName);
	   else
	       printf("\n\nClosed file %s \n", csmFileName);
        }
        else
	{
	   printf( "\n\n" );
        }

        printf( " First valid data was at offset %d\n", firstValidOffset );
        if( flowType )
	{
	    irowIcol = 0;
	    for( j=0; j<BUILD_SIZE; ++j )
	    {
	        if( activeEvid[j] != -1 ) 
		{
		    incompleteBuild++;
		    irowIcol++;
		}
	    }
	    printf("Channel E    Header     Data  Trailer    Error     Mark    Other  NoHead NoTrail\n");
	    for (icol = 0; icol < 18; ++icol)
	    {
	        if( buildChannels[icol] == 1 )
		{
		    printf("TDC[%2d] * %8d %8d %8d %8d %8d %8d %7d %7d\n", icol, TDCHead[CNT][icol], 
			   TDCData[CNT][icol], TDCTrail[CNT][icol], TDCErr[CNT][icol], 
			   TDCMark[CNT][icol], TDCOther[CNT][icol], noHead[icol], noTrail[icol] );
		}
		else
		{
		    printf("TDC[%2d]   %8d %8d %8d %8d %8d %8d %7d %7d\n", icol, TDCHead[CNT][icol], 
			   TDCData[CNT][icol], TDCTrail[CNT][icol], TDCErr[CNT][icol], 
			   TDCMark[CNT][icol], TDCOther[CNT][icol], noHead[icol], noTrail[icol] );
		}
		headerct = headerct + TDCHead[CNT][icol];
		datact = datact + TDCData[CNT][icol];
		trailerct = trailerct + TDCTrail[CNT][icol];
	    }

	    if( dupHeadTrail )
	    {
	        printf( " Channel    DupHead  DupTrail\n" );
		for( icol=0; icol<18; ++icol )
		{
		    printf( " TDC[%2d] %8d %8d\n", icol, dupHead[icol], dupTrail[icol] );
		}
	    }

	    printf( "\nData statistics\n" );
	    printf( "headerct, datact, trailerct, WordIDEct, tdcPEct, CommPEct\n" );
	    printf( "%5d %8d  %8d  %8d %8d %8d\n", headerct, datact, trailerct, 
		        WordIDEct, tdcPEct, CommPEct );
	    printf( "NoMatchHeadEr, NoMatchTrailEr, NoHeadEr, EVIDer\n" );
	    printf( "%10d %15d  %8d %8d\n", NoMatchHeadError, NoMatchTrailError, 
			NoHeadError, EVIDerror );
	    printf( " There were %d bad data cycles (flag word interval != 19)\n", badCycleCount );
	    printf( " %d empty data cycles were suppressed from output\n", emptyCycleCount-1 );
	    printf( " %d Events were completely built\n %d Events were incomplete\n", goodBuild,
		    incompleteBuild );
	    printf( "      %d of these incompletes were pending at run's end\n", irowIcol );
	    printf( " %d Events were forcibly flushed\n", forcedFlush );
	    printf( " The largest number of simultaneously building triggers was %d\n",
		    activeTriggers+1 );
        }
	else
	{
	    printf("Channel    Header     Data  Trailer    Error     Mark    Other\n");
	    for (icol = 0; icol < 18; ++icol)
	    {
	        printf("TDC[%2d] %8d %8d %8d %8d %8d %8d\n", icol, TDCHead[CNT][icol], 
		       TDCData[CNT][icol], TDCTrail[CNT][icol], TDCErr[CNT][icol], 
		       TDCMark[CNT][icol], TDCOther[CNT][icol] );
		headerct = headerct + TDCHead[CNT][icol];
		datact = datact + TDCData[CNT][icol];
		trailerct = trailerct + TDCTrail[CNT][icol];
	    }

	    printf( "\nData statistics\n" );
	    printf( "headerct, datact, trailerct, WordIDEct\n" );
	    printf( "%5d %8d  %8d  %8d\n", headerct, datact, trailerct, 
		        WordIDEct );
	    printf( "datactEr, NoMatchHeadEr, NoMatchTrailEr, NoHeadEr, EVIDer\n" );
	    printf( "%9d %10d %15d  %8d %8d\n", datacterror, NoMatchHeadError, NoMatchTrailError, 
			NoHeadError, EVIDerror );
	    printf( "CSM HeaderCnt, TrailerCnt, ETrailerCnt, ErrorCnt, builtEvents\n" );
	    printf( "    %9d   %9d    %9d  %8d    %9d\n", CSMHeaderCnt, CSMTrailerCnt, CSMETrailerCnt,
		     CSMErrorCnt, builtEvents );
	    if( CSMErrorCnt )
	    {
		printf( "CSM Error summary:\n  Missing trailers %d, Missing headers %d"
			", RAM full %d, data wait timeout %d\n", errReason1, errReason2, errReason3,
			errReason4 );
	    }
	}
	printf( " %d Events acquired to disk during this run\n\n", eventCount );
        printBigValue( inWords, overInWords, bigPrint );
	printf( "     Incoming words on fiber: %s\n     Words saved to disk: %d\n",
		bigPrint, outWords );
} /* End csmAcquire */

/************************/
int captureOneEvent(void)
/************************/

/*
 * Acquire data to a disk file, but only for the duration of one Event.
 * EB for now just the same as csmAcquire.  
 * Completely empty data cycles are suppressed, but counted for debug
 * purposes.
 */

{
        unsigned int gsave[4096];
	unsigned int buildBuffer[4096];
	unsigned int TDCHead[3][18], TDCTrail[3][18], TDCData[3][18], TDCErr[3][18];
	unsigned int TDCMark[3][18], TDCOther[3][18];
	int icol, irowIcol;
	unsigned int WordIDEct, maskflagct, tdcerrorct, NoMatchHeadError, NoHeadError;
	unsigned int NoMatchTrailError, EVIDerror;
	unsigned int CommPEct, tdcPEct;
	int headerct, datact, trailerct;

	int CSMHeaderCnt, CSMTrailerCnt, CSMETrailerCnt, builtEvents, CSMErrorCnt;
	int cycleOK;
	unsigned int totaldatact, datacterror;
	int errReason1, errReason2, errReason3, errReason4;
	int evid, bcid;

	unsigned int *dataptr, data, *evLocData; 
	unsigned int WordID;
	int TDCChIndex = 0, dataindex = 0, offset = 0, firstflag = 0, firstValidOffset = -1;
        int fileIsNull = 0;

	char inChar[20];
	FILE *fout;
	int j;
	pid_t myPid;
	int fsize;
	int eventReady;

	oneEventInfo_T evInfo;

	int dontCare;

	myPid = getpid();

	WordIDEct = 0;
	maskflagct = 0;
	tdcerrorct = 0;
	headerct = 0;
	datact = 0;
	trailerct = 0;
	NoMatchHeadError = 0;
	NoHeadError = 0;
	NoMatchTrailError = 0;
	EVIDerror = 0;
	irowIcol = 3*18*sizeof(unsigned int);
	memset( TDCHead, 0, irowIcol );
	memset( TDCData, 0, irowIcol );
	memset( TDCTrail, 0, irowIcol );
	memset( TDCErr, 0, irowIcol );
	memset( TDCMark, 0, irowIcol );
	memset( TDCOther, 0, irowIcol );

	if( flowType )
	{
	     CommPEct = 0;
	     tdcPEct = 0;

	     printf( "     Specify valid data channels for built event checking\n" );
	     for( j=0; j<18; ++j )
	     {
	         printf( "Is channel %d enabled (1=yes)? ", j );
	         buildChannels[j] = getdecd( buildChannels[j] );
		 evInfo.maskArray[j] = buildChannels[j];
	     }
	     printf( "\n" );
	}
	else
	{
	     CSMHeaderCnt = 0;
	     CSMTrailerCnt = 0;
	     CSMETrailerCnt = 0;
	     CSMErrorCnt = 0;
	     totaldatact = 0;
	     datacterror = 0;
	     cycleOK = 0;
	     errReason1 = 0;
	     errReason2 = 0;
	     errReason3 = 0;
	     errReason4 = 0;
	}

	evInfo.checkSignals = 1;          // Watch for quit signal
	evInfo.dataBuffer = gsave;        // Acquired data buffer
	evInfo.bufLen = 4096;
	evInfo.builtEvent = buildBuffer;  // Built event buffer
	evInfo.eventLen = 4096;

	while( 1 )
	{
	     printf( "Acquire data to disk file named: " ); fflush(stdout);
             csmFileName[0] = '\0';
	     getstrd( csmFileName, csmFileName );
             if( strcmp(csmFileName, "/dev/null") == 0 )
	     {
	        fileIsNull = 1;
             }
             else
	     {
	        if( (fout=fopen( csmFileName, "rb" )) == NULL )
	        {
	             printf( "    Write to file named \"%s\"\n", csmFileName ); fflush(stdout);
	        }
	        else
	        {
		     fclose( fout );
		     printf( "File \"%s\" exists: over-write it? (Y/N): ", csmFileName ); fflush(stdout);
                     inChar[0] = getfstchar();
		     if( toupper(inChar[0]) == 'N' )
		          continue;
	        }
             }

	     if( !fileIsNull && ((fout=fopen( csmFileName, "wb" )) == NULL) )
	     {
		  printf("--> Cannot open %s for writing!\n", csmFileName);
	     }
	     else
	     {
		  if( !fileIsNull ) printf("    Opened file ""%s"" \n", csmFileName);
		  printf("\n *** From another shell, send a \"kill"
                      " -QUIT %d\" command to end acquisition\n\n", myPid );
		  break;
	     }
	}
	
/********************************************************************************/

	eventReady = captureEvent( flowType, &evInfo );
	fsize = evInfo.bufSize;
	if( eventReady )
	  {
	    printf( "Found event built with evid=%d of size %d\n\n", evInfo.evid, evInfo.bufSize );
	  }
	else
	  {
	    printf( "No built event found in captured buffers\n" );
	  }

	    dataptr = gsave;
	    offset = 0;
	    
            if( flowType )   // TDM data
	    {
		if( !firstflag )  // sync up.  Find an active channel
		{
		    TDCChIndex = -1;
		    for( j=0; j<fsize; ++j )
		    {
			if( (*(dataptr+j) & 0xf0000000) == 0xd0000000 )
			{
			    offset = j;
                            firstValidOffset = j;
			    TDCChIndex = 18;
			    break;
			}
		    }
		    if( firstValidOffset != -1 )
			 firstflag = 1;
		}
		if( firstflag )
		{
		    for( dataindex=offset; dataindex<fsize; ++dataindex )
		    {
		        evLocData = dataptr+dataindex;
			WordID = *evLocData & 0xf0000000;

		/* Check for TDC Parity error */
			if( (*evLocData & 0x08000000) != 0 )
			     ++tdcPEct;

			switch( WordID )
			{
			case 0x00000000:
			     if( *evLocData != 0x04000000 ) CommPEct++;
			     ++TDCChIndex;
			     break;
			case 0x20000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     ++maskflagct;
			     if( TDCChIndex < 18 ) ++TDCMark[CNT][TDCChIndex];
			     printf( "       Data word for channel %d, major ID = 0x%x\n", TDCChIndex, WordID );
			     ++TDCChIndex;
			     break;
			case 0x30000000:
			case 0x40000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     if( TDCChIndex < 18 )
			     {
			         ++TDCData[CNT][TDCChIndex];
			         if( !TDCHead[FLAG][TDCChIndex] ) ++NoHeadError;
			     }
			     printf( "       Data word for channel %d, major ID = 0x%x\n", TDCChIndex, WordID );
			     ++TDCChIndex;
			     break;
			case 0x60000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     tdcerrorct++;
			     if( TDCChIndex < 18 ) ++TDCErr[CNT][TDCChIndex];
			     printf( "       Data word for channel %d, major ID = 0x%x\n", TDCChIndex, WordID );
			     ++TDCChIndex;
			     break;
			case 0x70000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     if( TDCChIndex < 18 ) ++TDCOther[CNT][TDCChIndex];
			     printf( "       Data word for channel %d, major ID = 0x%x\n", TDCChIndex, WordID );
			     ++TDCChIndex;
			     break;
			case 0xa0000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     if( TDCChIndex < 18 )
			     {
			         TDCTrail[DATA][TDCChIndex] = 0;
			         ++TDCHead[CNT][TDCChIndex];
			         if (TDCHead[FLAG][TDCChIndex]) ++NoMatchTrailError;
			         TDCHead[FLAG][TDCChIndex] = 1;
			         TDCHead[DATA][TDCChIndex] = *evLocData;
			         if( buildChannels[TDCChIndex] == 1 )
			         {
			             printf( "Header, AMT %d data=0x%8x\n", TDCChIndex, *evLocData );
			         }
			     }
			     ++TDCChIndex;
			     break;
			case 0xc0000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     if( TDCChIndex < 18 )
			     {
			         TDCTrail[DATA][TDCChIndex] = *evLocData;
			         TDCTrail[CNT][TDCChIndex]++;
			         if (!TDCHead[FLAG][TDCChIndex]) ++NoMatchHeadError;
			         TDCHead[FLAG][TDCChIndex] = 0;
			         if (TDCHead[DATA][TDCChIndex] && (TDCHead[DATA][TDCChIndex] & 0x00fff000) != (TDCTrail[DATA][TDCChIndex] & 0x00fff000)) ++EVIDerror;
			         if( buildChannels[TDCChIndex] == 1 )
			         {
			             printf( "Trailer, AMT %d data=0x%8x\n", TDCChIndex, *evLocData );
			         }
			         TDCHead[DATA][TDCChIndex] = 0;
			     }
			     ++TDCChIndex;
			     break;
			case 0xd0000000:
			     dontCare = checkParity( *evLocData, &CommPEct );
//			     if( *evLocData != 0xd0000000 ) CommPEct++;
			     TDCChIndex = 0;
			     break;
			default:
			     dontCare = checkParity( *evLocData, &CommPEct );
			     if( TDCChIndex < 18 ) ++TDCOther[CNT][TDCChIndex];
			     ++WordIDEct;
			     printf( "       Data word for channel %d, data = 0x%8.8x\n", TDCChIndex, *evLocData ); // if( cont ) 
			     ++TDCChIndex;
			     break;
			}   // End of switch.....
		    }
		}   // end data check for TDM-style data
	    }       // end TDM-style data

	    else    // begin Event-Built (EB-style) data
	    {
		for( dataindex=offset; dataindex<fsize; ++dataindex )
		{
		    evLocData = dataptr+dataindex;
		    if( !cycleOK )
		    {
		        WordID = *evLocData & 0xff000000;
		        switch( WordID )
		        {
		        case 0x59000000:
			     cycleOK = 1;
			     evid = (*evLocData)>>12 & 0x00000FFF;
			     bcid = *evLocData & 0x00000FFF;
			     printf( "CSM Header at data offset %d with evid %d and bcid %d\n",
				 dataindex, evid, bcid );
			     break;
		        default:
			     break;
		        }
		    }

		    if( cycleOK )
		    {
			 WordID = *evLocData & 0xF0000000;
		      // compute tdcno from encoded values
			 TDCChIndex = (*evLocData)>>24 & 0xF;
			 switch(WordID)
			 {
			 case 0x00000000:
			      break;
			 case 0x20000000:
			      printf( "       Data word for channel %d, major ID = 0x%x\n", TDCChIndex, WordID );
			      ++maskflagct;
			      ++TDCMark[CNT][TDCChIndex];
			      ++totaldatact;
			      break;
			 case 0x30000000:
			 case 0x40000000:
			   // re-compute tdcno from encoded values if needed
			      if( (*evLocData & 0x00c00000) == 0x00c00000 )
			      {
				  TDCChIndex = 16 + (((*evLocData)>>26) & 0x1);
			      }
			      printf( "       Data word for channel %d, major ID = 0x%x\n", TDCChIndex, WordID );
			      ++TDCData[CNT][TDCChIndex];
			      ++totaldatact;
			      break;
			 case 0x60000000:
			      printf( "       Data word for channel %d, major ID = 0x%x\n", TDCChIndex, WordID );
			      tdcerrorct++;
			      ++TDCErr[CNT][TDCChIndex];
			      ++totaldatact;
			      break;
			 case 0x70000000:
			      printf( "       Data word for channel %d, major ID = 0x%x\n", TDCChIndex, WordID );
			      ++TDCOther[CNT][TDCChIndex];
			      ++totaldatact;
			      break;
			 case 0xa0000000:
			      evid = (*evLocData)>>12 & 0x00000FFF;
			      bcid = *evLocData & 0x00000FFF;
			      printf( "   Header, chan=%d data=0x%8x evid=%d bcid=%d\n", 
				      TDCChIndex, *evLocData, evid, bcid );
			      TDCTrail[DATA][TDCChIndex] = 0;
			      ++TDCHead[CNT][TDCChIndex];
			      if (TDCHead[FLAG][TDCChIndex]) ++NoMatchTrailError;
			      TDCHead[FLAG][TDCChIndex] = 1;
			      TDCHead[DATA][TDCChIndex] = *evLocData;
			      ++totaldatact;
			      break;
			 case 0xc0000000:
			      evid = (*evLocData)>>12 & 0x00000FFF;
			      bcid = *evLocData & 0x00000FFF;
			      printf( "   Trailer, chan=%d data=0x%8x evid=%d, wordCount=%d\n",
				      TDCChIndex, *evLocData, evid, bcid );
			      TDCTrail[DATA][TDCChIndex] = *evLocData;
			      TDCTrail[CNT][TDCChIndex]++;
			      if (!TDCHead[FLAG][TDCChIndex]) ++NoMatchHeadError;
			      TDCHead[FLAG][TDCChIndex] = 0;
//			      if (TDCTrail[DATA][TDCChIndex] & 0x00000fff != totaldatact) ++datacterror;
			      if (TDCHead[DATA][TDCChIndex] && (TDCHead[DATA][TDCChIndex] & 0x00fff000) != (TDCTrail[DATA][TDCChIndex] & 0x00fff000)) ++EVIDerror;
			      TDCHead[DATA][TDCChIndex] = 0;
			      ++totaldatact;
			      break;
			 case 0x50000000:
			      WordID = *evLocData & 0x0F000000;
			      if( WordID == 0x09000000 )
			      {
				  CSMHeaderCnt++;
				  totaldatact = 1;
			      }
			      else if( WordID == 0x0 ) /* CSM Error word */
			      {
			          evid = *evLocData & 0x0003FFFF;
				  printf( "CSM error word with 18 indicators 0x%5.5x\n", evid );
				  CSMErrorCnt++;
				  if( *evLocData & 0x00800000 ) errReason1++;
				  if( *evLocData & 0x00400000 ) errReason2++;
				  if( *evLocData & 0x00200000 ) errReason3++;
				  if( *evLocData & 0x00100000 ) errReason4++;
				  ++totaldatact;
			      }
			      else if( WordID == 0x0d000000 )  /* Error trailer */
			      {
			          evid = (*evLocData)>>12 & 0x00000FFF;
				  bcid = *evLocData & 0x00000FFF;
				  printf( "CSM error Trailer with evid %d and word count %d\n",
					  evid, bcid );
				  CSMETrailerCnt++;
				  cycleOK = 0;
				  ++totaldatact;
				  if( (*evLocData & 0x00000FFF) != totaldatact ) ++datacterror;
				  totaldatact = 0;
			      }
			      else if( WordID == 0x0b000000 )  /* Normal trailer */
			      {
			          evid = (*evLocData)>>12 & 0x00000FFF;
				  bcid = *evLocData & 0x00000FFF;
				  printf( "CSM Trailer with evid %d and word count %d\n", 
					  evid, bcid );
				  CSMTrailerCnt++;
				  cycleOK = 0;
				  ++totaldatact;
				  if( (*evLocData & 0x00000FFF) != totaldatact ) ++datacterror;
				  totaldatact = 0;
			      }
			      break;
			 default:
			      printf( "       Data word for channel %d, data = 0x%x\n", TDCChIndex, *evLocData );
			      ++TDCOther[CNT][TDCChIndex];
			      ++WordIDEct;
			      ++totaldatact;
			      break;
			 } /* end switch */
		    }
		}
	    }

	  /* OK, now write the buffer to disk */
            if( !fileIsNull && evInfo.bufSize )
	    {
	        fwrite( gsave, sizeof( unsigned int ), evInfo.bufSize, fout );
            }

/********************************************************************************/

      /* fclose(fout); */
        if( !fileIsNull )
	{
	   if( fclose(fout) )
	       printf("\n\nCannot close %s \n", csmFileName);
	   else
	       printf("\n\nClosed file %s \n", csmFileName);
        }
        else
	{
	   printf( "\n\n" );
        }

	printf( "The event built from this data contains %d words, which are", 
		evInfo.eventSize );
	for( icol=0; icol<evInfo.eventSize; ++icol )
	{
	    if( icol%8 == 0 ) printf( "\n" );
	    printf( "%8.8x ", buildBuffer[icol] );
	}
	if( (evInfo.eventSize)%8 != 0 ) printf( "\n" );

        if( flowType )
	{
	    printf("\n %d Empty data cycles were skipped\n\n", evInfo.emptyCycles );
	    printf(" Channel E    Header     Data  Trailer    Error     Mark   Other  NoHead NoTrail\n");
	    for (icol = 0; icol < 18; ++icol)
	    {
	        if( buildChannels[icol] == 1 )
		{
		    printf(" TDC[%2d] * %8d %8d %8d %8d %8d %7d %7d %7d\n", icol, TDCHead[CNT][icol], 
			   TDCData[CNT][icol], TDCTrail[CNT][icol], TDCErr[CNT][icol], 
			   TDCMark[CNT][icol], TDCOther[CNT][icol], noHead[icol], noTrail[icol] );
		}
		else
		{
		    printf(" TDC[%2d]   %8d %8d %8d %8d %8d %7d %7d %7d\n", icol, TDCHead[CNT][icol], 
			   TDCData[CNT][icol], TDCTrail[CNT][icol], TDCErr[CNT][icol], 
			   TDCMark[CNT][icol], TDCOther[CNT][icol], noHead[icol], noTrail[icol] );
		}
		headerct = headerct + TDCHead[CNT][icol];
		datact = datact + TDCData[CNT][icol];
		trailerct = trailerct + TDCTrail[CNT][icol];
	    }

	    printf( "\nData statistics\n" );
	    printf( "headerct, datact, trailerct, WordIDEct, tdcPEct, CommPEct\n" );
	    printf( "%5d %8d  %8d  %8d %8d %8d\n", headerct, datact, trailerct, 
		        WordIDEct, tdcPEct, CommPEct );
	    printf( "NoMatchHeadEr, NoMatchTrailEr, NoHeadEr, EVIDer\n" );
	    printf( "%10d %15d  %8d %8d\n", NoMatchHeadError, NoMatchTrailError, 
			NoHeadError, EVIDerror );
	    printf( " There were %d bad data cycles (flag word interval != 19)\n", 
		    evInfo.badCycleCount );
	    printf( " %d Events were forcibly flushed\n", forcedFlush );
        }
	else
	{
	    printf("Channel    Header     Data  Trailer    Error     Mark    Other\n");
	    for (icol = 0; icol < 18; ++icol)
	    {
		printf("TDC[%2d] %8d %8d %8d %8d %8d %8d\n", icol, TDCHead[CNT][icol], 
		       TDCData[CNT][icol], TDCTrail[CNT][icol], TDCErr[CNT][icol], 
		       TDCMark[CNT][icol], TDCOther[CNT][icol] );
		headerct = headerct + TDCHead[CNT][icol];
		datact = datact + TDCData[CNT][icol];
		trailerct = trailerct + TDCTrail[CNT][icol];
	    }

	    printf( "\nData statistics\n" );
	    printf( "headerct, datact, trailerct, WordIDEct\n" );
	    printf( "%5d %8d  %8d  %8d\n", headerct, datact, trailerct, 
		        WordIDEct );
	    printf( "datactEr, NoMatchHeadEr, NoMatchTrailEr, NoHeadEr, EVIDer\n" );
	    printf( "%9d %10d %15d  %8d %8d\n", datacterror, NoMatchHeadError, NoMatchTrailError, 
			NoHeadError, EVIDerror );
	    printf( "CSM HeaderCnt, TrailerCnt, ETrailerCnt, ErrorCnt, builtEvents\n" );
	    printf( "    %9d   %9d    %9d  %8d\n", CSMHeaderCnt, CSMTrailerCnt, CSMETrailerCnt,
		     CSMErrorCnt );
	    if( CSMErrorCnt )
	    {
		printf( "CSM Error summary:\n  Missing trailers %d, Missing headers %d"
			", RAM full %d, data wait timeout %d\n", errReason1, errReason2, errReason3,
			errReason4 );
	    }
	}
} /* End captureOneEvent */

/**************************/
main(int argc, char *argv[])
/**************************/
{
  static int ret, fun = 1, occ = 1;
  static u_int data;
  static int whichLink = 0;
  int status;

  if ((argc==2)&&(sscanf(argv[1],"%d",&occ)==1)) {argc--;} else {occ;}
  if (argc != 1)
  {
     printf("This is CSMtest. \n\n");
     printf("Usage: CSMtest [FILAR occurrence]\n");
     exit(0);
  }

  strcpy( csmFileName, "/home/data/" );
  memset( buildChannels, 0, 18*sizeof(int) );

// Quit handler, for getting out of acquisition loops
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = SigQuitHandler;
  ret=sigaction(SIGQUIT, &sa, NULL);
  if (ret < 0)
  {
    printf("Cannot install signal handler for QUIT (error=%d)\n", ret);
    exit(0);
  }

  ts_open(1, TS_DUMMY);
  if( (status=uio_init()) )
    {
      exit( status );
    }
  if( (status=filar_map(occ)) )
    {
      exit( status );
    }

  ret = IO_PCIConfigReadUInt(shandle, 0x8, &data);
  if (ret != IO_RCC_SUCCESS)
  {
    rcc_error_print(stdout, ret);
    exit(-1);
  }


  printf("\n\n\nThis is FILARSCOPE running on a card of revision %02x\n", data & 0xff);
  while(fun != 0)
  {
// Interrupt handler, for cleanup after ^C
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SigIntHandler;
    ret=sigaction(SIGINT, &sa, NULL);
    if (ret < 0)
    {
      printf("Cannot install signal handler for INT (error=%d)\n", ret);
      exit(0);
    }
// Interrupt handler, for cleanup after shell "kill <pid>"
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SigIntHandler;
    ret=sigaction(SIGTERM, &sa, NULL);
    if (ret < 0)
    {
      printf("Cannot install signal handler for TERM (error=%d)\n", ret);
      exit(0);
    }
    sigsetjmp( env, 1 );

    printf("\n");
    printf("Select an option:\n");
    printf("  1 Print help           6 Reset the FILAR       7 Reset the S-Link\n");
    printf(" 10 Configure FILAR     12 CSM Test             13 Acquire Data\n");
    printf(" 14 Capture One Trigger 15 Accumulate Raw Data\n");
    printf("  0 Quit\n");
    printf("Your choice ");
    fun = getdecd(fun);
    if (fun == 1) mainhelp();
    if (fun == 6) cardreset();
    if (fun == 7) 
      {
	printf( "reset link %d (0=all): ", whichLink );
	whichLink = getdecd( whichLink );
	linkreset( whichLink );
      }
    if (fun == 10) filarconf( 1, &flowType, 0 );
    if (fun == 12) csmtest();
    if (fun == 13) csmAcquire();
    if (fun == 14) captureOneEvent();
    if (fun == 15) captureRaw();
  }

  filar_unmap();
  uio_exit();
  ts_close(TS_DUMMY);
  exit(0);

//    printf("  1 Print help           2 Dump PCI conf. space  3 Dump PCI MEM registers\n");
//    printf("  4 Write to a register  5 Read ACK FIFO         6 Reset the FILAR\n");
//    printf("  7 Reset the S-Link     8 Dump a buffer         9 SLIDAS test\n");
//    printf(" 10 Configure FILAR     11 Set latency counter  12 CSM Test\n");
//    printf(" 13 Acquire Data        14 Capture One Trigger\n");
//    if (fun == 2) dumpconf();
//    if (fun == 3) dumpmem();
//    if (fun == 4) setreg();
//    if (fun == 5) readack();
//    if (fun == 8) dumpbuff();
//    if (fun == 9) slidastest();
//    if (fun == 11) setlat();

}
