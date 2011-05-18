// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// PIDEnergy.C                                                          //
//                                                                      //
// Make run sets depending on the stability in time of a calibration.   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


TCanvas* gCFit;
TH1* gHOverview;
TH1* gH;
TH3* gH3;
TFile* gFile;
TF1* gFitFunc;
TLine* gLine;
Double_t gMin;
Double_t gMax;


//______________________________________________________________________________
void Fit(Int_t run)
{
    // Perform fit.
    
    Char_t tmp[256];
    
    // delete old function
    if (gFitFunc) delete gFitFunc;

    // create fitting function
    if (gFitFunc) delete gFitFunc;
    sprintf(tmp, "fGauss_%d", run);
    gFitFunc = new TF1(tmp, "expo(0)+gaus(2)");
    gFitFunc->SetLineColor(2);
    
    // estimate peak position
    TSpectrum s;
    s.Search(gH, 10, "goff nobackground", 0.05);
    Double_t peak = TMath::MaxElement(s.GetNPeaks(), s.GetPositionX());
    
    // prepare fitting function
    gFitFunc->SetRange(gMin, gMax);
    gFitFunc->SetParameter(2, gH->GetXaxis()->FindBin(peak));
    gFitFunc->SetParLimits(2, 0, 100000);
    gFitFunc->SetParameter(3, peak);
    gFitFunc->SetParameter(4, 20);
    gFitFunc->SetParLimits(4, 18, 100);
    
    for (Int_t i = 0; i < 10; i++)
        if (!gH->Fit(gFitFunc, "RBQ0")) break;
    
    // get peak
    peak = gFitFunc->GetParameter(3);

    // indicator line
    gLine->SetX1(peak);
    gLine->SetX2(peak);
    gLine->SetY1(0);
    gLine->SetY2(gH->GetMaximum());

    // draw 
    gCFit->cd();
    gH->GetXaxis()->SetRangeUser(0, 2000);
    gH->Draw();
    gFitFunc->Draw("same");
    gLine->Draw("same");

    // fill overview histogram
    gHOverview->SetBinContent(run+1, peak);
    gHOverview->SetBinError(run+1, 0.0001);
}

//______________________________________________________________________________
void PIDEnergy()
{
    // Main method.
    
    Char_t tmp[256];
    
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // general configuration
    Bool_t watch = kTRUE;
    const Char_t* data = "Data.PID.E1";
    const Char_t* hName = "CaLib_PID_dE_E_013";
    Double_t yMin = 0;
    Double_t yMax = 2000;
    gMin = 500;
    gMax = 1100;

    // configuration (December 2007)
    //const Char_t calibration[] = "LD2_Dec_07";
    //const Char_t* fLoc = "/usr/puma_scratch0/werthm/A2/Dec_07/AR/out/droop";

    // configuration (February 2009)
    //const Char_t calibration[] = "LD2_Feb_09";
    //const Char_t* fLoc = "/usr/puma_scratch0/werthm/A2/Feb_09/AR/out/droop";
    
    // configuration (May 2009)
    const Char_t calibration[] = "LD2_May_09";
    const Char_t* fLoc = "/usr/puma_scratch0/werthm/A2/May_09/AR/out/droop";

    // create histogram
    gHOverview = new TH1F("Overview", "Overview", 40000, 0, 40000);
    TCanvas* cOverview = new TCanvas();
    gHOverview->GetYaxis()->SetRangeUser(yMin, yMax);
    gHOverview->Draw("E1");
    
    // create line
    gLine = new TLine();
    gLine->SetLineColor(kBlue);
    gLine->SetLineWidth(2);

    // init fitting function
    gFitFunc = 0;
    
    // create fitting canvas
    gCFit = new TCanvas();
    
    // get number of sets
    Int_t nSets = TCMySQLManager::GetManager()->GetNsets(data, calibration);
    
    // total number of runs
    Int_t nTotRuns = 0;

    // first and last runs
    Int_t first_run, last_run;

    // loop over sets
    for (Int_t i = 0; i < nSets; i++)
    {
        // get runs of set
        Int_t nRuns;
        Int_t* runs = TCMySQLManager::GetManager()->GetRunsOfSet(data, calibration, i, &nRuns);
    
        // loop over runs
        for (Int_t j = 0; j < nRuns; j++)
        {
            // save first and last runs
            if (i == 0 && j == 0) first_run = runs[j];
            if (i == nSets-1 && j == nRuns-1) last_run = runs[j];

            // clean-up
            if (gH) delete gH;
            if (gH3) delete gH3;
            if (gFile) delete gFile;
            gH = 0;
            gH3 = 0;
            gFile = 0;

            // load ROOT file
            sprintf(tmp, "%s/ARHistograms_CB_%d.root", fLoc, runs[j]);
            gFile = new TFile(tmp);

            // check file
            if (!gFile) continue;
            if (gFile->IsZombie()) continue;

            // load histogram
            gH3 = (TH3*) gFile->Get(hName);
            if (!gH3) continue;
            if (!gH3->GetEntries()) continue;
            if (gH3->GetEntries() < 5000) continue;

            // project histogram
            gH3->GetXaxis()->SetRangeUser(40, 60);
            sprintf(tmp, "Proj_%d_y", runs[j]);
            gH = (TH1D*) gH3->Project3D(tmp);
            gH->Rebin(3);

            // fit the histogram
            Fit(runs[j]);
            
            // update canvases and sleep
            if (watch)
            {
                cOverview->Update();
                gCFit->Update();
                //gSystem->Sleep(10);
            }
     
            // count run
            nTotRuns++;
        }

        // clean-up
        delete runs;

        // draw runset markers
        cOverview->cd();
        
        // get first run of set
        Int_t frun = TCMySQLManager::GetManager()->GetFirstRunOfSet(data, calibration, i);

        // draw line
        TLine* aLine = new TLine(frun, yMin, frun, yMax);
        aLine->SetLineColor(kBlue);
        aLine->SetLineWidth(2);
        aLine->Draw("same");
    }
    
    // adjust axis
    gHOverview->GetXaxis()->SetRangeUser(first_run-10, last_run+10);

    TFile* fout = new TFile("runset_overview.root", "recreate");
    cOverview->Write();
    delete fout;

    printf("%d runs analyzed.\n", nTotRuns);

    gSystem->Exit(0);
}

