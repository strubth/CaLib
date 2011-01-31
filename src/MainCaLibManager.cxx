// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// CaLibManager                                                         //
//                                                                      //
// Manage a CaLib database.                                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include <ncurses.h>
#include <signal.h>

#include "TCMySQLManager.h"

#define KEY_ENTER_MINE 13
#define KEY_ESC 27


// locally used enum for run entries
enum ERunEntry
{
    kPATH,
    kTARGET,
    kTARGET_POL,
    kTARGET_POL_DEG,
    kBEAM_POL,
    kBEAM_POL_DEG
};
typedef ERunEntry RunEntry_t;

// locally used enum for calibration types
enum ECalibType
{
    kTARG_POS,
    kTAGG_TIME,
    kCB_ENERGY,
    kCB_QUAD_ENERGY,
    kCB_TIME,
    kCB_TIME_WALK,
    kTAPS_PED_LG,
    kTAPS_PED_SG,
    kTAPS_ENERGY_LG,
    kTAPS_ENERGY_SG,
    kTAPS_QUAD_ENERGY,
    kTAPS_TIME,
    kTAPS_LED1,
    kTAPS_LED2,
    kPID_PHI,
    kPID_DROOP,
    kPID_ENERGY,
    kPID_TIME,
    kVETO_PED,
    kVETO_ENERGY,
    kVETO_TIME
};
typedef ECalibType CalibType_t;

// global variables
Int_t gNrow;
Int_t gNcol;
Char_t gCalibration[256];
CalibType_t gCalibrationType;
CalibData_t gCalibrationData;

// function prototypes
void MainMenu();
void RunEditor();
void CalibEditor();
void PrintStatusMessage(const Char_t* message);
void SelectCalibration();
void SelectCalibrationData();

//______________________________________________________________________________
void Finish(Int_t sig)
{   
    // Exit the program.

    endwin();    exit(0);
}

//______________________________________________________________________________
void DrawHeader(const Char_t* title = "CaLib Manager")
{
    // Draw a header.

    // turn on bold
    attron(A_BOLD);

    // draw lines
    for (Int_t i = 0; i < gNcol; i++) 
    {   
        mvprintw(0, i, "#");
        mvprintw(4, i, "#");
    }
    
    // draw lines
    for (Int_t i = 1; i < 5; i++)
    {
        mvprintw(i, 0, "#");
        mvprintw(i, gNcol-1, "#");
    }

    // draw title
    mvprintw(2, (gNcol-strlen(title)) / 2, "%s", title);
    
    // turn off bold
    attroff(A_BOLD);
}

//______________________________________________________________________________
Int_t ShowMenu(const Char_t* title, const Char_t* message, 
               Int_t nEntries, const Char_t* entries[], Int_t active)
{
    // Show the main menu.

    // clear the screen
    clear();

    // draw header
    DrawHeader();
    
    // draw title
    attron(A_UNDERLINE);
    mvprintw(6, 2, "%s", title);
    attroff(A_UNDERLINE);
    
    // draw message
    mvprintw(8, 2, "%s:", message);

    // draw entries
    for (Int_t i = 0; i < nEntries; i++)
    {
        if (i == active) attron(A_REVERSE);
        mvprintw(10+i, 2, "%s", entries[i]);
        if (i == active) attroff(A_REVERSE);
    }

    // user information
    PrintStatusMessage("Use UP and DOWN keys to select - hit ENTER or RIGHT key to confirm");
    
    // wait for input
    for (;;)
    {
        // get key
        Int_t c = getch();
     
        //
        // decide what to do
        //
        
        // go to previous entry
        if (c == KEY_UP)
        {
            if (active > 0) 
            {
                return ShowMenu(title, message, nEntries, entries, active-1);
            }
        }
        // go to next entry
        else if (c == KEY_DOWN)
        {
            if (active < nEntries-1) 
            {
                return ShowMenu(title, message, nEntries, entries, active+1);
            }
        }
        // choose entry
        else if (c == KEY_ENTER_MINE || c == KEY_ENTER || c == KEY_RIGHT)
        {
            return active;
        }
    }
}

//______________________________________________________________________________
void WriteTableEntry(WINDOW* win, const Char_t* str, Int_t colLength, 
                     Int_t att = A_NORMAL)
{
    // Write the table entry 'str' to the window 'win' using the column length 'colLength'
    // at the current cursor position.
    
    // set attribute
    wattron(win, att);

    // check if string is empty
    if (strlen(str) == 0)
    {
        UInt_t half = colLength / 2;

        // write '-' with spaces
        for (UInt_t i = 0; i < half; i++) wprintw(win, " ");
        wprintw(win, "-");
        for (Int_t i = half+1; i < colLength+4; i++) wprintw(win, " ");
    }
    else
    {
        // write entry
        wprintw(win, "%s", str);

        // fill with spaces
        for (Int_t i = strlen(str); i < colLength+4; i++) wprintw(win, " ");
    }
    
    // unset attribute
    wattroff(win, att);
}

//______________________________________________________________________________
WINDOW* FormatRunTable(TCContainer& c, Int_t* outColLengthTot, WINDOW** outHeader)
{
    // Format the run table using the runs in 'c' and return the created window.
    // Save the maximum column length to 'outColLengthTot'.
    // Save the header window in 'outHeader'.

    // get number of runs
    Int_t nRuns = c.GetNRuns();

    // define col headers
    const Char_t* colHead[] = { "Run", "Path", "File name", "Time", "Description",
                                "Run note", "File size", "Target", "Target pol.",
                                "Target pol. deg.", "Beam pol.", "Beam pol. deg." };

    // determine maximum col lengths
    Char_t tmp_str[256];
    const Char_t* tmp = 0;
    UInt_t colLength[12];

    // loop over headers
    for (Int_t i = 0; i < 12; i++) colLength[i] = strlen(colHead[i]);

    // loop over runs
    for (Int_t i = 0; i < nRuns; i++)
    {
        // run number
        sprintf(tmp_str, "%d", c.GetRun(i)->GetRun());
        if (strlen(tmp_str) > colLength[0]) colLength[0] = strlen(tmp_str);

        // path
        tmp = c.GetRun(i)->GetPath();
        if (strlen(tmp) > colLength[1]) colLength[1] = strlen(tmp);
    
        // file name
        tmp = c.GetRun(i)->GetFileName();
        if (strlen(tmp) > colLength[2]) colLength[2] = strlen(tmp);
        
        // time
        tmp = c.GetRun(i)->GetTime();
        if (strlen(tmp) > colLength[3]) colLength[3] = strlen(tmp);
        
        // description
        tmp = c.GetRun(i)->GetDescription();
        if (strlen(tmp) > colLength[4]) colLength[4] = strlen(tmp);
        
        // run note
        tmp = c.GetRun(i)->GetRunNote();
        if (strlen(tmp) > colLength[5]) colLength[5] = strlen(tmp);
 
        // size
        sprintf(tmp_str, "%lld", c.GetRun(i)->GetSize());
        if (strlen(tmp_str) > colLength[6]) colLength[6] = strlen(tmp_str);

        // target
        tmp = c.GetRun(i)->GetTarget();
        if (strlen(tmp) > colLength[7]) colLength[7] = strlen(tmp);
        
        // target polarization
        tmp = c.GetRun(i)->GetTargetPol();
        if (strlen(tmp) > colLength[8]) colLength[8] = strlen(tmp);
        
        // target polarization degree
        sprintf(tmp_str, "%f", c.GetRun(i)->GetTargetPolDeg());
        if (strlen(tmp_str) > colLength[9]) colLength[9] = strlen(tmp_str);
        
        // beam polarization
        tmp = c.GetRun(i)->GetBeamPol();
        if (strlen(tmp) > colLength[10]) colLength[10] = strlen(tmp);
 
        // beam polarization degree
        sprintf(tmp_str, "%f", c.GetRun(i)->GetBeamPolDeg());
        if (strlen(tmp_str) > colLength[11]) colLength[11] = strlen(tmp_str);
    }
    
    // calculate the maximum col length (12*4 spaces)
    Int_t colLengthTot = 12*4;
    for (Int_t i = 0; i < 12; i++) colLengthTot += colLength[i];
    *outColLengthTot = colLengthTot;

    // create the table and the header window
    WINDOW* table = newpad(nRuns, colLengthTot);
    WINDOW* header = newpad(1, colLengthTot);
     
    // add header content
    wmove(header, 0, 0);
    for (Int_t i = 0; i < 12; i++) 
        WriteTableEntry(header, colHead[i], colLength[i], A_BOLD);

    // add table content
    for (Int_t i = 0; i < nRuns; i++)
    {
        // move cursor
        wmove(table, i, 0);

        // run number
        sprintf(tmp_str, "%d", c.GetRun(i)->GetRun());
        WriteTableEntry(table, tmp_str, colLength[0]);

        // path
        WriteTableEntry(table, c.GetRun(i)->GetPath(), colLength[1]);
        
        // file name
        WriteTableEntry(table, c.GetRun(i)->GetFileName(), colLength[2]);
        
        // time
        WriteTableEntry(table, c.GetRun(i)->GetTime(), colLength[3]);
        
        // description
        WriteTableEntry(table, c.GetRun(i)->GetDescription(), colLength[4]);
        
        // run note
        WriteTableEntry(table, c.GetRun(i)->GetRunNote(), colLength[5]);
        
        // size
        sprintf(tmp_str, "%lld", c.GetRun(i)->GetSize());
        WriteTableEntry(table, tmp_str, colLength[6]);

        // target
        WriteTableEntry(table, c.GetRun(i)->GetTarget(), colLength[7]);
        
        // target polarization
        WriteTableEntry(table, c.GetRun(i)->GetTargetPol(), colLength[8]);
        
        // target polarization degree
        sprintf(tmp_str, "%f", c.GetRun(i)->GetTargetPolDeg());
        WriteTableEntry(table, tmp_str, colLength[9]);

        // beam polarization
        WriteTableEntry(table, c.GetRun(i)->GetBeamPol(), colLength[10]);
        
        // beam polarization degree
        sprintf(tmp_str, "%f", c.GetRun(i)->GetBeamPolDeg());
        WriteTableEntry(table, tmp_str, colLength[11]);
    }
    
    *outHeader = header;

    return table;
}

//______________________________________________________________________________
WINDOW* FormatCalibTable(TCContainer& c, Int_t* outColLengthTot, WINDOW** outHeader)
{
    // Format the calibration table using the calibrations in 'c' and return 
    // the created window.
    // Save the maximum column length to 'outColLengthTot'.
    // Save the header window in 'outHeader'.

    // get number of calibrations
    Int_t nCalib = c.GetNCalibrations();

    // define col headers
    const Char_t* colHead[] = { "First run", "Last run", "Change time", "Description" };

    // determine maximum col lengths
    Char_t tmp_str[256];
    const Char_t* tmp = 0;
    UInt_t colLength[4];

    // loop over headers
    for (Int_t i = 0; i < 4; i++) colLength[i] = strlen(colHead[i]);

    // loop over calibrations
    for (Int_t i = 0; i < nCalib; i++)
    {
        // first run
        sprintf(tmp_str, "%d", c.GetCalibration(i)->GetFirstRun());
        if (strlen(tmp_str) > colLength[0]) colLength[0] = strlen(tmp_str);
        
        // last run
        sprintf(tmp_str, "%d", c.GetCalibration(i)->GetLastRun());
        if (strlen(tmp_str) > colLength[1]) colLength[1] = strlen(tmp_str);

        // change time
        tmp = c.GetCalibration(i)->GetChangeTime();
        if (strlen(tmp) > colLength[2]) colLength[2] = strlen(tmp);
        
        // description
        tmp = c.GetCalibration(i)->GetDescription();
        if (strlen(tmp) > colLength[3]) colLength[3] = strlen(tmp);
    }
    
    // calculate the maximum col length (4*4 spaces)
    Int_t colLengthTot = 4*4;
    for (Int_t i = 0; i < 4; i++) colLengthTot += colLength[i];
    *outColLengthTot = colLengthTot;

    // create the table and the header window
    WINDOW* table = newpad(nCalib, colLengthTot);
    WINDOW* header = newpad(1, colLengthTot);
     
    // add header content
    wmove(header, 0, 0);
    for (Int_t i = 0; i < 4; i++) 
        WriteTableEntry(header, colHead[i], colLength[i], A_BOLD);

    // add table content
    for (Int_t i = 0; i < nCalib; i++)
    {
        // move cursor
        wmove(table, i, 0);

        // first run
        sprintf(tmp_str, "%d", c.GetCalibration(i)->GetFirstRun());
        WriteTableEntry(table, tmp_str, colLength[0]);

        // last run
        sprintf(tmp_str, "%d", c.GetCalibration(i)->GetLastRun());
        WriteTableEntry(table, tmp_str, colLength[1]);

        // change time
        WriteTableEntry(table, c.GetCalibration(i)->GetChangeTime(), colLength[2]);
        
        // description
        WriteTableEntry(table, c.GetCalibration(i)->GetDescription(), colLength[3]);
    }
    
    *outHeader = header;

    return table;
}

//______________________________________________________________________________
void PrintStatusMessage(const Char_t* message)
{
    // Print a status message.
       
    mvprintw(gNrow-1, 0, message);
}

//______________________________________________________________________________
void RunBrowser()
{
    // Show the run browser.
    
    // create a CaLib container
    TCContainer c("container");

    // dump all runs
    TCMySQLManager::GetManager()->DumpRuns(&c);
    
    // get number of runs
    Int_t nRuns = c.GetNRuns();

    // clear the screen
    clear();

    // draw header
    DrawHeader();
    
    // draw title
    attron(A_UNDERLINE);
    mvprintw(6, 2, "RUN BROWSER");
    attroff(A_UNDERLINE);
    
    // build the windows
    Int_t colLengthTot;
    WINDOW* header = 0;
    WINDOW* table = FormatRunTable(c, &colLengthTot, &header);

    // user information
    Char_t tmp[256];
    sprintf(tmp, "%d runs found. Use UP/DOWN keys to scroll "
                 "(PAGE-UP or 'p' / PAGE-DOWN or 'n' for fast mode) - hit ESC or 'q' to exit", nRuns);
    PrintStatusMessage(tmp);
 
    // refresh windows
    refresh();
    prefresh(header, 0, 0, 8, 2, 8, gNcol-3);
    prefresh(table, 0, 0, 9, 2, gNrow-3, gNcol-3);
   
    Int_t first_row = 0;
    Int_t first_col = 0;
    Int_t winHeight = gNrow-3-9;
    Int_t winWidth = gNcol;

    // wait for input
    for (;;)
    {
        // get key
        Int_t c = getch();
        
        //
        // decide what to do
        //
        
        // go up one entry
        if (c == KEY_UP)
        {
            if (first_row > 0) first_row--;
        }
        // go down one entry
        else if (c == KEY_DOWN)
        {
            if (first_row < nRuns-winHeight-1) first_row++;
        }
        // go up one page
        else if (c == KEY_PPAGE || c == 'p')
        {
            if (first_row > winHeight-1) first_row -= winHeight+1;
            else first_row = 0;
        }
        // go down one page
        else if (c == KEY_NPAGE || c == 'n')
        {
            if (first_row < nRuns-winHeight-winHeight) first_row += winHeight+1;
            else first_row = nRuns-winHeight-1;
        }
        // go right
        else if (c == KEY_RIGHT)
        {
            if (first_col < colLengthTot-winWidth) first_col += 10;
            else first_row = colLengthTot-winWidth;
        }
        // go left
        else if (c == KEY_LEFT)
        {
            if (first_col > 0) first_col -= 10;
            else continue;
        }
        // exit
        else if (c == KEY_ESC || c == 'q') break;

        // update window
        prefresh(header, 0, first_col, 8, 2, 8, gNcol-3);
        prefresh(table, first_row, first_col, 9, 2, gNrow-3, gNcol-3);
    }
    
    // clean-up
    delwin(table);
    delwin(header);

    // go back to run editor
    RunEditor();
}

//______________________________________________________________________________
void CalibBrowser()
{
    // Show the calibration browser.

    // select a calibration
    SelectCalibration();

    // select calibration data
    SelectCalibrationData();
    
    // create a CaLib container
    TCContainer c("container");

    // dump calibrations
    TCMySQLManager::GetManager()->DumpCalibrations(&c, gCalibration, gCalibrationData);
    
    // get number of calibrations
    Int_t nCalib = c.GetNCalibrations();

    // clear the screen
    clear();

    // draw header
    DrawHeader();
    
    // draw title
    attron(A_UNDERLINE);
    mvprintw(6, 2, "CALIBRATION BROWSER");
    attroff(A_UNDERLINE);
    
    // build the windows
    Int_t colLengthTot;
    WINDOW* header = 0;
    WINDOW* table = FormatCalibTable(c, &colLengthTot, &header);

    // user information
    Char_t tmp[256];
    sprintf(tmp, "%d sets found. Use UP/DOWN keys to scroll "
                 "(PAGE-UP or 'p' / PAGE-DOWN or 'n' for fast mode) - hit ESC or 'q' to exit", nCalib);
    PrintStatusMessage(tmp);
 
    // refresh windows
    refresh();
    prefresh(header, 0, 0, 8, 2, 8, gNcol-3);
    prefresh(table, 0, 0, 9, 2, gNrow-3, gNcol-3);
   
    Int_t first_row = 0;
    Int_t first_col = 0;
    Int_t winHeight = gNrow-3-9;
    Int_t winWidth = gNcol;

    // wait for input
    for (;;)
    {
        // get key
        Int_t c = getch();
        
        //
        // decide what to do
        //
        
        // go up one entry
        if (c == KEY_UP)
        {
            if (first_row > 0) first_row--;
        }
        // go down one entry
        else if (c == KEY_DOWN)
        {
            if (first_row < nCalib-winHeight-1) first_row++;
        }
        // go up one page
        else if (c == KEY_PPAGE || c == 'p')
        {
            if (first_row > winHeight-1) first_row -= winHeight+1;
            else first_row = 0;
        }
        // go down one page
        else if (c == KEY_NPAGE || c == 'n')
        {
            if (first_row < nCalib-winHeight-winHeight) first_row += winHeight+1;
            else first_row = nCalib-winHeight-1;
        }
        // go right
        else if (c == KEY_RIGHT)
        {
            if (first_col < colLengthTot-winWidth) first_col += 10;
            else first_row = colLengthTot-winWidth;
        }
        // go left
        else if (c == KEY_LEFT)
        {
            if (first_col > 0) first_col -= 10;
            else continue;
        }
        // exit
        else if (c == KEY_ESC || c == 'q') break;

        // update window
        prefresh(header, 0, first_col, 8, 2, 8, gNcol-3);
        prefresh(table, first_row, first_col, 9, 2, gNrow-3, gNcol-3);
    }
    
    // clean-up
    delwin(table);
    delwin(header);

    // go back to calibration editor
    CalibEditor();
}

//______________________________________________________________________________
void ChangeRunEntry(const Char_t* title, const Char_t* name, RunEntry_t entry)
{
    // Change the run entry 'entry' with name for a certain range of runs.
    // Use 'title' as the window title.
    
    Int_t first_run;
    Int_t last_run;
    Char_t new_value[256];
    Char_t answer[16];

    // clear the screen
    clear();
    
    // echo input 
    echo();
 
    // draw header
    DrawHeader();
    
    // draw title
    attron(A_UNDERLINE);
    mvprintw(6, 2, title);
    attroff(A_UNDERLINE);
 
    // ask first run
    mvprintw(8, 2, "Enter first run                            : ");
    scanw("%d", &first_run);

    // ask last run
    mvprintw(9, 2, "Enter last run                             : ");
    scanw("%d", &last_run);

    // ask new value 
    mvprintw(10, 2, "Enter new %-32s : ", name);
    scanw("%s", new_value);

    // ask confirmation
    mvprintw(12, 2, "Changing %s for runs %d to %d to '%s'", name, first_run, last_run, new_value);
    mvprintw(14, 6, "Are you sure to continue? (yes/no) : ");
    scanw("%s", answer);
    if (strcmp(answer, "yes")) 
    {
        mvprintw(16, 2, "Aborted.");
    }
    else
    {
        Bool_t ret = kFALSE;
        
        // check what to change
        if (entry == kPATH) 
            ret = TCMySQLManager::GetManager()->ChangeRunPath(first_run, last_run, new_value);
        else if (entry == kTARGET)
            ret = TCMySQLManager::GetManager()->ChangeRunTarget(first_run, last_run, new_value);
        else if (entry == kTARGET_POL)
            ret = TCMySQLManager::GetManager()->ChangeRunTargetPol(first_run, last_run, new_value);
        else if (entry == kTARGET_POL_DEG)
            ret = TCMySQLManager::GetManager()->ChangeRunTargetPolDeg(first_run, last_run, atof(new_value));
        else if (entry == kBEAM_POL)
            ret = TCMySQLManager::GetManager()->ChangeRunBeamPol(first_run, last_run, new_value);
        else if (entry == kBEAM_POL_DEG)
            ret = TCMySQLManager::GetManager()->ChangeRunBeamPolDeg(first_run, last_run, atof(new_value));

        // check return value
        if (ret)
        {
            mvprintw(16, 2, "%s for runs %d to %d was successfully changed to '%s'", 
                            name, first_run, last_run, new_value);
        }
        else
            mvprintw(16, 2, "There was an error during %s changing for runs %d to %d!", 
                            name, first_run, last_run);
    }

    // user information
    PrintStatusMessage("Hit ESC or 'q' to exit");
  
    // wait for input
    for (;;)
    {
        // get key
        Int_t c = getch();
        
        // leave loop
        if (c == KEY_ESC || c == 'q') break;
    }

    // don't echo input 
    noecho();
 
    // go back to run editor
    RunEditor();
}

//______________________________________________________________________________
void SelectCalibration()
{
    // Show the calibration selection.
    
    // get all calibrations
    TList* c = TCMySQLManager::GetManager()->GetAllCalibrations();

    // check if there are some calibrations
    if (!c) return;

    // get number of calibrations
    Int_t nCalib = c->GetSize();

    // menu configuration
    const Char_t mTitle[] = "CALIBRATION SELECTION";
    const Char_t mMsg[] = "Select a calibration";
    const Int_t mN = nCalib;
    Char_t* mEntries[mN];
    for (Int_t i = 0; i < mN; i++) mEntries[i] = new Char_t[256];
 
    // fill calibrations
    TIter next(c);
    TObjString* s;
    Int_t d = 0;
    while ((s = (TObjString*)next()))
        strcpy(mEntries[d++], s->GetString().Data());

    // clean-up
    delete c;

    // show menu
    Int_t choice = ShowMenu(mTitle, mMsg, mN, (const Char_t**)mEntries, 0);

    // save selected calibration
    strcpy(gCalibration, mEntries[choice]);

    // clean-up
    for (Int_t i = 0; i < mN; i++) delete [] mEntries[i];
}

//______________________________________________________________________________
void SelectCalibrationType()
{
    // Show the calibration type selection.
    
    // menu configuration
    const Char_t mTitle[] = "CALIBRATION TYPE SELECTION";
    const Char_t mMsg[] = "Select a calibration type";
    const Int_t mN = 21;
    const Char_t* mEntries[] = { "Target position",
                                 "Tagger time",
                                 "CB energy",
                                 "CB quadratic energy correction",
                                 "CB time",
                                 "CB time walk",
                                 "TAPS LG pedestal",
                                 "TAPS SG pedestal",
                                 "TAPS LG energy",
                                 "TAPS SG energy",
                                 "TAPS quadratic energy correction",
                                 "TAPS time",
                                 "TAPS LED1 threshold",
                                 "TAPS LED2 threshold",
                                 "PID phi angle",
                                 "PID droop correction",
                                 "PID energy",
                                 "PID time",
                                 "Veto pedestal",
                                 "Veto energy",
                                 "Veto time" };
    
    // show menu
    Int_t choice = ShowMenu(mTitle, mMsg, mN, mEntries, 0);

    // save selected calibration type
    gCalibrationType = (CalibType_t)choice;
}

//______________________________________________________________________________
void SelectCalibrationData()
{
    // Show the calibration data selection.
    
    // menu configuration
    const Char_t mTitle[] = "CALIBRATION DATA SELECTION";
    const Char_t mMsg[] = "Select a calibration data";
    const Int_t mN = TCConfig::kCalibNData-1;
    
    // show menu
    Int_t choice = ShowMenu(mTitle, mMsg, mN, TCConfig::kCalibDataNames+1, 0);
    
    // save selected calibration type
    gCalibrationData = (CalibData_t)(choice+1);
}

//______________________________________________________________________________
void RenameCalibration()
{
    // Rename a calibration.
    
    Char_t newName[256];
    Char_t answer[16];

    // select a calibration
    SelectCalibration();
    
    // clear the screen
    clear();
    
    // echo input 
    echo();
    
    // draw header
    DrawHeader();
    
    // draw title
    attron(A_UNDERLINE);
    mvprintw(6, 2, "RENAME CALIBRATION");
    attroff(A_UNDERLINE);
  
    // ask new name
    mvprintw(8, 2, "Old calibration identifier                 : %s", gCalibration);
    mvprintw(9, 2, "Enter new calibration identifier           : ");
    scanw("%s", newName);

    // ask confirmation
    mvprintw(11, 6, "Renaming calibration '%s' to '%s'", gCalibration, newName);
    mvprintw(13, 6, "Are you sure to continue? (yes/no) : ");
    scanw("%s", answer);
    if (strcmp(answer, "yes")) 
    {
        mvprintw(15, 2, "Aborted.");
    }
    else
    {
        // rename calibration
        Bool_t ret = TCMySQLManager::GetManager()->ChangeCalibrationName(gCalibration, newName);

        // check return value
        if (ret)
        {
            mvprintw(15, 2, "Renamed calibration '%s' to '%s'", 
                            gCalibration, newName);
        }
        else
            mvprintw(15, 2, "There was an error during renaming the calibration '%s' to '%s'!", 
                            gCalibration, newName);
    }
    
    // user information
    PrintStatusMessage("Hit ESC or 'q' to exit");
  
    // wait for input
    for (;;)
    {
        // get key
        Int_t c = getch();
        
        // leave loop
        if (c == KEY_ESC || c == 'q') break;
    }
 
    // don't echo input 
    noecho();
    
    // go back
    CalibEditor();
}

//______________________________________________________________________________
void RunEditor()
{
    // Show the run editor.
    
    // menu configuration
    const Char_t mTitle[] = "RUN EDITOR";
    const Char_t mMsg[] = "Select a run operation";
    const Int_t mN = 8;
    const Char_t* mEntries[] = { "Browse runs",
                                 "Change path",
                                 "Change target",
                                 "Change target polarization",
                                 "Change degree of target polarization",
                                 "Change beam polarization",
                                 "Change degree of beam polarization",
                                 "Go back" };
    
    // show menu
    Int_t choice = ShowMenu(mTitle, mMsg, mN, mEntries, 0);

    // decide what do to
    switch (choice)
    {
        case 0: RunBrowser();
        case 1: ChangeRunEntry("CHANGE PATH", "path", kPATH);
        case 2: ChangeRunEntry("CHANGE TARGET", "target", kTARGET);
        case 3: ChangeRunEntry("CHANGE TARGET POLARIZATION", 
                               "target polarization", kTARGET_POL);
        case 4: ChangeRunEntry("CHANGE DEGREE OF TARGET POLARIZATION", 
                               "degree of target polarization", kTARGET_POL_DEG);
        case 5: ChangeRunEntry("CHANGE BEAM POLARIZATION", 
                               "beam polarization", kBEAM_POL);
        case 6: ChangeRunEntry("CHANGE DEGREE OF BEAM POLARIZATION", 
                               "degree of beam polarization", kBEAM_POL_DEG);
        case 7: MainMenu();
    }
    
    // go back to main menu
    MainMenu();
}

//______________________________________________________________________________
void CalibEditor()
{
    // Show the calibration editor.
    
    // menu configuration
    const Char_t mTitle[] = "CALIBRATION EDITOR";
    const Char_t mMsg[] = "Select a calibration operation";
    const Int_t mN = 4;
    const Char_t* mEntries[] = { "Browse calibration",
                                 "Rename calibration",
                                 "Change description",
                                 "Go back" };
    
    // show menu
    Int_t choice = ShowMenu(mTitle, mMsg, mN, mEntries, 0);

    // decide what do to
    switch (choice)
    {
        case 0: CalibBrowser();
        case 1: RenameCalibration();
        case 3: MainMenu();
    }
}

//______________________________________________________________________________
void CalibManagement()
{
    // Show the calibration management.
    
    // menu configuration
    const Char_t mTitle[] = "CALIBRATION MANAGEMENT";
    const Char_t mMsg[] = "Select a calibration set operation";
    const Int_t mN = 5;
    const Char_t* mEntries[] = { "Add set",
                                 "Remove set",
                                 "Split set",
                                 "Merge set",
                                 "Go back" };
    
    // show menu
    Int_t choice = ShowMenu(mTitle, mMsg, mN, mEntries, 0);

    // decide what do to
    switch (choice)
    {
        case 4: MainMenu();
    }
}

//______________________________________________________________________________
void MainMenu()
{
    // Show the main menu.
    
    // menu configuration
    const Char_t mTitle[] = "MAIN MENU";
    const Char_t mMsg[] = "Select an operation";
    const Int_t mN = 4;
    const Char_t* mEntries[] = { "Run editor",
                                 "Calibration editor",
                                 "Calibration set management",
                                 "Exit" };
    
    // show menu
    Int_t choice = ShowMenu(mTitle, mMsg, mN, mEntries, 0);

    // decide what do to
    switch (choice)
    {
        case 0: RunEditor();
        case 1: CalibEditor();
        case 2: CalibManagement();
        case 3: Finish(0);
    }
}

//______________________________________________________________________________
Int_t main(Int_t argc, Char_t* argv[])
{
    // Main method.

    // set-up signal for CTRL-C
    signal(SIGINT, Finish);

    // init ncurses
    initscr();
    
    // enable keyboard mapping (non-BSD feature) 
    keypad(stdscr, TRUE);
    
    // tell curses not to do NL->CR/NL on output 
    nonl();
    
    // take input chars one at a time, don't wait for <\\>n 
    cbreak();
    
    // don't echo input 
    noecho();
 
    // get number of rows and columns
    getmaxyx(stdscr, gNrow, gNcol);
    
    // set MySQL manager to silence mode
    TCMySQLManager::GetManager()->SetSilenceMode(kTRUE);
    
    // show main menu
    MainMenu();

    // exit program
    Finish(0);
}

