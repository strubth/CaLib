
/*******************************************************************
 *                                                                 *
 * Date: 26.12.2008     Author: Irakli                             *
 *                                                                 *
 ******************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This class is used to read config files for different settings like:       //
//                                                                            //
// Crystal energy or time cuts, histogram name,                               //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "iReadConfig.hh"

ClassImp(iReadConfig)

//------------------------------------------------------------------------------
iReadConfig::iReadConfig()
{
    nLine=0;

    // Check if env. variable $CALIB exist
    //
    if (gSystem->Getenv("CALIB"))
        strCaLibPath = gSystem->Getenv("CALIB");
    else
        strCaLibPath = gSystem->pwd();

    this->ReadConfigFile("config/config.cfg");
}

//------------------------------------------------------------------------------
iReadConfig::iReadConfig(Char_t* szFin)
{
    nLine=0;

    this->ReadConfigFile(szFin);
}

//------------------------------------------------------------------------------
iReadConfig::~iReadConfig()
{
}

//------------------------------------------------------------------------------
void iReadConfig::ReadConfigFile(const Char_t* szFin)
{
    // Build File name
    char szCalibFile[128];
    sprintf(szCalibFile,
            "%s/%s",
            strCaLibPath.Data(),
            szFin);

    //
    ifstream infile;
    infile.open(szCalibFile);

    if (!infile.is_open())
    {
        printf("\n ERROR: opening \"%s\" file ! ! !\n\n", szFin);
    }
    else
    {
        printf("\n ---------------------------------------- \n");
        printf(" Read File : \"%s\"\n", szFin);

        while (infile.good())
        {
            strLine[nLine]="";
            strLine[nLine].ReadLine(infile);

            if (strLine[nLine].BeginsWith("#"))
            {
                //          printf("Comment : %s \n", strLine[nLine].Data());
            }
            //
            else if (strLine[nLine].Contains("FILE"))
            {
                TString strFile = this->ExtractName(strLine[nLine]);

                this->ReadConfigFile(strFile.Data());
            }
            nLine++;
            if (nLine >= MAX_LINE)
            {
                printf("\n ERROR: Number of lines is more than MAX_LINE ! ! !\n\n");
                gSystem->Exit(0);
            }
        }
    }

    infile.close();
    return;
}

//------------------------------------------------------------------------------
TString iReadConfig::ExtractName(TString strIn)
{
    Ssiz_t aa = strIn.First(":")+1;
    Ssiz_t bb = strIn.Length()-aa;

    TString cc = strIn(aa, bb);
    cc.ReplaceAll(" ","");
    return cc;
}

//------------------------------------------------------------------------------
TString iReadConfig::GetConfigCuts(TString name)
{
    TString strOut;
    Char_t szCuts[2][16];

    for (int i=0; i<nLine; i++)
    {
        if (!(strLine[i].BeginsWith("#")) &&
                strLine[i].Contains(name))
        {
            sscanf(strLine[i].Data(),
                   "%*s %s %s",
                   szCuts[0],
                   szCuts[1]);

            strOut = szCuts[0];
            strOut += "  ";
            strOut += szCuts[1];
        }
    }
    return strOut;
}

//------------------------------------------------------------------------------
TString iReadConfig::GetConfigName(TString name)
{
    TString strOut;

    for (int i=0; i<nLine; i++)
    {
        if (!(strLine[i].BeginsWith("#")) &&
                strLine[i].Contains(name))
        {
            strOut = this->ExtractName(strLine[i]);
        }
    }
    return strOut;
}
//------------------------------------------------------------------------------
TString iReadConfig::GetConfigNamePart(TString name, Int_t& run)
{
    TString strOut;
    Char_t szCfgFile[256];

    for (int i=0; i<nLine; i++)
    {
        if (!(strLine[i].BeginsWith("#")) &&
                strLine[i].Contains(name))
        {
            sscanf(strLine[i].Data(),
                   "%*s %s %i",
                   szCfgFile,
                   &run);

            strOut = szCfgFile;
        }
    }

    return strOut;
}
//------------------------------------------------------------------------------

