
/*************************************************
 *                                               *
 *                                               *
 *                                               *
 ************************************************/

Int_t nLine;
iChangeARconfig ic;

//
void auto_prepare_qsub( int run )
{
	ic.CopyARconfig( run, "~/job/temp.sh", "myjob.sh" );

	gROOT->ProcessLine(".! qsub -q server.q myjob.sh");

	return;
}
