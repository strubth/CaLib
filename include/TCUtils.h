/*************************************************************************
 * Author: Dominik Werthmueller, Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCUtils                                                              //
//                                                                      //
// CaLib utility methods namespace                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCUTILS_H
#define TCUTILS_H

#include "Rtypes.h"

class TH1;

namespace TCUtils
{
    void FindBackground(TH1* h, Double_t peak, Double_t low, Double_t high,
                        Double_t* outPar0, Double_t* outPar1);
    TH1* DeriveHistogram(TH1* inH);
    void ZeroBins(TH1* inH, Double_t th = 0);
    Double_t Pi0Func(Double_t* x, Double_t* par);
    Double_t GaussLowExpTailPol3(Double_t* x, Double_t* par);
    Double_t GetHistogramMinimum(TH1* h);
    Double_t GetHistogramMinimumPosition(TH1* h);
    void FormatHistogram(TH1* h, const Char_t* ident);
    Bool_t IsCBHole(Int_t elem);
    Int_t GetVetoInFrontOfElement(Int_t id, Int_t maxTAPS);
    Int_t GetTAPSRing(Int_t id, Int_t maxTAPS);
    Bool_t IsTAPSPWO(Int_t id, Int_t maxTAPS);
    Double_t GetDiffPercent(Double_t oldValue, Double_t newValue);
    Int_t ReadCommaSepList(const TString* s, Int_t* outList);
}

#endif

