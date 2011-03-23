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
    TH2* fMainHisto2BG;                     // histogram with mean photon energy of pi0 (background)
    TH2* fMainHisto3BG;                     // histogram with mean photon energy of eta (background)
    TH1* fFitHisto1b;                       // fitting histogram
    TH1* fFitHisto2;                        // fitting histogram
    TH1* fFitHisto3;                        // fitting histogram
    TF1* fFitFunc1b;                        // additional fitting function
    TF1* fFitFuncBG;                        // pi0 background function
    TF1* fFitFunc1bBG;                      // eta background function
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
    Double_t fPi0Prompt[2];                 // pi0 prompt range
    Double_t fPi0BG1[2];                    // pi0 background 1 range
    Double_t fPi0BG2[2];                    // pi0 background 2 range
    Double_t fEtaPrompt[2];                 // eta prompt range
    Double_t fEtaBG1[2];                    // eta background 1 range
    Double_t fEtaBG2[2];                    // eta background 2 range
    Bool_t fIsFitPi0;                       // pi0/eta fitting toggle

    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

    Double_t BGFunc(Double_t* x, Double_t* par);

public:
    TCCalibCBQuadEnergy();
    virtual ~TCCalibCBQuadEnergy();
    
    virtual void Write();
    virtual void PrintValues();

    ClassDef(TCCalibCBQuadEnergy, 0) // CB quadratic energy correction
};

#endif

