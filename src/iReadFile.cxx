
/*******************************************************************
 *                                                                 *
 * Date: 2.2.2009      Author: Irakli                              *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// A module for                                                               //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "iReadFile.hh"

ClassImp(iReadFile)

//_______________________________________________
iReadFile::iReadFile()
{
    this->Init();
}

//_______________________________________________
iReadFile::~iReadFile()
{
}

//_______________________________________________
Bool_t iReadFile::ReadFile(TString szFileName)
{
    // read file and store in MAX_LINE long TString array

    infile.open(szFileName);
    if (!infile.is_open())
    {
        printf("\n ---------------------------------------- \n");
        printf(" ERROR: opening : \"%s\" file ! ! !\n", szFileName.Data());
        return kFALSE;
    }
    else
    {
        printf("\n ---------------------------------------- \n");
        printf(" Read File : \"%s\"\n", szFileName.Data());
    }

    nline = 0;
    while (infile.good())
    {
        if (nline > MAX_LINE)
        {
            printf("\n ---------------------------------------- \n");
            printf("\n Total number of lines in file \"%s\" "
                   "exceeds MAX possible number of lines %i !!!\n",
                   szFileName.Data(), MAX_LINE);
            printf("\n Please change MAX_LINE value in file \"iReadFile.h\"\n\n");
            printf("\n ---------------------------------------- \n");
            exit(0);
        }
        else
        {
            strFileLine[nline]="";
            strFileLine[nline].ReadLine(infile);
            nline++;
        }
    }

    printf("\n ---------------------------------------- \n");
    printf("\n Total number of lines %i\n", nline);

    infile.close();

    // parse lines from string array
    this->ReadLines();

    return kTRUE;
}

//_______________________________________________
Bool_t iReadFile::ReadLines()
{
    nCrystal = 0;
    Int_t nTWalk = 0;
    Int_t nSGate = 0;

    for (int i=0; i<nline; i++)
    {
        if (strFileLine[i].BeginsWith("#"))
        {
            //      printf("Comment : %s \n", strFileLine[i].Data());
        }
        else if (strFileLine[i].BeginsWith("Element:"))
        {
            sscanf(strFileLine[i].Data(),
                   "%*s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
                   p[nCrystal][0], p[nCrystal][1], p[nCrystal][2],  p[nCrystal][3],
                   p[nCrystal][4], p[nCrystal][5], p[nCrystal][6],  p[nCrystal][7],
                   p[nCrystal][8], p[nCrystal][9], p[nCrystal][10], p[nCrystal][11],
                   p[nCrystal][12], p[nCrystal][13], p[nCrystal][14], p[nCrystal][15]);

            CBgain[nCrystal] = atof(p[nCrystal][4]);
            CBoffs[nCrystal] = atof(p[nCrystal][8]);
            CBtime[nCrystal] = atof(p[nCrystal][9]);

            CBposx[nCrystal] = atof(p[nCrystal][10]);
            CBposy[nCrystal] = atof(p[nCrystal][11]);
            CBposz[nCrystal] = atof(p[nCrystal][12]);

            if (nCrystal < iConfig::kMaxPID)
            {
                PIDphi[nCrystal] = atof(p[nCrystal][12]);
            }

            if (nCrystal < iConfig::kMaxTAPS)
            {
                TAPSoffset[nCrystal] = atof(p[nCrystal][8]);
                TAPSgain[nCrystal]   = atof(p[nCrystal][9]);

                TAPSposx[nCrystal] = atof(p[nCrystal][10]);
                TAPSposy[nCrystal] = atof(p[nCrystal][11]);
                TAPSposz[nCrystal] = atof(p[nCrystal][12]);
            }

            if (nCrystal < iConfig::kMaxTAGGER)
            {
                TaggerOffset[nCrystal] = atof(p[nCrystal][8]);
                TaggerGain[nCrystal]   = atof(p[nCrystal][9]);
            }

            // print all parameters
            //      for(int i=0; i<MAX_PAR; i++)
            //        printf("  %s ", p[nCrystal][i]);
            //      cout << endl;

            nCrystal++;
        }
        //
        // - - - Time Walk - - -
        //
        else if (strFileLine[i].BeginsWith("TimeWalk:"))
        {
            /*
              elemtn       rise      threshold    shift
              TimeWalk:    0        14.6331   -0.0001   -0.0179051   -1.09246
            */

            sscanf(strFileLine[i].Data(),
                   "%*s %*s %s %s %s %s",
                   szTw[ nTWalk][0],
                   szTw[ nTWalk][1],
                   szTw[ nTWalk][2],
                   szTw[ nTWalk][3]
                  );

            for (int i = 0; i < iConfig::kWalkNpar; i++)
                CBtwalk[ nTWalk + i*iConfig::kMaxCB ] = atof(szTw[nTWalk][i]);

            // print all parameters
            //
            //          for(int i = 0; i < TWALK_NPAR; i++)
            //        printf("  %10.6f ", CBtwalk[ nTWalk + i ]);
            //      cout << endl;

            nTWalk++;
        }
        //
        // - - - S h o r t  G a t e - - -
        //
        else if (strFileLine[i].BeginsWith("TAPSSG:"))
        {
            sscanf(strFileLine[i].Data(),
                   "%*s %s %s %s %s %s",
                   szSg[nSGate][0],
                   szSg[nSGate][1],
                   szSg[nSGate][2],
                   szSg[nSGate][3],
                   szSg[nSGate][4]);

            nSGate++;
        }
    }
    printf("\n ---------------------------------------- \n");

    if (nCrystal)
        printf("\n Total number of Elements  %i\n", nCrystal);

    if (nTWalk)
        printf("\n Total number of TimeWalk  %i\n", nTWalk);

    if (nSGate)
        printf("\n Total number of ShortGate %i\n", nSGate);

    return kTRUE;
}

//_______________________________________________
void iReadFile::Init()
{
    // needed in ReadFile
    for (Int_t i = 0; i < iConfig::kMaxCB; i++)
    {
        for (Int_t j = 0; j < MAX_PAR; j++)
            p[i][j] = new Char_t[MAX_PAR];

        CBgain[i]     = 0;

        for (Int_t j = 0; j < iConfig::kWalkNpar; j++)
        {
            CBtwalk[i+j] = 0;
            szTw[i][j] = new Char_t[MAX_PAR];
        }
    }

    for (Int_t i = 0; i < iConfig::kMaxPID; i++)
    {
        PIDphi[i] = 0;
    }

    for (Int_t i = 0; i < iConfig::kMaxTAPS; i++)
    {
        TAPSoffset[i] = 0;
        TAPSgain[i]   = 0;

        for (Int_t j = 0; j < MAX_PAR; j++)
            szSg[i][j] = new Char_t[MAX_PAR];
    }

    for (Int_t i = 0; i < iConfig::kMaxTAGGER; i++)
    {
        TaggerOffset[i] = 0;
        TaggerGain[i]   = 0;
    }

    return;
}

// EOF
