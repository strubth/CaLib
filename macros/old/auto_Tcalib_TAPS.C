
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

//--------------------------------------------------
void auto_Tcalib_TAPS( int set )
{ 
  // - - - E N E R G Y - - - 
//    iTAPS1gEnergyCalib *gEcalib  = new iTAPS1gEnergyCalib( set );
  
//    gEcalib->AutoWrite();
//    gEcalib->AutoExit();
  
//    gEcalib->DoAll(0., "gEcalib", 438);

  // - - - T I M E - - -
  iCalibTAPS2gTime * gTcalib  = new iCalibTAPS2gTime( set );
  
  gTcalib->AutoWrite();
  gTcalib->AutoExit();
  
  gTcalib->DoAll(0., "gTcalib", 438);

  return;
}
