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
        Error("TCMySQLManager", "Cannot connect to the database '%s' on '%s@%s'!",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
        return;
    }
    else if (fDB->IsZombie())
    {
        Error("TCMySQLManager", "Cannot connect to the database '%s' on '%s@%s'!",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
        return;
    }
    else
    {
        Info("TCMySQLManager", "Connected to the database '%s' on '%s@%s'!",
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
        Error("SendQuery", "No connection to the database!");
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
        Error("SearchRunEntry", "Could not find the information '%s' for run %d!",
                                 name, run);
        return kFALSE;
    }
    if (!res->GetRowCount())
    {
        Error("SearchRunEntry", "Could not find the information '%s' for run %d!",
                                 name, run);
        return kFALSE;
    }

    // get row
    TSQLRow* row = res->Next();
  
    // write the information
    const Char_t* field = row->GetField(0);
    if (!field) strcpy(outInfo, "");
    else strcpy(outInfo, field);

    // clean-up
    delete row;
    delete res;
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::SearchSetEntry(CalibData_t data, const Char_t* calibration, Int_t set,
                                      const Char_t* name, Char_t* outInfo)
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
        Error("SearchSetEntry", "No runset %d found in table '%s'!", set, table);
        return kFALSE;
    }
    if (!res->GetRowCount())
    {
        Error("SearchSetEntry", "No runset %d found in table '%s'!", set, table);
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
Bool_t TCMySQLManager::ChangeRunEntry(Int_t run, const Char_t* name, const Char_t* value)
{
    // Change the run entry 'name' for the run 'run' to 'value'.
    
    Char_t query[256];

    // create the query
    sprintf(query,
            "UPDATE %s SET %s = '%s' "
            "WHERE run = %d",
            TCConfig::kCalibMainTableName, name, value, run);

    // read from database
    TSQLResult* res = SendQuery(query);
    
    // check result
    if (!res)
    {
        Error("ChangeRunEntry", "Could not set the information '%s' for run %d to '%s'!",
                                name, run, value);
        return kFALSE;
    }

    // clean-up
    delete res;
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ChangeSetEntry(CalibData_t data, const Char_t* calibration, Int_t set,
                                      const Char_t* name, const Char_t* value)
{
    // Change the information 'name' of the 'set'-th set of the calibration 'calibration'
    // for the calibration data 'data' to 'value'.
    
    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("ChangeSetEntry", "No data table found!");
        return 0;
    }
    
    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);

    // create the query
    sprintf(query,
            "UPDATE %s SET %s = '%s' "
            "WHERE calibration = '%s' AND "
            "first_run = %d",
            table, name, value, calibration, first_run);

    // read from database
    TSQLResult* res = SendQuery(query);
    
    // check result
    if (!res)
    {
        Error("ChangeSetEntry", "Could not set the information '%s' for set %d of '%s' to '%s'!",
                                name, set, TCConfig::kCalibDataNames[(Int_t)data], value);
        return kFALSE;
    }

    // clean-up
    delete res;
 
    return kTRUE;
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetNsets(CalibData_t data, const Char_t* calibration)
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
        Error("GetNsets", "No runsets found in table '%s'!", table);
        return 0;
    }

    // count rows
    Int_t rows = res->GetRowCount();
    delete res;

    return rows;
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetFirstRunOfSet(CalibData_t data, const Char_t* calibration, Int_t set)
{
    // Get the first run of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(data, calibration, set, "first_run", tmp)) return atoi(tmp);
    else 
    {
        Error("GetFirstRunOfSet", "Could not find first run of set!");
        return 0;
    }
}

//______________________________________________________________________________
Int_t TCMySQLManager::GetLastRunOfSet(CalibData_t data, const Char_t* calibration, Int_t set)
{
    // Get the last run of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(data, calibration, set, "last_run", tmp)) return atoi(tmp);
    else 
    {
        Error("GetLastRunOfSet", "Could not find last run of set!");
        return 0;
    }
}

//______________________________________________________________________________
void TCMySQLManager::GetDescriptionOfSet(CalibData_t data, const Char_t* calibration, 
                                         Int_t set, Char_t* outDesc)
{
    // Get the description of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(data, calibration, set, "description", tmp)) strcpy(outDesc, tmp);
    else Error("GetDescriptionOfSet", "Could not find description of set!");
}

//______________________________________________________________________________
void TCMySQLManager::GetFillTimeOfSet(CalibData_t data, const Char_t* calibration, 
                                      Int_t set, Char_t* outTime)
{
    // Get the fill time of the runsets 'set' for the calibration identifier
    // 'calibration' and the calibration data 'data'.

    Char_t tmp[256];

    // get the data
    if (SearchSetEntry(data, calibration, set, "filled", tmp)) strcpy(outTime, tmp);
    else Error("GetFillTimeOfSet", "Could not find fill time of set!");
}

//______________________________________________________________________________
Int_t* TCMySQLManager::GetRunsOfSet(CalibData_t data, const Char_t* calibration, 
                                    Int_t set, Int_t* outNruns = 0)
{
    // Return the list of runs that are in the set 'set' of the calibration data 'data'
    // for the calibration identifier 'calibration'.
    // If 'outNruns' is not zero the number of runs will be written to this variable.
    // ATTENTION: the run array has to be destroyed by the caller!

    Char_t query[256];

    // get first and last run
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);
    Int_t last_run = GetLastRunOfSet(data, calibration, set);

    // check first run
    if (!first_run)
    {
        Error("GetRunsOfSet", "Could not find runs of set %d!", set);
        return 0;
    }

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
Int_t TCMySQLManager::GetSetForRun(CalibData_t data, const Char_t* calibration, Int_t run)
{
    // Return the number of the set of the calibration data 'data' for the calibration
    // identifier 'calibration' the run 'run' belongs to.
    // Return -1 if there is no such set.

    // get number of sets
    Int_t nSet = GetNsets(data, calibration);
    if (!nSet) return -1;
    
    // check if  run exists
    Char_t tmp[256];
    if (!SearchRunEntry(run, "run", tmp))
    {
        Error("GetSetForRun", "Run has no valid run number!");
        return -1;
    }
 
    // loop over sets
    for (Int_t i = 0; i < nSet; i++)
    {
        // get first and last run
        Int_t first_run = GetFirstRunOfSet(data, calibration, i);
        Int_t last_run = GetLastRunOfSet(data, calibration, i);

        // check if run is in this set
        if (run >= first_run && run <= last_run) return i;
    }

    // no set found here
    return -1;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ReadParametersRun(CalibData_t data, const Char_t* calibration, Int_t run, 
                                         Double_t* par, Int_t length)
{
    // Read 'length' parameters of the calibration data 'data' for the calibration identifier
    // 'calibration' valid for the run 'run' from the database to the value array 'par'.
    // Return kFALSE if an error occured, otherwise kTRUE.

    // get set
    Int_t set = GetSetForRun(data, calibration, run);

    // check set
    if (set == -1)
    {
        Error("ReadParametersRun", "No set of '%s' found for run %d",
              TCConfig::kCalibDataNames[(Int_t)data], run);
        return kFALSE;
    }

    // call main parameter reading method
    return ReadParameters(data, calibration, set, par, length);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::ReadParameters(CalibData_t data, const Char_t* calibration, Int_t set, 
                                      Double_t* par, Int_t length)
{
    // Read 'length' parameters of the 'set'-th set of the calibration data 'data'
    // for the calibration identifier 'calibration' from the database to the value array 'par'.
    // Return kFALSE if an error occured, otherwise kTRUE.

    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("ReadParameters", "No data table found!");
        return kFALSE;
    }

    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);

    // check first run
    if (!first_run)
    {
        Error("ReadParameters", "No calibration found for set %d of '%s'!", 
              set, TCConfig::kCalibDataNames[(Int_t)data]);
        return kFALSE;
    }

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
        Error("ReadParameters", "No calibration found for set %d of '%s'!", 
              set, TCConfig::kCalibDataNames[(Int_t)data]);
        return kFALSE;
    }
    else if (!res->GetRowCount())
    {
        Error("ReadParameters", "No calibration found for set %d of '%s'!", 
              set, TCConfig::kCalibDataNames[(Int_t)data]);
        delete res;
        return kFALSE;
    }

    // get data (parameters start at field 5)
    TSQLRow* row = res->Next();
    for (Int_t i = 0; i < length; i++) par[i] = atof(row->GetField(i+5));

    // clean-up
    delete row;
    delete res;
    
    // user information
    Info("ReadParameters", "Read %d parameters of '%s' from the database", length, TCConfig::kCalibDataNames[(Int_t)data]);
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCMySQLManager::WriteParameters(CalibData_t data, const Char_t* calibration, Int_t set, 
                                       Double_t* par, Int_t length)
{
    // Write 'length' parameters of the 'set'-th set of the calibration data 'data'
    // for the calibration identifier 'calibration' from the value array 'par' to the database.
    // Return kFALSE if an error occured, otherwise kTRUE.

    Char_t table[256];
 
    // get the data table
    if (!SearchTable(data, table))
    {
        Error("WriteParameters", "No data table found!");
        return kFALSE;
    }

    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);

    // check first run
    if (!first_run)
    {
        Error("WriteParameters", "Could not write parameters of '%s'!",
                                 TCConfig::kCalibDataNames[(Int_t)data]);
        return kFALSE;
    }

    // prepare the insert query
    TString query = TString::Format("UPDATE %s SET ", table);
    
    // read all parameters and write them to new query
    for (Int_t j = 0; j < length; j++)
    {
        // append parameter to query
        query.Append(TString::Format("par_%03d = %lf", j, par[j]));
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
        return kFALSE;
    }
    else
    {
        delete res;
        Info("WriteParameters", "Wrote %d parameters of '%s' to the database", 
                                length, TCConfig::kCalibDataNames[(Int_t)data]);
        return kTRUE;
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
            Warning("AddRunFiles", "Run %d of file '%s/%s' could not be added to the database!", 
                    f->GetRun(), path, f->GetFileName());
        }
        else
        {
            nRunAdded++;
            delete res;
        }
    }

    // user information
    Info("AddRunFiles", "Added %d runs to the database", nRunAdded);
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
        // no detector
        case kDETECTOR_NODET:
        {
            // do nothing
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
Bool_t TCMySQLManager::AddSet(CalibData_t data, const Char_t* calibration, const Char_t* desc,
                              Int_t first_run, Int_t last_run, Double_t* par, Int_t length)
{
    // Create a new set of the calibration data 'data' with the calibration identifier
    // 'calibration' for the runs 'first_run' to 'last_run'. Use 'desc' as a 
    // description. Read the 'length' parameters from 'par'.
    // Return kFALSE when an error occured, otherwise kTRUE.
 
    Char_t table[256];
    
    //
    // check if first and last run exist
    //
    
    // check first run
    if (!SearchRunEntry(first_run, "run", table))
    {
        Error("AddSet", "First run has no valid run number!");
        return kFALSE;
    }
    
    // check last run
    if (!SearchRunEntry(last_run, "run", table))
    {
        Error("AddSet", "Last run has no valid run number!");
        return kFALSE;
    }

    //
    // check if the run range is ok
    //

    // check if first run is smaller than last run
    if (first_run > last_run)
    {
        Error("AddSet", "First run of set has to be smaller than last run!");
        return kFALSE;
    }

    // get number of existing sets
    Int_t nSet = GetNsets(data, calibration);

    // loop over sets
    for (Int_t i = 0; i < nSet; i++)
    {
        // get first and last runs of set
        Int_t setLow = GetFirstRunOfSet(data, calibration, i);
        Int_t setHigh = GetLastRunOfSet(data, calibration, i);

        // check if first run is member of this set
        if (first_run >= setLow && first_run <= setHigh)
        {
            Error("AddSet", "First run is already member of set %d", i);
            return kFALSE;
        }

        // check if last run is member of this set
        if (last_run >= setLow && last_run <= setHigh)
        {
            Error("AddSet", "Last run is already member of set %d", i);
            return kFALSE;
        }

        // check if sets are not overlapping
        if ((setLow >= first_run && setLow <= last_run) ||
            (setHigh >= first_run && setHigh <= last_run))
        {
            Error("AddSet", "Run overlap with set %d", i);
            return kFALSE;
        }
    }

    //
    // create the set
    //
    
    // get the data table
    if (!SearchTable(data, table))
    {
        Error("AddSet", "No data table found!");
        return kFALSE;
    }

    // prepare the insert query
    TString ins_query = TString::Format("INSERT INTO %s SET calibration = '%s', description = '%s', first_run = %d, last_run = %d,",
                                        table, calibration, desc, first_run, last_run);
    
    // read all parameters and write them to new query
    for (Int_t j = 0; j < length; j++)
    {
        // append parameter to query
        ins_query.Append(TString::Format("par_%03d = %lf", j, par[j]));
        if (j != length - 1) ins_query.Append(",");
    }

    // write data to database
    TSQLResult* res = SendQuery(ins_query.Data());
    
    // check result
    if (!res)
    {
        Error("AddSet", "Could not add the set of '%s' for runs %d to %d!", 
                        TCConfig::kCalibDataNames[(Int_t)data], first_run, last_run);
        return kFALSE;
    }
    else
    {
        delete res;
        Info("AddSet", "Added set of '%s' for runs %d to %d", 
             TCConfig::kCalibDataNames[(Int_t)data], first_run, last_run);
        return kTRUE;
    }
}

//______________________________________________________________________________
Bool_t TCMySQLManager::AddSet(CalibData_t data, const Char_t* calibration, const Char_t* desc,
                              Int_t first_run, Int_t last_run, Double_t par)
{
    // Create a new set of the calibration data 'data' with the calibration identifier
    // 'calibration' for the runs 'first_run' to 'last_run'. Use 'desc' as a 
    // description. Set all parameters to the value 'par'.
    // Return kFALSE when an error occured, otherwise kTRUE.
    
    // get maximum number of parameters
    Int_t length = TCConfig::kCalibDataTableLengths[(Int_t)data];
    
    // create and fill parameter array
    Double_t par_array[length];
    for (Int_t i = 0; i < length; i++) par_array[i] = par;

    // set parameters
    return AddSet(data, calibration, desc, first_run, last_run, par_array, length);
}

//______________________________________________________________________________
Bool_t TCMySQLManager::RemoveSet(CalibData_t data, const Char_t* calibration, Int_t set)
{
    // Remove the set 'set' from the calibration 'calibration' of the calibration data
    // 'data'.
    
    Char_t query[256];
    Char_t table[256];

    // get the data table
    if (!SearchTable(data, table))
    {
        Error("RemoveSet", "No data table found!");
        return kFALSE;
    }

    // get the first run of the set
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);
    
    // check first run
    if (!first_run)
    {
        Error("RemoveSet", "Could not delete set %d in '%s' of calibration '%s'!",
                           set, TCConfig::kCalibDataNames[(Int_t)data], calibration);
        return kFALSE;
    }

    // create the query
    sprintf(query,
            "DELETE FROM %s WHERE "
            "calibration = '%s' AND "
            "first_run = %d",
            table, calibration, first_run);

    // read from database
    TSQLResult* res = SendQuery(query);

    // check result
    if (!res)
    {
        Error("RemoveSet", "Could not delete set %d in '%s' of calibration '%s'!",
                           set, TCConfig::kCalibDataNames[(Int_t)data], calibration);
        return kFALSE;
    }
    else
    {
        Info("RemoveSet", "Deleted set %d in '%s' of calibration '%s'",
                          set, TCConfig::kCalibDataNames[(Int_t)data], calibration);
        return kTRUE;
    }
}

//______________________________________________________________________________
Bool_t TCMySQLManager::SplitSet(CalibData_t data, const Char_t* calibration, Int_t set,
                                Int_t lastRunFirstSet)
{
    // Split the set 'set' of the calibration data 'data' and the calibration identifier
    // 'calibration' into two sets. 'lastRunFirstSet' will be the last run of the first 
    // set. The first run of the second set will be the next found run in the database.
    
    Char_t query[256];
    Char_t tmp[256];
 
    // check if splitting run exists
    if (!SearchRunEntry(lastRunFirstSet, "run", tmp))
    {
        Error("SplitSet", "Splitting run has no valid run number!");
        return kFALSE;
    }
 
    // check if splitting run is in set
    Int_t first_run = GetFirstRunOfSet(data, calibration, set);
    Int_t last_run = GetLastRunOfSet(data, calibration, set);
    if (lastRunFirstSet < first_run || lastRunFirstSet > last_run)
    {
        Error("SplitSet", "Splitting run has to be in set!");
        return kFALSE;
    }

    //
    // get the first and last run of the second set
    //
    
    // create the query
    sprintf(query,
            "SELECT run FROM %s "
            "WHERE run > %d "
            "ORDER by run LIMIT 1",
            TCConfig::kCalibMainTableName, lastRunFirstSet);

    // read from database
    TSQLResult* res = SendQuery(query);
    if (!res)
    {
        Error("SplitSet", "Cannot find first run of second set!");
        return kFALSE;
    }
    if (!res->GetRowCount())
    {
        Error("SplitSet", "Cannot find first run of second set!");
        return kFALSE;
    }
    
    // get row
    TSQLRow* row = res->Next();
  
    // get the first run of the second set
    Int_t firstRunSecondSet = 0;
    const Char_t* field = row->GetField(0);
    if (!field) 
    {
        Error("SplitSet", "Cannot find first run of second set!");
        return kFALSE;
    }
    else firstRunSecondSet = atoi(field);
    
    // clean-up
    delete row;
    delete res;
    
    // get the last run of the second set
    Int_t lastRunSecondSet = last_run;

    // 
    // backup values
    //

    // backup the description
    Char_t desc[256];
    if (!SearchSetEntry(data, calibration, set, "description", desc))
    {
        Error("SplitSet", "Cannot read description of set %d in '%s' of calibration '%s'",
                          set, TCConfig::kCalibDataNames[(Int_t)data], calibration);
        return kFALSE;
    }

    // get number of parameters
    Int_t nPar = TCConfig::kCalibDataTableLengths[(Int_t)data];

    // backup parameters
    Double_t par[nPar];
    if (!ReadParameters(data, calibration, set, par, nPar))
    {
        Error("SplitSet", "Cannot backup parameters of set %d in '%s' of calibration '%s'",
                          set, TCConfig::kCalibDataNames[(Int_t)data], calibration);
    }

    // 
    // modify first set
    //

    // change the last run of the first set
    sprintf(tmp, "%d", lastRunFirstSet);
    if (!ChangeSetEntry(data, calibration, set, "last_run", tmp))
    {
        Error("SplitSet", "Cannot change last run of set %d in '%s' of calibration '%s'",
                          set, TCConfig::kCalibDataNames[(Int_t)data], calibration);
        return kFALSE;
    }
    else
    {
        Info("SplitSet", "Changed last run of set %d in '%s' of calibration '%s' to %d",
                         set, TCConfig::kCalibDataNames[(Int_t)data], calibration, lastRunFirstSet);
    }
    
    //
    // add second set
    //
    
    if (!AddSet(data, calibration, desc, firstRunSecondSet, lastRunSecondSet, par, nPar))
    {
        Error("SplitSet", "Cannot split set %d in '%s' of calibration '%s'",
                          set, TCConfig::kCalibDataNames[(Int_t)data], calibration);
        return kFALSE;
    }
    
    return kTRUE;
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
        Int_t nSet = GetNsets((CalibData_t)i, calibration);

        // loop over sets
        for (Int_t j = 0; j < nSet; j++)
        {
            // read parameters
            ReadParameters((CalibData_t)i, calibration, j, par, nPar);
            
            // add the calibration
            TCCalibration* c = container->AddCalibration(calibration);
            
            // set calibration data
            c->SetCalibData((CalibData_t)i);

            // set description
            GetDescriptionOfSet((CalibData_t)i, calibration, j, tmp);
            c->SetDescription(tmp);

            // set first and last run
            c->SetFirstRun(GetFirstRunOfSet((CalibData_t)i, calibration, j));
            c->SetLastRun(GetLastRunOfSet((CalibData_t)i, calibration, j));
            
            // set fill time
            GetFillTimeOfSet((CalibData_t)i, calibration, j, tmp);
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
void TCMySQLManager::ImportRuns(TCContainer* container)
{
    // Import all runs from the CaLib container 'container' to the database.
    
    // get number of runs
    Int_t nRun = container->GetNRuns();

    // loop over runs
    Int_t nRunAdded = 0;
    for (Int_t i = 0; i < nRun; i++)
    {
        // get the run
        TCRun* r = container->GetRun(i);
        
        // prepare the insert query
        TString ins_query = TString::Format("INSERT INTO %s SET "
                                            "run = %d, "
                                            "path = '%s', "
                                            "filename = '%s', "
                                            "time = '%s', "
                                            "description = '%s', "
                                            "run_note = '%s', "
                                            "size = %lld, "
                                            "target = '%s', "
                                            "target_pol = '%s', "
                                            "target_pol_deg = %lf, "
                                            "beam_pol = '%s', "
                                            "beam_pol_deg = %lf",
                                            TCConfig::kCalibMainTableName, 
                                            r->GetRun(),
                                            r->GetPath(),
                                            r->GetFileName(),
                                            r->GetTime(),
                                            r->GetDescription(),
                                            r->GetRunNote(),
                                            r->GetSize(),
                                            r->GetTarget(),
                                            r->GetTargetPol(),
                                            r->GetTargetPolDeg(),
                                            r->GetBeamPol(),
                                            r->GetBeamPolDeg());

        // try to write data to database
        TSQLResult* res = SendQuery(ins_query.Data());
        if (res == 0)
        {
            Warning("ImportRuns", "Run %d could not be added to the database!", 
                    r->GetRun());
        }
        else
        {
            Info("ImportRuns", "Added run %d to the database", r->GetRun());
            nRunAdded++;
            delete res;
        }
    }

    // user information
    Info("ImportRuns", "Added %d runs to the database", nRunAdded);
}

//______________________________________________________________________________
void TCMySQLManager::ImportCalibrations(TCContainer* container, const Char_t* newCalibName)
{
    // Import all calibrations from the CaLib container 'container' to the database.
    // If 'newCalibName' is non-zero rename the calibration to 'newCalibName'
    
    // get number of calibrations
    Int_t nCalib = container->GetNCalibrations();

    // loop over calibrations
    Int_t nCalibAdded = 0;
    for (Int_t i = 0; i < nCalib; i++)
    {
        // get the calibration
        TCCalibration* c = container->GetCalibration(i);
        
        // add the set with new calibration identifer or the same
        const Char_t* calibration;
        if (newCalibName) calibration = newCalibName;
        else calibration = c->GetCalibration();

        // add the set
        if (AddSet(c->GetCalibData(), calibration, c->GetDescription(), 
                   c->GetFirstRun(), c->GetLastRun(), c->GetParameters(), c->GetNParameters()))
        {
            Info("ImportCalibrations", "Added calibration '%s' of '%s' to the database",
                 calibration, TCConfig::kCalibDataNames[(Int_t)c->GetCalibData()]);
            nCalibAdded++;
        }
        else
        {
            Error("ImportCalibrations", "Calibration '%s' of '%s' could not be added to the database!",
                  calibration, TCConfig::kCalibDataNames[(Int_t)c->GetCalibData()]);
        }
    }

    // user information
    Info("ImportCalibrations", "Added %d calibrations to the database", nCalibAdded);
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

//______________________________________________________________________________
void TCMySQLManager::Import(const Char_t* filename, Bool_t runs, Bool_t calibrations,
                            const Char_t* newCalibName)
{
    // Import run and/or calibration data from the ROOT file 'filename'
    //
    // If 'runs' is kTRUE all run information is imported.
    // If 'calibrations' is kTRUE all calibration information is imported.
    // If 'newCalibName' is non-zero rename the calibration to 'newCalibName'

    // try to open the ROOT file
    TFile* f = new TFile(filename);
    if (!f)
    {
        Error("Import", "Could not open the ROOT file '%s'!", filename);
        return;
    }
    if (f->IsZombie())
    {
        Error("Import", "Could not open the ROOT file '%s'!", filename);
        return;
    }

    // to load the CaLib container
    TCContainer* c = (TCContainer*) f->Get("CaLib_Dump");
    if (!c)
    {
        Error("Import", "No CaLib container found in ROOT file '%s'!", filename);
        delete f;
        return;
    }

    // import runs
    if (runs) 
    {
        // get number of runs
        Int_t nRun = c->GetNRuns();

        // check if some runs were found
        if (nRun)
        {
            // ask for user confirmation
            Char_t answer[256];
            printf("\n%d runs were found in the ROOT file '%s'\n"
                   "They will be added to the database '%s' on '%s'\n", 
                   nRun, filename, fDB->GetDB(), fDB->GetHost());
            printf("Are you sure to continue? (yes/no) : ");
            scanf("%s", answer);
            if (strcmp(answer, "yes")) 
            {
                printf("Aborted.\n");
            }
            else
            {
                // import all runs
                ImportRuns(c);
            }
        }
        else
        {
            Error("Import", "No runs were found in ROOT file '%s'!", filename);
        }
    }

    // import calibrations
    if (calibrations) 
    {
        // get number of calibrations
        Int_t nCalib = c->GetNCalibrations();

        // check if some calibrations were found
        if (nCalib)
        {
            // get name of calibrations
            const Char_t* calibName = c->GetCalibration(0)->GetCalibration();

            // ask for user confirmation
            Char_t answer[256];
            printf("\n%d calibrations named '%s' were found in the ROOT file '%s'\n"
                   "They will be added to the database '%s' on '%s'\n", 
                   nCalib, calibName, filename, fDB->GetDB(), fDB->GetHost());
            if (newCalibName) printf("The calibrations will be renamed to '%s'\n", newCalibName);
            printf("Are you sure to continue? (yes/no) : ");
            scanf("%s", answer);
            if (strcmp(answer, "yes")) 
            {
                printf("Aborted.\n");
            }
            else
            {
                // import all runs
                ImportCalibrations(c, newCalibName);
            }
        }
        else
        {
            Error("Import", "No calibrations were found in ROOT file '%s'!", filename);
        }
    }

    // clean-up
    delete f;
}

