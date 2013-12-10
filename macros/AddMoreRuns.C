// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// AddMoreRuns.C                                                        //
//                                                                      //
// Add more runs to an existing calibration database.                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void AddMoreRuns()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // macro configuration: just change here for your needs and leave
    // the other parts of the code unchanged
    const Char_t rawfilePath[]      = "/kernph/data/A2/H-Butanol/Nov_13";
    const Char_t target[]           = "H-Butanol";
    const Int_t newFirstRun         = 0;            // 0 to keep current first run
    const Int_t newLastRun          = 1312;         // 0 to keep current first run
    const Char_t calibName[]        = "H-Butanol_Nov_13";

    // add more raw files to the database
    TCMySQLManager::GetManager()->AddRunFiles(rawfilePath, target);
    
    // set new run range
    TCMySQLManager::GetManager()->ChangeCalibrationRunRange(calibName, newFirstRun, newLastRun);
     
    gSystem->Exit(0);
}

