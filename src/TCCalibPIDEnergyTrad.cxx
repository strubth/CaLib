// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPIDEnergyTrad                                                 //
//                                                                      //
// Calibration module for the PID energy (traditional method).          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibPIDEnergyTrad.h"

ClassImp(TCCalibPIDEnergyTrad)


//______________________________________________________________________________
TCCalibPIDEnergyTrad::TCCalibPIDEnergyTrad()
    : TCCalib("PID.Energy.Trad", "PID energy calibration (traditional)", kCALIB_PID_E1, TCConfig::kMaxPID)
{
    // Empty constructor.

    // init members
    fFitHisto2 = 0;
    fFitFunc2 = 0;
    fFileManager = 0;
    fPed = 0;
    fGain = 0;
    fPionMC = 0;
    fProtonMC = 0;
    fPionData = 0;
    fProtonData = 0;
    fLine = 0;
    fLine2 = 0;
    fDelay = 0;
    fMCHisto = 0;
    fMCFile = 0;
}

//______________________________________________________________________________
TCCalibPIDEnergyTrad::~TCCalibPIDEnergyTrad()
{
    // Destructor. 
 
    if (fFitHisto2) delete fFitHisto2;
    if (fFitFunc2) delete fFitFunc2;
    if (fFileManager) delete fFileManager;
    if (fPed) delete [] fPed;
    if (fGain) delete [] fGain;
    if (fLine) delete fLine;
    if (fLine2) delete fLine2;
    if (fMCHisto) delete fMCHisto;
    if (fMCFile) delete fMCFile;
}

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::Init()
{
    // Init the module.
    
    // init members
    fFitHisto2 = 0;
    fFitFunc2 = 0;
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fPed = new Double_t[fNelem];
    fGain = new Double_t[fNelem];
    fPionMC = 0;
    fProtonMC = 0;
    fPionData = 0;
    fProtonData = 0;
    fLine =  new TLine();
    fLine2 =  new TLine();
    fDelay = 0;
    fMCHisto = 0;
    fMCFile = 0;

    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
    fLine2->SetLineColor(4);
    fLine2->SetLineWidth(3);

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.Histo.Fit.Name");
    
    // get MC histogram file
    TString fileMC;
    if (!TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.MC.File"))
    {
        Error("Init", "MC file name was not found in configuration!");
        return;
    }
    else fileMC = *TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.MC.File");
    
    // get MC histogram name
    TString histoMC;
    if (!TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.Histo.MC.Name"))
    {
        Error("Init", "MC histogram name was not found in configuration!");
        return;
    }
    else histoMC = *TCReadConfig::GetReader()->GetConfig("PID.Energy.Trad.Histo.MC.Name");

    // get projection fit display delay
    fDelay = TCReadConfig::GetReader()->GetConfigInt("PID.Energy.Trad.Fit.Delay");

    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(kCALIB_PID_E0, fCalibration.Data(), fSet[0], fPed, fNelem);
    TCMySQLManager::GetManager()->ReadParameters(kCALIB_PID_E1, fCalibration.Data(), fSet[0], fGain, fNelem);

    // draw main histogram
    fCanvasFit->Divide(1, 3, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();

    // open the MC file
    fMCFile = new TFile(fileMC.Data());
    if (!fMCFile)
    {
        Error("Init", "Could not open MC file!");
        return;
    }
    if (fMCFile->IsZombie())
    {
        Error("Init", "Could not open MC file!");
        return;
    }

    // load the MC histogram
    fMCHisto = (TH2*) fMCFile->Get(histoMC.Data());
    if (!fMCHisto)
    {
        Error("Init", "Could not open MC histogram!");
        return;
    }
    
    // draw main histogram
    fCanvasFit->cd(1);
    fMCHisto->Draw("colz");
    
    // user information
    Info("Init", "Fitting MC data");

    // perform fitting for the MC histogram
    TCUtils::FormatHistogram(fMCHisto, "PID.Energy.Trad.Histo.Fit");
    FitSlice(fMCHisto);
    fCanvasFit->Update();
    gSystem->Sleep(5);

    // user information
    Info("Init", "Fitting of MC data finished");
}

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::FitSlice(TH2* h)
{
    // Fit the energy slice of the dE vs E histogram 'h'.
    
    Char_t tmp[256];

    // get configuration
    Double_t lowLimit, highLimit;
    TCReadConfig::GetReader()->GetConfigDoubleDouble("PID.Energy.Trad.Fit.Range", &lowLimit, &highLimit);
    Int_t firstBin = h->GetXaxis()->FindBin(lowLimit);
    Int_t lastBin = h->GetXaxis()->FindBin(highLimit);
     
    // 
    // Pion fit
    //

    // create projection
    sprintf(tmp, "%s_Proj_1", h->GetName());
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h->ProjectionY(tmp, firstBin, lastBin, "e");
    
   
    //
    // proton fit
    //
    
    // create projection
    sprintf(tmp, "%s_Proj_2", h->GetName());
    if (fFitHisto2) delete fFitHisto2;
    fFitHisto2 = (TH1D*) h->ProjectionY(tmp, firstBin, lastBin, "e");
        
    // look for peaks
    TSpectrum s;
    s.Search(fFitHisto, 10, "goff nobackground", 0.05);
    fPionData = TMath::MinElement(s.GetNPeaks(), s.GetPositionX());
    fProtonData = TMath::MaxElement(s.GetNPeaks(), s.GetPositionX());
    
    // format line
    fLine->SetY1(0);
    fLine->SetY2(fFitHisto->GetMaximum() + 20);
    fLine->SetX1(fPionData);
    fLine->SetX2(fPionData);
    
    // format line
    fLine2->SetY1(0);
    fLine2->SetY2(fFitHisto2->GetMaximum() + 20);
    fLine2->SetX1(fProtonData);
    fLine2->SetX2(fProtonData);
           
    // plot histogram and line
    fCanvasFit->cd(2);
    fFitHisto->GetXaxis()->SetRangeUser(fPionData-2, fPionData+2);
    fFitHisto->Draw("hist");
    fLine->Draw();
    
    // plot histogram and line
    fCanvasFit->cd(3);
    fFitHisto2->GetXaxis()->SetRangeUser(fProtonData-3, fProtonData+3);
    fFitHisto2->Draw("hist");
    fLine2->Draw();
    
    // save MC data
    if (h == fMCHisto)
    {
        fPionMC = fPionData;
        fProtonMC = fProtonData;
    }

    fCanvasFit->Update();
}

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram name
    sprintf(tmp, "%s_%03d", fHistoName.Data(), elem);
   
    // get histogram
    TH3* h3 = (TH3*) fFileManager->GetHistogram(tmp);
    if (!h3)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // create 2D projection
    if (fMainHisto) delete fMainHisto;
    sprintf(tmp, "%02d_yxe", elem);
    h3->GetZaxis()->SetRangeUser(85, 95);
    fMainHisto = (TH2D*) h3->Project3D(tmp);
    fMainHisto->SetTitle(tmp);
    delete h3;
 
    // draw main histogram
    fCanvasFit->cd(1);
    TCUtils::FormatHistogram(fMainHisto, "PID.Energy.Trad.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
 
    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {   
        // fit the energy slices
        FitSlice((TH2*)fMainHisto);


    } // if: sufficient statistics
}

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t noval = kFALSE;

    // check if fit was performed
    if (fMainHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fPionData) fPionData = fLine->GetX1();
        if (fLine2->GetX1() != fProtonData) fProtonData = fLine2->GetX1();
        
        // calculate peak differences
        Double_t diffMC = fProtonMC - fPionMC;
        Double_t diffData = fProtonData - fPionData;

        // calculate pedestal
        fPed[elem] = 100 * (fPionMC*fProtonData - fProtonMC*fPionData) / -diffMC;

        // calculate gain
        fGain[elem] = 0.01 * diffMC / diffData;
    }
    else
    {
        fPed[elem] = 0.;
        fGain[elem] = 0.001;
        noval = kTRUE;
    }

    // user information
    printf("Element: %03d    Pion: %12.8f    Proton: %12.8f    Pedestal: %12.8f    Gain: %12.8f",
           elem, fPionData, fProtonData, fPed[elem], fGain[elem]);
    if (noval) printf("    -> no fit");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    Pedestal: %12.8f    Gain: %12.8f\n",
               i, fPed[i], fGain[i]);
    }
}

//______________________________________________________________________________
void TCCalibPIDEnergyTrad::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    for (Int_t i = 0; i < fNset; i++)
    {
        TCMySQLManager::GetManager()->WriteParameters(kCALIB_PID_E0, fCalibration.Data(), fSet[i], fPed, fNelem);
        TCMySQLManager::GetManager()->WriteParameters(kCALIB_PID_E1, fCalibration.Data(), fSet[i], fGain, fNelem);
    }
}

