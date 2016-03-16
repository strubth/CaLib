/************************************************************************
 * Author: Thomas Strub                                                 *
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCARHistoLoader                                                      //
//                                                                      //
// AR histogram loading class.                                          //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TFile.h"
#include "TH2.h"
#include "TH3.h"
#include "TError.h"

#include "TCARHistoLoader.h"

ClassImp(TCARHistoLoader)

//______________________________________________________________________________
TH1** TCARHistoLoader::CreateHistoArray(const Char_t* hname, Int_t& nhistos)
{
   // Returns an array of histogram pointers (cf. description of function
   // 'CreateHistoArray(const Char_t*)' below).

   // create the histogram array
   TH1** h = CreateHistoArray(hname);

   // pass length of array
   nhistos = 0;
   if (h) nhistos = fNRuns;

   // return array
   return h;
}

//______________________________________________________________________________
TH1** TCARHistoLoader::CreateHistoArray(const Char_t* hname)
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
    TH1** hOut =  new TH1*[fNRuns];

    // init histo found flag
    Bool_t isFound = kFALSE;

    // loop over files
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // init pointer to histo
        hOut[i] = 0;

        // check for file
        if (!fFiles[i]) continue;

        // get histogram
        TH1* h = (TH1*) fFiles[i]->Get(hname);

        // check for histogram
        if (!h)
        {
            Error("CreateHistoArray", "Histogram '%s' was not found in file '%s'!",
                                      hname, fFiles[i]->GetName());
            continue;
        }

        // check whether object is a histogram
        if (!h->InheritsFrom("TH1"))
        {
            Error("CreateHistoOfProj", "Object named '%s' of file '%s' is not a histogram!",
                                       hname, fFiles[i]->GetName());
            continue;
        }

        // update found flag
        isFound = kTRUE;

        // detach histogram from file
        h->SetDirectory(0);

        // set histogram name
        Char_t tmp[256];
        sprintf(tmp, "%s_%i", hname, fRuns[i]);
        h->SetName(tmp);

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
TH2D* TCARHistoLoader::CreateHistoOfProj(const Char_t* hname, const Char_t projaxis)
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
            if (isX) strcpy(axistitle, h->GetXaxis()->GetTitle());
            if (isY) strcpy(axistitle, h->GetYaxis()->GetTitle());
            if (isZ) strcpy(axistitle, h->GetZaxis()->GetTitle());

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

