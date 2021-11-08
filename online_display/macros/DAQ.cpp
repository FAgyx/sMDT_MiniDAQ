#define SPEEDFACTOR 1
#define NEWTDC_NUMBER 17
#define WIDTH_RES 1


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sstream>


#include "src/sockhelp.c"

// Plot includes
#include <iostream>
#include <bitset>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>



#include "macros/udp_client_init.c"
#include "macros/channel_packet.c"

// ROOT includes
#include "TFile.h"
#include "TDirectory.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1.h"

// Geometry includes
#include "src/reco/RecoUtility.cxx"

// global define inculdes
#include "macros/GlobalIncludes.h"

// Muon Reconstruction includes
#include "src/reco/Signal.cxx"
#include "src/reco/EventID.cxx"
#include "src/reco/Event.cxx"
#include "src/reco/EventDisplay.cxx"
#include "src/reco/Geometry.cxx"
#include "src/reco/Hit.cxx"
#include "src/reco/Cluster.cxx"
#include "src/reco/TimeCorrection.cxx"
#include "src/reco/ParameterSet.cxx"
#include "src/reco/ConfigParser.cxx"
#include "src/reco/Track.cxx"
#include "src/reco/IOUtility.cxx"
#include "src/reco/Optimizer.cxx"
#include "src/reco/ResolutionResult.cxx"
#include "src/reco/Parameterization.cxx"
#include "src/reco/RTParam.cxx"
#include "src/reco/TrackParam.cxx"
#include "src/reco/Observable.cxx"
#include "src/reco/T0Reader.cxx"
#include "src/reco/T0Fit.cxx"
#include "MuonReco/T0Fit.h"


#ifndef ONLINE_DAQ_MONITOR
#define ONLINE_DAQ_MONITOR

//#define DECODER_DEBUG // comment this line when debugging information is not needed
//#define SET_MAXWORDS // comment this line if you want to decode the whole data words
#define SAVE_TRACKS_OUT_OF_ROOT // comment this line if you don't need to save plots out of rootfile 

 

using namespace std;
using namespace MuonReco;



class DAQ_monitor
{
public:
  DAQ_monitor(short portno_input, int bisno=1);
  void error(const char *msg);
  void tcp_server_setup(in_addr_t server_ip_int);
  void DataDecode();
  
  
private:
  TDirectory *event_track[2];
  char track_group_name[128];
  std::vector<TH1F*> p_tdc_adc_time;
  std::vector<TH1F*> p_tdc_tdc_time_corrected;
  std::vector<std::vector<TH1F*>> p_tdc_chnl_adc_time;
  std::vector<std::vector<TH1F*>> p_tdc_chnl_tdc_time_corrected;
  std::vector<std::vector<TH1F*>> p_tdc_chnl_adc_time_raw;
  std::vector<std::vector<TH1F*>> p_tdc_chnl_tdc_time_corrected_raw;
  std::vector<std::vector<double>> p_tdc_hit_rate;
  std::vector<std::vector<double>> nHits;
  std::vector<std::vector<double>> nMiss;
  std::vector<double> p_tdc_hit_rate_x;
  std::vector<bool> isDrawn;
  

  TH2D* tube_efficiency;
  std::vector<TH1F*> p_tdc_hit_rate_graph;
  Geometry geo;
  TimeCorrection tc;
  EventDisplay *ed;
  RecoUtility ru;
  ResolutionResult *rr;
  TrackParam tp;
  RTParam* rtp;
  TCanvas *adc_canvas, *tdc_canvas, *rate_canvas, *trigger_rate_canvas, *residual_canvas, *EDCanvas, *eff_canvas;
  short tcp_portno;
  int sockfd, newsockfd, udp_sock_fd;
  struct sockaddr_in udp_servaddr;
  in_addr_t tcp_server_ip;
  int pad_num;
  unsigned int buffer[4096];
  int sockReadCount, bytes_recv, total_bytes_recv;
  ofstream oFile;
  FILE *fp_rate_File;
  char oFile_name[30];
  char filename_time[30];
  ifstream data_in_flow;
  TFile *p_output_rootfile;
  TH2D *hitByLC, *badHitByLC, *goodHitByLC;
  unsigned long total_triggers, total_events, total_triggers_pass, total_events_pass;
  unsigned long total_signals, total_signals_pass, total_events_fail;
  int hitL, hitC;
  int current_track_group;
  std::vector<std::vector<Bool_t>> first_signal_flag;
  int temp_track_group;
  bool pass_event_check;
  unsigned int word;
  unsigned int header_type;
  EventID currEventID, prevEventID;
  vector<Signal> trigVec, sigVec, nonzeroTrigVec;
  bitset<4> header;
  Signal sig;
  Event  event, event_raw, ed_event;
  TTree* eTree;
  stringstream *filar_1, *filar_2;
  time_t start_time, current_time;
  double DAQ_time;
  int status;
  int tracking_evt_no = 0;
  TH1D* residuals;
  
};


DAQ_monitor::DAQ_monitor(short portno_input, int bisno/*=1*/){
  tcp_portno = portno_input;
  geo = Geometry();
  
  ConfigParser cp;
  if (bisno == 1) {
    cp = ConfigParser("../smdt-reco/conf/BIS1.conf");
  }
  else {
    cp = ConfigParser("../smdt-reco/conf/BIS2-6.conf");
  }

  residuals = new TH1D("residuals", "Residuals;Residual[#mum];Number of hits/2#mum", 500, -500, 500);
  tp = TrackParam();

  rtp = new RTParam(cp);

  //rtp->LoadTxt("src/Rt_BMG_6_1.dat"); // offending line

  tp.SetRT(rtp);
  tp.setMaxResidual(1000000);
  tp.setVerbose(0);

  geo.Configure(cp.items("Geometry"));
  tc = TimeCorrection();
  ed = new EventDisplay();
  //ed->SetRT(rtp);  // TODO: GET A WORKING RT ONLINE
  ru = RecoUtility();
  TString h_name;
  
  p_tdc_adc_time.resize(Geometry::MAX_TDC);
  p_tdc_tdc_time_corrected.resize(Geometry::MAX_TDC);
  p_tdc_hit_rate_x.resize(Geometry::MAX_TDC_CHANNEL);
  isDrawn.resize(Geometry::MAX_TDC);  
  
  p_tdc_chnl_adc_time.resize(Geometry::MAX_TDC);
  p_tdc_chnl_adc_time_raw.resize(Geometry::MAX_TDC);
  p_tdc_chnl_tdc_time_corrected.resize(Geometry::MAX_TDC);
  p_tdc_chnl_tdc_time_corrected_raw.resize(Geometry::MAX_TDC);
  p_tdc_hit_rate.resize(Geometry::MAX_TDC);
  p_tdc_hit_rate_graph.resize(Geometry::MAX_TDC);	
  first_signal_flag.resize(Geometry::MAX_TDC);
  nHits.resize(Geometry::MAX_TDC);
  nMiss.resize(Geometry::MAX_TDC);

  for (size_t i = 0; i < Geometry::MAX_TDC; ++i ) {
    p_tdc_chnl_adc_time[i].resize(Geometry::MAX_TDC_CHANNEL);
    p_tdc_chnl_adc_time_raw[i].resize(Geometry::MAX_TDC_CHANNEL);
    p_tdc_chnl_tdc_time_corrected[i].resize(Geometry::MAX_TDC_CHANNEL);
    p_tdc_chnl_tdc_time_corrected_raw[i].resize(Geometry::MAX_TDC_CHANNEL);
    p_tdc_hit_rate[i].resize(Geometry::MAX_TDC_CHANNEL);		
    first_signal_flag[i].resize(Geometry::MAX_TDC_CHANNEL);
    nHits[i].resize(Geometry::MAX_TDC_CHANNEL);
    nMiss[i].resize(Geometry::MAX_TDC_CHANNEL);
  }
  
  
  
  for (size_t i = 0; i < Geometry::MAX_TDC; ++i) {
    for (size_t j = 0; j < Geometry::MAX_TDC_CHANNEL; ++j) {
      p_tdc_hit_rate[i][j] = 0;
    }
  }
  
  
  
  for (int i = 0; i < Geometry::MAX_TDC_CHANNEL; i++){
    p_tdc_hit_rate_x[i] = i;
  }
  for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
      int local_tdc_id = tdc_id % 18;
      h_name.Form("tdc_%d_tdc_time_spectrum_corrected", tdc_id);
      p_tdc_tdc_time_corrected[tdc_id] = new TH1F(h_name, TString::Format("tdc_%d_tdc_time_spectrum;time/ns;entries", local_tdc_id),TDC_HIST_TOTAL_BIN, TDC_HIST_LEFT, TDC_HIST_RIGHT);
      if (tdc_id >= 18) p_tdc_tdc_time_corrected[tdc_id]->SetLineColor(kRed);
      
      h_name.Form("tdc_%d_adc_time_spectrum", tdc_id);
      p_tdc_adc_time[tdc_id] = new TH1F(h_name, TString::Format("tdc_%d_adc_time_spectrum;time/ns;entries", local_tdc_id), ADC_HIST_TOTAL_BIN, ADC_HIST_LEFT, ADC_HIST_RIGHT);
      if (tdc_id >= 18) p_tdc_adc_time[tdc_id]->SetLineColor(kRed);

      h_name.Form("tdc_%d_hit_rate", tdc_id);
      p_tdc_hit_rate_graph[tdc_id] = new TH1F(h_name, TString::Format("tdc_%d_tdc_hit_rate;Channel No.;Rate(Hz)", local_tdc_id), 24, -0.5, 23.5);
      if (tdc_id < 18)
        p_tdc_hit_rate_graph[tdc_id]->SetFillColor(4);
      else
        p_tdc_hit_rate_graph[tdc_id]->SetFillColor(kRed);

      p_tdc_hit_rate_graph[tdc_id]->SetBarWidth(0.4);
      p_tdc_hit_rate_graph[tdc_id]->SetBarOffset(0.1 + 0.4*(tdc_id<18));
      p_tdc_hit_rate_graph[tdc_id]->SetStats(0);

      p_tdc_hit_rate_graph[tdc_id]->SetMaximum(1);
      p_tdc_hit_rate_graph[tdc_id]->SetMinimum(0);
      
      for(int tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
	h_name.Form("tdc_%d_chnl_%d_adc_time_spectrum", tdc_id,tdc_chnl_id);
	p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id] = new TH1F(h_name, h_name, ADC_HIST_TOTAL_BIN, ADC_HIST_LEFT, ADC_HIST_RIGHT);
	h_name.Form("tdc_%d_chnl_%d_adc_time_raw_spectrum", tdc_id,tdc_chnl_id);
	p_tdc_chnl_adc_time_raw[tdc_id][tdc_chnl_id] = new TH1F(h_name, h_name, ADC_HIST_TOTAL_BIN, ADC_HIST_LEFT, ADC_HIST_RIGHT);
	
	h_name.Form("tdc_%d_chnl_%d_tdc_time_spectrum_corrected", tdc_id,tdc_chnl_id);
	p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id] = new TH1F(h_name, h_name,TDC_HIST_TOTAL_BIN, TDC_HIST_LEFT, TDC_HIST_RIGHT);
	h_name.Form("tdc_%d_chnl_%d_tdc_time_spectrum_raw_corrected", tdc_id,tdc_chnl_id);
	p_tdc_chnl_tdc_time_corrected_raw[tdc_id][tdc_chnl_id] = new TH1F(h_name, h_name,TDC_HIST_TOTAL_BIN, TDC_HIST_LEFT, TDC_HIST_RIGHT);
	
      
    }
  } // end for: all TDC

  tube_efficiency = new TH2D("tube_efficiency", ";Layer;Column",
			       Geometry::MAX_TUBE_COLUMN,-0.5,Geometry::MAX_TUBE_COLUMN-0.5,
			       Geometry::MAX_TUBE_LAYER,-0.5,Geometry::MAX_TUBE_LAYER-0.5);

  adc_canvas = new TCanvas("c1", "ADC Plots",0,0,2160,750);
  adc_canvas->Divide(6,2);
  tdc_canvas = new TCanvas("c2", "TDC Plots",0,750,2160,750);
  tdc_canvas->Divide(6,2);
  rate_canvas = new TCanvas("c3", "Hit Rate Plots",2160,0,1800,750);
  rate_canvas->Divide(6,2);
  trigger_rate_canvas = new TCanvas("c4", "Trigger Board",1440,750,400,300);
  residual_canvas = new TCanvas("c5", "Residuals", 2100,900,400,300);
  EDCanvas = new TCanvas("c6", "Event Display", 2700, 900, 800, 800);
  eff_canvas = new TCanvas("C7", "Efficiency", 2300, 900, 400, 300);

  printf("Canvases created and divided.\n");
  for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
    if (geo.IsActiveTDC(tdc_id) || tdc_id == geo.TRIGGER_MEZZ) {
      int local_tdc_id = tdc_id%18;
      if (tdc_id == geo.TRIGGER_MEZZ){
	
	trigger_rate_canvas->cd();
	p_tdc_hit_rate_graph[tdc_id]->Draw("B");
      }
      else{
	pad_num = 6*((local_tdc_id+1)%2)+(local_tdc_id/2)+1;
        TString opts;
        if (isDrawn[tdc_id]) opts = "same";
        else opts = "";

	adc_canvas->cd(pad_num);
	p_tdc_adc_time[tdc_id]->Draw(opts);
	tdc_canvas->cd(pad_num);
	p_tdc_tdc_time_corrected[tdc_id]->Draw(opts);
	rate_canvas->cd(pad_num);
	// gPad->SetLogy();
	p_tdc_hit_rate_graph[tdc_id]->Draw(opts + " B");
	//if (gSystem->ProcessEvents()) break;
        isDrawn[tdc_id] = 1;
      }
    }
  }
  pad_num = 12;
  adc_canvas->cd();
  adc_canvas->Modified();
  adc_canvas->Update();
  tdc_canvas->cd();
  tdc_canvas->Modified();
  tdc_canvas->Update();
  rate_canvas->cd();
  rate_canvas->Modified();
  rate_canvas->Update();
  residual_canvas->cd();
  residuals->Draw();
  residual_canvas->Modified();
  residual_canvas->Update();
  eff_canvas->cd();
  tube_efficiency->Draw("colz");
  eff_canvas->Modified();
  eff_canvas->Update();
  gSystem->ProcessEvents();
  printf("Canvases updated.\n");
  
  // TCP SERVER SETUP	
  tcp_server_ip = INADDR_ANY;
  // tcp_server_ip = inet_addr("141.211.96.10");
  // tcp_server_ip = inet_addr("141.213.133.230");
  tcp_server_setup(tcp_server_ip);
  // TCP SERVER SETUP DONE
  
  
  // UDP SERVER SETUP
  udp_sock_fd = udp_client_init(UDP_PORT);
  
  in_addr_t udp_server_ip= inet_addr("127.0.0.1");
  
  memset(&udp_servaddr, 0, sizeof(udp_servaddr));       
  // Filling server information 
  udp_servaddr.sin_family = AF_INET; 
  udp_servaddr.sin_port = htons(UDP_PORT); 
  udp_servaddr.sin_addr.s_addr = udp_server_ip;
  // UDP SERVER SETUP DONE
  
  time_t sys_time;
  struct tm * timeinfo;
  sys_time = time(0);
  timeinfo = localtime(&sys_time);
  memset(filename_time, 0, sizeof(filename_time)); 
  
  strftime(filename_time, 30, "%Y%m%d_%H%M%S", timeinfo);
  sprintf(oFile_name,"./data/%s.dat",filename_time);
  oFile.open(oFile_name, ios::out | ios::binary);
  printf("File %s opened for raw data recording.\n",oFile_name);
  
  data_in_flow.open(oFile_name);
  p_output_rootfile = new TFile("output.root", "RECREATE");
  
  hitByLC = new TH2D("hitByLC", "All hits on tubes (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
  hitByLC->SetStats(0);
  badHitByLC = new TH2D("badHitByLC", "Hits on tubes outside window (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
  badHitByLC->SetStats(0);
  goodHitByLC = new TH2D("goodHitByLC", "Hits on tubes inside window (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
  goodHitByLC->SetStats(0);
  
  total_triggers = 0;
  total_events   = 0;
  total_triggers_pass = 0;
  total_events_pass = 0;
  total_signals = 0;
  total_signals_pass = 0;
  total_events_fail  = 0;
  event = Event();
  event_raw = Event();
  prevEventID = EventID(0x00000000);
  eTree = new TTree("eTree", "eTree");
  eTree->Branch("event", "Event", &event);
  
  filar_1 = new stringstream;
  filar_2 = new stringstream;
  cout << "Processing..." << endl;
    
}


void DAQ_monitor::DataDecode(){
  total_bytes_recv = 0;
  bzero(buffer,sizeof(buffer));
  bytes_recv = sock_read(newsockfd, (char *) buffer, sizeof(buffer));
  total_bytes_recv += bytes_recv;
  sockReadCount = 1;
  printf("\nReceiving data...\n");
  printf("Received message %i\n",sockReadCount);
  // time(&start_time);
  int iter = 0;     
  while (1) {	 	
    iter++;
    std::cout << "CSM NUMBER: " << buffer[2] << std::endl;
    oFile.write( (const char *) buffer,bytes_recv);
    
    bytes_recv = sock_read(newsockfd, (char *) buffer, sizeof(buffer));

    total_bytes_recv += bytes_recv;
    sockReadCount++;

    
    data_in_flow.tellg(); //Needed to ensure reading continues
    int nloop = 0;
    while (data_in_flow.read((char *) &word, sizeof(word))) {
      nloop++;
      header = word >> 28; // get the four bits header of this word
      header_type = static_cast<unsigned int>((header.to_ulong()));
      
      if (header_type == Signal::GROUP_HEADER) {
	// if (header_type == Signal::GROUP_HEADER || header_type == Signal::TDC_HEADER || header_type == Signal::TDC_TRAILER) {
	currEventID = EventID(word);

	// analyze data if we reached a header for a new event 
	if (currEventID.ID() != prevEventID.ID()) {
	  prevEventID = currEventID;
	  
	  total_events++;
          int isCSM2 = 0;
	  if (trigVec.size() == 0) {
            trigVec = nonzeroTrigVec; // HERE WE ARE READING CSM 2
            for (Signal s : sigVec) {
              s.SetIsCSM2(1);
            }
            isCSM2 = 1;
          }
	  else nonzeroTrigVec = trigVec;

	  event_raw = Event(trigVec, sigVec, currEventID);
	  ru.DoHitFinding(&event_raw,    &tc, geo);
	  for (Hit h : event_raw.WireHits()) {
            size_t tdc = h.TDC();
            if (isCSM2) tdc += 18;
	    p_tdc_chnl_adc_time_raw          [tdc][h.Channel()]->Fill(h.ADCTime()); 
	    p_tdc_chnl_tdc_time_corrected_raw[tdc][h.Channel()]->Fill(h.CorrTime()); 
	    p_tdc_tdc_time_corrected         [tdc]->Fill(h.CorrTime());
	    p_tdc_adc_time                   [tdc]->Fill(h.ADCTime()); 
	  }
	  for (Hit h : event_raw.TriggerHits()) {
	    p_tdc_chnl_adc_time_raw		[h.TDC()][h.Channel()]->Fill(h.ADCTime()); 
	    p_tdc_chnl_tdc_time_corrected_raw	[h.TDC()][h.Channel()]->Fill(h.TDCTime());  
	  }
	  
	  
	  event = Event(trigVec, sigVec, currEventID);

	  ru.DoHitFinding(&event,    &tc, geo);
	  ru.DoHitClustering(&event);
	  pass_event_check = kTRUE;
	  pass_event_check = ru.CheckEvent(event, &status);
	  event.SetPassCheck(pass_event_check);
	  event.CheckClusterTime();
	  
	  if (pass_event_check) {
	    eTree->Fill();
	    for (Cluster c : event.Clusters()) {
	      for (Hit h : c.Hits()) {
		p_tdc_chnl_adc_time	[h.TDC()][h.Channel()]->Fill(h.ADCTime()); 
		p_tdc_chnl_tdc_time_corrected	[h.TDC()][h.Channel()]->Fill(h.CorrTime()); 
		
		geo.GetHitLayerColumn(h.TDC(), h.Channel(), &hitL, &hitC);
		hitByLC->Fill(hitC, hitL);
		if (h.CorrTime() <-200 || h.CorrTime()>200)
		  badHitByLC->Fill(hitC, hitL);
		else
		  goodHitByLC->Fill(hitC, hitL);
	      }
	    }
	    TTree* optTree = new TTree("optTree", "optTree");
            optTree->Branch("event", "Event", &event);
            optTree->Fill();
	    tp.setTarget(optTree);
	    tp.setRangeSingle(0);
            tp.setIgnoreNone();
	    tp.optimize();

	    for (Cluster c : event.Clusters()) {
	      for (Hit h : c.Hits()) {
		residuals->Fill(tp.Residual(h)*1000.0);
	      }
	    }
            ed_event = event;
            std::cout << "About to fill eff underlying thing" << std::endl;
	    // fill efficiency distribution
	    double _hitX, _hitY;
	    for (int iL = 0; iL < Geometry::MAX_TUBE_LAYER; iL++) {
	      for (int iC = 0; iC < Geometry::MAX_TUBE_COLUMN; iC++) {
		if (geo.IsActiveLayerColumn(iL, iC)) {
		  geo.GetHitXY(iL, iC, &_hitX, &_hitY);
		  // get track x position and figure out what tube(s) it may go through
		  double trackDist = tp.Distance(Hit(0, 0, 0, 0, 0, 0, iL, iC, _hitX, _hitY));
		  if (trackDist <= Geometry::column_distance/2) {
		    Bool_t tubeIsHit = kFALSE;
		    for (Hit h : event.WireHits()) {
		      if (h.Layer() == iL && h.Column() == iC) tubeIsHit = kTRUE;
		    }
		    if (!tubeIsHit)   {
		      nMiss[iL][iC] = nMiss[iL][iC] + 1.0;
		    }
		    else {
		      nHits[iL][iC] = nHits[iL][iC] + 1.0;
		    }
		  } // end if: track passes through gas volume
		} // end if: check only active tubes
	      } // end for: column
	    } // end for: layer   
            std::cout << "done." << std::endl;	
            delete optTree;
	  }
	  
	  
	  for (int i = 0; i != Geometry::MAX_TDC; i++) {
	    for (int j = 0; j != Geometry::MAX_TDC_CHANNEL; j++) {
	      first_signal_flag[i][j] = kFALSE;
	    }
	  }
	  
	  if (pass_event_check) {
	    total_events_pass++;
	  }
	  else {
	    total_events_fail++;
	  }
	  
	  if (pass_event_check)
	    sprintf(track_group_name, "events_passing");
	  else
	    sprintf(track_group_name, "events_failing");
	  
	  if ((pass_event_check && total_events_pass == 1) || (!pass_event_check && total_events_fail == 1) ) {
	    event_track[(int)pass_event_check] = p_output_rootfile->mkdir(track_group_name);
	  }
	  
	  // Plot every 100th event
	  if ( pass_event_check && (total_events_pass % (10*SPEEDFACTOR) == 0)) {
	    //printf("Entered event display loop\n");
	    
	    //printf("(int) pass_event_check = %i\n", (int) pass_event_check);
	    event_track[(int)pass_event_check]->cd();
	    //ed->DrawTubeHistAndEvent(event, geo, goodHitByLC);
	    gSystem->ProcessEvents();
	    ed->Clear();
	  }
	  
	  sigVec.clear();
	  trigVec.clear();
	}
      }
      else if (header_type == Signal::RISING || header_type == Signal::FALLING) {
	sig = Signal(word, currEventID);
	
	if (!first_signal_flag[sig.TDC()][sig.Channel()]) {
	  sig.SetIsFirstSignal(kTRUE);
	  first_signal_flag[sig.TDC()][sig.Channel()] = kTRUE;
	}
	
	if (sig.TDC() == geo.TRIGGER_MEZZ && sig.Channel() == geo.TRIGGER_CH) {
	  trigVec.push_back(sig);
	  if (header_type == Signal::FALLING) {
	    total_triggers++;
	  }
	}
	else{
	  sigVec.push_back(sig);
	  if (header_type == Signal::RISING) total_signals++;
	}
      }
    }
    if (data_in_flow.fail()) {
      data_in_flow.clear();
    }
    
    for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
      if (geo.IsActiveTDC(tdc_id)) {
	for(int tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
	  p_tdc_hit_rate[tdc_id][tdc_chnl_id] = 
	    p_tdc_chnl_adc_time_raw[tdc_id][tdc_chnl_id]->GetEntries()/1.55*1000/total_events;
	}
      }
    }

    if(iter%SPEEDFACTOR==0){
      float max = 0.0;
      for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; ++tdc_id) {
        isDrawn[tdc_id] = 0;
        max = 0.0;
        for (int iBin = 1; iBin <= p_tdc_hit_rate_graph[tdc_id]->GetNbinsX(); ++iBin) {
          p_tdc_hit_rate_graph[tdc_id]->SetBinContent(iBin, p_tdc_hit_rate[tdc_id][iBin-1]);
          if (p_tdc_hit_rate[tdc_id][iBin-1] > max) max = p_tdc_hit_rate[tdc_id][iBin-1];
          p_tdc_hit_rate_graph[tdc_id]->SetMaximum(max);
       }        
      }
      
      for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
	string text_content;
	
        int CSM = tdc_id >= 18;
        int local_tdc_id = tdc_id % 18;

	if (geo.IsActiveTDC(tdc_id) || tdc_id == geo.TRIGGER_MEZZ) {
	  if (tdc_id == geo.TRIGGER_MEZZ) {
	    trigger_rate_canvas->cd();
	    text_content ="Entries = "+to_string((int)total_triggers);
	  }
	  else{
	    rate_canvas->cd(6*((local_tdc_id+1)%2)+(local_tdc_id/2)+1);
	    text_content ="Entries = "+to_string((int)p_tdc_adc_time[tdc_id]->GetEntries());
	  }
	  TString h_name;
	  h_name.Form("tdc_%d_hit_rate", local_tdc_id);
	  int brother = 0;
	  if (CSM) brother = local_tdc_id;
          else brother = tdc_id + 18;
          max = (p_tdc_hit_rate_graph[tdc_id]->GetMaximum() > p_tdc_hit_rate_graph[brother]->GetMaximum()) ? p_tdc_hit_rate_graph[tdc_id]->GetMaximum() : p_tdc_hit_rate_graph[brother]->GetMaximum();
          max = (1 > max) ? 1 : max;
          p_tdc_hit_rate_graph[tdc_id]->SetMaximum(1.25*max);

          TString opts;
          if (CSM && isDrawn[local_tdc_id]) opts = "B same";
          else opts = "B";

	  p_tdc_hit_rate_graph[tdc_id]->Draw(opts);
	  TText *xlabel = new TText();
          xlabel -> SetTextColor(kBlack);
	  xlabel -> SetNDC();
	  xlabel -> SetTextFont(42);
	  xlabel -> SetTextSize(0.05);
	  xlabel -> SetTextAngle(0);
	  xlabel -> DrawText(0.1+float(CSM)*0.4, 0.85, text_content.c_str());
	  text_content = "Max  = "+to_string(TMath::MaxElement(MAX_CHNL_COUNT,&p_tdc_hit_rate[tdc_id][0])).substr(0,6)+" kHz";
	  xlabel -> DrawText(0.1+float(CSM)*0.4, 0.8, text_content.c_str());
	  text_content = "Mean = "+to_string(TMath::Mean(MAX_CHNL_COUNT,&p_tdc_hit_rate[tdc_id][0])).substr(0,6)+" kHz";
	  xlabel -> DrawText(0.1+float(CSM)*0.4, 0.75, text_content.c_str());
          xlabel -> SetTextColor(kRed);
          xlabel -> DrawText(0.5, 0.9, "CSM 2");
          xlabel -> SetTextColor(kBlue);
          xlabel -> DrawText(0.1, 0.9, "CSM 1");
          xlabel -> SetTextColor(kBlack);
	  TLine *l = new TLine(-0.5,0.5,23.5,0.5);
	  l->Draw();

          if (tdc_id != geo.TRIGGER_MEZZ) {
            if (CSM && isDrawn[local_tdc_id]) opts = "same";
            else opts = "";
	    adc_canvas->cd(6*((local_tdc_id+1)%2)+(local_tdc_id/2)+1);
            p_tdc_adc_time[tdc_id]->Draw(opts);
            tdc_canvas->cd(6*((local_tdc_id+1)%2)+(local_tdc_id/2)+1);
            p_tdc_tdc_time_corrected[tdc_id]->Draw(opts);

	    std::cout << "About to fill actual th2" << std::endl;
	    int hitL, hitC;
	    for (int iCh = 0; iCh < Geometry::MAX_TDC_CHANNEL; ++iCh) {	      
	      geo.GetHitLayerColumn(tdc_id, iCh, &hitL, &hitC);
              if (nHits.at(tdc_id).at(iCh))
	        tube_efficiency->SetBinContent(hitC+1, hitL+1, nHits[tdc_id][iCh]/(nHits[tdc_id][iCh] + nMiss[tdc_id][iCh]));
	    }
	    std::cout << "DONE" << std::endl;
          }

          isDrawn[local_tdc_id] = 1;

	}
      }
      for (int i = 1; i != pad_num+1; i++) {
	adc_canvas->cd(i);
	gPad->Modified();
	tdc_canvas->cd(i);
	gPad->Modified();	
	rate_canvas->cd(i);
	gPad->Modified();			
      }

      // Update plots
      adc_canvas->cd();
      adc_canvas->Modified();
      adc_canvas->Update();
      tdc_canvas->cd();
      tdc_canvas->Modified();
      tdc_canvas->Update();
      rate_canvas->cd();
      rate_canvas->Update();
      trigger_rate_canvas->cd();
      trigger_rate_canvas->Update();
      residual_canvas->cd();
      residuals->Draw();
      residual_canvas->Update();
      EDCanvas->cd();
      ed_event.AddTrack(Track(tp.slope(), tp.y_int()));
      ed->DrawEvent(ed_event, geo, NULL);

      std::cout << "Updating plots..." << std::endl;
      eff_canvas->cd();
      eff_canvas->Modified();
      eff_canvas->Update();
      std::cout << "Done." << std::endl;


      struct Channel_packet p_chnl;
      
      for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
	if (geo.IsActiveTDC(tdc_id)) {
	  if (tdc_id == geo.TRIGGER_MEZZ) {
	    //write p_chnl information
	    p_chnl.tdc_id = geo.TRIGGER_MEZZ;
	    p_chnl.tdc_chnl_id = geo.TRIGGER_CH;
	    
	    //write ADC histogram
	    p_chnl.adc_entries_raw = 0;
	    for(int bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
	      p_chnl.adc_hist[bin_index] = 0;
	    }
	    //write TDC histogram
	    p_chnl.adc_entries_raw = 0;
	    for(int bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
	      p_chnl.tdc_hist[bin_index] = 0;
	    }
	    //write ADC raw histogram
	    p_chnl.adc_entries_raw = p_tdc_chnl_adc_time_raw[geo.TRIGGER_MEZZ][geo.TRIGGER_CH]->GetEntries();
	    for(int bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
	      p_chnl.adc_hist_raw[bin_index] = p_tdc_chnl_adc_time_raw[geo.TRIGGER_MEZZ][geo.TRIGGER_CH]->GetBinContent(bin_index+1);
	    }
	    //write TDC raw histogram
	    p_chnl.tdc_entries_raw = p_tdc_chnl_adc_time_raw[geo.TRIGGER_MEZZ][geo.TRIGGER_CH]->GetEntries();
	    for(int bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
	      p_chnl.tdc_hist_raw[bin_index] = p_tdc_chnl_tdc_time_corrected_raw[geo.TRIGGER_MEZZ][geo.TRIGGER_CH]->GetBinContent(bin_index+1);
	    }
	    //sending UDP packet outs
	    sendto(udp_sock_fd, (char *)&p_chnl, sizeof(p_chnl), 
		   MSG_CONFIRM, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
	  }
	  
	  
	  else {
	    for(int tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
	      //write p_chnl information
	      p_chnl.tdc_id = tdc_id;
	      p_chnl.tdc_chnl_id = tdc_chnl_id;
	      
	      //write ADC histogram
	      p_chnl.adc_entries = p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetEntries();
	      for(int bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
		p_chnl.adc_hist[bin_index] = p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
	      }
	      
	      //write TDC histogram
	      p_chnl.tdc_entries = p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetEntries();
	      for(int bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
		p_chnl.tdc_hist[bin_index] = p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
	      }
	      
	      //write ADC raw histogram
	      p_chnl.adc_entries_raw = p_tdc_chnl_adc_time_raw[tdc_id][tdc_chnl_id]->GetEntries();
	      for(int bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
		p_chnl.adc_hist_raw[bin_index] = p_tdc_chnl_adc_time_raw[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
	      }
	      
	      //write TDC raw histogram
	      p_chnl.tdc_entries_raw = p_tdc_chnl_tdc_time_corrected_raw[tdc_id][tdc_chnl_id]->GetEntries();
	      for(int bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
		p_chnl.tdc_hist_raw[bin_index] = p_tdc_chnl_tdc_time_corrected_raw[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
	      }
	      
	      //sending UDP packet out
	      // printf("TDC=%d, CHNL=%d, Entries=%d\n",p_chnl.tdc_id,p_chnl.tdc_chnl_id,p_chnl.adc_entries);
	      sendto(udp_sock_fd, (char *)&p_chnl, sizeof(p_chnl), 
		     MSG_CONFIRM, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
	    }										    	
	  }
	}
       std::cout << "Wrote out packet" << std::endl;
      }
      if(bytes_recv<=0)break;
    }
    
    if (gSystem->ProcessEvents()) {
      std::cout << "Processing Events" << std::endl;
      break;
    }
  }
  
  p_output_rootfile->cd();
  for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
    if (geo.IsActiveTDC(tdc_id)) {
      if (tdc_id == geo.TRIGGER_MEZZ) continue;
      p_tdc_adc_time[tdc_id]->Write();
      p_tdc_tdc_time_corrected[tdc_id]->Write();
    }
  }
  
  p_output_rootfile->Write();
  eTree->Write();
  int nEntries = eTree->GetEntries();
  delete p_output_rootfile;
  
  
  oFile.close();
  close(newsockfd);
  close(sockfd);
  data_in_flow.close();
  
  
  // create output file
  
  system("mkdir output");
  chdir("output");
  char output_directoryname[256];
  memset(output_directoryname, 0, sizeof(output_directoryname));
  sprintf(output_directoryname,"mkdir %s",filename_time);
  system(output_directoryname);
  chdir(filename_time);
  
  char rate_canvas_name[256];
  memset(rate_canvas_name, 0, sizeof(rate_canvas_name));
  sprintf(rate_canvas_name,"%s_rate.png",filename_time);
  rate_canvas->Print(rate_canvas_name);
  
  char adc_canvas_name[256];
  memset(adc_canvas_name, 0, sizeof(adc_canvas_name));
  sprintf(adc_canvas_name,"%s_adc.png",filename_time);
  adc_canvas->Print(adc_canvas_name);
  
  char tdc_canvas_name[256];
  memset(tdc_canvas_name, 0, sizeof(tdc_canvas_name));
  sprintf(tdc_canvas_name,"%s_tdc.png",filename_time);
  tdc_canvas->Print(tdc_canvas_name);
  
  char trigger_canvas_name[256];
  memset(trigger_canvas_name, 0, sizeof(trigger_canvas_name));
  sprintf(trigger_canvas_name,"%s_trigger_rate.png",filename_time);
  trigger_rate_canvas->Print(trigger_canvas_name);
  
  char fp_rate_File_name[256];
  memset(fp_rate_File_name, 0, sizeof(fp_rate_File_name)); 
  sprintf(fp_rate_File_name,"%s_rate.csv",filename_time);
  
  fp_rate_File=fopen(fp_rate_File_name,"w");
  fprintf(fp_rate_File,"tdc_id,");
  for(int tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
    fprintf(fp_rate_File,"%d,",tdc_chnl_id);
  }
  for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
    if (geo.IsActiveTDC(tdc_id)) {
      fprintf(fp_rate_File,"\n%d,",tdc_id);
      for(int tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
	fprintf(fp_rate_File,"%.4f,",p_tdc_hit_rate[tdc_id][tdc_chnl_id]);
      }
    }
  }
  fprintf(fp_rate_File,"%d,",total_events);
  fclose(fp_rate_File);
  
  
  
  
  printf("Files and sockets closed.\n");
  printf("Socket was read %u times.\n", sockReadCount);
  printf("Socket received %u bytes of data.\n", total_bytes_recv);
  printf("Total Triggers: %lu\n",total_triggers);
  printf("Total Events: %lu\n",total_events);
  printf("Pass Triggers: %lu\n",total_triggers_pass);
  printf("Pass Events: %lu\n",total_events_pass);
  printf("Total Signals: %lu\n",total_signals);
  printf("Pass Triggers: %lu\n",total_triggers_pass);
  printf("N tree entries: %d\n",nEntries);
  
  printf("\n ==== Program Done ==== \n");
  
  return; 
  
}




void DAQ_monitor::tcp_server_setup(in_addr_t server_ip_int){
  socklen_t clilen;
  char server_ip_address[INET_ADDRSTRLEN],client_ip_address[INET_ADDRSTRLEN];
  struct sockaddr_in serv_addr, cli_addr;
  
  // Creating socket file descriptor 
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
    perror("socket failed"); 
    exit(EXIT_FAILURE); 
  }
  //Forcefully attaching socket to the port number from main() argument
  //error for local IP
  int opt = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
    perror("setsockopt"); 
    exit(EXIT_FAILURE); 
  } 
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = server_ip_int;
  serv_addr.sin_port = htons(tcp_portno);
  inet_ntop(AF_INET, &(serv_addr.sin_addr), server_ip_address, INET_ADDRSTRLEN);
  printf("Waiting for client to connect to IP: %s Port: %u\n", server_ip_address, tcp_portno);
  
  // Forcefully attaching socket  
  if (bind(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr))<0) { 
    perror("bind failed"); 
    exit(EXIT_FAILURE); 
  } 
  if (listen(sockfd, 5) < 0) { 
    perror("listen"); 
    exit(EXIT_FAILURE); 
  } 
  
  clilen = sizeof(cli_addr);
  if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen))<0) { 
    perror("accept"); 
    exit(EXIT_FAILURE); 
  }
  time_t local_time;
  local_time = time(0);
  inet_ntop(AF_INET, &(cli_addr.sin_addr), client_ip_address, INET_ADDRSTRLEN);
  printf("Connected to client IP:%s\n at %s\n", client_ip_address, ctime(&local_time));
  return;	
}



void DAQ_monitor::error(const char *msg)
{
  perror(msg);
  exit(1);
}

#endif


int DAQ(short portno, int bisno=1){
  DAQ_monitor *p_DAQ_monitor = new DAQ_monitor(portno, bisno);
  p_DAQ_monitor->DataDecode();
  return 1;
}
