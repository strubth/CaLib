
/*******************************************************************
 *                                                                 *
 * Date: 25.08.2010     Author: Manuel                             *
 *                                                                 *
 * This macro is used for the energy calibration of PID            *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

#include "iCalibPIDenergy.hh"

ClassImp(iCalibPIDenergy)

//_____________________________________________
iCalibPIDenergy::iCalibPIDenergy()
{
    this->Init();

    if (gROOT->GetFile())
        histofile = gFile;
    else
        histofile = TFile::Open(strPIDHistoFile);

    mcfile = TFile::Open(strPIDPathMC);

    printf("\n ---------------------------------------- \n");
    printf(" Read Data File: \"%s\"\n", histofile->GetName());

    if (!strPIDHistoName.Length())
        hdat_dE_E = (TH2F*) histofile->Get("iPIDdE_CBE_cut");
    else
        hdat_dE_E = (TH2F*) histofile->Get(strPIDHistoName.Data());

    if (!hdat_dE_E)
    {
        printf("\n ERROR: The histogram named: %s can not be found!!!\n"
               "          Check in \"config.cfg\" file PID.HEName !\n", strPIDHistoName.Data());
        gSystem->Exit(0);
    }
    else
        printf(" Read Histo Name: \"%s\"\n", strPIDHistoName.Data());

    printf("\n ---------------------------------------- \n");
    printf(" Read MC File: \"%s\"\n", mcfile->GetName());

    if (!strPIDHistoName.Length())
        hmc_dE_E = (TH2F*) mcfile->Get("iPIDdE_CBE_cut");
    else
        hmc_dE_E = (TH2F*) mcfile->Get(strPIDHistoName.Data());

    if (!hmc_dE_E)
    {
        printf("\n ERROR: The histogram named: %s can not be found!!!\n"
               "          Check in \"config.cfg\" file PID.HEName !\n", strPIDHistoName.Data());
        gSystem->Exit(0);
    }
    else
        printf(" Read Histo Name: \"%s\"\n", strPIDHistoName.Data());

    this->InitGUI();

    this->DoFor(1);
}

//_____________________________________________
iCalibPIDenergy::iCalibPIDenergy(TH2F* h1)
{

    this->Init();

    hdat_dE_E = (TH2F*) h1;

    this->InitGUI();

    this->DoFor(1);
}
//_____________________________________________
iCalibPIDenergy::iCalibPIDenergy(Int_t set)
{
    fSet = set;

    this->Init();

    // sum up all files contained in this runset
    iFileManager f(set, ECALIB_PID_E1);

    //hCBPhiVsPID = (TH2F*) h1;
    if (hdat_dE_E)
    {
        hdat_dE_E->Delete();
        hdat_dE_E=0;
    }

    if (f.GetHistogram(strPIDHistoName.Data()))
    {
        hdat_dE_E = (TH2F*) f.GetHistogram(strPIDHistoName.Data());
    }
    else
    {
        printf("\n ERROR: main histogram does not exist!!! \n");
        gSystem->Exit(0);
    }
    this->InitGUI();

    this->DoFor(1);
}
//_____________________________________________
iCalibPIDenergy::~iCalibPIDenergy()
{
    delete c1;
    delete c2;
    delete h4gr;
}
//_____________________________________________
void iCalibPIDenergy::Init()
{
    // Initialize all variables in this class
    // print help
    this->Help();

    // get proton and charged pion MC .root file
    strPIDPathMC = *this->GetConfig("PID.MCPath");

    // get histo name
    strPIDHistoName = *this->GetConfig("PID.HEName");

    // iMySQLReader
    //  Char_t szTableName[64];

    //  sprintf( szTableName, "%s_pid_Ecalib", strTarget.Data() );
    //  this->ReadDBtable( fSet, fOldPhi, iConfig::kMaxPID, szTableName );

    return;
}
//_____________________________________________
void iCalibPIDenergy::InitGUI()
{
    c1 = new TCanvas("c1", "", 0, 0, 800, 800);
    c1->Divide(2, 2, 0.001, 0.001);
    c1->cd(1);
    c1->GetPad(1)->SetLogz();
    hdat_dE_E->Draw("colz");

    c1->cd(2);
    c1->GetPad(2)->SetLogz();
    hmc_dE_E->Draw("colz");

    // create graph
    c2 = new TCanvas("c2", "",
                     430, 0, 900, 400);
    h4gr = new TH2F("h4gr", ";PID Number;Peak Pos [MeV]",
                    26, -1, 25, 1000, -500, 500);
    h4gr->SetStats(kFALSE);
    h4gr->Draw();

    grDat = new TGraphErrors();
    grDat->SetNameTitle("gr_dat_", "");
    grDat->SetMarkerStyle(7);
    grDat->SetLineColor(27);

    grMC = new TGraphErrors();
    grMC->SetNameTitle("gr_mc_", "");
    grMC->SetMarkerStyle(7);
    grMC->SetMarkerColor(2);
    grMC->SetLineColor(2);

    Char_t szName[32];
    for (Int_t i = 0; i < iConfig::kMaxPID; i++)
    {
        sprintf(szName, "fFitData_%i", i);
        fFitData[i] = new TF1(szName, "expo(0)+landau(2)+gaus(5)");
        fFitData[i]->SetLineColor(2);

        sprintf(szName, "fFitMC_%i", i);
        fFitMC[i] = new TF1(szName, "expo(0)+landau(2)+gaus(5)");

        lProtonDat[i] = new TLine();
        lProtonMC[i]  = new TLine();
        lPionDat[i]   = new TLine();
        lPionMC[i]    = new TLine();
    }
    return;
}
//_____________________________________________
void iCalibPIDenergy::Help()
{
    cout <<"****************************************************************" << endl;
    cout <<"* You are working with PID Energy calibration module           *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"* using PID deposited and CB cluster energy                    *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"* Yeah mon!                                                    *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"*                                                              *" << endl;
    cout <<"****************************************************************" << endl;
    return;
}
//_____________________________________________
void iCalibPIDenergy::DrawThis(Int_t id)
{
    // draw all histograms on 2 canvas
    //
    c1->cd(3);
    if (hdatProj[(id-1)])
    {
        //       hProj[(id-1)]->GetXaxis()->SetRangeUser(0, 300);
        hdatProj[(id-1)]->Draw();
    }

    if (fFitData[(id-1)])
    {
        fFitData[(id-1)]->SetLineColor(2);
        fFitData[(id-1)]->Draw("same");

        lProtonDat[(id-1)]->Draw("same");
        lPionDat[(id-1)]->Draw("same");
    }

    c1->cd(4);
    if (hmcProj[(id-1)])
    {
        //       hProj[(id-1)]->GetXaxis()->SetRangeUser(0, 300);
        hmcProj[(id-1)]->Draw();
    }

    if (fFitMC[(id-1)])
    {
        fFitMC[(id-1)]->SetLineColor(2);
        fFitMC[(id-1)]->Draw("same");

        lProtonMC[(id-1)]->Draw("same");
        lPionMC[(id-1)]->Draw("same");
    }

    //   c1->Modified();
    c1->Update();

    //
    c2->cd();
    grDat->Draw("p");
    grMC->Draw("p");
    //   c2->Modified();
    c2->Update();

    return;
}

//_____________________________________________
void iCalibPIDenergy::GetProjection(Int_t id)
{
    // get projection from 2D histogram
    //
    Char_t szName[32];
    sprintf(szName, "hdatProj_%i", id);
    //  hProj[(id-1)] = (TH1D*) hCBPhiVsPID->ProjectionY(szName, id, id, "e");
    //  hProj[(id-1)] = (TH1D*) hCBPhiVsPID->ProjectionY(szName, id, id);
    hdatProj[(id-1)] = (TH1D*) hdat_dE_E->ProjectionX(szName, id, id);

    sprintf(szName, "hmcProj_%i", id);
    hmcProj[(id-1)]  = (TH1D*) hmc_dE_E->ProjectionX(szName, id, id);

    return;
}

//_____________________________________________
void iCalibPIDenergy::Calculate(Int_t id)
{
    // calculate new gain
    //
    /*  if( hProj[(id-1)]->GetEntries() )
        {
          if( lOffset[(id-1)]->GetX1() != mean_gaus[(id-1)] )
            {
            mean_gaus[(id-1)] = lOffset[(id-1)]->GetX1();
            cout << "Mean Gauss and Line not equal" << endl;
            }
          //
          //fNewPhi[(id-1)] = mean_gaus[(id-1)]-fOldPhi[(id-1)];
          fNewPhi[(id-1)] = mean_gaus[(id-1)];
          //fOldPhi[(id-1)] = PIDphi[(id-1)];

          printf("Element: %03i \t new = %8.4f \t old = %8.4f \n",
             id, fNewPhi[(id-1)], fOldPhi[(id-1)] );

          grNewPhi->SetPoint(     (id-1), id, mean_gaus[(id-1)] );
          grNewPhi->SetPointError((id-1), 0., fFitPeak[(id-1)]->GetParError(3)  );
        }
      else
        {
          printf("Element: %03i \t new = %f \t old = %f \n",
             id, 0.0, 0.0 );
        }
      */
    return;
}

//_____________________________________________
void iCalibPIDenergy::DoFor(Int_t id)
{
    // go through all crystals
    //
    currentCrystal = id;
    this->GetProjection(id);

    if ((hdatProj[(id-1)])->GetEntries()) this->DoFit(id);

    if ((hmcProj[(id-1)])->GetEntries())  this->DoFit(id);

    //this->FitPIDphi( id, hProj[(id-1)] );

    this->DrawThis(id);

    return;
}
//------------------------------------------------------------------------------
void iCalibPIDenergy::DoFit(Int_t id)
{
    // Fit the Pi0 peak of an element in the corresponding histogram
    /*
      //
      Int_t maxbin = hProj[(id-1)]->GetMaximumBin();
      peackval = hProj[(id-1)]->GetBinCenter(maxbin);

      cout << "Peakval: " << peackval << endl ;

      fFitPeak[(id-1)]->SetRange( peackval-50, peackval+50 );

      //peak splitted
      if(((peakval + 50) > 180) || ((peackval-50) <180) )
      {
        Int_t count = 1;
        for (Int_t binx = (hProj[(id-1)]->FindBin(-180)); binx < (hProj[(id-1)]->FindBin(180)+1); binx++)
        {
            if(hProj[(id-1)]->GetBinCenter(binx)<=0) hProj[(id-1)]->Fill(180 + count, (hProj[(id-1)]->GetBinContent(binx)));
            else hProj[(id-1)]->Fill(-3*180 + count, (hProj[(id-1)]->GetBinContent(binx)));
        count++;
        }
      }

      fFitPeak[(id-1)]->SetLineColor(2);

      fFitPeak[(id-1)]->SetParameters( 0.0, 0.0,
                      hProj[(id-1)]->GetMaximum(), peackval, 8.);
      hProj[(id-1)]->Fit(fFitPeak[(id-1)], "+R0Q");
      mean_gaus[(id-1)] = fFitPeak[(id-1)]->GetParameter(3); // store peak value
      erro_gaus[(id-1)] = fFitPeak[(id-1)]->GetParError(3);  // store peak error

      if (mean_gaus[(id-1)]>180)
      {
        Double_t mean = -360 + mean_gaus[(id-1)];
        mean_gaus[(id-1)] = mean;
      }
      if (mean_gaus[(id-1)]<-180)
      {
        Double_t mean = 360 - mean_gaus[(id-1)];
        mean_gaus[(id-1)] = mean;
      }
      //
      lOffset[(id-1)]->SetVertical();
      lOffset[(id-1)]->SetLineColor(4);
      lOffset[(id-1)]->SetLineWidth(3);
      lOffset[(id-1)]->SetY1( 0 );
      lOffset[(id-1)]->SetY2( hProj[(id-1)]->GetMaximum() + 20 );
      lOffset[(id-1)]->SetX1( mean_gaus[(id-1)] );
      lOffset[(id-1)]->SetX2( mean_gaus[(id-1)] );

      //   // Do check if everithing is ok
      //   if( fPol1Gaus[(id-1)]->GetParameter(4) < 5. ||
      //       fPol1Gaus[(id-1)]->GetParameter(4) > 10. )
    */
    return;
}
//_____________________________________________
void iCalibPIDenergy::Write()
{
    //
    //  this->WriteConfigFile_PID( fNewPhi );

    //Write in DB
    //Char_t szTableName[64];
    //sprintf( szTableName, "%s_pid_phi", strTarget.Data() );
    //this->WriteInDB( fSet, fNewPhi, iConfig::kMaxPID, szTableName );

    //save pictures
    Char_t szName[100];
    sprintf(szName,
            "%s/pid/phi/hGr_set%02i.gif",
            this->GetConfig("HTML.PATH")->Data(),
            fSet);
    Char_t szName1[100];
    sprintf(szName1,
            "%s/pid/phi/hPID_set%02i.gif",
            this->GetConfig("HTML.PATH")->Data(),
            fSet);

    //check if Directory for pictures exist, otherwise create
    Char_t szMakeDir[100];
    sprintf(szMakeDir,
            "mkdir -p %s/pid/phi/",
            this->GetConfig("HTML.PATH")->Data());
    gSystem->Exec(szMakeDir);

    c1->SaveAs(szName1);
    c2->SaveAs(szName);
    return;
}

// EOF
