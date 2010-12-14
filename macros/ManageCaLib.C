// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, 2010
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ManageCaLib.C                                                        //
//                                                                      //
// Manage the CaLib database.                                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void ClearScreen()
{
    // Clear the screen.
    
    printf("\033[2J");
 
}

//______________________________________________________________________________
void RunMenu()
{
    // Show the run menu.
}

//______________________________________________________________________________
void SetMenu()
{
    // Show the set menu.
}

//______________________________________________________________________________
void MainMenu()
{
    // Show the main menu.

    // show the menu
    ClearScreen();
    printf("*********************************************************\n");
    printf(" ManageCaLib                                            *\n");
    printf("*********************************************************\n");
    printf("\n");
    printf("Operations:\n");
    printf("1) Run operations\n");
    printf("2) Set operations\n");
    printf("3) Exit\n");
    printf("\n");
    printf("Your choice : ");
    
    // get user answer
    Int_t op;
    scanf("%d", &op);
    switch (op)
    {
        case 1:
            RunMenu();
            break;
        case 2:
            SetMenu();
            break;
        case 3:
            gSystem->Exit(0);
        default:
            MainMenu();
    }
}

//______________________________________________________________________________
void ManageCaLib()
{
    // load CaLib
    gSystem->Load("libCaLib.so");

    // create the MySQLManager
    iMySQLManager m;

    // show the main menu
    MainMenu();

}

