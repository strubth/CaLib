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

#include "iReadConfig.hh"


using namespace std;


// calibrationd data enumeration
// NOTE: This enum has to be synchronized with fgCalibDataTableNames!
enum ECalibData
{
    // tagger data
    ECALIB_TAGG_T0 = 0,

    // CB data
    ECALIB_CB_T0,
    ECALIB_CB_WALK0,
    ECALIB_CB_WALK1,
    ECALIB_CB_WALK2,
    ECALIB_CB_WALK3,
    ECALIB_CB_E1,
    ECALIB_CB_EQUAD0,
    ECALIB_CB_EQUAD1,
    ECALIB_CB_PI0IM,

    // TAPS data
    ECALIB_TAPS_T0,
    ECALIB_TAPS_T1,
    ECALIB_TAPS_LG_E0,
    ECALIB_TAPS_LG_E1,
    ECALIB_TAPS_SG_E0,
    ECALIB_TAPS_SG_E1,
    ECALIB_TAPS_EQUAD0,
    ECALIB_TAPS_EQUAD1,
    ECALIB_TAPS_PI0IM,

    // PID data
    ECALIB_PID_T0,
    ECALIB_PID_E0,
    ECALIB_PID_E1,

    // VETO data
    ECALIB_VETO_T0,
    ECALIB_VETO_T1,
    ECALIB_VETO_E0,
    ECALIB_VETO_E1,
};
typedef ECalibData CalibData_t;


class iMySQLManager : public virtual iReadConfig
{

private:
    TSQLServer* fDB;
    TString fCalibration;

    void CreateMainTable();
    void CreateDataTable(CalibData_t data, Int_t nElem);
    
    static const Char_t* fgCalibDataTableNames[];
    static const Char_t* fgCalibMainTableName;
    static const Char_t* fgCalibMainTableFormat; 
    static const Char_t* fgCalibDataTableHeader;
 
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
    
    ClassDef(iMySQLManager, 0)   // Connection to MySQL Server
};

#endif

