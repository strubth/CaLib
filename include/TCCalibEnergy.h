// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibEnergy                                                        //
//                                                                      //
// Base energy calibration module class.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBENERGY_H
#define TCCALIBENERGY_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibEnergy : public TCCalib
{

private:
    Double_t fPi0Pos;                   // pi0 position
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibEnergy() : TCCalib(), fPi0Pos(0), fLine(0) { }
    TCCalibEnergy(const Char_t* name, const Char_t* title, const Char_t* data,
                  Int_t nElem);
    virtual ~TCCalibEnergy();

    ClassDef(TCCalibEnergy, 0) // Base energy calibration class
};


class TCCalibCBEnergy : public TCCalibEnergy
{
    
public:
    TCCalibCBEnergy()
        : TCCalibEnergy("CB.Energy", "CB energy calibration", 
                        "Data.CB.E1", 
                        TCConfig::kMaxCB) { }
    virtual ~TCCalibCBEnergy() { }

    ClassDef(TCCalibCBEnergy, 0) // CB energy calibration
};


class TCCalibTAPSEnergyLG : public TCCalibEnergy
{
    
public:
    TCCalibTAPSEnergyLG()
        : TCCalibEnergy("TAPS.Energy.LG", "TAPS LG energy calibration", 
                        "Data.TAPS.LG.E1",
                        TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSEnergyLG() { }

    ClassDef(TCCalibTAPSEnergyLG, 0) // TAPS LG energy calibration
};

#endif

