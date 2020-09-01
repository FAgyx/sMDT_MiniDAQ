

#ifndef MUON_DAQMONITOR
#define MUON_DAQMONITOR

#include "triggerless/Geometry.cpp"
#include "triggerless/TimeCorrection.cpp"
#include "src/sockhelp.c"
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

#include "triggerless/GlobalIncludes.h"

#include "triggerless/Signal.cpp"
#include "triggerless/Hit.cpp"
#include "triggerless/Event.cpp"
#include "triggerless/Separator.cpp"
#include "triggerless/TriggerMatch.cpp"


namespace Muon {
	class DAQ_monitor{
	
	public:
		DAQ_monitor(short portno_input);
		// void error(const char *msg);
		void tcp_server_setup(in_addr_t server_ip_int);
		void DataDecode_triggerless();
		// void DataDecode_dualCSM();


	private:
		// TDirectory *event_track[2];
		// char track_group_name[128];
		TH1F *p_tdc_adc_time[Geometry::MAX_TDC];
		TH1F *p_tdc_tdc_time_corrected[Geometry::MAX_TDC];
		TH1F *p_tdc_chnl_adc_time[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
		TH1F *p_tdc_chnl_tdc_time_corrected[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
		TH1F *p_tdc_chnl_adc_time_raw[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
		TH1F *p_tdc_chnl_tdc_time_corrected_raw[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
		double p_tdc_hit_rate[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
		double p_tdc_hit_rate_x[Geometry::MAX_TDC_CHANNEL];

		TGraph *p_tdc_hit_rate_graph[Geometry::MAX_TDC];
		Geometry geo;
		TimeCorrection tc;
		TCanvas *adc_canvas, *tdc_canvas, *rate_canvas, *trigger_rate_canvas;
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
		// int current_track_group;
		// Bool_t first_signal_flag[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
		int temp_track_group;
		bool pass_event_check;
		unsigned int word;
		unsigned int header_type;
		vector<Signal> trigVec, lsigVec, tsigVec;
		bitset<4> header;
		Signal sig;
		Event  event, event_raw;
		Separator sep;
		TTree* eTree;
		stringstream *filar_1, *filar_2;
		time_t start_time, current_time;
		double DAQ_time;	
	};


	DAQ_monitor::DAQ_monitor(short portno_input){
		tcp_portno = portno_input;
		geo = Geometry();
		geo.SetRunN();
		tc = TimeCorrection();
		// ed = new EventDisplay();
		TString h_name;
		memset(p_tdc_hit_rate, 0, sizeof(p_tdc_hit_rate));
		for (int i = 0; i < Geometry::MAX_TDC_CHANNEL; i++){
			p_tdc_hit_rate_x[i] = i;
		}
		for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
			if (geo.IsActiveTDC(tdc_id)) {
				h_name.Form("tdc_%d_tdc_time_spectrum_corrected", tdc_id);
				p_tdc_tdc_time_corrected[tdc_id] = new TH1F(h_name, h_name,TDC_HIST_TOTAL_BIN, TDC_HIST_LEFT, TDC_HIST_RIGHT);
				p_tdc_tdc_time_corrected[tdc_id]->GetXaxis()->SetTitle("time/ns");
				p_tdc_tdc_time_corrected[tdc_id]->GetYaxis()->SetTitle("entries");

				h_name.Form("tdc_%d_adc_time_spectrum", tdc_id);
				p_tdc_adc_time[tdc_id] = new TH1F(h_name, h_name, ADC_HIST_TOTAL_BIN, ADC_HIST_LEFT, ADC_HIST_RIGHT);
				p_tdc_adc_time[tdc_id]->GetXaxis()->SetTitle("time/ns");
				p_tdc_adc_time[tdc_id]->GetYaxis()->SetTitle("entries");

				h_name.Form("tdc_%d_hit_rate", tdc_id);
				p_tdc_hit_rate_graph[tdc_id] = new TGraph(Geometry::MAX_TDC_CHANNEL, p_tdc_hit_rate_x, p_tdc_hit_rate[tdc_id]);
				p_tdc_hit_rate_graph[tdc_id]->SetFillColor(4);
				p_tdc_hit_rate_graph[tdc_id]->SetTitle(h_name);
				p_tdc_hit_rate_graph[tdc_id]->GetXaxis()->SetTitle("Channel No.");
				p_tdc_hit_rate_graph[tdc_id]->GetXaxis()->SetLimits(-0.5,23.5);
				p_tdc_hit_rate_graph[tdc_id]->GetHistogram()->SetMaximum(0.5);
				p_tdc_hit_rate_graph[tdc_id]->GetYaxis()->SetTitle("Rate(Hz)");

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
				
			}
		} // end for: all TDC

		adc_canvas = new TCanvas("c1", "ADC Plots",0,0,1440,750);
		adc_canvas->Divide(4,2);
		tdc_canvas = new TCanvas("c2", "TDC Plots",0,750,1440,750);
		tdc_canvas->Divide(4,2);
		rate_canvas = new TCanvas("c3", "Hit Rate Plots",1440,0,1800,750);
		rate_canvas->Divide(4,2);
		trigger_rate_canvas = new TCanvas("c4", "Trigger Board",1440,750,400,300);
		// trigger_rate_canvas->SetLogy();
		// tdc_canvas->SetWindowPosition(710,0);
		printf("Canvases created and divided.\n");
		for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
			if (geo.IsActiveTDC(tdc_id)) {
				if (tdc_id == TRIGGER_MEZZ){
					trigger_rate_canvas->cd();
					p_tdc_hit_rate_graph[tdc_id]->Draw("AB");
				}
				else{
					pad_num = geo.TDC_COL[tdc_id]+4*(1-geo.TDC_ML[tdc_id]);
					adc_canvas->cd(pad_num);
					p_tdc_adc_time[tdc_id]->Draw();
					tdc_canvas->cd(pad_num);
					p_tdc_tdc_time_corrected[tdc_id]->Draw();
					rate_canvas->cd(pad_num);
					// gPad->SetLogy();
					p_tdc_hit_rate_graph[tdc_id]->Draw("AB");
					//if (gSystem->ProcessEvents()) break;
				}
			}
		}
		pad_num = 8;
		adc_canvas->cd();
		adc_canvas->Modified();
		adc_canvas->Update();
		tdc_canvas->cd();
		tdc_canvas->Modified();
		tdc_canvas->Update();
		rate_canvas->cd();
		rate_canvas->Modified();
		rate_canvas->Update();
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
		// event = Event();
		// event_raw = Event();
		// eTree = new TTree("eTree", "eTree");
		// eTree->Branch("event", "Event", &event);

		filar_1 = new stringstream;
		filar_2 = new stringstream;
		cout << "Processing..." << endl;


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



	void DAQ_monitor::DataDecode_triggerless(){
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
			oFile.write( (const char *) buffer,bytes_recv);

			bytes_recv = sock_read(newsockfd, (char *) buffer, sizeof(buffer));
			// time(&current_time);
		 	// DAQ_time = difftime(current_time,start_time);
		 	// printf("DAQ time = %.f\n",DAQ_time);
			total_bytes_recv += bytes_recv;
			sockReadCount++;
			// printf("Received Packet %i\n",sockReadCount);


			//printf("current position in file =  %i\n", (int) data_in_flow.tellg());
			//clock_t tStart = clock();
			data_in_flow.tellg(); //Needed to ensure reading continues
			int nloop = 0;
			while (data_in_flow.read((char *) &word, sizeof(word))) {
		 	 	//if (nloop == 0) printf("Started reading file again.\n");
		 	 	nloop++;
		 	 	sig = Signal(word);
		 	 	if (sig.TDC() == SEPARATOR_MEZZ && sig.Channel() == SEPARATOR_CH){
		 	 		// printf("Got a separator word\n");
		 	 		sep = Separator(trigVec, lsigVec, tsigVec);
		 	 		trigVec.clear();
		 	 		lsigVec.clear();
		 	 		tsigVec.clear();
		 	 		TriggerMatch(&sep,61*128,-30*128,tc);
		 	 		for (Event e : sep.Events()){
		 	 			for (Hit h : e.WireHits()){
		 	 				p_tdc_chnl_adc_time_raw				[h.TDC()][h.Channel()]->Fill(h.ADCbin()*TIMERES); 
	      					p_tdc_chnl_tdc_time_corrected_raw	[h.TDC()][h.Channel()]->Fill(h.DriftBin()*TIMERES); 
	      					p_tdc_tdc_time_corrected 			[h.TDC()]->Fill(h.DriftBin()*TIMERES);
	      	      			p_tdc_adc_time          			[h.TDC()]->Fill(h.ADCbin()*TIMERES);
		 	 			}
		 	 		}
		 	 		for (Hit h : sep.TriggerHits()){
		 	 			p_tdc_chnl_adc_time_raw				[h.TDC()][h.Channel()]->Fill(h.ADCbin()*TIMERES); 
      					p_tdc_chnl_tdc_time_corrected_raw	[h.TDC()][h.Channel()]->Fill(h.DriftBin()*TIMERES);
		 	 		}

		 	 	}  //SEPARATOR signal
		 	 	else if (sig.TDC() == TRIGGER_MEZZ && sig.Channel() == TRIGGER_CH){
		 	 		// printf("Got a trigger word\n");
		 	 		trigVec.push_back(sig);
		 	 		if (sig.Type() == Signal::RISING) total_triggers++;
		 	 	}	//Trigger signal
		 	 	else{	//Hit signal
		 	 		// printf("Got a hit word\n");
					if (sig.Type() == Signal::RISING) {
						lsigVec.push_back(sig);
						total_signals++;
					}
					else {
						tsigVec.push_back(sig);
					}
		 	 	}	//Hit signal

	    
	    		
			}  //while (bytes_recv > 0)



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
				// for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
				// 	string text_content;
					
				// 	if (geo.IsActiveTDC(tdc_id)) {
				// 		if (tdc_id == TRIGGER_MEZZ) {
				// 			trigger_rate_canvas->cd();
				// 			text_content ="Entries = "+to_string((int)total_triggers);
				// 		}
				// 		else{
				// 			rate_canvas->cd(geo.TDC_COL[tdc_id]+4*(1-geo.TDC_ML[tdc_id]));
				// 			text_content ="Entries = "+to_string((int)p_tdc_adc_time[tdc_id]->GetEntries());
				// 		}
				// 		TString h_name;
				// 		h_name.Form("tdc_%d_hit_rate", tdc_id);
				// 		delete p_tdc_hit_rate_graph[tdc_id];
				// 		p_tdc_hit_rate_graph[tdc_id] = new TGraph(Geometry::MAX_TDC_CHANNEL, p_tdc_hit_rate_x, p_tdc_hit_rate[tdc_id]);
				// 		p_tdc_hit_rate_graph[tdc_id]->SetFillColor(4);
				// 		p_tdc_hit_rate_graph[tdc_id]->SetTitle(h_name);
				// 		p_tdc_hit_rate_graph[tdc_id]->GetXaxis()->SetTitle("Channel No.");
				// 		double tmp_yrange = p_tdc_hit_rate_graph[tdc_id]->GetHistogram()->GetMaximum();
				// 		p_tdc_hit_rate_graph[tdc_id]->GetHistogram()->SetMaximum(tmp_yrange>0.5?tmp_yrange:0.5);
					
				// 		p_tdc_hit_rate_graph[tdc_id]->GetXaxis()->SetLimits(-0.5,23.5);
				// 		p_tdc_hit_rate_graph[tdc_id]->GetYaxis()->SetTitle("Rate(kHz)");					
				// 		p_tdc_hit_rate_graph[tdc_id]->Draw("AB");
				// 		TText *xlabel = new TText();
				// 		xlabel -> SetNDC();
				// 		xlabel -> SetTextFont(42);
				// 		xlabel -> SetTextSize(0.05);
				// 		xlabel -> SetTextAngle(0);
				// 		xlabel -> DrawText(0.5, 0.9, text_content.c_str());
				// 		TLine *l = new TLine(-0.5,0.5,23.5,0.5);
				// 		l->Draw();
				// 		// for(int j=0;j<24;j++)
				// 		// 	cout<<","<<p_tdc_hit_rate[tdc_id][j];
				// 		// cout<<endl;					
				// 	}
				// }
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
			 	rate_canvas->cd();
			 	rate_canvas->Update();
			 	trigger_rate_canvas->cd();
			 	trigger_rate_canvas->Update();
			 	struct Channel_packet p_chnl;

			 // 	for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
				//  	if (geo.IsActiveTDC(tdc_id)) {
				//  		if (tdc_id == geo.TRIGGER_MEZZ) {
				//  			//write p_chnl information
				//  			p_chnl.tdc_id = geo.TRIGGER_MEZZ;
				// 	    	p_chnl.tdc_chnl_id = geo.TRIGGER_CH;

				// 			//write ADC histogram
				// 	    	p_chnl.adc_entries_raw = 0;
				// 	    	for(int bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
				// 				p_chnl.adc_hist[bin_index] = 0;
				// 			}
				// 			//write TDC histogram
				// 	    	p_chnl.adc_entries_raw = 0;
				// 	    	for(int bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
				// 				p_chnl.tdc_hist[bin_index] = 0;
				// 			}
				// 			//write ADC raw histogram
				// 	    	p_chnl.adc_entries_raw = p_tdc_chnl_adc_time_raw[geo.TRIGGER_MEZZ][geo.TRIGGER_CH]->GetEntries();
				// 	    	for(int bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
				// 				p_chnl.adc_hist_raw[bin_index] = p_tdc_chnl_adc_time_raw[geo.TRIGGER_MEZZ][geo.TRIGGER_CH]->GetBinContent(bin_index+1);
				// 			}
				// 			//write TDC raw histogram
				// 	    	p_chnl.tdc_entries_raw = p_tdc_chnl_adc_time_raw[geo.TRIGGER_MEZZ][geo.TRIGGER_CH]->GetEntries();
				// 	    	for(int bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
				// 				p_chnl.tdc_hist_raw[bin_index] = p_tdc_chnl_tdc_time_corrected_raw[geo.TRIGGER_MEZZ][geo.TRIGGER_CH]->GetBinContent(bin_index+1);
				// 			}
				// 			//sending UDP packet outs
				// 			sendto(udp_sock_fd, (char *)&p_chnl, sizeof(p_chnl), 
	   //      				MSG_CONFIRM, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
				//  		}


				// 	    else {
				// 	    	for(int tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
				// 		    	//write p_chnl information
				// 		    	p_chnl.tdc_id = tdc_id;
				// 		    	p_chnl.tdc_chnl_id = tdc_chnl_id;

				// 		    	//write ADC histogram
				// 		    	p_chnl.adc_entries = p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetEntries();
				// 		    	for(int bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
				// 					p_chnl.adc_hist[bin_index] = p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
				// 				}
								
				// 				//write TDC histogram
				// 				p_chnl.tdc_entries = p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetEntries();
				// 		    	for(int bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
				// 					p_chnl.tdc_hist[bin_index] = p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
				// 				}

				// 				//write ADC raw histogram
				// 		    	p_chnl.adc_entries_raw = p_tdc_chnl_adc_time_raw[tdc_id][tdc_chnl_id]->GetEntries();
				// 		    	for(int bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
				// 					p_chnl.adc_hist_raw[bin_index] = p_tdc_chnl_adc_time_raw[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
				// 				}
								
				// 				//write TDC raw histogram
				// 				p_chnl.tdc_entries_raw = p_tdc_chnl_tdc_time_corrected_raw[tdc_id][tdc_chnl_id]->GetEntries();
				// 		    	for(int bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
				// 					p_chnl.tdc_hist_raw[bin_index] = p_tdc_chnl_tdc_time_corrected_raw[tdc_id][tdc_chnl_id]->GetBinContent(bin_index+1);
				// 				}

				// 				//sending UDP packet out
				// 				// printf("TDC=%d, CHNL=%d, Entries=%d\n",p_chnl.tdc_id,p_chnl.tdc_chnl_id,p_chnl.adc_entries);
				// 				sendto(udp_sock_fd, (char *)&p_chnl, sizeof(p_chnl), 
		  //       				MSG_CONFIRM, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
		  //       			}										    	
				// 	    }
				// 	}
				// }

			 // 	ofstream outputfile;
			 // 	string outputfile_name = "chnl_hist.csv";
				// outputfile.open(outputfile_name.c_str());

				// for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
				//  	if (geo.IsActiveTDC(tdc_id)) {
				//  		if (tdc_id == geo.TRIGGER_MEZZ) continue;

				// 	    for(int tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
				// 	    	//write ADC histogram:tdc_id, chnl_id, 0, hist_content
				// 	    	outputfile<<tdc_id<<","<<tdc_chnl_id<<",0,"\
				// 	    	<<p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetEntries()<<",";
				// 	    	for(int bin_index=1;bin_index<TOTAL_BIN_QUANTITY/2;bin_index++){
				// 				outputfile<<p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetBinContent(bin_index)<<",";
				// 			}
				// 			outputfile<<p_tdc_chnl_adc_time[tdc_id][tdc_chnl_id]->GetBinContent(TOTAL_BIN_QUANTITY/2)<<endl;

				// 			//write TDC histogram:tdc_id, chnl_id, 1, hist_content
				// 			outputfile<<tdc_id<<","<<tdc_chnl_id<<",1,"\
				// 			<<p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetEntries()<<",";
				// 	    	for(int bin_index=1;bin_index<TOTAL_BIN_QUANTITY;bin_index++){
				// 				outputfile<<p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetBinContent(bin_index)<<",";
				// 			}						
				// 			outputfile<<p_tdc_chnl_tdc_time_corrected[tdc_id][tdc_chnl_id]->GetBinContent(TOTAL_BIN_QUANTITY)<<endl;				    	
				// 	    }
				// 	}
				// } // end for: all TDC
				// outputfile.close();
				// udp_client(8080, outputfile_name.c_str());
				if(bytes_recv<=0)break;
		 	}

		 	if (gSystem->ProcessEvents())
	            break;

	        //printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

		}//while
		 //f = fwrite(buffer, sizeof(unsigned int), sizeof(buffer), incomingDataFile);
		 //if (f < 0) error("ERROR writing to file");
		 //The last sock_read returns -1 since the client closed socket so final write doesn't happen

		p_output_rootfile->cd();
		for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
		 	if (geo.IsActiveTDC(tdc_id)) {
		 		if (tdc_id == TRIGGER_MEZZ) continue;
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

		// system("mkdir output");
		// chdir("output");
		// char output_directoryname[256];
		// memset(output_directoryname, 0, sizeof(output_directoryname));
		// sprintf(output_directoryname,"mkdir %s",filename_time);
		// system(output_directoryname);
		// chdir(filename_time);

		// char rate_canvas_name[256];
		// memset(rate_canvas_name, 0, sizeof(rate_canvas_name));
		// sprintf(rate_canvas_name,"%s_rate.png",filename_time);
		// rate_canvas->Print(rate_canvas_name);

		// char adc_canvas_name[256];
		// memset(adc_canvas_name, 0, sizeof(adc_canvas_name));
		// sprintf(adc_canvas_name,"%s_adc.png",filename_time);
		// adc_canvas->Print(adc_canvas_name);

		// char tdc_canvas_name[256];
		// memset(tdc_canvas_name, 0, sizeof(tdc_canvas_name));
		// sprintf(tdc_canvas_name,"%s_tdc.png",filename_time);
		// tdc_canvas->Print(tdc_canvas_name);

		// char trigger_canvas_name[256];
		// memset(trigger_canvas_name, 0, sizeof(trigger_canvas_name));
		// sprintf(trigger_canvas_name,"%s_trigger_rate.png",filename_time);
		// trigger_rate_canvas->Print(trigger_canvas_name);

		// char fp_rate_File_name[256];
		// memset(fp_rate_File_name, 0, sizeof(fp_rate_File_name)); 
		// sprintf(fp_rate_File_name,"%s_rate.csv",filename_time);

		// fp_rate_File=fopen(fp_rate_File_name,"w");
		// fprintf(fp_rate_File,"tdc_id,");
		// for(int tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
		// 	fprintf(fp_rate_File,"%d,",tdc_chnl_id);
		// }
		// for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
		//  	if (geo.IsActiveTDC(tdc_id)) {
		//  		fprintf(fp_rate_File,"\n%d,",tdc_id);
		//  		for(int tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
		//  			fprintf(fp_rate_File,"%.4f,",p_tdc_hit_rate[tdc_id][tdc_chnl_id]);
		//  		}
		//  	}
		// }
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



}

#endif
