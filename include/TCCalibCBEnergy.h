// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibCBEnergy                                                      //
//                                                                      //
// Calibration module for the CB energy.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBCBENERGY_H
#define TCCALIBCBENERGY_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCUtils.h"
#include "TCFileManager.h"


class TCCalibCBEnergy : public TCCalib
{

private:
    Double_t fPi0Pos;                   // pi0 position
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibCBEnergy();
    virtual ~TCCalibCBEnergy();

    ClassDef(TCCalibCBEnergy, 0) // CB energy calibration
};

#endif

