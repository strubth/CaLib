// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTaggerTime                                                    //
//                                                                      //
// Calibration module for the Tagger time.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTaggerTime.h"

ClassImp(TCCalibTaggerTime)


//______________________________________________________________________________
TCCalibTaggerTime::TCCalibTaggerTime()
    : TCCalib("Tagger.Time", "Tagger time calibration", kCALIB_TAGG_T0, 
              TCReadConfig::GetReader()->GetConfigInt("Tagger.Elements"))
{
    // Empty constructor.

    // init members
    fTimeGain = 0.11771;
    fMean = 0;
    fLine = 0;
}

//______________________________________________________________________________
TCCalibTaggerTime::~TCCalibTaggerTime()
{
    // Destructor. 
    
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibTaggerTime::Init()
{
    // Init the module.
    
    // init members
    fMean = 0;
    fLine = new TLine();
    
    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
 
    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("Tagger.Time.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("Tagger.Time.Histo.Fit.Name");
    
    // read old parameters
    TCMySQLManager::GetManager()->ReadParameters(fSet, fData, fOldVal, fNelem);

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
    fOverviewHisto = new TH1F("Overview", ";Element;Time_{Tagger-TAPS} [ns]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
    
    // get parameters from configuration file
    Double_t low = TCReadConfig::GetReader()->GetConfigDouble("Tagger.Time.Histo.Overview.Yaxis.Min");
    Double_t upp = TCReadConfig::GetReader()->GetConfigDouble("Tagger.Time.Histo.Overview.Yaxis.Max");
    fFitHistoXmin = TCReadConfig::GetReader()->GetConfigDouble("Tagger.Time.Histo.Fit.Xaxis.Min");
    fFitHistoXmax = TCReadConfig::GetReader()->GetConfigDouble("Tagger.Time.Histo.Fit.Xaxis.Max");

    // ajust overview histogram
    if (low || upp) fOverviewHisto->GetYaxis()->SetRangeUser(low, upp);

    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
    fMainHisto->GetXaxis()->SetRangeUser(fFitHistoXmin, fFitHistoXmax);
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibTaggerTime::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    
    // init variables
    Double_t factor = 5.0;
    Double_t peakval = 0;
    
    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fTime_%i", elem);
        fFitFunc = new TF1(tmp, "pol1(0)+gaus(2)");
        fFitFunc->SetLineColor(2);
    
        // estimate peak position
        peakval = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());

        // temporary
        fMean = peakval;

        // first iteration
        fFitFunc->SetRange(peakval - 5, peakval + 5);
        fFitFunc->SetParameters(500, -1, fFitHisto->GetMaximum(), peakval, 0.5);
        fFitFunc->SetParLimits(4, 0.02, 2.); // sigma
        fFitHisto->Fit(fFitFunc, "RBQ0");
        
        // search the right peak
	if (fFitFunc->GetParameter(3) < peakval-0.3 || fFitFunc->GetParameter(3) > peakval+0.3)
	{
	  fFitFunc->SetParLimits(4, 0.0002, 0.6);
	  fFitHisto->Fit(fFitFunc, "RBQ0");
	}

        // second iteration
        peakval = fFitFunc->GetParameter(3);
        Double_t sigma = fFitFunc->GetParameter(4);
        fFitFunc->SetRange(peakval -factor*sigma, peakval +factor*sigma);
        fFitHisto->Fit(fFitFunc, "RBQ0");

        // final results
        fMean = fFitFunc->GetParameter(3); // store peak value

        // draw mean indicator line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        fLine->SetX1(fMean);
        fLine->SetX2(fMean);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    fFitHisto->GetXaxis()->SetRangeUser(fFitHistoXmin, fFitHistoXmax);
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
void TCCalibTaggerTime::Calculate(Int_t elem)
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
           "old offset: %12.8f    new offset: %12.8f",
           elem, fMean, fOldVal[elem], fNewVal[elem]);
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   

