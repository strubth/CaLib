
//
void htmldoc()
{
    gROOT->Reset();
    if (!gROOT->GetClass("iFileManager")) gSystem->Load("libCaLib.so");

    THtml h;
    h.SetInputDir(".");
    h.SetOutputDir("htmldoc");
    h.SetAuthorTag("* Author: Irakli Keshelashvili");
    h.SetProductName("CaLib");

    h.MakeAll();
}

