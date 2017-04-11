/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibCBTimeWalk                                                    //
//                                                                      //
// Calibration module for the CB time walk.                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TH1.h"
#include "TH2.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TCLine.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TSystem.h"

#include "TCCalibCBTimeWalk.h"
#include "TCConfig.h"
#include "TCMySQLManager.h"
#include "TCFileManager.h"
#include "TCReadConfig.h"
#include "TCUtils.h"

ClassImp(TCCalibCBTimeWalk)

//______________________________________________________________________________
TCCalibCBTimeWalk::TCCalibCBTimeWalk()
    : TCCalib("CB.TimeWalk", "CB time walk calibration", "Data.CB.Walk.Par0", TCConfig::kMaxCB)
{
    // Empty constructor.

    // init members
    fFileManager = 0;
    fPar0 = 0;
    fPar1 = 0;
    fPar2 = 0;
    fPar3 = 0;
    fGFitPoints = 0;
    fTimeProj = 0;
    fLine = 0;
    fDelay = 0;
    fUseEnergyWeight = kTRUE;
}

//______________________________________________________________________________
TCCalibCBTimeWalk::~TCCalibCBTimeWalk()
{
    // Destructor.

    if (fFileManager) delete fFileManager;
    if (fPar0) delete [] fPar0;
    if (fPar1) delete [] fPar1;
    if (fPar2) delete [] fPar2;
    if (fPar3) delete [] fPar3;
    if (fGFitPoints) delete fGFitPoints;
    if (fTimeProj) delete fTimeProj;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Init()
{
    // Init the module.

    // init members
    fFileManager = new TCFileManager(fData.Data(), fCalibration.Data(), fNset, fSet);
    fPar0 = new Double_t[fNelem];
    fPar1 = new Double_t[fNelem];
    fPar2 = new Double_t[fNelem];
    fPar3 = new Double_t[fNelem];
    fGFitPoints = 0;
    fTimeProj = 0;
    fLine =  new TCLine();
    fDelay = 0;

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("CB.TimeWalk.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("CB.TimeWalk.Histo.Fit.Name");

    // get projection fit display delay
    fDelay = TCReadConfig::GetReader()->GetConfigInt("CB.TimeWalk.Fit.Delay");

    // init weigths
    fUseEnergyWeight = kTRUE;

    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters("Data.CB.Walk.Par0", fCalibration.Data(), fSet[0], fPar0, fNelem);
    TCMySQLManager::GetManager()->ReadParameters("Data.CB.Walk.Par1", fCalibration.Data(), fSet[0], fPar1, fNelem);
    TCMySQLManager::GetManager()->ReadParameters("Data.CB.Walk.Par2", fCalibration.Data(), fSet[0], fPar2, fNelem);
    TCMySQLManager::GetManager()->ReadParameters("Data.CB.Walk.Par3", fCalibration.Data(), fSet[0], fPar3, fNelem);

    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();

    // draw the overview histogram
    fCanvasResult->cd();
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.

    Char_t tmp[256];

    // get configuration
    Double_t lowLimit, highLimit;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("CB.TimeWalk.Histo.Fit.Xaxis.Range", &lowLimit, &highLimit);

    // create histogram name
    sprintf(tmp, "%s_%03d", fHistoName.Data(), elem);

    // delete old histogram
    if (fMainHisto) delete fMainHisto;

    // get histogram
    fMainHisto = fFileManager->GetHistogram(tmp);
    if (!fMainHisto)
    {
        Error("Fit", "Main histogram does not exist!\n");
        return;
    }

    // draw main histogram
    if (fMainHisto->GetEntries() > 0) fCanvasFit->cd(1)->SetLogz(1);
    else fCanvasFit->cd(1)->SetLogz(0);
    TCUtils::FormatHistogram(fMainHisto, "CB.TimeWalk.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();

    // check for sufficient statistics
    if (fMainHisto->GetEntries() < 1000)
    {
        Error("Fit", "Not enough statistics!\n");
        return;
    }

    // create new graph (fit data)
    if (fGFitPoints) delete fGFitPoints;
    fGFitPoints = new TGraphErrors();
    fGFitPoints->SetMarkerStyle(20);
    fGFitPoints->SetMarkerColor(4);

    // prepare stuff for adding
    Int_t added = 0;
    Double_t added_e = 0;
    Double_t added_w = 0;

    // get bins for fitting range
    Int_t startBin = fMainHisto->GetXaxis()->FindBin(lowLimit);
    Int_t endBin = fMainHisto->GetXaxis()->FindBin(highLimit);

    // loop over energy bins
    for (Int_t i = startBin; i <= endBin; i++)
    {
        // create time projection
        sprintf(tmp, "ProjTime_%d_%d", elem, i);
        TH1* proj = (TH1D*) ((TH2*) fMainHisto)->ProjectionY(tmp, i, i, "e");

        // first loop (after fit)
        if (added == 0)
        {
            if (fTimeProj) delete fTimeProj;
            fTimeProj = (TH1*) proj->Clone("TimeProjection");

            // reset values
            added_e = 0;
            added_w = 0;
        }
        else
        {
            fTimeProj->Add(proj);
        }

        // add up bin contribution
        Double_t weight = fUseEnergyWeight ? proj->GetEntries() : 1.;
        added_w += weight;
        added_e += fMainHisto->GetXaxis()->GetBinCenter(i) * weight;
        added++;

        delete proj;

        // check if projection has enough entries
        if (fTimeProj->GetEntries() < 100 && i < endBin)
            continue;

        // calculate mean energy
        Double_t energy = added_e/added_w;

        //
        // fit time projection
        //

        // create fitting function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fWalkProfile_%i", elem);
        fFitFunc = new TF1(tmp, "gaus(0)");
        fFitFunc->SetLineColor(kBlue);

        // prepare fitting function
        Int_t maxbin = fTimeProj->GetMaximumBin();
        Double_t peak = fTimeProj->GetBinCenter(maxbin);
        Double_t max = fTimeProj->GetMaximum();
        fFitFunc->SetRange(peak - 20, peak + 20);
        fFitFunc->SetParameters(max, peak, 1.);
        fFitFunc->SetParLimits(0, max*0.5, max*1.5); // peak height
        fFitFunc->SetParLimits(1, peak - 5, peak + 5); // peak position
        fFitFunc->SetParLimits(2, 0.5, 20.0); // sigma

        // perform fit
        fTimeProj->Fit(fFitFunc, "RBQ0");

        // get parameters
        Double_t mean = fFitFunc->GetParameter(1);
        Double_t error = fFitFunc->GetParError(1);

        // format line
        fLine->SetPos(mean);

        // check fit error
        if (error < 1.)
        {
            Int_t n = fGFitPoints->GetN();
            fGFitPoints->SetPoint(n, energy, mean);
            fGFitPoints->SetPointError(n, 0., error);
        }

        // plot projection fit
        if (fDelay > 0)
        {
            fCanvasFit->cd(2);
            fTimeProj->GetXaxis()->SetRangeUser(mean - 30, mean + 30);
            fTimeProj->Draw("hist");
            fFitFunc->Draw("same");
            fLine->Draw();
            fCanvasFit->Update();
            gSystem->Sleep(fDelay);
        }

        // reset value
        added = 0;

    } // for: loop over energy bins

    //
    // fit profile
    //

    // check for enough points
    if (fGFitPoints->GetN() < 3)
    {
        Error("Fit", "Not enough fit points!");
        return;
    }

    // create fitting function
    sprintf(tmp, "fTWalk_%d", elem);
    if (fFitFunc) delete fFitFunc;
    fFitFunc = new TF1(tmp, "[0] + [1] / TMath::Power(x + [2], [3])", lowLimit, highLimit);
    fFitFunc->SetLineColor(kBlue);
    fFitFunc->SetNpx(2000);

    // prepare fitting function
    fFitFunc->SetParameters(-50, 60, 0.2, 0.3);
    //fFitFunc->SetParLimits(0, 30, 80);
    fFitFunc->SetParLimits(1, -5000, 5000);
    fFitFunc->SetParLimits(2, -1, 10);
    fFitFunc->SetParLimits(3, 0, 1);

    // perform fit
    for (Int_t i = 0; i < 10; i++)
        if (!fGFitPoints->Fit(fFitFunc, "RB0Q")) break;

    // read parameters
    fPar0[elem] = fFitFunc->GetParameter(0);
    fPar1[elem] = fFitFunc->GetParameter(1);
    fPar2[elem] = fFitFunc->GetParameter(2);
    fPar3[elem] = fFitFunc->GetParameter(3);

    // draw energy projection and fit
    fCanvasResult->cd();
    fGFitPoints->Draw("ap");
    fGFitPoints->GetXaxis()->SetLimits(lowLimit, highLimit);
    fFitFunc->Draw("same");
    fGFitPoints->Draw("psame");
    fCanvasResult->Update();

    fCanvasFit->cd(1);
    fFitFunc->Draw("same");
    fCanvasFit->Update();
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.

    Bool_t noval = kFALSE;

    // check no fits
    if (fPar0[elem] == 0 && fPar1[elem] == 0 &&
        fPar2[elem] == 0 && fPar3[elem] == 0) noval = kTRUE;

    // user information
    printf("Element: %03d    Par0: %12.8f    "
           "Par1: %12.8f    Par2: %12.8f    Par3: %12.8f",
           elem, fPar0[elem], fPar1[elem], fPar2[elem], fPar3[elem]);
    if (noval) printf("    -> no fit");
    if (TCUtils::IsCBHole(elem)) printf(" (hole)");
    printf("\n");
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    Par0: %12.8f    "
               "Par1: %12.8f    Par2: %12.8f    Par3: %12.8f\n",
               i, fPar0[i], fPar1[i], fPar2[i], fPar3[i]);
    }
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::WriteValues()
{
    // Write the obtained calibration values to the database.

    // write values to database
    for (Int_t i = 0; i < fNset; i++)
    {
        TCMySQLManager::GetManager()->WriteParameters("Data.CB.Walk.Par0", fCalibration.Data(), fSet[i], fPar0, fNelem);
        TCMySQLManager::GetManager()->WriteParameters("Data.CB.Walk.Par1", fCalibration.Data(), fSet[i], fPar1, fNelem);
        TCMySQLManager::GetManager()->WriteParameters("Data.CB.Walk.Par2", fCalibration.Data(), fSet[i], fPar2, fNelem);
        TCMySQLManager::GetManager()->WriteParameters("Data.CB.Walk.Par3", fCalibration.Data(), fSet[i], fPar3, fNelem);
    }
}

