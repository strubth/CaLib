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
    const Int_t kMaxTargPos   =   1;
    const Int_t kMaxCrystal   = 720;
    const Int_t kMaxCB        = 720;
    const Int_t kMaxTAPS      = 510;
    const Int_t kMaxPID       =  24;
    const Int_t kMaxVeto      = 510;
    const Int_t kMaxTAGGER    = 352;
    
    // maximum theta bins for TAPS quadratic energy correction
    const Int_t kMaxTAPSThetaBins =  30;
    
    // number of data types
    const Int_t kCalibNData = 33;
   
    // data names
    // NOTE: This has to be synchronized with the enum ECalibData
    const Char_t* kCalibDataNames[] =
    {   
        // empty element
        "empty",
        
        // target position
        "Target Position",

        // tagger data
        "Tagger time offset",

        // CB data
        "CB time offset", 
        "CB time walk par0", "CB time walk par1", "CB time walk par2", "CB time walk par3",
        "CB ADC gain", "CB quadratic energy corr. par0", "CB quadratic energy corr. par1",

        // TAPS data
        "TAPS time offset", "TAPS TDC gain", 
        "TAPS LG ADC pedestal", "TAPS LG ADC gain",
        "TAPS SG ADC pedestal", "TAPS SG ADC gain", 
        "TAPS quadratic energy corr. par0", "TAPS quadratic energy corr. par1",
        "TAPS LED1 threshold", "TAPS LED2 threshold",

        // PID data
        "PID azimuthal angle", 
        "PID time offset", 
        "PID ADC pedestal", "PID ADC gain", 
        "PID droop corr. par0", "PID droop corr. par1", "PID droop corr. par2", "PID droop corr. par3",

        // Veto data
        "Veto time offset", "Veto TDC gain", 
        "Veto ADC pedestal", "Veto ADC gain"
    };

    // data table names
    // NOTE: This has to be synchronized with the enum ECalibData
    const Char_t* kCalibDataTableNames[] =
    {   
        // empty element
        "empty",
        
        // target position
        "target_pos",

        // tagger data
        "tagg_t0",

        // CB data
        "cb_t0",
        "cb_walk0", "cb_walk1", "cb_walk2", "cb_walk3",
        "cb_e1", "cb_equad0", "cb_equad1",

        // TAPS data
        "taps_t0", "taps_t1", 
        "taps_lg_e0", "taps_lg_e1",
        "taps_sg_e0", "taps_sg_e1", 
        "taps_equad0", "taps_equad1",
        "taps_led1", "taps_led2",

        // PID data
        "pid_phi", 
        "pid_t0", 
        "pid_e0", "pid_e1", 
        "pid_droop0", "pid_droop1", "pid_droop2", "pid_droop3",

        // Veto data
        "veto_t0", "veto_t1",
        "veto_e0", "veto_e1"
    };
    
    // data table parameter lengths
    // NOTE: This has to be synchronized with the enum ECalibData
    const Int_t kCalibDataTableLengths[] =
    {
        // empty element
        0,
        
        // target position
        kMaxTargPos,

        // tagger data
        kMaxTAGGER,

        // CB data
        kMaxCB, 
        kMaxCB, kMaxCB, kMaxCB, kMaxCB,
        kMaxCB, kMaxCB, kMaxCB,

        // TAPS data
        kMaxTAPS, kMaxTAPS, 
        kMaxTAPS, kMaxTAPS,
        kMaxTAPS, kMaxTAPS, 
        kMaxTAPS, kMaxTAPS,
        kMaxTAPS, kMaxTAPS,

        // PID data
        kMaxPID, 
        kMaxPID, 
        kMaxPID, kMaxPID, 
        kMaxPID, kMaxPID, kMaxPID, kMaxPID,

        // Veto data
        kMaxVeto, kMaxVeto, 
        kMaxVeto, kMaxVeto
    };
 
    // name of the main table
    const Char_t* kCalibMainTableName = "run_main";

    // format of the main table
    const Char_t* kCalibMainTableFormat = 
                    "run INT NOT NULL,"
                    "path VARCHAR(256),"
                    "filename VARCHAR(256),"
                    "time DATETIME,"
                    "description VARCHAR(256),"
                    "run_note VARCHAR(256),"
                    "size BIGINT,"
                    "target VARCHAR(20),"
                    "target_pol VARCHAR(128),"
                    "target_pol_deg DOUBLE,"
                    "beam_pol VARCHAR(128),"
                    "beam_pol_deg DOUBLE,"
                    "changed TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
                    "                  ON UPDATE CURRENT_TIMESTAMP,"
                    "PRIMARY KEY (run) ";

    // header of the data tables
    const Char_t* kCalibDataTableHeader =
                    "calibration VARCHAR(256),"
                    "description VARCHAR(1024),"
                    "first_run INT NOT NULL,"
                    "last_run INT NOT NULL,"
                    "changed TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
                    "                  ON UPDATE CURRENT_TIMESTAMP,";
    
    // additional settings for the data tables
    const Char_t* kCalibDataTableSettings = ",PRIMARY KEY (calibration, first_run) ";
    
    // version numbers
    const Int_t kContainerFormatVersion = 1;

    // constants
    const Double_t kPi0Mass = 134.9766;
    const Double_t kEtaMass = 547.853;
}

