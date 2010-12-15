
/*******************************************************************
 *                                                                 *
 * Date: 24.01.2010     Author: Irakli                             *
 *                                                                 *
 * Time Walk Calibration module                                    *
 *                                                                 *
 ******************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// A module for CB time walk calibration.                                     //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "iCalibCBTimeWalk.hh"

ClassImp(iCalibCBTimeWalk)

//------------------------------------------------------------------------------
Double_t TWFunc(Double_t* x, Double_t* par)
{
    return par[0] + par[1] / TMath::Power(x[0] + par[2], par[3]);
}

//------------------------------------------------------------------------------
iCalibCBTimeWalk::iCalibCBTimeWalk()
{
    this->Init();

    if (gROOT->GetFile())
        histofile = gFile;

    printf("\n ---------------------------------------- \n");
    printf(" Read Histo File: \"%s\"\n", histofile->GetName());

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibCBTimeWalk::iCalibCBTimeWalk(Int_t set)
{
    fSet = set;

    this->Init();

    // sum up all files conteined in this runset
    fFileManager = new iFileManager(set, ECALIB_CB_WALK0);

    this->InitGUI();
    this->DoFor(1);
}

//------------------------------------------------------------------------------
iCalibCBTimeWalk::iCalibCBTimeWalk(Int_t set, TH2F* h1)
{
    //
}

//------------------------------------------------------------------------------
iCalibCBTimeWalk::~iCalibCBTimeWalk()
{
    delete c1;
    delete c2;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::Init()
{
    // get histogram name
    strHName = *this->GetConfig("CB.HTWName:");

    //
    for (int i=0; i<iConfig::kMaxCB; i++)
    {
        TWalk0[i] = 0;
        TWalk1[i] = 0;
        TWalk2[i] = 0;
        TWalk3[i] = 0;

        hhTWalk[i] = 0;
        hhEWpro[i] = 0;
        hhTWpro[i] = 0;

        fTWalk[i] = 0;
    }

    // read from database
    iMySQLManager m;
    m.ReadParameters(fSet, ECALIB_CB_WALK0, TWalk0, iConfig::kMaxCB);
    m.ReadParameters(fSet, ECALIB_CB_WALK1, TWalk1, iConfig::kMaxCB);
    m.ReadParameters(fSet, ECALIB_CB_WALK2, TWalk2, iConfig::kMaxCB);
    m.ReadParameters(fSet, ECALIB_CB_WALK3, TWalk3, iConfig::kMaxCB);


    for (int i=0; i<3 ; i++)
    {
        printf(" TWalk %03i %f %f %f %f\n",
               i, TWalk0[i], TWalk1[i], TWalk2[i], TWalk3[i]);
    }

    return;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::InitGUI()
{
    //   this->Help();

    //   //
    c1 = new TCanvas("c1", "cHistos", 0, 0, 600, 800);
    //  c1->Divide(1, 3, 0.001, 0.001);
    c1->Divide(1, 2, 0.001, 0.001);
    c1->cd(1);

    // create graph
    c2 = new TCanvas("c2", "cGraph", 650, 0, 700, 400);

    return;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::Help()
{
    cout <<"************************************************************" << endl;
    cout <<"* You are working with Time walk calibration modul         *" << endl;
    cout <<"*                                                          *" << endl;
    cout <<"*                                                          *" << endl;
    cout <<"************************************************************" << endl;

    return;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::GetProjection(Int_t id)
{
    return;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::Calculate(Int_t id)
{
    // not needed for time calib
    return;
}

//------------------------------------------------------------------------------
Bool_t iCalibCBTimeWalk::CheckCrystalNumber(Int_t id)
{
    if (id < 1 || id > iConfig::kMaxCB)
    {
        cerr << "ERROR: wrong number of Crystal" << endl;
        return kTRUE;
    }

    return kFALSE;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::DrawThis(Int_t id)
{
    // draw projection with fit function and line

    if (hhTWalk[(id-1)]->GetEntries() > 15)
    {
        c1->cd(1);
        c1->GetPad(1)->SetLogz(1);
        hhTWalk[(id-1)]->Draw("Colz");
        //      hhTWalk[(id-1)]->Draw("Colz;Cont2");

        if (hhEWpro[(id-1)] && fTWalk[(id-1)])
        {
            fTWalk[(id-1)]->SetLineColor(13);
            fTWalk[(id-1)]->Draw("same");

            hhEWpro[(id-1)]->SetMarkerStyle(20);
            hhEWpro[(id-1)]->SetMarkerColor(4);
            hhEWpro[(id-1)]->Draw("sameE");
        }

        //
        c1->cd(2);
        if (hhTWpro[(id-1)])
        {
            hhTWpro[(id-1)]->SetFillColor(32);
            hhTWpro[(id-1)]->Draw();
            lOffset[(id-1)]->Draw();
        }

        //       c1->cd(3);
        //       if( hhEWpro[(id-1)] && fTWalk[(id-1)] )
        //    {
        //      hhEWpro[(id-1)]->Draw("E");

        //      fTWalk[(id-1)]->Draw("same");
        //    }
    }

    c1->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::DrawGraph()
{
    //
    c2->cd();

    Double_t xlow = 10;
    Double_t xupp = 700;
    Double_t ylow = 0;
    Double_t yupp = 0;

    if (hhEWpro[(currentCrystal-1)])
    {
        hhEWpro[(currentCrystal-1)]->GetXaxis()->SetRangeUser(xlow, xupp);

        ylow = fTWalk[(currentCrystal-1)]->Eval(xupp) -5.0;
        yupp = fTWalk[(currentCrystal-1)]->Eval(xlow) +20.0;

        hhEWpro[(currentCrystal-1)]->GetYaxis()->SetRangeUser(ylow, yupp);
        //      hhEWpro[(currentCrystal-1)]->GetYaxis()->SetRangeUser( 0, 150 );

        hhEWpro[(currentCrystal-1)]->Draw("E");

        if (fTWalk[(currentCrystal-1)])
            fTWalk[(currentCrystal-1)]->Draw("same");
    }
    c2->Update();

    return;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::DoFor(Int_t id)
{
    //
    //
    //

    printf(" - - - TWalk for NaI : %03i - - - \n", id);

    // check if such crystal exists
    //
    if (CheckCrystalNumber(id))
        return;
    currentCrystal = id;

    Char_t szName[32];

    Double_t low = 0;   // range for twalk fit
    Double_t upp = 700;

    // check if 2D TWalk histo is extracted
    //
    if (!hhTWalk[(id-1)])
    {
        // get TWalk histo
        //
        TString strTmp;
        char nnn[4];
        strTmp = strHName.Copy();
        sprintf(nnn, "%03i", (id-1));
        strTmp.ReplaceAll("NNN", nnn);

        printf(" - - -           Histo : %s - - - \n", strTmp.Data());

        // check if runset is called
        //
        if (fSet)
        {
            //if (fMainHisto) delete fMainHisto;
            //fMainHisto = (TH2*) this->GetHistogram("no be added");
        }
        else
        {
            hhTWalk[(id-1)] = (TH2F*) histofile->Get(strTmp);   // 2D plot
        }
        //      hhTWalk[(id-1)]->RebinX( 4 );

        // check if time peaks are extracted
        // and if statistic is enough in 2D TvsE histo
        //
        if (!hhEWpro[(id-1)] && hhTWalk[(id-1)]->GetEntries() > 1.E+3)
        {
            // for TWalk profile
            //
            sprintf(szName, "hTWalk%03i", (id-1));
            //      hhEWpro[(id-1)] = hhTWalk[(id-1)]->ProjectionX(szName, 80, 100);
            hhEWpro[(id-1)] = hhTWalk[(id-1)]->ProjectionX(szName);
            //      hhEWpro[(id-1)]->Reset();

            // loop over all energy bins
            //
            for (int i=0; i<hhEWpro[(id-1)]->GetNbinsX(); i++)
            {
                if (hhEWpro[(id-2)])
                {
                    hhEWpro[(id-2)]->Reset();
                    hhEWpro[(id-2)]->Delete();
                    hhEWpro[(id-2)] = 0;
                }

                // check if projection has enough entries
                //
                if ((hhEWpro[(id-1)]->GetBinContent(i)) > 30.)
                    // if( ( hhEWpro[(id-1)]->Integral() ) > 150. )
                    // if( ( hhEWpro[(id-1)]->GetMaximum() ) > 10. )
                {
                    // find fit range
                    //
                    if (!low && hhEWpro[(id-1)]->GetBinContent(i-1))
                        low = hhEWpro[(id-1)]->GetBinLowEdge(i);
                    else
                        upp = hhEWpro[(id-1)]->GetBinLowEdge(i+1);

                    //          low = 1; // !!!!!!!!!!!!!!!!!!!!!

                    sprintf(szName, "hTProj");
                    hhTWpro[(id-1)] = hhTWalk[(id-1)]->ProjectionY(szName, i, i);

                    //          this->FitCBtwalk( id , hhTWpro[(id-1)]);
                    this->DoFit(id);

                    if (erro_gaus[(id-1)] < 1.)
                    {
                        hhEWpro[(id-1)]->SetBinContent(i, mean_gaus[(id-1)]);
                        hhEWpro[(id-1)]->SetBinError(i, erro_gaus[(id-1)]);
                    }

                    //          if( !i )
                    //            this->DrawThis(id);

                    if (!(i%1))
                    {
                        hhTWpro[(id-1)]->GetXaxis()->SetRangeUser(-100, 100);
                        c1->cd(2);
                        if (hhTWpro[(id-1)])
                        {
                            hhTWpro[(id-1)]->Draw();
                            lOffset[(id-1)]->Draw();
                            c1->Update();
                        }
                    }
                }
                else
                {
                    hhEWpro[(id-1)]->SetBinContent(i, 0);
                    hhEWpro[(id-1)]->SetBinError(i, 0);
                }
            }
        }
    }

    // delete N'th previous histo
    //
    int NN = 10;
    if ((id>NN) && hhTWalk[(id-10)])
    {
        hhTWalk[(id-NN)]->Delete();
        hhTWalk[(id-NN)] = 0;
    }

    // check if gaus profile exist
    //
    if (hhEWpro[(id-1)])
    {
        // check if fit function exist
        //
        if (!fTWalk[(id-1)])
        {
            sprintf(szName, "fTWalk_%03i", id);
            fTWalk[(id-1)] = new TF1(szName, ::TWFunc, low, upp, 4);

            fTWalk[(id-1)]->SetParameters(TWalk0[(id-1)],
                                          TWalk1[(id-1)],
                                          TWalk2[(id-1)],
                                          TWalk3[(id-1)]);

            fTWalk[(id-1)]->SetParLimits(0, -200, 200);
            fTWalk[(id-1)]->SetParLimits(1,  -100, 400); // 400 is very important
            fTWalk[(id-1)]->SetParLimits(2, -60, 20);
            fTWalk[(id-1)]->SetParLimits(3,  0.8, 1.4);
        }

        //      hhEWpro[(id-1)]->Fit(fTWalk[(id-1)], "+RQ0");

        // do fit at least 3 times in order to minimize chi^2
        //
        //       for(int i=0; i<3; i++)
        //          {
        //    fTWalk[(id-1)]->GetRange( low, upp );
        //        low += 2;
        //        upp -= 2;
        fTWalk[(id-1)]->SetRange(low, upp);
        hhEWpro[(id-1)]->Fit(fTWalk[(id-1)], "+RQ0");

        fChi2NDF[(id-1)] = (fTWalk[(id-1)]->GetChisquare() /
                            fTWalk[(id-1)]->GetNDF());

        //    if( fChi2NDF[(id-1)] < 50.)
        //              break;
        printf("Chi^2 %f\n", fChi2NDF[(id-1)]);
        //    }
        printf("\n -- old TWalk %03i %f %f %f %f\n",
               id,
               TWalk0[(id-1)], TWalk1[(id-1)], TWalk2[(id-1)], TWalk3[(id-1)]);

        TWalk0[(id-1)] = fTWalk[(id-1)]->GetParameter(0);
        TWalk1[(id-1)] = fTWalk[(id-1)]->GetParameter(1);
        TWalk2[(id-1)] = fTWalk[(id-1)]->GetParameter(2);
        TWalk3[(id-1)] = fTWalk[(id-1)]->GetParameter(3);

        printf(" -- new TWalk %03i %f %f %f %f\n\n",
               id,
               TWalk0[(id-1)], TWalk1[(id-1)], TWalk2[(id-1)], TWalk3[(id-1)]);
    }

    if (hhTWalk[(id-1)])
    {
        this->DrawThis(id);
        this->DrawGraph();
    }

    return;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::DoFit(Int_t id)
{
    // Fit the TWalk projections

    if (!fFitPeak[(id-1)])
    {
        Char_t szName[64];
        sprintf(szName, "fTWalk_%i", (id-1));
        fFitPeak[(id-1)] = new TF1(szName, "pol0(0)+gaus(1)");
        fFitPeak[(id-1)] ->SetLineColor(2);
    }

    //
    Int_t maxbin = hhTWpro[(id-1)]->GetMaximumBin();
    peackval = hhTWpro[(id-1)]->GetBinCenter(maxbin);
    fFitPeak[(id-1)]->SetRange(peackval-6, peackval+6);

    fFitPeak[(id-1)]->SetParameters(1., hhTWpro[(id-1)]->GetMaximum(), peackval, 1.);

    fFitPeak[(id-1)]->SetParLimits(0, 0, 10000); // offset
    fFitPeak[(id-1)]->SetParLimits(1, 0, 10000); // peack
    fFitPeak[(id-1)]->SetParLimits(2, -1000, 1000); // peack position
    fFitPeak[(id-1)]->SetParLimits(3, 0.1, 20.0); // sigma

    hhTWpro[(id-1)]->Fit(fFitPeak[(id-1)], "+RQ");

    mean_gaus[(id-1)] =   fFitPeak[(id-1)]->GetParameter(2); // store peak value
    //  erro_gaus[(id-1)] =   fFitPeak[(id-1)]->GetParError(2);  // store peak error
    erro_gaus[(id-1)] =   fFitPeak[(id-1)]->GetParError(3);  // store peak error

    //
    lOffset[(id-1)]->SetVertical();
    lOffset[(id-1)]->SetLineColor(4);
    lOffset[(id-1)]->SetLineWidth(3);
    lOffset[(id-1)]->SetY1(0);
    lOffset[(id-1)]->SetY2(hhTWpro[(id-1)]->GetMaximum() + 20);
    lOffset[(id-1)]->SetX1(mean_gaus[(id-1)]);
    lOffset[(id-1)]->SetX2(mean_gaus[(id-1)]);

    return;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::DoFit()
{
    return;
}

//------------------------------------------------------------------------------
void iCalibCBTimeWalk::Write()
{
    //

    // write to database
    iMySQLManager m;
    m.WriteParameters(fSet, ECALIB_CB_WALK0, TWalk0, iConfig::kMaxCB);
    m.WriteParameters(fSet, ECALIB_CB_WALK1, TWalk1, iConfig::kMaxCB);
    m.WriteParameters(fSet, ECALIB_CB_WALK2, TWalk2, iConfig::kMaxCB);
    m.WriteParameters(fSet, ECALIB_CB_WALK3, TWalk3, iConfig::kMaxCB);

    //   // write histogram
    //   Char_t szName[56];
    //   sprintf( szName,
    //            "/usr/users/irakli/public_html/files/calib/%s/cb/Tcalib/hGr_set%02i.gif",
    //       strTarget.Data(),
    //            fSet );
    //   c2->SaveAs( szName );

    return;
}

// EOF
