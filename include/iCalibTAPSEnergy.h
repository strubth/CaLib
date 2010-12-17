/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibTAPSEnergy                                                     //
//                                                                      //
// Calibration module for the TAPS energy.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBTAPSENERGY_H
#define ICALIBTAPSENERGY_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "iCalib.h"
#include "iUtils.h"
#include "iFileManager.h"


class iCalibTAPSEnergy : public iCalib
{

private:
    Double_t* fPi0IMOld;                // old pi0 invariant mass values
    Double_t* fPi0IMNew;                // old pi0 invariant mass values
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    iCalibTAPSEnergy();
    virtual ~iCalibTAPSEnergy();

    ClassDef(iCalibTAPSEnergy, 0)   // TAPS energy calibration
};

#endif

