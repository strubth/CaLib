/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibCBTimeWalk                                                     //
//                                                                      //
// Calibration module for the CB time walk.                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBCBTIMEWALK_H
#define ICALIBCBTIMEWALK_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "iCalib.h"
#include "iFileManager.h"


class iCalibCBTimeWalk : public iCalib
{

private:
    iFileManager* fFileManager;         // file manager
    Double_t* fPar0;                    // time walk parameter 0
    Double_t* fPar1;                    // time walk parameter 1
    Double_t* fPar2;                    // time walk parameter 2
    Double_t* fPar3;                    // time walk parameter 3

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    iCalibCBTimeWalk();
    virtual ~iCalibCBTimeWalk();

    virtual void Write();
    virtual void PrintValues();

    ClassDef(iCalibCBTimeWalk, 0)   // CB time walk calibration
};

#endif

