/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTargetPosition                                                //
//                                                                      //
// Calibration module for the target position.                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTARGETPOSITION_H
#define TCCALIBTARGETPOSITION_H

#include "TCCalib.h"

class TLine;

class TCCalibTargetPosition : public TCCalib
{

private:
    Double_t fSigmaPrev;                // previous pi0 peak sigma
    TLine* fLine;                       // indicator line

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTargetPosition();
    virtual ~TCCalibTargetPosition();

    virtual void WriteValues();
    virtual void PrintValues();

    ClassDef(TCCalibTargetPosition, 0) // target position calibration
};

#endif

