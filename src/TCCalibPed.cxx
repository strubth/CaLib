// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibPed                                                           //
//                                                                      //
// Base pedestal calibration module class.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibPed.h"

ClassImp(TCCalibPed)


//______________________________________________________________________________
TCCalibPed::TCCalibPed(const Char_t* name, const Char_t* title, CalibData_t data,
                       Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Constructor.
    
    // init members
    fADC = 0;
    fFileManager = 0;
    fMean = 0;
    fLine = 0;
}

//______________________________________________________________________________
TCCalibPed::~TCCalibPed()
{
    // Destructor. 
    
    if (fADC) delete [] fADC;
    if (fFileManager) delete fFileManager;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibPed::Init()
{
    // Init the module.
    
    Char_t tmp[256];

    // init members
    fADC = new Int_t[fNelem];
    for (Int_t i = 0; i < fNelem; i++) fADC[i] = 0;
    fFileManager = new TCFileManager(fSet, fData);
    fMean = 0;
    fLine = new TLine();
    
    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
  
    // read ADC numbers
    ReadADC();

    // read old parameters
    TCMySQLManager::GetManager()->ReadParameters(fSet, fData, fOldVal, fNelem);

    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;Pedestal position [Channel]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
    
    // prepare main canvas
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    
    // draw the overview histogram
    fCanvasResult->cd();
    sprintf(tmp, "%s.Histo.Overview", GetName());
    TCUtils::FormatHistogram(fOverviewHisto, tmp);
    fOverviewHisto->Draw("P");
}    

//______________________________________________________________________________
void TCCalibPed::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // load the pedestal histogram
    if (fFitHisto) delete fFitHisto;
    sprintf(tmp, "ADC%d", fADC[elem]);
    fFitHisto = fFileManager->GetHistogram(tmp);
    
    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fPed_%i", elem);
        fFitFunc = new TF1(tmp, "gaus");
        fFitFunc->SetLineColor(2);
        
        // estimate peak position
        fMean = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());
        if (fMean > 120) fMean = 100;

        // configure fitting function
        fFitFunc->SetRange(fMean - 10, fMean + 10);
        fFitFunc->SetLineColor(2);
        fFitFunc->SetParameters(1, fMean, 0.1);
        fFitHisto->Fit(fFitFunc, "RB0Q");

        // final results
        fMean = fFitFunc->GetParameter(1); 

        // draw mean indicator line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        
        // set indicator line
        fLine->SetX1(fMean);
        fLine->SetX2(fMean);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    sprintf(tmp, "%s.Histo.Fit", GetName());
    TCUtils::FormatHistogram(fFitHisto, tmp);
    fFitHisto->Draw("hist");
    
    // draw fitting function
    if (fFitFunc) fFitFunc->Draw("same");
    
    // draw indicator line
    fLine->Draw();
    
    // update canvas
    fCanvasFit->Update();
    
    // update overview
    if (elem % 20 == 0)
    {
        fCanvasResult->cd();
        fOverviewHisto->Draw("E1");
        fCanvasResult->Update();
    }   
}

//______________________________________________________________________________
void TCCalibPed::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // save pedestal position
        fNewVal[elem] = fMean;
    
        // if new value is negative take old
        if (fNewVal[elem] < 0) 
        {
            fNewVal[elem] = fOldVal[elem];
            unchanged = kTRUE;
        }

        // update overview histogram
        fOverviewHisto->SetBinContent(elem+1, fMean);
        fOverviewHisto->SetBinError(elem+1, 0.0000001);
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    printf("Element: %03d    "
           "old pedestal: %12.8f    new pedestal: %12.8f",
           elem, fOldVal[elem], fNewVal[elem]);
    if (unchanged) printf("    -> unchanged");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibPed::ReadADC()
{
    // Read the ADC number of each element from the data file registered in the
    // configuration.
    
    Char_t tmp[256];
    const Char_t* filename;

    // get file name
    sprintf(tmp, "%s.ADCList", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("ReadADC", "ADC list file was not found!");
        return;
    }
    else filename = TCReadConfig::GetReader()->GetConfig(tmp)->Data();
    
    // open the file
    std::ifstream infile;
    infile.open(filename);
        
    // check if file is open
    if (!infile.is_open())
    {
        Error("ReadADC", "Could not open ADC list file '%s'", filename);
    }
    else
    {
        Info("ReadADC", "Reading ADC list from file '%s'", filename);
        
        // read the file
        while (infile.good())
        {
            TString line;
            Int_t n, adc;
            line.ReadLine(infile);
                
            // trim line
            line.Remove(TString::kBoth, ' ');
            
            // skip comments
            if (line.BeginsWith("#")) continue;
            else
            { 
                // get element and ADC
                sscanf(line.Data(), "%d%d", &n, &adc);
                
                // save ADC
                if (n >= 0 && n < fNelem) fADC[n] = adc;
            }
        }
    }
    
    // close the file
    infile.close();
} 

