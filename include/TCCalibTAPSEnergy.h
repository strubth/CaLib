// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSEnergy                                                    //
//                                                                      //
// Calibration module for the TAPS energy.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTAPSENERGY_H
#define TCCALIBTAPSENERGY_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCUtils.h"
#include "TCFileManager.h"


class TCCalibTAPSEnergy : public TCCalib
{

private:
    Double_t fPi0Pos;                   // pi0 position
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTAPSEnergy();
    virtual ~TCCalibTAPSEnergy();

    ClassDef(TCCalibTAPSEnergy, 0)   // TAPS energy calibration
};

#endif

