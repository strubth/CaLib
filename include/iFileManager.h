/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iFileManager                                                         //
//                                                                      //
// Histogram building class.                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef IFILEMANAGER_H 
#define IFILEMANAGER_H

#include "TFile.h"
#include "TH1.h"

#include "iReadConfig.h"
#include "iMySQLManager.h"


class iFileManager
{

private:
    TString fInputFilePatt;                 // input file pattern
    TList* fFiles;                          // list of files
    Int_t fSet;                             // number of set
    CalibData_t fCalibData;                 // calibration data
    
    void BuildFileList();

public:
    iFileManager() : fInputFilePatt(0), fFiles(0), 
                     fSet(0), fCalibData(kCALIB_NODATA) { }
    iFileManager(Int_t set, CalibData_t data);
    virtual ~iFileManager();

    TH1* GetHistogram(const Char_t* name);

    ClassDef(iFileManager, 0)  // Histogram building class
};

#endif

