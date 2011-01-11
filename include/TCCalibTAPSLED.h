// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSLED                                                       //
//                                                                      //
// Calibration module for the TAPS LED.                                 //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTAPSLED_H
#define TCCALIBTAPSLED_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TMath.h"

#include "TCCalib.h"
#include "TCFileManager.h"
#include "TCUtils.h"


class TCCalibTAPSLED : public TCCalib
{

private:
    TH2* fMainHisto2;                   // normalization histogram
    Double_t fThr;                      // threshold position
    TLine* fLine;                       // mean indicator line

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);
    
    Double_t FindThreshold(TH1* h, Double_t level);

public:
    TCCalibTAPSLED() : TCCalib(), fMainHisto2(0), fThr(0), fLine(0) { }
    TCCalibTAPSLED(const Char_t* name, const Char_t* title, CalibData_t data,
                   Int_t nElem);
    virtual ~TCCalibTAPSLED();

    ClassDef(TCCalibTAPSLED, 0) // TAPS LED calibration base class
};


class TCCalibTAPSLED1 : public TCCalibTAPSLED
{

public:
    TCCalibTAPSLED1()
        : TCCalibTAPSLED("TAPS.LED1", "TAPS LED1 calibration", 
                         kCALIB_TAPS_LED1,
                         TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSLED1() { }

    ClassDef(TCCalibTAPSLED1, 0) // TAPS LED1 calibration
};


class TCCalibTAPSLED2 : public TCCalibTAPSLED
{

public:
    TCCalibTAPSLED2()
        : TCCalibTAPSLED("TAPS.LED2", "TAPS LED2 calibration", 
                         kCALIB_TAPS_LED2,
                         TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSLED2() { }

    ClassDef(TCCalibTAPSLED2, 0) // TAPS LED2 calibration
};

#endif

