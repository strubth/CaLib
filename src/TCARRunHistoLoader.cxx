/*************************************************************************
 * Author: Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCARRunHistoLoader                                                   //
//                                                                      //
// AR Histogram loading class for run by run calibration.               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCARRunHistoLoader.h"

ClassImp(TCARRunHistoLoader)


//______________________________________________________________________________
TH1** TCARRunHistoLoader::CreateHistoArray(const Char_t* hname, Int_t& nhistos)
{
   // Returns an array of histogram pointers (cf descr. below).

   // create the histogram array
   TH1** h = CreateHistoArray(hname);

   // pass length of array
   nhistos = 0;
   if (h) nhistos = fNRuns;

   // return array
   return h;
}

//______________________________________________________________________________
TH1** TCARRunHistoLoader::CreateHistoArray(const Char_t* hname)
{
    // Returns an array of histogram pointers of length 'fNRuns'. The i-th array
    // element points to the histogram with name 'hname' loaded from the file
    // 'fFiles[i]' (AR file of the run 'fRuns[i]'). The name is changed to
    // 'hname_fRuns[i]'.
    // If the histogram cannot be found for some file, the NULL pointer is set
    // for the corresponding array element.
    // If ...
    // NOTE: the array (incl. histograms) has to be destroyed by the caller.

    // create file list first
    if (!fFiles && !CreateFileList()) return 0;

    // create histogram array
    TH1** hOut =  new TH1*[fNRuns];

    // init flag
    Bool_t ische_nitte = kTRUE;

    // loop over files
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // init pointer to histo
        hOut[i] = 0;

        // check for file
        if (!fFiles[i]) continue;

        // get histogram
        TH1* h = (TH1*) fFiles[i]->Get(hname);

        // check if histogram is there
        if (!h)
        {
            Warning("GetHistogram", "Histogram '%s' was not found in file '%s'",
                                    hname, fFiles[i]->GetName());
            continue;
        }

        // check if object is really a histogram
        if (!h->InheritsFrom("TH1"))
        {
            Error("GetHistogram", "Object '%s' found in file '%s' is not a histogram!",
                                  hname, fFiles[i]->GetName());
            continue;
        }

        ische_nitte = kFALSE;

        // detach from file
        h->SetDirectory(0);

        // set name
        Char_t tmp[128];
        sprintf(tmp, "%s_%i", hname, fRuns[i]);
        h->SetName(tmp);

        // add to list
        hOut[i] = h;
    }

    // reset output array if no histo loaded
    if (ische_nitte)
    {
        delete [] hOut;
        hOut = 0;
    }

    return hOut;
}

//______________________________________________________________________________
TH2* TCARRunHistoLoader::CreateHistoOfProj(const Char_t* hname, const Char_t projaxis)
{
    // WARNING: Not tested yet!!!

    // init projection axis flag
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
    if (!fFiles && !CreateFileList()) return 0;

    // declare out histo
    TH2D* hOut = 0;

    // declare reference histo
    TH1* href = 0;

    // loop over runs
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // check file pointer (CreateFileList() already reports an error on that)
        if (!fFiles[i]) continue;

        // get histogram of this run
        TH1* h = (TH1*) fFiles[i]->Get(hname);

        // check for histogram pointer
        if (!h)
        {
            Error("CreateHistoOfProj", "Run %i does not have a histogram named '%s'!", fRuns[i], hname);
            continue;
        }

        // check whether its a histogram
        if (!h->InheritsFrom("TH1"))
        {
            Error("CreateHistoOfProj", "Object named '%s' of run %i is not a histogramm!", hname, fRuns[i]);
            delete h;
            continue;
        }

        // check whether reference histo was already set (cf. end of loop)
        if (!href)
        {
            // check histo dimension vs. projection axis compatibility
            if (isY && h->GetDimension() < 2)
            {
                Error("CreateHistoOfProj", "Cannot project on y-axis in a 1-dim histogram!");
                continue;
            }
            if (isZ && h->GetDimension() < 3)
            {
                Error("CreateHistoOfProj", "Cannot project on z-axis in a 2-dim histogram!");
                continue;
            }
        }
        else
        {
            // check consistency
            // ... to be done. Unfortunately the CheckConsistency() member function of TH1 is protected.
            // For the moment its the user's responsibility
        }

        // declare projection
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

        // create output histo if not created yet
        if (!hOut)
        {
            // improve!!! (assumes standard binning)
            hOut = new TH2D("xxx","xxx", hp->GetNbinsX(), hp->GetBinLowEdge(1), hp->GetBinLowEdge(hp->GetNbinsX() + 1), fNRuns, 0, fNRuns);
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
        if (!href) href = h;

    } // loop over runs

    // clean up
    if (href) delete href;

    return hOut;
}

