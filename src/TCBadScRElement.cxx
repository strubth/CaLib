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


#include "TCBadScRElement.h"

ClassImp(TCBadScRElement)

//______________________________________________________________________________
TCBadScRElement::TCBadScRElement(const TCBadScRElement &elem)
  : TCBadElement(elem)
{
    // Copy constructor

    // copy members
    strcpy(fCalibData, elem.fCalibData);
    fRunNumber = elem.GetRunNumber();
}

//______________________________________________________________________________
Int_t TCBadScRElement::Set(Int_t runno, Int_t nbad, const Int_t* bad, Int_t nscr)
{
    // Sets run number to 'runno', number of bad scaler reads to 'nbad' and the
    // list of bad scaler reads to 'bad'. Old values will be overwritten.
    // Returns the new number of bad scaler reads set.

    // set run number
    fRunNumber = runno;

    // set total number of scaler reads
    fNElem = nscr;

    // set bad scaler reads
    return SetBad(nbad, bad);
}

