// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSPSA                                                       //
//                                                                      //
// Calibration module for TAPS PSA.                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTAPSPSA.h"

ClassImp(TCCalibTAPSPSA)


//______________________________________________________________________________
TCCalibTAPSPSA::TCCalibTAPSPSA()
    : TCCalib("TAPS.PSA", "TAPS PSA", "Data.TAPS.SG.E1",
              TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements"))
{
    // Empty constructor.

    // init members
    fNpoints = 0;
    fRadius = 0;
    fPhotonMean = 0;
    fPhotonSigma = 0;
    fLPhotonMean = 0;
    fLPhotonSigma = 0;
    fFileManager = 0;
    fAngleProj = 0;
    fDelay = 0;
    
}

//______________________________________________________________________________
TCCalibTAPSPSA::~TCCalibTAPSPSA()
{
    // Destructor. 
    
    if (fRadius) delete [] fRadius;
    if (fPhotonMean) delete [] fPhotonMean;
    if (fPhotonSigma) delete [] fPhotonSigma;
    if (fLPhotonMean) delete fLPhotonMean;
    if (fLPhotonSigma) delete fLPhotonSigma;
    if (fFileManager) delete fFileManager;
    if (fAngleProj) delete fAngleProj;
}

//______________________________________________________________________________
void TCCalibTAPSPSA::Init()
{
    // Init the module.
    
    // init members
    fNpoints = 0;
    fRadius = new Double_t[500];
    fPhotonMean = new Double_t[500];
    fPhotonSigma = new Double_t[500];
    fLPhotonMean = 0;
    fLPhotonSigma = 0;
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fAngleProj = 0;
    fDelay = 0;

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("TAPS.PSA.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("TAPS.PSA.Histo.Fit.Name");
    
    // get projection fit display delay
    fDelay = TCReadConfig::GetReader()->GetConfigInt("TAPS.PSA.Fit.Delay");

    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
}

//______________________________________________________________________________
void TCCalibTAPSPSA::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // get configuration
    Double_t lowLimitX, highLimitX, lowLimitY, highLimitY;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("TAPS.PSA.Histo.Fit.Xaxis.Range", &lowLimitX, &highLimitX);
    TCReadConfig::GetReader()->GetConfigDoubleDouble("TAPS.PSA.Histo.Fit.Yaxis.Range", &lowLimitY, &highLimitY);
  
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
    TCUtils::FormatHistogram(fMainHisto, "TAPS.PSA.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
    
    // reset points
    fNpoints = 0;

    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {
        // cast to 2-dim histo
        TH2* h2 = (TH2*) fMainHisto;

        // prepare stuff for adding
        Int_t added = 0;
        Double_t added_r[500];
        
        // get bins for fitting range
        Int_t startBin = h2->GetYaxis()->FindBin(lowLimitY);
        Int_t endBin = h2->GetYaxis()->FindBin(highLimitY);

        // loop over energy bins
        for (Int_t i = startBin; i <= endBin; i++)
        {   
            // create angle projection
            sprintf(tmp, "ProjAngle_%d_%d", elem, i);
            TH1* proj = (TH1D*) h2->ProjectionX(tmp, i, i, "e");
            
            // check if in adding mode
            if (added)
            {
                // add projection
                fAngleProj->Add(proj);
                delete proj;
                 
                // save bin contribution
                added_r[added++] = h2->GetYaxis()->GetBinCenter(i);
            }
            else 
            {
                if (fAngleProj) delete fAngleProj;
                fAngleProj = proj;
            }

            // check if projection has enough entries
            if (fAngleProj->GetEntries() < 0.02*h2->GetEntries() && i < endBin)
            {
                // start adding mode
                if (!added)
                {
                    // enter adding mode
                    added_r[added++] = h2->GetYaxis()->GetBinCenter(i);
                }

                // go to next bin
                continue;
            }
            else
            {
                Double_t radius = 0;
                
                // finish adding mode
                if (added)
                {
                    // calculate energy
                    for (Int_t j = 0; j < added; j++) radius += added_r[j];
                    radius /= (Double_t)added;
                    
                    added = 0;
                }
                else
                {
                    radius = h2->GetYaxis()->GetBinCenter(i);
                }
                
                // skip point in punch-through region
                if (radius > 210 && radius < 310) continue;
                
                // rebin
                fAngleProj->Rebin(2);
                            
                // find peaks
                //TSpectrum s;
                //s.Search(fAngleProj, 2, "goff nobackground", 0.01);
                //Double_t peakPhoton = TMath::MaxElement(s.GetNPeaks(), s.GetPositionX());
                Double_t peakPhoton = 45;
                 
                // create fitting function
                if (fFitFunc) delete fFitFunc;
                sprintf(tmp, "fFunc_%i", elem);
                fFitFunc = new TF1(tmp, "gaus(0)+pol2(3)", peakPhoton-2, peakPhoton+2);
                fFitFunc->SetLineColor(2);

                // prepare fitting function
                fFitFunc->SetParameters(1, 45, 1, 1, 1, 1);
                fFitFunc->SetParLimits(0, 1, 1e5);
                fFitFunc->SetParLimits(1, 44, 47);
                fFitFunc->SetParLimits(2, 0.1, 2);

                // perform fit
                for (Int_t i = 0; i < 10; i++)
                    if (!fAngleProj->Fit(fFitFunc, "RB0Q")) break;

                // second iteration
                fFitFunc->SetRange(fFitFunc->GetParameter(1) - 2.5*fFitFunc->GetParameter(2),
                                   fFitFunc->GetParameter(1) + 2.5*fFitFunc->GetParameter(2));
                
                // perform fit
                for (Int_t i = 0; i < 10; i++)
                    if (!fAngleProj->Fit(fFitFunc, "RB0Q")) break;

                // get parameters
                fRadius[fNpoints] = radius;
                fPhotonMean[fNpoints] = fFitFunc->GetParameter(1);
                fPhotonSigma[fNpoints] = fFitFunc->GetParameter(1) - fFitFunc->GetParameter(2);
                fNpoints++;

                // plot projection fit  
                if (fDelay > 0)
                {
                    fCanvasFit->cd(2);
                    fAngleProj->Draw("hist");
                    fFitFunc->Draw("same");
                    fCanvasFit->Update();
                    gSystem->Sleep(fDelay);
                }
            
            } // if: projection has sufficient statistics
        
        } // for: loop over energy bins
        

        // 
        // create lines
        //
        if (fLPhotonMean) delete fLPhotonMean;
        fLPhotonMean = new TPolyLine(fNpoints, fPhotonMean, fRadius);
        fLPhotonMean->SetLineWidth(2);

        if (fLPhotonSigma) delete fLPhotonSigma;
        fLPhotonSigma = new TPolyLine(fNpoints, fPhotonSigma, fRadius);
        fLPhotonSigma->SetLineWidth(2);
        fLPhotonSigma->SetLineStyle(2);
        
        // draw lines
        fCanvasFit->cd(1);
        fLPhotonMean->Draw();
        fLPhotonSigma->Draw();

        // set log axis
        fCanvasFit->cd(1)->SetLogz();
    }
    else
    {
        fCanvasFit->cd(1)->SetLogz(kFALSE);
    }

    // update canvas
    fCanvasFit->Update();
}

//______________________________________________________________________________
void TCCalibTAPSPSA::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fMainHisto->GetEntries())
    {
    }
    else
    {
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    processed ", elem);
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibTAPSPSA::PrintValues()
{
    // Disable this method.
    
    Info("PrintValues", "Not implemented in this module");
}

//______________________________________________________________________________
void TCCalibTAPSPSA::Write()
{
    // Write the obtained calibration values to the database.
    
}

