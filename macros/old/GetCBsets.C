/*****************************************************
 *                                                   *
 *                                                   *
 *                                                   *
 *****************************************************/

#include <iostream>
#include <fstream>

#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TLine.h>

// 
int sets;
int set[100];
int set_start[100];
int set_stopp[100];

TLine* lset_start[100];
TLine* lset_stopp[100];
TLatex* tset[100];

int GetSets() { return sets; };
int GetStart(int i) { return set_start[i]; };
int GetStop(int i)  { return set_stopp[i]; };

TLine* GetStartLine(int i) { return lset_start[i]; };
TLine* GetStopLine(int i)  { return lset_stopp[i]; };

void DrawSets( int bot, int upp )
{
  TSQLServer *dbh;
  TSQLResult *res;
  TSQLRow    *row;
  Char_t     szQuery[256];
  
  // DataBase
  dbh = TSQLServer::Connect("mysql://phys-bonanza/acqu", "read", "readonly");
  
  sprintf(szQuery, 
	  "SELECT runset, start_run, stop_run FROM C_sets_main;");
  
  res = dbh->Query(szQuery);
  dbh->Close();
  
  sets = res->GetRowCount();

  char szText[32];  
  //
  for( int i=0; i<sets; i++)
    {
      row = res->Next();
 
      set[i]   = atol( row->GetField(0) );
      set_start[i] = atol( row->GetField(1) );
      set_stopp[i]  = atol( row->GetField(2) );

      lset_start[i] = new TLine( set_start[i]-0.5, 0, set_start[i]-0.5, 1 );
      lset_stopp[i]  = new TLine(  set_stopp[i]+0.5, 0,  set_stopp[i]+0.5, 1 );

      lset_start[i]->SetLineWidth(3);
      lset_start[i]->SetLineColor(2);
      lset_start[i]->SetY1( bot );
      lset_start[i]->SetY2( upp );
      lset_start[i]->Draw();
      

      lset_stopp[i]->SetLineWidth(5);
      lset_stopp[i]->SetLineColor(4);
      lset_stopp[i]->SetY1( bot );
      lset_stopp[i]->SetY2( upp );
      lset_stopp[i]->Draw();

      sprintf( szText, "Day-%02i", set[i]);
      tset[i] = new TLatex( set_start[i]+5, (upp-0.2*(upp-bot)), szText);
      tset[i]->SetTextColor(13);
      tset[i]->SetTextAngle(45);
      tset[i]->SetTextSize(0.05);

      //     printf(" %i   %i   %i \n", set[i], set_start[i], set_stopp[i]);
      
      tset[i]->Draw();
    }
  return;
}

