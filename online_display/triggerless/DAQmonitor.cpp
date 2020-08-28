

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

#include "macros/GlobalIncludes.h"

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
		// void DataDecode_triggerless();
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
		vector<Signal> trigVec, sigVec;
		bitset<4> header;
		Signal sig;
		Event  event, event_raw;
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
				if (tdc_id == geo.TRIGGER_MEZZ){
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

}

#endif
