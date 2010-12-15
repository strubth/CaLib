/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibTAPS1gEnergy                                                   //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ITAPS1GENERGYCALIB_HH
#define ITAPS1GENERGYCALIB_HH

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


class iCalibTAPS1gEnergy
    : public virtual iReadConfig,
      public iReadFile,
      public iFitHisto,
      public iCrystalNavigator
{

private:
    Int_t fRun;
    Int_t fSet;

    TString strTAPSHistoName;
    TString strTAPSHistoFile;

    Double_t oldGain[iConfig::kMaxTAPS];
    Double_t newGain[iConfig::kMaxTAPS];

    Double_t newPi0IM[iConfig::kMaxTAPS];
    
    TCanvas* c1;
    TFile* histofile;
    TH2F*  hTAPS1gIM;

    TH1D*  hIMProj[iConfig::kMaxTAPS];

    Double_t peakval;
    TCanvas* c2;

    TH1F* hhIM; 
    TF1* fPol0;
 
public:
    iCalibTAPS1gEnergy();            
    iCalibTAPS1gEnergy(Int_t);       
    iCalibTAPS1gEnergy(Int_t, TH2F*);
    virtual ~iCalibTAPS1gEnergy();   

    void Help();
    virtual void Init();
    virtual void InitGUI();

    void DrawThis(Int_t);
    void DrawGraph();

    void GetProjection(Int_t);
    void Calculate(Int_t);

    void DoFor(Int_t);

    void DoFit();
    void DoFit(Int_t);

    void Write();
    
    TH1F* GetIMDist() { return hhIM; }
    TF1* GetPol0() { return fPol0; }

    ClassDef(iCalibTAPS1gEnergy, 0)   // 1g in TAPS vs CB energy calibration
};

#endif

