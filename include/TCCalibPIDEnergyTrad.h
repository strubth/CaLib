/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPIDEnergyTrad                                                 //
//                                                                      //
// Calibration module for the PID energy (traditional method).          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBPIDENERGYTRAD_H
#define TCCALIBPIDENERGYTRAD_H

#include "TCCalib.h"

class TLine;
class TH2;
class TFile;
class TCFileManager;

class TCCalibPIDEnergyTrad : public TCCalib
{

private:
    TCFileManager* fFileManager;        // file manager
    Double_t* fPed;                     // pedestal values
    Double_t* fGain;                    // gain values
    Double_t fPionMC;                   // pion position in simulation
    Double_t fProtonMC;                 // proton position in simulation
    Double_t fPionData;                 // pion position in data
    Double_t fProtonData;               // proton position in data
    TH1* fPionPos;                      // pion peak position histogram
    TH1* fProtonPos;                    // proton peak position histogram
    TLine* fLine;                       // mean indicator line
    TLine* fLine2;                       // mean indicator line
    Int_t fDelay;                       // projection fit display delay
    TH2* fMCHisto;                      // MC histogram
    TFile* fMCFile;                     // MC ROOT file

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

    void FitSlice(TH2* h);

public:
    TCCalibPIDEnergyTrad();
    virtual ~TCCalibPIDEnergyTrad();

    virtual void WriteValues();
    virtual void PrintValues();

    ClassDef(TCCalibPIDEnergyTrad, 0) // PID energy calibration (traditional method)
};

#endif

