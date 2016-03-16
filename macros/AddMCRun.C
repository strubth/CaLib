/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// AddMCRun.C                                                           //
//                                                                      //
// Add a MC dummy run number to the run database.                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void AddMCRun()
{
    // load CaLib
    gSystem->Load("libCaLib.so");

    // macro configuration: just change here for your beamtime and leave
    // the other parts of the code unchanged
    const Char_t target[]           = "H-Butanol";
    const Int_t dummyRun            = 999004;
    const Char_t calibDesc[]        = "MC calibration for May 2010 beamtime";

    // add raw files to the database
    TCMySQLManager::GetManager()->AddRun(dummyRun, target, calibDesc);

    gSystem->Exit(0);
}

