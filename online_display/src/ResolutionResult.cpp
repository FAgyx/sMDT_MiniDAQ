#include "src/Geometry.cpp"
#include "src/ChiSquare.cpp"
#include "src/IOUtility.cpp"
#include "src/Observable.cpp"
#include "src/TrackParam.cpp"
#include "src/T0Fit.h"

#ifndef MUON_RESOLUTION_RESULT
#define MUON_RESOLUTION_RESULT

namespace Muon {

  class ResolutionResult {
  public:
    ResolutionResult(Geometry & geometry);
    ~ResolutionResult();

    TH1D*     residuals;
    TH1D*     chiSq;
    TH1D*     radius;
    TH1D*     hitsPerEvent;
    TH2D*     resVsHitRadius;
    TH2D*     resVsHitTime;
    TProfile* meanResVsHitTime;
    TH1D*     resolutionRadius;
    TH1D*     tubeResolutions;
    TGraph*   tubeResVsMeanADC;

    TCanvas*  c_res;

    double    resolution;
    double    chiSqCut   = 1000;

    double    nTubesHit = 0;
    double    nTubesTrackThrough = 0;
    
    void      Draw         ();
    void      Clear        ();
    void      FillResiduals(TrackParam & tp);
    void      FillChiSq    (TrackParam & tp);
    void      DrawADCPlots (TString t0path);

    TF1*      FitResVsTime (int npar);
    TH1D*     chiSqProb;

    void      SaveImages   (TString outdir);

    Bool_t    PassCuts     (Hit h, const Geometry & geo);
    
    void      Write        ();
    void      Load         (TString fname);

    void   GetWidthAndError(double* wid, double* err, TH1* hist);
    void   GetMean         (double* mean, TH1* hist);


  private:
    static TString   ROOT_OUTPUT_NAME;
    static TString   RESIDUALS;
    static TString   CHISQ;
    static TString   RADIUS;
    static TString   HITSPEREVENT;
    static TString   RESVSHITRADIUS;
    static TString   RESVSHITTIME;
    static TString   MEANRESVSHITTIME;
    static TString   RESOLUTIONRADIUS;

    static const int MAXHITS = 12;

    TH1D* tubeLevelRes[Geometry::MAX_TDC*Geometry::MAX_TDC_CHANNEL];
    

    Geometry* geo;

    TF1 *doubGaus;
    TF1* chiSquareDist;


    TPaveText* DoubGausLegend     (TH1* h);
    void       PopulateResVsRadius();
  };

  TString ResolutionResult::ROOT_OUTPUT_NAME = "ResolutionResult";
  TString ResolutionResult::RESIDUALS        = "RR_residuals";
  TString ResolutionResult::CHISQ            = "RR_chiSq";
  TString ResolutionResult::RADIUS           = "RR_radius";
  TString ResolutionResult::HITSPEREVENT     = "RR_hitsPerEvent";
  TString ResolutionResult::RESVSHITRADIUS   = "RR_resVsHitRadius";
  TString ResolutionResult::RESVSHITTIME     = "RR_resVsHitTime";
  TString ResolutionResult::MEANRESVSHITTIME = "RR_meanResVsHitTime";
  TString ResolutionResult::RESOLUTIONRADIUS = "RR_resolutionRadius"; 


  ResolutionResult::ResolutionResult(Geometry & geometry) {
    
    geo = &geometry;
    
    residuals = new TH1D("residuals",  "Residuals",   100, -1000,1000);
    residuals->GetXaxis()->SetTitle("Residual (um)");
    residuals->GetYaxis()->SetTitle("Number of hits/40um");

    chiSq     = new TH1D("chiSq",      "Chi Squared", 30, 0,    30);
    chiSq->GetXaxis()->SetTitle("Chi Squared");
    chiSq->GetYaxis()->SetTitle("Number of hits");
    
    chiSqProb = new TH1D("chiSqProb", "", 100, 0, 100);

    radius    = new TH1D("radius", "radius", 100, 0, Geometry::radius);
    radius->GetXaxis()->SetTitle("radius (mm)");
    radius->GetYaxis()->SetTitle("Number of hits");

    hitsPerEvent = new TH1D("hitsPerEvent", "Number of Hits used per track", 12, 0, 12);
    hitsPerEvent->GetXaxis()->SetTitle("Number of Hits");
    hitsPerEvent->GetYaxis()->SetTitle("Number of Tracks");

    resVsHitRadius = new TH2D("resVsHitRadius", "Residuals vs. Hit Radius", 100, 0, Geometry::radius, 100, -1000,1000);
    resVsHitRadius->GetXaxis()->SetTitle("Hit Radius (mm)");
    resVsHitRadius->GetYaxis()->SetTitle("Residual (um)");
    
    resVsHitTime   = new TH2D("resVsHitTime", "Residuals vs. Hit Time", 100, -1, 1, 100, -500,500);
    resVsHitTime->GetXaxis()->SetTitle("Hit Normalized Drift Time");
    resVsHitTime->GetYaxis()->SetTitle("Residual (um)");
    
    meanResVsHitTime = new TProfile("meanResVsHitTime", "Mean Residual vs. Hit Time", 100, -1, 1);
    meanResVsHitTime->GetXaxis()->SetTitle("Hit Normalized Drift Time");
    meanResVsHitTime->GetYaxis()->SetTitle("Mean Residual (um)");

    resolutionRadius = new TH1D("resolutionRadius", "", 7, 0, 7);
    resolutionRadius->GetXaxis()->SetTitle("Drift Radius (mm)");
    resolutionRadius->GetYaxis()->SetTitle("Narrow width gaussian");


    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
	tubeLevelRes[tdc*Geometry::MAX_TDC_CHANNEL + chan] = new TH1D(TString::Format("tdc_%i_chan_%i_res", tdc, chan), "", 100, -1000, 1000);
      }
    }

    tubeResolutions = new TH1D("tubeResolutions", "", 200, 0, 200);
    tubeResolutions->GetXaxis()->SetTitle("Resolution (#mu m)");
    tubeResolutions->GetYaxis()->SetTitle("Number of Tubes");

    doubGaus = new TF1("doubGaus", "[0]*exp(-0.5*((x-[1])/[2])**2) + [3]*exp(-0.5*((x-[1])/[4])**2)", -500, 500);
    doubGaus->SetParameter(0, 10000);
    doubGaus->SetParameter(1, 0);
    doubGaus->SetParameter(2, 100);
    doubGaus->SetParameter(3, 2000);
    doubGaus->SetParameter(4, 250);

  }

  ResolutionResult::~ResolutionResult() {
    
    delete residuals;
    delete chiSq;
    delete radius;
    delete resVsHitRadius;
    delete resVsHitTime;
    delete meanResVsHitTime;
    delete hitsPerEvent;
    delete resolutionRadius;
  }

  void ResolutionResult::Clear() {
    residuals->Reset();
    chiSq->Reset();
    radius->Reset();
    resVsHitRadius->Reset();
    resVsHitTime->Reset();
    meanResVsHitTime->Reset();
    hitsPerEvent->Reset();
    resolutionRadius->Reset();
  }

  void ResolutionResult::Draw() {


    chiSquareDist = ChiSquareDist(hitsPerEvent);
    PopulateResVsRadius();

    c_res = new TCanvas("c_res", "Resolution Result", 1000,700);
    c_res->Divide(3, 3);
    c_res->cd(1);

    doubGaus->SetParameter(0, 10000);
    doubGaus->SetParameter(1, 0);
    doubGaus->SetParameter(2, 100);
    doubGaus->SetParameter(3, 2000);
    doubGaus->SetParameter(4, 250);
    residuals->Fit("doubGaus", "R");
    residuals->Draw();
    TPaveText* tpt = DoubGausLegend(residuals);
    tpt->Draw();
    c_res->cd(2);
    chiSq->Draw();
    chiSquareDist->SetLineColor(kRed);
    chiSquareDist->Draw("same");
    c_res->cd(3);
    radius->Draw();
    c_res->cd(4);
    resVsHitRadius->Draw("colz");
    c_res->cd(5);
    resVsHitTime->Draw("colz");
    c_res->cd(6);
    meanResVsHitTime->Draw();
    c_res->cd(7);
    hitsPerEvent->Draw();
    c_res->cd(8);
    resolutionRadius->Fit("pol2");
    resolutionRadius->Draw();
    c_res->cd(9);
    
    gPad->Modified();
    gPad->Update();

    cout << endl;
    cout << "Resolution results:" << endl;
    //cout << " resolution: " << residuals->GetFunction("gaus") << endl;
    cout << " efficiency: " << nTubesHit/nTubesTrackThrough << endl;
    cout << endl;

    double tubeW, tubeE;
    double minW  = DBL_MAX;
    int bestTDC  = 0;
    int bestChan = 0;
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
	if (geo->IsActiveTDCChannel(tdc, chan) && tdc != geo->TRIGGER_MEZZ) {
	  doubGaus->SetParameter(0, 100);
	  doubGaus->SetParameter(1, 0);
	  doubGaus->SetParameter(2, 100);
	  doubGaus->SetParameter(3, 20);
	  doubGaus->SetParameter(4, 250);

	  tubeLevelRes[tdc*Geometry::MAX_TDC_CHANNEL + chan]->Fit("doubGaus", "R");
	  GetWidthAndError(&tubeW, &tubeE, tubeLevelRes[tdc*Geometry::MAX_TDC_CHANNEL + chan]);
	  tubeResolutions->Fill(tubeW);
	  if (tubeW < minW) {
	    minW = tubeW;
	    bestTDC = tdc;
	    bestChan = chan;
	  }
	}
      }
    }
    tubeResolutions->Draw();
    cout << "The minimum tube width is " << minW << endl;
    cout << "On tube  TDC=" << bestTDC << ", chan=" << bestChan << endl;
  }

  void ResolutionResult::DrawADCPlots(TString t0path) {
    TFile t0File(t0path);
    TVectorD *fitParams;

    
    double tubeW, tubeE, tubeM;
    vector<double> ADCPeak;
    vector<double> TubeRes;

    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
	if (geo->IsActiveTDCChannel(tdc, chan) && tdc != geo->TRIGGER_MEZZ) {
	  fitParams = (TVectorD*)t0File.Get(TString::Format("FitData_tdc_%i_channel_%i", tdc, chan));
	  ADCPeak.push_back((*fitParams)[T0Fit::ADC_WIDTH_INDX]);
	  GetWidthAndError(&tubeW, &tubeE, tubeLevelRes[tdc*Geometry::MAX_TDC_CHANNEL + chan]);
	  GetMean(&tubeM, tubeLevelRes[tdc*Geometry::MAX_TDC_CHANNEL + chan]);
	  TubeRes.push_back(tubeM);
	}
      }
    }

    // tgraph
    tubeResVsMeanADC = new TGraph(ADCPeak.size(), &(ADCPeak[0]), &(TubeRes[0]));
    TCanvas* ADCCanv = new TCanvas("ADCCanv", "Plots relating to ADC distributions");
    ADCCanv->cd();
    tubeResVsMeanADC->Draw();
  }


  void ResolutionResult::FillChiSq(TrackParam & tp) {
    // fill Chi Square and DOF distibutions
    chiSq->Fill(tp.chiSq);
    hitsPerEvent->Fill(tp.DOF + tp.size());
    
  }

  void ResolutionResult::FillResiduals(TrackParam & tp) {
    
    // fill residual distributions
    tp.hitIndex = -1;
    double res, dist, err, rad, ntime;
    for (Cluster c : tp.e->Clusters()) {
      for (Hit h : c.Hits()) {
	tp.hitIndex++;
	rad   = tp.rtfunction->Eval(h);
	res   = tp.Residual(h);
        dist  = tp.Distance(h);
        err   = Hit::RadiusError(dist);
	ntime = tp.rtfunction->NormalizedTime(h.CorrTime(), h.TDC(), h.Channel());
	if (dist>0 && dist < Geometry::radius && TMath::Abs(res) < tp.maxResidual*err) {
	  if ((tp.anySkip() && tp.skip()) || !tp.anySkip()) {
	    residuals->Fill(1000*res);
	    radius->Fill(rad);
	    resVsHitRadius->Fill(rad,res*1000.0);
	    resVsHitTime  ->Fill(ntime,res*1000.0);
	    meanResVsHitTime->Fill(ntime,res*1000.0);	      
	    tubeLevelRes[h.TDC()*Geometry::MAX_TDC_CHANNEL + h.Channel()]->Fill(res*1000.0);
	  }
	}
      }
    }

    // fill efficiency distribution
    
  }


  TF1* ResolutionResult::FitResVsTime(int npar) {
    TF1* deltaFit = (TF1*) gROOT->GetFunction(TString("chebyshev") + to_string(npar));
    meanResVsHitTime->Fit(deltaFit);
    return deltaFit;
  }


  Bool_t ResolutionResult::PassCuts(Hit h, const Geometry & geo) {
    return kTRUE;
    /*
    int hitL, hitC;
    geo.GetHitLayerColumn(h.TDC(), h.Channel(), &hitL, &hitC);
    if (geo.MultiLayer(hitL) == ML) 
      return kTRUE;
    else 
      return kFALSE;
    */
  }

  void ResolutionResult::SaveImages(TString outdir) {
    c_res->SaveAs(IOUtility::join(outdir, "ResolutionResult.png"));
    TCanvas* plot = new TCanvas("plot", "Draw single plots here");
    plot->cd();

    chiSquareDist = ChiSquareDist(hitsPerEvent);
    PopulateResVsRadius();

    TH1* py;
    double minWidth = DBL_MAX;
    double widthErr = 0;
    double upper;
    int binL, binU;

    double width1, width2, mean, height1, height2, area1, area2;
    TString index;
    for (double lower = 0; lower < 7; lower+=1) {
      upper = lower+1;
      binL  = (int)(1 + 13.33*lower);
      binU  = (int)(13.33*upper);


      py = resVsHitRadius->ProjectionY("_py", binL, binU);
      doubGaus->SetParameter(0, 800);
      doubGaus->SetParameter(1, 0);
      doubGaus->SetParameter(2, 100);
      doubGaus->SetParameter(3, 400);
      doubGaus->SetParameter(4, 250);
      py->Fit("doubGaus", "R");
      py->Draw();
      TPaveText* tpt = DoubGausLegend(py);
      tpt->Draw();
      index.Form("%.1f", lower);

      plot->SaveAs(IOUtility::join(outdir, ResolutionResult::RESIDUALS + index + ".png"));
    }



    residuals       ->Draw();
    TPaveText* tpt  = DoubGausLegend(residuals);
    tpt             ->Draw();
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::RESIDUALS + ".png"));
    chiSq           ->Draw();
    chiSquareDist   ->SetLineColor(kRed);
    chiSquareDist   ->Draw("same");
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::CHISQ + ".png"));
    radius          ->Draw();
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::RADIUS + ".png"));
    hitsPerEvent    ->Draw();
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::HITSPEREVENT + ".png"));
    resVsHitRadius  ->Draw();
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::RESVSHITTIME + ".png"));
    meanResVsHitTime->Draw();
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::RESVSHITRADIUS + ".png"));
    resolutionRadius->Draw();
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::RESOLUTIONRADIUS + ".png"));


    cout << "OVERFLOW: " << radius->GetBinContent(radius->GetNbinsX()+1) << endl;
  }

  void ResolutionResult::PopulateResVsRadius() {
    TH1* py;
    double minWidth = DBL_MAX;
    double widthErr = 0;
    double upper;
    int binL, binU;

    // fit results
    double width1, width2, w1_err, w2_err, amp1, amp2, amp1_err, amp2_err;
    double maxRad = TMath::Floor(Geometry::radius);
    for (double lower = 0; lower < maxRad; lower+=1) {
      upper = lower+1;
      binL  = (int)(1 + 13.33*lower);
      binU  = (int)(13.33*upper);
      

      py = resVsHitRadius->ProjectionY("_py", binL, binU);
      doubGaus->SetParameter(0, 800);
      doubGaus->SetParameter(1, 0);
      doubGaus->SetParameter(2, 100);
      doubGaus->SetParameter(3, 400);
      doubGaus->SetParameter(4, 250);
      py->Fit("doubGaus", "R");
      width1   = TMath::Abs(py->GetFunction("doubGaus")->GetParameter(2));
      width2   = TMath::Abs(py->GetFunction("doubGaus")->GetParameter(4));
      amp1     = TMath::Abs(py->GetFunction("doubGaus")->GetParameter(0));
      amp2     = TMath::Abs(py->GetFunction("doubGaus")->GetParameter(3));

      w1_err   = TMath::Abs(py->GetFunction("doubGaus")->GetParError(2));
      w2_err   = TMath::Abs(py->GetFunction("doubGaus")->GetParError(4));
      amp1_err = TMath::Abs(py->GetFunction("doubGaus")->GetParError(0));
      amp2_err = TMath::Abs(py->GetFunction("doubGaus")->GetParError(3));

      Observable sigma_n     = (width1 < width2) ? Observable(width1, w1_err) : Observable(width2, w2_err);
      Observable sigma_w     = (width2 > width1) ? Observable(width2, w2_err) : Observable(width1, w1_err);
      Observable amp_n       = (width1 < width2) ? Observable(amp1, amp1_err) : Observable(amp2, amp2_err);
      Observable amp_w       = (width2 > width1) ? Observable(amp2, amp2_err) : Observable(amp1, amp1_err);

      Observable sigma = (sigma_n*amp_n + sigma_w*amp_w)/(amp_n + amp_w);


      resolutionRadius->SetBinContent((int)(lower+1), sigma.val);
      resolutionRadius->SetBinError((int)(lower+1),   sigma.err);

    }

  }
  
  void ResolutionResult::GetWidthAndError(double* wid, double* err, TH1* hist) {
    TF1* dg = hist->GetFunction("doubGaus");
    
    double width1, width2, w1_err, w2_err, amp1, amp2, amp1_err, amp2_err;

    width1   = TMath::Abs(dg->GetParameter(2));
    width2   = TMath::Abs(dg->GetParameter(4));
    amp1     = TMath::Abs(dg->GetParameter(0));
    amp2     = TMath::Abs(dg->GetParameter(3));

    w1_err   = TMath::Abs(dg->GetParError(2));
    w2_err   = TMath::Abs(dg->GetParError(4));
    amp1_err = TMath::Abs(dg->GetParError(0));
    amp2_err = TMath::Abs(dg->GetParError(3));


    Observable sigma_n     = (width1 < width2) ? Observable(width1, w1_err) : Observable(width2, w2_err);
    Observable sigma_w     = (width2 > width1) ? Observable(width2, w2_err) : Observable(width1, w1_err);
    Observable amp_n       = (width1 < width2) ? Observable(amp1, amp1_err) : Observable(amp2, amp2_err);
    Observable amp_w       = (width2 > width1) ? Observable(amp2, amp2_err) : Observable(amp1, amp1_err);

    Observable sigma = (sigma_n*amp_n + sigma_w*amp_w)/(amp_n + amp_w);

    *wid = sigma.val;
    *err = sigma.err;
  }

  void ResolutionResult::GetMean(double* mean, TH1* hist) {
    TF1* dg = hist->GetFunction("doubGaus");
    *mean = dg->GetParameter(1);
  }

  TPaveText* ResolutionResult::DoubGausLegend(TH1* h) {
    double width1, width2, w1_err, w2_err, mean, mean_err, amp1, amp2, amp1_err, amp2_err;
    
    width1   = TMath::Abs(h->GetFunction("doubGaus")->GetParameter(2));
    width2   = TMath::Abs(h->GetFunction("doubGaus")->GetParameter(4));
    mean     = TMath::Abs(h->GetFunction("doubGaus")->GetParameter(1));
    amp1     = TMath::Abs(h->GetFunction("doubGaus")->GetParameter(0));
    amp2     = TMath::Abs(h->GetFunction("doubGaus")->GetParameter(3));

    w1_err   = TMath::Abs(h->GetFunction("doubGaus")->GetParError(2));
    w2_err   = TMath::Abs(h->GetFunction("doubGaus")->GetParError(4));
    mean_err = TMath::Abs(h->GetFunction("doubGaus")->GetParError(1));
    amp1_err = TMath::Abs(h->GetFunction("doubGaus")->GetParError(0));
    amp2_err = TMath::Abs(h->GetFunction("doubGaus")->GetParError(3));
    
    Observable sigma_n     = (width1 < width2) ? Observable(width1, w1_err) : Observable(width2, w2_err);
    Observable sigma_w     = (width2 > width1) ? Observable(width2, w2_err) : Observable(width1, w1_err);
    Observable amp_n       = (width1 < width2) ? Observable(amp1, amp1_err) : Observable(amp2, amp2_err);
    Observable amp_w       = (width2 > width1) ? Observable(amp2, amp2_err) : Observable(amp1, amp1_err);
    
    Observable sigma = (sigma_n*amp_n + sigma_w*amp_w)/(amp_n + amp_w);
    


    TPaveText* tpt = new TPaveText(0.1, 0.4, 0.3, 0.9, "NDC");
    tpt->AddText(TString::Format("A_{n}=%.2f#pm%.2f",      amp_n  .val, amp_n  .err));
    tpt->AddText(TString::Format("#sigma_{n}=%.2f#pm%.2f", sigma_n.val, sigma_n.err));
    tpt->AddText(TString::Format("A_{w}=%.2f#pm%.2f",      amp_w  .val, amp_w  .err));
    tpt->AddText(TString::Format("#sigma_{w}=%.2f#pm%.2f", sigma_w.val, sigma_w.err));
    tpt->AddText(TString::Format("#mu = %.2f#pm%.2f",      mean,    mean_err));
    //tpt->AddText(TString::Format("A_{1}/A_{2} = %.2f", area1/area2));
    tpt->AddText(TString::Format("#sigma = %.2f#pm%.2f",   sigma.val, sigma.err ));
    return tpt;
    
  }

  void ResolutionResult::Write() {
    residuals       ->Write(ResolutionResult::RESIDUALS);
    chiSq           ->Write(ResolutionResult::CHISQ);
    radius          ->Write(ResolutionResult::RADIUS);
    hitsPerEvent    ->Write(ResolutionResult::HITSPEREVENT);
    resVsHitRadius  ->Write(ResolutionResult::RESVSHITTIME);
    resVsHitTime    ->Write(ResolutionResult::RESVSHITRADIUS);
    meanResVsHitTime->Write(ResolutionResult::MEANRESVSHITTIME);
    resolutionRadius->Write(ResolutionResult::RESOLUTIONRADIUS);

    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
	if (geo->IsActiveTDCChannel(tdc, chan)) 
	  tubeLevelRes[tdc*Geometry::MAX_TDC_CHANNEL+chan]->Write(TString::Format("tdc_%i_chan_%i_res", tdc, chan));
      }
    }
  }
  
  void ResolutionResult::Load(TString fname) {
    TFile* file = new TFile(fname);
    residuals        = (TH1D*)    file->Get(ResolutionResult::RESIDUALS);
    chiSq            = (TH1D*)    file->Get(ResolutionResult::CHISQ);
    radius           = (TH1D*)    file->Get(ResolutionResult::RADIUS);
    hitsPerEvent     = (TH1D*)    file->Get(ResolutionResult::HITSPEREVENT);
    resVsHitRadius   = (TH2D*)    file->Get(ResolutionResult::RESVSHITTIME);
    resVsHitTime     = (TH2D*)    file->Get(ResolutionResult::RESVSHITRADIUS);
    meanResVsHitTime = (TProfile*)file->Get(ResolutionResult::MEANRESVSHITTIME);
    resolutionRadius = (TH1D*)    file->Get(ResolutionResult::RESOLUTIONRADIUS);

    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
        if (geo->IsActiveTDCChannel(tdc, chan))
          tubeLevelRes[tdc*Geometry::MAX_TDC_CHANNEL+chan] = (TH1D*) file->Get(TString::Format("tdc_%i_chan_%i_res", tdc, chan));
      }
    }
  }
}

#endif
