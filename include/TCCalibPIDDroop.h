/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPIDDroop                                                      //
//                                                                      //
// Calibration module for the PID droop correction.                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBPIDDROOP_H
#define TCCALIBPIDDROOP_H

#include "TCCalib.h"

class TH2;
class TH3;
class TGraph;
class TLine;
class TFile;
class TCFileManager;

class TCCalibPIDDroop : public TCCalib
{

private:
    TFile* fOutFile;                    // output file
    TCFileManager* fFileManager;        // file manager
    TH2* fProj2D;                       // dE vs E projection
    TGraph* fLinPlot;                   // linear fitting histogram
    Int_t fNpeak;                       // number of proton peaks
    Int_t fNpoint;                      // number of points in graph
    Double_t* fPeak;                    //[fNpeak] proton peak positions
    Double_t* fTheta;                   //[fNpeak] theta positions
    TLine* fLine;                       // mean indicator line
    Int_t fDelay;                       // projection fit display delay

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

    Bool_t FitHisto(Double_t* outPeak);
    void FitSlices(TH3* h, Int_t elem);

public:
    TCCalibPIDDroop();
    virtual ~TCCalibPIDDroop();

    virtual void WriteValues();
    virtual void PrintValues();

    ClassDef(TCCalibPIDDroop, 0) // PID droop correction
};

#endif

