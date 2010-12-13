
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

//--------------------------------------------------
void auto_Ecalib_TAPS( int set )
{ 
  // - - - E N E R G Y - - - 
    iCalibTAPS1gEnergy *gEcalib  = new iCalibTAPS1gEnergy( set );
  
    gEcalib->AutoWrite();
    gEcalib->AutoExit();
  
    gEcalib->DoAll(0., "gEcalib", 438);

  return;
}
