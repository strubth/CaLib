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


#include "TCBadElement.h"

ClassImp(TCBadElement)

//______________________________________________________________________________
TCBadElement::TCBadElement(const TCBadElement &elem)
{
    // Copy constructor

    // copy members
    fNElem = elem.GetNElem();
    fNBad = elem.GetNBad();
    fBad = new Int_t[fNBad];
    for (Int_t i = 0; i < fNBad; i++)
        fBad[i] = elem.GetBad()[i];
}

//______________________________________________________________________________
TCBadElement::TCBadElement(Int_t nbad, const Int_t* bad, Int_t nelem)
{
    // Constructor

    // init members
    fNElem = nelem;
    fNBad = 0;
    fBad = 0;

    // add the bad elements
    AddBad(nbad, bad);
}

//______________________________________________________________________________
Int_t TCBadElement::MergeNSort(Int_t nbad, const Int_t* bad, Int_t* &bad_sort, Int_t nelem)
{
    // Sorts the array 'bad' with 'nbad' elements, removes doublicate entries
    // and, if 'nelem' >= 0, removes entries which are out of range.
    // Returns the number of elements in the sorted and merged array
    // 'bad_sort'.

    // init temporary return variables
    Int_t nbad_new = 0;
    Int_t* bad_new = new Int_t[nbad];

    // init variables in order to sort and merge
    Int_t lastentry = -1;
    Int_t nextentry = -1;

    // loop over sorted array (max nbad elements)
    for (Int_t i = 0; i < nbad; i++)
    {
        // loop over unsorted array to find next larger entry
        for (Int_t j = 0; j < nbad; j++)
        {
            // check for larger entry
            if (lastentry < bad[j])
            {
                // check for first larger entry
                if(lastentry == nextentry) nextentry = bad[j];

                // check for smaller next entry
                else if (bad[j] < nextentry) nextentry = bad[j];
            }
        }

        // check whether no larger entry was found
        if (lastentry == nextentry) break;

        // set lastentry for next loop
        lastentry = nextentry;

        // check whether next entry is out of range
        if (nelem >= 0 && nextentry >= nelem)
        {
            // print warning
            //Warning("MergeNSort", "Bad element '%d' ignored since it is out of the defined range of [0,%d]!", nextentry, nelem);
        }
        else
        {
            // save next larger entry
            bad_new[nbad_new++] = nextentry;
        }
    }

    // check whether a merge was done
    if (nbad_new != nbad)
    {
       // create new temp array
       Int_t* bad_tmp = new Int_t[nbad_new];

       // copy array
       for (Int_t i = 0; i < nbad_new; i++)
           bad_tmp[i] = bad_new[i];

       // delete array and set pointer
       if (bad_new) delete [] bad_new;
       bad_new = bad_tmp;
    }

    // set pointer to result array & return
    bad_sort = bad_new;

    return nbad_new;
}

//______________________________________________________________________________
Bool_t TCBadElement::IsBad(Int_t bad) const
{
    // Returns kTRUE if 'bad' is in the list of bad elements, returns kFALSE
    // otherwise.

    for (Int_t i = 0; i < fNBad; i++)
    {
        if (fBad[i] < bad) continue;
        if (fBad[i] == bad)
            return kTRUE;
        else
            return kFALSE;
    }

    return kFALSE;
}

//______________________________________________________________________________
Int_t TCBadElement::SetBad(Int_t nbad, const Int_t* bad)
{
    // Sets the number of elements 'fNElem' to 'nbad' and copies the the array
    // of bad elements 'bad' to 'fBad'. Old values will be overwritten. Returns
    // the new number of bad scaler reads.

    // reset number of bad elements
    fNBad = 0;

    // delete old bad element array
    if (fBad) delete [] fBad;
    fBad = 0;

    // add the bad scaler reads
    return AddBad(nbad, bad);
}

//______________________________________________________________________________
Int_t TCBadElement::SetNElem(Int_t nelem)
{
    // Sets the total number of scaler reads and removes entries from the bad
    // scaler list which are out of range. Returns the new number of bad scaler
    // reads.

    // check range
    if (nelem < -1) return -1;

    // set total scaler reads
    fNElem = nelem;

    // save and reset old bad elements
    Int_t nbad_old = fNBad;
    Int_t* bad_old = fBad;
    fNBad = 0;
    fBad = 0;

    // re-merge with new fNElem value
    AddBad(nbad_old, bad_old);

    // delete old array
    if (bad_old) delete [] bad_old;

    // return new number of bad elements
    return fNElem;
}

//______________________________________________________________________________
Int_t TCBadElement::AddBad(Int_t bad)
{
    // Adds the single bad element 'bad' to the list of bad elements. Returns
    // the new number of bad elements.

    Int_t bad_tmp = bad;

    return AddBad(1, &bad_tmp);
}

//______________________________________________________________________________
Int_t TCBadElement::AddBad(Int_t nbad, const Int_t* bad)
{
    // Adds the 'nbad' bad elements of the array 'bad' to the list of bad
    // elements. Returns the new number of bad elements.

    // nothing to do if nbad <= 0
    if (nbad <= 0) return fNBad;

    // set up new array for old & new bad elements
    Int_t nbad_tmp = fNBad + nbad;
    Int_t* bad_tmp = new Int_t[nbad_tmp];

    // fill old bad elements
    for (Int_t i = 0; i < fNBad; i++) bad_tmp[i] = fBad[i];

    // fill new bad elements
    for (Int_t i = 0; i < nbad; i++) bad_tmp[fNBad + i] = bad[i];

    // delete old array
    if (fBad) delete fBad;

    // get sorted & merged array
    Int_t* bad_new = 0;
    Int_t nbad_new = MergeNSort(nbad_tmp, bad_tmp, bad_new, fNElem);

    // set new number of bad elements
    fNBad = nbad_new;

    // set new pointer to bad elements array
    fBad = bad_new;

    // clean up
    if (bad_tmp) delete bad_tmp;

    return fNBad;
}

//______________________________________________________________________________
Int_t TCBadElement::RemBad(Int_t &bad)
{
    // Removes the bad element 'bad' from the list of bad elements. Returns the
    // new number of elements.

    // find index of bad element
    Int_t ind;
    for (ind = 0; ind < fNBad; ind++)
        if (fBad[ind] == bad) break;

    // no change if not in list
    if (ind == fNBad) return fNBad;

    // create new array
    Int_t* bad_new = new Int_t[fNBad - 1];

    // copy old values
    for (Int_t i = 0; i < ind; i++) bad_new[i] = fBad[i];
    for (Int_t i = ind + 1; i < fNBad; i++) bad_new[i-1] = fBad[i];

    // delete old array and set new pointer
    if (fBad) delete fBad;
    fBad = bad_new;

    // return
    return --fNBad;
}

//______________________________________________________________________________
void TCBadElement::RemBad()
{
    // Removes all bad elements.

    // reset members
    fNBad = 0;
    if (fBad) delete [] fBad;
    fBad = 0;
}

