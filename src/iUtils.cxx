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


#include "iUtils.hh"


//______________________________________________________________________________
void iUtils::FindBackground(TH1* h, Double_t peak, Double_t low, Double_t high,
                            Double_t* outPar0, Double_t* outPar1)
{
    // Estimate the background.

    Double_t x1, x2, y1, y2;

    x1 = peak - low;
    x2 = peak + high;

    y1 = h->Integral(h->FindBin(x1)-1, h->FindBin(x1)+1, "w")/3.;
    y2 = h->Integral(h->FindBin(x2)-1, h->FindBin(x2)+1, "w")/3.;

    *outPar0 = (y1-y2)/(x1-x2);
    *outPar1 = ((y1+y2) - *outPar0*(x1+x2))/2.;
}

