/************************************************************************
 * Author: Thomas Strub                                                 *
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCBadElement                                                         //
//                                                                      //
// Class containing an array of bad elements.                           //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCBADELEMENT_H
#define TCBADELEMENT_H

#include "Rtypes.h"

class TCBadElement
{

protected:
    Int_t fNElem;                       // total number of elements
    Int_t fNBad;                        // number of bad elements
    Int_t* fBad;              //[fNBad] // list of bad elements

    static Int_t MergeNSort(Int_t nbad, const Int_t* bad, Int_t* &bad_sort, Int_t nelem = -1);

public:
    TCBadElement()
      : fNElem(-1),
        fNBad(0),
        fBad(0) { };
    TCBadElement(const TCBadElement &elem);
    TCBadElement(Int_t nbad, const Int_t* bad, Int_t nelem = -1);
    virtual ~TCBadElement() { if (fBad) delete [] fBad; };

    Int_t GetNElem() const { return fNElem; };
    Int_t GetNBad() const { return fNBad; };
    const Int_t* GetBad() const { return fBad; };

    Int_t SetNElem(Int_t nelem);
    Int_t SetBad(Int_t nbad, const Int_t* bad);

    void UnSNElem() { fNElem = -1; };

    Bool_t IsBad(Int_t bad) const;

    Int_t AddBad(const Int_t bad);
    Int_t AddBad(const Int_t nbad, const Int_t* bad);

    Int_t RemBad(Int_t &bad);
    void RemBad();

    ClassDef(TCBadElement, 0) // Bad element class
};

#endif

