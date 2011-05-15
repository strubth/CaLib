// SVN Info: $Id$

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

#include "TCanvas.h"
#include "TH3.h"
#include "TLine.h"
#include "TMath.h"
#include "TGraph.h"
#include "TSpectrum.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibPIDDroop : public TCCalib
{

private:
    Double_t* fPar0;                    // droop correction parameter 0
    Double_t* fPar1;                    // droop correction parameter 1
    Double_t* fPar2;                    // droop correction parameter 2
    Double_t* fPar3;                    // droop correction parameter 3
    TCFileManager* fFileManager;        // file manager
    TH2* fProj2D;                       // dE vs E projection
    TGraph* fLinPlot;                   // linear fitting histogram
    Int_t fNpeak;                       // number of proton peaks
    Double_t* fPeak;                    //[fNpeak] proton peak positions
    Double_t* fTheta;                   //[fNpeak] theta positions
    TLine* fLine;                       // mean indicator line
    Int_t fDelay;                       // projection fit display delay

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);
    
    Double_t FitHisto();
    void FitSlices(TH3* h, Int_t elem);

public:
    TCCalibPIDDroop();
    virtual ~TCCalibPIDDroop();

    virtual void Write();
    virtual void PrintValues();


    ClassDef(TCCalibPIDDroop, 0) // PID droop correction
};

#endif

