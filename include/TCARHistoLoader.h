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


#ifndef TCARHISTOLOADER_H
#define TCARHISTOLOADER_H

#include "TCARFileLoader.h"

class TH1;
class TH2D;

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
    TH2D* CreateHistoOfProj(const Char_t* hname, const Char_t projaxis = 'X');

    ClassDef(TCARHistoLoader, 0) // AR histogram loading class
};

#endif

