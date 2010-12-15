/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibCBEnergy                                                       //
//                                                                      //
// Calibration module for the CB energy.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBCBENERGY_HH
#define ICALIBCBENERGY_HH

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "iCalib.hh"
#include "iUtils.hh"
#include "iConfig.hh"
#include "iMySQLManager.hh"
#include "iFileManager.hh"


class iCalibCBEnergy : public iCalib
{

private:
    Double_t* fPi0IMOld;                // old pi0 invariant mass values
    Double_t* fPi0IMNew;                // old pi0 invariant mass values
    TLine* fLine;                       // indicator line
    
    virtual void CustomizeGUI();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    iCalibCBEnergy() : iCalib(), 
                       fPi0IMOld(0), fPi0IMNew(0), fLine(0) { }
    iCalibCBEnergy(Int_t set);
    virtual ~iCalibCBEnergy();

    ClassDef(iCalibCBEnergy, 0)   // CB energy calibration
};

#endif

