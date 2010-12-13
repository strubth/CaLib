/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller, Lilian Witthauer
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibCB2gTime                                                       //
//                                                                      //
// Calibration module for the CB time.                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "iCalibCB2gTime.hh"

ClassImp(iCalibCB2gTime)


//______________________________________________________________________________
iCalibCB2gTime::iCalibCB2gTime(Int_t set)
    : iCalib(set, MAX_CB)
{
    // Constructor.
    
    // create arrays
    fGaussMean = new Double_t[fNelem];
    fGaussError = new Double_t[fNelem];
    fLineOffset = new TLine*[fNelem];

    // init arrays
    for (Int_t i = 0; i < fNelem; i++)
    {
        fGaussMean[i] = 0;
        fGaussError[i] = 0;
        fLineOffset[i] = new TLine();
    }
    
    // read configuration
    iReadConfig r;

    // get histogram name
    fHistoName = r.GetConfigName("CB.Time.Histo.Name");
    
    // get time gain for CB TDCs
    fTimeGain = atof(r.GetConfigName("CB.Time.TDC.Gain").Data());

    // read old parameters
    iMySQLManager m;
    m.ReadParameters(fSet, ECALIB_CB_T0, fOldVal, fNelem);

    // sum up all files contained in this runset
    iFileManager f;
    f.DoForSet(fSet, ECALIB_CB_T0, fHistoName);

    if (f.GetMainHisto())
    {
        fMainHisto = (TH2F*) f.GetMainHisto();
    }
    else
    {
        Error("iCalibCB2gTime", "Main histogram does not exist!\n");
        gSystem->Exit(0);
    }
}

//______________________________________________________________________________
iCalibCB2gTime::~iCalibCB2gTime()
{
    if (fGaussMean) delete [] fGaussMean;
    if (fGaussError) delete [] fGaussError;
    if (fLineOffset)
    {
        for (Int_t i = 0; i < fNelem; i++) delete fLineOffset[i];
        delete [] fLineOffset;
    }
}

//______________________________________________________________________________
void iCalibCB2gTime::CustomizeGUI()
{
    // Customize the GUI of this calibration module.

    // set log scale for main histogram
    fCanvasFit->cd(1)->SetLogz();
    
    // draw main histogram
    fMainHisto->Draw("colz");

    // create graph
    hhOffset = new TH1F("hhOffset", ";Crystal Number;Time_{CB-CB} [ns]",
                        722, -0.5, 721.5);
    hhOffset->SetMarkerStyle(28);
    hhOffset->SetMarkerColor(4);

    iReadConfig r;
    Double_t low =  atof(r.GetConfigName("CBtimeGraph.low"));
    Double_t upp =  atof(r.GetConfigName("CBtimeGraph.upp"));

    if (low || upp)
        hhOffset->GetYaxis()->SetRangeUser(low, upp);
    hhOffset->Draw("P");

    fPol0 = new TF1("Pol0", "pol0", 0, 720);
    fPol0->SetParameter(0, 1.);
    fPol0->SetLineColor(2);

    return;
}

//______________________________________________________________________________
void iCalibCB2gTime::PrintInfo()
{
    // Print information about this calibration module.

    printf("************************************************************\n");
    printf("* CB time calibration module                               *\n");
    printf("*                                                          *\n");
    printf("*                                                          *\n");
    printf("************************************************************\n");
}

//______________________________________________________________________________
void iCalibCB2gTime::PrepareFit(Int_t elem)
{
    // Prepare the fitting of element 'elem'.
    
    currentCrystal = id;
    
    // get projection
    Char_t tmp[32];
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    fProjHisto[(elem-1)] = (TH1D*) h2->ProjectionX(tmp, elem, elem);
}

//______________________________________________________________________________
void iCalibCB2gTime::Calculate(Int_t id)
{
    // calculate new gain
    //
    if (fProjHisto[(id-1)]->GetEntries())
    {
        if (fLineOffset[(id-1)]->GetX1() != fGaussMean[(id-1)])
            fGaussMean[(id-1)] = fLineOffset[(id-1)]->GetX1();

        fNewVal[(id-1)] = fOldVal[(id-1)] + fGaussMean[(id-1)] / fTimeGain;

        printf("Element: %03i peak = %10.6f \t newoffset = %10.6f \t "
               "oldoffset = %10.6f \n",
               id, fGaussMean[(id-1)], fNewVal[(id-1)], fOldVal[(id-1)]);

        hhOffset->SetBinContent(id, fGaussMean[(id-1)]);
        hhOffset->SetBinError(id, 0.1);
    }
    else
    {
        printf("Element: %03i peak = %10.6f \t newoffset = %10.6f \t "
               "oldoffset = %10.6f \n",
               id, fGaussMean[(id-1)], 0.0, fOldVal[(id-1)]);
    }
    return;
}

//______________________________________________________________________________
void iCalibCB2gTime::Draw(Int_t elem)
{
    // Draw the result after fitting element 'elem'.

    fCanvasFit->cd(2);
    fProjHisto[(id-1)]->SetFillColor(35);
    fProjHisto[(id-1)]->GetXaxis()->SetRangeUser(peackval -50., peackval +50.);
    fProjHisto[(id-1)]->Draw();

    if (fFitFunc[(id-1)])
    {
        fFitFunc[(id-1)]->Draw("same");
    }
    fLineOffset[(id-1)]->Draw();

    fCanvasFit->Update();
}

//______________________________________________________________________________
void iCalibCB2gTime::DrawGraph()
{
    //
    fCanvasRes->cd();
    hhOffset->GetYaxis()->SetRangeUser(-1, 1);
    hhOffset->Draw("E1");

    fPol0->Draw("same");
    fCanvasRes->Update();

    return;
}

//______________________________________________________________________________
void iCalibCB2gTime::DoFor(Int_t id)
{



    //   if( !(id % 20) || id < MAX_CB )
    //     {
    //       this->DrawGraph();
    //     }
    if (id == fNelem)
    {
        hhOffset->Fit(fPol0, "+R0", "");
        this->DrawGraph();
    }
    return;
}

//______________________________________________________________________________
void iCalibCB2gTime::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t szName[64];

    int maxbin;
    double sigma;
    double rms;
    double factor = 3.0;

    if ((fProjHisto[(elem-1)])->GetEntries() > 20)
    {
        sprintf(szName, "fTime_%i", (elem-1));
        fFitFunc[(elem-1)] = new TF1(szName, "gaus");
        fFitFunc[(elem-1)]->SetLineColor(2);


        maxbin = fProjHisto[(elem-1)]->GetMaximumBin();
        peackval = fProjHisto[(elem-1)]->GetBinCenter(maxbin);

        // temporary
        fGaussMean[(elem-1)] = peackval;
        rms = fProjHisto[(elem-1)]->GetRMS();

        // first iteration
        fFitFunc[(elem-1)]->SetRange(peackval -3.8, peackval +3.8);
        fFitFunc[(elem-1)]->SetParameters(fProjHisto[(elem-1)]->GetMaximum(), peackval, 0.5);

        //    fFitFunc[(id-1)]->SetParLimits(0, 0, 3); //
        //    fFitFunc[(id-1)]->SetParLimits(2, 0, 100); // peack
        //    fFitFunc[(id-1)]->SetParLimits(3, 0.8, 3.0); // sigma

        fProjHisto[(elem-1)]->Fit(fFitFunc[(elem-1)], "+R0Q");

        // second iteration
        peackval =   fFitFunc[(elem-1)]->GetParameter(1);
        sigma =   fFitFunc[(elem-1)]->GetParameter(2);

        fFitFunc[(elem-1)]->SetRange(peackval -factor*sigma,
                                   peackval +factor*sigma);
        fProjHisto[(elem-1)]->Fit(fFitFunc[(elem-1)], "+R0Q");


        // final results
        fGaussMean[(elem-1)] = fFitFunc[(elem-1)]->GetParameter(1); // store peak value
        fGaussError[(elem-1)] = fFitFunc[(elem-1)]->GetParError(1);  // store peak error

        //
        fLineOffset[(elem-1)]->SetVertical();
        fLineOffset[(elem-1)]->SetLineColor(4);
        fLineOffset[(elem-1)]->SetLineWidth(3);
        fLineOffset[(elem-1)]->SetY1(0);
        fLineOffset[(elem-1)]->SetY2(fProjHisto[(elem-1)]->GetMaximum() + 20);

        // check if mass is in normal range
        if (fGaussMean[(elem-1)] > 10 ||
                fGaussMean[(elem-1)] < 200)
        {
            fLineOffset[(elem-1)]->SetX1(fGaussMean[(elem-1)]);
            fLineOffset[(elem-1)]->SetX2(fGaussMean[(elem-1)]);
        }
        else
        {
            fLineOffset[(elem-1)]->SetX1(135.);
            fLineOffset[(elem-1)]->SetX2(135.);
        }
    }
}

//______________________________________________________________________________
void iCalibCB2gTime::Write()
{
    // write into DB
    //
    iMySQLManager m;
    m.WriteParameters(fSet, ECALIB_CB_T0, fNewVal, fNelem);

    iReadConfig r;

    // write histogram
    Char_t szName[100];
    sprintf(szName,
            "/%s/cb/Tcalib/hGr_set%02i.gif",
            r.GetConfigName("HTML.PATH").Data(),
            fSet);
    //check if Directory for pictures exist, otherwise create
    Char_t szMakeDir[100];
    sprintf(szMakeDir,
            "mkdir -p %s/cb/Tcalib/",
            r.GetConfigName("HTML.PATH").Data());
    gSystem->Exec(szMakeDir);

    fCanvasRes->SaveAs(szName);

    return;
}

