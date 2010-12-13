
/*******************************************************************
 *                                                                 *
 * Date: 20.01.2009    Author: Irakli                              *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

// List of calibration modules
//
iCalibCBpi0Energy    *gCBECalib    = 0;
iCalibCB2gTime       *gCBTCalib    = 0;
iCalibCBTimeWalk     *gCBTWalk     = 0;

iCalibPIDphi         *gPIDPhiCalib = 0;

iCalibTAPS1gEnergy   *gTAPSECalib  = 0;
iCalibTAPS2gTime     *g2TAPSCalib  = 0;

iCalibTaggerTime     *gTaggerCalib = 0;

iCalibTAPSTaggerTime *gTAPSTCalib  = 0;

iCalibTAGGERvsTAPSTime *gTAGGCalib   = 0;


#include <TGButton.h>
#include <TGButtonGroup.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TG3DLine.h>
#include <TApplication.h>

////////////////////////////////////////////////////////////////////////////////
class ButtonWindow : public TGMainFrame {
private:
  TGMainFrame *fMainFrame; 

protected:

  TGTextButton *fTB_Init;
  TGTextButton *fTB_Prev;
  TGTextButton *fTB_Next;
  TGTextButton *fTB_Print;
  TGTextButton *fTB_Subm;
  TGTextButton *fTB_DoAll;
  TGTextButton *fTB_Quit;

  TGComboBox* fCBox_Module;
  TGListBox *fLB_RunSet;

  TGNumberEntry *fNE_Veto;
  TGNumberEntry *fNE_Delay;

  Int_t fMax;

public:
  ButtonWindow();

  void CheckModules();
  void Submit();
  void DoNext();
  void DoPrev();
  void DoAll();
  void DoModulSelection(Int_t);
  void SetEnabled(Bool_t);
   
  Char_t szModule[64];
  Char_t TextBox[2000];

  ClassDef(ButtonWindow, 0)
    };


//------------------------------------------------------------
ButtonWindow::ButtonWindow() 
{
  this->CheckModules();

  // Main test window.
  fMainFrame = new TGMainFrame(gClient->GetRoot(), 600, 500);
  fMainFrame->SetWindowName("Control Panel");
  // fMainFrame->SetLayoutBroken(kTRUE);
  
  // ---------------------------------------------------------------------------------
  TGButtonGroup *horizontal = new TGButtonGroup(fMainFrame, "Run Set Window", kHorizontalFrame);
  horizontal->SetTitlePos(TGGroupFrame::kLeft);

  fCBox_Module = new TGComboBox(horizontal, "Choose calibration module");
  fCBox_Module->Resize(100, 30);
  horizontal->AddFrame(fCBox_Module, new TGLayoutHints(kLHintsExpandX));

  fCBox_Module->AddEntry("CB Energy Calibration",            0);
  fCBox_Module->AddEntry("CB Vs CB Time Calibration",        1);
  fCBox_Module->AddEntry("CB Time Walk Calibration",         2);
  fCBox_Module->AddEntry("PID Vs CB Phi Calibration",        3);
  fCBox_Module->AddEntry("TAPS Vs Tagger Time Calibration",  4);
  fCBox_Module->AddEntry("TAPS Vs TAPS Time Calibration",    5);
  fCBox_Module->AddEntry("Tagger Time Calibration",          6);
  fCBox_Module->AddEntry("TAPS Vs CB Energy Calibration",    7);
  fCBox_Module->AddEntry("TAGGER Vs TAPS Time Calibration",  8);

  fCBox_Module->Connect("Selected(Int_t)", "ButtonWindow", this, "ReadRunsets(Int_t)");
  
  fLB_RunSet = new TGListBox(horizontal);
  fLB_RunSet->Resize(200, 100);
  fLB_RunSet->SetMultipleSelections(kFALSE);            // implement this
  horizontal->AddFrame(fLB_RunSet, new TGLayoutHints(kLHintsLeft | kLHintsExpandY));
  
  fTB_Init = new TGTextButton(horizontal, "Start module");
  fTB_Init->SetTextJustify(36);
  fTB_Init->Resize(80, 50);
  fTB_Init->ChangeOptions(fTB_Init->GetOptions() | kFixedSize);
  fTB_Init->Connect("Pressed()", "ButtonWindow", this, "StartModule()");
  horizontal->AddFrame(fTB_Init, new TGLayoutHints(kLHintsRight));


  fMainFrame->AddFrame(horizontal, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
  
  // ---------------------------------------------------------------------------------
  TGButtonGroup *horizontal_2 = new TGButtonGroup(fMainFrame, "Manual navigation", kHorizontalFrame);
  horizontal_2->SetTitlePos(TGGroupFrame::kLeft);

  fTB_Prev = new TGTextButton(horizontal_2,"Prev.");
  fTB_Prev->SetTextJustify(36);
  fTB_Prev->Resize(80, 50);
  fTB_Prev->ChangeOptions(fTB_Prev->GetOptions() | kFixedSize);
  fTB_Prev->SetToolTipText("Switch to previous crystal", 200);
  fTB_Prev->Connect("Pressed()", "ButtonWindow", this, "DoPrev()");

  fNE_Veto = new TGNumberEntry(horizontal_2, (Int_t) 1, 3, -1,(TGNumberFormat::EStyle) 0,
			       (TGNumberFormat::EAttribute) 0,(TGNumberFormat::ELimit) 1, 1, 720);
  fNE_Veto->SetIntNumber(1);
  fNE_Veto->Resize(160, 50);
  horizontal_2->AddFrame(fNE_Veto, new TGLayoutHints(kLHintsLeft | kLHintsExpandY));

  fTB_Subm = new TGTextButton(horizontal_2, "Submit");
  fTB_Subm->SetTextJustify(36);
  fTB_Subm->Resize(80,50);

  fTB_Subm->ChangeOptions(fTB_Subm->GetOptions() | kFixedSize);
  fTB_Subm->SetToolTipText("Switch to previous crystal", 200);
  fTB_Subm->Connect("Released()", "ButtonWindow", this, "Submit()");

  fTB_Next = new TGTextButton(horizontal_2,"Next");
  fTB_Next->SetTextJustify(36);
  fTB_Next->SetMargins(0,0,0,0);
  fTB_Next->Resize(80, 50);
  fTB_Next->ChangeOptions(fTB_Next->GetOptions() | kFixedSize);
  fTB_Next->SetToolTipText("Switch to next crystal", 200);

  fTB_Next->Connect("Pressed()", "ButtonWindow", this, "DoNext()");

  fMainFrame->AddFrame(horizontal_2, new TGLayoutHints(kLHintsExpandX, 5,5,5,5));
     
  // ---------------------------------------------------------------------------------
  TGButtonGroup *horizontal_3 = new TGButtonGroup(fMainFrame, "Automatic navigation", kHorizontalFrame);
  horizontal_3->SetTitlePos(TGGroupFrame::kLeft);

  fNE_Delay = new TGNumberEntry(horizontal_3, (Int_t) 1, 3, -1,(TGNumberFormat::EStyle) 1,
				(TGNumberFormat::EAttribute) 1,(TGNumberFormat::ELimit) 0,0,10);
  fNE_Delay->SetIntNumber(1.);
  fNE_Delay->Resize(160, 50);
  horizontal_3->AddFrame(fNE_Delay, new TGLayoutHints(kLHintsLeft | kLHintsExpandY));

  fTB_DoAll = new TGTextButton(horizontal_3,"Start / Stop");
  fTB_DoAll->SetTextJustify(36);
  fTB_DoAll->Resize(80, 50);
  fTB_DoAll->ChangeOptions(fTB_DoAll->GetOptions() | kFixedSize);
  fTB_DoAll->SetToolTipText("Continue automatically", 200);
  fTB_DoAll->Connect("Pressed()", "ButtonWindow", this, "DoAll()");

  fTB_DoFit = new TGTextButton(horizontal_3, "Fit");
  fTB_DoFit->SetTextJustify(36);
  fTB_DoFit->Resize(80, 50);
  fTB_DoFit->ChangeOptions(fTB_DoFit->GetOptions() | kFixedSize);
  fTB_DoFit->SetToolTipText("Continue automatically", 200);
  fTB_DoFit->Connect("Pressed()", "ButtonWindow", this, "DoFit()");

  TGCheckButton *disable = new TGCheckButton(horizontal_3, "Switch state\nEnable/Disable");
  disable->SetOn();
  disable->Connect("Toggled(Bool_t)", "ButtonWindow", this, "SetEnabled(Bool_t)");
  horizontal_3->AddFrame(disable, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  fMainFrame->AddFrame(horizontal_3, new TGLayoutHints(kLHintsExpandX, 5,5,5,5));

  // ---------------------------------------------------------------------------------
  TGButtonGroup *horizontal_4 = new TGButtonGroup(fMainFrame, "Horizontal Position", kHorizontalFrame);
  horizontal_4->SetTitlePos(TGGroupFrame::kLeft);
  
  fTB_Write = new TGTextButton(horizontal_4, "Write");
  fTB_Write->Resize(80,50);
  fTB_Write->ChangeOptions(fTB_Write->GetOptions() | kFixedSize);
  fTB_Write->Connect("Pressed()", "ButtonWindow", this, "DoWrite()");

  fTB_Print = new TGTextButton(horizontal_4,"Print");
  fTB_Print->Resize(80,50);
  fTB_Print->ChangeOptions(fTB_Print->GetOptions() | kFixedSize);
  fTB_Print->Connect("Pressed()", "ButtonWindow", this, "Print()");

  // 
  fTB_Quit = new TGTextButton(horizontal_4, "Quit");
  fTB_Quit->Resize(80,50);
  fTB_Quit->ChangeOptions(fTB_Quit->GetOptions() | kFixedSize );
  fTB_Quit->Connect("Pressed()", "TApplication", gApplication, "Terminate()");

  fMainFrame->AddFrame(horizontal_4, new TGLayoutHints(kLHintsExpandX, 5,5,5,5));

  // ------------------------Text Edit for calibration file---------------------------------------------------------
  TGButtonGroup *horizontal_5 = new TGButtonGroup(fMainFrame, "Set the file Name", kHorizontalFrame);
  horizontal_5->SetTitlePos(TGGroupFrame::kLeft);

  TGFont *ufont;         // will reflect user font changes
  ufont = gClient->GetFont("-*-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1");

  TGGC   *uGC;           // will reflect user GC changes
  // graphics context changes
  GCValues_t valEntry1270;
  valEntry1270.fMask = kGCForeground | kGCBackground | kGCFillStyle | kGCFont | kGCGraphicsExposures;
  gClient->GetColorByName("#000000",valEntry1270.fForeground);
  gClient->GetColorByName("#c0c0c0",valEntry1270.fBackground);
  valEntry1270.fFillStyle = kFillSolid;
  valEntry1270.fFont = ufont->GetFontHandle();
  valEntry1270.fGraphicsExposures = kFALSE;
  uGC = gClient->GetGC(&valEntry1270, kTRUE);
  TGTextEntry *fTextEntry1270 = new TGTextEntry( horizontal_5, new TGTextBuffer(15),-1,uGC->GetGC(),ufont->GetFontStruct(),
						 kSunkenFrame | kDoubleBorder | kOwnBackground);
  fTextEntry1270->SetMaxLength(255);
  fTextEntry1270->SetAlignment(kTextLeft);
  fTextEntry1270->SetText("FilePath of root file");
  fTextEntry1270->Resize(288,fTextEntry1270->GetDefaultHeight());
  horizontal_5->AddFrame(fTextEntry1270, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
  fTextEntry1270->MoveResize(80,48,288,22);   
  sprintf(TextBox, fTextEntry1270->GetText() ); 

  fMainFrame->AddFrame(horizontal_5, new TGLayoutHints(kLHintsExpandX, 5,5,5,5));
 
  //===============================
  fMainFrame->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  fMainFrame->DontCallClose();

  // Map all subwindows of main frame
  fMainFrame->MapSubwindows();

  // Initialize the layout algorithm
  fMainFrame->Resize(fMainFrame->GetDefaultSize());

  fMainFrame->SetWMSizeHints( 600, 500, 800, 700, 0 ,0);
  fMainFrame->MapRaised();
  fMainFrame->Move(500, 500);
}

//------------------------------------------------------------
void ButtonWindow::CheckModules()
{
  if( gCBECalib )
    sprintf(szModule, "gCBECalib");
  if( g2TAPSCalib )
    sprintf(szModule, "g2TAPSCalib");
  if( gTaggerCalib )
    sprintf(szModule, "gTaggerCalib");
}

//------------------------------------------------------------
void ButtonWindow::Submit()
{
  int n = fNE_Veto->GetNumber();
  Int_t runset = (Int_t) fNE_RunSet->GetNumber();
  
  Char_t szCommand[64];
  sprintf( szCommand, "%s->DoFor(%i)", szModule, n);
  //sprintf( szCommand, "%s->DoFor(%i)", szModule, runset );
  gROOT->ProcessLine( szCommand );
}

//------------------------------------------------------------
void ButtonWindow::DoPrev()
{
  // 
  Char_t szCommand[64];
  sprintf( szCommand, "%s->Prev()", szModule );
  gROOT->ProcessLine( szCommand );
}

//------------------------------------------------------------
void ButtonWindow::DoNext()
{
  // 
  Char_t szCommand[64];
  sprintf( szCommand,
	   "%s->Next(%i)", 
	   szModule, fMax );
  gROOT->ProcessLine( szCommand );
}

//------------------------------------------------------------
void ButtonWindow::DoWrite()
{
  // 
  Char_t szCommand[64];
  sprintf( szCommand, "%s->Write()", szModule );
  gROOT->ProcessLine( szCommand );
  // printf("\n COMMENTED!!!!! \n");
}

//------------------------------------------------------------
void ButtonWindow::Print()
{
  // 
  gROOT->ProcessLine("PrintAll()");
}

//------------------------------------------------------------
void ButtonWindow::DoAll()
{
  // 
  Float_t n = fNE_Delay->GetNumber();
  
  Char_t szCommand[64];
  sprintf(szCommand, 
	  "%s->DoAll(%f,\"%s\", %i)",
	  szModule, n*1.E+3, szModule, fMax); // ms
  //   cout << szCommand << endl;

  //  
  gROOT->ProcessLine( szCommand );
}

//------------------------------------------------------------
void ButtonWindow::DoFit()
{
  Char_t szCommand[64];
  sprintf(szCommand, "%s->DoFit()", szModule); // 
  cout << szCommand << endl;

  //  
  gROOT->ProcessLine( szCommand );
}

//------------------------------------------------------------
void ButtonWindow::ReadRunsets(Int_t i)
{
  CalibData_t data;

  // get the table used in a calibration module
  switch(i)
    {
      // - - - CB vs CB Energy - - -
    case 0:
      data = ECALIB_CB_E1;
      break;
      // - - - CB vs CB time - - -
    case 1:
      data = ECALIB_CB_T0;
      break;
      // - - - CB TWalk - - -
    case 2:
      data = ECALIB_CB_WALK0;
      break;
      // - - - PID - - -
    case 3:
      data = ECALIB_PID_E1;
      break;
      // - - - TAPS vs TAGGER time - - -
    case 4:
      data = ECALIB_TAPS_T0;
      break;
      // - - - TAPS vs TAPS time - - -
    case 5:
      data = ECALIB_TAPS_T0;
      break;
      // - - - TAGGER time - - -
    case 6:
      data = ECALIB_TAGG_T0;
      break;
      // - - - TAPS vs CB Energy - - -
    case 7:
      data = ECALIB_TAPS_LG_E1;
      break;
      // - - - TAGGER vs TAPS Time - - -
    case 8:
      data = ECALIB_TAPS_T0;
      break;
      // - - - D E F A U L T - - -
    default:
      printf("No module is selected!!!\n");
    }

    // get the number of runsets
    iMySQLManager m;
    Int_t nsets = m.GetNsets(data);
    
    // fill the runsets into the list
    fLB_RunSet->RemoveAll();
    for (Int_t i = 0; i < nsets; i++)
    {
        // get the first and last runs
        Int_t first_run = m.GetFirstRunOfSet(data, i);
        Int_t last_run = m.GetLastRunOfSet(data, i);
    
        // add list entry
        Char_t tmp[256];
        sprintf(tmp, "Set %d (Run %d to %d)", i, first_run, last_run);
        fLB_RunSet->AddEntry(tmp, i);
    }

    fLB_RunSet->Layout();
}

//------------------------------------------------------------
void ButtonWindow::StartModule()
{
  Char_t szCommand[128];


  // get the selected module
  Int_t module = fCBox_Module->GetSelected();

  // get the selected runset
  Int_t runset = fLB_RunSet->GetSelected();

  switch(module)
    {
      // - - - CB vs CB Energy - - -
    case 0:
      sprintf( szModule, "gCBECalib" );
      sprintf( szCommand, "%s = new iCalibCBpi0Energy(%i)", szModule, runset );
      fMax = 720;
      gROOT->ProcessLine( szCommand );
      break;
      // - - - CB vs CB time - - -
    case 1:
      sprintf( szModule, "gCBTCalib" );
      sprintf( szCommand, "%s = new iCalibCB2gTime(%i)", szModule, runset );
      fMax = 720;
      gROOT->ProcessLine( szCommand );
      break;
      // - - - CB TWalk - - -
    case 2:
      sprintf( szModule, "gCBTWalk" );
      sprintf( szCommand, "%s = new iCalibCBTimeWalk(%i)", szModule, runset );
      fMax = 720;
      gROOT->ProcessLine( szCommand );
      break;
      // - - - PID - - -
    case 3:
      sprintf( szModule, "gPIDPhiCalib" );
      sprintf( szCommand, "%s = new iCalibPIDphi(%i)", szModule, runset );
      fMax = 24;
      gROOT->ProcessLine( szCommand );
      break;
      // - - - TAPS vs TAGGER time - - -
    case 4:
      sprintf( szModule, "gTAPSTCalib" );
      sprintf( szCommand, "%s = new iCalibTAPSTaggerTime()", szModule );
      fMax = 438;
      gROOT->ProcessLine( szCommand );
      break;
      // - - - TAPS vs TAPS time - - -
    case 5:
      sprintf( szModule, "g2TAPSCalib" );
      sprintf( szCommand, "%s = new iCalibTAPS2gTime(%i)", szModule, runset );
      fMax = 438;
      gROOT->ProcessLine( szCommand );
      break;
      // - - - TAGGER time - - -
    case 6:
      sprintf( szModule, "gTaggerCalib" );
      sprintf( szCommand, "%s = new iCalibTaggerTime(%i)", szModule , runset);
      fMax = 352;
      gROOT->ProcessLine( szCommand );
      break;
      // - - - TAPS vs CB Energy - - -
    case 7:
      sprintf( szModule, "gTAPSECalib" );
      sprintf( szCommand, "%s = new iCalibTAPS1gEnergy(%i)", szModule, runset );
      fMax = 438;
      gROOT->ProcessLine( szCommand );
      break;
      // - - - TAGGER vs TAPS Time - - -
    case 8:
      sprintf( szModule, "gTAGGCalib" );
      sprintf( szCommand, "%s = new iCalibTAGGERvsTAPSTime(%i)", szModule, runset );
      fMax = 352;
      gROOT->ProcessLine( szCommand );
      break;
      // - - - D E F A U L T - - -
    default:
      printf("No module is selected!!!\n");
    }
    
    printf("Selected Object is %s\n", szModule);  

    // start the module
    sprintf( szCommand, "%s->Start()", szModule );
    gROOT->ProcessLine(szCommand);

}

//------------------------------------------------------------
void ButtonWindow::SetEnabled(Bool_t b)
{
  cout << "Enable : " << b << endl;
}

void create_pointers()
{
  // Default Object
  //                     

  return;
}

//////////////////////////////////////////////////////////////
void gui_helper()
{
  create_pointers();

  // Main program.

  ButtonWindow *guiBW = new ButtonWindow();

  return;
}  
