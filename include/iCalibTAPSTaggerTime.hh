/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibTAPSTaggerTime                                                 //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBTAPSTAGGERTIME_HH 
#define ICALIBTAPSTAGGERTIME_HH 


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
#include "iReadConfig.hh"
#include "iReadFile.hh"
#include "iFitHisto.hh"
#include "iCrystalNavigator.hh"


using namespace std;


class iCalibTAPSTaggerTime
    : public iReadConfig,
      public iReadFile,
      public iFitHisto,
      public iCrystalNavigator
{
    
private:
    TString strTAPSvsTaggerHistoName;
    TString strTAPSvsTaggerCalibFile;
    TString strTAPSvsTaggerHistoFile;
    
    TCanvas* c1;
    TCanvas* c2;
    TH2F* hTAPSvsTagger;
    TH1D* hTimeProj[iConfig::kMaxTAPS];
    TH1F* hhOffset;
    TF1* fPol0;
    ifstream infile;
    TString  strLine;
    ofstream outfile;
    TFile* histofile;
    Double_t oldOffset[iConfig::kMaxTAPS];
    Double_t newOffset[iConfig::kMaxTAPS];
 
public:
    iCalibTAPSTaggerTime();
    iCalibTAPSTaggerTime(TH2F*);
    virtual ~iCalibTAPSTaggerTime();

    void Help();
    void Init();
    
    void DrawThis(Int_t);
    void DrawGraph();
    void GetProjection(Int_t);
    void Calculate(Int_t);
    void DoFor(Int_t);
    void DoFit();
    void DoFit(Int_t);
    Bool_t  CheckCrystalNumber(Int_t);
   
    TH1F* GetOffsetDist() { return hhOffset; }
    TF1* GetPol0() { return fPol0; }

    ClassDef(iCalibTAPSTaggerTime, 0)   // TAPS vs Tagger time calibration
};

#endif

