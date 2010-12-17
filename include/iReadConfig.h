/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iReadConfig                                                          //
//                                                                      //
// Read CaLib configuration files.                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef IREADCONFIG_H
#define IREADCONFIG_H

#include <fstream>

#include "TSystem.h"
#include "TString.h"
#include "THashTable.h"
#include "TError.h"


class iConfigElement : public TObject
{

private:
    TString key;                // config key
    TString value;              // config value

public:
    iConfigElement(const Char_t* k, const Char_t* v) : key(k), value(v) {  } 
    virtual ~iConfigElement() { }
    TString* GetKey() { return &key; }
    TString* GetValue() { return &value; }
    virtual const Char_t* GetName() const { return key.Data(); }
    virtual ULong_t Hash() const { return key.Hash(); }
};


class iReadConfig
{

private:
    THashTable* fConfigTable;       // hash table containing config elements
    TString fCaLibPath;             // path of the calib source

    void ReadConfigFile(const Char_t* cfgFile);
    iConfigElement* CreateConfigElement(TString line);
    
    static iReadConfig* fgReadConfig;

public:
    iReadConfig();
    iReadConfig(Char_t* cfgFile);
    virtual ~iReadConfig();

    TString* GetConfig(TString configKey);
    Int_t GetConfigInt(TString configKey);
    Double_t GetConfigDouble(TString configKey);
    
    static iReadConfig* GetReader() 
    {
        if (!fgReadConfig) fgReadConfig = new iReadConfig();
        return fgReadConfig; 
    }

    ClassDef(iReadConfig, 0)   // Configuration file reader
};

#endif

