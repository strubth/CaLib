// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibCBTime                                                        //
//                                                                      //
// Calibration module for the CB time.                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibCBTime.h"

ClassImp(TCCalibCBTime)


//______________________________________________________________________________
TCCalibCBTime::TCCalibCBTime()
    : TCCalib("CB.Time", "CB time calibration", kCALIB_CB_T0, TCConfig::kMaxCB)
{
    // Empty constructor.

    // init members
    fTimeGain = 0.11771;
    fMean = 0;
    fLine = 0;
}

//______________________________________________________________________________
TCCalibCBTime::~TCCalibCBTime()
{
    // Destructor. 
    
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibCBTime::Init()
{
    // Init the module.
    
    // init members
    fMean = 0;
    fLine = new TLine();
    
    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
 
    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("CB.Time.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("CB.Time.Histo.Fit.Name");
    
    // get time gain for CB TDCs
    if (!TCReadConfig::GetReader()->GetConfig("CB.Time.TDCGain")) fTimeGain = 0.11771;
    else fTimeGain = TCReadConfig::GetReader()->GetConfigDouble("CB.Time.TDCGain");

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
    fOverviewHisto = new TH1F("Overview", ";Element;Time_{CB-CB} [ns]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
    
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
    TCUtils::FormatHistogram(fMainHisto, "CB.Time.Histo.Fit");
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    TCUtils::FormatHistogram(fOverviewHisto, "CB.Time.Histo.Overview");
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibCBTime::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    
    // init variables
    Double_t factor = 2.5;
    Double_t peakval = 0;
    
    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fTime_%i", elem);

	// the fit function
	fFitFunc = new TF1("fFitFunc", "gaus(0)");
	fFitFunc->SetLineColor(2);
	
	// get important parameter positions
	Double_t maxPos = fFitHisto->GetXaxis()->GetBinCenter(fFitHisto->GetMaximumBin());
	Double_t max = fFitHisto->GetBinContent(fFitHisto->GetMaximumBin());

	// configure fitting function
	fFitFunc->SetParameters(max, maxPos, 8);
	fFitFunc->SetParLimits(0, max - 100, max + 50);    // height of the gaus
	fFitFunc->SetParLimits(1, maxPos - 2, maxPos + 2); // peak position of the gaus
	fFitFunc->SetParLimits(2, 4, 16);                  // sigma of the gaus
    
        // estimate peak position
        peakval = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());

        // temporary
        fMean = peakval;

        // first iteration
	fFitFunc->SetRange(peakval - 3.8, peakval + 3.8);
	//fFitFunc->SetParameters(fFitHisto->GetMaximum(), peakval, 7);
	fFitHisto->Fit(fFitFunc, "RBQ0");

        // second iteration
	peakval = fFitFunc->GetParameter(1);
        Double_t sigma = fFitFunc->GetParameter(2);
        fFitFunc->SetRange(peakval -factor*sigma, peakval +factor*sigma);
        fFitHisto->Fit(fFitFunc, "RBQ0");

        // final results
        fMean = fFitFunc->GetParameter(1); // store peak value

        // draw mean indicator line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        fLine->SetX1(fMean);
        fLine->SetX2(fMean);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    TCUtils::FormatHistogram(fFitHisto, "CB.Time.Histo.Fit");
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
void TCCalibCBTime::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fMean) fMean = fLine->GetX1();

        // calculate the new offset
        fNewVal[elem] = fOldVal[elem] + fMean / fTimeGain;
    
        // update overview histogram
        fOverviewHisto->SetBinContent(elem + 1, fMean);
        fOverviewHisto->SetBinError(elem + 1, 0.0000001);
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    Peak: %12.8f    "
           "old offset: %12.8f    new offset: %12.8f",
           elem, fMean, fOldVal[elem], fNewVal[elem]);
    if (unchanged) printf("    -> unchanged");
    if (TCUtils::IsCBHole(elem)) printf(" (hole)");
    printf("\n");
}   

