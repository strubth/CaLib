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


#include "TCACQUFile.h"

ClassImp(TCACQUFile)


//______________________________________________________________________________
TCACQUFile::TCACQUFile() 
    : TObject()
{
    // Constructor.
    
    // init members
    fFormat = kRawUnknown;
    fTime[0] = '\0';
    fDescription[0] = '\0';
    fRunNote[0] = '\0';
    fOutFile[0] = '\0';
    fRun = 0;
    fSize = 0;
    fFileName[0] = '\0';
}

//______________________________________________________________________________
RawFileType_t TCACQUFile::CheckFileType(const Char_t* file)
{
    // Check and return the type of the file 'file'.

    Char_t header[6];
    const unsigned char hGZ[3] = { 0x1f, 0x8b, 0x08 };
    const unsigned char hXZ[6] = { 0xfd, 0x37, 0x7a, 0x58, 0x5a, 0x00 };
    
    // open the file
    FILE* f = fopen(file, "r");
    
    // check file opening
    if (f)
    {
        // read 6 bytes
        fread(header, 1, 6, f);
        
        // close the file
        fclose(f);

        // check headers
        if (!memcmp(header, hGZ, 3)) return kFileGZ;
        else if (!memcmp(header, hXZ, 6)) return kFileXZ;
        else return kFileUnComp;
    } 
    else return kFileBad;
}

//______________________________________________________________________________
RawFileFormat_t TCACQUFile::CheckFileFormat(const Char_t* hdr)
{
    // Return the file type corresponding to the header 'hrd'.

    // Mk2
    Int_t n = 0;
    for (Int_t i = 0; i < 8; i++)
        if (hdr[i] == 0x10) n++;
    if (n == 8) return kRawMk2;

    // Mk1
    n = 0;
    for (Int_t i = 0; i < 4; i++)
        if (hdr[i] == 0x10) n++;
    if (n == 4) return kRawMk1;
    
    // other cases
    return kRawUnknown;
}

//______________________________________________________________________________
void TCACQUFile::RemoveControlChars(Char_t* string)
{            
    // Remove various control characters from the string 'string'.

    TString s(string);
    s.Remove(TString::kBoth, '\n');
    s.Remove(TString::kBoth, '\t');
    s.Remove(TString::kBoth, ' ');
    strcpy(string, s.Data());
}

//______________________________________________________________________________
void TCACQUFile::ReadFile(const Char_t* path, const Char_t* fname)
{
    // Read the file 'fname' located in 'path'.
    
    // ACQU record length
    const UInt_t recLength = 32768;

    // some variables
    Char_t buffer[recLength];
    UInt_t* datum;
    
    // set full file name
    Char_t filename[256];
    sprintf(filename, "%s/%s", path, fname);

    // set actual file name
    strcpy(fFileName, fname);

    // identify file type
    RawFileType_t ftype = CheckFileType(filename);
  
    // open the file
    FILE* file = OpenFile(filename, ftype);
    
    // check if file was opened
    if (!file)
    {
        Error("ReadFile", "Could not open '%s'", filename);
        return;
    }
  
    // read the complete file
    while (1)
    {
        // try to read a record
        if (fread(buffer, 1, recLength, file) != recLength) break;
        
        // set 4 byte datum pointer
        datum = (UInt_t*) buffer;
        
        // identify header buffer
        if (*datum == 0x10101010)
        {
            // check if format is Mk2 or Mk1 and parse the header
            if (*(datum+1) == 0x10101010) 
            {
                fFormat = kRawMk2;
                ParseHeader(buffer+kMk2Marker, fFormat);
            }
            else 
            {
                fFormat = kRawMk1;
                ParseHeader(buffer+kMk1Marker, fFormat);
            }

            break;
        }
        //// identify Mk1/Mk2 data buffer record (from EnumConst.h in acqu_core/AcquRoot)
        //else if (*datum == 0x20202020 || *datum == 0x70707070) 
        //{
        //    datum++;

        //    // loop over the complete data buffer
        //    while (datum < ((UInt_t*)buffer)+recLength)
        //    {
        //        if (*datum == 0xfefefefe) fNScalerReads++;
        //        datum++;
        //    }
        //}
    }

    // close the file
    CloseFile(file, ftype);

    // set file size
    FileStat_t fileinfo;
    gSystem->GetPathInfo(filename, fileinfo);
    fSize = fileinfo.fSize;
}

//______________________________________________________________________________
FILE* TCACQUFile::OpenFile(const Char_t* file, RawFileType_t type)
{
    // Open the file 'file' having the type 'type' and return the file pointer.
    // Return 0 if the file could not be opened.
    
    Char_t tmp[256];

    // try to open the file
    if (type == kFileUnComp) return fopen(file, "r");
    else if (type == kFileGZ)
    {
        sprintf(tmp, "zcat %s 2> /dev/null", file);
        return popen(tmp, "r");
    }
    else if (type == kFileXZ)
    {
        sprintf(tmp, "xzcat %s", file);
        return popen(tmp, "r");
    }
    else return 0;
}

//______________________________________________________________________________
void TCACQUFile::CloseFile(FILE* file, RawFileType_t type)
{
    // Close the file 'file' according to its type 'type'.

    if (type == kFileUnComp) fclose(file);
    else if (type == kFileGZ) pclose(file);
    else if (type == kFileXZ) pclose(file);
}
 
//______________________________________________________________________________
void TCACQUFile::ParseHeader(const Char_t* buffer, RawFileFormat_t format)
{
    // Parse the Mk1/2 header (set via 'format') stored in 'buffer' and set the
    // class members accordingly.

    // configure parsing
    Int_t sTime = format == kRawMk1 ? (Int_t)kMk1SizeTime : (Int_t)kMk2SizeTime;
    Int_t sDesc = format == kRawMk1 ? (Int_t)kMk1SizeDesc : (Int_t)kMk2SizeDesc;
    Int_t sRNote = format == kRawMk1 ? (Int_t)kMk1SizeRNote : (Int_t)kMk2SizeRNote;
    Int_t sFName = format == kRawMk1 ? (Int_t)kMk1SizeFName : (Int_t)kMk2SizeFName;
    
    // start parsing
    const Char_t* pos = buffer;

    // read time
    memcpy(fTime, pos, sTime);
    pos += sTime;
    RemoveControlChars(fTime);
    
    // read description
    memcpy(fDescription, pos, sDesc);
    pos += sDesc;
    RemoveControlChars(fDescription);
    
    // read run note
    memcpy(fRunNote, pos, sRNote);
    pos += sRNote;
    RemoveControlChars(fRunNote);

    // read output file
    memcpy(fOutFile, pos, sFName);
    pos += sFName;
    RemoveControlChars(fOutFile);

    // read run number
    if (format == kRawMk1) 
    {
        UShort_t r;
        memcpy(&r, pos, sizeof(UShort_t));
        fRun = (Int_t) r;
    }
    else memcpy(&fRun, pos, sizeof(Int_t));
}

//______________________________________________________________________________
void TCACQUFile::Print(Option_t* option) const
{
    // Print the content of this class.

    Char_t format[16];
    if (fFormat == kRawMk1) strcpy(format, "Mk1");
    else if (fFormat == kRawMk2) strcpy(format, "Mk2");
    else strcpy(format, "unknown");

    printf("ACQU File Information\n");
    printf("Format        : '%s'\n", format);
    printf("Time          : '%s'\n", fTime);
    printf("Description   : '%s'\n", fDescription);
    printf("Run note      : '%s'\n", fRunNote);
    printf("Output file   : '%s'\n", fOutFile);
    printf("Run number    : %d\n", fRun);
    printf("Size in bytes : %lld\n", fSize);
    printf("File name     : '%s'\n", fFileName);
    printf("\n");
}

