/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iUtils                                                               //
//                                                                      //
// CaLib utility methods namespace                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef IUTILS_H
#define IUTILS_H

#include "TH1.h"


namespace iUtils
{
    void FindBackground(TH1* h, Double_t peak, Double_t low, Double_t high,
                        Double_t* outPar0, Double_t* outPar1);
}

#endif

