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
    const Char_t rawfilePath[]      = "/usr/lynx_scratch1/data/A2/D-Butanol/Feb_14";
    const Char_t target[]           = "D-Butanol";
    const Int_t newFirstRun         = 1924;            // 0 to keep current first run
    const Int_t newLastRun          = 3011;            // 0 to keep current first run
    const Char_t calibName[]        = "D-Butanol_Feb_14";

    // add more raw files to the database
    TCMySQLManager::GetManager()->AddRunFiles(rawfilePath, target, "CBTaggTAPS");
    TCMySQLManager::GetManager()->AddRunFiles(rawfilePath, target, "CBTaggTAPSPed");
    
    // set new run range
    TCMySQLManager::GetManager()->ChangeCalibrationRunRange(calibName, newFirstRun, newLastRun);
     
    gSystem->Exit(0);
}

