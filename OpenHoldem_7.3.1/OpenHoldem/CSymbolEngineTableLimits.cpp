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

#include "stdafx.h"
#include "CSymbolEngineTableLimits.h"

#include <assert.h>
#include "CBetroundCalculator.h"
#include "CBlindGuesser.h"
#include "CPreferences.h"
#include "CScraper.h"
#include "CScraperAccess.h"
#include "CSymbolEngineDealerchair.h"
#include "CSymbolEngineIsTournament.h"
#include "CTableState.h"
#include "debug.h"
#include "FloatingPoint_Comparisions.h"
#include "MagicNumbers.h"
#include "Median.h"
#include "Numericalfunctions.h"

CSymbolEngineTableLimits	*p_symbol_engine_tablelimits = NULL;

CSymbolEngineTableLimits::CSymbolEngineTableLimits() {
	// The values of some symbol-engines depend on other engines.
	// As the engines get later called in the order of initialization
	// we assure correct ordering by checking if they are initialized.
	assert(p_symbol_engine_dealerchair != NULL);
	//
	// Actually this symbol-engine also depends on 
	// p_symbol_engine_is_tournament.
	// This is a circular dependency, but this does not really matter,
	// as the value of istournament() should be constant for the
	// entire session, so it does not matter, if we use old values
	// from the last heartbeat.
}

CSymbolEngineTableLimits::~CSymbolEngineTableLimits() {
}

void CSymbolEngineTableLimits::InitOnStartup() {
	ResetOnConnection();
}

void CSymbolEngineTableLimits::ResetOnConnection() {
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] ResetOnConnection()\n");
	number_of_saved_tablelimits = 0;
	for (int i=0; i<k_number_of_hands_to_autolock_blinds_for_cashgames; i++)	{
		tablelimits_first_N_hands_sblind[i] = 0;
		tablelimits_first_N_hands_bblind[i] = 0;
		tablelimits_first_N_hands_bbet[i]   = 0;
	}
	_ante = 0;
	_gametype = k_gametype_unknown;
  tablelimit_best_guess.bbet = 0;
  tablelimit_best_guess.bblind  = 0;
  tablelimit_best_guess.sblind = 0;
  tablelimit_locked_for_complete_session.bbet = 0;
  tablelimit_locked_for_complete_session.bblind = 0;
	tablelimit_locked_for_complete_session.sblind = 0;
  blinds_locked_for_complete_session = false;
  // ResetOnHandreset also cares about tablelimit_locked_for_current_hand
	ResetOnHandreset();
}

void CSymbolEngineTableLimits::ResetOnHandreset() {
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] ResetOnHandreset()\n");
	blinds_locked_for_current_hand = false;
	tablelimit_locked_for_current_hand.sblind = 0;
	tablelimit_locked_for_current_hand.bblind = 0;
	tablelimit_locked_for_current_hand.bbet   = 0;
	// We have to reset the known good values also,
	// * as the blinds could change (tournament)
	// * as they could be wrong and we would assume,
	//   that we know the blinds and don't calculate anything.
	tablelimit_best_guess.sblind = 0;
	tablelimit_best_guess.bblind = 0;
	tablelimit_best_guess.bbet   = 0;
}

void CSymbolEngineTableLimits::ResetOnNewRound() {
}

void CSymbolEngineTableLimits::ResetOnMyTurn() {
}

void CSymbolEngineTableLimits::ResetOnHeartbeat() {
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] ResetOnHeartbeat()\n");
  if (TableLimitsNeedToBeGuessed()) {
    CBlindGuesser _blind_guesser;
    _blind_guesser.Guess(&tablelimit_best_guess.sblind,
      &tablelimit_best_guess.bblind,
      &tablelimit_best_guess.bbet);
    AutoLockBlinds(); // here???
  }
}

bool CSymbolEngineTableLimits::TableLimitsNeedToBeGuessed() {
  if (blinds_locked_for_current_hand || blinds_locked_for_complete_session) {
    return false;
  }
  return true;
}

void CSymbolEngineTableLimits::AutoLockBlindsForCashgamesAfterNHands() {
	if (p_symbol_engine_istournament == NULL)	{
		// Initialization phase
		// Null pointer possible due to circular dependency
		// Simply leave because it is too early to lock the blinds
		return;
	}
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] AutoLockBlindsForCashgamesAfterNHands()\n");
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] blinds_locked_for_complete_session: %d\n", blinds_locked_for_complete_session);
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] istournament: %d\n", IsTrue(p_symbol_engine_istournament->istournament()));
	if (blinds_locked_for_complete_session) {
    write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] blinds_locked_for_complete_session\n");
		write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] Leaving CSymbolEngineTableLimits::AutoLockBlindsForCashgamesAfterNHands() early\n");
		return;
  }
  if (p_symbol_engine_istournament->istournament())	{
		write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] istournament\n");
		write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] Leaving CSymbolEngineTableLimits::AutoLockBlindsForCashgamesAfterNHands() early\n");
		return;
	}
	if (number_of_saved_tablelimits == k_number_of_hands_to_autolock_blinds_for_cashgames) {
		// We simply take the median as the "correct" value.
		// This works, as long as less than half of the values are too small
		// and less than half of the values are too high.
		// Rasonable assumption, otherwise we have name serious problem anyway.
		tablelimit_locked_for_complete_session.sblind = median(tablelimits_first_N_hands_sblind, k_number_of_hands_to_autolock_blinds_for_cashgames);
		tablelimit_locked_for_complete_session.bblind = median(tablelimits_first_N_hands_bblind, k_number_of_hands_to_autolock_blinds_for_cashgames);
		tablelimit_locked_for_complete_session.bbet   = median(tablelimits_first_N_hands_bbet,   k_number_of_hands_to_autolock_blinds_for_cashgames);
		blinds_locked_for_complete_session = true;
		write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] Locking blinds at %.2f / %.2f / %.2f\n", tablelimit_locked_for_complete_session.sblind, 
			tablelimit_locked_for_complete_session.bblind, tablelimit_locked_for_complete_session.bbet);
	}	else {
		write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] Not yet enough hands to lock blinds permanent.\n");
	}
}

void CSymbolEngineTableLimits::RememberBlindsForCashgames()
{
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] RememberBlindsForCashgames()\n");
	if (number_of_saved_tablelimits < k_number_of_hands_to_autolock_blinds_for_cashgames)
	{
		write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] Saving blinds for hand %d\n", number_of_saved_tablelimits);
		tablelimits_first_N_hands_sblind[number_of_saved_tablelimits] = tablelimit_locked_for_current_hand.sblind;
		tablelimits_first_N_hands_bblind[number_of_saved_tablelimits] = tablelimit_locked_for_current_hand.bblind;
		tablelimits_first_N_hands_bbet[number_of_saved_tablelimits]   = tablelimit_locked_for_current_hand.bbet;
		number_of_saved_tablelimits++;
	}
	else
	{
		write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] Already enough saved blinds.\n");
	}
}

void CSymbolEngineTableLimits::AutoLockBlindsForCurrentHand() {
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] AutoLockBlindsForCurrentHand()\n");
	blinds_locked_for_current_hand = true;
	tablelimit_locked_for_current_hand.sblind = tablelimit_best_guess.sblind;
	tablelimit_locked_for_current_hand.bblind = tablelimit_best_guess.bblind;
	tablelimit_locked_for_current_hand.bbet	  = tablelimit_best_guess.bbet;
	write_log(preferences.debug_table_limits(), 
    "[CSymbolEngineTableLimits] Locked blinds at %.2f / %.2f / %f.2\n", 
    tablelimit_locked_for_current_hand.sblind,
		tablelimit_locked_for_current_hand.bblind, 
    tablelimit_locked_for_current_hand.bbet);
	RememberBlindsForCashgames();
}

void CSymbolEngineTableLimits::SetGametype(int gametype)
{
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] SetGametype() new gametype: %d\n", gametype);
	assert(gametype >= k_gametype_unknown);
	assert(gametype <= k_gametype_FL);
	_gametype = gametype;
}

void CSymbolEngineTableLimits::AutoLockBlinds()
{
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] AutoLockBlinds()\n");
	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] blinds_locked_for_current_hand: %d\n", blinds_locked_for_current_hand);
	//!!!write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] ReasonableBlindsForCurrentHand(): %d\n", ReasonableBlindsForCurrentHand());
	//!!!!!if (!blinds_locked_for_current_hand && ReasonableBlindsForCurrentHand())
	{
		AutoLockBlindsForCurrentHand();
		AutoLockBlindsForCashgamesAfterNHands();
	}
}

void CSymbolEngineTableLimits::CalcTableLimits() { 
 	write_log(preferences.debug_table_limits(), "[CSymbolEngineTableLimits] CalcTableLimits()\n");
	//!!!!!if (!IsCalculationNeccessary())	return;
	// if we still do not have blinds, then infer them from the posted bets

	//!!!AcceptNewValuesIfGood();
	AutoLockBlinds();
	// Calc miminum betsizes for every streeet (after! we have potentially locked the blinds)
	//!!!!!CalcBetsizesForEveryStreet();
}
/*
void CSymbolEngineTableLimits::AcceptNewValuesIfGood()
{
	if ((tablelimit_unreliable_input.sblind >= 0.01) 
		&& (tablelimit_unreliable_input.bblind > tablelimit_unreliable_input.sblind)
		&& (tablelimit_unreliable_input.bbet >= tablelimit_unreliable_input.bblind))
	{
		tablelimit_last_known_good_value.sblind = tablelimit_unreliable_input.sblind;
		tablelimit_last_known_good_value.bblind = tablelimit_unreliable_input.bblind;
		tablelimit_last_known_good_value.bbet   = tablelimit_unreliable_input.bbet;
	}
}*/

STableLimit CSymbolEngineTableLimits::BestTableLimitsToBeUsed() {
  if (blinds_locked_for_complete_session) {
		return tablelimit_locked_for_complete_session;
	}	else if (blinds_locked_for_current_hand) {
		return tablelimit_locked_for_current_hand;
	}	else {
		return tablelimit_best_guess;
	}
}

double CSymbolEngineTableLimits::sblind() {
	return BestTableLimitsToBeUsed().sblind;
}

double CSymbolEngineTableLimits::bblind(){
	return BestTableLimitsToBeUsed().bblind;
}

double CSymbolEngineTableLimits::bigbet(){
	return BestTableLimitsToBeUsed().bbet;
}

double CSymbolEngineTableLimits::ante() {
	return _ante; 
}

double CSymbolEngineTableLimits::buyin() {
  if (p_scraper->s_limit_info()->buyin > 0) {
    return p_scraper->s_limit_info()->buyin;
  }
  return k_undefined_zero;
}

double CSymbolEngineTableLimits::bet(int betround) {
	assert(betround >= k_betround_preflop);
	assert(betround <= k_betround_river);
  if (isfl() && (betround >= k_betround_turn)) {
    return bigbet();
  }
	return bblind();
}

double CSymbolEngineTableLimits::bet() {
	return (bet(p_betround_calculator->betround()));
}

CString CSymbolEngineTableLimits::GetGametypeAsString() {
	CString result = "";
	if (isnl())	{
		result = "NL";
	}	else if (ispl()) {
		result = "PL";
	}	else if (isfl()) {
		result = "FL";
	}	else {
		result = "?L";
	}
	if (p_symbol_engine_istournament->istournament()) {
		result += "T";
	}
	return result;
}

bool CSymbolEngineTableLimits::EvaluateSymbol(const char *name, double *result, bool log /* = false */) {
  FAST_EXIT_ON_OPENPPL_SYMBOLS(name);
	if (memcmp(name, "is", 2)==0)	{
		if (memcmp(name, "isnl", 4)==0 && strlen(name)==4) {
			*result = isnl();
		}	else if (memcmp(name, "ispl", 4)==0 && strlen(name)==4)	{
			*result = ispl();
		}	else if (memcmp(name, "isfl", 4)==0 && strlen(name)==4)	{
			*result = isfl();
		}	else {
			// Invalid symbol
			return false;
		}
		// Valid symbol
		return true;
	}	else if (memcmp(name, "bet", 3)==0)	{
		if (memcmp(name, "bet", 3)==0 && strlen(name)==3) {
			*result = bet();
      return true;
		}	else if (memcmp(name, "bet", 3)==0 && strlen(name)==4) {
      char betround = name[3];
      if ((betround >= '1') && (betround <= '4')) {
			  *result = bet(name[3]-'0');
        return true;
      }
		}
    // Invalid symbol
		return false;
	}
	if (memcmp(name, "bblind", 6)==0 && strlen(name)==6) {
		*result = bblind();
	}	else if (memcmp(name, "sblind", 6)==0 && strlen(name)==6)	{
		*result = sblind();
	}	else if (memcmp(name, "ante", 4)==0 && strlen(name)==4)	{
		*result = ante();
	}	else if (memcmp(name, "lim", 3)==0 && strlen(name)==3) {
		*result = gametype();
	}	else if (memcmp(name, "buyin", 5)==0 && strlen(name)==5) {
		*result = buyin();
  }	else {
		// Symbol of a different symbol-engine
		return false;
	}
	// Valid symbol
	return true;
}

CString CSymbolEngineTableLimits::SymbolsProvided() {
  CString list = "isnl ispl isfl lim bet bblind sblind ante buyin ";
  list += RangeOfSymbols("bet%i", k_betround_preflop, k_betround_river);
  return list;
}



