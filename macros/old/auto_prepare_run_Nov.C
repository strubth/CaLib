
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

void auto_prepare_run_Nov( Int_t run )
{
  Char_t szFileName[256]; //
  Char_t szCommand[256];  //
  Char_t szDir[256];      // directory path
  
  TString strAcquPath = gSystem->Getenv("acqu");
  if( !strAcquPath.Length() )
    gSystem->Exit(1);

  //  
  TString strTempPath = TString( strAcquPath + "/data/He3_AUTO_temp_Nov_08" );
  sprintf( szDir, 
	   "%s/data/AUTO/AUTO_%i",
  	   strAcquPath.Data(),
  	   run );
  
  // - - - cleanup - - - 
  //
  sprintf( szCommand, 
  	   ".! rm -fr %s",
  	   szDir );
  cout << szCommand << endl;

  gROOT->ProcessLine( szCommand );
  
  // - - - M a k e  D i r - - - 
  //
  sprintf( szCommand, 
           ".! mkdir %s",
           szDir );
  cout << szCommand << endl;

  gROOT->ProcessLine( szCommand );

  // T A G G E R
  sprintf( szCommand, 
           ".! mkdir %s/Tagger",
           szDir );
  cout << szCommand << endl;
  gROOT->ProcessLine( szCommand );

  // C B
  sprintf( szCommand, 
           ".! mkdir %s/CB",
           szDir );
  cout << szCommand << endl;
  gROOT->ProcessLine( szCommand );

  // T A P S 
  sprintf( szCommand, 
           ".! mkdir %s/TAPS",
           szDir );
  cout << szCommand << endl;
  gROOT->ProcessLine( szCommand );


  // - - - Writing Files - - -
  //
  iChangeARconfig ic;

  // change CB.Offline
  //
  ic.CopyARconfig( run, 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/He3_AUTO_temp_Nov_08/CB.Offline", 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/AUTO/XXXX/CB.Offline" );
    
  // change CBanalys.dat
  //
  ic.CopyARconfig( run, 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/He3_AUTO_temp_Nov_08/CBanalysis.dat", 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/AUTO/XXXX/CBanalysis.dat" );

  // change Tagger/Tagger.dat
  //
  ic.CopyARconfig( run, 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/He3_AUTO_temp_Nov_08/Tagger/Tagger.dat", 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/AUTO/XXXX/Tagger/Tagger.dat" );

  // change CB/CB_PID.dat
  //
  ic.CopyARconfig( run, 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/He3_AUTO_temp_Nov_08/CB/CB_PID.dat", 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/AUTO/XXXX/CB/CB_PID.dat" );

  // change TAPS/TAPS.dat
  //
  ic.CopyARconfig( run, 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/He3_AUTO_temp_Nov_08/TAPS/TAPS.dat", 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/AUTO/XXXX/TAPS/TAPS.dat" );

  // prepare CBServer.dat file
  //
  ic.CopyARconfig( run, 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/He3_AUTO_temp_Nov_08/CBServer.Offline", 
		   "/usr/users/witth/AcquRoot/acqu/acqu/data/AUTO/XXXX/CBServer.Offline" );
  
  // - - - T A G G E R - - - 
  //
  sprintf( szFileName, 
	   "%s/Tagger/FP.dat_temp",
	   strTempPath.Data() );
  iWriteTAGGERconfigFile *iwta = new iWriteTAGGERconfigFile( szFileName );
 
  sprintf( szFileName, 
	   "%s/Tagger/FP.dat",
	   szDir );
  iwta->WriteTAGGERconfig4run( run,
			  szFileName );
  delete iwta;
 
  // - - - C B - - - 
  //
  sprintf( szFileName, 
	   "%s/CB/NaI.dat_temp",
	   strTempPath.Data() );
  iWriteCBconfigFile *iwc = new iWriteCBconfigFile( szFileName );
  
  sprintf( szFileName, 
	   "%s/CB/NaI.dat",
	   szDir );
  iwc->WriteCBconfig4run( run,
			  szFileName );
  delete iwc;
  // - - - PID - - - 
  //
  sprintf( szFileName, 
	   "%s/CB/PID.dat_temp",
	   strTempPath.Data() );
  iWritePIDconfigFile *iwPID = new iWritePIDconfigFile( szFileName );
  
  sprintf( szFileName, 
	   "%s/CB/PID.dat",
	   szDir );
  iwPID->WritePIDconfig4run( run,
			  szFileName );
  delete iwPID;
 
  // - - - T A P S - - - 
  //
  sprintf( szFileName, 
	   "%s/TAPS/BaF2.dat_temp",
	   strTempPath.Data() );
  iWriteTAPSconfigFile *iwt = new iWriteTAPSconfigFile( szFileName );
  
  sprintf( szFileName, 
  	   "%s/TAPS/BaF2.dat", 
  	   szDir );
  iwt->WriteTAPSconfig4run( run,
     			    szFileName );
  delete iwt;
 
  //
  
  gSystem->Exit(0);

  return;
}
