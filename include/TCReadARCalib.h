// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCReadARCalib                                                        //
//                                                                      //
// Read AcquRoot calibration files.                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCREADARCALIB_H
#define TCREADARCALIB_H

#include <fstream>

#include "TSystem.h"
#include "TString.h"
#include "TList.h"
#include "TError.h"


class TCARElement : public TObject
{

private:
    Char_t fADC[16];                        // ADC identifier
    Double_t fEnergyLow;                    // energy low threshold
    Double_t fEnergyHigh;                   // energy high threshold
    Double_t fPed;                          // ADC pedestal
    Double_t fADCGain;                      // ADC gain
    Char_t fTDC[16];                        // TDC identifier
    Double_t fTimeLow;                      // time low threshold
    Double_t fTimeHigh;                     // time high threshold
    Double_t fOffset;                       // TDC offset
    Double_t fTDCGain;                      // TDC gain
    Double_t fX;                            // x coordinate
    Double_t fY;                            // y coordinate
    Double_t fZ;                            // z coordinate

public:
    TCARElement() 
    {
        fADC[0] = '\0';
        fEnergyLow = 0;
        fEnergyHigh = 0;
        fPed = 0;
        fADCGain = 0;
        fTDC[0] = '\0';
        fTimeLow = 0;
        fTimeHigh = 0;
        fOffset = 0;
        fTDCGain = 0;
        fX = 0;
        fY = 0;
        fZ = 0;
    }
    virtual ~TCARElement() { }

    Bool_t Parse(const Char_t* line)
    {
        // read the calibration line
        Int_t n = sscanf(line, "%*s%s%lf%lf%lf%lf%s%lf%lf%lf%lf%lf%lf%lf", 
                         fADC, &fEnergyLow, &fEnergyHigh, &fPed, &fADCGain, 
                         fTDC, &fTimeLow, &fTimeHigh, &fOffset, &fTDCGain, 
                         &fX, &fY, &fZ);

        // check read-in
        return n == 13 ? kTRUE : kFALSE;
    }

    void SetADC(const Char_t* adc) { strcpy(fADC, adc); }
    void SetEnergyLow(Double_t low) { fEnergyLow = low; }
    void SetEnergyHigh(Double_t high) { fEnergyHigh = high; }
    void SetPedestal(Double_t ped) { fPed = ped; }
    void SetADCGain(Double_t gain) { fADCGain = gain; }
    void SetTDC(const Char_t* tdc) { strcpy(fTDC, tdc); }
    void SetTimeLow(Double_t low) { fEnergyLow = low; }
    void SetTimeHigh(Double_t high) { fEnergyHigh = high; }
    void SetOffset(Double_t off) { fOffset = off; }
    void SetTDCGain(Double_t gain) { fTDCGain = gain; }
    void SetX(Double_t x) { fX = x; }
    void SetY(Double_t y) { fY = y; }
    void SetZ(Double_t z) { fZ = z; }

    const Char_t* GetADC() const { return fADC; }
    Double_t GetEnergyLow() const { return fEnergyLow; }
    Double_t GetEnergyHigh() const { return fEnergyHigh; }
    Double_t GetPedestal() const { return fPed; }
    Double_t GetADCGain() const { return fADCGain; }
    const Char_t* GetTDC() const { return fTDC; }
    Double_t GetTimeLow() const { return fTimeLow; }
    Double_t GetTimeHigh() const { return fTimeHigh; }
    Double_t GetOffset() const { return fOffset; }
    Double_t GetTDCGain() const { return fTDCGain; }
    Double_t GetX() const { return fX; }
    Double_t GetY() const { return fY; }
    Double_t GetZ() const { return fZ; }
};


class TCReadARCalib
{

private:
    TList* fElements;               // list of detector elements
    
    void ReadCalibFile(const Char_t* filename, const Char_t* elemIdent);

public:
    TCReadARCalib() : fElements(0) { }
    TCReadARCalib(const Char_t* calibFile, const Char_t* elemIdent = "Element:");
    virtual ~TCReadARCalib();
    
    TList* GetElements() const { return fElements; }
    Int_t GetNelements() const { return fElements ? fElements->GetSize() : 0; }

    ClassDef(TCReadARCalib, 0) // AcquRoot calibration file reader
};

#endif

