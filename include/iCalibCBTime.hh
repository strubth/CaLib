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
#include "iConfig.hh"
#include "iMySQLManager.hh"
#include "iFileManager.hh"


class iCalibCBTime : public iCalib
{

private:
    Double_t fTimeGain;                 // CB TDC gain
    Double_t fMean;                     // mean time position
    TLine* fLine;                       // indicator line
    
    virtual void CustomizeGUI();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    iCalibCBTime() : iCalib(), 
                     fTimeGain(0), fMean(0), fLine(0) { }
    iCalibCBTime(Int_t set);
    virtual ~iCalibCBTime();

    ClassDef(iCalibCBTime, 0)   // CB time calibration
};

#endif

