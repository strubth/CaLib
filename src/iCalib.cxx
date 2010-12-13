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
    fMainHisto = 0;
    fCanvasFit = 0;
    fCanvasRes = 0;

    // create arrays
    fOldVal = new Double_t[fNelem];
    fNewVal = new Double_t[fNelem];
    fFitFunc = new TF1*[fNelem];
    fProjHisto = new TH1*[fNelem];

    // init arrays
    for (Int_t i = 0; i < fNelem; i++)
    {
        fOldVal[i] = 0;
        fNewVal[i] = 0;
        fProjHisto[i] = 0;
        fFitFunc[i] = 0;
    }
}

//______________________________________________________________________________
iCalib::~iCalib()
{
    // Destructor.
    
    if (fOldVal) delete [] fOldVal;
    if (fNewVal) delete [] fNewVal;
    if (fFitFunc)
    {
        for (Int_t i = 0; i < fNelem; i++) 
            if (fFitFunc[i]) delete fFitFunc[i];
        delete [] fFitFunc;
    }
    if (fCanvasFit) delete fCanvasFit;
    if (fCanvasRes) delete fCanvasRes;
}

//______________________________________________________________________________
void iCalib::InitGUI()
{
    // Init the basic GUI of a calibration module.
    
    // draw the fitting canvas
    fCanvasFit = new TCanvas("Fitting", "Fitting", 0, 0, 400, 800);
    fCanvasFit->Divide(1, 2, 0.001, 0.001);

    // draw the result canvas
    fCanvasRes = new TCanvas("Result", "Result", 630, 0, 900, 400);

    // call the sub-class GUI customization method
    CustomizeGUI();
}

//______________________________________________________________________________
void iCalib::Start()
{
    // Start the calibration module.

    // print user information
    PrintInfo();

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
        Error("ProcessElement", "Element %d out of bound! (elements: %d)", elem, fNelem);
        return;
    }

    // prepare the fit
    PrepareFit(elem);

    // perform the fit
    Fit(elem);

    // draw the result
    Draw(elem);
}

