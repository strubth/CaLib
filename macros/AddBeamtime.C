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


//______________________________________________________________________________
void AddBeamtime()
{
    // load CaLib
    gSystem->Load("libCaLib.so");

    // macro configuration: just change here for your beamtime and leave
    // the other parts of the code unchanged
    const Char_t rawfilePath[]      = "/kernph/data/A2/H-Butanol/Nov_13";
    const Char_t target[]           = "H-Butanol";
    const Int_t firstRun            = 201;
    const Int_t lastRun             = 1357;
    const Char_t calibName[]        = "H-Butanol_Nov_13";
    const Char_t calibDesc[]        = "Standard calibration for November 2013 beamtime";
    const Char_t calibFileTagger[]  = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Nov_13/Tagger/FP.dat";
    const Char_t calibFileCB[]      = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Nov_13/CB/NaI.dat";
    const Char_t calibFileTAPS[]    = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Nov_13/TAPS/BaF2_PWO.dat";
    const Char_t calibFilePID[]     = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Nov_13/PID/PID.dat";
    const Char_t calibFileVeto[]    = "/usr/users/werthm/AcquRoot/acqu/acqu/data/Nov_13/TAPS/Veto.dat";

    // add raw files to the database
    TCMySQLManager::GetManager()->AddRunFiles(rawfilePath, target);

    // read AcquRoot calibration of tagger
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAGG, calibFileTagger,
                                             calibName, calibDesc,
                                             firstRun, lastRun);

    // init tagging efficiency table
    TCMySQLManager::GetManager()->AddSet("Type.Tagger.Eff", calibName, calibDesc,
                                         firstRun, lastRun, 0);

    // init beam polarization table
    TCMySQLManager::GetManager()->AddSet("Type.Tagger.Pol", calibName, calibDesc,
                                         firstRun, lastRun, 0);

    // read AcquRoot calibration of CB
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_CB, calibFileCB,
                                             calibName, calibDesc,
                                             firstRun, lastRun);

    // init CB time walk calibration
    TCMySQLManager::GetManager()->AddSet("Type.CB.Time.Walk", calibName, calibDesc,
                                         firstRun, lastRun, 0);

    // init CB quadratic energy correction
    TCMySQLManager::GetManager()->AddSet("Type.CB.Energy.Quad", calibName, calibDesc,
                                         firstRun, lastRun, 0);

    // init CB LED calibration
    TCMySQLManager::GetManager()->AddSet("Type.CB.LED", calibName, calibDesc,
                                         firstRun, lastRun, 0);

    // read AcquRoot calibration of TAPS
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_TAPS, calibFileTAPS,
                                             calibName, calibDesc,
                                             firstRun, lastRun);

    // init TAPS quadratic energy correction
    TCMySQLManager::GetManager()->AddSet("Type.TAPS.Energy.Quad", calibName, calibDesc,
                                         firstRun, lastRun, 0);

    // init TAPS LED calibration
    TCMySQLManager::GetManager()->AddSet("Type.TAPS.LED1", calibName, calibDesc,
                                         firstRun, lastRun, 0);
    TCMySQLManager::GetManager()->AddSet("Type.TAPS.LED2", calibName, calibDesc,
                                         firstRun, lastRun, 0);

    // read AcquRoot calibration of PID
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_PID, calibFilePID,
                                             calibName, calibDesc,
                                             firstRun, lastRun);

    // read AcquRoot calibration of Veto
    TCMySQLManager::GetManager()->AddCalibAR(kDETECTOR_VETO, calibFileVeto,
                                             calibName, calibDesc,
                                             firstRun, lastRun);

    gSystem->Exit(0);
}

