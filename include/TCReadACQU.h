/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCReadACQU                                                           //
//                                                                      //
// Read a list of ACQU raw files.                                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCREADACQU_H
#define TCREADACQU_H

#include "TList.h"
#include "TError.h"
#include "TSystemDirectory.h"

#include "TCACQUFile.h"


class TCReadACQU
{

private:
    Char_t* fPath;                  // path of files
    TList* fFiles;                  // list of files

    void ReadFiles(const Char_t* runPrefix);

public:
    TCReadACQU() : fPath(0), fFiles(0) { }
    TCReadACQU(const Char_t* path, const Char_t* runPrefix);
    virtual ~TCReadACQU();
    
    TList* GetFiles() const { return fFiles; }
    Int_t GetNFiles() const { return fFiles ? fFiles->GetSize() : 0; }
    TCACQUFile* GetFile(Int_t n) const { return fFiles ? (TCACQUFile*)fFiles->At(n) : 0; }

    ClassDef(TCReadACQU, 0) // ACQU raw file reader
};

#endif

