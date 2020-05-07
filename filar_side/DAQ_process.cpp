void DAQ_process(){
  int nfifos1, nfifos2, fifodata, gotData1, gotData2;
  nfifos1 = 0;
  nfifos2 = 0;
  fifodata = 0;

  while (DAQState != State_Idle) {
    gotData1 = FALSE;
    gotData2 = FALSE;

    while (DAQState != State_Idle) {
      fifodata = filar->fifostat;

      nfifos1 = fifodata & 0x0000000f;
      nfifos2 = (fifodata >> 8 ) & 0x0000000f;

      if (nfifos1 >= p_CollectCSMData_1->numberFilledFIFOs) p_CollectCSMData_1->numberFilledFIFOs = nfifos1; 
      else if (p_CollectCSMData_1->numberFilledFIFOs >= 15) p_CollectCSMData_1->numberFilledFIFOs = 16+nfifos1;

      if (nfifos2 >= p_CollectCSMData_2->numberFilledFIFOs) p_CollectCSMData_2->numberFilledFIFOs = nfifos2; 
      else if (p_CollectCSMData_2->numberFilledFIFOs >= 15) p_CollectCSMData_2->numberFilledFIFOs = 16+nfifos2;



      if (p_CollectCSMData_1->numberFilledFIFOs > 0) gotData1 = TRUE;
      if (p_CollectCSMData_2->numberFilledFIFOs > 0) gotData2 = TRUE;
      if(gotData1|gotData2) break;      
    }

    
    if (!(gotData1|gotData2)) break;
    if(gotData1)p_CollectCSMData_1->DataAssembling();
    if(gotData2)p_CollectCSMData_2->DataAssembling();
    
    
    
  } //while (DAQState != State_Idle)
  p_CollectCSMData_1->EndOfCollecting();
  p_CollectCSMData_2->EndOfCollecting();
}