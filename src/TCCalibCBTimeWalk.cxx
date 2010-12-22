// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibCBTimeWalk                                                    //
//                                                                      //
// Calibration module for the CB time walk.                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibCBTimeWalk.h"

ClassImp(TCCalibCBTimeWalk)


//______________________________________________________________________________
TCCalibCBTimeWalk::TCCalibCBTimeWalk()
    : TCCalib("CB.TimeWalk", "CB time walk calibration", kCALIB_CB_WALK1, TCConfig::kMaxCB)
{
    // Empty constructor.

    // init members
    fFileManager = 0;
    fPar0 = 0;
    fPar1 = 0;
    fPar2 = 0;
    fPar3 = 0;
    fEnergyProj = 0;
    fTimeProj = 0;
}

//______________________________________________________________________________
TCCalibCBTimeWalk::~TCCalibCBTimeWalk()
{
    // Destructor. 
    
    if (fFileManager) delete fFileManager;
    if (fPar0) delete [] fPar0;
    if (fPar1) delete [] fPar1;
    if (fPar2) delete [] fPar2;
    if (fPar3) delete [] fPar3;
    if (fEnergyProj) delete fEnergyProj;
    if (fTimeProj) delete fTimeProj;
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Init()
{
    // Init the module.
    
    // init members
    fFileManager = new TCFileManager(fSet, fData);
    fPar0 = new Double_t[fNelem];
    fPar1 = new Double_t[fNelem];
    fPar2 = new Double_t[fNelem];
    fPar3 = new Double_t[fNelem];
    fEnergyProj = 0;
    fTimeProj = 0;

    // get histogram name
    if (!TCReadConfig::GetReader()->GetConfig("CB.TimeWalk.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("CB.TimeWalk.Histo.Fit.Name");
    
    // read old parameters
    TCMySQLManager::GetManager()->ReadParameters(fSet, kCALIB_CB_WALK0, fPar0, fNelem);
    TCMySQLManager::GetManager()->ReadParameters(fSet, kCALIB_CB_WALK1, fPar1, fNelem);
    TCMySQLManager::GetManager()->ReadParameters(fSet, kCALIB_CB_WALK2, fPar2, fNelem);
    TCMySQLManager::GetManager()->ReadParameters(fSet, kCALIB_CB_WALK3, fPar3, fNelem);

    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();

    // draw the overview histogram
    fCanvasResult->cd();
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram name
    sprintf(tmp, "%s_%03d", fHistoName.Data(), elem);
    
    // delete old histogram
    if (fMainHisto) delete fMainHisto;
    
    // get histogram
    fMainHisto = fFileManager->GetHistogram(tmp);
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // draw main histogram
    fCanvasFit->cd(1);
    TCUtils::FormatHistogram(fMainHisto, "CB.TimeWalk.Histo.Fit");
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
 
    // check for sufficient statistics
    if (fMainHisto->GetEntries() > 1000)
    {
        // create energy projection
        sprintf(tmp, "ProjEnergy_%d", elem);
        TH2* h2 = (TH2*) fMainHisto;
        if (fEnergyProj) delete fEnergyProj;
        fEnergyProj = (TH1D*) h2->ProjectionX(tmp, 1, h2->GetNbinsY(), "e");

        // loop over all energy bins
        Int_t nBins = fEnergyProj->GetNbinsX();
        for (Int_t i = 1; i <= nBins; i++)
        {   
            printf("bin %d\n", i);
            // check if projection has enough entries
            if (fEnergyProj->GetBinContent(i) > 30)
            {
                // find fit range
                //if (!low && fEnergyProj->GetBinContent(i-1))
                //    low = fEnergyProj->GetBinLowEdge(i);
                //else
                //    upp = fEnergyProj->GetBinLowEdge(i+1);
                
                // create time projection
                if (fTimeProj) delete fTimeProj;
                sprintf(tmp, "ProjTime_%d_%d", elem, i);
                fTimeProj = (TH1D*) h2->ProjectionY(tmp, i, i, "e");
                
                //
                // fit time projection
                //
                
                // create fitting function
                if (fFitFunc) delete fFitFunc;
                sprintf(tmp, "fWalkProfile_%i", elem);
                fFitFunc = new TF1(tmp, "pol0(0)+gaus(1)");
                fFitFunc->SetLineColor(2);

                // prepare fitting function
                Int_t maxbin = fTimeProj->GetMaximumBin();
                Double_t peak = fTimeProj->GetBinCenter(maxbin);
                fFitFunc->SetRange(peak - 6, peak + 6);
                fFitFunc->SetParameters(1., fTimeProj->GetMaximum(), peak, 1.);
                fFitFunc->SetParLimits(0, 0, 10000); // offset
                fFitFunc->SetParLimits(1, 0, 10000); // peak
                fFitFunc->SetParLimits(2, -1000, 1000); // peak position
                fFitFunc->SetParLimits(3, 0.1, 20.0); // sigma
                
                // perform fit
                fTimeProj->Fit(fFitFunc, "RBQ0");

                // get parameters
                Double_t mean = fFitFunc->GetParameter(2);
                Double_t error = fFitFunc->GetParError(2);

                // format line
                //fLine->SetVertical();
                //fLine->SetLineColor(4);
                //fLine->SetLineWidth(3);
                //fLine->SetY1(0);
                //fLine->SetY2(hhTWpro[(id-1)]->GetMaximum() + 20);
                //fLine->SetX1(mean_gaus[(id-1)]);
                //fLine->SetX2(mean_gaus[(id-1)]);

                // check fit error
                if (error < 1.)
                {
                    fEnergyProj->SetBinContent(i, mean);
                    fEnergyProj->SetBinError(i, error);
                }

                // plot projection fit   
                //if (!(i % 1))
                //{
                //    fTimeProj->GetXaxis()->SetRangeUser(-100, 100);
                //    fCanvasFit->cd(2);
                //    if (fTimeProj)
                //    {
                //        fTimeProj->Draw();
                //        //fLine->Draw();
                //        fCanvasFit->Update();
                //    }
                //}
            } // if: projection has sufficient statistics
            else
            {
                fEnergyProj->SetBinContent(i, 0);
                fEnergyProj->SetBinError(i, 0);
            }
        } // for: loop over energy bins

        // get configuration
        Double_t low, upp;
        TCReadConfig::GetReader()->GetConfigDoubleDouble("CB.TimeWalk.Histo.Fit.Range", &low, &upp);

        // draw energy projection
        fCanvasResult->cd();
        fEnergyProj->Draw("E1");
        fCanvasResult->Update();

    } // if: sufficient statistics
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;
    
    /*
    // check if fit was performed
    if (!fFitHisto->GetEntries()) unchanged = kTRUE;

    // user information
    printf("Element: %03d    Par0: %12.8f    "
           "Par1: %12.8f    Par2: %12.8f    Par3: %12.8f",
           elem, fPar0[elem], fPar1[elem], fPar2[elem], fPar3[elem]);
    if (unchanged) printf("    -> unchanged");
    if (TCUtils::IsCBHole(elem)) printf(" (hole)");
    printf("\n");
    */
}   

//______________________________________________________________________________
void TCCalibCBTimeWalk::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    Par0: %12.8f    "
               "Par1: %12.8f    Par2: %12.8f    Par3: %12.8f",
               i, fPar0[i], fPar1[i], fPar2[i], fPar3[i]);
    }
}

//______________________________________________________________________________
void TCCalibCBTimeWalk::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    TCMySQLManager::GetManager()->WriteParameters(fSet, kCALIB_CB_WALK0, fPar0, fNelem);
    TCMySQLManager::GetManager()->WriteParameters(fSet, kCALIB_CB_WALK1, fPar1, fNelem);
    TCMySQLManager::GetManager()->WriteParameters(fSet, kCALIB_CB_WALK2, fPar2, fNelem);
    TCMySQLManager::GetManager()->WriteParameters(fSet, kCALIB_CB_WALK3, fPar3, fNelem);
}

