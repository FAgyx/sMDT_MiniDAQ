#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
// #define BCID_RANGE 1  // just look at range of TTC bcid values
#define CSM_AMT_MATCH 1  // match bcid and evid between csm/amt

const int READ_BYTES = 100;

int main()
{
  char filename[80];
  FILE *fyle;
  char inbuf[110];
  int j, ninput;
  int k, kstart;
  int recordsIn;
  unsigned short wordType, fullWordType, wordVal;
#ifdef BCID_RANGE
  long bcidVals[4100];
#endif
#ifdef CSM_AMT_MATCH
  long bcidDelta[11];
  long evidDelta[11];
  short bcidMisMatch;
  short evidMisMatch;
  short csmEvid, csmBcid;
  short amtHBcid, amtHEvid;
  short amtTEvid;
  short csmTEvid;
  short shortWordType;
  short bDelta, eDelta;
  long errorTrails, errorWords, padWords;
  long tdcHeaders;
  long tdcTrailers;
  long tdcData;
  long tdcMark;
  long tdcErrors;
  long tdcOther;
  long AMTwords;
  long countHeads;
  float fAmtWords, fTriggers, fAvg;

#endif
  int firstFound;
  int countTrails;
  int DUMP_EVENTS;

// --------- Initialize -----------

  inbuf[READ_BYTES] = '\0';
  recordsIn = 0;
  firstFound = 0;
  countTrails = 0;
  DUMP_EVENTS = 0;     // set to 1 to print events to stdout
#ifdef BCID_RANGE
  memset( (void *) bcidVals, 0, (size_t) 4100*sizeof(long) );
#endif
#ifdef CSM_AMT_MATCH
  memset( (void *) bcidDelta, 0, (size_t) 11*sizeof(long) );
  memset( (void *) evidDelta, 0, (size_t) 11*sizeof(long) );
  evidMisMatch = 10000;
  bcidMisMatch = 10000;
  errorTrails = 0;
  errorWords = 0;
  padWords = 0;
  tdcHeaders = 0;
  tdcTrailers = 0;
  tdcData = 0;
  tdcMark = 0;
  tdcErrors = 0;
  tdcOther = 0;
  AMTwords = 0;
  countHeads = 0;
#endif

  printf(" Enter input file name: " );
  scanf( "%s", filename );
  if( (fyle=fopen(filename,"rb")) == NULL )
  {
    printf( "Cannot open file \"%s\"\n", filename );
    exit(1);
  }

// --------- Process ---------

  while( (ninput=fread( inbuf, 1, READ_BYTES, fyle )) == READ_BYTES )
  {
/*
    for( j=0; j<ninput; ++j )
    {
      k = ((unsigned short) inbuf[j]) & 0xff;
      printf( "%02x", k );
    }
    printf("\n");
*/

    recordsIn++;
    for( j=0; j<ninput; j+=4 )
    {
#ifdef __sparc  // Solaris is Little Endian, but don't rely on _LITTLE_ENDIAN
      swab( (void *) (inbuf+j+2), (char *)(&fullWordType), 2 );
      swab( (void *) (inbuf+j), (char *)(&wordVal), 2 );
#else           // Linux in our case, on our built boxes, is Big Endian
      memcpy( (void *) (&fullWordType), (void *) (inbuf+j+2), 2 );
      memcpy( (void *) (&wordVal), (void *) (inbuf+j), 2 );
#endif
      wordType = fullWordType>>8 & 0xFF;
      if( wordType == 0x59 )         // CSM header
      {
        if( !firstFound )
        {
          firstFound = 1;
          printf( "First CSM header found in record %d at offset %d\n", recordsIn, j );
        }
#ifdef BCID_RANGE
        wordVal = wordVal & 0xFFF;   // BCID from header
        bcidVals[wordVal]++;
#endif
#ifdef CSM_AMT_MATCH
        csmEvid = ((fullWordType & 0xFF) << 4) | ((wordVal & 0xF000)>>8);
        csmBcid = wordVal & 0xFFF;
#endif
      }

#ifdef CSM_AMT_MATCH
      if( firstFound )
      {
         shortWordType = wordType >> 4;
	 switch( shortWordType )
	   {
	   case 0x5:
              if( wordType == 0x5b )
              {
                countTrails++;
                csmTEvid = ((fullWordType & 0xFF) << 4) | ((wordVal & 0xF000)>>8);
                if( DUMP_EVENTS ) printf( "\n" );
                if( countTrails == 1 )      // First trailer encountered
                {
                   bcidMisMatch = csmBcid - amtHBcid;
                   evidMisMatch = csmEvid - amtHEvid;
                   printf( "CSM-AMT evid = %d\nCSM-AMT bcid = %d\n", evidMisMatch, bcidMisMatch );
                }
                bDelta = csmBcid - amtHBcid - bcidMisMatch + 5;
                if( bDelta < -4000 ) bDelta += 4096;
                if( bDelta > 4000  ) bDelta -= 4096;
                eDelta = csmEvid - amtHEvid - evidMisMatch + 5;
                if( eDelta < -4000 ) eDelta += 4096;
                if( eDelta > 4000  ) eDelta -= 4096;

                if( bDelta < 0 )
                {
                   bDelta = 0;
                }
                else if( bDelta > 10 )
                {
                   bDelta = 10;
                }
                if( eDelta < 0 )
                {
                   eDelta = 0;
                }
                else if( eDelta > 10 )
                {
                   eDelta = 10;
                }
                bcidDelta[bDelta]++;
                evidDelta[eDelta]++;
                if( amtHEvid != amtTEvid )
                   printf("\n --> AMT EVID misMatch, head/trail=%d/%d\n", amtHEvid, amtTEvid );
                if( csmEvid != csmTEvid )
                   printf("\n --> CSM EVID misMatch, head/trail=%d/%d\n", csmEvid, csmTEvid );

//                if( countTrails == 10 ) exit(0);
              }
	      else if( wordType == 0x5d )
	      {
	          errorTrails++;
	      }
	      else if( wordType == 0x50 )
	      {
	          errorWords++;
	      }
	      else if( wordType == 0x52 )
	      {
	          padWords++;
	      }
	      else if( wordType == 0x59 )
	      {
		  countHeads++;
	      }
	      break;
	   case 0xa:
	      tdcHeaders++;
	      AMTwords++;
              amtHBcid = wordVal & 0xFFF;
              amtHEvid = ((fullWordType & 0xFF) << 4) | ((wordVal & 0xF000)>>8);
	      break;
	   case 0xc:
	      tdcTrailers++;
	      AMTwords++;
              amtTEvid = ((fullWordType & 0xFF) << 4) | ((wordVal & 0xF000)>>8);
	      break;
	   case 0x2:
	      tdcMark++;
	      AMTwords++;
	      break;
	   case 0x6:
	      tdcErrors++;
	      AMTwords++;
	      break;
	   case 0x3:
	   case 0x4:
	      tdcData++;
              amtTEvid = ((fullWordType & 0xFF) << 4) | ((wordVal & 0xF000)>>8);
	      AMTwords++;
	      break;
	   case 0x7:
	      tdcOther++;
	      AMTwords++;
	      break;
	   default:
	      tdcOther++;
	      AMTwords++;
	      break;
	   }
      }

      if( firstFound && DUMP_EVENTS )
      {
        printf( " %04x%04x", fullWordType, wordVal );
      }
#endif
    }
  }

// --------- Termination processing ------------
  fclose( fyle );
  printf( "Processed %d records of %d bytes each\n", recordsIn, READ_BYTES );
  printf( "Found %d fully built events\n", countTrails );

#ifdef BCID_RANGE
  for( k=3500; k<4096; k+=10 )   // Just print out the end of this
  {
    for( j=0; j<10; ++j )
    {
       printf( "%5d", k+j );
    }
    printf("\n");
    for( j=0; j<10; ++j )
    {
       printf( "%5d", bcidVals[k+j] );
    }
    printf("\n\n");
  }
#endif
#ifdef CSM_AMT_MATCH
  printf( "\n CSM types:   Headers    Errors   ErrTrails   GoodTrails    PadWords\n"
	  "           %10d%10d  %10d   %10d  %10d\n", countHeads, errorWords, errorTrails, 
	  countTrails, padWords );
  printf( " AMT types:   Headers      Data   Trailers      Mark     Error     Other\n"
	  "           %10d%10d %10d%10d%10d%10d\n", tdcHeaders, tdcData, tdcTrailers,
	  tdcMark, tdcErrors, tdcOther );
  fAmtWords = AMTwords;
  fTriggers = countTrails;
  if( countTrails )
      fAvg = fAmtWords/fTriggers;
  else
      fAvg = 0.0;
  printf( " Average number of AMT words per built event = %8.2f\n", fAvg );
  printf( "\n    CSM-AMT Delta-Evid distribution centered at %d\n     -5     -4     -3     -2"
         "     -1      0      1      2      3      4      5\n", evidMisMatch );
  for( k=0; k<11; ++k ) printf("%7d", evidDelta[k] );
  printf( "\n\n    CSM-AMT Delta-Bcid distribution centered at %d\n     -5     -4     -3     -2"
         "     -1      0      1      2      3      4      5\n", bcidMisMatch );
  for( k=0; k<11; ++k ) printf("%7d", bcidDelta[k] );
  printf( "\n" );
#endif
  exit(0);
}
