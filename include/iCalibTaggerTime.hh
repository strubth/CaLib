/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibTaggerTime                                                     //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ITAGGERTIMECALIB_HH
#define ITAGGERTIMECALIB_HH

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
#include "iHistoManager.hh"
#include "iReadFile.hh"
#include "iFitHisto.hh"
#include "iCrystalNavigator.hh"
#include "iMySQLManager.hh"


using namespace std;


class iCalibTaggerTime
    : public virtual iReadConfig,
      public iReadFile,
      public iFitHisto,
      public iCrystalNavigator,
      public iFileManager,
      public iHistoManager
{

private:
    TString strTaggerHistoName;
    TString strTaggerCalibFile;
    TString strTaggerHistoFile;
    Int_t fRun;
    Int_t fSet;

    
    TCanvas* c1;
    TH2F* hTagger; 
    TH1D* hTimeProj[MAX_TAGGER]; 
    TCanvas* c2; 

    TH1F* hhOffset; 
    TF1* fPol0;
    TH1F* GetOffsetDist() { return hhOffset;  }
    TF1* GetPol0() { return fPol0; }
    ifstream infile;
    TString  strLine;

    ofstream outfile;

    TFile* histofile;

    Double_t oldOffset[MAX_TAGGER];
    Double_t newOffset[MAX_TAGGER];
 
public:
    iCalibTaggerTime();
    iCalibTaggerTime(TH2F*);
    iCalibTaggerTime(Int_t);
    virtual ~iCalibTaggerTime();

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

    Bool_t  CheckCrystalNumber(Int_t);

    ClassDef(iCalibTaggerTime, 0)   // Tagger vs Tagger
};

#endif

