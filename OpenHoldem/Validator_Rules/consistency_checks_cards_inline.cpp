//
// Cards
//

#define CARD_UNKNOWN 0x00
#define CARDBACK     0xff


BEGIN_TESTCASE
    TESTCASE_ID ("0400")
    HEURISTIC_RULE (false)
    REASONING ("All cards have to be unique. Partial rule for $pc0")
    PRECONDITION ((gws("$pc0") != CARD_UNKNOWN) && (gws("$pc0") != CARDBACK))
    POSTCONDITION ((gws("$pc0") != gws("$pc1")) && (gws("$pc0") != gws("$cc0")) && (gws("$pc0") != gws("$cc1")) && (gws("$pc0") != gws("$cc2")) && (gws("$pc0") != gws("$cc3")) && (gws("$pc0") != gws("$cc4")))
    SYMBOLS_POSSIBLY_AFFECTED ("$pc0. $pc1, $cc0, $cc1, $cc2, $cc3, $cc4")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0401")
    HEURISTIC_RULE (false)
    REASONING ("All cards have to be unique. Partial rule for $pc1")
    PRECONDITION ((gws("$pc1") != CARD_UNKNOWN) && (gws("$pc1") != CARDBACK))
    POSTCONDITION ((gws("$pc0") != gws("$pc1")) && (gws("$pc1") != gws("$cc0")) && (gws("$pc1") != gws("$cc1")) && (gws("$pc1") != gws("$cc2")) && (gws("$pc1") != gws("$cc3")) && (gws("$pc1") != gws("$cc4")))
    SYMBOLS_POSSIBLY_AFFECTED ("$pc0. $pc1, $cc0, $cc1, $cc2, $cc3, $cc4")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0402")
    HEURISTIC_RULE (false)
    REASONING ("All cards have to be unique. Partial rule for $cc0")
    PRECONDITION ((gws("$cc0") != CARD_UNKNOWN) && (gws("$cc0") != CARDBACK))
    POSTCONDITION ((gws("$cc0") != gws("$pc0")) && (gws("$cc0") != gws("$pc1")) && (gws("$cc0") != gws("$cc1")) && (gws("$cc0") != gws("$cc2")) && (gws("$cc0") != gws("$cc3")) && (gws("$cc0") != gws("$cc4")))
    SYMBOLS_POSSIBLY_AFFECTED ("$pc0. $pc1, $cc0, $cc1, $cc2, $cc3, $cc4")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0403")
    HEURISTIC_RULE (false)
    REASONING ("All cards have to be unique. Partial rule for $cc1")
    PRECONDITION ((gws("$cc1") != CARD_UNKNOWN) && (gws("$cc1") != CARDBACK))
    POSTCONDITION ((gws("$cc1") != gws("$pc0")) && (gws("$cc1") != gws("$pc1")) && (gws("$cc1") != gws("$cc0")) && (gws("$cc1") != gws("$cc2")) && (gws("$cc1") != gws("$cc3")) && (gws("$cc1") != gws("$cc4")))
    SYMBOLS_POSSIBLY_AFFECTED ("$pc0. $pc1, $cc0, $cc1, $cc2, $cc3, $cc4")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0404")
    HEURISTIC_RULE (false)
    REASONING ("All cards have to be unique. Partial rule for $cc2")
    PRECONDITION ((gws("$cc2") != CARD_UNKNOWN) && (gws("$cc2") != CARDBACK))
    POSTCONDITION ((gws("$cc2") != gws("$pc0")) && (gws("$cc2") != gws("$pc1")) && (gws("$cc2") != gws("$cc0")) && (gws("$cc2") != gws("$cc1")) && (gws("$cc2") != gws("$cc3")) && (gws("$cc2") != gws("$cc4")))
    SYMBOLS_POSSIBLY_AFFECTED ("$pc0. $pc1, $cc0, $cc1, $cc2, $cc3, $cc4")
END_TESTCASE



BEGIN_TESTCASE
    TESTCASE_ID ("0405")
    HEURISTIC_RULE (false)
    REASONING ("All cards have to be unique. Partial rule for $cc3")
    PRECONDITION ((gws("$cc3") != CARD_UNKNOWN) && (gws("$cc3") != CARDBACK))
    POSTCONDITION ((gws("$cc3") != gws("$pc0")) && (gws("$cc3") != gws("$pc1")) && (gws("$cc3") != gws("$cc0")) && (gws("$cc3") != gws("$cc1")) && (gws("$cc3") != gws("$cc2")) && (gws("$cc3") != gws("$cc4")))
    SYMBOLS_POSSIBLY_AFFECTED ("$pc0. $pc1, $cc0, $cc1, $cc2, $cc3, $cc4")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0406")
    HEURISTIC_RULE (false)
    REASONING ("All cards have to be unique. Partial rule for $cc4")
    PRECONDITION ((gws("$cc4") != CARD_UNKNOWN) && (gws("$cc4") != CARDBACK))
    POSTCONDITION ((gws("$cc4") != gws("$pc0")) && (gws("$cc4") != gws("$pc4")) && (gws("$cc4") != gws("$cc0")) && (gws("$cc4") != gws("$cc1")) && (gws("$cc4") != gws("$cc2")) && (gws("$cc4") != gws("$cc3")))
    SYMBOLS_POSSIBLY_AFFECTED ("$pc0. $pc1, $cc0, $cc1, $cc2, $cc3, $cc4")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0407")
    HEURISTIC_RULE (false)
    REASONING ("If there's a turn card, there also have to be all 3 flop cards.")
    PRECONDITION ((gws("$cc3") != CARD_UNKNOWN) && (gws("$cc3") != CARDBACK))
    POSTCONDITION ((gws("$cc0") != CARD_UNKNOWN) && (gws("$cc0") != CARDBACK) && (gws("$cc1") != CARD_UNKNOWN) && (gws("$cc1") != CARDBACK) && (gws("$cc2") != CARD_UNKNOWN) && (gws("$cc2") != CARDBACK))
    SYMBOLS_POSSIBLY_AFFECTED ("$cc0, $cc1, $cc2, $cc3")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0408")
    HEURISTIC_RULE (false)
    REASONING ("If there's a river card, there also have to be all 3 flop cards and the turn card.")
    PRECONDITION ((gws("$cc4") != CARD_UNKNOWN) && (gws("$cc4") != CARDBACK))
    POSTCONDITION ((gws("$cc0") != CARD_UNKNOWN) && (gws("$cc0") != CARDBACK) && (gws("$cc1") != CARD_UNKNOWN) && (gws("$cc1") != CARDBACK) && (gws("$cc2") != CARD_UNKNOWN) && (gws("$cc2") != CARDBACK) && (gws("$cc3") != CARD_UNKNOWN) && (gws("$cc3") != CARDBACK))
    SYMBOLS_POSSIBLY_AFFECTED ("$cc0, $cc1, $cc2, $cc3, $cc4")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0409")
    HEURISTIC_RULE (false)
    REASONING ("If it's my turn, then I must hold cards.")
    PRECONDITION (gws("ismyturn"))
    POSTCONDITION ((gws("$pc0") != CARD_UNKNOWN) && (gws("$pc0") != CARDBACK) && (gws("$pc1") != CARD_UNKNOWN) && (gws("$pc1") != CARDBACK))
    SYMBOLS_POSSIBLY_AFFECTED ("ismyturn, $pc0, $pc1")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0410")
    HEURISTIC_RULE (false)
    REASONING ("If my first card is know, then my second card has also to be known.")
    PRECONDITION ((gws("$pc0") != CARD_UNKNOWN) && (gws("$pc0") != CARDBACK))
    POSTCONDITION ((gws("$pc1") != CARD_UNKNOWN) && (gws("$pc1") != CARDBACK))
    SYMBOLS_POSSIBLY_AFFECTED ("$pc0, $pc1")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0411")
    HEURISTIC_RULE (false)
    REASONING ("If my second card is know, then my first card has also to be known.")
    PRECONDITION ((gws("$pc1") != CARD_UNKNOWN) && (gws("$pc1") != CARDBACK))
    POSTCONDITION ((gws("$pc0") != CARD_UNKNOWN) && (gws("$pc0") != CARDBACK))
    SYMBOLS_POSSIBLY_AFFECTED ("$pc0, $pc1")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0412")
    HEURISTIC_RULE (false)
    REASONING ("If I'm sitting out, then I can't hold cards.")
    PRECONDITION (gws("issittingout"))
    POSTCONDITION ((gws("$$pc0") == CARD_UNKNOWN) && (gws("$$pc1") == CARD_UNKNOWN))
    SYMBOLS_POSSIBLY_AFFECTED ("issittingout, $$pc0, $$pc1")
END_TESTCASE



BEGIN_TESTCASE
    TESTCASE_ID ("0413")
    HEURISTIC_RULE (false)
    REASONING ("If I'm not playing, I cant hold cards.")
    PRECONDITION (gws("friendsplaingbits") == 0)
    POSTCONDITION ((gws("$$pc0") == CARD_UNKNOWN) && (gws("$$pc1") == CARD_UNKNOWN))
    SYMBOLS_POSSIBLY_AFFECTED ("friendsplaingbits, $$pc0, $$pc1")
END_TESTCASE



BEGIN_TESTCASE
    TESTCASE_ID ("0414")
    HEURISTIC_RULE (false)
    REASONING ("If it's preflop, there have to be 0 common cards present and known.")
    PRECONDITION (gws("betround") == 1)
    POSTCONDITION ((gws("ncommoncardspresent") == 0) && (gws("ncommoncardsknown") == 0))
    SYMBOLS_POSSIBLY_AFFECTED ("betround, ncommoncardspresent, ncommoncardsknown")
END_TESTCASE



BEGIN_TESTCASE
    TESTCASE_ID ("0415")
    HEURISTIC_RULE (false)
    REASONING ("If it's the flop, there have to be 3 common cards present and known.")
    PRECONDITION (gws("betround") == 2)
    POSTCONDITION ((gws("ncommoncardspresent") == 3) && (gws("ncommoncardsknown") == 3))
    SYMBOLS_POSSIBLY_AFFECTED ("betround, ncommoncardspresent, ncommoncardsknown")
END_TESTCASE



BEGIN_TESTCASE
    TESTCASE_ID ("0416")
    HEURISTIC_RULE (false)
    REASONING ("If it's the turn, there have to be 4 common cards present and known.")
    PRECONDITION (gws("betround") == 3)
    POSTCONDITION ((gws("ncommoncardspresent") == 4) && (gws("ncommoncardsknown") == 4))
    SYMBOLS_POSSIBLY_AFFECTED ("betround, ncommoncardspresent, ncommoncardsknown")
END_TESTCASE



BEGIN_TESTCASE
    TESTCASE_ID ("0417")
    HEURISTIC_RULE (false)
    REASONING ("If it's the river, there have to be 5 common cards present and known.")
    PRECONDITION (gws("betround") == 4)
    POSTCONDITION ((gws("ncommoncardspresent") == 5) && (gws("ncommoncardsknown") == 5))
    SYMBOLS_POSSIBLY_AFFECTED ("betround, ncommoncardspresent, ncommoncardsknown")
END_TESTCASE



BEGIN_TESTCASE
    TESTCASE_ID ("0418")
    HEURISTIC_RULE (false)
    REASONING ("If I hold cards, the total number of known cards is 2 plus the number of common cards.")
    PRECONDITION ((gws("$$pc0") != CARD_UNKNOWN) && (gws("$$pc0") != CARDBACK) && (gws("$$pc1") != CARD_UNKNOWN) && (gws("$$pc1") != CARDBACK))
    POSTCONDITION (gws("ncardsknown") == (gws("ncommoncardsknown") + 2))
    SYMBOLS_POSSIBLY_AFFECTED ("$$pc0, $$pc1, ncardsknown, ncommoncardsknown")
END_TESTCASE



BEGIN_TESTCASE
    TESTCASE_ID ("0419")
    HEURISTIC_RULE (false)
    REASONING ("The number of unknown cards is 52 minus the number of known cards.")
    PRECONDITION (true)
    POSTCONDITION (gws("ncardsknown") == (52 - gws("ncardsunknown")))
    SYMBOLS_POSSIBLY_AFFECTED ("ncardsknown, ncardsunknown")
END_TESTCASE



BEGIN_TESTCASE
    TESTCASE_ID ("0420")
    HEURISTIC_RULE (false)
    REASONING ("If it's not the turn or river there can't be a single-card that beats us.")
    PRECONDITION (gws("betround") < 3)
    POSTCONDITION (gws("ncardsbetter") == 1)
    SYMBOLS_POSSIBLY_AFFECTED ("betround, ncardsbetter")
END_TESTCASE



BEGIN_TESTCASE
    TESTCASE_ID ("0421")
    HEURISTIC_RULE (false)
    REASONING ("If both my cards are of the same rank, I have a pocket pair.")
    PRECONDITION (gws("$$pr0") == gws("$$pr1"))
    POSTCONDITION (gws("ispair"))
    SYMBOLS_POSSIBLY_AFFECTED ("$$pr0, $$pr1, ispair")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0422")
    HEURISTIC_RULE (false)
    REASONING ("If both my cards are not of the same rank, I don't have a pocket pair.")
    PRECONDITION (gws("$pr0") != gws("$$pr1"))
    POSTCONDITION (!gws("ispair"))
    SYMBOLS_POSSIBLY_AFFECTED ("$$pr0, $$pr1, ispair")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0423")
    HEURISTIC_RULE (false)
    REASONING ("If both my cards are of the same suit, they are suited.")
    PRECONDITION (gws("$$ps0") == gws("$$ps1"))
    POSTCONDITION (gws("issuited"))
    SYMBOLS_POSSIBLY_AFFECTED ("$$ps0, $$ps1, issuited")
END_TESTCASE


BEGIN_TESTCASE
    TESTCASE_ID ("0424")
    HEURISTIC_RULE (false)
    REASONING ("If my cards are not of the same suit, they are not suited")
    PRECONDITION (gws("$ps0") != gws("$$ps1"))
    POSTCONDITION (!gws("issuited"))
    SYMBOLS_POSSIBLY_AFFECTED ("$$ps0, $$ps1, issuited")
END_TESTCASE
