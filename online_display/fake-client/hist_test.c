#include <stdio.h>
#include <stdlib.h>


// ROOT includes
#include "TFile.h"
#include "TDirectory.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1.h"


#define MAXTDCNUM 6
#define MAXCHNLNUM 24
using namespace std;

void hist_test(){
	ofstream outputfile;
	outputfile.open("hist.csv");
	TH1I *myhist[MAXTDCNUM*MAXCHNLNUM];
	char hist_name[30];
	for(int i=0;i<MAXTDCNUM*MAXCHNLNUM;i++){
		sprintf(hist_name,"tdc%dchnl%d",i/MAXCHNLNUM,i%MAXCHNLNUM);
		myhist[i] = new TH1I(hist_name, "hist",1024, -400, 400);
		for(int j=0;j<10000;j++)
			// myhist->Fill(rand()%400);
			myhist[i]->Fill(rand()%(800*177)/177.0-400);
	}
	

	// // myhist->DrawCopy();
	// int i = 135;
	// cout<<i/MAXCHNLNUM<<","<<i%MAXCHNLNUM<<",";
	// printf("First bin = %f, Last bin = %f\n",myhist[0]->GetBinContent(1),myhist[0]->GetBinContent(1024));
	for(int i=0;i<MAXTDCNUM*MAXCHNLNUM;i++){
		outputfile<<i/MAXCHNLNUM<<","<<i%MAXCHNLNUM<<",";
		for(int j=1;j<1024;j++){
			outputfile<<myhist[i]->GetBinContent(j)<<",";
		}
		outputfile<<myhist[i]->GetBinContent(1024)<<"\n";

	}
	// for(int i=0;i<MAXTDCNUM*MAXCHNLNUM;i++){
	// 	delete[] myhist[i];
	// }

	


}