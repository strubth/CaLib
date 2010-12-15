/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibPIDphi                                                         //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICBPHIPIDIDCALIB_HH
#define ICBPHIPIDIDCALIB_HH

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
#include "iFitHisto.hh"
#include "iCrystalNavigator.hh"
#include "iFileManager.hh"


using namespace std;


class iCalibPIDphi
    : public virtual iReadConfig,
      public iFitHisto,
      public iCrystalNavigator
{

private:
    Int_t fRun;
    Int_t fSet;
    TString strPIDHistoName;
    TString strPIDCalibFile;
    TString strPIDHistoFile;
    TString strPIDTgain;

    TCanvas* c1;
    TFile* histofile;
    TH2F*  hCBPhiVsPID; 
    TH1D*  hProj[iConfig::kMaxCrystal]; 
    TLine* lOffset[iConfig::kMaxCrystal];
    Double_t peakval; 
    TCanvas*      c2;
    TH2F*         h4gr;
    TGraphErrors* grOldPhi;
    TGraphErrors* grNewPhi; 
    ifstream infile;
    TString  strLine;
    ofstream outfile;
    Double_t fOldPhi[iConfig::kMaxPID];
    Double_t fNewPhi[iConfig::kMaxPID];
    Double_t mean_gaus[iConfig::kMaxPID];

public:
    iCalibPIDphi();
    iCalibPIDphi(TH2F*);
    iCalibPIDphi(Int_t);
    virtual ~iCalibPIDphi();

    void Help();
    void Init();
    void InitGUI();

    void DrawThis(Int_t);
    void GetProjection(Int_t);
    void Calculate(Int_t);
    void Write();

    void DoFor(Int_t);
    void DoFit(Int_t);

    ClassDef(iCalibPIDphi, 0)   // PID angular calibration vs CB phi
};

#endif

