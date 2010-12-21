// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTaggerTime                                                    //
//                                                                      //
// Calibration module for the Tagger time.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTAGGERTIME_H
#define TCCALIBTAGGERTIME_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCFileManager.h"
#include "TCUtils.h"


class TCCalibTaggerTime : public TCCalib
{

private:
    Double_t fTimeGain;                 // Tagger TDC gain
    Double_t fMean;                     // mean time position
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTaggerTime();
    virtual ~TCCalibTaggerTime();

    ClassDef(TCCalibTaggerTime, 0) // Tagger time calibration
};

#endif

