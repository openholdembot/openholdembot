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

#ifndef INC_CGAMESTATE_H
#define INC_CGAMESTATE_H

#include "CDllExtension.h"
#include "..\CCritSec\CCritSec.h"
#include "MagicNumbers.h"

class CGameState {
  friend class CSymbolEngineTableStats;
 public:
	// public functions
	CGameState();
	~CGameState();
	void ProcessGameState(const SHoldemState *pstate);
	void ProcessFtr(const SHoldemState *pstate);
	void CaptureState();
	bool ProcessThisFrame();
	const int LastRaised(const int round);
	const double SortedBalance(const int rank);
 public:
	// public accessors
	const int hands_played();
	const SHoldemState * state(const int i) { if (i>=0 && i<=255) return &_state[i]; else return NULL; }
	const int state_index() { return _state_index; }
#define ENT CSLock lock(m_critsec);
	// public mutators
	void	set_new_hand(const bool b) { ENT _new_hand = b; }
#undef ENT
 protected:
  // Gets still used by the module CTableStats,
  // which formerly was part of CGameState.
  // Ugly mess, but less ugly and less messy than before.
  ftr_info _m_ftr[k_number_of_holdem_states_for_DLL]; // ?? WTF is ftr?
	int      _m_ftr_ndx;
 private:
  void WriteSummaryHeading();
 private:
	// private variables - use public accessors and public mutators to address these
	int					_hands_played;
	bool				_new_hand;
	SHoldemState		_state[k_number_of_holdem_states_for_DLL];
	unsigned char		_state_index;
 private:
	// private functions and variables - not available via accessors or mutators
	void ProcessStateEngine(const SHoldemState *pstate, const bool pstate_changed) ;
	void ProcessFtrEngine(const SHoldemState *pstate) ;
	void DumpState(void) ;
	SHoldemState		_m_holdem_state[k_number_of_holdem_states_for_DLL];
	unsigned char		_m_ndx;
	int					_nopponentsplaying_last;
	bool				_process_game_state;
	bool				_safe_to_process_state;
	bool				_end_of_hand;
	bool				_first_pass_capture;
	int					_oh_br_last;
	bool				_pot_raised;
	int					_pf_limpers_n;
	bool				_my_first_action_this_round;
	bool				_my_first_action_this_hand;
	bool				_small_blind_posted;
	bool				_big_blind_posted;
	double				_bets_last;
	SHoldemState	_m_game_state[k_number_of_holdem_states_for_DLL];
	unsigned char	_m_game_ndx;
	bool				_chair_actions[k_max_number_of_players][k_number_of_betrounds][w_num_action_types];   // 10 chairs, 4 rounds, number of action types
	int					_ftr_dealer_chair_last;
	int					_ftr_ncommoncardsknown_last;
	int					_ftr_nplayersdealt_last;
	CCritSec			m_critsec;
};

extern CGameState *p_game_state;

#endif //INC_CGAMESTATE_H