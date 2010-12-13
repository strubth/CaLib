
/*************************************************
 *                                               *
 *                                               *
 *                                               *
 ************************************************/

void rootlogon()
{
  //
  //
  gROOT->ProcessLine (".x ./macros/istyle.C");
  //  gROOT->ProcessLine (".x /usr/users/irakli/rootlogon.C");

  //
  gSystem->SetIncludePath ("-I./include");
  
  gSystem->Load("./lib/libCaLib.so");

  //
  //  if( !gROOT->IsBatch() )
  //    gROOT->ProcessLine(".x macros/gui_helper.C"); // I change the path

  cout << endl;
  return;
}
