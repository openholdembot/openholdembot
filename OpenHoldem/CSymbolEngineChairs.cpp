//*******************************************************************************
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//*******************************************************************************
//
// Purpose: Various chair symbols, ecept dealerchair, userchair and raischair,
//   which are of special importance and get handled by other symbol-engines.
//
//*******************************************************************************

#include "stdafx.h"
#include "CSymbolEngineChairs.h"

#include "CBetroundCalculator.h"
#include "CScraper.h"
#include "CSymbolEngineActiveDealtPlaying.h"
#include "CSymbolEnginePokerAction.h"
#include "CSymbolEngineTableLimits.h"
#include "CSymbolEngineUserchair.h"
#include "CTableState.h"
#include "..\CTablemap\CTablemap.h"

CSymbolEngineChairs *p_symbol_engine_chairs = NULL;

CSymbolEngineChairs::CSymbolEngineChairs() {
  // The values of some symbol-engines depend on other engines.
	// As the engines get later called in the order of initialization
	// we assure correct ordering by checking if they are initialized.
  assert(p_symbol_engine_active_dealt_playing != NULL);
  assert(p_symbol_engine_poker_action != NULL);
  assert(p_symbol_engine_userchair != NULL);
  assert(p_symbol_engine_tablelimits != NULL);
}

CSymbolEngineChairs::~CSymbolEngineChairs() {
}

void CSymbolEngineChairs::InitOnStartup() {
  ResetOnConnection();
}

void CSymbolEngineChairs::ResetOnConnection() {
  _nchairs = p_tablemap->nchairs();
  ResetOnHandreset();
}

void CSymbolEngineChairs::ResetOnHandreset() {
  ResetOnNewRound();
}

void CSymbolEngineChairs::ResetOnNewRound() {
  _opponent_headsup_chair = k_undefined;
  _smallblind_chair = k_undefined;
  _bigblind_chair = k_undefined;
  _cutoff_chair = k_undefined;
  _firstcaller_chair = k_undefined;
  _lastcaller_chair = k_undefined;
  _firstraiser_chair = k_undefined;
}

void CSymbolEngineChairs::ResetOnMyTurn() {
  // Only well-defined at my turn and requires userchair for calculation.
  // But it seems some "casinos" like test-suite can break that condition.
  // http://www.maxinmontreal.com/forums/viewtopic.php?f=110&t=17915#p124550
  //assert(p_symbol_engine_userchair->userchair_confirmed());
  CalculateOpponentHeadsupChair();
  CalculateSmallBlindChair();
  CalculateBigBlindChair();
  CalculateCutOffChair();
  CalculateCallerChairs();
  CalculateFirstRaiserChair();
}

void CSymbolEngineChairs::ResetOnHeartbeat() {
}

void CSymbolEngineChairs::CalculateOpponentHeadsupChair(){
  _opponent_headsup_chair = k_undefined;
	if (p_symbol_engine_active_dealt_playing->nopponentsplaying() > 1) return;
	for (int i = 0; i < k_max_number_of_players; ++i)	{
		if (IsBitSet(p_symbol_engine_active_dealt_playing->opponentsplayingbits(), i)) {
      _opponent_headsup_chair = i;
      return;
    }
	}
}

void CSymbolEngineChairs::CalculateSmallBlindChair() {
  _smallblind_chair = GetChairByDealposition(1);
}

void CSymbolEngineChairs::CalculateBigBlindChair() {
  _bigblind_chair = GetChairByDealposition(2);
}

void CSymbolEngineChairs::CalculateCutOffChair() {
  int cutoff_dealposition = p_symbol_engine_active_dealt_playing->nplayersdealt() - 1;
  _cutoff_chair = GetChairByDealposition(cutoff_dealposition);
}

void CSymbolEngineChairs::CalculateCallerChairs() {
  _firstcaller_chair = k_undefined;
  _lastcaller_chair = k_undefined;
  double last_raisers_bet = p_table_state->User()->_bet;
  if ((p_betround_calculator->betround() == k_betround_preflop) 
      && (last_raisers_bet < p_symbol_engine_tablelimits->bblind())) {
    // Avoid problems with so-called "blind-raisers"
    last_raisers_bet = p_symbol_engine_tablelimits->bblind();
  }
  for (int i=1; i<_nchairs; ++i) {
    int next_chair = (p_symbol_engine_userchair->userchair() + i) % _nchairs;
    double next_bet = p_table_state->_players[next_chair]._bet;
    if ((next_bet == last_raisers_bet) && (next_bet > 0)) {
      // We have a caller, at least the temporary last one
      _lastcaller_chair = next_chair;
      if (_firstcaller_chair == k_undefined) {
        // We found the first caller
        _firstcaller_chair = next_chair;
      } 
    } else if (next_bet > last_raisers_bet) {
      // New raiser found (necessary for potential new last_caller)
      last_raisers_bet = next_bet; 
    }
  }
}

void CSymbolEngineChairs::CalculateFirstRaiserChair() {
  _firstraiser_chair = k_undefined;
  double users_bet = p_table_state->User()->_bet;
  for (int i=1; i<_nchairs; ++i) {
    int next_chair = (p_symbol_engine_userchair->userchair() + i) % _nchairs;
    double next_bet = p_table_state->_players[next_chair]._bet;
    if ((next_bet > users_bet) && (next_bet > p_symbol_engine_tablelimits->bblind())) {
      _firstraiser_chair = next_chair; 
      return;
    }
  }
}

int CSymbolEngineChairs::GetChairByDealposition(int dealposition) {
  for (int i=0; i<_nchairs; ++i) {
    if (p_symbol_engine_poker_action->DealPosition(i) == dealposition) {
      return i;
    }
  }
  return k_undefined;
}

bool CSymbolEngineChairs::EvaluateSymbol(const char *name, double *result, bool log /* = false */) {
  if (memcmp(name, "opponent_chair_headsup", 22)==0) {
		*result = _opponent_headsup_chair;  
		return true;
	} else if (memcmp(name, "smallblind_chair", 16)==0) {
		*result = _smallblind_chair;
		return true;
	} else if (memcmp(name, "bigblind_chair", 14)==0) {
		*result = _bigblind_chair;
		return true;
	} else if (memcmp(name, "cutoff_chair", 12)==0) {
		*result = _cutoff_chair;
		return true;
	} else if (memcmp(name, "firstcaller_chair", 17)==0) {
		*result = _firstcaller_chair;
		return true;
	} else if (memcmp(name, "lastcaller_chair", 16)==0) {
		*result = _lastcaller_chair;
		return true;
	} else if (memcmp(name, "firstraiser_chair", 17)==0) {
		*result = _firstraiser_chair;
		return true;
	}
  // Symbol of a different symbol-engine
	return false;
}

CString CSymbolEngineChairs::SymbolsProvided() {
  return "opponent_chair_headsup smallblind_chair bigblind_chair "
    "cutoff_chair firstcaller_chair lastcaller_chair firstraiser_chair ";
}

