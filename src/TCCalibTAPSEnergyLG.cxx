// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSEnergyLG                                                  //
//                                                                      //
// Calibration module for the TAPS LG energy.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTAPSEnergyLG.h"

ClassImp(TCCalibTAPSEnergyLG)


//______________________________________________________________________________
TCCalibTAPSEnergyLG::TCCalibTAPSEnergyLG()
    : TCCalib("TAPS.Energy.LG", "TAPS LG energy calibration", kCALIB_TAPS_LG_E1,
              TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements"))
{
    // Empty constructor.
    
    // init members
    fPi0Pos = 0;
    fLine = 0;

}

//______________________________________________________________________________
TCCalibTAPSEnergyLG::~TCCalibTAPSEnergyLG()
{
    // Destructor. 
    
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibTAPSEnergyLG::Init()
{
    // Init the module.
    
    // init members
    fPi0Pos = 0;
    fLine = new TLine();
    
    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
 
    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("TAPS.Energy.LG.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("TAPS.Energy.LG.Histo.Fit.Name");
    
    // read old parameters
    TCMySQLManager::GetManager()->ReadParameters(fData, fCalibration.Data(), fSet, fOldVal, fNelem);
    
    // copy to new parameters
    for (Int_t i = 0; i < fNelem; i++) fNewVal[i] = fOldVal[i];

    // sum up all files contained in this runset
    TCFileManager f(fData, fCalibration.Data(), fSet);
    
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
    TCUtils::FormatHistogram(fMainHisto, "TAPS.Energy.LG.Histo.Fit");
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    TCUtils::FormatHistogram(fOverviewHisto, "TAPS.Energy.LG.Histo.Overview");
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibTAPSEnergyLG::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    
    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fEnergy_%i", elem);
	
	// the fit function
	fFitFunc = new TF1("fFitFunc", "gaus(0)+pol3(3)", 0, 1000);

	// get important parameter position
	Double_t maxPos = fFitHisto->GetXaxis()->GetBinCenter(fFitHisto->GetMaximumBin());
	Double_t max = fFitHisto->GetBinContent(fFitHisto->GetMaximumBin());

	// configure fitting function
	fFitFunc->SetParameters(max, maxPos, 15, 1, 1, 1, 0.1);
	fFitFunc->SetParLimits(0, 0, 1000);
	fFitFunc->SetParLimits(1, 125, 145);
	fFitFunc->SetParLimits(2, 5, 25);
	fFitFunc->SetRange(60, 180);
	fFitFunc->SetLineColor(2);
	fFitHisto->Fit(fFitFunc, "RBQ0");

        // estimate peak position
        fPi0Pos = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());
        if (fPi0Pos < 100 || fPi0Pos > 160) fPi0Pos = 135;

        // estimate background
        Double_t bgPar0, bgPar1;
        TCUtils::FindBackground(fFitHisto, fPi0Pos, 50, 50, &bgPar0, &bgPar1);
        	
        // draw mean indicator line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        
        // check if mass is in normal range
        if (fPi0Pos < 80 || fPi0Pos > 200) fPi0Pos = 135;

	// final results
        fPi0Pos = fFitFunc->GetParameter(1); 
        
        // set indicator line
        fLine->SetX1(fPi0Pos);
        fLine->SetX2(fPi0Pos);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    TCUtils::FormatHistogram(fFitHisto, "TAPS.Energy.LG.Histo.Fit");
    fFitHisto->Draw("hist");
    
    // draw fitting function
    if (fFitFunc) fFitFunc->Draw("same");
    
    // draw indicator line
    fLine->Draw();
    
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
void TCCalibTAPSEnergyLG::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fPi0Pos) fPi0Pos = fLine->GetX1();
        
        // calculate the new offset
        fNewVal[elem] = fOldVal[elem] * (TCConfig::kPi0Mass / fPi0Pos);
    
        // if new value is negative take old
        if (fNewVal[elem] < 0) 
        {
            fNewVal[elem] = fOldVal[elem];
            unchanged = kTRUE;
        }

        // update overview histogram
        fOverviewHisto->SetBinContent(elem+1, fPi0Pos);
        fOverviewHisto->SetBinError(elem+1, 0.0000001);
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
    printf("\n");
}   

