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


#ifndef ICALIBTAPSENERGY_HH
#define ICALIBTAPSENERGY_HH

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "iCalib.hh"
#include "iUtils.hh"
#include "iFileManager.hh"


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

