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
#pragma link C++ namespace iConfig;
#pragma link C++ namespace iUtils;
#pragma link C++ class iFileManager+;
#pragma link C++ class iReadConfig+;
#pragma link C++ class iMySQLManager+;
#pragma link C++ class iCalib+;
#pragma link C++ class iCrystalNavigator+;      // delete this
#pragma link C++ class iFitHisto+;              // delete this

// calibration classes
#pragma link C++ class iCalibCBEnergy+;
#pragma link C++ class iCalibCBTime+;

#pragma link C++ class iCalibPIDphi+;
#pragma link C++ class iCalibPIDenergy+;
#pragma link C++ class iCalibTaggerTime+;
#pragma link C++ class iCalibCBTimeWalk+;
#pragma link C++ class iCalibTAPS1gEnergy+;
#pragma link C++ class iCalibTAPS2gTime+;
#pragma link C++ class iCalibTAPSTaggerTime+;
#pragma link C++ class iCalibTAGGERvsTAPSTime+;

#endif

