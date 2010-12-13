
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

//--------------------------------------------------
void auto_calib_CB( int set )
{ 
  // - - - C B - - -
  //  
  iCBvsCBtimeCalib* gCBECalib = new iCBvsCBtimeCalib( set );
  
  gCBECalib->AutoWrite();
  gCBECalib->AutoExit();
  
  gCBECalib->DoAll(0., "gCBECalib", 720);

  return;
}

//--------------------------------------------------
void auto_calib_all( )
{
  for(int i=1; i<=12; i++)
    {}
}
