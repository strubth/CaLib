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

#include "iCalib.hh"
#include "iConfig.hh"
#include "iMySQLManager.hh"
#include "iFileManager.hh"


class iCalibCB2gTime : public iCalib
{

private:
    Double_t fTimeGain;                 // CB TDC gain
    TLine* fLine;                       // indicator line
    TH1* fOverviewHisto;                // overview result histogram
    TF1* fOverviewFunc;                 // overview histogram fitting function

    virtual void CustomizeGUI();
    virtual void Process(Int_t elem);

public:
    iCalibCB2gTime(Int_t set);
    virtual ~iCalibCB2gTime();

    virtual void Write();

    ClassDef(iCalibCB2gTime, 0)   // CB time calibration
};

#endif

