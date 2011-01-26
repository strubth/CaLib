// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSLED                                                       //
//                                                                      //
// Calibration module for the TAPS LED.                                 //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTAPSLED.h"

ClassImp(TCCalibTAPSLED)


//______________________________________________________________________________
TCCalibTAPSLED::TCCalibTAPSLED(const Char_t* name, const Char_t* title, CalibData_t data,
                               Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Empty constructor.

    // init members
    fMainHisto2 = 0;
    fThr = 0;
    fLine = 0;
}

//______________________________________________________________________________
TCCalibTAPSLED::~TCCalibTAPSLED()
{
    // Destructor. 
    
    if (fMainHisto2) delete fMainHisto2;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibTAPSLED::Init()
{
    // Init the module.
    
    Char_t tmp[256];

    // init members
    fMainHisto2 = 0;
    fThr = 0;
    fLine = new TLine();

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);

    // get histogram name
    sprintf(tmp, "%s.Histo.Fit.Name", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig(tmp);
    
    // get normalization histogram name
    TString normHistoName;
    sprintf(tmp, "%s.Histo.Norm.Name", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else normHistoName = *TCReadConfig::GetReader()->GetConfig(tmp);
    
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
    
    // get the main normalization histogram
    fMainHisto2 = (TH2*) f.GetHistogram(normHistoName.Data());
    if (!fMainHisto2)
    {
        Error("Init", "Normalization histogram does not exist!\n");
        return;
    }
   
    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;LED threshold [MeV]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
 
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    sprintf(tmp, "%s.Histo.Fit", GetName());
    fCanvasFit->cd(1);
    TCUtils::FormatHistogram(fMainHisto, tmp);
    TCUtils::FormatHistogram(fMainHisto2, tmp);
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    sprintf(tmp, "%s.Histo.Overview", GetName());
    TCUtils::FormatHistogram(fOverviewHisto, tmp);
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibTAPSLED::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // get configuration
    Double_t threshold;
    sprintf(tmp, "%s.Threshold.Level", GetName());
    threshold = TCReadConfig::GetReader()->GetConfigDouble(tmp);
     
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    
    // create projection of the normalization histogram
    TH1* hNorm = (TH1D*) fMainHisto2->ProjectionX(tmp, elem+1, elem+1, "e");
    fFitHisto->Divide(hNorm);
    delete hNorm;

    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // determine threshold
        fThr =  FindThreshold(fFitHisto, threshold);

        // draw mean indicator line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        fLine->SetX1(fThr);
        fLine->SetX2(fThr);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    sprintf(tmp, "%s.Histo.Fit", GetName());
    fFitHisto->Draw("hist");
    
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
void TCCalibTAPSLED::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fMainHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fThr) fThr = fLine->GetX1();

        // calculate the new gain
        fNewVal[elem] = fThr;
    
        // update overview histogram
        fOverviewHisto->SetBinContent(elem + 1, fThr);
        fOverviewHisto->SetBinError(elem + 1, 0.000001);
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    "
           "old threshold: %12.8f    new threshold: %12.8f    diff: %6.2f %%",
           elem, fOldVal[elem], fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   

//______________________________________________________________________________
Double_t TCCalibTAPSLED::FindThreshold(TH1* h, Double_t level)
{
    // Find the position where the bin content of h changes from
    // < 'level' to > 'level'.

    Double_t pos;
    Double_t posUnder = 0;
    Double_t content;

    // loop over all bins
    for (Int_t i = 0; i < h->GetNbinsX(); i++)
    {
        pos = h->GetBinCenter(i);
        content = h->GetBinContent(i);
     
        if (content > level) return (pos + posUnder) / 2.;   
        else posUnder = pos;
    }
    
    // return 0 if position was not found
    return 0;
}

