/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iConfig                                                              //
//                                                                      //
// CaLib configuration namespace                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "iConfig.hh"


// Declare namespace member
iReadConfig* iConfig::fReadConfig = 0;


//______________________________________________________________________________
iReadConfig* iConfig::GetRC()
{
    // Return the global configuration reader.

    // Create reader if necessary
    if (!fReadConfig) fReadConfig = new iReadConfig();

    return fReadConfig;
}

