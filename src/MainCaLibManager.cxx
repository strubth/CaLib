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


// global variables
Int_t gNrow;
Int_t gNcol;


// function prototypes
void MainMenu();
void RunEditor();


//______________________________________________________________________________
void Finish(Int_t sig)
{   
    // Exit the program.

    endwin();
    exit(0);
}

//______________________________________________________________________________
void DrawHeader(const Char_t* title)
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
Int_t ShowMenu(const Char_t* title, Int_t nEntries, const Char_t* entries[], Int_t active)
{
    // Show the main menu.

    // clear the screen
    clear();

    // draw header
    DrawHeader("CaLib Manager");
    
    // draw title
    attron(A_UNDERLINE);
    mvprintw(6, 2, "%s", title);
    attroff(A_UNDERLINE);
    
    // draw entries
    for (Int_t i = 0; i < nEntries; i++)
    {
        if (i == active) attron(A_REVERSE);
        mvprintw(8+i, 2, "%s", entries[i]);
        if (i == active) attroff(A_REVERSE);
    }

    // user information
    mvprintw(gNrow-1, 0, "Use UP and DOWN keys to select - hit ENTER or RIGHT key to confirm");
    
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
                return ShowMenu(title, nEntries, entries, active-1);
            }
        }
        // go to next entry
        else if (c == KEY_DOWN)
        {
            if (active < nEntries-1) 
            {
                return ShowMenu(title, nEntries, entries, active+1);
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
    
    // calculate the maximum col length (12*2 spaces)
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
    DrawHeader("CaLib Manager");
    
    // draw title
    attron(A_UNDERLINE);
    mvprintw(6, 2, "RUN BROWSER");
    attroff(A_UNDERLINE);
    
    // build the windows
    Int_t colLengthTot;
    WINDOW* header = 0;
    WINDOW* table = FormatRunTable(c, &colLengthTot, &header);

    // user information
    mvprintw(gNrow-1, 0, "%d runs found. Use UP/DOWN keys to scroll "
             "(PAGE-UP or 'p' / PAGE-DOWN or 'n' for fast mode) - hit ESC or 'q' to exit", nRuns);
 
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
        else if (c == KEY_ESC || c == 'q')
        {
            delwin(table);
            RunEditor();
        }

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
void RunEditor()
{
    // Show the run editor.
    
    // menu configuration
    const Char_t mTitle[] = "RUN EDITOR";
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
    Int_t choice = ShowMenu(mTitle, mN, mEntries, 0);

    // decide what do to
    switch (choice)
    {
        case 0: RunBrowser();
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
    const Int_t mN = 4;
    const Char_t* mEntries[] = { "Change name",
                                 "Change description",
                                 "Change parameters",
                                 "Go back" };
    
    // show menu
    Int_t choice = ShowMenu(mTitle, mN, mEntries, 0);

    // decide what do to
    switch (choice)
    {
        case 3: MainMenu();
    }
}

//______________________________________________________________________________
void CalibManagement()
{
    // Show the calibration management.
    
    // menu configuration
    const Char_t mTitle[] = "CALIBRATION MANAGEMENT";
    const Int_t mN = 5;
    const Char_t* mEntries[] = { "Add set",
                                 "Remove set",
                                 "Split set",
                                 "Merge set",
                                 "Go back" };
    
    // show menu
    Int_t choice = ShowMenu(mTitle, mN, mEntries, 0);

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
    const Int_t mN = 4;
    const Char_t* mEntries[] = { "Run editor",
                                 "Calibration editor",
                                 "Calibration set management",
                                 "Exit" };
    
    // show menu
    Int_t choice = ShowMenu(mTitle, mN, mEntries, 0);

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

