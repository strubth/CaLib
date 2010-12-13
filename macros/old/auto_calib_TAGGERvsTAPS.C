
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

//--------------------------------------------------
void auto_calib_TAGGERvsTAPS( int set )
{ 
  // - - - E N E R G Y - - - 
//    iTAPS1gEnergyCalib *gEcalib  = new iTAPS1gEnergyCalib( set );
  
//    gEcalib->AutoWrite();
//    gEcalib->AutoExit();
  
//    gEcalib->DoAll(0., "gEcalib", 384);

  // - - - T I M E - - -
  iCalibTAGGERvsTAPSTime * gTcalib  = new iCalibTAGGERvsTAPSTime( set );
  
  gTcalib->AutoWrite();
  gTcalib->AutoExit();
  
  gTcalib->DoAll(0., "gTcalib", 352);

  return;
}
