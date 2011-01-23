// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCMySQLManager                                                       //
//                                                                      //
// This class handles all the communication with the MySQL server.      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCMYSQLMANAGER_H
#define TCMYSQLMANAGER_H

#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TList.h"
#include "TError.h"
#include "TObjString.h"

#include "TCConfig.h"
#include "TCReadConfig.h"
#include "TCReadACQU.h"
#include "TCReadARCalib.h"
#include "TCContainer.h"


class TCMySQLManager
{

private:
    TSQLServer* fDB;

    void CreateMainTable();
    void CreateDataTable(CalibData_t data, Int_t nElem);
    
    Bool_t SearchTable(CalibData_t data, Char_t* outTableName);
    Bool_t SearchRunEntry(Int_t run, const Char_t* data, Char_t* outInfo);
    TList* SearchDistinctEntries(const Char_t* data, const Char_t* table);

    static TCMySQLManager* fgMySQLManager;

public:
    TCMySQLManager();
    virtual ~TCMySQLManager();

    TSQLResult* SendQuery(const Char_t* query);
    Bool_t IsConnected();
    
    TList* GetAllCalibrations();
    TList* GetAllTargets();
    Int_t GetNsets(const Char_t* calibration, CalibData_t data);
    Int_t GetFirstRunOfSet(const Char_t* calibration, CalibData_t data, Int_t set);
    Int_t GetLastRunOfSet(const Char_t* calibration, CalibData_t data, Int_t set);
    Int_t* GetRunsOfSet(const Char_t* calibration, CalibData_t data, 
                        Int_t set, Int_t* outNruns);
    Long64_t GetUnixTimeOfRun(Int_t run);

    void ReadParameters(const Char_t* calibration, Int_t set, CalibData_t data, 
                        Double_t* par, Int_t length);
    void WriteParameters(const Char_t* calibration, Int_t set, CalibData_t data, 
                         Double_t* par, Int_t length);
    
    void AddSet(CalibData_t data, const Char_t* calib, const Char_t* desc,
                Int_t first_run, Int_t last_run, Double_t* par, Int_t length);
    void AddSet(CalibData_t data, const Char_t* calib, const Char_t* desc,
                Int_t first_run, Int_t last_run, Double_t par);

    void InitDatabase();
    void AddRunFiles(const Char_t* path, const Char_t* target);
    void AddCalibAR(CalibDetector_t det, const Char_t* calibFileAR,
                    const Char_t* calib, const Char_t* desc,
                    Int_t first_run, Int_t last_run);
    
    void ExportRuns(const Char_t* filename, Int_t first_run, Int_t last_run);
    void ExportCalibrations(const Char_t* filename, const Char_t* calibration);

    static TCMySQLManager* GetManager()
    {
        if (!fgMySQLManager) fgMySQLManager = new TCMySQLManager();
        return fgMySQLManager;
    }
    
    ClassDef(TCMySQLManager, 0) // Communication with MySQL Server
};

#endif

