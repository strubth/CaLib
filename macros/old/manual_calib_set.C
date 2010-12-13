
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
void manual_calib_set( int set,
		       Char_t *szFin="/usr/users/irakli/AcquRoot/acqu/acqu/data/AUTO/CB/NaI.dat" )
{ 
  // add histograms
  //
  gCalMan->DoForSet( set, "hCB2gIM_cut" );
//  gCalMan->DoForSet( set, "Calib.TAPS.2g.Time" );
  
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
    }
  
  //
  //  
  gCBECalib = new iCBPi0EnergyCalib( set, hh );
  
  //  gCBECalib->AutoWrite();
  //  gCBECalib->AutoExit();
  
  //  gCBECalib->DoAll(0., "gCBECalib", 720);
  gCBECalib->DoAll( 720 );
  
//   Char_t szName[56];
//   sprintf( szName, 
//            "../../public_html/files/calib/hGr_set%02i.gif",
//            set );
//   c2->SaveAs( szName );

  //  gSystem->Exit(0);  
 
  return;
}
