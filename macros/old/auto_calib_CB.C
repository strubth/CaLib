
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

//--------------------------------------------------
void auto_calib_CB( int set )
{ 
  // - - - E N E R G Y - - -
//   iCalibCBpi0Energy* gCBECalib = new iCalibCBpi0Energy( set );
  
//   gCBECalib->AutoWrite();
//   gCBECalib->AutoExit();
  
//   gCBECalib->DoAll(0., "gCBECalib", 720);
  
  
  // - - - T I M E - - -
//    iCalibCB2gTime* gCBECalib = new iCalibCB2gTime( set );
  
//    gCBECalib->AutoWrite();
//    gCBECalib->AutoExit();
  
//    gCBECalib->DoAll(0., "gCBECalib", 720);

  
  // - - - T W A L K - - -
   iCalibCBTimeWalk* gCBTwalk = new  iCalibCBTimeWalk( set );
  
   gCBTwalk->AutoWrite();
   gCBTwalk->AutoExit();
  
   gCBTwalk->DoAll(0., "gCBTwalk", 720);

  return;
}
