// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// MakeRunSets.C                                                        //
//                                                                      //
// Make run sets depending on the stability in time of a calibration.   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


TH1* gHOverview;
TF1* gFitFunc;
TLine* gLine;


//______________________________________________________________________________
void Fit(TH1* h, Int_t run)
{
    // Perform fit.
    
    Char_t tmp[256];

    // delete old function
    if (gFitFunc) delete gFitFunc;
    sprintf(tmp, "fEnergy_%i", run);
    gFitFunc = new TF1(tmp, "pol1+gaus(2)");
    gFitFunc->SetLineColor(2);
    
    // estimate peak position
    Double_t fPi0Pos = h->GetBinCenter(h->GetMaximumBin());
    if (fPi0Pos < 100 || fPi0Pos > 160) fPi0Pos = 135;

    // estimate background
    Double_t bgPar0, bgPar1;
    TCUtils::FindBackground(h, fPi0Pos, 50, 50, &bgPar0, &bgPar1);
    
    // configure fitting function
    gFitFunc->SetRange(fPi0Pos - 50, fPi0Pos + 50);
    gFitFunc->SetLineColor(2);
    gFitFunc->SetParameters( 3.8e+2, -1.90, 150, fPi0Pos, 8.9);
    gFitFunc->SetParLimits(4, 3, 40);  
    Int_t fitres = h->Fit(gFitFunc, "RB0Q");
    
    // check failed fits
    if (fitres) 
    {
        printf("fit failed\n");
        return;
    }

    // indicator line
    gLine->SetX1(fPi0Pos);
    gLine->SetX2(fPi0Pos);
    gLine->SetY1(0);
    gLine->SetY2(h->GetMaximum());

    // draw 
    h->GetXaxis()->SetRangeUser(0, 200);
    h->Draw();
    gFitFunc->Draw("same");
    gLine->Draw("same");

    // final results
    fPi0Pos = gFitFunc->GetParameter(3); 

    // fill overview histogram
    gHOverview->SetBinContent(run+1, fPi0Pos);
    gHOverview->SetBinError(run+1, 0.0001);
}

//______________________________________________________________________________
void MakeRunSets()
{
    // Main method.
    
    Char_t tmp[256];
    
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // configuration (December 2007)
    Int_t first_run = 13089;
    Int_t last_run = 13841;
    const Char_t* hName = "CaLib_CB_IM_Neut";
    const Char_t* fLoc = "/usr/panther_scratch0/werthm/A2/Dec_07/AR/out";

    // configuration (February 2009)
    //Int_t first_run = 21082;
    //Int_t last_run = 22625;
    //const Char_t* hName = "CaLib_CB_IM_Neut";
    //const Char_t* fLoc = "/usr/panther_scratch0/werthm/A2/Feb_09/AR/out";
    
    // configuration (May 2009)
    //Int_t first_run = 22626;
    //Int_t last_run = 23728;
    //const Char_t* hName = "CaLib_CB_IM_Neut";
    //const Char_t* fLoc = "/usr/panther_scratch0/werthm/A2/May_09/AR/out";

    // create histogram
    gHOverview = new TH1F("Overview", "Overview", 40000, 0, 40000);
    TCanvas* cOverview = new TCanvas();
    gHOverview->GetXaxis()->SetRangeUser(first_run-10, last_run+10);
    gHOverview->GetYaxis()->SetRangeUser(110, 160);
    gHOverview->Draw("E1");

    // create line
    gLine = new TLine();
    gLine->SetLineColor(kBlue);
    gLine->SetLineWidth(2);


    // init fitting function
    gFitFunc = 0;

    // create fitting canvas
    TCanvas* cFit = new TCanvas();

    // loop over runs
    for (Int_t i = first_run; i <= last_run; i++)
    {
        // load ROOT file
        sprintf(tmp, "%s/ARHistograms_CB_%d.root", fLoc, i);
        TFile* f = new TFile(tmp);

        // check file
        if (!f) continue;
        if (f->IsZombie()) continue;

        // load histogram
        TH2* h2 = (TH2*) f->Get(hName);
        if (!h2) continue;
        if (!h2->GetEntries()) continue;

        // project histogram
        sprintf(tmp, "Proj_%d", i);
        TH1* h = h2->ProjectionX(tmp);

        // fit the histogram
        Fit(h, i);
        
        // update canvases and sleep
        //cOverview->Update();
        //cFit->Update();
        //gSystem->Sleep(100);
    }

    TFile* fout = new TFile("runset_overview.root", "recreate");
    cOverview->Write();
    delete fout;
}

