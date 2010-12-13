// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, 2010
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// InitCaLib.C                                                          //
//                                                                      //
// Create the CaLib tables in a MySQL database.                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TString.h"
#include "TError.h"


// MySQL configuration
const Char_t* gDBHost = "phys-jaguar";
const Char_t* gDBName = "calib";
const Char_t* gDBUser = "acqu_rw";

// CaLib database format configuration
const Char_t* gTableMainName = "run_main";
const Char_t* gTableMainFormat = 
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

const Char_t* gTableDataHeader =
                "calibration VARCHAR(256),"
                "description VARCHAR(1024),"
                "filled TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
                "                 ON UPDATE CURRENT_TIMESTAMP,"
                "first_run INT,"
                "last_run INT,";
  
// TAGG data
Int_t gNTAGG = 352;
Int_t gNTableTAGG = 1;
const Char_t* gTableTAGG[1] = { "tagg_t0" };

// CB data
Int_t gNCB = 720;
Int_t gNTableCB = 8;
const Char_t* gTableCB[8] = { "cb_e1",
                              "cb_t0",
                              "cb_walk0",
                              "cb_walk1",
                              "cb_walk2",
                              "cb_walk3",
                              "cb_equad_0",
                              "cb_equad_1" };

// TAPS data
Int_t gNTAPS = 438;
Int_t gNTableTAPS = 8;
const Char_t* gTableTAPS[8] = { "taps_lg_e0",
                                "taps_lg_e1",
                                "taps_sg_e0",
                                "taps_sg_e1",
                                "taps_t0",
                                "taps_t1",
                                "taps_equad_0",
                                "taps_equad_1" };

// PID data
Int_t gNPID = 24;
Int_t gNTablePID = 3;
const Char_t* gTablePID[3] = { "pid_e0",
                               "pid_e1",
                               "pid_t0" };

// VETO data
Int_t gNVETO = 384;
Int_t gNTableVETO = 4;
const Char_t* gTableVETO[4] = { "veto_e0",
                                "veto_e1",
                                "veto_t0",
                                "veto_t1" };

// global variables
TSQLServer* gDB;


//______________________________________________________________________________
Bool_t IsConnected()
{
    // Check if the connection to the database is there.

    return gDB ? kTRUE : kFALSE;
}

//______________________________________________________________________________
TSQLResult* SendQuery(const Char_t* query, Bool_t delRes = kTRUE)
{
    // Send a query to the database and return the result.
    // Delete the result (and return 0) if 'delRes' is kTRUE.
    
    // check server connection
    if (!IsConnected())
    {
        Error("SendQuery", "No connection to database!");
        return 0;
    }

    // execute query
    TSQLResult* res = gDB->Query(query);
    
    // return the query
    if (delRes)
    {
        delete res;
        return 0;
    }
    else return res;
}

//______________________________________________________________________________
void CreateMainTable()
{
    // Create the main table for CaLib.
    
    // delete the old table if it exists
    SendQuery(TString::Format("DROP TABLE IF EXISTS %s", gTableMainName).Data());
    
    // create the table
    SendQuery(TString::Format("CREATE TABLE %s ( %s )", 
                              gTableMainName, gTableMainFormat).Data());
}

//______________________________________________________________________________
void AddDataTables(Int_t n, const Char_t** names, Int_t nElem)
{
    // Register data 'n' tables for 'nElem' elements using the names from 'names'.
    
    // loop over data tables
    for (Int_t i = 0; i < n; i++)
    {
        printf("Adding data table %s\n", names[i]);
        
        // delete the old table if it exists
        SendQuery(TString::Format("DROP TABLE IF EXISTS %s", names[i]).Data());
        
        // prepare CREATE TABLE query
        TString query;
        query.Append(TString::Format("CREATE TABLE %s ( %s ", names[i], gTableDataHeader));

        // loop over elements
        for (Int_t j = 0; j < nElem; j++)
        {
            query.Append(TString::Format("par_%03d DOUBLE DEFAULT 0", j));
            if (j != nElem - 1) query.Append(", ");
        }

        // finish preparing the query
        query.Append(" )");
        
        // submit the query
        SendQuery(query.Data());
    }
}

//______________________________________________________________________________
void InitCaLib()
{
    // Main function.

    // ask for user confirmation
    Char_t answer[256];
    printf("\nWARNING: You are about to initialize a new CaLib database.\n"
           "         All existing tables in the database '%s' on '%s'\n"
           "         will be deleted!\n\n", gDBName, gDBHost);
    printf("Are you sure to continue? (yes/no) : ");
    scanf("%s", answer);
    if (strcmp(answer, "yes")) 
    {
        printf("Aborted.\n");
        gSystem->Exit(-1);
    }
 
    // ask for password
    Char_t passwd[256];
    printf("Password (will be echoed) for %s@%s: ", gDBUser, gDBHost);
    scanf("%s", passwd);

    // connect to the SQL server
    Char_t tmp[256];
    sprintf(tmp, "mysql://%s/%s", gDBHost, gDBName);
    gDB = TSQLServer::Connect(tmp, gDBUser, passwd);

    // check server connection
    if (!IsConnected())
    {
        Error("InitCaLib", "Could not connect to database!");
        gSystem->Exit(-1);
    }

    // create the main table
    CreateMainTable();

    // create tagger tables
    AddDataTables(gNTableTAGG, gTableTAGG, gNTAGG);

    // create CB tables
    AddDataTables(gNTableCB, gTableCB, gNCB);

    // create TAPS tables
    AddDataTables(gNTableTAPS, gTableTAPS, gNTAPS);
    
    // create PID tables
    AddDataTables(gNTablePID, gTablePID, gNPID);

    // create VETO tables
    AddDataTables(gNTableVETO, gTableVETO, gNVETO);

    // disconnect from server
    delete gDB;
    
    gSystem->Exit(0);
}

