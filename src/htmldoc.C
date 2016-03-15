/*************************************************************************
 * Author: Dominik Werthmueller, 2016
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// htmldoc.C                                                            //
//                                                                      //
// ROOT macro for the creation of the HTML documentation.               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


//______________________________________________________________________________
void htmldoc(const Char_t* parent)
{
    gROOT->Reset();
    gSystem->Load("libCaLib.so");
    gEnv->SetValue("Unix.*.Root.Html.SourceDir", TString::Format("%s/include:%s/src", parent, parent));
    THtml h;
    h.SetOutputDir("htmldoc");
    h.SetAuthorTag(" * Author:");
    h.SetProductName("CaLib");
    h.MakeAll();
    gSystem->Exit(0);
}

