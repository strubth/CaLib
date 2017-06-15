/************************************************************************
 * Author: Thomas Strub                                                 *
 ************************************************************************/

////////////////////////////////////////////////////////////////////////////////
//
// TCARHistoLoader
//
// AcquRoot histogram loading class.
//
// A) Loading histograms with static functions
//    1)  Load a single histogram from a file f
//          TH1* h = TCARHistoLoader::GetHisto(f, "MyHistogram", kFALSE);
//
//    2)  Load multiple histograms from a file f
//          Int_t n;
//          TH1** hs = TCARHistoLoader::GetHistos(f, "MyHistograms_[0-9]+", n, kFALSE);
//        Returns an array of histograms which names mach the pattern
//        "MyHistograms_[0-9]+". The Length of the array is returned through the
//        argument 'n'.
//
// B) Loading histograms:
//   1.1) Load single histograms from one file, e.g.,
//          TH1* h1 = hl.GetHistoForRun("MyHistogram", 2347, "#NAME");
//          TH1* h2 = hl.GetHistoForIndex("MyHistogram", hl.FindRunIndex(2347));
//        Now, h1 and h2 are clones of the same original histogram 'MyHistogram',
//        where h1 is named "MyHistogram" and h2 is named "MyHistogram_2347".
//
//   1.2) Load multiple histograms from one file, e.g.,
//          Int_t n;
//          TH1** hs = hl.GetHistosForIndex("^MyHistogram_[0-9]+$", 2347, n);
//        Now, hs is an array of length n of histograms having the names matching
//        the pattern, i.e., 'MyHistogram_' followed by a number.
//
//   2)   Create summed up histograms, e.g.,
//          TH1* h = hl.CreateHistoSum("MyHistogram");
//
//   3.1) Create array of histograms, e.g.,
//          TH1** hs = hl.CreateHistoArray("MyHistogram");
//        Now, hs an array of length hl.GetNRuns(), where the i-th element is the
//        histogram of the i-th file with the run with runnumber hl.GetRuns()[i].
//
//   3.2) Create array of projection of histos.
//          TH1** hsp1 = hl.CreateHistoArrayOfProj("MyHistogram");
//          TH1** hsp2 = hl.CreateHistoArrayOfProj("MyHistogram", 'Y', 23, kLastBin);
//
//   4)   Create a single histo of projections
//          TH1* h = hl.CreateHistoOfProj("MyHistogram", 'X', 'Y');
//        Now, h is a 2-dim histogram, where the y-slice of its i-th y-bin is the
//        x-projection of the histogram 'MyHistogram' of the i-th file.
//
//
// C) Naming histograms:
//    By default, the histograms will be renamed to avoid potential memory
//    leaks (for the standard behaviour, such memory leaks will not occurr
//    because the histograms will not be owned by any directory (c.f D)).
//    In general the name of a loaded histogram will be syffixed by an
//    underscore followed by the run number. The default naming is as follows.
//
//      Single histograms: NAME_RUN
//      Arrays           : NAME_RUN
//      Sums             : NAME_Sum
//      Projections      : NAME_px,py,pz
//
//    The default histogram naming can be changed by the 'histogram name pattern'
//    arument. The histogram names will be replaced, where the following
//    patterns will be replaced.
//      '#NAME'     replaced with the original histogram name
//      '#RUN'      replaced with the runnumber of the file
//      '#INDEX'    replaced with the index of the file
//
// D) Histogram ownership:
//    By default, the histograms are not owned by any directory. This can be
//    changed by hl.SetHistoDirectory(), e.g.,
//      SetHistoDirectory(gDirectory);
//
// Have fun!
//
////////////////////////////////////////////////////////////////////////////////


#include "TCARHistoLoader.h"
#include "TROOT.h"
#include "TClass.h"
#include "TKey.h"
#include "TMath.h"
#include "TH2.h"
#include "TH3.h"
#include "TFile.h"
#include "TError.h"
#include "TRegexp.h"

ClassImp(TCARHistoLoader)


const Int_t TCARHistoLoader::kLastBin = -2147483648; // = (Int_t) 2^31


//______________________________________________________________________________
void TCARHistoLoader::SetHistoName(TH1* h, const Char_t* hnamepatt, Int_t index)
{
    // Rename histo 'h'.

    // create string
    TString hname(hnamepatt);

    // replace with histogram name
    hname.ReplaceAll("#NAME", h->GetName());

    // replace with runnumber
    Char_t snumber[128];
    sprintf(snumber, "%d", fRuns[index]);
    hname.ReplaceAll("#RUN", snumber);

    // replace with index
    Char_t sindex[128];
    sprintf(sindex, "%d", index);
    hname.ReplaceAll("#INDEX", sindex);

    // set new name
    h->SetName(hname);
}

//______________________________________________________________________________
TH1* TCARHistoLoader::GetHisto(const TFile* f, const Char_t* hname, Bool_t detach /*= kTRUE*/)
{
    // Basic *static* histogram getter method! Returns the histogram named
    // 'hname' from the file 'f'. If detach is kTRUE it is detached from the
    // file.
    // Returns 0 if the histogram does not exist.

    // check for file
    if (!f) return 0;

    // get list of histos
    TList* list = f->GetListOfKeys();

    // init result
    TH1* hOut = 0;

    // loop over keys
    TIter next(list);
    TKey* key;
    while ((key = (TKey*) next()))
    {
        // check for histogram
        TClass* cl = gROOT->GetClass(key->GetClassName());
        if (!cl->InheritsFrom("TH1")) continue;

        // check name
        if (strcmp(hname, key->GetName())) continue;

        // get histogram (detached)
        Bool_t status = TH1::AddDirectoryStatus();
        if (detach) TH1::AddDirectory(kFALSE);
        else TH1::AddDirectory(kTRUE);

        hOut = (TH1*) key->ReadObj();

        TH1::AddDirectory(status);

        break;

    } // loop over keys

    return hOut;
}

//______________________________________________________________________________
TH1** TCARHistoLoader::GetHistos(const TFile* f, const Char_t* hpatt, Int_t& nhistos, Bool_t detach /*= kTRUE*/)
{
    // Basic *static* histogram array getter method! Returns an array of
    // histograms maching the pattern 'hpatt' from the file 'f'. Its length is
    // returned via 'nhistos,'
    // If detach is kTRUE it is detached from the file.
    // Returns 0 if the histogram does not exist.

    // init return variable
    nhistos = 0;

    // check for file
    if (!f) return 0;

    // get list of histos
    TList* list = f->GetListOfKeys();
    list->Sort();

    // get maximal number of histograms
    Int_t nmaxhistos = list->GetSize();;

    // prepare histogram array
    TH1** hOut_tmp = new TH1*[nmaxhistos];

    // ceate regexp
    TRegexp r(hpatt);

    // loop over keys
    TIter next(list);
    TKey* key;
    while ((key = (TKey*) next()))
    {
        // check for histogram
        TClass* cl = gROOT->GetClass(key->GetClassName());
        if (!cl->InheritsFrom("TH1")) continue;

        // get name
        TString hname(key->GetName());

        // check pattern
        if (!hname.Contains(r)) continue;

        // get histogram (detached)
        Bool_t status = TH1::AddDirectoryStatus();
        if (detach) TH1::AddDirectory(kFALSE);
        else TH1::AddDirectory(kTRUE);

        TH1* h = (TH1*) key->ReadObj();

        TH1::AddDirectory(status);

        // add to list
        hOut_tmp[nhistos] = h;
        nhistos++;

    } // loop over keys

    // create final array
    TH1** hOut = new TH1*[nhistos];
    for (Int_t i = 0; i < nhistos; i++)
        hOut[i] = hOut_tmp[i];

    // clean up
    delete [] hOut_tmp;

    return hOut;
}

//______________________________________________________________________________
TH1* TCARHistoLoader::GetHistoForRun(const Char_t* hname, Int_t runnumber, const Char_t* houtnamepatt /* = 0*/)
{
    // Returns the pointer to the histogram with name 'hname' for the runnumber
    // 'runnumber' (c.f, 'GetHisto()' for further information). If 'runnumber'
    // is not a valid runnumber the NULL pointer is returned.

    // get index
    Int_t index = FindRunIndex(runnumber);

    // check whether runnumber was found
    if (index < 0)
    {
        Error("GetHistoForRun", "Runnumber '%d' is not a valid runnumber!", runnumber);
        return 0;
    }

    // return histo
    return GetHistoForIndex(hname, index, houtnamepatt);
}

//______________________________________________________________________________
TH1* TCARHistoLoader::GetHistoForIndex(const Char_t* hname, Int_t index, const Char_t* houtnamepatt /*= 0*/)
{
    // Returns the pointer to the histogram with name 'hname' loaded from the
    // file 'fFiles[index]' (i.e., the AR file of the run with run number
    // 'fRuns[i]'). The histogram name is suffixed with an underscore followed
    // by the associated the run number or renamed according to the 'houtnamepatt'.
    // If the file does not exist or if the histogram cannot be found, the NULL
    // pointer returned
    // NOTE: the histogram has to be destroyed by the caller.

    // check index
    if (0 > index || index >= fNRuns)
    {
        Error("GetHistoForIndex", "Index '%d' out allowed range [0,%d]!", index, TMath::Max(0, fNRuns-1));
        return 0;
    }

    // load files first (if not already loaded)
    if (!LoadFiles()) return 0;

    // check for file
    if (!fFiles[index]) return 0;

    // get histogram detached
    TH1* h = GetHisto(fFiles[index], hname);

    // check for histogram
    if (!h)
    {
        Error("GetHistoForIndex", "Histogram '%s' was not found in file '%s'!",
                                  hname, fFiles[index]->GetName());
        return 0;
    }

    // set histogram name
    if (houtnamepatt)
    {
        // set user name
        SetHistoName(h, houtnamepatt, index);
    }
    else
    {
        // set default name
        Char_t tmp[256];
        sprintf(tmp, "%s_%i", hname, fRuns[index]);
        h->SetName(tmp);
    }

    // set directory
    if (TH1::AddDirectoryStatus())
        h->SetDirectory(fHistoDirectory);

    return h;
}

//______________________________________________________________________________
TH1** TCARHistoLoader::GetHistosForRun(const Char_t* hpatt, Int_t runnumber, Int_t& nhistos, const Char_t* houtnamepatt /*= 0*/)
{
    // Returns an array of histograms for the run 'run' maching the pattern
    // 'hpatt'.

    // get index
    Int_t index = FindRunIndex(runnumber);

    // check whether runnumber was found
    if (index < 0)
    {
        Error("GetHistosForRun", "Runnumber '%d' is not a valid runnumber!", runnumber);
        return 0;
    }

    // return histo
    return GetHistosForIndex(hpatt, index, nhistos, houtnamepatt);
}

//______________________________________________________________________________
TH1** TCARHistoLoader::GetHistosForIndex(const Char_t* hpatt, Int_t index, Int_t& nhistos, const Char_t* houtnamepatt /*= 0*/)
{
    // Returns an array of histograms for the run 'run' maching the pattern
    // 'hpatt'.

    // check index
    if (0 > index || index >= fNRuns)
    {
        Error("GetHistosForIndex", "Index '%d' out allowed range [0,%d]!", index, TMath::Max(0, fNRuns-1));
        return 0;
    }

    // load files first (if not already loaded)
    if (!LoadFiles()) return 0;

    // get the histos
    TH1** hOut = GetHistos(fFiles[index], hpatt, nhistos);

    if (!hOut) return 0;

    // loop over histos
    for (Int_t i = 0; i < nhistos; i++)
    {
        // set histogram name
        if (houtnamepatt)
        {
            // set user name
            SetHistoName(hOut[i], houtnamepatt, index);
        }
        else
        {
            // set default name
            Char_t tmp[256];
            sprintf(tmp, "%s_%i", hOut[i]->GetName(), fRuns[index]);
            hOut[i]->SetName(tmp);
        }

        // set directory
        if (TH1::AddDirectoryStatus())
            hOut[i]->SetDirectory(fHistoDirectory);
    }

    return hOut;
}

//______________________________________________________________________________
TH1* TCARHistoLoader::CreateHistoSum(const Char_t* hname, const Char_t* houtnamepatt /*= 0*/)
{
    // Creates the summed-up histogram with name 'hname'.
    // NOTE: the histogram has to be destroyed by the caller.

    // load files first (if not already loaded)
    if (!LoadFiles()) return 0;

    // init pointer to sum histo
    TH1* hSum = 0;

    // loop over files
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // get histogram detached
        Bool_t status = TH1::AddDirectoryStatus();
        TH1::AddDirectory(kFALSE);
        TH1* h = GetHistoForIndex(hname, i);
        TH1::AddDirectory(status);

        // check for histo
        if (!h) continue;

        // sum up histos
        if (!hSum) // AddDir???
            hSum = (TH1*) h->Clone();
        else
            hSum->Add(h);

        // delete histo
        delete h;
    }

    // set histogram name
    if (houtnamepatt)
    {
        // set user name
        SetHistoName(hSum, houtnamepatt, -1);
    }
    else
    {
        // set default name
        Char_t tmp[256];
        sprintf(tmp, "%s_Sum", hSum->GetName());
        hSum->SetName(tmp);
    }

    // set directory
    if (TH1::AddDirectoryStatus())
        hSum->SetDirectory(fHistoDirectory);

    return hSum;
}

//______________________________________________________________________________
TH1** TCARHistoLoader::CreateHistoSumArray(const Char_t* hpatt, Int_t& nhistos, const Char_t* houtnamepatt /*= 0*/)
{
    // Returns an array of histogram pointers (cf. description of function
    // 'CreateHistoArray(const Char_t*)' below).

    // load files first (if not already loaded)
    if (!LoadFiles()) return 0;

    // init pointer to sum histo
    TH1** hSum = 0;

    // loop over files
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // number of histos
        Int_t n = 0;

        // get histogram detached
        TH1** h = GetHistosForIndex(hpatt, i, n, houtnamepatt);

        // check for histo
        if (!h) continue;

        // sum up histo arrays
        if (!hSum) // AddDir???
        {
            hSum = h;
            nhistos = n;
            continue;
        }

        // check number of histos
        if (n != nhistos)
        {
            Warning("CreateHistoSumArray", "Number of histograms does not match!");
        }
        else
        {
            // loop over single histos of array
            for (Int_t j = 0; j < n; j++)
            {
                hSum[j]->Add(h[j]);
                delete h[j];
            }
        }

        // delete histo
        delete h;

        printf("Added %d/%d\n", i, fNRuns);
    }

    return hSum;
}

//______________________________________________________________________________
TH1** TCARHistoLoader::CreateHistoArray(const Char_t* hname, const Char_t* houtnamepatt /*= 0*/)
{
    // Creates an array of histogram pointers of length 'fNRuns'. The i-th array
    // element points to the histogram with name 'hname' loaded from the file
    // 'fFiles[i]' (i.e., the AR file of the run with run number 'fRuns[i]').
    // The individual histogram names are suffixed with an underscore followed
    // by the associated the run number.
    // If the histogram cannot be found for some file, the NULL pointer is set
    // for the corresponding array element. If no histogram is found the NULL
    // pointer is returned for the output histogram array.
    // NOTE: the array (incl. histograms) has to be destroyed by the caller.

    // load files first (if not already loaded)
    if (!LoadFiles()) return 0;

    // create histogram array
    TH1** hOut = new TH1*[fNRuns];

    // init histo found flag
    Bool_t isFound = kFALSE;

    // loop over files
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // init pointer to histo
        hOut[i] = 0;

        // get histogram
        TH1* h = GetHistoForIndex(hname, i, houtnamepatt); //??? pat ???

        // check for histo
        if (!h) continue;

        // update found flag
        isFound = kTRUE;

        // add to list
        hOut[i] = h;
    }

    // reset output array if no histogram was loaded
    if (!isFound)
    {
        delete [] hOut;
        hOut = 0;
    }

    return hOut;
}

//______________________________________________________________________________
TH1** TCARHistoLoader::CreateHistoArrayOfProj(const Char_t* hname, const Char_t projaxis /*= 'X'*/,
                                              Int_t fbin1 /*= 1*/, Int_t lbin1 /*= kLastBin*/,
                                              Int_t fbin2 /*= 1*/, Int_t lbin2 /*= kLastBin*/,
                                              Option_t* option /*= ""*/, const Char_t* houtnamepatt /*= 0*/)
{
    // Creates an array of histogram pointers of length 'fNRuns'. The i-th array
    // element is the projection on the axis 'projaxis' of the histogram with
    // name 'hname' loaded from the file 'fFiles[i]' (i.e., the AR file of the
    // run with run number 'fRuns[i]').
    // If the histogram cannot be found for some file, the NULL pointer is set
    // for the corresponding array element. If no histogram is found the NULL
    // pointer is returned for the output histogram array.
    // NOTE: the array (incl. histograms) has to be destroyed by the caller.

    // relative weight flag
    Bool_t relW = kFALSE;

    TString opt  = option;
    opt.ToLower();
    Int_t idx = opt.Index("w");
    if (idx >= 0)
    {
        relW = kTRUE;
        opt.Remove(idx, 9);
    }

    // init projection axis flags
    Bool_t isX = kFALSE;
    Bool_t isY = kFALSE;
    Bool_t isZ = kFALSE;

    // process projection axis arument
    if      (projaxis == 'x' || projaxis == 'X') isX = kTRUE;
    else if (projaxis == 'y' || projaxis == 'Y') isY = kTRUE;
    else if (projaxis == 'z' || projaxis == 'Z') isZ = kTRUE;
    else
    {
        Error("CreateHistoArrayOfProj", "'%c' is not a valid axis!", projaxis);
        return 0;
    }

    // load files first (if not already loaded)
    if (!LoadFiles()) return 0;

    // create histogram array
    TH1** hOut = new TH1*[fNRuns];

    // init histo found flag
    Bool_t isFound = kFALSE;

    // loop over files
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // init pointer to histo
        hOut[i] = 0;

        // get histogram detached
        Bool_t status = TH1::AddDirectoryStatus();
        TH1::AddDirectory(kFALSE);
        TH1* h = GetHistoForIndex(hname, i);
        TH1::AddDirectory(status);

        // check for histo
        if (!h) continue;

        // declare projection histogram
        TH1* hp = 0;

        // create projection histogram name
        Char_t hpname[256];

        // prepare histogram name
        if (houtnamepatt)
        {
            // user defined name
            SetHistoName(h, houtnamepatt, i);
            strcpy(hpname, h->GetName());
        }
        else
        {
            // standard name, i.e. "<histoname>_<runnumber>_p<axis>"
            sprintf(hpname, "%s_%d", h->GetName(), GetRuns()[i]);
            if(isX) strcat(hpname, "_px");
            if(isY) strcat(hpname, "_py");
            if(isZ) strcat(hpname, "_pz");
        }

        // project histogram
        status = TH1::AddDirectoryStatus();
        TH1::AddDirectory(kFALSE);
        if (h->GetDimension() == 1)
        {
            // no projection needed
            if (isX)
            {
                hp = (TH1*) h;
                hp->SetName(hpname);
            }
            else
                Error("CreateHistoArrayOfProj", "Cannot project 1D histogram to %c-axis.", projaxis);
        }
        else if (h->GetDimension() == 2)
        {
            // set last bin
            Int_t lastbin1 = lbin1;
            if (lbin1 == kLastBin)
            {
                if (isX) lastbin1 = ((TH2*) h)->GetNbinsY();
                if (isY) lastbin1 = ((TH2*) h)->GetNbinsX();
            }

            // project
            if (isX || isY)
            {
                if (relW)
                {
                    hp = CreateProjection_RelWeight(projaxis, h);
                    hp->SetName(hpname);
                }
                else
                {
                    if      (isX) hp = (TH1D*) ((TH2*) h)->ProjectionX(hpname, fbin1, lastbin1, opt.Data());
                    else if (isY) hp = (TH1D*) ((TH2*) h)->ProjectionY(hpname, fbin1, lastbin1, opt.Data());
                }
            }
            else
                Error("CreateHistoArrayOfProj", "Cannot project 2D histogram to z-axis.");
        }
        else if (h->GetDimension() == 3)
        {
            // set last bin1
            Int_t lastbin1 = lbin1;
            if (lbin1 == kLastBin)
            {
                if (isX) lastbin1 = ((TH2*) h)->GetNbinsY();
                if (isY) lastbin1 = ((TH2*) h)->GetNbinsX();
                if (isZ) lastbin1 = ((TH2*) h)->GetNbinsX();
            }

            // set last bin2
            Int_t lastbin2 = lbin2;
            if (lbin2 == kLastBin)
            {
                if (isX) lastbin2 = ((TH2*) h)->GetNbinsZ();
                if (isY) lastbin2 = ((TH2*) h)->GetNbinsZ();
                if (isZ) lastbin2 = ((TH2*) h)->GetNbinsY();
            }

            // project
            if (relW)
            {
                hp = CreateProjection_RelWeight(projaxis, h);
                hp->SetName(hpname);
            }
            else
            {
                if (isX) hp = (TH1D*) ((TH3*) h)->ProjectionX(hpname, fbin1, lastbin1, fbin2, lastbin2, opt.Data());
                if (isY) hp = (TH1D*) ((TH3*) h)->ProjectionY(hpname, fbin1, lastbin1, fbin2, lastbin2, opt.Data());
                if (isZ) hp = (TH1D*) ((TH3*) h)->ProjectionZ(hpname, fbin1, lastbin1, fbin2, lastbin2, opt.Data());
            }
        } // end if dimension 1,2 or 3

        TH1::AddDirectory(status);

        // set directory
        if (TH1::AddDirectoryStatus())
            hp->SetDirectory(fHistoDirectory);

        // update found flag
        if (hp) isFound = kTRUE;

        // add to list
        if (hp) hOut[i] = hp;

        // clean up (except for ProjectionX of 1D histogramm)
        if (h != hp) delete h;

    } // end loop over runs

    // reset output array if no histogram was loaded
    if (!isFound)
    {
        delete [] hOut;
        hOut = 0;
    }

    return hOut;
}

//______________________________________________________________________________
TH2D* TCARHistoLoader::CreateHistoOfProj(const Char_t* hname, const Char_t projaxis /*= 'X'*/, const Char_t runaxis /* = 'X'*/,
                                         Int_t fbin1 /*= 1*/, Int_t lbin1 /*= kLastBin*/,
                                         Int_t fbin2 /*= 1*/, Int_t lbin2 /*= kLastBin*/,
                                         Option_t* option /*= ""*/)
{
    // Creates a TH2D histogram with 'fNRuns' y-bins. Its i-th y-slice is filled
    // with the projection on the axis 'projaxis' of the histogram named 'hname'
    // from the file 'fFiles[i]' (i.e., the AR file of the run with run number
    // 'fRuns[i]').
    // NOTE: the histogram has to be destroyed by the caller.

    // relative weight flag
    Bool_t relW = kFALSE;

    TString opt  = option;
    opt.ToLower();
    Int_t idx = opt.Index("w");
    if (idx >= 0)
    {
        relW = kTRUE;
        opt.Remove(idx, 9);
    }

    // init run index axis
    Bool_t runsOnX = kTRUE;

    // init projection axis flags
    Bool_t isX = kFALSE;
    Bool_t isY = kFALSE;
    Bool_t isZ = kFALSE;

    // process run axis argument
    if      (runaxis == 'X' || runaxis == 'x') runsOnX = kTRUE;
    else if (runaxis == 'Y' || runaxis == 'y') runsOnX = kFALSE;
    else
    {
        Error("CreateHistoOfProj", "'%c' is not a valid axis!", runaxis);
        return 0;
    }

    // process projection axis arument
    if      (projaxis == 'x' || projaxis == 'X') isX = kTRUE;
    else if (projaxis == 'y' || projaxis == 'Y') isY = kTRUE;
    else if (projaxis == 'z' || projaxis == 'Z') isZ = kTRUE;
    else
    {
        Error("CreateHistoOfProj", "'%c' is not a valid axis!", projaxis);
        return 0;
    }

    // get list of files first
    if (!LoadFiles()) return 0;

    // declare out histo
    TH2D* hOut = 0;

    // declare reference histo
    TH1* href = 0;

    // loop over files
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // check for file
        if (!fFiles[i]) continue;

        // get histogram detached
        Bool_t status = TH1::AddDirectoryStatus();
        TH1::AddDirectory(kFALSE);
        TH1* h = GetHistoForIndex(hname, i, "#NAME");
        TH1::AddDirectory(status);

        // check for histogram
        if (!h)
        {
            Error("CreateHistoOfProj", "Histogram '%s' was not found in file '%s'!",
                                      hname, fFiles[i]->GetName());
            continue;
        }

        // check whether object is a histogram
        if (!h->InheritsFrom("TH1"))
        {
            Error("CreateHistoOfProj", "Object named '%s' of file '%s' is not a histogram!",
                                       hname, fFiles[i]->GetName());

            // delete h form memory
            h->ResetBit(kMustCleanup);
            delete h;

            continue;
        }

        // check whether reference histo was already set (cf. end of loop)
        if (!href)
        {
            // check histo dimension vs. projection axis compatibility
            if (isY && h->GetDimension() < 2)
            {
                Error("CreateHistoOfProj", "Cannot project on y-axis in a 1-dim. histogram!");

                // delete h form memory
                h->ResetBit(kMustCleanup);
                delete h;

                return 0;
            }
            else if (isZ && h->GetDimension() < 3)
            {
                Error("CreateHistoOfProj", "Cannot project on z-axis in a 1- or 2-dim. histogram!");

                // delete h form memory
                h->ResetBit(kMustCleanup);
                delete h;

                return 0;
            }
        }
        /*
        else
        {
            // check consistency
            // ... to be done. Unfortunately the CheckConsistency() member function of TH1 is protected.
            // For the moment its the user's responsibility
        }
        */

        // declare projection histogram
        TH1* hp = 0;

        // project histogram (detached)
        TH1::AddDirectory(kFALSE);

        if (h->GetDimension() == 1)
        {
            // no projection needed
            hp = (TH1*) h;
        }
        else if (h->GetDimension() == 2)
        {
            if (relW)
                hp = CreateProjection_RelWeight(projaxis, h);
            else
            {
                if (isX) hp = (TH1D*) ((TH2*) h)->ProjectionX("p", fbin1, lbin1, opt.Data());
                if (isY) hp = (TH1D*) ((TH2*) h)->ProjectionY("p", fbin1, lbin1, opt.Data());
            }
        }
        else if (h->GetDimension() == 3)
        {
            if (relW)
                hp = CreateProjection_RelWeight(projaxis, h);
            else
            {
                if (isX) hp = (TH1D*) ((TH3*) h)->ProjectionX("p", fbin1, lbin1, fbin2, lbin2, opt.Data());
                if (isY) hp = (TH1D*) ((TH3*) h)->ProjectionY("p", fbin1, lbin1, fbin2, lbin2, opt.Data());
                if (isZ) hp = (TH1D*) ((TH3*) h)->ProjectionZ("p", fbin1, lbin1, fbin2, lbin2, opt.Data());
            }
        }

        TH1::AddDirectory(status);

        // create output histogram (if not created yet)
        if (!hOut)
        {
            // improve!!! (assumes standard binning)

            // get name
            Char_t name[256];
            sprintf(name, "%s_p%c", h->GetName(), projaxis);

            // get projection axis title
            const Char_t* axistitle = 0;
            if (isX) axistitle = h->GetXaxis()->GetTitle();
            if (isY) axistitle = h->GetYaxis()->GetTitle();
            if (isZ) axistitle = h->GetZaxis()->GetTitle();

            Char_t newtitle[256];
            if (runsOnX)
            {
                sprintf(newtitle, "%s;Run index;%s", name, axistitle);
                hOut = new TH2D(name, newtitle, fNRuns, 0, fNRuns, hp->GetNbinsX(), hp->GetBinLowEdge(1), hp->GetBinLowEdge(hp->GetNbinsX() + 1));
            }
            else
            {
                sprintf(newtitle, "%s;%s;Run index", name, axistitle);
                hOut = new TH2D(name, newtitle, hp->GetNbinsX(), hp->GetBinLowEdge(1), hp->GetBinLowEdge(hp->GetNbinsX() + 1), fNRuns, 0, fNRuns);
            }
        }

        // loop over bins
        Int_t nbins = runsOnX ? hOut->GetNbinsY() : hOut->GetNbinsX();
        for (Int_t j = 0; j < nbins; j++)
        {
            // add bin content and bin error
            if (runsOnX)
            {
                hOut->SetBinContent(i+1, j+1, hp->GetBinContent(j+1));
                hOut->SetBinError(i+1, j+1, hp->GetBinError(j+1));
            }
            else
            {
                hOut->SetBinContent(j+1, i+1, hp->GetBinContent(j+1));
                hOut->SetBinError(j+1, i+1, hp->GetBinError(j+1));
            }
        }

        // clean up
        if (h->GetDimension() > 1)
            if (hp) delete hp;

        // set pointer to reference histo
        if (!href) href = (TH1*) h->Clone("href");

        // delete h form memory
        h->ResetBit(kMustCleanup);
        delete h;

    } // loop over runs

    // clean up
    if (href) delete href;

    // set directory
    if (TH1::AddDirectoryStatus())
        hOut->SetDirectory(fHistoDirectory);

    return hOut;
}

//______________________________________________________________________________
TH1* TCARHistoLoader::CreateProjection_RelWeight(Char_t projaxis, const TH1* h)
{
    // WARNING: This function is still under development. Behaviour and
    //          signature might change in future!
    //
    // Creates a weighted projection of the histogramm 'h' on the axis 'projaxis'.
    // The inverse weight of the bin (i, j, k) projected to the K-axis is
    //   i_w[i][j] = sum_k BC(i,j,k)/mean
    // where BC ist the bin content and
    //   mean = sum_i,j BC(i,j,k) / (nbins_I*nbinx_K)

    // check histo
    if (!h) return 0;

    // init projection axis flags
    Bool_t onX = kFALSE;
    Bool_t onY = kFALSE;
    Bool_t onZ = kFALSE;

    // check projection axis arument
    if      (projaxis == 'x' || projaxis == 'X') onX = kTRUE;
    else if (projaxis == 'y' || projaxis == 'Y') onY = kTRUE;
    else if (projaxis == 'z' || projaxis == 'Z') onZ = kTRUE;
    else
    {
        Error("CreateProjection_RelWeight", "'%c' is not a valid axis!", projaxis);
        return 0;
    }

    // get number of bins & axis range
    Int_t nbinsI, nbinsJ, nbinsO;
    Double_t pxmin, pxmax;
    if (onX)
    {
        nbinsO = h->GetNbinsX();
        nbinsI = h->GetNbinsY();
        nbinsJ = h->GetNbinsZ();
        pxmin = h->GetXaxis()->GetBinLowEdge(1);
        pxmax = h->GetXaxis()->GetBinUpEdge(nbinsO);
    }
    else if (onY)
    {
        nbinsI = h->GetNbinsX();
        nbinsO = h->GetNbinsY();
        nbinsJ = h->GetNbinsZ();
        pxmin = h->GetYaxis()->GetBinLowEdge(1);
        pxmax = h->GetYaxis()->GetBinUpEdge(nbinsO);
    }
    else
    {
        nbinsI = h->GetNbinsX();
        nbinsJ = h->GetNbinsY();
        nbinsO = h->GetNbinsZ();
        pxmin = h->GetZaxis()->GetBinLowEdge(1);
        pxmax = h->GetZaxis()->GetBinUpEdge(nbinsO);
    }

    // create histo
    Char_t name[256];
    sprintf(name, "%s_p%c", h->GetName(), projaxis);
    Char_t title[256];
    if (onX) sprintf(title, "%s;%s", name, h->GetXaxis()->GetTitle());
    if (onY) sprintf(title, "%s;%s", name, h->GetYaxis()->GetTitle());
    if (onZ) sprintf(title, "%s;%s", name, h->GetZaxis()->GetTitle());
    TH1* hout = new TH1D(name, title, nbinsO, pxmin, pxmax);
    hout->Sumw2();

    // calculate inverse weights (include under/overflow)
    Double_t w[nbinsI+2][nbinsJ+2];
    for (Int_t i = 0; i <= nbinsI+1; i++)
    {
        for (Int_t j = 0; j <= nbinsJ+1; j++)
        {
            w[i][j] = 0;

            for (Int_t o = 0; o <= nbinsO+1; o++)
            {
                Int_t bin;
                if      (onX) bin = h->GetBin(o,i,j);
                else if (onY) bin = h->GetBin(i,o,j);
                else          bin = h->GetBin(i,j,o);
                w[i][j] += h->GetBinContent(bin);
            }
        }
    }

    // calculate mean value (exclude under/overflow)
    Double_t mean = 0;
    for (Int_t i = 1; i <= nbinsI; i++)
    {
        for (Int_t j = 1; j <= nbinsJ; j++)
        {
            mean += w[i][j];
        }
    }
    mean /= nbinsI*nbinsJ;

    // calculate inverse weights (include under/overflow)
    for (Int_t i = 0; i <= nbinsI+1; i++)
    {
        for (Int_t j = 0; j <= nbinsJ+1; j++)
        {
            w[i][j] = w[i][j]/mean;
        }
    }

    // loop over proj axis
    for (Int_t o = 0; o <= nbinsO+1; o++)
    {
        // init bin content and error
        Double_t cont = 0;
        Double_t err2 = 0;

        // loop over first axis
        for (Int_t i = 0; i <= nbinsI+1; i++)
        {
            // loop over second axis
            for (Int_t j = 0; j <= nbinsJ+1; j++)
            {
                // get global bin
                Int_t bin;
                if      (onX) bin = h->GetBin(o,i,j);
                else if (onY) bin = h->GetBin(i,o,j);
                else          bin = h->GetBin(i,j,o);

                // helper
                Double_t inv_weight = w[i][j];

                // get bin content and error
                if (inv_weight)
                {
                    cont += h->GetBinContent(bin) / inv_weight;
                    Double_t err = h->GetBinError(bin) / inv_weight;
                    err2 += err*err;
                }
            }
        }
        hout->SetBinContent(o, cont);
        hout->SetBinError(o, TMath::Sqrt(err2));
    }

    // return
    return hout;
}

// finito

