
/***********************************************************
 *                                                         *
 *                                                         *
 *                                                         *
 *                                                         *
 **********************************************************/

//--------------------------------------------------
void auto_calib_CB_new( int set )
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
   iCalibCBTimeWalk* gCBTwalk1 = new  iCalibCBTimeWalk( set,1 );
  
   gCBTwalk1->AutoWrite();
  
   gCBTwalk1->DoAll(0., "gCBTwalk", 1, 50);
   
   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk049");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_049");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj049");
   delete h3;
   
   delete gCBTwalk1;
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk2 = new  iCalibCBTimeWalk( set,51 );
  
   gCBTwalk2->AutoWrite();
  
   gCBTwalk2->DoAll(0., "gCBTwalk",51, 100);

   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk099");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_099");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj099");
   delete h3;
    
   delete gCBTwalk2;
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk3 = new  iCalibCBTimeWalk( set,101 );
  
   gCBTwalk3->AutoWrite();
   gCBTwalk3->DoAll(0., "gCBTwalk", 101, 150);
   
   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk149");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_149");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj149");
   delete h3;

   delete gCBTwalk3;
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk4 = new  iCalibCBTimeWalk( set,151 );
  
   gCBTwalk4->AutoWrite();
  
   gCBTwalk4->DoAll(0., "gCBTwalk",151, 200);

   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk199");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_199");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj199");
   delete h3;

    delete gCBTwalk4;
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk5 = new  iCalibCBTimeWalk( set,201 );
  
   gCBTwalk5->AutoWrite();
  
   gCBTwalk5->DoAll(0., "gCBTwalk", 201, 250);
   
   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk249");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_249");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj249");
   delete h3;

   delete gCBTwalk5;
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk6 = new  iCalibCBTimeWalk( set,251 );
  
   gCBTwalk6->AutoWrite();
  
   gCBTwalk6->DoAll(0., "gCBTwalk", 251, 300);

   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk299");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_299");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj299");
   delete h3;

   delete gCBTwalk6;
//--------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk7 = new  iCalibCBTimeWalk( set,301 );
  
   gCBTwalk7->AutoWrite();
   gCBTwalk7->DoAll(0., "gCBTwalk", 301, 350);
   
   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk349");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_349");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj349");
   delete h3;

   delete gCBTwalk7;
//------------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk8 = new  iCalibCBTimeWalk( set,351 );
  
   gCBTwalk8->AutoWrite();
  
   gCBTwalk8->DoAll(0., "gCBTwalk", 351, 400);

   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk399");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_399");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj399");
   delete h3;

   delete gCBTwalk8;
//------------------------------------------------------------------
iCalibCBTimeWalk* gCBTwalk9 = new  iCalibCBTimeWalk( set,401 );
  
   gCBTwalk9->AutoWrite();
  
   gCBTwalk9->DoAll(0., "gCBTwalk", 401, 450);
   
   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk449");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_449");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj449");
   delete h3;

   delete gCBTwalk9;
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk10 = new  iCalibCBTimeWalk( set,451 );
  
   gCBTwalk10->AutoWrite();
  
   gCBTwalk10->DoAll(0., "gCBTwalk", 451, 500);

   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk499");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_499");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj499");
   delete h3;

   delete gCBTwalk10;
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk11 = new  iCalibCBTimeWalk( set,501 );
  
   gCBTwalk11->AutoWrite();
   gCBTwalk11->DoAll(0., "gCBTwalk", 501, 550);
   
   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk549");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_549");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj549");
   delete h3;

   delete gCBTwalk11;
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk12 = new  iCalibCBTimeWalk( set,551 );
  
   gCBTwalk12->AutoWrite();
  
   gCBTwalk12->DoAll(0., "gCBTwalk", 551, 600);

   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk599");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_599");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj599");
   delete h3;
   
   delete gCBTwalk12;
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk13 = new  iCalibCBTimeWalk( set,601 );
  
   gCBTwalk13->AutoWrite();
  
   gCBTwalk13->DoAll(0., "gCBTwalk", 601, 650);
   
   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk649");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_649");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj649");
   delete h3;

   delete gCBTwalk13;
//---------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk14 = new  iCalibCBTimeWalk( set,651 );
  
   gCBTwalk14->AutoWrite();
  
   gCBTwalk14->DoAll(0., "gCBTwalk", 651, 700);

   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk699");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_699");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj699");
   delete h3;

   delete gCBTwalk14;
//--------------------------------------------------------------
   iCalibCBTimeWalk* gCBTwalk15 = new  iCalibCBTimeWalk( set,701 );
  
   gCBTwalk15->AutoWrite();
   gCBTwalk15->DoAll(0., "gCBTwalk", 701, 720);
   
   TH2F* h1=(TH2F*)gROOT->FindObject("hTWalk719");
   delete h1;
   
   TH2F* h2=(TH2F*)gROOT->FindObject("CaLib_CB_Walk_E_T_719");
   delete h2;

   TH1D* h3=(TH1D*)gROOT->FindObject("hTProj719");
   delete h3;

   delete gCBTwalk15;
//------------------------------------------------------------------
 //gROOT->ProcessLine(".ls");
  return;
}
