
#include <stdio.h>

#include <iostream>
#include <fstream>
//using namespace std;

#include <TROOT.h>
#include <TFile.h>
#include <TH2F.h>

TFile *fFile;

//
int run1 = 20003;
int run2 = 20655;

TH2F *hNewSum;

void hadd( Char_t szListFile[]  = "filelist.txt",
	   Char_t szHistoName[] = "MyEta2gMissingE_0")
{
  ifstream fin;
  fin.open( szListFile );
  if( !fin.is_open() )
    {
      cerr << " ERROR: can't open file " << szListFile << endl;
      return;
    }
  
  Int_t nn = 0;
  Char_t szRootFile[128];
  TH2F *htmp;
  
  while( fin.good() )
    {
      fin >> szRootFile;
      
      fFile = TFile::Open( szRootFile );
      
      if( fFile->IsZombie() )
	{
	  cerr << " ERROR: can't open file " << fFile->GetName() << endl;
	  continue;
	}
      else
	{
	  cout << " open file -- " << fFile->GetName() << endl;
	}
      
      gROOT->cd(); // very important!!!
      
      if( !nn )
 	{
	  htmp = (TH2F*)  fFile->Get( szHistoName );
	  hNewSum = (TH2F*) htmp->Clone();
 	}
      else
	{
	  htmp = (TH2F*) fFile->Get( szHistoName );
 	  hNewSum->Add( htmp );
	}
      nn++;
      
      fFile->Close();
    }
  
  fin.close();
  
  printf("\n NOTE: %i files where added... \n", nn);

  Char_t szOut[64];
  sprintf( szOut, 
	   "%s_SUM.root",
 	   szHistoName);
  hNewSum->SaveAs(szOut);

  return;
}
