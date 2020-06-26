// /* A simple server in the internet domain using TCP
//    The port number is passed as an argument */
// //#ifdef __cplusplus
// //extern "C" {
// //#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

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

#include "macros/server_setup.c"
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
#include "src/CheckEvent.cpp"

// global define inculdes
#include "macros/GlobalIncludes.h"

// Muon Reconstruction includes
#include "src/Signal.cpp"
#include "src/EventID.cpp"
#include "src/Event.cpp"
#include "src/EventDisplay.cpp"
#include "src/Geometry.cpp"
#include "src/Hit.cpp"
#include "src/Cluster.cpp"
#include "src/TimeCorrection.cpp"
#include "src/HitFinder.cpp"
#include "src/HitCluster.cpp"

//#define DECODER_DEBUG // comment this line when debugging information is not needed
//#define SET_MAXWORDS // comment this line if you want to decode the whole data words
#define SAVE_TRACKS_OUT_OF_ROOT // comment this line if you don't need to save plots out of rootfile 

 
#define SPEEDFACTOR 1

using namespace std;
using namespace Muon;
void error(const char *msg)
{
	perror(msg);
	exit(1);
}
void server_setup(int* sockfd, int* newsockfd, in_addr_t server_ip_int, short portno);


int onlineDAQ(int portno)
{
	TDirectory *event_track[2];
	char track_group_name[128];

	TH1F *p_tdc_adc_time[Geometry::MAX_TDC];
	TH1F *p_tdc_tdc_time_corrected[Geometry::MAX_TDC];

	TH1F *p_tdc_chnl_adc_time[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
	TH1F *p_tdc_chnl_tdc_time_corrected[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];

	TH1F *p_tdc_chnl_adc_time_raw[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
	TH1F *p_tdc_chnl_tdc_time_corrected_raw[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];

	Geometry geo = Geometry();
	geo.SetRunN(0);
	// static TimeCorrection tc = TimeCorrection();
	// static EventDisplay   ed = EventDisplay();

	// TString h_name;
	// for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
	// 	if (geo.IsActiveTDC(tdc_id)) {
	// 		if (tdc_id == 5) continue;

	// 		h_name.Form("tdc_%d_tdc_time_spectrum_corrected", tdc_id);
	// 		p_tdc_tdc_time_corrected[tdc_id] = new TH1F(h_name, h_name,TDC_HIST_TOTAL_BIN, TDC_HIST_LEFT, TDC_HIST_RIGHT);
	// 		p_tdc_tdc_time_corrected[tdc_id]->GetXaxis()->SetTitle("time/ns");
	// 		p_tdc_tdc_time_corrected[tdc_id]->GetYaxis()->SetTitle("entries");

	// 		h_name.Form("tdc_%d_adc_time_spectrum", tdc_id);
	// 		p_tdc_adc_time[tdc_id] = new TH1F(h_name, h_name, ADC_HIST_TOTAL_BIN, ADC_HIST_LEFT, ADC_HIST_RIGHT);
	// 		p_tdc_adc_time[tdc_id]->GetXaxis()->SetTitle("time/ns");
	// 		p_tdc_adc_time[tdc_id]->GetYaxis()->SetTitle("entries");
	// 		for(Int_t tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
	// 			h_name.Form("tdc_%d_chnl_%d_adc_time_spectrum", tdc_id,tdc_chnl_id);
	// 			p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id] = new TH1F(h_name, h_name, ADC_HIST_TOTAL_BIN, ADC_HIST_LEFT, ADC_HIST_RIGHT);
	// 			h_name.Form("tdc_%d_chnl_%d_adc_time_raw_spectrum", tdc_id,tdc_chnl_id);
	// 			p_tdc_chnl_adc_time_raw[tdc_id][tdc_chnl_id] = new TH1F(h_name, h_name, ADC_HIST_TOTAL_BIN, ADC_HIST_LEFT, ADC_HIST_RIGHT);

	// 			h_name.Form("tdc_%d_chnl_%d_tdc_time_spectrum_corrected", tdc_id,tdc_chnl_id);
	// 			p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id] = new TH1F(h_name, h_name,TDC_HIST_TOTAL_BIN, TDC_HIST_LEFT, TDC_HIST_RIGHT);
	// 			h_name.Form("tdc_%d_chnl_%d_tdc_time_spectrum_raw_corrected", tdc_id,tdc_chnl_id);
	// 			p_tdc_chnl_tdc_time_corrected_raw[tdc_id][tdc_chnl_id] = new TH1F(h_name, h_name,TDC_HIST_TOTAL_BIN, TDC_HIST_LEFT, TDC_HIST_RIGHT);
			
	// 		}
	// 	}
	// } // end for: all TDC

	 // //clock_t tStart = clock();
	 // TCanvas *adc_canvas = new TCanvas("c1", "ADC Plots");
	 // adc_canvas->Divide(3,2);

	 // TCanvas *tdc_canvas = new TCanvas("c2", "TDC Plots");
	 // tdc_canvas->Divide(3,2);
	 // tdc_canvas->SetWindowPosition(710,0);
	 // printf("Canvases created and divided.\n");

  // 	 int pad_num = 1;
	 // for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
	 // 	if (geo.IsActiveTDC(tdc_id)) {
	 // 		if (tdc_id == 5) continue;
	 // 		adc_canvas->cd(pad_num);
	 // 		p_tdc_adc_time[tdc_id]->Draw();
	 // 		tdc_canvas->cd(pad_num);
	 // 		p_tdc_tdc_time_corrected[tdc_id]->Draw();
	 // 		//printf("Created pads %i for tdc %i.\n",pad_num,tdc_id);
	 // 		//sleep(5);
	 // 		pad_num++;
	 // 		//if (gSystem->ProcessEvents()) break;
		// }
	 // }
	 // adc_canvas->cd();
	 // adc_canvas->Modified();
	 // adc_canvas->Update();
	 // tdc_canvas->cd();
	 // tdc_canvas->Modified();
	 // tdc_canvas->Update();
	 // gSystem->ProcessEvents();
	 // printf("Canvases updated.\n");
	 // //printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);


	//  // TCP SERVER SETUP
	// int sockfd, newsockfd;
	// in_addr_t server_ip;
	// server_ip = INADDR_ANY;
	// // server_ip = inet_addr("141.211.96.10");
	// // server_ip = inet_addr("141.213.133.230");
	// server_setup(&sockfd, &newsockfd, INADDR_ANY, portno);

 //    // TCP SERVER SETUP DONE


 //    // UDP SERVER SETUP

 //    int udp_sock_fd;
 // 	udp_sock_fd = udp_client_init(UDP_PORT);

 // 	in_addr_t udp_server_ip= inet_addr("127.0.0.1");
	// struct sockaddr_in udp_servaddr; 
	// memset(&udp_servaddr, 0, sizeof(udp_servaddr));       
 //    // Filling server information 
 //    udp_servaddr.sin_family = AF_INET; 
 //    udp_servaddr.sin_port = htons(UDP_PORT); 
 //    udp_servaddr.sin_addr.s_addr = udp_server_ip;
 //    // UDP SERVER SETUP DONE





	// unsigned int buffer[4096];
	// int sockReadCount, bytes_recv, total_bytes_recv;

	// ofstream oFile;
	// time_t sys_time;
	// struct tm * timeinfo;
	// char timestr[30];
	// sys_time = time(0);
	// timeinfo = localtime(&sys_time);
	// strftime(timestr, 30, "./data/%Y%m%d_%H%M%S.dat", timeinfo);
	// printf("%s\n",timestr);
	// // timestr = "1.dat";
	// oFile.open(timestr, ios::out | ios::binary);
	// printf("File %s opened for raw data recording.\n",timestr);



 //     // PLOTTING SETU
 //  	 TString input_filename = timestr;
 //  	 ifstream data_in_flow;
 //   	 data_in_flow.open(input_filename.Data());


 //  	 TFile *p_output_rootfile = new TFile("output.root", "RECREATE");

	//  TH2D* hitByLC = new TH2D("hitByLC", "All hits on tubes (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
 //  	 hitByLC->SetStats(0);
	//  TH2D* badHitByLC = new TH2D("badHitByLC", "Hits on tubes outside window (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
	//  badHitByLC->SetStats(0);
 //  	 TH2D* goodHitByLC = new TH2D("goodHitByLC", "Hits on tubes inside window (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
	//  goodHitByLC->SetStats(0);

	//  int hitL, hitC;
	//  int current_track_group = 0;
	//  Bool_t first_signal_flag[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
	//  int temp_track_group;
	//  bool pass_event_check;
	//  unsigned int word;
	//  unsigned int header_type;
	//  EventID currEventID;
	//  EventID prevEventID = EventID(0x00000000);
	//  vector<Signal> trigVec;
	//  vector<Signal>  sigVec;
	//  bitset<4> header;
	//  Signal sig;
	//  Event  event = Event();
	//  Event  event_raw = Event();
	//  TTree* eTree = new TTree("eTree", "eTree");
	//  eTree->Branch("event", "Event", &event);
	//  cout << "Processing..." << endl;

	//  unsigned long total_triggers = 0;
	//  unsigned long total_events   = 0;
	//  unsigned long total_triggers_pass = 0;
	//  unsigned long total_events_pass = 0;
	//  unsigned long total_signals = 0;
	//  unsigned long total_signals_pass = 0;
	//  unsigned long total_events_fail  = 0;


 //  	 // DECODING / PLOTTING SETUP DONE


 //     total_bytes_recv = 0;
 //     bzero(buffer,sizeof(buffer));
 //     bytes_recv = sock_read(newsockfd, (char *) buffer, sizeof(buffer));
 //     total_bytes_recv += bytes_recv;
 //     sockReadCount = 1;
 //     printf("\nReceiving data...\n");
 //     printf("Received message %i\n",sockReadCount);
 //     int iter = 0;
	//  while (bytes_recv > 0) {
	// 	iter++;
	// 	oFile.write( (const char *) buffer,bytes_recv);

	// 	bytes_recv = sock_read(newsockfd, (char *) buffer, sizeof(buffer));
	// 	total_bytes_recv += bytes_recv;
	// 	sockReadCount++;
	// 	printf("Received Packet %i\n",sockReadCount);


	// 	//printf("current position in file =  %i\n", (int) data_in_flow.tellg());
	// 	//clock_t tStart = clock();
	// 	data_in_flow.tellg(); //Needed to ensure reading continues
	// 	int nloop = 0;
	// 	while (data_in_flow.read((char *) &word, sizeof(word))) {
	//  	 	//if (nloop == 0) printf("Started reading file again.\n");
	//  	 	nloop++;
 //    		header = word >> 28; // get the four bits header of this word
 //    		header_type = static_cast<unsigned int>((header.to_ulong()));
    
 //    		if (header_type == Signal::GROUP_HEADER || header_type == Signal::TDC_HEADER || header_type == Signal::TDC_TRAILER) {
 //    			currEventID = EventID(word);

 //    			// analyze data if we reached a header for a new event 
 //      			if (currEventID.ID() != prevEventID.ID()) {
 //      				//cout << "got back here" << endl;
 //      				prevEventID = currEventID;
 //      				// if (total_events %(100*SPEEDFACTOR) == 0) cout << "Processing Event " << total_events << endl;

 //      				total_events++;
              		
 //              		event_raw = Event(trigVec, sigVec, currEventID);
 //              		DoHitFinding(&event_raw,    tc, 0);
 //              		for (Hit h : event_raw.WireHits()) {
 //      					p_tdc_chnl_adc_time_raw				[h.TDC()][h.Channel()]->Fill(h.ADCTime()); 
 //      					p_tdc_chnl_tdc_time_corrected_raw	[h.TDC()][h.Channel()]->Fill(h.CorrTime()); 
 //      				}

 //              		event = Event(trigVec, sigVec, currEventID);

 //      				DoHitFinding(&event,    tc, 40);
	// 		      	DoHitClustering(&event, geo);
	// 		      	// pass_event_check = kTRUE;
	// 		      	pass_event_check = CheckEvent(event, geo);
	// 		      	event.SetPassCheck(pass_event_check);
	// 		      	event.CheckClusterTime();
	// 		      	//cout << "1" << endl;

	// 		      	if (pass_event_check) {
	// 		      		//cout << "2" << endl;
	// 		      		//printf(".");
	// 		      		eTree->Fill();
 //      	  				for (Cluster c : event.Clusters()) {
 //      	  					for (Hit h : c.Hits()) {
 //      	  						p_tdc_tdc_time_corrected[h.TDC()]->Fill(h.CorrTime());
 //      	      					p_tdc_adc_time          [h.TDC()]->Fill(h.ADCTime()); 

 //      	      					p_tdc_chnl_adc_time				[h.TDC()][h.Channel()]->Fill(h.ADCTime()); 
 //      	      					p_tdc_chnl_tdc_time_corrected	[h.TDC()][h.Channel()]->Fill(h.CorrTime()); 

	// 			      	        geo.GetHitLayerColumn(h.TDC(), h.Channel(), &hitL, &hitC);
	// 			      	        hitByLC->Fill(hitC, hitL);
	// 			      	        if (h.CorrTime() <-200 || h.CorrTime()>200)
	// 			      	        	badHitByLC->Fill(hitC, hitL);
	// 			      	        else
	// 			      	        	goodHitByLC->Fill(hitC, hitL);
	// 			      	        //p_hits_distribution[hitL]->Fill(hitC);
	// 			      	    }
	// 			      	}
	// 			    }

	// 			    //cout << "4" << endl;

	// 			    for (Int_t i = 0; i != Geometry::MAX_TDC; i++) {
	// 			    	for (Int_t j = 0; j != Geometry::MAX_TDC_CHANNEL; j++) {
	// 			    		first_signal_flag[i][j] = kFALSE;
	// 			    	}
	// 			    }

	// 			    if (pass_event_check) {
	// 			    	total_events_pass++;
	// 			    }
	// 		      	else {
	// 		      		total_events_fail++;
	// 		      	}

	// 		      	if (pass_event_check)
 //  	    				sprintf(track_group_name, "events_passing");
 //  	  				else
 //  	    				sprintf(track_group_name, "events_failing");

	// 		      	if ((pass_event_check && total_events_pass == 1) || (!pass_event_check && total_events_fail == 1) ) {
 //      	  				event_track[(int)pass_event_check] = p_output_rootfile->mkdir(track_group_name);
 //      	    			//printf("(int) pass_event_check = %i\n", (int) pass_event_check);
 //      	  			}

	// 		      	// Plot every 100th event
	// 		      	if ( pass_event_check && (total_events_pass % (50*SPEEDFACTOR) == 0)) {
	// 		      		//printf("Entered event display loop\n");

 //      	    			//printf("(int) pass_event_check = %i\n", (int) pass_event_check);
	// 		      		event_track[(int)pass_event_check]->cd();
	// 		      		ed.DrawEvent(event, geo, NULL);
	// 		      		gSystem->ProcessEvents();
	// 		      		ed.Clear();
	// 		      	}

	// 		      	sigVec.clear();
 //      				trigVec.clear();
	// 		    }
	// 		}
	// 		else if (header_type == Signal::RISING || header_type == Signal::FALLING) {
	// 			sig = Signal(word, currEventID);

	// 			if (!first_signal_flag[sig.TDC()][sig.Channel()]) {
	// 				sig.SetIsFirstSignal(kTRUE);
	// 				first_signal_flag[sig.TDC()][sig.Channel()] = kTRUE;
	// 			}

	// 			if (sig.TDC() == geo.TRIGGER_MEZZ && sig.Channel() == geo.TRIGGER_CH) {
	// 				trigVec.push_back(sig);
	// 				if (header_type == Signal::RISING) total_triggers++;
	// 			}
	// 			else if (sig.TDC() != geo.TRIGGER_MEZZ) {
	// 				sigVec.push_back(sig);
	// 				if (header_type == Signal::RISING) total_signals++;
	// 			}
	// 		}
	// 	}  //while (bytes_recv > 0)
	// 	if (data_in_flow.fail()) {
	// 		data_in_flow.clear();
	// 	}

	// 	if(iter%SPEEDFACTOR==0){

	// 		for (int i = 1; i != pad_num+1; i++) {
	// 			adc_canvas->cd(i);
	// 			gPad->Modified();
	// 			tdc_canvas->cd(i);
	// 			gPad->Modified();
	// 		}

	// 		// Update plots
	// 		adc_canvas->cd();
	// 		adc_canvas->Modified();
	// 	 	adc_canvas->Update();
	// 	 	tdc_canvas->cd();
	// 		tdc_canvas->Modified();
	// 	 	tdc_canvas->Update();


			
			




	// 	 	struct Channel_packet p_chnl;

	// 	 	for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
	// 		 	if (geo.IsActiveTDC(tdc_id)) {
	// 		 		if (tdc_id == 5) continue;


	// 			    for(Int_t tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
	// 			    	//write p_chnl information
	// 			    	p_chnl.tdc_id = tdc_id;
	// 			    	p_chnl.tdc_chnl_id = tdc_chnl_id;

	// 			    	//write ADC histogram
	// 			    	p_chnl.adc_entries = p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetEntries();
	// 			    	for(Int_t bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
	// 						p_chnl.adc_hist[bin_index] = p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
	// 					}
						
	// 					//write TDC histogram
	// 					p_chnl.tdc_entries = p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetEntries();
	// 			    	for(Int_t bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
	// 						p_chnl.tdc_hist[bin_index] = p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
	// 					}

	// 					//write ADC raw histogram
	// 			    	p_chnl.adc_entries_raw = p_tdc_chnl_adc_time_raw[tdc_id][tdc_chnl_id]->GetEntries();
	// 			    	for(Int_t bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
	// 						p_chnl.adc_hist_raw[bin_index] = p_tdc_chnl_adc_time_raw[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
	// 					}
						
	// 					//write TDC raw histogram
	// 					p_chnl.tdc_entries_raw = p_tdc_chnl_tdc_time_corrected_raw[tdc_id][tdc_chnl_id]->GetEntries();
	// 			    	for(Int_t bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
	// 						p_chnl.tdc_hist_raw[bin_index] = p_tdc_chnl_tdc_time_corrected_raw[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
	// 					}

	// 					//sending UDP packet out
	// 					// printf("TDC=%d, CHNL=%d, Entries=%d\n",p_chnl.tdc_id,p_chnl.tdc_chnl_id,p_chnl.adc_entries);
	// 					sendto(udp_sock_fd, (char *)&p_chnl, sizeof(p_chnl), 
 //        				MSG_CONFIRM, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));										    	
	// 			    }
	// 			}
	// 		}

				

		 	

	// 	 // 	ofstream outputfile;
	// 	 // 	string outputfile_name = "chnl_hist.csv";
	// 		// outputfile.open(outputfile_name.c_str());

	// 		// for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
	// 		//  	if (geo.IsActiveTDC(tdc_id)) {
	// 		//  		if (tdc_id == 5) continue;

	// 		// 	    for(Int_t tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
	// 		// 	    	//write ADC histogram:tdc_id, chnl_id, 0, hist_content
	// 		// 	    	outputfile<<tdc_id<<","<<tdc_chnl_id<<",0,"\
	// 		// 	    	<<p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetEntries()<<",";
	// 		// 	    	for(Int_t bin_index=1;bin_index<TOTAL_BIN_QUANTITY/2;bin_index++){
	// 		// 				outputfile<<p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetBinContent(bin_index)<<",";
	// 		// 			}
	// 		// 			outputfile<<p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetBinContent(TOTAL_BIN_QUANTITY/2)<<endl;

	// 		// 			//write TDC histogram:tdc_id, chnl_id, 1, hist_content
	// 		// 			outputfile<<tdc_id<<","<<tdc_chnl_id<<",1,"\
	// 		// 			<<p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetEntries()<<",";
	// 		// 	    	for(Int_t bin_index=1;bin_index<TOTAL_BIN_QUANTITY;bin_index++){
	// 		// 				outputfile<<p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetBinContent(bin_index)<<",";
	// 		// 			}						
	// 		// 			outputfile<<p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetBinContent(TOTAL_BIN_QUANTITY)<<endl;				    	
	// 		// 	    }
	// 		// 	}
	// 		// } // end for: all TDC
	// 		// outputfile.close();
	// 		// udp_client(8080, outputfile_name.c_str());

	//  	}

	//  	if (gSystem->ProcessEvents())
 //            break;

 //        //printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

	//  }
	//  //f = fwrite(buffer, sizeof(unsigned int), sizeof(buffer), incomingDataFile);
	//  //if (f < 0) error("ERROR writing to file");
	//  //The last sock_read returns -1 since the client closed socket so final write doesn't happen

	//  p_output_rootfile->cd();
	//  for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
	//  	if (geo.IsActiveTDC(tdc_id)) {
	//  		if (tdc_id == 5) continue;
	//  		p_tdc_adc_time[tdc_id]->Write();
	//  		p_tdc_tdc_time_corrected[tdc_id]->Write();
	//  	}
	//  }

	//  p_output_rootfile->Write();
	//  eTree->Write();
	//  int nEntries = eTree->GetEntries();
	//  delete p_output_rootfile;

	//  oFile.close();
	//  close(newsockfd);
	//  close(sockfd);
	//  data_in_flow.close();
	//  printf("Files and sockets closed.\n");
	//  printf("Socket was read %u times.\n", sockReadCount);
	//  printf("Socket received %u bytes of data.\n", total_bytes_recv);

	//  printf("Total Triggers: %lu\n",total_triggers);
	//  printf("Total Events: %lu\n",total_events);
	//  printf("Pass Triggers: %lu\n",total_triggers_pass);
	//  printf("Pass Events: %lu\n",total_events_pass);
	//  printf("Total Signals: %lu\n",total_signals);
	//  printf("Pass Triggers: %lu\n",total_triggers_pass);
	//  printf("N tree entries: %d\n",nEntries);

	//  printf("\n ==== Program Done ==== \n");

	 return 0; 
}





// //#ifdef __cplusplus
// //}
// //#endif
