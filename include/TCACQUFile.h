/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCACQUFile                                                           //
//                                                                      //
// Read raw ACQU MK1/2 files.                                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCACQUFILE_H
#define TCACQUFILE_H

#include "TObject.h"

// Mk1 format header field sizes
enum EMk1HeaderSize
{
    kMk1Marker    =   4,
    kMk1SizeTime  =  26,
    kMk1SizeDesc  = 133,
    kMk1SizeRNote = 133,
    kMk1SizeFName =  40
};

// Mk2 format header field sizes
enum EMk2HeaderSize
{
    kMk2Marker    =   8,
    kMk2SizeTime  =  32,
    kMk2SizeDesc  = 256,
    kMk2SizeRNote = 256,
    kMk2SizeFName = 128
};

// type of raw files
enum ERawFileType
{
    kFileBad,
    kFileUnComp,
    kFileGZ,
    kFileXZ
};
typedef ERawFileType RawFileType_t;

// types of raw file formats
enum ERawFileFormat
{
    kRawUnknown,
    kRawMk1,
    kRawMk2
};
typedef ERawFileFormat RawFileFormat_t;

class TCACQUFile : public TObject
{

private:
    RawFileFormat_t fFormat;                // raw file format
    Char_t fTime[kMk2SizeTime];             // run start time (ascii)
    Char_t fDescription[kMk2SizeDesc];      // description of experiment
    Char_t fRunNote[kMk2SizeRNote];         // particular run note
    Char_t fOutFile[kMk2SizeFName];         // output file
    UShort_t fRun;                          // run number
    Long64_t fSize;                         // file size in bytes
    Char_t fFileName[256];                  // actual filename

    RawFileType_t CheckFileType(const Char_t* file);
    RawFileFormat_t CheckFileFormat(const Char_t* hdr);
    FILE* OpenFile(const Char_t* file, RawFileType_t type);
    void CloseFile(FILE* file, RawFileType_t type);
    void RemoveControlChars(Char_t* string);
    void ParseHeader(const Char_t* buffer, RawFileFormat_t format);

public:
    TCACQUFile();
    virtual ~TCACQUFile() { }

    void ReadFile(const Char_t* path, const Char_t* fname);
    virtual void Print(Option_t* option = "") const;
    void PrintListing() { printf("%s\t%s\t%s\t%s\t%lld\n", fFileName, fTime, fDescription, fRunNote, fSize); }

    Bool_t IsGoodDataFile()
    {
        if (fFormat == kRawMk1) return kTRUE;
        else if (fFormat == kRawMk2) return kTRUE;
        else return kFALSE;
    }

    const Char_t* GetTime() const { return fTime; }
    const Char_t* GetDescription() const { return fDescription; }
    const Char_t* GetRunNote() const { return fRunNote; }
    const Char_t* GetOutFile() const { return fOutFile; }
    UShort_t GetRun() const { return fRun; }
    Long64_t GetSize() const { return fSize; }
    const Char_t* GetFileName() const { return fFileName; }

    ClassDef(TCACQUFile, 0) // ACQU file class
};

#endif

