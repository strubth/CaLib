/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibCBEnergy                                                       //
//                                                                      //
// Calibration module for the CB energy.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBCBENERGY_H
#define ICALIBCBENERGY_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "iCalib.h"
#include "iUtils.h"
#include "iFileManager.h"


class iCalibCBEnergy : public iCalib
{

private:
    Double_t* fPi0IMOld;                // old pi0 invariant mass values
    Double_t* fPi0IMNew;                // old pi0 invariant mass values
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    iCalibCBEnergy();
    virtual ~iCalibCBEnergy();

    ClassDef(iCalibCBEnergy, 0)   // CB energy calibration
};

#endif

