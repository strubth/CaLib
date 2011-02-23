// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// CBEnergy.C                                                           //
//                                                                      //
// Check calibration of runsets.                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


TCanvas* gCFit;
TH1* gHOverview;
TH1* gH;
TH2* gH2;
TFile* gFile;
TF1* gFitFunc;
TLine* gLine;


//______________________________________________________________________________
void Fit(Int_t run)
{
    // Perform fit.
    
    Char_t tmp[256];

    // delete old function
    if (gFitFunc) delete gFitFunc;
    sprintf(tmp, "fEnergy_%i", run);
    gFitFunc = new TF1(tmp, "pol1+gaus(2)");
    gFitFunc->SetLineColor(2);
    
    // estimate peak position
    Double_t fPi0Pos = gH->GetBinCenter(gH->GetMaximumBin());
    if (fPi0Pos < 100 || fPi0Pos > 160) fPi0Pos = 135;

    // estimate background
    Double_t bgPar0, bgPar1;
    TCUtils::FindBackground(gH, fPi0Pos, 50, 50, &bgPar0, &bgPar1);
    
    // configure fitting function
    gFitFunc->SetRange(fPi0Pos - 20, fPi0Pos + 20);
    gFitFunc->SetLineColor(2);
    gFitFunc->SetParameters( 3.8e+2, -1.90, 150, fPi0Pos, 8.9);
    gFitFunc->SetParLimits(4, 3, 40);  
    Int_t fitres = gH->Fit(gFitFunc, "RB0Q");
    
    // get position
    fPi0Pos = gFitFunc->GetParameter(3);

    // check failed fits
    if (fitres) 
    {
        printf("Run %d: fit failed\n", run);
        return;
    }

    // indicator line
    gLine->SetX1(fPi0Pos);
    gLine->SetX2(fPi0Pos);
    gLine->SetY1(0);
    gLine->SetY2(gH->GetMaximum());

    // draw 
    gCFit->cd();
    gH->GetXaxis()->SetRangeUser(0, 200);
    gH->Draw();
    gFitFunc->Draw("same");
    gLine->Draw("same");

    // fill overview histogram
    gHOverview->SetBinContent(run+1, fPi0Pos);
    gHOverview->SetBinError(run+1, 0.0001);
}

//______________________________________________________________________________
void CBEnergy()
{
    // Main method.
    
    Char_t tmp[256];
    
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // general configuration
    CalibData_t data = kCALIB_CB_E1;
    const Char_t* hName = "CaLib_CB_IM_Neut";

    // configuration (December 2007)
    const Char_t calibration[] = "LD2_Dec_07";

    // configuration (February 2009)
    //const Char_t calibration[] = "LD2_Feb_09";
    
    // configuration (May 2009)
    //const Char_t calibration[] = "LD2_May_09";
    
    // get number of sets
    Int_t nSets = TCMySQLManager::GetManager()->GetNsets(data, calibration);

    // create canvas
    Int_t n = TMath::Sqrt(nSets);
    TCanvas* cOverview = new TCanvas();
    cOverview->Divide(n, nSets / n);

    // loop over sets
    for (Int_t i = 0; i < nSets; i++)
    { 
        // create file manager
        TCFileManager m(data, calibration, 1, &i);
        
        // get histo
        TH1* h = m.GetHistogram(hName);
    
        // draw histo
        cOverview->cd(i+1);
        h->Draw();
    }
    
    TFile* fout = new TFile("runset_overview.root", "recreate");
    cOverview->Write();
    delete fout;
}

