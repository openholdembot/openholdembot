//
// Number of Bets (73..77)
//

BEGIN_TESTCASE
    TESTCASE_ID ("0073")
    REASONING ("Range check for symbol nbetstocall")
    PRECONDITION (true)
    POSTCONDITION (gws("nbetstocall") >= 0)
    SYMBOLS_POSSIBLY_AFFECTED ("nbetstocall")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0074")
    REASONING ("Range check for symbol nbetstorais")
    PRECONDITION (gws("nplayersdealt") > 1)
    POSTCONDITION (gws("nbetstorais") > 0)
    SYMBOLS_POSSIBLY_AFFECTED ("nbetstorais, nplayersdealt")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0075")
    REASONING ("Range check for symbol ncurrentbets")
    PRECONDITION (true)
    POSTCONDITION (gws("ncurrentbets") >= 0)
    SYMBOLS_POSSIBLY_AFFECTED ("ncurrentbets")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0076")
    REASONING ("Range check for symbol ncallbets")
    PRECONDITION (true)
    POSTCONDITION (gws("ncallbets") >= 0)
    SYMBOLS_POSSIBLY_AFFECTED ("ncallbets")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0077")
    REASONING ("Range check for symbol nraisbets")
    PRECONDITION (true)
    POSTCONDITION (gws("nraisbets") >= 1)
    SYMBOLS_POSSIBLY_AFFECTED ("nraisbets")
END_TESTCASE