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
//   2)   Load summed up histograms, e.g.,
//          TH1* h = hl.CreateHistoSum("MyHistogram");
//
//   3.1) Load array of histograms, e.g.,
//          TH1** hs = hl.CreateHistoArray("MyHistogram");
//        Now, hs an array of length hl.GetNRuns(), where the i-th element is the
//        histogram of the i-th file with the run with runnumber hl.GetRuns()[i].
//
//   3.2) Load array of projection of histos.
//          TH1** hsp1 hl.CreateHistoArrayOfProj("MyHistogram");
//          TH1** hsp2 hl.CreateHistoArrayOfProj("MyHistogram", 'Y', 23, kLastBin);
//
//   4) ...
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
        if (!strcmp(hname, key->GetName()) == 0)
            continue;

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
        TH1* h = GetHistoForIndex(hname, i, kFALSE);
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
        TH1* h = GetHistoForIndex(hname, i, kFALSE);
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
            sprintf(hpname, h->GetName());
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

            if      (isX) hp = (TH1D*) ((TH2*) h)->ProjectionX(hpname, fbin1, lastbin1, option);
            else if (isY) hp = (TH1D*) ((TH2*) h)->ProjectionY(hpname, fbin1, lastbin1, option);
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

            // set last bin1
            Int_t lastbin2 = lbin2;
            if (lbin2 == kLastBin)
            {
                if (isX) lastbin2 = ((TH2*) h)->GetNbinsZ();
                if (isY) lastbin2 = ((TH2*) h)->GetNbinsZ();
                if (isZ) lastbin2 = ((TH2*) h)->GetNbinsY();
            }

            if (isX) hp = (TH1D*) ((TH3*) h)->ProjectionX(hpname, fbin1, lastbin1, fbin2, lastbin2, option);
            if (isY) hp = (TH1D*) ((TH3*) h)->ProjectionY(hpname, fbin1, lastbin1, fbin2, lastbin2, option);
            if (isZ) hp = (TH1D*) ((TH3*) h)->ProjectionZ(hpname, fbin1, lastbin1, fbin2, lastbin2, option);
        }

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
TH2D* TCARHistoLoader::CreateHistoOfProj(const Char_t* hname, const Char_t projaxis /*= 'X'*/,
                                         Int_t fbin1 /*= 1*/, Int_t lbin1 /*= kLastBin*/,
                                         Int_t fbin2 /*= 1*/, Int_t lbin2 /*= kLastBin*/,
                                         Option_t* option /*= ""*/)
{
    // Creates a TH2D histogram with 'fNRuns' y-bins. Its i-th y-slice is filled
    // with the projection on the axis 'projaxis' of the histogram named 'hname'
    // from the file 'fFiles[i]' (i.e., the AR file of the run with run number
    // 'fRuns[i]').
    // NOTE: the histogram has to be destroyed by the caller.

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

        // get histogram
        TH1* h = (TH1*) fFiles[i]->Get(hname);

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

        // project histogram
        if (h->GetDimension() == 1)
        {
            // no projection needed
            hp = (TH1*) h;
        }
        else if (h->GetDimension() == 2)
        {
            if (isX) hp = (TH1D*) ((TH2*) h)->ProjectionX("pproj", 1, -1, "e");
            if (isY) hp = (TH1D*) ((TH2*) h)->ProjectionY("pproj", 1, -1, "e");
        }
        else if (h->GetDimension() == 3)
        {
            if (isX) hp = (TH1D*) ((TH3*) h)->ProjectionX("pproj", 1, -1, 1, -1, "e");
            if (isY) hp = (TH1D*) ((TH3*) h)->ProjectionY("pproj", 1, -1, 1, -1, "e");
            if (isZ) hp = (TH1D*) ((TH3*) h)->ProjectionZ("pproj", 1, -1, 1, -1, "e");
        }

        // create output histogram (if not created yet)
        if (!hOut)
        {
            // improve!!! (assumes standard binning)

            // get name
            Char_t name[256];
            sprintf(name, "%s", h->GetName());

            // get projection axis title
            Char_t axistitle[256];
            if (isX) sprintf(axistitle, h->GetXaxis()->GetTitle());
            if (isY) sprintf(axistitle, h->GetYaxis()->GetTitle());
            if (isZ) sprintf(axistitle, h->GetZaxis()->GetTitle());

            Char_t newtitle[256];
            sprintf(newtitle, "%s;%s;Run index", h->GetTitle(), axistitle);
            hOut = new TH2D(name, newtitle, hp->GetNbinsX(), hp->GetBinLowEdge(1), hp->GetBinLowEdge(hp->GetNbinsX() + 1), fNRuns, 0, fNRuns);
        }

        // loop over bins
        for (Int_t j = 0; j < hOut->GetNbinsX(); j++)
        {
            // add bin content and bin error
            hOut->SetBinContent(j+1, i+1, hp->GetBinContent(j+1));
            hOut->SetBinError(j+1, i+1, hp->GetBinError(j+1));
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

    return hOut;
}

// finito

