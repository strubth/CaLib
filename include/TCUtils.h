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


namespace TCUtils
{
    void FindBackground(TH1* h, Double_t peak, Double_t low, Double_t high,
                        Double_t* outPar0, Double_t* outPar1);
}

#endif

