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
    TSQLServer* fDB;                            // MySQL database conneciton
    static TCMySQLManager* fgMySQLManager;      // pointer to static instance of this class

    void CreateMainTable();
    void CreateDataTable(CalibData_t data, Int_t nElem);
    
    TSQLResult* SendQuery(const Char_t* query);
    
    Bool_t SearchTable(CalibData_t data, Char_t* outTableName);
    Bool_t SearchRunEntry(Int_t run, const Char_t* name, Char_t* outInfo);
    Bool_t SearchSetEntry(const Char_t* calibration, const Char_t* name,
                          CalibData_t data, Int_t set, Char_t* outInfo);
    TList* SearchDistinctEntries(const Char_t* data, const Char_t* table);
    
    void DumpRuns(TCContainer* container, Int_t first_run, Int_t last_run);
    void DumpCalibrations(TCContainer* container, const Char_t* calibration);

public:
    TCMySQLManager();
    virtual ~TCMySQLManager();

    Bool_t IsConnected();
    
    TList* GetAllCalibrations();
    TList* GetAllTargets();
    
    Int_t GetNsets(const Char_t* calibration, CalibData_t data);
    Int_t GetFirstRunOfSet(const Char_t* calibration, CalibData_t data, Int_t set);
    Int_t GetLastRunOfSet(const Char_t* calibration, CalibData_t data, Int_t set);
    void GetDescriptionOfSet(const Char_t* calibration, CalibData_t data, 
                             Int_t set, Char_t* outDesc);
    void GetFillTimeOfSet(const Char_t* calibration, CalibData_t data, 
                          Int_t set, Char_t* outTime);
    Int_t* GetRunsOfSet(const Char_t* calibration, CalibData_t data, 
                        Int_t set, Int_t* outNruns);

    void ReadParameters(const Char_t* calibration, Int_t set, CalibData_t data, 
                        Double_t* par, Int_t length);
    void WriteParameters(const Char_t* calibration, Int_t set, CalibData_t data, 
                         Double_t* par, Int_t length);
    
    void AddSet(CalibData_t data, const Char_t* calib, const Char_t* desc,
                Int_t first_run, Int_t last_run, Double_t* par, Int_t length);
    void AddSet(CalibData_t data, const Char_t* calib, const Char_t* desc,
                Int_t first_run, Int_t last_run, Double_t par);
    void AddRunFiles(const Char_t* path, const Char_t* target);
    void AddCalibAR(CalibDetector_t det, const Char_t* calibFileAR,
                    const Char_t* calib, const Char_t* desc,
                    Int_t first_run, Int_t last_run);
    
    void InitDatabase();
    void Export(const Char_t* filename, Int_t first_run = 0, Int_t last_run = 0, 
                const Char_t* calibration = 0);

    static TCMySQLManager* GetManager()
    {
        // return a pointer to the static instance of this class
        if (!fgMySQLManager) fgMySQLManager = new TCMySQLManager();
        return fgMySQLManager;
    }
    
    ClassDef(TCMySQLManager, 0) // Communication with MySQL Server
};

#endif

