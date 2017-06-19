/************************************************************************
 * Author: Thomas Strub                                                 *
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCARFileLoader                                                       //
//                                                                      //
// AR file loading class.                                               //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCARFILELOADER_H
#define TCARFILELOADER_H

#include "Rtypes.h"
#include "TString.h"

class TFile;

class TCARFileLoader
{

protected:
    TString* fInputFilePathPatt;        // input file path pattern

    Int_t fNRuns;                       // number of runs
    Int_t* fRuns;            //[fNRuns]    list of run numbers

    Int_t fNFiles;                      // number of files (= number of runs)
    TFile** fFiles;          //[fNRuns]    list of files
    Int_t fNOpenFiles;                  // number of files opend

    void ResetInputFilePathPatt() { if (fInputFilePathPatt) delete fInputFilePathPatt; fInputFilePathPatt = 0; };
    void ResetRunsList() { if (fRuns) delete [] fRuns; fNRuns = 0; fRuns = 0; };
    void ResetFileList();

    Bool_t CreateFileList();

public:
    TCARFileLoader()
      : fInputFilePathPatt(0),
        fNRuns(0), fRuns(0),
        fNFiles(0), fFiles(0),
        fNOpenFiles(0) { };
    TCARFileLoader(const Char_t* inputfilepathpatt);
    TCARFileLoader(Int_t nruns, const Int_t* runs, const Char_t* inputfilepathpatt = 0);
    TCARFileLoader(const Char_t* data, const Char_t* calibration,
                  Int_t n_sets, Int_t* sets, const Char_t* inputfilepathpatt);
    virtual ~TCARFileLoader();

    static Bool_t IsRegularFile(const Char_t* file);
    static Bool_t IsDirectory(const Char_t* dir);

    static Bool_t CheckInputFilePathPatt(const Char_t* inputfilepathpatt);
    static Bool_t CreateAutoDetectInputFilePathPatt(const Char_t* dir, Char_t*& outpatt, Bool_t bestmatch = kFALSE);
    static Bool_t CreateRunListFromInputFilePathPatt(const Char_t* inputfilepathpatt, Int_t& nruns, Int_t*& runs);

    Bool_t SetInputFilePathPatt(const Char_t* inputfilepathpatt);
    Bool_t SetInputFilePathPattFromConfigFile();
    Bool_t SetInputFilePathPattAutoDetect(const Char_t* dir, Bool_t bestmatch = kFALSE);

    void SetRuns(Int_t nruns, Int_t* runs);
    Bool_t SetRunsFromDataBase(const Char_t* calibration);
    Bool_t SetRunsFromInputFilePathPatt(const Char_t* inputfilepathpatt = 0);

    const Char_t* GetInputFilePathPatt() const { return fInputFilePathPatt ? fInputFilePathPatt->Data() : 0; };

    Int_t GetNRuns() const { return fNRuns; };
    const Int_t* GetRuns() const { return fRuns; };

    Int_t GetNFiles() const { return fNFiles; };
    TFile * const * GetFiles() const { return fFiles; };
    Int_t GetNOpenFiles() const { return fNOpenFiles; };

    Bool_t LoadFiles() { return fFiles ? kTRUE : CreateFileList(); };

    Int_t FindRunIndex(Int_t run) const;

    ClassDef(TCARFileLoader, 0) // AR file loading class
};

#endif
