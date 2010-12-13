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

TCanvas *cProj = new TCanvas("cProj", "Projection histo", 0, 0, 500, 300);

TCanvas *cGrap = new TCanvas("cGrap", "Projection histo", 0, 500, 1200, 500);

TH1D *hpx;

Double_t max_bin = 1000.5;
//TH1F *hGr = new TH1F( "hGr", ";Run/Date;IM_{#pi^{0}}", Int_t(max_bin), 0.5, max_bin );

TH1F *hGr = new TH1F("hGr", ";Run/Date;IM_{#pi^{0}}", 3620, 20003.5, 20655.5);

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
      TH2F *h2d = (TH2F*) file0->Get("MyTimeTAPSVSTAPS2D");
      //      TH2F *h2d = (TH2F*) file0->Get("MyCBTAPSIMHistoAll");
      
      //      hpx = (TH1D*) h2d->ProjectionX();
      hpx = (TH1D*) h2d->ProjectionY();
      // hpx = (TH1D*) h2d->ProjectionY("hpx", 1, 1);
      //         hpx->Rebin(4);

      Int_t maxbin = hpx->GetMaximumBin();
      Double_t peack = hpx->GetBinCenter( maxbin );
      
      Double_t min =  peack - 3.;
      Double_t max =  peack + 3.;
      
      
      TF1 *f1 = new TF1( "f1", "pol1+gaus(2)", min, max );
      f1->SetParameters( 1., 1., hpx->GetMaximum(), peack, 0.5);

      hpx->Fit(f1, "+R0Q", "");

      // ---- Draw ----
      cProj->cd(1);
      h2d->Draw("colz");

      cProj->cd(2);

      hpx->SetTitle(szFile);
      hpx->GetXaxis()->SetRangeUser(-3, 3);

      hpx->Draw();

      f1->SetLineColor(2);
      f1->Draw("same");

      cProj->Update();

      // for fit
      val[0] = f1->GetParameter(3); // mean 
      //      val[1] = f1->GetParError(3); // error mean
      val[1] = f1->GetParameter(4); // sigma

      // for 1 element
      //       val[0] = hpx->GetMean(); 
      //       val[1] = hpx->GetRMS();

      file0->Close();  
    }
  else
    {
      val[0] = 0.0;
      val[1] = 0.0;
    }

  return;
}

//-----------------------------------------
void DrawGraph()
{
  // ---- Draw ----
  cGrap->cd();

  //  hGr->Draw("P");
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

Int_t i=1;

//-----------------------------------------
void Next( int run, UInt_t utime=0 )
{
  Char_t   szFile[64];
  Double_t val[2];

  if( run )
    {
      sprintf( szFile,
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

      DrawGraph();
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
void ana_C_all_TAPS_Time()
{
  cProj->Divide(2, 1, 0.001, 0.001);

  // DataBase
  dbh = TSQLServer::Connect("mysql://phys-bonanza/acqu", "read", "readonly");
 
  sprintf(szQuery, 
	  "SELECT run, UNIX_TIMESTAMP(date) "
	  "FROM C_run_main "
          "WHERE status LIKE \'%%OK%%\' " 
          "AND event > 1000 "
          "ORDER BY run ;");
  
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
  DrawGraph( -1, 1 );

  //-------------------------------------
  gROOT->LoadMacro("macros/GetCBsets.C");
  DrawSets( -1, 1 );

  return;
}

