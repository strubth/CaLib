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
    TCMySQLManager::GetManager()->Export("dump.root", 13089, 13841, "LD2_Domi");
    
    gSystem->Exit(0);
}

