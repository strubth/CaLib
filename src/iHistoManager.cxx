
/*******************************************************************
 *                                                                 *
 * Date: 01.04.2009     Author: Irakli                             *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

#include "iHistoManager.hh"

ClassImp(iHistoManager)

//------------------------------------------------------------------------------
iHistoManager::iHistoManager()
{
    //
    //  this->Init();

    //
    if (gROOT->GetFile())
    {
        //       fHistoFile[0] = (TFile*) gFile;
        //     }
        //   else
        //     {
        //       TString strCBHistoFile = this->GetConfigName("CB.TH2D");
        //       fHistoFile[0] = TFile::Open( strCBHistoFile )
    }
}

//------------------------------------------------------------------------------
iHistoManager::~iHistoManager()
{
}

//------------------------------------------------------------------------------
void iHistoManager::Init()
{
    return;
}

