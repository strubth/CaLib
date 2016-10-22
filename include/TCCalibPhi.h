/*************************************************************************
 * Authors: Dominik Werthmueller, Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPhi                                                           //
//                                                                      //
// Calibration module for phi angle calibrations.                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBDPHI_H
#define TCCALIBDPHI_H

#include "TCCalib.h"
#include "TCConfig.h"

class TCLine;
class TCanvas;
class TH1;
class TF1;

class TCCalibPhi : public TCCalib
{

private:
    Double_t fMean;                     // mean time position
    TCLine* fLine;                      // indicator line
    TCanvas* fCanvasResult2;            // second result canvas
    TH1* fOverviewHisto2;               // second overview histogram
    TF1* fFitFunc2;                     // second fitting function

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

    TH1* GetMappedHistogram(TH1* histo);

public:
    TCCalibPhi() : TCCalib(), fMean(0), fLine(0), fCanvasResult2(0),
                   fOverviewHisto2(0), fFitFunc2(0) { }
    TCCalibPhi(const Char_t* name, const Char_t* title, const Char_t* data,
               Int_t nElem);
    virtual ~TCCalibPhi();

    virtual void WriteValues();

    ClassDef(TCCalibPhi, 0) // phi calibration
};

class TCCalibPIDPhi : public TCCalibPhi
{

public:
    TCCalibPIDPhi() : TCCalibPhi("PID.Phi", "PID phi angle calibration",
                                 "Data.PID.Phi",
                                 TCConfig::kMaxPID) { }
    virtual ~TCCalibPIDPhi() { }

    ClassDef(TCCalibPIDPhi, 0) // PID phi angle calibration class
};

class TCCalibPizzaPhi : public TCCalibPhi
{

public:
    TCCalibPizzaPhi() : TCCalibPhi("Pizza.Phi", "Pizza phi angle calibration",
                                   "Data.Pizza.Phi",
                                   TCConfig::kMaxPizza) { }
    virtual ~TCCalibPizzaPhi() { }

    ClassDef(TCCalibPizzaPhi, 0) // Pizza phi angle calibration class
};

#endif

