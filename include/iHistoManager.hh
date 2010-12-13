/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iHistoManager                                                        //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef IHISTOMANAGER_HH
#define IHISTOMANAGER_HH

#include "stdio.h"

#include <iostream>
#include <fstream>

#include <TObject.h>
#include "TROOT.h"
#include "TSystem.h"
#include <TString.h>
#include <TFile.h>

#include "iConfig.hh"
#include "iReadConfig.hh"


using namespace std;


class iHistoManager
    : public virtual iReadConfig
{

private:
    TFile* fHistoFile[21];
    
    virtual void Init();

public:
    iHistoManager();
    virtual ~iHistoManager();

    ClassDef(iHistoManager, 0)
};

#endif

