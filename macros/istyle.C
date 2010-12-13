{
  TStyle *iStyle= new TStyle("IRAKLI","approved plots style");

  // use plain black on white colors
  iStyle->SetFrameBorderMode(  0 );
  iStyle->SetFrameFillColor( 10 );
  iStyle->SetCanvasBorderMode( 0 );
  iStyle->SetPadBorderMode(    0 );

  iStyle->SetPadColor(        10 );
  iStyle->SetCanvasColor(     10 );
  iStyle->SetStatColor(       10 );
  iStyle->SetFillColor(       10 );

  // set the paper & margin sizes
  iStyle->SetPaperSize(       20, 26 );

  iStyle->SetPadTopMargin(    0.05 );
  iStyle->SetPadRightMargin(  0.05 );
  iStyle->SetPadBottomMargin( 0.16 );
  iStyle->SetPadLeftMargin(   0.12 );
  
  // use large Times-Roman fonts
  iStyle->SetTextFont(  132  );
  iStyle->SetTextSize(  0.08 );

  iStyle->SetLabelFont( 132, "x" );
  iStyle->SetLabelFont( 132, "y" );
  iStyle->SetLabelFont( 132, "z" );

  iStyle->SetLabelSize( 0.04, "x" );
  iStyle->SetTitleSize( 0.05, "x" );
  iStyle->SetLabelSize( 0.04, "y" );
  iStyle->SetTitleSize( 0.05, "y" );
  iStyle->SetLabelSize( 0.04, "z" );
  iStyle->SetTitleSize( 0.05, "z" );

  // use bold lines and markers
//  iStyle->SetMarkerStyle(20);
//  iStyle->SetLineColor(2);
//   iStyle->SetHistLineWidth(1.85);
//   iStyle->SetLineStyleString(2,"[12 12]"); // postscript dashes
  
  // get rid of X error bars and y error bar caps
//   iStyle->SetErrorX(0.001);

  // do not display any of the standard histogram decorations
  
  // put tick marks on top and RHS of plots
  iStyle->SetPadTickX(1);
  iStyle->SetPadTickY(1);

  iStyle->SetOptFit(1);
  iStyle->SetFitFormat("12.7g");

  iStyle->cd();
}
