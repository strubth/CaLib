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


#ifndef ICALIB_H
#define ICALIB_H

#include "TString.h"
#include "TError.h"
#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TTimer.h"
#include "TSystem.h"
#include "TTimeStamp.h"
#include "TStyle.h"

#include "iMySQLManager.h"
#include "iReadConfig.h"


class iCalib : public TNamed
{

protected:
    CalibData_t fData;          // used calibration data
    Int_t fSet;                 // set to be calibrated
    TString fHistoName;         // name of the calibration histogram
    Int_t fNelem;               // number of calibration values
    Int_t fCurrentElem;         // number of current element
    Double_t* fOldVal;          //[fNelem] old calibration value array
    Double_t* fNewVal;          //[fNelem] new calibration value array
    
    TH1* fMainHisto;            // main histogram 
    TH1* fFitHisto;             // fitting histogram
    TF1* fFitFunc;              // fitting function
    Double_t fFitHistoXmin;     // fitting histogram x-axis minimum
    Double_t fFitHistoXmax;     // fitting histogram x-axis maximum

    TH1* fOverviewHisto;        // overview result histogram

    TCanvas* fCanvasFit;        // canvas containing the fits
    TCanvas* fCanvasResult;     // canvas containing the results
    
    TTimer* fTimer;             // slow-motion timer

    virtual void Init() = 0;
    virtual void Fit(Int_t elem) = 0;
    virtual void Calculate(Int_t elem) = 0;

public:
    iCalib() : TNamed(),
               fData(kCALIB_NODATA), 
               fSet(0), fHistoName(), 
               fNelem(0), fCurrentElem(0),
               fOldVal(0), fNewVal(0),
               fMainHisto(0), fFitHisto(0), fFitFunc(0),
               fFitHistoXmin(0), fFitHistoXmax(0),
               fOverviewHisto(0),
               fCanvasFit(0), fCanvasResult(0), 
               fTimer(0) { }
    iCalib(const Char_t* name, const Char_t* title, CalibData_t data,
           Int_t nElem) 
        : TNamed(name, title),
          fData(data), 
          fSet(0), fHistoName(), 
          fNelem(nElem), fCurrentElem(0),
          fOldVal(0), fNewVal(0),
          fMainHisto(0), fFitHisto(0), fFitFunc(0),
          fFitHistoXmin(0), fFitHistoXmax(0),
          fOverviewHisto(0),
          fCanvasFit(0), fCanvasResult(0), 
          fTimer(0) { }
    virtual ~iCalib();
    
    virtual void Write();

    void Start(Int_t set);
    void ProcessAll(Int_t msecDelay = 0);
    void ProcessElement(Int_t elem);
    void Previous();
    void Next();
    void StopProcessing();
    void PrintValues();
    
    CalibData_t GetCalibData() const { return fData; }

    ClassDef(iCalib, 0)         // Base calibration module class
};

#endif

