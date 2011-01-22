// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCMySQLManager                                                       //
//                                                                      //
// This class handles all the communication with the MySQL server.      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCMySQLManager.h"

ClassImp(TCMySQLManager)


// init static class members
TCMySQLManager* TCMySQLManager::fgMySQLManager = 0;


//______________________________________________________________________________
TCMySQLManager::TCMySQLManager()
{
    // Constructor.

    fDB = 0;

    // get database configuration
    TString* strDBHost;
    TString* strDBName;
    TString* strDBUser;
    TString* strDBPass;

    // get database hostname
    if (!(strDBHost = TCReadConfig::GetReader()->GetConfig("DB.Host")))
    {
        Error("TCMySQLManager", "Database host not included in configuration file!");
        return;
    }

    // get database name
    if (!(strDBName = TCReadConfig::GetReader()->GetConfig("DB.Name")))
    {
        Error("TCMySQLManager", "Database name not included in configuration file!");
        return;
    }

    // get database user
    if (!(strDBUser = TCReadConfig::GetReader()->GetConfig("DB.User")))
    {
        Error("TCMySQLManager", "Database user not included in configuration file!");
        return;
    }
    
    // get database password
    if (!(strDBPass = TCReadConfig::GetReader()->GetConfig("DB.Pass")))
    {
        Error("TCMySQLManager", "Database password not included in configuration file!");
        return;
    }

    // get calibration
    if (TString* calib = TCReadConfig::GetReader()->GetConfig("DB.Calibration"))
    {
        fCalibration = *calib;
    }
    else
    {
        Error("TCMySQLManager", "Calibration name not included in configuration file!");
        return;
    }
    
    // open connection to MySQL server on localhost
    Char_t szMySQL[200];
    sprintf(szMySQL, "mysql://%s/%s", strDBHost->Data(), strDBName->Data());
    fDB = TSQLServer::Connect(szMySQL, strDBUser->Data(), strDBPass->Data());
    if (!fDB)
    {
        Error("TCMySQLManager", "Cannot connect to database %s on %s@%s!\n",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
        return;
    }
    else if (fDB->IsZombie())
    {
        Error("TCMySQLManager", "Cannot connect to database %s on %s@%s!\n",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
        return;
    }
    else
    {
        Info("TCMySQLManager", "Connected to database %s on %s@%s!\n",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
    }
}

//______________________________________________________________________________
TCMySQLManager::~TCMySQLManager()
{
    // Destructor.

    // close DB
    if (fDB) delete fDB;
}

//______________________________________________________________________________
TSQLResult* TCMySQLManager::SendQuery(const Char_t* query)
{
    // Send a query to the database and return the result.

    // check server connection
    if (!IsConnected())
    {
        Error("SendQuery", "No connection to database!");
        return 0;
    }

    // execute query
    return fDB->Query(query);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::IsConnected()
{
    // Check if the connection to the database is open.

    if (!fDB)
    {
        Error("IsConnected", "Cannot access database!");
        return kFALSE;
    }
    else
        return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::SearchTable(CalibData_t data, Char_t* outTableName)
{
    // Search the table name of the calibration quantity 'data' and write it
    // to 'outTableName'.
    // Return kTRUE when a true table was found, otherwise kFALSE.
    
    // copy table name
    strcpy(outTableName, TCConfig::kCalibDataTableNames[(Int_t)data]);
    
    // check for empty table
    if (data == kCALIB_NODATA) return kFALSE;
    else return kTRUE;
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetNsets(CalibData_t data)
{
    // Get the number of runsets for the calibration data 'data'.

    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("GetNsets", "No data table found!");
        return 0;
    }

    // create the query
    sprintf(query,
            "SELECT DISTINCT first_run FROM %s WHERE "
            "calibration = '%s' "
            "ORDER BY first_run ASC",
            table, fCalibration.Data());

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("GetNsets", "No runsets found in table %s!\n", table);
        return 0;
    }

    // count rows
    Int_t rows = res->GetRowCount();
    delete res;

    return rows;
}

//______________________________________________________________________________
Long64_t TCMySQLManager::GetUnixTimeOfRun(Int_t run)
{
    // Return the Unix time of the run 'run'.

    Char_t query[256];

    // create the query
    sprintf(query,
            "SELECT UNIX_TIMESTAMP(date) FROM %s WHERE run = %d", TCConfig::kCalibMainTableName, run);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("GetUnixTimeOfRun", "Run %d was not found in main table!\n", run);
        return 0;
    }

    // get unix time
    TSQLRow* row = res->Next();
    Long64_t unix_time = atol(row->GetField(0));

    // clean-up
    delete row;
    delete res;

    return unix_time;
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetFirstRunOfSet(CalibData_t data, Int_t set)
{
    // Get the first run of the runsets 'set' for the calibration data 'data'.

    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("GetFirstRunOfSet", "No data table found!");
        return 0;
    }

    // create the query
    sprintf(query,
            "SELECT DISTINCT first_run FROM %s WHERE "
            "calibration = '%s' "
            "ORDER BY first_run ASC LIMIT 1 OFFSET %d",
            table, fCalibration.Data(), set);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("GetFirstRunOfSet", "No runset %d found in table %s!\n", set, table);
        return 0;
    }

    // get first run
    TSQLRow* row = res->Next();
    Int_t first_run = atoi(row->GetField(0));

    // clean-up
    delete row;
    delete res;

    return first_run;
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetLastRunOfSet(CalibData_t data, Int_t set)
{
    // Get the last run of the runsets 'set' for the calibration data 'data'.

    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("GetLastRunOfSet", "No data table found!");
        return 0;
    }

    // create the query
    sprintf(query,
            "SELECT DISTINCT last_run FROM %s WHERE "
            "calibration = '%s' "
            "ORDER BY first_run ASC LIMIT 1 OFFSET %d",
            table, fCalibration.Data(), set);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("GetLastRunOfSet", "No runset %d found in table %s!\n", set, table);
        return 0;
    }

    // get first run
    TSQLRow* row = res->Next();
    Int_t last_run = atoi(row->GetField(0));

    // clean-up
    delete row;
    delete res;

    return last_run;
}

//______________________________________________________________________________
Int_t* TCMySQLManager::GetRunsOfSet(CalibData_t data, Int_t set, Int_t* outNruns = 0)
{
    // Return the list of runs that are in the set 'set' of the calibration data 'data'.
    // If 'outNruns' is not zero the number of runs will be written to this variable.
    // ATTENTION: the run array has to be destroyed by the caller!

    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("GetRunsOfSet", "No data table found!");
        return 0;
    }

    //
    // get the first and the last run of the set
    //

    // create the query
    sprintf(query,
            "SELECT DISTINCT first_run,last_run FROM %s WHERE "
            "calibration = '%s' "
            "ORDER BY first_run ASC LIMIT 1 OFFSET %d",
            table, fCalibration.Data(), set);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("GetRunsOfSet", "No runset %d found in table %s!\n", set, table);
        return 0;
    }

    // get first and last run
    TSQLRow* row = res->Next();
    Int_t first_run = atoi(row->GetField(0));
    Int_t last_run = atoi(row->GetField(1));

    // clean-up
    delete row;
    delete res;

    //
    // get all the runs that lie between first and last run
    //

    // create the query
    sprintf(query,
            "SELECT run FROM %s "
            "WHERE date >= ( SELECT date FROM %s WHERE run = %d) "
            "AND date <= ( SELECT date FROM %s WHERE run = %d) "
            "ORDER by run,date",
            TCConfig::kCalibMainTableName, TCConfig::kCalibMainTableName, first_run, 
            TCConfig::kCalibMainTableName, last_run);

    // read from database
    res = SendQuery(query);

    // get number of runs
    Int_t nruns = res->GetRowCount();

    // create run array
    Int_t* runs = new Int_t[nruns];

    // read all runs
    for (Int_t i = 0; i < nruns; i++)
    {
        // get next run
        row = res->Next();

        // save run number
        runs[i] = atoi(row->GetField(0));

        // clean-up
        delete row;
    }

    // clean-up
    delete res;

    // write number of runs
    if (outNruns) *outNruns = nruns;

    return runs;
}

//______________________________________________________________________________
void TCMySQLManager::ReadParameters(Int_t set, CalibData_t data, Double_t* par, Int_t length)
{
    // Read 'length' parameters of the 'set'-th set of the calibration data 'data'
    // from the database to the value array 'par'.

    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("ReadParameters", "No data table found!");
        return;
    }

    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(data, set);

    // create the query
    sprintf(query,
            "SELECT * FROM %s WHERE "
            "calibration = '%s' AND "
            "first_run = %d "
            "ORDER BY filled DESC LIMIT 1",
            table, fCalibration.Data(), first_run);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("ReadParameters", "No calibration found for set %d in table %s!\n", set, table);
        return;
    }
    else if (!res->GetRowCount())
    {
        Error("ReadParameters", "No calibration found for set %d in table %s!\n", set, table);
        delete res;
        return;
    }

    // get data (parameters start at field 5)
    TSQLRow* row = res->Next();
    for (Int_t i = 0; i < length; i++) par[i] = atof(row->GetField(i+5));

    // clean-up
    delete row;
    delete res;
    
    // user information
    Info("ReadParameters", "%d parameters read from table '%s'", length, table);
}

//______________________________________________________________________________
void TCMySQLManager::WriteParameters(Int_t set, CalibData_t data, Double_t* par, Int_t length)
{
    // Write 'length' parameters of the 'set'-th set of the calibration data 'data'
    // from the value array 'par' to the database.

    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("WriteParameters", "No data table found!");
        return;
    }

    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(data, set);

    // create the query
    sprintf(query,
            "SELECT description,last_run FROM %s WHERE "
            "calibration = '%s' AND "
            "first_run = %d "
            "ORDER BY filled DESC LIMIT 1",
            table, fCalibration.Data(), first_run);

    // read missing information from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("WriteParameters", "No calibration found for set %d in table %s!\n", set, table);
        return;
    }
    else if (!res->GetRowCount())
    {
        Error("WriteParameters", "No calibration found for set %d in table %s!\n", set, table);
        delete res;
        return;
    }

    // get data
    TSQLRow* row = res->Next();
    Char_t desc[256] = "";
    if (row->GetField(0)) strcpy(desc, row->GetField(0));
    Int_t last_run = atoi(row->GetField(1));

    // clean-up
    delete row;
    delete res;

    // add the set
    AddSet(data, fCalibration.Data(), desc, first_run, last_run, par, length);
}

//______________________________________________________________________________
void TCMySQLManager::InitDatabase()
{
    // Init a new CaLib database on a MySQL server.
    
    // ask for user confirmation
    Char_t answer[256];
    printf("\nWARNING: You are about to initialize a new CaLib database.\n"
           "         All existing tables in the database '%s' on '%s'\n"
           "         will be deleted!\n\n", fDB->GetDB(), fDB->GetHost());
    printf("Are you sure to continue? (yes/no) : ");
    scanf("%s", answer);
    if (strcmp(answer, "yes")) 
    {
        printf("Aborted.\n");
        return;
    }
    
    // create the main table
    CreateMainTable();

    // create TAGG tables
    CreateDataTable(kCALIB_TAGG_T0, TCConfig::kMaxTAGGER);

    // create CB tables
    CreateDataTable(kCALIB_CB_T0, TCConfig::kMaxCB);
    CreateDataTable(kCALIB_CB_WALK0, TCConfig::kMaxCB);
    CreateDataTable(kCALIB_CB_WALK1, TCConfig::kMaxCB);
    CreateDataTable(kCALIB_CB_WALK2, TCConfig::kMaxCB);
    CreateDataTable(kCALIB_CB_WALK3, TCConfig::kMaxCB);
    CreateDataTable(kCALIB_CB_E1, TCConfig::kMaxCB);
    CreateDataTable(kCALIB_CB_EQUAD0, TCConfig::kMaxCB);
    CreateDataTable(kCALIB_CB_EQUAD1, TCConfig::kMaxCB);

    // create TAPS tables
    CreateDataTable(kCALIB_TAPS_T0, TCConfig::kMaxTAPS);
    CreateDataTable(kCALIB_TAPS_T1, TCConfig::kMaxTAPS);
    CreateDataTable(kCALIB_TAPS_LG_E0, TCConfig::kMaxTAPS);
    CreateDataTable(kCALIB_TAPS_LG_E1, TCConfig::kMaxTAPS);
    CreateDataTable(kCALIB_TAPS_SG_E0, TCConfig::kMaxTAPS);
    CreateDataTable(kCALIB_TAPS_SG_E1, TCConfig::kMaxTAPS);
    CreateDataTable(kCALIB_TAPS_EQUAD0, TCConfig::kMaxTAPS);
    CreateDataTable(kCALIB_TAPS_EQUAD1, TCConfig::kMaxTAPS);
    CreateDataTable(kCALIB_TAPS_LED1, TCConfig::kMaxTAPS);
    CreateDataTable(kCALIB_TAPS_LED2, TCConfig::kMaxTAPS);

    // create PID tables
    CreateDataTable(kCALIB_PID_PHI, TCConfig::kMaxPID);
    CreateDataTable(kCALIB_PID_T0, TCConfig::kMaxPID);
    CreateDataTable(kCALIB_PID_E0, TCConfig::kMaxPID);
    CreateDataTable(kCALIB_PID_E1, TCConfig::kMaxPID);
    CreateDataTable(kCALIB_PID_DROOP0, TCConfig::kMaxPID);
    CreateDataTable(kCALIB_PID_DROOP1, TCConfig::kMaxPID);
    CreateDataTable(kCALIB_PID_DROOP2, TCConfig::kMaxPID);
    CreateDataTable(kCALIB_PID_DROOP3, TCConfig::kMaxPID);

    // create VETO tables
    CreateDataTable(kCALIB_VETO_T0, TCConfig::kMaxVETO); 
    CreateDataTable(kCALIB_VETO_T1, TCConfig::kMaxVETO);
    CreateDataTable(kCALIB_VETO_E0, TCConfig::kMaxVETO);
    CreateDataTable(kCALIB_VETO_E1, TCConfig::kMaxVETO);
}

//______________________________________________________________________________
void TCMySQLManager::AddRunFiles(const Char_t* path)
{
    // Look for raw ACQU files in 'path' and add all runs to the database.

    // read the raw files
    TCReadACQU r(path);
    Int_t nRun = r.GetNFiles();
    
    // ask for user confirmation
    Char_t answer[256];
    printf("\n%d runs were found in '%s'\n"
           "They will be added to the database '%s' on '%s'\n", 
           nRun, path, fDB->GetDB(), fDB->GetHost());
    printf("Are you sure to continue? (yes/no) : ");
    scanf("%s", answer);
    if (strcmp(answer, "yes")) 
    {
        printf("Aborted.\n");
        return;
    }

    // loop over runs
    for (Int_t i = 0; i < nRun; i++)
    {
        TCACQUFile* f = r.GetFile(i);
        
        // prepare the insert query
        TString ins_query = TString::Format("INSERT INTO %s SET "
                                            "run = %d, "
                                            "path = '%s', "
                                            "filename = '%s', "
                                            "time = STR_TO_DATE('%s', '%%a %%b %%d %%H:%%i:%%S %%Y'), "
                                            "description = '%s', "
                                            "run_note = '%s', "
                                            "size = %d",
                                            TCConfig::kCalibMainTableName, 
                                            f->GetRun(),
                                            path,
                                            f->GetFileName(),
                                            f->GetTime(),
                                            f->GetDescription(),
                                            f->GetRunNote(),
                                            f->GetSize());

        // write data to database
        TSQLResult* res = SendQuery(ins_query.Data());
        delete res;
    }

    // user information
    Info("AddRunFiles", "Added %d runs to database", nRun);
}

//______________________________________________________________________________
void TCMySQLManager::CreateMainTable()
{
    // Create the main table for CaLib.
    
    // user information
    Info("CreateMainTable", "Creating main CaLib table");

    // delete the old table if it exists
    TSQLResult* res = SendQuery(TString::Format("DROP TABLE IF EXISTS %s", TCConfig::kCalibMainTableName).Data());
    delete res;

    // create the table
    res = SendQuery(TString::Format("CREATE TABLE %s ( %s )", 
                                     TCConfig::kCalibMainTableName, TCConfig::kCalibMainTableFormat).Data());
    delete res;
}

//______________________________________________________________________________
void TCMySQLManager::CreateDataTable(CalibData_t data, Int_t nElem)
{
    // Create the table for the calibration data 'data' for 'nElem' elements.
    
    Char_t table[256];
    
    // get the table name
    if (!SearchTable(data, table))
    {
        Error("CreateDataTable", "No data table found!");
        return;
    }

    Info("CreateDataTable", "Adding data table %s", table);
        
    // delete the old table if it exists
    TSQLResult* res = SendQuery(TString::Format("DROP TABLE IF EXISTS %s", table));
    delete res;

    // prepare CREATE TABLE query
    TString query;
    query.Append(TString::Format("CREATE TABLE %s ( %s ", table, TCConfig::kCalibDataTableHeader));

    // loop over elements
    for (Int_t j = 0; j < nElem; j++)
    {
        query.Append(TString::Format("par_%03d DOUBLE DEFAULT 0", j));
        if (j != nElem - 1) query.Append(", ");
    }

    // finish preparing the query
    query.Append(" )");
    
    // submit the query
    res = SendQuery(query.Data());
    delete res;
}

//______________________________________________________________________________
TList* TCMySQLManager::GetAllTargets()
{
    // Return a list of TStrings containing all targets in the database.
    // If no targets were found 0 is returned.
    // NOTE: The list must be destroyed by the caller.

    Char_t query[256];

    // get all targets
    sprintf(query, "SELECT DISTINCT target from %s", TCConfig::kCalibMainTableName);
    TSQLResult* res = SendQuery(query);

    // count rows
    if (!res->GetRowCount())
    {
        delete res;
        return 0;
    }

    // get number of targets
    Int_t ntarget = res->GetRowCount();

    // create list
    TList* list = new TList();
    list->SetOwner(kTRUE);

    // read all targets and add them to the list
    for (Int_t i = 0; i < ntarget; i++)
    {
        // get next run
        TSQLRow* row = res->Next();

        // save run number
        list->Add(new TObjString(row->GetField(0)));

        // clean-up
        delete row;
    }

    // clean-up
    delete res;

    // return the list
    return list;
}

//______________________________________________________________________________
void TCMySQLManager::AddSet(CalibData_t data, const Char_t* calib, const Char_t* desc,
                            Int_t first_run, Int_t last_run, Double_t* par, Int_t length)
{
    // Write 'length' parameters of the calibration data 'data' from the value array 
    // 'par' to the database. Use 'calib' as calibration name, 'desc' as description
    // as well as 'first_run' and 'last_run'.
    
    Char_t table[256];
 
    // get the data table
    if (!SearchTable(data, table))
    {
        Error("AddSet", "No data table found!");
        return;
    }

    // prepare the insert query
    TString ins_query = TString::Format("INSERT INTO %s SET calibration = '%s', description = '%s', first_run = %d, last_run = %d,",
                                        table, calib, desc, first_run, last_run);
    
    // read all parameters and write them to new query
    for (Int_t j = 0; j < length; j++)
    {
        // append parameter to query
        ins_query.Append(TString::Format("par_%03d = %f", j, par[j]));
        if (j != length - 1) ins_query.Append(",");
    }

    // write data to database
    TSQLResult* res = SendQuery(ins_query.Data());
    delete res;

    // user information
    Info("AddSet", "%d parameters written to table '%s'", length, table);
}

