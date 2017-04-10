/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// CBElemThresholds.C                                                   //
//                                                                      //
// Determine the thresholds of the CB detector elements.                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#if !defined(__CINT__) || defined(__MAKECINT__)
#include "TOLoader.h"
#endif


//______________________________________________________________________________
Double_t FindFirstFilledBin(TH1* h)
{
    // Return the bin center of the first filled bin.

    for (Int_t i = 1; i <= h->GetNbinsX(); i++)
    {
        if (h->GetBinContent(i) > 0)
            return h->GetBinCenter(i);
    }

    return 0;
}

//______________________________________________________________________________
void CBElemThresholds()
{
    // load CaLib
    gSystem->Load("libCaLib.so");

    // configuration
    const Char_t calibration[]  = "LD2_Mar_13";
    const Int_t set             = 0;
    const Double_t currThr      = 2;
    const Char_t* fADC          = "/home/werthm/src/ROOT/acqu/acqu_user/data/Mar_13/CB/NaI.dat";
    const Char_t* fIn           = "/home/werthm/loc/presort/data/Mar_13/all.root";

    // read gains
    Double_t gain[720];
    TCMySQLManager::GetManager()->ReadParameters("Data.CB.E1", calibration, set, gain, 720);

    // create threshold histogram
    TH1* hThr = new TH1F("Threshold distribution", "Threshold distribution", 200, 0, 20);

    // read the ADC calibration file
    TCReadARCalib c(fADC, kFALSE);

    // get element list and loop over elements
    TList* list = c.GetElements();
    TIter next(list);
    TCARElement* e;
    TH1* h;
    Int_t n = 0;
    Int_t nAbove = 0;
    Double_t thr[720];
    Double_t min, max;
    while ((e = (TCARElement*)next()))
    {
        // load histogram
        TOLoader::LoadObject(fIn, TString::Format("ADC%s", e->GetADC()).Data(), &h, "", "q");

        // find first filled bin
        Double_t x = FindFirstFilledBin(h);

        // calculate threshold in MeV
        thr[n] = x * gain[n];

        // exclude holes
        if (!TCUtils::IsCBHole(n))
        {
            // save min/max
            if (!n)
            {
                min = thr[n];
                max = thr[n];
            }
            else
            {
                min = TMath::Min(min, thr[n]);
                max = TMath::Max(max, thr[n]);
            }

            // fill distribution histo
            hThr->Fill(thr[n]);

            // count elements above currently set threshold
            if (thr[n] > currThr) nAbove++;
        }

        // user info
        printf("%3d  %f\n", n, thr[n]);

        // clean-up
        delete h;

        n++;
    }

    printf("Minimum threshold           : %.2f\n", min);
    printf("Maximum threshold           : %.2f\n", max);
    printf("Above threshold of %.2f MeV : %d (%.1f%%)\n", currThr, nAbove, 100.*nAbove/672.);

    hThr->Draw();
}

