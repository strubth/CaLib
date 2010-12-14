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
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem, elem);
    
    // init variables
    Double_t factor = 3.0;
    Double_t peakval = 0;
    Double_t mean = 0;
    
    // check for sufficient statistics
    if (fFitHisto->GetEntries() > 20)
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fTime_%i", (elem-1));
        fFitFunc = new TF1(tmp, "gaus");
        fFitFunc->SetLineColor(2);

        peakval = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());

        // temporary
        mean = peakval;

        // first iteration
        fFitFunc->SetRange(peakval - 3.8, peakval + 3.8);
        fFitFunc->SetParameters(fFitHisto->GetMaximum(), peakval, 0.5);
        fFitHisto->Fit(fFitFunc, "+R0Q");

        // second iteration
        peakval = fFitFunc->GetParameter(1);
        Double_t sigma = fFitFunc->GetParameter(2);
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

    // draw histogram
    fFitHisto->SetFillColor(35);
    fFitHisto->GetXaxis()->SetRangeUser(peakval -50., peakval +50.);
    fCanvasFit->cd(2);
    fFitHisto->Draw();
    
    // draw fitting function
    if (fFitFunc) fFitFunc->Draw("same");
    
    // draw indicator line
    fLine->Draw();
    
    // update canvas
    fCanvasFit->Update();
    
    // update overview
    if (elem % 20 == 0)
    {
        fCanvasResult->cd();
        fOverviewHisto->GetYaxis()->SetRangeUser(-1, 1);
        fOverviewHisto->Draw("E1");

        fOverviewFunc->Draw("same");
        fOverviewHisto->Fit(fOverviewFunc, "+R0Q", "");
        fCanvasResult->Update();
    }   

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // calculate the new offset
        fNewVal[(elem-1)] = fOldVal[(elem-1)] + mean / fTimeGain;
    
        // user information
        printf("Element: %03i peak = %10.6f \t newoffset = %10.6f \t "
               "oldoffset = %10.6f\n",
               elem, mean, fNewVal[(elem-1)], fOldVal[(elem-1)]);
    
        // update overview histogram
        fOverviewHisto->SetBinContent(elem, mean);
        fOverviewHisto->SetBinError(elem, 0.1);
    }
    else
    {   
        // user information
        printf("Element: %03i peak = %10.6f \t newoffset = %10.6f \t "
               "oldoffset = %10.6f    no change\n",
               elem, mean, fOldVal[(elem-1)], fOldVal[(elem-1)]);
    }
}   

//______________________________________________________________________________
void iCalibCB2gTime::Write()
{
    // Write the obtained calibration values to the database.
    
    // write the new time offsets to the database
    iMySQLManager m;
    m.WriteParameters(fSet, ECALIB_CB_T0, fNewVal, fNelem);
        
    // get path for image saving
    iReadConfig r;
    TString path = r.GetConfigName("HTML.PATH").Data();
    
    // save overview picture
    if (!(path == ""))
    {
        Char_t tmp[256];

        // create directory
        sprintf(tmp, "%s/cb/Tcalib", path.Data());
        gSystem->mkdir(tmp, kTRUE);
    
        // save canvas
        sprintf(tmp, "%s/cb/Tcalib/overview_set_%d.png", path.Data(), fSet);
        fCanvasResult->SaveAs(tmp);
    }
}

