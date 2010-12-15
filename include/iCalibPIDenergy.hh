/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibPIDenergy                                                      //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBPIDENERGY_HH
#define ICALIBPIDENERGY_HH

#include <iostream>

#include <TObject.h>
#include "TROOT.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TMath.h"
#include "TTimer.h"

#include "TString.h"
#include "TH1D.h"
#include "TH2F.h"
#include "TF1.h"
#include "TGraphErrors.h"

#include "iConfig.hh"
#include "iReadConfig.hh"
#include "iReadFile.hh"
#include "iFitHisto.hh"
#include "iCrystalNavigator.hh"
#include "iMySQLManager.hh"
#include "iFileManager.hh"

using namespace std;

class iCalibPIDenergy
    : public virtual iReadConfig,
      public iReadFile,
      public iFitHisto,
      public iCrystalNavigator
{

private:
    Int_t fRun;
    Int_t fSet;
    
    TString strPIDHistoName;
    TString strPIDCalibFile;
    TString strPIDHistoFile;
    TString strPIDPathMC;
    TString strPIDTgain;

    TCanvas* c1;
    TFile* histofile;
    TFile* mcfile;
    TH2F*  hdat_dE_E; 
    TH2F*  hmc_dE_E; 

    TH1D*  hdatProj[iConfig::kMaxCrystal]; 
    TH1D*  hmcProj[iConfig::kMaxCrystal]; 
    TLine* lProtonDat[iConfig::kMaxCrystal];
    TLine* lProtonMC[iConfig::kMaxCrystal];
    TLine* lPionDat[iConfig::kMaxCrystal];
    TLine* lPionMC[iConfig::kMaxCrystal];

    Double_t peakval; 

    TF1* fFitData[iConfig::kMaxCrystal];
    TF1* fFitMC[iConfig::kMaxCrystal];
    
    TCanvas*      c2; 
    TH2F*         h4gr;

    TGraphErrors* grDat; 
    TGraphErrors* grMC; 

    ifstream infile;
    TString  strLine;
    ofstream outfile;

    Double_t fOldPhi[iConfig::kMaxPID];
    Double_t fNewPhi[iConfig::kMaxPID];
    Double_t mean_gaus[iConfig::kMaxPID];

public:
    iCalibPIDenergy();
    iCalibPIDenergy(TH2F*);
    iCalibPIDenergy(Int_t);
    virtual ~iCalibPIDenergy();

    void Help();
    void Init();
    void InitGUI();

    void DrawThis(Int_t);
    void GetProjection(Int_t);
    void Calculate(Int_t);
    void Write();

    void DoFor(Int_t);
    void DoFit(Int_t);

    ClassDef(iCalibPIDenergy, 0)   // PID angular calibration vs CB phi
};

#endif

