#define CSM_UTILS_DEFINE 1

// ****** NOTE: 1/31/04, NOT YET MODIFIED FOR MORE THAN ONE CHANNEL OF 4-CHAN GOLA

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include "rcc_error/rcc_error.h"
#include "io_rcc/io_rcc.h"
#include "cmem_rcc/cmem_rcc.h"

#include "filar_utils.h"
#include "filar_storage.h"
#include "csm_utils.h"

#define MIN_CAPTURE_CYCLES 0    // The minimum number of TDM cycles to capture 
                                //   in evInfo->dataBuffer after event building begins

void buildEvInit( oneEventInfo_T *evInfo )
{
  // Initialize the built event portion of the evInfo structure returned to user
    int j;

    evInfo->emptyCycles = 0;
    evInfo->badCycleCount = 0;
    evInfo->eventSize = 0;
    evInfo->bufSize = 0;
    gsaveLoc = 0;
    evInfo->evid = -1;
    for( j=0, evInfo->activeChanCount=0; j<18; ++j )
    {
	if( evInfo->maskArray[j] )
	  (evInfo->activeChanCount)++;
    }
}


int daqEvInit( int numbuffersreq, int activeChannel )
{
  // filar initialization
  // If numbuffersreq is zero, don't fill the FIFO but
  //    do print out the init message.
	activeChannel = 0x0011;  //open channel1,channel2
    unsigned int data = 0xffffffff;
    int bufnr;
    int status = 0;
    filar->ocr = (filar->ocr | 0x08208200);  //disable all
    if(activeChannel & 0x0001) data &= 0xfffffdff;
    if(activeChannel & 0x0010) data &= 0xffff7fff;
    if(activeChannel & 0x0100) data &= 0xffdfffff;
    if(activeChannel & 0x1000) data &= 0xf7ffffff;
    filar->ocr = filar->ocr & data;
    active[1] = (data & 0x00000200) ? 0 : 1;
	active[2] = (data & 0x00008000) ? 0 : 1;
	active[3] = (data & 0x00200000) ? 0 : 1;
	active[4] = (data & 0x08000000) ? 0 : 1;

  /* reset the comm link */
  /* Reset Card */
    if( cardreset() )
        status=1;

  /* Allocate numbuffersreq buffers for channel 1*/
  /* to do: Finish off channels other than 1  */
    for (bufnr = 0; bufnr < numbuffersreq; bufnr++)
    {
        // printf("bufnr %2d paddr[1][bufnr] 0x%08x paddr[2][bufnr] 0x%08x\n",bufnr,paddr[1][bufnr],paddr[2][bufnr]);
	filar->req1 = paddr[1][bufnr];
	filar->req2 = paddr[2][bufnr];
	filar->req3 = paddr[3][bufnr];
	filar->req4 = paddr[4][bufnr];
    }

  /* reset and bring up the optical link now */
    if( linkreset( activeChannel ) )
        status+=1;

    if( numbuffersreq == 0 )
        printf("===== InitDAQ Done ===========================\n");
    return status;
}


int getNextEvent( oneEventInfo_T *evInfo, int flowIsTDM, int *offset, unsigned int *dataptr, unsigned int fsize )
{
  // Pull next event from a buffer.  
  // Return = 1 for success.
  // Return = 0 for event not found before buffer ends
  // Also updates offset into input buffer in evInfo->endOffset

        int j;
        int dataindex;
	int haveData;
	unsigned int *evLocData;
	unsigned int *dataBuffer;
	unsigned int *builtEvent;
	unsigned int WordID;
	int buildID;

	haveData = 0;
	dataBuffer = evInfo->dataBuffer;
	builtEvent = evInfo->builtEvent;
	activeChanCount = evInfo->activeChanCount;
//printf("First 20 words of buffer:\n" );
//for( j=0; j<20; ++j )  printf( "%8.8x ", *(dataptr+j) );
//printf("\n");
            if( flowIsTDM )   // TDM data
	    {
		if( !firstflag )  // sync up.  Find an active channel
		{
		    TDCChIndex = -1;
		    for( j=0; j<fsize; ++j )
		    {
			if( (*(dataptr+j) & 0xf0000000) == 0xd0000000 )
			{
			    *offset = j;
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
		    for( dataindex=*offset; dataindex<fsize; ++dataindex )
		    {
		        evLocData = dataptr+dataindex;
			WordID = *evLocData & 0xf0000000;

			switch( WordID )
			{
			case 0x00000000:
			     ++TDCChIndex;
			     if( dataBuffer && (evInfo->bufSize < evInfo->bufLen) )
			     {
				 *(dataBuffer+gsaveLoc+TDCChIndex) = *evLocData;
				 (evInfo->bufSize)++;
			     }
			     break;

			case 0xa0000000:
			     if( TDCChIndex < 18 )
			     {
			         if( evInfo->maskArray[TDCChIndex] == 1 )
			         {
			             buildID = whichId( *evLocData, 0 );
				     if( firstBuildId == -1 )
				         firstBuildId = buildID;
				     thisBuildId[TDCChIndex] = buildID;
			             haveFlushId = checkFlush( buildID, TDCChIndex );
			             haveHeader[buildID][TDCChIndex] = 1;
				     if( (bufPtrSize[thisBuildId[TDCChIndex]]==0) && builtEvent )
				     {
				         if( bufPtrs[thisBuildId[TDCChIndex]] )
					 {
					     addWordToBuffer( bufPtrs[thisBuildId[TDCChIndex]], (BD_CSM_HEAD | 
							  (*evLocData & (BD_EVID_MSK | BD_BCID_MSK))),
							  &(bufPtrSize[thisBuildId[TDCChIndex]]) );
					     evInfo->eventSize = 1;
					 }
				     }
			         }
			     }
			     if( TDCChIndex<18 && bufPtrSize[thisBuildId[TDCChIndex]] )
			     {
			         if( haveFlushId==-1 && evInfo->maskArray[TDCChIndex]==1
				     && firstBuildId==thisBuildId[TDCChIndex] && builtEvent)
			         {
				     if( bufPtrs[thisBuildId[TDCChIndex]] )
				     {
				       addWordToBuffer( bufPtrs[thisBuildId[TDCChIndex]], 
							(*evLocData & BD_HDR_TRLR) | 
							((TDCChIndex & BD_SHRT_CHAN)<<24), 
							&(bufPtrSize[thisBuildId[TDCChIndex]]) );
				     }
				 }
			     }
			     ++TDCChIndex;
			     if( dataBuffer && (evInfo->bufSize < evInfo->bufLen) )
			     {
			         *(dataBuffer+gsaveLoc+TDCChIndex) = *evLocData;
				 (evInfo->bufSize)++;
			     }
			     validDataInCycle = 1;
			     break;

			case 0xc0000000:
			     if( TDCChIndex < 18 )
			     {
			         if( evInfo->maskArray[TDCChIndex] == 1 )
			         {
			             buildID = whichId( *evLocData, 0 );
			             haveFlushId = checkFlush( buildID, TDCChIndex );
			             if( haveTrailer[buildID][TDCChIndex] != 1 ) 
			             {
			                 haveTrailer[buildID][TDCChIndex] = 1;
				         if( haveHeader[buildID][TDCChIndex] == 1 ) matchedCount[buildID]++;
				         if( matchedCount[buildID] == activeChanCount )
				         {
					     cycleQuit = 1;
				             if( eFlush[buildID] )
				               incompleteBuild++;
				             else
				               goodBuild++;
					     flushEvid = activeEvid[buildID];
				             activeEvid[buildID] = -1;
					     quitTimeBuildId = buildID;
				         }
			             }
			         }
			     }
			     if( TDCChIndex<18 && bufPtrSize[thisBuildId[TDCChIndex]] )
			     {
			         if( haveFlushId==-1 && evInfo->maskArray[TDCChIndex]==1 && builtEvent )
			         {
				     if( bufPtrs[thisBuildId[TDCChIndex]] )
				     {
				       addWordToBuffer( bufPtrs[thisBuildId[TDCChIndex]],
							(*evLocData & BD_HDR_TRLR) | 
							((TDCChIndex & BD_SHRT_CHAN)<<24), 
							&(bufPtrSize[thisBuildId[TDCChIndex]]) );
				     }
				 }
			     }
			     ++TDCChIndex;
			     if( dataBuffer && (evInfo->bufSize < evInfo->bufLen) )
			     {
			         *(dataBuffer+gsaveLoc+TDCChIndex) = *evLocData;
				 (evInfo->bufSize)++;
			     }
			     validDataInCycle = 1;
			     break;

			case 0xd0000000:
			     if( TDCChIndex != 18 ) 
                             {
                                 evInfo->badCycleCount++;
			         validDataInCycle = 0;
                             }
			     TDCChIndex = 0;
			     if( validDataInCycle )
			     {
			         if( (gsaveLoc+19) > evInfo->bufLen ) 
				 {
				     cont = 0;
				 }
				 else
				 {
				     gsaveLoc += 19;
				 }
				 ++haveData;
				 validDataInCycle = 0;
			     }
			     else if( notVeryFirstCycle )
			     {
			         evInfo->emptyCycles++;
			     }
			     if( cycleQuit==1 && haveData>MIN_CAPTURE_CYCLES )
			         cont = 0;
			     if( dataBuffer && cont )
			     {
				 *(dataBuffer+gsaveLoc+TDCChIndex) = *evLocData;
				 (evInfo->bufSize)++;
			     }
			     break;

			case 0x30000000:
			case 0x40000000:
			     if( TDCChIndex<18 && evInfo->eventSize )
			     {
			         if( evInfo->maskArray[TDCChIndex]==1 && builtEvent && (thisBuildId[TDCChIndex]>=0) )
			         {
				     if( bufPtrs[thisBuildId[TDCChIndex]] )
				     {
				         addWordToBuffer( bufPtrs[thisBuildId[TDCChIndex]],
							formedTdc( TDCChIndex, *evLocData ), 
							&(bufPtrSize[thisBuildId[TDCChIndex]]) );
				     }
				 }
			     }
			     ++TDCChIndex;
			     if( dataBuffer && (evInfo->bufSize < evInfo->bufLen) )
			     {
			         *(dataBuffer+gsaveLoc+TDCChIndex) = *evLocData;
				 (evInfo->bufSize)++;
			     }
			     validDataInCycle = 1;
			     break;

			case 0x20000000:
			case 0x60000000:
			case 0x70000000:
			default:
			     if( TDCChIndex<18 && evInfo->eventSize )
			     {
			         if( evInfo->maskArray[TDCChIndex]==1 && builtEvent && (thisBuildId[TDCChIndex]>=0) )
			         {
				     if( bufPtrs[thisBuildId[TDCChIndex]] )
				     {
				       addWordToBuffer( bufPtrs[thisBuildId[TDCChIndex]], *evLocData, 
							&(bufPtrSize[thisBuildId[TDCChIndex]]) );
				     }
				 }
			     }
			     ++TDCChIndex;
			     if( dataBuffer && (evInfo->bufSize < evInfo->bufLen) )
			     {
			         *(dataBuffer+gsaveLoc+TDCChIndex) = *evLocData;
				 (evInfo->bufSize)++;
			     }
			     validDataInCycle = 1;
			     break;
			}   // End of switch.....

			notVeryFirstCycle = 1;

			if( haveFlushId != -1 )
			{
			    cycleQuit = 1;
			    quitOnEvFlush = haveFlushId+1;
			}

			if( cont == 0 )
			{
			    if( builtEvent )
			    {
				if( quitOnEvFlush )
				{
				     --quitOnEvFlush;
				     if( bufPtrs[quitOnEvFlush] )
				     {
				         addWordToBuffer( bufPtrs[quitOnEvFlush], (BD_CSM_ETRL | 
						      ((flushEvid<<12) & BD_EVID_MSK) |
						      ((bufPtrSize[quitOnEvFlush]+1) & BD_BCID_MSK)),
							&(bufPtrSize[quitOnEvFlush]) );
				     }
				     if( evInfo->eventLen < bufPtrSize[quitOnEvFlush] )
				         bufPtrSize[quitOnEvFlush] = evInfo->eventLen;
				     memcpy( builtEvent, bufPtrs[quitOnEvFlush], sizeof(unsigned int)*
					     bufPtrSize[quitOnEvFlush] );
				     evInfo->eventSize = bufPtrSize[quitOnEvFlush];
				     evInfo->evid = flushEvid;
				     quitTimeBuildId = quitOnEvFlush;
				}
				else
				{
				     if( bufPtrs[quitTimeBuildId] )
				     {
				         addWordToBuffer( bufPtrs[quitTimeBuildId], (BD_CSM_TRL | 
						      ((flushEvid<<12) & BD_EVID_MSK) |
						      ((bufPtrSize[quitTimeBuildId]+1) & BD_BCID_MSK)),
							&(bufPtrSize[quitTimeBuildId]) );
				     }
				     if( evInfo->eventLen < bufPtrSize[quitTimeBuildId] )
				         bufPtrSize[quitTimeBuildId] = evInfo->eventLen;
				     memcpy( builtEvent, bufPtrs[quitTimeBuildId], sizeof(unsigned int)*
					     bufPtrSize[quitTimeBuildId] );
				     evInfo->eventSize = bufPtrSize[quitTimeBuildId];
				     evInfo->evid = flushEvid;
				}
			    }
			    returnCaptureBuffer( bufPtrs[quitTimeBuildId] );
			    *offset = dataindex;
			    gsaveLoc = 0;
			    if( TDCChIndex == 0 )
			        TDCChIndex = 18;
			    cycleQuit = 0;
			    quitOnEvFlush = 0;
			    return 1;    // Success, have an event of some kind
			}
		    }       // End loop over words in buffer
		}   // end data check for TDM-style data
	    }       // end TDM-style data

	    else    // begin Event-Built (EB-style) data
	    {
		for( dataindex=*offset; dataindex<fsize; ++dataindex )
		{
		    evLocData = dataptr+dataindex;
		    if( dataBuffer && (gsaveLoc < evInfo->bufLen) )
		    {
			*(dataBuffer+gsaveLoc) = *evLocData;
			++gsaveLoc;
		    }
		    if( !cycleOK )
		    {
			if( (*evLocData & 0xff000000) == 0x59000000 )
			{
			    evInfo->evid = (*evLocData & 0x00fff000)>>12;
			    cycleOK = 1;
			}
		    }

		    if( cycleOK )
		    {
			 WordID = *evLocData & 0xF0000000;
		      // compute tdcno from encoded values
			 TDCChIndex = (*evLocData)>>24 & 0xF;    // Standard, in TDC header/trailer
			 switch(WordID)
			 {
			 case 0x30000000:
			 case 0x40000000:
			   // re-compute tdcno from encoded values if needed
			      if( (*evLocData & 0x00c00000) == 0x00c00000 )
			      {
				  TDCChIndex = 16 + (((*evLocData)>>26) & 0x1);
			      }
			      break;
			 case 0x50000000:
			      WordID = *evLocData & 0x0F000000;
			      if( WordID==0x0d000000 || WordID == 0x0b000000 )  /* Trailer */
			      {
				  cont = 0;
				  cycleOK = 0;
				  cycleQuit = 1;
			      }
			      break;
			 case 0x00000000:
			 case 0x20000000:
			 case 0x60000000:
			 case 0x70000000:
			 case 0xa0000000:
			 case 0xc0000000:
			 default:
			      break;
			 } /* end switch */

			 if( builtEvent && evInfo->eventSize<evInfo->eventLen )
			 {
			     *(builtEvent+evInfo->eventSize) = *evLocData;
			     (evInfo->eventSize)++;
			 }
		    }
		    if( cycleQuit )           // End, have one event now
		    {
			evInfo->bufSize = gsaveLoc;
			gsaveLoc = 0;
			*offset = dataindex;
			cycleQuit = 0;
		        return 1;
		    }
		}
	    }
	    *offset = 0;
//printf( "TDCChIndex=%d, gsaveLoc=%d, firstflag=%d\n", TDCChIndex, gsaveLoc, firstflag );
	    return 0;
}


int captureEvent( int flowIsTDM, oneEventInfo_T *evInfo )

/*
 * Acquire one event into the specified call buffers.  Do not exceed the length
 * of the buffers.
 *
 * This is a complete capture cycle, with all initializations performed and
 * GOLA completely handled.  Function getNextEvent is called to do the actual
 * building and event capture.
 *
 *   Inputs in evInfo:
 * evInfo->maskArray elements must be filled prior to entry, 
 *        with 1/0 -> channel enabled/not.
 * evInfo->checkSignals & 0x1 TRUE means watch for sigINT signal (CSMtest environment)
 * evInfo->checkSignals & 0x2 TRUE means watch *(evInfo->watchVariable) and quit when
 *         it takes on the value evInfo->watchValue
 * evInfo->checkSignals & 0x4 TRUE means quit if more than 2 seconds have elapsed
 * evInfo->dataBuffer (and bufLen) is a buffer of indicated length to contain
 *        incoming, as-is data.
 * evInfo->builtEvent (and eventLen) is a buffer of indicated length to contain
 *        the first built (or flushed) event in CSM-0 format.
 *
 * Remainder of evInfo filled for return.
 *
 * Function return value is 0 for success, 
 * or one for failure due to buffer fillup or quit signal.
 */

{
	clock_t startup;
	clock_t endup;
	int numbuffersreq = 30, bufnr;
	unsigned int fsizedata, fifodata;
	unsigned int *dataptr;
	unsigned int notready;
	int jumpBack;
	unsigned int fsize;
	int j;
	int offset;
	int returnStat, tstat;
	unsigned int loopCount;

	returnStat = 0;                       // Default to return with error

	buildInit();                          // this should happen but once per buffer set

	buildEvInit( evInfo );                // evInfo inits, for each returned event

	tstat = daqEvInit( numbuffersreq, 0 );   // filar resets and ready sequence
	bufnr = 0;

/********************************************************************************/
      /* Initialize acquire control variables */
	notready = 1;

	cont = 1;
	loopCount = 0;

	if( evInfo->checkSignals & 0x4 )
	  {
	    startup = clock();
	  }

	while( cont )
	{
	  /* Wait until current buffer is full */
          /* Look at CH1 ACK_Available until data becomes available */

	    if( evInfo->checkSignals & 0x1 )
	      {
		jumpBack = sigsetjmp( env, 1 );
		if( jumpBack )
		  {
		    break;
		  }
	      }

	    while( notready ) 
	      {
		fifodata = filar->fifostat;
// to do: check here on fifo only for correctly active channel (1=f, 2=f00, 3=f0000, 4=f000000)
		if (fifodata & 0x0000000f)
		  {
		    break;
		  }
		if( evInfo->checkSignals & 0x2 )
		  {
		    if( *(evInfo->watchVariable) == evInfo->watchValue )
		      {
			cont = 0;
			break;
		      }
		  }

		if( (evInfo->checkSignals & 0x4) && ((++loopCount)%1000==0) )
		  {
		    endup = clock();
		    if( 10*(endup-startup)/CLOCKS_PER_SEC > 20 )
		      {
			cont = 0;
			break;
		      }
		  }
	      }
	    if( cont == 0 )
	      {
		break;
	      }

	 /* 
	  * Read the ACK FIFO to get the size of the data packet in the data
	  * buffer.  It had better always be the same for all that we grab!
	  * fsize in 32bit words. 1M word max
	  */
// to do: or ack2, 3, 4 for active[1,2,3,4]
	    fsizedata = filar->ack1;
	    fsize = fsizedata & 0xfffff;

	 /* 
	  * Get a pointer to the data and write it out to disk
	  * after first doing a few checks (very few)
	  */
//to do: [1] becomes fiber channel #
	    dataptr = (unsigned int *) uaddr[1][bufnr];
	    offset = 0;

/*
	    while( (returnStat=getNextEvent( evInfo, flowIsTDM, &offset, dataptr, fsize )) )
	      {
		printf( "Built --> " );
		for( j=0; j<evInfo->eventSize; ++j )
		  {
		    printf( "%8.8x ", *(evInfo->builtEvent+j) );
		  }
		printf("\n" );
		cont = 1;
		if( returnStat )
		{
		    evInfo->eventSize = 0;
		    evInfo->bufSize = 0;
		}
	      }
	    if( bufnr == 1 ) break;
*/
	    returnStat = getNextEvent( evInfo, flowIsTDM, &offset, dataptr, fsize );
	    if( returnStat == 1 )
	      break;        // Success, found event

	 /* Fill the next address into the req fifo for the next loop */
//to do: [1] becomes fiber channel #, req1->reqN
	    filar->req1 = paddr[1][bufnr];
         /* switch to the next buffer */
	    if( ++bufnr == numbuffersreq )
	    {
		bufnr = 0;
	    }
	}     // End of acquisition loop

/********************************************************************************/

	return returnStat;

} /* End captureEvent */


void buildInit()
{
// Complete init of build counters and pointers
  int j;

  memset( noHead,  0, 18*sizeof(unsigned int) );
  memset( noTrail, 0, 18*sizeof(unsigned int) );
  newSeqNo = 0;
  incompleteBuild = 0;
  goodBuild = 0;
  forcedFlush = 0;
  activeTriggers = 0;
  firstBuildId = -1;
  for( j=0; j<BUILD_SIZE; ++j )
  {
      seqNo[j] = -1;
      activeEvid[j] = -1;
      thisBuildId[j] = -1;
  }
  flushEvid = -1;

// Initialize variables used in repeated calls to getNextEvent
  firstflag = 0;
  firstValidOffset = -1;
  TDCChIndex = 0;
  
  cycleOK = 0;
  validDataInCycle = 0;
  gsaveLoc = 0;
  haveFlushId = -1;
  cycleQuit = 0;
  quitOnEvFlush = 0;
  notVeryFirstCycle = 0;

// Set up the default set of 3 capture buffers
  initCaptureBuffers();               // Init the built event capture buffers
  setCaptureBuffer( buffer1, 4096 );  // Set up 3 buffers
  setCaptureBuffer( buffer2, 4096 );
  setCaptureBuffer( buffer3, 4096 );
}


int checkParity( unsigned int dataWord, unsigned int *CommPEct )
{
/*
 * Check for fiber transmission parity error
 */
  int bitct, shift;

  bitct = 0;
  for (shift = 0; shift < 32; ++shift)
  {
      if( (dataWord & parityBits[shift]) != 0 )  bitct++;
  }

  if (!(bitct & 0x00000001)) 
  {
      ++(*CommPEct);
      return 1;
  }
  else
  {
      return 0;
  }
}


unsigned int formedTdc( int ichan, unsigned int data )
{
  unsigned int builtTdc;

  if( ichan < 16 )
  {
      builtTdc = (data & BUILD_MASK1) | ((unsigned int) ichan << 24);
  }
  else
  {
      builtTdc = BUILD_MASK3 | (data & BUILD_MASK2) 
	  | ((data & BUILD_MASK3) << 2) | ((ichan&0x1)<<26);
  }
  return builtTdc;
}


int whichId( unsigned int dataWord, int doPrint )
{
  unsigned int evid, bcid;
  int j, k;
  int retVal;

  evid = (dataWord>>12) & 0x00000FFF;

  //  See if this evid is currently in build
  for( j=0; j<BUILD_SIZE; ++j )
  {
      if( evid == activeEvid[j] )  
      {
        if( doPrint && cont ) 
	{
	    bcid = dataWord & 0x00000FFF;
	    if( (dataWord & 0xF0000000) == 0xa0000000 )
	    {
	        printf("Found existing buildID=%d for evid=0x%x(%d), bcid=%d\n", j, 
		       evid, evid, bcid );
	    }
	    else
	    {
	        printf("Found existing buildID=%d for evid=0x%x(%d), wrdCnt=%d\n", j, 
		       evid, evid, bcid );
	    }
	}
        return j;
      }
  }

  // OK, not now building, if a slot is free, add it and prepare to build it
  retVal = 0;
  for( j=0; j<BUILD_SIZE; ++j )
  {
      if( j > activeTriggers ) activeTriggers = j;
      if( activeEvid[j] == -1 )
      {
          activeEvid[j] = evid;
          for( k=0; k<18; ++k )
          {
	      haveHeader[j][k] = 0;
	      haveTrailer[j][k] = 0;
          }
          seqNo[j] = newSeqNo++;
	  matchedCount[j] = 0;
	  eFlush[j] = 0;
	  bufPtrs[j] = getCaptureBuffer();
	  bufPtrSize[j] = 0;
          if( doPrint && cont ) 
	  {
	      bcid = dataWord & 0x00000FFF;
	      if( (dataWord & 0xF0000000) == 0xa0000000 )
	      {
	          printf("Get new buildID=%d for evid=0x%x(%d), bcid=%d\n", j, 
		       evid, evid, bcid );
	      }
	      else
	      {
	          printf("Get new buildID=%d for evid=0x%x(%d), wrdCnt=%d\n", j, 
		       evid, evid, bcid );
	      }
	  }
          return j;
      }
      else if( seqNo[j] < seqNo[retVal] )
      {
          retVal = j;
      }
  }

  // Too bad, no free slots, blow away the oldest -- dangerous
  // This could cause a mis-count, but is really the easiest way of flushing
  if( doPrint && cont ) 
  {
      bcid = dataWord & 0x00000FFF;
      if( (dataWord & 0xF0000000) == 0xa0000000 )
      {
	  printf("Blow away oldest buildID=%d for evid=0x%x(%d), bcid=%d\n", j, 
	       evid, evid, bcid );
      }
      else
      {
	  printf("Blow away oldest buildID=%d for evid=0x%x(%d), wrdCnt=%d\n", j, 
	       evid, evid, bcid );
      }
  }
  for( j=0; j<18; ++j )
  {
      if( buildChannels[j] )
      {
	if( haveHeader[retVal][j]==1 && haveTrailer[retVal][j]==0 ) noTrail[j]++;
	if( haveTrailer[retVal][j]==1 && haveHeader[retVal][j]==0 ) noHead[j]++;
	haveHeader[retVal][j] = 0;
	haveTrailer[retVal][j] = 0;
      }
  }
  activeEvid[retVal] = evid;
  seqNo[retVal] = newSeqNo++;
  matchedCount[retVal] = 0;
  eFlush[retVal] = 0;
  if( bufPtrs[retVal] == NULL )
    bufPtrs[retVal] = getCaptureBuffer();
  bufPtrSize[retVal] = 0;
  incompleteBuild++;
  return retVal;
}


int checkFlush( int buildID, int chanNo )
{
  // Check if a previously encountered trigger got all its data
  // from this mezz board.  If not, mark it for flushing as it
  // will never complete.
  // Return value is -1 if nothing flushed, else returns buffer
  // number of lowest sequenced event flushed.

  int j;
  int flushId = -1;

  for( j=0; j<BUILD_SIZE; ++j )
  {
      if( (j!=buildID) && (activeEvid[j]>=0) && (seqNo[j]<seqNo[buildID]) )
      {
	  if( haveTrailer[j][chanNo]==0 || haveHeader[j][chanNo]==0 )
	  {
//	      printf( "Flush buildID %d w evid %d due to channel %d on evid %d\n", 
//		    buildID, seqNo[j], chanNo, seqNo[buildID] );
/*
	      if( flushId==-1 ) 
	      {
		  flushId = j;
	      }
	      else if( seqNo[j] < seqNo[flushId] )
	      {
		  flushId = j;
	      }
*/

	      if( eFlush[j] == 0 ) forcedFlush++;
	      eFlush[j] = 1;
	      if( haveTrailer[j][chanNo] == 0 )
	      {
		  haveTrailer[j][chanNo] = 1;
		  noTrail[chanNo]++;
	      }
	      if( haveHeader[j][chanNo] == 0 )
	      {
		  haveHeader[j][chanNo] = 1;
		  noHead[chanNo]++;
	      }
	      matchedCount[j]++;
	      if( matchedCount[j] == activeChanCount )
	      {
		  incompleteBuild++;
		  flushEvid = activeEvid[j];
		  activeEvid[j] = -1;
		  returnCaptureBuffer( bufPtrs[j] );
		  if( flushId==-1 ) 
		  {
		      flushId = j;
		  }
		  else if( seqNo[j] < seqNo[flushId] )
		  {
		      flushId = j;
		  }
	      }
	  }
      }
  }
  return flushId;
}


/********************************************************************************************/
void incBigValue( unsigned int *baseValue, unsigned int *overValue, unsigned int increment )
/********************************************************************************************/
{
    *baseValue += increment;
    if( *baseValue >= 1000000000 )
    {
        (*overValue)++;
	*baseValue -= 1000000000;
    }
}


/************************************************************************************/
void printBigValue( unsigned int baseValue, unsigned int overValue, char *outString )
/************************************************************************************/
{
    int checkLen;
    if( overValue )
    {
        sprintf( outString, "%d", overValue );
	checkLen = strlen( outString );
	sprintf( outString+checkLen, "%9.9d", baseValue );
    }
    else
    {
	sprintf( outString, "%d", baseValue );
    }
}


void setCaptureBuffer( unsigned int *pBuf, int bufSize )
{
  if( numBufsToVend < BUILD_SIZE )
    {
      pBufsToVend[numBufsToVend++] = pBuf;
      if( bufSize > bufMaxSize )
	bufMaxSize = bufSize;
    }
}


void initCaptureBuffers()
{
  numBufsToVend = 0;
  bufsVended = 0;
  bufNextUse = 0;
  bufToReturn = 0;
  bufMaxSize = 0;
}


unsigned int * getCaptureBuffer()
{
  unsigned int *useThisBuffer;

  if( bufsVended >= numBufsToVend )
    {
      return NULL;
    }

  useThisBuffer = pBufsToVend[bufNextUse++];
  if( bufNextUse == numBufsToVend )
    bufNextUse = 0;
  bufsVended++;
  return useThisBuffer;
}

void returnCaptureBuffer( unsigned int *pBuf )
{
  if( pBuf != NULL )
    {
      pBufsToVend[bufToReturn++] = pBuf;
      if( bufToReturn == numBufsToVend )
	bufToReturn = 0;
      bufsVended--;
    }
}


void addWordToBuffer( unsigned int *pBuf, unsigned int newWord, int *bufFillSize )
{
  if( *bufFillSize < bufMaxSize )
    {
      *(pBuf + *bufFillSize) = newWord;
      ++(*bufFillSize);
    }
}
