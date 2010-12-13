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
#include "TF1.h"
#include "TCanvas.h"


class iCalib 
{

protected:
    Int_t fSet;                 // set to be calibrated
    TString fHistoName;         // name of the calibration histogram
    Int_t fNelem;               // number of calibration values
    Double_t* fOldVal;          //[fNelem] old calibration value array
    Double_t* fNewVal;          //[fNelem] new calibration value array
    TH1* fMainHisto;            // main histogram 
    TH1** fProjHisto;           //[fNelem] array of projected histograms
    TF1** fFitFunc;             //[fNelem] array of fitting functions
    TCanvas* fCanvasFit;        // canvas containing the fits
    TCanvas* fCanvasRes;        // canvas containing the results
    
    void InitGUI();
    void ProcessElement(Int_t elem);
    virtual void PrintInfo() = 0;
    virtual void CustomizeGUI() = 0;
    virtual void PrepareFit(Int_t elem) = 0;
    virtual void Fit(Int_t elem) = 0;
    virtual void Draw(Int_t elem) = 0;

public:
    iCalib() : fSet(0), fHistoName(), 
               fNelem(0), fOldVal(0), fNewVal(0),
               fMainHisto(0), fProjHisto(0),
               fFitFunc(0),
               fCanvasFit(0), fCanvasRes(0) { }
    iCalib(Int_t set, Int_t nElem);
    virtual ~iCalib();
    
    void Start();
    virtual void Write() = 0;

    ClassDef(iCalib, 0)         // Base calibration module class
};

#endif

