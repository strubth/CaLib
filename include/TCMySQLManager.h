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
    Bool_t fSilence;                            // silence mode toggle
    static TCMySQLManager* fgMySQLManager;      // pointer to static instance of this class
    
    void CreateMainTable();
    void CreateDataTable(CalibData_t data, Int_t nElem);
    
    TSQLResult* SendQuery(const Char_t* query);
    
    Bool_t SearchTable(CalibData_t data, Char_t* outTableName);
    Bool_t SearchRunEntry(Int_t run, const Char_t* name, Char_t* outInfo);
    Bool_t SearchSetEntry(CalibData_t data, const Char_t* calibration, Int_t set,
                          const Char_t* name, Char_t* outInfo);
    TList* SearchDistinctEntries(const Char_t* data, const Char_t* table);
    
    Bool_t ChangeRunEntries(Int_t first_run, Int_t last_run, 
                            const Char_t* name, const Char_t* value);
    Bool_t ChangeSetEntry(CalibData_t data, const Char_t* calibration, Int_t set,
                          const Char_t* name, const Char_t* value);

    void ImportRuns(TCContainer* container);
    void ImportCalibrations(TCContainer* container, const Char_t* newCalibName);

public:
    TCMySQLManager();
    virtual ~TCMySQLManager();

    void SetSilenceMode(Bool_t s) { fSilence = s; }
    Bool_t IsConnected();
    
    TList* GetAllCalibrations();
    TList* GetAllTargets();
    
    Int_t GetNsets(CalibData_t data, const Char_t* calibration);
    Int_t GetFirstRunOfSet(CalibData_t data, const Char_t* calibration, Int_t set);
    Int_t GetLastRunOfSet(CalibData_t data, const Char_t* calibration, Int_t set);
    void GetDescriptionOfSet(CalibData_t data, const Char_t* calibration, 
                             Int_t set, Char_t* outDesc);
    void GetChangeTimeOfSet(CalibData_t data, const Char_t* calibration, 
                            Int_t set, Char_t* outTime);
    Int_t* GetRunsOfSet(CalibData_t data, const Char_t* calibration,
                        Int_t set, Int_t* outNruns);
    Int_t GetSetForRun(CalibData_t data, const Char_t* calibration, Int_t run);

    Bool_t ReadParameters(CalibData_t data, const Char_t* calibration, Int_t set, 
                          Double_t* par, Int_t length);
    Bool_t ReadParametersRun(CalibData_t data, const Char_t* calibration, Int_t run, 
                             Double_t* par, Int_t length);
    Bool_t WriteParameters(CalibData_t data, const Char_t* calibration, Int_t set, 
                           Double_t* par, Int_t length);
    
    Bool_t ChangeRunPath(Int_t first_run, Int_t last_run, const Char_t* path);
    Bool_t ChangeRunTarget(Int_t first_run, Int_t last_run, const Char_t* target);
    Bool_t ChangeRunTargetPol(Int_t first_run, Int_t last_run, const Char_t* target_pol);
    Bool_t ChangeRunTargetPolDeg(Int_t first_run, Int_t last_run, Double_t target_pol_deg);
    Bool_t ChangeRunBeamPol(Int_t first_run, Int_t last_run, const Char_t* beam_pol);
    Bool_t ChangeRunBeamPolDeg(Int_t first_run, Int_t last_run, Double_t beam_pol_deg);
    
    Bool_t ChangeCalibrationName(const Char_t* calibration, const Char_t* newCalibration);

    Bool_t AddSet(CalibData_t data, const Char_t* calibration, const Char_t* desc,
                  Int_t first_run, Int_t last_run, Double_t* par, Int_t length);
    Bool_t AddSet(CalibData_t data, const Char_t* calibration, const Char_t* desc,
                  Int_t first_run, Int_t last_run, Double_t par);
    Bool_t RemoveSet(CalibData_t data, const Char_t* calibration, Int_t set);
    Bool_t SplitSet(CalibData_t data, const Char_t* calibration, Int_t set,
                    Int_t lastRunFirstSet);
    Bool_t MergeSets(CalibData_t data, const Char_t* calibration, 
                     Int_t set1, Int_t set2);

    void AddRunFiles(const Char_t* path, const Char_t* target);
    void AddCalibAR(CalibDetector_t det, const Char_t* calibFileAR,
                    const Char_t* calib, const Char_t* desc,
                    Int_t first_run, Int_t last_run);
    
    void InitDatabase();
    
    void DumpRuns(TCContainer* container, Int_t first_run = 0, Int_t last_run = 0);
    void DumpAllCalibrations(TCContainer* container, const Char_t* calibration);
    void DumpCalibrations(TCContainer* container, const Char_t* calibration, 
                          CalibData_t data);
    
    void Export(const Char_t* filename, Int_t first_run, Int_t last_run, 
                const Char_t* calibration);
    void Import(const Char_t* filename, Bool_t runs, Bool_t calibrations,
                const Char_t* newCalibName = 0);

    static TCMySQLManager* GetManager()
    {
        // return a pointer to the static instance of this class
        if (!fgMySQLManager) fgMySQLManager = new TCMySQLManager();
        return fgMySQLManager;
    }
    
    ClassDef(TCMySQLManager, 0) // Communication with MySQL Server
};

#endif

