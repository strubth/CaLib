// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Import.C                                                             //
//                                                                      //
// Import CaLib run data and calibrations from ROOT files.              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void Import()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // import CaLib data
    TCMySQLManager::GetManager()->Import("backup_Dec_07_2011-2-26_17:31.root", kFALSE, kTRUE, "LD2_Dec_07_even");
  
    gSystem->Exit(0);
}

