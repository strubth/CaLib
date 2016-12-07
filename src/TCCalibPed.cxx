/*************************************************************************
 * Author: Dominik Werthmueller, Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPed                                                           //
//                                                                      //
// Base pedestal calibration module class.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TH2.h"
#include "TF1.h"
#include "TCLine.h"
#include "TCanvas.h"
#include "TList.h"

#include "TCCalibPed.h"
#include "TCFileManager.h"
#include "TCReadARCalib.h"
#include "TCMySQLManager.h"
#include "TCUtils.h"

ClassImp(TCCalibPed)

//______________________________________________________________________________
TCCalibPed::TCCalibPed(const Char_t* name, const Char_t* title, const Char_t* data,
                       Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Constructor.

    // init members
    fADC = 0;
    fFileManager = 0;
    fMean = 0;
    fLine = 0;
}

//______________________________________________________________________________
TCCalibPed::~TCCalibPed()
{
    // Destructor.

    if (fADC) delete [] fADC;
    if (fFileManager) delete fFileManager;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibPed::Init()
{
    // Init the module.

    Char_t tmp[256];

    // init members
    fADC = new Int_t[fNelem];
    for (Int_t i = 0; i < fNelem; i++) fADC[i] = 0;
    fMean = 0;
    fLine = new TCLine();

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);

    // read ADC numbers
    ReadADC();

    // get histogram name
    sprintf(tmp, "%s.Histo.Fit.Name", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Warning("Init", "Histogram name was not found in configuration!");
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig(tmp);

    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(fData, fCalibration.Data(), fSet[0], fOldVal, fNelem);

    // copy to new parameters
    for (Int_t i = 0; i < fNelem; i++) fNewVal[i] = fOldVal[i];

    // sum up all files contained in this runset
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);

    // get the main calibration histogram
    if (fHistoName) fMainHisto = fFileManager->GetHistogram(fHistoName.Data());
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
    }

    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;Pedestal position [Channel]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);

    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    if (fMainHisto)
    {
       fCanvasFit->cd(1)->SetLogz();
       sprintf(tmp, "%s.Histo.Fit", GetName());
       TCUtils::FormatHistogram(fMainHisto, tmp);
       fMainHisto->Draw("colz");
    }

    // draw the overview histogram
    fCanvasResult->cd();
    sprintf(tmp, "%s.Histo.Overview", GetName());
    TCUtils::FormatHistogram(fOverviewHisto, tmp);
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibPed::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.

    Char_t tmp[256];

    // remove old fit histo
    if (fFitHisto) delete fFitHisto;

    // check for main histo
    if (fMainHisto)
    {
        // create histogram projection for this element
        sprintf(tmp, "ProjHisto_%i", elem);
        TH2* h2 = (TH2*) fMainHisto;
        fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    }
    else
    {
        // load the pedestal histogram
        sprintf(tmp, "ADC%d", fADC[elem]);
        fFitHisto = fFileManager->GetHistogram(tmp);
    }

    // skip empty channels
    if (!fFitHisto) return;

    // dummy position
    fMean = 100;

    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fPed_%i", elem);
        fFitFunc = new TF1(tmp, "gaus");
        fFitFunc->SetLineColor(2);

        // check for main histogram
        if (!fMainHisto) // old method using raw adc spectra
        {
            // estimate peak position
            TH1* hDeriv = TCUtils::DeriveHistogram(fFitHisto);
            hDeriv->GetXaxis()->SetRangeUser(0, 1000);
            fMean = hDeriv->GetBinCenter(hDeriv->GetMaximumBin());
            delete hDeriv;

            // configure fitting function
            fFitFunc->SetRange(fMean - 5, fMean + 2);
            fFitFunc->SetParameters(1, fMean, 0.1);
            fFitFunc->SetParLimits(2, 0.001, 5);
        }
        else // new method using pedestal histos
        {
            // estimate peak position
            fMean = fFitHisto->GetXaxis()->GetBinCenter(fFitHisto->GetMaximumBin());

            Double_t max = fFitHisto->GetMaximum();

            // configure fitting function
            fFitFunc->SetRange(fMean - 5, fMean + 5);
            fFitFunc->SetParameters(max, fMean, 0.5);
            fFitFunc->SetParLimits(0, 0.8*max, 1.2*max);
            fFitFunc->SetParLimits(1, fMean - 10, fMean + 10);
            fFitFunc->SetParLimits(2, 0.05, 5);
        }

        // do fit
        fFitHisto->Fit(fFitFunc, "RBQ0");

        // final results
        fMean = fFitFunc->GetParameter(1);

        // set indicator line
        fLine->SetPos(fMean);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    fFitHisto->GetXaxis()->SetRangeUser(fMean-10, fMean+30);
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
void TCCalibPed::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.

    Bool_t unchanged = kFALSE;

    // check if histogram was loaded
    if (!fFitHisto)
    {
        printf("Element: %03d    "
               "old pedestal: %12.8f    new pedestal: %12.8f    diff: %6.2f %%",
               elem, fOldVal[elem], fNewVal[elem],
               TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
        printf("    -> unchanged");
        printf("\n");
        return;
    }

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetPos() != fMean) fMean = fLine->GetPos();

        // save pedestal position
        fNewVal[elem] = fMean;

        // if new value is negative take old
        if (fNewVal[elem] < 0)
        {
            fNewVal[elem] = fOldVal[elem];
            unchanged = kTRUE;
        }

        // update overview histogram
        fOverviewHisto->SetBinContent(elem+1, fNewVal[elem]);
        fOverviewHisto->SetBinError(elem+1, 0.0000001);
    }
    else
    {
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    "
           "old pedestal: %12.8f    new pedestal: %12.8f    diff: %6.2f %%",
           elem, fOldVal[elem], fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}

//______________________________________________________________________________
void TCCalibPed::ReadADC()
{
    // Read the ADC number of each element from the data file registered in the
    // configuration.

    Char_t tmp[256];
    const Char_t* filename;

    // get file name
    sprintf(tmp, "%s.ADCList", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("ReadADC", "ADC list file was not found!");
        return;
    }
    else filename = TCReadConfig::GetReader()->GetConfig(tmp)->Data();

    // read the calibration file with the correct element identifier
    if (this->InheritsFrom("TCCalibTAPSPedSG")) strcpy(tmp, "TAPSSG:");
    else strcpy(tmp, "Element:");
    TCReadARCalib c(filename, kFALSE, tmp);

    // check number of detectors
    if (c.GetNelements() != fNelem)
    {
        Error("ReadADC", "Number of elements in calibration file differs "
                         "from number requested by this module! (%d != %d)",
                         c.GetNelements(), fNelem);
        return;
    }

    // get element list and fill ADC numbers
    TList* list = c.GetElements();
    TIter next(list);
    TCARElement* e;
    Int_t n = 0;
    while ((e = (TCARElement*)next())) fADC[n++] = atoi(e->GetADC());
}

