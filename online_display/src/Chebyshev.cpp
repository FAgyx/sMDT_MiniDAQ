

namespace Muon {
  class Chebyshev {
  public:
    Chebyshev(int n, double xmin, double xmax) :
      fA(xmin), fB(xmin),
      fT(std::vector<double>(n)) {}

    double operator() (const double * xx, const double *p) {
      double x = (2.0 * xx[0] - fA - fB)/(fB-fA);
      int order = fT.size();
      if (order == 1) return p[0];
      if (order == 2) return p[0] + x*p[1];
      // build the polynomials
      fT[0] = 1;
      fT[1] = x;
      for (int i = 1; i < order; ++i) {
	fT[i+1] = 2*x*fT[i] - fT[i-1];
      }
      double sum = p[0]*fT[0];
      for (int i = 1; i <= order; ++i) {
	sum += p[i] * fT[i];
      }
      return sum;
    }
    
  private:
    double fA;
    double fB;
    vector<double> fT; // polynomial
    vector<double> fC; // coefficients
  };

  void exampleFitChebyshev() {
    TH1* h1 = new TH1D("h1", "h1", 100,-2,2);
    h1->FillRandom("gaus");
    /*
    double xmin = -2; double xmax = 2;
    double n=4;
    Chebyshev * cheb = new Chebyshev(n,xmin,xmax);
    TF1* f1 = new TF1("f1", cheb,xmin,xmax,n+1,"Chebyshev");
    for (int i = 0; i<=n;++i) f1->SetParameter(i,1);
    */
    h1->Fit("chebyshev5");
    //h1->Draw();
  }
}
