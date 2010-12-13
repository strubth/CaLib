/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iReadFile                                                            //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef IREADFILE_HH
#define IREADFILE_HH

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include <TObject.h>
#include <TFile.h>
#include <TString.h>

#include "iConfig.hh"

#define MAX_LINE 3300
#define MAX_PAR  16


using namespace std;


class iReadFile
{

private:
    ifstream infile;

protected:
    Int_t nline;    // total number of lines in calib. file
    Int_t nCrystal; // total number of initialized crustals in calib. file

    TString  strLine;
    TString  strFileLine[ MAX_LINE ];

    // common for all
    Char_t*   p[ MAX_CB ][ MAX_PAR ];

    // only for CB time walk
    Char_t*   szTw[ MAX_CB ][ TWALK_NPAR ];

    // only for TAPS short gate
    Char_t* szSg[ MAX_TAPS ][ MAX_PAR ];

    // - - - CB - - -
    //  Char_t   szCBadc[ 8 ][ MAX_CB ];
    Double_t CBgain[ MAX_CB ];

    //  Char_t   szCBtdc[ 8 ][ MAX_CB ];
    Double_t CBoffs[ MAX_CB ];
    Double_t CBtime[ MAX_CB ];
    Double_t CBposx[ MAX_CB ];
    Double_t CBposy[ MAX_CB ];
    Double_t CBposz[ MAX_CB ];
    Double_t CBtwalk[ MAX_CB* TWALK_NPAR ];

    // - - - PID - - -
    Char_t   szPIDadc[ MAX_PID ][ 8 ];
    Char_t   szPIDtdc[ MAX_PID ][ 8 ];

    Double_t PIDphi[ MAX_PID ];


    // - - - TAPS - - -
    //
    Char_t   szTAPSadc[ MAX_TAPS ][ 8 ];
    Char_t   szTAPStdc[ MAX_TAPS ][ 8 ];

    Double_t TAPSoffset[ MAX_TAPS ];
    Double_t TAPSgain[   MAX_TAPS ];

    Double_t TAPSposx[ MAX_TAPS ];
    Double_t TAPSposy[ MAX_TAPS ];
    Double_t TAPSposz[ MAX_TAPS ];

    // - - - TAGGER - - -
    //
    Char_t   szTAGGERadc[ MAX_TAGGER ][ 8 ];
    Char_t   szTAGGERtdc[ MAX_TAGGER ][ 8 ];

    Double_t TaggerOffset[ MAX_TAGGER ];
    Double_t TaggerGain[   MAX_TAGGER ];

public:
    iReadFile();
    virtual ~iReadFile();

    virtual void Init();

    Bool_t ReadFile(TString);
    Bool_t ReadLines();

    // - - - C B - - -
    Double_t*  GetCBgain() { return CBgain; };
    Double_t   GetCBgain(Int_t i) { return CBgain[i]; };
    Double_t*  GetCBtime() { return CBtime; };
    Double_t   GetCBtime(Int_t i) { return CBtime[i]; };
    Double_t*  GetCBoffs() { return CBoffs; };
    Double_t   GetCBoffs(Int_t i) { return CBoffs[i]; };
    Double_t*  GetCBposx() { return CBposx; };
    Double_t   GetCBposx(Int_t i) { return CBposx[i]; };
    Double_t*  GetCBposy() { return CBposy; };
    Double_t   GetCBposy(Int_t i) { return CBposy[i]; };
    Double_t*  GetCBposz() { return CBposz; };
    Double_t   GetCBposz(Int_t i) { return CBposz[i]; };
    Double_t*  GetCBtwalk() { return CBtwalk; };
    Double_t   GetCBtwalk(Int_t i) { return CBtwalk[i]; };

    // - - - P I D - - -
    Double_t* GetPIDphi() { return PIDphi; };
    Double_t  GetPIDphi(Int_t i) { return PIDphi[i]; };

    // - - - T A P S - - -
    Double_t* GetTAPSoffset() { return TAPSoffset; };
    Double_t  GetTAPSoffset(Int_t i) { return TAPSoffset[i]; };
    Double_t* GetTAPSgain() { return TAPSgain; };
    Double_t  GetTAPSgain(Int_t i) { return TAPSgain[i]; };
    Double_t*  GetTAPSposx() { return TAPSposx; };
    Double_t   GetTAPSposx(Int_t i) { return TAPSposx[i]; };
    Double_t*  GetTAPSposy() { return TAPSposy; };
    Double_t   GetTAPSposy(Int_t i) { return TAPSposy[i]; };
    Double_t*  GetTAPSposz() { return TAPSposz; };
    Double_t   GetTAPSposz(Int_t i) { return TAPSposz[i]; };
    Char_t* GetTAPSSG(Int_t i, Int_t j) { return szSg[i][j]; };

    // - - - c o m m o n - - -
    Char_t*  GetCharPar(Int_t i, Int_t j) { return p[i][j]; };
    Int_t    GetIntPar(Int_t i, Int_t j) { return atoi(p[i][j]); };
    Double_t GetDoublePar(Int_t i, Int_t j) { return atof(p[i][j]); };

    ClassDef(iReadFile, 0)   // Read AR calibration .dat files
};

#endif

