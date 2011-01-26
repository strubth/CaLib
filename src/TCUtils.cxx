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
Double_t TCUtils::GetHistogramMinimum(TH1* h)
{
    // Return the non-zero minimum bin content of the histogram 'h'.

    Double_t min = 1e100;

    // loop over bins
    Int_t nBins = h->GetNbinsX();
    for (Int_t i = 1; i <= nBins; i++)
    {
        // get bin content
        Double_t c = h->GetBinContent(i);
        
        // check bin content
        if (c < min && c != 0) min = c;
    }

    return min;
}

//______________________________________________________________________________
Double_t TCUtils::GetHistogramMinimumPosition(TH1* h)
{
    // Return the position of the non-zero minimum bin content of the histogram 'h'.

    Double_t min = 1e100;
    Double_t minPos = 0;

    // loop over bins
    Int_t nBins = h->GetNbinsX();
    for (Int_t i = 1; i <= nBins; i++)
    {
        // get bin content
        Double_t c = h->GetBinContent(i);

        // get position
        Double_t pos = h->GetBinCenter(i);
        
        // check bin content
        if (c < min && c != 0) 
        {
            min = c;
            minPos = pos;
        }
    }

    return minPos;
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
        if (rebin > 1) 
        {
            // check for 2d-histogram
            if (h->InheritsFrom("TH2")) 
            {
                TH2* h2 = (TH2*) h;
                h2->RebinX(rebin);
            }
            else h->Rebin(rebin);
        }
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

//______________________________________________________________________________
Int_t TCUtils::GetVetoInFrontOfElement(Int_t id, Int_t maxTAPS)
{
    // Return the index of the veto that is installed in front of the
    // BaF2 or PWO element with the index 'id' depending on the TAPS setup
    // configured by the number of TAPS elements 'maxTAPS'.
    
    // check TAPS setup
    switch (maxTAPS)
    {
        case 384:
        {
            return id;
        }
        case 402:
        {
            // 1st PWO ring
            if (id >=   0 && id <=   3) return   0;
            if (id >=  67 && id <=  70) return  64;
            if (id >= 134 && id <= 137) return 128;
            if (id >= 201 && id <= 204) return 192;
            if (id >= 268 && id <= 271) return 256;
            if (id >= 335 && id <= 338) return 320;
            
            // other elements
            else return id - 3*(id/67 + 1);
        }
        case 438:
        {
            // 1st PWO ring
            if (id >=   0 && id <=   3) return   0;
            if (id >=  73 && id <=  76) return  64;
            if (id >= 146 && id <= 149) return 128;
            if (id >= 219 && id <= 222) return 192;
            if (id >= 292 && id <= 295) return 256;
            if (id >= 365 && id <= 368) return 320;
            
            // 2nd PWO ring
            if (id >=   4 && id <=   7) return   1;
            if (id >=   8 && id <=  11) return   2;
            if (id >=  77 && id <=  80) return  65;
            if (id >=  81 && id <=  84) return  66;
            if (id >= 150 && id <= 153) return 129;
            if (id >= 154 && id <= 157) return 130;
            if (id >= 223 && id <= 226) return 193;
            if (id >= 227 && id <= 230) return 194;
            if (id >= 296 && id <= 299) return 257;
            if (id >= 300 && id <= 303) return 258;
            if (id >= 369 && id <= 372) return 321;
            if (id >= 373 && id <= 376) return 322;
            
            // other elements
            else return id - 9*(id/73 + 1);
        }
        default:
        {
            return id;
        }
    }
}

//______________________________________________________________________________
Double_t TCUtils::GetDiffPercent(Double_t oldValue, Double_t newValue)
{
    // Return the relative difference in percent between 'oldValue' and 'newValue'.

    // calculate difference
    Double_t diff = newValue - oldValue;

    // return relative difference
    if (oldValue == 0 && newValue == 0) return 0;
    else if (oldValue == 0. && newValue != 0) return 100 * diff / newValue;
    else return 100 * diff / oldValue;
}

