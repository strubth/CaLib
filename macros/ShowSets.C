/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ShowSets.C                                                           //
//                                                                      //
// Show the number of sets for all calibration data types.              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void ShowSets()
{
    // load CaLib
    gSystem->Load("libCaLib.so");

    // macro configuration: just change here for your beamtime and leave
    // the other parts of the code unchanged
    const Char_t calibName[]        = "He4_Oct_16";

    // loop over calibration data types
    TIter next(TCMySQLManager::GetManager()->GetDataTable());
    TCCalibData* d;
    while ((d = (TCCalibData*)next()))
    {
        Int_t nsets = TCMySQLManager::GetManager()->GetNsets(d->GetName(), calibName);
        printf("%-40s : %d set(s)\n", d->GetTitle(), nsets);
    }

    gSystem->Exit(0);
}

