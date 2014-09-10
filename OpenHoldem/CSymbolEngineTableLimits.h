//****************************************************************************** 
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//****************************************************************************** 
//
// Purpose:
//
//****************************************************************************** 

#ifndef INC_CTABLELIMITS_H
#define INC_CTABLELIMITS_H

#include "MagicNumbers.h"
#include "CVirtualSymbolEngine.h"

#define BIG_BLIND p_symbol_engine_tablelimits->bblind()
#define SMALL_BLIND p_symbol_engine_tablelimits->sblind()

struct STableLimit
{
	double sblind;
	double bblind;
	double bbet;
	// We don't deal with antes here, as they are very rarely used.
	// They get stored separately and not auto-locked.
};

class CSymbolEngineTableLimits: public CVirtualSymbolEngine
{
public:
	CSymbolEngineTableLimits();
	~CSymbolEngineTableLimits();
	// public mutators
public:
	void InitOnStartup();
	void ResetOnConnection();
	void ResetOnHandreset();
	void ResetOnNewRound();
	void ResetOnMyTurn();
	void ResetOnHeartbeat();
	void CalcTableLimits();
public:
	bool EvaluateSymbol(const char *name, double *result, bool log = false);
  CString SymbolsProvided();
public:
  // public accessors, formerly part of the symbol-structure.
	double sblind();
	double bblind();
	double bigbet();
	double ante();
	double bet(int betround);
	double bet(); // for current betting round
public:
	int gametype()		{ return _gametype; }	// former p_symbol_engine_various_data_lookup->sym()->lim
	bool isnl()			{ return (gametype() == k_gametype_NL); }
	bool ispl()			{ return (gametype() == k_gametype_PL); }
	bool isfl()			{ return (gametype() == k_gametype_FL); }
public:
	CString GetGametypeAsString();
private:
	void SetSmallBlind(double small_blind);
	void SetBigBlind(double big_blind);
	void SetBigBet(double big_bet);
	void SetAnte(double ante);
	void SetBet(int betround, double bet);
	void SetGametype(int gametype); 
	// public accessors
public:
	// private functions
private:
	void AutoLockBlinds();
	void AutoLockBlindsForCurrentHand();
	void AutoLockBlindsForCashgamesAfterNHands();
	bool ReasonableBlindsForCurrentHand();
	void RememberBlindsForCashgames();
	bool IsCalculationNeccessary();
	void CalcTableLimits_NL_PL();
	void CalcTableLimits_FL_AndUnknownGametype();
	void SwapBlindsIfSbGreaterThanBBAndNotZero();
	void SearchTableForSbAndBbValue();
	void CalcBetsizesForEveryStreet();
	void AcceptNewValuesIfGood();
	double GuessSmallBlindFromBigBlind();
	double GuessBigBlindFromSmallBlind();
	double GuessBigBetFromBigBlind();
	void AdjustForReasonableness();
	void ResetBets();
	// private data members
private:
	bool blinds_locked_for_current_hand; 
	bool blinds_locked_for_complete_session;
private:
	STableLimit tablelimit_unreliable_input;
	STableLimit tablelimit_last_known_good_value;
	STableLimit tablelimit_locked_for_current_hand;
	STableLimit tablelimit_locked_for_complete_session;
private:
	static const int k_number_of_hands_to_autolock_blinds_for_cashgames = 5;
	int number_of_saved_tablelimits;
private:
	// We store the table-limits for the first N hands in separate arrays
	// instead of TableLimit structures, as we need to operate on arrays later (median function).
	double tablelimits_first_N_hands_sblind[k_number_of_hands_to_autolock_blinds_for_cashgames];
	double tablelimits_first_N_hands_bblind[k_number_of_hands_to_autolock_blinds_for_cashgames];
	double tablelimits_first_N_hands_bbet[k_number_of_hands_to_autolock_blinds_for_cashgames];
private:
	int		_gametype;
	double	_ante;
	// Index 1..4 is for current bettinground, 0 is unused
	double _betsizes_for_all_bettingrounds[k_number_of_betrounds+1]; 
};

extern CSymbolEngineTableLimits *p_symbol_engine_tablelimits;

#endif // INC_CTABLELIMITS_H
