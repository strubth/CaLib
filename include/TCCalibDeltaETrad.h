/*************************************************************************
 * Author: Dominik Werthmueller, Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibDeltaETrad                                                    //
//                                                                      //
// Calibration module for DeltaE calibrations (traditional method).     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBDELTAETRAD_H
#define TCCALIBDELTAETRAD_H

#include "TCCalib.h"
#include "TCConfig.h"

class TCLine;
class TH2;
class TFile;
class TCFileManager;

class TCCalibDeltaETrad : public TCCalib
{

private:
    TCFileManager* fFileManager;        // file manager
    Double_t* fPedOld;                  // old pedestal values
    Double_t* fPedNew;                  // new pedestal values
    Double_t* fGainOld;                 // old gain values
    Double_t* fGainNew;                 // new gain values
    Double_t fPionMC;                   // pion position in simulation
    Double_t fProtonMC;                 // proton position in simulation
    Double_t fPionData;                 // pion position in data
    Double_t fProtonData;               // proton position in data
    TH1* fPionPos;                      // pion peak position histogram
    TH1* fProtonPos;                    // proton peak position histogram
    TCLine* fLine;                      // mean indicator line
    TCLine* fLine2;                     // mean indicator line
    Int_t fDelay;                       // projection fit display delay
    TH2* fMCHisto;                      // MC histogram
    TFile* fMCFile;                     // MC ROOT file

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

    void FitSlice(TH2* h);

public:
    TCCalibDeltaETrad() : TCCalib(), fFileManager(0),
                          fPedOld(0), fPedNew(0), fGainOld(0), fGainNew(0),
                          fPionMC(0), fProtonMC(0), fPionData(0), fProtonData(0),
                          fPionPos(0), fProtonPos(0), fLine(0), fLine2(0), fDelay(0),
                          fMCHisto(0), fMCFile(0) { }
    TCCalibDeltaETrad(const Char_t* name, const Char_t* title, const Char_t* data,
                      Int_t nElem);
    virtual ~TCCalibDeltaETrad();

    virtual void WriteValues();
    virtual void PrintValues();

    ClassDef(TCCalibDeltaETrad, 0) // DeltaE energy calibration (traditional method)
};

class TCCalibPIDEnergyTrad : public TCCalibDeltaETrad
{

public:
    TCCalibPIDEnergyTrad()
        : TCCalibDeltaETrad("PID.Energy.Trad", "PID energy calibration (traditional)",
                            "Data.PID.E1", TCConfig::kMaxPID) { }
    virtual ~TCCalibPIDEnergyTrad() { }

    ClassDef(TCCalibPIDEnergyTrad, 0) // PID energy calibration class
};

class TCCalibPizzaEnergyTrad : public TCCalibDeltaETrad
{

public:
    TCCalibPizzaEnergyTrad()
        : TCCalibDeltaETrad("Pizza.Energy.Trad", "Pizza energy calibration (traditional)",
                            "Data.Pizza.E1", TCConfig::kMaxPizza) { }
    virtual ~TCCalibPizzaEnergyTrad() { }

    ClassDef(TCCalibPizzaEnergyTrad, 0) // Pizza detector energy calibration class
};

#endif

