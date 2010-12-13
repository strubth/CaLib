/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iReadConfig                                                          //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef IREADCONFIG_HH
#define IREADCONFIG_HH

#include "stdio.h"

#include <iostream>
#include <fstream>

#include <TSystem.h>
#include <TObject.h>
#include <TString.h>

#include "iConfig.hh"

using namespace std;

const int MAX_LINE = 1000;

class iReadConfig
{

private:
    Int_t   nLine;
    TString strLine[MAX_LINE];
    TString strCaLibPath;

    void ReadConfigFile(const Char_t*);

public:
    iReadConfig();
    iReadConfig(Char_t*);
    virtual ~iReadConfig();

    TString ExtractName(TString);

    TString GetConfigName(TString);
    TString GetConfigCuts(TString);
    TString GetConfigNamePart(TString, Int_t&);

    ClassDef(iReadConfig, 0)   // Read config/config.cfg file for CaLib
};

#endif

