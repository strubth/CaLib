/*************************************************************************
 * Author: Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCARFileLoader                                                       //
//                                                                      //
// AR Histogram loading base class.                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCARFILELOADER_H 
#define TCARFILELOADER_H

#include "TFile.h"
#include "TCReadConfig.h"


class TCARFileLoader
{

private:

    // ----------------------------- data members ------------------------------

    Int_t fFileListIndex;               //! index of the next file within file list

protected:

    Int_t fNRuns;                       // number of runs
    Int_t* fRuns;                       //[fNRuns] list of run numbers

    TFile** fFiles;                     //[fNRuns] list of files
    Int_t fNGoodFiles;                  // number of files opend

    TString* fInputFilePathPatt;        // input file path pattern


    // --------------------------- members methodes ----------------------------

    Bool_t CreateFileList();

public:
    TCARFileLoader()
      : fFileListIndex(0),
        fNRuns(0), fRuns(0),
        fFiles(0),
        fNGoodFiles(0),
        fInputFilePathPatt(0) { };
    TCARFileLoader(Int_t nruns, const Int_t* runs, const Char_t* inputfilepathpatt = 0);
    virtual ~TCARFileLoader();

    void SetRuns(Int_t nruns, Int_t* runs);
    void SetInputFilePatt(const Char_t* inputfielpathpatt);

    Bool_t LoadFiles() { return fFiles ? kTRUE : CreateFileList(); };

    Bool_t IsGood();
    TFile* NextFile();
    TFile* NextOpenFile();
    void End() { fFileListIndex = 0; };

    Int_t GetNRuns() const { return fNRuns; };
    const Int_t* GetRuns() const { return fRuns; };
    Int_t GetRun() const { return (fFileListIndex > 0) ? fRuns[fFileListIndex-1] : 0; };

    TFile * const * GetFiles() const { return fFiles; };
    Int_t GetNOpenFiles() const { return fNGoodFiles; };

    ClassDef(TCARFileLoader, 0) // AR Histogram loading base class
};

#endif

