
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

//--------------------------------------------------
void auto_calib_set( int set,
		     Char_t *szFin="" )
{ 
  // - - - C B - - -
  //  
  iCBPi0EnergyCalib* gCBECalib = new iCBPi0EnergyCalib( set );
  
  gCBECalib->AutoWrite();
  gCBECalib->AutoExit();
  
  gCBECalib->DoAll(0., "gCBECalib", 720);

  // - - - T A P S - - - 
//   iTAPSVsTAPSTimeCalib *g2TAPSCalib  = new iTAPSVsTAPSTimeCalib( set );
  
//   g2TAPSCalib->AutoWrite();
//   g2TAPSCalib->AutoExit();
  
//   g2TAPSCalib->DoAll(0., "g2TAPSCalib", 720);

  return;
}
