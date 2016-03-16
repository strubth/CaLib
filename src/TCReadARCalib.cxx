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


#include <sstream>
#include <fstream>

#include "TList.h"
#include "TError.h"

#include "TCReadARCalib.h"

ClassImp(TCARElement)
ClassImp(TCARTimeWalk)
ClassImp(TCARNeighbours)
ClassImp(TCReadARCalib)

//______________________________________________________________________________
TCARElement::TCARElement()
    : TObject()
{
    // Constructor.

    fIsTagger = kFALSE;
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
    fTaggCalib = 0;
    fTaggOverlap = 0;
    fTaggScaler = 0;
}

//______________________________________________________________________________
Bool_t TCARElement::Parse(const Char_t* line, Bool_t isTagger)
{
    // Parse the line 'line'.

    // set tagger toggle
    fIsTagger = isTagger;

    // tagger calibration file or not
    if (fIsTagger)
    {
        // read the calibration line
        Int_t n = sscanf(line, "%*s%s%lf%lf%lf%lf%s%lf%lf%lf%lf%lf%lf%lf%lf%lf%d",
                         fADC, &fEnergyLow, &fEnergyHigh, &fPed, &fADCGain,
                         fTDC, &fTimeLow, &fTimeHigh, &fOffset, &fTDCGain,
                         &fX, &fY, &fZ, &fTaggCalib, &fTaggOverlap, &fTaggScaler);

        // check read-in
        return n == 16 ? kTRUE : kFALSE;
    }
    else
    {
        // read the calibration line
        Int_t n = sscanf(line, "%*s%s%lf%lf%lf%lf%s%lf%lf%lf%lf%lf%lf%lf",
                         fADC, &fEnergyLow, &fEnergyHigh, &fPed, &fADCGain,
                         fTDC, &fTimeLow, &fTimeHigh, &fOffset, &fTDCGain,
                         &fX, &fY, &fZ);

        // check read-in
        return n == 13 ? kTRUE : kFALSE;
    }
}

//______________________________________________________________________________
void TCARElement::Format(Char_t* out)
{
    // Format a config line into 'out'.

    // tagger calibration file or not
    if (fIsTagger)
    {
        // write the calibration line
        sprintf(out, "%7s %6.3lf %6.1lf %7.2lf %8.6lf "
                     "%7s %7.1lf %7.1lf %8.2lf %8.6lf "
                     "%8.3lf %8.3lf %8.3lf %8.3lf %5.3lf %3d",
                     fADC, fEnergyLow, fEnergyHigh, fPed, fADCGain,
                     fTDC, fTimeLow, fTimeHigh, fOffset, fTDCGain,
                     fX, fY, fZ, fTaggCalib, fTaggOverlap, fTaggScaler);
    }
    else
    {
        // write the calibration line
        sprintf(out, "%7s %6.3lf %6.1lf %7.2lf %8.6lf "
                     "%7s %7.1lf %7.1lf %8.2lf %8.6lf "
                     "%8.3lf %8.3lf %8.3lf",
                     fADC, fEnergyLow, fEnergyHigh, fPed, fADCGain,
                     fTDC, fTimeLow, fTimeHigh, fOffset, fTDCGain,
                     fX, fY, fZ);
    }
}

//______________________________________________________________________________
Bool_t TCARTimeWalk::Parse(const Char_t* line)
{
    // Parse the line 'line'.

    // read the calibration line
    Int_t n = sscanf(line, "%*s%d%lf%lf%lf%lf",
                           &fIndex, &fPar0, &fPar1, &fPar2, &fPar3);

    // check read-in
    return n == 5 ? kTRUE : kFALSE;
}

//______________________________________________________________________________
void TCARTimeWalk::Format(Char_t* out)
{
    // Format a config line into 'out'.

    // write the calibration line
    sprintf(out, "%3d %11.6lf %11.6lf %11.6lf %11.6lf",
                 fIndex, fPar0, fPar1, fPar2, fPar3);
}

//______________________________________________________________________________
Bool_t TCARNeighbours::Parse(const Char_t* line)
{
    // Parse the line 'line'.

    // create stringstream
    std::istringstream iss(line);

    // skip tag
    std::string tag;
    iss >> tag;

    // read number of neighbours
    iss >> fNneighbours;

    // create array
    if (fNeighbours) delete [] fNeighbours;
    fNeighbours = new Int_t[fNneighbours];

    // read neighbours
    for (Int_t i = 0; i < fNneighbours; i++)
        iss >> fNeighbours[i];

    return kTRUE;
}

//______________________________________________________________________________
TCReadARCalib::TCReadARCalib(const Char_t* calibFile, Bool_t isTagger,
                             const Char_t* elemIdent, const Char_t* nebrIdent)
{
    // Constructor using the calibration file 'calibFile', the element
    // identifier 'elemIdent' and the next-neighbour identifier 'nebrIdent'.
    // 'isTagger' has to be kTRUE for tagger calibration files.

    // init members
    fElements = new TList();
    fElements->SetOwner(kTRUE);
    fTimeWalks = new TList();
    fTimeWalks->SetOwner(kTRUE);
    fNeighbours = new TList();
    fNeighbours->SetOwner(kTRUE);

    // read the calibration file
    ReadCalibFile(calibFile, isTagger, elemIdent, nebrIdent);
}

//______________________________________________________________________________
TCReadARCalib::~TCReadARCalib()
{
    // Destructor.

    if (fElements) delete fElements;
    if (fTimeWalks) delete fTimeWalks;
    if (fNeighbours) delete fNeighbours;
}

//______________________________________________________________________________
Int_t TCReadARCalib::GetNelements() const
{
    return fElements ? fElements->GetSize() : 0;
}

//______________________________________________________________________________
TCARElement* TCReadARCalib::GetElement(Int_t n) const
{
    return fElements ? (TCARElement*)fElements->At(n) : 0;
}

//______________________________________________________________________________
Int_t TCReadARCalib::GetNtimeWalks() const
{
    return fTimeWalks ? fTimeWalks->GetSize() : 0;
}

//______________________________________________________________________________
TCARTimeWalk* TCReadARCalib::GetTimeWalk(Int_t n) const
{
    return fTimeWalks ? (TCARTimeWalk*)fTimeWalks->At(n) : 0;
}

//______________________________________________________________________________
Int_t TCReadARCalib::GetNneighbours() const
{
    return fNeighbours ? fNeighbours->GetSize() : 0;
}

//______________________________________________________________________________
TCARNeighbours* TCReadARCalib::GetNeighbour(Int_t n) const
{
    return fNeighbours ? (TCARNeighbours*)fNeighbours->At(n) : 0;
}

//______________________________________________________________________________
void TCReadARCalib::ReadCalibFile(const Char_t* filename, Bool_t isTagger,
                                  const Char_t* elemIdent, const Char_t* nebrIdent)
{
    // Read the calibration file 'filename'.

    // open the file
    std::ifstream infile;
    infile.open(filename);

    // check if file is open
    if (!infile.is_open())
    {
        Error("ReadCalibFile", "Could not open calibration file '%s'", filename);
    }
    else
    {
        Info("ReadCalibFile", "Reading calibration file '%s'", filename);

        // read the file
        while (infile.good())
        {
            TString line;
            line.ReadLine(infile);

            // trim line
            line.Remove(TString::kBoth, ' ');

            // search element statements
            if (line.BeginsWith(elemIdent))
            {
                // create element
                TCARElement* elem = new TCARElement();

                // try to read parameters
                if (elem->Parse(line, isTagger))
                {
                    // add element to list
                    fElements->Add(elem);
                }
                else
                {
                    Error("ReadCalibFile", "Could not read element in "
                          "calibration file '%s'", filename);
                }
            }
            // search time walk statements
            else if (line.BeginsWith("TimeWalk:"))
            {
                // create time walk
                TCARTimeWalk* tw = new TCARTimeWalk();

                // try to read parameters
                if (tw->Parse(line))
                {
                    // add time walk to list
                    fTimeWalks->Add(tw);
                }
                else
                {
                    Error("ReadCalibFile", "Could not read time walk in "
                          "calibration file '%s'", filename);
                }
            }
            // search neighbours statements
            else if (line.BeginsWith(nebrIdent))
            {
                // create element
                TCARNeighbours* elem = new TCARNeighbours();

                // try to read parameters
                if (elem->Parse(line))
                {
                    // add element to list
                    fNeighbours->Add(elem);
                }
                else
                {
                    Error("ReadCalibFile", "Could not read neighbours in "
                          "calibration file '%s'", filename);
                }
            }
        }
    }

    // close the file
    infile.close();
}

