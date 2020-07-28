int test_draw(){
	gStyle->SetOptStat(10); //only print entries
	gStyle->SetTitleX(999.);//hist no title
	gStyle->SetTitleY(999.);
	gStyle->SetStatY(0.9);                
	// Set y-position (fraction of pad size)
	gStyle->SetStatX(0.9);                
	// Set x-position (fraction of pad size)
	gStyle->SetStatW(0.25);                
	// Set width of stat-box (fraction of pad size)
	gStyle->SetStatH(0.25);                
	// Set height of stat-box (fraction of pad size)
	TH1D *p = new TH1D("c1","c1",10,0,10);
	p->Fill(1);
	p->Draw();
	return 0;
}