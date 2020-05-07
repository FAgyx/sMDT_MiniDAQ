#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

// ROOT includes
#include "TFile.h"
#include "TDirectory.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1.h"

#include "macros/GlobalIncludes.h"
#include "macros/udp_server.c"

// Muon Reconstruction includes
#include "src/Geometry.cpp"

#include "macros/draw_hist_single_channel.c"

using namespace std;
using namespace Muon;

void draw_hist(int arg_tdc_id, int arg_chnl_id){

	ifstream inputfile;
	string inputfile_name = "udp_chnl_hist.csv";
	if(udp_server(8080, inputfile_name.c_str())==0){
		cout<<"Can not open file "<<inputfile_name<<endl;
		return;
	};
	inputfile.open(inputfile_name.c_str());
	int *p_adc_array[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];
	int *p_tdc_array[Geometry::MAX_TDC][Geometry::MAX_TDC_CHANNEL];

	Geometry geo = Geometry();
	geo.SetRunN(0);
	string temp;
	int tdc_id, tdc_chnl_id, is_tdc_spectrum, entries;
	if (!geo.IsActiveTDC(arg_tdc_id)||arg_tdc_id==5||arg_chnl_id>23||arg_chnl_id<0) {
		cout<<"Not an active TDC or invalid channel number!\n";
		return;
	}

	//allocate needed memory for adc_array and tdc_array
	for (tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
		if (geo.IsActiveTDC(tdc_id)) {
			if (tdc_id == 5) continue;
			for(tdc_chnl_id = 0; tdc_chnl_id != Geometry::MAX_TDC_CHANNEL; tdc_chnl_id++){
				p_adc_array[tdc_id][tdc_chnl_id] = new int[TOTAL_BIN_QUANTITY/2+3];
				p_tdc_array[tdc_id][tdc_chnl_id] = new int[TOTAL_BIN_QUANTITY+3];
			}
		}
	}


	// read csv file to memory
	for(string line;getline(inputfile, line);){
		stringstream line_stream(line);
		getline(line_stream, temp,',');
		tdc_id = stoi(temp);
		getline(line_stream, temp,',');
		tdc_chnl_id = stoi(temp);
		getline(line_stream, temp,',');
		is_tdc_spectrum = stoi(temp);
		getline(line_stream, temp,',');
		entries = stoi(temp);
		if(is_tdc_spectrum){			
			*(p_tdc_array[tdc_id][tdc_chnl_id]+0)=tdc_id;
			*(p_tdc_array[tdc_id][tdc_chnl_id]+1)=tdc_chnl_id;
			*(p_tdc_array[tdc_id][tdc_chnl_id]+2)=is_tdc_spectrum;
			*(p_tdc_array[tdc_id][tdc_chnl_id]+3)=entries;
			for(int array_index = 4;array_index<TOTAL_BIN_QUANTITY+4;array_index++){
				getline(line_stream, temp,',');
				*(p_tdc_array[tdc_id][tdc_chnl_id]+array_index)=stoi(temp);
			}
		}
		else{
			*(p_adc_array[tdc_id][tdc_chnl_id]+0)=tdc_id;
			*(p_adc_array[tdc_id][tdc_chnl_id]+1)=tdc_chnl_id;
			*(p_adc_array[tdc_id][tdc_chnl_id]+2)=is_tdc_spectrum;
			*(p_adc_array[tdc_id][tdc_chnl_id]+3)=entries;
			for(int array_index = 4;array_index<TOTAL_BIN_QUANTITY/2+4;array_index++){
				getline(line_stream, temp,',');
				*(p_adc_array[tdc_id][tdc_chnl_id]+array_index)=stoi(temp);
			}
		}
	}

	TCanvas *adc_canvas = new TCanvas("c1", "ADC Plots");
	draw_hist_single_channel(p_adc_array[arg_tdc_id][arg_chnl_id],TOTAL_BIN_QUANTITY);
	TCanvas *tdc_canvas = new TCanvas("c2", "TDC Plots");
	tdc_canvas->SetWindowPosition(710,0);
	draw_hist_single_channel(p_tdc_array[arg_tdc_id][arg_chnl_id],TOTAL_BIN_QUANTITY);
	return; 
		

}