
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

//--------------------------------------------------
void auto_calib_CB_new_1( int set )
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
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk1 = new  iCalibCBTimeWalk( set,190 );
  
   //gCBTwalk1->AutoWrite();

    gCBTwalk1->DoFor(190);
 //  gCBTwalk1->DoAll(0., "gCBTwalk", 40, 43);
   /*   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk049");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_049");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj049");
   delete h3;
   
   delete gCBTwalk1;
   delete gCBTwalk15;*/
//------------------------------------------------------------------
 //gROOT->ProcessLine(".ls");
  return;
}
