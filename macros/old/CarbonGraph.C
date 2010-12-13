
iStyle->SetOptStat(0);

double start = 16101.5;
double stopp = 16728.5;
int    nbins = int(stopp-start);

TCanvas *cGrap = new TCanvas("cGrap", "Projection histo", 0, 500, 1200, 500);
TH1F *hGr = new TH1F("hGr", ";Run Number;", nbins, start, stopp );

//-----------------------------------------
void DrawGraph( int bot, int upp )
{
  // ---- Draw ----
  cGrap->cd();
  
  hGr->GetYaxis()->SetLabelOffset(0.02);
  hGr->SetMarkerStyle(20);
  hGr->SetMarkerColor(40);
  hGr->GetYaxis()->SetRangeUser( bot, upp);
  
  //  hGr->Draw("P");
  hGr->Draw("E1");
  
  
  cGrap->Update();
  return;
}
