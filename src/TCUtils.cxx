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


#include "TCUtils.h"


//______________________________________________________________________________
void TCUtils::FindBackground(TH1* h, Double_t peak, Double_t low, Double_t high,
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

//______________________________________________________________________________
void TCUtils::FormatHistogram(TH1* h, const Char_t* ident)
{
    // Apply the formatting read from the configuration file for the identifier
    // 'ident' to the histogram 'h'.
    
    Char_t key[256];
    
    // rebin
    sprintf(key, "%s.Rebin", ident);
    if (TString* value = TCReadConfig::GetReader()->GetConfig(key))
    {
        Int_t rebin = atoi(value->Data());
        if (rebin > 1) h->Rebin(rebin);
    }

    // x-axis range
    sprintf(key, "%s.Xaxis.Range", ident);
    if (TString* value = TCReadConfig::GetReader()->GetConfig(key))
    {
        Double_t min, max;
        sscanf(value->Data(), "%lf%lf", &min, &max);
        h->GetXaxis()->SetRangeUser(min, max);
    }
    
    // y-axis range
    sprintf(key, "%s.Yaxis.Range", ident);
    if (TString* value = TCReadConfig::GetReader()->GetConfig(key))
    {
        Double_t min, max;
        sscanf(value->Data(), "%lf%lf", &min, &max);
        h->GetYaxis()->SetRangeUser(min, max);
    }
}

//______________________________________________________________________________
Bool_t TCUtils::IsCBHole(Int_t elem)
{
    // Check if the element 'elem' belongs to a "CB hole".

    Int_t nholes = 48;
    Int_t cb_holes[] = {26, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40,
                        311, 315, 316, 318, 319,
                        353, 354, 355, 356, 357, 358, 359, 
                        360, 361, 362, 363, 364, 365, 366,
                        400, 401, 402, 405, 408,
                        679, 681, 682, 683, 684, 685, 686, 687, 688, 689,
                        691, 692};
    
    // loop over holes
    for (Int_t i = 0; i < nholes; i++)
    {
        if (cb_holes[i] == elem) return kTRUE;
    }

    return kFALSE;
}

