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
    
    // init members
    fLine = new TLine();

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
    
    // get the main calibration histogram
    fMainHisto = (TH2F*) f.GetMainHisto();
    if (!fMainHisto)
    {
        Error("iCalibCB2gTime", "Main histogram does not exist!\n");
        gSystem->Exit(0);
    }
    
    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Crystal Number;Time_{CB-CB} [ns]", 722, -0.5, 721.5);
    fOverviewHisto->SetMarkerStyle(28);
    fOverviewHisto->SetMarkerColor(4);
    
    // get parameters from configuration file
    Double_t low = atof(r.GetConfigName("CBtimeGraph.low"));
    Double_t upp = atof(r.GetConfigName("CBtimeGraph.upp"));
    
    // ajust overview histogram
    if (low || upp) fOverviewHisto->GetYaxis()->SetRangeUser(low, upp);
    
    // create the overview fitting function
    fOverviewFunc = new TF1("OverviewFunc", "pol0", 0, 720);
    fOverviewFunc->SetParameter(0, 1.);
    fOverviewFunc->SetLineColor(2);
}

//______________________________________________________________________________
iCalibCB2gTime::~iCalibCB2gTime()
{
    // Destructor. 
    
    if (fLine) delete fLine;
    if (fOverviewHisto) delete fOverviewHisto;
    if (fOverviewFunc) delete fOverviewFunc;
}

//______________________________________________________________________________
void iCalibCB2gTime::CustomizeGUI()
{
    // Customize the GUI of this calibration module.

    // draw main histogram
    fCanvasFit->cd(1)->SetLogz();
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void iCalibCB2gTime::Process(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    // create projection
    Char_t tmp[32];
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem, elem);
    
    Char_t szName[64];

    int maxbin;
    double sigma;
    double rms;
    double factor = 3.0;
    Double_t peakval = 0;
    Double_t mean = 0;

    if (fFitHisto->GetEntries() > 20)
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(szName, "fTime_%i", (elem-1));
        fFitFunc = new TF1(szName, "gaus");
        fFitFunc->SetLineColor(2);


        maxbin = fFitHisto->GetMaximumBin();
        peakval = fFitHisto->GetBinCenter(maxbin);

        // temporary
        mean = peakval;
        rms = fFitHisto->GetRMS();

        // first iteration
        fFitFunc->SetRange(peakval -3.8, peakval +3.8);
        fFitFunc->SetParameters(fFitHisto->GetMaximum(), peakval, 0.5);
        fFitHisto->Fit(fFitFunc, "+R0Q");

        // second iteration
        peakval = fFitFunc->GetParameter(1);
        sigma = fFitFunc->GetParameter(2);
        fFitFunc->SetRange(peakval -factor*sigma, peakval +factor*sigma);
        fFitHisto->Fit(fFitFunc, "+R0Q");

        // final results
        mean = fFitFunc->GetParameter(1); // store peak value

        // draw mean indicator line
        fLine->SetVertical();
        fLine->SetLineColor(4);
        fLine->SetLineWidth(3);
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        fLine->SetX1(mean);
        fLine->SetX2(mean);
    }

    fFitHisto->SetFillColor(35);
    fFitHisto->GetXaxis()->SetRangeUser(peakval -50., peakval +50.);
    fCanvasFit->cd(2);
    fFitHisto->Draw();

    if (fFitFunc)
    {
        fFitFunc->Draw("same");
    }
    fLine->Draw();

    fCanvasFit->Update();
    
   
    // calculate new gain
    //
    if (fFitHisto->GetEntries())
    {
        if (fLine->GetX1() != mean)
            mean = fLine->GetX1();

        fNewVal[(elem-1)] = fOldVal[(elem-1)] + mean / fTimeGain;

        printf("Element: %03i peak = %10.6f \t newoffset = %10.6f \t "
               "oldoffset = %10.6f \n",
               elem, mean, fNewVal[(elem-1)], fOldVal[(elem-1)]);

        fOverviewHisto->SetBinContent(elem, mean);
        fOverviewHisto->SetBinError(elem, 0.1);
    }
    else
    {
        printf("Element: %03i peak = %10.6f \t newoffset = %10.6f \t "
               "oldoffset = %10.6f \n",
               elem, mean, 0.0, fOldVal[(elem-1)]);
    }
    
    // update overview
    fCanvasResult->cd();
    fOverviewHisto->GetYaxis()->SetRangeUser(-1, 1);
    fOverviewHisto->Draw("E1");

    fOverviewFunc->Draw("same");
    fOverviewHisto->Fit(fOverviewFunc, "+R0Q", "");
    fCanvasResult->Update();
}

//______________________________________________________________________________
void iCalibCB2gTime::Write()
{
    // Write the obtained calibration values to the database.
    
    // write the new time offsets to the database
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

    fCanvasResult->SaveAs(szName);
}

