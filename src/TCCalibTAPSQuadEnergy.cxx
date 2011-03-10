// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSQuadEnergy                                                //
//                                                                      //
// Calibration module for the quadratic TAPS energy correction.         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTAPSQuadEnergy.h"

ClassImp(TCCalibTAPSQuadEnergy)


//______________________________________________________________________________
TCCalibTAPSQuadEnergy::TCCalibTAPSQuadEnergy()
    : TCCalib("TAPS.QuadEnergy", "TAPS quadratic energy correction", kCALIB_TAPS_EQUAD0, TCConfig::kMaxTAPSThetaBins)
{
    // Empty constructor.
    
    // init members
    fPar0 = 0;
    fPar1 = 0;
    fMainHisto2 = 0;
    fMainHisto3 = 0;
    fFitHisto1b = 0;
    fFitHisto2 = 0;
    fFitHisto3 = 0;
    fFitFunc1b = 0;
    fPi0Pos = 0;
    fEtaPos = 0;
    fPi0MeanE = 0;
    fEtaMeanE = 0;
    fLinePi0 = 0;
    fLineEta = 0;
    fLineMeanEPi0 = 0;
    fLineMeanEEta = 0;
    fPi0PosHisto = 0;
    fEtaPosHisto = 0;
}

//______________________________________________________________________________
TCCalibTAPSQuadEnergy::~TCCalibTAPSQuadEnergy()
{
    // Destructor. 
    
    if (fPar0) delete [] fPar0;
    if (fPar1) delete [] fPar1;
    if (fMainHisto2) delete fMainHisto2;
    if (fMainHisto3) delete fMainHisto3;
    if (fFitHisto1b) delete fFitHisto1b;
    if (fFitHisto2) delete fFitHisto2;
    if (fFitHisto3) delete fFitHisto3;
    if (fFitFunc1b) delete fFitFunc1b;
    if (fLinePi0) delete fLinePi0;
    if (fLineEta) delete fLineEta;
    if (fLineMeanEPi0) delete fLineMeanEPi0;
    if (fLineMeanEEta) delete fLineMeanEEta;
    if (fPi0PosHisto) delete fPi0PosHisto;
    if (fEtaPosHisto) delete fEtaPosHisto;
}

//______________________________________________________________________________
void TCCalibTAPSQuadEnergy::Init()
{
    // Init the module.
    
    // init members
    fPar0 = new Double_t[fNelem];
    fPar1 = new Double_t[fNelem];
    fFitHisto1b = 0;
    fFitHisto2 = 0;
    fFitHisto3 = 0;
    fFitFunc1b = 0;
    fPi0Pos = 0;
    fEtaPos = 0;
    fPi0MeanE = 0;
    fEtaMeanE = 0;
    fLinePi0 = new TLine();
    fLineEta = new TLine();
    fLineMeanEPi0 = new TLine();
    fLineMeanEEta = new TLine();
    
    // configure lines
    fLinePi0->SetLineColor(4);
    fLinePi0->SetLineWidth(3);
    fLineEta->SetLineColor(4);
    fLineEta->SetLineWidth(3);
    fLineMeanEPi0->SetLineColor(4);
    fLineMeanEPi0->SetLineWidth(3);
    fLineMeanEEta->SetLineColor(4);
    fLineMeanEEta->SetLineWidth(3);
  
    // get main histogram name
    if (!TCReadConfig::GetReader()->GetConfig("TAPS.QuadEnergy.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("TAPS.QuadEnergy.Histo.Fit.Name");
    
    // get mean pi0 energy histogram name
    TString hMeanPi0Name;
    if (!TCReadConfig::GetReader()->GetConfig("TAPS.QuadEnergy.Histo.MeanE.Pi0.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else hMeanPi0Name = *TCReadConfig::GetReader()->GetConfig("TAPS.QuadEnergy.Histo.MeanE.Pi0.Name");
    
    // get mean eta energy histogram name
    TString hMeanEtaName;
    if (!TCReadConfig::GetReader()->GetConfig("TAPS.QuadEnergy.Histo.MeanE.Eta.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else hMeanEtaName = *TCReadConfig::GetReader()->GetConfig("TAPS.QuadEnergy.Histo.MeanE.Eta.Name");
      
    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(kCALIB_TAPS_EQUAD0, fCalibration.Data(), fSet[0], fPar0, fNelem);
    TCMySQLManager::GetManager()->ReadParameters(kCALIB_TAPS_EQUAD1, fCalibration.Data(), fSet[0], fPar1, fNelem);

    // sum up all files contained in this runset
    TCFileManager f(fData, fCalibration.Data(), fNset, fSet);
    
    // get the main calibration histogram
    fMainHisto = f.GetHistogram(fHistoName.Data());
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // get the pi0 mean energy histogram
    fMainHisto2 = (TH2*) f.GetHistogram(hMeanPi0Name.Data());
    if (!fMainHisto2)
    {
        Error("Init", "Pi0 mean energy histogram does not exist!\n");
        return;
    }
    
    // get the eta mean energy histogram
    fMainHisto3 = (TH2*) f.GetHistogram(hMeanEtaName.Data());
    if (!fMainHisto3)
    {
        Error("Init", "Eta mean energy histogram does not exist!\n");
        return;
    }
    
    // create the pi0 overview histogram
    fPi0PosHisto = new TH1F("Pi0 position overview", ";Element;#pi^{0} peak position [MeV]", fNelem, 0, fNelem);
    fPi0PosHisto->SetMarkerStyle(2);
    fPi0PosHisto->SetMarkerColor(4);
    TCUtils::FormatHistogram(fPi0PosHisto, "TAPS.QuadEnergy.Histo.Overview.Pi0");
    
    // create the eta overview histogram
    fEtaPosHisto = new TH1F("Eta position overview", ";Element;#eta peak position [MeV]", fNelem, 0, fNelem);
    fEtaPosHisto->SetMarkerStyle(2);
    fEtaPosHisto->SetMarkerColor(4);
    TCUtils::FormatHistogram(fEtaPosHisto, "TAPS.QuadEnergy.Histo.Overview.Eta");
    
    // prepare fit histogram canvas
    fCanvasFit->Divide(1, 4, 0.001, 0.001);

    // draw the overview histograms
    fCanvasResult->Divide(1, 2, 0.001, 0.001);
    fCanvasResult->cd(1);
    fPi0PosHisto->Draw("P");
    fCanvasResult->cd(2);
    fEtaPosHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibTAPSQuadEnergy::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // get the 2g invariant mass histograms
    sprintf(tmp, "ProjHisto_%d", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    if (fFitHisto1b) delete fFitHisto1b;
    fFitHisto = (TH1*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    sprintf(tmp, "ProjHisto_%db", elem);
    fFitHisto1b = (TH1*) fFitHisto->Clone(tmp);
    TCUtils::FormatHistogram(fFitHisto, "TAPS.QuadEnergy.Histo.Fit.Pi0.IM");
    TCUtils::FormatHistogram(fFitHisto1b, "TAPS.QuadEnergy.Histo.Fit.Eta.IM");

    // get pi0 mean energy projection
    sprintf(tmp, "ProjHistoMeanPi0_%d", elem);
    h2 = (TH2*) fMainHisto2;
    if (fFitHisto2) delete fFitHisto2;
    fFitHisto2 = h2->ProjectionX(tmp, elem+1, elem+1, "e");
    TCUtils::FormatHistogram(fFitHisto2, "TAPS.QuadEnergy.Histo.Fit.Pi0.MeanE");

    // get eta mean energy projection
    sprintf(tmp, "ProjHistoMeanEta_%d", elem);
    h2 = (TH2*) fMainHisto3;
    if (fFitHisto3) delete fFitHisto3;
    fFitHisto3 = h2->ProjectionX(tmp, elem+1, elem+1, "e");
    TCUtils::FormatHistogram(fFitHisto3, "TAPS.QuadEnergy.Histo.Fit.Eta.MeanE");

    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old functions
        if (fFitFunc) delete fFitFunc;
        if (fFitFunc1b) delete fFitFunc1b;
        
        // create pi0 fitting function
        sprintf(tmp, "fPi0_%i", elem);
        fFitFunc = new TF1(tmp, "gaus(0)+pol2(3)", 100, 170);
        fFitFunc->SetLineColor(2);
        
        // create pi0 fitting function
        sprintf(tmp, "fEta_%i", elem);
        fFitFunc1b = new TF1(tmp, "gaus(0)+pol2(3)", 450, 650);
        fFitFunc1b->SetLineColor(2);
         
        // configure fitting functions
        fFitFunc->SetParameters(fFitHisto->GetMaximum(), 135, 10, 1, 1, 1);
        fFitFunc->SetParLimits(0, 0, 1e6);
        fFitFunc->SetParLimits(1, 120, 140);
        fFitFunc->SetParLimits(2, 0, 40);
        fFitFunc1b->SetParameters(fFitHisto1b->GetMaximum(), 547, 20, 1, 1, 0.1);
        fFitFunc1b->SetParLimits(0, 0, 1e6);
        fFitFunc1b->SetParLimits(1, 500, 570);
        fFitFunc1b->SetParLimits(2, 0, 50);

        // fit peaks
        fFitHisto->Fit(fFitFunc, "RBQ0");
        fFitHisto1b->Fit(fFitFunc1b, "RBQ0");
        
        // get results
        fPi0Pos = fFitFunc->GetParameter(1);
        fEtaPos = fFitFunc1b->GetParameter(1);
        fPi0MeanE = fFitHisto2->GetMean();
        fEtaMeanE = fFitHisto3->GetMean();

        // draw pi0 position indicator line
        fLinePi0->SetY1(0);
        fLinePi0->SetY2(fFitHisto->GetMaximum() + 20);
        
        // draw eta position indicator line
        fLineEta->SetY1(0);
        fLineEta->SetY2(fFitHisto->GetMaximum() + 20);
        
        // check if mass is in normal range
        if (fPi0Pos < 80 || fPi0Pos > 200) fPi0Pos = 135;
        if (fEtaPos < 450 || fEtaPos > 650) fEtaPos = 547;
        
        // set indicator lines
        fLinePi0->SetX1(fPi0Pos);
        fLinePi0->SetX2(fPi0Pos);
        fLineEta->SetX1(fEtaPos);
        fLineEta->SetX2(fEtaPos);

        // set lines
        fLineMeanEPi0->SetX1(fPi0MeanE);
        fLineMeanEPi0->SetX2(fPi0MeanE);
        fLineMeanEPi0->SetY1(0);
        fLineMeanEPi0->SetY2(fFitHisto2->GetMaximum());
        fLineMeanEEta->SetX1(fEtaMeanE);
        fLineMeanEEta->SetX2(fEtaMeanE);
        fLineMeanEEta->SetY1(0);
        fLineMeanEEta->SetY2(fFitHisto3->GetMaximum());
    }

    // draw pi0 stuff
    fCanvasFit->cd(1); 
    fFitHisto->SetFillColor(35);
    fFitHisto->Draw("hist");
    if (fFitFunc) fFitFunc->Draw("same");
    fLinePi0->Draw();
    
    // draw eta stuff
    fCanvasFit->cd(2); 
    fFitHisto1b->SetFillColor(35);
    fFitHisto1b->Draw("hist");
    if (fFitFunc1b) fFitFunc1b->Draw("same");
    fLineEta->Draw();
    
    fCanvasFit->cd(3); 
    fFitHisto2->SetFillColor(35);
    fFitHisto2->Draw("hist");
    fLineMeanEPi0->Draw();
    
    fCanvasFit->cd(4); 
    fFitHisto3->SetFillColor(35);
    fFitHisto3->Draw("hist");
    fLineMeanEEta->Draw();

    // update canvas
    fCanvasFit->Update();
    
    // update overview
    if (elem % 20 == 0)
    {
        fCanvasResult->cd(1);
        fPi0PosHisto->Draw("E1");
        fCanvasResult->cd(2);
        fEtaPosHisto->Draw("E1");
        fCanvasResult->Update();
    }   
}

//______________________________________________________________________________
void TCCalibTAPSQuadEnergy::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t no_corr = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if pi0 line position was modified by hand
        if (fLinePi0->GetX1() != fPi0Pos) fPi0Pos = fLinePi0->GetX1();
        
        // check if etaline position was modified by hand
        if (fLineEta->GetX1() != fEtaPos) fEtaPos = fLineEta->GetX1();
        
        // calculate quadratic correction factors
        Double_t mean_E_ratio = fEtaMeanE / fPi0MeanE;
        Double_t pion_im_ratio = TCConfig::kPi0Mass * TCConfig::kPi0Mass / fPi0Pos / fPi0Pos;
        Double_t eta_im_ratio = TCConfig::kEtaMass * TCConfig::kEtaMass / fEtaPos / fEtaPos;
        fPar0[elem] = (eta_im_ratio - mean_E_ratio*pion_im_ratio) / (1. - mean_E_ratio);
        fPar1[elem] = (pion_im_ratio - fPar0[elem]) / fPi0MeanE;
        
        // check values
        if (TMath::IsNaN(fPar0[elem]) || TMath::IsNaN(fPar1[elem]))
        {
            fPar0[elem] = 1;
            fPar1[elem] = 0;
            no_corr = kTRUE;
        }

        // update overview histograms
        fPi0PosHisto->SetBinContent(elem+1, fPi0Pos);
        fPi0PosHisto->SetBinError(elem+1, 0.0000001);
        fEtaPosHisto->SetBinContent(elem+1, fEtaPos);
        fEtaPosHisto->SetBinError(elem+1, 0.0000001);
    }
    else
    {   
        fPar0[elem] = 1;
        fPar1[elem] = 0;
        no_corr = kTRUE;
    }

    // user information
    printf("Element: %03d    Pi0: %12.8f    "
           "Eta: %12.8f    Par0: %12.8f    Par1: %12.8f",
           elem, fPi0Pos, fEtaPos, fPar0[elem], fPar1[elem]);
    if (no_corr) printf("    -> no correction");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibTAPSQuadEnergy::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    Par0: %12.8f    "
               "Par1: %12.8f\n",
               i, fPar0[i], fPar1[i]);
    }
}

//______________________________________________________________________________
void TCCalibTAPSQuadEnergy::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    for (Int_t i = 0; i < fNset; i++)
    {
        TCMySQLManager::GetManager()->WriteParameters(kCALIB_TAPS_EQUAD0, fCalibration.Data(), fSet[i], fPar0, fNelem);
        TCMySQLManager::GetManager()->WriteParameters(kCALIB_TAPS_EQUAD1, fCalibration.Data(), fSet[i], fPar1, fNelem);
    }
}

