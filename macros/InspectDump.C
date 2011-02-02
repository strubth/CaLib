// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// InspectDump.C                                                        //
//                                                                      //
// Inspect CaLib run data and calibrations in a ROOT file.              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void InspectDump()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // load CaLib container
    TFile f("dump.root");
    TCContainer* c = (TCContainer*) f.Get("CaLib_Dump");
    
    // show run information
    c->ShowRuns();

    // show calibration information
    c->ShowCalibrations();

    gSystem->Exit(0);
}
