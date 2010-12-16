/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalib                                                               //
//                                                                      //
// Abstract calibration module class.                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "iCalib.hh"

ClassImp(iCalib)


//______________________________________________________________________________
iCalib::~iCalib()
{
    // Destructor.
    
    if (fOldVal) delete [] fOldVal;
    if (fNewVal) delete [] fNewVal;
    if (fMainHisto) delete fMainHisto;
    if (fFitHisto) delete fFitHisto;
    if (fFitFunc) delete fFitFunc;
    if (fOverviewHisto) delete fOverviewHisto;
    if (fCanvasFit) delete fCanvasFit;
    if (fCanvasResult) delete fCanvasResult;
    if (fTimer) delete fTimer;
}

//______________________________________________________________________________
void iCalib::Start(Int_t set)
{
    // Start the calibration module for the set 'set'.
    
    // init members
    fSet = set;
    fHistoName = "";
    fCurrentElem = 0;

    fMainHisto = 0;
    fFitHisto = 0;
    fFitFunc = 0;
    
    fOverviewHisto = 0;

    fCanvasFit = 0;
    fCanvasResult = 0;
    
    fTimer = 0;
    
    // create arrays
    fOldVal = new Double_t[fNelem];
    fNewVal = new Double_t[fNelem];

    // init arrays
    for (Int_t i = 0; i < fNelem; i++)
    {
        fOldVal[i] = 0;
        fNewVal[i] = 0;
    }
    
    // user information
    Info("Start", "Starting calibration module %s", GetName());
    Info("Start", "Module description: %s", GetTitle());

    // draw the fitting canvas
    fCanvasFit = new TCanvas("Fitting", "Fitting", 0, 0, 400, 800);
    fCanvasFit->Divide(1, 2, 0.001, 0.001);

    // draw the result canvas
    fCanvasResult = new TCanvas("Result", "Result", 630, 0, 900, 400);
    
    // init sub-class
    Init();

    // start with the first element
    ProcessElement(0);
}

//______________________________________________________________________________
void iCalib::ProcessElement(Int_t elem)
{
    // Process the element 'elem'.

    // check if element is in range
    if (elem < 0 || elem >= fNelem)
    {
        // stop timer when it was active
        if (fTimer) 
        {
            fTimer->Stop();
            delete fTimer;
            fTimer = 0;
            return;
        }
        
        // calculate last element
        if (elem == fNelem) Calculate(fCurrentElem);
        
        // exit
        return;
    }
    
    // calculate previous element
    if (elem != fCurrentElem) Calculate(fCurrentElem);

    // set current element
    fCurrentElem = elem;

    // process element
    Fit(elem);
}

//______________________________________________________________________________
void iCalib::ProcessAll(Int_t msecDelay)
{
    // Process all elements using 'msecDelay' milliseconds delay.
    
    // check for delay
    if (msecDelay > 0)
    {
        // create timer
        fTimer = new TTimer();
        fTimer->Connect("Timeout()", "iCalib", this, "Next()");

        // start automatic iteration
        fTimer->Start(msecDelay);
    }
    else
    {
        // loop over elements
        for (Int_t i = 0; i < fNelem; i++) Next();
    }
}

//______________________________________________________________________________
void iCalib::Previous()
{
    // Process the previous element.

    ProcessElement(fCurrentElem - 1);
}

//______________________________________________________________________________
void iCalib::Next()
{
    // Process the next element.
    
    ProcessElement(fCurrentElem + 1);
}

//______________________________________________________________________________
void iCalib::StopProcessing()
{
    // Stop processing when in automatic mode.
    
    // stop timer when it was active
    if (fTimer) 
    {
        fTimer->Stop();
        delete fTimer;
        fTimer = 0;
    }
}

//______________________________________________________________________________
void iCalib::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    old value: %12.8f    new value: %12.8f\n",
               i, fOldVal[i], fNewVal[i]);
    }
}

//______________________________________________________________________________
void iCalib::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    iMySQLManager::GetManager()->WriteParameters(fSet, fData, fNewVal, fNelem);
        
    // save overview picture
    if (TString* path = iReadConfig::GetReader()->GetConfig("Log.Images"))
    {
        Char_t tmp[256];
        // create directory
        sprintf(tmp, "%s/%s", path->Data(), GetName());
        gSystem->mkdir(tmp, kTRUE);
    
        // save canvas
        sprintf(tmp, "%s/%s/overview_set_%d.png", path->Data(), GetName(), fSet);
        fCanvasResult->SaveAs(tmp);
    }
}

