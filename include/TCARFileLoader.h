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

class TFile;
class TString;

class TCARFileLoader
{
private:

    // ----------------------------- data members ------------------------------

    Int_t fFileListIndex;           //! // index of the next file within file list

protected:

    Int_t fNRuns;                       // number of runs
    Int_t* fRuns;            //[fNRuns] // list of run numbers

    TFile** fFiles;          //[fNRuns] // list of files
    Int_t fNOpenFiles;                  // number of files opend

    TString* fInputFilePathPatt;        // input file path pattern


    // --------------------------- members methodes ----------------------------

    void ResetFileList();
    Bool_t CreateFileList();

public:
    TCARFileLoader()
      : fFileListIndex(0),
        fNRuns(0), fRuns(0),
        fFiles(0),
        fNOpenFiles(0),
        fInputFilePathPatt(0) { };
    TCARFileLoader(Int_t nruns, const Int_t* runs, const Char_t* inputfilepathpatt = 0);
    virtual ~TCARFileLoader();

    void SetRuns(Int_t nruns, Int_t* runs);
    void SetInputFilePatt(const Char_t* inputfilepathpatt);
    const Char_t* GetImputFilePatt();

    Bool_t LoadFiles() { return fFiles ? kTRUE : CreateFileList(); };

    Bool_t IsGood();
    TFile* NextFile();
    TFile* NextOpenFile();
    inline void End() { fFileListIndex = 0; };

    inline Int_t GetRun() const { return (fFileListIndex > 0) ? fRuns[fFileListIndex-1] : 0; };
    inline const TFile* GetFile() const { return (fFileListIndex > 0) ? fFiles[fFileListIndex-1] : 0; }

    inline Int_t GetNRuns() const { return fNRuns; };
    inline const Int_t* GetRuns() const { return fRuns; };

    inline TFile * const * GetFiles() const { return fFiles; };
    inline Int_t GetNOpenFiles() const { return fNOpenFiles; };

    Int_t FindRunIndex(Int_t run);

    ClassDef(TCARFileLoader, 0) // AR file loading class
};

#endif

