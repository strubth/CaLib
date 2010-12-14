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
iCalib::iCalib(Int_t set, Int_t nElem)
{
    // Constructor.

    // init members
    fSet = set;
    fNelem = nElem;
    fCurrentElem = 0;
    fMainHisto = 0;
    fFitHisto = 0;
    fFitFunc = 0;
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
}

//______________________________________________________________________________
iCalib::~iCalib()
{
    // Destructor.
    
    if (fOldVal) delete [] fOldVal;
    if (fNewVal) delete [] fNewVal;
    if (fMainHisto) delete fMainHisto;
    if (fFitHisto) delete fFitHisto;
    if (fFitFunc) delete fFitFunc;
    if (fCanvasFit) delete fCanvasFit;
    if (fCanvasResult) delete fCanvasResult;
    if (fTimer) delete fTimer;
}

//______________________________________________________________________________
void iCalib::InitGUI()
{
    // Init the basic GUI of a calibration module.
    
    // draw the fitting canvas
    fCanvasFit = new TCanvas("Fitting", "Fitting", 0, 0, 400, 800);
    fCanvasFit->Divide(1, 2, 0.001, 0.001);

    // draw the result canvas
    fCanvasResult = new TCanvas("Result", "Result", 630, 0, 900, 400);

    // call the sub-class GUI customization method
    CustomizeGUI();
}

//______________________________________________________________________________
void iCalib::Start()
{
    // Start the calibration module.

    // init the GUI
    InitGUI();

    // start with the first element
    ProcessElement(1);
}

//______________________________________________________________________________
void iCalib::ProcessElement(Int_t elem)
{
    // Process the element 'elem'.

    // check if element is in range
    if (elem <= 0 || elem > fNelem)
    {
        // stop timer when it was active
        if (fTimer) 
        {
            fTimer->Stop();
            delete fTimer;
            fTimer = 0;
            return;
        }

        Error("ProcessElement", "Element %d out of bound! (elements: %d)", elem, fNelem);
        return;
    }
    
    // set current element
    fCurrentElem = elem;

    // process element
    Process(elem);
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
               i+1, fOldVal[i], fNewVal[i]);
    }
}

