/************************************************************************
 * Author: Thomas Strub                                                 *
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibRunBadScR                                                     //
//                                                                      //
// Beamtime calibration module class for run by run bad scaler reads    //
// calibration.                                                         //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBRUNBADSCR_H
#define TCCALIBRUNBADSCR_H

#include "TCCalibRun.h"

class TBox;
class TArrow;
class TH1;
class TH2;
class TCanvas;
class TCBadScRElement;

class TCCalibRunBadScR : public TCCalibRun
{

protected:
    //----------------------------- data members -------------------------------

    const Char_t* fMainHistoName;       //         name of main histo
    const Char_t* fScalerHistoName;     //         name of scaler histo

    TH2** fMainHistos;                  //[fNRuns] array of pointers to the run's main histo ...
                                        //         ... (detector element vs. scaler reads)
    TH1** fProjHistos;                  //[fNRuns] array of pointers to projected main histos
    TH1** fProjNormHistos;              //[fNRuns] array of pointers to projected, normalized main histos

    TH2** fScalerHistos;                //[fNRuns] array of pointers to the run's scaler histo ...
                                        //         ... (scalers vs. scaler reads)
    TH2* fEmptyMainHisto;               //         empty main histo (dummy)
    TH1* fEmptyProjHisto;               //         empty projected main histo (dummy)
    TH1* fEmptyProjNormHisto;           //         empty projected, normalized main histo (dummy)

    TH1* fOverviewHisto;                //         overview histo ...
    TH1* fOverviewNormHisto;            //         inormalized overview histo ...
                                        //         ... (detector hits per scaler read vs. run)
    Int_t fScP2;                        //         number of P2 scaler (= P2 bin-1)
    Int_t fScFree;                      //         number of free scaler (= free bin-1) for livetime
    Int_t fScLive;                      //         number of live scaler (= live bin-1) for livetime

    TCBadScRElement** fBadScROld;       //[fNRuns] array of pointers to the run's bad scaler reads (old)
    TCBadScRElement** fBadScRNew;       //[fNruns] array of pointers to the rus's bad scaler reads (new)

    Int_t fRangeMax;                    //         maximal range of

    TCBadScRElement* fBadScRCurr;       //!        bad scaler reads of the current run

    TBox** fBadScRCurrBox;              //!        array of TBoxes (displays the bad scaler reads)

    TArrow* fLastReadMarker;            //!        marker arrow for last scaler read
    TLine* fRunMarker;                  //!        marker for current run in overview canvas

    Int_t fLastMouseBin;                //!        last mouse click position (i.e., bin in main histo)
    Int_t fUserInterval;                //         number of scaler read bins for displayed
    Int_t fUserLastInterval;            //         last interval displayed

    TCanvas* fCanvasMain;               //         main canvas
    TCanvas* fCanvasOverview;           //         overview canvas

    //---------------------------- member methods ------------------------------

    void SetBadScalerReads(Int_t bscr1, Int_t bscr2);
    void SetBadScalerRead(Int_t bscr);

    inline Bool_t IsGood() { return (Bool_t) fMainHistos[fIndex]; };
    void ChangeInterval(Int_t i);

    void UpdateOverviewHisto();

    // setup functions
    virtual Bool_t SetConfig();
    virtual Bool_t Init();

    // run handle functions
    virtual void PrepareCurr();
    virtual void ProcessCurr();
    virtual void SaveValCurr();
    virtual void CleanUpCurr();

    // graphic functions
    virtual void UpdateCanvas();

public:
    TCCalibRunBadScR()
      : TCCalibRun(),
        fMainHistoName(0), fScalerHistoName(0),
        fMainHistos(0), fProjHistos(0), fProjNormHistos(0),fScalerHistos(0),
        fEmptyMainHisto(0), fEmptyProjHisto(0), fEmptyProjNormHisto(0),
        fOverviewHisto(0), fOverviewNormHisto(0),
        fScP2(-1), fScFree(-1), fScLive(-1),
        fBadScROld(0), fBadScRNew(0),
        fRangeMax(0),
        fBadScRCurr(0),
        fBadScRCurrBox(0),
        fLastReadMarker(0),
        fRunMarker(0),
        fLastMouseBin(0), fUserInterval(100), fUserLastInterval(1),
        fCanvasMain(0), fCanvasOverview(0) { };
    TCCalibRunBadScR(const Char_t* name, const Char_t* title, const Char_t* data, Bool_t istruecalib)
      : TCCalibRun(name, title, data, istruecalib),
        fMainHistoName(0), fScalerHistoName(0),
        fMainHistos(0), fProjHistos(0), fProjNormHistos(0), fScalerHistos(0),
        fEmptyMainHisto(0), fEmptyProjHisto(0), fEmptyProjNormHisto(0),
        fOverviewHisto(0), fOverviewNormHisto(0),
        fScP2(-1), fScFree(-1), fScLive(-1),
        fBadScROld(0), fBadScRNew(0),
        fRangeMax(0),
        fBadScRCurr(0),
        fBadScRCurrBox(0),
        fLastReadMarker(0),
        fRunMarker(0),
        fLastMouseBin(0), fUserInterval(100), fUserLastInterval(1),
        fCanvasMain(0), fCanvasOverview(0) { };
    virtual ~TCCalibRunBadScR();

    virtual Bool_t Write();

    virtual void EventHandler(Int_t event, Int_t ox, Int_t oy, TObject* selected);

    ClassDef(TCCalibRunBadScR, 0) // Bad scaler read calibration module class
};

#endif

