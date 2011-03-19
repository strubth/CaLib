// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCReadACQU                                                           //
//                                                                      //
// Read raw ACQU MK1 files.                                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCREADACQU_H
#define TCREADACQU_H

#include <zlib.h>

#include "TList.h"
#include "TError.h"
#include "TSystem.h"
#include "TSystemDirectory.h"


class TCACQUFile : public TObject
{

private:
    Char_t fStartMarker[4];         // start marker
    Char_t fTime[26];               // run start time (ascii)
    Char_t fDescription[133];       // description of experiment
    Char_t fRunNote[133];           // particular run note
    Char_t fOutFile[40];            // output file
    UShort_t fRun;                  // run number
    Long64_t fSize;                 // file size in bytes
    Char_t fFileName[256];          // actual filename

public:
    TCACQUFile() : TObject()
    {
        memset(fStartMarker, 0, sizeof(fStartMarker));
        fTime[0] = '\0';
        fDescription[0] = '\0';
        fRunNote[0] = '\0';
        fOutFile[0] = '\0';
        fRun = 0;
        fSize = 0;
        fFileName[0] = '\0';
    }
    virtual ~TCACQUFile() { }

    void RemoveLineBreak(Char_t* string, Int_t length)
    {
        for (Int_t i = 0; i < length; i++)
        {
            if (string[i] == '\n') string[i] = '\0';
        }
    }

    void ReadFile(const Char_t* path, const Char_t* fname)
    {
        // set full file name
        Char_t filename[256];
        sprintf(filename, "%s/%s", path, fname);

        // set actual file name
        strcpy(fFileName, fname);

        // try to open the file
        void* file = gzopen(fFileName, "r");
        if (!file)
        {
            Error("Read", "Could not open '%s'", fFileName);
            return;
        }
        
        // read start marker
        gzread(file, fStartMarker, sizeof(fStartMarker));
        
        // read time
        gzread(file, fTime, sizeof(fTime));
        RemoveLineBreak(fTime, sizeof(fTime)/sizeof(Char_t));
        
        // skip tab
        gzseek(file, sizeof(Char_t), SEEK_CUR);

        // read description (minus 1 byte)
        gzread(file, fDescription, sizeof(fDescription)-1);
        RemoveLineBreak(fDescription, sizeof(fDescription)/sizeof(Char_t));
        
        // trim description
        TString s(fDescription);
        s.Remove(TString::kBoth, ' ');
        strcpy(fDescription, s.Data());

        // read run note
        gzread(file, fRunNote, sizeof(fRunNote));
        RemoveLineBreak(fRunNote, sizeof(fRunNote)/sizeof(Char_t));
        
        // trim run note
        s = fRunNote;
        s.Remove(TString::kBoth, ' ');
        strcpy(fRunNote, s.Data());
        
        // read output file
        gzread(file, fOutFile, sizeof(fOutFile));
        
        // read run number
        gzread(file, &fRun, sizeof(UShort_t));
        
        // close the file
        gzclose(file);
        
        // set file size
        FileStat_t fileinfo;
        gSystem->GetPathInfo(fFileName, fileinfo);
        fSize = fileinfo.fSize;
    }
  
    void Print()
    {
        printf("ACQU File Information\n");
        printf("Start Marker  : 0x%x 0x%x 0x%x 0x%x\n",
               fStartMarker[0], fStartMarker[1], fStartMarker[2], fStartMarker[3]);
        printf("Time          : %s\n", fTime);
        printf("Description   : %s\n", fDescription);
        printf("Run note      : %s\n", fRunNote);
        printf("Output file   : %s\n", fOutFile);
        printf("Run number    : %d\n", fRun);
        printf("Size in bytes : %lld\n", fSize);
        printf("File name     : %s\n", fFileName);
        printf("\n");
    }

    void PrintListing() { printf("%s\t%s\t%s\t%s\t%lld\n", fFileName, fTime, fDescription, fRunNote, fSize); }

    Bool_t IsGoodStartMarker()
    {
        if (fStartMarker[0] == 0x10 &&
            fStartMarker[1] == 0x10 &&
            fStartMarker[2] == 0x10 &&
            fStartMarker[3] == 0x10) return kTRUE;
        else return kFALSE;
    }

    const Char_t* GetStartMarker() const { return fStartMarker; }
    const Char_t* GetTime() const { return fTime; }
    const Char_t* GetDescription() const { return fDescription; }
    const Char_t* GetRunNote() const { return fRunNote; }
    const Char_t* GetOutFile() const { return fOutFile; }
    UShort_t GetRun() const { return fRun; }
    Long64_t GetSize() const { return fSize; }
    const Char_t* GetFileName() const { return fFileName; }
    
    ClassDef(TCACQUFile, 0) // ACQU file class
};


class TCReadACQU
{

private:
    Char_t* fPath;                  // path of files
    TList* fFiles;                  // list of files

    void ReadFiles();

public:
    TCReadACQU() : fPath(0), fFiles(0) { }
    TCReadACQU(const Char_t* path);
    virtual ~TCReadACQU();
    
    TList* GetFiles() const { return fFiles; }
    Int_t GetNFiles() const { return fFiles ? fFiles->GetSize() : 0; }
    TCACQUFile* GetFile(Int_t n) const { return fFiles ? (TCACQUFile*)fFiles->At(n) : 0; }

    ClassDef(TCReadACQU, 0) // ACQU raw file reader
};

#endif

