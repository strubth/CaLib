// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// AddBeamtime.C                                                        //
//                                                                      //
// Add a beamtime including raw data files and initial calibrations     //
// from AcquRoot configuration files to a CaLib database.               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


void AddBeamtime()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // macro configuration: just change here for your beamtime and leave
    // the other parts of the code unchanged
    const Char_t rawfilePath[]      = "/kernph/data/A2/LD2/Dec_07";
    const Char_t target[]           = "LD2";
    const Int_t firstRun            = 13089;
    const Int_t lastRun             = 13841;
    const Char_t calibName[]        = "LD2_Dec_07_Eta";
    const Char_t calibDesc[]        = "Calibration for December 2007 optimized for eta-production analysis";
    const Char_t calibFileTagger[]  = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Dec_07/Tagger/FP_Dec07.dat";
    const Char_t calibFileCB[]      = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Dec_07/CB/NaI.dat";
    const Char_t calibFileTAPS[]    = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Dec_07/TAPS/BaF2.dat";
    const Char_t calibFilePID[]     = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Dec_07/PID/PID.dat";
    const Char_t calibFileVETO[]    = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Dec_07/TAPS/Veto.dat";

    // add raw files to the database
    TCMySQLManager::GetManager()->AddRunFiles(rawfilePath, target);
    
    // add target position calibration
    TCMySQLManager::GetManager()->AddSet(kCALIB_TARGET_POS, calibName, calibDesc,
                                         firstRun, lastRun, 0);
 
    // read AcquRoot calibration of tagger
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAGG, calibFileTagger,
                                             calibName, calibDesc,
                                             firstRun, lastRun);
     
    // read AcquRoot calibration of CB
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_CB, calibFileCB,
                                             calibName, calibDesc,
                                             firstRun, lastRun);
    
    // init CB time walk calibration
    TCMySQLManager::GetManager()->AddSet(kCALIB_CB_WALK0, calibName, calibDesc,
                                         firstRun, lastRun, 0);
    TCMySQLManager::GetManager()->AddSet(kCALIB_CB_WALK1, calibName, calibDesc,
                                         firstRun, lastRun, 0);
    TCMySQLManager::GetManager()->AddSet(kCALIB_CB_WALK2, calibName, calibDesc,
                                         firstRun, lastRun, 0);
    TCMySQLManager::GetManager()->AddSet(kCALIB_CB_WALK3, calibName, calibDesc,
                                         firstRun, lastRun, 0);
     
    // init CB quadratic energy correction
    TCMySQLManager::GetManager()->AddSet(kCALIB_CB_EQUAD0, calibName, calibDesc,
                                         firstRun, lastRun, 0);
    TCMySQLManager::GetManager()->AddSet(kCALIB_CB_EQUAD1, calibName, calibDesc,
                                         firstRun, lastRun, 0);
 
    // read AcquRoot calibration of TAPS
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAPS, calibFileTAPS,
                                             calibName, calibDesc,
                                             firstRun, lastRun);
    
    // init TAPS quadratic energy correction
    TCMySQLManager::GetManager()->AddSet(kCALIB_TAPS_EQUAD0, calibName, calibDesc,
                                         firstRun, lastRun, 0);
    TCMySQLManager::GetManager()->AddSet(kCALIB_TAPS_EQUAD1, calibName, calibDesc,
                                         firstRun, lastRun, 0);
 
    // init TAPS LED calibration
    TCMySQLManager::GetManager()->AddSet(kCALIB_TAPS_LED1, calibName, calibDesc,
                                         firstRun, lastRun, 0);
    TCMySQLManager::GetManager()->AddSet(kCALIB_TAPS_LED2, calibName, calibDesc,
                                         firstRun, lastRun, 0);
 
    // read AcquRoot calibration of PID
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_PID, calibFilePID,
                                             calibName, calibDesc,
                                             firstRun, lastRun);
    
    // init PID droop correction
    TCMySQLManager::GetManager()->AddSet(kCALIB_PID_DROOP0, calibName, calibDesc,
                                         firstRun, lastRun, 0);
    TCMySQLManager::GetManager()->AddSet(kCALIB_PID_DROOP1, calibName, calibDesc,
                                         firstRun, lastRun, 0);
    TCMySQLManager::GetManager()->AddSet(kCALIB_PID_DROOP2, calibName, calibDesc,
                                         firstRun, lastRun, 0);
    TCMySQLManager::GetManager()->AddSet(kCALIB_PID_DROOP3, calibName, calibDesc,
                                         firstRun, lastRun, 0);
     
    // read AcquRoot calibration of VETO
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_VETO, calibFileVETO,
                                             calibName, calibDesc,
                                             firstRun, lastRun);
     
    gSystem->Exit(0);
}

