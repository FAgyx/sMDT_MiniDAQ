/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
//#ifdef __cplusplus
//extern "C" {
//#endif

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

// ROOT includes
#include "TFile.h"
#include "TDirectory.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1.h"

// Geometry includes
#include "src/CheckEvent.cpp"

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

#define TOTAL_BIN_QUANTITY 1024 // set bin quantity of the plot 

using namespace std;
using namespace Muon;

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int server_plotting(int portno)
{
	 TH1F *p_tdc_adc_time[Geometry::MAX_TDC];
	 TH1F *p_tdc_tdc_time_corrected[Geometry::MAX_TDC];

	 Geometry geo = Geometry();
  	 geo.SetRunN(0);
  	 static TimeCorrection tc = TimeCorrection();

  	 TString h_name;
  	 for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
  	 	if (geo.IsActiveTDC(tdc_id)) {
  	 		if (tdc_id == 5) continue;

	  	 	h_name.Form("tdc_%d_tdc_time_spectrum_corrected", tdc_id);
	    	p_tdc_tdc_time_corrected[tdc_id] = new TH1F(h_name, h_name,TOTAL_BIN_QUANTITY, -400, 400);
	    	p_tdc_tdc_time_corrected[tdc_id]->GetXaxis()->SetTitle("time/ns");
	    	p_tdc_tdc_time_corrected[tdc_id]->GetYaxis()->SetTitle("entries");
		    
		    h_name.Form("tdc_%d_adc_time_spectrum", tdc_id);
		    p_tdc_adc_time[tdc_id] = new TH1F(h_name, h_name, TOTAL_BIN_QUANTITY / 2, 0, 400);
		    p_tdc_adc_time[tdc_id]->GetXaxis()->SetTitle("time/ns");
		    p_tdc_adc_time[tdc_id]->GetYaxis()->SetTitle("entries");
		}
	 } // end for: all TDC

	 //clock_t tStart = clock();
	 TCanvas *adc_canvas = new TCanvas("c1", "ADC Plots");
	 adc_canvas->Divide(3,2);

	 TCanvas *tdc_canvas = new TCanvas("c2", "TDC Plots");
	 tdc_canvas->Divide(3,2);
	 tdc_canvas->SetWindowPosition(710,0);
	 printf("Canvases created and divided.\n");

  	 int pad_num = 1;
	 for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
	 	if (geo.IsActiveTDC(tdc_id)) {
	 		if (tdc_id == 5) continue;
	 		adc_canvas->cd(pad_num);
	 		p_tdc_adc_time[tdc_id]->Draw();
	 		tdc_canvas->cd(pad_num);
	 		p_tdc_tdc_time_corrected[tdc_id]->Draw();
	 		//printf("Created pads %i for tdc %i.\n",pad_num,tdc_id);
	 		//sleep(5);
	 		pad_num++;
	 		//if (gSystem->ProcessEvents()) break;
		}
	 }
	 adc_canvas->cd();
	 adc_canvas->Modified();
	 adc_canvas->Update();
	 tdc_canvas->cd();
	 tdc_canvas->Modified();
	 tdc_canvas->Update();
	 gSystem->ProcessEvents();
	 printf("Canvases updated.\n");
	 //printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);


	 // SERVER SETUP
	 int sockfd, newsockfd;
	 socklen_t clilen;
	 char server_ip_address[INET_ADDRSTRLEN],client_ip_address[INET_ADDRSTRLEN];
	 unsigned int buffer[4096];
	 struct sockaddr_in serv_addr, cli_addr;
	 int b,n,f,c;
	 int sockReadCount, bytes_recv, total_bytes_recv;
	 FILE *incomingDataFile, *logFile;
	 size_t	uint_size, char_size;
  	 time_t localTime;
  	 struct tm *localDateTime;
     char myDateTime[30];

	 uint_size = sizeof(unsigned int);	// 4
	 char_size = sizeof(char); 			// 1

	 /*if (argc < 2) {
		 fprintf(stderr,"ERROR, no port provided\n");
		 exit(1);
	 }*/
	 sockfd = socket(AF_INET, SOCK_STREAM, 0);
	 if (sockfd < 0) 
		error("ERROR opening socket");
	 bzero((char *) &serv_addr, sizeof(serv_addr));
	 serv_addr.sin_family = AF_INET;
	 serv_addr.sin_addr.s_addr = INADDR_ANY;
	 //serv_addr.sin_addr.s_addr = inet_addr("141.211.96.35");
	 serv_addr.sin_port = htons(portno);

	 inet_ntop(AF_INET, &(serv_addr.sin_addr), server_ip_address, INET_ADDRSTRLEN);
	 printf("Waiting for client to connect to IP: %s Port: %u\n", server_ip_address, portno);
	 
	 b = ::bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr));
	 if (b < 0) error("ERROR on binding");

	 listen(sockfd,5);
	 clilen = sizeof(cli_addr);
	 newsockfd = accept(sockfd, 
				 (struct sockaddr *) &cli_addr, 
				 &clilen);
	 if (newsockfd < 0) 
		  error("ERROR on accept");

	 inet_ntop(AF_INET, &(cli_addr.sin_addr), client_ip_address, INET_ADDRSTRLEN);
	 printf("Connected to client IP:%s\n", client_ip_address);

	 ofstream oFile;
     oFile.open("received_data.dat", ios::out | ios::binary);

     /*logFile = NULL;
     logFile = fopen("serv_socket.log","w");
     if (logFile == NULL) printf("Unable to open log file.\n");*/

     /*localTime = time(NULL);
  	 localDateTime = localtime(&localTime);
  	 strftime(myDateTime, 30, "%Y%m%d", localDateTime);
  	 sscanf(myDateTime,"%x", &DAQStartDate);
  	 strftime(myDateTime, 30, "%H%M%S", localDateTime);*/

     // SERVER SETUP DONE

     // PLOTTING SETU
  	 TString input_filename = "received_data.dat";
  	 ifstream data_in_flow;
   	 data_in_flow.open(input_filename.Data());


  	 TFile *p_output_rootfile = new TFile("output.root", "RECREATE");

	 TH2D* hitByLC = new TH2D("hitByLC", "All hits on tubes (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
  	 hitByLC->SetStats(0);
	 TH2D* badHitByLC = new TH2D("badHitByLC", "Hits on tubes outside window (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
	 badHitByLC->SetStats(0);
  	 TH2D* goodHitByLC = new TH2D("goodHitByLC", "Hits on tubes inside window (that passed clustering)", 54,-0.5,53.5,8,-0.5,7.5);
	 goodHitByLC->SetStats(0);

	 int hitL, hitC;
	 int current_track_group = 0;
	 Bool_t first_signal_flag[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
	 int temp_track_group;
	 bool pass_event_check;
	 unsigned int word;
	 unsigned int header_type;
	 EventID currEventID;
	 EventID prevEventID = EventID(0x00000000);
	 vector<Signal> trigVec;
	 vector<Signal>  sigVec;
	 bitset<4> header;
	 Signal sig;
	 Event  event = Event();
	 TTree* eTree = new TTree("eTree", "eTree");
	 eTree->Branch("event", "Event", &event);
	 cout << "Processing..." << endl;

	 unsigned long total_triggers = 0;
	 unsigned long total_events   = 0;
	 unsigned long total_triggers_pass = 0;
	 unsigned long total_events_pass = 0;
	 unsigned long total_signals = 0;
	 unsigned long total_signals_pass = 0;
	 unsigned long total_events_fail  = 0;


  	 // DECODING / PLOTTING SETUP DONE


     total_bytes_recv = 0;
     bzero(buffer,sizeof(buffer));
     bytes_recv = sock_read(newsockfd, (char *) buffer, sizeof(buffer));
     total_bytes_recv += bytes_recv;
     sockReadCount = 1;
     printf("\nReceiving data...\n");
     printf("Received message %i\n",sockReadCount);
     int iter = 0;
	 while (bytes_recv > 0) {
	 	 iter++;
	 	 oFile.write( (const char *) buffer,bytes_recv);
		 
	 	 bytes_recv = sock_read(newsockfd, (char *) buffer, sizeof(buffer));
	 	 total_bytes_recv += bytes_recv;
	 	 sockReadCount++;
	 	 printf("Received Packet %i\n",sockReadCount);

	 	 
	 	 //printf("current position in file =  %i\n", (int) data_in_flow.tellg());
	 	 //clock_t tStart = clock();
	 	 data_in_flow.tellg(); //Needed to ensure reading continues
	 	 int nloop = 0;
	 	 while (data_in_flow.read((char *) &word, sizeof(word))) {
	 	 	//if (nloop == 0) printf("Started reading file again.\n");
	 	 	nloop++;
    		header = word >> 28; // get the four bits header of this word
    		header_type = static_cast<unsigned int>((header.to_ulong()));
    
    		if (header_type == Signal::GROUP_HEADER || header_type == Signal::TDC_HEADER || header_type == Signal::TDC_TRAILER) {
    			currEventID = EventID(word);

    			// analyze data if we reached a header for a new event 
      			if (currEventID.ID() != prevEventID.ID()) {
      				//cout << "got back here" << endl;
      				prevEventID = currEventID;
      				if (total_events %100 == 0) cout << "Processing Event " << total_events << endl;

      				total_events++;
              		event = Event(trigVec, sigVec, currEventID);
      				DoHitFinding(&event,    tc);
			      	DoHitClustering(&event, geo);
			      	pass_event_check = kTRUE;
			      	//pass_event_check = CheckEvent(event, geo);
			      	event.SetPassCheck(pass_event_check);
			      	event.CheckClusterTime();
			      	//cout << "1" << endl;

			      	if (pass_event_check) {
			      		//cout << "2" << endl;
			      		//printf(".");
			      		eTree->Fill();
      	  				for (Cluster c : event.Clusters()) {
      	  					for (Hit h : c.Hits()) {

      	  						p_tdc_tdc_time_corrected[h.TDC()]->Fill(h.CorrTime());
      	      					p_tdc_adc_time          [h.TDC()]->Fill(h.ADCTime()); 

				      	        //p_tdc_time_corrected[h.TDC()][h.Channel()]->Fill(h.CorrTime());
				      	        //p_tdc_time          [h.TDC()][h.Channel()]->Fill(h.DriftTime());
				      	        //p_adc_time          [h.TDC()][h.Channel()]->Fill(h.ADCTime());

				      	        geo.GetHitLayerColumn(h.TDC(), h.Channel(), &hitL, &hitC);
				      	        hitByLC->Fill(hitC, hitL);
				      	        if (h.CorrTime() <-200 || h.CorrTime()>200)
				      	        	badHitByLC->Fill(hitC, hitL);
				      	        else
				      	        	goodHitByLC->Fill(hitC, hitL);
				      	        //p_hits_distribution[hitL]->Fill(hitC);
				      	    }
				      	}
				    }

				    //cout << "4" << endl;

				    for (Int_t i = 0; i != Geometry::MAX_TDC; i++) {
				    	for (Int_t j = 0; j != Geometry::MAX_TDC_CHANNEL; j++) {
				    		first_signal_flag[i][j] = kFALSE;
				    	}
				    }

				    if (pass_event_check) {
				    	total_events_pass++;
				    }
			      	else {
			      		total_events_fail++;
			      	}

			      	sigVec.clear();
      				trigVec.clear();
			    }
			    //cout << "5" << endl;
			}
			else if (header_type == Signal::RISING || header_type == Signal::FALLING) {
				//cout << "6" << endl;
				sig = Signal(word, currEventID);

				if (!first_signal_flag[sig.TDC()][sig.Channel()]) {
					sig.SetIsFirstSignal(kTRUE);
					first_signal_flag[sig.TDC()][sig.Channel()] = kTRUE;
				}

				if (sig.TDC() == geo.TRIGGER_MEZZ && sig.Channel() == geo.TRIGGER_CH) {
					trigVec.push_back(sig);
					if (header_type == Signal::RISING) total_triggers++;
				}
				else if (sig.TDC() != geo.TRIGGER_MEZZ) {
					sigVec.push_back(sig);
					if (header_type == Signal::RISING) total_signals++;
				}
			}
			//cout << "7" << endl;
		}
		if (data_in_flow.fail()) {
			//printf("Clearing file stream.\n");
			data_in_flow.clear();
		}

		//cout << "8" << endl;
		for (int i = 1; i != pad_num+1; i++) {
			adc_canvas->cd(i);
			gPad->Modified();
			tdc_canvas->cd(i);
			gPad->Modified();
		}

		// Update plots
		adc_canvas->cd();
		adc_canvas->Modified();
	 	adc_canvas->Update();
	 	tdc_canvas->cd();
		tdc_canvas->Modified();
	 	tdc_canvas->Update();

	 	if (gSystem->ProcessEvents())
            break;

        //printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

	 }
	 //f = fwrite(buffer, sizeof(unsigned int), sizeof(buffer), incomingDataFile);
	 //if (f < 0) error("ERROR writing to file");
	 //The last sock_read returns -1 since the client closed socket so final write doesn't happen

	 p_output_rootfile->cd();
	 for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
	 	if (geo.IsActiveTDC(tdc_id)) {
	 		if (tdc_id == 5) continue;
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

	 printf("\n ==== Program done ==== \n");

	 return 0; 
}

//#ifdef __cplusplus
//}
//#endif