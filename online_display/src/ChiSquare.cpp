

#ifndef MUON_CHI_SQUARE
#define MUON_CHI_SQUARE

namespace Muon {
  int MAXHITS = 12;
  TF1* chi[12];
  Bool_t init = kFALSE;

  void InitChiSquare() {
    for (int dof = 1; dof <= MAXHITS-2; dof++) {
      cout << "set bin " << dof-1 << endl;
      chi[dof-1] = new TF1(TString::Format("chi_%i", dof), "[0]*TMath::Power(2, -[1]/2)/TMath::Gamma([1]/2)*TMath::Power(x, [1]/2\
-1)*TMath::Exp(-x/2)", 0, 100);
      chi[dof-1]->SetParameter(1, dof);
    }
    init = kTRUE;
  }

  void SetChiSquarePars(TH1* nHitsDist) {
    int dof;
    for (int nHits = 3; nHits <= 12; nHits++) {
      dof = nHits - 2;
      chi[dof-1]->SetParameter(0, nHitsDist->GetBinContent(nHits+1));
      cout << "Setting parameter: " << dof << " to " << nHitsDist->GetBinContent(nHits+1) << endl;
    }
  }

  double ChiSquareCalc(double *x, double* par) {
    double result = 0;
    int dof;
    for (int nHits = 3; nHits <= MAXHITS; nHits++) {
      dof = nHits - 2;
      result += chi[dof-1]->EvalPar(x, 0);
    }
    return result;
  }

  TF1* ChiSquareDist(TH1* nHitsDist) {
    if (!init) InitChiSquare();
    SetChiSquarePars(nHitsDist);
    TF1* chiSquareDist = new TF1("chiSquareDist", ChiSquareCalc, 0, 100, 0);
    return chiSquareDist;
  }
}

#endif
