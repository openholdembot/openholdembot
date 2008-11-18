//
// Versus
//

BEGIN_TESTCASE
    TESTCASE_ID ("0237")
    REASONING ("Range check for symbol vs$nhands")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$nhands") >= 0) && (gws("vs$nhands") <= 2652))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$nhands")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0238")
    REASONING ("Range check for symbol vs$nhandshi")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$nhandshi") >= 0) && (gws("vs$nhandshi") <= 2652))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$nhandshi")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0239")
    REASONING ("Range check for symbol vs$nhandsti")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$nhandsti") >= 0) && (gws("vs$nhandsti") <= 2652))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$nhandsti")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0240")
    REASONING ("Range check for symbol vs$nhandslo")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$nhandslo") >= 0) && (gws("vs$nhandslo") <= 2652))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$nhandslo")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0241")
    REASONING ("Range check for symbol vs$prwin")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prwin") >= 0) && (gws("vs$prwin") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prwin")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0242")
    REASONING ("Range check for symbol vs$prtie")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prtie") >= 0) && (gws("vs$prtie") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prtie")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0243")
    REASONING ("Range check for symbol vs$prlos")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prlos") >= 0) && (gws("vs$prlos") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prlos")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0244")
    REASONING ("Range check for symbol vs$prwinhi")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prwinhi") >= 0) && (gws("vs$prwinhi") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prwinhi")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0245")
    REASONING ("Range check for symbol vs$prtiehi")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prtiehi") >= 0) && (gws("vs$prtiehi") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prtiehi")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0246")
    REASONING ("Range check for symbol vs$prloshi")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prloshi") >= 0) && (gws("vs$prloshi") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prloshi")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0247")
    REASONING ("Range check for symbol vs$prwinti")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prwinti") >= 0) && (gws("vs$prwinti") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prwinti")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0248")
    REASONING ("Range check for symbol vs$prtieti")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prtieti") >= 0) && (gws("vs$prtieti") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prtieti")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0249")
    REASONING ("Range check for symbol vs$prlosti")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prlosti") >= 0) && (gws("vs$prlosti") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prlosti")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0250")
    REASONING ("Range check for symbol vs$prwinlo")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prwinlo") >= 0) && (gws("vs$prwinlo") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prwinlo")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0251")
    REASONING ("Range check for symbol vs$prtielo")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prtielo") >= 0) && (gws("vs$prtielo") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prtielo")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0252")
    REASONING ("Range check for symbol vs$prloslo")
    PRECONDITION (true)
    POSTCONDITION ((gws("vs$prloslo") >= 0) && (gws("vs$prloslo") <= 1))
    SYMBOLS_POSSIBLY_AFFECTED ("vs$prloslo")
END_TESTCASE


//
// No change to test
//   vs$x$prwin, vs$x$prlos, vs$x$prtie,
//   where x can be an arbitrary number.
//