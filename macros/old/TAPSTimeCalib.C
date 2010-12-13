// SVN Info: $Id: TAPSTimeCalib.C 201 2008-04-25 12:36:03Z werthm $

/*************************************************************************
 * Author: Dominik Werthmueller / Igal Jaegle, 2008
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAPSTimeCalib                                                        //
//                                                                      //
// This macro is used for the calibration of the TAPS vs. TAPS timing.  //
// (Compilation required)                                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TTimeStamp.h"
#include "TLine.h"
#include "TText.h"
#include "TMath.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TSystem.h"
#include "TPad.h"

#include <iostream>
#include <fstream>


using namespace std;


Double_t peakLimitLow;
Double_t peakLimitHigh;
TText* aText;
TLine* aLine;
TF1* func;


//______________________________________________________________________________
Double_t TimeFunc(Double_t* x, Double_t* par)
{
    // Fit function for the coincidence peak in the time spectrum.
    
    if (x[0] > par[1]-3 && x[0] < par[1]+3) 
        return par[0]*TMath::Gaus(x[0], par[1], par[2]) + par[3] + par[4]*x[0] +  par[5]*x[0]*x[0];
    else return par[3] + par[4]*x[0] +  par[5]*x[0]*x[0];
}


//______________________________________________________________________________
Double_t FitElement(TH2F* inH2, UInt_t id)
{
    // Fit the time coincidence peak of an element in the corresponding histogram
    // projection.
 
    Char_t name[256];

    // get projection
    sprintf(name, "Element_%d", id);
    TH1D* hProj = (TH1D*) inH2->ProjectionY(name, id+1, id+1);
    
    // check if histogram was filled
    if (hProj->GetEntries() == 0)
    {
        cout << "No histogram found at index " << id << endl;
        return -999;
    }
    
    cout << "Fitting histogram at index " << id << endl;

    hProj->Draw();
    hProj->Rebin(4);

    Double_t maxPos = hProj->GetYaxis()->GetBinCenter(hProj->GetMaximumBin());
    
    func->SetParameters(500, maxPos, 1, 1, 0, -0.5);

    hProj->Fit("TimeFunc", "RBQ");

    // Get coincidence position
    Double_t posOut = func->GetParameter(1);
    
    hProj->GetXaxis()->SetRangeUser(-5, 5);

    // correct bad fits
    //if (id ==   ) posOut = ;
    
    aText->SetTextSize(0.1);
    aText->SetTextAlign(11);
    sprintf(name, "ID: %d", id);
    aText->DrawText(0.6, 0.8, name);
    sprintf(name, "Pos: %6.4f", posOut);
    aText->DrawText(0.6, 0.7, name);

    aLine->DrawLine(posOut, 0, posOut, 1e9); 

    return posOut;
}

//______________________________________________________________________________
void TAPSTimeCalib()
{    
    // Work method.
    
    // Ajust Style
    gROOT->SetStyle("Plain");
    gStyle->SetFuncColor(kRed);
    gStyle->SetFuncWidth(0.5);
    gStyle->SetHistLineWidth(0.5);
    gStyle->SetPadBorderMode(0);
    gStyle->SetFrameBorderMode(0);
    gStyle->SetNdivisions(505);
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat("");
    
    // Init variables
    TCanvas* c1 = 0;
    TVirtualPad* aPad;
    UInt_t currElement = 0;
    UInt_t goodFits = 0;
    Double_t oldoffset;
    Double_t newoffset;
    Double_t tdcgain;
    Char_t line[256];
    Char_t name[256];
    Char_t form[256];
    Char_t desc[32]; 
    Char_t* p[13];
    for (Int_t i = 0; i < 13; i++) p[i] = new Char_t[16];
    
    aText = new TText();
    aText->SetTextFont(42);
    aText->SetNDC(kTRUE);
    
    aLine = new TLine();
    aLine->SetLineColor(kBlue);
    aLine->SetLineWidth(0.5);
    
    func = new TF1("TimeFunc", TimeFunc, -10, 10, 6);
 
    // file names
    const Char_t* cData = "/data/rundata/2008_11_h3/AR/igal/ARHistograms_CB_iter6.root";
    const Char_t* cIn   = "/home/basel/AcquRoot/acqu/acqu/data/Nov_08/TAPS/BaF2.dat";
    //const Char_t* h2Name = "MyTimeTAPSVSTAPS2D";
    const Char_t* h2Name = "MyTimeTAPSVSTagger2D";
    
    // open the data file
    TFile* fData = new TFile(cData);
    if (fData->IsZombie())
    {
        cout << "ERROR: Could not open ROOT file " << cData << endl;
        return;
    }

    // load the histogram
    TH2F* h2 = (TH2F*) fData->Get(h2Name);

    // DEBUG SINGLE CHANNEL
    //FitElement(h2, 119);
    //return;


    // open the TAPS calibration files
    FILE* fIn;
    if((fIn = fopen(cIn, "r")) == NULL)
    {
        cout << "ERROR: Could not open file " << cIn << endl;
        return;
    }
    
    ofstream fout("TAPSTimeCalib.dat");
    fout << "TAPS TIME CALIBRATION" << endl << endl;
    fout << "ROOT file           : " << cData << endl;
    fout << "Old calibration file: " << cIn << endl;
    fout << endl;

    TTimeStamp tStamp;
  
    fout << "  ID  ADC     Coinc_Pos   old_offset   new_offset   Change" << endl;
    fout << "----------------------------------------------------------" << endl;
    
    // Read lines from original until EOF
    while (fgets(line, 256, fIn))
    {
        sscanf(line, "%s", desc);   

        // Check if an element description line is found
        if(!strcmp(desc, "Element:"))
        {
	  // read parameters
	  Int_t n = sscanf(line, "%*s%s%s%s%s%s%s%s%s", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
	  n += sscanf(line, "%*s%*s%*s%*s%*s%*s%*s%*s%*s%lf%lf%s%s%s", &oldoffset, &tdcgain, p[10], p[11], p[12]);
   
            // create new canvas for 64 elements (1 block) if 64 elements were processed
            if (currElement % 64 == 0)
            {
                if (c1) 
		  {   
                    sprintf(name, "TAPSTime_%d_to_%d.eps", currElement - 64, currElement-1);
                    c1->Print(name);
                    delete c1;
		  }
                sprintf(name, "64 Elements starting from %d", currElement);
                c1 = new TCanvas(name, name, 1400, 1000);
                c1->Divide(8, 8, 0.0001, 0.001);
                aPad = c1->cd(1);
            }
            else
	      {
		aPad = c1->cd((currElement % 64) + 1);
	      }
            aPad->SetLeftMargin(0.0000001);
            aPad->SetRightMargin(0.0000001);
            aPad->SetTopMargin(0.00000001);
            aPad->SetBottomMargin(0.0000001);

            // copy offset
            newoffset = oldoffset;

            // output id and ADC
            sprintf(form, " %3d  %s", currElement, p[0]);
            fout << form;
                 
            // Try to fit the element
            Double_t coincPos = FitElement(h2, currElement);
	    
            // write dash to log file if histogram was empty
            if (coincPos == -999) 
	      {
                fout << "      -  ";
	      }
            // write coincidence peak position to log file 
            else 
            {
	      sprintf(form, "   %9.6f", coincPos);
	      fout << form;
	      goodFits++;
	      
            }
            
            // calculate new offset
            newoffset = oldoffset + coincPos / tdcgain;
	    
            // write old and new offset to log file
            Double_t change = TMath::Abs(newoffset - oldoffset) / oldoffset * 100;
            sprintf(form, "    %9.4f    %9.4f  %5.2f%%\n", oldoffset, newoffset, change);
            fout << form;
            
            // increment element line counter
            currElement++;
        }
    }
    
    // save last canvas
    if (c1) 
    {   
      c1->Print("TAPSTime_320_to_383.eps");
        delete c1;
    }

    fout << "-------------" << endl;
    fout << endl;
    fout << "God fits in " << goodFits << " histograms." << endl;
  

    // close files
    fData->Close();
    fout.close();
    fclose(fIn);
 
    gSystem->Exit(0);
}

