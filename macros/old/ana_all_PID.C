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
TCanvas *cGrap = new TCanvas("cGrap", "Projection histo", 0, 500, 1200, 500);

TH1D *hpx;

//TH1F *hGr = new TH1F("hGr", ";Run/Date;IM_{#pi^{0}}", 1600, 0.5, 1600.5);
TH1F *hGr = new TH1F("hGr", ";Run/Date;", 3620, 14431.5, 18051.5);

TTimer * timer;

// mysql
TSQLResult *res;
TSQLRow    *row;
Char_t     szQuery[256];

// file
ifstream infile;

//-----------------------------------------
void GetPeak(Char_t* szFile, Double_t *val)
{
  Char_t szTitle[32];

  TFile *file0;
  file0 = TFile::Open(szFile);

  if( file0 )
    {   
      //      TH2F *h2d = (TH2F*) file0->Get("iCBthetaVsPID");
      TH2F *h2d = (TH2F*) file0->Get("PIDanglePhi2D");

      //      hpx = (TH1D*) h2d->ProjectionX();
      hpx = (TH1D*) h2d->ProjectionY("hpy", 0, 0);

      Int_t maxbin = hpx->GetMaximumBin();
      Double_t peack = hpx->GetBinCenter( maxbin );
      
      Double_t min =  peack - 10.;
      Double_t max =  peack + 10.;
      
      TF1 *f1 = new TF1( "f1", "pol1+gaus(2)", min, max );
      f1->SetParameters( 100., -1.5, hpx->GetMaximum(), peack, 1.);

      hpx->Fit(f1, "+R0Q", "");

      // ---- Draw ----
      cProj->cd();
      hpx->SetTitle(szFile);
      hpx->GetXaxis()->SetRangeUser(0, 300);
      hpx->Draw();

      f1->SetLineColor(2);
      f1->Draw("same");

      cProj->Update();

      val[0] = f1->GetParameter(3);
      val[1] = f1->GetParError(3);

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

  hGr->Draw("P");

  cGrap->Update();
  return;
}

void SaveAll()
{
  hGr->Print("hGr.eps");
  hGr->SaveAs("hGr.root");
  
  return;
}

//-----------------------------------------
void MySqlNext()
{
  Int_t    nn;
  UInt_t   utime;
  UInt_t   run;
  Char_t   szFile[64];
  Double_t val[2];

  row = res->Next();

  if( row )
    {
      sprintf( szFile,
	       // 	   "/usr/users/irakli/AcquRoot/ilib/out/CB_%s.dat_hCB2gIM_cut.root", 
	       //	       "/usr/users/irakli/AcquRoot/ilib/out/CB_%s_hCB2gIM_cut.root", 
	       //	       "/usr/tiger_scratch0/irakli/OUT/ARHistograms_CB_%s.root", 
	       //	       "/usr/leopard_scratch1/irakli/OUT/omega/ARHistograms_CB_%s.root",
	       "/usr/leopard_scratch3/irakli/OUT/histos/CB_%s.root",
	       //	       "/kernph/leopard_scratch1/irakli/OUT/omega_iter0/ARHistograms_CB_%s.root",
	       row->GetField(0) );

      run   = atol( row->GetField(0) );
      utime = atol( row->GetField(1) );
      
      GetPeak( szFile, val );
      printf("file: %s\n", szFile);

      if( !val[0] ) return;

      static Char_t szX[5];

//       sprintf(szX, "%5i", run);
//       hGr->Fill( szX, val[0] );

      hGr->Fill( run, val[0] );


      //      hGr->SetBinError(  hGr->FindBin(run), val[1] );

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
void FileNext()
{
//   Int_t    nn;
//   UInt_t   utime;
//   UInt_t   run;
//   Char_t   szFile[64];
//   Double_t val[2];
//   TString  strLine[256];
 
//   while( infile.good() )
//     {
//       infile >> szLine;

//       strLine.ReadLine(infile);

//       if( szLine )
// 	{
// 	  sprintf( szFile,
// 		   // 	   "/usr/users/irakli/AcquRoot/ilib/out/CB_%s.dat_hCB2gIM_cut.root", 
// 		   //	       "/usr/users/irakli/AcquRoot/ilib/out/CB_%s_hCB2gIM_cut.root", 
// 		   //	       "/usr/tiger_scratch0/irakli/OUT/ARHistograms_CB_%s.root", 
// 		   "/usr/leopard_scratch1/irakli/OUT/omega/ARHistograms_CB_%s.root",
// 		   //		   "/usr/leopard_scratch3/irakli/OUT/histos/CB_%s.root",
// 		   row->GetField(0) );

// 	  run   = atol( row->GetField(0) );
// 	  utime = atol( row->GetField(1) );
      
// 	  GetPeak( szFile, val );
// 	  printf("file: %s\n", szFile);

// 	  if( !val[0] ) return;

//  	  static Char_t szX[5];

// // 	  sprintf(szX, "%5i", run);
// // 	  hGr->Fill( szX, val[0] );

// 	  hGr->Fill( run, val[0] );
	  
// 	  //      hGr->SetBinError(  hGr->FindBin(run), val[1] );

// 	  cout << val[0] << "  " << val[1] << endl;

// 	  DrawGraph();
// 	}
//       else
// 	{
// 	  timer->Stop();
// 	  SaveAll();
// 	} 
//     }

  return;
}

//-----------------------------------------
void ana_all_PID()
{
  // DataBase
  TSQLServer *dbh = TSQLServer::Connect("mysql://phys-bonanza/acqu", "read", "readonly");
  //  dbh->SelectDataBase("acqu");
 
  sprintf(szQuery, 
	  "SELECT run, UNIX_TIMESTAMP(date) FROM Nb_run_main "
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
  timer = new TTimer( 1 ); //

  timer->SetCommand("MySqlNext()");
  timer->Start();
  
  //-------------------------------------
  //  hGr->Print("hGr.eps");
  hGr->GetYaxis()->SetLabelOffset(0.02);
  //   gr->GetXaxis()->SetTimeDisplay(1);
  //   gr->GetXaxis()->SetTimeFormat("#splitline{%y/%m/%d}{%H:%M}%F1970-01-01 00:00:00");
  hGr->SetMarkerStyle(20);
  //  hGr->GetYaxis()->SetRangeUser(130, 140);

  return;
}


//-----------------------------------------
void ana_all_PID(char szfile)
{
  infile.open( szfile );
  if( !infile.is_open() )
    {
      cerr << "ERROR: opening file - " << szfile << endl;
      return; 
    }
  //-------------------------------------
  timer = new TTimer( 1 ); //

  timer->SetCommand("FileNext()");
  timer->Start();
  
  //-------------------------------------
  //  hGr->Print("hGr.eps");
  hGr->GetYaxis()->SetLabelOffset(0.02);
  //   gr->GetXaxis()->SetTimeDisplay(1);
  //   gr->GetXaxis()->SetTimeFormat("#splitline{%y/%m/%d}{%H:%M}%F1970-01-01 00:00:00");
  hGr->SetMarkerStyle(20);
  hGr->GetYaxis()->SetRangeUser(130, 140);

  infile.close();
  return;
}

