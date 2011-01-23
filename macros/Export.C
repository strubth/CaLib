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


void Export()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // export CaLib data
    TCMySQLManager::GetManager()->Export("export.root", 35105, 35110, "LD2_Domi");
    
    gSystem->Exit(0);
}

