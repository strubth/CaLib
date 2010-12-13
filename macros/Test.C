void Test()
{
    // load CaLib   
    gSystem->Load("libCaLib.so");

    // connect to database
    iMySQLManager m;
    
    Int_t nSets = m.GetNsets("cb_t0");
    printf("Number of sets in cb_t0: %d\n", nSets);

    for (Int_t i = 0; i < nSets; i++)
    {
        printf("First run of set %d in cb_t0: %d\n", i, m.GetFirstRunOfSet("cb_t0", i));

        //Double_t par[24];
        //m.ReadParameters(i, "pid_e1", par, 24);

        //printf("Parameters: ");
        //for (Int_t j = 0; j < 24; j++) printf("%f, ", par[j]);
        //printf("\n\n");
    }
}

