// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibCBRiseTime                                                    //
//                                                                      //
// Calibration module for the CB rise time.                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBCBRISETIME_H
#define TCCALIBCBRISETIME_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCFileManager.h"
#include "TCUtils.h"


class TCCalibCBRiseTime : public TCCalib
{

private:
    Double_t fMean;                     // mean time position
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibCBRiseTime();
    virtual ~TCCalibCBRiseTime();

    ClassDef(TCCalibCBRiseTime, 0) // CB rise time calibration
};

#endif

