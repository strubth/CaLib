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


#include "TCARFileLoader.h"

ClassImp(TCARFileLoader)


//______________________________________________________________________________
TCARFileLoader::TCARFileLoader(Int_t nruns, const Int_t* runs, const Char_t* inputfilepathpatt)
{
    // Constructor

    // init members
    fFileListIndex = 0;

    fNRuns = nruns;
    fRuns = new Int_t[fNRuns];
    for (Int_t i = 0; i < fNRuns; i++)
        fRuns[i] = runs[i];

    fFiles = 0;

    fNGoodFiles = 0;

    fInputFilePathPatt = 0;
    if (inputfilepathpatt) fInputFilePathPatt = new TString(inputfilepathpatt);
}

//______________________________________________________________________________
TCARFileLoader::~TCARFileLoader()
{
    // Destructor

    if (fRuns) delete [] fRuns;
    if (fFiles)
    {
        for (Int_t i = 0; i < fNRuns; i++)
           if (fFiles[i]) delete fFiles[i];
        delete [] fFiles;
    }
    if (fInputFilePathPatt) delete fInputFilePathPatt;
}

//______________________________________________________________________________
Bool_t TCARFileLoader::CreateFileList()
{
    // Creates the array 'fFiles' of pointers to open files (length 'fNRuns').
    // The path of the i-th file is gained form the pattern 'fInputFilePathPatt'
    // by replacing "RUN" by the i-th run number of the array 'fRuns'. If a file
    // cannot be opened the NULL pointer is set for this file.
    // If 'fInputFilePathPatt' is NULL the pattern will be taken from the config
    // file using the TCReadConfig reader.
    // Returns 'kTRUE' on success, 'kFALSE' otherwise.

    // check for run list
    if (!fRuns)
    {
        Error("CreateFileList", "Run list does not exist!");
        return kFALSE;
    }

    // delete old file list
    if (fFiles)
    {
        for (Int_t i = 0; i < fNRuns; i++)
            if (fFiles[i]) delete fFiles[i];
        delete [] fFiles;
    }

    // reset number of good (i.e., open) files
    fNGoodFiles = 0;

    // read input file pattern form config file
    if (!fInputFilePathPatt)
    {
        if (TString* f = TCReadConfig::GetReader()->GetConfig("File.Input.Rootfiles"))
        {
            fInputFilePathPatt = new TString(*f);

            // check file pattern
            if (!fInputFilePathPatt->Contains("RUN"))
            {
                Error("CreateFileList", "Error in file pattern configuration!");
                return kFALSE;
            }
        }
        else
        {
            Error("CreateFileList", "Could not load input file pattern from configuration file!");
            return kFALSE;
        }
    }

    // save the current directory
    TString currdir(gDirectory->GetPath());

    // create file array
    fFiles = new TFile*[fNRuns];

    // loop over runs
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // init file pointer
        fFiles[i] = 0;

        // construct file name
        TString filename(*fInputFilePathPatt);
        filename.ReplaceAll("RUN", TString::Format("%d", fRuns[i]));

        // try to open the file
        TFile* f = TFile::Open(filename.Data(), "READ");

        // check for non-existing file
        if (!f)
        {
            Warning("CreateFileList", "%03d : Could not open file '%s'", i, filename.Data());
            continue;
        }

        // check bad file
        if (f->IsZombie())
        {
            Warning("CreateFileList", "%03d : Could not open zombie file '%s'", i, filename.Data());
            delete f;
            continue;
        }

        // add good file to list
        fFiles[i] = f;

        // increment counter
        fNGoodFiles++;

        // user information
        Info("CreateFileList", "%03d : added file '%s'", i, f->GetName());
    }

    // recover the current directory
    gDirectory->cd(currdir.Data());

    return kTRUE;
}

//______________________________________________________________________________
void TCARFileLoader::SetRuns(Int_t nruns, Int_t* runs)
{
    // Deletes old run list 'fRuns' and the file list 'fFiles'. Sets up the new
    // run list 'fRuns'.

    // delete members first
    if (fRuns) delete [] fRuns;
    if (fFiles)
    {
        for (Int_t i = 0; i < fNRuns; i++)
            if (fFiles[i]) delete fFiles[i];
        delete [] fFiles;
    }

    // set up new run list
    fNRuns = nruns;
    fRuns = new Int_t[fNRuns];
    for (Int_t i = 0; i < fNRuns; i++)
        fRuns[i] = runs[i];
}

//______________________________________________________________________________
void TCARFileLoader::SetInputFilePatt(const Char_t* inputfilepathpatt)
{
    // Sets the path pattern 'fInputFilePathPatt' to 'inputfilepathpatt'. If
    // 'inputfilepathpatt' is NULL 'fInputFilePathPatt' is set to NULL.
    // The list of files 'fFiles' is deleted.

    // delete file list
    if (fFiles)
    {
        for (Int_t i = 0; i < fNRuns; i++)
            if (fFiles[i]) delete fFiles[i];
        delete [] fFiles;
    }

    // delete old pattern
    if (fInputFilePathPatt) delete fInputFilePathPatt;
    fInputFilePathPatt = 0;

    // set new pattern
    if (inputfilepathpatt) fInputFilePathPatt = new TString(inputfilepathpatt);
}

//______________________________________________________________________________
Bool_t TCARFileLoader::IsGood()
{
    // Returns 'kTRUE' if there is a next file pointer (incl. NULL) in the list
    // of files 'fFiles'. Returns 'kFALSE' there is no next file or if 'fFiles'
    // is NULL.

    // load files first
    if (!LoadFiles()) return kFALSE;

    // check range
    if (fFileListIndex >= 0 && fFileListIndex < fNRuns)
        return kTRUE;
    else
        return kFALSE;
}

//______________________________________________________________________________
TFile* TCARFileLoader::NextFile()
{
    // Returns the pointer (incl. NULL) to the next file in the file list
    // 'fFiles'. Returns NULL if there is no next file.

    //return IsGood() ? fFiles[fFileListIndex++] : 0;

    if (IsGood())
        return fFiles[fFileListIndex++];
    else
    {
        // set end of list flag
        fFileListIndex = -1;

        return 0;
    }
}

//______________________________________________________________________________
TFile* TCARFileLoader::NextOpenFile()
{
    // Returns the pointer (excl. NULL) to the next file in the file list
    // 'fFiles'. Returns NULL if there is no next file.

    // init result
    TFile* f = 0;

    // loop over next files
    while (IsGood())
        if ((f = NextFile())) break;

    // set end of list flag
    if (!f) fFileListIndex = -1;

    return f;
}

