/*************************************************************************
 * Author: Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCReFit                                                              //
//                                                                      //
// (Re-)fit method namespace                                            //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TH1.h"
#include "TF1.h"
#include "TRandom.h"

#include "TCFitUtils.h"


//______________________________________________________________________________
Bool_t TCFitUtils::ReFit(TH1* h, TF1* f, Option_t* option /*= ""*/, Int_t n /*= 10*/)
{
    // Returns the best fit out of 'n' tries. Between the tries the parameter
    // are randomized.

    // check input
    if (!h || !f) return kFALSE;

    // local fit function
    TF1 func(*f);

    // init return value
    Bool_t success = kFALSE;

    // try n times
    for (Int_t i = 0; i < n; i++)
    {
        // try a fit
        if (h->Fit(&func, option)) continue;
        success = kTRUE;

        // get better fit
        TF1* g = GetBestChi2Func(&func, f);
        if (g) *f = *g;

        // randomize parameters
        RandomizeParameters(&func);
    }

    // return
    return success;
}

//______________________________________________________________________________
TF1* TCFitUtils::GetBestChi2Func(TF1* f1, TF1* f2)
{
    // Return pointer to the function providing the smaller chi square value.
    // If both chi square values are zero the NULL pointer is returned.

    // get chi squares
    Double_t chi2_1 = f1->GetChisquare();
    Double_t chi2_2 = f2->GetChisquare();

    // check for finite chi squares
    if (chi2_1 == 0. && chi2_2 == 0.) return 0;
    if (chi2_1 == 0.) return f2;
    if (chi2_2 == 0.) return f1;

    // return better chi square
    if (f1->GetChisquare() < f2->GetChisquare())
        return f1;
    else
        return f2;
}

//______________________________________________________________________________
void TCFitUtils::RandomizeParameter(TF1* f, Int_t i)
{
    // Sets the i-th parameter of function 'f' to a random value within the
    // parameter limits.

    // get par limits
    Double_t lo, hi;
    f->GetParLimits(i, lo, hi);

    // check limits
    if (lo >= hi) return;

    // randomize parameter
    f->SetParameter(i, lo + (hi-lo)*gRandom->Rndm());
}

//______________________________________________________________________________
void TCFitUtils::RandomizeParameters(TF1* f, Bool_t* isrand /*= 0*/)
{
    // Randomizes the parameters of function 'f'

    // loop over parameters
    for (Int_t i = 0; i < f->GetNpar(); i++)
    {
        // check randomization flags
        if (isrand && !isrand[i]) continue;

        // randomize
        RandomizeParameter(f, i);
    }
}

