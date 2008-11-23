//
// Hand reset
//

BEGIN_TESTCASE
    TESTCASE_ID ("0600")
    REASONING ("Did the hand last 'long enough' to reach the current betting round, or was there an incorrect hand-reset in the meantime? We assume, the game has to last at least 1 second per (finished) betting round and per player still in the hand.")
    PRECONDITION (handnumber > 1)
    POSTCONDITION (gws("elapsedhand") >= ((gws("betround")-1) * gws("nplayersplaying")))
    SYMBOLS_POSSIBLY_AFFECTED ("elapsedhand, betround, nplayersplaying, handnumber")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0601")
    REASONING ("If it is preflop and I did not yet act and I am not blind and it is not the first hand (to avoid problems, if I did post the first blind in some other position), then I can't have a bet in front of me (or something is going wrong, e.g. a false handreset, resetting the action symbols).")
    PRECONDITION ((gws("betround") == 1) && !gws("didcheck") && !gws("didcall") && !gws("didrais") && ! gws("didswag") && (gws("dealposition") > 2) && (gws("handnumber") > 1))
    POSTCONDITION (gws("currentbet") == 0)
    SYMBOLS_POSSIBLY_AFFECTED ("betround, didcheck, didcall, didrais, didswag, dealposition, handnumber, currentbet")
END_TESTCASE




