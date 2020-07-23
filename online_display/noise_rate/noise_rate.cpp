/* time example */
#include <stdio.h>      /* printf */
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <unistd.h>
#include "TCanvas.h"
#include "TGraph.h"
#include <stdlib.h>

static double mt_t0_fermi(double *x , double *par) {
  const double &t(x[0]), &t_0(par[0]), &T(par[1]), &back(par[2]),&A(par[3]);
  return (back + A/(1+exp(-(t-t_0)/T)));
 //  return (par[2] + par[3]/(1+exp(-(x[0]-par[0])/par[1])));
}	

int noise_rate()
{
	// TPaveText *t0PT = new TPaveText(0.55,0.65,0.98,0.82,"NDC");
	// TPaveText *tmPT = new TPaveText(0.55,0.50,0.98,0.65,"NDC");
	// TPaveText *tdPT = new TPaveText(0.55,0.42,0.98,0.50,"NDC");
	TPaveText *t0PT = new TPaveText(0.68,0.73,0.90,0.82,"NDC");
	TPaveText *tmPT = new TPaveText(0.68,0.64,0.90,0.73,"NDC");
	TPaveText *tdPT = new TPaveText(0.68,0.59,0.90,0.64,"NDC");
	TCanvas *p_canvas;
	// TF1 *ft0 = new TF1("mt_t0_fermi",mt_t0_fermi,0,10,4);
	TGraph *p_graph;

	double x[5] = {1,2,3,4,5};
	double y[5]=  {1,2,3,4,5};
	p_graph = new TGraph(5,x,y);
	p_graph->Draw("AB");
	
	string p="name";
	char a[]="namea";

	t0PT->AddText(Form("T0 %.2lf #pm %.2lf ns",3.2,4.1));
  t0PT->AddText(Form("Slope %.2lf #pm %.2lf /ns",3.2,4.1));
  t0PT->SetTextColor(2);
  t0PT->Draw();

  tmPT->AddText(Form("Tmax %.1lf #pm %.1lf ns",  3.2,4.1));
  tmPT->AddText(Form("Slope %.1lf #pm %.1lf /ns",3.2,4.1));
  tmPT->SetTextColor(8);
  tmPT->Draw();

  tdPT->AddText(Form("DTmax %.1lf #pm %.1lf ns",3.2,4.1));
  tdPT->SetTextColor(1);
  tdPT->Draw();


	// time_t sys_time;
	// struct tm * timeinfo;
	// sys_time = time(0);
	// timeinfo = localtime(&sys_time);
	// char filename_time[256];
	// memset(filename_time, 0, sizeof(filename_time)); 
	// strftime(filename_time, 30, "%Y%m%d_%H%M%S", timeinfo);
	// system("mkdir output");
	// chdir("output");
	// char output_directoryname[256];
	// memset(output_directoryname, 0, sizeof(output_directoryname));
	// sprintf(output_directoryname,"mkdir %s",filename_time);
	// system(output_directoryname);
	// chdir(filename_time);

	// system("touch test");

  // time_t start_time, current_time;


  // time(&start_time);  /* get current time; same as: timer = time(NULL)  */
  // sleep(5);
  // time(&current_time);
  // double seconds;

  // seconds = difftime(current_time,start_time);

  // printf ("time diff = %.f \n", seconds);

  // return 0;
	// TCanvas *p_canvas;
	// TGraph *p_graph;
	// p_canvas = new TCanvas("c1", "test",0,0,1080,510);
	// double x[5] = {1,2,3,4,5};
	// double y[5];
	// int i=0;
	// string p="name";
	// char a[]="namea";

	// while(1){



		// cout<<i++<<endl;
		// for (int i = 0; i < 5; ++i)
		// 	y[i]=rand()%100;
		// p_graph = new TGraph(5,x,y);
		// p_graph->SetFillColor(40);

		
		
		// // cout<<p_graph->GetName();
		// cout<<p_graph->GetName()<<endl;
		// p_graph->SetTitle(p.c_str());
		// cout<<p_graph->GetName()<<endl;
		// p_graph->GetXaxis()->SetTitle("Channel");
		// p_graph->GetYaxis()->SetTitle("Rate(Hz)");
		// p_graph->Draw("AB");
		// TText *xlabel = new TText();
		// string text_content;
		// int x =10;
		// text_content ="text="+to_string(x);
		// xlabel-> SetNDC();
		// xlabel -> SetTextFont(42);
		// xlabel -> SetTextSize(0.03);
		// xlabel -> SetTextAngle(0);
		// xlabel -> DrawText(0.7, 0.9, text_content.c_str());
		// p_canvas->cd();
		// p_canvas->Modified();
		// p_canvas->Update();
		// sleep(2);
		// delete p_graph;
		// delete xlabel;


	// }






	return 0;




}