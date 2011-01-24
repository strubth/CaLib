// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCFileManager                                                        //
//                                                                      //
// Histogram building class.                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCFILEMANAGER_H 
#define TCFILEMANAGER_H

#include "TFile.h"
#include "TH1.h"

#include "TCReadConfig.h"
#include "TCMySQLManager.h"


class TCFileManager
{

private:
    TString fInputFilePatt;                 // input file pattern
    TList* fFiles;                          // list of files
    CalibData_t fCalibData;                 // calibration data
    TString fCalibration;                   // calibration identifier
    Int_t fSet;                             // number of set
    
    void BuildFileList();

public:
    TCFileManager() : fInputFilePatt(0), fFiles(0), 
                      fCalibData(kCALIB_EMPTY), fCalibration(), fSet(0) { }
    TCFileManager(CalibData_t data, const Char_t* calibration, Int_t set);
    virtual ~TCFileManager();

    TH1* GetHistogram(const Char_t* name);

    ClassDef(TCFileManager, 0) // Histogram building class
};

#endif

