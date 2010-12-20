// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibCBQuadEnergy                                                  //
//                                                                      //
// Calibration module for the CB quadratic energy correction.           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBCBQUADENERGY_H
#define TCCALIBCBQUADENERGY_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"
#include "TMath.h"

#include "TCCalib.h"
#include "TCFileManager.h"


class TCCalibCBQuadEnergy : public TCCalib
{

private:
    Double_t* fPar0;                        // correction parameter 0
    Double_t* fPar1;                        // correction parameter 1
    TH2* fMainHisto2;                       // histogram with mean photon energy of pi0
    TH2* fMainHisto3;                       // histogram with mean photon energy of eta
    TH1* fFitHisto1b;                       // fitting histogram
    TH1* fFitHisto2;                        // fitting histogram
    TH1* fFitHisto3;                        // fitting histogram
    TF1* fFitFunc1b;                        // additional fitting function
    Double_t fPi0Pos;                       // pi0 position
    Double_t fEtaPos;                       // eta position
    Double_t fPi0MeanE;                     // pi0 mean energy
    Double_t fEtaMeanE;                     // eta mean energy
    TLine* fLinePi0;                        // pi0 indicator line
    TLine* fLineEta;                        // eta indicator line
    TLine* fLineMeanEPi0;                   // pi0 mean photon energy indicator line
    TLine* fLineMeanEEta;                   // eta mean photon energy indicator line
    TH1* fPi0PosHisto;                      // histogram of pi0 positions
    TH1* fEtaPosHisto;                      // histogram of eta positions

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibCBQuadEnergy();
    virtual ~TCCalibCBQuadEnergy();
    
    virtual void Write();
    virtual void PrintValues();

    ClassDef(TCCalibCBQuadEnergy, 0)   // CB quadratic energy correction
};

#endif

