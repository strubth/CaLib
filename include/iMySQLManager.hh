/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iMySQLManager                                                        //
//                                                                      //
// This class handles all the communication with the MySQL server.      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef IMYSQLMANAGER_HH
#define IMYSQLMANAGER_HH

#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TList.h"
#include "TError.h"
#include "TObjString.h"

#include "iConfig.hh"
#include "iReadConfig.hh"


class iMySQLManager
{

private:
    TSQLServer* fDB;
    TString fCalibration;

    void CreateMainTable();
    void CreateDataTable(CalibData_t data, Int_t nElem);
    
    static iMySQLManager* fgMySQLManager;

public:
    iMySQLManager();
    virtual ~iMySQLManager();

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

    void InitDatabase();

    static iMySQLManager* GetManager()
    {
        if (!fgMySQLManager) fgMySQLManager = new iMySQLManager();
        return fgMySQLManager;
    }
    
    ClassDef(iMySQLManager, 0)   // Communication with MySQL Server
};

#endif

