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


#include "TCCalibRun.h"

ClassImp(TCCalibRun)


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

    if (fIsStarted)
    {
        Error("Start", "Is already started!");
        return kFALSE;
    }

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

    if (fIsStarted)
    {
        Error("Start", "Was already started!");
        return kFALSE;
    }

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

    // start with the first run
    Process(0);

    return kTRUE;
}

//______________________________________________________________________________
void TCCalibRun::Process(Int_t index)
{
    // Processes item 'item'.

    if (!fIsStarted)
    {
        Error("Process", "Not yet started!");
        return;
    }

    // check range
    if (index < 0 || index >= fNRuns)
    {
        Error("Process", "Run index  %d out of allowed range [0,%d]", index, fNRuns);
        return;
    }

    // clean up current run
    CleanUpRun();

    // set current item
    fIndex = index; 

    // setup new item
    PrepareRun();

    // perform item
    ProcessRun();

    // update graphics
    UpdateCanvas();
}

//______________________________________________________________________________
void TCCalibRun::Previous()
{
    // Processes the previous run

    if (!fIsStarted)
    {
        Error("Process", "Not yet started!");
        return;
    }

    // process previous run in list
    Process(fIndex - 1);
}

//______________________________________________________________________________
void TCCalibRun::Next()
{
    // Saves run infos and process the next run. 

    if (!fIsStarted)
    {
        Error("Process", "Not yet started!");
        return;
    }

    // save run
    SaveValRun();

    // process next run in list
    Process(fIndex + 1);
}

//______________________________________________________________________________
void TCCalibRun::Ignore()
{
    // Processes next run w/o saving the calculation of the current current run.

    if (!fIsStarted)
    {
        Error("Process", "Not yet started!");
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
        // left key
        if (oy == kKey_Left) Previous();

        // right key
        if (oy == kKey_Right) Next();
    }
}

