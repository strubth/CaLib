
/*******************************************************************
 *                                                                 *
 * Date: 13.04.2009     Author: Irakli                             *
 *                                                                 *
 * This macro is used for                                          *
 *                                                                 *
 * calibration of the TAGGER vs TAPS Time                          *
 *                                                                 *
 ******************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// A module for                                                               //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "iCalibTAGGERvsTAPSTime.hh"

ClassImp(iCalibTAGGERvsTAPSTime)

//------------------------------------------------------------------------------
iCalibTAGGERvsTAPSTime::iCalibTAGGERvsTAPSTime()
{
    this->Init();

    // check if file is attached
    if (gROOT->GetFile())
        histofile = gFile;
    else
        histofile = TFile::Open(strTaggerTAPSHistoFile);   //Directory of root file
    printf("\n ---------------------------------------- \n");
    printf(" Read Histo File: \"%s\"\n", histofile->GetName());

    // check if file contains default histo
    if (!strTaggerTAPSHistoName.Length())
        hTaggerVsTAPS = (TH2F*) histofile->Get("MyTimeTaggerVSTAPS2D");   // 2D plot
    else
        hTaggerVsTAPS = (TH2F*) histofile->Get(strTaggerTAPSHistoName.Data());   // 2D plot

    cout << strTaggerTAPSHistoName << endl;

    if (!hTaggerVsTAPS)
    {
        printf("\n ERROR: The histogram named: %s can not be found!!!\n"
               "          Check in \"config.cfg\" file TAGGERvsTAPS.HName!\n",
               strTaggerTAPSHistoName.Data());
        gSystem->Exit(0);
    }
    else
        printf(" Read Histo Name: \"%s\"\n",
               strTaggerTAPSHistoName.Data());

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAGGERvsTAPSTime::iCalibTAGGERvsTAPSTime(Int_t set)
{
    fSet = set;
    this->Init();

    // sum up all files contained in this runset
    this->DoForSet(set, ECALIB_TAGG_T0, strTaggerTAPSHistoName);

    if (hTaggerVsTAPS)
    {
        hTaggerVsTAPS->Delete();
        hTaggerVsTAPS=0;
    }

    if (this->GetMainHisto())
    {
        hTaggerVsTAPS = (TH2F*) this->GetMainHisto();
    }
    else
    {
        printf("\n ERROR: main histogram does not exist!!! \n");
        gSystem->Exit(0);
    }

    //
    hTaggerVsTAPS->RebinY(rebin);

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAGGERvsTAPSTime::iCalibTAGGERvsTAPSTime(Int_t set, TH2F* h1)
{
    fSet = set;

    this->Init();
    this->InitGUI();

    hTaggerVsTAPS = (TH2F*) h1;

    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAGGERvsTAPSTime::~iCalibTAGGERvsTAPSTime()
{
    // needs to be done
    delete c1;
    delete c2;
}

//------------------------------------------------------------------------------
void iCalibTAGGERvsTAPSTime::Init()
{
    // get histo name and binning
    strTaggerTAPSHistoName = this->GetConfigName("Tagger.HName");
    rebin = atoi(this->GetConfigName("Tagger.Ybin"));

    this->Help();
    //this->ReadFile( strTaggerCalibFile );

    // get time gain for Tagger chatch TDC's
    TaggerTgain = atof(this->GetConfigName("Tagger.Tgain").Data());

    // needed in ReadFile
    for (Int_t i = 0; i < MAX_TAGGER; i++)
    {
        newToffs[i] = oldToffs[i] = 0;
        mean_gaus[i] = 0;

        hTimeProj[i] = 0;
    }

    // read from database
    iMySQLManager m;
    m.ReadParameters(fSet, ECALIB_TAGG_T0, oldToffs, MAX_TAGGER);

    //   for ( Int_t i = 0; i < 5 ; i++ )
    //     {
    //       printf(" file--par_%03i = %f\n", i, oldToffs[i] );
    //     }

    return;
}

//------------------------------------------------------------------------------
void iCalibTAGGERvsTAPSTime::InitGUI()
{
    //
    c1 = new TCanvas("c1", "cHistos", 0, 0, 400, 800);
    c1->Divide(1, 2, 0.001, 0.001);
    c1->cd(1);
    c1->GetPad(1)->SetLogz();
    hTaggerVsTAPS->Draw("colz");

    // create graph
    c2 = new TCanvas("c2", "cGraph", 630, 0, 900, 400);
    hhOffset = new TH1F("hhOffset", ";Crystal Number;Time_{Tagger-TAPS} [ns]",
                        MAX_TAPS+2, -0.5, MAX_TAPS+1.5);
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
void iCalibTAGGERvsTAPSTime::Help()
{
    cout <<"*************************************************************" << endl;
    cout <<"* You are working with Time calibration modul               *" << endl;
    cout <<"*                                                           *" << endl;
    cout <<"*                                                           *" << endl;
    cout <<"*************************************************************" << endl;
    return;
}

//------------------------------------------------------------------------------
void iCalibTAGGERvsTAPSTime::GetProjection(Int_t id)
{
    if (!hTimeProj[(id-1)])
    {
        // get projection
        Char_t szName[32];
        sprintf(szName, "hTimeProj_%i", id);

        hTimeProj[(id-1)] = (TH1D*) hTaggerVsTAPS->ProjectionX(szName, id, id);
        //      hTimeProj[(id-1)] = (TH1D*) hTaggerVsTAPS->ProjectionY(szName, id, id);
    }
    return;
}

//------------------------------------------------------------------------------
void iCalibTAGGERvsTAPSTime::Calculate(Int_t id)
{
    // calculate new offset
    //
    if (hTimeProj[(id-1)]->GetEntries())
    {
        if (lOffset[(id-1)]->GetX1() != mean_gaus[(id-1)])
            mean_gaus[(id-1)] = lOffset[(id-1)]->GetX1();

        newToffs[(id-1)] = oldToffs[(id-1)] + mean_gaus[(id-1)]/TaggerTgain;

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
Bool_t iCalibTAGGERvsTAPSTime::CheckCrystalNumber(Int_t id)
{
    if (id < 1 || id > MAX_TAGGER)
    {
        cerr << "ERROR: bad number of Crystal" << endl;
        return kTRUE;
    }

    return kFALSE;
}

//------------------------------------------------------------------------------
void iCalibTAGGERvsTAPSTime::DrawThis(Int_t id)
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
void iCalibTAGGERvsTAPSTime::DrawGraph()
{
    //

    //
    c2->cd();
    hhOffset->GetYaxis()->SetRangeUser(-2, 2);
    hhOffset->Draw("E1");

    fPol0->Draw("same");
    c2->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibTAGGERvsTAPSTime::DoFor(Int_t id)
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

    //  if( id == MAX_TAGGER )
    if (!(id % 10) || id == MAX_TAGGER)
    {
        hhOffset->Fit(fPol0, "+R0", "");
        this->DrawGraph();
    }

    return;
}

//------------------------------------------------------------------------------
void iCalibTAGGERvsTAPSTime::DoFit()
{
    Int_t id = currentCrystal;
    this->DoFit(id);
    this->DrawThis(id);

    return;
}

//------------------------------------------------------------------------------
void iCalibTAGGERvsTAPSTime::DoFit(Int_t id)
{
    // Fit the TAGGER vs TAPS time histogram
    //
    Char_t szName[64];
    int maxbin;
    double sigma;
    double rms    = 0.5;
    double factor = 5.0;

    sprintf(szName, "fTime_%i", (id-1));
    fFitPeak[(id-1)] = new TF1(szName, "pol1(0)+gaus(2)");
    fFitPeak[(id-1)]->SetLineColor(2);

    if (hTimeProj[(id-1)]->GetEntries())
    {
        maxbin = hTimeProj[(id-1)]->GetMaximumBin();
        peackval = hTimeProj[(id-1)]->GetBinCenter(maxbin);

        // temporary
        mean_gaus[(id-1)] = peackval;

        // first iteration
        //
        fFitPeak[(id-1)]->SetRange(peackval -5.0, peackval +5.0);

        fFitPeak[(id-1)]->SetParameters(500.,
                                        -1.0,
                                        hTimeProj[(id-1)]->GetMaximum(),
                                        peackval,
                                        rms);

        // set limits for time resolution 700ps
        //
        fFitPeak[(id-1)]->SetParLimits(4, 0.5, 3.); // sigma
        hTimeProj[(id-1)]->Rebin(4);
        hTimeProj[(id-1)]->Fit(fFitPeak[(id-1)], "+R0Q");

        // second iteration
        //
        peackval = fFitPeak[(id-1)]->GetParameter(3);
        sigma = fFitPeak[(id-1)]->GetParameter(4);

        fFitPeak[(id-1)]->SetRange(peackval -factor*sigma,
                                   peackval +factor*sigma);

        hTimeProj[(id-1)]->Fit(fFitPeak[(id-1)], "+R0Q");

        // final results
        mean_gaus[(id-1)] = fFitPeak[(id-1)]->GetParameter(3); // store peak value
        erro_gaus[(id-1)] = fFitPeak[(id-1)]->GetParError(3);  // store peak error

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
void iCalibTAGGERvsTAPSTime::Write()
{
    // write to database
    iMySQLManager m;
    m.WriteParameters(fSet, ECALIB_TAGG_T0, newToffs, MAX_TAGGER);

    //
    // Save 2D histo and Graph
    //
    Char_t szName[256];

    //check if Directory for pictures exist, otherwise create
    Char_t szMakeDir[100];
    sprintf(szMakeDir,
            "mkdir -p %s/tagger/Tcalib/",
            this->GetConfigName("HTML.PATH").Data());
    gSystem->Exec(szMakeDir);

    // Save for 2D histo
    this->DrawThis(1);
    sprintf(szName,
            "%s/tagger/Tcalib/h2D_set%02i.gif",
            this->GetConfigName("HTML.PATH").Data(),
            fSet);
    c1->SaveAs(szName);

    // Save distribution graph
    sprintf(szName,
            "%s/tagger/Tcalib/hGr_set%02i.gif",
            this->GetConfigName("HTML.PATH").Data(),
            fSet);
    c2->SaveAs(szName);

    return;
}

// EOF
