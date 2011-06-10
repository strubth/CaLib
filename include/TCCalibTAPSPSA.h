// SVN Info: $Id$

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

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TMath.h"
#include "TPolyLine.h"
#include "TSpectrum.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibTAPSPSA : public TCCalib
{

private:
    Int_t fNpoints;                     // number of points
    Double_t* fRadius;                  // energy radius
    Double_t* fPhotonMean;              // photon means
    Double_t* fPhotonSigma;             // photon sigmas
    TPolyLine* fLPhotonMean;            // photon mean line
    TPolyLine* fLPhotonSigma;           // photon sigma
    TCFileManager* fFileManager;        // file manager
    TH1* fAngleProj;                    // angle projection
    Double_t fDelay;                    // display delay
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTAPSPSA();
    virtual ~TCCalibTAPSPSA();
    
    virtual void Write();
    virtual void PrintValues();

    ClassDef(TCCalibTAPSPSA, 0) // TAPS PSA
};

#endif

