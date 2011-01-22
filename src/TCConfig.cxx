// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCConfig                                                             //
//                                                                      //
// CaLib configuration namespace                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCConfig.h"


namespace TCConfig
{   
    // detector elements
    const Int_t kMaxCrystal   = 720;
    const Int_t kMaxCB        = 720;
    const Int_t kMaxTAPS      = 510;
    const Int_t kMaxPID       =  24;
    const Int_t kMaxVETO      = 510;
    const Int_t kMaxTAGGER    = 352;

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
        "cb_e1", "cb_equad0", "cb_equad1",

        // TAPS data
        "taps_t0", "taps_t1", "taps_lg_e0", "taps_lg_e1",
        "taps_sg_e0", "taps_sg_e1", "taps_equad0", "taps_equad1",
        "taps_led1", "taps_led2",

        // PID data
        "pid_phi", "pid_t0", "pid_e0", "pid_e1", 
        "pid_droop0", "pid_droop1", "pid_droop2", "pid_droop3",

        // VETO data
        "veto_t0", "veto_t1", "veto_e0", "veto_e1"
    };

    // name of the main table
    const Char_t* kCalibMainTableName = "run_main";

    // format of the main table
    const Char_t* kCalibMainTableFormat = 
                    "run INT NOT NULL DEFAULT 0,"
                    "path VARCHAR(256),"
                    "filename VARCHAR(256),"
                    "time DATETIME,"
                    "description VARCHAR(256),"
                    "run_note VARCHAR(256),"
                    "size INT,"
                    "target VARCHAR(20),"
                    "target_pol VARCHAR(20),"
                    "target_pol_deg DOUBLE,"
                    "beam_pol VARCHAR(20),"
                    "beam_pol_deg DOUBLE,"
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
    
    // maximum theta bins for TAPS quadratic energy correction
    const Int_t kMaxTAPSThetaBins =  30;

    // constants
    const Double_t kPi0Mass = 134.9766;
    const Double_t kEtaMass = 547.853;
}

