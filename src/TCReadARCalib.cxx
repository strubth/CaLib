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


#include "TCReadARCalib.h"

ClassImp(TCReadARCalib)


//______________________________________________________________________________
TCReadARCalib::TCReadARCalib(const Char_t* calibFile, 
                             const Char_t* elemIdent,
                             const Char_t* nebrIdent)
{
    // Constructor using the calibration file 'calibFile', the element
    // identifier 'elemIdent' and the next-neighbour identifier 'nebrIdent'.
    
    // init members
    fElements = new TList();
    fElements->SetOwner(kTRUE);
    fNeighbours = new TList();
    fNeighbours->SetOwner(kTRUE);
    
    // read the calibration file
    ReadCalibFile(calibFile, elemIdent, nebrIdent);
}

//______________________________________________________________________________
TCReadARCalib::~TCReadARCalib()
{
    // Destructor.
    
    if (fElements) delete fElements;
    if (fNeighbours) delete fNeighbours;
}

//______________________________________________________________________________
void TCReadARCalib::ReadCalibFile(const Char_t* filename, 
                                  const Char_t* elemIdent,
                                  const Char_t* nebrIdent)
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
                if (elem->Parse(line))
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

