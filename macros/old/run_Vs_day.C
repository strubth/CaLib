/*****************************************************
 *                                                   *
 *                                                   *
 *                                                   *
 *****************************************************/

// DataBase
TSQLServer *dbh = TSQLServer::Connect("mysql://phys-bonanza/acqu", "read", "readonly");


TCanvas *cProj = new TCanvas("cProj", "Projection histo", 0, 0, 500, 500);
TCanvas *cGrap = new TCanvas("cGrap", "Projection histo", 0, 500, 1200, 500);

TH1D *hpx;

TH1F *hGr = new TH1F("hGr", ";Date [month-day];Number of Runs", 40, 0, 40);

TTimer * timer;

// mysql
TSQLResult *res;
TSQLRow    *row;
Char_t     szQuery[256];

//-----------------------------------------
void DrawGraph()
{
  //  hGr->Print("hGr.eps");
  //  hGr->GetYaxis()->SetLabelOffset(0.02);
//   gr->GetXaxis()->SetTimeDisplay(1);
//   gr->GetXaxis()->SetTimeFormat("#splitline{%y/%m/%d}{%H:%M}%F1970-01-01 00:00:00");

  // ---- Draw ----
  cGrap->cd();

  hGr->SetMarkerStyle(20);
  hGr->Draw("P");
  //  hGr->Draw("E1");

  cGrap->Update();
  return;
}

void SaveAll()
{
  hGr->Print("hGr.eps");
  hGr->SaveAs("hGr.root");
  
  return;
}

//----------------------------------------
void run_Vs_day()
{

  UInt_t   utime;
  UInt_t   run;

  int month[] = {4, 5, 8};

  Char_t date[32];

  for( int i=0; i<3; i++)
    for( int d=1; d<32; d++ )
      {
	sprintf( szQuery, 
		 "SELECT SUM(event), date "
		 "FROM acqu.Nb_run_main "
		 "WHERE date LIKE '2008-%02i%%-%02i%%' AND status='OK' AND event>5000;",
		 month[i], d);
	
	cout << szQuery << endl;
	res = dbh->Query(szQuery);
	
	Int_t nRow   = res->GetRowCount();

	if( !nRow )
	  continue;

	sprintf( date, "%02i-%02i", month[i], d);
		
	hGr->Fill( date, nRow );
      } 
  
  dbh->Close();
  
  DrawGraph();
  
  return;
}
