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


#ifndef ICONFIG_HH
#define ICONFIG_HH

#include "iReadConfig.hh"


namespace iConfig
{   
    // detector elements
    static const Int_t kMaxCrystal = 720;
    static const Int_t kMaxCB      = 720;
    static const Int_t kMaxTAPS    = 438;
    static const Int_t kMaxPID     =  24;
    static const Int_t kMaxVETO    = 384;
    static const Int_t kMaxTAGGER  = 352;
    
    // constants
    static const Int_t kWalkNpar   = 4;
    static const Double_t kPi0Mass = 134.9766;
    
    // global configuration reader
    extern iReadConfig* fReadConfig;
    iReadConfig* GetRC();
}

#endif

