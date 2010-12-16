/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iConfig                                                              //
//                                                                      //
// CaLib configuration namespace                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "iConfig.hh"


namespace iConfig
{   
    // detector elements
    const Int_t kMaxCrystal = 720;
    const Int_t kMaxCB      = 720;
    const Int_t kMaxTAPS    = 438;
    const Int_t kMaxPID     =  24;
    const Int_t kMaxVETO    = 384;
    const Int_t kMaxTAGGER  = 352;

    // data table names
    // NOTE: This has to be synchronized with the enum ECalibData
    const Char_t* kCalibDataTableNames[] =
    {   
        // empty element
        "empty",

        // tagger data
        "tagg_t0",

        // CB data
        "cb_t0", "cb_walk0", "cb_walk1", "cb_walk2", "cb_walk3",
        "cb_e1", "cb_equad0", "cb_equad1", "cb_pi0im",

        // TAPS data
        "taps_t0", "taps_t1", "taps_lg_e0", "taps_lg_e1",
        "taps_sg_e0", "taps_sg_e1", "taps_equad0", "taps_equad1",
        "taps_pi0im",

        // PID data
        "pid_t0", "pid_e0", "pid_e1", 

        // VETO data
        "veto_t0", "veto_t1", "veto_e0", "veto_e1"
    };

    // name of the main table
    const Char_t* kCalibMainTableName = "run_main";

    // format of the main table
    const Char_t* kCalibMainTableFormat = 
                    "run INT NOT NULL DEFAULT 0,"
                    "filename VARCHAR(256),"
                    "status VARCHAR(20),"
                    "target VARCHAR(20),"
                    "target_pol VARCHAR(20),"
                    "target_pol_deg DOUBLE,"
                    "beam_pol VARCHAR(20),"
                    "beam_pol_deg DOUBLE,"
                    "size INT,"
                    "event INT,"
                    "date DATETIME,"
                    "comment VARCHAR(256),"
                    "filled TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
                    "                 ON UPDATE CURRENT_TIMESTAMP,"
                    "PRIMARY KEY (`run`)";

    // header of the data tables
    const Char_t* kCalibDataTableHeader =
                    "calibration VARCHAR(256),"
                    "description VARCHAR(1024),"
                    "filled TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
                    "                 ON UPDATE CURRENT_TIMESTAMP,"
                    "first_run INT,"
                    "last_run INT,";

    // constants
    const Int_t kWalkNpar   = 4;
    const Double_t kPi0Mass = 134.9766;
}

