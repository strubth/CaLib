/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Calibrate.C                                                          //
//                                                                      //
// Non-GUI calibrations using CaLib.                                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void Calibrate()
{
    // load CaLib
    gSystem->Load("libCaLib.so");

    // get the calibration module
    TCCalibVetoEnergy c;
    c.Start(Domi_Calib, 0);
    c.ProcessAll();
}

