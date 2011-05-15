// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibLED                                                           //
//                                                                      //
// Calibration module for LED thresholds.                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibLED.h"

ClassImp(TCCalibLED)


//______________________________________________________________________________
TCCalibLED::TCCalibLED(const Char_t* name, const Char_t* title, CalibData_t data,
                       Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Empty constructor.

    // init members
    fMainHisto2 = 0;
    fDeriv = 0;
    fThr = 0;
    fLine = 0;
}

//______________________________________________________________________________
TCCalibLED::~TCCalibLED()
{
    // Destructor. 
    
    if (fMainHisto2) delete fMainHisto2;
    if (fDeriv) delete fDeriv;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibLED::Init()
{
    // Init the module.
    
    Char_t tmp[256];

    // init members
    fMainHisto2 = 0;
    fDeriv = 0;
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
    if (TCReadConfig::GetReader()->GetConfig(tmp))
        normHistoName = *TCReadConfig::GetReader()->GetConfig(tmp);

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
    if (normHistoName != "")
    {
        fMainHisto2 = (TH2*) f.GetHistogram(normHistoName.Data());
        if (!fMainHisto2)
        {
            Error("Init", "Normalization histogram does not exist!\n");
            return;
        }
    }

    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;LED threshold [MeV]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
 
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    sprintf(tmp, "%s.Histo.Fit", GetName());
    TCUtils::FormatHistogram(fMainHisto, tmp);
    if (fMainHisto2) TCUtils::FormatHistogram(fMainHisto2, tmp);

    // draw the overview histogram
    fCanvasResult->cd();
    sprintf(tmp, "%s.Histo.Overview", GetName());
    TCUtils::FormatHistogram(fOverviewHisto, tmp);
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibLED::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    
    // create projection of the normalization histogram
    if (fMainHisto2)
    {
        TH1* hNorm = (TH1D*) fMainHisto2->ProjectionX(tmp, elem+1, elem+1, "e");
        fFitHisto->Divide(hNorm);
        delete hNorm;
    }

    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // check if normalization histogram is there
        if (fMainHisto2)
        {
            // 
            // ratio method
            //
            
            Double_t posUnder = 0;
            fThr = 0;

            // loop over all bins
            for (Int_t i = 0; i < fFitHisto->GetNbinsX(); i++)
            {
                Double_t pos = fFitHisto->GetBinCenter(i);
                Double_t content = fFitHisto->GetBinContent(i);
                
                // check if ratio is nearly 1
                if (content > 0.99) 
                {
                    fThr = (pos + posUnder) / 2.;   
                    break;
                }
                else posUnder = pos;
            }
        }
        else
        {
            // 
            // derivation method
            //

            // derive historam
            if (fDeriv) delete fDeriv;
            fDeriv = TCUtils::DeriveHistogram(fFitHisto);
            TCUtils::ZeroBins(fDeriv);

            // get maximum
            fThr = fDeriv->GetBinCenter(fDeriv->GetMaximumBin());

            // create fitting function
            if (fFitFunc) delete fFitFunc;
            sprintf(tmp, "Fitfunc_%d", elem);
            fFitFunc = new TF1(tmp, "gaus", fThr-8, fThr+8);
            fFitFunc->SetLineColor(kRed);
            fFitFunc->SetParameters(fDeriv->GetMaximum(), fThr, 1);

            // fit
            fDeriv->Fit(fFitFunc, "RBQ0");
            fThr = fFitFunc->GetParameter(1);
            
            // correct bad position
            if (fThr < fDeriv->GetXaxis()->GetXmin() || fThr > fDeriv->GetXaxis()->GetXmax()) 
                fThr = 0.5 * (fDeriv->GetXaxis()->GetXmin() + fDeriv->GetXaxis()->GetXmax());

            // draw histogram
            fCanvasFit->cd(2);
            fDeriv->GetXaxis()->SetRangeUser(fThr-20, fThr+20);
            fDeriv->Draw("hist");

            // draw function
            fFitFunc->Draw("same");

            // draw indicator line
            fLine->Draw();
        }

        // draw mean indicator line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        fLine->SetX1(fThr);
        fLine->SetX2(fThr);

        // draw histogram
        fFitHisto->SetFillColor(35);
        fCanvasFit->cd(1);
        //fFitHisto->GetXaxis()->SetRangeUser(fThr-20, fThr+20);
        fFitHisto->Draw("hist");
         
        // draw indicator line
        fLine->Draw();
    }

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
void TCCalibLED::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t empty = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
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
        // set large threshold
        fNewVal[elem] = 9999;
        empty = kTRUE;
    }

    // user information
    printf("Element: %03d    "
           "old threshold: %14.8f    new threshold: %14.8f    diff: %6.2f %%",
           elem, fOldVal[elem], fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (empty) printf("    -> empty");
    printf("\n");
}   

