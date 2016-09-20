/******************************************************************************
 * Author: Thomas Strub                                                       *
 ******************************************************************************/

////////////////////////////////////////////////////////////////////////////////
//
// TCLine
//
// Always horizontal/vertical indicator line with cool move.
//
// Includes some work-arrounds for buggy/unsatisfying TLine, i.e.,
//   (a) TCLine::IsVertical/IsHorizontal are now virtual & const functions,
//   (b) TCLine::SetVertical/SetHorizontal can be applied before line is drawn,
//   (c) TCLine::SetVertical/SetHorizontal are now virtual function.
//
// Have fun!
//
////////////////////////////////////////////////////////////////////////////////


#include "TCLine.h"
#include "TROOT.h"
#include "TIterator.h"
#include "TH1.h"
#include "TMath.h"
#include "TCanvas.h"

ClassImp(TCLine)

//______________________________________________________________________________
TCLine::ETCLineOrientation TCLine::GetLineOrientation() const
{
    // Returns the line orientation. Work-around for non virtual setter
    // SetHorizontal/SetVertical and non const getter (IsHorizontal/
    // IsVertical) of TLine.

    if (TestBit(TLine::kVertical) || TestBit(TLine::kHorizontal))
    {
        // check
        if (TestBit(TLine::kVertical))
            return kVertical;
        else //if (TLine::IsHorizontal())
            return kHorizontal;
    }

    // return line orientation
    return fLineOrientation;
}

//______________________________________________________________________________
TH1* TCLine::GetHisto() const
{
    // Returns the first histo (if any) of the current pad (if any).

    // init
    TH1* h = 0;

    // find first histo in current pad
    TIter next(gPad->GetListOfPrimitives());
    TObject* obj;
    while ((obj = next()))
    {
        if (obj->InheritsFrom("TH1"))
        {
            h = (TH1*) obj;
            break;
        }
    }

    return h;
}

//______________________________________________________________________________
void TCLine::Init()
{
    // Initialize line

    // format line
    SetLineWidth(2);
    SetLineColor(2);

    //TCLine::SetVertical(0)
    SetBit(TLine::kVertical, kTRUE);
    fLineOrientation = kVertical;
    fLineMoving = kLineMovingFree;
}

//______________________________________________________________________________
void TCLine::UpdateLength()
{
    // Sets the length of the line according to the current pads height/width.

    // declare pad ranges
    Double_t x1, y1, x2, y2;

    // check for pad
    if (!gPad) return;

    // get pad ranges
    gPad->GetRangeAxis(x1, y1, x2, y2);

    if (IsVertical())
    {
        fX1 = fX2;

        fY1 = y1;
        fY2 = y2;
    }
    else //if (IsHorizontal())
    {
        fY1 = fY2;

        fX1 = x1;
        fX2 = x2;
    }

    return;
}

//______________________________________________________________________________
void TCLine::Copy(TObject& obj) const
{
    // Copy this to 'obj'.

    // call parent copy
    TLine::Copy(obj);

    // copy members save
    if (obj.InheritsFrom("TCLine"))
    {
        ((TCLine&) obj).fLineOrientation = fLineOrientation;
        ((TCLine&) obj).fLineMoving = fLineMoving;
    }
}

//______________________________________________________________________________
void TCLine::SetVertical(Double_t pos)
{
    // Sets the line vertical at position 'pos'.

    SetBit(TLine::kHorizontal, kFALSE);
    SetBit(TLine::kVertical, kTRUE);

    fLineOrientation = kVertical;

    //fY1 = fY2;
    fX2 = fX1 = pos;
}

//______________________________________________________________________________
void TCLine::SetHorizontal(Double_t pos)
{
    // Sets the line horizontal at position 'pos'.

    SetBit(TLine::kVertical, kFALSE);
    SetBit(TLine::kHorizontal, kTRUE);

    fLineOrientation = kHorizontal;

    //fX1 = fX2;
    fY2 = fY1 = pos;
}

//______________________________________________________________________________
void TCLine::ExecuteEvent(Int_t event, Int_t cpx, Int_t cpy)
{
    // Execute action corresponding to one event.
    // This function overloads the one of TLine.
    // Opaque moving is not (yet) supported.

    // approximate difference from endpoints in terms of pixels
    const Int_t kMaxDiff = 20;

    // old cursor pos
    static Int_t cpxold, cpyold;

    // line points
    static Int_t lpx1, lpx2, lpy1, lpy2;

    // old line points
    static Int_t lpx1old, lpy1old, lpx2old, lpy2old;

    static Double_t x1new, x2new, y1new, y2new;


    // picked flag (point1, point2, line)
    static Bool_t isP1, isP2, isL;

    // moved flag
    static Bool_t moved;

    // opaque mode
    static Bool_t opaque;

    // histo
    static TH1* h;

    // helpers
    Int_t dx1, dx2, dy1, dy2;
    Int_t d1, d2;

    // do nothing if not editable
    if (!gPad->IsEditable()) return;

    switch (event)
    {

    // process line was left klicked
    case kButton1Down:
        // init movement flag
        moved = kFALSE;

        // Change line attributes only if necessary
        TAttLine::Modify();

        // set line style
        gVirtualX->SetLineColor(-1); // inverse color
        gVirtualX->SetLineStyle(2);  // dashed

        // save and change opaque mode
        opaque = gPad->OpaqueMoving();
        gPad->GetCanvas()->MoveOpaque(kFALSE);

        // init pointer to histo
        h = 0;

    // process mouse motion
    case kMouseMotion:
        // it follows the ROOT standard code to check to check whether the line
        // was picked at one of the end points of the line itself. This is
        // currently not used, but left here for later use.

        // save position
        if (TestBit(kLineNDC))
        {
            lpx1 = gPad->UtoPixel(fX1);
            lpy1 = gPad->VtoPixel(fY1);
            lpx2 = gPad->UtoPixel(fX2);
            lpy2 = gPad->VtoPixel(fY2);
        }
        else
        {
            lpx1 = gPad->XtoAbsPixel(gPad->XtoPad(fX1));
            lpy1 = gPad->YtoAbsPixel(gPad->YtoPad(fY1));
            lpx2 = gPad->XtoAbsPixel(gPad->XtoPad(fX2));
            lpy2 = gPad->YtoAbsPixel(gPad->YtoPad(fY2));
        }

        // reset flags
        isP1 = isP2 = isL = kFALSE;

        // simply calc difference to point no. 1
        d1  = abs(lpx1 - cpx) + abs(lpy1 - cpy);

        // check difference
        if (d1 < kMaxDiff)
        {
            // save old point no. 1
            lpx1old = lpx1;
            lpy1old = lpy1;

            // set point no. 1 flag
            isP1 = kTRUE;

            gPad->SetCursor(kPointer);
            return;
        }

        // simply calc difference to point no. 2
        d2  = abs(lpx2 - cpx) + abs(lpy2 - cpy);

        // check difference
        if (d2 < kMaxDiff)
        {
            // save old point no. 2
            lpx2old = lpx2;
            lpy2old = lpy2;

            // set point no. 2 flag
            isP2 = kTRUE;

            gPad->SetCursor(kPointer);
            return;
        }

        // set line flag
        isL = kTRUE;

        // save old cursor position
        cpxold = cpx;
        cpyold = cpy;

        gPad->SetCursor(kMove);

        return;

    // process line was moved while left clicked
    case kButton1Motion:

        // check whether line was moved before
        if (!moved)
        {
            // init movement flag
            moved = kTRUE;
        }
        else
        {
            // 'delete' old line
            gVirtualX->DrawLine(lpx1, lpy1, lpx2, lpy2);
        }

        // get maximal range values
        Double_t xmin, ymin, xmax, ymax;
        gPad->GetRangeAxis(xmin, ymin, xmax, ymax);

        // get position in terms of pixels
        if (TestBit(kLineNDC))
        {
            xmin = gPad->UtoPixel(xmin);
            xmax = gPad->UtoPixel(xmax);
            ymin = gPad->VtoPixel(ymin);
            ymax = gPad->VtoPixel(ymax);
        }
        else
        {
            xmin = gPad->XtoAbsPixel(gPad->XtoPad(xmin));
            xmax = gPad->XtoAbsPixel(gPad->XtoPad(xmax));
            ymin = gPad->YtoAbsPixel(gPad->YtoPad(ymin));
            ymax = gPad->YtoAbsPixel(gPad->YtoPad(ymax));
        }

        // current bring cursor position into histo range
        if (cpx < xmin) cpx = xmin;
        if (cpx > xmax) cpx = xmax;
        if (cpy > ymin) cpy = ymin;
        if (cpy < ymax) cpy = ymax;

        // init movement
        dx2 = dx1 = 0;
        dy2 = dy1 = 0;

        // calculate movement (i.e., dx1, dx2, dy1, dy2)

        // free
        if (fLineMoving == kLineMovingFree)
        {
            if (IsVertical())
            {
                dx2 = dx1 = cpx - cpxold;
            }
            else //if (IsHorizontal())
            {
                dy2 = dy1 = cpy - cpyold;
            }
        }
        // bin center
        else if (fLineMoving == kLineMovingBinCenter)
        {
            // find first histo in current pad
            if (!h)
            {
                h = GetHisto();

                // check whether hist was found
                if (!h)
                {
                    Error("ExecuteEvent", "No histogram found!");
                }
            }

            if (h)
            {
                if (IsVertical())
                {
                    // get bins
                    Int_t binold = h->GetXaxis()->FindBin(gPad->PadtoX(gPad->AbsPixeltoX(lpx1)));
                    Int_t binnew = h->GetXaxis()->FindBin(gPad->PadtoX(gPad->AbsPixeltoX(cpx)));

                    if (binold != binnew)
                    {
                        x2new = x1new = h->GetXaxis()->GetBinCenter(binnew);
                        dx2 = dx1 = gPad->XtoAbsPixel(gPad->XtoPad(x1new)) - lpx1;
                    }
                }
                else //if (IsHorizontal())
                {
                    // get bins
                    Int_t binold = h->GetYaxis()->FindBin(gPad->PadtoY(gPad->AbsPixeltoY(lpy1)));
                    Int_t binnew = h->GetYaxis()->FindBin(gPad->PadtoY(gPad->AbsPixeltoY(cpy)));

                    if (binold != binnew)
                    {
                        y2new = y1new = h->GetYaxis()->GetBinCenter(binnew);
                        dy2 = dy1 = gPad->YtoAbsPixel(gPad->YtoPad(y1new)) - lpy1;
                    }
                }
            }
        }
        // bin border
        else if (fLineMoving == kLineMovingBinBorder)
        {
            // find first histo in current pad
            if (!h)
            {
                h = GetHisto();

                // check whether hist was found
                if (!h)
                {
                    Error("ExecuteEvent", "No histogram found!");
                }
            }

            if (h)
            {
                if (IsVertical())
                {
                    // pos
                    Double_t pos = gPad->PadtoX(gPad->AbsPixeltoX(cpx));

                    // get bins
                    Int_t binnew = h->GetXaxis()->FindBin(pos);

                    // get edges
                    Double_t lo = h->GetXaxis()->GetBinLowEdge(binnew);
                    Double_t hi = h->GetXaxis()->GetBinUpEdge(binnew);

                    if (pos-lo < hi-pos)
                    {
                        x2new = x1new = lo;
                        dx2 = dx1 = gPad->XtoAbsPixel(gPad->XtoPad(x1new)) - lpx1;
                    }
                    else
                    {
                        x2new = x1new = hi;
                        dx2 = dx1 = gPad->XtoAbsPixel(gPad->XtoPad(x1new)) - lpx1;
                    }
                }
                else //if (IsHorizontal())
                {
                    // pos
                    Double_t pos = gPad->PadtoY(gPad->AbsPixeltoY(cpy));

                    // get bins
                    Int_t binnew = h->GetYaxis()->FindBin(pos);

                    // get edges
                    Double_t lo = h->GetYaxis()->GetBinLowEdge(binnew);
                    Double_t hi = h->GetYaxis()->GetBinUpEdge(binnew);

                    if (pos-lo < hi-pos)
                    {
                        y2new = y1new = lo;
                        dy2 = dy1 = gPad->YtoAbsPixel(gPad->YtoPad(y1new)) - lpy1;
                    }
                    else
                    {
                        y2new = y1new = hi;
                        dy2 = dy1 = gPad->YtoAbsPixel(gPad->YtoPad(y1new)) - lpy1;
                    }
                }
            }
        }

        // set new position
        lpx1 += dx1;
        lpx2 += dx2;
        lpy1 += dy1;
        lpy2 += dy2;

        // draw new line
        gVirtualX->DrawLine(lpx1, lpy1, lpx2, lpy2);

        // save current cursor position
        cpxold = cpx;
        cpyold = cpy;

        break;

    case kButton1Up: // check out the new code

        // reset opaque mode
        gPad->GetCanvas()->MoveOpaque(opaque);

        if (gROOT->IsEscaped())
        {
            gROOT->SetEscape(kFALSE);
            break;
        }

        // set new line points
        if (fLineMoving == kLineMovingFree)
        {
            fX1 = gPad->PadtoX(gPad->AbsPixeltoX(lpx1));
            fY1 = gPad->PadtoY(gPad->AbsPixeltoY(lpy1));
            fX2 = gPad->PadtoX(gPad->AbsPixeltoX(lpx2));
            fY2 = gPad->PadtoY(gPad->AbsPixeltoY(lpy2));
        }
        else if (fLineMoving == kLineMovingBinCenter && h)
        {
            if (IsVertical())
            {
                Int_t bin = h->GetXaxis()->FindBin(gPad->PadtoX(gPad->AbsPixeltoX(lpx1)));
                fX2 = fX1 = h->GetXaxis()->GetBinCenter(bin);
            }
            else //if (IskHorizontal())
            {
                Int_t bin = h->GetYaxis()->FindBin(gPad->PadtoY(gPad->AbsPixeltoY(lpy1)));
                fY2 = fY1 = h->GetYaxis()->GetBinCenter(bin);
            }
        }
        else if (fLineMoving == kLineMovingBinBorder && h)
        {
            if (IsVertical())
            {
                fX2 = fX1 = x1new;
            }
            else //if (IskHorizontal())
            {
                fY2 = fY1 = y1new;
            }
        }

        gPad->Modified(kTRUE);
        gVirtualX->SetLineColor(-1);

        // reset line style ?
        // gVirtualX->SetLineStyle(<int oldstyle>);

        break;

    // do not get the following lines
    case kButton1Locate:

        ExecuteEvent(kButton1Down, cpx, cpy);
        while (1) {
            cpx = cpy = 0;
            event = gVirtualX->RequestLocator(1, 1, cpx, cpy);
            ExecuteEvent(kButton1Motion, cpx, cpy);

            if (event != -1)
            {
                // button is released
                ExecuteEvent(kButton1Up, cpx, cpy);
                return;
            }
        }
    } // end switch
}

