/*****************************************************
 *                                                   *
 *                                                   *
 *                                                   *
 *****************************************************/

#include <iostream>
#include <fstream>

#include <TCanvas.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TFile.h>
#include <TTimer.h>
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>

TCanvas *cProj = new TCanvas("cProj", "Projection histo", 0, 0, 500, 500);
TH1D *hpx;

Double_t max_bin = 1000.5;
//TH1F *hGr = new TH1F( "hGr", ";Run/Date;IM_{#pi^{0}}", Int_t(max_bin), 0.5, max_bin );

TTimer * timer;

// mysql
TSQLServer *dbh;
TSQLResult *res;
TSQLRow    *row;
Char_t     szQuery[256];

// file
ifstream infile;

//-----------------------------------------
void GetPi0Mass(Char_t* szFile, Double_t *val)
{
  Char_t szTitle[32];

  TFile *file0;
  file0 = TFile::Open(szFile);

  if( file0 )
    {   
      TH2F *h2d = (TH2F*) file0->Get("MyCBIMHistoAll");
      //      TH2F *h2d = (TH2F*) file0->Get("CalibCB_2g_Time");
      //      TH2F *h2d = (TH2F*) file0->Get("CalibCB_2g_IM");

      
      hpx = (TH1D*) h2d->ProjectionX();
      //      hpx = (TH1D*) h2d->ProjectionX("hpx", 222, 222);
//         hpx->Rebin(4);

      Int_t maxbin = hpx->GetMaximumBin();
      Double_t peack = hpx->GetBinCenter( maxbin );
      
      Double_t min =  peack - 30.;
      Double_t max =  peack + 30.;
      
      TF1 *f1 = new TF1( "f1", "pol1+gaus(2)", min, max );
      f1->SetParameters( 100., -1.5, hpx->GetMaximum(), peack, 7.);

      hpx->Fit(f1, "+R0Q", "");

      // ---- Draw ----
      cProj->cd();
      hpx->SetTitle(szFile);
      //      hpx->GetXaxis()->SetRangeUser(0, 300);
      hpx->GetXaxis()->SetRangeUser(-30, 30);
      hpx->Draw();

      f1->SetLineColor(2);
      f1->Draw("same");

      cProj->Update();

      val[0] = f1->GetParameter(3);
      val[1] = f1->GetParError(3);
//      val[1] = f1->GetParameter(4);

      file0->Close();  
    }
  else
    {
      val[0] = 0.0;
      val[1] = 0.0;
    }

  return;
}

void SaveAll()
{
  hGr->Print("hGr.gif");
  hGr->SaveAs("hGr.root");
  
  return;
}

Int_t i=1;

//-----------------------------------------
void Next( int run, UInt_t utime=0 )
{
  Char_t   szFile[64];
  Double_t val[2];

  if( run )
    {
      sprintf( szFile,
	       // 	   "/usr/users/irakli/AcquRoot/ilib/out/CB_%s.dat_hCB2gIM_cut.root", 
	       //	       "/usr/users/irakli/AcquRoot/ilib/out/CB_%s_hCB2gIM_cut.root", 
	       //	       "/usr/tiger_scratch0/irakli/OUT/ARHistograms_CB_%s.root", 
	       //	       "/usr/leopard_scratch1/irakli/OUT/omega/ARHistograms_CB_%s.root",
               "/usr/leopard_scratch3/irakli/OUT/histos/CB_%5i.root",
	       run );

      GetPi0Mass( szFile, val );
      printf("file: %s\n", szFile);

      if( !val[0] ) return;

      static Char_t szX[5];

      //       sprintf(szX, "%5i", run);
      //       hGr->Fill( szX, val[0] );

/*     hGr->Fill( i, val[0] );
     hGr->SetBinError( hGr->FindBin(i), val[1] );
     i++;
*/

//     if( val[0] < 160 && val[0] > 120 )
      hGr->Fill( run, val[0] );
      hGr->SetBinError(  hGr->FindBin(run), val[1] );

      cout << val[0] << "  " << val[1] << endl;

      //      DrawGraph();
    }
  else
    {
      timer->Stop();
      SaveAll();
    } 

  return;
}

//-----------------------------------------
void MySqlNext()
{
  UInt_t   utime;
  UInt_t   run;
  Char_t   szFile[64];

  row = res->Next();

  if( row )
    {
      run   = atol( row->GetField(0) );
      utime = atol( row->GetField(1) );
      
      Next( run, utime );
    }
  else
    {
      timer->Stop();
      SaveAll();
    } 

  return;
}

//-----------------------------------------
void FileNext()
{
  Int_t run;
 
  cout << "+++++++++++++++++++++++" << endl;

  while( infile.good() )
    {
      infile >> run;
      
      cout << run << endl;

      if( run )
	{
	  Next( run );
	}
      else
	{
	  timer->Stop();
	  SaveAll();
	  infile.close();
	} 
    }
  
  return;
}

//-----------------------------------------
void ana_C_all_CB()
{
  // DataBase
  dbh = TSQLServer::Connect("mysql://phys-bonanza/acqu", "read", "readonly");
 
  sprintf(szQuery, 
	  "SELECT run, UNIX_TIMESTAMP(date) FROM C_run_main "
          "WHERE status = \"%%OK%%\" " 
          "AND event > 1000 "
          "ORDER BY run ;");
  
  //           AND cb_calib = \"OK\" 
  //            AND date < \"2008-08-06 08:30\";");

  res = dbh->Query(szQuery);
  dbh->Close();
  
  Int_t nRow   = res->GetRowCount();
  Int_t nField = res->GetFieldCount();

  //-------------------------------------
  timer = new TTimer( 0.01 ); //

  timer->SetCommand("MySqlNext()");
  timer->Start();
  
  //-------------------------------------
  gROOT->LoadMacro("macros/CarbonGraph.C");

  int bot=120;
  int upp=150;

  DrawGraph( bot, upp );

  //-------------------------------------
  gROOT->LoadMacro("macros/GetCBsets.C");
  DrawSets( bot, upp );

  return;
}


//-----------------------------------------
void ana_C_all_CB(Char_t *szfile)
{
  infile.open( szfile );
  if( !infile.is_open() )
    {
      cerr << "ERROR: opening file - " << szfile << endl;
      return; 
    }
  //-------------------------------------
  timer = new TTimer( 0.1 ); //

  timer->SetCommand("FileNext()");
  timer->Start();
  
  return;
}

