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

TTimer * timer;

// mysql
TSQLServer *dbh;
TSQLResult *res;
TSQLRow    *row;
Char_t     szQuery[256];

// file
ifstream infile;

//-----------------------------------------
void GetMean(Char_t* szFile, Double_t *val)
{
  Char_t szTitle[32];

  TFile *file0;
  file0 = TFile::Open(szFile);

  if( file0 )
    {   
      TH2F *h2d = (TH2F*) file0->Get("CalibTAPS2g_Time");
      
      hpx = (TH1D*) h2d->ProjectionX();
      // hpx = (TH1D*) h2d->ProjectionX("hpx", 2, 2);
      //      hpx = (TH1D*) h2d->ProjectionX("hpx", 63, 63);
      hpx->Rebin( 8 );

      Int_t maxbin = hpx->GetMaximumBin();
      Double_t peack = hpx->GetBinCenter( maxbin );
      
      Double_t min =  peack - hpx->GetRMS();
      Double_t max =  peack + hpx->GetRMS();
      
      //      TF1 *f1 = new TF1( "f1", "pol1+gaus(2)", min, max );
      //      f1->SetParameters( 1., 1., hpx->GetMaximum(), peack, 1.);

      TF1 *f1 = new TF1( "f1", "gaus", min, max );
      f1->SetParameters( hpx->GetMaximum(), peack, 1.);

      hpx->Fit(f1, "+R0Q", "");

      // ---- Draw ----
      cProj->cd();
      hpx->SetTitle(szFile);
      hpx->GetXaxis()->SetRangeUser(-20, 20);
      hpx->Draw();

      f1->SetLineColor(2);
      f1->Draw("same");

      cProj->Update();

      val[0] = f1->GetParameter(1);
      val[1] = f1->GetParameter(2);

      //       val[0] = f1->GetParameter(3);
      //       //      val[1] = hpx->GetRMS();
      //       //      val[1] = f1->GetParError(3);
      //       val[1] = f1->GetParameter(4);

      file0->Close();  
    }
  else
    {
      val[0] = 0.0;
      val[1] = 0.0;
    }

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

      GetMean( szFile, val );
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

      cGrap->Modified(1);
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
void ana_C_all_TAPS()
{
  // DataBase
  dbh = TSQLServer::Connect("mysql://phys-bonanza/acqu", "read", "readonly");
 
  sprintf(szQuery, 
	  "SELECT run, UNIX_TIMESTAMP(date) FROM C_run_main "
          "WHERE status = \"OK\" " 
          "AND event > 40000 "
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
  DrawGraph( -20, 20 );

  //-------------------------------------
  gROOT->LoadMacro("macros/GetCBsets.C");
  DrawSets( -20, 20 );
  
  return;
}


