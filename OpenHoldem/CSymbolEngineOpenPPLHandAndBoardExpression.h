//*****************************************************************************
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//*****************************************************************************
//
// Purpose:
//
//*****************************************************************************

#ifndef INC_CSYMBOLENGINE_HAND_AND_BOARD_EXPRESSION_H
#define INC_CSYMBOLENGINE_HAND_AND_BOARD_EXPRESSION_H

#include "CVirtualSymbolEngine.h"

class CSymbolEngineOpenPPLHandAndBoardExpression: public CVirtualSymbolEngine
{
public:
	CSymbolEngineOpenPPLHandAndBoardExpression();
	~CSymbolEngineOpenPPLHandAndBoardExpression();
public:
	// Mandatory reset-functions
	void InitOnStartup();
	void ResetOnConnection();
	void ResetOnHandreset();
	void ResetOnNewRound();
	void ResetOnMyTurn();
	void ResetOnHeartbeat();
public:
	// Public accessors
	bool EvaluateSymbol(const char *name, double *result, bool log = false);
	CString SymbolsProvided();
private:
	int PrimeCodedRanks(int card_0,	int card_1, 
		int opt_card_2 = CARD_UNDEFINED, int opt_card_3 = CARD_UNDEFINED, 
		int opt_card_4 = CARD_UNDEFINED);
	int PrimeCodedRanks(CString card_expression);
private:
	int _prime_coded_hole_cards;
	int _prime_coded_board_cards;
private:
	int prime_coded_available_ranks;
	bool is_hand_expression;
	bool is_board_expression;
	CString hand_or_board_expression;
};

extern CSymbolEngineOpenPPLHandAndBoardExpression *p_symbol_engine_open_ppl_hand_and_board_expression;

#endif INC_CSYMBOLENGINE_HAND_AND_BOARD_EXPRESSION_H