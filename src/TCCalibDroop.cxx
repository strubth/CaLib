/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibDroop                                                         //
//                                                                      //
// Calibration module for droop corrections.                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TFile.h"
#include "TH2.h"
#include "TH3.h"
#include "TSpectrum.h"
#include "TF1.h"
#include "TGraph.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TSystem.h"

#include "TCCalibDroop.h"
#include "TCFileManager.h"
#include "TCReadConfig.h"
#include "TCUtils.h"

ClassImp(TCCalibDroop)

//______________________________________________________________________________
TCCalibDroop::TCCalibDroop(const Char_t* name, const Char_t* title, const Char_t* data,
                           Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Constructor.

    // init members
    fOutFile = 0;
    fFileManager = 0;
    fProj2D = 0;
    fLinPlot = 0;
    fNpeak = 0;
    fNpoint = 0;
    fPeak = 0;
    fTheta = 0;
    fLine = 0;
    fDelay = 0;
}

//______________________________________________________________________________
TCCalibDroop::~TCCalibDroop()
{
    // Destructor.

    // close the output file
    if (fOutFile)
    {
        Info("Calculate", "Closing output file '%s'", fOutFile->GetName());
        delete fOutFile;
    }

    if (fFileManager) delete fFileManager;
    if (fProj2D) delete fProj2D;
    if (fLinPlot) delete fLinPlot;
    if (fPeak) delete [] fPeak;
    if (fTheta) delete [] fTheta;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibDroop::Init()
{
    // Init the module.

    Char_t tmp[256];

    // init members
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fProj2D = 0;
    fLinPlot = 0;
    fNpeak = 0;
    fNpoint = 0;
    fPeak = 0;
    fTheta = 0;
    fLine = new TLine();
    fDelay = 0;

    // try to open the output file
    TString str = GetName();
    str.ReplaceAll(".Droop", "");
    sprintf(tmp, "%s_Droop_Corr_%s.root", str.Data(), fCalibration.Data());
    fOutFile = new TFile(tmp, "update");
    if (fOutFile->IsZombie())
    {
        Error("Init", "Could not create output file '%s'!", tmp);
        return;
    }
    else
    {
        Info("Init", "Opening output file '%s'", tmp);
    }

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

    // get projection fit display delay
    sprintf(tmp, "%s.Fit.Delay", GetName());
    fDelay = TCReadConfig::GetReader()->GetConfigInt(tmp);

    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
}

//______________________________________________________________________________
Bool_t TCCalibDroop::FitHisto(Double_t* outPeak)
{
    // Fit the 'fFitHisto' and write the position of the proton peak to 'outPeak'.
    // Return kTRUE on success, otherwise kFALSE.

    // look for peaks
    TSpectrum s;
    s.Search(fFitHisto, 10, "goff nobackground", 0.01);
    Double_t peakPion = TMath::MinElement(s.GetNPeaks(), s.GetPositionX());
    Double_t peak = 0;
    Double_t diff = 1e3;
    for (Int_t i = 0; i < s.GetNPeaks(); i++)
    {
        if (TMath::Abs(s.GetPositionX()[i] - 3) < diff)
        {
            diff = TMath::Abs(s.GetPositionX()[i] - 3);
            peak = s.GetPositionX()[i];
        }
    }

    // adjustments for PID/pizza detector
    Double_t fmax = 0;
    if (this->InheritsFrom("TCCalibPIDDroop"))
    {
        fmax = peak+4;
    }
    else if (this->InheritsFrom("TCCalibPizzaDroop"))
    {
        peak = 7;
        fmax = peak+3;
    }

    // create fitting function
    if (fFitFunc) delete fFitFunc;
    fFitFunc = new TF1("Fitfunc", "expo(0)+landau(2)+gaus(5)", 0.1*peakPion, fmax);
    fFitFunc->SetLineColor(2);

    // prepare fitting function
    fFitFunc->SetParameters(9.25568, -3.76050e-01,
                            5e+03, peakPion, 2.62472e-01,
                            6e+03, peak, 0.4);
    fFitFunc->SetParLimits(2, 0, 1e6);
    fFitFunc->SetParLimits(3, 0.9*peakPion, 1.1*peakPion);
    if (this->InheritsFrom("TCCalibPIDDroop"))
        fFitFunc->SetParLimits(6, 2, 5);
    else if (this->InheritsFrom("TCCalibPizzaDroop"))
        fFitFunc->SetParLimits(6, 5, 9);
    fFitFunc->SetParLimits(5, 0, 1e5);
    fFitFunc->SetParLimits(4, 0.1, 1);
    fFitFunc->SetParLimits(7, 0.3, 5);

    // perform first fit
    Int_t fitRes;
    for (Int_t i = 0; i < 20; i++)
        if (!(fitRes = fFitHisto->Fit(fFitFunc, "RB0Q"))) break;

    // save peak
    if (outPeak) *outPeak = fFitFunc->GetParameter(6);

    // reject bad fits
    if (fFitFunc->GetParameter(7) / fFitFunc->GetParameter(5) > 1 || fitRes)
        return kFALSE;

    return kTRUE;
}

//______________________________________________________________________________
void TCCalibDroop::FitSlices(TH3* h, Int_t elem)
{
    // Fit the theta slices of the dE vs E histogram 'h'.

    Char_t tmp[256];

    // get configuration
    Double_t lowLimit, highLimit;
    Double_t lowEnergy, highEnergy;
    TCReadConfig::GetReader()->GetConfigDoubleDouble(TString::Format("%s.Fit.Range", GetName()).Data(), &lowLimit, &highLimit);
    TCReadConfig::GetReader()->GetConfigDoubleDouble(TString::Format("%s.Fit.Energy.Range", GetName()).Data(), &lowEnergy, &highEnergy);
    Double_t interval = TCReadConfig::GetReader()->GetConfigDouble(TString::Format("%s.Fit.Interval", GetName()).Data());

    // count points
    fNpeak = (highLimit - lowLimit) / interval;
    fNpoint = 0;

    // prepare arrays
    if (!fPeak) fPeak = new Double_t[fNpeak];
    if (!fTheta) fTheta = new Double_t[fNpeak];

    //
    // get global proton position
    //

    // create 2D projection
    if (fProj2D) delete fProj2D;
    fProj2D = (TH2D*) h->Project3D("Proj2DTot_yxe");
    sprintf(tmp, "%02d total", elem);
    fProj2D->SetTitle(tmp);

    // create 1D projection
    if (fFitHisto) delete fFitHisto;
    Int_t firstBin = fProj2D->GetXaxis()->FindBin(lowEnergy);
    Int_t lastBin = fProj2D->GetXaxis()->FindBin(highEnergy);
    fFitHisto = (TH1D*) fProj2D->ProjectionY("ProjTot", firstBin, lastBin, "e");
    sprintf(tmp, "%02d total %.f < E < %.f", elem, lowEnergy, highEnergy);
    fFitHisto->SetTitle(tmp);

    // fit histo
    Double_t peakTotal;
    FitHisto(&peakTotal);

    // format line
    fLine->SetY1(0);
    fLine->SetY2(fFitHisto->GetMaximum() + 20);
    fLine->SetX1(peakTotal);
    fLine->SetX2(peakTotal);

    // plot projection fit
    if (fDelay > 0)
    {
        TCUtils::FormatHistogram(fProj2D, TString::Format("%s.Histo.Fit", GetName()).Data());
        fCanvasFit->cd(1);
        fProj2D->Draw("colz");
        fFitHisto->GetXaxis()->SetRangeUser(0, peakTotal+3);
        fCanvasFit->cd(2);
        fFitHisto->Draw("hist");
        fFitFunc->Draw("same");
        fLine->Draw();
        fCanvasFit->Update();
        gSystem->Sleep(2000);
    }


    //
    // loop over theta slices
    //
    Double_t start = lowLimit;
    while (start < highLimit)
    {
        // set theta axis range
        h->GetZaxis()->SetRangeUser(start, start + interval);

        // create 2D projection
        if (fProj2D) delete fProj2D;
        sprintf(tmp, "Proj2D_%d_yxe", (Int_t)start);
        fProj2D = (TH2D*) h->Project3D(tmp);
        if (this->InheritsFrom("TCCalibPIDDroop"))
            sprintf(tmp, "%02d dE vs E : %.f < #theta < %.f", elem, start, start + interval);
        else if (this->InheritsFrom("TCCalibPizzaDroop"))
            sprintf(tmp, "%02d dE vs E : %.f < r_{paddle} < %.f", elem, start, start + interval);
        fProj2D->SetTitle(tmp);

        // create 1D projection
        if (fFitHisto) delete fFitHisto;
        sprintf(tmp, "Proj_%d", (Int_t)start);
        Int_t firstBin = fProj2D->GetXaxis()->FindBin(lowEnergy);
        Int_t lastBin = fProj2D->GetXaxis()->FindBin(highEnergy);
        fFitHisto = (TH1D*) fProj2D->ProjectionY(tmp, firstBin, lastBin, "e");
        if (this->InheritsFrom("TCCalibPIDDroop"))
            sprintf(tmp, "%02d dE : %.f < #theta < %.f, %.f < E < %.f", elem, start, start + interval,
                                                                        lowEnergy, highEnergy);
        else if (this->InheritsFrom("TCCalibPizzaDroop"))
            sprintf(tmp, "%02d dE : %.f < r_{paddle} < %.f, %.f < E < %.f", elem, start, start + interval,
                                                                            lowEnergy, highEnergy);
        fFitHisto->SetTitle(tmp);

        // fit histo
        Double_t peak;
        Bool_t fitRes = FitHisto(&peak);

        // on success
        if (fitRes)
        {
            // format line
            fLine->SetY1(0);
            fLine->SetY2(fFitHisto->GetMaximum() + 20);
            fLine->SetX1(peak);
            fLine->SetX2(peak);

            // save peak and theta position
            if (this->InheritsFrom("TCCalibPIDDroop"))
                fPeak[fNpoint] = peak / peakTotal;
            else if (this->InheritsFrom("TCCalibPizzaDroop"))
                fPeak[fNpoint] = peak;
            fTheta[fNpoint] = start + interval / 2.;

            // count point
            fNpoint++;
        }

        // plot projection fit
        if (fDelay > 0)
        {
            TCUtils::FormatHistogram(fProj2D, TString::Format("%s.Histo.Fit", GetName()).Data());
            fCanvasFit->cd(1);
            fProj2D->Draw("colz");
            fFitHisto->GetXaxis()->SetRangeUser(0, peak+4);
            fCanvasFit->cd(2);
            fFitHisto->Draw("hist");
            if (fitRes)
            {
                fFitFunc->Draw("same");
                fLine->Draw();
            }
            fCanvasFit->Update();
            gSystem->Sleep(fDelay);
        }

        // increment loop variables
        start += interval;

     } // while: loop over energy slices
}

//______________________________________________________________________________
void TCCalibDroop::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.

    Char_t tmp[256];

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

    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {
        // fit the theta slices
        FitSlices((TH3*)fMainHisto, elem);

        // normalize peak positions
        if (this->InheritsFrom("TCCalibPizzaDroop"))
        {
            Double_t norm = fPeak[0];
            for (Int_t i = 0; i < fNpoint; i++) fPeak[i] /= norm;
        }

        // create linear plot
        if (fLinPlot) delete fLinPlot;
        fLinPlot = new TGraph(fNpoint, fTheta, fPeak);
        sprintf(tmp, "Droop_Corr_%02d", elem);
        fLinPlot->SetName(tmp);
        fLinPlot->SetTitle(tmp);
        if (this->InheritsFrom("TCCalibPIDDroop"))
        {
            fLinPlot->GetXaxis()->SetTitle("CB Cluster theta angle [deg]");
            fLinPlot->GetYaxis()->SetTitle("bin proton peak pos. / total proton peak pos.");
        }
        else if (this->InheritsFrom("TCCalibPizzaDroop"))
        {
            fLinPlot->GetXaxis()->SetTitle("Pizza paddle radius [cm]");
            fLinPlot->GetYaxis()->SetTitle("bin proton peak pos. / bin 0 proton peak pos.");
        }
        fLinPlot->SetMarkerStyle(20);
        fLinPlot->SetMarkerSize(1);
        fLinPlot->SetMarkerColor(kBlue);
        fLinPlot->GetYaxis()->SetRangeUser(0.5, 1.5);

        // plot linear plot
        fCanvasResult->cd();
        fLinPlot->Draw("ap");
        fCanvasResult->Update();

    } // if: sufficient statistics
}

//______________________________________________________________________________
void TCCalibDroop::Calculate(Int_t elem)
{
    // Nothing to do here.

}

//______________________________________________________________________________
void TCCalibDroop::PrintValues()
{
    // Disable this method.

    Info("PrintValues", "Not implemented in this module");
}

//______________________________________________________________________________
void TCCalibDroop::WriteValues()
{
    // Write to output file instead of database.

    // check if output file exits
    if (!fOutFile)
    {
        Error("Write", "Cannot save droop correction to output file!");
        return;
    }

    // save TGraph to output file
    fOutFile->cd();
    fLinPlot->Write();
    Info("Write", "Droop correction '%s' was written to '%s'", fLinPlot->GetName(), fOutFile->GetName());
}

