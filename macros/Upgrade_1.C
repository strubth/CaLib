// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Upgrade_1.C                                                          //
//                                                                      //
// Upgrade the CaLib database from version 0.1.11 to 0.2.0.             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void Upgrade_1()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // get database configuration
    TString* strDBHost;
    TString* strDBName;
    TString* strDBUser;
    TString* strDBPass;
 
    // get database hostname
    if (!(strDBHost = TCReadConfig::GetReader()->GetConfig("DB.Host")))
    {
        Error("Upgrade_1", "Database host not included in configuration file!");
        gSystem->Exit(0);
    }

    // get database name
    if (!(strDBName = TCReadConfig::GetReader()->GetConfig("DB.Name")))
    {
        Error("Upgrade_1", "Database name not included in configuration file!");
        gSystem->Exit(0);
    }

    // get database user
    if (!(strDBUser = TCReadConfig::GetReader()->GetConfig("DB.User")))
    {
        Error("Upgrade_1", "Database user not included in configuration file!");
        gSystem->Exit(0);
    }
    
    // get database password
    if (!(strDBPass = TCReadConfig::GetReader()->GetConfig("DB.Pass")))
    {
        Error("Upgrade_1", "Database password not included in configuration file!");
        gSystem->Exit(0);
    }

    // open connection to MySQL server on localhost
    Char_t szMySQL[200];
    sprintf(szMySQL, "mysql://%s/%s", strDBHost->Data(), strDBName->Data());
    TSQLServer* db = TSQLServer::Connect(szMySQL, strDBUser->Data(), strDBPass->Data());
    if (!db)
    {
        Error("TCMySQLManager", "Cannot connect to the database '%s' on '%s@%s'!",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
        gSystem->Exit(0);
    }
    else if (db->IsZombie())
    {
        Error("TCMySQLManager", "Cannot connect to the database '%s' on '%s@%s'!",
               strDBName->Data(), strDBUser->Data(), strDBHost->Data());
        gSystem->Exit(0);
    }
    else
    {
        Info("TCMySQLManager", "Connected to the database '%s' on '%s@%s'",
                            strDBName->Data(), strDBUser->Data(), strDBHost->Data());
    }
    
    // ask for user confirmation
    Char_t answer[256];
    printf("\nWARNING: You are about to update your existing CaLib database '%s' on '%s'\n"
           "         It is highly recommended to create a complete backup using\n", 
           "         e.g. mysqldump before proceeding", db->GetDB(), db->GetHost());
    printf("Are you sure to continue? (yes/no) : ");
    scanf("%s", answer);
    if (strcmp(answer, "yes")) 
    {
        printf("Aborted.\n");
        return;
        gSystem->Exit(0);
    }
 
    // create queries
    const Int_t nQuery = 2;
    const Char_t* query[nQuery] = { "ALTER TABLE run_main ADD scr_n INT DEFAULT -1 AFTER size" ,
                                    "ALTER TABLE run_main ADD scr_bad VARCHAR(512) AFTER scr_n" };
    
    // loop over queries
    Int_t queryOk = 0;
    for (Int_t i = 0; i < nQuery; i++)
    {
        TSQLResult* res = db->Query(query[i]);
    
        // check result
        if (!res)
        {
            Error("Upgrade_1", "Could not execute query %d!", i+1);
        }
        else
        {
            Info("Upgrade_1", "Executed query %d", i+1);
            delete res;
            queryOk++;
        }
    }

    // check final result
    if (queryOk == nQuery)
        Info("Upgrade_1", "Performed upgrade of database");
    else
        Error("Upgrade_1", "Could not perform upgrade of database!");

    // clean-up
    delete db;

    gSystem->Exit(0);
}

