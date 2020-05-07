
CollectCSMData::CollectCSMData(int filar_chnl, FILE * p_CSM_file, FILE *p_raw_file, FILE *p_log_file){
  //arguments initial

  CSMDataFile = p_CSM_file;
  rawDataFile = p_raw_file;
  logFile = p_log_file;
  uintSize = sizeof(unsigned int);
  //public initial  
  memset(nEdge, 0, uintSize*MAXNUMBERMEZZANINE*24*2);
  memset(nPair, 0, uintSize*MAXNUMBERMEZZANINE*24);
  memset(nEvents, 0, uintSize*MAXNUMBERMEZZANINE);
  memset(nGoodHit, 0, uintSize*MAXNUMBERMEZZANINE*24);
  memset(nASDNoise, 0, uintSize*MAXNUMBERMEZZANINE*24);
  memset(timeSpectrum, 0, uintSize*MAXNUMBERMEZZANINE*24*4000);
  memset(timeWidth, 0, uintSize*MAXNUMBERMEZZANINE*24*512);
  memset(singleCounter, 0, uintSize*NUMBERANALYSISCOUNTER);
  singleCounter[MINEVENTSIZE] = 1000000;
  memset(analysisCounter, 0, uintSize*NUMBERANALYSISCOUNTER);
  analysisCounter[MINEVENTSIZE] = 1000000;

  numberFilledFIFOs = 0;
  requestForStop = FALSE;
  requestForPause = FALSE;

  nbMezzCard = 0;

  
  AMTDataMode0 = buffer[TCPEDGEPAIRENABLES0];
  AMTDataMode1 = buffer[TCPEDGEPAIRENABLES1];
  // if(filar_chnl==1){  //enable from TCP for filar_chnl 1
  //   enables = buffer[TCPEDGEPAIRENABLES0];
  //   mezzEnables = buffer[TCPMEZZENABLES];
  //   for (i = 0; i < 10; i++) {
  //     mezzCardEnable[i] = (mezzEnables >> i) & 1;
  //     if (mezzCardEnable[i]) nbMezzCard++;
  //     lEdgeOn[i] = (enables >> (3*i + LEADINGEDGEENABLE)) & 1;
  //     tEdgeOn[i] = (enables >> (3*i + TRAILINGEDGEENABLE)) & 1;
  //     pairOn[i] = (enables >> (3*i + PAIRENABLE)) & 1;
  //     // printf("mezzCardEnable[%d]=%d, lEdgeOn[%d]=%d, tEdgeOn[%d]=%d, pairOn[%d]=%d\n",
  //     //   i,mezzCardEnable[i],i,lEdgeOn[i],i,tEdgeOn[i],i,pairOn[i]);
  //   }
  //   enables = buffer[TCPEDGEPAIRENABLES1];    
  //   for (i = 10; i < MAXNUMBERMEZZANINE; i++) {
  //     mezzCardEnable[i] = (mezzEnables >> i) & 1;
  //     if (mezzCardEnable[i]) nbMezzCard++;
  //     lEdgeOn[i] = (enables >> (3*(i%10) + LEADINGEDGEENABLE)) & 1;
  //     tEdgeOn[i] = (enables >> (3*(i%10) + TRAILINGEDGEENABLE)) & 1;
  //     pairOn[i] = (enables >> (3*(i%10) + PAIRENABLE)) & 1;
  //   }
  // }
  if(filar_chnl==1){
    for(i=0;i<MAXNUMBERMEZZANINE;i++){
      mezzCardEnable[i]= 0;
      lEdgeOn[i] = 0;
      tEdgeOn[i] = 0;
      pairOn[i] = 0;
    }
    mezzCardEnable[1]= 1;
    lEdgeOn[1] = 1;
    tEdgeOn[1] = 1;
  }

  if(filar_chnl==2){
    for(i=0;i<MAXNUMBERMEZZANINE;i++){
      mezzCardEnable[i]= 0;
      lEdgeOn[i] = 0;
      tEdgeOn[i] = 0;
      pairOn[i] = 0;
    }
    mezzCardEnable[1]= 1;
    lEdgeOn[1] = 1;
    tEdgeOn[1] = 1;
  }

  //private initial  
  filar_chnl_no = filar_chnl;
  bufnr = 0;
  if ((strlen(CSMDataFilename) > 4) && (strstr(CSMDataFilename, "/dev/null") == NULL)) {
    strncpy(eventFilename, CSMDataFilename, strlen(CSMDataFilename)-4);
    eventFilename[strlen(CSMDataFilename)-4] = '\0';
  }
  else strcpy(eventFilename, "sample_data");
  strcat(eventFilename, ".evt");
  sockWriteCount = 0;
  bytes = 0;
  totalBytes = 0;
  fileSize = 0;
  rawFileSize = 0;
  nPackedEvt = 0;
  nDataWords = 0;
  evtNumber = 0;
  reconEVID = 0;
  evtRDBuf = 0;
  triggerNumber = 0;
  gotTriggerLow = FALSE;
  previousTriggerTime = 0;

  for (i = 0; i < 1000000; i++) trigTHist[i] = 0;
  for (i = 0; i < 64; i++) {
    eventBuf[0][i] = 0;
    bufEmpty[i] = TRUE;
    evtError[i] = 0;
    evtWarning[i] = 0;
    gotTrailer[i] = 0;
    gotHeader[i] = 0;
    evtWRIndex[i] = 0;
    for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
      evtSize[mezz][i] = 0;
      nTDCHits[mezz][i] = 0;
      evtCSMFIFOOV[mezz][i] = 0;
    }
    trigOverflow[i] = FALSE;
    gotBadTDCIDAtHD[i] = FALSE;
    gotBadTDCIDAtTR[i] = FALSE;
    beginEvent[i] = 0;
    printEvent[i] = 0;
  }
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; mezz++) {
    evtWRBuf[mezz] = 0;
    nTDCHeader[mezz] = 0;
    nTDCTrailer[mezz] = 0; 
    nTDCEdge[mezz] = 0;
    nTDCPair[mezz] = 0; 
    nTDCMask[mezz] = 0;
    nTDCError[mezz] = 0; 
    nTDCDebug[mezz] = 0;
    nTDCOthers[mezz] = 0;
    previousData[mezz] = 0;
    nReceivedData[mezz] = 0; 
    nUnexpected[mezz] = 0; 
  }
  nCommParityError = 0;
  collectHits = TRUE;
  collectedEVT = 0;
  checkCSMOverflows = FALSE;
  if ((CSMVersion > 0x20) || (CSMVersion < 0x10)) checkCSMOverflows = TRUE; 
  checkExpectedData = FALSE;
  if ((CSMVersion > 0x01) && (CSMVersion < 0x10)) checkExpectedData = TRUE; 
  ndata = -1;
  channel = MAXNUMBERMEZZANINE;
  badCycleAtBeginning = FALSE;
  
  DAQState = State_Running;
  startTime0 = (unsigned int) time(NULL);
  localTime = time(NULL);
  localDateTime = localtime(&localTime);
  strftime(myDateTime, 30, "%Y%m%d", localDateTime);
  sscanf(myDateTime,"%x", &DAQStartDate);
  strftime(myDateTime, 30, "%H%M%S", localDateTime);
  sscanf(myDateTime,"%x", &DAQStartTime);
  strftime(DAQStartedAt, 30, "%Y:%m:%d %H:%M:%S", localDateTime);
  DAQStartedAt[20] = '\0';
  saveRunCondition = 0;
  printf("\tWaiting for data...");
}


void CollectCSMData::DataAssembling(){
  // Read the ACK FIFO to get the size of the data packet in the data
  // buffer.  It had better always be the same for all that we grab!
  // fsize in 32bit words, 1M word max. And get a pointer to the data
  // datasize = (filar->ack1) & (0xFFFFF);
  // dataptr = (unsigned int *) uaddr[1][bufnr];
  switch(filar_chnl_no){
    case 1: datasize = (filar->ack1) & (0xFFFFF);dataptr = (unsigned int *) uaddr[1][bufnr];break;
    case 2: datasize = (filar->ack2) & (0xFFFFF);dataptr = (unsigned int *) uaddr[2][bufnr];break;
    case 3: datasize = (filar->ack3) & (0xFFFFF);dataptr = (unsigned int *) uaddr[3][bufnr];break;
    case 4: datasize = (filar->ack4) & (0xFFFFF);dataptr = (unsigned int *) uaddr[4][bufnr];break;
    default:break;
  }

  singleCounter[NTOTALWORD] += datasize;
  if (rawDataFile != NULL) {
    rawFileSize += datasize + 1;
    if ((rawFileSize*uintSize) >= 2000000000) {
      printf( "\n\n--> File size exceeds the file system limit (2GB)! <--\n" );
      printf( "--> No more raw data will be saved into disk\n");
      fclose(rawDataFile);
      printf("\nRaw Data file <%s> is closed.\n", rawDataFilename);
      rawDataFile = NULL;
    }
    else {
      fwrite(&datasize, sizeof(unsigned int), 1, rawDataFile);
      fwrite(dataptr, sizeof(unsigned int), datasize, rawDataFile);
    }
  }

  if (CSMVersion%2 == 1) {
    if (channel > MAXNUMBERMEZZANINE) channel %= MAXNUMBERMEZZANINE;
    data = *(dataptr + MAXNUMBERMEZZANINE - channel);
    if ((data&SWORDHEADERMASK) != SWORDHEADER) {
      for (i = 0; i <= MAXNUMBERMEZZANINE; i++) {
         data = *(dataptr + MAXNUMBERMEZZANINE - channel);
         if ((data&SWORDHEADERMASK) == SWORDHEADER) {
           channel = MAXNUMBERMEZZANINE - i;
           break;
         }
      }
      singleCounter[NBADCYCLEATBEGINNING]++;
      badCycleAtBeginning = TRUE;
    }
    else badCycleAtBeginning = FALSE;
    for (i = 0; i < datasize; i++) {
      data = *(dataptr + i);
      if ((data&CSMPARITYBITS) != 0) {
        mezz = channel % MAXNUMBERMEZZANINE;
        evtError[evtWRBuf[mezz]]++;
        singleCounter[CSMPARITYERROR]++;
        singleCounter[PARITYERRORINTDC+mezz]++;
      }
      if (synchWordControl == STATUSTRIGTIMEINSYNCHWORD) {
        if (data == (SWORDHEADER|0x05c00000)) synchWord = TRUE;
        else if ((data&SWORDHEADERMASK) == SWORDHEADER) {
          synchWord = TRUE;
          if (checkParity(data, &nCommParityError) != 0) {
            singleCounter[TXPARITYERROR]++;
            singleCounter[UNKNOWNSWORD];
          }
          else {
            if (((data&SWORDBIT27) != 0) || ((data&SWORDBIT25) != 0)) singleCounter[UNKNOWNSWORD]++;
            else {
              if ((data&SWORDLHCLOCKBIT) == 0) singleCounter[LHCCLOCKUNLOCKED]++;
              if ((data&SWORDXMTLOCK1BIT) == 0) singleCounter[XMT1CLOCKUNLOCKED]++;
              if ((data&SWORDXMTLOCK2BIT) == 0) singleCounter[XMT2CLOCKUNLOCKED]++;
              if ((data&SWORDPHASEERRORBIT) != 0) singleCounter[CSMPHASEERROR]++;
              if ((data&SWORDI2COPERATIONBIT) != 0) singleCounter[I2COPERATIONFAILED]++;
              if ((data&SWORDTTCRXI2CCHECKERROR) != 0) singleCounter[UNEXPECTEDTTCRXSETUP]++;
              if ((data&SWORDCSMERRORBIT) != 0) singleCounter[CSMHASERROR]++;
              control = data & SWORDCONTROLMASK;
              if (control == SWORDSTATUSONLY);         // Do nothing
              else {
                dataLow = data&SWORDTRIGGERTIMEMASK; 
                if (control == SWORDTRIGGERTIMELOW) {
                  singleCounter[NUMBERTRIGGERLOW]++;
                  previousTriggerTime = triggerTime;
                  triggerTime = dataLow;
                  if (gotTriggerLow) singleCounter[NUMBERUNMATCHEDTRIGGERLOW]++;
                  gotTriggerLow = TRUE;
                }
                else if (control == SWORDTRIGGERTIMEHIGH) {
                  singleCounter[NUMBERTRIGGERHIGH]++;
                  if (!gotTriggerLow) singleCounter[NUMBERUNMATCHEDTRIGGERHIGH]++;
                  else {
                    triggerTime |= (dataLow << 16);
                    if (triggerTime != previousTriggerTime) {
                      singleCounter[NUMBERTRIGGER]++;
                      triggerNumber++;
                    }
                    else singleCounter[NUMBERSAMETRIGGER]++;
                    if ((triggerNumber != 1) || (previousTriggerTime != 0)) {
                      if (triggerTime < previousTriggerTime) {
                        diff = 0xFFFFFFFF - previousTriggerTime;
                        diff += triggerTime;
                      }
                      else diff = triggerTime - previousTriggerTime;
                      if (diff >= 1000000) diff = 999999;
                      trigTHist[diff]++;
                    }
                  }
                  gotTriggerLow = FALSE;
                }
                else singleCounter[UNKNOWNSWORD]++;
              }              
            } 
          }
        }
        else synchWord = FALSE;
      }
      else {
        if (data == SWORDHEADER) synchWord = TRUE;
        else synchWord = FALSE;
      }
      if (synchWord) {
        if (badCycleAtBeginning) singleCounter[NBADCYCLE]++;
        else if (channel == MAXNUMBERMEZZANINE) {
          singleCounter[NGOODCYCLE]++;
          if (ndata == 0) singleCounter[NEMPTYCYCLE]++;
        }
        else singleCounter[NBADCYCLE]++;
        channel = 0;
        ndata = 0;
      }
      else if (data != CSMIDLEWORD) {
        ndata++;
        mezz = channel % MAXNUMBERMEZZANINE;
        if (checkParity(data, &nCommParityError) != 0) {
          evtError[evtWRBuf[mezz]]++;
          singleCounter[TXPARITYERROR]++;
          singleCounter[TXPARITYERRORINTDC+mezz]++;
        }
        overflow1 = (data&CSMFIFOOV1BITS);
        overflow2 = (data&CSMFIFOOV2BITS);
        data &= (~TDCNUMBERMASK);
        data |= ((mezz%16)<<TDCNUMBERBIT0LOCATION);
        singleCounter[NDATAWORD]++;
        wordID = (data>>MAINIDBIT0LOCATION) & MAINIDBITS;
        copyData = TRUE;
#ifdef CHECKEXPECTEDDATA
        if (checkExpectedData) {
          if (wordID == MYTDC_HEADER) {             // got a TDC header
            nReceivedData[mezz] = 0; 
            nUnexpected[mezz] = 0; 
          }
          else if (wordID == MYTDC_TRAILER) {       // got a TDC trailer
            if (!(overflow1 || overflow2)) {
              if (nUnexpected[mezz] > 0) {
                singleCounter[TDCUNEXPECTEDDATA] += nUnexpected[mezz];
                singleCounter[TDCUNEXPECTEDDATAINTDC+mezz]  += nUnexpected[mezz];
              }
            }
            nReceivedData[mezz] = 0; 
            nUnexpected[mezz] = 0;
            
          }
          else if ((wordID == MYTDC_EDGE) || (lEdgeOn[mezz] && (wordID == MYTDC_LEDGE)) ||
                   (tEdgeOn[mezz] && (wordID == MYTDC_TEDGE))) {       // got a TDC egde word
            nReceivedData[mezz]++;
            receivedData = *(dataptr + i) & 0xFFFFFFF;
            expected = 8*nReceivedData[mezz] | (1<<STYPEBIT0LOCATION);
            if (((receivedData-expected)&(~ODDPARITYBITS)) != 0) nUnexpected[mezz]++;
          }
          else if ((wordID == MYTDC_PAIR) || (wordID == MYTDC_MASK) ||
                   (wordID == MYTDC_ERROR) || (wordID == MYTDC_DEBUG)) {
            nReceivedData[mezz]++;
            receivedData = *(dataptr + i) & 0xFFFFFFF;
            expected = 8*nReceivedData[mezz] | (1<<STYPEBIT0LOCATION);
            if (((receivedData-expected)&(~ODDPARITYBITS)) == 0) nUnexpected[mezz]++;
          }
          else nReceivedData[mezz]++;
        }
#endif
        if (mezzCardEnable[mezz]) {
          if (wordID == MYTDC_HEADER) {             // got a TDC header
            if (suppressTDCHeader) copyData = FALSE;
            nTDCHeader[mezz]++;
            evid = (data>>EVIDBIT0LOCATION) & EVIDBITS;
            bcid[mezz] = (data>>BCIDBIT0LOCATION) & BCIDBITS;
            singleCounter[CURREVENTID] = evid;
            if (bufEmpty[evtWRBuf[mezz]]) {
              bufEmpty[evtWRBuf[mezz]] = FALSE;
              if (suppressCSMHeader && suppressCSMTrailer) {
                eventBuf[0][evtWRBuf[mezz]] = 0;
                evtWRIndex[evtWRBuf[mezz]] = 1;
              }
              else if (suppressCSMHeader) {
                eventBuf[0][evtWRBuf[mezz]] = 1;
                evtWRIndex[evtWRBuf[mezz]] = 1;
              }
              else if (suppressCSMTrailer) {
                eventBuf[0][evtWRBuf[mezz]] = 1;
                eventBuf[1][evtWRBuf[mezz]] = data & 0xFFFFFF;
                evtWRIndex[evtWRBuf[mezz]] = 2;
              }
              else {
                eventBuf[0][evtWRBuf[mezz]] = 2;
                eventBuf[1][evtWRBuf[mezz]] = data & 0xFFFFFF;
                evtWRIndex[evtWRBuf[mezz]] = 2;
              }
              bufEVID[evtWRBuf[mezz]] = evid;
              bufBCID[evtWRBuf[mezz]] = bcid[mezz];
              beginEvent[evtWRBuf[mezz]] = i;
            }
            else if (evid != bufEVID[evtWRBuf[mezz]]) {
              if (reqEVIDMatch) {
                nloop = 0;
                while (!bufEmpty[evtWRBuf[mezz]]) {
                  if (evid == bufEVID[evtWRBuf[mezz]]) break;
                  evtWRBuf[mezz]++;
                  evtWRBuf[mezz] %= 64;
                  if ((nloop++) >= 64) break;
                }
                if ((nloop++) >= 64) {
                  for(int i=0;i<18;i++)
                  {

                    printf("\nfilar_chnl_no=%d\n",filar_chnl_no);
                    printf("mezz[%d]=%d\n",i,mezzCardEnable[i]);
                  }
                  printf("\nEvent Builder Error: Try to build too many events (>64) at same time.\n");
                  printf("                     Unable to build event any more, stopping DAQ now.\n");
                  DAQState = State_Idle;
                  requestForStop = TRUE;
                  break;
                }
                if (bufEmpty[evtWRBuf[mezz]]) {
                  bufEmpty[evtWRBuf[mezz]] = FALSE;
                  if (suppressCSMHeader && suppressCSMTrailer) {
                    eventBuf[0][evtWRBuf[mezz]] = 0;
                    evtWRIndex[evtWRBuf[mezz]] = 1;
                  }
                  else if (suppressCSMHeader) {
                    eventBuf[0][evtWRBuf[mezz]] = 1;
                    evtWRIndex[evtWRBuf[mezz]] = 1;
                  }
                  else if (suppressCSMTrailer) {
                    eventBuf[0][evtWRBuf[mezz]] = 1;
                    eventBuf[1][evtWRBuf[mezz]] = data & 0xFFFFFF;
                    evtWRIndex[evtWRBuf[mezz]] = 2;
                  }
                  else {
                    eventBuf[0][evtWRBuf[mezz]] = 2;
                    eventBuf[1][evtWRBuf[mezz]] = data & 0xFFFFFF;
                    evtWRIndex[evtWRBuf[mezz]] = 2;
                  }
                  bufEVID[evtWRBuf[mezz]] = evid;
                  bufBCID[evtWRBuf[mezz]] = bcid[mezz];
                  beginEvent[evtWRBuf[mezz]] = i;
                }
              }
              else {
                evtError[evtWRBuf[mezz]]++;
                singleCounter[EVIDMISMATCH]++;
                singleCounter[TDCHEADEREVIDMISMATCH]++;
                singleCounter[TDCHEADEREVIDMISMATCHINTDC+mezz]++;
              }
            }
            if ((gotHeader[evtWRBuf[mezz]]&(1<<mezz)) != 0) {
              printEvent[evtWRBuf[mezz]] = EXTRATDCHEADER;
              evtError[evtWRBuf[mezz]]++;
              singleCounter[EXTRATDCHEADER]++;
              singleCounter[TDCHEADEREXTRATDCID+mezz]++;
            }
            else gotHeader[evtWRBuf[mezz]] |= (1 << mezz);
            for (chan = 0; chan < 24; chan++) {
              previousTDCTime[chan][mezz] = -1;
              previousEdgeErr[chan][mezz] = 0;
              previousEdge[chan][mezz] = -1;
            }
          }
          else if (wordID == MYTDC_MASK) {         // got a TDC mask word
            nTDCMask[mezz]++;
            singleCounter[TDCMASKWORD]++;
            singleCounter[TDCMASKWORDINTDC+mezz]++;
            if ((data&0xFFFFFF) == 0) {
              evtError[evtWRBuf[mezz]]++;
              singleCounter[TDCMASKEMPTY]++;
            }
          }
          else if (wordID == MYTDC_ERROR) {        // got a TDC error word
            if (checkCSMOverflows) { 
              if (overflow1 != 0) {
                evtCSMFIFOOV[mezz][evtWRBuf[mezz]]++;
                singleCounter[CSMFIFOOVERFLOW1]++;
                singleCounter[CSMFIFOOVERFLOW1INTDC+mezz]++;
              }
              if (overflow2 != 0) {
                evtCSMFIFOOV[mezz][evtWRBuf[mezz]] += 10000;
                singleCounter[CSMFIFOOVERFLOW2]++;
                singleCounter[CSMFIFOOVERFLOW2INTDC+mezz]++;
              }
              if ((overflow1 != 0) && (overflow2 != 0)) {
                singleCounter[CSMFIFOOVERFLOWS]++;
                singleCounter[CSMFIFOOVERFLOWSINTDC+mezz]++;
              }
            }
            nTDCError[mezz]++;
            singleCounter[TDCERROR]++;
            singleCounter[TDCERRORINTDC+mezz]++;
            if ((data&errWordMask) == 0) {
              evtError[evtWRBuf[mezz]]++;
              singleCounter[TDCERROREMPTY]++;
            }
            else {
              evtWarning[evtWRBuf[mezz]]++;
              // Bit01 in TDCError word is for trigger FIFO overflow for AMT1
              // Bit10 in TDCError word is for trigger FIFO overflow in AMT2/AMT3
              if (AMTType == TYPEAMT1) {
                if ((data&2) != 0) trigOverflow[evtWRBuf[mezz]] = TRUE;
                singleCounter[TDCSOFTERROR]++;
              }
              else if (AMTType == TYPEHPTDC) {
                if ((data&(1 << 13)) != 0) trigOverflow[evtWRBuf[mezz]] = TRUE;
                if ((data&0x3FFF) != 0) singleCounter[TDCSOFTERROR]++;
                if ((data&(1 << 14)) != 0) {
                  evtError[evtWRBuf[mezz]]++;
                  singleCounter[TDCHARDERROR]++;
                  singleCounter[HPTDCFATALCHIPERROR]++;
                  singleCounter[HPTDCFATALCHIPERRORINTDC+mezz]++;
                }
              }
              else {
                if ((data&(1 << 10)) != 0) trigOverflow[evtWRBuf[mezz]] = TRUE;
                if ((data&0x1FF) != 0) singleCounter[TDCHARDERROR]++;
                if ((data&0x3E00) != 0) singleCounter[TDCSOFTERROR]++;
              }
              for (j = 0; j < 14; j++) {
                if ((data&(1 << j)) != 0) {
                  if ((AMTType != TYPEAMT1) && (j < 9)) evtError[evtWRBuf[mezz]]++;
                  singleCounter[TDCERRORLIST+j]++;
                }
              }
            }
          }
          else if (wordID == MYTDC_DEBUG) {      // got a TDC debug word
            nTDCDebug[mezz]++;
            singleCounter[TDCDEBUGWORD]++;
            singleCounter[TDCDEBUGWORDINTDC+mezz]++;
          }
          else if ((wordID == MYTDC_EDGE) || (lEdgeOn[mezz] && (wordID == MYTDC_LEDGE)) ||
                   (tEdgeOn[mezz] && (wordID == MYTDC_TEDGE))) {       // got a TDC egde word
            nTDCEdge[mezz]++;
            nTDCHits[mezz][evtWRBuf[mezz]]++;
            chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
            if (HPTDC) {
              edgeErrBit = 0;  // No such bit from HPTDC
              tdcTime = (data>>STDCTIMEBIT0LOCATION) & SHPTDCTIMEBITS;
            }
            else {
              edgeErrBit = (data>>SERRORBIT0LOCATION) & SERRORBITS;
              if (edgeErrBit == 1) {
                evtWarning[evtWRBuf[mezz]]++;
                singleCounter[TDCEDGEERROR]++;
                singleCounter[TDCEDGEERRORINTDC+mezz]++;
              }
              tdcTime = (data>>STDCTIMEBIT0LOCATION) & STDCTIMEBITS;
            }
            tdcTime /= TDCDivider;
            if (tdcTime <= previousTDCTime[chan][mezz]) {
              evtWarning[evtWRBuf[mezz]]++;
              singleCounter[TDCTIMEWRONGORDER]++;
              singleCounter[TDCTIMEWRONGORDERINTDC+mezz]++;
            }
            if ((tdcTime-previousTDCTime[chan][mezz]) < 20) {
              if (edgeErrBit == 1) {
                singleCounter[TDCEDGEERRW20]++;
                singleCounter[TDCEDGEERRW20INTDC+mezz]++;
              }
              edgeErrBit = 0;
              if (previousEdgeErr[chan][mezz] == 1) {
                singleCounter[TDCEDGEERRW20]++;
                singleCounter[TDCEDGEERRW20INTDC+mezz]++;
              }
            }
            if (HPTDC) {
              if (wordID == MYTDC_LEDGE) edge = 1;
              else edge = 0;
            }
            else edge = (data>>STYPEBIT0LOCATION) & STYPEBITS;
            if (collectHits) nEdge[edge][chan][mezz]++;
            if (edge == 1) {
              if (tdcTime < 4000) timeSpectrum[tdcTime][chan][mezz]++;
              else timeSpectrum[3999][chan][mezz]++;
              if (lEdgeOn[mezz] == 0) {
                evtError[evtWRBuf[mezz]]++;
                singleCounter[TDCUNEXPECTEDDATA]++;
                singleCounter[TDCUNEXPECTEDDATAINTDC+mezz]++;
              }
            }
            else {
              if (previousEdge[chan][mezz] == 1) {
                width = tdcTime - previousTDCTime[chan][mezz];
                if (width < 0) width = 0;
                else if (width > 511) width = 511;
                timeWidth[width][chan][mezz]++;
                if (collectHits) {
                  if (width > minWidth) nGoodHit[chan][mezz]++;
                  else nASDNoise[chan][mezz]++;
                }
              }
              else timeWidth[0][chan][mezz]++;
              if (tEdgeOn[mezz] == 0) {
                evtError[evtWRBuf[mezz]]++;
                singleCounter[TDCUNEXPECTEDDATA]++;
                singleCounter[TDCUNEXPECTEDDATAINTDC+mezz]++;
              }
            }
            if ((lEdgeOn[mezz] == 1) && (tEdgeOn[mezz] == 1) && (edge == previousEdge[chan][mezz])) {
              evtWarning[evtWRBuf[mezz]]++;
              singleCounter[TDCCONTSAMEEDGE]++;
              singleCounter[TDCCONTSAMEEDGEINTDC+mezz]++;
              if (edge == 0) {
                singleCounter[TDCCONTTRAILINGEDGE]++;
                singleCounter[TDCCONTTRAILINGEDGEINTDC+mezz]++;
              }
            }
            previousEdge[chan][mezz] = edge;
            previousTDCTime[chan][mezz] = tdcTime;
            previousEdgeErr[chan][mezz] = edgeErrBit;
            tdc = mezz % 16;
            data &= (~WIRENUMBERMASK);
            if (mezz == 16) {
              if (chan < 8) tdc = 0;
              else if (chan < 16) tdc = 1;
              else tdc = 2;
              chan %= 8;
              chan += 24;
            }
            else if (mezz == 17) {
              if (chan < 8) tdc = 4;
              else if (chan < 16) tdc = 5;
              else tdc = 6;
              chan %= 8;
              chan += 24;
            }
            data |= ((tdc<<TDCNUMBERBIT0LOCATION) | (chan<<CHANNELNUMBERBIT0LOCATION));
          }
          else if (wordID == MYTDC_PAIR) {        // got a TDC pair word
            nTDCPair[mezz]++;
            nTDCHits[mezz][evtWRBuf[mezz]]++;
            chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
            if (HPTDC) tdcTime = (data>>PHPTDCTIMEBIT0LOCATION) & PHPTDCTIMEBITS;
            else tdcTime = (data>>PTDCTIMEBIT0LOCATION) & PTDCTIMEBITS;
            tdcTime /= TDCDivider;
            if (tdcTime <= previousTDCTime[chan][mezz]) {
              evtWarning[evtWRBuf[mezz]]++;
              singleCounter[TDCTIMEWRONGORDER]++;
              singleCounter[TDCTIMEWRONGORDERINTDC+mezz]++;
            }
            previousTDCTime[chan][mezz] = tdcTime;
            if (tdcTime < 4000) timeSpectrum[tdcTime][chan][mezz]++;
            else timeSpectrum[3999][chan][mezz]++;
            if (HPTDC) width = (data>>PWIDTHHPTDCBIT0LOCATION) & PWIDTHHPTDCBITS;
            else width = (data>>PWIDTHBIT0LOCATION) & PWIDTHBITS;
            width /= widthDivider;
            timeWidth[width][chan][mezz]++;
            if (collectHits) {
              nPair[chan][mezz]++;
              if (width > minWidth) nGoodHit[chan][mezz]++;
              else nASDNoise[chan][mezz]++;
            }
            if (pairOn[mezz] == 0) {
              evtError[evtWRBuf[mezz]]++;
              singleCounter[TDCUNEXPECTEDDATA]++;
              singleCounter[TDCUNEXPECTEDDATAINTDC+mezz]++;
            }
            tdc = mezz % 16;
            data &= (~WIRENUMBERMASK);
            if (mezz == 16) {
              if (chan < 8) tdc = 0;
              else if (chan < 16) tdc = 1;
              else tdc = 2;
              chan %= 8;
              chan += 24;
            }
            else if (mezz == 17) {
              if (chan < 8) tdc = 4;
              else if (chan < 16) tdc = 5;
              else tdc = 6;
              chan %= 8;
              chan += 24;
            }
            data |= ((tdc<<TDCNUMBERBIT0LOCATION) | (chan<<CHANNELNUMBERBIT0LOCATION));
          }
          else if (wordID == MYTDC_TRAILER) {       // got a TDC trailer
            if (checkCSMOverflows) { 
              if (overflow1 != 0) {
                evtCSMFIFOOV[mezz][evtWRBuf[mezz]]++;
                singleCounter[CSMFIFOOVERFLOW1]++;
                singleCounter[CSMFIFOOVERFLOW1INTDC+mezz]++;
              }
              if (overflow2 != 0) {
                evtCSMFIFOOV[mezz][evtWRBuf[mezz]] += 10000;
                singleCounter[CSMFIFOOVERFLOW2]++;
                singleCounter[CSMFIFOOVERFLOW2INTDC+mezz]++;
              }
              if ((overflow1 != 0) && (overflow2 != 0)) {
                singleCounter[CSMFIFOOVERFLOWS]++;
                singleCounter[CSMFIFOOVERFLOWSINTDC+mezz]++;
              }
            }
            if (suppressTDCTrailer) copyData = FALSE;
            nTDCTrailer[mezz]++;
            evid = (data>>EVIDBIT0LOCATION) & EVIDBITS;
            singleCounter[CURREVENTID] = evid;
            if (evid != bufEVID[evtWRBuf[mezz]]) {
              evtError[evtWRBuf[mezz]]++;
              singleCounter[EVIDMISMATCH]++;
              singleCounter[TDCTRAILEREVIDMISMATCH]++;
              singleCounter[TDCTRAILEREVIDMISMATCHINTDC+mezz]++;
            }
            if (bcid[mezz] != bufBCID[evtWRBuf[mezz]]) {
              if (!trigOverflow[evtWRBuf[mezz]]) {
                printEvent[evtWRBuf[mezz]] = TDCBCIDMISMATCH;
                evtError[evtWRBuf[mezz]]++;
                singleCounter[BCIDMISMATCH]++;
                singleCounter[TDCBCIDMISMATCH]++;
                singleCounter[TDCBCIDMISMATCHINTDC+mezz]++;
              }
              else singleCounter[NBCIDERRDUETRIGFIFOOV]++;
            }            
            if ((gotTrailer[evtWRBuf[mezz]]&(1<<mezz)) != 0) {
              printEvent[evtWRBuf[mezz]] = EXTRATDCTRAILER;
              evtError[evtWRBuf[mezz]]++;
              singleCounter[EXTRATDCTRAILER]++;
              singleCounter[TDCTRAILEREXTRATDCID+mezz]++;
            }
            else gotTrailer[evtWRBuf[mezz]] |= (1 << mezz);
            if (data == previousData[mezz]) notSameTrailer = FALSE;
            else notSameTrailer = TRUE;
          }
          else {
            nTDCOthers[mezz]++;
            evtError[evtWRBuf[mezz]]++;
            singleCounter[EXTRAWORD]++;
            singleCounter[EXTRAWORDINTDC+mezz]++;
          }
          countData = TRUE;
          if (bufEmpty[evtWRBuf[mezz]] && (data == previousData[mezz])) {
            countData = FALSE;
            copyData = FALSE;
            if (wordID == MYTDC_HEADER) {
              singleCounter[EXTRATDCHEADER]++;
              singleCounter[TDCHEADEREXTRATDCID+mezz]++;
            }
            else if (wordID == MYTDC_TRAILER) {
              singleCounter[EXTRATDCTRAILER]++;
              singleCounter[TDCTRAILEREXTRATDCID+mezz]++;
            }
          }
          else if (bufEmpty[evtWRBuf[mezz]]) {
            singleCounter[NBUFFERPOINTERERROR]++;
            if (analysisCounter[NBUFFERPOINTERERROR] < 100) {
              printf("\nEvent buffer pointer error: mezz = %d, evtWRBuf[mezz] = %d\n", mezz, evtWRBuf[mezz]);
              printf("evtNumber = 0x%08x, data = 0x%08x, numberFilledFIFOs = %d\n", evtNumber, data, numberFilledFIFOs);
              printf("        i = %10d, data = 0x%08x\n", i, *(dataptr+i));
              printf("evtRDBuf = %d, evtSize[mezz][evtWRBuf[mezz]] = %d\n", evtRDBuf, evtSize[mezz][evtWRBuf[mezz]]);
              printf("bufEVID[evtWRBuf[mezz]] = 0x%03x, bufBCID[evtWRBuf[mezz]] = 0x%03x\n", bufEVID[evtWRBuf[mezz]], bufBCID[evtWRBuf[mezz]]);
              //for (j = 0; j < i+10; j++) printf("%8d 0x%08x\n", j, *(dataptr + j));
            }
            bufEmpty[evtWRBuf[mezz]] = FALSE;
            if (suppressCSMHeader && suppressCSMTrailer) {
              eventBuf[0][evtWRBuf[mezz]] = 0;
              evtWRIndex[evtWRBuf[mezz]] = 1;
            }
            else if (suppressCSMHeader) {
              eventBuf[0][evtWRBuf[mezz]] = 1;
              evtWRIndex[evtWRBuf[mezz]] = 1;
            }
            else if (suppressCSMTrailer) {
              eventBuf[0][evtWRBuf[mezz]] = 1;
              eventBuf[1][evtWRBuf[mezz]] = data & 0xFFFFFF;
              evtWRIndex[evtWRBuf[mezz]] = 2;
            }
            else {
              eventBuf[0][evtWRBuf[mezz]] = 2;
              if ((wordID == MYTDC_HEADER) || (wordID == MYTDC_TRAILER)) eventBuf[1][evtWRBuf[mezz]] = (evid) << EVIDBIT0LOCATION;
              else eventBuf[1][evtWRBuf[mezz]] = (evtNumber%4096) << EVIDBIT0LOCATION;
              evtWRIndex[evtWRBuf[mezz]] = 2;
            }
            if ((wordID == MYTDC_HEADER) || (wordID == MYTDC_TRAILER)) bufEVID[evtWRBuf[mezz]] = evid;
            else bufEVID[evtWRBuf[mezz]] = evtNumber%4096;
            bufBCID[evtWRBuf[mezz]] = 0;
            beginEvent[evtWRBuf[mezz]] = i;
          }
          previousData[mezz] = data;
          if (copyData) {
            eventBuf[0][evtWRBuf[mezz]]++;
            if (evtWRIndex[evtWRBuf[mezz]] < MAXEVTRECORDM2)
              eventBuf[evtWRIndex[evtWRBuf[mezz]]++][evtWRBuf[mezz]] = data;
            else if (eventBuf[0][evtWRBuf[mezz]] < (MAXEVTRECORD+100))
              printf("mezz = %d, evtWRBuf[mezz] = %d, eventBuf[0][evtWRBuf[mezz]] = %u, data = 0x%08x\n", mezz, evtWRBuf[mezz], eventBuf[0][evtWRBuf[mezz]], data);
            else if (eventBuf[0][evtWRBuf[mezz]] > (MAXEVTRECORD+0x3FFFFFFF)) {
              printf("mezz = %d, evtWRBuf[mezz] = %d, eventBuf[0][evtWRBuf[mezz]] = %u, data = 0x%08x\n", mezz, evtWRBuf[mezz], eventBuf[0][evtWRBuf[mezz]], data);
              printf("\nEvent Builder Error: Too many junk data at same time.\n");
              printf("                     Unable to build event any more, stopping DAQ now.\n");
              DAQState = State_Idle;
              requestForStop = TRUE;
              break;
            }
          }
          if (countData) evtSize[mezz][evtWRBuf[mezz]]++;
          if (wordID == MYTDC_TRAILER) {           // got a TDC trailer
            wc = (data>>WORDCOUNTBIT0LOCATION) & WORDCOUNTBITS;
            if (checkCSMOverflows) { 
              overflow1 = evtCSMFIFOOV[mezz][evtWRBuf[mezz]]%10000;
              overflow2 = evtCSMFIFOOV[mezz][evtWRBuf[mezz]]/10000;
              diff = wc - evtSize[mezz][evtWRBuf[mezz]];
              if ((diff == 0) && (evtCSMFIFOOV[mezz][evtWRBuf[mezz]] != 0)) {
                printEvent[evtWRBuf[mezz]] = WRONGCSMFIFOOVERFLOW;
                evtError[evtWRBuf[mezz]]++;
                singleCounter[WRONGCSMFIFOOVERFLOW]++;
                singleCounter[WRONGCSMFIFOOVERFLOWINTDC+mezz]++;
              }
              else if (diff > 0) {
                if (evtCSMFIFOOV[mezz][evtWRBuf[mezz]] == 0) {
                  evtError[evtWRBuf[mezz]]++;
                  singleCounter[TDCWCMISMATCH]++;
                  singleCounter[TDCWCMISMATCHLIST+mezz]++;
                  singleCounter[WORDCOUNTERERROR]++;
                }
                else {
                  singleCounter[EXPECTEDTDCWCERROR]++;
                  singleCounter[EXPECTEDTDCWCERRORINTDC+mezz]++;
                  singleCounter[DISCARDEDTDCWORDS] += diff;
                  singleCounter[DISCARDEDTDCWORDSINTDC+mezz] += diff;
                  if ((overflow1 != 0) && (overflow2 == 0)) {
                    singleCounter[DISCARDEDTRAILINGONLY] += diff;
                    singleCounter[DISCARDEDTRAILINGONLYINTDC+mezz] += diff;
                  }
                  else if ((overflow1 == 0) && (overflow2 != 0)) {
                    singleCounter[DISCARDEDLEADINGONLY] += diff;
                    singleCounter[DISCARDEDLEADINGONLYINTDC+mezz] += diff;
                  }
                }
              }
              else if (diff != 0) {
                evtError[evtWRBuf[mezz]]++;
                singleCounter[TDCWCMISMATCH]++;
                singleCounter[TDCWCMISMATCHLIST+mezz]++;
                singleCounter[WORDCOUNTERERROR]++;
                if (wc < evtSize[mezz][evtWRBuf[mezz]]) {
                  printEvent[evtWRBuf[mezz]] = UNEXPECTEDTDCWCERROR;
                  singleCounter[UNEXPECTEDTDCWCERROR]++;
                  singleCounter[UNEXPECTEDTDCWCERRORINTDC+mezz]++;
                }
              }
            }
            else if (wc != evtSize[mezz][evtWRBuf[mezz]]) {
              evtError[evtWRBuf[mezz]]++;
              singleCounter[TDCWCMISMATCH]++;
              singleCounter[TDCWCMISMATCHLIST+mezz]++;
              singleCounter[WORDCOUNTERERROR]++;
              if (wc < evtSize[mezz][evtWRBuf[mezz]]) {
                 singleCounter[UNEXPECTEDTDCWCERROR]++;
                 singleCounter[UNEXPECTEDTDCWCERRORINTDC+mezz]++;
              }
            }
            if (gotTrailer[evtWRBuf[mezz]] == mezzEnables) {  // Event ready
              if (gotHeader[evtWRBuf[mezz]] != mezzEnables) {
                for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
                  got = (gotHeader[evtWRBuf[mezz]] >> j) & 1;
                  if ((got == 0) && (mezzCardEnable[j] == 1)) {
                    // Missing TDC Header
                    evtError[evtWRBuf[mezz]]++;
                    singleCounter[MISSTDCHEADER]++;
                    singleCounter[TDCHEADERMISSTDCID+j]++;
                  }
                }
              }
              nhits = 0;
              nothers = 0;
              for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
                if (mezzCardEnable[j] == 1) {
                  nhits += nTDCHits[j][evtWRBuf[mezz]];
                  if (nTDCHits[j][evtWRBuf[mezz]] == 0) {
                    singleCounter[EVENTNOTDCHITSLIST+j]++;
                    if (evtSize[j][evtWRBuf[mezz]] <= 2) singleCounter[EVENTONLYTDCHDANDTRLIST+j]++;
                    else nothers++;
                  }
                  nEvents[j] = collectedEVT;
                  if (nEvents[j] > 0) nEvents[j]++;
                }
                evtSize[j][evtWRBuf[mezz]] = 0;
                nTDCHits[j][evtWRBuf[mezz]] = 0;
                evtCSMFIFOOV[j][evtWRBuf[mezz]] = 0;
              }
              if (nhits == 0) {
                singleCounter[EVENTNOTDCHITS]++;
                if (nothers == 0) singleCounter[EVENTONLYTDCHDANDTR]++;
              }
              if ((numberOfEvent != 0) && (collectedEVT >= numberOfEvent)) collectHits = FALSE;
              else collectedEVT++;
              if (gotBadTDCIDAtHD[evtWRBuf[mezz]]) {
                evtError[evtWRBuf[mezz]]++;
                singleCounter[TDCHEADERWRONGTDCID]++;
              }
              if (gotBadTDCIDAtTR[evtWRBuf[mezz]]) {
                evtError[evtWRBuf[mezz]]++;
                singleCounter[TDCTRAILERWRONGTDCID]++;
              }
              if (evtError[evtWRBuf[mezz]] > 0) singleCounter[WRONGEVENT]++;
              if (evtWarning[evtWRBuf[mezz]] > 0) singleCounter[WARNINGEVENT]++;
              if (saveFlaggedEvent && (printEvent[evtWRBuf[mezz]] > 0) && (beginEvent[evtWRBuf[mezz]] < i)) {
                if (eventFile = fopen(eventFilename, "r")) {
                  fclose(eventFile);
                  eventFile = fopen(eventFilename, "a");
                  if (eventFile == NULL) printf("\n");
                  else fprintf(eventFile, "\n");
                }
                else eventFile = fopen(eventFilename, "w");
                if (eventFile != NULL) {
                  fprintf(eventFile, "New Print Out : ");
                  if (printEvent[evtWRBuf[mezz]] == TDCBCIDMISMATCH) {
                    fprintf(eventFile, "TDC BCID Mismatch\n");
                  }
                  else if (printEvent[evtWRBuf[mezz]] == WRONGCSMFIFOOVERFLOW) {
                    fprintf(eventFile, "Wrong CSM FIFO Overflow Bit\n");
                  }
                  else if (printEvent[evtWRBuf[mezz]] == UNEXPECTEDTDCWCERROR) {
                    fprintf(eventFile, "TDC WC < # Received Words\n");
                  }
                  else if (printEvent[evtWRBuf[mezz]] == EXTRATDCHEADER) {
                    fprintf(eventFile, "Extra TDC Header\n");
                  }
                  else if (printEvent[evtWRBuf[mezz]] == EXTRATDCTRAILER) {
                    fprintf(eventFile, "Extra TDC Trailer\n");
                  }
                  else {
                    fprintf(eventFile, "----------\n");
                  }
                  fprintf(eventFile, "  Index   dataWord\n");
                }
                else {
                  printf("\n");
                  printf("New Print Out : ");
                  if (printEvent[evtWRBuf[mezz]] == TDCBCIDMISMATCH) {
                    printf("TDC BCID Mismatch\n");
                  }
                  else if (printEvent[evtWRBuf[mezz]] == WRONGCSMFIFOOVERFLOW) {
                    printf("Wrong CSM FIFO Overflow Bit\n");
                  }
                  else if (printEvent[evtWRBuf[mezz]] == UNEXPECTEDTDCWCERROR) {
                    printf("TDC WC < # Received Words\n");
                  }
                  else {
                    printf("----------\n");
                  }
                  printf("  Index   dataWord\n");
                }
                ndata = 0;
                wc = 0;
                for (j = beginEvent[evtWRBuf[mezz]]; j <= i; j++) {
                  if ((*(dataptr+j)&SWORDHEADERMASK) == SWORDHEADER) break;
                  wc++;
                  if (wc > MAXNUMBERMEZZANINE) break;
                }
                if ((wc > 0) && (wc <= MAXNUMBERMEZZANINE)) {
                  if (eventFile == NULL) printf(" %6d 0x%08x  ", ndata, SWORDHEADER);
                  else fprintf(eventFile, " %6d 0x%08x  ", ndata, SWORDHEADER);
                  RawDataInterpretation(SWORDHEADER, eventFile);
                  ndata++;
                  for (j = wc; j < MAXNUMBERMEZZANINE; j++) {
                    if (eventFile == NULL) printf(" %6d 0x%08x  ", ndata, CSMIDLEWORD);
                    else fprintf(eventFile, " %6d 0x%08x  ", ndata, CSMIDLEWORD);
                    RawDataInterpretation(CSMIDLEWORD, eventFile);
                    ndata++;
                  }
                }
                for (j = beginEvent[evtWRBuf[mezz]]; j <= i; j++) {
                  if (eventFile == NULL) printf(" %6d 0x%08x  ", ndata, *(dataptr+j));
                  else fprintf(eventFile, " %6d 0x%08x  ", ndata, *(dataptr+j));
                  RawDataInterpretation(*(dataptr+j), eventFile);
                  ndata++;
                }
                if (eventFile != NULL) fclose(eventFile);
                else printf("\n");
              }
              evtError[evtWRBuf[mezz]] = 0;
              evtWarning[evtWRBuf[mezz]] = 0;
              gotHeader[evtWRBuf[mezz]] = 0;
              gotTrailer[evtWRBuf[mezz]] = 0;
              trigOverflow[evtWRBuf[mezz]] = FALSE;
              gotBadTDCIDAtHD[evtWRBuf[mezz]] = FALSE;
              gotBadTDCIDAtTR[evtWRBuf[mezz]] = FALSE;
              beginEvent[evtWRBuf[mezz]] = 0;
              printEvent[evtWRBuf[mezz]] = FALSE;
              nwords = eventBuf[0][evtWRBuf[mezz]];
              singleCounter[NEVENTWORD] += nwords;
              singleCounter[CURREVENTSIZE] = nwords;
              if (nwords > singleCounter[MAXEVENTSIZE]) singleCounter[MAXEVENTSIZE] = nwords;
              if (nwords < singleCounter[MINEVENTSIZE]) singleCounter[MINEVENTSIZE] = nwords;
              singleCounter[ANALYSEDEVENT]++;
              singleCounter[PROCESSEDEVENT]++;
              if (evtWRBuf[mezz] != evtRDBuf) {
                printf("\nEvent Builder Error: mezz=%d, evtWRBUF[mezz]=%d, evtRDBuf=%d\n", mezz, evtWRBuf[mezz], evtRDBuf);
                printf("                     will force event(s) out.\n");
                printf("evtNumber = 0x%08x, data = 0x%08x, numberFilledFIFOs = %d\n", evtNumber, data, numberFilledFIFOs);
                printf("        i = %10d, data = 0x%08x\n", i, *(dataptr+i));
                //for (j = 0; j < i+10; j++) printf("%8d 0x%08x\n", j, *(dataptr + j)); 
                while (evtWRBuf[mezz] != evtRDBuf) {
                  singleCounter[DISCARDEDEVENT]++;
                  nHeader = 0;
                  nTrailer = 0;
                  for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
                    if (((gotHeader[evtRDBuf] >> j) & 1) == 1) nHeader++;
                    if (((gotTrailer[evtRDBuf] >> j) & 1) == 1) nHeader++;
                  }
                  if ((nbMezzCard > 1) && ((nHeader+nTrailer) == 1)) {
                    for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
                      got = (gotHeader[evtRDBuf] >> j) & 1;
                      if (got == 1) {
                        // Unexpected header, counted as extra
                        evtError[evtRDBuf]++;
                        singleCounter[EXTRATDCHEADER]++;
                        singleCounter[TDCHEADEREXTRATDCID+j]++;
                      }
                    }
                    for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
                      got = (gotTrailer[evtRDBuf] >> j) & 1;
                      if (got == 1) {
                        // Unexpected header, counted as extra
                        evtError[evtRDBuf]++;
                        singleCounter[EXTRATDCTRAILER]++;
                        singleCounter[TDCTRAILEREXTRATDCID+j]++;
                      }
                    }
                  }
                  else {
                    if (gotHeader[evtRDBuf] != mezzEnables) {
                      for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
                        got = (gotHeader[evtRDBuf] >> j) & 1;
                        if ((got == 0) && (mezzCardEnable[j] == 1)) {
                          // Missing TDC Header
                          evtError[evtRDBuf]++;
                          singleCounter[MISSTDCHEADER]++;
                          singleCounter[TDCHEADERMISSTDCID+j]++;
                        }
                      }
                    }
                    if (gotTrailer[evtRDBuf] != mezzEnables) {
                      for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
                        got = (gotTrailer[evtRDBuf] >> j) & 1;
                        if ((got == 0) && (mezzCardEnable[j] == 1)) {
                          // Missing TDC Trailer
                          evtError[evtRDBuf]++;
                          singleCounter[MISSTDCTRAILER]++;
                          singleCounter[TDCTRAILERMISSTDCID+j]++;
                        }
                      }
                    }
                  }
                  nhits = 0;
                  nothers = 0;
                  for (j = 0; j < MAXNUMBERMEZZANINE; j++) {
                    if (mezzCardEnable[j] == 1) {
                      nhits += nTDCHits[j][evtRDBuf];
                      if (nTDCHits[j][evtRDBuf] == 0) {
                        singleCounter[EVENTNOTDCHITSLIST+j]++;
                        if (evtSize[j][evtRDBuf] <= 2) singleCounter[EVENTONLYTDCHDANDTRLIST+j]++;
                        else nothers++;
                      }
                    }
                    evtSize[j][evtRDBuf] = 0;
                    nTDCHits[j][evtRDBuf] = 0;
                    evtCSMFIFOOV[j][evtRDBuf] = 0;
                  }
                  if (nhits == 0) {
                    singleCounter[EVENTNOTDCHITS]++;
                    if (nothers == 0) singleCounter[EVENTONLYTDCHDANDTR]++;
                  }
                  if ((numberOfEvent != 0) && (collectedEVT >= numberOfEvent)) collectHits = FALSE;
                  else collectedEVT++;
                  if (gotBadTDCIDAtHD[evtRDBuf]) {
                    evtError[evtRDBuf]++;
                    singleCounter[TDCHEADERWRONGTDCID]++;
                  }
                  if (gotBadTDCIDAtTR[evtRDBuf]) {
                    evtError[evtRDBuf]++;
                    singleCounter[TDCTRAILERWRONGTDCID]++;
                  }
                  if (evtError[evtRDBuf] > 0) singleCounter[WRONGEVENT]++;
                  if (evtWarning[evtRDBuf] > 0) singleCounter[WARNINGEVENT]++;
                  evtError[evtRDBuf] = 0;
                  evtWarning[evtRDBuf] = 0;
                  gotHeader[evtRDBuf] = 0;
                  gotTrailer[evtRDBuf] = 0;
                  trigOverflow[evtRDBuf] = FALSE;
                  gotBadTDCIDAtHD[evtRDBuf] = FALSE;
                  gotBadTDCIDAtTR[evtRDBuf] = FALSE;
                  beginEvent[evtRDBuf] = 0;
                  printEvent[evtRDBuf] = FALSE;
                  nwords = eventBuf[0][evtRDBuf];
                  singleCounter[NEVENTWORD] += nwords;
                  singleCounter[CURREVENTSIZE] = nwords;
                  if (nwords > singleCounter[MAXEVENTSIZE]) singleCounter[MAXEVENTSIZE] = nwords;
                  if (nwords < singleCounter[MINEVENTSIZE]) singleCounter[MINEVENTSIZE] = nwords;
                  singleCounter[ANALYSEDEVENT]++;
                  singleCounter[PROCESSEDEVENT]++;
                  // EVID and WC word
                  if (suppressCSMHeader) evidOld = bufEVID[evtRDBuf];
                  else {
                    // CSM header
                    eventBuf[1][evtRDBuf] |= (CSM_WORD<<MAINIDBIT0LOCATION) | (CSM_HEAD<<SUBIDBIT0LOCATION);
                    evidOld = (eventBuf[1][evtRDBuf]>>EVIDBIT0LOCATION) & EVIDBITS;
                  }
                  data = (evidOld<<EVIDBIT0LOCATION) | (nwords<<WORDCOUNTBIT0LOCATION);
                  eventBuf[0][evtRDBuf] = data;
                  if (!suppressCSMTrailer) {
                    // CSM trailer
                    data |= (CSM_WORD<<MAINIDBIT0LOCATION) | (CSM_GOOD<<SUBIDBIT0LOCATION);
                    eventBuf[evtWRIndex[evtRDBuf]++][evtRDBuf] = data;
                  }
                  if ((12+nwords) > MAXEVTRECORD) {
                    // Drop huge event and flag it
                    nwords = 0;
                    eventBuf[0][evtRDBuf] |= (WORDCOUNTBITS<<WORDCOUNTBIT0LOCATION);
                    eventBuf[0][evtRDBuf] |= (WCONLYBITS<<WCONLYBIT0LOCATION);
                    singleCounter[NUMBERHUGEEVENT]++;
                    singleCounter[DISCARDEDEVENT]++;
                  }
                  if (CSMDataFile != NULL) {
                    if ((nDataWords+12+nwords) > MAXEVTRECORD) {
                      totalDAQTime = ((unsigned int) time(NULL)) - startTime0;
                      dataBuf[0] = nDataWords + 11;
                      dataBuf[1] = runNumber;
                      dataBuf[2] = evtNumber;
                      dataBuf[5] = nPackedEvt;
                      dataBuf[6] = mezzEnables;
                      dataBuf[9] = totalDAQTime;
                      dataBuf[10] = nDataWords;
                      saveRunCondition++;
                      if (saveRunCondition > 5) {
                        dataBuf[3] = MDTChamberType;
                        dataBuf[4] = suppressCSMHeader + 10*suppressCSMTrailer + 100*suppressTDCHeader
                                     + 1000*suppressTDCTrailer + 10000*MDTChamberHVStatus;
                        localTime = time(NULL);
                        localDateTime = localtime(&localTime);
                        strftime(myDateTime, 30, "%Y%m%d", localDateTime);
                        sscanf(myDateTime,"%x", &currentDAQDate);
                        strftime(myDateTime, 30, "%H%M%S", localDateTime);
                        sscanf(myDateTime,"%x", &currentDAQTime);
                        strftime(DAQStartedAt, 30, "%Y:%m:%d %H:%M:%S", localDateTime);
                        dataBuf[7] = currentDAQDate;
                        dataBuf[8] = currentDAQTime;
                      }
                      else if (saveRunCondition == 1) {
                        dataBuf[3] = MDTChamberType;
                        dataBuf[4] = suppressCSMHeader + 10*suppressCSMTrailer + 100*suppressTDCHeader
                                     + 1000*suppressTDCTrailer + 10000*MDTChamberHVStatus;
                        dataBuf[7] = DAQStartDate;
                        dataBuf[8] = DAQStartTime;
                        dataBuf[10] |= (saveRunCondition << 16);
                      }
                      else if (saveRunCondition == 2) {
                        dataBuf[3] = CSMVersion;
                        dataBuf[4] = selectedTrigger;
                        dataBuf[7] = selectedTriggerRate;
                        dataBuf[8] = expectedTriggerRate;
                        dataBuf[10] |= (saveRunCondition << 16);
                      }
                      else if (saveRunCondition == 3) {
                        dataBuf[3] = CSMPairDebug;
                        dataBuf[4] = widthSelection;
                        dataBuf[7] = checkSystemNoise;
                        dataBuf[8] = mappingMDTChamber;
                        dataBuf[10] |= (saveRunCondition << 16);
                      }
                      else if (saveRunCondition == 4) {
                        dataBuf[3] = AMTDataMode0;
                        dataBuf[4] = AMTDataMode1;
                        dataBuf[7] = nominalThresholdUsed;
                        dataBuf[8] = nominalThreshold;
                        dataBuf[10] |= (saveRunCondition << 16);
                      }
                      else if (saveRunCondition == 5) {
                        dataBuf[3] = integrationGate;
                        dataBuf[4] = minWidth;
                        dataBuf[7] = rundownCurrent;
                        dataBuf[8] = startMezzCard;
                        dataBuf[10] |= (saveRunCondition << 16);
                      }
                      fileSize += nDataWords + 11;
                      if ((fileSize*uintSize) >= 2000000000) {
                        printf( "\n\n--> File size exceeds the file system limit (2GB)! <--\n" );
                        printf( "--> No more data will be saved into disk\n");
                        fclose(CSMDataFile);
                        printf("\nData file <%s> is closed.\n", CSMDataFilename);
                        printf("\n Extra Data file is closed.\n");
                        CSMDataFile = NULL;
                      }
                      else {
                        fwrite(dataBuf, sizeof(unsigned int), nDataWords+11, CSMDataFile);
                        if (osock != -1) {
                          bytes = sock_write(osock, (const char *) dataBuf,sizeof(unsigned int)*(nDataWords+11));
                          if (bytes > 0) {
                            totalBytes += bytes;
                            sockWriteCount++;
                            printf("Sent packet %i\n",sockWriteCount);
                          }
                          else {
                            fprintf(logFile, "ERROR - ");
                          }
                          fprintf(logFile, "%s: %u - socket sent %i bytes. %u data words \n",myDateTime,sockWriteCount,bytes,nDataWords);
                        }
                      }
                      nPackedEvt = 0;
                      nDataWords = 0;
                    }
                    for (j = 0; j < nwords+1; j++) {
                      dataBuf[11+nDataWords+j] = eventBuf[j][evtRDBuf];
                      DAQDebug(("j = %4d eventBuf[j][] = 0x%08x\n", j, eventBuf[j][evtRDBuf]));
                    }
                    nPackedEvt++;
                    nDataWords += nwords + 1;
                  }
                  evtNumber++;
                  eventBuf[0][evtRDBuf] = 0;
                  bufEmpty[evtRDBuf] = TRUE;
                  evtRDBuf++;
                  evtRDBuf %= 64;
                }
                nwords = eventBuf[0][evtRDBuf];
              }
              // EVID and WC word
              data = (evid<<EVIDBIT0LOCATION) | (nwords<<WORDCOUNTBIT0LOCATION);
              eventBuf[0][evtWRBuf[mezz]] = data;
              if (!suppressCSMHeader) {
                // CSM header
                eventBuf[1][evtWRBuf[mezz]] |= (CSM_WORD<<MAINIDBIT0LOCATION) | (CSM_HEAD<<SUBIDBIT0LOCATION);
              }
              if (!suppressCSMTrailer) {
                // CSM trailer
                data |= (CSM_WORD<<MAINIDBIT0LOCATION) | (CSM_GOOD<<SUBIDBIT0LOCATION);
                eventBuf[evtWRIndex[evtWRBuf[mezz]]++][evtWRBuf[mezz]] = data;
              }
              if ((12+nwords) > MAXEVTRECORD) {
                // Drop huge event and flag it
                nwords = 0;
                eventBuf[0][evtWRBuf[mezz]] |= (WORDCOUNTBITS<<WORDCOUNTBIT0LOCATION);
                eventBuf[0][evtWRBuf[mezz]] |= (WCONLYBITS<<WCONLYBIT0LOCATION);
                singleCounter[NUMBERHUGEEVENT]++;
                singleCounter[DISCARDEDEVENT]++;
              }
              if (CSMDataFile != NULL) {
                if ((nDataWords+12+nwords) > MAXEVTRECORD) {
                  totalDAQTime = ((unsigned int) time(NULL)) - startTime0;
                  dataBuf[0] = nDataWords + 11;
                  dataBuf[1] = runNumber;
                  dataBuf[2] = evtNumber;
                  dataBuf[5] = nPackedEvt;
                  dataBuf[6] = mezzEnables;
                  dataBuf[9] = totalDAQTime;
                  dataBuf[10] = nDataWords;
                  saveRunCondition++;
                  if (saveRunCondition > 5) {
                    dataBuf[3] = MDTChamberType;
                    dataBuf[4] = suppressCSMHeader + 10*suppressCSMTrailer + 100*suppressTDCHeader
                                 + 1000*suppressTDCTrailer + 10000*MDTChamberHVStatus;
                    localTime = time(NULL);
                    localDateTime = localtime(&localTime);
                    strftime(myDateTime, 30, "%Y%m%d", localDateTime);
                    sscanf(myDateTime,"%x", &currentDAQDate);
                    strftime(myDateTime, 30, "%H%M%S", localDateTime);
                    sscanf(myDateTime,"%x", &currentDAQTime);
                    strftime(DAQStartedAt, 30, "%Y:%m:%d %H:%M:%S", localDateTime);
                    dataBuf[7] = currentDAQDate;
                    dataBuf[8] = currentDAQTime;
                  }
                  else if (saveRunCondition == 1) {
                    dataBuf[3] = MDTChamberType;
                    dataBuf[4] = suppressCSMHeader + 10*suppressCSMTrailer + 100*suppressTDCHeader
                                 + 1000*suppressTDCTrailer + 10000*MDTChamberHVStatus;
                    dataBuf[7] = DAQStartDate;
                    dataBuf[8] = DAQStartTime;
                    dataBuf[10] |= (saveRunCondition << 16);
                  }
                  else if (saveRunCondition == 2) {
                    dataBuf[3] = CSMVersion;
                    dataBuf[4] = selectedTrigger;
                    dataBuf[7] = selectedTriggerRate;
                    dataBuf[8] = expectedTriggerRate;
                    dataBuf[10] |= (saveRunCondition << 16);
                  }
                  else if (saveRunCondition == 3) {
                    dataBuf[3] = CSMPairDebug;
                    dataBuf[4] = widthSelection;
                    dataBuf[7] = checkSystemNoise;
                    dataBuf[8] = mappingMDTChamber;
                    dataBuf[10] |= (saveRunCondition << 16);
                  }
                  else if (saveRunCondition == 4) {
                    dataBuf[3] = AMTDataMode0;
                    dataBuf[4] = AMTDataMode1;
                    dataBuf[7] = nominalThresholdUsed;
                    dataBuf[8] = nominalThreshold;
                    dataBuf[10] |= (saveRunCondition << 16);
                  }
                  else if (saveRunCondition == 5) {
                    dataBuf[3] = integrationGate;
                    dataBuf[4] = minWidth;
                    dataBuf[7] = rundownCurrent;
                    dataBuf[8] = startMezzCard;
                    dataBuf[10] |= (saveRunCondition << 16);
                  }
                  fileSize += nDataWords + 11;
                  if ((fileSize*uintSize) >= 2000000000) {
                    printf( "\n\n--> File size exceeds the file system limit (2GB)! <--\n" );
                    printf( "--> No more data will be saved into disk\n");
                    fclose(CSMDataFile);
                    printf("\nData file <%s> is closed.\n", CSMDataFilename);
                    printf("\n Extra Data file is closed.\n");
                    CSMDataFile = NULL;
                  }
                  else {
                    fwrite(dataBuf, sizeof(unsigned int), nDataWords+11, CSMDataFile);
                    if (osock != -1) {
                      bytes = sock_write(osock, (const char *) dataBuf,sizeof(unsigned int)*(nDataWords+11));
                      if (bytes > 0) {
                        totalBytes += bytes;
                        sockWriteCount++;
                        printf("Sent packet %i\n",sockWriteCount);
                      }
                      else {
                        fprintf(logFile, "ERROR - ");
                      }
                      fprintf(logFile, "%s: %u - socket sent %i bytes. %u data words \n",myDateTime,sockWriteCount,bytes,nDataWords);
                    }
                  }
                  nPackedEvt = 0;
                  nDataWords = 0;
                }
                for (j = 0; j < nwords+1; j++) {
                  dataBuf[11+nDataWords+j] = eventBuf[j][evtWRBuf[mezz]];
                  //DAQDebug(("j = %4d eventBuf[j][] = 0x%08x\n", j, eventBuf[j][evtWRBuf[mezz]]));
                }
                nPackedEvt++;
                nDataWords += nwords + 1;
              }
              evtRDBuf++;
              evtRDBuf %= 64;
              if ((evtNumber%10000) == 0) {
                printf("\rRunNumber = %u, EventNumber = %u", runNumber, evtNumber);
                fflush(stdout);
              }
              evtNumber++;
              eventBuf[0][evtWRBuf[mezz]] = 0;
              evtWRIndex[evtWRBuf[mezz]] = 0;
              bufEmpty[evtWRBuf[mezz]] = TRUE;
            }
            //printf("i = %d, data = 0x%08x, mezz = %d, evtWRBuf[mezz] = %d\n", i, data, mezz, evtWRBuf[mezz]);
            if (notSameTrailer) {
              evtWRBuf[mezz]++;
              evtWRBuf[mezz] %= 64;
            }
          }
        }
        else {
          if (wordID == MYTDC_HEADER) {             // got a TDC header
            nTDCHeader[mezz]++;
            gotBadTDCIDAtHD[evtRDBuf] = TRUE;
            evtError[evtRDBuf]++;
            for (j = 0; j < 64; j++) {
              bufptr = evtRDBuf;
              while (bufptr != evtWRBuf[mezz]) {
                bufptr++;
                bufptr %= 64;
                gotBadTDCIDAtHD[bufptr] = TRUE;
                evtError[bufptr]++;
              }
            }
            singleCounter[EXTRATDCHEADER]++;
            singleCounter[TDCHEADEREXTRATDCID+mezz]++;
          }
          else if (wordID == MYTDC_TRAILER) {       // got a TDC trailer
            nTDCTrailer[mezz]++;
            gotBadTDCIDAtTR[evtRDBuf] = TRUE;
            evtError[evtRDBuf]++;
            for (j = 0; j < 64; j++) {
              bufptr = evtRDBuf;
              while (bufptr != evtWRBuf[mezz]) {
                bufptr++;
                bufptr %= 64;
                gotBadTDCIDAtTR[bufptr] = TRUE;
                evtError[bufptr]++;
              }
            }
            singleCounter[EXTRATDCTRAILER]++;
            singleCounter[TDCTRAILEREXTRATDCID+mezz]++;
          }
          else {
            if (wordID == MYTDC_EDGE) nTDCEdge[mezz]++;
            else if (lEdgeOn[mezz] && (wordID == MYTDC_LEDGE)) nTDCEdge[mezz]++;
            else if (tEdgeOn[mezz] && (wordID == MYTDC_TEDGE)) nTDCEdge[mezz]++;
            else if (wordID == MYTDC_PAIR) nTDCPair[mezz]++;
            else if (wordID == MYTDC_MASK) nTDCMask[mezz]++;
            else if (wordID == MYTDC_ERROR) nTDCError[mezz]++;
            else if (wordID == MYTDC_DEBUG) nTDCDebug[mezz]++;
            else nTDCOthers[mezz]++;
            evtError[evtRDBuf]++;
            for (j = 0; j < 64; j++) {
              bufptr = evtRDBuf;
              while (bufptr != evtWRBuf[mezz]) {
                bufptr++;
                bufptr %= 64;
                evtError[bufptr]++;
              }
            }
            singleCounter[EXTRAWORD]++;
            singleCounter[EXTRAWORDINTDC+mezz]++;
          }
        }
        channel++;
      }
      else channel++; 
    }
  }//if CSMversion
  // Fill the next address into the req fifo for the next loop,
    // and switch to the next buffer
    // filar->req1 = paddr[1][bufnr];
  switch(filar_chnl_no){
    case 1: filar->req1 = paddr[1][bufnr];break;
    case 2: filar->req2 = paddr[2][bufnr];break;
    case 3: filar->req3 = paddr[3][bufnr];break;
    case 4: filar->req4 = paddr[4][bufnr];break;
    default:break;
  }
  if (++bufnr == nbRequestedBuf) bufnr = 0;
  numberFilledFIFOs--;
  if (numberFilledFIFOs < 0) numberFilledFIFOs = 0;
}


void CollectCSMData::EndOfCollecting(){

  if (CSMDataFile != NULL) {
    fclose(CSMDataFile);
    printf("\nData file <%s> is closed.\n", CSMDataFilename);
    CSMDataFile = NULL;
  }
  if (rawDataFile != NULL) {
    fclose(rawDataFile);
    printf("\nRaw Data file <%s> is closed.\n", rawDataFilename);
    rawDataFile = NULL;
  }
  if (logFile != NULL) {
    fclose(logFile);
    logFile = NULL;
    printf("\nLog file is closed.\n");
  }
  for (j = 0; j <  NUMBERANALYSISCOUNTER; j++) {
    if (j == ANALYSEDEVENT) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[ANALYSEDEVENTHIGH]++;
      analysisCounter[ANALYSEDEVENT] = sum;
    }
    else if (j == NTOTALWORD) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[NTOTALWORDHIGH]++;
      analysisCounter[NTOTALWORD] = sum;
    }
    else if (j == PROCESSEDEVENT) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[PROCESSEDEVENTHIGH]++;
      analysisCounter[PROCESSEDEVENT] = sum;
    }
    else if (j == NDATAWORD) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[NDATAWORDHIGH]++;
      analysisCounter[NDATAWORD] = sum;
    }
    else if (j == NEVENTWORD) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[NEVENTWORDHIGH]++;
      analysisCounter[NEVENTWORD] = sum;
    }
    else if (j == NGOODCYCLE) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[NGOODCYCLEHIGH]++;
      analysisCounter[NGOODCYCLE] = sum;
    }
    else if (j == NEMPTYCYCLE) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[NEMPTYCYCLEHIGH]++;
      analysisCounter[NEMPTYCYCLE] = sum;
    }
    else if ((j == CURREVENTSIZE) || (j == CURREVENTID)) {
      analysisCounter[j] = singleCounter[j];
    }
    else if (j == MINEVENTSIZE) {
      if ((singleCounter[ANALYSEDEVENT] > 0) && (analysisCounter[j] > singleCounter[j]))
        analysisCounter[j] = singleCounter[j];
    }
    else if (j == MAXEVENTSIZE) {
      if (analysisCounter[j] < singleCounter[j]) analysisCounter[j] = singleCounter[j];
    }
    else if ((j == LHCCLOCKUNLOCKED) || (j == XMT1CLOCKUNLOCKED) || (j == XMT2CLOCKUNLOCKED) ||
             (j == CSMPHASEERROR) || (j == I2COPERATIONFAILED) || (j == UNEXPECTEDTTCRXSETUP) ||
             (j == CSMHASERROR) || (j == UNKNOWNSWORD)) {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[j] = 0xFFFFFFFF;
      else analysisCounter[j] += singleCounter[j];
    }
    else {
      sum = analysisCounter[j] + singleCounter[j];
      if (sum < analysisCounter[j]) analysisCounter[j] = 0xFFFFFFFF;
      else analysisCounter[j] += singleCounter[j];
    }
  }
  SaveErrorSummaryFile();
  SaveTDCTimeSpectrum();
  SaveTriggerTimeDistribution(trigTHist);
  printf ("\nData statistics for Run %u\n", runNumber);
  printf(" MezzCard     Header       Edge       Pair      Error       Mask      Debug    Trailer      Other\n");
  for (mezz = 0; mezz < MAXNUMBERMEZZANINE; ++mezz) {
    printf(" %8d %10u %10u %10u %10u %10u %10u %10u %10u\n", mezz, nTDCHeader[mezz], nTDCEdge[mezz], nTDCPair[mezz], 
                                                             nTDCError[mezz], nTDCMask[mezz], nTDCDebug[mezz],
                                                             nTDCTrailer[mezz], nTDCOthers[mezz]);   
  }
  printf("\n");
  PrintAnalysisCounter(NULL, NTOTALWORD, "Number of Total Received Words", 45);
  PrintAnalysisCounter(NULL, NDATAWORD, "Number of Received Data Words", 45);
  if ((CSMVersion%2) == 1) {
    PrintAnalysisCounter(NULL, NGOODCYCLE, "Number of Good Cycles", 45);
    PrintAnalysisCounter(NULL, NEMPTYCYCLE, "Number of Empty Cycles", 45);
    PrintAnalysisCounter(NULL, NBADCYCLE, "Number of Bad Cycles", 45);
    PrintAnalysisCounter(NULL, NBADCYCLEATBEGINNING, "Number of Bad Cycles at Beginning", 45);
    PrintAnalysisCounter(NULL, CSMPARITYERROR, "Number of Data Word with TDC Parity Error", 45);
    PrintAnalysisCounter(NULL, TXPARITYERROR, "Number Communiation Parity Errors", 45);
  }
  PrintAnalysisCounter(NULL, ANALYSEDEVENT, "Number of Analyzed Events", 45);
  PrintAnalysisCounter(NULL, WRONGEVENT, "Number of Event with Error", 45);
  PrintAnalysisCounter(NULL, WARNINGEVENT, "Number of Event with Warning", 45);
  PrintAnalysisCounter(NULL, DISCARDEDEVENT, "Number of Discarded Events", 45);
  PrintAnalysisCounter(NULL, NUMBERHUGEEVENT, "Number of Huge Events (>4000)", 45);
  PrintAnalysisCounter(NULL, WORDCOUNTERERROR, "Number of Word Counter Error", 45);
  PrintAnalysisCounter(NULL, EVIDMISMATCH, "Number of EVID Mismatch", 45);
  PrintAnalysisCounter(NULL, BCIDMISMATCH, "Number of BCID Mismatch", 45);
  printf("Socket sent %i bytes of data.\n", totalBytes);
  printf("=== DAQ is stopped ===\n");
  if ((dataAnalysisControl > 0) && (strstr(CSMDataFilename, "/dev/null") == NULL)) {
    printf("\n");
    CheckDataAnalysisPackageInstallations();
    if (!anaInstalled) {
      printf("Unable to do data analysis since data analysis package is not installed at this machine.\n");
      return;
    }
    strcpy(CMDString, anaPath);
    strcat(CMDString, "/tdcSpectrum.job ");
    strcat(CMDString, CSMDataFilename);
    strcat(CMDString, " -nowait");
    if (dataAnalysisControl == 2) strcat(CMDString, " -fit");
    system(CMDString);
  }
}


void CollectCSMData::DataInterpretation(unsigned int data, FILE *file) {
  int wordID, subID, TDC, EVID, BCID, nword, Chan, Wid, Coarse, Fine, Type, Error, tdcTime;

  wordID = (data>>MAINIDBIT0LOCATION) & MAINIDBITS;
  TDC = (data>>TDCNUMBERBIT0LOCATION) & TDCNUMBERBITS;
  if ((wordID == MYTDC_EDGE) || (wordID == MYTDC_LEDGE) || (wordID == MYTDC_TEDGE) || (wordID == MYTDC_PAIR)) {
    Chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
    if (Chan >= 24) {
      if (TDC < 3) {
        Chan -= 24;
        Chan += 8 * TDC;
        TDC = 16;
      }
      else if (TDC > 3 && TDC < 7) {
        Chan -= 24;
        Chan += 8 * (TDC - 4);
        TDC = 17;
      }
    }
  }
  if (wordID == MYTDC_HEADER) {
    BCID = (data>>BCIDBIT0LOCATION) & BCIDBITS;
    EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
    if (file == NULL)
      printf("TDC header, TDC=%d, EVID=0x%03x, BCID=0x%03x", TDC, EVID, BCID);
    else
      fprintf(file, "TDC header, TDC=%d, EVID=0x%03x, BCID=0x%03x", TDC, EVID, BCID);
  }
  else if (wordID == MYTDC_TRAILER) {
    nword = (data>>WORDCOUNTBIT0LOCATION) & WORDCOUNTBITS;
    EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
    if (file == NULL)
      printf("TDC trailer, TDC=%d, EVID=0x%03x, WC=%d", TDC, EVID, nword);
    else
      fprintf(file, "TDC trailer, TDC=%d, EVID=0x%03x, WC=%d", TDC, EVID, nword);
  }
  else if (wordID == MYTDC_MASK) {
    if (file == NULL)
      printf("TDC mask, TDC=%d, MaskFlag=0x%06x", TDC, data&0xFFFFFF);
    else
      fprintf(file, "TDC mask, TDC=%d, MaskFlag=0x%06x", TDC, data&0xFFFFFF);
  }
  else if (wordID == MYTDC_DEBUG) {
    if (file == NULL)
      printf("TDC debug, TDC=%d, 24Bits=0x%06x", TDC, data&0xFFFFFF);
    else
      fprintf(file, "TDC debug, TDC=%d, 24Bits=0x%06x", TDC, data&0xFFFFFF);
  }
  else if (wordID == MYTDC_ERROR) {
    Error = data&errWordMask;
    if (file == NULL)
      printf("TDC error, TDC=%d, Errors=0x%04x", TDC, Error);
    else
      fprintf(file, "TDC error, TDC=%d, Errors=0x%04x", TDC, Error);
  }
  else if ((wordID == MYTDC_EDGE) || (lEdgeOn[TDC] && (wordID == MYTDC_LEDGE)) ||
     (tEdgeOn[TDC] && (wordID == MYTDC_TEDGE))) {       // got a TDC egde word
    if (file == NULL) {
      if (HPTDC) printf("TDC edge, TDC Chan Type TDCTime =");
      else printf("TDC edge, TDC Chan Type Error Coarse Fine =");
    }
    else {
      if (HPTDC) fprintf(file, "TDC edge, TDC Chan Type TDCTime =");
      else fprintf(file, "TDC edge, TDC Chan Type Error Coarse Fine =");
    }
    Chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
    if (HPTDC) {
      tdcTime = (data>>STDCTIMEBIT0LOCATION) & SHPTDCTIMEBITS;
      if (lEdgeOn[TDC] && (wordID == MYTDC_LEDGE)) Type = 1;
      else Type = 0;
      Error = 0;
    }
    else {
      Coarse = (data>>SCOARSETIMEBIT0LOCATION) & SCOARSETIMEBITS;
      Fine = (data>>FINETIMEBIT0LOCATION) & FINETIMEBITS;
      Type = (data>>STYPEBIT0LOCATION) & STYPEBITS;
      Error = (data>>SERRORBIT0LOCATION) & SERRORBITS;
    }
    if (Chan >= 24) {
      if (TDC < 3) {
        Chan -= 24;
        Chan += 8 * TDC;
        TDC = 16;
      }
      else if (TDC > 3 && TDC < 7) {
        Chan -= 24;
        Chan += 8 * (TDC - 4);
        TDC = 17;
      }
    }
    if (file == NULL) printf(" %d %d ", TDC, Chan);
    else fprintf(file, " %d %d ", TDC, Chan);
    if (Type) {
      if (file == NULL) printf("Leading ");
      else fprintf(file, "Leading ");
    }
    else {
      if (file == NULL) printf("Trailing ");
      else fprintf(file, "Trailing ");
    }
    if (Error) {
      if (file == NULL) printf("YES ");
      else fprintf(file, "YES ");
    }
    else {
      if (file == NULL) printf("NO ");
      else fprintf(file, "NO ");
    }
    if (HPTDC) {
      if (file == NULL) printf("%d", tdcTime);
      else fprintf(file, "%d", tdcTime);
    }
    else {
      if (file == NULL) printf("%d %d", Coarse, Fine);
      else fprintf(file, "%d %d", Coarse, Fine);
    }
  }
  else if (wordID == MYTDC_PAIR) {
    if (file == NULL) {
      if (HPTDC) printf("TDC combined, TDC Chan Wid TDCTime =");
      else printf("TDC combined, TDC Chan Wid Coarse Fine =");
    }
    else {
      if (HPTDC) fprintf(file, "TDC combined, TDC Chan Wid TDCTime =");
      else fprintf(file, "TDC combined, TDC Chan Wid Coarse Fine =");
    }
    Chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
    if (HPTDC) {
      Wid = (data>>PWIDTHHPTDCBIT0LOCATION) & PWIDTHHPTDCBITS;
      tdcTime = (data>>PHPTDCTIMEBIT0LOCATION) & PHPTDCTIMEBITS;
    }
    else {
      Wid = (data>>PWIDTHBIT0LOCATION) & PWIDTHBITS;
      Coarse = (data>>PCOARSETIMEBIT0LOCATION) & PCOARSETIMEBITS;
      Fine = (data>>FINETIMEBIT0LOCATION) & FINETIMEBITS;
    }
    if (Chan >= 24) {
      if (TDC < 3) {
        Chan -= 24;
        Chan += 8 * TDC;
        TDC = 16;
      }
      else if (TDC > 3 && TDC < 7) {
        Chan -= 24;
        Chan += 8 * (TDC - 4);
        TDC = 17;
      }
    }
    if (file == NULL) {
      if (HPTDC) printf(" %d %d %d %d %d", TDC, Chan, Wid, tdcTime);
      else printf(" %d %d %d %d %d", TDC, Chan, Wid, Coarse, Fine);
    }
    else {
      if (HPTDC) fprintf(file, " %d %d %d %d %d", TDC, Chan, Wid, tdcTime);
      else fprintf(file, " %d %d %d %d %d", TDC, Chan, Wid, Coarse, Fine);
    }
  }
  else if (wordID == CSM_WORD) {
    subID = (data>>SUBIDBIT0LOCATION) & SUBIDBITS;
    BCID = (data>>BCIDBIT0LOCATION) & BCIDBITS;
    EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
    nword = BCID;
    if (subID == CSM_HEAD) {
      if (file == NULL)
        printf("CSM header, EVID=0x%03x, BCID=0x%03x", EVID, BCID);
      else
        fprintf(file, "CSM header, EVID=0x%03x, BCID=0x%03x", EVID, BCID);
    }
    else if (subID == CSM_GOOD) {
      if (file == NULL)
        printf("CSM trailer, EVID=0x%03x, WC=%d", EVID, nword);
      else
        fprintf(file, "CSM trailer, EVID=0x%03x, WC=%d", EVID, nword);
    }
    else if (subID == CSM_ABORT) {
      if (file == NULL)
        printf("CSM abort, EVID=0x%03x, WC=%d", EVID, nword);
      else
        fprintf(file, "CSM abort, EVID=0x%03x, WC=%d", EVID, nword);
    }
    else if (subID == CSM_PARITY) {
      Error = (data<<18) & 0x3F;
      TDC = data & 0x3FFFF;
      if (file == NULL)
        printf("CSM error, Bit23:18=0x%02x, TDCWParityError=%05X", Error, TDC);
      else
        fprintf(file, "CSM error, Bit23:18=0x%02x, TDCWParityError=%05X", Error, TDC);
    }
    else if (subID == CSM_ERROR) {
      Error = (data<<18) & 0x3F;
      TDC = data & 0x3FFFF;
      if (file == NULL)
        printf("CSM error, Bit23:18=0x%03x, AbnormalTDC=%05X", Error, TDC);
      else
        fprintf(file, "CSM error, Bit23:18=0x%03x, AbnormalTDC=%05X", Error, TDC);
    }
    else {
      if (file == NULL)
        printf("Unknown CSM word");
      else
        fprintf(file, "Unknown CSM word");
    }
  }
  else {
    if (file == NULL)
      printf("Unknown word");
    else
      fprintf(file, "Unknown word");
  }
  if (file == NULL) printf("\n");
  else fprintf(file, "\n");
}



void CollectCSMData::RawDataInterpretation(unsigned int data, FILE *file) {
  int wordID, subID, EVID, BCID, nword, Chan, Wid, Coarse, Fine, Type, Error, tdcTime;
  static int TDC = -1;
  static unsigned int count = 0, CSMData, CSMStatus, control;
  char CSMParityErr, TXParityErr, overflow1, overflow2;

  wordID = (data>>MAINIDBIT0LOCATION) & MAINIDBITS;
  if ((data&CSMPARITYERRORBIT) != 0) CSMParityErr = 'Y';
  else CSMParityErr = 'N';
  if (checkParity(data, &count) != 0) TXParityErr = 'Y';
  else TXParityErr = 'N';
  if (file == NULL) printf("CSMPErr=%c, TXPErr=%c, ", CSMParityErr, TXParityErr);
  else fprintf(file, "CSMPErr=%c, TXPErr=%c, ", CSMParityErr, TXParityErr);
  if (wordID == ID_SYNCHWORD) {
    count = 0;
    TDC = -1;
    if (synchWordControl) {
      CSMStatus = (data>>SWORDCSMSTATUSBIT0LOC) & SWORDCSMSTATUSBITS;
      control = (data>>SWORDCONTROLBIT0LOC) & SWORDCONTROLBITS;
      CSMData = (data>>SWORDTRIGGERTIMEBIT0LOC) & SWORDTRIGGERTIMEBITS;
      if (file == NULL) printf("SynchWord, status=0x%02x, control=%u, Bit15:0=0x%04x", CSMStatus, control, CSMData);
      else fprintf(file, "SynchWord, status=0x%02x, control=%u, Bit15:0=0x%04x", CSMStatus, control, CSMData);
    }
    else {
      CSMData = (data>>CSMDATABIT0LOCATION) & CSMDATABITS;
      if (file == NULL) printf("SynchWord, Bit25:0=0x%07x", CSMData);
      else fprintf(file, "SynchWord, Bit25:0=0x%07x", CSMData);
    }
  }
  else {
    TDC++;
    TDC %= MAXNUMBERMEZZANINE;
    if (wordID == ID_IDLEWORD) {
      CSMData = (data>>CSMDATABIT0LOCATION) & CSMDATABITS;
      if (file == NULL) printf("CSMIdleWord, Bit25:0=0x%07x", CSMData);
      else fprintf(file, "CSMIdleWord, Bit25:0=0x%07x", CSMData);
    }
    else if (wordID == MYTDC_HEADER) {
      BCID = (data>>BCIDBIT0LOCATION) & BCIDBITS;
      EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
      if (file == NULL)
        printf("TDC header, TDC=%d, EVID=0x%03x, BCID=0x%03x", TDC, EVID, BCID);
      else
        fprintf(file, "TDC header, TDC=%d, EVID=0x%03x, BCID=0x%03x", TDC, EVID, BCID);
    }
    else if (wordID == MYTDC_TRAILER) {
      nword = (data>>WORDCOUNTBIT0LOCATION) & WORDCOUNTBITS;
      EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
      if (CSMVersion > 0x20) {
        if (data&CSMFIFOOV1BITS) overflow1 = 'Y';
        else overflow1 = 'N';
        if (data&CSMFIFOOV2BITS) overflow2 = 'Y';
        else overflow2 = 'N';
        if (file == NULL)
          printf("TDC trailer, TDC=%d, EVID=0x%03x, WC=%d, OverF1=%c, OverF2=%c", TDC, EVID, nword, overflow1, overflow2);
        else
          fprintf(file, "TDC trailer, TDC=%d, EVID=0x%03x, WC=%d, OverF1=%c, OverF2=%c", TDC, EVID, nword, overflow1, overflow2);
      }
      else if (file == NULL)
        printf("TDC trailer, TDC=%d, EVID=0x%03x, WC=%d", TDC, EVID, nword);
      else
        fprintf(file, "TDC trailer, TDC=%d, EVID=0x%03x, WC=%d", TDC, EVID, nword);
    }
    else if (wordID == MYTDC_MASK) {
      if (file == NULL)
        printf("TDC mask, TDC=%d, MaskFlag=0x%06x", TDC, data&0xFFFFFF);
      else
        fprintf(file, "TDC mask, TDC=%d, MaskFlag=0x%06x", TDC, data&0xFFFFFF);
    }
    else if (wordID == MYTDC_DEBUG) {
      if (file == NULL)
        printf("TDC debug, TDC=%d, 24Bits=0x%06x", TDC, data&0xFFFFFF);
      else
        fprintf(file, "TDC debug, TDC=%d, 24Bits=0x%06x", TDC, data&0xFFFFFF);
    }
    else if (wordID == MYTDC_ERROR) {
      Error = data&errWordMask;
      if (file == NULL)
        printf("TDC error, TDC=%d, Errors=0x%04x", TDC, Error);
      else
        fprintf(file, "TDC error, TDC=%d, Errors=0x%04x", TDC, Error);
    }
    else if ((wordID == MYTDC_EDGE) || (lEdgeOn[TDC] && (wordID == MYTDC_LEDGE)) ||
       (tEdgeOn[TDC] && (wordID == MYTDC_TEDGE))) {       // got a TDC egde word
      if (file == NULL) {
        if (HPTDC) printf("TDC edge, TDC Chan Type TDCTime =");
        else printf("TDC edge, TDC Chan Type Error Coarse Fine =");
      }
      else {
        if (HPTDC) fprintf(file, "TDC edge, TDC Chan Type TDCTime =");
        else fprintf(file, "TDC edge, TDC Chan Type Error Coarse Fine =");
      }
      Chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
      if (HPTDC) {
        tdcTime = (data>>STDCTIMEBIT0LOCATION) & SHPTDCTIMEBITS;
        if (lEdgeOn[TDC] && (wordID == MYTDC_LEDGE)) Type = 1;
        else Type = 0;
        Error = 0;
      }
      else {
        Coarse = (data>>SCOARSETIMEBIT0LOCATION) & SCOARSETIMEBITS;
        Fine = (data>>FINETIMEBIT0LOCATION) & FINETIMEBITS;
        Type = (data>>STYPEBIT0LOCATION) & STYPEBITS;
        Error = (data>>SERRORBIT0LOCATION) & SERRORBITS;
      }
      if (Chan >= 24) {
        if (TDC < 3) {
          Chan -= 24;
          Chan += 8 * TDC;
          TDC = 16;
        }
        else if (TDC > 3 && TDC < 7) {
          Chan -= 24;
          Chan += 8 * (TDC - 4);
          TDC = 17;
        }
      }
      if (file == NULL) printf(" %d %d ", TDC, Chan);
      else fprintf(file, " %d %d ", TDC, Chan);
      if (Type) {
        if (file == NULL) printf("Leading ");
        else fprintf(file, "Leading ");
      }
      else {
        if (file == NULL) printf("Trailing ");
        else fprintf(file, "Trailing ");
      }
      if (HPTDC) {
        if (file == NULL) printf("%d", tdcTime);
        else fprintf(file, "%d", tdcTime);
      }
      else {
        if (Error) {
          if (file == NULL) printf("YES ");
          else fprintf(file, "YES ");
        }
        else {
          if (file == NULL) printf("NO ");
          else fprintf(file, "NO ");
        }
        if (file == NULL) printf("%d %d", Coarse, Fine);
        else fprintf(file, "%d %d", Coarse, Fine);
      }
    }
    else if (wordID == MYTDC_PAIR) {
      if (file == NULL) {
        if (HPTDC) printf("TDC combined, TDC Chan Wid TDCTime =");
        else printf("TDC combined, TDC Chan Wid Coarse Fine =");
      }
      else {
        if (HPTDC) fprintf(file, "TDC combined, TDC Chan Wid TDCTime =");
        else fprintf(file, "TDC combined, TDC Chan Wid Coarse Fine =");
      }
      Chan = (data>>CHANNELNUMBERBIT0LOCATION) & CHANNELNUMBERBITS;
      if (HPTDC) {
        Wid = (data>>PWIDTHHPTDCBIT0LOCATION) & PWIDTHHPTDCBITS;
        tdcTime = (data>>PHPTDCTIMEBIT0LOCATION) & PHPTDCTIMEBITS;
      }
      else {
        Wid = (data>>PWIDTHBIT0LOCATION) & PWIDTHBITS;
        Coarse = (data>>PCOARSETIMEBIT0LOCATION) & PCOARSETIMEBITS;
        Fine = (data>>FINETIMEBIT0LOCATION) & FINETIMEBITS;
      }
      if (Chan >= 24) {
        if (TDC < 3) {
          Chan -= 24;
          Chan += 8 * TDC;
          TDC = 16;
        }
        else if (TDC > 3 && TDC < 7) {
          Chan -= 24;
          Chan += 8 * (TDC - 4);
          TDC = 17;
        }
      }
      if (file == NULL) {
        if (HPTDC) printf(" %d %d %d %d %d", TDC, Chan, Wid, tdcTime);
        else printf(" %d %d %d %d %d", TDC, Chan, Wid, Coarse, Fine);
      }
      else {
        if (HPTDC) fprintf(file, " %d %d %d %d %d", TDC, Chan, Wid, tdcTime);
        else fprintf(file, " %d %d %d %d %d", TDC, Chan, Wid, Coarse, Fine);
      }
    }
    else if (wordID == CSM_WORD) {
      subID = (data>>SUBIDBIT0LOCATION) & SUBIDBITS;
      BCID = (data>>BCIDBIT0LOCATION) & BCIDBITS;
      EVID = (data>>EVIDBIT0LOCATION) & EVIDBITS;
      nword = BCID;
      if (subID == CSM_HEAD) {
        if (file == NULL)
          printf("CSM header, EVID=0x%03x, BCID=0x%03x", EVID, BCID);
        else
          fprintf(file, "CSM header, EVID=0x%03x, BCID=0x%03x", EVID, BCID);
      }
      else if (subID == CSM_GOOD) {
        if (file == NULL)
          printf("CSM trailer, EVID=0x%03x, WC=%d", EVID, nword);
        else
          fprintf(file, "CSM trailer, EVID=0x%03x, WC=%d", EVID, nword);
      }
      else if (subID == CSM_ABORT) {
        if (file == NULL)
          printf("CSM abort, EVID=0x%03x, WC=%d", EVID, nword);
        else
          fprintf(file, "CSM abort, EVID=0x%03x, WC=%d", EVID, nword);
      }
      else if (subID == CSM_PARITY) {
        Error = (data<<18) & 0x3F;
        TDC = data & 0x3FFFF;
        if (file == NULL)
          printf("CSM error, Bit23:18=0x%02x, TDCWParityError=%05X", Error, TDC);
        else
          fprintf(file, "CSM error, Bit23:18=0x%02x, TDCWParityError=%05X", Error, TDC);
      }
      else if (subID == CSM_ERROR) {
        Error = (data<<18) & 0x3F;
        TDC = data & 0x3FFFF;
        if (file == NULL)
          printf("CSM error, Bit23:18=0x%03x, AbnormalTDC=%05X", Error, TDC);
        else
          fprintf(file, "CSM error, Bit23:18=0x%03x, AbnormalTDC=%05X", Error, TDC);
      }
      else {
        if (file == NULL)
          printf("Unknown CSM word");
        else
          fprintf(file, "Unknown CSM word");
      }
    }
    else {
      if (file == NULL)
        printf("Unknown word");
      else
        fprintf(file, "Unknown word");
    }
  }
  if (file == NULL) printf("\n");
  else fprintf(file, "\n");
}


