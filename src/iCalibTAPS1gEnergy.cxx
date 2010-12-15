
/*******************************************************************
 *                                                                 *
 * Date: 05.01.2010     Author: Irakli                             *
 *                                                                 *
 * This macro is used for the Pi0 invariant mass based energy      *
 *                                                                 *
 * calibbration of the Crystal Ball detector.                      *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// A module for TAPS pi0 calibration using 1g(CB) and 1g(TAPS)                //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


#include "iCalibTAPS1gEnergy.hh"

ClassImp(iCalibTAPS1gEnergy)

//------------------------------------------------------------------------------
iCalibTAPS1gEnergy::iCalibTAPS1gEnergy()
{
    this->Init();

    // print help
    this->Help();

    // Check if file is attached
    if (gROOT->GetFile())
    {
        histofile = (TFile*) gFile;
    }
    // OR default file from config.cfg
    else if (strTAPSHistoFile.Length())
    {
        histofile = TFile::Open(strTAPSHistoFile);
    }
    //
    else
    {
        printf(" ERROR: Missing default input file!!!\n"
               "        Check in \"config.cfg\" file!\n");
        gSystem->Exit(0);
    }
    printf("\n ---------------------------------------- \n");
    printf(" Read Histo File: \"%s\"\n", histofile->GetName());

    // check if histo name is defined
    if (!strTAPSHistoName.Length())
        hTAPS1gIM = (TH2F*) histofile->Get("CalibTAPS1g_IM");
    else
        hTAPS1gIM = (TH2F*) histofile->Get(strTAPSHistoName.Data());

    if (!hTAPS1gIM)
    {
        printf(" ERROR: The histogram named: %s can not be found!!!\n"
               "        Check in \"config.cfg\" file TAPS.HName!\n",
               strTAPSHistoName.Data());
        gSystem->Exit(0);
    }
    else
        printf(" Read Histo Name: \"%s\"\n", strTAPSHistoName.Data());

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAPS1gEnergy::iCalibTAPS1gEnergy(Int_t set)
{
    fSet = set;
    this->Init();

    // sum up all files conteined in this runset
    this->DoForSet(set, ECALIB_TAPS_LG_E1, strTAPSHistoName);

    if (hTAPS1gIM)
    {
        hTAPS1gIM->Delete();
        hTAPS1gIM=0;
    }

    hTAPS1gIM = (TH2F*) this->GetMainHisto();

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAPS1gEnergy::iCalibTAPS1gEnergy(Int_t run, TH2F* h1)
{
    fRun = run;
    this->Init();

    // print help
    this->Help();

    hTAPS1gIM = (TH2F*) h1;

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibTAPS1gEnergy::~iCalibTAPS1gEnergy()
{
    delete c1;
    delete c2;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::Init()
{
    //
    strTAPSHistoName = *this->GetConfig("TAPS.HEName");

    // read from database
    iMySQLManager m;
    m.ReadParameters(fSet, ECALIB_TAPS_LG_E1, oldGain, iConfig::kMaxTAPS);
    m.ReadParameters(fSet, ECALIB_TAPS_PI0IM, newPi0IM, iConfig::kMaxTAPS);

    // Initialize all variables in this class
    // needed in ReadFile
    for (Int_t i = 0; i < iConfig::kMaxTAPS; i++)
    {
        newGain[i] = oldGain[i]; //

        hIMProj[i] = 0; //
    }

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::InitGUI()
{
    //
    c1 = new TCanvas("c1", "Histos", 0, 0, 400, 800);
    c1->Divide(1, 2, 0.001, 0.001);
    c1->cd(1);
    c1->GetPad(1)->SetLogz();
    hTAPS1gIM->Draw("colz");

    // create graph
    Char_t szTitle[56];
    c2 = new TCanvas("c2", "Graph", 430, 0, 900, 400);
    sprintf(szTitle,
            "Set of runs N %02i;NaI Number;#pi^{0} IM [MeV]",
            fSet);
    hhIM = new TH1F("hhIM", szTitle,
                    iConfig::kMaxTAPS, 0.5, iConfig::kMaxTAPS+0.5);
    hhIM->SetMarkerStyle(28);
    hhIM->SetMarkerColor(4);
    //  hhIM->SetStats(kFALSE);
    hhIM->GetYaxis()->SetRangeUser(120, 150);
    hhIM->Draw("P");

    fPol0 = new TF1("Pol0", "pol0", 0, iConfig::kMaxTAPS);
    fPol0->SetParameter(0, 135.);
    fPol0->SetLineColor(2);

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::Help()
{
    cout << endl;
    cout << "***********************************************************" << endl;
    cout << "* You are working with TAPS Energy calibration modul      *" << endl;
    cout << "*                                                         *" << endl;
    cout << "*                                                         *" << endl;
    cout << "*                                                         *" << endl;
    cout << "***********************************************************" << endl;
    cout << endl;

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::DrawThis(Int_t id)
{
    // draw all histograms on 2 canvas
    //
    c1->cd(2);
    if (hIMProj[(id-1)])
    {
        hIMProj[(id-1)]->GetXaxis()->SetRangeUser(0, 500);
        hIMProj[(id-1)]->SetLineColor(48);
        hIMProj[(id-1)]->Draw();
        //      hIMProj[(id-1)]->Draw("E1same");
    }

    if (fFitPeak[(id-1)])
    {
        fFitPeak[(id-1)]->SetLineColor(2);
        fFitPeak[(id-1)]->Draw("same");

        lOffset[(id-1)]->Draw("same");
    }
    c1->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::DrawGraph()
{
    //
    c2->cd();
    hhIM->SetLineColor(46);
    hhIM->Draw("E1");

    fPol0->Draw("same");
    c2->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::GetProjection(Int_t id)
{
    // get projection from 2D histogram
    //
    if (!hIMProj[(id-1)])
    {
        Char_t szName[32];
        sprintf(szName, "hIMProj_%i", id);
        hIMProj[(id-1)] = (TH1D*) hTAPS1gIM->ProjectionX(szName, id, id);
        hIMProj[(id-1)] ->Rebin(8);
    }

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::Calculate(Int_t id)
{
    // calculate new gain
    //
    if (hIMProj[(id-1)]->GetEntries() > 100)
    {
        // check if you shift blue line
        // if changed take blue line position
        if (lOffset[(id-1)]->GetX1() != mean_gaus[(id-1)])
            mean_gaus[(id-1)] = lOffset[(id-1)]->GetX1();

        newPi0IM[(id-1)] = mean_gaus[(id-1)];

        // oldGain * PI0_MASS / pos[currElement];
        newGain[(id-1)] = oldGain[(id-1)] * (iConfig::kPi0Mass / mean_gaus[(id-1)]);

        // if new value is negative take old
        if (newGain[(id-1)] < 0)
            newGain[(id-1)] = oldGain[(id-1)];

        //      if( id == 1 || !(id % 100) || id == iConfig::kMaxTAPS )
        printf("Element: %03i \t new = %f \t old = %f \t %f \n",
               id, newGain[(id-1)], oldGain[(id-1)], mean_gaus[(id-1)]);

        hhIM->SetBinContent(id, mean_gaus[(id-1)]);
        hhIM->SetBinError(id, fFitPeak[(id-1)]->GetParError(3));
    }
    //   else
    //     {
    //       printf( "Element: %03i \t new = %f \t old = %f \n",
    //          id, 0.0, oldGain[(id-1)] );
    //     }

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::DoFor(Int_t id)
{
    // go through all crystals
    //
    currentCrystal = id;
    this->GetProjection(id);

    this->DoFit(id);

    this->DrawThis(id);

    if (!(id % 20) || id == iConfig::kMaxTAPS)
    {
        this->DrawGraph();
    }

    if (id == iConfig::kMaxTAPS)
        hhIM->Fit(fPol0, "+R0", "");

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::DoFit()
{
    Int_t id = currentCrystal;
    this->DoFit(id);
    this->DrawThis(id);

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::DoFit(Int_t id)
{
    // method for manual fitting
    //

    char szName[32];
    Int_t maxbin;

    if ((hIMProj[(id-1)])->GetEntries())
    {
        // Fit the Pi0 peak of an element in the corresponding histogram

        sprintf(szName, "fPi0_%i", (id-1));
        fFitPeak[(id-1)] = new TF1(szName, "pol1+gaus(2)");
        fFitPeak[(id-1)] ->SetLineColor(2);

        //
        if (!newPi0IM[(id-1)])
        {
            maxbin = hIMProj[(id-1)]->GetMaximumBin();
            peackval = hIMProj[(id-1)]->GetBinCenter(maxbin);
        }
        else
        {
            peackval = newPi0IM[(id-1)];
        }

        if (peackval < 70 || peackval > 160)
            peackval = 135.;

        Double_t* BG = this->FindBG(hIMProj[(id-1)], peackval);

        double low = peackval-20.;
        double upp = peackval+20.;

        fFitPeak[(id-1)]->SetName(szName);
        fFitPeak[(id-1)]->SetRange(peackval-50, peackval+50);

        fFitPeak[(id-1)]->SetLineColor(2);

        fFitPeak[(id-1)]->SetParameters(BG[1], BG[0],
                                        hIMProj[(id-1)]->GetMaximum(), peackval, 9.);

        fFitPeak[(id-1)]->SetParLimits(0, 0, 1.e+6);
        fFitPeak[(id-1)]->SetParLimits(1, -100, 0);

        fFitPeak[(id-1)]->SetParLimits(2, 0, 1.e+6);
        fFitPeak[(id-1)]->SetParLimits(3, low, upp);
        fFitPeak[(id-1)]->SetParLimits(4, 2, 20);

        hIMProj[(id-1)]->Fit(fFitPeak[(id-1)], "+R0Q");

        mean_gaus[(id-1)] = fFitPeak[(id-1)]->GetParameter(3); // store peak value
        erro_gaus[(id-1)] = fFitPeak[(id-1)]->GetParError(3);  // store peak error

        //
        lOffset[(id-1)]->SetVertical();
        lOffset[(id-1)]->SetLineColor(4);
        lOffset[(id-1)]->SetLineWidth(3);
        lOffset[(id-1)]->SetY1(0);
        lOffset[(id-1)]->SetY2(hIMProj[(id-1)]->GetMaximum() + 20);

        // check if mass is in normal range
        if (mean_gaus[(id-1)] > 10 ||
                mean_gaus[(id-1)] < 200)
        {
            lOffset[(id-1)]->SetX1(mean_gaus[(id-1)]);
            lOffset[(id-1)]->SetX2(mean_gaus[(id-1)]);
        }
        else
        {
            mean_gaus[(id-1)] = 135.0;

            lOffset[(id-1)]->SetX1(mean_gaus[(id-1)]);
            lOffset[(id-1)]->SetX2(mean_gaus[(id-1)]);
        }

        newPi0IM[(id-1)] = mean_gaus[(id-1)];

    }

    this->DrawThis(id);

    return;
}

//------------------------------------------------------------------------------
void iCalibTAPS1gEnergy::Write()
{

    // write to database
    iMySQLManager m;
    m.WriteParameters(fSet, ECALIB_TAPS_LG_E1, newGain, iConfig::kMaxTAPS);
    m.WriteParameters(fSet, ECALIB_TAPS_PI0IM, newPi0IM, iConfig::kMaxTAPS);

    //
    //
    Char_t szName[100];
    sprintf(szName,
            "%s/taps/Ecalib/hGr_set%02i.gif",
            this->GetConfig("HTML.PATH")->Data(),
            fSet);
    //check if Directory for pictures exist, otherwise create
    Char_t szMakeDir[100];
    sprintf(szMakeDir,
            "mkdir -p %s/taps/Ecalib/",
            this->GetConfig("HTML.PATH")->Data());
    gSystem->Exec(szMakeDir);

    c2->SaveAs(szName);

    return;
}

// EOF
