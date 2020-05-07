/**************************************************************
   Class to hold an RT function read from file
***************************************************************/
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <TGraph.h>
#include <TH1F.h>
#include <TMath.h>

const int MAXPTS = 100;   //max allowed points in RT function
enum  RTtype{MDT,sMDT};

class RtFunction {
public:
  char    name[255];      //name of RT file
  char    header[255];    //header line of RT file
  char    key[255];       //key for plots (use RT file name if not supplied)
  Int_t   npts;           //number of pts in RT func
  Float_t radius[MAXPTS]; //tube radius (mm) 
  Float_t time[MAXPTS];   //drift time (ns)
  Float_t resol[MAXPTS];  //resolution (ns)
  RTtype  rttype;         //RT type; MDT or sMDT
  TGraph *RTgr;           //RT function graph      
  TGraph *RTdergr;        //RT derivative graph  
  TGraph *Resolgr;        //Resolution function graph
  RtFunction(const char *fname, const char *fkey="" );
  Float_t GetRadius( Float_t t );  //compute radius [mm] from time
  Float_t GetTime( Float_t r );    //compute time [ns] from radius 
  Float_t GetResolution( Float_t r ); //compute resolution [um] from radius 

};

/**************************************************************
   Read RT function from file
    fname = file with RT data
    fkey  = key for plots (use RT file name if not supplied)
***************************************************************/

RtFunction::RtFunction( const char *fname, const char *fkey ) {
  Int_t   i;
  Float_t r,t,res,dtdr[MAXPTS],t2[MAXPTS];

  // Save key
  if( fkey!=NULL && fkey[0] ) for( i=0; i<255; i++ ) key[i]=fkey[i];
  else                        for( i=0; i<255; i++ ) key[i]=fname[i];
  
  // Open input file 
  ifstream fin(fname);
  if( fin.is_open() == 0 )  {
    printf("ERROR: cannot open RT file %s\n",fname);
    npts=0;
    return;
  }

  //  Skip first line which is header info
  //  Use getline to get entire line, using >> only gives first item on line
  fin.getline(header,255);
  // If gasmon RT there is no header line beginning with 'v'
  // If so reset to start of file
  if( header[0] != 'v' ) {
    //fin.seekg (0, ios::beg);
    fin.close();
    fin.open(fname);
  }
  for(npts=0; npts<MAXPTS; npts++) {
    fin >> r >> t >> res;
    if( fin.eof() ) break;
    radius[npts] = r;
    time[npts]   = t;
    resol[npts]  = res*1000.;    //convert mm to microns
    if( npts ) {
      Float_t dt = t - time[npts-1];
      t2[npts-1] = (t + time[npts-1])/2.;
      if( dt > 0. ) dtdr[npts-1] = 1000.*(r - radius[npts-1])/dt;
      else          dtdr[npts-1] = 0;
    }
  }
  rttype = MDT;
  if( radius[npts-1] < 10. ) rttype = sMDT;

  cout << rttype << endl;

  RTgr = new TGraph( npts, time, radius );
  if( rttype == MDT) RTgr->SetLineColor(2);
  RTgr->SetLineWidth((Width_t)2.0);
  RTgr->SetMarkerSize(0.5);
  RTgr->SetMarkerColor(2);
  RTgr->SetMarkerStyle(21);
  RTdergr = new TGraph( npts-1, t2, dtdr );
  if( rttype == MDT) RTdergr->SetLineColor(2);
  RTdergr->SetLineWidth((Width_t)2.0);
  RTdergr->SetMarkerSize(0.5);
  RTdergr->SetMarkerColor(2);
  RTdergr->SetMarkerStyle(21);
  Resolgr = new TGraph( npts, radius, resol );
  if( rttype == MDT) Resolgr->SetLineColor(2);
  Resolgr->SetLineWidth((Width_t)2.0);
  Resolgr->SetMarkerSize(0.5);
  Resolgr->SetMarkerColor(2);
  Resolgr->SetMarkerStyle(21);
  //  remove the ".dat" from the filename and find last '/'
  int j=0;
  for( i=0; fname[i] != '\0' ;i++ ) {
    if( fname[i] == '/' ) j=i+1;
  }
  sprintf(name,"%s",&fname[j]);
  name[i-4] = '\0';
  //  printf("New RT function %i pts name %s\n",npts,name);
}


/**************************************************************
  Compute radius from time
***************************************************************/
Float_t RtFunction::GetRadius( Float_t t ) {
  Int_t i;
  if( t < 0. ) t = 0.;
  for( i=1; i<npts; i++ ) if( time[i] > t ) break;
  if(i == npts ) i--;

  if( time[i]-time[i-1] == 0. ) {
    printf("ERROR RT->GetRadius t=%lf i=%i time[i]=%lf RT=%s\n",
	   t,i,time[i],name);
    return radius[npts-1];
  }
  //  printf("i=%i r[%i]=%.2f t=%.2f t[%i]=%2f\n",i,i-1,radius[i-1],t,i-1,time[i-1]);

  return radius[i-1] + (t-time[i-1])*(radius[i]-radius[i-1])/(time[i]-time[i-1]);
}


/**************************************************************
  Compute time from radius 
***************************************************************/
Float_t RtFunction::GetTime( Float_t r ) {
  Int_t i;
  if( r < 0. ) r = 0.;
// find bin for radius.   radius[i-1] < r < radius[i]
// but r could be larger than radius[npts-1] in which case i = npts-1 
  for( i=1; i<npts; i++ ) {
    if( radius[i] > r ) break;
  } 
  if(i == npts ) i--;

  if( radius[i]-radius[i-1] == 0. ) {
    printf("ERROR RT->GetTime r=%lf i=%i radius[i]=%lf RT=%s\n",
	   r,i,radius[i],name);
    return time[npts-1];
  }

  return time[i-1] + (r-radius[i-1])*(time[i]-time[i-1])/(radius[i]-radius[i-1]);
}


/**************************************************************
  Compute resolution from the radius.  Resolution return in microns
***************************************************************/
Float_t RtFunction::GetResolution( Float_t r ) {
  Int_t i;
  if( r < 0. ) r = 0.;
// find bin for radius.   radius[i-1] < r < radius[i]
// but r could be larger than radius[npts-1] in which case i = npts-1 
  for( i=1; i<npts; i++ ) if( radius[i] > r ) break;
 
  if( radius[i]-radius[i-1] == 0. ) {
    printf("ERROR RT->GetTime r=%lf i=%i radius[i]=%lf RT=%s\n",
	   r,i,radius[i],name);
    return resol[npts-1];
  }
  return resol[i-1] + (r-radius[i-1])*(resol[i]-resol[i-1])/(radius[i]-radius[i-1]);
}


/**************************************************************
   Class to hold the difference between 2 RT functions rt1-rt2
***************************************************************/
class RtDifference {
public:
  RtFunction *rt1, *rt2;//2 RT functions being compared
  Int_t   npts;         //number of pts in RT func
  //  Float_t max_radius;   //max radius for RT difference
  Float_t dradius[MAXPTS]; //difference in tube radius vs drift time(micron) 
  Float_t dtime[MAXPTS];   //difference in drift time vs radius (ns) 
  Int_t   irt;          //RTdifference number (bookkeeping-used in histo names)
  Float_t maxdr;        //maximum radius difference
  Float_t mindr;        //minimum radius difference
  Float_t maxdt;        //maximum time difference
  Float_t mindt;        //minmum time difference
  TGraph *RTdgr;        //graph of dradius vs drift time        
  TGraph *RTdtgr;       //graph of dtime vs radius
  TGraph *RTdrrgr;      //graph of dradius vs radius
  TH1F   *RTdrh;        //TH1F of differences dradius  
  TH1F   *RTdth;        //TH1F of differences dtime  
  RtDifference( RtFunction *rtt1, RtFunction *rtt2, Int_t irt );
  int Filldth(float lo, float hi);    //fill RTdth
  int Filldrh(float lo, float hi);    //fill RTdrh
};


/**************************************************************
   Calculate difference between RT functions 
   Difference is given in microns
   rt1, rt2 => 2 RT functions to take the difference of
   irt => Number of RT difference (bookkeeping)
***************************************************************/
RtDifference::RtDifference( RtFunction *rtt1, RtFunction *rtt2, Int_t irtt ) {
  irt = irtt;
  if( rtt1->radius[rtt1->npts-1] < rtt2->radius[rtt2->npts-1] ) {
    rt1 = rtt1;
    rt2 = rtt2;
  } else {
    rt1 = rtt2;
    rt2 = rtt1;
  }
  npts = rt1->npts;
  if( rt1->npts == 0 || rt2->npts == 0 ) {
    npts = 0;
    return;
  } 
  // Loop over time points of RT1
  maxdr = 0.;
  mindr = 0.;
  maxdt = 0.;
  mindr = 0.;
  for( int i=0; i<rt1->npts; i++ ) {
    dradius[i] = 1000.*(rt1->radius[i] - rt2->GetRadius(rt1->time[i]));
    if( dradius[i] > maxdr ) maxdr = dradius[i];
    if( dradius[i] < mindr ) mindr = dradius[i];
    dtime[i] = rt1->time[i] - rt2->GetTime(rt1->radius[i]); 
    if( dtime[i] > maxdt ) maxdt = dtime[i];
    if( dtime[i] < mindt ) mindt = dtime[i];
    /* printf("%i t1=%.2f t2=%.2f dt=%.2f r1=%.4f r2=%.4f dr=%.1f\n",i, */
    /* 	   rt1->time[i],rt2->GetTime(rt1->radius[i]),dtime[i], */
    /* 	   rt1->radius[i],rt2->GetRadius(rt1->time[i]),dradius[i]); */
  }

  RTdgr = new TGraph( rt1->npts, rt1->time, dradius );
  if( rt1->rttype == MDT) RTdgr->SetLineColor(2);
  RTdgr->SetLineWidth((Width_t)2.0);
  RTdtgr = new TGraph( rt1->npts, rt1->radius, dtime );
  if( rt1->rttype == MDT) RTdtgr->SetLineColor(2);
  RTdtgr->SetLineWidth((Width_t)2.0);
  RTdrrgr = new TGraph( rt1->npts, rt1->radius, dradius );
  if( rt1->rttype == MDT) RTdrrgr->SetLineColor(2);
  RTdrrgr->SetLineWidth((Width_t)2.0);
}  //end RtDifference::RtDifference


/**************************************************************
  Filldth (do as separate function so can set histogram limits)
**************************************************************/
int RtDifference::Filldth( float lo, float hi) {
  RTdth = new TH1F(Form("RTdth%i",irt),
		   Form("#Deltat %s-%s",rt1->key,rt2->key),
		   200,lo,hi);
  RTdth->GetXaxis()->SetTitle("#Deltat [ns]");  

//New histogram method - take even steps in R
  Float_t dr = rt1->radius[(rt1->npts)-1]/1000.;      
  Float_t r = 0.;
// Loop over 1000 radius pts and find RT difference
  for( int n=0; n<1000; n++ ) {
    Float_t t1 = rt1->GetTime(r);
    Float_t t2 = rt2->GetTime(r);
    //    printf("n=%3i r=%.3f t1=%.3f t1=%.3f dt=%.3f\n",n,r,t1,t2,t2-t1);
    RTdth->Fill(t2 - t1);
    r += dr;
  }
  return 0;
}  //end RtDifference::Filldth


/**************************************************************
  Filldrh (do as separate function so can set limits)
**************************************************************/
int RtDifference::Filldrh( float lo, float hi) {
  RTdrh = new TH1F(Form("RTdrh%i",irt),
		   Form("#DeltaR %s-%s",rt1->key,rt2->key),
		   200,lo,hi);
  RTdrh->GetXaxis()->SetTitle("#DeltaR [#mum]");  

//New histogram method - take even steps in R
  Float_t dr = rt1->radius[(rt1->npts)-1]/1000.;      
  Float_t r = 0.;
// Loop over 1000 radius pts and find RT difference
  for( int n=0; n<1000; n++ ) {
    Float_t t1 = rt1->GetTime(r);
    Float_t diff = 1000*(rt2->GetRadius(t1) - r);
    RTdrh->Fill(diff);
    r += dr;
  }
  return 0;
}  //end RtDifference::Filldrh
