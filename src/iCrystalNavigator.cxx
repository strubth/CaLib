
/*******************************************************************
 *                                                                 *
 * Date: 8.12.2008     Author: Irakli                              *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

#include "iCrystalNavigator.hh"

ClassImp(iCrystalNavigator)

//------------------------------------------------------------------------------
iCrystalNavigator::iCrystalNavigator()
{
    this->Init();
}

//------------------------------------------------------------------------------
iCrystalNavigator::~iCrystalNavigator()
{
    if (timer) delete timer;
}

//------------------------------------------------------------------------------
void iCrystalNavigator::Init()
{
    // timer and boolen variable for auto scanning of crystals
    timer = new TTimer(1000); //
    play  = kFALSE;           //

    // boolen variables for auto exit
    write = kFALSE;
    exit  = kFALSE;

    return;
}

//------------------------------------------------------------------------------
void iCrystalNavigator::Next(Int_t max)
{
    Int_t prev = currentCrystal;
    currentCrystal++;

    if (currentCrystal > max)
    {
        printf("\n Maximum number of elements %i is reached!\n",
               max);

        currentCrystal = max;
        this->Calculate(max);

        if (play)
            this->Stop();

        if (write)
            this->Write();

        if (exit)
            gSystem->Exit(0);
        return;
    }
    else
    {
        this->Calculate(prev);
        this->DoFor(currentCrystal);
    }

    return;
}
//------------------------------------------------------------------------------
/*void iCrystalNavigator::Next(Int_t min, Int_t max)
{
  currentCrystal = min;
  Int_t prev = currentCrystal;
  currentCrystal++;

  if( currentCrystal > max )
    {
      printf("\n Maximum number of elements %i is reached!\n",
         max);

      currentCrystal = max;
      this->Calculate( max );

      if( play )
    this->Stop();

      if( write )
    this->Write();

      if( exit )
        gSystem->Exit(0);
      return;
    }
  else
    {
      this->Calculate( prev );
      this->DoFor( currentCrystal );
    }

 return;
}*/
//------------------------------------------------------------------------------
void iCrystalNavigator::Prev()
{
    Int_t prev = currentCrystal;

    currentCrystal--;
    if (currentCrystal < 1)
    {
        currentCrystal = 1;
        this->Calculate(1);
        if (play)
            this->Stop();
    }
    else
    {
        this->Calculate(prev);
        this->DoFor(currentCrystal);
    }

    return;
}

//------------------------------------------------------------------------------
void iCrystalNavigator::DoAll(Double_t delay, Char_t* szModule, Int_t max)
{
    Char_t szCommand[64];

    if (delay)
    {
        sprintf(szCommand, "%s->Next(%i)", szModule, max);
        //   cout << szCommand << endl;

        timer->SetTime(delay);
        timer->SetCommand(szCommand);

        cout << endl;
        if (play)
        {
            timer->Stop();
            play = kFALSE;
        }
        else
        {
            timer->Start();
            play = kTRUE;
        }
    }
    else
    {
        for (int i=1; i<(max+1); i++)
        {
            this->Next(max);
        }
    }

    return;
}
//------------------------------------------------------------------------------
void iCrystalNavigator::DoAll(Double_t delay, Char_t* szModule, Int_t min, Int_t max)
{
    Char_t szCommand[64];

    if (delay)
    {
        sprintf(szCommand, "%s->Next(%i)", szModule, max);
        //   cout << szCommand << endl;

        timer->SetTime(delay);
        timer->SetCommand(szCommand);

        cout << endl;
        if (play)
        {
            timer->Stop();
            play = kFALSE;
        }
        else
        {
            timer->Start();
            play = kTRUE;
        }
    }
    else
    {
        for (int i=min; i<(max+1); i++)
        {
            this->Next(max);
        }
    }

    return;
}
//------------------------------------------------------------------------------
/*void iCrystalNavigator::DoAll_FromTo(Double_t delay, Char_t *szModule, Int_t min, Int_t max)
{
  Char_t szCommand[64];

  if( delay )
    {
      sprintf(szCommand, "%s->Next(%i, %i)", szModule, min, max);
      //   cout << szCommand << endl;

      timer->SetTime( delay );
      timer->SetCommand(szCommand);

      cout << endl;
      if( play )
    {
      timer->Stop();
      play = kFALSE;
    }
      else
    {
      timer->Start();
      play = kTRUE;
    }
    }
  else
    {
      for( int i=min; i<(max+1); i++)
    {
      this->Next( min, max );
    }
    }

  return;
*/

//------------------------------------------------------------------------------
void iCrystalNavigator::DoAll(Int_t max)
{
    for (int i=1; i<(max+1); i++)
    {
        this->Next(max);
    }

    return;
}

//------------------------------------------------------------------------------
void iCrystalNavigator::Stop()
{
    timer->Stop();
    play = kFALSE;

    return;
}

//------------------------------------------------------------------------------
void iCrystalNavigator::AutoWrite()
{
    write = kTRUE;

    return;
}

//------------------------------------------------------------------------------
void iCrystalNavigator::AutoExit()
{
    exit = kTRUE;

    return;
}
