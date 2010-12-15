
/*******************************************************************
 *                                                                 *
 * Date: 30.03.2009     Author: Irakli                             *
 *                                                                 *
 * This macro is used for the Pi0 invariant mass based energy      *
 *                                                                 *
 * calibbration of the Crystal Ball detector.                      *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// A module for CB pi0 calibration using 2g in CB.                            //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


#include "iCalibCBpi0Energy.hh"

ClassImp(iCalibCBpi0Energy)

//------------------------------------------------------------------------------
iCalibCBpi0Energy::iCalibCBpi0Energy()
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
    else if (strCBHistoFile.Length())
    {
        histofile = TFile::Open(strCBHistoFile);
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
    //
    if (!strCBHistoName.Length())
        hCB2gIM = (TH2F*) histofile->Get("MyCBIMHistoAll");
    else
        hCB2gIM = (TH2F*) histofile->Get(strCBHistoName.Data());

    if (!hCB2gIM)
    {
        printf(" ERROR: The histogram named: %s can not be found!!!\n"
               "        Check in \"config/CB.cfg\" file CB.HName!\n",
               strCBHistoName.Data());
        gSystem->Exit(0);
    }
    else
        printf(" Read Histo Name: \"%s\"\n", strCBHistoName.Data());

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibCBpi0Energy::iCalibCBpi0Energy(Int_t set)
{
    fSet = set;
    this->Init();

    // sum up all files conteined in this runset
    this->DoForSet(set, ECALIB_CB_E1, strCBHistoName);

    // delete empty histo
    if (hCB2gIM)
    {
        hCB2gIM->Delete();
        hCB2gIM=0;
    }
    if (this->GetMainHisto())
    {
        // point to sum histo
        hCB2gIM = (TH2F*) this->GetMainHisto();
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
iCalibCBpi0Energy::iCalibCBpi0Energy(Int_t run, TH2F* h1)
{
    fRun = run;
    this->Init();

    // print help
    this->Help();

    hCB2gIM = (TH2F*) h1;

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibCBpi0Energy::~iCalibCBpi0Energy()
{
    delete c1;
    delete c2;
}

//------------------------------------------------------------------------------
void iCalibCBpi0Energy::Init()
{
    //
    strCBHistoName = *this->GetConfig("CB.HEName");

    strCBCalibFile = *this->GetConfig("CB.Calib");

    // Initialize all variables in this class
    // needed in ReadFile
    for (Int_t i = 0; i < iConfig::kMaxCB; i++)
    {
        newGain[i] = oldGain[i] = 0; //

        hIMProj[i] = 0; //
    }

    // read from database
    iMySQLManager m;
    m.ReadParameters(fSet, ECALIB_CB_E1, oldGain, iConfig::kMaxCB);
    m.ReadParameters(fSet, ECALIB_CB_E1, newGain, iConfig::kMaxCB);
    m.ReadParameters(fSet, ECALIB_CB_PI0IM, oldPi0IM, iConfig::kMaxCB);


    //   for ( Int_t i = 0; i < 5 ; i++ )
    //     {
    //       printf(" file--par_%03i = %f\n", i, oldGain[i] );
    //     }


    //   // if file is set in config.cfg
    //   // try to read pars from DB
    //   // if not from database
    //   //
    //   if( strCBCalibFile.Length() )
    //     {
    //       // read NaI.dat config file
    //       //
    //       this->ReadFile( strCBCalibFile );

    //       for (Int_t i = 0; i < MAX_CRYSTAL; i++)
    //    oldGain[i] = CBgain[i];
    //     }
    //   else if( fSet )
    //     {
    //       // read from database
    //       this->ReadCBpar4set( fSet, oldGain );
    //     }
    //   else
    //     {
    //       printf("\n ERROR: reading CB par-s!!! \n");
    //       gSystem->Exit(0);
    //     }

    //   for (Int_t i = 0; i < MAX_CRYSTAL; i++)
    //     {
    //       printf(" file--par_%03i = %f", i, CBgain[i] );
    //       printf("  sql--par_%03i = %f\n", i, oldGain[i] ); //
    //     }
    return;
}

//------------------------------------------------------------------------------
//-----------------------------------------------------------------
void iCalibCBpi0Energy::InitGUI()
{
    //
    c1 = new TCanvas("c1", "Histos", 0, 0, 400, 800);
    c1->Divide(1, 2, 0.001, 0.001);
    c1->cd(1);
    c1->GetPad(1)->SetLogz();
    hCB2gIM->Draw("colz");

    // create graph
    Char_t szTitle[56];
    c2 = new TCanvas("c2", "Graph", 430, 0, 900, 400);
    sprintf(szTitle,
            "Set of runs N %02i;NaI Number;#pi^{0} IM [MeV]",
            fSet);
    hhIM = new TH1F("hhIM", szTitle,
                    iConfig::kMaxCB, 0.5, iConfig::kMaxCB+0.5);
    hhIM->SetMarkerStyle(28);
    hhIM->SetMarkerColor(4);
    //  hhIM->SetStats(kFALSE);

    Double_t low =  atof(this->GetConfig("CBenergyGraph.low")->Data());
    Double_t upp =  atof(this->GetConfig("CBenergyGraph.upp")->Data());

    if (low || upp)
        hhIM->GetYaxis()->SetRangeUser(low, upp);
    hhIM->Draw("P");

    fPol0 = new TF1("Pol0", "pol0", 0, iConfig::kMaxCB);
    fPol0->SetParameter(0, 135.);
    fPol0->SetLineColor(2);

    return;
}

//------------------------------------------------------------------------------
void iCalibCBpi0Energy::Help()
{
    cout << endl;
    cout << "****************************************************************" << endl;
    cout << "* You are working with CB Energy calibration modul             *" << endl;
    cout << "*                                                              *" << endl;
    cout << "*                                                              *" << endl;
    cout << "*                                                              *" << endl;
    cout << "****************************************************************" << endl;
    cout << endl;

    return;
}

//------------------------------------------------------------------------------
void iCalibCBpi0Energy::DrawThis(Int_t id)
{
    // draw all histograms on 2 canvas
    //
    c1->cd(2);
    if (hIMProj[(id-1)])
    {
        hIMProj[(id-1)]->GetXaxis()->SetRangeUser(0, 300);
        hIMProj[(id-1)]->Draw();
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
void iCalibCBpi0Energy::DrawGraph()
{
    //
    c2->cd();
    hhIM->Draw("E1");

    fPol0->Draw("same");
    c2->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibCBpi0Energy::GetProjection(Int_t id)
{
    // get projection from 2D histogram
    //
    if (!hIMProj[(id-1)])
    {
        Char_t szName[32];
        sprintf(szName, "hIMProj_%i", id);
        hIMProj[(id-1)] = (TH1D*) hCB2gIM->ProjectionX(szName, id, id, "e");
        hIMProj[(id-1)] ->Rebin(4);
    }

    return;
}

//------------------------------------------------------------------------------
void iCalibCBpi0Energy::Calculate(Int_t id)
{
    // calculate new gain
    //
    if (hIMProj[(id-1)]->GetEntries() > 80)
    {
        // check if you shift blue line
        // if changed take blue line position
        if (lOffset[(id-1)]->GetX1() != mean_gaus[(id-1)])
            mean_gaus[(id-1)] = lOffset[(id-1)]->GetX1();

        if (TMath::Abs(mean_gaus[(id-1)]) >= 1000.) mean_gaus[(id-1)] = 135.;
        newPi0IM[(id-1)] = mean_gaus[(id-1)];

        // oldGain * PI0_MASS / pos[currElement];
        newGain[(id-1)] = oldGain[(id-1)] * (iConfig::kPi0Mass / mean_gaus[(id-1)]);

        // if new value is negative take old
        if (newGain[(id-1)] < 0)
            newGain[(id-1)] = oldGain[(id-1)];

        //      if( id == 1 || !(id % 100) || id == MAX_CB )
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
void iCalibCBpi0Energy::DoFor(Int_t id)
{
    // go through all crystals
    //
    currentCrystal = id;
    this->GetProjection(id);

    this->DoFit(id);

    this->DrawThis(id);

    if (!(id % 20) || id == iConfig::kMaxCB)
    {
        this->DrawGraph();
    }

    if (id == iConfig::kMaxCB)
        hhIM->Fit(fPol0, "+R0", "");

    return;
}

//------------------------------------------------------------------------------
void iCalibCBpi0Energy::DoFit()
{
    Int_t id = currentCrystal;
    this->DoFit(id);
    this->DrawThis(id);

    return;
}


//------------------------------------------------------------------------------
void iCalibCBpi0Energy::DoFit(Int_t id)
{

    // Fit the Pi0 peak of an element in the corresponding histogram

    Char_t szName[64];

    sprintf(szName, "fPi0_%i", (id-1));
    fFitPeak[(id-1)] = new TF1(szName, "pol1+gaus(2)");
    fFitPeak[(id-1)] ->SetLineColor(2);

    if ((hIMProj[(id-1)])->GetEntries())
    {
        //
        Int_t maxbin =  hIMProj[(id-1)]->GetMaximumBin();
        peackval =  hIMProj[(id-1)]->GetBinCenter(maxbin);

        if (peackval < 100 || peackval > 160)
            peackval = 135.;

        Double_t* BG = this->FindBG(hIMProj[(id-1)], peackval);

        fFitPeak[(id-1)]->SetName(szName);
        fFitPeak[(id-1)]->SetRange(peackval-70, peackval+50);

        fFitPeak[(id-1)]->SetLineColor(2);

        fFitPeak[(id-1)]->SetParameters(BG[1], BG[0],  hIMProj[(id-1)]->GetMaximum(), peackval, 9.);
        //fFitPeak[(id-1)]->SetParLimits(3, 50., 200.); //mean
        fFitPeak[(id-1)]->SetParLimits(4, 5., 150); // sigma

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
            lOffset[(id-1)]->SetX1(135.);
            lOffset[(id-1)]->SetX2(135.);
        }
    }

    return;
}

//------------------------------------------------------------------------------
void iCalibCBpi0Energy::Write()
{
    //

    // write to database
    iMySQLManager m;
    m.WriteParameters(fSet, ECALIB_CB_E1, newGain, iConfig::kMaxCB);
    m.WriteParameters(fSet, ECALIB_CB_PI0IM, newPi0IM, iConfig::kMaxCB);


    //
    //save pictures
    Char_t szName[100];
    sprintf(szName,
            "%s/cb/Ecalib/hGr_set%02i.gif",
            this->GetConfig("HTML.PATH")->Data(),
            fSet);
    //check if Directory for pictures exist, otherwise create
    Char_t szMakeDir[100];
    sprintf(szMakeDir,
            "mkdir -p %s/cb/Ecalib/",
            this->GetConfig("HTML.PATH")->Data());
    gSystem->Exec(szMakeDir);

    c2->SaveAs(szName);

    return;
}

// EOF
