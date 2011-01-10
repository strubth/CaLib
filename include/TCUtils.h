// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
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

#include "TH1.h"

#include "TCReadConfig.h"


namespace TCUtils
{
    void FindBackground(TH1* h, Double_t peak, Double_t low, Double_t high,
                        Double_t* outPar0, Double_t* outPar1);
    Double_t GetHistogramMinimum(TH1* h);
    void FormatHistogram(TH1* h, const Char_t* ident);
    Bool_t IsCBHole(Int_t elem);
    Int_t GetVetoInFrontOfElement(Int_t id, Int_t maxTAPS);
}

#endif

