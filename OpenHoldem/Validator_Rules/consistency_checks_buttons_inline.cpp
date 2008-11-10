#define BUTTON_FOLD  0x01
#define BUTTON_CALL  0x02
#define BUTTON_RAISE 0x04
#define BUTTON_ALLIN 0x08
#define BUTTON_CHECK 0x10



BEGIN_TESTCASE
    TESTCASE_ID ("0500")
    REASONING ("If it's my turn, then there has to be always a fold button.")
    PRECONDITION (gws("ismyturn"))
    POSTCONDITION (((int)gws("myturnbits") & BUTTON_FOLD) != 0)
    SYMBOLS_POSSIBLY_AFFECTED ("ismyturn, myturnbits")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0501")
    REASONING ("If it's my turn, then there has to be always a check or a call button.")
    PRECONDITION (gws("ismyturn"))
    POSTCONDITION ((((int)gws("myturnbits") & BUTTON_CHECK) != 0) || (((int)gws("myturnbits") & BUTTON_CALL) != 0))
    SYMBOLS_POSSIBLY_AFFECTED ("ismyturn, myturnbits")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0502")
    REASONING ("If it's my turn and there are no bet's to call, then there has to be a check button.")
    PRECONDITION (gws("ismyturn") && (gws("nbetstocall") == 0))
    POSTCONDITION (((int)gws("myturnbits") & BUTTON_CHECK) != 0)
    SYMBOLS_POSSIBLY_AFFECTED ("ismyturn, myturnbits, nbetstocall")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0503")
    REASONING ("If it's my turn and there's a bet to call, then there has to be a call button.")
    PRECONDITION (gws("ismyturn") && (gws("nbetstocall") >= 0))
    POSTCONDITION (((int)gws("myturnbits") & BUTTON_CHECK) != 0)
    SYMBOLS_POSSIBLY_AFFECTED ("ismyturn, myturnbits, nbetstocall")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0504")
    REASONING ("If there's a check button, then there can't be a call button.")
    PRECONDITION (((int)gws("myturnbits") & BUTTON_CHECK) != 0)
    POSTCONDITION (((int)gws("myturnbits") & BUTTON_CALL) == 0)
    SYMBOLS_POSSIBLY_AFFECTED ("myturnbits")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0505")
    REASONING ("If there's a call button, then there can't be a check button.")
    PRECONDITION (((int)gws("myturnbits") & BUTTON_CALL) == 0)
    POSTCONDITION (((int)gws("myturnbits") & BUTTON_CHECK) != 0)
    SYMBOLS_POSSIBLY_AFFECTED ("myturnbits")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0506")
    REASONING ("If my balance is 0, then there can't be any button, as I'm already allin or not involved in the game at all.")
    PRECONDITION (gws("balance") == 0)
    POSTCONDITION (gws("myturnbits") == 0)
    SYMBOLS_POSSIBLY_AFFECTED ("myturnbits, balance")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0507")
    REASONING ("If it's a NL- or a PL-game and it's my turn and my balance is greater than the bets to call, then there has to be always a raise button.")
    PRECONDITION (gws("ismyturn") && (gws("isnl") || gws("ispl")) && ((gws("balance")/gws("bets")) > gws("nbetstocall")))
    POSTCONDITION (((int)gws("myturnbits") & BUTTON_RAISE) != 0)
    SYMBOLS_POSSIBLY_AFFECTED ("ismyturn, isnl, ispl, balance, bets, nbetstocall, myturnbits")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0508")
    REASONING ("If it's a FL game and it's my turn and there are not more than 3 bets, then there has to be a raise button.")
    PRECONDITION (gws("isfl") && gws("ismyturn") && (gws("ncallbets") <= 3))
    POSTCONDITION (((int)gws("myturnbits") & BUTTON_RAISE) != 0)
    SYMBOLS_POSSIBLY_AFFECTED ("isfl, ismyturn, ncallbets, myturnbits")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0509")
    REASONING ("If it's a NL game and it's my turn, then there has to be always an allin button.")
    PRECONDITION (gws("isnl") && gws("ismyturn"))
    POSTCONDITION (((int)gws("myturnbits") & BUTTON_ALLIN) != 0)
    SYMBOLS_POSSIBLY_AFFECTED ("isnl, ismyturn, myturnbits")
END_TESTCASE

