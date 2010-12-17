/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibTAPSTime                                                       //
//                                                                      //
// Calibration module for the TAPS time.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "iCalibTAPSTime.h"

ClassImp(iCalibTAPSTime)


//______________________________________________________________________________
iCalibTAPSTime::iCalibTAPSTime()
    : iCalib("TAPS.Time", "TAPS time calibration", kCALIB_TAPS_T0, 
              iReadConfig::GetReader()->GetConfigInt("TAPS.Elements"))
{
    // Empty constructor.

    // init members
    fTimeGain = new Double_t[fNelem];
    fMean = 0;
    fLine = 0;
}

//______________________________________________________________________________
iCalibTAPSTime::~iCalibTAPSTime()
{
    // Destructor. 
    
    if (fTimeGain) delete [] fTimeGain;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void iCalibTAPSTime::Init()
{
    // Init the module.
    
    // init members
    fMean = 0;
    fLine = new TLine();

    // get histogram name
    if (!iReadConfig::GetReader()->GetConfig("TAPS.Time.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *iReadConfig::GetReader()->GetConfig("TAPS.Time.Histo.Fit.Name");
    
    // get time gain for TAPS TDCs
    iMySQLManager::GetManager()->ReadParameters(fSet, kCALIB_TAPS_T1, fTimeGain, fNelem);

    // read old parameters
    iMySQLManager::GetManager()->ReadParameters(fSet, fData, fOldVal, fNelem);

    // sum up all files contained in this runset
    iFileManager f(fSet, fData);
    
    // get the main calibration histogram
    fMainHisto = f.GetHistogram(fHistoName.Data());
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;Time_{TAPS-TAPS} [ns]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
    
    // get parameters from configuration file
    Double_t low = iReadConfig::GetReader()->GetConfigDouble("TAPS.Time.Histo.Overview.Yaxis.Min");
    Double_t upp = iReadConfig::GetReader()->GetConfigDouble("TAPS.Time.Histo.Overview.Yaxis.Max");
    fFitHistoXmin = iReadConfig::GetReader()->GetConfigDouble("TAPS.Time.Histo.Fit.Xaxis.Min");
    fFitHistoXmax = iReadConfig::GetReader()->GetConfigDouble("TAPS.Time.Histo.Fit.Xaxis.Max");

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
void iCalibTAPSTime::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1);
    
    // init variables
    Double_t factor = 2.0;
    Double_t peakval = 0;
    
    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fTime_%i", elem);
        fFitFunc = new TF1(tmp, "gaus");
        fFitFunc->SetLineColor(2);
    
        // estimate peak position
        peakval = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());

        // temporary
        fMean = peakval;

        // first iteration
        fFitFunc->SetRange(peakval - 0.8, peakval + 0.8);
        fFitFunc->SetParameters(fFitHisto->GetMaximum(), peakval, 0.5);
        fFitHisto->Fit(fFitFunc, "+R0Q");

        // second iteration
        peakval = fFitFunc->GetParameter(1);
        Double_t sigma = fFitFunc->GetParameter(2);
        fFitFunc->SetRange(peakval -factor*sigma, peakval +factor*sigma);
        fFitHisto->Fit(fFitFunc, "+R0Q");

        // final results
        fMean = fFitFunc->GetParameter(1); // store peak value

        // draw mean indicator line
        fLine->SetVertical();
        fLine->SetLineColor(4);
        fLine->SetLineWidth(3);
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        fLine->SetX1(fMean);
        fLine->SetX2(fMean);
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
void iCalibTAPSTime::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fMean) fMean = fLine->GetX1();

        // calculate the new offset
        fNewVal[elem] = fOldVal[elem] + fMean / fTimeGain[elem];
    
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
    printf("Element: %03d    Peak: %12.8f    Gain: %12.8f    "
           "old offset: %12.8f    new offset: %12.8f",
           elem, fMean, fTimeGain[elem], fOldVal[elem], fNewVal[elem]);
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   

