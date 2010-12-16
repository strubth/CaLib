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

// calibration classes
#pragma link C++ class iCalibCBEnergy+;
#pragma link C++ class iCalibCBTime+;

#endif

