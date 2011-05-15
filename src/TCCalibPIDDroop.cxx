// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPIDDroop                                                      //
//                                                                      //
// Calibration module for the PID droop correction.                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibPIDDroop.h"

ClassImp(TCCalibPIDDroop)


//______________________________________________________________________________
TCCalibPIDDroop::TCCalibPIDDroop()
    : TCCalib("PID.Droop", "PID droop correction", kCALIB_PID_DROOP0, TCConfig::kMaxPID)
{
    // Empty constructor.

    // init members
    fPar0 = 0;
    fPar1 = 0;
    fPar2 = 0;
    fPar3 = 0;
    fFileManager = 0;
    fProj2D = 0;
    fLinPlot = 0;
    fNpeak = 0;
    fPeak = 0;
    fTheta = 0;
    fLine = 0;
    fDelay = 0;
}

//______________________________________________________________________________
TCCalibPIDDroop::~TCCalibPIDDroop()
{
    // Destructor. 
    
    if (fPar0) delete [] fPar0;
    if (fPar1) delete [] fPar1;
    if (fPar2) delete [] fPar2;
    if (fPar3) delete [] fPar3;
    if (fFileManager) delete fFileManager;
    if (fProj2D) delete fProj2D;
    if (fLinPlot) delete fLinPlot;
    if (fPeak) delete [] fPeak;
    if (fTheta) delete [] fTheta;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibPIDDroop::Init()
{
    // Init the module.
    
    // init members
    fPar0 = new Double_t[fNelem];
    fPar1 = new Double_t[fNelem];
    fPar2 = new Double_t[fNelem];
    fPar3 = new Double_t[fNelem];
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fProj2D = 0;
    fLinPlot = 0;
    fNpeak = 0;
    fPeak = 0;
    fTheta = 0;
    fLine = new TLine();
    fDelay = 0;

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("PID.Droop.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("PID.Droop.Histo.Fit.Name");
    
    // get projection fit display delay
    fDelay = TCReadConfig::GetReader()->GetConfigInt("PID.Droop.Fit.Delay");

    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(kCALIB_PID_DROOP0, fCalibration.Data(), fSet[0], fPar0, fNelem);
    TCMySQLManager::GetManager()->ReadParameters(kCALIB_PID_DROOP1, fCalibration.Data(), fSet[0], fPar1, fNelem);
    TCMySQLManager::GetManager()->ReadParameters(kCALIB_PID_DROOP2, fCalibration.Data(), fSet[0], fPar2, fNelem);
    TCMySQLManager::GetManager()->ReadParameters(kCALIB_PID_DROOP3, fCalibration.Data(), fSet[0], fPar3, fNelem);

    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
}

//______________________________________________________________________________
Double_t TCCalibPIDDroop::FitHisto()
{
    // Fit the 'fFitHisto' and return the position of the proton peak.

    // look for peaks
    TSpectrum s;
    s.Search(fFitHisto, 10, "goff nobackground", 0.03);
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
    
    // create fitting function
    if (fFitFunc) delete fFitFunc;
    fFitFunc = new TF1("Fitfunc", "expo(0)+landau(2)+gaus(5)", 0.3*peakPion, peak+3);
    fFitFunc->SetLineColor(2);
    
    // prepare fitting function
    fFitFunc->SetParameters(9.25568, -3.76050e-01, 
                            5e+03, peakPion, 2.62472e-01, 
                            6e+03, peak, 5.82477);
    fFitFunc->SetParLimits(2, 0, 1e6);
    fFitFunc->SetParLimits(3, 0.85*peakPion, 1.15*peakPion);
    fFitFunc->SetParLimits(6, 0.85*peak, 1.15*peak);
    fFitFunc->SetParLimits(5, 0, 1e5);
    fFitFunc->SetParLimits(4, 0.1, 5);
    fFitFunc->SetParLimits(7, 0.1, 10);
  
    // perform first fit
    for (Int_t i = 0; i < 10; i++)
        if (!fFitHisto->Fit(fFitFunc, "RB0Q")) break;

    // return peak
    return fFitFunc->GetParameter(6);
}

//______________________________________________________________________________
void TCCalibPIDDroop::FitSlices(TH3* h, Int_t elem)
{
    // Fit the theta slices of the dE vs E histogram 'h'.
    
    Char_t tmp[256];

    // get configuration
    Double_t lowLimit, highLimit;
    Double_t lowEnergy, highEnergy;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("PID.Droop.Fit.Range", &lowLimit, &highLimit);
    TCReadConfig::GetReader()->GetConfigDoubleDouble("PID.Droop.Fit.Energy.Range", &lowEnergy, &highEnergy);
    Double_t interval = TCReadConfig::GetReader()->GetConfigDouble("PID.Droop.Fit.Interval");
    
    // count points
    fNpeak = (highLimit - lowLimit) / interval;
    
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
    Double_t peakTotal = FitHisto();
    
    // format line
    fLine->SetY1(0);
    fLine->SetY2(fFitHisto->GetMaximum() + 20);
    fLine->SetX1(peakTotal);
    fLine->SetX2(peakTotal);
  
    // plot projection fit  
    if (fDelay > 0)
    {
        fCanvasFit->cd(1);
        TCUtils::FormatHistogram(fProj2D, "PID.Droop.Histo.Fit");
        fProj2D->Draw("colz");
        fCanvasFit->cd(2);
        fFitHisto->GetXaxis()->SetRangeUser(0, peakTotal+3);
        fFitHisto->Draw("hist");
        fFitFunc->Draw("same");
        fLine->Draw();
        fCanvasFit->Update();
        gSystem->Sleep(fDelay);
    }


    //
    // loop over theta slices
    //
    Double_t start = lowLimit;
    Int_t nfit = 0;
    while (start < highLimit)
    {
        // set theta axis range
        h->GetZaxis()->SetRangeUser(start, start + interval);
        
        // create 2D projection
        if (fProj2D) delete fProj2D;
        sprintf(tmp, "Proj2D_%d_yxe", (Int_t)start);
        fProj2D = (TH2D*) h->Project3D(tmp);
        sprintf(tmp, "%02d dE vs E : %.f < #theta < %.f", elem, start, start + interval);
        fProj2D->SetTitle(tmp);

        // create 1D projection
        if (fFitHisto) delete fFitHisto;
        sprintf(tmp, "Proj_%d", (Int_t)start);
        Int_t firstBin = fProj2D->GetXaxis()->FindBin(lowEnergy);
        Int_t lastBin = fProj2D->GetXaxis()->FindBin(highEnergy);
        fFitHisto = (TH1D*) fProj2D->ProjectionY(tmp, firstBin, lastBin, "e");
        sprintf(tmp, "%02d dE : %.f < #theta < %.f, %.f < E < %.f", elem, start, start + interval,
                                                             lowEnergy, highEnergy);
        fFitHisto->SetTitle(tmp);
            
        // fit histo
        Double_t peak = FitHisto();

        // format line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        fLine->SetX1(peak);
        fLine->SetX2(peak);
        
        // save peak and theta position
        fPeak[nfit] = peak / peakTotal;
        fTheta[nfit] = start + interval / 2.;

        // plot projection fit  
        if (fDelay > 0)
        {
            fCanvasFit->cd(1);
            TCUtils::FormatHistogram(fProj2D, "PID.Droop.Histo.Fit");
            fProj2D->Draw("colz");
            fCanvasFit->cd(2);
            fFitHisto->GetXaxis()->SetRangeUser(0, peak+3);
            fFitHisto->Draw("hist");
            fFitFunc->Draw("same");
            fLine->Draw();
            fCanvasFit->Update();
            gSystem->Sleep(fDelay);
        }

        // increment loop variables
        start += interval;
        nfit++;
        
    } // while: loop over energy slices
}

//______________________________________________________________________________
void TCCalibPIDDroop::Fit(Int_t elem)
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

        // create linear plot
        if (fLinPlot) delete fLinPlot;
        fLinPlot = new TGraph(fNpeak, fTheta, fPeak);
        sprintf(tmp, "Element %d", elem);
        fLinPlot->SetName(tmp);
        fLinPlot->SetTitle(tmp);
        fLinPlot->GetXaxis()->SetTitle("Cluster theta angle [deg]");
        fLinPlot->GetYaxis()->SetTitle("#theta bin proton peak / total proton peak pos.");
        fLinPlot->SetMarkerStyle(2);
        fLinPlot->SetMarkerSize(2);
        fLinPlot->SetMarkerColor(kBlue);
        
        // create linear fitting function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "Func_%d", elem);
        fFitFunc = new TF1(tmp, "pol3");
        fFitFunc->SetLineColor(kRed);
        
        // fit linear plot
        fFitFunc->SetRange(0.9*TMath::MinElement(fNpeak, fTheta), 
                           1.1*TMath::MaxElement(fNpeak, fTheta));
        fLinPlot->Fit(fFitFunc, "RB0Q");

        // plot linear plot
        fCanvasResult->cd();
        fLinPlot->Draw("ap");
        fFitFunc->Draw("same");
        fCanvasResult->Update();

    } // if: sufficient statistics
}

//______________________________________________________________________________
void TCCalibPIDDroop::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t noval = kFALSE;

    // check if fit was performed
    if (fMainHisto->GetEntries())
    {
        // set parameters
        fPar0[elem] = fFitFunc->GetParameter(0);
        fPar1[elem] = fFitFunc->GetParameter(1);
        fPar2[elem] = fFitFunc->GetParameter(2);
        fPar3[elem] = fFitFunc->GetParameter(3);
    }
    else
    {
        fPar0[elem] = 0.;
        fPar1[elem] = 1.;
        fPar2[elem] = 0.;
        fPar3[elem] = 0.;
        noval = kTRUE;
    }

    // user information
    printf("Element: %03d    Par0: %12.8f    "
           "Par1: %12.8f    Par2: %12.8f    Par3: %12.8f",
           elem, fPar0[elem], fPar1[elem], fPar2[elem], fPar3[elem]);
    if (noval) printf("    -> no fit");
    printf("\n");
    
    // save canvas
    Char_t tmp[256];
    sprintf(tmp, "Elem_%d", elem);
    SaveCanvas(fCanvasResult, tmp);
}   

//______________________________________________________________________________
void TCCalibPIDDroop::PrintValues()
{
    // Print out the old and new values for all elements.

    //// loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    Par0: %12.8f    "
               "Par1: %12.8f    Par2: %12.8f    Par3: %12.8f\n",
               i, fPar0[i], fPar1[i], fPar2[i], fPar3[i]);
     }
}

//______________________________________________________________________________
void TCCalibPIDDroop::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    for (Int_t i = 0; i < fNset; i++)
    {
        TCMySQLManager::GetManager()->WriteParameters(kCALIB_PID_DROOP0, fCalibration.Data(), fSet[i], fPar0, fNelem);
        TCMySQLManager::GetManager()->WriteParameters(kCALIB_PID_DROOP1, fCalibration.Data(), fSet[i], fPar1, fNelem);
        TCMySQLManager::GetManager()->WriteParameters(kCALIB_PID_DROOP2, fCalibration.Data(), fSet[i], fPar2, fNelem);
        TCMySQLManager::GetManager()->WriteParameters(kCALIB_PID_DROOP3, fCalibration.Data(), fSet[i], fPar3, fNelem);
    }
}

