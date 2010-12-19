/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSEnergy                                                    //
//                                                                      //
// Calibration module for the TAPS energy.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTAPSEnergy.h"

ClassImp(TCCalibTAPSEnergy)


//______________________________________________________________________________
TCCalibTAPSEnergy::TCCalibTAPSEnergy()
    : TCCalib("TAPS.Energy", "TAPS energy calibration", kCALIB_TAPS_LG_E1,
              TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements"))
{
    // Empty constructor.
    
    // init members
    fPi0IMOld = 0;
    fPi0IMNew = 0;
    fLine = 0;

}

//______________________________________________________________________________
TCCalibTAPSEnergy::~TCCalibTAPSEnergy()
{
    // Destructor. 
    
    if (fPi0IMOld) delete [] fPi0IMOld;
    if (fPi0IMNew) delete [] fPi0IMNew;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibTAPSEnergy::Init()
{
    // Init the module.
    
    // init members
    fPi0IMOld = new Double_t[fNelem];
    fPi0IMNew = new Double_t[fNelem];
    fLine = new TLine();

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("TAPS.Energy.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("TAPS.Energy.Histo.Fit.Name");
    
    // read old parameters
    TCMySQLManager::GetManager()->ReadParameters(fSet, fData, fOldVal, fNelem);
    //TCMySQLManager::GetManager()->ReadParameters(fSet, ECALIB_TAPS_PI0IM, fPi0IMOld, fNelem);

    // sum up all files contained in this runset
    TCFileManager f(fSet, fData);
    
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
    
    // get parameters from configuration file
    Double_t low = TCReadConfig::GetReader()->GetConfigDouble("TAPS.Energy.Histo.Overview.Yaxis.Min");
    Double_t upp = TCReadConfig::GetReader()->GetConfigDouble("TAPS.Energy.Histo.Overview.Yaxis.Max");
    fFitHistoXmin = TCReadConfig::GetReader()->GetConfigDouble("TAPS.Energy.Histo.Fit.Xaxis.Min");
    fFitHistoXmax = TCReadConfig::GetReader()->GetConfigDouble("TAPS.Energy.Histo.Fit.Xaxis.Max");
    
    // ajust overview histogram
    if (low || upp) fOverviewHisto->GetYaxis()->SetRangeUser(low, upp);

    // draw main histogram
    fCanvasFit->cd(1)->SetLogz();
    fMainHisto->GetXaxis()->SetRangeUser(fFitHistoXmin, fFitHistoXmax);
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibTAPSEnergy::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1);
    
    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fEnergy_%i", elem);
        fFitFunc = new TF1(tmp, "pol1+gaus(2)");
        fFitFunc->SetLineColor(2);
        
        // estimate peak position
        fPi0IMNew[elem] = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());
        if (fPi0IMNew[elem] < 100 || fPi0IMNew[elem] > 160) fPi0IMNew[elem] = 135;

        // estimate background
        Double_t bgPar0, bgPar1;
        TCUtils::FindBackground(fFitHisto, fPi0IMNew[elem], 50, 50, &bgPar0, &bgPar1);
        
        // configure fitting function
        fFitFunc->SetRange(fPi0IMNew[elem] - 70, fPi0IMNew[elem] + 50);
        fFitFunc->SetLineColor(2);
        fFitFunc->SetParameters( 3.8e+2, -1.90, 150, fPi0IMNew[elem], 8.9);
        fFitFunc->SetParLimits(4, 3, 20);  
        //fFitFunc->SetParameters(bgPar0, bgPar1, fFitHisto->GetMaximum(), fPi0IMNew[elem], 9.);
        fFitFunc->SetParLimits(4, 1., 200); //5,150 sigma
        fFitHisto->Fit(fFitFunc, "+R0Q");

        // final results
        fPi0IMNew[elem] = fFitFunc->GetParameter(3); 

        // draw mean indicator line
        fLine->SetVertical();
        fLine->SetLineColor(4);
        fLine->SetLineWidth(3);
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        
        // check if mass is in normal range
        if (fPi0IMNew[elem] < 80 || fPi0IMNew[elem] > 200) fPi0IMNew[elem] = 135;
        
        // set indicator line
        fLine->SetX1(fPi0IMNew[elem]);
        fLine->SetX2(fPi0IMNew[elem]);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    fFitHisto->GetXaxis()->SetRangeUser(fFitHistoXmin, fFitHistoXmax);
    fFitHisto->Draw();
    
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
void TCCalibTAPSEnergy::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fPi0IMNew[elem]) fPi0IMNew[elem] = fLine->GetX1();
        
        // set new pi0 position
        fPi0IMNew[elem] = fPi0IMNew[elem];

        // calculate the new offset
        fNewVal[elem] = fOldVal[elem] * (TCConfig::kPi0Mass / fPi0IMNew[elem]);
    
        // if new value is negative take old
        if (fNewVal[elem] < 0) 
        {
            fNewVal[elem] = fOldVal[elem];
            unchanged = kTRUE;
        }

        // update overview histogram
        fOverviewHisto->SetBinContent(elem+1, fPi0IMNew[elem]);
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
           "old gain: %12.8f    new gain: %12.8f",
           elem, fPi0IMNew[elem], fOldVal[elem], fNewVal[elem]);
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   

