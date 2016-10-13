/************************************************************************
 * Authors: Thomas Strub                                                *
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibRunBadScR_TimeShift                                           //
//                                                                      //
// Beamtime calibration module class for run by run time shift bad      //
// scaler reads calibration.                                            //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBRUNBADSCR_TIMESHIFT_H
#define TCCALIBRUNBADSCR_TIMESHIFT_H

#include "TCCalibRunBadScR.h"

class TCCalibRunBadScR_TimeShift : public TCCalibRunBadScR
{

protected:
    virtual void CalibMethodDefault();

public:
    TCCalibRunBadScR_TimeShift()
      : TCCalibRunBadScR() { }
    TCCalibRunBadScR_TimeShift(const Char_t* name, const Char_t* title, const Char_t* data, Bool_t istruecalib)
      : TCCalibRunBadScR(name, title, data, istruecalib) { }
    virtual ~TCCalibRunBadScR_TimeShift() { }

    ClassDef(TCCalibRunBadScR_TimeShift, 0) // Sudden time shift bad scaler read calibration class
};

class TCCalibRunBadScR_TimeShiftTagger : public TCCalibRunBadScR_TimeShift
{

public:
    TCCalibRunBadScR_TimeShiftTagger()
      : TCCalibRunBadScR_TimeShift("BadScR.TimeShiftTagger", "Bad scaler read calibration (tagger time shift)", "Data.Run.BadScR.TimeShiftTagger", kTRUE) { }
    virtual ~TCCalibRunBadScR_TimeShiftTagger() { }

    ClassDef(TCCalibRunBadScR_TimeShiftTagger, 0) // Sudden tagger time shift bad scaler read calibration class
};

#endif

