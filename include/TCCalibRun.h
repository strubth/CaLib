/*************************************************************************
 * Author: Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibRun                                                           //
//                                                                      //
// Abstract beamtime calibration module class for run by run            //
// calibration.                                                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBRUN_H
#define TCCALIBRUN_H

#include "TNamed.h"
#include "TCMySQLManager.h"
#include "TCARRunHistoLoader.h"
#include "KeySymbols.h"
#include "TCanvas.h"


class TCCalibRun : public TNamed
{

protected:
    //----------------------------- data members -------------------------------

    TString* fCalibration;              // calibration identifier
    TString* fCalibData;                // calibration data type

    Int_t fNRuns;                       // number of runs
    Int_t* fRuns;                       //[fNRuns] array of run numbers

    Int_t fIndex;                       //! index of current item performed

    Bool_t fIsStarted;                  // is started flag


    //---------------------------- member methods ------------------------------

    // setup functions
    virtual Bool_t SetConfig();
    virtual Bool_t Init();

    // run handle functions
    virtual void PrepareRun() = 0;
    virtual void ProcessRun() = 0;
    virtual void SaveValRun() = 0;
    virtual void CleanUpRun() = 0;

    // graphic functions
    virtual void UpdateCanvas() { };

public:
    TCCalibRun()
        : TNamed(),
          fCalibration(0), fCalibData(0),
          fNRuns(0), fRuns(0), fIndex(0), fIsStarted(kFALSE) { };
    TCCalibRun(const Char_t* name, const Char_t* title, const Char_t* data)
        : TNamed(name, title),
          fCalibration(0), fCalibData(new TString(data)),
          fNRuns(0), fRuns(0), fIndex(0), fIsStarted(kFALSE) { };
    virtual ~TCCalibRun();

    // start functions
    Bool_t Start(Int_t nruns, const Int_t* runs);
    Bool_t Start(const Char_t* calibration);
    Bool_t IsStarted() { return fIsStarted; };

    // navigation functions
    virtual void Process(Int_t run);
    virtual void Previous();
    virtual void Next();
    virtual void Ignore();
    virtual void Finish() { };
    virtual Bool_t Write() = 0;

    // event handler
    virtual void EventHandler(Int_t event, Int_t ox, Int_t oy, TObject* selected);

    ClassDef(TCCalibRun, 0) // Abstract item calibration module class
};

#endif

