/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibVetoCorr                                                      //
//                                                                      //
// Calibration module for the Veto correlation.                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBVETOCORR_H
#define TCCALIBVETOCORR_H

#include "TCCalib.h"

class TCReadARCalib;

class TCCalibVetoCorr : public TCCalib
{

private:
    Int_t fMax;                         // maximum element
    TCReadARCalib* fARCalib;            // AcquRoot calibration file reader

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

    void ReadNeighbours();

public:
    TCCalibVetoCorr();
    virtual ~TCCalibVetoCorr();

    virtual void WriteValues();
    virtual void PrintValues();
    virtual void PrintValuesChanged();

    ClassDef(TCCalibVetoCorr, 0) // Veto correlation
};

#endif

