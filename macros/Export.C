// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Export.C                                                             //
//                                                                      //
// Export CaLib run data and calibrations to ROOT files.                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void Export()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // export CaLib data
    TCMySQLManager::GetManager()->Export("backup_Dec_07.root", 0, 0, "LD2_Dec_07");
    TCMySQLManager::GetManager()->Export("backup_Feb_09.root", 0, 0, "LD2_Feb_09");
    TCMySQLManager::GetManager()->Export("backup_May_09.root", 0, 0, "LD2_May_09");
    
    gSystem->Exit(0);
}

