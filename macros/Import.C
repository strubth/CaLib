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


void Import()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // import CaLib data
    TCMySQLManager::GetManager()->Import("dump.root", kTRUE, kTRUE);
  
    gSystem->Exit(0);
}

