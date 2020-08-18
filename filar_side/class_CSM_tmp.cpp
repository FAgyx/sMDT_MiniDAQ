void CollectCSMData::DataAssembling_triggerless(){
  // Read the ACK FIFO to get the size of the data packet in the data
  // buffer.  It had better always be the same for all that we grab!
  // fsize in 32bit words, 1M word max. And get a pointer to the data
  // datasize = (filar->ack1) & (0xFFFFF);
  // dataptr = (unsigned int *) uaddr[1][bufnr];
  unsigned int triggerless_data_buff[4096];
  int triggerless__index = 0;
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
          // if (wordID == MYTDC_HEADER) {             // got a TDC header
          if ((wordID == MYTDC_HEADER)||(wordID == TDC_HEADER)) {             // got a TDC/HPTDC header 
            nReceivedData[mezz] = 0; 
            nUnexpected[mezz] = 0; 
          }
          // else if (wordID == MYTDC_TRAILER) {       // got a TDC trailer
          else if ((wordID == MYTDC_TRAILER)||(wordID == TDC_TRAILER)) {       // got a TDC/HPTDC trailer
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
          // if (wordID == MYTDC_HEADER) {             // got a TDC header
          if ((wordID == MYTDC_HEADER)||(wordID == TDC_HEADER)) {             // got a TDC/HPTDC header 

          }
          else if (wordID == MYTDC_MASK) {         // got a TDC mask word

          }
          else if (wordID == MYTDC_ERROR) {        // got a TDC error word
            printf("Error Word = %08x\n",data);
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
            triggerless_data_buff[triggerless__index] = data;
            triggerless__index++;

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
            triggerless_data_buff[triggerless__index] = data;
            triggerless__index++;
          }  
          if(triggerless__index == 4095){
            fwrite(triggerless_data_buff, sizeof(unsigned int), 4096, CSMDataFile);
            triggerless__index = 0;
          }
        } //if (mezzCardEnable[mezz])
        channel++;
      }
      else channel++; 
    } //for (i = 0; i < datasize; i++)
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