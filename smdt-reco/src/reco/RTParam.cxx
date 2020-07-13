#include "MuonReco/RTParam.h"

namespace MuonReco {

  RTParam::RTParam(Geometry g) : Optimizer(), Parameterization(npar+1) {
    geo = &g;
    func = (TF1*) gROOT->GetFunction(TString("chebyshev") + std::to_string(npar));
    der  = (TF1*) gROOT->GetFunction(TString("chebyshev") + std::to_string(npar));
  }

  

  RTParam::~RTParam() {
    //delete func;
  }



  double RTParam::D(int index, Hit h) {
    
    if (ignoreTube[h.TDC()*Geometry::MAX_TDC_CHANNEL + h.Channel()]) return 0;
    
    double rval;
    for (int i = 0; i < size(); i++) {
      der->SetParameter(i, 0);
    }
    
    der->SetParameter(index, 1);


    double time = NormalizedTime(h.CorrTime(), h.TDC(), h.Channel());
    if      (time < -1.0) rval = 0;
    else if (time >  1.0) rval = 0;
    else                  rval = der->Eval(time);

    return rval;
  }
  
  double RTParam::Residual(Hit h) {
    return -dependencies.at(0)->Residual(h);
  }

  double RTParam::Eval(double time) {
    if      (time < -1) return-1;
    else if (time >  1) return Geometry::max_drift_dist;
    else                return func->Eval(time);
  }

  double RTParam::Eval(Hit h) {
    SyncTF1ToParam();
    double nTime = NormalizedTime(h.CorrTime(), h.TDC(), h.Channel());
    return Eval(nTime);
  }
  
  double RTParam::Distance(Hit h) {
    return Eval(h);
  }
  
  double RTParam::NormalizedTime(double time, int tdc_id, int ch_id) {
    int index = tdc_id*Geometry::MAX_TDC_CHANNEL + ch_id;
    return (time-t0[index])/(tF[index])*2 - 1;
  }

  void RTParam::operator +=(Parameterization delta) {
    std::vector<double> temp(this->size());
    for (int i = 0; i != this->size(); i++) {
      temp[i] = param[i] + delta[i];
    }
    this->param = temp;
    SyncTF1ToParam();
  }

  void RTParam::Initialize(TString t0path, TString decodedDataPath) {
    TString fitVecName, histName;

    TFile t0File(t0path);
    TFile driftFile(decodedDataPath);
    
    TVectorD *fitParams;
    TH1D     *driftTimes = new TH1D("driftTimes", "", 1024, -1,1);
    TH1D     *tempHist;

    for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
      for (int ch_id = 0; ch_id != Geometry::MAX_TDC_CHANNEL; ch_id++) {
	if (tdc_id == geo->TRIGGER_MEZZ || !geo->IsActiveTDCChannel(tdc_id, ch_id)) {continue;}
	fitVecName.Form("FitData_tdc_%d_channel_%d", tdc_id, ch_id);
        fitParams = (TVectorD*)t0File.Get(fitVecName);
	t0[tdc_id*Geometry::MAX_TDC_CHANNEL + ch_id] = (*fitParams)[T0Fit::T0_INDX]       ;// - 10;
	tF[tdc_id*Geometry::MAX_TDC_CHANNEL + ch_id] = (*fitParams)[T0Fit::MAX_DRIFT_INDX];// + 20;

	histName.Form("TDC_%02d_of_%02d_Time_Spectrum/tdc_%d_channel_%d_tdc_time_spectrum", 
		      tdc_id, Geometry::MAX_TDC, tdc_id, ch_id);
	tempHist = (TH1D*) driftFile.Get(histName);
	
	for (int b = 0; b <= tempHist->GetNbinsX(); b++) {
	  driftTimes->Fill(NormalizedTime(tempHist->GetBinCenter(b), tdc_id, ch_id), tempHist->GetBinContent(b));
	}
	//driftTimes->Draw();
      }
    }


    Initialize();
    cumul = (TH1D*)driftTimes->GetCumulative();
    double maxBC = cumul->GetBinContent(cumul->GetNbinsX());
    
    for (int b = 0; b <= cumul->GetNbinsX(); b++) {
      cumul->SetBinContent(b, cumul->GetBinContent(b)/maxBC*Geometry::max_drift_dist);
      cumul->SetBinError(b, 0.01);
    }
    
    cumul->SetStats(0);
    cumul->GetXaxis()->SetTitle("Drift time (ns)");
    cumul->GetYaxis()->SetTitle("Drift radius (mm)");
    cumul->SetTitle("RT function Initial guess");
    cumul->Fit(func);
    cumul->Draw();
    func->SetLineColor(kRed);
    func->Draw("same");
    gPad->Modified();
    gPad->Update();
    func->Print();

    SyncParamToTF1();

    Print();
  }

  
  void RTParam::Initialize() {
    
    for (int i = 0; i <= size(); i++) {
      func->SetParameter(i, 0);
    }
    func->SetParameter(0, 0.2);
    func->SetParameter(1, 1.0);
    func->SetParameter(2, -0.15);
    
    SyncParamToTF1();
  }

  void RTParam::SyncParamToTF1() {
    for (int i = 0; i < size(); i++) {
      param[i] = func->GetParameter(i);
    }
  }

  void RTParam::SyncTF1ToParam() {
    for(int i = 0; i <size(); i++) {
      func->SetParameter(i, param[i]);
    }
  }


  /*
  void RTParam::iterateRT(ResolutionResult* rr) {
    TF1* delta = rr->FitResVsTime(npar);
    for (int i = 0; i < size(); i++) {
      std::cout << "Initial: " << param[i] << std::endl;
      std::cout << "Delta:   " << delta->GetParameter(i) << std::endl;
      param[i] = param[i] + delta->GetParameter(i);
      std::cout << "Final:   " << param[i] << std::endl;
    }
    SyncTF1ToParam();
  }
  */
  TF1* RTParam::GetFunction() {
    return func;
  }

  TF1* RTParam::RTDifference(RTParam* other) {
    TF1* diff = (TF1*) gROOT->GetFunction(TString("chebyshev") + std::to_string(npar));
    for (int i = 0; i < size(); i++) {
      diff->SetParameter(i, ((*this)[i] - (*other)[i]) *1000);
    }
    return diff;
  }

  void RTParam::SetIgnoreTDC(int tdc) {
    for (int chan = 0; chan < Geometry::MAX_TDC_CHANNEL; chan++) {
      SetIgnoreTube(tdc, chan);
    }
  }

  void RTParam::SetIgnoreTube(int tdc, int chan) {
    ignoreTube[tdc*Geometry::MAX_TDC_CHANNEL + chan] = 1;
  }

  void RTParam::SetIgnoreAll() {
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
	ignoreTube[tdc*Geometry::MAX_TDC_CHANNEL + chan] = 1;
      }
    }
  }

  void RTParam::ClearIgnore() {
    ignoreTube.reset();
  }

  void RTParam::SetActiveTube(int tdc, int chan) {
    ignoreTube[tdc*Geometry::MAX_TDC_CHANNEL + chan] = 0;
  }
  
  void RTParam::GetFirstActive(int* tdcOut, int* chanOut) {
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
        if (!ignoreTube[tdc*Geometry::MAX_TDC_CHANNEL + chan]) {
	  *tdcOut  = tdc;
	  *chanOut = chan;
	  return;
	}	  
      }
    }
  }

  void RTParam::Write(TString tag/* = ""*/) {
    TVectorD data = TVectorD(npar + 1);
    for (Int_t i = 0; i <= npar; i++) {
      data[i] = param[i];
    }
    data.Write("RTParam" + tag);
  }

  void RTParam::Load(TFile* infile, TString tag/* = ""*/) {
    TVectorD* data;
    data = (TVectorD*)infile->Get("RTParam" + tag);
    for (Int_t i = 0; i <= npar; i++) {
      param[i] = (*data)[i];
    }
    SyncTF1ToParam();
  }
  
  void RTParam::LoadTxt(TString fname) {
    /*
      .txt file must be in format of 
      ARBITRARY HEADER LINE TO BE IGNORED
      <radius> <time> <error>
      <radius> <time> <error>   
      <radius> <time> <error>   
      ...
      <radius> <time> <error>   
     */
    double radius, time, error;
    std::vector<double> v_radius, v_time, v_error = std::vector<double>();
    std::ifstream infile(fname);
    std::string line;
    int lineNum = 0;
    while (getline(infile, line)) {
      std::istringstream iss(line);
      if (lineNum != 0) {
	if (!(iss >> radius >> time >> error)) { break; }
	v_radius.push_back(radius);
	v_time  .push_back(time);
	v_error .push_back(error);
      }
      lineNum++;
    }

    TH1D* hist = new TH1D("hist", "", lineNum-1, -1, 1);
    for (int b = 0; b < v_time.size(); b++) {
      v_time.at(b)   = ((v_time.at(b) / v_time.at(lineNum-2))-.5 )*2;
      v_radius.at(b) = v_radius.at(b);
      v_error.at(b)  = v_error .at(b);
      hist->SetBinContent(b+1, v_time.at(b), v_radius.at(b));
      hist->SetBinError  (b+1, v_error.at(b));
    }
    hist->Fit(func);
    SyncParamToTF1();
    
  }

  void RTParam::WriteOutputTxt(TString outdir) {
    for (Int_t tdc_id = 0; tdc_id != Geometry::MAX_TDC; tdc_id++) {
      for (Int_t ch_id = 0; ch_id != Geometry::MAX_TDC_CHANNEL; ch_id++) {
	if (!geo->IsActiveTDCChannel(tdc_id, ch_id)) {continue;}
	
	TString outfile = IOUtility::join(outdir, TString::Format("TDC_%i_CH_%i_RT.txt", tdc_id, ch_id));
	// write out via eval, 100 pts
	std::ofstream rtfile;
	rtfile.open(outfile);
	rtfile << "R t\n";
	double tmax = tF[tdc_id*Geometry::MAX_TDC_CHANNEL + ch_id];
	std::cout << "TMAX: " << tmax << std::endl;
	for (double time = 0; time <= tmax; time += tmax/99.0) {
	  std::cout << time << std::endl;
	  rtfile << TString::Format("%.3f %.3f\n", Eval((time/tmax-0.5)*2.0), time);
	}
      }
    }
  }

  void RTParam::Print() {
    std::cout << "RT Function Parameterization Object" << std::endl;
    std::cout << "Chebyshev polynomial coefficients: " << std::endl;
    for (int i = 0; i < size(); i++) {
      std::cout << i << ": " << param[i] << std::endl;
      std::cout << i << ": " << func->GetParameter(i) << std::endl;
    }
  }


  void RTParam::Draw() {
    func->Draw();
  }

  void RTParam::SaveImage(TString outdir) {
    TCanvas* canv = new TCanvas("canv", "RT Function");
    canv->cd();
    func->Print();
    TH1* hist = new TH1D("hist", "RT Function", 1000, -1, 1);

    hist->Add(func);
    hist->SetStats(0);
    hist->GetXaxis()->SetTitle("Normalized Time [T0, TMax]");
    hist->GetYaxis()->SetTitle("Drift Distance (mm)");
    hist->Draw();
    //func->Draw("lsame");
    canv->SaveAs(IOUtility::join(outdir, "rtfunction.png"));
  }

  void RTParam::PrintActive() {
    std::cout << "Active Tubes: " << std::endl;
    for (int tdc = 0; tdc != Geometry::MAX_TDC; tdc++) {
      for (int chan = 0; chan != Geometry::MAX_TDC_CHANNEL; chan++) {
	if (ignoreTube[tdc*Geometry::MAX_TDC_CHANNEL + chan]) {continue;}
	std::cout << "TDC: " << tdc << " Chan: " << chan << std::endl;
      }
    }
    std::cout << std::endl;
  }
}

