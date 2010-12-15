
/*******************************************************************
 *                                                                 *
 * Date: 13.04.2009     Author: Irakli                             *
 *                                                                 *
 * This macro is used for                                          *
 *                                                                 *
 * calibbration of the TAPS Vs TAPS Time                           *
 *                                                                 *
 ******************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// A module for                                                               //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "iCalibTAPS2gTime.hh"

ClassImp(iCalibTAPS2gTime)

//------------------------------------------------------------------------------
iCalibTAPS2gTime::iCalibTAPS2gTime()
{
    this->Init();

    // check if file is attached
    if (gROOT->GetFile())
        histofile = gFile;
    else
        histofile = TFile::Open(strTAPSHistoFile);   //Directory of root file
    printf("\n ---------------------------------------- \n");
    printf(" Read Histo File: \"%s\"\n", histofile->GetName());

    // check if file contains default histo
    if (!strTAPSHistoName.Length())
        hTAPSVsTAPS = (TH2F*) histofile->Get("MyTimeTAPSVSTAPS2D");   // 2D plot
    else
        hTAPSVsTAPS = (TH2F*) histofile->Get(strTAPSHistoName.Data());   // 2D plot

    if (!hTAPSVsTAPS)
    {
        printf("\n ERROR: The histogram named: %s can not be found!!!\n"
               "          Check in \"config.cfg\" file 2TAPS.HName!\n",
               strTAPSHistoName.Data());
        gSystem->Exit(0);
    }
    else
        printf(" Read Histo Name: \"%s\"\n",
               strTAPSHistoName.Data());

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAPS2gTime::iCalibTAPS2gTime(Int_t set)
{
    fSet = set;
    this->Init();

    // sum up all files conteined in this runset
    this->DoForSet(set, ECALIB_TAPS_T0, strTAPSHistoName);

    if (hTAPSVsTAPS)
    {
        hTAPSVsTAPS->Delete();
        hTAPSVsTAPS=0;
    }

    if (this->GetMainHisto())
    {
        hTAPSVsTAPS = (TH2F*) this->GetMainHisto();
    }
    else
    {
        printf("\n ERROR: main histogram does not exist!!! \n");
        gSystem->Exit(0);
    }

    //
    //  hTAPSVsTAPS->RebinY( rebin );

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAPS2gTime::iCalibTAPS2gTime(Int_t set, TH2F* h1)
{
    fSet = set;

    this->Init();
    this->InitGUI();

    hTAPSVsTAPS = (TH2F*) h1;

    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAPS2gTime::~iCalibTAPS2gTime()
{
    // needs to be done
    delete c1;
    delete c2;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::Init()
{
    // get histo name and binning
    strTAPSHistoName = *this->GetConfig("2TAPS.HName");
    rebin = atoi(this->GetConfig("2TAPS.Ybin")->Data());

    this->Help();
    //this->ReadFile( strTAPSCalibFile );

    // needed in ReadFile
    for (Int_t i = 0; i < iConfig::kMaxCrystal; i++)
    {
        newTgain[i] = oldTgain[i] = 0;
        newToffs[i] = oldToffs[i] = 0;
        mean_gaus[i] = 0;

        hTimeProj[i] = 0;
    }

    // read from database
    iMySQLManager m;
    m.ReadParameters(fSet, ECALIB_TAPS_T0, oldToffs, iConfig::kMaxTAPS);
    m.ReadParameters(fSet, ECALIB_TAPS_T1, oldTgain, iConfig::kMaxTAPS);

    //   for ( Int_t i = 0; i < 5 ; i++ )
    //     {
    //       printf(" file--par_%03i = %f\n", i, oldToffs[i] );
    //     }

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::InitGUI()
{
    //
    c1 = new TCanvas("c1", "cHistos", 0, 0, 400, 800);
    c1->Divide(1, 2, 0.001, 0.001);
    c1->cd(1);
    c1->GetPad(1)->SetLogz();
    hTAPSVsTAPS->Draw("colz");

    // create graph
    c2 = new TCanvas("c2", "cGraph", 630, 0, 900, 400);
    hhOffset = new TH1F("hhOffset", ";Crystal Number;Time_{TAPS-TAPS} [ns]",
                        iConfig::kMaxTAPS+2, -0.5, iConfig::kMaxTAPS+1.5);
    hhOffset->SetMarkerStyle(28);
    hhOffset->SetMarkerColor(4);
    //  hhOffset->SetStats(kFALSE);
    //  hhOffset->GetYaxis()->SetRangeUser(130, 140);

    hhOffset->GetYaxis()->SetRangeUser(-1, 1);

    hhOffset->Draw("P");

    fPol0 = new TF1("Pol0", "pol0", 0, 720);
    fPol0->SetParameter(0, 1.);
    fPol0->SetLineColor(2);

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::Help()
{
    cout <<"*************************************************************" << endl;
    cout <<"* You are working with Time calibration modul               *" << endl;
    cout <<"*                                                           *" << endl;
    cout <<"*                                                           *" << endl;
    cout <<"*************************************************************" << endl;
    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::GetProjection(Int_t id)
{
    if (!hTimeProj[(id-1)])
    {
        // get projection
        Char_t szName[32];
        sprintf(szName, "hTimeProj_%i", id);

        // Igal
        // hTimeProj[(id-1)] = (TH1D*) hTAPSVsTAPS->ProjectionY(szName, id-1, id-1);

        // New
        //      hTimeProj[(id-1)] = (TH1D*) hTAPSVsTAPS->ProjectionY(szName, id, id);

        // Domi
        hTimeProj[(id-1)] = (TH1D*) hTAPSVsTAPS->ProjectionX(szName, id, id);

    }
    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::Calculate(Int_t id)
{
    // calculate new gain
    //
    if (hTimeProj[(id-1)]->GetEntries())
    {
        if (lOffset[(id-1)]->GetX1() != mean_gaus[(id-1)])
            mean_gaus[(id-1)] = lOffset[(id-1)]->GetX1();

        //      newToffs[(id-1)] = oldToffs[(id-1)] - mean_gaus[(id-1)]/oldTgain[(id-1)];

        newToffs[(id-1)] = oldToffs[(id-1)] + mean_gaus[(id-1)]/oldTgain[(id-1)];

        printf("Element: %03i peak = %10.6f \t "
               "newToffs = %10.6f \t "
               "oldToffs = %10.6f \n",
               id, mean_gaus[(id-1)],
               newToffs[(id-1)],
               oldToffs[(id-1)]);

        hhOffset->SetBinContent(id, mean_gaus[(id-1)]);
        hhOffset->SetBinError(id, 0.1);
    }
    else
    {
        printf("Element: %03i "
               "peak = %10.6f \t "
               "newToffs = %10.6f \t "
               "oldToffs = %10.6f \n",
               id,
               mean_gaus[(id-1)],
               0.0,
               oldToffs[(id-1)]);
    }
    return;
}

//------------------------------------------------------------------------------
Bool_t iCalibTAPS2gTime::CheckCrystalNumber(Int_t id)
{
    //  if( id < 1 || id > 384 )
    if (id < 1 || id > iConfig::kMaxTAPS)
    {
        cerr << "ERROR: bad number of Crystal" << endl;
        return kTRUE;
    }

    return kFALSE;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::DrawThis(Int_t id)
{
    // draw projection with fit function and line

    c1->cd(2);
    hTimeProj[(id-1)]->GetXaxis()->SetRangeUser(peackval -10., peackval +10.);
    hTimeProj[(id-1)]->SetFillColor(22);
    hTimeProj[(id-1)]->Draw();

    if (fFitPeak[(id-1)])
    {
        fFitPeak[(id-1)]->Draw("same");
    }
    lOffset[(id-1)]->Draw();

    c1->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::DrawGraph()
{
    //

    //
    c2->cd();
    hhOffset->GetYaxis()->SetRangeUser(-1, 1);
    hhOffset->Draw("E1");

    fPol0->Draw("same");
    c2->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::DoFor(Int_t id)
{
    if (CheckCrystalNumber(id))
        return;

    currentCrystal = id;

    this->GetProjection(id);

    // check if peak height is enough
    //
    if (hTimeProj[(id-1)]->GetMaximum() < 20)
    {
        Int_t ratio = (hTimeProj[(id-1)]->GetEntries() /
                       hTimeProj[(id-1)]->GetMaximum());

        if (ratio > 6)
            hTimeProj[(id-1)]->Rebin(4);
        else if (ratio > 10)
            hTimeProj[(id-1)]->Rebin(8);
    }

    //
    this->DoFit(id);

    this->DrawThis(id);

    if (id == iConfig::kMaxTAPS)
        //  if( !(id % 10) || id == 384 )
    {
        hhOffset->Fit(fPol0, "+R0", "");
        this->DrawGraph();
    }

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::DoFit()
{
    Int_t id = currentCrystal;
    this->DoFit(id);
    this->DrawThis(id);

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::DoFit(Int_t id)
{
    // Fit the TAPS time histogram
    //
    Char_t szName[64];
    int maxbin;
    double sigma;
    double rms;
    double factor = 2.0;

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
        fFitPeak[(id-1)]->SetParameters(hTimeProj[(id-1)]->GetMaximum(),
                                        peackval, 0.5);

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

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS2gTime::Write()
{

    // write to database
    iMySQLManager m;
    m.WriteParameters(fSet, ECALIB_TAPS_T0, newToffs, iConfig::kMaxTAPS);


    //
    Char_t szName[100];
    sprintf(szName,
            "%s/taps/Tcalib/hGr_set%02i.gif",
            this->GetConfig("HTML.PATH")->Data(),
            fSet);
    //check if Directory for pictures exist, otherwise create
    Char_t szMakeDir[100];
    sprintf(szMakeDir,
            "mkdir -p %s/taps/Tcalib/",
            this->GetConfig("HTML.PATH")->Data());
    gSystem->Exec(szMakeDir);

    c2->SaveAs(szName);

    return;
}

// EOF
