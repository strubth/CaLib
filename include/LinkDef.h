// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// LinkDef.h                                                            //
//                                                                      //
// CaLib dictionary header file.                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifdef __CINT__

// turn everything off
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link off all typedef;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ enum ECalibData;
#pragma link C++ typedef CalibData_t;

// common classes
#pragma link C++ namespace TCConfig;
#pragma link C++ namespace TCUtils;
#pragma link C++ class TCFileManager+;
#pragma link C++ class TCReadConfig+;
#pragma link C++ class TCReadARCalib+;
#pragma link C++ class TCMySQLManager+;
#pragma link C++ class TCCalib+;
#pragma link C++ class TCCalibPed+;

// misc calibration classes
#pragma link C++ class TCCalibTargetPosition+;

// Tagger calibration classes
#pragma link C++ class TCCalibTaggerTime+;

// CB calibration classes
#pragma link C++ class TCCalibCBEnergy+;
#pragma link C++ class TCCalibCBQuadEnergy+;
#pragma link C++ class TCCalibCBTime+;
#pragma link C++ class TCCalibCBRiseTime+;
#pragma link C++ class TCCalibCBTimeWalk+;

// TAPS calibration classes
#pragma link C++ class TCCalibTAPSEnergyLG+;
#pragma link C++ class TCCalibTAPSEnergySG+;
#pragma link C++ class TCCalibTAPSPedLG+;
#pragma link C++ class TCCalibTAPSPedSG+;
#pragma link C++ class TCCalibTAPSPedVETO+;
#pragma link C++ class TCCalibTAPSQuadEnergy+;
#pragma link C++ class TCCalibTAPSTime+;
#pragma link C++ class TCCalibTAPSLED+;
#pragma link C++ class TCCalibTAPSLED1+;
#pragma link C++ class TCCalibTAPSLED2+;

// PID calibration classes
#pragma link C++ class TCCalibPIDPhi+;
#pragma link C++ class TCCalibPIDDroop+;
#pragma link C++ class TCCalibPIDEnergy+;

// Veto calibration classes
#pragma link C++ class TCCalibVetoCorr+;
#pragma link C++ class TCCalibVetoEnergy+;

#endif

