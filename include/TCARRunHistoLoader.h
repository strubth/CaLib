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


#ifndef TCARRUNHISTOLOADER_H 
#define TCARRUNHISTOLOADER_H

#include "TCARHistoLoader.h"


class TCARRunHistoLoader : public TCARHistoLoader
{

public:
    TCARRunHistoLoader()
      : TCARHistoLoader() { };
    TCARRunHistoLoader(Int_t nruns, const Int_t* runs, const Char_t* inputfilepathpatt = 0)
      : TCARHistoLoader(nruns, runs, inputfilepathpatt) { };
    virtual ~TCARRunHistoLoader() { };

    TH1** CreateHistoArray(const Char_t* hname, Int_t& nhistos);
    TH1** CreateHistoArray(const Char_t* hname);
    TH2*  CreateHistoOfProj(const Char_t* hname, const Char_t projaxis = 'X'); 

    ClassDef(TCARRunHistoLoader, 0) // AR Histogram loading class (run by run)
};

#endif

