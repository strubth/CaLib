/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibCB2gTime                                                       //
//                                                                      //
// Calibration module for the CB time.                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBCB2GTIME_HH
#define ICALIBCB2GTIME_HH

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TError.h"

#include "iCalib.hh"
#include "iReadConfig.hh"
#include "iCrystalNavigator.hh"
#include "iFileManager.hh"
#include "iMySQLManager.hh"


class iCalibCB2gTime
    : public iCalib,
      public iCrystalNavigator
{

private:
    Double_t fTimeGain;
    Double_t* fGaussMean;
    Double_t* fGaussError;
    TLine** fLineOffset;
    

    TH1F* hhOffset;
    TF1* fPol0;

    Double_t peackval;
    
    virtual void PrintInfo();
    virtual void CustomizeGUI();
    virtual void PrepareFit(Int_t elem);
    virtual void Fit(Int_t elem);
    virtual void Draw(Int_t elem);

public:
    iCalibCB2gTime(Int_t);
    virtual ~iCalibCB2gTime();

    void Calculate(Int_t);
    virtual void Write();

    
    
    ClassDef(iCalibCB2gTime, 0)   // CB vs CB time calibration
};

#endif

