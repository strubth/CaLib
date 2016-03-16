/************************************************************************
 * Author: Thomas Strub                                                 *
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibRun                                                           //
//                                                                      //
// Abstract run by run calibration class.                               //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBRUN_H
#define TCCALIBRUN_H

#include "TNamed.h"

class TCCalibRun : public TNamed
{

protected:
    //----------------------------- data members -------------------------------

    TString* fCalibration;              // calibration identifier
    TString* fCalibData;                // calibration data type

    Bool_t fIsTrueCalib;                // flag for a true calibration class

    Int_t fNRuns;                       // number of runs
    Int_t* fRuns;            //[fNRuns] // array of run numbers

    Int_t fIndex;                   //! // index of current item performed

    Bool_t fIsStarted;                  // is started flag


    //---------------------------- member methods ------------------------------

    // setup functions
    virtual Bool_t SetConfig();
    virtual Bool_t Init();

    // run handle functions
    virtual void PrepareCurr() = 0;
    virtual void ProcessCurr() = 0;
    virtual void SaveValCurr() = 0;
    virtual void CleanUpCurr() = 0;

    // graphic functions
    virtual void UpdateCanvas() { };

public:
    TCCalibRun()
        : TNamed(),
          fCalibration(0), fCalibData(0), fIsTrueCalib(kFALSE),
          fNRuns(0), fRuns(0), fIndex(0), fIsStarted(kFALSE) { };
    TCCalibRun(const Char_t* name, const Char_t* title, const Char_t* data, Bool_t istruecalib = kFALSE)
        : TNamed(name, title),
          fCalibration(0), fCalibData(new TString(data)), fIsTrueCalib(istruecalib),
          fNRuns(0), fRuns(0), fIndex(0), fIsStarted(kFALSE) { };
    virtual ~TCCalibRun();

    void SetIsTrueCalib(Bool_t istruecalib = kTRUE) { fIsTrueCalib = istruecalib; };
    virtual Bool_t IsTrueCalib() const { return fIsTrueCalib; };

    // start functions
    Bool_t Start(Int_t nruns, const Int_t* runs);
    Bool_t Start(const Char_t* calibration);
    Bool_t IsStarted() { return fIsStarted; };

    // navigation functions
    virtual void Process(Int_t index);
    virtual void Previous();
    virtual void Next();
    virtual void Skip();
    virtual void Finish() { };
    virtual Bool_t Write() = 0;

    // event handler
    virtual void EventHandler(Int_t event, Int_t ox, Int_t oy, TObject* selected);

    ClassDef(TCCalibRun, 0) // Abstract run by run calibration class
};

#endif

