// example.C

#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <RQ_OBJECT.h>



#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <fcntl.h>
  



#include "TNtuple.h"
#include "TGraph.h"
#include "TH1.h"

// #include "macros/draw_hist_udp.cpp"
// #include "macros/udp_server_init.c"
#include "macros/channel_packet.c"
#include "macros/GlobalIncludes.h"

class MyMainFrame {
   RQ_OBJECT("MyMainFrame")
private:
   TGMainFrame          *fMain;
   // TRootEmbeddedCanvas *fEcanvas;
   int                  my_sockfd;
   TGNumberEntry        *fNumberEntry_TDCID;
   TGNumberEntry        *fNumberEntry_CHNLID;
   TGCheckButton        *plot_ADC;
   TGCheckButton        *plot_TDC;
   TGCheckButton        *plot_Raw;
   TGCheckButton        *bulk_plot_chnl[MAX_CHNL_COUNT];
   TGCheckButton        *bulk_plot_tdc[MAX_TDC_COUNT];

   TCanvas              *p_Canvas[100];
   int                  canvas_count;


public:
   MyMainFrame();
   virtual ~MyMainFrame();
   void DoDraw();
   int udp_server_init(in_addr_t server_ip, int port_no);
   void draw_hist_udp(int arg_tdc_id, int arg_chnl_id);
   void draw_hist_p_chnl(struct Channel_packet *p_chnl);
   void SelectAllChnl();
   void ClearAllChnl();
   void SelectAllTDC();
   void ClearAllTDC();
   void Bulk_Plot();
   void Close_Plots();

};
// MyMainFrame::MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h) {
MyMainFrame::MyMainFrame(){
   canvas_count = 0;
   // Create a main frame
   // fMain = new TGMainFrame(p,w,h);
   fMain = new TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame);
   fMain->SetName("fMainFrame");
   fMain->SetLayoutBroken(kTRUE);


   fNumberEntry_TDCID = new TGNumberEntry(fMain, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
   fNumberEntry_TDCID->SetName("tdc_id_Number");
   fMain->AddFrame(fNumberEntry_TDCID, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fNumberEntry_TDCID->MoveResize(70,100,58,20);
   fNumberEntry_TDCID->SetLimits(TGNumberFormat::kNELLimitMinMax,0,MAX_TDC_COUNT-1); //low&up limit

   fNumberEntry_CHNLID = new TGNumberEntry(fMain, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
   fNumberEntry_CHNLID->SetName("chnl_id_Number");
   fMain->AddFrame(fNumberEntry_CHNLID, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fNumberEntry_CHNLID->MoveResize(150,100,58,20);
   fNumberEntry_CHNLID->SetLimits(TGNumberFormat::kNELLimitMinMax,0,MAX_CHNL_COUNT-1); //low&up limit



   TGLabel *tdc_id_label = new TGLabel(fMain,"tdc_id");
   tdc_id_label->SetTextJustify(36);
   tdc_id_label->SetMargins(0,0,0,0);
   tdc_id_label->SetWrapLength(-1);
   fMain->AddFrame(tdc_id_label, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   tdc_id_label->MoveResize(70,70,57,16);
   TGLabel *chnl_id_label = new TGLabel(fMain,"chnl_id");
   chnl_id_label->SetTextJustify(36);
   chnl_id_label->SetMargins(0,0,0,0);
   chnl_id_label->SetWrapLength(-1);
   fMain->AddFrame(chnl_id_label, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   chnl_id_label->MoveResize(150,70,57,16);
   plot_ADC = new TGCheckButton(fMain,"plot_ADC");
   plot_ADC->SetTextJustify(36);
   plot_ADC->SetMargins(0,0,0,0);
   plot_ADC->SetWrapLength(-1);
   fMain->AddFrame(plot_ADC, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   plot_ADC->MoveResize(272,24,109,17);

   plot_TDC = new TGCheckButton(fMain,"plot_TDC");
   plot_TDC->SetTextJustify(36);
   plot_TDC->SetMargins(0,0,0,0);
   plot_TDC->SetWrapLength(-1);
   fMain->AddFrame(plot_TDC, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   plot_TDC->MoveResize(272,48,109,17);

   plot_Raw = new TGCheckButton(fMain,"plot_Raw");
   plot_Raw->SetTextJustify(36);
   plot_Raw->SetMargins(0,0,0,0);
   plot_Raw->SetWrapLength(-1);
   fMain->AddFrame(plot_Raw, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   plot_Raw->MoveResize(272,80,109,17);


   TGTextButton *fTextButton_SelectAll = new TGTextButton(fMain,"SelectAll");
   fTextButton_SelectAll->SetTextJustify(36);
   fTextButton_SelectAll->SetMargins(0,0,0,0);
   fTextButton_SelectAll->SetWrapLength(-1);
   fTextButton_SelectAll->Resize(92,22);
   fMain->AddFrame(fTextButton_SelectAll, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextButton_SelectAll->MoveResize(400,20,92,22);
   fTextButton_SelectAll->Connect("Clicked()","MyMainFrame",this,"SelectAllChnl()");


   TGTextButton *fTextButton_ClearAll = new TGTextButton(fMain,"ClearAll");
   fTextButton_ClearAll->SetTextJustify(36);
   fTextButton_ClearAll->SetMargins(0,0,0,0);
   fTextButton_ClearAll->SetWrapLength(-1);
   fTextButton_ClearAll->Resize(92,22);
   fMain->AddFrame(fTextButton_ClearAll, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextButton_ClearAll->MoveResize(500,20,92,22);
   fTextButton_ClearAll->Connect("Clicked()","MyMainFrame",this,"ClearAllChnl()");

   for(int i=MAX_CHNL_COUNT-1;i>=0;i--){      
      TString chnl_name;
      chnl_name.Form("chnl[%d]", i);
      bulk_plot_chnl[i] = new TGCheckButton(fMain,chnl_name);
      bulk_plot_chnl[i]->SetTextJustify(36);
      bulk_plot_chnl[i]->SetMargins(0,0,0,0);
      bulk_plot_chnl[i]->SetWrapLength(-1);
      fMain->AddFrame(bulk_plot_chnl[i], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      bulk_plot_chnl[i]->MoveResize(400+(23-i)/4*75,50+(23-i)%4*24,75,20);
   }


   TGTextButton *fTextButton_SelectAll_tdc = new TGTextButton(fMain,"SelectAll");
   fTextButton_SelectAll_tdc->SetTextJustify(36);
   fTextButton_SelectAll_tdc->SetMargins(0,0,0,0);
   fTextButton_SelectAll_tdc->SetWrapLength(-1);
   fTextButton_SelectAll_tdc->Resize(92,22);
   fMain->AddFrame(fTextButton_SelectAll_tdc, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextButton_SelectAll_tdc->MoveResize(400,150,92,22);
   fTextButton_SelectAll_tdc->Connect("Clicked()","MyMainFrame",this,"SelectAllTDC()");


   TGTextButton *fTextButton_ClearAll_tdc = new TGTextButton(fMain,"ClearAll");
   fTextButton_ClearAll_tdc->SetTextJustify(36);
   fTextButton_ClearAll_tdc->SetMargins(0,0,0,0);
   fTextButton_ClearAll_tdc->SetWrapLength(-1);
   fTextButton_ClearAll_tdc->Resize(92,22);
   fMain->AddFrame(fTextButton_ClearAll_tdc, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextButton_ClearAll_tdc->MoveResize(500,150,92,22);
   fTextButton_ClearAll_tdc->Connect("Clicked()","MyMainFrame",this,"ClearAllTDC()");

   for(int i=0;i<MAX_TDC_COUNT;i++){      
      TString tdc_name;
      tdc_name.Form("tdc[%d]", i);
      bulk_plot_tdc[i] = new TGCheckButton(fMain,tdc_name);
      bulk_plot_tdc[i]->SetTextJustify(36);
      bulk_plot_tdc[i]->SetMargins(0,0,0,0);
      bulk_plot_tdc[i]->SetWrapLength(-1);
      fMain->AddFrame(bulk_plot_tdc[i], new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
      bulk_plot_tdc[i]->MoveResize(400+i%6*75,180+i/6*24,75,20);
   }

   TGTextButton *fTextButton_Bulk_Plot = new TGTextButton(fMain,"Bulk_Plot");
   fTextButton_Bulk_Plot->SetTextJustify(36);
   fTextButton_Bulk_Plot->SetMargins(0,0,0,0);
   fTextButton_Bulk_Plot->SetWrapLength(-1);
   fTextButton_Bulk_Plot->Resize(92,22);
   fMain->AddFrame(fTextButton_Bulk_Plot, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextButton_Bulk_Plot->MoveResize(500,250,92,22);
   fTextButton_Bulk_Plot->Connect("Clicked()","MyMainFrame",this,"Bulk_Plot()");



   TGTextButton *fTextButton_Draw = new TGTextButton(fMain,"Draw",-1,
   TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
   fTextButton_Draw->SetTextJustify(36);
   fTextButton_Draw->SetMargins(0,0,0,0);
   fTextButton_Draw->SetWrapLength(-1);
   fTextButton_Draw->Resize(92,22);
   fMain->AddFrame(fTextButton_Draw, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextButton_Draw->MoveResize(70,150,92,22);

   TGTextButton *fTextButton_Exit = new TGTextButton(fMain,"Exit","gApplication->Terminate(0)");
   fTextButton_Exit->SetTextJustify(36);
   fTextButton_Exit->SetMargins(0,0,0,0);
   fTextButton_Exit->SetWrapLength(-1);
   fTextButton_Exit->Resize(92,22);
   fMain->AddFrame(fTextButton_Exit, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextButton_Exit->MoveResize(70,200,92,22);


   TGTextButton *fTextButton_Close_Plots = new TGTextButton(fMain,"Close_Plots");
   fTextButton_Close_Plots->SetTextJustify(36);
   fTextButton_Close_Plots->SetMargins(0,0,0,0);
   fTextButton_Close_Plots->SetWrapLength(-1);
   fTextButton_Close_Plots->Resize(92,22);
   fMain->AddFrame(fTextButton_Close_Plots, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextButton_Close_Plots->MoveResize(170,150,92,22);
   fTextButton_Close_Plots->Connect("Clicked()","MyMainFrame",this,"Close_Plots()");

   

   // // Create canvas widget
   // fEcanvas = new TRootEmbeddedCanvas("Ecanvas",fMain,200,200);
   // fMain->AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX |
   //                 kLHintsExpandY, 10,10,10,1));
   // Create a horizontal frame widget with buttons
   // TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain,200,40);
   // TGTextButton *draw = new TGTextButton(hframe,"&Draw");
   fTextButton_Draw->Connect("Clicked()","MyMainFrame",this,"DoDraw()");


   // Set a name to the main frame
   fMain->SetWindowName("Online Monitor");

   // Map all subwindows of main frame
   fMain->MapSubwindows();

   // Initialize the layout algorithm
   fMain->Resize(1200,600);

   // Map main frame
   fMain->MapWindow();


   in_addr_t server_ip;
   // int bytes_recv;
   server_ip = INADDR_ANY;
   // server_ip = inet_addr("141.211.96.10");

   my_sockfd = udp_server_init(server_ip, UDP_PORT); 
}

void MyMainFrame::DoDraw() {
   draw_hist_udp(fNumberEntry_TDCID->GetIntNumber(),fNumberEntry_CHNLID->GetIntNumber());
}

void MyMainFrame::Bulk_Plot(){

   struct Channel_packet p_chnl;
   int bytes_recv;
   int tdc_id[MAX_TDC_COUNT];
   int chnl_id[MAX_CHNL_COUNT];
   int tdc_count=0;
   int chnl_count=0;
   memset(tdc_id, 0, sizeof(tdc_id));  
   memset(chnl_id, 0, sizeof(chnl_id)); 
   for(int i=0;i<MAX_TDC_COUNT;i++){
      if(bulk_plot_tdc[i]->IsOn()){
         tdc_id[i] = 1;
         tdc_count++;
      }
   }
   for(int i=0;i<MAX_CHNL_COUNT;i++){
      if(bulk_plot_chnl[i]->IsOn()){
         chnl_id[i] = 1;
         chnl_count++;
      }
   }
   int plot_count = 0;
   plot_count= tdc_count * chnl_count;
   // printf("plot_count =%d\n",plot_count);
   int loop_i = 1000;

    
   while(plot_count&&loop_i){
      bytes_recv = recvfrom(my_sockfd, (char *) &p_chnl, sizeof(p_chnl), 
               MSG_WAITALL, 0, 0);
      loop_i--;
      if(tdc_id[p_chnl.tdc_id] && chnl_id[p_chnl.tdc_chnl_id]){
         draw_hist_p_chnl(&p_chnl);
         plot_count--;
      }
   }   
}

void MyMainFrame::SelectAllChnl(){
   for(int i=0;i<MAX_CHNL_COUNT;i++){
      bulk_plot_chnl[i]->SetState(kButtonDown);
   }
}
void MyMainFrame::ClearAllChnl(){
   for(int i=0;i<MAX_CHNL_COUNT;i++){
      bulk_plot_chnl[i]->SetState(kButtonUp);
   }
}

void MyMainFrame::SelectAllTDC(){
   for(int i=0;i<MAX_TDC_COUNT;i++){
      bulk_plot_tdc[i]->SetState(kButtonDown);
   }
}
void MyMainFrame::ClearAllTDC(){
   for(int i=0;i<MAX_TDC_COUNT;i++){
      
      bulk_plot_tdc[i]->SetState(kButtonUp);
   }
}



MyMainFrame::~MyMainFrame() {
   // Clean up used widgets: frames, buttons, layout hints
   fMain->Cleanup();
   delete fMain;
}
void MyMainFrame::draw_hist_udp(int arg_tdc_id, int arg_chnl_id){

   struct Channel_packet p_chnl;
   int bytes_recv;
   while(1){
      bytes_recv = recvfrom(my_sockfd, (char *) &p_chnl, sizeof(p_chnl), 
               MSG_WAITALL, 0, 0);
      if(p_chnl.tdc_id==arg_tdc_id && p_chnl.tdc_chnl_id==arg_chnl_id)break;
   }
   draw_hist_p_chnl(&p_chnl);
}

int MyMainFrame::udp_server_init(in_addr_t server_ip, int port_no) { 
   int sockfd; 
   struct sockaddr_in servaddr; 

   // Creating socket file descriptor 
   if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
     perror("socket creation failed"); 
     exit(EXIT_FAILURE); 
   } 

   memset(&servaddr, 0, sizeof(servaddr));  

   // Filling server information 
   servaddr.sin_family    = AF_INET; // IPv4 
   servaddr.sin_addr.s_addr = server_ip; 
   servaddr.sin_port = htons(port_no); 

   // Bind the socket with the server address 
   if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
         sizeof(servaddr)) < 0 ) 
   { 
     perror("bind failed"); 
     exit(EXIT_FAILURE); 
   } 
   return sockfd; 
} 


void MyMainFrame::Close_Plots(){
   for(;canvas_count-- > 0;)delete p_Canvas[canvas_count];
         canvas_count = 0;
   return;
}
void MyMainFrame::draw_hist_p_chnl(struct Channel_packet *p_chnl){

   TH1F *p_th1_hist;
   TString h_name;
   TString tmp;
   if(plot_Raw->IsOn()){
      //draw ADC raw hist
      if(plot_ADC->IsOn()){
         h_name.Form("tdc_%d_chnl_%d_adc_raw_spectrum", p_chnl->tdc_id, p_chnl->tdc_chnl_id);
         tmp.Form("plot_%02d",canvas_count);
         if(canvas_count >= CANVAS_COUNT_MAX)Close_Plots();
         p_Canvas[canvas_count] = new TCanvas(tmp,tmp,(canvas_count/4)*400,(canvas_count%4)*300,400,300); 
         canvas_count++;

         p_th1_hist = new TH1F(h_name, h_name, ADC_HIST_TOTAL_BIN, ADC_HIST_LEFT, ADC_HIST_RIGHT);
         p_th1_hist->GetXaxis()->SetTitle("time/ns");
         p_th1_hist->GetYaxis()->SetTitle("entries");
         for(Int_t bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
            p_th1_hist->SetBinContent(bin_index+1,p_chnl->adc_hist_raw[bin_index]);
         }  
         p_th1_hist->SetEntries(p_chnl->adc_entries_raw);
         p_th1_hist->DrawCopy();
         delete p_th1_hist;
      }
   


      //draw TDC hist
      if(plot_TDC->IsOn()){
         h_name.Form("tdc_%d_chnl_%d_tdc_raw_spectrum", p_chnl->tdc_id, p_chnl->tdc_chnl_id);
         tmp.Form("plot_%02d",canvas_count);
         if(canvas_count >= CANVAS_COUNT_MAX)Close_Plots();
         p_Canvas[canvas_count] = new TCanvas(tmp,tmp,(canvas_count%4)*400,(canvas_count/4)*300,400,300); 
         canvas_count++;

         p_th1_hist = new TH1F(h_name, h_name, TDC_HIST_TOTAL_BIN, TDC_HIST_LEFT, TDC_HIST_RIGHT);
         p_th1_hist->GetXaxis()->SetTitle("time/ns");
         p_th1_hist->GetYaxis()->SetTitle("entries");
         for(Int_t bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
         p_th1_hist->SetBinContent(bin_index+1,p_chnl->tdc_hist_raw[bin_index]);
         } 
         p_th1_hist->SetEntries(p_chnl->tdc_entries_raw);
         p_th1_hist->DrawCopy();
         delete p_th1_hist;
      }

   }
   else{
         //draw ADC hist
      if(plot_ADC->IsOn()){
         h_name.Form("tdc_%d_chnl_%d_adc_spectrum", p_chnl->tdc_id, p_chnl->tdc_chnl_id);
         tmp.Form("plot_%02d",canvas_count);
         if(canvas_count >= CANVAS_COUNT_MAX)Close_Plots();
         p_Canvas[canvas_count] = new TCanvas(tmp,tmp,(canvas_count%4)*400,(canvas_count/4)*300,400,300); 
         canvas_count++;

         p_th1_hist = new TH1F(h_name, h_name, ADC_HIST_TOTAL_BIN, ADC_HIST_LEFT, ADC_HIST_RIGHT);
         p_th1_hist->GetXaxis()->SetTitle("time/ns");
         p_th1_hist->GetYaxis()->SetTitle("entries");
         for(Int_t bin_index=0;bin_index<ADC_HIST_TOTAL_BIN;bin_index++){
            p_th1_hist->SetBinContent(bin_index+1,p_chnl->adc_hist[bin_index]);
         }  
         p_th1_hist->SetEntries(p_chnl->adc_entries);
         p_th1_hist->DrawCopy();
         delete p_th1_hist;
      }
      


      //draw TDC hist
      if(plot_TDC->IsOn()){
         h_name.Form("tdc_%d_chnl_%d_tdc_spectrum", p_chnl->tdc_id, p_chnl->tdc_chnl_id);
         tmp.Form("plot_%02d",canvas_count);
         if(canvas_count >= CANVAS_COUNT_MAX)Close_Plots();
         p_Canvas[canvas_count] = new TCanvas(tmp,tmp,(canvas_count%4)*400,(canvas_count/4)*300,400,300); 
         canvas_count++;

         p_th1_hist = new TH1F(h_name, h_name, TDC_HIST_TOTAL_BIN, TDC_HIST_LEFT, TDC_HIST_RIGHT);
         p_th1_hist->GetXaxis()->SetTitle("time/ns");
         p_th1_hist->GetYaxis()->SetTitle("entries");
         for(Int_t bin_index=0;bin_index<TDC_HIST_TOTAL_BIN;bin_index++){
         p_th1_hist->SetBinContent(bin_index+1,p_chnl->tdc_hist[bin_index]);
         } 
         p_th1_hist->SetEntries(p_chnl->tdc_entries);
         p_th1_hist->DrawCopy();
         delete p_th1_hist;
      }
   }
   return;
}





void draw_GUI() {
   
   // in_addr_t server_ip;
   // int bytes_recv;
   // server_ip = INADDR_ANY;   
   // // server_ip = inet_addr("141.211.96.10");

   // sockfd = udp_server_init(server_ip, UDP_PORT); 


   // Popup the GUI...
   new MyMainFrame();
}