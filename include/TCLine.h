/************************************************************************
 * Author: Thomas Strub                                                 *
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCLine                                                               //
//                                                                      //
// Always horizontal/vertical indicator line with cool move.            //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCLINE_H
#define TCLINE_H

#include "TLine.h"
class TH1;

class TCLine : public TLine
{

public:
    enum ETCLineOrientation { kVertical, kHorizontal };
    enum ETCLineMoving { kLineMovingFree, kLineMovingBinCenter, kLineMovingBinBorder };

protected:
    ETCLineOrientation fLineOrientation;
    ETCLineMoving fLineMoving;

    virtual void Init();
    virtual ETCLineOrientation GetLineOrientation() const;
    virtual TH1* GetHisto() const;
    virtual void UpdateLength();


public:
    TCLine()
      : TLine() { Init(); };
    TCLine(ETCLineOrientation lo, Double_t pos)
      : TLine() { Init(); fLineOrientation = lo; SetPos(pos); };
    TCLine(const TCLine& l)
      : TLine(l), fLineOrientation(l.fLineOrientation), fLineMoving(l.fLineMoving) { };
    virtual ~TCLine() { };

    virtual void Copy(TObject& obj) const;

    virtual Bool_t IsVertical() const { return GetLineOrientation() == kVertical; }
    virtual Bool_t IsHorizontal() const { return GetLineOrientation() == kHorizontal; }

    virtual void SetVertical(int = 0) { SetVertical(GetPos()); }; // *TOGGLE* *GETTER=IsVertical
    virtual void SetVertical(Double_t pos);
    virtual void SetHorizontal(int = 0) { SetHorizontal(GetPos()); }; // *TOGGLE* *GETTER=IsHorizontal
    virtual void SetHorizontal(Double_t pos);

    virtual void SetX1(Double_t x1) { IsVertical() ? SetPos(x1) : TLine::SetX1(x1); }
    virtual void SetX2(Double_t x2) { IsVertical() ? SetPos(x2) : TLine::SetX2(x2); }
    virtual void SetY1(Double_t y1) { IsHorizontal() ? SetPos(y1) : TLine::SetY1(y1); }
    virtual void SetY2(Double_t y2) { IsHorizontal() ? SetPos(y2) : TLine::SetY2(y2); }

    virtual Double_t GetPos() const { return IsVertical() ? GetX1() : GetY1(); }
    virtual void SetPos(Double_t pos) { IsVertical() ? SetVertical(pos) : SetHorizontal(pos); } // *MENU*

    virtual void SetLineMovingFree() { fLineMoving = kLineMovingFree; };
    virtual void SetLineMovingBinCenter() { fLineMoving = kLineMovingBinCenter; };
    virtual void SetLineMovingBinBorder() { fLineMoving = kLineMovingBinBorder; };

    virtual void Paint(Option_t*) { UpdateLength(); TLine::Paint(); };
    virtual void ExecuteEvent(Int_t event, Int_t px, Int_t py);

    ClassDef(TCLine, 0);
};


class TCLineH : public TCLine
{

public:
    TCLineH(Double_t pos = 0)
      : TCLine(kHorizontal, pos) { }
    virtual ~TCLineH() { };

    ClassDef(TCLineH, 0);
};


class TCLineV : public TCLine
{

public:
    TCLineV(Double_t pos = 0)
      : TCLine(kVertical, pos) { }
    virtual ~TCLineV() { };

    ClassDef(TCLineV, 0);
};


#endif
