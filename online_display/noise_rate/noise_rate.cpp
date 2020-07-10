/* time example */
#include <stdio.h>      /* printf */
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <unistd.h>
#include "TCanvas.h"
#include "TGraph.h"
#include <stdlib.h>

int noise_rate()
{
  // time_t start_time, current_time;


  // time(&start_time);  /* get current time; same as: timer = time(NULL)  */
  // sleep(5);
  // time(&current_time);
  // double seconds;

  // seconds = difftime(current_time,start_time);

  // printf ("time diff = %.f \n", seconds);

  // return 0;
	TCanvas *p_canvas;
	TGraph *p_graph;
	p_canvas = new TCanvas("c1", "test",0,0,1080,510);
	double x[5] = {1,2,3,4,5};
	double y[5];
	int i=0;
	string p="name";
	char a[]="namea";

	while(1){


		cout<<i++<<endl;
		for (int i = 0; i < 5; ++i)
			y[i]=rand()%100;
		p_graph = new TGraph(5,x,y);
		p_graph->SetFillColor(40);

		
		
		// cout<<p_graph->GetName();
		cout<<p_graph->GetName()<<endl;
		p_graph->SetTitle(p.c_str());
		cout<<p_graph->GetName()<<endl;
		p_graph->GetXaxis()->SetTitle("Channel");
		p_graph->GetYaxis()->SetTitle("Rate(Hz)");
		p_graph->Draw("AB");
		TText *xlabel = new TText();
		string text_content;
		int x =10;
		text_content ="text="+to_string(x);
		xlabel-> SetNDC();
		xlabel -> SetTextFont(42);
		xlabel -> SetTextSize(0.03);
		xlabel -> SetTextAngle(0);
		xlabel -> DrawText(0.7, 0.9, text_content.c_str());
		p_canvas->cd();
		p_canvas->Modified();
		p_canvas->Update();
		sleep(2);
		delete p_graph;
		delete xlabel;


	}






	return 0;




}