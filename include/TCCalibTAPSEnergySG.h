// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSEnergySG                                                  //
//                                                                      //
// Calibration module for the TAPS SG energy.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTAPSENERGYSG_H
#define TCCALIBTAPSENERGYSG_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TMath.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibTAPSEnergySG : public TCCalib
{

private:
    Double_t fMean;                     // mean position
    TCFileManager* fFileManager;        // file manager
    TLine* fLine;                       // mean indicator line

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTAPSEnergySG();
    virtual ~TCCalibTAPSEnergySG();

    ClassDef(TCCalibTAPSEnergySG, 0) // TAPS SG energy calibration
};

#endif

