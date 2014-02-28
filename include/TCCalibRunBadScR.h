/*************************************************************************
 * Author: Thomas Strub
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibRunBadScR                                                     //
//                                                                      //
// Beamtime calibration module class for run by run bad scaler reads    //
// calibration.                                                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBRUNBADSCR_H
#define TCCALIBRUNBADSCR_H

#include "TCCalibRun.h"
#include "TCBadScRElement.h"
#include "TBox.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"


class TCCalibRunBadScR : public TCCalibRun
{

protected:
    //----------------------------- data members -------------------------------

    const Char_t* fMainHistoName;                 //[128] name of main histo
    const Char_t* fScalerHistoName;               //[128] name of scaler histo

    TH2** fMainHistos;                            //[fNRuns] array of pointers to the run's main histo ...
                                                  // ... (detector element vs. scaler reads)
    TH1** fProjHistos;                            //[fNRuns] array of pointers to projected main histos

    TH2** fScalerHistos;                          //[fNRuns] array of pointers to the run's scaler histo ...
                                                  // ... (scalers vs. scaler reads)
    TH2* fEmptyMainHisto;
    TH1* fEmptyProjHisto;

    TH1* fOverviewHisto;                          // overview histo ...
                                                  // ... (normalized summed up detector hits vs. run)
    TH1* fOverviewHistoCurrRun;

    Int_t fScP2;                                  // number of P2 scaler (P2 bin - 1)
    Int_t fScFree;                                // number of free scaler (free bin - 1) for livetime
    Int_t fScInh;                                 // number of inh. scaler (inh. bin - 1) for livetime

    Bool_t fUseEventInfoNScR;

    TCBadScRElement** fBadScROld;                 //[fNRuns] array of pointers to the run's bad scaler reads (old)
    TCBadScRElement** fBadScRNew;                 //[fNruns] array of pointers to the rus's bad scaler reads (new)

    Int_t fRangeMax;

    TCBadScRElement* fBadScRCurr;                 //! bad scaler reads of the current run

    TBox** fBadScRCurrBox;                        //! array of TBoxes (displays the bad scaler reads)

    Int_t fLastMouseBin;                          //! last mouse click position (i.e., bin in main histo)
    Int_t fUserInterval;
    Int_t fUserLastInterval;

    TCanvas* fCanvasMain;                         // main canvas
    TCanvas* fCanvasOverview;                     // overview canvas

    //---------------------------- member methods ------------------------------

    void SetBadScalerReads(Int_t bscr1, Int_t bscr2);
    void SetBadScalerRead(Int_t bscr);

    inline Bool_t IsGood() { return fMainHistos[fIndex]; };
    void ChangeInterval(Int_t i);

    void UpdateOverviewHisto();

    // setup functions
    virtual Bool_t SetConfig();         // set configuration from config file
    virtual Bool_t Init();              // child initializer

    // run handle functions
    virtual void PrepareRun();
    virtual void ProcessRun();
    virtual void SaveValRun();
    virtual void CleanUpRun();

    // graphic functions
    virtual void UpdateCanvas();        // update canvas methode

public:
    TCCalibRunBadScR()
      : TCCalibRun(),
        fMainHistoName(0), fScalerHistoName(0),
        fMainHistos(0), fProjHistos(0), fScalerHistos(0),
        fOverviewHisto(0),
        fScP2(-1), fScFree(-1), fScInh(-1),
        fBadScROld(0), fBadScRNew(0),
        fRangeMax(0),
        fBadScRCurr(0),
        fBadScRCurrBox(0),
        fLastMouseBin(0), fUserInterval(100), fUserLastInterval(1),
        fCanvasMain(0), fCanvasOverview(0) { };
    TCCalibRunBadScR(const Char_t* name, const Char_t* title, const Char_t* data)
      : TCCalibRun(name, title, data),
        fMainHistoName(0), fScalerHistoName(0),
        fMainHistos(0), fProjHistos(0), fScalerHistos(0),
        fOverviewHisto(0),
        fScP2(-1), fScFree(-1), fScInh(-1),
        fBadScROld(0), fBadScRNew(0),
        fRangeMax(0),
        fBadScRCurr(0),
        fBadScRCurrBox(0),
        fLastMouseBin(0), fUserInterval(100), fUserLastInterval(1),
        fCanvasMain(0), fCanvasOverview(0) { };
    virtual ~TCCalibRunBadScR();

    virtual Bool_t Write();

    virtual void EventHandler(Int_t event, Int_t ox, Int_t oy, TObject* selected);

    ClassDef(TCCalibRunBadScR, 0) // Bad scaler read calibration module class
};


class TCCalibRunBadScR_NaI : public TCCalibRunBadScR
{

public:
    TCCalibRunBadScR_NaI()
      : TCCalibRunBadScR("BadScR.NaI", "Bad scaler read calibration (NaI)", "Data.Run.BadScR.NaI") { };
    virtual ~TCCalibRunBadScR_NaI() { };

    ClassDef(TCCalibRunBadScR_NaI, 0) // NaI bad scaler read calibration class
};


class TCCalibRunBadScR_PID : public TCCalibRunBadScR
{

public:
    TCCalibRunBadScR_PID()
      : TCCalibRunBadScR("BadScR.PID", "Bad scaler read calibration (PID)", "Data.Run.BadScR.PID") { };
    virtual ~TCCalibRunBadScR_PID() { };

    ClassDef(TCCalibRunBadScR_PID, 0) // PID bad scaler read calibration class
};


class TCCalibRunBadScR_BaF2 : public TCCalibRunBadScR
{

public:
    TCCalibRunBadScR_BaF2()
      : TCCalibRunBadScR("BadScR.BaF2", "Bad scaler read calibration (BaF2)", "Data.Run.BadScR.BaF2") { };
    virtual ~TCCalibRunBadScR_BaF2() { };

    ClassDef(TCCalibRunBadScR_BaF2, 0) // BaF2 bad scaler read calibration class
};


class TCCalibRunBadScR_PWO : public TCCalibRunBadScR
{

public:
    TCCalibRunBadScR_PWO()
      : TCCalibRunBadScR("BadScR.PWO", "Bad scaler read calibration (PWO)", "Data.Run.BadScR.PWO") { };
    virtual ~TCCalibRunBadScR_PWO() { };

    ClassDef(TCCalibRunBadScR_PWO, 0) // PWO bad scaler read calibration class
};

#endif

