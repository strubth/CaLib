/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller, Thomas Strub
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
class TGraphErrors;
class TCLine;
class TCFileManager;

class TCCalibCBTimeWalk : public TCCalib
{

private:
    TCFileManager* fFileManager;        // file manager
    Double_t* fPar0;                    // time walk parameter 0
    Double_t* fPar1;                    // time walk parameter 1
    Double_t* fPar2;                    // time walk parameter 2
    Double_t* fPar3;                    // time walk parameter 3

    TGraphErrors* fGFitPoints;          // energy progection fit points
    TH1* fTimeProj;                     // time projection histogram
    TCLine* fLine;                      // mean indicator line
    Int_t fDelay;                       // projection fit display delay
    Bool_t fUseEnergyWeight;            // flag for energy weight
    Bool_t fUsePointDensityWeight;      // flag for energy weight

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibCBTimeWalk();
    virtual ~TCCalibCBTimeWalk();

    virtual void WriteValues();
    virtual void PrintValues();

    ClassDef(TCCalibCBTimeWalk, 0) // CB time walk calibration
};

#endif

