/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTAPSPSA                                                       //
//                                                                      //
// Calibration module for TAPS PSA.                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTAPSPSA_H
#define TCCALIBTAPSPSA_H

#include "TCCalib.h"

class TPolyLine;
class TGraph;
class TH1;
class TCFileManager;

class TCCalibTAPSPSA : public TCCalib
{

private:
    Int_t fNpoints;                     // number of points
    Double_t* fRadiusMean;              // radius
    Double_t* fRadiusSigma;             // radius
    Double_t* fMean;                    // means
    Double_t* fSigma;                   // sigmas
    TPolyLine* fLMean;                  // mean line
    TPolyLine* fLSigma;                 // sigma
    TGraph** fGMean;                    // mean graph
    TGraph** fGSigma;                   // sigma graph
    TCFileManager* fFileManager;        // file manager
    TH1* fAngleProj;                    // angle projection
    Double_t fDelay;                    // display delay

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTAPSPSA();
    virtual ~TCCalibTAPSPSA();

    virtual void WriteValues();
    virtual void PrintValues();

    ClassDef(TCCalibTAPSPSA, 0) // TAPS PSA
};

#endif

