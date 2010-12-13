
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

//--------------------------------------------------
void auto_calib_CB_TWalk( int set )
{ 
   iCalibCBTimeWalk* gCBTwalk = new  iCalibCBTimeWalk( set );
  
   gCBTwalk->AutoWrite();
   gCBTwalk->AutoExit();

   gCBTwalk->DoAll(0., "gCBTwalk", 720);
   
   return;
}
