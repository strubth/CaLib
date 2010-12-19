// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSTime                                                      //
//                                                                      //
// Calibration module for the TAPS time.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTAPSTIME_H
#define TCCALIBTAPSTIME_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibTAPSTime : public TCCalib
{

private:
    Double_t* fTimeGain;                // TAPS TDC gain
    Double_t fMean;                     // mean time position
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTAPSTime();
    virtual ~TCCalibTAPSTime();

    ClassDef(TCCalibTAPSTime, 0)   // TAPS time calibration
};

#endif

