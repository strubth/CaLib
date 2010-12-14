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


#ifndef ICALIB_HH
#define ICALIB_HH

#include "TString.h"
#include "TError.h"
#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TTimer.h"


class iCalib 
{

protected:
    Int_t fSet;                 // set to be calibrated
    TString fHistoName;         // name of the calibration histogram
    Int_t fNelem;               // number of calibration values
    Int_t fCurrentElem;         // number of current element
    Double_t* fOldVal;          //[fNelem] old calibration value array
    Double_t* fNewVal;          //[fNelem] new calibration value array
    
    TH1* fMainHisto;            // main histogram 
    TH1* fFitHisto;             // fitting histogram
    TF1* fFitFunc;              // fitting function
    TCanvas* fCanvasFit;        // canvas containing the fits
    TCanvas* fCanvasResult;     // canvas containing the results
    
    TTimer* fTimer;             // slow-motion timer

    void InitGUI();
    virtual void CustomizeGUI() = 0;
    virtual void Process(Int_t elem) = 0;

public:
    iCalib() : fSet(0), fHistoName(), 
               fNelem(0), fCurrentElem(0),
               fOldVal(0), fNewVal(0),
               fMainHisto(0), fFitHisto(0),
               fFitFunc(0),
               fCanvasFit(0), fCanvasResult(0), 
               fTimer(0) { }
    iCalib(Int_t set, Int_t nElem);
    virtual ~iCalib();
    
    void Start();
    void ProcessAll(Int_t msecDelay = 0);
    void ProcessElement(Int_t elem);
    void Previous();
    void Next();
    virtual void Write() = 0;

    ClassDef(iCalib, 0)         // Base calibration module class
};

#endif

