/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ExportDB.C                                                           //
//                                                                      //
// Export the complete database to an SQLite file.                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void ExportDB(const Char_t* filename)
{
    // load CaLib
    gSystem->Load("libCaLib.so");

    // export database
    if (!TCMySQLManager::GetManager()->ExportDatabase(filename))
        Error("ExportDB", "Database could not be exported!");

    gSystem->Exit(0);
}

