// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSEnergySG                                                  //
//                                                                      //
// Calibration module for the TAPS SG energy.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTAPSEnergySG.h"

ClassImp(TCCalibTAPSEnergySG)


//______________________________________________________________________________
TCCalibTAPSEnergySG::TCCalibTAPSEnergySG()
    : TCCalib("TAPS.Energy.SG", "TAPS SG energy calibration", kCALIB_TAPS_SG_E1,
              TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements"))
{
    // Empty constructor.

    // init members
    fMean = 0;
    fFileManager = 0;
    fLine = 0;
}

//______________________________________________________________________________
TCCalibTAPSEnergySG::~TCCalibTAPSEnergySG()
{
    // Destructor. 
    
    if (fFileManager) delete fFileManager;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibTAPSEnergySG::Init()
{
    // Init the module.
    
    // init members
    fMean = 0;
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fLine = new TLine();

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("TAPS.Energy.SG.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("TAPS.Energy.SG.Histo.Fit.Name");
    
    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(fData, fCalibration.Data(), fSet[0], fOldVal, fNelem);
    
    // copy to new parameters
    for (Int_t i = 0; i < fNelem; i++) fNewVal[i] = fOldVal[i];
    
    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;photon PSA angle [deg]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
 
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();

    // draw the overview histogram
    fCanvasResult->cd();
    TCUtils::FormatHistogram(fOverviewHisto, "TAPS.Energy.SG.Histo.Overview");
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibTAPSEnergySG::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // get configuration
    Double_t lowLimit, highLimit;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("TAPS.Energy.SG.Photon.Energy.Range", &lowLimit, &highLimit);
     
    // create histogram name
    sprintf(tmp, "%s_%03d", fHistoName.Data(), elem);
   
    // delete old histogram
    if (fMainHisto) delete fMainHisto;
  
    // get histogram
    fMainHisto = fFileManager->GetHistogram(tmp);
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // draw main histogram
    fCanvasFit->cd(1);
    TCUtils::FormatHistogram(fMainHisto, "TAPS.Energy.SG.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
    
    // init variables
    Double_t factor = 2.0;
    Double_t peakval = 0;
  
    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {
        // create energy projection
        sprintf(tmp, "PSAProj_%d", elem);
        TH2* h2 = (TH2*) fMainHisto;
        if (fFitHisto) delete fFitHisto;
        Int_t binMin = h2->GetYaxis()->FindBin(lowLimit);
        Int_t binMax = h2->GetYaxis()->FindBin(highLimit);
        fFitHisto = (TH1D*) h2->ProjectionX(tmp, binMin, binMax, "e");
        
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fPSA_%i", elem);
        fFitFunc = new TF1(tmp, "gaus");
        fFitFunc->SetLineColor(2);

	// get important parameter positions
	Double_t maxPos = fFitHisto->GetXaxis()->GetBinCenter(fFitHisto->GetMaximumBin());
	Double_t max = fFitHisto->GetBinContent(fFitHisto->GetMaximumBin());

	// configure fitting function
	fFitFunc->SetParameters(max, maxPos, 4);
	fFitFunc->SetParLimits(0, max - 20, max + 20); // height of the gaus
	fFitFunc->SetParLimits(1, maxPos - 2, maxPos + 2); // peak position of the gaus
	fFitFunc->SetParLimits(2, 0.1, 5);                  // sigma of the gaus

	// estimate peak position
        peakval = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());

        // temporary
        fMean = peakval;

        // first iteration
        fFitFunc->SetRange(peakval - 0.8, peakval + 0.8);
        fFitFunc->SetParameters(fFitHisto->GetMaximum(), peakval, 0.5);
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
    TCUtils::FormatHistogram(fFitHisto, "TAPS.Energy.SG.Histo.Fit");
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
void TCCalibTAPSEnergySG::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fMainHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fMean) fMean = fLine->GetX1();

        // calculate the new gain
        fNewVal[elem] = fOldVal[elem] / TMath::Tan(fMean*TMath::DegToRad());
    
        // update overview histogram
        fOverviewHisto->SetBinContent(elem + 1, fMean);
        fOverviewHisto->SetBinError(elem + 1, 0.000001);
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    Peak: %12.8f    "
           "old gain: %12.8f    new gain: %12.8f    diff: %6.2f %%",
           elem, fMean, fOldVal[elem], fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   

