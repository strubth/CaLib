
/*******************************************************************
 *                                                                 *
 * Date: 05.05.2009     Author: Irakli                             *
 *                                                                 *
 * This macro is used for                                          *
 *                                                                 *
 * calibbration of the Tagger                                      *
 *                                                                 *
 ******************************************************************/

#include "iCalibTaggerTime.hh"

ClassImp(iCalibTaggerTime)

//------------------------------------------------------------------------------
iCalibTaggerTime::iCalibTaggerTime()
{
    this->Init();

    if (gROOT->GetFile())
    {
        histofile = gFile;
    }
    else if (strTaggerHistoFile.Length())
    {
        histofile = TFile::Open(strTaggerHistoFile);   //Directory of root file
    }
    else
    {
        printf(" ERROR: Missing default input file!!!\n"
               "        Check in \"config.cfg\" file!\n");
        gSystem->Exit(0);
    }
    printf("\n ---------------------------------------- \n");
    printf(" Read Histo File: \"%s\"\n", histofile->GetName());


    if (!strTaggerHistoName.Length())
        hTagger = (TH2F*) histofile->Get("MyTimeTaggerVSTagger2D"); // 2D plot
    else
        hTagger = (TH2F*) histofile->Get(strTaggerHistoName.Data());   // 2D plot

    if (!hTagger)
    {
        printf("\n ERROR: The histogram named: %s can not be found!!!\n"
               "          Check in \"config.cfg\" file Tagger.HName!\n",
               strTaggerHistoName.Data());
        gSystem->Exit(0);
    }
    else
        printf(" Read Histo Name: \"%s\"\n", strTaggerHistoName.Data());

    //this->Init();
    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTaggerTime::iCalibTaggerTime(TH2F* h1)
{
    this->Init();

    hTagger = (TH2F*) h1;
    this->InitGUI();
    this->DoFor(1);
}
//------------------------------------------------------------------------------
iCalibTaggerTime::iCalibTaggerTime(Int_t set)
{
    fSet = set;
    this->Init();

    // sum up all files conteined in this runset
    iFileManager f(set, ECALIB_TAGG_T0);

    if (f.GetHistogram(strTaggerHistoName.Data()))
    {
        // point to sum histo
        hTagger= (TH2F*) f.GetHistogram(strTaggerHistoName.Data());
    }
    else
    {
        printf("\n ERROR: main histogram does not exist!!! \n");
        gSystem->Exit(0);
    }

    this->InitGUI();
    this->DoFor(1);
}
//------------------------------------------------------------------------------
iCalibTaggerTime::~iCalibTaggerTime()
{
    delete c1;
    delete c2;
}

//------------------------------------------------------------------------------
void iCalibTaggerTime::Init()
{
    this->Help();

    //
    strTaggerHistoName = *this->GetConfig("Tagger.HName");

    strTaggerCalibFile = *this->GetConfig("Tagger.Calib");

    //this->ReadFile( strTaggerCalibFile );

    // needed in ReadFile
    for (Int_t i = 0; i < iConfig::kMaxTAGGER; i++)
    {
        newOffset[i] = oldOffset[i] = 0;
        mean_gaus[i] =0;

        hTimeProj[i] = 0;

        for (Int_t j = 0; j < 13; j++)
            p[i][j] = new Char_t[16];
    }

    // read from database
    iMySQLManager m;
    m.ReadParameters(fSet, ECALIB_TAGG_T0, oldOffset, iConfig::kMaxTAGGER);


    return;
}
//------------------------------------------------------------------------------
void iCalibTaggerTime::InitGUI()
{

    c1 = new TCanvas("c1", "cHistos", 0, 0, 400, 800);
    c1->Divide(1, 2, 0.001, 0.001);
    c1->cd(1);
    c1->GetPad(1)->SetLogz();
    hTagger->Draw("colz");

    // create graph
    c2 = new TCanvas("c2", "cGraph", 630, 0, 900, 400);
    hhOffset = new TH1F("hhOffset", ";Tagger Number;Time [ns]",
                        iConfig::kMaxTAGGER, 0.5, iConfig::kMaxTAGGER+0.5);
    hhOffset->SetMarkerStyle(28);
    hhOffset->SetMarkerColor(4);
    //  hhOffset->SetStats(kFALSE);
    //  hhOffset->GetYaxis()->SetRangeUser(130, 140);
    hhOffset->Draw("P");

    fPol0 = new TF1("Pol0", "pol0", 0, 720);
    fPol0->SetParameter(0, 135.);
    fPol0->SetLineColor(2);

    return;
}
//------------------------------------------------------------------------------
void iCalibTaggerTime::Help()
{
    cout <<"************************************************************" << endl;
    cout <<"* You are working with Time calibration modul              *" << endl;
    cout <<"*                                                          *" << endl;
    cout <<"************************************************************" << endl;
    return;
}

//------------------------------------------------------------------------------
void iCalibTaggerTime::GetProjection(Int_t id)
{
    if (!hTimeProj[(id-1)])
    {
        // get projection
        Char_t szName[32];
        sprintf(szName, "hTimeProj_%i", id);

        //       hTimeProj[(id-1)] = (TH1D*) hTagger->ProjectionY(szName, id, id, "e");
        hTimeProj[(id-1)] = (TH1D*) hTagger->ProjectionX(szName, id, id);

        hTimeProj[(id-1)]->Rebin(4);
    }
    return;
}

//------------------------------------------------------------------------------
void iCalibTaggerTime::Calculate(Int_t id)
{
    // calculate new gain
    //
    if (hTimeProj[(id-1)]->GetEntries())
    {
        if (lOffset[(id-1)]->GetX1() != mean_gaus[(id-1)])
            mean_gaus[(id-1)] = lOffset[(id-1)]->GetX1();

        newOffset[(id-1)] = (oldOffset[(id-1)] +
                             mean_gaus[(id-1)]/0.1170
                             + 869.5); // not good cas 0

        printf("Element: %03i peak = %10.6f \t newOffset = %10.6f \t"
               " oldOffset = %10.6f - NEW\n",
               //       id, mean_gaus[(id-1)], newOffset[(id-1)], TaggerOffset[(id-1)] );
               id, mean_gaus[(id-1)], newOffset[(id-1)], oldOffset[(id-1)]);

        hhOffset->SetBinContent(id, mean_gaus[(id-1)]);
        hhOffset->SetBinError(id, 1.);
    }
    else
    {
        newOffset[(id-1)] =  oldOffset[(id-1)];
        printf("Element: %03i peak = %10.6f \t newOffset = %10.6f \t "
               "oldOffset = %10.6f - OLD\n",
               id, mean_gaus[(id-1)], newOffset[(id-1)], oldOffset[(id-1)]);
    }
    return;
}

//------------------------------------------------------------------------------
Bool_t iCalibTaggerTime::CheckCrystalNumber(Int_t id)
{
    if (id < 1 || id > iConfig::kMaxTAGGER)
    {
        cerr << "ERROR: bad number of Crystal" << endl;
        return kTRUE;
    }

    return kFALSE;
}

//------------------------------------------------------------------------------
void iCalibTaggerTime::DrawThis(Int_t id)
{
    c1->cd(2);
    hTimeProj[(id-1)]->GetXaxis()->SetRangeUser(peackval -50., peackval +50.);
    hTimeProj[(id-1)]->Draw();

    if (fPol0Gaus[(id-1)])
    {
        fPol0Gaus[(id-1)]->Draw("same");
    }
    lOffset[(id-1)]->Draw();

    c1->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibTaggerTime::DrawGraph()
{
    //
    c2->cd();
    hhOffset->Draw("E1");

    fPol0->Draw("same");
    c2->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibTaggerTime::DoFor(Int_t id)
{
    if (CheckCrystalNumber(id))
        return;

    currentCrystal = id;

    this->GetProjection(id);

    if ((hTimeProj[(id-1)])->GetEntries() > 10)
        this->DoFit(id);
    //this->FitTAGGtime( id , hTimeProj[(id-1)]);

    this->DrawThis(id);

    if (!(id % 10) || id == iConfig::kMaxTAGGER)
    {
        hhOffset->Fit(fPol0, "+R0", "");
        this->DrawGraph();
    }

    return;
}
//------------------------------------------------------------------------------
void iCalibTaggerTime::DoFit()
{
    Int_t id = currentCrystal;
    this->DoFit(id);
    this->DrawThis(id);

    return;
}

//------------------------------------------------------------------------------
void iCalibTaggerTime::DoFit(Int_t id)
{
    // Fit the tagger time histogram
    //
    int maxbin;
    double sigma;
    double rms;
    double factor = 3.0;
    Char_t szName[256];

    sprintf(szName, "fTime_%i", (id-1));
    fGaus[(id-1)] = new TF1(szName, "gaus");
    fGaus[(id-1)]->SetLineColor(2);

    maxbin = hTimeProj[(id-1)]->GetMaximumBin();
    peackval = hTimeProj[(id-1)]->GetBinCenter(maxbin);

    // temporary
    mean_gaus[(id-1)] = peackval;
    rms = hTimeProj[(id-1)]->GetRMS();

    // first iteration
    fGaus[(id-1)]->SetRange(peackval -0.8, peackval +0.8);
    fGaus[(id-1)]->SetParameters(hTimeProj[(id-1)]->GetMaximum(), peackval, 10);

    fGaus[(id-1)]->SetParLimits(3, 0.8, 20.0); // sigma

    hTimeProj[(id-1)]->Fit(fGaus[(id-1)], "+R0Q");

    // second iteration
    peackval = fGaus[(id-1)]->GetParameter(1);
    sigma = fGaus[(id-1)]->GetParameter(2);

    fGaus[(id-1)]->SetRange(peackval -factor*sigma,
                            peackval +factor*sigma);
    hTimeProj[(id-1)]->Fit(fGaus[(id-1)], "+RQ");

    // final results
    mean_gaus[(id-1)] = fGaus[(id-1)]->GetParameter(1); // store peak value
    erro_gaus[(id-1)] = fGaus[(id-1)]->GetParError(1);  // store peak error

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
    return;

}

// EOF

