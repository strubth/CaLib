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
    Int_t fSet;                             // number of set
    CalibData_t fCalibData;                 // calibration data
    
    void BuildFileList();

public:
    TCFileManager() : fInputFilePatt(0), fFiles(0), 
                      fSet(0), fCalibData(kCALIB_EMPTY) { }
    TCFileManager(Int_t set, CalibData_t data);
    virtual ~TCFileManager();

    TH1* GetHistogram(const Char_t* name);

    ClassDef(TCFileManager, 0) // Histogram building class
};

#endif

