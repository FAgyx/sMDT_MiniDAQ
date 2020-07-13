#include "MuonReco/EventDisplay.h"

namespace MuonReco {

  EventDisplay::EventDisplay() {
    eCanv     = new TCanvas("eCanv", "Event Display", 1200, 800);
    hit_model = std::vector<TEllipse*>();
    boxes     = std::vector<TBox*>();
  }

  EventDisplay::~EventDisplay() {
    Clear();
    delete eCanv;
  }

  void EventDisplay::Clear() {
    for (auto it = hit_model.begin(); it != hit_model.end(); ++it) {
      delete (*it);
    }
    hit_model = std::vector<TEllipse*>();

    for (auto it = track_model.begin(); it != track_model.end(); ++it) {
      delete (*it);
    }
    track_model = std::vector<TLine*>();

    for (auto it = boxes.begin(); it != boxes.end(); ++it) {
      delete (*it);
    }
    boxes = std::vector<TBox*>();
  }
  
  void EventDisplay::Divide(int nX, int nY) {
    eCanv->Divide(nX, nY);
    eCanv->Update();
  }

  void EventDisplay::DrawTDCBorders() {
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int ch = 0; ch != Geometry::MAX_TDC_CHANNEL; ch++) {
	
      }
    }
  }

  void EventDisplay::SetRT(RTParam* rtp) {
    rtfunction = rtp;
  }

  void EventDisplay::DrawEvent(Event &e, Geometry &geo, TDirectory* outdir/*=NULL*/) {
    char canvas_name[256];
    char canvas_output_name[256];
    double hit_x, hit_y;
    
    // initialize the canvas and draw the background geometry
    
    sprintf(canvas_name, "event_id_%lu", e.ID());
    strcpy(canvas_output_name, canvas_name);
    strcat(canvas_output_name, ".png");
    
    eCanv->cd(1);
    
    geo.Draw(e.ID());

    std::bitset<Geometry::MAX_TDC*Geometry::MAX_TDC_CHANNEL> wireIsDrawn;
    
    wireIsDrawn.reset();

    
    // draw this event using the highest level avaliable reconstructed objects

    if (e.Tracks().size() != 0) {
      for (Track t : e.Tracks()) {
	track_model.push_back(new TLine(0, t.YInt(), 810,t.YInt()+810*t.Slope()));
      }
    }
    if (e.Clusters().size() != 0) {
      // draw using clusters 
      for (Cluster c : e.Clusters()) {
	for (Hit h : c.Hits()) {
	  hit_x = h.X();
	  hit_y = h.Y();
	  if (rtfunction != 0) {
	    hit_model.push_back(new TEllipse(hit_x, hit_y, rtfunction->Eval(h), rtfunction->Eval(h)));
	    hit_model.back()->SetFillColor(kBlue);
	  }
	  else {
	    hit_model.push_back(new TEllipse(hit_x, hit_y, Geometry::radius, Geometry::radius));
	    hit_model.back()->SetFillColor(kGreen);
	  }
	  wireIsDrawn[h.TDC()*Geometry::MAX_TDC_CHANNEL + h.Channel()] = 1;
	}
      }
    }
    if (e.WireSignals().size() != 0) {
      // draw using signals
      for (size_t i = 0; i < e.WireSignals().size(); i++) {
	if (!wireIsDrawn[e.WireSignals().at(i).TDC()*Geometry::MAX_TDC_CHANNEL + e.WireSignals().at(i).Channel()]) {
	  if (e.WireSignals().at(i).Type() == Signal::RISING) {
	    Geometry::GetHitXY(e.WireSignals().at(i).TDC(), e.WireSignals().at(i).Channel(), &hit_x, &hit_y);
	    hit_model.push_back(new TEllipse(hit_x, hit_y, Geometry::radius, Geometry::radius));
	    hit_model.back()->SetFillColor(kRed);
	    wireIsDrawn[e.WireSignals().at(i).TDC()*Geometry::MAX_TDC_CHANNEL + e.WireSignals().at(i).Channel()] = 1;
	  }
	}
      }
    }
    
    for (auto hit : hit_model) {
      hit->Draw();
    }
    for (auto track: track_model) {
      track->Draw();
    }
    
    eCanv->Update();
    if (outdir != NULL) {
      outdir->WriteTObject(eCanv);
      eCanv->SaveAs(canvas_output_name);
    }

    
  } // end method: Event Display :: Draw Event


  void EventDisplay::DrawTrackZoom(Event &e, Geometry &geo, int ML, TDirectory* outdir/*=NULL*/) {
    /*
    TString title = TString();
    title.Form("Track zoom on MultiLayer %d", ML);
    zoomCanv = new TCanvas("zoomCanv", title.Data(), 800, 800);
    */
    if (ML == 0) 
      eCanv->cd(4);
    else if (ML == 1) 
      eCanv->cd(2);

    double xmin, xmax, ymin, ymax, ymid;
    Track tr = e.Tracks().back();
    if (ML == 0) {
      ymin = 0;
    }
    else if (ML == 1) {
      ymin = Geometry::ML_distance;
    }

    ymax = ymin + 3*Geometry::layer_distance + 2*Geometry::radius;
    ymid = (ymax+ymin)/2;
    xmin = (ymid-tr.YInt())/tr.Slope()-45;
    xmax = (ymid-tr.YInt())/tr.Slope()+45;
    
    TString ml_label;
    ml_label.Form("MultiLayer %d", ML);
    geo.Draw(e.ID(), xmin, ymin, xmax, ymax, ml_label);

    for (auto hit : hit_model) {
      hit->Draw();
    }
    for (auto track: track_model) {
      track->Draw();
    }

    eCanv->Update();

    if (outdir != NULL) {
      char canvas_name[256];
      char canvas_output_name[256];
      sprintf(canvas_name, "event_id_%lu", e.ID());
      strcpy(canvas_output_name, canvas_name);
      strcat(canvas_output_name, ".png");
      outdir->WriteTObject(eCanv);

      eCanv->SaveAs(canvas_output_name);  
    }
  } // end method: Event Display:: Draw Track Zoom


  void EventDisplay::SaveCanvas(TString name, TDirectory* outdir) {    
    eCanv->SaveAs(name);
  }

  void EventDisplay::DrawTubeHist(Geometry &geo, TH2D* hist, TDirectory* outdir/*=NULL*/, Bool_t noROOT /*= kFALSE*/) {
    if (hist->GetNbinsX() != Geometry::MAX_TUBE_COLUMN || hist->GetNbinsY() != Geometry::MAX_TUBE_LAYER) {
      std::cout << "EventDisplay::DrawTubeHist"                         << std::endl;
      std::cout << "    Provided Histogram bin dimensions do not match" << std::endl;
      std::cout << "    The geometry.  Skipping execution."             << std::endl;
      return;
    } 
    
    
    char canvas_name[256];
    char canvas_output_name[256];
    double hit_x, hit_y;
    int hitL, hitC;

    sprintf(canvas_name, hist->GetName());
    strcpy(canvas_output_name, canvas_name);
    strcat(canvas_output_name, ".png");

    eCanv = new TCanvas(canvas_name,canvas_name, 1200, 480);
    TPad *pad1 = new TPad("pad1", "",0.0,0.0,0.95,1.0);
    TPad *pad2 = new TPad("pad2", "",0.87,0.0,0.97,1.0);

    pad1->cd();
    geo.Draw(hist->GetTitle());
    geo.DrawTDCLabel();

    // set color references
    gStyle->SetPalette(kBird);
    int nCol = gStyle->GetNumberOfColors();
    double diff = hist->GetMaximum();
    if (diff == 0)
      diff = hist->GetMaximum(0);
    double min  = hist->GetMinimum();
    if (min == 0)
      min = hist->GetMinimum(0);
    diff -= min;
    diff *= 1.01;
    int col;


    // iterate over active non trigger tdc and draw with color set by th2d
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int ch = 0; ch != Geometry::MAX_TDC_CHANNEL; ch++) {
	if (geo.IsActiveTDCChannel(tdc, ch) && tdc != geo.TRIGGER_MEZZ) {
	  geo.GetHitLayerColumn(tdc, ch, &hitL, &hitC);
	  Geometry::GetHitXY(tdc, ch, &hit_x, &hit_y);
	  hit_model.push_back(new TEllipse(hit_x, hit_y, Geometry::radius, Geometry::radius));
	  col = (int)((hist->GetBinContent(hitC+1, hitL+1)-min)/diff * nCol);
	  //col = palette->GetValueColor(hist->GetBinContent(hitC+1, hitL+1));
	  hit_model.back()->SetFillColor(gStyle->GetColorPalette(col) );

	}
      }
    }
    
    for (auto hit : hit_model) {
      hit->Draw();
    }    


    // add a colorbar
    double x[2] = {0,1};
    double y[2] = {min,min+diff};
    TGraph* colorbar = new TGraph(2,x,y);
    colorbar->SetTitle("");
    pad2->cd();
    pad2->SetRightMargin(0.5);
    colorbar->SetMarkerStyle(0);
    colorbar->GetXaxis()->SetLabelSize(0);
    colorbar->GetXaxis()->SetTickLength(0);
    colorbar->GetYaxis()->SetLabelSize(0.15);
    colorbar->GetYaxis()->SetLabelOffset(0);
    colorbar->Draw("AP Y+");


    for (double i = 0; i < 1; i+=0.1) {
      boxes.push_back(new TBox(0,i*diff+min,1,(i+.1)*diff+min));
      col = i*nCol;
      boxes.back()->SetFillColor(gStyle->GetColorPalette(col));
    }
    for (auto box : boxes) {
      box->Draw();
    }



    eCanv->cd();
    pad1->Draw();
    pad2->Draw();
    //eCanv->SetRightMargin(0);
    eCanv->Update();
    if (outdir != NULL) {
      outdir->WriteTObject(eCanv);
      eCanv->SaveAs(canvas_output_name);
    }
    else if (noROOT) {
      eCanv->SaveAs(canvas_output_name);
    }

  }
}
