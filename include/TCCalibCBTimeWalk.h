/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibCBTimeWalk                                                    //
//                                                                      //
// Calibration module for the CB time walk.                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBCBTIMEWALK_H
#define TCCALIBCBTIMEWALK_H

#include "TCCalib.h"

class TH1;
class TLine;
class TCFileManager;

class TCCalibCBTimeWalk : public TCCalib
{

private:
    TCFileManager* fFileManager;        // file manager
    Double_t* fPar0;                    // time walk parameter 0
    Double_t* fPar1;                    // time walk parameter 1
    Double_t* fPar2;                    // time walk parameter 2
    Double_t* fPar3;                    // time walk parameter 3
    TH1* fEnergyProj;                   // energy projection histogram
    TH1* fTimeProj;                     // time projection histogram
    TLine* fLine;                       // mean indicator line
    Int_t fDelay;                       // projection fit display delay

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibCBTimeWalk();
    virtual ~TCCalibCBTimeWalk();

    virtual void WriteValues();
    virtual void PrintValues();

    Double_t TWFunc(Double_t* x, Double_t* par);

    ClassDef(TCCalibCBTimeWalk, 0) // CB time walk calibration
};

#endif

