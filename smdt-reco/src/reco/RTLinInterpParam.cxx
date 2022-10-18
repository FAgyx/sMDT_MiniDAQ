#include "MuonReco/RTLinInterpParam.h"

namespace MuonReco {
  RTLinInterpParam::RTLinInterpParam() : Optimizer(), Parameterization(npar) {
    t0 = TubeMap<double>(Geometry::MAX_TUBE_LAYER, Geometry::MAX_TUBE_COLUMN);
    tF = TubeMap<double>(Geometry::MAX_TUBE_LAYER, Geometry::MAX_TUBE_COLUMN);
  }

  RTLinInterpParam::RTLinInterpParam(ConfigParser cp) : RTLinInterpParam() {
    isMC = cp.items("General").getBool("IsMC", 0, 0);
  }

  RTLinInterpParam::~RTLinInterpParam() {
    if (gr) delete gr;
  }

  void RTLinInterpParam::Initialize() {

    TString fname = "raw/Rt_BMG_6_1.dat";
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
    
    gr = new TGraph(v_radius.size(), &v_time[0], &v_radius[0]);

    SyncParamToTGraph();

    indexMinTime = std::vector<double>();
    indexMaxTime = std::vector<double>();
    double _r1, _t1, _r2, _t2;
    for (int i = 0; i < gr->GetN()-1; i++) {
      gr->GetPoint(i,   _t1, _r1);
      gr->GetPoint(i+1, _t2, _r2);
      indexMinTime.push_back(_t1);
      indexMaxTime.push_back(_t2);
    }
    indexMinTime.push_back(_t2);
    indexMaxTime.push_back(_t2+1);
  }

  void RTLinInterpParam::Initialize(TString t0path, TString decodedDataPath) {
    _t0path = t0path;
    std::cout << "topath: " << t0path << std::endl << "decoded: " << decodedDataPath << std::endl;


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

	std::cout << "tdc: " << tdc_id << ", chan: " << ch_id << std::endl;
	std::cout << "layer: " << layer << ", column: " << column << std::endl;
	std::cout << "t0: " << (*fitParams)[T0Fit::T0_INDX] << " tF: " << (*fitParams)[T0Fit::MAX_DRIFT_INDX] << std::endl;

        t0.set(layer, column, (*fitParams)[T0Fit::T0_INDX]); 
        tF.set(layer, column, (*fitParams)[T0Fit::MAX_DRIFT_INDX]);

        histName.Form("TDC_%02d_of_%02d_Time_Spectrum/tdc_%d_channel_%d_tdc_time_spectrum",
                      tdc_id, Geometry::MAX_TDC, tdc_id, ch_id);
        tempHist = (TH1D*) driftFile.Get(histName);

        for (int b = 0; b <= tempHist->GetNbinsX(); b++) {
          driftTimes->Fill(NormalizedTime(tempHist->GetBinCenter(b), layer, column), tempHist->GetBinContent(b));
        }

      }
      delete fitParams;
    }


    Initialize();
    SyncParamToTGraph();
  }

  double RTLinInterpParam::Eval(double time) {
    if      (time < -1) return 0;
    else if (time > 1)  return Geometry::max_drift_dist;
    else                return gr->Eval((time+1.)/2.*188.688);
  }

  double RTLinInterpParam::Eval(Hit h, double deltaT0/*=0*/, double slewScaleFactor/*=1.0*/, double sigPropSF/*=1.0*/) {
    SyncTGraphToParam();
    double nTime = NormalizedTime(h.CorrTime()+deltaT0, h.Layer(), h.Column());
    return Eval(nTime);
  }

  double RTLinInterpParam::NormalizedTime(double time, int layer, int column) {
    return (time-t0.get(layer, column))/(tF.get(layer, column))*2 - 1;
  }

  double RTLinInterpParam::D(int index, Hit h) {
    double nTime = NormalizedTime(h.CorrTime(), h.Layer(), h.Column());
    double time  = (nTime+1.)/2.*188.688;
    if (time >= indexMinTime.at(index) && time < indexMaxTime.at(index)) {
      return 1-(time-indexMinTime.at(index))/(indexMaxTime.at(index)-indexMinTime.at(index));
    }
    else if (index < 0 && time >=indexMinTime.at(index-1) && time < indexMaxTime.at(index-1)) {
      return (time-indexMinTime.at(index-1))/(indexMaxTime.at(index-1)-indexMinTime.at(index-1));
    }
    else return 0;
  }

  double RTLinInterpParam::Residual(Hit h) {
    return -dependencies.at(0)->Residual(h);
  }

  void RTLinInterpParam::Print() {
    if (gr) {
      std::cout << "=== RT Function ===" << std::endl;
      std::cout << "" << std::endl;
      double _r, _t;
      for (int i = 0; i < gr->GetN(); i++) {
	gr->GetPoint(i, _t, _r);
	std::cout << i << " : " << _t << " : " << _r  << std::endl;
      }
    }
    else {
      std::cout << "Print method of uninitialized rt function" << std::endl;
    }
  }

  double RTLinInterpParam::Distance(Hit h) {
    return Eval(h);
  }

  void RTLinInterpParam::operator+=(Parameterization delta) {
    if (delta.size() == this->size()) {
      for (int i = 0; i < npar; i++) {
	param[i] = this->operator[](i) + delta[i];
      }
      SyncTGraphToParam();
    }
    else {
      std::cout << "Cannot add parameterizations with different dimensions" << std::endl;
      throw 1;
    }
  }

  void RTLinInterpParam::SyncTGraphToParam() {
    double _r, _t;
    for (int i = 0; i < npar; i++) {
      gr->GetPoint(i, _t, _r);
      gr->SetPoint(i, _t, param[i]);
    }
  }

  void RTLinInterpParam::SyncParamToTGraph() {
    double _r, _t;
    for (int i = 0; i < npar; i++) {
      gr->GetPoint(i, _t, _r);
      param[i] = _r;
    }
  }

  void RTLinInterpParam::Write(TString tag /*= ""*/) {
    gr->Write("RTParam" + tag);
  }

  void RTLinInterpParam::Load(TFile* infile, TString tag /*= ""*/) {
    gr = (TGraph*)infile->Get("RTParam" + tag);
    SyncParamToTGraph();
  }

  void RTLinInterpParam::SaveImage(TString outdir) {
    TCanvas* c1 = new TCanvas("c1", "R(t) function");
    c1->cd();
    gr->GetXaxis()->SetTitle("Time [ns]");
    gr->GetYaxis()->SetTitle("Radius [mm]");
    gr->Draw();
    c1->SaveAs(IOUtility::join(outdir, "rtfunction.png"));
  }

}
