/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iFileManager                                                         //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef IFILEMANAGER_HH 
#define IFILEMANAGER_HH

#include <iostream>
#include <fstream>

#include <TObject.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TFile.h>
#include <TString.h>
#include <TError.h>
#include <TH2F.h>
#include <TH2D.h>

#include "iReadConfig.hh"
#include "iMySQLManager.hh"


using namespace std;


class iFileManager
{
private:
    Int_t  fRun;
    Int_t  fSet;
    Int_t  fNRun;
    Int_t* fRunArray;
    
    TString strRunTimeWindow;
    TString strRunEventNumMin;
    TString strRunFileNumber;
    TString strRUNFilesChain; 

    TString strCalibModulName;
    TFile* fFile[800];
    TH2F* hMain;

public:
    iFileManager();
    virtual ~iFileManager();

    void BuildFileArray();
    void BuildFileArray(Int_t, CalibData_t data);
    void BuildHistArray(TString);
    void DoForSet(Int_t, CalibData_t, TString);
    
    void CloseAllFiles();

    TH2F* GetMainHisto() { return hMain; };
    TString GetModulName() { return strCalibModulName; };
    TFile* GetFile(Int_t n) { return fFile[n]; };
    Int_t GetRun() { return fRun; };
    Int_t GetSet() { return fSet; };

    ClassDef(iFileManager, 0)  // Sum up .root files
};

#endif

