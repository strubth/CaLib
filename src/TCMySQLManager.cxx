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

    // open connection to MySQL server on localhost
    Char_t szMySQL[200];
    sprintf(szMySQL, "mysql://%s/%s", strDBHost->Data(), strDBName->Data());
    fDB = TSQLServer::Connect(szMySQL, strDBUser->Data(), strDBPass->Data());
    if (!fDB)
    {
        Error("TCMySQLManager", "Cannot connect to database '%s' on '%s@%s'!\n",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
        return;
    }
    else if (fDB->IsZombie())
    {
        Error("TCMySQLManager", "Cannot connect to database '%s' on '%s@%s'!\n",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
        return;
    }
    else
    {
        Info("TCMySQLManager", "Connected to database '%s' on '%s@%s'!\n",
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
    if (data == kCALIB_EMPTY) return kFALSE;
    else return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::SearchRunEntry(Int_t run, const Char_t* name, Char_t* outInfo)
{
    // Search the information 'name' for the run 'run' and write it to 'outInfo'.
    // Return kTRUE when the information was found, otherwise kFALSE.
    
    Char_t query[256];

    // create the query
    sprintf(query,
            "SELECT %s FROM %s "
            "WHERE run = %d",
            name, TCConfig::kCalibMainTableName, run);

    // read from database
    TSQLResult* res = SendQuery(query);
    
    // check result
    if (!res)
    {
        Error("SearchRunEntry", "Could not find the information '%s' for run %d",
                                      name, run);
        return kFALSE;
    }

    // get row
    TSQLRow* row = res->Next();
  
    // write the information
    const Char_t* field = row->GetField(0);
    if (!field) strcpy(outInfo, "");
    else strcpy(outInfo, row->GetField(0));

    // clean-up
    delete row;
    delete res;
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::SearchSetEntry(const Char_t* calibration, const Char_t* name,
                                      CalibData_t data, Int_t set, Char_t* outInfo)
{
    // Search the information 'name' for the calibration identifier 'calibration' and 
    // the calibration data 'data' for the set number 'set' and write it to 'outInfo'.
    // Return kTRUE when the information was found, otherwise kFALSE.
  
    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("SearchSetEntry", "No data table found!");
        return kFALSE;
    }

    // create the query
    sprintf(query,
            "SELECT %s FROM %s WHERE "
            "calibration = '%s' "
            "ORDER BY first_run ASC LIMIT 1 OFFSET %d",
            name, table, calibration, set);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("SearchSetEntry", "No runset %d found in table '%s'!\n", set, table);
        return kFALSE;
    }

    // get data
    TSQLRow* row = res->Next();
    const Char_t* d = row->GetField(0);
    if (!d) strcpy(outInfo, "");
    else strcpy(outInfo, d);

    // clean-up
    delete row;
    delete res;
    
    return kTRUE;
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetNsets(const Char_t* calibration, CalibData_t data)
{
    // Get the number of runsets for the calibration identifier 'calibration'
    // and the calibration data 'data'.

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
            "SELECT first_run FROM %s WHERE "
            "calibration = '%s' "
            "ORDER BY first_run ASC",
            table, calibration);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("GetNsets", "No runsets found in table '%s'!\n", table);
        return 0;
    }

    // count rows
    Int_t rows = res->GetRowCount();
    delete res;

    return rows;
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetFirstRunOfSet(const Char_t* calibration, CalibData_t data, Int_t set)
{
    // Get the first run of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(calibration, "first_run", data, set, tmp)) return atoi(tmp);
    else 
    {
        Error("GetFirstRunOfSet", "Could not find first run of set!");
        return 0;
    }
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetLastRunOfSet(const Char_t* calibration, CalibData_t data, Int_t set)
{
    // Get the last run of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(calibration, "last_run", data, set, tmp)) return atoi(tmp);
    else 
    {
        Error("GetLastRunOfSet", "Could not find last run of set!");
        return 0;
    }
}

//______________________________________________________________________________
void TCMySQLManager::GetDescriptionOfSet(const Char_t* calibration, CalibData_t data, 
                                         Int_t set, Char_t* outDesc)
{
    // Get the description of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(calibration, "description", data, set, tmp)) strcpy(outDesc, tmp);
    else Error("GetDescriptionOfSet", "Could not find description of set!");
}

//______________________________________________________________________________
void TCMySQLManager::GetFillTimeOfSet(const Char_t* calibration, CalibData_t data, 
                                         Int_t set, Char_t* outTime)
{
    // Get the fill time of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(calibration, "filled", data, set, tmp)) strcpy(outTime, tmp);
    else Error("GetFillTimeOfSet", "Could not find fill time of set!");
}

//______________________________________________________________________________
Int_t* TCMySQLManager::GetRunsOfSet(const Char_t* calibration, CalibData_t data, 
                                    Int_t set, Int_t* outNruns = 0)
{
    // Return the list of runs that are in the set 'set' of the calibration data 'data'
    // for the calibration identifier 'calibration'.
    // If 'outNruns' is not zero the number of runs will be written to this variable.
    // ATTENTION: the run array has to be destroyed by the caller!

    Char_t query[256];

    // get first and last run
    Int_t first_run = GetFirstRunOfSet(calibration, data, set);
    Int_t last_run = GetLastRunOfSet(calibration, data, set);

    //
    // get all the runs that lie between first and last run
    //

    // create the query
    sprintf(query,
            "SELECT run FROM %s "
            "WHERE time >= ( SELECT time FROM %s WHERE run = %d) "
            "AND time <= ( SELECT time FROM %s WHERE run = %d) "
            "ORDER by run,time",
            TCConfig::kCalibMainTableName, TCConfig::kCalibMainTableName, first_run, 
            TCConfig::kCalibMainTableName, last_run);

    // read from database
    TSQLResult* res = SendQuery(query);

    // get number of runs
    Int_t nruns = res->GetRowCount();

    // create run array
    Int_t* runs = new Int_t[nruns];

    // read all runs
    for (Int_t i = 0; i < nruns; i++)
    {
        // get next run
        TSQLRow* row = res->Next();

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
void TCMySQLManager::ReadParameters(const Char_t* calibration, Int_t set, CalibData_t data, 
                                    Double_t* par, Int_t length)
{
    // Read 'length' parameters of the 'set'-th set of the calibration data 'data'
    // for the calibration identifier 'calibration' from the database to the value array 'par'.

    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("ReadParameters", "No data table found!");
        return;
    }

    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(calibration, data, set);

    // create the query
    sprintf(query,
            "SELECT * FROM %s WHERE "
            "calibration = '%s' AND "
            "first_run = %d",
            table, calibration, first_run);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("ReadParameters", "No calibration found for set %d in table '%s'!\n", set, table);
        return;
    }
    else if (!res->GetRowCount())
    {
        Error("ReadParameters", "No calibration found for set %d in table '%s'!\n", set, table);
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
    Info("ReadParameters", "Read %d parameters of '%s' from database", length, TCConfig::kCalibDataNames[(Int_t)data]);
}

//______________________________________________________________________________
void TCMySQLManager::WriteParameters(const Char_t* calibration, Int_t set, CalibData_t data, 
                                     Double_t* par, Int_t length)
{
    // Write 'length' parameters of the 'set'-th set of the calibration data 'data'
    // for the calibration identifier 'calibration' from the value array 'par' to the database.

    Char_t table[256];
 
    // get the data table
    if (!SearchTable(data, table))
    {
        Error("WriteParameters", "No data table found!");
        return;
    }

    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(calibration, data, set);

    // prepare the insert query
    TString query = TString::Format("UPDATE %s SET ", table);
    
    // read all parameters and write them to new query
    for (Int_t j = 0; j < length; j++)
    {
        // append parameter to query
        query.Append(TString::Format("par_%03d = %f", j, par[j]));
        if (j != length - 1) query.Append(",");
    }
    
    // finish query
    query.Append(TString::Format("WHERE calibration = '%s' AND first_run = %d",
                                 calibration, first_run));
 
    // write data to database
    TSQLResult* res = SendQuery(query.Data());
    
    // check result
    if (!res)
    {
        Error("WriteParameters", "Could not write parameters of '%s'!", 
                                  TCConfig::kCalibDataNames[(Int_t)data]);
    }
    else
    {
        delete res;
        Info("WriteParameters", "Wrote %d parameters of '%s' to database", 
                                length, TCConfig::kCalibDataNames[(Int_t)data]);
    }
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

    // create the data tables
    for (Int_t i = kCALIB_EMPTY+1; i < TCConfig::kCalibNData; i++)
    {
        // create the data table
        CreateDataTable((CalibData_t)i, TCConfig::kCalibDataTableLengths[i]);
    }
}

//______________________________________________________________________________
void TCMySQLManager::AddRunFiles(const Char_t* path, const Char_t* target)
{
    // Look for raw ACQU files in 'path' and add all runs to the database
    // using the target specifier 'target'.

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
    Int_t nRunAdded = 0;
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
                                            "size = %lld,"
                                            "target = '%s'",
                                            TCConfig::kCalibMainTableName, 
                                            f->GetRun(),
                                            path,
                                            f->GetFileName(),
                                            f->GetTime(),
                                            f->GetDescription(),
                                            f->GetRunNote(),
                                            f->GetSize(),
                                            target);

        // try to write data to database
        TSQLResult* res = SendQuery(ins_query.Data());
        if (res == 0)
        {
            Warning("AddRunFiles", "Run %d of file '%s/%s' could not be added!", 
                    f->GetRun(), path, f->GetFileName());
        }
        else
        {
            nRunAdded++;
            delete res;
        }
    }

    // user information
    Info("AddRunFiles", "Added %d runs to database", nRunAdded);
}

//______________________________________________________________________________
void TCMySQLManager::AddCalibAR(CalibDetector_t det, const Char_t* calibFileAR,
                                const Char_t* calib, const Char_t* desc,
                                Int_t first_run, Int_t last_run)
{
    // Read the calibration for the detector 'det' from the AcquRoot calibration
    // file 'calibFileAR' and create calibration sets for the runs 'first_run'
    // to 'last_run' using the calibration name 'calib' and the description
    // 'desc'.

    // read the calibration file
    TCReadARCalib r(calibFileAR, kFALSE);

    // get the number of elements
    Int_t nDet = r.GetNelements();

    // user information
    if (nDet) Info("AddCalibAR", "Found calibrations for %d detector elements", nDet);
    else
    {
        Error("AddCalibAR", "No detector elements found in calibration file!");
        return;
    }

    // create generic parameter arrays
    Double_t e0[nDet];
    Double_t e1[nDet];
    Double_t t0[nDet];
    Double_t t1[nDet];

    // read generic parameters
    for (Int_t i = 0; i < nDet; i++)
    {
        e0[i] = r.GetElement(i)->GetPedestal();
        e1[i] = r.GetElement(i)->GetADCGain();
        t0[i] = r.GetElement(i)->GetOffset();
        t1[i] = r.GetElement(i)->GetTDCGain();
    }

    // read detector specific calibration values
    // and write the data to the database (depends also
    // on the detector)
    switch (det)
    {
        // tagger
        case kDETECTOR_TAGG:
        {
            // write to database
            AddSet(kCALIB_TAGG_T0, calib, desc, first_run, last_run, t0, nDet);
            
            break;
        }
        // CB
        case kDETECTOR_CB:
        {
            // write to database
            AddSet(kCALIB_CB_E1, calib, desc, first_run, last_run, e1, nDet);
            AddSet(kCALIB_CB_T0, calib, desc, first_run, last_run, t0, nDet);
            
            break;
        }
        // TAPS
        case kDETECTOR_TAPS:
        {
            // read the calibration file again for the SG stuff
            TCReadARCalib rSG(calibFileAR, kFALSE, "TAPSSG:");

            // get the number of elements
            Int_t nDetSG = rSG.GetNelements();

            // user information
            if (!nDetSG) 
            {
                Error("AddCalibAR", "No TAPS SG detector elements found in calibration file!");
                return;
            }

            // create SG parameter arrays
            Double_t e0SG[nDetSG];
            Double_t e1SG[nDetSG];
                
            // read SG parameters
            for (Int_t i = 0; i < nDetSG; i++)
            {
                e0SG[i] = rSG.GetElement(i)->GetPedestal();
                e1SG[i] = rSG.GetElement(i)->GetADCGain();
            }

            // write to database
            AddSet(kCALIB_TAPS_LG_E0, calib, desc, first_run, last_run, e0, nDet);
            AddSet(kCALIB_TAPS_LG_E1, calib, desc, first_run, last_run, e1, nDet);
            AddSet(kCALIB_TAPS_SG_E0, calib, desc, first_run, last_run, e0SG, nDetSG);
            AddSet(kCALIB_TAPS_SG_E1, calib, desc, first_run, last_run, e1SG, nDetSG);
            AddSet(kCALIB_TAPS_T0, calib, desc, first_run, last_run, t0, nDet);
            AddSet(kCALIB_TAPS_T1, calib, desc, first_run, last_run, t1, nDet);
            
            break;
        }
        // PID
        case kDETECTOR_PID:
        {
            // create special parameter arrays
            Double_t phi[nDet];

            // read special parameters
            for (Int_t i = 0; i < nDet; i++)
            {
                phi[i] = r.GetElement(i)->GetZ();
            }

            // write to database
            AddSet(kCALIB_PID_PHI, calib, desc, first_run, last_run, phi, nDet);
            AddSet(kCALIB_PID_E0, calib, desc, first_run, last_run, e0, nDet);
            AddSet(kCALIB_PID_E1, calib, desc, first_run, last_run, e1, nDet);
            AddSet(kCALIB_PID_T0, calib, desc, first_run, last_run, t0, nDet);
            
            break;
        }
        // VETO
        case kDETECTOR_VETO:
        {
            // write to database
            AddSet(kCALIB_VETO_E0, calib, desc, first_run, last_run, e0, nDet);
            AddSet(kCALIB_VETO_E1, calib, desc, first_run, last_run, e1, nDet);
            AddSet(kCALIB_VETO_T0, calib, desc, first_run, last_run, t0, nDet);
            AddSet(kCALIB_VETO_T1, calib, desc, first_run, last_run, t1, nDet);
            
            break;
        }
    }
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

    Info("CreateDataTable", "Adding data table '%s' for %d elements", table, nElem);
        
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
    query.Append(TCConfig::kCalibDataTableSettings);
    query.Append(" )");
    
    // submit the query
    res = SendQuery(query.Data());
    delete res;
}

//______________________________________________________________________________
TList* TCMySQLManager::SearchDistinctEntries(const Char_t* data, const Char_t* table)
{
    // Return a list of TStrings containing all distinct entries of the type
    // 'data' in the table 'table' of the CaLib database.
    // If nothing is found 0 is returned.
    // NOTE: The list must be destroyed by the caller.
    
    Char_t query[256];

    // get all entries
    sprintf(query, "SELECT DISTINCT %s from %s", data, table);
    TSQLResult* res = SendQuery(query);

    // count rows
    if (!res->GetRowCount())
    {
        delete res;
        return 0;
    }

    // get number of entries
    Int_t n = res->GetRowCount();

    // create list
    TList* list = new TList();
    list->SetOwner(kTRUE);

    // read all entries and add them to the list
    for (Int_t i = 0; i < n; i++)
    {
        // get next run
        TSQLRow* row = res->Next();

        // save target
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
TList* TCMySQLManager::GetAllTargets()
{
    // Return a list of TStrings containing all targets in the database.
    // If no targets were found 0 is returned.
    // NOTE: The list must be destroyed by the caller.
    
    return SearchDistinctEntries("target", TCConfig::kCalibMainTableName);
}

//______________________________________________________________________________
TList* TCMySQLManager::GetAllCalibrations()
{
    // Return a list of TStrings containing all calibration identifiers in the database.
    // If no calibrations were found 0 is returned.
    // NOTE: The list must be destroyed by the caller.

    return SearchDistinctEntries("calibration", TCConfig::kCalibDataTableNames[1]);
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
    
    // check result
    if (!res)
    {
        Error("AddSet", "Could not add the set of '%s' for runs %d to %d!", 
                        TCConfig::kCalibDataNames[(Int_t)data], first_run, last_run);
    }
    else
    {
        delete res;
        Info("AddSet", "Added set of '%s' for runs %d to %d", 
             TCConfig::kCalibDataNames[(Int_t)data], first_run, last_run);
    }
}

//______________________________________________________________________________
void TCMySQLManager::AddSet(CalibData_t data, const Char_t* calib, const Char_t* desc,
                            Int_t first_run, Int_t last_run, Double_t par)
{
    // Set all parameters of the calibration data 'data' to the value 'par' 
    // in the database. Use 'calib' as calibration name, 'desc' as description
    // as well as 'first_run' and 'last_run'.
    
    // get maximum number of parameters
    Int_t length = TCConfig::kCalibDataTableLengths[(Int_t)data];
    
    // create and fill parameter array
    Double_t par_array[length];
    for (Int_t i = 0; i < length; i++) par_array[i] = par;

    // set parameters
    AddSet(data, calib, desc, first_run, last_run, par_array, length);
}

//______________________________________________________________________________
void TCMySQLManager::DumpRuns(TCContainer* container, Int_t first_run, Int_t last_run)
{
    // Dump the run information from run 'first_run' to run 'last_run' to 
    // the CaLib container 'container'.
    
    Char_t query[256];
    Char_t tmp[256];

    // create the query
    sprintf(query,
            "SELECT run FROM %s "
            "WHERE run >= %d "
            "AND run <= %d "
            "ORDER by run",
            TCConfig::kCalibMainTableName, first_run, last_run);

    // read from database
    TSQLResult* res = SendQuery(query);

    // get number of runs
    Int_t nruns = res->GetRowCount();

    // loop over runs
    for (Int_t i = 0; i < nruns; i++)
    {
        // get next run
        TSQLRow* row = res->Next();
        
        // get run number
        Int_t run_number = atoi(row->GetField(0));

        // add new run
        TCRun* run = container->AddRun(run_number);
        
        // set path
        if (SearchRunEntry(run_number, "path", tmp)) run->SetPath(tmp);
        
        // set filename
        if (SearchRunEntry(run_number, "filename", tmp)) run->SetFileName(tmp);
        
        // set time
        if (SearchRunEntry(run_number, "time", tmp)) run->SetTime(tmp);
        
        // set description
        if (SearchRunEntry(run_number, "description", tmp)) run->SetDescription(tmp);
        
        // set run_note
        if (SearchRunEntry(run_number, "run_note", tmp)) run->SetRunNote(tmp);
        
        // set size
        if (SearchRunEntry(run_number, "size", tmp)) 
        {
            Long64_t size;
            sscanf(tmp, "%lld", &size);
            run->SetSize(size);
        }

        // set target
        if (SearchRunEntry(run_number, "target", tmp)) run->SetTarget(tmp);
        
        // set target polarization
        if (SearchRunEntry(run_number, "target_pol", tmp)) run->SetTargetPol(tmp);
        
        // set target polarization degree
        if (SearchRunEntry(run_number, "target_pol_deg", tmp)) run->SetTargetPolDeg(atof(tmp));
        
        // set beam polarization
        if (SearchRunEntry(run_number, "beam_pol", tmp)) run->SetBeamPol(tmp);

        // set beam polarization degree
        if (SearchRunEntry(run_number, "beam_pol_deg", tmp)) run->SetBeamPolDeg(atof(tmp));

        // user information
        Info("DumpRuns", "Dumped run %d", run_number);

        // clean-up
        delete row;
    }
    
    // clean-up
    delete res;
}

//______________________________________________________________________________
void TCMySQLManager::DumpCalibrations(TCContainer* container, const Char_t* calibration)
{
    // Dump all calibrations with the calibration identifier 'calibration' to
    // the CaLib container 'container'.
    
    Char_t tmp[256];

    // loop over calibration data
    for (Int_t i = kCALIB_EMPTY+1; i < TCConfig::kCalibNData; i++)
    {
        // get number of parameters
        Int_t nPar = TCConfig::kCalibDataTableLengths[i];

        // create the parameter array
        Double_t par[nPar];

        // get the number of sets
        Int_t nSet = GetNsets(calibration, (CalibData_t)i);

        // loop over sets
        for (Int_t j = 0; j < nSet; j++)
        {
            // read parameters
            ReadParameters(calibration, j, (CalibData_t)i, par, nPar);
            
            // add the calibration
            TCCalibration* c = container->AddCalibration(calibration);
            
            // set calibration data
            c->SetCalibData((CalibData_t)i);

            // set description
            GetDescriptionOfSet(calibration, (CalibData_t)i, j, tmp);
            c->SetDescription(tmp);

            // set first and last run
            c->SetFirstRun(GetFirstRunOfSet(calibration, (CalibData_t)i, j));
            c->SetLastRun(GetLastRunOfSet(calibration, (CalibData_t)i, j));
            
            // set fill time
            GetFillTimeOfSet(calibration, (CalibData_t)i, j, tmp);
            c->SetFillTime(tmp);

            // set parameters
            c->SetParameters(nPar, par);
        }

        // user information
        Info("DumpCalibrations", "Dumped %d sets of '%s' of the calibration '%s'",
                                  nSet, TCConfig::kCalibDataNames[i], calibration);
    }
}

//______________________________________________________________________________
void TCMySQLManager::Export(const Char_t* filename, Int_t first_run, Int_t last_run, 
                            const Char_t* calibration)
{
    // Export run and/or calibration data to the ROOT file 'filename'
    //
    // If 'first_run' is non-zero AND 'last_run' is non-zero run information from run
    // 'first_run' to run 'last_run' is exported.
    //
    // If 'calibration' is non-zero the calibration with the identifier 'calibration'
    // is exported.

    // create new container
    TCContainer* container = new TCContainer("CaLib_Dump");
    
    // dump runs to container
    if (first_run && last_run) DumpRuns(container, first_run, last_run);

    // dump runs to container
    if (calibration) DumpCalibrations(container, calibration);

    // save container to ROOT file
    container->SaveAs(filename);
    
    // clean-up
    delete container;
}

