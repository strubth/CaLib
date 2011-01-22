// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// InstallCaLib                                                         //
//                                                                      //
// Install the CaLib database.                                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


void InstallCaLib()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // install CaLib
    TCMySQLManager::GetManager()->InitDatabase();
    
    gSystem->Exit(0);
}

