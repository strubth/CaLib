// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibCBEnergy                                                      //
//                                                                      //
// Calibration module for the CB energy.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibCBEnergy.h"

ClassImp(TCCalibCBEnergy)


//______________________________________________________________________________
TCCalibCBEnergy::TCCalibCBEnergy()
    : TCCalib("CB.Energy", "CB energy calibration", kCALIB_CB_E1, TCConfig::kMaxCB)
{
    // Empty constructor.
    
    // init members
    fPi0Pos = 0;
    fLine = 0;

}

//______________________________________________________________________________
TCCalibCBEnergy::~TCCalibCBEnergy()
{
    // Destructor. 
    
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibCBEnergy::Init()
{
    // Init the module.
    
    // init members
    fPi0Pos = 0;
    fLine = new TLine();
    
    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
 
    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("CB.Energy.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("CB.Energy.Histo.Fit.Name");
    
    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(fData, fCalibration.Data(), fSet[0], fOldVal, fNelem);
    
    // copy to new parameters
    for (Int_t i = 0; i < fNelem; i++) fNewVal[i] = fOldVal[i];

    // sum up all files contained in this runset
    TCFileManager f(fData, fCalibration.Data(), fNset, fSet);
    
    // get the main calibration histogram
    fMainHisto = f.GetHistogram(fHistoName.Data());
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;2#gamma inv. mass [MeV]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
    
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
    TCUtils::FormatHistogram(fMainHisto, "CB.Energy.Histo.Fit");
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    TCUtils::FormatHistogram(fOverviewHisto, "CB.Energy.Histo.Overview");
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibCBEnergy::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    
    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    TCUtils::FormatHistogram(fFitHisto, "CB.Energy.Histo.Fit");
    fFitHisto->Draw("hist");
     
    // check for sufficient statistics
    if (fFitHisto->GetEntries() > 1000)
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fEnergy_%i", elem);
        fFitFunc = new TF1(tmp, "gaus(0)+pol3(3)");
        fFitFunc->SetLineColor(2);
        
        // estimate peak position
        fPi0Pos = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());
        if (fPi0Pos < 100 || fPi0Pos > 160) fPi0Pos = 135;

        // configure fitting function
        fFitFunc->SetRange(fPi0Pos - 50, fPi0Pos + 80);
        fFitFunc->SetLineColor(2);
        fFitFunc->SetParameters(fFitHisto->GetMaximum(), fPi0Pos, 8, 1, 1, 1, 0.1);
        fFitFunc->SetParLimits(1, 130, 140);  
        fFitFunc->SetParLimits(2, 3, 15);  
        
        // fit
        for (Int_t i = 0; i < 10; i++)
            if (!fFitHisto->Fit(fFitFunc, "RBQ0")) break;

        // final results
        fPi0Pos = fFitFunc->GetParameter(1); 

        // draw mean indicator line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        
        // check if mass is in normal range
        if (fPi0Pos < 80 || fPi0Pos > 200) fPi0Pos = 135;
        
        // set indicator line
        fLine->SetX1(fPi0Pos);
        fLine->SetX2(fPi0Pos);
   
        // draw fitting function
        if (fFitFunc) fFitFunc->Draw("same");
    
        // draw indicator line
        fLine->Draw();
    }
    
    // update canvas
    fCanvasFit->Update();

    // update overview
    if (elem % 20 == 0)
    {
        fCanvasResult->cd();
        fOverviewHisto->Draw("E1");
        fCanvasResult->Update();
    }   
}

//______________________________________________________________________________
void TCCalibCBEnergy::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries() > 1000)
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fPi0Pos) fPi0Pos = fLine->GetX1();
        
        // calculate the new offset
        fNewVal[elem] = fOldVal[elem] * TCConfig::kPi0Mass / fPi0Pos;
        
        // if new value is negative take old
        if (fNewVal[elem] < 0) 
        {
            fNewVal[elem] = fOldVal[elem];
            unchanged = kTRUE;
        }

        // update overview histogram
        fOverviewHisto->SetBinContent(elem+1, fPi0Pos);
        fOverviewHisto->SetBinError(elem+1, 0.0000001);
    
        // update average calculation
        fAvr += fPi0Pos;
        fAvrDiff += TMath::Abs(fPi0Pos - TCConfig::kPi0Mass);
        fNcalc++;
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    Pi0: %12.8f    "
           "old gain: %12.8f    new gain: %12.8f    diff: %6.2f %%",
           elem, fPi0Pos, fOldVal[elem], fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (unchanged) printf("    -> unchanged");
    if (TCUtils::IsCBHole(elem)) printf(" (hole)");
    printf("\n");

    // show average
    if (elem == fNelem-1)
    {
        fAvr /= (Double_t)fNcalc;
        fAvrDiff /= (Double_t)fNcalc;
        printf("Average pi0 position           : %.3f MeV\n", fAvr);
        printf("Average difference to pi0 mass : %.3f MeV\n", fAvrDiff);
    }
}   

