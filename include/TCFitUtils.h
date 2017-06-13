/*************************************************************************
 * Author: Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCFitUtils                                                           //
//                                                                      //
// (Re-)fit method namespace                                            //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCFITUTILS_H
#define TCFITUTILS_H

#include "Rtypes.h"

class TH1;
class TF1;

namespace TCFitUtils
{
    Bool_t ReFit(TH1* h, TF1* f, Option_t* option = "", Int_t n = 10);
    TF1* GetBestChi2Func(TF1* f1, TF1* f);
    void RandomizeParameter(TF1* f, Int_t i);
    void RandomizeParameters(TF1* f, Bool_t* isrand = 0);
}

#endif

