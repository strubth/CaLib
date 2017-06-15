/************************************************************************
 * Author: Thomas Strub                                                 *
 ************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCARHistoLoader                                                      //
//                                                                      //
// AcquRoot histogram loading class.                                    //
//                                                                      //
// Have fun!                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCARHISTOLOADER_H
#define TCARHISTOLOADER_H

#include "TCARFileLoader.h"

class TH1;
class TH2D;

class TCARHistoLoader : public TCARFileLoader
{

private:
    TDirectory* fHistoDirectory;        // histo ownership

protected:

    void SetHistoName(TH1* h, const Char_t* hnamepatt, Int_t index);

public:
    static const Int_t kLastBin;        // last bin of axis marker

    TCARHistoLoader()
      : TCARFileLoader(),
        fHistoDirectory(0) { }
    TCARHistoLoader(const Char_t* inputfilepathpatt)
      : TCARFileLoader(inputfilepathpatt),
        fHistoDirectory(0) { }
    TCARHistoLoader(Int_t nruns, const Int_t* runs, const Char_t* inputfilepathpatt = 0)
      : TCARFileLoader(nruns, runs, inputfilepathpatt),
        fHistoDirectory(0) { }
    virtual ~TCARHistoLoader() { }

    static TH1* GetHisto(const TFile* f, const Char_t* hname, Bool_t detach = kTRUE);
    static TH1** GetHistos(const TFile* f, const Char_t* hpatt, Int_t& nhistos, Bool_t detach = kTRUE);

    void SetHistoDirectory(TDirectory* histodir) { fHistoDirectory = histodir; };
    const TDirectory* GetHistoDirectory() const { return fHistoDirectory; };

    TH1* GetHistoForRun(const Char_t* hname, Int_t runnumber, const Char_t* houtnamepatt = 0);
    TH1* GetHistoForIndex(const Char_t* hname, Int_t index, const Char_t* houtnamepatt = 0);

    TH1** GetHistosForRun(const Char_t* hpatt, Int_t runnumber, Int_t& nhistos, const Char_t* houtnamepatt = 0);
    TH1** GetHistosForIndex(const Char_t* hpatt, Int_t index, Int_t& nhistos, const Char_t* houtnamepatt = 0);

    TH1* CreateHistoSum(const Char_t* hname, const Char_t* houtnamepatt = 0);

    TH1** CreateHistoArray(const Char_t* hname, const Char_t* houtnamepatt = 0);
    TH1** CreateHistoSumArray(const Char_t* hpatt, Int_t& nhistos, const Char_t* houtnamepatt = 0);

    TH1** CreateHistoArrayOfProj(const Char_t* hname, const Char_t projaxis = 'X',
                                 Int_t fbin1 = 1, Int_t lbin1 = kLastBin,
                                 Int_t fbin2 = 1, Int_t lbin2 = kLastBin,
                                 Option_t* option = "", const Char_t* houtnamepatt = 0);

    TH2D* CreateHistoOfProj(const Char_t* hname, const Char_t projaxis = 'X', const Char_t runaxis = 'Y',
                            Int_t fbin1 = 1, Int_t lbin1 = kLastBin,
                            Int_t fbin2 = 1, Int_t lbin2 = kLastBin,
                            Option_t* option = "");

    ClassDef(TCARHistoLoader, 0) // AR histogram loading class
};

#endif

