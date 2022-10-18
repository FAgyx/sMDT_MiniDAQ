#include <stdio.h>

#include "TFile.h"
#include "TString.h"
#include "TDirectory.h"
#include "TROOT.h"
#include "TKey.h"
#include "TSystem.h"
#include "TTree.h"
#include "TVectorD.h"

#include "MuonReco/IOUtility.h"
#include "MuonReco/Geometry.h"
#include "MuonReco/ArgParser.h"
#include "MuonReco/ConfigParser.h"

using namespace MuonReco;
// Author: Rene Brun
// retrieved from: ROOT tutorials copyFiles.C
// with slight modification
void CopyDir(TDirectory *source, TString overrideName = "") {
  //copy all objects and subdirs of directory source as a subdir of the current directory   
  source->ls();
  TDirectory *savdir = gDirectory;
  TDirectory *adir;
  if (!overrideName.CompareTo(""))
    adir = savdir->mkdir(source->GetName());
  else
    adir = savdir->mkdir(overrideName);
  adir->cd();
  //loop on all entries of this directory
  TKey *key;
  TIter nextkey(source->GetListOfKeys());
  while ((key = (TKey*)nextkey())) {
    key->Print();
    const char *classname = key->GetClassName();
    TClass *cl = gROOT->GetClass(classname);
    if (!cl) continue;
    if (cl->InheritsFrom("TDirectory")) {
      source->cd(key->GetName());
      TDirectory *subdir = gDirectory;
      adir->cd();
      CopyDir(subdir);
      adir->cd();
    } else if (cl->InheritsFrom("TTree")) {
      TTree *T = (TTree*)source->Get(key->GetName());
      adir->cd();
      TTree *newT = T->CloneTree();
      newT->Write();
    } else {
      source->cd();
      TObject *obj = key->ReadObj();
      adir->cd();
      obj->Write();
      delete obj;
    }
  }
  adir->SaveSelf(kTRUE);
  savdir->cd();
}


int main(int argc, char* argv[]) {
  
  if (argc < 5) {
    std::cout << "Run this script with the required options:" << std::endl << 
      "--conf: configuration file" << std::endl <<
      "--name: human readable name of output file" << std::endl;
      
    return 1;
  }

  // set up
  ArgParser    ap  = ArgParser(argc, argv);
  ConfigParser cp  = ConfigParser(ap.getTString("--conf"));
  int runN         = cp.items("General").getInt("RunNumber");
  TString filename = ap.getTString("--name");
  
  Geometry geo = Geometry();
  geo.Configure(cp.items("Geometry"));

  TFile* f_db = new TFile(IOUtility::getDBFile(runN, filename), "RECREATE");

  //
  // get raw distributions from decodeRawData step
  //

  TFile* f_rawdataout = TFile::Open(IOUtility::getDecodedOutputFilePath(runN));
  TDirectory* rawDists = f_db->mkdir("RawDistributions");

  for (int tdc_id = 0; tdc_id != Geometry::MAX_TDC-1; tdc_id++) {
    TString dirname = TString::Format("TDC_%02d_of_%02d_Time_Spectrum", tdc_id, Geometry::MAX_TDC);
    TDirectory* dir = f_rawdataout->GetDirectory(dirname);
    rawDists->cd();
    CopyDir(dir);
  }
  f_rawdataout->Close();

  //
  // get T0 Fit results
  //
  TDirectoryFile* f_t0 = TFile::Open(IOUtility::getT0FilePath(runN));  
  f_db->cd();
  CopyDir(f_t0, "T0Fits");
  f_t0->Close();

  //
  // get fit residuals
  //
  TFile* f_fitRes = TFile::Open(IOUtility::getFitResidualPath(runN));
  f_db->cd();
  CopyDir(f_fitRes, "BiasedResiduals");
  f_fitRes->Close();

  //
  // get hit residuals
  //
  TFile* f_hitRes = TFile::Open(IOUtility::getHitResidualPath(runN));
  f_db->cd();
  CopyDir(f_hitRes, "UnbiasedResiduals");
  f_hitRes->Close();


  //
  // get resolution curve
  //
  TFile* f_res = TFile::Open(IOUtility::join(IOUtility::getResolutionOutputDir(runN), "resolutionHist.root"));
  f_db->cd();
  CopyDir(f_res, "Resolution");
  f_res->Close();

  //
  // get tube efficiency
  //
  TFile* f_tubeEff = TFile::Open(IOUtility::join(IOUtility::getEfficiencyOutputDir(runN), "tubeEfficiency.root"));
  f_db->cd();
  CopyDir(f_tubeEff, "Efficiency");  
  f_tubeEff->Close();


  // 
  // get total efficiency
  //
  TFile* f_eff = TFile::Open(IOUtility::join(IOUtility::getEfficiencyOutputDir(runN), "totalEfficiency.root"));
  TVectorD *v = (TVectorD*)(f_eff->Get("Observable"));
  f_db->cd();
  v->Write("totalEfficiency");
  f_eff->Close();
  
  //
  // get single hit resolution
  //
  TFile* f_shr = TFile::Open(IOUtility::getResolutionObservable(runN, "Nominal"));
  v = (TVectorD*)(f_shr->Get("Observable"));
  f_db->cd();
  v->Write("singleHitResolution");
  f_shr->Close();

  // clean up 
  f_db->Write();
  return 0;
}
