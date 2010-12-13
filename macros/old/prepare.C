
/***************************************************
*                                                  *
*                                                  *
*                                                  *
***************************************************/

void prepare( int run  = 0 )
{
  iWriteCBconfigFile *iwc = new iWriteCBconfigFile();
  iwc->ReadCBpar( run );
  iwc->WriteCBconfig();

  return;
}

