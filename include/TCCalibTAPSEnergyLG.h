// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSEnergyLG                                                  //
//                                                                      //
// Calibration module for the TAPS LG energy.                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTAPSENERGYLG_H
#define TCCALIBTAPSENERGYLG_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibTAPSEnergyLG : public TCCalib
{

private:
    Double_t fPi0Pos;                   // pi0 position
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTAPSEnergyLG();
    virtual ~TCCalibTAPSEnergyLG();

    ClassDef(TCCalibTAPSEnergyLG, 0) // TAPS LG energy calibration
};

#endif

