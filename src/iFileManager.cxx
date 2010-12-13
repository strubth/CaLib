
/*******************************************************************
 *                                                                 *
 * Date: 9.12.2008     Author: Irakli                              *
 *                                                                 *
 *                                                                 *
 ******************************************************************/


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Class to create file chain for given runset. The                           //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "iFileManager.hh"

ClassImp(iFileManager)

//------------------------------------------------------------------------------
iFileManager::iFileManager()
{
    strRUNFilesChain = this->GetConfigName("RUN.ChainHFiles");
    printf(" -- file names -- : %s\n", strRUNFilesChain.Data());
    fNRun = 0;
    fRunArray = 0;
}

//------------------------------------------------------------------------------
iFileManager::~iFileManager()
{
    this->CloseAllFiles();
}

//------------------------------------------------------------------------------
void iFileManager::DoForSet(Int_t set,
                            CalibData_t data,
                            TString strHistoName)
{
    //
    //
    //
    fSet  = set;

    this->BuildFileArray(fSet, data);
    this->BuildHistArray(strHistoName);
    //  this->CloseAllFiles();

    return;
}

//------------------------------------------------------------------------------
void iFileManager::BuildFileArray()
{
    // check if file is attached
    Int_t NFiles = gROOT->GetListOfFiles()->GetSize();
    TFile* fin = (TFile*) gROOT->GetListOfFiles()->First();

    for (int i=0; i < NFiles; i++)
    {
        cout << fin->GetName() << endl;
    }

    return;
}

//------------------------------------------------------------------------------
void iFileManager::BuildFileArray(Int_t set, CalibData_t data)
{
    //
    //
    //
    iMySQLManager m;
    fRunArray = m.GetRunsOfSet(data, set, &fNRun);

    TString strOrg;
    TString strFile;
    Char_t  szRun[5] = {0};

    //printf("\n\ntable: %s   set: %d    runs: %d\n", szTableName, set, fNRun);

    for (int i=0; i < fNRun; i++)
    {
        sprintf(szRun, "%05i", fRunArray[i]);

        strFile = "";
        strOrg = strRUNFilesChain.Copy();
        if (strOrg.Contains("NNNNN")) strFile = strOrg.ReplaceAll("NNNNN", szRun);
        else
        {
            sprintf(szRun, "%i", fRunArray[i]);
            strFile = strOrg.ReplaceAll("NNNN", szRun);
        }
        printf(" -- N: %3i   run: %5i   filename: %s\n",
               i, fRunArray[i], strFile.Data());

        fFile[i] = TFile::Open(strFile.Data());

        if (!fFile[i])
            continue;

        if (fFile[i]->IsZombie())
        {
            Error("BuildFileArray", "zombie file %s", fFile[i]->GetName());
            fFile[i] = 0;
            continue;
        }
    }

    // clean-up
    delete fRunArray;

    return;
}

//------------------------------------------------------------------------------
void iFileManager::BuildHistArray(TString szName)
{
    //
    //
    //

    //
    TH2F* htmp;

    gROOT->cd(); // very important!!!

    Bool_t first=kTRUE;
    for (int i=0; i < fNRun; i++)
    {
        if (!this->GetFile(i))
            continue;
        if (!(this->GetFile(i)->Get(szName)))
        {
            cerr << " ERROR: no histogram with name: " << szName << endl;
            gSystem->Exit(0);
            return;
        }
        if (first)
        {
            htmp = (TH2F*) this->GetFile(i)->Get(szName);
            hMain = (TH2F*) htmp->Clone();
            first = kFALSE;
        }
        else
        {
            htmp = (TH2F*) this->GetFile(i)->Get(szName);
            hMain->Add(htmp);
        }
    }

    return;
}

//------------------------------------------------------------------------------
void iFileManager::CloseAllFiles()
{
    //
    // Close all open .root files

    gROOT->cd(); // very important
    for (int i=0; i<fNRun; i++)
    {
        //      if( !this->GetFile(i) )
        if (fFile[i])
        {
            fFile[i]->Close();

            if (i==0)
                printf(" -- File: %s is closed!\n", fFile[i]->GetName());
            else
                printf(" .");
        }
        //      cout << fFile[i] << endl;
    }
    printf("\n -- File: %s is closed!\n", fFile[(fNRun-1)]->GetName());
    printf(" -- All files are closed!;\n");

    return;
}
