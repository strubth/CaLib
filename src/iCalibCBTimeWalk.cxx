/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibCBTimeWalk                                                     //
//                                                                      //
// Calibration module for the CB time walk.                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "iCalibCBTimeWalk.h"

ClassImp(iCalibCBTimeWalk)


//______________________________________________________________________________
iCalibCBTimeWalk::iCalibCBTimeWalk()
    : iCalib("CB.TimeWalk", "CB time walk calibration", kCALIB_CB_WALK1, iConfig::kMaxCB)
{
    // Empty constructor.

    // init members
    fFileManager = 0;
    fPar0 = 0;
    fPar1 = 0;
    fPar2 = 0;
    fPar3 = 0;
}

//______________________________________________________________________________
iCalibCBTimeWalk::~iCalibCBTimeWalk()
{
    // Destructor. 
    
    if (fFileManager) delete fFileManager;
    if (fPar0) delete [] fPar0;
    if (fPar1) delete [] fPar1;
    if (fPar2) delete [] fPar2;
    if (fPar3) delete [] fPar3;
}

//______________________________________________________________________________
void iCalibCBTimeWalk::Init()
{
    // Init the module.
    
    // init members
    fFileManager = new iFileManager(fSet, fData);
    fPar0 = new Double_t[fNelem];
    fPar1 = new Double_t[fNelem];
    fPar2 = new Double_t[fNelem];
    fPar3 = new Double_t[fNelem];

    // get histogram name
    if (!iReadConfig::GetReader()->GetConfig("CB.TimeWalk.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *iReadConfig::GetReader()->GetConfig("CB.TimeWalk.Histo.Fit.Name");
    
    // read old parameters
    iMySQLManager::GetManager()->ReadParameters(fSet, kCALIB_CB_WALK0, fPar0, fNelem);
    iMySQLManager::GetManager()->ReadParameters(fSet, kCALIB_CB_WALK1, fPar1, fNelem);
    iMySQLManager::GetManager()->ReadParameters(fSet, kCALIB_CB_WALK2, fPar2, fNelem);
    iMySQLManager::GetManager()->ReadParameters(fSet, kCALIB_CB_WALK3, fPar3, fNelem);

    // get parameters from configuration file
    fFitHistoXmin = iReadConfig::GetReader()->GetConfigDouble("CB.TimeWalk.Histo.Fit.Xaxis.Min");
    fFitHistoXmax = iReadConfig::GetReader()->GetConfigDouble("CB.Timeralk.Histo.Fit.Xaxis.Max");

    // draw main histogram
    fCanvasFit->cd(1)->SetLogz();

    // draw the overview histogram
    fCanvasResult->cd();
}

//______________________________________________________________________________
void iCalibCBTimeWalk::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    
    // create histogram name
    sprintf(tmp, "%s_%03d", fHistoName.Data(), elem);
    
    // delete old histogram
    if (fMainHisto) delete fMainHisto;

    fMainHisto = (TH2*) fFileManager->GetHistogram(tmp);
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
 
    // draw main histogram
    fCanvasFit->cd(1);
    fMainHisto->GetXaxis()->SetRangeUser(fFitHistoXmin, fFitHistoXmax);
    fMainHisto->Draw("colz");
    fCanvasFit->Update();
}

//______________________________________________________________________________
void iCalibCBTimeWalk::Calculate(Int_t elem)
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
    printf("\n");
    */
}   

//______________________________________________________________________________
void iCalibCBTimeWalk::PrintValues()
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
void iCalibCBTimeWalk::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    iMySQLManager::GetManager()->WriteParameters(fSet, kCALIB_CB_WALK0, fPar0, fNelem);
    iMySQLManager::GetManager()->WriteParameters(fSet, kCALIB_CB_WALK1, fPar1, fNelem);
    iMySQLManager::GetManager()->WriteParameters(fSet, kCALIB_CB_WALK2, fPar2, fNelem);
    iMySQLManager::GetManager()->WriteParameters(fSet, kCALIB_CB_WALK3, fPar3, fNelem);
}

