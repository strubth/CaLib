
/*******************************************************************
 *                                                                 *
 * Date: 15.12.2008     Author: Lilian, Irakli                     *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// A module for fitting proceduress                                           //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "iFitHisto.hh"

using namespace std;

ClassImp(iFitHisto)

//------------------------------------------------------------------------------

iFitHisto::iFitHisto()
{
    this->Init();
}

//------------------------------------------------------------------------------

iFitHisto::~iFitHisto()
{
    /*for(int i=0; i<MAX_CRYSTAL;i++)
    {
        if(fPol0Gaus[i]) delete fPol0Gaus[i];
        if(fGaus[i]) delete fGaus[i];
        if(lOffset[i]) delete lOffset[i];
        if(fFitPeak[i]) delete fFitPeak[i];
    }*/
    /*    if(fPol0Gaus) delete [] fPol0Gaus;
        if(fGaus) delete [] fGaus;
        if(lOffset) delete [] lOffset;
        if(fFitPeak) delete [] fFitPeak;*/
}

//------------------------------------------------------------------------------

void iFitHisto::Init()
{
    //
    for (Int_t i = 0; i < iConfig::kMaxCrystal; i++)
    {
        fGaus[i]     =0;
        fPol0Gaus[i] =0;

        lOffset[i] = new TLine();
    }

    minus = -50.;
    plus = 50.;

    return;
}

//
// - - - P I D - - -
//

//------------------------------------------------------------------------------
void iFitHisto::FitPIDphi(Int_t id, TH1D* h1)
{
    // Fit the Pi0 peak of an element in the corresponding histogram

    //
    Int_t maxbin = h1->GetMaximumBin();
    peackval = h1->GetBinCenter(maxbin);

    //
    Char_t szName[32];
    sprintf(szName, "fPID_%i", id);

    //
    fFitPeak[(id-1)]->SetName(szName);
    fFitPeak[(id-1)]->SetRange(peackval-50, peackval+50);

    fFitPeak[(id-1)]->SetLineColor(2);

    fFitPeak[(id-1)]->SetParameters(0.0,
                                    h1->GetMaximum(), peackval, 8.);
    h1->Fit(fFitPeak[(id-1)], "+R0Q");
    mean_gaus[(id-1)] = fFitPeak[(id-1)]->GetParameter(3); // store peak value
    erro_gaus[(id-1)] = fFitPeak[(id-1)]->GetParError(3);  // store peak error

    //
    lOffset[(id-1)]->SetVertical();
    lOffset[(id-1)]->SetLineColor(4);
    lOffset[(id-1)]->SetLineWidth(3);
    lOffset[(id-1)]->SetY1(0);
    lOffset[(id-1)]->SetY2(h1->GetMaximum() + 20);
    lOffset[(id-1)]->SetX1(mean_gaus[(id-1)]);
    lOffset[(id-1)]->SetX2(mean_gaus[(id-1)]);

    //   // Do check if everithing is ok
    //   if( fPol1Gaus[(id-1)]->GetParameter(4) < 5. ||
    //       fPol1Gaus[(id-1)]->GetParameter(4) > 10. )

    return;
}

//
// - - - T A G G E R - - -
//

//------------------------------------------------------------------------------
void iFitHisto::FitTAGGtime(Int_t id, TH1D* h1)
{
    // Fit the tagger time histogram
    //
    int maxbin;
    double sigma;
    double rms;
    double factor = 3.0;

    sprintf(szName, "fTime_%i", (id-1));
    fGaus[(id-1)] = new TF1(szName, "gaus");
    fGaus[(id-1)]->SetLineColor(2);

    maxbin = h1->GetMaximumBin();
    peackval = h1->GetBinCenter(maxbin);

    // temporary
    mean_gaus[(id-1)] = peackval;
    rms = h1->GetRMS();

    // first iteration
    fGaus[(id-1)]->SetRange(peackval -0.8, peackval +0.8);
    fGaus[(id-1)]->SetParameters(h1->GetMaximum(), peackval, 0.5);

    fGaus[(id-1)]->SetParLimits(3, 0.8, 3.0); // sigma

    h1->Fit(fGaus[(id-1)], "+R0Q");

    // second iteration
    peackval = fGaus[(id-1)]->GetParameter(1);
    sigma = fGaus[(id-1)]->GetParameter(2);

    fGaus[(id-1)]->SetRange(peackval -factor*sigma,
                            peackval +factor*sigma);
    h1->Fit(fGaus[(id-1)], "+R0Q");

    // final results
    mean_gaus[(id-1)] = fGaus[(id-1)]->GetParameter(1); // store peak value
    erro_gaus[(id-1)] = fGaus[(id-1)]->GetParError(1);  // store peak error

    //
    lOffset[(id-1)] = new TLine();
    lOffset[(id-1)]->SetVertical();
    lOffset[(id-1)]->SetLineColor(4);
    lOffset[(id-1)]->SetLineWidth(3);
    lOffset[(id-1)]->SetY1(0);
    lOffset[(id-1)]->SetY2(h1->GetMaximum() + 20);

    // check if mass is in normal range
    if (mean_gaus[(id-1)] > 10 ||
            mean_gaus[(id-1)] < 200)
    {
        lOffset[(id-1)]->SetX1(mean_gaus[(id-1)]);
        lOffset[(id-1)]->SetX2(mean_gaus[(id-1)]);
    }
    else
    {
        lOffset[(id-1)]->SetX1(135.);
        lOffset[(id-1)]->SetX2(135.);
    }
    return;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Double_t* iFitHisto::FindBG(TH1D* h1, Double_t peackval)
{
    Double_t x1, x2, y1, y2;

    x1 = peackval+minus;
    x2 = peackval+plus;

    y1 = h1->Integral(h1->FindBin(x1)-1, h1->FindBin(x1)+1, "w")/3.;
    y2 = h1->Integral(h1->FindBin(x2)-1, h1->FindBin(x2)+1, "w")/3.;

    bg_par[0] = (y1-y2)/(x1-x2);
    bg_par[1] = ((y1+y2) - bg_par[0]*(x1+x2))/2.;
    //   printf("par[0] %f   par[1] %f \n", bg_par[0], bg_par[1]);

    return bg_par;
}
