/*************************************************************************
 * Author: Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCrystalNavigator                                                    //
//                                                                      //
// ...                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICRYSTALNAVIGATOR_HH
#define ICRYSTALNAVIGATOR_HH

#include <iostream>
#include <fstream>

#include "TSystem.h"
#include "TTimer.h"

#include "iConfig.hh"


using namespace std;


class iCrystalNavigator
{

protected:
    Int_t currentCrystal; //

    // timer
    TTimer* timer;        //
    Bool_t play;          //
    Bool_t write;         // if kTRUE write after last crystal
    Bool_t exit;          // if kTRUE exit  after last crystal

public:
    iCrystalNavigator();
    virtual ~iCrystalNavigator();

    void Init();

    virtual void Calculate(Int_t) = 0;
    virtual void DoFor(Int_t) = 0;
    virtual void Write() = 0;

    void Next(Int_t);
    void Prev();
    void DoAll(Int_t);
    void DoAll(Double_t, Char_t*, Int_t);
    void DoAll(Double_t, Char_t*, Int_t, Int_t);
    void Stop();

    // auto methods at the end
    void AutoWrite();
    void AutoExit();

    ClassDef(iCrystalNavigator, 0)   // A basic class to navigate between crystals
};

#endif

