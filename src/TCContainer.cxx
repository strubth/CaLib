// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCContainer                                                          //
//                                                                      //
// Run information and calibration storage class.                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCContainer.h"

ClassImp(TCContainer)


//______________________________________________________________________________
TCContainer::TCContainer(const Char_t* name)
    : TNamed(name, name)
{
    // Constructor.

    // init members
    fRuns = new TList();
    fRuns->SetOwner(kTRUE);
    fCalibrations = new TList();
    fCalibrations->SetOwner(kTRUE);
}

//______________________________________________________________________________
TCContainer::~TCContainer()
{
    // Destructor.
    
    if (fRuns) delete fRuns;
    if (fCalibrations) delete fCalibrations;
}

//______________________________________________________________________________
TCRun* TCContainer::AddRun(Int_t run)
{
    // Add a new run using the run number 'run' to the list of runs and return
    // the created run object.
    
    // create the run
    TCRun* r = new TCRun();

    // set the run number
    r->SetRun(run);

    // add it to the list of runs
    fRuns->Add(r);

    return r;
}

//______________________________________________________________________________
TCCalibration* TCContainer::AddCalibration(const Char_t* calibration)
{
    // Add a new calibration using the name 'calibration' to the list of
    // calibrations and return the created calibration object.
    
    // create the calibration
    TCCalibration* c = new TCCalibration();

    // set the name
    c->SetCalibration(calibration);

    // add it to the list of calibrations
    fCalibrations->Add(c);

    return c;
}

//______________________________________________________________________________
void TCContainer::SaveAs(const Char_t* filename)
{
    // Save this container to the ROOT file 'filename'.

    // try to open the ROOT file
    TFile* f = new TFile(filename, "RECREATE");
    if (!f)
    {
        Error("SaveAs", "Could not create file '%s'!", filename);
        return;
    }
    if (f->IsZombie())
    {
        Error("SaveAs", "Could not create file '%s'!", filename);
        return;
    }

    // save me
    this->Write();
    
    // user information
    Info("SaveAs", "CaLib data was saved to '%s'", filename);

    // close file
    delete f;
}

//______________________________________________________________________________
void TCContainer::Print()
{
    // Print content information.

    printf("CaLib Container Information\n");
    printf("Number of runs         : %d\n", GetNRuns());
    printf("Number of calibrations : %d\n", GetNCalibrations());
    printf("\n");
}

//______________________________________________________________________________
void TCContainer::ShowRuns()
{
    // Print the information of all runs.

    // loop over runs
    TIter next(fRuns);
    TCRun* r;
    while ((r = (TCRun*)next())) r->Print();
}

//______________________________________________________________________________
void TCContainer::ShowCalibrations()
{
    // Print the information of all calibrations.

    // loop over runs
    TIter next(fCalibrations);
    TCCalibration* c;
    while ((c = (TCCalibration*)next())) c->Print();
}

