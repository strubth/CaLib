/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibType                                                          //
//                                                                      //
// Calibration type class.                                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTYPE_H
#define TCCALIBTYPE_H

#include "TNamed.h"

class TCCalibData;

class TCCalibType : public TNamed
{

private:
    TList* fData;                   // list of associated data (enties not owned)

public:
    TCCalibType() : TNamed() { }
    TCCalibType(const Char_t* name, const Char_t* title);
    virtual ~TCCalibType();

    TList* GetData() const { return fData; }
    TCCalibData* GetData(Int_t n);

    void AddData(TCCalibData* data);
    virtual void Print(Option_t* option = "") const;

    ClassDef(TCCalibType, 1) // Calibration type class
};

#endif

