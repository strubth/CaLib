/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTime                                                          //
//                                                                      //
// Base time calibration module class.                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTIME_H
#define TCCALIBTIME_H

#include "TCCalib.h"
#include "TCConfig.h"
#include "TCReadConfig.h"

class TCLine;

class TCCalibTime : public TCCalib
{

private:
    Double_t* fTimeGain;                // TDC gain array
    Double_t fMean;                     // mean time position
    TCLine* fLine;                      // indicator line

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTime() : TCCalib(), fTimeGain(0), fMean(0), fLine(0) { }
    TCCalibTime(const Char_t* name, const Char_t* title, const Char_t* data,
                Int_t nElem);
    virtual ~TCCalibTime();

    ClassDef(TCCalibTime, 0) // Base time calibration class
};

class TCCalibTaggerTime : public TCCalibTime
{

public:
    TCCalibTaggerTime()
        : TCCalibTime("Tagger.Time", "Tagger time calibration",
                     "Data.Tagger.T0",
                     TCReadConfig::GetReader()->GetConfigInt("Tagger.Elements")) { }
    virtual ~TCCalibTaggerTime() { }

    ClassDef(TCCalibTaggerTime, 0) // Tagger time calibration class
};

class TCCalibCBTime : public TCCalibTime
{

public:
    TCCalibCBTime()
        : TCCalibTime("CB.Time", "CB time calibration",
                     "Data.CB.T0",
                     TCConfig::kMaxCB) { }
    virtual ~TCCalibCBTime() { }

    ClassDef(TCCalibCBTime, 0) // CB time calibration class
};

class TCCalibCBRiseTime : public TCCalibTime
{

public:
    TCCalibCBRiseTime()
        : TCCalibTime("CB.RiseTime", "CB rise time calibration",
                     "Data.CB.Walk.Par0",
                     TCConfig::kMaxCB) { }
    virtual ~TCCalibCBRiseTime() { }

    ClassDef(TCCalibCBRiseTime, 0) // CB rise time calibration class
};

class TCCalibTAPSTime : public TCCalibTime
{

public:
    TCCalibTAPSTime()
        : TCCalibTime("TAPS.Time", "TAPS time calibration",
                     "Data.TAPS.T0",
                     TCReadConfig::GetReader()->GetConfigInt("TAPS.Elements")) { }
    virtual ~TCCalibTAPSTime() { }

    ClassDef(TCCalibTAPSTime, 0) // TAPS time calibration class
};

class TCCalibPIDTime : public TCCalibTime
{

public:
    TCCalibPIDTime()
        : TCCalibTime("PID.Time", "PID time calibration",
                     "Data.PID.T0",
                     TCConfig::kMaxPID) { }
    virtual ~TCCalibPIDTime() { }

    ClassDef(TCCalibPIDTime, 0) // PID time calibration class
};

class TCCalibVetoTime : public TCCalibTime
{

public:
    TCCalibVetoTime()
        : TCCalibTime("Veto.Time", "Veto time calibration",
                     "Data.Veto.T0",
                     TCReadConfig::GetReader()->GetConfigInt("Veto.Elements")) { }
    virtual ~TCCalibVetoTime() { }

    ClassDef(TCCalibVetoTime, 0) // Veto time calibration class
};

class TCCalibPizzaTime : public TCCalibTime
{

public:
    TCCalibPizzaTime()
        : TCCalibTime("Pizza.Time", "Pizza time calibration",
                     "Data.Pizza.T0",
                     TCConfig::kMaxPizza) { }
    virtual ~TCCalibPizzaTime() { }

    ClassDef(TCCalibPizzaTime, 0) // Pizza detector time calibration class
};

#endif

