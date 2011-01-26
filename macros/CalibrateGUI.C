// SVN Info: $Id$

/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// CalibrateGUI.C                                                       //
//                                                                      //
// GUI calibrations using CaLib.                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


class ButtonWindow;

// global variables
TList* gCalibrations;
TList* gCaLibModules;
void* gCurrentModule;
Bool_t gCalibSelected;
ButtonWindow* gMainWindow;


class ButtonWindow : public TGMainFrame 
{

private:
    TGMainFrame *fMainFrame; 
    TGTextButton *fTB_Init;
    TGTextButton *fTB_Prev;
    TGTextButton *fTB_Next;
    TGTextButton *fTB_Print;
    TGTextButton *fTB_PrintChanges;
    TGTextButton *fTB_Goto;
    TGTextButton *fTB_DoAll;
    TGTextButton *fTB_Quit;
    TGComboBox* fCBox_Calibration;
    TGComboBox* fCBox_Module;
    TGListBox *fLB_RunSet;
    TGNumberEntry *fNE_Elem;
    TGNumberEntry *fNE_Delay;

public:
    ButtonWindow();

    void Goto();
    void DoNext();
    void DoPrev();
    void DoAll();
    void DoModulSelection(Int_t);
};


//______________________________________________________________________________
ButtonWindow::ButtonWindow() 
{
    // Main test window.
    fMainFrame = new TGMainFrame(gClient->GetRoot(), 600, 500);
    fMainFrame->SetWindowName("CaLib Control Panel");
    // fMainFrame->SetLayoutBroken(kTRUE);
    
    TGButtonGroup *horizontal0 = new TGButtonGroup(fMainFrame, "Calibration configuration", kHorizontalFrame);
    horizontal0->SetTitlePos(TGGroupFrame::kLeft);

    fCBox_Calibration = new TGComboBox(horizontal0, "Choose calibration");
    fCBox_Calibration->Resize(100, 30);
    horizontal0->AddFrame(fCBox_Calibration, new TGLayoutHints(kLHintsExpandX));
      
    // fill calibrations
    gCalibrations = TCMySQLManager::GetManager()->GetAllCalibrations();
    for (Int_t i = 0; i < gCalibrations->GetSize(); i++)
    {
        TObjString* s = (TObjString*) gCalibrations->At(i);
        fCBox_Calibration->AddEntry(s->GetString().Data(), i);
    }
    
    fCBox_Calibration->Connect("Selected(Int_t)", "ButtonWindow", this, "EnableModuleSelection(Int_t)");

    fMainFrame->AddFrame(horizontal0, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    // ---------------------------------------------------------------------------------
    TGButtonGroup *horizontal = new TGButtonGroup(fMainFrame, "Main configuration", kHorizontalFrame);
    horizontal->SetTitlePos(TGGroupFrame::kLeft);

    fCBox_Module = new TGComboBox(horizontal, "Choose calibration module");
    fCBox_Module->Resize(100, 30);
    horizontal->AddFrame(fCBox_Module, new TGLayoutHints(kLHintsExpandX));
      
    // fill modules
    for (Int_t i = 0; i < gCaLibModules->GetSize(); i++)
    {
        TCCalib* cmod = (TCCalib*) gCaLibModules->At(i);
        fCBox_Module->AddEntry(cmod->GetTitle(), i);
    }

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

    fNE_Elem = new TGNumberEntry(horizontal_2, (Int_t) 1, 3, -1,(TGNumberFormat::EStyle) 0,
                      (TGNumberFormat::EAttribute) 0,(TGNumberFormat::ELimit) 1, 1, 720);
    fNE_Elem->SetIntNumber(1);
    fNE_Elem->Resize(160, 50);
    horizontal_2->AddFrame(fNE_Elem, new TGLayoutHints(kLHintsLeft | kLHintsExpandY));

    fTB_Goto = new TGTextButton(horizontal_2, "Go to");
    fTB_Goto->SetTextJustify(36);
    fTB_Goto->Resize(80,50);

    fTB_Goto->ChangeOptions(fTB_Goto->GetOptions() | kFixedSize);
    fTB_Goto->SetToolTipText("Go to crystal", 200);
    fTB_Goto->Connect("Released()", "ButtonWindow", this, "Goto()");

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
                            (TGNumberFormat::EAttribute) 1,(TGNumberFormat::ELimit) 1,1,10);
    fNE_Delay->SetIntNumber(1.);
    fNE_Delay->Resize(160, 50);
    horizontal_3->AddFrame(fNE_Delay, new TGLayoutHints(kLHintsLeft | kLHintsExpandY));

    fTB_DoAll = new TGTextButton(horizontal_3,"Start");
    fTB_DoAll->SetTextJustify(36);
    fTB_DoAll->Resize(80, 50);
    fTB_DoAll->ChangeOptions(fTB_DoAll->GetOptions() | kFixedSize);
    fTB_DoAll->SetToolTipText("Continue automatically", 200);
    fTB_DoAll->Connect("Pressed()", "ButtonWindow", this, "DoAll()");

    fTB_Stop = new TGTextButton(horizontal_3, "Stop");
    fTB_Stop->SetTextJustify(36);
    fTB_Stop->Resize(80, 50);
    fTB_Stop->ChangeOptions(fTB_Stop->GetOptions() | kFixedSize);
    fTB_Stop->SetToolTipText("Stop processing", 200);
    fTB_Stop->Connect("Pressed()", "ButtonWindow", this, "Stop()");

    fMainFrame->AddFrame(horizontal_3, new TGLayoutHints(kLHintsExpandX, 5,5,5,5));

    // ---------------------------------------------------------------------------------
    TGButtonGroup *horizontal_4 = new TGButtonGroup(fMainFrame, "Calibration control", kHorizontalFrame);
    horizontal_4->SetTitlePos(TGGroupFrame::kLeft);

    fTB_Write = new TGTextButton(horizontal_4, "Write");
    fTB_Write->Resize(80,50);
    fTB_Write->ChangeOptions(fTB_Write->GetOptions() | kFixedSize);
    fTB_Write->Connect("Pressed()", "ButtonWindow", this, "DoWrite()");

    fTB_Print = new TGTextButton(horizontal_4,"Print");
    fTB_Print->Resize(80,50);
    fTB_Print->ChangeOptions(fTB_Print->GetOptions() | kFixedSize);
    fTB_Print->Connect("Pressed()", "ButtonWindow", this, "Print()");

    fTB_PrintChanges = new TGTextButton(horizontal_4,"Changes");
    fTB_PrintChanges->Resize(80,50);
    fTB_PrintChanges->ChangeOptions(fTB_Print->GetOptions() | kFixedSize);
    fTB_PrintChanges->Connect("Pressed()", "ButtonWindow", this, "PrintChanges()");

    fTB_Quit = new TGTextButton(horizontal_4, "Quit");
    fTB_Quit->Resize(80,50);
    fTB_Quit->ChangeOptions(fTB_Quit->GetOptions() | kFixedSize );
    fTB_Quit->Connect("Pressed()", "ButtonWindow", this, "Quit()");

    fMainFrame->AddFrame(horizontal_4, new TGLayoutHints(kLHintsExpandX, 5,5,5,5));
    fMainFrame->Connect("CloseWindow()", "ButtonWindow", this, "Quit()");
    fMainFrame->DontCallClose();

    // Map all subwindows of main frame
    fMainFrame->MapSubwindows();

    // Initialize the layout algorithm
    fMainFrame->Resize(fMainFrame->GetDefaultSize());

    fMainFrame->SetWMSizeHints( 600, 400, 800, 700, 0 ,0);
    fMainFrame->MapRaised();
    fMainFrame->Move(500, 500);
}

//______________________________________________________________________________
void ButtonWindow::Goto()
{
    // Go to a certain element in the current module.

    // get the element number
    Int_t n = fNE_Elem->GetNumber();
  
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->ProcessElement(n);
}

//______________________________________________________________________________
void ButtonWindow::DoPrev()
{
    // Go to the previous element in the current module.
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->Previous();
}

//______________________________________________________________________________
void ButtonWindow::DoNext()
{
    // Go to the next element in the current module.
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->Next();
}

//______________________________________________________________________________
void ButtonWindow::DoWrite()
{
    // Write the values of the current module to the database.

    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->Write();
}

//______________________________________________________________________________
void ButtonWindow::Print()
{
    // Print the values obtained by the current module.
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->PrintValues();
}

//______________________________________________________________________________
void ButtonWindow::PrintChanges()
{
    // Print the changed values obtained by the current module.
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->PrintValuesChanged();
}

//______________________________________________________________________________
void ButtonWindow::Quit()
{
    // Quit the application.
    
    // delete list of modules
    delete gCaLibModules;
    
    // quit ROOT
    gApplication->Terminate();
}

//______________________________________________________________________________
void ButtonWindow::DoAll()
{
    // Process all elements automatically.

    Float_t delay = fNE_Delay->GetNumber();
    
    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->ProcessAll(1000*delay);
}

//______________________________________________________________________________
void ButtonWindow::Stop()
{
    // Stop automatic processing of the current module.

    if (gCurrentModule)
        ((TCCalib*)gCurrentModule)->StopProcessing();
}

//______________________________________________________________________________
void ButtonWindow::EnableModuleSelection(Int_t i)
{
    // Enable the module selection.
    
    gCalibSelected = kTRUE;
}

//______________________________________________________________________________
void ButtonWindow::ReadRunsets(Int_t i)
{
    // Read the runsets for the calibration data of the 'i'-th module
    // in the module selection combo box
    
    // check if calibration was selected
    if (!gCalibSelected)
    {
        TGMsgBox* msg = new TGMsgBox(gClient->GetRoot(), gMainWindow, "Error", "Please select first "
                                     "the calibration you want to work with!",
                                     kMBIconStop, kMBOk, 0, kFitWidth | kFitHeight, kTextLeft);
        return;
    }

    // get the selected calibration
    TObjString* calibration = (TObjString*) gCalibrations->At(fCBox_Calibration->GetSelected());

    // get the calibration data of the module
    TCCalib* c = (TCCalib*) gCaLibModules->At(i);
    CalibData_t data = c->GetCalibData();
    
    // get the number of runsets
    Int_t nsets = TCMySQLManager::GetManager()->GetNsets(data, calibration->GetString().Data());
    
    // fill the runsets into the list
    fLB_RunSet->RemoveAll();
    for (Int_t i = 0; i < nsets; i++)
    {
        // get the first and last runs
        Int_t first_run = TCMySQLManager::GetManager()->GetFirstRunOfSet(data, calibration->GetString().Data(), i);
        Int_t last_run = TCMySQLManager::GetManager()->GetLastRunOfSet(data, calibration->GetString().Data(), i);
    
        // add list entry
        Char_t tmp[256];
        sprintf(tmp, "Set %d (Run %d to %d)", i, first_run, last_run);
        fLB_RunSet->AddEntry(tmp, i);
    }
    
    // update list box
    fLB_RunSet->Layout();
}

//______________________________________________________________________________
void ButtonWindow::StartModule()
{
    // Start the selected module.

    // get the selected module
    Int_t module = fCBox_Module->GetSelected();

    // get the selected runset
    Int_t runset = fLB_RunSet->GetSelected();

    // get the calibration module
    gCurrentModule = (TCCalib*) gCaLibModules->At(module);
    
    // get the selected calibration
    TObjString* calibration = (TObjString*) gCalibrations->At(fCBox_Calibration->GetSelected());
    
    // start the module
    ((TCCalib*)gCurrentModule)->Start(calibration->GetString().Data(), runset);
}

//______________________________________________________________________________
void CreateModuleList()
{
    // Find all calibration modules and create a list.
    
    // create the module list
    gCaLibModules = new TList();
    gCaLibModules->SetOwner(kTRUE);

    // init class list
    gClassTable->Init();

    // loop over all classes
    Int_t nClasses = gClassTable->Classes();
    for (Int_t i = 0; i < nClasses; i++)
    {
        // get class name
        TString c(gClassTable->At(i));

        // get TCCalib* classes
        if (c.BeginsWith("TCCalib"))
        {
            // skip non-module classes
            if (c == "TCCalib") continue;
            if (c == "TCCalibPed") continue;
            if (c == "TCCalibTime") continue;
            if (c == "TCCalibTAPSLED") continue;

            // add module to list if it is really a module
            TClass tc(c.Data());
            if (tc.InheritsFrom("TCCalib")) gCaLibModules->Add((TCCalib*) tc.New());
        }
    }
}

//______________________________________________________________________________
void CalibrateGUI()
{
    // Main method.

    // load CaLib
    gSystem->Load("libCaLib.so");
    
    // find CaLib modules
    CreateModuleList();
    
    // no current module
    gCurrentModule = 0;
    
    // calibration not yet selected
    gCalibSelected = kFALSE;

    // Main method.
    gMainWindow = new ButtonWindow();
}

