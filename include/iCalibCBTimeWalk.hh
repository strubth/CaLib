/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibCBTimeWalk                                                     //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBCBTIMEWALK_HH
#define ICALIBCBTIMEWALK_HH

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

#include "iFileManager.hh"
#include "iReadConfig.hh"
#include "iReadFile.hh"
#include "iHistoManager.hh"
#include "iFitHisto.hh"
#include "iCrystalNavigator.hh"
#include "iMySQLManager.hh"


using namespace std;


class iCalibCBTimeWalk
    : public virtual iReadConfig,
      public iFileManager,
      public iHistoManager,
      public iReadFile,
      public iFitHisto,
      public iCrystalNavigator
{

private:
    TString strHName;
    TString strCBCalibFile;
    
    Int_t fRun;
    Int_t fSet;
    TCanvas* c1;
    TCanvas* c2; 

    TH2F* hhTWalk[MAX_CB]; 
    TH1D* hhEWpro[MAX_CB]; 
    TH1D* hhTWpro[MAX_CB]; 
    TF1* fTWalk[MAX_CB]; 
    Double_t fChi2NDF[MAX_CB];
    TFile* histofile;
    Double_t TWalk0[MAX_CRYSTAL];
    Double_t TWalk1[MAX_CRYSTAL];
    Double_t TWalk2[MAX_CRYSTAL];
    Double_t TWalk3[MAX_CRYSTAL];

public:
    iCalibCBTimeWalk();
    iCalibCBTimeWalk(Int_t);
    iCalibCBTimeWalk(Int_t, TH2F*);
    virtual ~iCalibCBTimeWalk();

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

    ClassDef(iCalibCBTimeWalk, 0)   // CB time walk correection class
};

#endif

