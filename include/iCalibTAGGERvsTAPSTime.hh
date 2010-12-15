/*************************************************************************
 * Author: Irakli Keshelashvili, Lilian Witthauer
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibTAGGERvsTAPSTime                                               //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBTAGGERVSTAPSTIME_HH
#define ICALIBTAGGERVSTAPSTIME_HH

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "TROOT.h"
#include "TSystem.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TTimeStamp.h"
#include "TString.h"
#include "TMath.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TLine.h"

#include "iConfig.hh"
#include "iFileManager.hh"
#include "iReadConfig.hh"

#include "iFitHisto.hh"
#include "iCrystalNavigator.hh"
#include "iMySQLManager.hh"

using namespace std;

class iCalibTAGGERvsTAPSTime
    : public virtual iReadConfig,
      public iFitHisto,
      public iCrystalNavigator
{

private:
    Int_t fRun;
    Int_t fSet;

    Int_t   rebin;

    TString strTaggerTAPSHistoName;
    TString strTaggerTAPSHistoFile;
    Double_t TaggerTgain;
    Double_t oldToffs[iConfig::kMaxTAGGER];
    Double_t newToffs[iConfig::kMaxTAGGER];
    TCanvas* c1;
    TCanvas* c2; 
    TH2F* hTaggerVsTAPS;
    TH1D* hTimeProj[iConfig::kMaxCrystal];
    TH1F* hhOffset; 
    TF1* fPol0;
    TFile* histofile;

public:
    iCalibTAGGERvsTAPSTime();
    iCalibTAGGERvsTAPSTime(Int_t);
    iCalibTAGGERvsTAPSTime(Int_t, TH2F*);
    virtual ~iCalibTAGGERvsTAPSTime();

    void Help();
    void Init();
    void InitGUI();

    void DrawThis(Int_t);
    void DrawGraph();

    void GetProjection(Int_t);
    void Calculate(Int_t);

    void DoFor(Int_t);
    void DoFit();
    void DoFit(Int_t);
    void Write();
    Bool_t CheckCrystalNumber(Int_t);
   
    TH1F* GetOffsetDist() { return hhOffset; }
    TF1* GetPol0() { return fPol0; }

    ClassDef(iCalibTAGGERvsTAPSTime, 0)   // Tagger vs TAPS time calibration
};

#endif

