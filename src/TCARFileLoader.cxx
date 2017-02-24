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


#include "TCARFileLoader.h"
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TFile.h"
#include "TCReadConfig.h"
#include "TCMySQLManager.h"
#include "TRegexp.h"
#include "TMath.h"


ClassImp(TCARFileLoader)


//______________________________________________________________________________
TCARFileLoader::TCARFileLoader(const Char_t* inputfilepathpatt)
{
    // Constructor

    // init members
    fInputFilePathPatt = 0;

    fNRuns = 0;
    fRuns = 0;

    fNFiles = 0;
    fFiles = 0;

    fNOpenFiles = 0;

    if (inputfilepathpatt)
    {
        // check if pattern is a directory
        if (IsDirectory(inputfilepathpatt))
        {
            // try to find pattern
            Char_t* outpat;
            CreateAutoDetectInputFilePathPatt(inputfilepathpatt, outpat, kTRUE);
            if (outpat)
            {
                fInputFilePathPatt = new TString(outpat);
                delete outpat;
            }
            else
            {
                Warning("TCARFileLoader", "Unable to find pattern.");
            }
        }
        else
        {
            if (CheckInputFilePathPatt(inputfilepathpatt))
                fInputFilePathPatt = new TString(inputfilepathpatt);
            else
                Warning("TCARFileLoader", "Error in pattern.");
        }
    }
}


//______________________________________________________________________________
TCARFileLoader::TCARFileLoader(Int_t nruns, const Int_t* runs, const Char_t* inputfilepathpatt)
{
    // Constructor

    // init members
    fInputFilePathPatt = 0;

    fNRuns = nruns;
    fRuns = new Int_t[fNRuns];
    for (Int_t i = 0; i < fNRuns; i++)
        fRuns[i] = runs[i];

    fNFiles = 0;
    fFiles = 0;

    fNOpenFiles = 0;

    if (inputfilepathpatt)
    {
        // check if pattern is a directory
        if (IsDirectory(inputfilepathpatt))
        {
            // try to find pattern
            Char_t* outpat;
            CreateAutoDetectInputFilePathPatt(inputfilepathpatt, outpat);
            if (outpat)
            {
                fInputFilePathPatt = new TString(outpat);
                delete outpat;
            }
            else
            {
                Warning("TCARFileLoader", "Unable to find pattern.");
            }
        }
        else
        {
            if (CheckInputFilePathPatt(inputfilepathpatt))
                fInputFilePathPatt = new TString(inputfilepathpatt);
            else
                Warning("TCARFileLoader", "Error in pattern.");
        }
    }
}


//______________________________________________________________________________
TCARFileLoader::~TCARFileLoader()
{
    // Destructor

    if (fRuns) delete [] fRuns;
    if (fFiles)
    {
        for (Int_t i = 0; i < fNFiles; i++)
           if (fFiles[i]) delete fFiles[i];
        delete [] fFiles;
    }
    if (fInputFilePathPatt) delete fInputFilePathPatt;
}


//______________________________________________________________________________
void TCARFileLoader::ResetFileList()
{
    // Deletes the array of open files 'fFiles' and resets 'fNOpenFiles'.

    // delete old file list
    if (fFiles)
    {
        for (Int_t i = 0; i < fNFiles; i++)
            if (fFiles[i]) delete fFiles[i];
        delete [] fFiles;
        fFiles = 0;
    }

    // reset number of files
    fNFiles = 0;

    // reset number of open files
    fNOpenFiles = 0;
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

    // reset file list
    ResetFileList();

    // check for input filepath patter
    if (!fInputFilePathPatt)
    {
        // read inputfile path pattern form config
        if (!SetInputFilePathPattFromConfigFile())
        {
            Error("CreateFileList", "No input file path pattern set!");
            return kFALSE;
        }
    }

    // check for run list
    if (!fRuns)
    {
        // set runs from input filepath pattern
        if (!SetRunsFromInputFilePathPatt())
        {
            Error("CreateFileList", "Run list does not exist!");
            return kFALSE;
        }
    }

    // save the current directory, since it will be changed when opening the files
    TString currdir(gDirectory->GetPath());

    // create file array
    fNFiles = fNRuns;
    fFiles = new TFile*[fNFiles];

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

        // add file to list
        fFiles[i] = f;

        // increment number of open files
        fNOpenFiles++;

        // user information
        Info("CreateFileList", "%03d : added file '%s'", i, f->GetName());
    }

    // recover the current directory
    gDirectory->cd(currdir.Data());

    return kTRUE;
}


//______________________________________________________________________________
Bool_t TCARFileLoader::IsRegularFile(const Char_t* file)
{
    // Returns kTRUE if 'file' is a regular file, kFALSE otherwise

    // check for existing file (code from protected TSystemDirectory::IsItDirectory())
    Long64_t size;
    Long_t id, flags, modtime;
    flags = id = size = modtime = 0;
    if (gSystem->GetPathInfo(file, &id, &size, &flags, &modtime))
        return kFALSE;
    if ((Int_t) flags != 0)
        return kFALSE;

    return kTRUE;
}


//______________________________________________________________________________
Bool_t TCARFileLoader::IsDirectory(const Char_t* dir)
{
    // Returns kTRUE if 'dir' is a valid directory, kFALSE otherwise

    // check argument
    if (!dir) return kFALSE;

    // check for existing directory (code from protected TSystemDirectory::IsItDirectory())
    Long64_t size;
    Long_t id, flags, modtime;
    flags = id = size = modtime = 0;
    if (gSystem->GetPathInfo(dir, &id, &size, &flags, &modtime))
        return kFALSE;
    if ((Int_t) (flags & 2) == 0)
        return kFALSE;

    return kTRUE;
}


//______________________________________________________________________________
Bool_t TCARFileLoader::CheckInputFilePathPatt(const Char_t* inputfilepathpatt)
{
    // Checks the validity of the input file path pattern, i.e., for a valid
    // directory and for a file name containing exactly one occurrence of the
    // string 'RUN'. Returns kTRUE if these conditions are fulfilled, kFALSE
    // otherwise.

    // check for NULL pointer
    if (!inputfilepathpatt) return kFALSE;

    // interpret variables
    Char_t* f = gSystem->ExpandPathName(inputfilepathpatt);

    // prepare directory / patten string
    TString loc(f);
    TString pat(f);

    // clean-up
    delete f;

    // check for directory
    if (loc.Last('/') >= 0)
    {
        // dir found
        loc.Remove(loc.Last('/'), loc.Length());

        // check for valid dir
        if (!IsDirectory(loc.Data()))
        {
            Error("CheckInputFilePathPatt", "Invalid directory '%s'!", loc.Data());
            return kFALSE;
        }

        pat.Remove(0, pat.Last('/')+1);
    }
    else
    {
        // dir is current dir
        loc = ".";
    }

    // get index of "RUN"
    Int_t index = pat.Index("RUN");

    // check for occurrence of "RUN"
    if (index < 0)
    {
        Error("CheckInputFilePathPatt", "Missing occurrence of 'RUN' string!");
        return kFALSE;
    }

    // get second index of "RUN"
    index = pat.Index("RUN", index+3);

    // check for second occurrence of "RUN"
    if (index > 0)
    {
        Error("CheckInputFilePathPatt", "Multiple occurrence of 'RUN' string!");
        return kFALSE;
    }

    return kTRUE;
}


//______________________________________________________________________________
Bool_t TCARFileLoader::CreateRunListFromInputFilePathPatt(const Char_t* inputfilepathpatt, Int_t& nruns, Int_t*& runs)
{
    // Returns the number of runs and the run list with the 'inputfilepathpatt'.
    // Returns kTRUE on success, kFALSE otherwise.

    // init return variables
    nruns = 0;
    runs = 0;

    // check input filepath pattern
    if (!CheckInputFilePathPatt(inputfilepathpatt)) return kFALSE;

    // get directory
    TString loc(inputfilepathpatt);
    if (loc.Last('/') >= 0)
        loc.Remove(loc.Last('/'), loc.Length());
    else
        loc = ".";

    // get filename pattern
    TString pat(inputfilepathpatt);
    if (pat.Last('/') >= 0)
        pat.Remove(0, pat.Last('/')+1);

    // try to get directory content
    TSystemDirectory dir("rawdir", loc);
    TList* list = dir.GetListOfFiles();

    // sort files
    list->Sort();

    // get prefix and suffix
    TString pat_pre(pat(0, pat.Index("RUN")));
    TString pat_suf(pat(pat.Index("RUN")+3, pat.Length()));

    // prepare regexpr, i.e., "xxxRUNyyy" --> "^xxx[0-9]+yyy$"
    pat.Prepend("^");
    pat.Append("$");
    pat.ReplaceAll("RUN", "[0-9]+");
    TRegexp r(pat.Data());

    // temp run list
    Int_t nruns_tmp = 0;
    Int_t runs_tmp[10000];

    // loop over directory content
    TIter next(list);
    TSystemFile* f;
    while ((f = (TSystemFile*) next()))
    {
        // get file name
        TString name(f->GetName());

        // look for correct pattern
        if (!name.Contains(r)) continue;

        // extract run number
        name.ReplaceAll(pat_pre, "");
        name.ReplaceAll(pat_suf, "");

        // set run
        runs_tmp[nruns_tmp] = atoi(name);
        nruns_tmp++;
    }

    // sort runs
    Int_t sort_tmp[10000];
    TMath::Sort(nruns_tmp, runs_tmp, sort_tmp, kFALSE);

    // return runs
    nruns = nruns_tmp;
    runs = new Int_t[nruns];
    for (Int_t i = 0; i < nruns; i++)
        runs[i] = runs_tmp[sort_tmp[i]];

    return kTRUE;
}


//______________________________________________________________________________
Bool_t TCARFileLoader::CreateAutoDetectInputFilePathPatt(const Char_t* dir, Char_t*& outpatt, Bool_t bestmatch /*= kFALSE*/)
{
    // Guesses the file path pattern from the content of directory 'dir' and
    // returns the result via 'outpatt'.

    // init out pattern
    outpatt = 0;

    // check directory
    if (!IsDirectory(dir)) return kFALSE;

    // get list of files
    TSystemDirectory sdir("rawdir", dir);
    TList* list = sdir.GetListOfFiles();

    // sort files
    list->Sort();

    // file patterns
    Int_t npat = 0;
    Int_t maxnpats = 16;
    TString** filepats = new TString*[maxnpats];
    Int_t* matchcount = new Int_t[maxnpats];
    for (Int_t i = 0; i < maxnpats; i++)
        matchcount[i] = 0;

    // loop over directory content
    TIter next(list);
    TSystemFile* f;
    while ((f = (TSystemFile*) next()))
    {
        // get file name
        TString name(f->GetName());

        // init helpers
        Ssiz_t index = 0;
        Ssiz_t length = 0;
        Ssiz_t start = 0;

        // loop over patterns of this filename
        while((index = name.Index("[0-9]+", &length, start)) >= 0)
        {
            // copy name
            TString str(name.Data());

            // replace number with RUN
            str.Replace(index, length, "RUN");

            // check matches
            Bool_t ismatch = kFALSE;
            for (Int_t i = 0; i < npat; i++)
            {
                if(filepats[i]->CompareTo(str) == 0)
                {
                    matchcount[i]++;
                    ismatch = kTRUE;
                    break;
                }
            }

            // check for match
            if (!ismatch)
            {
                // increase capacity
                if (npat == maxnpats)
                {
                    // double capacity
                    TString** filepats_tmp = new TString*[2*maxnpats];
                    Int_t* matchcount_tmp = new Int_t[2*maxnpats];

                    // copy old
                    for (Int_t i = 0; i < maxnpats; i++)
                    {
                        filepats_tmp[i] = filepats[i];
                        matchcount_tmp[i] = matchcount[i];

                        // init new
                        matchcount_tmp[maxnpats + i] = 0;
                    }

                    // delete old arrays
                    delete [] filepats;
                    delete [] matchcount;

                    // set pointers
                    filepats = filepats_tmp;
                    matchcount = matchcount_tmp;

                    // update max number of pats
                    maxnpats *= 2;
                }

                // save new pat
                filepats[npat] = new TString(str);
                matchcount[npat]++;
                npat++;
            }

            // set new start pos
            start = index + length;

        }// loop over patterns

        // check for bestmach option
        if (!bestmatch && npat > 1) break;

    }// loop over files

    // init success
    Bool_t success = kTRUE;

    // check whether pattern was found
    if (npat == 0)
    {
        Error("AutoDetectInputFilePathPatt", "No matches found!");
        success = kFALSE;
    }
    else if (!bestmatch && npat > 1)
    {
        Error("AutoDetectInputFilePathPatt", "Ambiguous matches found, e.g., '%s' and '%s'!", filepats[0]->Data(), filepats[1]->Data());
        success = kFALSE;
    }
    else
    {
        // easy reading helper
        TString*& filepat = filepats[TMath::LocMax(npat, matchcount)];

        // create string (+2 because of '/' and  '\0')
        outpatt = new Char_t[strlen(dir) + filepat->Length() + 2];

        // set outpatt
        sprintf(outpatt, "%s/%s", dir, filepat->Data());
    }

    // debug
    //for (Int_t i = 0; i < npat; i++)
    //   printf("N: %d Patt: %s\n", matchcount[i], filepats[i]->Data());

    // clean up
    for (Int_t i = 0; i < npat; i++)
        delete filepats[i];
    delete [] filepats;
    delete [] matchcount;

    return success;
}


//______________________________________________________________________________
Bool_t TCARFileLoader::SetInputFilePathPatt(const Char_t* inputfilepathpatt)
{
    // Sets the path pattern 'fInputFilePathPatt' to 'inputfilepathpatt'. If
    // 'inputfilepathpatt' is NULL 'fInputFilePathPatt' is set to NULL.
    // The list of files 'fFiles' is deleted.

    // reset file list
    ResetFileList();

    // reset input filepath pattern
    ResetInputFilePathPatt();

    // set input filepath pattern only if valid
    if (inputfilepathpatt)
        fInputFilePathPatt = new TString(inputfilepathpatt);

    return (Bool_t) fInputFilePathPatt;
}


//______________________________________________________________________________
Bool_t TCARFileLoader::SetInputFilePathPattFromConfigFile()
{
    // Gets the input filepath pattern from config file

    // reset input filepath pattern
    ResetInputFilePathPatt();

    // read input file pattern form config file
    if (TString* patt = TCReadConfig::GetReader()->GetConfig("File.Input.Rootfiles"))
    {
        // set input filepath pattern only if valid
        if (CheckInputFilePathPatt(patt->Data()))
            fInputFilePathPatt = new TString(*patt);
        else
            return kFALSE;
    }
    else
    {
        Error("SetInputFilePathPattFromConfigFile", "Could not load input filepath pattern from configuration file!");
        return kFALSE;
    }

    return kTRUE;
}


//______________________________________________________________________________
Bool_t TCARFileLoader::SetInputFilePathPattAutoDetect(const Char_t* dir, Bool_t bestmatch)
{
    // Sets the input file path pattern by guessing it.

    // reset input filepath pattern
    ResetInputFilePathPatt();

    Char_t* patt;
    if (CreateAutoDetectInputFilePathPatt(dir, patt, bestmatch))
        fInputFilePathPatt = new TString(patt);

    if (patt) delete patt;

    return (Bool_t) fInputFilePathPatt;
}


//______________________________________________________________________________
void TCARFileLoader::SetRuns(Int_t nruns, Int_t* runs)
{
    // Resets the file list and deletes the run list 'fRuns'. Sets up the new
    // run list 'fRuns'.

    // reset runs and file list
    ResetRunsList();
    ResetFileList();

    // set up new run list
    fNRuns = nruns;
    fRuns = new Int_t[fNRuns];
    for (Int_t i = 0; i < fNRuns; i++)
        fRuns[i] = runs[i];
}


//______________________________________________________________________________
Bool_t TCARFileLoader::SetRunsFromDataBase(const Char_t* calibration)
{
    // Loads the number of runs and the list of runs from the database. Returns
    // kTRUE on success, kFALSE otherwise.

    // reset run and file list
    ResetRunsList();
    ResetFileList();

    // try to connect to database
    if (!TCMySQLManager::GetManager())
    {
        Error("SetRunsFromDataBase", "Could not connect to database!");
        return kFALSE;
    }

    // get runs from database
    fRuns = TCMySQLManager::GetManager()->GetRunsOfCalibration(calibration, &fNRuns);
    if (!fRuns)
    {
        Error("SetRunsFromDataBase", "Could not get runs for calibration '%s'!", calibration);
        return kFALSE;
    }

    return kTRUE;
}


//______________________________________________________________________________
Bool_t TCARFileLoader::SetRunsFromInputFilePathPatt(const Char_t* inputfilepathpatt)
{
    // Loads the number of runs and the list with the 'fInputFilePathPatt'.
    // Returns kTRUE on success, kFALSE otherwise.

    // reset run and file list
    ResetRunsList();
    ResetFileList();

    // check whether input filepath pattern  was given as an argument
    if (inputfilepathpatt)
    {
        // reset input filepath pattern
        ResetInputFilePathPatt();

        // set new input filepath pattern only if valid
        if (CheckInputFilePathPatt(inputfilepathpatt))
            fInputFilePathPatt = new TString(inputfilepathpatt);
        else
            return kFALSE;
    }
    else
    {
        // check for input filepath pattern
        if (!fInputFilePathPatt)
        {
            Error("SetRunsFromInputFilePathPatt", "Input filepath pattern not set!");
            return kFALSE;
        }
    }

    // create run list
    return CreateRunListFromInputFilePathPatt(fInputFilePathPatt->Data(), fNRuns, fRuns);
}


//______________________________________________________________________________
Int_t TCARFileLoader::FindRunIndex(Int_t run) const
{
    // Returns the index of the run with run number 'run' within the list of
    // runs 'fRuns'. If it cannot be found -1 is returned;

    // loop over runs
    for (Int_t i = 0; i < fNRuns; i++)
        if (fRuns[i] == run) return i;

    return -1;
}

//finito
