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


//______________________________________________________________________________
void Fit(TH1* h, Double_t* outPos, Double_t* outFWHM)
{
    // Perform fit.
    
    Char_t tmp[256];

    // delete old function
    TF1* func = new TF1(tmp, "pol1+gaus(2)");
    func->SetLineColor(2);
    
    // estimate peak position
    Double_t fPi0Pos = h->GetBinCenter(h->GetMaximumBin());

    // configure fitting function
    func->SetRange(fPi0Pos - 0.8, fPi0Pos + 0.8);
    func->SetLineColor(2);
    func->SetParameters( 0.1, 0.1, h->GetMaximum(), 0, 0.1);
    Int_t fitres = h->Fit(func, "RB0Q");
    
    // get position and FWHM
    fPi0Pos = func->GetParameter(3);
    *outPos = fPi0Pos;
    *outFWHM = 2.35*func->GetParameter(4);

    // check failed fits
    if (fitres) 
    {
        printf("Run %d: fit failed\n", run);
        return;
    }

    // indicator line
    TLine* line = new TLine();
    line->SetX1(fPi0Pos);
    line->SetX2(fPi0Pos);
    line->SetY1(0);
    line->SetY2(h->GetMaximum());

    // draw 
    h->GetXaxis()->SetRangeUser(-3, 3);
    h->Draw();
    func->Draw("same");
    line->Draw("same");
}

//______________________________________________________________________________
void TAPSTime()
{
    // Main method.
    
    Char_t tmp[256];
    
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // general configuration
    CalibData_t data = kCALIB_CB_E1;
    const Char_t* hName = "CaLib_TAPS_Time_Neut";

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
    
    // create arrays
    Double_t* pos = new Double_t[nSets];
    Double_t* fwhm = new Double_t[nSets];

    // loop over sets
    for (Int_t i = 0; i < nSets; i++)
    { 
        // create file manager
        TCFileManager m(data, calibration, 1, &i);
        
        // get histo
        TH2* h2 = (TH2*) m.GetHistogram(hName);
        
        // project histo
        sprintf(tmp, "Proj_%d", i);
        TH1* h = (TH1*) h2->ProjectionX(tmp);
        
        // fit histo
        cOverview->cd(i+1);
        Fit(h, &pos[i], &fwhm[i]);
    }

    // show results
    for (Int_t i = 0; i < nSets; i++)
        printf("Set %02d:   Pos: %.2f    FWHM: %.2f\n", i, pos[i], fwhm[i]);
    
    TFile* fout = new TFile("runset_overview.root", "recreate");
    cOverview->Write();
    delete fout;
}

