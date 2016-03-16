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


#include "TGClient.h"
#include "TBox.h"
#include "TCanvas.h"
#include "TArrow.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TH2.h"
#include "TFile.h"
#include "KeySymbols.h"

#include "TCCalibRunBadScR.h"
#include "TCBadScRElement.h"
#include "TCReadConfig.h"
#include "TCARHistoLoader.h"
#include "TCMySQLManager.h"

ClassImp(TCCalibRunBadScR)

//______________________________________________________________________________
TCCalibRunBadScR::~TCCalibRunBadScR()
{
    // Destructor

    // not owned by this class: fMainHistoName, fScalerHistoName

    if (fMainHistos)
    {
        for (Int_t i = 0; i < fNRuns; i++)
            if (fMainHistos[i]) delete fMainHistos[i];
        delete [] fMainHistos;
    }
    if (fProjHistos)
    {
        for (Int_t i = 0; i < fNRuns; i++)
            if (fProjHistos[i]) delete fProjHistos[i];
        delete [] fProjHistos;
    }
    if (fProjNormHistos)
    {
        for (Int_t i = 0; i < fNRuns; i++)
            if (fProjNormHistos[i]) delete fProjNormHistos[i];
        delete [] fProjNormHistos;
    }
    if (fScalerHistos)
    {
        for (Int_t i = 0; i < fNRuns; i++)
            if (fScalerHistos[i]) delete fScalerHistos[i];
        delete [] fScalerHistos;
    }

    if (fOverviewHisto) delete fOverviewHisto;
    if (fOverviewNormHisto) delete fOverviewNormHisto;

    if (fBadScROld)
    {
        for (Int_t i = 0; i < fNRuns; i++)
            if (fBadScROld[i]) delete fBadScROld[i];
        delete [] fBadScROld;
    }
    if (fBadScRNew)
    {
        for (Int_t i = 0; i < fNRuns; i++)
            if (fBadScRNew[i]) delete fBadScRNew[i];
        delete [] fBadScRNew;
    }

    if (fBadScRCurrBox)
    {
        for (Int_t i = 0; i < fBadScRCurr->GetNElem(); i++)
            if (fBadScRCurrBox[i]) delete fBadScRCurrBox[i];
        delete [] fBadScRCurrBox;
    }

    if (fLastReadMarker) delete fLastReadMarker;
    if (fRunMarker) delete fRunMarker;

    if (fBadScRCurr) delete fBadScRCurr;

    if (fCanvasMain) delete fCanvasMain;
    if (fCanvasOverview) delete fCanvasOverview;
}

//______________________________________________________________________________
Bool_t TCCalibRunBadScR::SetConfig()
{
    // Reads configuration values from config file via 'TCReadConfig'

    // call parent SetConfig()
    TCCalibRun::SetConfig();

    // temporary string
    Char_t tmp[128];

    // get main histogram name
    sprintf(tmp, "%s.Histo.Main.Name", GetName());
    if (!(fMainHistoName = TCReadConfig::GetReader()->GetConfig(tmp)->Data()))
    {
        Error("Start", "Main histo was not found in configuration!");
        return kFALSE;
    }

    // get scaler histogram name
    sprintf(tmp, "BadScR.Histo.Scaler.Name");
    if (!(fScalerHistoName = TCReadConfig::GetReader()->GetConfig(tmp)->Data()))
    {
        Error("Start", "Scaler histo was not found in configuration!");
        return kFALSE;
    }

    // get P2 scaler number
    sprintf(tmp, "BadScR.Scaler.P2");
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Start", "P2 scaler was not found in configuration!");
    }
    else fScP2 = TCReadConfig::GetReader()->GetConfigInt(tmp);

    // get free scaler number
    sprintf(tmp, "BadScR.Scaler.Free");
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Start", "Free scaler was not found in configuration!");
    }
    else fScFree = TCReadConfig::GetReader()->GetConfigInt(tmp);

    // get live scaler number
    sprintf(tmp, "BadScR.Scaler.Live");
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Start", "Live scaler was not found in configuration!");
    }
    else fScLive = TCReadConfig::GetReader()->GetConfigInt(tmp);

    // get user interval
    sprintf(tmp, "BadScR.Histo.Main.UserRange");
    if (TCReadConfig::GetReader()->GetConfig(tmp))
        fUserInterval = TCReadConfig::GetReader()->GetConfigInt(tmp);

    return kTRUE;
}

//______________________________________________________________________________
Bool_t TCCalibRunBadScR::Init()
{
    // Loads main and scaler histogras, creates (normalized) projections of main
    // histograms, reads old bad scaler reads from the calib database, sets up
    // the overview histogram and creates the canvas.

    // call parent Init()
    TCCalibRun::Init();

    // adjust style
    gStyle->SetOptStat(0);
    gStyle->SetPadRightMargin(0.03);
    gStyle->SetPadLeftMargin(0.05);
    gStyle->SetLabelSize(0.06, "X");
    gStyle->SetLabelSize(0.06, "Y");

    // force style for loaded histograms too
    gROOT->ForceStyle();


    // load & prepare histos ---------------------------------------------------

    // init histo loader
    TCARHistoLoader rhl(fNRuns, fRuns);

    // user info
    Info("Init", "Loading files...");

    // load files
    if (!rhl.LoadFiles())
    {
        Error("Init", "Could not load any files!");
        return kFALSE;
    }
    else if (!rhl.GetNOpenFiles())
    {
        Error("Init", "Could not load any files!");
        return kFALSE;
    }

    // user info
    Info("Init", "Loading main histograms...");

    // load main histograms
    if (!(fMainHistos = (TH2**) rhl.CreateHistoArray(fMainHistoName)))
    {
        Error("Init", "Could not load any main histograms named '%s'!", fMainHistoName);
        return kFALSE;
    }

    // user info
    Info("Init", "Loading scaler histograms...");

    // load scaler histograms
    if (!(fScalerHistos = (TH2**) rhl.CreateHistoArray(fScalerHistoName)))
    {
        Warning("Init", "Could not load any scaler histograms named '%s'!", fScalerHistoName);
        Warning("Init", "Histograms will not be normalized!");
    }

    // create projection array
    fProjHistos = new TH1*[fNRuns];

    // user info
    Info("Init", "Projecting main histograms...");

    // create main histogram projections
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // init pointer
        fProjHistos[i] = 0;

        // check for main histo
        if(!fMainHistos[i]) continue;

        // get projection
        Char_t tmp[128];
        sprintf(tmp, "%s_px", fMainHistos[i]->GetName());
        fProjHistos[i] = (TH1D*) fMainHistos[i]->ProjectionX(tmp, 1, -1);
        fProjHistos[i]->SetTitle("");
        fProjHistos[i]->GetXaxis()->SetLabelSize(0);
    }

    // create normalized projection array
    fProjNormHistos = new TH1*[fNRuns];

    // check whether scaler histo was loaded
    if (fScalerHistos)
    {
        // user infos
        Info("Init", "Normalizing projection histograms...");
        if (fScP2 <= -1)
            Warning("Init", "Histograms will not be P2 corrected.");
        if (fScFree <= -1 || fScLive <= -1)
            Warning("Init", "Histograms will not be livetime corrected.");
    }

    // loop over runs
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // init pointer
        fProjNormHistos[i] = 0;

        // check for histo
        if (!fProjHistos[i]) continue;

        // clone it
        Char_t tmp[256];
        sprintf(tmp, "%s_%s", fProjHistos[i]->GetName(), "_norm");
        fProjNormHistos[i] = (TH1D*) fProjHistos[i]->Clone(tmp);
        fProjNormHistos[i]->UseCurrentStyle();//SetLabelSize(0.06);

        // check whether scaler histo was loaded
        if (!fScalerHistos) continue;

        // check for scaler histo
        if (!fScalerHistos[i])
        {
            Warning("Init", "No scaler histogram for run '%i'. Will not be normalized.", fRuns[i]);
            continue;
        }

        // loop over scaler reads
        for (Int_t j = 0; j < fProjNormHistos[i]->GetNbinsX(); j++)
        {
            // init scalers
            Double_t p2 = 1.;
            Double_t lt = 1.;

            // get p2 scaler
            if (fScP2 >= 0)
                p2 = fScalerHistos[i]->GetBinContent(j+1, fScP2+1);

            // get livetime
            if (fScFree >= 0 && fScLive >= 0)
            {
                if (fScalerHistos[i]->GetBinContent(j+1, fScFree+1) > 0.)
                    lt = fScalerHistos[i]->GetBinContent(j+1, fScLive+1) / fScalerHistos[i]->GetBinContent(j+1, fScFree+1);
                else
                    lt = 0.;
            }

            // normalization factor
            Double_t norm = p2 * lt;

            // normalize histo
            if (norm > 0.)
                fProjNormHistos[i]->SetBinContent(j+1, fProjNormHistos[i]->GetBinContent(j+1) / norm);
            else
                fProjNormHistos[i]->SetBinContent(j+1, 0.);
        }
    }


    // load & prepare bad scaler reads -----------------------------------------

    // create bad scaler read arrays
    fBadScROld = new TCBadScRElement*[fNRuns];
    fBadScRNew = new TCBadScRElement*[fNRuns];

    // user info
    Info("Init", "Reading old bad scaler reads from database...");

    // read bad scaler reads from database
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // init bad scaler read array
        fBadScROld[i] = 0;
        fBadScRNew[i] = 0;

        // get number of scaler reads from database
        Int_t nscr = TCMySQLManager::GetManager()->GetRunNScR(fRuns[i]);

        // get number of scaler reads from event info histo
        if (rhl.GetFiles()[i])
        {
            // get the event info histo for this run
            TH1* h = (TH1*) rhl.GetFiles()[i]->Get("EventInfo");

            // check for same number of scaler reads
            if (h && nscr != h->GetBinContent(TCConfig::kNScREventHBin))
            {
                 if (nscr == -1)
                     Warning("Init", "Number of scaler reads for run '%i' is not set in the database yet!", fRuns[i]);
                 else
                     Error("Init", "Number of scaler reads mismatch for run '%i' (database: '%i' vs. EventInfo histogram: '%i'!",
                           fRuns[i], nscr, (Int_t) h->GetBinContent(TCConfig::kNScREventHBin));

                 // use number of scaler reads from event info histo
                 nscr = h->GetBinContent(TCConfig::kNScREventHBin);
            }
        }

        // init helpers
        Int_t nbadscr = 0;
        Int_t* badscr = 0;

        // get bad scaler reads from database
        if (!TCMySQLManager::GetManager()->GetRunBadScR(fRuns[i], nbadscr, badscr, (*fCalibData).Data()))
        {
            Error("Init", "Could not read bad scaler read from database for run '%i'!", fRuns[i]);
            fBadScROld[i] = new TCBadScRElement(fRuns[i], 0, 0, nscr);
        }
        else
        {
            // set up old bad scaler read element for this run
            fBadScROld[i] = new TCBadScRElement(fRuns[i], nbadscr, badscr, nscr);
        }

        // copy to new bad scaler read element for this run
        if (fBadScROld[i]) fBadScRNew[i] = new TCBadScRElement(*fBadScROld[i]);

        // set max scaler reads
        if (fBadScRNew[i] && fBadScRNew[i]->GetNElem() > fRangeMax)
            fRangeMax = fBadScRNew[i]->GetNElem();

        // clean up
        if (badscr) delete [] badscr;

    }//end loop over runs

    // set max. range to number of scaler reads + 2
    fRangeMax += 2;

    // creat empty histos
    for (Int_t i = 0; i < fNRuns; i++)
    {
        if (fMainHistos[i])
        {
            fEmptyMainHisto = (TH2*) fMainHistos[i]->Clone("EmptyMainHisto");
            fEmptyMainHisto->GetXaxis()->SetRange(1, fRangeMax);
            fEmptyMainHisto->Reset();
            fEmptyProjHisto = (TH1D*) fProjHistos[i]->Clone("EmptyProjHisto");
            fEmptyProjHisto->GetXaxis()->SetRange(1, fRangeMax);
            fEmptyProjHisto->Reset();
            fEmptyProjNormHisto = (TH1D*) fProjNormHistos[i]->Clone("EmptyProjNormHisto");
            fEmptyProjNormHisto->GetXaxis()->SetRange(1, fRangeMax);
            fEmptyProjNormHisto->Reset();
            break;
        }
    }


    // prepare overview histo --------------------------------------------------

    // init overview histo
    fOverviewHisto = new TH1F("OverviewHisto", "Overview histogram", fNRuns, 0, fNRuns);
    fOverviewNormHisto = new TH1F("OverviewNormHisto", "Normalized overview histogram", fNRuns, 0, fNRuns);

    // initialize overview histo for all runs
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // set index
        fIndex = i;

        // set current bad scr
        fBadScRCurr = fBadScRNew[i];

        // update
        UpdateOverviewHisto();

        // reset members (for the sake of beauty already here)
        fIndex = 0;
        fBadScRCurr = 0;
    }

    // create last read marker
    fLastReadMarker = new TArrow();
    fLastReadMarker->SetOption("<|-|");
    fLastReadMarker->SetArrowSize(0.015);
    fLastReadMarker->SetLineWidth(3);
    fLastReadMarker->SetLineColor(kBlack);
    fLastReadMarker->SetFillColor(kBlack);

    // create run marker
    fRunMarker = new TLine();
    fRunMarker->SetLineWidth(2);
    fRunMarker->SetLineColor(kRed);


    // setup main canvas
    fCanvasMain = new TCanvas("Main", "Main", 0, 0, gClient->GetDisplayWidth(), gClient->GetDisplayHeight()/2.+50);
    fCanvasMain->Divide(1, 3, 0.001, 0.001);
    fCanvasMain->GetPad(1)->SetMargin(0.03, 0.03, 0.02, 0.1);
    fCanvasMain->GetPad(2)->SetMargin(0.03, 0.03, 0.02, 0.02);
    fCanvasMain->GetPad(3)->SetMargin(0.03, 0.03, 0.1, 0.02);
    fCanvasMain->GetPad(1)->SetBit(kCannotPick);
    fCanvasMain->GetPad(2)->SetBit(kCannotPick);
    fCanvasMain->GetPad(3)->SetBit(kCannotPick);

    // setup overview canvas
    fCanvasOverview = new TCanvas("Overview", "Overview", 0, gClient->GetDisplayHeight(), 800, gClient->GetDisplayHeight()/4.+20);
    fCanvasOverview->Divide(1, 2, 0.001, 0.001);

    // disable ROOT zoom box
    fCanvasMain->MoveOpaque(0);

    // connect event handler
    fCanvasMain->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "TCCalibRunBadScR", this,
                         "EventHandler(Int_t, Int_t, Int_t, TObject*)");

    return kTRUE;
}

//______________________________________________________________________________
void TCCalibRunBadScR::PrepareCurr()
{
    // Prepares everything to process the current run

    // update run marker
    fRunMarker->SetX1(fIndex+0.5);
    fRunMarker->SetX2(fIndex+0.5);
    fRunMarker->SetY1(0);
    fRunMarker->SetY2(fOverviewHisto->GetMaximum());

    // check for valid run
    if (!IsGood())
    {
        // user info
        printf("Processing run %05d: empty histograms!\n", fRuns[fIndex]);
        return;
    }

    // user info
    printf("Processing run %05d:\n", fRuns[fIndex]);

    // set standatd user range
    fMainHistos[fIndex]->GetXaxis()->SetRangeUser(0, fMainHistos[fIndex]->GetXaxis()->GetBinUpEdge(fRangeMax));
    fProjHistos[fIndex]->GetXaxis()->SetRangeUser(0, fProjHistos[fIndex]->GetXaxis()->GetBinUpEdge(fRangeMax));

    // copy new scaler read to current
    fBadScRCurr = new TCBadScRElement(*(fBadScRNew[fIndex]));

    // set up boxes
    if (fBadScRCurr->GetNElem() > 0)
    {
        fBadScRCurrBox = new TBox*[fBadScRCurr->GetNElem()];
        for (Int_t i = 0; i < fBadScRCurr->GetNElem(); i++)
            fBadScRCurrBox[i] = 0;
    }

    // set up bad scaler reads
    fBadScRCurr->RemBad();
    for (Int_t i = 0; i < fBadScRNew[fIndex]->GetNBad(); i++)
        SetBadScalerRead(fBadScRNew[fIndex]->GetBad()[i]);

    // mark last scaler read
    fLastReadMarker->SetX1(fBadScRCurr->GetNElem());
    fLastReadMarker->SetX2(fBadScRCurr->GetNElem()+3);
    fLastReadMarker->SetY1(fMainHistos[fIndex]->GetYaxis()->GetXmax()/2);
    fLastReadMarker->SetY2(fMainHistos[fIndex]->GetYaxis()->GetXmax()/2);
}

//______________________________________________________________________________
void TCCalibRunBadScR::ProcessCurr()
{
    // Preforms the current run indexed by 'fIndex'.

    // nothing to do here so far.
    return;

    // check for valid run
    if (!IsGood()) return;

    // it follows some test code for automatic calibration ... to be finished...

    //
    Int_t ngood = fBadScRCurr->GetNElem() - fBadScRCurr->GetNBad();
    Double_t mean = 0;
    for (Int_t i = 0; i < fBadScRCurr->GetNElem(); i++)
    {
        // add up values for good scr
        if (!fBadScRCurr->IsBad(i))
            mean += fProjHistos[fIndex]->GetBinContent(i+1);
    }
    mean /= ngood;

    //
    while (kTRUE)
    {
        Int_t scr = -1;
        Double_t value = mean;

        // loop over scaler reads
        for (Int_t i = 0; i < fBadScRCurr->GetNElem(); i++)
        {

            // continue if already bad
            if (fBadScRCurr->IsBad(i)) continue;

            // look for smaler bin value
            if (fProjHistos[fIndex]->GetBinContent(i+1) < value)
            {
                scr = i;
                value = fProjHistos[fIndex]->GetBinContent(i+1);
            }

        }

        // check tolerance
        if (scr >= 0 && value < mean *0.9)
        {
            // set bad scaler read
            SetBadScalerRead(scr);

            // update
            mean = (mean*ngood - fProjHistos[fIndex]->GetBinContent(scr+1))/(ngood-1);
            ngood--;
        }
        else
        {
            break;
        }
    }
}

//______________________________________________________________________________
void TCCalibRunBadScR::UpdateCanvas()
{
    // Updates all canvases.

    // main canvas top
    if (IsGood())
    {
        // top pad

        // set editable
        fCanvasMain->GetPad(1)->SetEditable(kTRUE);
        fCanvasMain->GetPad(2)->SetEditable(kTRUE);
        fCanvasMain->GetPad(3)->SetEditable(kTRUE);

        // set cannot pic flag axis
        fMainHistos[fIndex]->GetXaxis()->SetBit(kCannotPick);
        fMainHistos[fIndex]->GetYaxis()->SetBit(kCannotPick);

        // set title
        fMainHistos[fIndex]->SetTitle(TString::Format("Run %d", fRuns[fIndex]));

        // draw
        fCanvasMain->cd(1);
        fMainHistos[fIndex]->Draw("colzX+");

        // draw boxes
        for (Int_t i = 0; i < fBadScRCurr->GetNElem(); i++)
            if (fBadScRCurrBox[i]) fBadScRCurrBox[i]->Draw("samel");

        // draw last read marker
        fLastReadMarker->Draw();

        // middle pad
        fCanvasMain->cd(2);

        // set same range as main histo
        Int_t minb = fMainHistos[fIndex]->GetXaxis()->GetFirst();
        Int_t maxb = fMainHistos[fIndex]->GetXaxis()->GetLast();
        Double_t min = fMainHistos[fIndex]->GetXaxis()->GetBinLowEdge(minb);
        Double_t max = fMainHistos[fIndex]->GetXaxis()->GetBinUpEdge(maxb);
        fProjHistos[fIndex]->GetXaxis()->SetRangeUser(min, max);

        // set y-range
        fProjHistos[fIndex]->GetYaxis()->SetRangeUser(0., fProjHistos[fIndex]->GetBinContent(fProjHistos[fIndex]->GetMaximumBin())*1.1);

        // set cannot pic flag
        fProjHistos[fIndex]->GetXaxis()->SetBit(kCannotPick);
        fProjHistos[fIndex]->GetYaxis()->SetBit(kCannotPick);

        // draw
        fProjHistos[fIndex]->Draw();

        // bottom pad
        fCanvasMain->cd(3);

        // set same range as main histo
        fProjNormHistos[fIndex]->GetXaxis()->SetRangeUser(min, max);

        // set y-range
        fProjNormHistos[fIndex]->GetYaxis()->SetRangeUser(0., fProjNormHistos[fIndex]->GetBinContent(fProjNormHistos[fIndex]->GetMaximumBin())*1.1);

        // set cannot pic flag
        fProjNormHistos[fIndex]->GetXaxis()->SetBit(kCannotPick);
        fProjNormHistos[fIndex]->GetYaxis()->SetBit(kCannotPick);

        // draw
        fProjNormHistos[fIndex]->Draw();
    }
    else
    {
        // set editable
        fCanvasMain->GetPad(1)->SetEditable(kTRUE);
        fCanvasMain->GetPad(2)->SetEditable(kTRUE);
        fCanvasMain->GetPad(3)->SetEditable(kTRUE);

        // clear pads & set cannot pic flag pad
        fCanvasMain->cd(1);
        fEmptyMainHisto->Draw("colzX+");

        fCanvasMain->cd(2);
        fEmptyProjHisto->Draw();

        fCanvasMain->cd(3);
        fEmptyProjNormHisto->Draw();

        // unset editable
        fCanvasMain->GetPad(1)->SetEditable(kFALSE);
        fCanvasMain->GetPad(2)->SetEditable(kFALSE);
        fCanvasMain->GetPad(3)->SetEditable(kFALSE);
    }

    // update main canvas
    fCanvasMain->Update();

    // update overview canvas
    fCanvasOverview->cd(1);
    fOverviewHisto->Draw();
    fRunMarker->Draw();

    fCanvasOverview->cd(2);
    fOverviewNormHisto->Draw();
    fCanvasOverview->Update();
}

//______________________________________________________________________________
void TCCalibRunBadScR::SaveValCurr()
{
    // Sets 'fBadScRNew[fIndex]' to the current bad scaler reads.

    // check for valid run
    if (!IsGood()) return;

    // delete old
    if (fBadScRNew[fIndex]) delete fBadScRNew[fIndex];

    // create new
    fBadScRNew[fIndex] = new TCBadScRElement(*(fBadScRCurr));

    // print user information
    printf("New: Run number: %i Number of bad scaler reads: %i\n", fRuns[fIndex], fBadScRCurr->GetNBad());
    if (fBadScRCurr->GetNBad() > 0)
    {
        printf("     Bad scaler reads: %d", fBadScRCurr->GetBad()[0]);
        for (Int_t i = 1; i < fBadScRCurr->GetNBad(); i++)
            printf(", %d", fBadScRCurr->GetBad()[i]);
        printf("\n");
    }
}

//______________________________________________________________________________
void TCCalibRunBadScR::SetBadScalerReads(Int_t bscr1, Int_t bscr2)
{
    // (Un)sets the scaler reads from 'bscr1' to 'bscr2'

    // loop over scaler reads and (un)set each of them
    for (Int_t i = bscr1; i <= bscr2; i++) SetBadScalerRead(i);
}

//______________________________________________________________________________
void TCCalibRunBadScR::SetBadScalerRead(Int_t bscr)
{
    // Adds/Removes a bad scaler read to/from the list of bad scaler reeads.

    // check range
    if (bscr < 0 || bscr >= fBadScRCurr->GetNElem()) return;

    // check if was set
    if (fBadScRCurr->IsBad(bscr))
    {
        // remove from list
        fBadScRCurr->RemBad(bscr);

        // delete box
        if (fBadScRCurrBox[bscr]) delete fBadScRCurrBox[bscr];
        fBadScRCurrBox[bscr] = 0;
    }
    else
    {
        // add to list
        fBadScRCurr->AddBad(bscr);

        // get coordinates
        Double_t xlow = fMainHistos[fIndex]->GetXaxis()->GetBinLowEdge(bscr+1);
        Double_t xup  = fMainHistos[fIndex]->GetXaxis()->GetBinUpEdge(bscr+1);
        Double_t ylow = fMainHistos[fIndex]->GetYaxis()->GetXmin();
        Double_t yup  = fMainHistos[fIndex]->GetYaxis()->GetXmax();

        // create box
        fBadScRCurrBox[bscr] = new TBox(xlow, ylow, xup, yup);
        fBadScRCurrBox[bscr]->SetFillStyle(3004);
        fBadScRCurrBox[bscr]->SetFillColor(kRed);
        fBadScRCurrBox[bscr]->SetLineStyle(1);
        fBadScRCurrBox[bscr]->SetLineWidth(1);
        fBadScRCurrBox[bscr]->SetLineColor(kRed);
        fBadScRCurrBox[bscr]->SetBit(kCannotPick);
    }
}

//______________________________________________________________________________
void TCCalibRunBadScR::CleanUpCurr()
{
    // Cleans everything up for the current run and updates overview histogram.

    // delete old boxes
    if (fBadScRCurrBox)
    {
        for (Int_t i = 0; i < fBadScRCurr->GetNElem(); i++)
            if (fBadScRCurrBox[i]) delete fBadScRCurrBox[i];
        delete [] fBadScRCurrBox;
        fBadScRCurrBox = 0;
    }

    // delete old curr bad scaler read
    if (fBadScRCurr) delete fBadScRCurr;

    // set curr to new in order to update overview histo
    fBadScRCurr = fBadScRNew[fIndex];

    // update overview histo
    UpdateOverviewHisto();

    // reset curr
    fBadScRCurr = 0;
}

//______________________________________________________________________________
void TCCalibRunBadScR::UpdateOverviewHisto()
{
    // Updates the overview histo for the current run indexed by 'fIndex', i.e.,
    // sums up the (normalized) counts from 'fProjHisto[fIndex]' for all good
    // scaler reads in 'fBadScRCurr' and divides the result by the number of
    // good scaler reads.

    // reset value
    fOverviewHisto->SetBinContent(fIndex+1, 0);
    fOverviewNormHisto->SetBinContent(fIndex+1, 0);

    // check for valid run
    if (!IsGood()) return;

    // loop over scaler reads
    for (Int_t j = 1; j < fBadScRCurr->GetNElem(); j++)
    {
        // fill overview histo, i.e., add up (normalized) counts for good scaler reads
        if (!fBadScRCurr->IsBad(j))
        {
            fOverviewHisto->AddBinContent(fIndex+1, fProjHistos[fIndex]->GetBinContent(j+1));
            fOverviewNormHisto->AddBinContent(fIndex+1, fProjNormHistos[fIndex]->GetBinContent(j+1));
        }
    }

    // get number of good scaler reads
    Int_t ngoodscr = fBadScRCurr->GetNElem() - fBadScRCurr->GetNBad();

    // devide by number of scaler reads
    if (ngoodscr > 0)
    {
        fOverviewHisto->SetBinContent(fIndex+1, fOverviewHisto->GetBinContent(fIndex+1) / ngoodscr);
        fOverviewNormHisto->SetBinContent(fIndex+1, fOverviewNormHisto->GetBinContent(fIndex+1) / ngoodscr);
    }

    return;
}

//______________________________________________________________________________
Bool_t TCCalibRunBadScR::Write()
{
    // Writes the bad scaler reads 'fBadScRNew' for all runs and the calibration
    // data type 'fCalibData' to the database.

    // check if calibration was started
    if (!fIsStarted)
    {
        Error("Write", "Not yet started!");
        return kFALSE;
    }

    // security check
    if (!fBadScRNew) return kFALSE;

    // init error counter
    Int_t errors = 0;

    // init run counter
    Int_t nruns = 0;

    // loop over runs
    for (Int_t i = 0; i < fNRuns; i++)
    {
        // check for existing bad scaler reads
        if (fBadScRNew[i])
        {
            // increment counter
            nruns++;

            // write bad scaler reads for run with index 'i'
            if (!TCMySQLManager::GetManager()->ChangeRunBadScR(fBadScRNew[i]->GetRunNumber(), fBadScRNew[i]->GetNBad(), fBadScRNew[i]->GetBad(), (*fCalibData).Data()))
            {
                // an error occurred
                Error("Write", "Could not write bad scaler reads for run '%i' to the database!", fRuns[i]);
                errors++;
            }
        }
        else
        {
        }
    }

    // print user info
    if (errors)
    {
        Error("Write", "%i error(s) while writing to the database occurred", errors);
        return kFALSE;
    }
    else
    {
        Info("Write", "Successfully written bad scaler reads of %i runs.", nruns);
    }

    return kTRUE;
}

//______________________________________________________________________________
void TCCalibRunBadScR::ChangeInterval(Int_t key)
{
    // Changes the range of the histos.

    // check for main histo
    if (!IsGood()) return;

    // get current axis binning
    Int_t axis_min = fMainHistos[fIndex]->GetXaxis()->GetFirst();
    Int_t axis_max = fMainHistos[fIndex]->GetXaxis()->GetLast();

    // init new axis binning
    Int_t axis_min_new = axis_min;
    Int_t axis_max_new = axis_max;

    // proccess keys
    switch (key)
    {
        // zoom in/out
        case kKey_Insert:
        {
            if (axis_max - axis_min == fUserInterval - 1)
            {
                // zoom out
                axis_min_new = 1;
                axis_max_new = fRangeMax;
                fUserLastInterval = axis_min;
            }
            else
            {
                // zoom in
                axis_min_new = fUserLastInterval;
                axis_max_new = axis_min_new + fUserInterval - 1;
            }
            break;
        }
        // go to interval with first scaler read
        case kKey_Home:
        {
            axis_min_new = 1;
            axis_max_new = axis_min_new + fUserInterval - 1;
            break;
        }
        // go to interval with last scaler read
        case kKey_End:
        {
            axis_min_new = (fBadScRCurr->GetNElem() / fUserInterval) * fUserInterval + 1;
            axis_max_new = axis_min_new + fUserInterval - 1;
            break;
        }
        // go one interval back
        case kKey_PageUp:
        {
            axis_min_new -= fUserInterval;
            if (axis_min_new <= 0) axis_min_new = 1;
            axis_max_new = axis_min_new + fUserInterval - 1;
            break;
        }
        // go one interval forth
        case kKey_PageDown:
        {
            axis_min_new += fUserInterval;
            axis_max_new = axis_min_new + fUserInterval - 1;
            if (axis_max_new > fBadScRCurr->GetNElem()+2)
            {
                ChangeInterval(kKey_End);
                return;
            }
            break;
        }
        // do something I cannot remenber what
        case kKey_o:
        {

        }
        default:
            return;
    }

    // change range and update canvas
    fMainHistos[fIndex]->GetXaxis()->SetRange(axis_min_new, axis_max_new);

    UpdateCanvas();

    return;
}

//______________________________________________________________________________
void TCCalibRunBadScR::EventHandler(Int_t event, Int_t ox, Int_t oy, TObject* selected)
{
    // Event handler method for (un)setting bad scaler reads.
    //
    // Checks first for the desired event on the desired pad. Then,
    //    i) if a key was pressed, it will change the interval
    //   ii) if the left mouse button was pressed at binx, this value is saved
    //       into 'fLastMouseBin'.
    //  iii) if the left mouse button was released at binx it calls the method
    //       'SetBadScalerReads(fLastMouseBin-1, xbin-1)' in order to (un)set
    //       the bins within this interval.
    // Finally, the canvas update.

    // process parent event handler
    TCCalibRun::EventHandler(event, ox, oy, selected);

    // check for main histo
    if (!IsGood()) return;

    // catch interval navigation keys
    if (event == kKeyPress) ChangeInterval(oy);

    // catch mouse click etc.
    if (!(event == kButton1Down ||
          event == kButton1Up ||
          event == kWheelUp ||
          event == kWheelDown
          )) return;

    // return if no selected pad
    if (!selected) return;

    // get name of selected pad
    TString name = selected->GetName();

    // catch axis zoom (not for MENU)
    if ((event == kButton1Up ||
         event == kWheelUp ||
         event == kWheelDown)
        && name.BeginsWith("xaxis")) UpdateCanvas();

    // check whether main histo was clicked
    if (!name.BeginsWith(fMainHistoName)) return;

    // catch mouse wheel histogram zoom
    if (event == kWheelUp || event == kWheelDown) UpdateCanvas();

    // change to main histo pad
    fCanvasMain->cd(1);

    // get pos & bin
    Int_t px = gPad->GetEventX();
    Double_t upx = gPad->AbsPixeltoX(px);
    Double_t x = gPad->PadtoX(upx);
    Int_t binx = fMainHistos[fIndex]->GetXaxis()->FindBin(x);

    // check for left mouse button press ...
    if (event == kButton1Down)
    {
        // save bin and return
        fLastMouseBin = binx;
        return;
    }
    // ... or release
    else if (event == kButton1Up)
    {
        // update current bad scaler reads
        SetBadScalerReads(fLastMouseBin-1, binx-1);

        // update overview histo
        UpdateOverviewHisto();

        // update canvas
        UpdateCanvas();
    }
}

