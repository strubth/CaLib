// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibLED                                                           //
//                                                                      //
// Calibration module for LED thresholds.                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBLED_H
#define TCCALIBLED_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TMath.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibLED : public TCCalib
{

private:
    TH2* fMainHisto2;                   // normalization histogram
    TH1* fDeriv;                        // derived histogram
    Double_t fThr;                      // threshold value
    TLine* fLine;                       // mean indicator line
    TLine* fLine2;                      // mean indicator line

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);
    
public:
    TCCalibLED() : TCCalib(), fMainHisto2(0), fDeriv(0), fThr(0), fLine(0), fLine2(0) { }
    TCCalibLED(const Char_t* name, const Char_t* title, CalibData_t data,
               Int_t nElem);
    virtual ~TCCalibLED();

    ClassDef(TCCalibLED, 0) // LED calibration base class
};


class TCCalibCBLED : public TCCalibLED
{

public:
    TCCalibCBLED()
        : TCCalibLED("CB.LED", "CB LED calibration", 
                     kCALIB_CB_LED,
                     TCConfig::kMaxCB) { }
    virtual ~TCCalibCBLED() { }

    ClassDef(TCCalibCBLED, 0) // CB LED calibration
};


class TCCalibTAPSLED1 : public TCCalibLED
{

public:
    TCCalibTAPSLED1()
        : TCCalibLED("TAPS.LED1", "TAPS LED1 calibration", 
                     kCALIB_TAPS_LED1,
                     TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSLED1() { }

    ClassDef(TCCalibTAPSLED1, 0) // TAPS LED1 calibration
};


class TCCalibTAPSLED2 : public TCCalibLED
{

public:
    TCCalibTAPSLED2()
        : TCCalibLED("TAPS.LED2", "TAPS LED2 calibration", 
                     kCALIB_TAPS_LED2,
                     TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSLED2() { }

    ClassDef(TCCalibTAPSLED2, 0) // TAPS LED2 calibration
};

#endif

