/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller, Lilian Witthauer
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibCBTime                                                         //
//                                                                      //
// Calibration module for the CB time.                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBCBTIME_HH
#define ICALIBCBTIME_HH

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "iCalib.hh"
#include "iFileManager.hh"


class iCalibCBTime : public iCalib
{

private:
    Double_t fTimeGain;                 // CB TDC gain
    Double_t fMean;                     // mean time position
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    iCalibCBTime();
    virtual ~iCalibCBTime();

    ClassDef(iCalibCBTime, 0)   // CB time calibration
};

#endif

