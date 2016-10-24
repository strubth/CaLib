/*************************************************************************
 * Author: Dominik Werthmueller, Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibDeltaETrad                                                    //
//                                                                      //
// Calibration module for DeltaE calibrations (traditional method).     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TH2.h"
#include "TH3.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TSystem.h"
#include "TSpectrum.h"
#include "TMath.h"

#include "TCCalibDeltaETrad.h"
#include "TCLine.h"
#include "TCFileManager.h"
#include "TCUtils.h"
#include "TCReadConfig.h"
#include "TCMySQLManager.h"

ClassImp(TCCalibDeltaETrad)

//______________________________________________________________________________
TCCalibDeltaETrad::TCCalibDeltaETrad(const Char_t* name, const Char_t* title, const Char_t* data,
                                     Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Constructor.

    // init members
    fFileManager = 0;
    fPedOld = 0;
    fPedNew = 0;
    fGainOld = 0;
    fGainNew = 0;
    fPionMC = 0;
    fProtonMC = 0;
    fPionData = 0;
    fProtonData = 0;
    fPionPos = 0;
    fProtonPos = 0;
    fLine = 0;
    fLine2 = 0;
    fDelay = 0;
    fMCHisto = 0;
    fMCFile = 0;
}

//______________________________________________________________________________
TCCalibDeltaETrad::~TCCalibDeltaETrad()
{
    // Destructor.

    if (fFileManager) delete fFileManager;
    if (fPedOld) delete [] fPedOld;
    if (fPedNew) delete [] fPedNew;
    if (fGainOld) delete [] fGainOld;
    if (fGainNew) delete [] fGainNew;
    if (fPionPos) delete fPionPos;
    if (fProtonPos) delete fProtonPos;
    if (fLine) delete fLine;
    if (fLine2) delete fLine2;
    if (fMCHisto) delete fMCHisto;
    if (fMCFile) delete fMCFile;
}

//______________________________________________________________________________
void TCCalibDeltaETrad::Init()
{
    // Init the module.

    Char_t tmp[256];

    // init members
    fFileManager = new TCFileManager(fData, fCalibration.Data(), fNset, fSet);
    fPedOld = new Double_t[fNelem];
    fPedNew = new Double_t[fNelem];
    fGainOld = new Double_t[fNelem];
    fGainNew = new Double_t[fNelem];
    fPionMC = 0;
    fProtonMC = 0;
    fPionData = 0;
    fProtonData = 0;
    fLine =  new TCLine();
    fLine2 =  new TCLine();
    fDelay = 0;
    fMCHisto = 0;
    fMCFile = 0;

    // configure line
    fLine->SetLineColor(kBlue);
    fLine->SetLineWidth(3);
    fLine2->SetLineColor(kGreen);
    fLine2->SetLineWidth(3);

    // get histogram name
    sprintf(tmp, "%s.Histo.Fit.Name", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig(tmp);

    // get MC histogram file
    TString fileMC;
    sprintf(tmp, "%s.MC.File", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Init", "MC file name was not found in configuration!");
        return;
    }
    else fileMC = *TCReadConfig::GetReader()->GetConfig(tmp);

    // get MC histogram name
    TString histoMC;
    sprintf(tmp, "%s.Histo.MC.Name", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Init", "MC histogram name was not found in configuration!");
        return;
    }
    else histoMC = *TCReadConfig::GetReader()->GetConfig(tmp);

    // get projection fit display delay
    sprintf(tmp, "%s.Fit.Delay", GetName());
    fDelay = TCReadConfig::GetReader()->GetConfigInt(tmp);

    // read old parameters (only from first set)
    TString peds = fData;
    peds.ReplaceAll("E1", "E0");
    TCMySQLManager::GetManager()->ReadParameters(peds.Data(), fCalibration.Data(), fSet[0], fPedOld, fNelem);
    TCMySQLManager::GetManager()->ReadParameters(fData.Data(), fCalibration.Data(), fSet[0], fGainOld, fNelem);

    // copy parameters
    memcpy(fPedNew, fPedOld, fNelem * sizeof(Double_t));
    memcpy(fGainNew, fGainOld, fNelem * sizeof(Double_t));

    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
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

    // create the pion position overview histogram
    fPionPos = new TH1F("Pion position overview", ";Element;pion peak position [MeV]", fNelem, 0, fNelem);
    fPionPos->SetMarkerStyle(2);
    fPionPos->SetMarkerColor(4);

    // create the proton position overview histogram
    fProtonPos = new TH1F("Proton position overview", ";Element;proton peak position [MeV]", fNelem, 0, fNelem);
    fProtonPos->SetMarkerStyle(2);
    fProtonPos->SetMarkerColor(4);

    // draw the overview histograms
    fCanvasResult->Divide(1, 2, 0.001, 0.001);
    fCanvasResult->cd(1);
    fPionPos->Draw("P");
    fCanvasResult->cd(2);
    fProtonPos->Draw("P");

    // user information
    Info("Init", "Fitting MC data");

    // perform fitting for the MC histogram
    sprintf(tmp, "%s.Histo.Fit", GetName());
    TCUtils::FormatHistogram(fMCHisto, tmp);
    FitSlice(fMCHisto);
    fCanvasFit->Update();
    gSystem->Sleep(5000);

    // user information
    Info("Init", "Fitting of MC data finished");
}

//______________________________________________________________________________
void TCCalibDeltaETrad::FitSlice(TH2* h)
{
    // Fit the energy slice of the dE vs E histogram 'h'.

    Char_t tmp[256];

    // get configuration
    Double_t lowLimit, highLimit;
    sprintf(tmp, "%s.Fit.Range", GetName());
    TCReadConfig::GetReader()->GetConfigDoubleDouble(tmp, &lowLimit, &highLimit);
    Int_t firstBin = h->GetXaxis()->FindBin(lowLimit);
    Int_t lastBin = h->GetXaxis()->FindBin(highLimit);

    // create projection
    sprintf(tmp, "%s_Proj", h->GetName());
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h->ProjectionY(tmp, firstBin, lastBin, "e");

    // look for peaks
    TSpectrum s;
    s.Search(fFitHisto, 10, "goff nobackground", 0.05);
    fPionData = TMath::MinElement(s.GetNPeaks(), s.GetPositionX());
    fProtonData = TMath::MaxElement(s.GetNPeaks(), s.GetPositionX());

    // create fitting function
    if (fFitFunc) delete fFitFunc;
    sprintf(tmp, "fFunc_%s", h->GetName());
    fFitFunc = new TF1(tmp, "expo(0)+landau(2)+gaus(5)", 0.2*fPionData, fProtonData+5);
    fFitFunc->SetLineColor(2);

    // prepare fitting function
    fFitFunc->SetParameters(9.25568, -3.76050e-01,
                            5e+03, fPionData, 2.62472e-01,
                            6e+03, fProtonData, 5.82477);
    fFitFunc->SetParLimits(2, 0, 1e6);
    fFitFunc->SetParLimits(3, 0.85*fPionData, 1.15*fPionData);
    fFitFunc->SetParLimits(6, 0.85*fProtonData, 1.15*fProtonData);
    fFitFunc->SetParLimits(5, 0, 1e5);
    fFitFunc->SetParLimits(4, 0.1, 5);
    fFitFunc->SetParLimits(7, 0.1, 10);

    // fit
    for (Int_t i = 0; i < 10; i++)
        if (!fFitHisto->Fit(fFitFunc, "RB0Q")) break;

    // reset range for second fit
    Double_t start;
    if (h == fMCHisto) start = 0.05;
    else start = fFitFunc->GetParameter(3) - 2.5*fFitFunc->GetParameter(4);
    fFitFunc->SetRange(start, fFitFunc->GetParameter(6) + 4*fFitFunc->GetParameter(7));

    // second fit
    for (Int_t i = 0; i < 10; i++)
        if (!fFitHisto->Fit(fFitFunc, "RB0Q")) break;

    // get positions
    fPionData = fFitFunc->GetParameter(3);
    fProtonData = fFitFunc->GetParameter(6);

    // correct weird fits
    if (fPionData < fFitHisto->GetXaxis()->GetXmin() || fPionData > fFitHisto->GetXaxis()->GetXmax())
        fPionData = (fFitHisto->GetXaxis()->GetXmin() + fFitHisto->GetXaxis()->GetXmax()) / 2;
    if (fProtonData < fFitHisto->GetXaxis()->GetXmin() || fProtonData > fFitHisto->GetXaxis()->GetXmax())
        fProtonData = (fFitHisto->GetXaxis()->GetXmin() + fFitHisto->GetXaxis()->GetXmax()) / 2;

    // format line
    fLine->SetPos(fPionData);

    // format line
    fLine2->SetPos(fProtonData);

    // plot histogram and line
    fCanvasFit->cd(2);
    fFitHisto->GetXaxis()->SetRangeUser(0, fFitFunc->GetParameter(6) + 4*fFitFunc->GetParameter(7));
    fFitHisto->Draw("hist");
    fFitFunc->Draw("same");
    fLine->Draw();
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
void TCCalibDeltaETrad::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.

    Char_t tmp[256];

    // create histogram name
    sprintf(tmp, "%s_%03d", fHistoName.Data(), elem);

    // get histogram
    TH1* hmain = (TH1*) fFileManager->GetHistogram(tmp);
    if (!hmain)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }

    // create 2D projection if necessary
    if (fMainHisto) delete fMainHisto;
    if (hmain->InheritsFrom("TH3"))
    {
        sprintf(tmp, "%02d_yxe", elem);
        fMainHisto = (TH2D*) ((TH3*)hmain)->Project3D(tmp);
        fMainHisto->SetTitle(tmp);
        delete hmain;
    }
    else if (hmain->InheritsFrom("TH2"))
    {
        fMainHisto = hmain;
    }
    else
    {
        Error("Init", "Main histogram has to be either TH3 or TH2!\n");
        delete hmain;
        return;
    }

    // draw main histogram
    fCanvasFit->cd(1);
    sprintf(tmp, "%s.Histo.Fit", GetName());
    TCUtils::FormatHistogram(fMainHisto, tmp);
    fMainHisto->Draw("colz");
    fCanvasFit->Update();

    // check for sufficient statistics
    if (fMainHisto->GetEntries())
    {
        // fit the energy slices
        FitSlice((TH2*)fMainHisto);
    }

    // update overview
    fCanvasResult->cd(1);
    fPionPos->Draw("E1");
    fCanvasResult->cd(2);
    fProtonPos->Draw("E1");
    fCanvasResult->Update();
}

//______________________________________________________________________________
void TCCalibDeltaETrad::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.

    Bool_t noval = kFALSE;

    // check if fit was performed
    if (fMainHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetPos() != fPionData) fPionData = fLine->GetPos();
        if (fLine2->GetPos() != fProtonData) fProtonData = fLine2->GetPos();

        // calculate adc values of data fits
        Double_t adc_pion = fPionData/fGainOld[elem] + fPedOld[elem];
        Double_t adc_proton = fProtonData/fGainOld[elem] + fPedOld[elem];

        // calculate new values
        fPedNew[elem] = (adc_pion - (fPionMC/fProtonMC)*adc_proton)/(1. - (fPionMC/fProtonMC));
        fGainNew[elem] = fProtonMC/(adc_proton - fPedNew[elem]);

        // update overview histograms
        fPionPos->SetBinContent(elem+1, fPionData);
        fPionPos->SetBinError(elem+1, 0.0000001);
        fProtonPos->SetBinContent(elem+1, fProtonData);
        fProtonPos->SetBinError(elem+1, 0.0000001);
    }
    else
    {
        noval = kTRUE;
    }

    // user information
    printf("Element: %03d    Pion: %12.8f    Proton: %12.8f    Pedestal: %12.8f (%+2.1f)  Gain: %12.8f (%+2.1f)",
           elem, fPionData, fProtonData,
           fPedNew[elem], TCUtils::GetDiffPercent(fPedOld[elem], fPedNew[elem]),
           fGainNew[elem], TCUtils::GetDiffPercent(fGainOld[elem], fGainNew[elem]));
    if (noval) printf("    -> no fit");
    printf("\n");
}

//______________________________________________________________________________
void TCCalibDeltaETrad::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    Pedestal: %12.8f    Gain: %12.8f\n",
               i, fPedNew[i], fGainNew[i]);
    }
}

//______________________________________________________________________________
void TCCalibDeltaETrad::WriteValues()
{
    // Write the obtained calibration values to the database.

    // name of pedestal data type
    TString peds = fData;
    peds.ReplaceAll("E1", "E0");

    // write values to database
    for (Int_t i = 0; i < fNset; i++)
    {

        TCMySQLManager::GetManager()->WriteParameters(peds.Data(), fCalibration.Data(), fSet[i], fPedNew, fNelem);
        TCMySQLManager::GetManager()->WriteParameters(fData.Data(), fCalibration.Data(), fSet[i], fGainNew, fNelem);
    }

    // save overview canvas
    SaveCanvas(fCanvasResult, "Overview");
}

