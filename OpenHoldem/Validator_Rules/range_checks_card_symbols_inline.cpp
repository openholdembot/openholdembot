//
// Card symbols - part 1
//

BEGIN_TESTCASE
    TESTCASE_ID ("0299")
    REASONING ("Range check for symbol $$ps0")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$ps0") >= 1) && (gws("$$ps0") <= 4) || (gws("$$ps0") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$ps0")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0300")
    REASONING ("Range check for symbol $$ps1")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$ps1") >= 1) && (gws("$$ps1") <= 4) || (gws("$$ps1") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$ps1")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0301")
    REASONING ("Range check for symbol $$cs0")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$cs0") >= 1) && (gws("$$cs0") <= 4) || (gws("$$cs0") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$cs0")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0302")
    REASONING ("Range check for symbol $$cs1")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$cs1") >= 1) && (gws("$$cs1") <= 4) || (gws("$$cs1") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$cs1")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0303")
    REASONING ("Range check for symbol $$cs2")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$cs2") >= 1) && (gws("$$cs2") <= 4) || (gws("$$cs2") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$cs2")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0304")
    REASONING ("Range check for symbol $$cs3")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$cs3") >= 1) && (gws("$$cs3") <= 4) || (gws("$$cs3") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$cs3")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0305")
    REASONING ("Range check for symbol $$cs4")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$cs4") >= 1) && (gws("$$cs4") <= 4) || (gws("$$cs4") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$cs4")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0306")
    REASONING ("Range check for symbol $$pr0")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$pr0") >= 1) && (gws("$$pr0") <= 14) || (gws("$$pr0") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$pr0")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0307")
    REASONING ("Range check for symbol $$pr1")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$pr1") >= 1) && (gws("$$pr1") <= 14) || (gws("$$pr1") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$pr1")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0308")
    REASONING ("Range check for symbol $$cr0")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$cr0") >= 1) && (gws("$$cr0") <= 14) || (gws("$$cr0") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$cr0")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0309")
    REASONING ("Range check for symbol $$cr1")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$cr1") >= 1) && (gws("$$cr1") <= 14) || (gws("$$cr1") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$cr1")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0310")
    REASONING ("Range check for symbol $$cr2")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$cr2") >= 1) && (gws("$$cr2") <= 14) || (gws("$$cr2") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$cr2")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0311")
    REASONING ("Range check for symbol $$cs3")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$cr3") >= 1) && (gws("$$cr3") <= 14) || (gws("$$cr3") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$cr3")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0312")
    REASONING ("Range check for symbol $$cr4")
    PRECONDITION (true)
    POSTCONDITION ((gws("$$cr4") >= 1) && (gws("$$cr4") <= 14) || (gws("$$cr4") == UNDEFINED_ZERO))
    SYMBOLS_POSSIBLY_AFFECTED ("$$cr4")
END_TESTCASE