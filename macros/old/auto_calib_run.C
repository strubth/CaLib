
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

iCalibManager *gCalMan   = new iCalibManager();

iWriteCBconfigFile *iwc = new iWriteCBconfigFile();

iCBPi0EnergyCalib* gCBECalib;

TH2F* hh;

//--------------------------------------------------
void prepare( int run  )
{  
  iwc->ReadCBpar( run );
  iwc->WriteCBconfig( run );

  return;
}

//--------------------------------------------------
void auto_calib( int run )
{ 
  // prepare NaI.dat file
  // 
  //  prepare( run );

  // add histograms
  //
  gCalMan->DoForRun( run );
  
   if(hh)
    {
      hh->Delete();
      hh=0;
    }

    if( gCalMan->GetMainHisto() )
     {
       hh = (TH2F*) gCalMan->GetMainHisto()->Clone();
     }
    else
     {
       printf("\n ERROR: main histogram does not exist!!! \n");
       gSystem->Exit(0);
  //
  //  
  gCBECalib = new iCBPi0EnergyCalib( run, hh );

  //  gCBECalib->AutoWrite();
  //  gCBECalib->AutoExit();

  gCBECalib->DoAll(0., "gCBECalib", 720);

  return;
}

//--------------------------------------------------
void auto_calib( char *szFile )
{
  //
  ifstream fin( szFile, ios::in);
  if( fin.is_open() ) 
    cout << szFile << " file is open." << endl;
  else
    cout << "\n ERROR: No file :" << szFile << endl;
  
  int nfile = 0;  
  int run[2000];

  while( fin.good() )
    {
      fin >> run[nfile];

      //       if( nfile>0 && run[nfile] == run[nfile-1] )
      //  	break;
      cout << run[nfile] << endl;
      
      auto_calib( run[nfile] );

      nfile++;
    }
  fin.close();

  return;
}
