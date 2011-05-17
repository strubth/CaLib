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
    
    // number of calibration data
    const Int_t kCalibNData = 29;
   
    // data names
    // NOTE: This has to be synchronized with the enum ECalibData
    const Char_t* kCalibDataNames[] =
    {   
        // empty element
        "empty",
        
        // tagger data
        "Tagger time offset",

        // CB data
        "CB time offset", 
        "CB time walk par0", "CB time walk par1", "CB time walk par2", "CB time walk par3",
        "CB ADC gain", 
        "CB quadratic energy corr. par0", "CB quadratic energy corr. par1",
        "CB LED threshold",

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
        
        // tagger data
        "tagg_t0",

        // CB data
        "cb_t0",
        "cb_walk0", "cb_walk1", "cb_walk2", "cb_walk3",
        "cb_e1", 
        "cb_equad0", "cb_equad1",
        "cb_led",

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
        
        // tagger data
        kMaxTAGGER,

        // CB data
        kMaxCB, 
        kMaxCB, kMaxCB, kMaxCB, kMaxCB,
        kMaxCB, 
        kMaxCB, kMaxCB,
        kMaxCB,

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

        // Veto data
        kMaxVeto, kMaxVeto, 
        kMaxVeto, kMaxVeto
    };
    
    // number of calibration types
    // NOTE: This has to be synchronized with the enum ECalibType
    const Int_t kCalibNType = 20;
    
    // type names
    // NOTE: This has to be synchronized with the enum ECalibType
    const Char_t* kCalibTypeNames[] =
    {   
        // empty element
        "empty",
        
        // tagger data
        "Tagger time",

        // CB data
        "CB time", 
        "CB time walk",
        "CB energy", 
        "CB quadratic energy correction",
        "CB LED threshold",

        // TAPS data
        "TAPS time",
        "TAPS LG pedestal", 
        "TAPS LG gain",
        "TAPS SG energy", 
        "TAPS quadratic energy correction",
        "TAPS LED1 threshold", 
        "TAPS LED2 threshold",

        // PID data
        "PID azimuthal angle", 
        "PID time", 
        "PID energy", 

        // Veto data
        "Veto time", 
        "Veto pedestal", 
        "Veto energy"
    };

    
    // number of data for the calibration types
    // NOTE: This has to be synchronized with the enum ECalibType
    const Int_t kCalibTypeNData[] =
    {   
        // empty element
        1,
        
        // tagger data
        1,

        // CB data
        1, 
        4,
        1, 
        2,
        1,

        // TAPS data
        2,
        1, 
        1,
        2, 
        2,
        1, 
        1,

        // PID data
        1, 
        1, 
        2, 

        // Veto data
        2, 
        1, 
        1
    };


    // data for the calibration types
    // NOTE: This has to be synchronized with the enum ECalibType
    const CalibData_t kCalibTypeData[][4] =
    {   
        // empty element
        { kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY },
        
        // tagger data
        { kCALIB_TAGG_T0, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY },

        // CB data
        { kCALIB_CB_T0, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY }, 
        { kCALIB_CB_WALK0, kCALIB_CB_WALK1, kCALIB_CB_WALK2, kCALIB_CB_WALK3 },
        { kCALIB_CB_E1, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY }, 
        { kCALIB_CB_EQUAD0, kCALIB_CB_EQUAD1, kCALIB_EMPTY, kCALIB_EMPTY },
        { kCALIB_CB_LED, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY },

        // TAPS data
        { kCALIB_TAPS_T0, kCALIB_TAPS_T1, kCALIB_EMPTY, kCALIB_EMPTY },
        { kCALIB_TAPS_LG_E0, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY }, 
        { kCALIB_TAPS_LG_E1, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY },
        { kCALIB_TAPS_SG_E0, kCALIB_TAPS_SG_E1, kCALIB_EMPTY, kCALIB_EMPTY }, 
        { kCALIB_TAPS_EQUAD0, kCALIB_TAPS_EQUAD1, kCALIB_EMPTY, kCALIB_EMPTY },
        { kCALIB_TAPS_LED1, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY }, 
        { kCALIB_TAPS_LED2, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY },

        // PID data
        { kCALIB_PID_PHI, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY }, 
        { kCALIB_PID_T0, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY }, 
        { kCALIB_PID_E0, kCALIB_PID_E1, kCALIB_EMPTY, kCALIB_EMPTY }, 

        // Veto data
        { kCALIB_VETO_T0, kCALIB_VETO_T1, kCALIB_EMPTY, kCALIB_EMPTY }, 
        { kCALIB_VETO_E0, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY }, 
        { kCALIB_VETO_E1, kCALIB_EMPTY, kCALIB_EMPTY, kCALIB_EMPTY }
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
                    "size BIGINT DEFAULT 0,"
                    "target VARCHAR(20),"
                    "target_pol VARCHAR(128),"
                    "target_pol_deg DOUBLE DEFAULT 0,"
                    "beam_pol VARCHAR(128),"
                    "beam_pol_deg DOUBLE DEFAULT 0,"
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
    const Char_t kCaLibVersion[] = "0.1.4";
    const Int_t kContainerFormatVersion = 2;
    extern const Char_t kCaLibDumpName[] = "CaLib_Dump";

    // constants
    const Double_t kPi0Mass = 134.9766;
    const Double_t kEtaMass = 547.853;
}

