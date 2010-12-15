/*************************************************************************
 * Author: Irakli Keshelashvili, Lilian Witthauer, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iFitHisto                                                            //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef IFITHISTO_HH
#define IFITHISTO_HH

#include <iostream>
#include <fstream>

#include <TH1D.h>
#include <TF1.h>
#include <TLine.h>
#include <TString.h>

#include "iConfig.hh"


using namespace std;


class iFitHisto
{

private:
    Char_t   szName[32];
    Double_t minus;
    Double_t plus;
    Double_t bg_par[2];

protected:
    Double_t peackval;
    Double_t mean_gaus[iConfig::kMaxCrystal];
    Double_t erro_gaus[iConfig::kMaxCrystal];
    TF1*   fFitPeak[iConfig::kMaxCrystal];  //
    TF1*   fGaus[iConfig::kMaxCrystal];     // used for TAPSvsTAPS
    TF1*   fPol0Gaus[iConfig::kMaxCrystal]; // used
    TF1*   fTWalk[iConfig::kMaxCrystal]; // used for CB pi0 Ecalib
    TLine* lOffset[iConfig::kMaxCrystal]; //

    Double_t* FindBG(TH1D*, Double_t);

public:
    iFitHisto();
    virtual ~iFitHisto();

    virtual void Init();
    void FitPIDphi(Int_t, TH1D*); 
    void FitTAGGtime(Int_t, TH1D*); 

    TF1* GetFitFunc(int id) const { return fFitPeak[(id-1)]; };
    Double_t GetPeak(int id) const { return mean_gaus[(id-1)]; };
    Double_t GetErro(int id) const { return erro_gaus[(id-1)]; };

    ClassDef(iFitHisto, 0) 
};

#endif

