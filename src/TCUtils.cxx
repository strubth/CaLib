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


#include "TH2.h"
#include "TMath.h"
#include "TObjArray.h"
#include "TObjString.h"

#include "TCUtils.h"
#include "TCReadConfig.h"

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
TH1* TCUtils::DeriveHistogram(TH1* inH)
{
    // Return the derivative of the histogram 'inH'.

    Char_t tmp[256];

    // get the number of bins
    Int_t nBins = inH->GetNbinsX();

    // create new histogram
    sprintf(tmp, "%s_Derivative", inH->GetName());
    TH1* h = new TH1D(tmp, tmp, nBins, inH->GetXaxis()->GetXmin(), inH->GetXaxis()->GetXmax());

    // loop over bins
    for (Int_t i = 1; i <= nBins-1; i++)
    {
        // x-difference
        Double_t xdiff = inH->GetBinCenter(i+1) - inH->GetBinCenter(i);

        // y-difference
        Double_t ydiff = inH->GetBinContent(i+1) - inH->GetBinContent(i);

        // fill derived histogram
        h->SetBinContent(i, ydiff / xdiff);
    }

    return h;
}

//______________________________________________________________________________
void TCUtils::ZeroBins(TH1* inH, Double_t th)
{
    // Set all bins of the histogram 'inH' that are lower than the value 'th' to zero.

    // get number of bins
    Int_t nbinsX = inH->GetNbinsX();
    Int_t nbinsY = inH->GetNbinsY();
    Int_t nbinsZ = inH->GetNbinsZ();

    // loop over bins
    // z bins
    for (Int_t i = 0; i <= nbinsZ+1; i++)
    {
        // y bins
        for (Int_t j = 0; j <= nbinsY+1; j++)
        {
            // x bins
            for (Int_t k = 0; k <= nbinsX+1; k++)
            {
                // set content to zero if negative
                if (inH->GetBinContent(k, j, i) < th) inH->SetBinContent(k, j, i, 0);
            }
        }
    }
}

//______________________________________________________________________________
Double_t TCUtils::Pi0Func(Double_t* x, Double_t* par)
{
    // Fitting function for the Pi0 peak in the invariant mass histogram.
    //
    // par[0] : N
    // par[1] : E_peak
    // par[2] : Gamma
    // par[3] : lambda
    // par[4] : log argument
    // par[5] : pol par 0
    // par[6] : pol par 1
    // par[7] : pol par 2
    // par[8] : pol par 3

    Double_t eDiff = x[0] - par[1];
    Double_t G = TMath::Exp(-4. * TMath::Log(par[4]) * eDiff * eDiff / par[2] / par[2]);
    Double_t out = par[5] + par[6]*x[0] + par[7]*x[0]*x[0] + par[8]*x[0]*x[0]*x[0];

    if (x[0] >= par[1]) return out + par[0] * G;                                  // above peak position
    else return out + par[0] * (G + TMath::Exp(eDiff / par[3]) * (1. - G));       // below peak position -> tail
}

//______________________________________________________________________________
Double_t TCUtils::GaussLowExpTailPol3(Double_t* x, Double_t* par)
{
    // Fitting function having a signal peak of the form of a gauss with a low
    // value exponential tail and a polynomial background of degree 3.

    // signal gauss with low value exp tail
    Double_t sig;
    if (par[1]-x[0] > par[3]*par[2])
        sig = par[0]*TMath::Exp(par[3]*par[3]/2. + par[3]*(x[0]-par[1])/par[2]);
    else
        sig = par[0]*TMath::Gaus(x[0], par[1], par[2]);

    // background pol3
    Double_t pol = par[4] + par[5]*x[0] + par[6]*x[0]*x[0] + par[7]*x[0]*x[0]*x[0];

    return sig + pol;
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
            // only rebin 1-dim. histograms
            if (h->GetDimension() == 1)
                h->Rebin(rebin);
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
Int_t TCUtils::GetTAPSRing(Int_t id, Int_t maxTAPS)
{
    // Return the number of the ring (1 to 11) the TAPS element
    // 'id' belongs to depending on the TAPS setup configured by the number of
    // TAPS elements 'maxTAPS'.

    // element layout (first and last elements of a ring)
    Int_t ringRange[11][2] = {{0, 0}, {1, 2}, {3, 5}, {6, 9}, {10, 14}, {15, 20},
                              {21, 27}, {28, 35}, {36, 44}, {45, 54}, {55, 63}};

    // get corresponding TAPS 2007 element number
    Int_t elem = GetVetoInFrontOfElement(id, maxTAPS);

    // map element to first block
    Int_t mid = elem % 64;

    if      (mid >= ringRange[0][0]  && mid <= ringRange[0][1])  return 1;
    else if (mid >= ringRange[1][0]  && mid <= ringRange[1][1])  return 2;
    else if (mid >= ringRange[2][0]  && mid <= ringRange[2][1])  return 3;
    else if (mid >= ringRange[3][0]  && mid <= ringRange[3][1])  return 4;
    else if (mid >= ringRange[4][0]  && mid <= ringRange[4][1])  return 5;
    else if (mid >= ringRange[5][0]  && mid <= ringRange[5][1])  return 6;
    else if (mid >= ringRange[6][0]  && mid <= ringRange[6][1])  return 7;
    else if (mid >= ringRange[7][0]  && mid <= ringRange[7][1])  return 8;
    else if (mid >= ringRange[8][0]  && mid <= ringRange[8][1])  return 9;
    else if (mid >= ringRange[9][0]  && mid <= ringRange[9][1])  return 10;
    else if (mid >= ringRange[10][0] && mid <= ringRange[10][1]) return 11;
    else
    {
        ::Error("GetTAPSRing", "TAPS ring could not be identified!");
        return 99;
    }
}

//______________________________________________________________________________
Bool_t TCUtils::IsTAPSPWO(Int_t id, Int_t maxTAPS)
{
    // Return kTRUE if the element 'id' belongs to the PWO rings of TAPS
    // depending on the TAPS setup configured by the number of
    // TAPS elements 'maxTAPS', otherwise return kFALSE.

    // check TAPS setup
    switch (maxTAPS)
    {
        case 384:
        {
            return kFALSE;
        }
        case 402:
        {
            if (GetTAPSRing(id, maxTAPS) == 1) return kTRUE;
            else return kFALSE;
        }
        case 438:
        {
            if (GetTAPSRing(id, maxTAPS) <= 2) return kTRUE;
            else return kFALSE;
        }
        default:
        {
            ::Error("IsTAPSPWO", "TAPS type could not be identified!");
            return kFALSE;
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

//______________________________________________________________________________
Int_t TCUtils::ReadCommaSepList(const TString* s, Int_t* outList)
{
    // Read comma-separated integers from 's' and save them to 'outList'.
    // Return the number of saved integers.

    // clone string
    TString sclone = *s;

    // tokenize
    TObjArray* arr = sclone.Tokenize(",");

    // fill list
    Int_t n = 0;
    for (Int_t i = 0; i < arr->GetSize(); i++)
    {
        // get substring
        TObjString* osp = (TObjString*) arr->At(i);
        if (!osp) continue;

        // trim and convert
        osp->GetString().ReplaceAll(" ", "");
        outList[n++] = osp->GetString().Atoi();
    }

    // clean-up
    if (arr) delete arr;

    return n;
}

