
/*******************************************************************
 *                                                                 *
 * Date: 15.12.2008     Author: Irakli                             *
 *                                                                 *
 * This macro is used for                                          *
 *                                                                 *
 * calibbration of the TAPS Vs Tagger Time                         *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// A module for                                                               //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "iCalibTAPSTaggerTime.hh"

ClassImp(iCalibTAPSTaggerTime)

//------------------------------------------------------------------------------
iCalibTAPSTaggerTime::iCalibTAPSTaggerTime()
{
    if (gROOT->GetFile())
        histofile = gFile;
    else
        histofile = TFile::Open(strTAPSvsTaggerHistoFile);   //Directory of root file
    printf("\n ---------------------------------------- \n");
    printf(" Read Histo File: \"%s\"\n", histofile->GetName());

    if (strTAPSvsTaggerHistoName.Length())
        hTAPSvsTagger = (TH2F*) histofile->Get(strTAPSvsTaggerHistoName.Data());   // 2D plot

    if (!hTAPSvsTagger)
    {
        printf("\n ERROR: The histogram named: %s can not be found!!!\n"
               "          Check in \"config.cfg\" file Tagger.HName!\n",
               strTAPSvsTaggerHistoName.Data());
        gSystem->Exit(0);
    }
    else
        printf(" Read Histo Name: \"%s\"\n", strTAPSvsTaggerHistoName.Data());

    this->Init();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAPSTaggerTime::iCalibTAPSTaggerTime(TH2F* h1)
{
    this->Init();

    hTAPSvsTagger = (TH2F*) h1;

    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAPSTaggerTime::~iCalibTAPSTaggerTime()
{
    delete c1;
    delete c2;
}

//------------------------------------------------------------------------------
void iCalibTAPSTaggerTime::Init()
{
    this->Help();
    //this->ReadFile(strTAPSvsTaggerCalibFile);


    // needed in ReadFile
    for (Int_t i = 0; i < iConfig::kMaxTAPS; i++)
    {
        newOffset[i] = oldOffset[i] = 0;
        mean_gaus[i] =0;

        hTimeProj[i] = 0;

     //   for (Int_t j = 0; j < 13; j++)
      //      p[i][j] = new Char_t[16];
    }

    //
    c1 = new TCanvas("c1", "cHistos", 0, 0, 400, 800);
    c1->Divide(1, 2, 0.001, 0.001);
    c1->cd(1);
    c1->GetPad(1)->SetLogz();
    hTAPSvsTagger->Draw("colz");

    // create graph
    c2 = new TCanvas("c2", "cGraph", 630, 0, 900, 400);
    hhOffset = new TH1F("hhOffset", ";Tagger Number;Time [ns]",
                        iConfig::kMaxTAPS, 0.5, iConfig::kMaxTAPS+0.5);
    hhOffset->SetMarkerStyle(28);
    hhOffset->SetMarkerColor(4);
    hhOffset->Draw("P");

    fPol0 = new TF1("Pol0", "pol0", 0, iConfig::kMaxTAPS);
    fPol0->SetParameter(0, 1.);
    fPol0->SetLineColor(2);

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPSTaggerTime::Help()
{
    cout <<"****************************************************************" << endl;
    cout <<"* You are working with Time calibration modul                  *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"****************************************************************" << endl;
    return;
}

//------------------------------------------------------------------------------
void iCalibTAPSTaggerTime::GetProjection(Int_t id)
{
    if (!hTimeProj[(id-1)])
    {
        // get projection
        Char_t szName[32];
        sprintf(szName, "hTimeProj_%i", id);

        hTimeProj[(id-1)] = (TH1D*) hTAPSvsTagger->ProjectionY(szName, id, id, "e");

        //       hTimeProj[(id-1)] = (TH1D*) hTAPSvsTagger->ProjectionY(szName, id, id);

        hTimeProj[(id-1)]->Rebin(4);
    }
    return;
}

//------------------------------------------------------------------------------
void iCalibTAPSTaggerTime::Calculate(Int_t id)
{
    // calculate new gain
    //
    if (hTimeProj[(id-1)]->GetEntries())
    {
        if (lOffset[(id-1)]->GetX1() != mean_gaus[(id-1)])
            mean_gaus[(id-1)] = lOffset[(id-1)]->GetX1();
        
        // domi
        //newOffset[(id-1)] = TAPSoffset[(id-1)] - mean_gaus[(id-1)]; // not good cas 0
       // printf("Element: %03i peak = %10.6f \t newOffset = %10.6f \t oldOffset = %10.6f \n",
       //        id, mean_gaus[(id-1)], newOffset[(id-1)], TAPSoffset[(id-1)]);

        hhOffset->SetBinContent(id, mean_gaus[(id-1)]);
        hhOffset->SetBinError(id, 1.);
    }
    else
    {
        printf("Element: %03i peak = %10.6f \t newOffset = %10.6f \t oldOffset = %10.6f \n",
               id, mean_gaus[(id-1)], 0.0, oldOffset[(id-1)]);
    }
    return;
}

//------------------------------------------------------------------------------
Bool_t iCalibTAPSTaggerTime::CheckCrystalNumber(Int_t id)
{
    if (id < 1 || id > iConfig::kMaxTAPS)
    {
        cerr << "ERROR: bad number of Crystal" << endl;
        return kTRUE;
    }

    return kFALSE;
}

//------------------------------------------------------------------------------
void iCalibTAPSTaggerTime::DrawThis(Int_t id)
{
    c1->cd(2);
    hTimeProj[(id-1)]->GetXaxis()->SetRangeUser(peackval -15.,
            peackval +15.);
    hTimeProj[(id-1)]->Draw();

    if (fPol0Gaus[(id-1)])
    {
        fPol0Gaus[(id-1)]->Draw("same");
    }
    lOffset[(id-1)]->Draw();

    c1->Update();

    //
    c2->cd();

    c2->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPSTaggerTime::DrawGraph()
{
    //
    c2->cd();
    hhOffset->Draw("E1");
    fPol0->Draw("same");
    c2->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPSTaggerTime::DoFor(Int_t id)
{
    if (CheckCrystalNumber(id))
        return;

    currentCrystal = id;

    this->GetProjection(id);

    this->DoFit(id);

    this->DrawThis(id);

    if (!(id % 10) || id == iConfig::kMaxTAPS)
    {
        //      hhOffset->Fit(fPol0, "+R0", "");
        this->DrawGraph();
    }

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPSTaggerTime::DoFit()
{
    Int_t id = currentCrystal;
    this->DoFit(id);
    this->DrawThis(id);

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPSTaggerTime::DoFit(Int_t id)
{
    // Fit the TAPS time histogram
    //
    Char_t szName[64];
    int maxbin;
    double sigma;
    double rms;
    double factor = 3.0;

    sprintf(szName, "fTime_%i", (id-1));
    fFitPeak[(id-1)] = new TF1(szName, "gaus");
    fFitPeak[(id-1)]->SetLineColor(2);

    if (hTimeProj[(id-1)]->GetEntries())
    {
        maxbin = hTimeProj[(id-1)]->GetMaximumBin();
        peackval = hTimeProj[(id-1)]->GetBinCenter(maxbin);

        // temporary
        mean_gaus[(id-1)] = peackval;
        rms = hTimeProj[(id-1)]->GetRMS();

        // first iteration
        fFitPeak[(id-1)]->SetRange(peackval -0.8, peackval +0.8);
        fFitPeak[(id-1)]->SetParameters(hTimeProj[(id-1)]->GetMaximum(), peackval, 0.5);

        fFitPeak[(id-1)]->SetParLimits(2, 0.07, 1.5); // sigma

        hTimeProj[(id-1)]->Fit(fFitPeak[(id-1)], "+R0Q");

        // second iteration
        peackval = fFitPeak[(id-1)]->GetParameter(1);
        sigma = fFitPeak[(id-1)]->GetParameter(2);

        fFitPeak[(id-1)]->SetRange(peackval -factor*sigma,
                                   peackval +factor*sigma);



        hTimeProj[(id-1)]->Fit(fFitPeak[(id-1)], "+R0Q");

        // final results
        mean_gaus[(id-1)] = fFitPeak[(id-1)]->GetParameter(1); // store peak value
        erro_gaus[(id-1)] = fFitPeak[(id-1)]->GetParError(1);  // store peak error

        //
        lOffset[(id-1)] = new TLine();
        lOffset[(id-1)]->SetVertical();
        lOffset[(id-1)]->SetLineColor(4);
        lOffset[(id-1)]->SetLineWidth(3);
        lOffset[(id-1)]->SetY1(0);
        lOffset[(id-1)]->SetY2(hTimeProj[(id-1)]->GetMaximum() + 20);

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
    }

}

// EOF
