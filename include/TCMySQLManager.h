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


class TCMySQLManager
{

private:
    TSQLServer* fDB;
    TString fCalibration;

    void CreateMainTable();
    void CreateDataTable(CalibData_t data, Int_t nElem);
    
    static TCMySQLManager* fgMySQLManager;

public:
    TCMySQLManager();
    virtual ~TCMySQLManager();

    TSQLResult* SendQuery(const Char_t* query);
    Bool_t IsConnected();
    void SearchTable(CalibData_t data, Char_t* outTableName);

    TList* GetAllTargets();
    Int_t GetNsets(CalibData_t data);
    Int_t GetFirstRunOfSet(CalibData_t data, Int_t set);
    Int_t GetLastRunOfSet(CalibData_t data, Int_t set);
    Int_t* GetRunsOfSet(CalibData_t data, Int_t set, Int_t* outNruns);
    Long64_t GetUnixTimeOfRun(Int_t run);

    void ReadParameters(Int_t set, CalibData_t data, Double_t* par, Int_t length);
    void WriteParameters(Int_t set, CalibData_t data, Double_t* par, Int_t length);
    void AddSet(CalibData_t data, const Char_t* calib, const Char_t* desc,
                Int_t first_run, Int_t last_run, Double_t* par, Int_t length);

    void InitDatabase();

    static TCMySQLManager* GetManager()
    {
        if (!fgMySQLManager) fgMySQLManager = new TCMySQLManager();
        return fgMySQLManager;
    }
    
    ClassDef(TCMySQLManager, 0)   // Communication with MySQL Server
};

#endif

