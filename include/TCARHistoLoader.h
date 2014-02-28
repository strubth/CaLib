/*************************************************************************
 * Author: Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCARHistoLoader                                                      //
//                                                                      //
// AR Histogram loading class for run by run calibration.               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCARHISTOLOADER_H 
#define TCARHISTOLOADER_H

#include "TCARFileLoader.h"
#include "TH2.h"
#include "TH3.h"


class TCARHistoLoader : public TCARFileLoader
{

public:
    TCARHistoLoader()
      : TCARFileLoader() { };
    TCARHistoLoader(Int_t nruns, const Int_t* runs, const Char_t* inputfilepathpatt = 0)
      : TCARFileLoader(nruns, runs, inputfilepathpatt) { };
    virtual ~TCARHistoLoader() { };

    TH1** CreateHistoArray(const Char_t* hname, Int_t& nhistos);
    TH1** CreateHistoArray(const Char_t* hname);
    TH2*  CreateHistoOfProj(const Char_t* hname, const Char_t projaxis = 'X'); 

    ClassDef(TCARHistoLoader, 0) // AR Histogram loading class (run by run)
};

#endif

