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


#include "KeySymbols.h"
#include "Buttons.h"
#include "TTimer.h"

#include "TCCalibRun.h"
#include "TCMySQLManager.h"

ClassImp(TCCalibRun)

// init static variable
Bool_t TCCalibRun::fIsStarted = kFALSE;

//______________________________________________________________________________
TCCalibRun::~TCCalibRun()
{
    // Destructor

    if (fCalibration) delete fCalibration;
    if (fCalibData) delete fCalibData;
    if (fRuns) delete [] fRuns;
}

//______________________________________________________________________________
Bool_t TCCalibRun::SetConfig()
{
    // Set configuration from config file. Can be called from the chlid class'
    // 'SetConfig()' methode initially called from 'Start()'.

    // nothing to do here yet

    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCCalibRun::Init()
{
    // Initialization methode. Can be called from the child class' 'Init()'
    // methode initially called from 'Start()'.

    // nothing to do here yet

    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCCalibRun::Start(Int_t nruns, const Int_t* runs)
{
    // Sets the configuration for base and child class ('SetConfig()'), prepares
    // run number array (from calib database), loads the histos, inits child and
    // processes first run.

    // check whether already started
    if (fIsStarted)
    {
        Error("Start", "Is already started!");
        return kFALSE;
    }

    // delete old run list
    if (fRuns) delete [] fRuns;

    // set runs
    fNRuns = nruns;
    fRuns = new Int_t[fNRuns];
    for (Int_t i = 0; i < fNRuns; i++)
        fRuns[i] = runs[i];

    // set child configuration
    if (!SetConfig()) return kFALSE;

    // init child (which can call parent init)
    if (!Init()) return kFALSE;

    // set started flag
    fIsStarted = kTRUE;

    // user info
    Info("Start", "Starting calibration...");

    // start with the first run
    Process(0);

    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCCalibRun::Start(const Char_t* calibration)
{
    // Sets the configuration for base and child class ('SetConfig()'), prepares
    // run number array (from calib database), loads the histos, inits child and
    // processes first run.

    // check whether already started
    if (fIsStarted)
    {
        Error("Start", "Was already started!");
        return kFALSE;
    }

    // delete old run list
    if (fRuns) delete [] fRuns;

    // set calibration
    fCalibration = new TString(calibration);

    // create the run list
    fRuns = TCMySQLManager::GetManager()->GetRunsOfCalibration((*fCalibration).Data(), &fNRuns);
    if (!fRuns) return kFALSE;

    // set child configuration
    if (!SetConfig()) return kFALSE;

    // init child (which can call parent init)
    if (!Init()) return kFALSE;

    // set started flag
    fIsStarted = kTRUE;

    // user info
    Info("Start", "Starting calibration...");

    // start with the first run
    Process(0);

    return kTRUE;
}

//______________________________________________________________________________
void TCCalibRun::ProcessAuto(Bool_t start /*= kTRUE*/, Int_t msecDelay /*= -1*/)
{
    // Process elements using 'msecDelay' milliseconds delay.

    // check whether already started
    if (!fIsStarted)
    {
        Error("ProcessAuto", "Not yet started!");
        return;
    }

    // set up timer
    static TTimer fTimer(100);

    // stop timer
    fTimer.Stop();

    // check stop signal
    if (start == kFALSE)
        return;

    // check range
    if (fIndex+1 >= fNRuns)
        return;

    // perform next run
    Next();

    // re-connect timer
    fTimer.Connect("Timeout()", "TCCalibRun", this, "ProcessAuto()");

    // start timer
    fTimer.Start(msecDelay, kFALSE);
}

//______________________________________________________________________________
void TCCalibRun::Process(Int_t index)
{
    // Processes run with index 'index'.

    // check whether already started
    if (!fIsStarted)
    {
        Error("Process", "Not yet started!");
        return;
    }

    // check whether index is in range
    if (index < 0 || index >= fNRuns)
    {
        Error("Process", "Run index  %d out of allowed range [0,%d]", index, fNRuns);
        return;
    }

    // clean up current run
    CleanUpCurr();

    // set current run index
    fIndex = index;

    // setup current run
    PrepareCurr();

    // process current run
    ProcessCurr();

    // update canvas
    UpdateCanvas();
}

//______________________________________________________________________________
void TCCalibRun::Previous()
{
    // Processes the previous run in list

    // check whether already started
    if (!fIsStarted)
    {
        Error("Previous", "Not yet started!");
        return;
    }

    // process previous run in list
    Process(fIndex - 1);
}

//______________________________________________________________________________
void TCCalibRun::Next()
{
    // Saves calibration values for current run and processes the next run.

    // check whether already started
    if (!fIsStarted)
    {
        Error("Next", "Not yet started!");
        return;
    }

    // save values for current run
    SaveValCurr();

    // process next run in list
    Process(fIndex + 1);
}

//______________________________________________________________________________
void TCCalibRun::ReProcess()
{
    // Reprocess the fit of the run with index 'fIndex', but with the
    // 'fIsReProcess' flag set.

    // check whether already started
    if (!fIsStarted)
    {
        Error("ReProcess", "Not yet started!");
        return;
    }

    // set re-process flag
    fIsReProcess = kTRUE;

    // process current run
    ProcessCurr();

    // update canvas
    UpdateCanvas();

    // unset re-process flag
    fIsReProcess = kFALSE;
}

//______________________________________________________________________________
void TCCalibRun::Skip()
{
    // Processes next run w/o saving the calibration values for the current.

    // check whether already started
    if (!fIsStarted)
    {
        Error("Skip", "Not yet started!");
        return;
    }

    // process next run in list
    Process(fIndex + 1);
}

//______________________________________________________________________________
void TCCalibRun::EventHandler(Int_t event, Int_t ox, Int_t oy, TObject* selected)
{
    // Event handler method.

    // catch key events ????
    if (event == kKeyPress)
    {
        // 'a' key
        if (oy == kKey_a) Previous();

        // 's' key
        if (oy == kKey_s) Next();
    }
}

