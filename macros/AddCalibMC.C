// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// AddCalibMC.C                                                         //
//                                                                      //
// Add a MC calibration including a dummy run number and initial        //
// calibrations from AcquRoot configuration files to a CaLib database.  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void AddCalibMC()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // macro configuration: just change here for your beamtime and leave
    // the other parts of the code unchanged
    const Char_t target[]           = "LD2";
    const Int_t dummyRun            = 999001;
    const Char_t calibName[]        = "LD2_MC_Feb_09";
    const Char_t calibDesc[]        = "MC calibration for February 2009 beamtime";
    const Char_t calibFileTagger[]  = "/usr/users/werthm/AcquRoot/acqu/acqu/data/MC_Feb_09/LadderMC.dat";
    const Char_t calibFileCB[]      = "/usr/users/werthm/AcquRoot/acqu/acqu/data/MC_Feb_09/NaI_MC.dat";
    const Char_t calibFileTAPS[]    = "/usr/users/werthm/AcquRoot/acqu/acqu/data/MC_Feb_09/BaF2_MC_09.dat";
    const Char_t calibFilePID[]     = "/usr/users/werthm/AcquRoot/acqu/acqu/data/MC_Feb_09/PID2_MC.dat";
    const Char_t calibFileVeto[]    = "/usr/users/werthm/AcquRoot/acqu/acqu/data/MC_Feb_09/Veto_MC.dat";

    // add raw files to the database
    TCMySQLManager::GetManager()->AddRun(dummyRun, target, calibDesc);
    
    // read AcquRoot calibration of tagger
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAGG, calibFileTagger,
                                             calibName, calibDesc,
                                             dummyRun, dummyRun);
    
    // read AcquRoot calibration of CB
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_CB, calibFileCB,
                                             calibName, calibDesc,
                                             dummyRun, dummyRun);
    
    // init CB quadratic energy correction
    TCMySQLManager::GetManager()->AddSet("Type.CB.Energy.Quad", calibName, calibDesc,
                                         dummyRun, dummyRun, 0);
    
    // init CB LED calibration
    TCMySQLManager::GetManager()->AddSet("Type.CB.LED", calibName, calibDesc,
                                         dummyRun, dummyRun, 0);
    
    // read AcquRoot calibration of TAPS
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAPS, calibFileTAPS,
                                             calibName, calibDesc,
                                             dummyRun, dummyRun);
    
    // init TAPS quadratic energy correction
    TCMySQLManager::GetManager()->AddSet("Type.TAPS.Energy.Quad", calibName, calibDesc,
                                         dummyRun, dummyRun, 0);
 
    // init TAPS LED calibration
    TCMySQLManager::GetManager()->AddSet("Type.TAPS.LED1", calibName, calibDesc,
                                         dummyRun, dummyRun, 0);
    TCMySQLManager::GetManager()->AddSet("Type.TAPS.LED2", calibName, calibDesc,
                                         dummyRun, dummyRun, 0);

    // read AcquRoot calibration of PID
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_PID, calibFilePID,
                                             calibName, calibDesc,
                                             dummyRun, dummyRun);
    
    // read AcquRoot calibration of Veto 
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_VETO, calibFileVeto,
                                             calibName, calibDesc,
                                             dummyRun, dummyRun);
    
    gSystem->Exit(0);
}

