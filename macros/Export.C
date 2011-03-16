// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Export.C                                                             //
//                                                                      //
// Export CaLib run data and calibrations to ROOT files.                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void Export()
{
    Char_t tmp[256];
    Char_t tstamp[256];

    // load CaLib
    gSystem->Load("libCaLib.so");
 
    // get time-stamp
    UInt_t day, month, year;
    UInt_t hour, min;
    TTimeStamp t;
    t.GetDate(kFALSE, 0, &year, &month, &day);
    t.GetTime(kFALSE, 0, &hour, &min);
    sprintf(tstamp, "%d-%02d-%02d_%02d.%02d", year, month, day, hour, min);

    // export CaLib data
    sprintf(tmp, "backup_Dec_07_%s.root", tstamp);
    TCMySQLManager::GetManager()->Export(tmp, 0, 0, "LD2_Dec_07");
    
    // export CaLib data
    sprintf(tmp, "backup_Feb_09_%s.root", tstamp);
    TCMySQLManager::GetManager()->Export(tmp, 0, 0, "LD2_Feb_09");

    // export CaLib data
    sprintf(tmp, "backup_May_09_%s.root", tstamp);
    TCMySQLManager::GetManager()->Export(tmp, 0, 0, "LD2_May_09");
    
    gSystem->Exit(0);
}

