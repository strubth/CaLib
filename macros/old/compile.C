/*******************************************************************
 *                                                                 *
 * Date: 22.04.2009     Author: Irakli                             *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 ******************************************************************/

//
ifstream infile;
TString  strLine;

void compile()
{
  TString szFileName = "module_list";
  infile.open(szFileName.Data());
  
  if ( !infile.is_open() )
    {
      printf("\n Error opening : \"%s\" file ! ! !\n\n", szFileName.Data());
      gSystem->Exit(0);
    }
  
  printf("\n ---------------------------------------- \n");
  printf(" Read File : \"%s\"\n", szFileName.Data());

  while ( infile.good() )
    {
      strLine="";
      strLine.ReadLine(infile);
      
      if( strLine.BeginsWith("#") )
	{
	  // 	      printf("Comment : %s \n", strLine.Data());
	}
      elise 
	{
	  strLine.ReplaceAll(" ","");
	  printf("module : %s \n", strLine.Data());

	  if( gSystem->CompileMacro( strLine.Data(), "k" ) )
	    printf( "Compiled %s library !\n", strLine.Data() );
	  else
	    printf( "Unable to compile %s library!\n", strLine.Data() );
	}
    }

  infile.close();
}
