/************************************************************************
 * Author: Thomas Strub                                                 *
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCBadScRElement                                                      //
//                                                                      //
// Class containing an array of bad scaler reads.                       //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCBADSCRELEMENT_H
#define TCBADSCRELEMENT_H

#include "TCBadElement.h"

class TCBadScRElement : public TCBadElement
{

protected:
    Char_t fCalibData[256];             // calibration data type
    Int_t fRunNumber;                   // run number

public:
    TCBadScRElement()
      : TCBadElement(),
        fCalibData(),
        fRunNumber(0) { };
    TCBadScRElement(const TCBadScRElement &elem);
    TCBadScRElement(Int_t runno, Int_t nscr = -1)
      : TCBadElement(0, 0, nscr),
        fCalibData(),
        fRunNumber(runno) { };
    TCBadScRElement(Int_t runno, Int_t nbad, const Int_t* bad, Int_t nscr = -1)
      : TCBadElement(nbad, bad, nscr),
        fCalibData(),
        fRunNumber(runno) { };
    virtual ~TCBadScRElement() { };

    Int_t Set(Int_t runno, Int_t nbad, const Int_t* bad, Int_t nscr = -1);

    void SetCalibData(const Char_t* name) { strcpy(fCalibData, name); };
    const Char_t* GetCalibData() const { return fCalibData; };

    Int_t GetRunNumber() const { return fRunNumber; };
    Int_t SetRunNumber(Int_t runno) { return fRunNumber = runno; };

    Int_t GetNScR() const { return GetNElem(); };
    Int_t SetNScR(Int_t nscr) { return SetNElem(nscr); };
    void UnSNScR() { UnSNElem(); };

    ClassDef(TCBadScRElement, 0) // Bad scaler read class
};

#endif

