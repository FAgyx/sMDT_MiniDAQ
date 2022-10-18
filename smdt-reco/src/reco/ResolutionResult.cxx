#include "MuonReco/ResolutionResult.h"

namespace MuonReco {

  TString ResolutionResult::ROOT_OUTPUT_NAME  = "ResolutionResult";
  TString ResolutionResult::RESIDUALS         = "RR_residuals";
  TString ResolutionResult::CHISQ             = "RR_chiSq";
  TString ResolutionResult::RADIUS            = "RR_radius";
  TString ResolutionResult::HITSPEREVENT      = "RR_hitsPerEvent";
  TString ResolutionResult::RESVSHITRADIUS    = "RR_resVsHitRadius";
  TString ResolutionResult::RESVSHITTIME      = "RR_resVsHitTime";
  TString ResolutionResult::MEANRESVSHITTIME  = "RR_meanResVsHitTime";
  TString ResolutionResult::RESOLUTIONRADIUS  = "RR_resolutionRadius"; 
  TString ResolutionResult::DELTAT0           = "RR_deltaT0";
  TString ResolutionResult::RESIDUALVSADC     = "RR_residualVsADC";
  TString ResolutionResult::SYSTEMATICT0SHIFT = "RR_systematicT0Shift";
  TString ResolutionResult::XDISPLACEMENT     = "RR_xDisplacement";
  TString ResolutionResult::TUBEWIDTHMAP      = "RR_tubeWidthMap";
  TString ResolutionResult::TUBEMEANMAP       = "RR_tubeMeanMap";
  TString ResolutionResult::SYSTVSANGLE       = "RR_systVsAngle";
  
  TString ResolutionResult::SYSTNAME          = "";
  double  ResolutionResult::MAXSHIFT          = 1.0;
  double  ResolutionResult::SYSTSF            = 1.0;
  
  bool ResolutionResult::init = kFALSE;
  TF1* ResolutionResult::chi[12];

  ResolutionResult::ResolutionResult(ConfigParser cp) {
    geo = new Geometry();

    runN     = cp.items("General").getInt("RunNumber");
    t0path   = IOUtility::getT0FilePath(runN);
    isMC     = cp.items("General").getBool("IsMC", 0, 0);

    if (!isMC) geo->Configure(cp.items("Geometry"));

    residuals = new TH1D("residuals",  "Residuals",   500, -500,500);
    residuals->GetXaxis()->SetTitle("Residual [#mum]");
    residuals->GetYaxis()->SetTitle("Number of hits/40#mum");

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

    resVsHitRadius = new TH2D("resVsHitRadius", "Residuals vs. Hit Radius", 100, 0, Geometry::radius, 2000, -1000,1000);
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

    residualVsADC    = new TH2D("residualVsADC", "", 50, 0, 0.02, 500, -10, 10);
    residualVsADC->GetXaxis()->SetTitle("1/ADC");
    residualVsADC->GetYaxis()->SetTitle("Residual/drift velocity [ns]");
    residualVsADC->GetZaxis()->SetTitle("Entries");
    residualVsADC->SetStats(0);

    xDisplacement = new TH1D("xDisplacement", ";x-#mu [mm];Number of hits/2mm", 100, -100, 100);    

    double min = (MAXSHIFT>0) ? 0 : MAXSHIFT;
    double max = (MAXSHIFT<0) ? 0 : MAXSHIFT;
    t0Systematic = new TH1D("t0systematic", ";1 #sigma shift in " + SYSTNAME + ";Number of events", 100, min, max);

    systVsAngle  = new TH2D("systVsAngle", ";1 #sigma shift in " + SYSTNAME  + ";#theta [degrees]", 100, min, max, 60, -60, 60);


    for (int layer = 0; layer != Geometry::MAX_TUBE_LAYER; layer++) {
      for (int column = 0; column != Geometry::MAX_TUBE_COLUMN; column++) {
	tubeLevelRes.set(layer, column, new TH1D(TString::Format("layer_%i_column_%i_res", layer, column), "", 100, -1000, 1000));
      }
    }

    tubeDoubGausWidth = new TH2D(TUBEWIDTHMAP, "Double gaussian weighted width",
				 Geometry::MAX_TUBE_COLUMN,-0.5,Geometry::MAX_TUBE_COLUMN-0.5,
				 Geometry::MAX_TUBE_LAYER,-0.5,Geometry::MAX_TUBE_LAYER-0.5);
    tubeDoubGausMean  = new TH2D(TUBEMEANMAP, "Double gaussian mean",
				 Geometry::MAX_TUBE_COLUMN,-0.5,Geometry::MAX_TUBE_COLUMN-0.5,
				 Geometry::MAX_TUBE_LAYER,-0.5,Geometry::MAX_TUBE_LAYER-0.5);

    tubeResolutions = new TH1D("tubeResolutions", "", 200, 0, 200);
    tubeResolutions->GetXaxis()->SetTitle("Resolution (#mu m)");
    tubeResolutions->GetYaxis()->SetTitle("Number of Tubes");

    tubeMeans = new TH1D("tubeMeans", ";Double Gaussian Mean [#mum];Tubes/2#mum", 30, -30, 30);

    deltaT0 = new TH1D("deltaT0", "", 100, -15, 15);
    deltaT0->GetXaxis()->SetTitle("#Delta T0 [ns]");
    deltaT0->GetYaxis()->SetTitle("Number of events");

    doubGaus = new TF1("doubGaus", "[0]*exp(-0.5*((x-[1])/[2])**2) + [3]*exp(-0.5*((x-[1])/[4])**2)", -1000, 1000);
    doubGaus->SetLineColor(kRed);
    
    singGaus = new TF1("singGaus", "[0]*exp(-0.5*((x-[1])/[2])**2)", -400, 400);
    singGaus->SetLineColor(kRed);
    singGaus->SetParLimits(1, -200, 200);
    singGaus->SetParLimits(2, 50, 500);    
    if (isMC) {
      doubGaus->SetParameter(0, 10000);
      doubGaus->SetParameter(1, 0);
      doubGaus->SetParameter(2, 1);
      doubGaus->SetParameter(3, 2000);
      doubGaus->SetParameter(4, 100);
    }
    else {
      doubGaus->SetParameter(0, 10000);
      doubGaus->SetParameter(1, 0);
      doubGaus->SetParameter(2, 100);
      doubGaus->SetParameter(3, 2000);
      doubGaus->SetParameter(4, 250);
    }

    radialBinProjections = std::vector<TH1*>();    
    init = kFALSE;

    angle_pattern_recog = 0;
    angle_optimized     = 0;
    number_iterations   = 0;

    trackFitTree = new TTree("trackFitTree", "Tree holding diagnostics from track fitting");
    trackFitTree->Branch("angle_pattern_recog", &angle_pattern_recog);
    trackFitTree->Branch("angle_optimized",     &angle_optimized);
    trackFitTree->Branch("number_iterations",   &number_iterations);
    trackFitTree->Branch("impact_par_opt",      &impact_par_opt);
    trackFitTree->Branch("hitN",                &hitN);
    trackFitTree->Branch("hitX",                &hitX);
    trackFitTree->Branch("hitY",                &hitY);
    trackFitTree->Branch("hitL",                &hitL);
    trackFitTree->Branch("hitC",                &hitC);
    trackFitTree->Branch("hitR",                &hitR);
    trackFitTree->Branch("hitRes",              &hitRes);
    trackFitTree->Branch("missedHitN",          &missedHitN);
    trackFitTree->Branch("missedHitR",          &missedHitR);
    trackFitTree->Branch("missedHitL",          &missedHitL);
    trackFitTree->Branch("missedHitC",          &missedHitC);
    trackFitTree->Branch("trackHitN",           &trackHitN);
    trackFitTree->Branch("trackHitR",           &trackHitR);
    trackFitTree->Branch("trackHitL",           &trackHitL);
    trackFitTree->Branch("trackHitC",           &trackHitC);
  }

  ResolutionResult::~ResolutionResult() {
    
    if (residuals)         delete residuals;
    if (chiSq)             delete chiSq;
    if (radius)            delete radius;
    if (resVsHitRadius)    delete resVsHitRadius;
    if (resVsHitTime)      delete resVsHitTime;
    if (meanResVsHitTime)  delete meanResVsHitTime;
    if (hitsPerEvent)      delete hitsPerEvent;
    if (resolutionRadius)  delete resolutionRadius;
    if (tubeDoubGausWidth) delete tubeDoubGausWidth;
    if (tubeDoubGausMean)  delete tubeDoubGausMean;
    
    for (auto p : radialBinProjections) if (p) delete p;
    radialBinProjections.clear();
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
    doubGaus->SetParameter(4, 200);    
    if (!isMC) residuals->Fit("doubGaus", "R");
    residuals->Draw();
    if (!isMC) {
      TPaveText* tpt = DoubGausLegend(residuals);
      tpt->Draw();
    }
    c_res->cd(2);
    chiSq->SetMaximum(1.2*((chiSquareDist->GetMaximum() > chiSq->GetMaximum()) ? chiSquareDist->GetMaximum() : chiSq->GetMaximum()));
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

    std::cout << std::endl;
    std::cout << "Resolution results:" << std::endl;
    //std::cout << " resolution: " << residuals->GetFunction("gaus") << std::endl;
    std::cout << " efficiency: " << nTubesHit/nTubesTrackThrough << std::endl;
    std::cout << std::endl;

    double tubeW, tubeE;
    double minW  = DBL_MAX;

    if (isMC) {
      for (int layer = 0; layer < Geometry::MAX_TUBE_LAYER; layer++) {
	for (int column = 0; column < Geometry::MAX_TUBE_COLUMN; column++) {
	  
	  if (tubeLevelRes.get(layer, column)->GetEntries()) {
	  
	    doubGaus->SetParameter(0, 100);
	    doubGaus->SetParameter(1, 0);
	    doubGaus->SetParameter(2, 100);
	    doubGaus->SetParameter(3, 20);
	    doubGaus->SetParameter(4, 250);
	    
	    tubeLevelRes.get(layer, column)->Fit("doubGaus", "R");
	    GetWidthAndError(&tubeW, &tubeE, tubeLevelRes.get(layer, column));
	    tubeResolutions->Fill(tubeW);
	    tubeMeans->Fill(doubGaus->GetParameter(1));
	    tubeDoubGausWidth->SetBinContent(column+1, layer+1, tubeW);
	    tubeDoubGausMean ->SetBinContent(column+1, layer+1, doubGaus->GetParameter(1));
	  }
	}
      }
      tubeResolutions->Draw();
    } // end if: MC
    else {
      int bestTDC  = 0;
      int bestChan = 0;
      std::cout << "1" << std::endl;
      T0Reader* t0Reader = T0Reader::GetInstance(t0path);
      std::cout << "2" << std::endl;
      TVectorD* fitParams = new TVectorD(NT0FITDATA);
      
      int tdc=0, chan=0, layer=0, column=0;
      std::cout << "Setting addresses" << std::endl;
      t0Reader->SetBranchAddresses(&tdc, &chan, &layer, &column, fitParams);
      std::cout << "Done" << std::endl;

      for (int iEntry = 0; iEntry < t0Reader->GetEntries(); iEntry++) {
	std::cout << "about to read entry " << iEntry << "/" << t0Reader->GetEntries() << std::endl;
	t0Reader->GetEntry(iEntry);
	if (chan<0) continue;
	std::cout << "done" << std::endl;
	doubGaus->SetParameter(0, 100);
	doubGaus->SetParameter(1, 0);
	doubGaus->SetParameter(2, 100);
	doubGaus->SetParameter(3, 20);
	doubGaus->SetParameter(4, 250);
	if (tubeLevelRes.get(layer, column) &&
	    tubeLevelRes.get(layer, column)->GetEntries()>1000) {
	  tubeLevelRes.get(layer, column)->Fit("doubGaus", "R");
	  GetWidthAndError(&tubeW, &tubeE, tubeLevelRes.get(layer, column));
	  tubeResolutions->Fill(tubeW);
	  tubeMeans->Fill(doubGaus->GetParameter(1));
	  tubeDoubGausWidth->SetBinContent(column+1, layer+1, tubeW);
	  tubeDoubGausMean ->SetBinContent(column+1, layer+1, doubGaus->GetParameter(1));
	  if (tubeW < minW) {
	    minW = tubeW;
	    bestTDC = tdc;
	    bestChan = chan;
	  }
	}
      }
      tubeResolutions->Draw();
      std::cout << "The minimum tube width is " << minW << std::endl;
      std::cout << "On tube  TDC=" << bestTDC << ", chan=" << bestChan << std::endl;
    } // end if: data    
  }

  void ResolutionResult::DrawADCPlots(TString t0path) {
    TFile t0File(t0path);
    TTree* fitData = 0;
    t0File.GetObject("FitData", fitData);
    TVectorD *fitParams= new TVectorD(NT0FITDATA);
    
    
    double tubeW, tubeE, tubeM;
    std::vector<double> ADCPeak;
    std::vector<double> TubeRes;
    
    std::cout << "C" << std::endl;
    
    if (fitData) {
      int tdc=0, chan=0, layer=0, column=0;
      fitData->SetBranchAddress("tdc", &tdc);
      fitData->SetBranchAddress("channel", &chan);
      fitData->SetBranchAddress("layer", &layer);
      fitData->SetBranchAddress("column", &column);
      for (int i = 0; i < NT0FITDATA; i++) {
        fitData->SetBranchAddress(fitDataNames[i], &(*fitParams)[i]);
      }

      for (int iEntry = 0; iEntry < fitData->GetEntries(); iEntry++) {
	  fitData->GetEntry(iEntry);
	  if (column>=0 && 
	      tubeLevelRes.get(layer, column) &&
	      tubeLevelRes.get(layer, column)->GetEntries()>1000) {
	    ADCPeak.push_back((*fitParams)[T0Fit::ADC_WIDTH_INDX]);
	    GetWidthAndError(&tubeW, &tubeE, tubeLevelRes.get(layer, column));
	    GetMean(&tubeM, tubeLevelRes.get(layer, column));
	    TubeRes.push_back(tubeM);
	  }
      }
    }
    else {
      std::cout << "Error reading from T0File" << std::endl;
      std::cout << "Fatal." << std::endl;
      throw 1;
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
    hitsPerEvent->Fill(tp.DOF + TrackParam::NPARS);
    
  }

  void ResolutionResult::FillResiduals(TrackParam & tp) {
    
    angle_optimized     = tp.getVerticalAngle();
    impact_par_opt      = tp.getImpactParameter();
    hitX.clear();
    hitY.clear();
    hitL.clear();
    hitC.clear();
    hitR.clear();
    hitRes.clear();
    missedHitR.clear();
    missedHitL.clear();
    missedHitC.clear();
    trackHitR.clear();
    trackHitL.clear();
    trackHitC.clear();

    if (angle_optimized < angular_cut) {
      // fill residual distributions
      tp.hitIndex = -1;
      double res, dist, err, rad, ntime;
      double xmean = (-1.0*tp.y_int() + (Geometry::ML_distance + 
					 (Geometry::MAX_TDC_LAYER-1)*Geometry::layer_distance + 
					 Geometry::radius)/2.0)/tp.slope();
      for (Cluster c : tp.e->Clusters()) {
	for (Hit h : c.Hits()) {
	  tp.hitIndex++;
	  rad   = tp.rtfunction->Eval(h, tp.deltaT0());
	  res   = tp.Residual(h);
	  if (tp.IsRight(h)) res *= -1.0;
	  dist  = tp.Distance(h);
	  err   = Hit::RadiusError(dist);
	  ntime = tp.rtfunction->NormalizedTime(h.CorrTime() - tp.deltaT0(), h.Layer(), h.Column());
	  if (dist>0 && dist < Geometry::radius && TMath::Abs(res) < tp.maxResidual*err) {
	    if ((tp.anySkip() && tp.skip()) || !tp.anySkip()) {
	      residuals       ->Fill(1000*res);
	      radius          ->Fill(rad);
	      resVsHitRadius  ->Fill(rad,res*1000.0);
	      resVsHitTime    ->Fill(ntime,res*1000.0);
	      meanResVsHitTime->Fill(ntime,res*1000.0);
	      xDisplacement   ->Fill(h.X() - xmean);
	      residualVsADC   ->Fill(1/h.ADCTime(), 25*res);// divide out nominal drift velocity
	      tubeLevelRes.get(h.Layer(), h.Column())->Fill(res*1000.0);
	      hitX.push_back(h.X());
	      hitY.push_back(h.Y());
	      hitL.push_back(h.Layer());
	      hitC.push_back(h.Column());
	      hitR.push_back(rad);
	      hitRes.push_back(1000*res);
	    }
	  }
	}
      }

      double _hitX, _hitY;
      // fill efficiency distribution
      for (int iL = 0; iL < Geometry::MAX_TUBE_LAYER; iL++) {
	for (int iC = 0; iC < Geometry::MAX_TUBE_COLUMN; iC++) {	  
	  if (geo->IsActiveLayerColumn(iL, iC)) {
	    geo->GetHitXY(iL, iC, &_hitX, &_hitY);
	    // get track x position and figure out what tube(s) it may go through
	    double trackDist = tp.Distance(Hit(0, 0, 0, 0, 0, 0, iL, iC, _hitX, _hitY));
	    if (doMCMCS) {
	      double sign = (gRandom->Uniform() < 0.5) ? -1.0 : 1.0;
	      if (gRandom->Uniform() < 0.51) {
		trackDist += sign*gRandom->Exp(9.4123/1000);
	      }
	      else {
		trackDist += sign*gRandom->Exp(58.782/1000);
	      }
	    }
	    
	    if (trackDist <= Geometry::column_distance/2) {
	      Bool_t tubeIsHit = kFALSE;
	      for (Hit h : tp.e->WireHits()) {
		if (h.Layer() == iL && h.Column() == iC) tubeIsHit = kTRUE;
	      }
	      if (!tubeIsHit)	{
		missedHitR.push_back(trackDist);
		missedHitL.push_back(iL);
		missedHitC.push_back(iC);
	      }
	      else {
		trackHitR.push_back(trackDist);
		trackHitL.push_back(iL);
		trackHitC.push_back(iC);
	      }
	    } // end if: track passes through gas volume
	  } // end if: check only active tubes
	} // end for: column
      } // end for: layer

      // fill delta t 0 distribution and other global distributions
      deltaT0->Fill(tp.deltaT0());  
      t0Systematic->Fill(tp.getSystShift()*SYSTSF);
      systVsAngle->Fill(tp.getSystShift()*SYSTSF, tp.getVerticalAngle()*TMath::RadToDeg());

      // fill diagnostics tree
      angle_pattern_recog = tp.initialAngle;
      angle_optimized     = tp.getVerticalAngle();
      number_iterations   = tp.getNIterations();
      
      hitN       = hitX.size();
      missedHitN = missedHitR.size();
      trackHitN  = trackHitR.size();
      trackFitTree->Fill();
    }
    else std::cout << "Failed cut: " << angle_optimized << std::endl;
  }

  void ResolutionResult::FillResidualByValue(double residual, double radius) {
    residuals       ->Fill(residual);
    resVsHitRadius  ->Fill(radius, residual);
  }


  TF1* ResolutionResult::FitResVsTime(int npar) {
    TF1* deltaFit = (TF1*) gROOT->GetFunction(TString("chebyshev") + std::to_string(npar));
    meanResVsHitTime->Fit(deltaFit);
    return deltaFit;
  }


  Bool_t ResolutionResult::PassCuts(Hit h, const Geometry & geo) {
    return kTRUE;
  }

  void ResolutionResult::SaveImages(TString outdir, Bool_t writeSlewResults) {
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


      py = resVsHitRadius->ProjectionY(("_py"), binL, binU);
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
    if (!isMC) {
      TPaveText* tpt  = DoubGausLegend(residuals);
      tpt             ->Draw();
    }
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
    deltaT0         ->Draw();
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::DELTAT0 + ".png"));
    t0Systematic    ->Draw();    
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::SYSTEMATICT0SHIFT + ".png"));
    gPad->SetRightMargin(0.15);
    systVsAngle     ->Draw("colz");
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::SYSTVSANGLE + ".png"));
    gPad->SetRightMargin(0.05);
    xDisplacement   ->Draw();    
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::XDISPLACEMENT + ".png"));
    tubeResolutions ->Draw();
    plot            ->SaveAs(IOUtility::join(outdir, "RR_tubeWidthHist.png"));
    tubeMeans       ->Draw();
    plot            ->SaveAs(IOUtility::join(outdir, "RR_tubeMeanHist.png"));
    plot            ->SetRightMargin(0.2);
    int maxDig = TGaxis::GetMaxDigits();
    TGaxis::SetMaxDigits(3);
    residualVsADC   ->Draw("colz");
    plot->Update();
    TPaletteAxis *palette =
      (TPaletteAxis*)residualVsADC->GetListOfFunctions()->FindObject("palette");
    //palette->SetX1NDC(0.88);
    //palette->SetX2NDC(0.92);
    palette->SetY1NDC(0.25);
    //palette->SetY2NDC(0.95);
    plot->Modified();

    // overlay mean residual vs adc
    std::cout << "Calculating mean residual vs. ADC" << std::endl;
    double maxADC = 200;
    double minADC = 70;
    double deltaADC = 10;
    std::vector<double> _x = std::vector<double>();
    std::vector<double> _y = std::vector<double>();
    std::vector<double> _ex = std::vector<double>();
    std::vector<double> _ey = std::vector<double>();
    for (double lower = minADC; lower < maxADC; lower += deltaADC) {
      upper = lower + deltaADC;
      binL  = (int)(1+lower/4.);
      binU  = (int)(1+upper/4.);
      std::cout << "binL: " << binL << " binU: " << binU << std::endl;
      py    = residualVsADC->ProjectionY("_py", binL, binU);
      
      _x .push_back(2/(upper+lower));
      if (_x.size() < 2)
	_ex.push_back(0.0015);
      else
	_ex.push_back(TMath::Abs(_x.at(_x.size()-1) - _x.at(_x.size()-2)));
      _y .push_back(py->GetMean());
      _ey.push_back(py->GetMeanError());
    }
    
    TF1* slewFunction = new TF1("slewFunction", "[0]*x+[1]", minADC, maxADC);
    slewFunction->SetParameter(0,0);
    slewFunction->SetParameter(1,0);
    
    TGraphErrors* tge = new TGraphErrors(_x.size(), &_x[0], &_y[0], &_ex[0], &_ey[0]);
    tge->SetLineColor(kRed);
    tge->SetLineWidth(2);
    tge->SetMarkerColor(kRed);
    tge->SetMarkerStyle(20);
    tge->SetMarkerSize (1);
    tge->Fit("slewFunction");
    tge->GetFunction("slewFunction")->SetLineColor(kBlack);
    tge->Draw("P Z E same");
    auto fitlegend = new TLegend(0.4,0.2,0.75,0.3);
    fitlegend->AddEntry(tge->GetFunction("slewFunction"), 
			TString::Format("t=%.2f/ADC + %.2f", 
					tge->GetFunction("slewFunction")->GetParameter(0), 
					tge->GetFunction("slewFunction")->GetParameter(1)), "l");
    fitlegend->Draw();
    tge->Print();
    plot            ->SaveAs(IOUtility::join(outdir, ResolutionResult::RESIDUALVSADC + ".png"));
    delete py;
    TGaxis::SetMaxDigits(maxDig);

    if (!isMC) {
      std::cout << "About to draw tube hist" << std::endl;
      EventDisplay ed = EventDisplay();
      ed.SetOutputDir(outdir);
      ed.DrawTubeHist(*geo, tubeDoubGausWidth);
      ed.DrawTubeHist(*geo, tubeDoubGausMean);
    }
    std::cout << "Finished drawing ResolutionResult" << std::endl;
    
    if (writeSlewResults) {
      TimeCorrection tc = TimeCorrection(runN);
      tc.SetScale(tge->GetFunction("slewFunction")->GetParameter(1));
      tc.SetCoefficient(tge->GetFunction("slewFunction")->GetParameter(0));
      tc.Write();
    }
    delete tge;

  }
  
  void ResolutionResult::WriteTree() {
    // write out tree to file
    trackFitTree->CloneTree()->Write("trackFitTree");
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
      if (py->GetEntries()) {
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

  }
  
  void ResolutionResult::GetWidthAndError(double* wid, double* err, TH1* hist) {
    TF1* gaus = hist->GetFunction("singGaus");
    if (gaus != 0) {
      *wid = TMath::Abs(gaus->GetParameter(2));
      *err = TMath::Abs(gaus->GetParError(2));
      return;
    }

    TF1* dg = hist->GetFunction("doubGaus");
    if (dg != 0) {    
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
      
      if (sigma_n.val < 30) {
	*wid = sigma_w.val;
	*err = sigma_w.err;
	return;
      }
      if (sigma_w.val > 400) {
	*wid = sigma_n.val;
	*err = sigma_n.err;
	return;
      }
      
      Observable sigma = (sigma_n*amp_n + sigma_w*amp_w)/(amp_n + amp_w);
      
      *wid = sigma.val;
      *err = sigma.err;
      return;
    }
    
    *wid = hist->GetStdDev();
    *err = hist->GetStdDevError();
    
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
    


    TPaveText* tpt = new TPaveText(0.19, 0.42, 0.39, 0.92, "NDC");
    tpt->AddText(TString::Format("A_{n}=%.2f#pm%.2f",      amp_n  .val, amp_n  .err));
    tpt->AddText(TString::Format("#sigma_{n}=%.2f#pm%.2f", sigma_n.val, sigma_n.err));
    tpt->AddText(TString::Format("A_{w}=%.2f#pm%.2f",      amp_w  .val, amp_w  .err));
    tpt->AddText(TString::Format("#sigma_{w}=%.2f#pm%.2f", sigma_w.val, sigma_w.err));
    tpt->AddText(TString::Format("#mu = %.2f#pm%.2f",      mean,    mean_err));
    //tpt->AddText(TString::Format("A_{1}/A_{2} = %.2f", area1/area2));
    tpt->AddText(TString::Format("#sigma = %.2f#pm%.2f",   sigma.val, sigma.err ));
    return tpt;
    
  }

  TPaveText* ResolutionResult::GausLegend(TH1* h) {
    double width, wid_err, mean, mean_err, amp, amp_err;
    TF1* g = h->GetFunction("singGaus");
    TPaveText* tpt = new TPaveText(0.19, 0.62, 0.39, 0.92, "NDC");

    if (g!=0) {
      width    = TMath::Abs(g->GetParameter(2));
      mean     = TMath::Abs(g->GetParameter(1));
      amp      = TMath::Abs(g->GetParameter(0));
      
      wid_err  = TMath::Abs(g->GetParError(2));
      mean_err = TMath::Abs(g->GetParError(1));
      amp_err  = TMath::Abs(g->GetParError(0));      

      tpt->AddText(TString::Format("A=%.2f#pm%.2f",      amp, amp_err));
    }
    else {
      width    = TMath::Abs(h->GetStdDev());
      mean     = TMath::Abs(h->GetMean());
      
      wid_err  = TMath::Abs(h->GetStdDevError());
      mean_err = TMath::Abs(h->GetMeanError());
    }
    tpt->AddText(TString::Format("#sigma_{n}=%.2f#pm%.2f", width, wid_err));
    tpt->AddText(TString::Format("#mu = %.2f#pm%.2f",      mean,    mean_err));
    return tpt;

  }

  void ResolutionResult::Write() {
    residuals        ->Write(ResolutionResult::RESIDUALS);
    chiSq            ->Write(ResolutionResult::CHISQ);
    radius           ->Write(ResolutionResult::RADIUS);
    hitsPerEvent     ->Write(ResolutionResult::HITSPEREVENT);
    resVsHitRadius   ->Write(ResolutionResult::RESVSHITTIME);
    resVsHitTime     ->Write(ResolutionResult::RESVSHITRADIUS);
    meanResVsHitTime ->Write(ResolutionResult::MEANRESVSHITTIME);
    resolutionRadius ->Write(ResolutionResult::RESOLUTIONRADIUS);
    residualVsADC    ->Write(ResolutionResult::RESIDUALVSADC);
    tubeDoubGausMean ->Write(ResolutionResult::TUBEMEANMAP);
    tubeDoubGausWidth->Write(ResolutionResult::TUBEWIDTHMAP);
    WriteTree();

    if (!isMC) {
      T0Reader* t0Reader = T0Reader::GetInstance(t0path);
      TVectorD *fitParams = new TVectorD(NT0FITDATA);
      int tdc_id=0, ch_id=0, layer=0, column=0;
      t0Reader->SetBranchAddresses(&tdc_id, &ch_id, &layer, &column, fitParams);
      for (int iEntry = 0; iEntry < t0Reader->GetEntries(); iEntry++) {
	t0Reader->GetEntry(iEntry);
	if (ch_id<0) continue;
	tubeLevelRes.get(layer, column)->Write(TString::Format("layer_%i_column_%i_res", layer, column));
      }
    }
  }
  
  void ResolutionResult::Load(TString fname) {
    TFile* file = new TFile(fname);
    residuals         = (TH1D*)    file->Get(ResolutionResult::RESIDUALS);
    chiSq             = (TH1D*)    file->Get(ResolutionResult::CHISQ);
    radius            = (TH1D*)    file->Get(ResolutionResult::RADIUS);
    hitsPerEvent      = (TH1D*)    file->Get(ResolutionResult::HITSPEREVENT);
    resVsHitRadius    = (TH2D*)    file->Get(ResolutionResult::RESVSHITTIME);
    resVsHitTime      = (TH2D*)    file->Get(ResolutionResult::RESVSHITRADIUS);
    meanResVsHitTime  = (TProfile*)file->Get(ResolutionResult::MEANRESVSHITTIME);
    resolutionRadius  = (TH1D*)    file->Get(ResolutionResult::RESOLUTIONRADIUS);
    residualVsADC     = (TH2D*)    file->Get(ResolutionResult::RESIDUALVSADC);
    tubeDoubGausMean  = (TH2D*)    file->Get(ResolutionResult::TUBEMEANMAP);
    tubeDoubGausWidth = (TH2D*)    file->Get(ResolutionResult::TUBEWIDTHMAP);
    trackFitTree      = (TTree*)   file->Get("trackFitTree");

    if (!isMC) {
      T0Reader* t0Reader = T0Reader::GetInstance(t0path);
      TVectorD *fitParams = new TVectorD(NT0FITDATA);
      int tdc_id=0, ch_id=0, layer=0, column=0;
      t0Reader->SetBranchAddresses(&tdc_id, &ch_id, &layer, &column, fitParams);
      for (int iEntry = 0; iEntry < t0Reader->GetEntries(); iEntry++) {
	t0Reader->GetEntry(iEntry);
	if (ch_id < 0) continue;
	tubeLevelRes.set(layer, column, (TH1D*) file->Get(TString::Format("layer_%i_column_%i_res", layer, column)));
      }
    }
  }


  void ResolutionResult::InitChiSquare() {
    for (int dof = 1; dof <= MAXHITS-2; dof++) {
      std::cout << "set bin " << dof-1 << std::endl;
      chi[dof-1] = new TF1(TString::Format("chi_%i", dof), "[0]*TMath::Power(2, -[1]/2)/TMath::Gamma([1]/2)*TMath::Power(x, [1]/2-1)*TMath::Exp(-x/2)", 0, 100);
      chi[dof-1]->SetParameter(1, dof);
    }
    init = kTRUE;
  }

  void ResolutionResult::SetChiSquarePars(TH1* nHitsDist) {
    int dof;
    for (int nHits = 4; nHits <= 12; nHits++) {
      dof = nHits - TrackParam::NPARS;
      chi[dof-1]->SetParameter(0, nHitsDist->GetBinContent(nHits+1));
      std::cout << "Setting parameter: " << dof << " to " << nHitsDist->GetBinContent(nHits+1) << std::endl;
    }
  }

  double ResolutionResult::ChiSquareCalc(double *x, double* par) {
    double result = 0;
    int dof;
    for (int nHits = 4; nHits <= MAXHITS; nHits++) {
      dof = nHits - TrackParam::NPARS;
      result += chi[dof-1]->EvalPar(x, 0);
    }
    return result;
  }

  TF1* ResolutionResult::ChiSquareDist(TH1* nHitsDist) {
    if (!init) InitChiSquare();
    SetChiSquarePars(nHitsDist);
    TF1* chiSquareDist = new TF1("chiSquareDist", ChiSquareCalc, 0, 100, 0);
    return chiSquareDist;
  }

  TH1* ResolutionResult::GetUncorrectedResolutionCurve(ResolutionResult & other, TString dir /*="."*/) {
    // in this context, other is a biased/unbiased resolution result to pair with this
    // unbiased/biased
    TCanvas* c_nocorr = new TCanvas("c_nocorr", "", 800, 600);
    c_nocorr->cd();
    std::cout << "start get uncorr" << std::endl;
    double upper;
    double maxRad = TMath::Floor(Geometry::radius);

    TH1* resolution = new TH1F("uncorrectedResolution", "", (int)maxRad, 0, (int)maxRad);

    TH1* thisResiduals, *otherResiduals;

    double w_this, w_other, e_this, e_other;

    for (double lower = 0; lower < maxRad; lower+=1.0) {
      upper = lower+1;

      trackFitTree->Draw(TString::Format("hitRes>>thisResiduals_%.2f(400,-800,800)", lower), 
			 TString::Format("hitR>=%.2f && hitR<%.2f", lower, upper));
      thisResiduals = (TH1*)(gROOT->FindObject(TString::Format("thisResiduals_%.2f", lower)));
      //thisResiduals->Fit("doubGaus");
      GetWidthAndError(&w_this, &e_this, thisResiduals);
      TPaveText* tpt = GausLegend(thisResiduals);
      tpt->Draw();

      c_nocorr->Print(IOUtility::join(dir, TString::Format("fitResiduals_nocorr_%d.png", (int)lower), kFALSE));

      other.trackFitTree->Draw(TString::Format("hitRes>>otherResiduals_%.2f(400,-800,800)", lower), 
			       TString::Format("hitR>=%.2f && hitR<%.2f", lower, upper));
      otherResiduals = (TH1*)(gROOT->FindObject(TString::Format("otherResiduals_%.2f", lower)));
      //otherResiduals->Fit("doubGaus");
      GetWidthAndError(&w_other, &e_other, otherResiduals);
      tpt = GausLegend(otherResiduals);
      tpt->Draw();

      c_nocorr->Print(IOUtility::join(dir, TString::Format("hitResiduals_nocorr_%d.png", (int)lower), kFALSE));

      Observable o1 = Observable(w_this, e_this);
      Observable o2 = Observable(w_other, e_other);
      
      o1 *= o2;
      Observable o3 = o1.power(0.5);

      resolution->SetBinContent((int)(lower+1), o3.val);
      resolution->SetBinError((int)(lower+1),   TMath::Sqrt(o3.err*o3.err + 8.6*8.6));
      delete thisResiduals;
      delete otherResiduals;
      delete tpt;
      c_nocorr->Clear();
    }
    std::cout << "end get uncorr" << std::endl;
    return resolution;
  }


  TH1* ResolutionResult::Deconvolute(ResolutionResult &other, Bool_t hitResiduals/*=kFALSE*/, Bool_t reconv/*=kFALSE*/) {
    // deconvolute total residual distribution
    TString name;
    if (hitResiduals)
      if (reconv)
	name = "HitResidualsConvolution.png";
      else
	name = "HitResidualsDeconvolution.png";
    else 
      if (reconv)
	name = "FitResidualsConvolution.png";
      else
	name = "FitResidualsDeconvolution.png";
    Deconvolute(other.residuals, this->residuals, name, reconv);

    // store projections for future use
    TH1* py;
    double upper, width, error;
    double maxRad = TMath::Floor(Geometry::radius);
    if (radialBinProjections.size() == 0) {
      std::cout << "CREATING NEW ORIGINALS" << std::endl;
      for (double lower = 0; lower < maxRad; lower+=1) {
	upper = lower+1;
	TString histName = "";
	if (hitResiduals) histName = TString::Format("h_py_copy_%i", (int)lower);
	else histName = TString::Format("f_py_copy_%i", (int)lower);

	trackFitTree->Draw(TString("hitRes>>") + histName + "(400,-800,800)", TString::Format("hitR>%.2f && hitR<%.2f", lower, upper));

	py = (TH1*)(gROOT->FindObject(histName));

	radialBinProjections.push_back(py);
      }
    }

    // deconvolute residual vs. radius distributions
    if (hitResiduals) name = "h_hitResVsRadius";
    else name = "h_fitResVsRadius";
    TH1* result = new TH1D(name, "residuals vs radius", 7, 0, 7);    
    TH1* otherpy;

    other.trackFitTree->Draw("hitRes>>MC_hist(400,-800,800)");
    otherpy = (TH1*)(gROOT->FindObject("MC_hist"));

    if (hitResiduals) 
      if (reconv)
	name = "HitResidual_%i_Convolution.png";
      else
	name = "HitResidual_%i_Deconvolution.png";
    else
      if (reconv)
	name = "FitResidual_%i_Convolution.png";
      else
	name = "FitResidual_%i_Deconvolution.png";

    for (double lower = 0; lower < maxRad; lower+=1) {
      upper = lower+1;

      py      = radialBinProjections.at((int)lower);

      if (py->GetEntries()) {
	Deconvolute(otherpy, py, TString::Format(name, (int)lower), reconv, kTRUE); // SAMESIGMA
	// 
	//Commented out following line to use STD
	GetWidthAndError(&width, &error, py);

	
	std::cout << lower << " mm Width: " << width << " +/- " << error << " microns" << std::endl;
	result->SetBinContent((int)lower + 1, width);
	//if (error < 15) result->SetBinError  ((int)lower + 1, error);
	//else result->SetBinError((int)lower + 1, 15);
	result->SetBinError((int)lower + 1, TMath::Sqrt(8.6*8.6 + result->GetBinError((int)lower + 1)*result->GetBinError((int)lower + 1)));
      }
    }
    result->GetXaxis()->SetTitle("Radius [mm]");
    result->GetYaxis()->SetTitle("Resolution [#mu m]");
    result->SetTitle("");
    delete otherpy;
    for (auto *hist : radialBinProjections) delete hist;
    radialBinProjections.clear();
    return result;
  }

  /**
   * Adjust histogram to have zero mean.  Useful to do before deconvolution
   */
  void ResolutionResult::SetZeroMean(TH1* hist) {
    double mean     = hist->GetMean();
    double binWidth = hist->GetBinWidth(hist->GetNbinsX()/2);
    if (TMath::Abs(mean) < binWidth) return;
    int nBinShift = mean/binWidth;

    int binStart, binEnd, binDelta;
    if (mean < 0) {
      binStart =  hist->GetNbinsX();
      binEnd   =  0;
      binDelta = -1;
    }
    else {
      binStart = 1;
      binEnd   = hist->GetNbinsX()+1;
      binDelta = 1;
    }

    for (int b = binStart; b != binEnd; b += binDelta) {
      if (b+nBinShift >= 1 && b+nBinShift <= hist->GetNbinsX()) {
	hist->SetBinContent(b, hist->GetBinContent(b+nBinShift));
      }
      else hist->SetBinContent(b, 0);
    }
  }

  void ResolutionResult::Deconvolute(TH1* g, TH1* h, TString name, Bool_t reconv /*=kFALSE*/, Bool_t sameSigma/*=kFALSE*/) {
    if (!reconv) SetZeroMean(h);
    TCanvas* c = new TCanvas("c", "FFT Deconvolution", 800, 600);

    c->Divide(3,4);
    c->cd(1);
    h->GetXaxis()->SetTitleSize(0.1);
    h->Draw();
    c->cd(2);
    g->GetXaxis()->SetTitleSize(0.1);
    g->Draw();

    int i;
    int nbins = g->GetNbinsX();
    int N = g->GetNbinsX()-1;
    TH1* fftg_r = 0;
    TH1* fftg_i = 0;
    TH1* fftg_m = 0;
    TH1* ffth_r = 0;
    TH1* ffth_i = 0;
    TH1* ffth_m = 0;
    TVirtualFFT::SetTransform(0);

    // Perform fourier transforms, RE, IM, MAG

    fftg_r = g->FFT(fftg_r, "RE");
    fftg_i = g->FFT(fftg_i, "IM");
    fftg_m = g->FFT(fftg_m, "MAG");
    c->cd(5);
    fftg_r->SetTitle("Real Component of FFT");
    fftg_r->Draw();
    c->cd(8);
    fftg_i->SetTitle("Imaginary Component of FFT");
    fftg_i->Draw();
    c->cd(11);
    fftg_m->SetTitle("Magnitude of FFT");
    fftg_m->Draw();

    ffth_r = h->FFT(ffth_r, "RE");
    ffth_i = h->FFT(ffth_i, "IM");
    ffth_m = h->FFT(ffth_m, "MAG");
    c->cd(4);
    ffth_r->SetTitle("Real Component of FFT");
    ffth_r->Draw();
    c->cd(7);
    ffth_i->SetTitle("Imaginary Component of FFT");
    ffth_i->Draw();
    c->cd(10);
    ffth_m->SetTitle("Magnitude of FFT");
    ffth_m->Draw();


    Double_t *re_g = new Double_t[fftg_r->GetNbinsX()];
    Double_t *im_g = new Double_t[fftg_i->GetNbinsX()];
    for (i = 0; i < fftg_r->GetNbinsX(); i++) {re_g[i] = fftg_r->GetBinContent(i+1);}
    for (i = 0; i < fftg_i->GetNbinsX(); i++) {im_g[i] = fftg_i->GetBinContent(i+1);}

    Double_t *re_h = new Double_t[ffth_r->GetNbinsX()];
    Double_t *im_h = new Double_t[ffth_i->GetNbinsX()];
    Double_t *mag_h = new Double_t[ffth_m->GetNbinsX()];
    Double_t max_mag_h = 0; 
    for (i = 0; i < ffth_r->GetNbinsX(); i++) {re_h[i] = ffth_r->GetBinContent(i+1);}
    for (i = 0; i < ffth_i->GetNbinsX(); i++) {im_h[i] = ffth_i->GetBinContent(i+1);}
    for (i = 0; i < ffth_m->GetNbinsX(); i++) {mag_h[i] = ffth_m->GetBinContent(i+1);}
    for (i = 0; i < ffth_m->GetNbinsX(); i++) {
      if (max_mag_h < ffth_m->GetBinContent(i+1)) max_mag_h = ffth_m->GetBinContent(i+1);
    }

    Double_t *re_f = new Double_t[nbins-1];
    Double_t *im_f = new Double_t[nbins-1];
    Double_t *mag_f = new Double_t[nbins-1];

    // Perform (de)convolution with noise supression.
    //
    // Noise supression: any bin with 1/cut percent of peak magnitude is set to zero.
    // any bins in between supressed bins are also supressed

    double cut       = 150;
    int    minCutBin = nbins;

    for (i = 0; i < nbins-1; i++)
      if (mag_h[i] > max_mag_h/cut) 
	if (reconv)
	  re_f[i] = re_g[i]*re_h[i]-im_g[i]*im_h[i]; 
	else 
	  re_f[i] = (re_g[i]*re_h[i] + im_g[i]*im_h[i])
	    /(re_g[i]*re_g[i] + im_g[i]*im_g[i]);
      else {
	if (i < minCutBin) minCutBin = i;
	re_f[i] = 0;
      }


    for (i = 0; i < nbins-1; i++)
      if (mag_h[i] > max_mag_h/cut)
	if (reconv)
	  im_f[i] = re_g[i]*im_h[i]+re_h[i]*im_g[i];
	else
	  im_f[i] = (im_h[i]-im_g[i])
	    /(re_g[i]*re_g[i] + im_g[i]*im_g[i]);
      else {
	if (i < minCutBin) minCutBin = i;
	im_f[i] = 0;
      }

    for (i = minCutBin+1; i < nbins-minCutBin-1; i++) {
      im_f[i] = 0;
      re_f[i] = 0;
    }

    for (i = 0; i < nbins-1; i++) mag_f[i] = re_f[i]*re_f[i] + im_f[i]*im_f[i];

    TH1* fftf_r = (TH1*)fftg_r->Clone("fftf_r");
    for (i = 0; i < fftf_r->GetNbinsX(); i++) {fftf_r->SetBinContent(i+1, re_f[i]);}
    c->cd(6);
    fftf_r->Draw();
    TH1* fftf_i = (TH1*)fftg_i->Clone("fftf_i");
    for (i = 0; i < fftf_i->GetNbinsX(); i++) {fftf_i->SetBinContent(i+1, im_f[i]);}
    c->cd(9);
    fftf_i->Draw();
    TH1* fftf_m = (TH1*)fftg_m->Clone("fftf_m");
    for (i = 0; i < fftf_m->GetNbinsX(); i++) {fftf_m->SetBinContent(i+1, mag_f[i]);}
    c->cd(12);
    fftf_m->Draw();

    TVirtualFFT* fft_back = TVirtualFFT::FFT(1, &N, "C2R M K");
    fft_back->SetPointsComplex(re_f,im_f);
    fft_back->Transform();
    TH1 *hb = 0;
    hb = TH1::TransformHisto(fft_back,hb,"Re");
    hb->SetTitle("Deconvoluted Result");

    TH1* deconv = (TH1*)h->Clone("deconv");
    deconv->SetTitle("Deconvoluted Result");
    double maxVal = 0;
    for (i = 0; i < deconv->GetNbinsX(); i++) {
      
      if ((i+(nbins/2+1))%nbins==0) deconv->SetBinContent(i+1, hb->GetBinContent((i+(nbins/2+2))%nbins));
      else deconv->SetBinContent(i+1, hb->GetBinContent((i+(nbins/2+1))%nbins));
      

      if (deconv->GetBinContent(i+1) > maxVal)
	maxVal = deconv->GetBinContent(i+1);
    }

    deconv->SetMaximum(maxVal);

    c->cd(3);
    deconv->Scale(h->Integral()/deconv->Integral());

    double error;
    for (int i = 0; i <= deconv->GetNbinsX(); i++) {
      error = isnan(TMath::Sqrt(deconv->GetBinContent(i+1))) ? 1 : TMath::Sqrt(deconv->GetBinContent(i+1)); 
      deconv->SetBinError(i+1, error);
    }

    fitTH1DoubleGaussian(deconv, sameSigma);
    //deconv->GetXaxis()->SetTitleSize(0.1);
    deconv->Draw();
    TPaveText* tpt;
    if (sameSigma) tpt = GausLegend(deconv);
    else           tpt = DoubGausLegend(deconv);
    tpt->Draw();    
    //TPaveText* tpth = DoubGausLegend(h);
    //tpth->Draw();
    c->Print(IOUtility::join(IOUtility::join(IOUtility::join(IOUtility::getResolutionOutputDir(runN), systName), deconvolutionSubDir), name, kFALSE));
    c->Divide(1,1);
    c->cd();
    deconv->Draw();
    tpt->Draw();
    c->Print(IOUtility::join(IOUtility::join(IOUtility::join(IOUtility::getResolutionOutputDir(runN), systName), deconvolutionSubDir), name, kFALSE));
    delete tpt;
    for (int i = 1; i <= h->GetNbinsX(); i++) {
      if (i==h->GetNbinsX() && deconv->GetBinContent(i-1)>0)
	h->SetBinContent(i+1,deconv->GetBinContent(i-1));
      else if (deconv->GetBinContent(i) < 0)
	h->SetBinContent(i,0);
      else
	h->SetBinContent(i, deconv->GetBinContent(i));

      h->SetBinError(i, (isnan(TMath::Sqrt(h->GetBinContent(i))) || TMath::Sqrt(h->GetBinContent(i)) < 1) ? 1 : TMath::Sqrt(h->GetBinContent(i)));
    }
    fitTH1DoubleGaussian(h, sameSigma);
    delete fftg_r;
    delete fftg_i;
    delete fftg_m;
    delete ffth_r;
    delete ffth_i;
    delete ffth_m;
    delete fftf_r;
    delete fftf_i;
    delete fftf_m;
    delete c;
  } // end function: Deconvolute

  void ResolutionResult::resetDoubleGaussianPars(TH1* hist, Bool_t sameSigma) {
    double nominalAmplitude = hist->GetBinContent((int)(hist->GetNbinsX()/2));
    std::cout << "Nominal amplitude: " << nominalAmplitude << std::endl;
    std::cout << "Integral: " << hist->Integral() << std::endl;
    std::cout << "Std: " << hist->GetStdDev() << std::endl;
    std::cout << "Middle Bin: " << hist->GetBinContent((int)(hist->GetNbinsX()/2)) << std::endl;
    std::cout << "Bin Width: " << hist->GetBinWidth((int)(hist->GetNbinsX()/2)) << std::endl;

    doubGaus->SetParameter(0, nominalAmplitude*0.66);
    doubGaus->SetParLimits(0, 0, nominalAmplitude*10);
    doubGaus->SetParameter(1, 0);
    doubGaus->SetParameter(3, nominalAmplitude*0.33);
    doubGaus->SetParLimits(3, 0, nominalAmplitude*10);
    if (sameSigma) {
      doubGaus->SetParameter(2, hist->GetStdDev());
      doubGaus->SetParameter(4, hist->GetStdDev());
    }
    else {
      doubGaus->SetParameter(2, hist->GetStdDev()*0.62);
      doubGaus->SetParLimits(2, 25, hist->GetStdDev());
      doubGaus->SetParameter(4, hist->GetStdDev()*1.3);
      doubGaus->SetParLimits(4, hist->GetStdDev(), hist->GetStdDev()*5);
    }

    // tried coment here
    doubGaus->SetRange(-2.5*hist->GetStdDev(), 2.5*hist->GetStdDev());
    
    double min, max;

    std::cout << "Initial configuration:" << std::endl;
    doubGaus->GetParLimits(0, min, max);
    std::cout << "0: " << doubGaus->GetParameter(0) << " [ " << min << ", " << max << " ]" <<std::endl;
    doubGaus->GetParLimits(1, min, max);
    std::cout << "1: " << doubGaus->GetParameter(1) << " [ " << min << ", " << max << " ]" <<std::endl;
    doubGaus->GetParLimits(2, min, max);
    std::cout << "2: " << doubGaus->GetParameter(2) << " [ " << min << ", " << max << " ]" <<std::endl;
    doubGaus->GetParLimits(3, min, max);
    std::cout << "3: " << doubGaus->GetParameter(3) << " [ " << min << ", " << max << " ]" <<std::endl;
    doubGaus->GetParLimits(4, min, max);
    std::cout << "4: " << doubGaus->GetParameter(4) << " [ " << min << ", " << max << " ]" <<std::endl;


    singGaus->SetParameter(0, 150);
    singGaus->SetParameter(1, 0);
    singGaus->SetParLimits(1, -200, 200);
    singGaus->SetParameter(2, 75);
    singGaus->SetParLimits(2, 50, 500);
  }

  void ResolutionResult::fitTH1DoubleGaussian(TH1* hist, Bool_t sameSigma) {
    resetDoubleGaussianPars(hist, sameSigma);
    singGaus->SetParameter(0, 150);
    if (!sameSigma) hist->Fit("doubGaus", "R");
  }

  void ResolutionResult::calculateResVsRadius(TH1* fitResiduals, TH1* hitResiduals) {
    std::cout << "Calculate res vs radiu" << std::endl;
    for (int b = 1; b <= fitResiduals->GetNbinsX(); b++) {
      std::cout << "Bin: " << b << std::endl;
      Observable sigmaF = Observable(fitResiduals->GetBinContent(b),
				     fitResiduals->GetBinError  (b));
      Observable sigmaH = Observable(hitResiduals->GetBinContent(b),
				     hitResiduals->GetBinError  (b));
      std::cout << "   " << sigmaF.val << " " << sigmaH.val << " ";
      sigmaF *= sigmaH;
      Observable sigma = sigmaF.power(0.5);
      fitResiduals->SetBinContent(b, sigma.val);
      fitResiduals->SetBinError  (b, sigma.err);
      std::cout << sigma.val << std::endl;
    }
  }

  TH1* ResolutionResult::throwRandomResidualDist(TF1* resVsRadius, int nResiduals, Bool_t fitResiduals/*=kTRUE*/) {
    TH1D*  fakeRes  = new TH1D("fakeRes", "Bootstrapped Residuals", 500, -500, 500);
    double radius, width, residual;
    TString name;
    for (int i = 0; i < nResiduals; i++) {
      radius   = gRandom->Uniform(7.1);
      width    = resVsRadius->Eval(radius);
      residual = gRandom->Gaus(0, width);
      fakeRes->Fill(residual);
    }
    fakeRes->Fit("doubGaus");
    if (fitResiduals) name = "FakeFitResiduals.png";
    else name = "FakeHitResiduals.png";
    TCanvas* c = new TCanvas("c", "", 800, 800);
    TPaveText* tpt = DoubGausLegend(fakeRes);
    fakeRes->Draw();
    tpt->Draw();
    c->Print(IOUtility::join(IOUtility::join(IOUtility::join(IOUtility::getResolutionOutputDir(runN), systName), deconvolutionSubDir), name, kFALSE));
    delete c;
    return fakeRes;
  }
}

