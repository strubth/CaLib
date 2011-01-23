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
 
    TFile f("export.root");
    TCContainer* c = (TCContainer*) f.Get("CaLib_Dump");
    c->Print();
    for (Int_t i = 0; i < c->GetNRuns(); i++)
    {
        c->GetRun(i)->Print();
    }
    
    gSystem->Exit(0);
}

