/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibTAPS2gTime                                                     //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBTAPS2GTIME_HH 
#define ICALIBTAPS2GTIME_HH 

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
#include "iReadFile.hh"

#include "iFitHisto.hh"
#include "iCrystalNavigator.hh"
#include "iMySQLManager.hh"


using namespace std;


class iCalibTAPS2gTime
    : public virtual iReadConfig,
      public iFileManager,
      public iReadFile,
      public iFitHisto,
      public iCrystalNavigator
{

private:
    Int_t fRun;
    Int_t fSet;

    Int_t rebin;

    // for TAPS Vs TAPS
    TString strTAPSHistoName;
    TString strTAPSCalibFile;
    TString strTAPSHistoFile;
    TString strTAPSHistoYbin;
    TString str2gTAPS_DB;

    Double_t oldTgain[MAX_CRYSTAL];
    Double_t oldToffs[MAX_CRYSTAL];

    Double_t newToffs[MAX_CRYSTAL];
    Double_t newTgain[MAX_CRYSTAL];
    
    TCanvas* c1;
    TH2F* hTAPSVsTAPS;
    TH1D* hTimeProj[MAX_CRYSTAL];
    TCanvas* c2;
    TF1* fPol0;
    TH1F* hhOffset; 
    TFile* histofile;
 
public:
    iCalibTAPS2gTime();
    iCalibTAPS2gTime(Int_t);
    iCalibTAPS2gTime(Int_t, TH2F*);
    virtual ~iCalibTAPS2gTime();

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

    Bool_t  CheckCrystalNumber(Int_t);

    TH1F* GetOffsetDist() { return hhOffset; }
    TF1* GetPol0() { return fPol0; }

    ClassDef(iCalibTAPS2gTime, 0)   // TAPS vs TAPS time calibration
};

#endif

