
//
// macro to copy content of one set into another set :)
//

void copy_set( Int_t from, Int_t into )
{
  Double_t par[720];

  // read
  //
  iMySQLReader *imr=new iMySQLReader();
  imr->ReadCBpar4set( from, par );
  
  
  // write
  //
  iMySQLWriter *imw = new iMySQLWriter();
  imw->Write_CB_Ecalib_DBset( into, par );
  
  return;
}
