#include "MuonReco/RTParam.h"

namespace MuonReco {

  RTParam::RTParam() : Optimizer(), Parameterization(npar+1) {
    func = (TF1*) gROOT->GetFunction(TString("chebyshev") + std::to_string(npar));
    der  = (TF1*) gROOT->GetFunction(TString("chebyshev") + std::to_string(npar));
    t0   = TubeMap<double>(Geometry::MAX_TUBE_LAYER, Geometry::MAX_TUBE_COLUMN);
    tF   = TubeMap<double>(Geometry::MAX_TUBE_LAYER, Geometry::MAX_TUBE_COLUMN);
    ignoreTube = TubeMap<bool>(Geometry::MAX_TUBE_LAYER, Geometry::MAX_TUBE_COLUMN);
  }

  RTParam::RTParam(ConfigParser cp) : RTParam() {
    isMC = cp.items("General").getBool("IsMC", 0, 0);
    int minEvent = cp.items("AutoCalibration").getInt("MinEvent");
    int nEvents  = cp.items("AutoCalibration").getInt("NEvents");
    useFullCheby = cp.items("AutoCalibration").getInt("UseFullCheby", 0, 0);
  }

  RTParam::~RTParam() {
    //delete func;
  }



  double RTParam::D(int index, Hit h) {
    //if (isMC) return 0;
    if (!useFullCheby && index % 2 == 0) return 0;
    if (ignoreTube.get(h.Layer(), h.Column())) return 0;
    
    double rval;
    for (int i = 0; i < size(); i++) {
      der->SetParameter(i, 0);
    }
    
    der->SetParameter(index, 1);

    double driftTime = (useCorrection ? h.CorrTime() : h.DriftTime());
    double time = NormalizedTime(driftTime, h.Layer(), h.Column());
    if      (time < -1.0) rval = 0;
    else if (time >  1.0) rval = 0;
    else {
      
      /*
      rval  = (func->Eval(1)-func->Eval(-1))*(der->Eval(time)-der->Eval(-1)) -
	(der->Eval(1)-der->Eval(-1))*(func->Eval(time)-func->Eval(-1));
      std::cout << "numer: " << rval << std::endl;
      std::cout << "at -1: " << func->Eval(-1) << std::endl;
      std::cout << "at 1: " << func->Eval(1) << std::endl;
      std::cout << "time: " << time << std::endl;
      std::cout << "indx: " << index << std::endl;
      std::cout << "denom: " << Geometry::max_drift_dist/(func->Eval(1)-func->Eval(-1))/(func->Eval(1)-func->Eval(-1)) << std::endl;
      rval *= Geometry::max_drift_dist/(func->Eval(1)-func->Eval(-1))/(func->Eval(1)-func->Eval(-1));
      */
      rval = der->Eval(time);
    }
    /*
    double original = func->GetParameter(index);
    double fx = this->Eval(time);
    std::cout << "f(x):   " << fx << std::endl;
    func->SetParameter(index, original+1.0);
    double fxh = this->Eval(time);
    std::cout << "f(x+h): " << fxh << std::endl;
    rval = (fxh - fx)/0.1;
    func->SetParameter(index, original);
    */
    return rval;
  }
  
  double RTParam::Residual(Hit h) {
    return -dependencies.at(0)->Residual(h);
  }

  double RTParam::Eval(double time) {
    if      (time < -1)    return 0;
    else if (time >  1)    return Geometry::max_drift_dist;
    else                   return func->Eval(time);//(func->Eval(time) - func->Eval(-1))/(func->Eval(1)-func->Eval(-1))*Geometry::max_drift_dist;
  }

  double RTParam::Eval(Hit h, double deltaT0/*=0*/, double slewScaleFactor/*=1.0*/, double sigPropSF/*=1.0*/) {
    //if (isMC) return h.Radius();
    SyncTF1ToParam();
    double driftTime = (useCorrection
			? h.CorrTime()*slewScaleFactor + h.DriftTime()*(1.0-slewScaleFactor) + 
			(h.Y()/Geometry::getMeanYPosition() - 1)*Geometry::tube_length/2.0/0.231 * (sigPropSF - 1.0)
			: h.DriftTime()); // 0.231 m/ns is signal propagation speed
    double nTime = NormalizedTime(driftTime + deltaT0, h.Layer(), h.Column());
    return Eval(nTime);
  }
  
  double RTParam::Distance(Hit h) {
    //if (isMC) return h.Radius();
    return Eval(h);
  }
  
  double RTParam::NormalizedTime(double time, int layer, int column) {
    return (time-t0.get(layer, column))/(tF.get(layer, column))*2 - 1;
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
    _t0path = t0path;

    // clear the information in t0 and tF
    for (int layer = 0; layer != Geometry::MAX_TUBE_LAYER; layer++) {
      for (int column = 0; column != Geometry::MAX_TUBE_COLUMN; column++) {
	t0.set(layer, column, 0);
	if (isMC)
	  tF.set(layer, column, 188.688);
	else
	  tF.set(layer, column, 0);
      }
    }

    TString fitVecName, histName;

    TFile driftFile(decodedDataPath);
    
    TH1D     *driftTimes = new TH1D("driftTimes", "", 1024, -1,1);
    TH1D     *tempHist;
    
    if (!isMC) {
      T0Reader* t0Reader = T0Reader::GetInstance(t0path);
      TVectorD *fitParams = new TVectorD(NT0FITDATA);

      int tdc_id=0, ch_id=0, layer=0, column=0;
      t0Reader->SetBranchAddresses(&tdc_id, &ch_id, &layer, &column, fitParams);

      for (int iEntry = 0; iEntry < t0Reader->GetEntries(); iEntry++) {
	t0Reader->GetEntry(iEntry);

	if (ch_id < 0) continue;

	t0.set(layer, column, (*fitParams)[T0Fit::T0_INDX]);// - 10;
	tF.set(layer, column, (*fitParams)[T0Fit::MAX_DRIFT_INDX]);// + 20;
	
	histName.Form("TDC_%02d_of_%02d_Time_Spectrum/tdc_%d_channel_%d_tdc_time_spectrum_corrected", 
		      tdc_id, Geometry::MAX_TDC, tdc_id, ch_id);
	tempHist = (TH1D*) driftFile.Get(histName);
	
	for (int b = 0; b <= tempHist->GetNbinsX(); b++) {
	  driftTimes->Fill(NormalizedTime(tempHist->GetBinCenter(b), layer, column), tempHist->GetBinContent(b));
	}
      }// end for: entries in t0Reader
      delete fitParams;
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
    // make sure f(-1)=0
    func->SetParameter(0, func->GetParameter(0) - func->Eval(-1));
    cumul->Draw();
    func->SetLineColor(kRed);
    func->Draw("same");
    gPad->Modified();
    gPad->Update();
    func->Print();

    SyncParamToTF1();    

    Print();
    std::cout << "F(-1): " << func->Eval(-1) << std::endl;

    driftFile.Close();
  }

  
  void RTParam::Initialize() {
    
    for (int i = 0; i <= size(); i++) {
      func->SetParameter(i, 0);
    }
    func->SetParameter(0, 4.0);
    func->SetParameter(1, 3.6);
    func->SetParameter(2, -0.4);
    
    SyncParamToTF1();
  }

  void RTParam::constrain(TMatrixD* delta) {
    // there are two orthogonal constrains: sum coeffs cannot change, 
    // difference between sum of odd and even coeffs cannot change
    double coeff1 = 0;
    double coeff2 = 0;
    TMatrixD constraint1 = TMatrixD(delta->GetNrows(), delta->GetNcols());
    TMatrixD constraint2 = TMatrixD(delta->GetNrows(), delta->GetNcols());
    if (constrainEndpoint) {
      for (int r = 0; r < constraint1.GetNrows(); ++r) {
	for (int c = 0; c < constraint1.GetNcols(); ++c) {
	  constraint1[r][c] = 1;
	}
      }
      constraint1 *= 1/TMath::Sqrt(constraint1.E2Norm());
      coeff1 = TMatrixD(constraint1, TMatrixD::kTransposeMult, *delta)[0][0];
    }
    
    if (constrainZero) {
      for(int r = 0; r < constraint2.GetNrows();++r) {
	for (int c = 0; c< constraint2.GetNcols(); ++c) {
	  constraint2[r][c] = ((r+c) % 2 == 0) - ((r+c) % 2 == 1);
	} 
      }
      constraint2 *= 1/TMath::Sqrt(constraint2.E2Norm());
      coeff2 = TMatrixD(constraint2, TMatrixD::kTransposeMult, *delta)[0][0];    
    }
    
    if (constrainEndpoint) *delta -= coeff1*constraint1;
    if (constrainZero)     *delta -= coeff2*constraint2;
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
    T0Reader* t0Reader = T0Reader::GetInstance(_t0path);
    TVectorD *fitParams = new TVectorD(NT0FITDATA);
    int _tdc=0, _chan=0, _layer=0, _column=0;
    t0Reader->SetBranchAddresses(&_tdc, &_chan, &_layer, &_column, fitParams);
    for (int iEntry = 0; iEntry < t0Reader->GetEntries(); iEntry++) {
      t0Reader->GetEntry(iEntry);
      if (_tdc==tdc) {
	SetIgnoreTube(_layer, _column);
      }
    }
    delete fitParams;
  }

  void RTParam::SetIgnoreTube(int layer, int column) {
    ignoreTube.set(layer, column, 1);
  }

  void RTParam::SetIgnoreAll() {
    for (int layer = 0; layer != Geometry::MAX_TUBE_LAYER; layer++) {
      for (int column = 0; column != Geometry::MAX_TUBE_COLUMN; column++) {
	ignoreTube.set(layer, column, 1);
      }
    }
  }

  void RTParam::ClearIgnore() {
    for (int layer = 0; layer != Geometry::MAX_TUBE_LAYER; layer++) {
      for (int column = 0; column != Geometry::MAX_TUBE_COLUMN; column++) {
        ignoreTube.set(layer, column, 0);
      }
    }
  }

  void RTParam::SetActiveTube(int layer, int column) {
    ignoreTube.set(layer, column, 0);
  }
  
  void RTParam::GetFirstActive(int* layerOut, int* columnOut) {
    for (int layer = 0; layer != Geometry::MAX_TUBE_LAYER; layer++) {
      for (int column = 0; column != Geometry::MAX_TUBE_COLUMN; column++) {
        if (!ignoreTube.get(layer, column)) {
	  *layerOut  = layer;
	  *columnOut = column;
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

  void RTParam::HardCodeT0TF(double tmin, double tmax) {
    for (int layer = 0; layer != Geometry::MAX_TUBE_LAYER; layer++) {
      for (int column = 0; column != Geometry::MAX_TUBE_COLUMN; column++) {
	t0.set(layer, column, tmin);
	tF.set(layer, column, tmax);
      }
    }
  }

  void RTParam::WriteOutputTxt(TString outdir) {
    for (Int_t layer = 0; layer != Geometry::MAX_TUBE_LAYER; layer++) {
      for (Int_t column = 0; column != Geometry::MAX_TUBE_COLUMN; column++) {
	if (t0.get(layer, column) == 0 && 
	    tF.get(layer, column) == 0) {continue;}
	
	TString outfile = IOUtility::join(outdir, TString::Format("LAYER_%i_COL_%i_RT.txt", layer, column));
	// write out via eval, 100 pts
	std::ofstream rtfile;
	rtfile.open(outfile);
	rtfile << "R t\n";
	double tmax = tF.get(layer, column);
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


  void RTParam::Draw(TString title/*=";Drift Time [ns];r(t) [mm]"*/, Bool_t setMinMax/*=kTRUE*/) {
    std::vector<double> x = std::vector<double>();
    std::vector<double> y = std::vector<double>();
    std::vector<double> ex = std::vector<double>();
    std::vector<double> ey = std::vector<double>();
    double delta = 0.01;
    for (double _x = -1; _x <= 1; _x += delta) {
      x.push_back((_x+1.)/2.*175.);
      y.push_back(func->Eval(_x));
      ex.push_back(delta/2.);
      ey.push_back(Hit::RadiusError(y.at(y.size()-1)));
    }
    auto ge = new TGraphErrors(x.size(), &x[0], &y[0], &ex[0], &ey[0]);
    ge->SetFillColor(6);
    ge->SetFillStyle(3005);
    ge->SetTitle(title);
    ge->SetLineWidth(1);
    ge->SetMarkerStyle(0);
    ge->SetLineColor(kBlue);
    if (setMinMax) {
      ge->SetMinimum(0);
      ge->SetMaximum(Geometry::max_drift_dist);
    }
    ge->Draw("a3 L");
  }

  void RTParam::SaveImage(TString outdir, TString title/*=";Drift Time [ns];t(t) [mm]"*/, 
			  Bool_t setMinMax /*=kTRUE*/) {
    SyncTF1ToParam();
    TCanvas* canv = new TCanvas("canv", "RT Function");
    canv->cd();
    func->Print();
    TH1* hist = new TH1D("hist", "RT Function", 1000, -1, 1);
    
    Draw(title, setMinMax);
    canv->SaveAs(IOUtility::join(outdir, TString("rtfunction_") + GetName() + ".png"));
  }

  void RTParam::PrintActive() {
    std::cout << "Active Tubes: " << std::endl;
    for (int layer = 0; layer != Geometry::MAX_TUBE_LAYER; layer++) {
      for (int column = 0; column != Geometry::MAX_TUBE_COLUMN; column++) {
	if (ignoreTube.get(layer, column)) {continue;}
	std::cout << "Layer: " << layer << " Column: " << column << std::endl;
      }
    }
    std::cout << std::endl;
  }
}

