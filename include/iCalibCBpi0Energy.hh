/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibCBpi0Energy                                                    //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICBPI0ENERGYCALIB_HH
#define ICBPI0ENERGYCALIB_HH

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
#include "iFileManager.hh"
#include "iReadConfig.hh"
#include "iReadFile.hh"
#include "iFitHisto.hh"
#include "iCrystalNavigator.hh"
#include "iMySQLManager.hh"


using namespace std;


class iCalibCBpi0Energy
    : public virtual iReadConfig,
      public iReadFile,
      public iFitHisto,
      public iCrystalNavigator
{

private:
    Int_t fRun;
    Int_t fSet;

    TString strCBHistoName;
    TString strCBCalibFile;
    TString strCBHistoFile;

    TCanvas* c1; 
    TFile* histofile;
    TH2F*  hCB2gIM;
    TH1D*  hIMProj[iConfig::kMaxCB];
    Double_t peakval;
    TCanvas* c2;

    TH1F* hhIM; 
    TF1* fPol0;
    Double_t oldGain[iConfig::kMaxCB];
    Double_t newGain[iConfig::kMaxCB];

    Double_t oldPi0IM[iConfig::kMaxCB];
    Double_t newPi0IM[iConfig::kMaxCB];
    
public:
    iCalibCBpi0Energy(); 
    iCalibCBpi0Energy(Int_t); 
    iCalibCBpi0Energy(Int_t, TH2F*);
    virtual ~iCalibCBpi0Energy();

    void Help();
    virtual void Init();
    virtual void InitGUI();

    void DrawThis(Int_t);
    void DrawGraph();

    void GetProjection(Int_t);
    void Calculate(Int_t);

    void DoFit();
    void DoFit(Int_t);

    void DoFor(Int_t);

    void Write();
    
    TH1F* GetIMDist() { return hhIM; }
    TF1* GetPol0() { return fPol0; }


    ClassDef(iCalibCBpi0Energy, 0)    // CB Pi0 Energy calibration module
};

#endif

