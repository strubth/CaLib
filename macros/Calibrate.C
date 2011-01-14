// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Calibrate                                                            //
//                                                                      //
// Non-GUI calibrations using CaLib.                                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


void Calibrate()
{
    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // get the calibration module
    TCCalibVetoEnergy c;
    c.Start(0);
    c.ProcessAll();
}

