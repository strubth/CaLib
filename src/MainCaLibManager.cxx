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


// global variables
Int_t gNrow;
Int_t gNcol;


// function prototypes
void MainMenu();


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
void MarkMenuEntry(Int_t oldEntry, Int_t newEntry)
{
    // Unmark the menu entry 'oldEntry' and mark 'newEntry' instead.

    mvprintw(8 + oldEntry, 2, "  ");
    mvprintw(8 + newEntry, 2, "->");
    mvprintw(gNrow-1, 0, "Use UP and DOWN keys to select - hit ENTER or RIGHT key to confirm");
}

//______________________________________________________________________________
Int_t ShowMenu(const Char_t* title, Int_t nEntries, const Char_t* entries[])
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
        mvprintw(8+i, 5, "%s", entries[i]);

    // ask user input
    mvprintw(gNrow-1, 0, "Your choice: ");
    
    Int_t active = 0;
    MarkMenuEntry(active, active);

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
                MarkMenuEntry(active, active-1);
                active--;
            }
        }
        // go to next entry
        else if (c == KEY_DOWN)
        {
            if (active < nEntries-1) 
            {
                MarkMenuEntry(active, active+1);
                active++;
            }
        }
        // choose entry
        else if (c == 13 || c == KEY_ENTER || c == KEY_RIGHT)
        {
            return active;
        }
    }
}

//______________________________________________________________________________
void RunEditor()
{
    // Show the run editor.
    
    // menu configuration
    const Char_t mTitle[] = "RUN EDITOR";
    const Int_t mN = 7;
    const Char_t* mEntries[] = { "Change path",
                                 "Change target",
                                 "Change target polarization",
                                 "Change degree of target polarization",
                                 "Change beam polarization",
                                 "Change degree of beam polarization",
                                 "Go back" };
    
    // show menu
    Int_t choice = ShowMenu(mTitle, mN, mEntries);

    // decide what do to
    switch (choice)
    {
        case 6: MainMenu();
    }
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
    Int_t choice = ShowMenu(mTitle, mN, mEntries);

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
    Int_t choice = ShowMenu(mTitle, mN, mEntries);

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
    Int_t choice = ShowMenu(mTitle, mN, mEntries);

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
    
    // show main menu
    MainMenu();

    // exit program
    Finish(0);
}

