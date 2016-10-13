/************************************************************************
 * Author: Thomas Strub
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibRunBadScR_TimeShift                                           //
//                                                                      //
// Beamtime calibration module class for run by run time shift bad      //
// scaler reads calibration.                                            //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibRunBadScR_TimeShift.h"
#include "TMath.h"
#include "TGraph.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TCBadScRElement.h"

ClassImp(TCCalibRunBadScR_TimeShift)

//______________________________________________________________________________
void TCCalibRunBadScR_TimeShift::CalibMethodDefault()
{
    // Look for a sudden shift of the time peak, and sets bad scaler reads.

    // allowed time shift tolerance [in ns]
    const Double_t tolerance = 5;

    // init helpers
    TGraph* g_peak_pos = new TGraph();
    Bool_t isbad = kFALSE;

    /* do not fit: is very slow and error-prone
    // get y-axis range
    Double_t y_min = fMainHistos[fIndex]->GetYaxis()->GetBinLowEdge(fMainHistos[fIndex]->GetYaxis()->GetFirst());
    Double_t y_max = fMainHistos[fIndex]->GetYaxis()->GetBinUpEdge(fMainHistos[fIndex]->GetYaxis()->GetLast());

    // fit function
    TF1 f("f", "[0] + gaus(1)", y_min, y_max);
    */

    // loop over all scaler reads
    for (Int_t i = 0; i < fBadScRCurr->GetNElem(); i++)
    {
        // get time projection
        TH1* p = fMainHistos[fIndex]->ProjectionY("_px", i+1, i+1);

        // process last scr
        if (i == fBadScRCurr->GetNElem() - 1 && i >= 1)
        {
            // get time projection of last scr
            TH1* l = fMainHistos[fIndex]->ProjectionY("_px", i, i);

            // check statistics
            if (l->Integral() < 0.1*p->Integral())
            {
                // set bad scr
                if (isbad)
                    if (!fBadScRCurr->IsBad(i)) SetBadScalerRead(i);

                // clean up
                delete p;
                delete l;

                // stop
                break;
            }
        }

        // init peak pos
        Double_t peak = p->GetXaxis()->GetBinCenter(p->GetMaximumBin());

        /* do not fit: is very slow and error-prone
        // helpers
        Double_t mean_y = p->GetMean(2);
        Double_t max = p->GetMaximum();

        // prepare function
        f.SetParameters(mean_y, max,  peak, 5);
        f.SetParLimits(0, 0, mean_y);
        f.SetParLimits(1, max-mean_y, max);
        f.SetParLimits(2, y_min, y_max);
        f.SetParLimits(3, 1, 10);

        // try fit max 10 times
        for (Int_t j = 0; j < 10; j++)
        {
            // fit and set result
            if (!p->Fit(&f, "RBQ0")) // slow & error-prone
            {
                peak = f.GetParameter(2);
                break;
            }
        }
        */

        // add point
        g_peak_pos->SetPoint(g_peak_pos->GetN(), 0.5 + i, peak);

        // check for time shift
        if (!isbad && g_peak_pos->GetN() >= 2)
        {
            Int_t n = g_peak_pos->GetN();

            // check for sudden shift
            if (TMath::Abs(g_peak_pos->GetY()[n-1] - g_peak_pos->GetY()[n-2]) > tolerance)
            {
                // set is bad flag
                isbad = kTRUE;

                // set previous bad scaler read
                if (!fBadScRCurr->IsBad(i-1)) SetBadScalerRead(i-1);
            }
        }

        // set bad scaler read
        if (isbad)
            if (!fBadScRCurr->IsBad(i)) SetBadScalerRead(i);

        delete p;
    }
}

// finito
