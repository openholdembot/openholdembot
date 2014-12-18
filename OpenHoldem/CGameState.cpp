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
#include "CGameState.h"

#include "assert.h"
#include "CAutoConnector.h"
#include "CBetroundCalculator.h"
#include "CEngineContainer.h"
#include "CHandresetDetector.h"
#include "CSymbolEngineActiveDealtPlaying.h"
#include "CSymbolEngineAutoplayer.h"
#include "CSymbolEngineCards.h"
#include "CSymbolEngineChipAmounts.h"
#include "CSymbolEngineDealerchair.h"
#include "CSymbolEngineHistory.h"
#include "CSymbolEnginePrwin.h"
#include "CSymbolEngineTime.h"
#include "CSymbolEngineUserchair.h"
#include "CScraper.h"
#include "CScraperAccess.h"
#include "CPreferences.h"
#include "CSymbolEngineTableLimits.h"
#include "CTableState.h"
#include "MagicNumbers.h"
#include "Numericalfunctions.h"

CGameState			*p_game_state = NULL;

CGameState::CGameState()
{
	_m_ndx = 0;
	_m_game_ndx = 0;
	_m_ftr_ndx = 0;
	_new_hand = true;
	_nopponentsplaying_last = 0;
	_process_game_state = false;
	_safe_to_process_state = false;
	_end_of_hand = false;
	_first_pass_capture = false;
	_oh_br_last = k_betround_preflop;
	_pot_raised = false;
	_pf_limpers_n = 0;
	_my_first_action_this_round = true;
	_my_first_action_this_hand = true;
	_small_blind_posted = false;
	_big_blind_posted = false;
	_bets_last = 0.0;

	_ftr_dealer_chair_last = 0;
	_ftr_ncommoncardsknown_last = 0;
	_ftr_nplayersdealt_last = 0;
}

CGameState::~CGameState()
{
}

void CGameState::ProcessGameState(const SHoldemState *pstate)
{
	bool		pstate_changed = false;
	int			betround = p_betround_calculator->betround();
	int			sym_nopponentsdealt = p_symbol_engine_prwin->nopponents_for_prwin();
	int			sym_nopponentsplaying = p_symbol_engine_active_dealt_playing->nopponentsplaying();
	bool		sym_ismyturn = p_symbol_engine_autoplayer->ismyturn();
	bool		sym_ismanual = p_symbol_engine_autoplayer->ismanual();

	// Has the number of opponents changed?
	if (sym_nopponentsplaying != _nopponentsplaying_last)
		_nopponentsplaying_last = sym_nopponentsplaying;

	// see if the new pstate has changed in an interesting way
	pstate_changed = false;
	if (pstate->m_dealer_chair != _m_holdem_state[(_m_ndx)&0xff].m_dealer_chair)
		pstate_changed = true;

	for (int i=0; i<k_max_number_of_players; i++)
	{
		if (pstate->m_player[i].m_balance != _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_balance)
			pstate_changed = true;

		if (pstate->m_player[i].m_currentbet != _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_currentbet)
			pstate_changed = true;

		if (pstate->m_player[i].m_cards[0] != _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_cards[0])
			pstate_changed = true;

		if (pstate->m_player[i].m_cards[1] != _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_cards[1])
			pstate_changed = true;

		if (pstate->m_player[i].m_balance_known != _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_balance_known &&
				pstate->m_player[i].m_cards[0] != 0 && pstate->m_player[i].m_cards[1] != 0)
			pstate_changed = true;
	}
	for (int i=0; i<k_number_of_community_cards; i++)
	{
		if (pstate->m_cards[i] != _m_holdem_state[(_m_ndx)&0xff].m_cards[i])
			pstate_changed = true;
	}

	// only process wh state if something interesting within the structure changes
	if (pstate!=NULL && (pstate_changed || sym_ismyturn))
		ProcessStateEngine(pstate, pstate_changed);
  set_new_hand(false);
}

void CGameState::ProcessFtr(const SHoldemState *pstate)
{
	ProcessFtrEngine(pstate);
}

void CGameState::CaptureState()
{
	bool				    playing = true;
	unsigned char		card = CARD_NOCARD;

	// figure out if I am playing
	int sym_chair = p_symbol_engine_userchair->userchair();
	if (!p_symbol_engine_userchair->userchair_confirmed())
	{
		playing = false;
	}
	else if (!p_table_state->User()->HasKnownCards())
	{
		playing = false;
	}

	// Poker window title
	char title[MAX_WINDOW_TITLE];
	GetWindowText(p_autoconnector->attached_hwnd(), title, MAX_WINDOW_TITLE);

	strncpy_s(_state[_state_index&0xff].m_title, 64, title, _TRUNCATE);
	_state[_state_index&0xff].m_title[63] = '\0';

	// Pot information
	for (int i=0; i<k_max_number_of_players; i++)
		_state[_state_index&0xff].m_pot[i] = p_scraper->pot(i);

	// Common cards
	for (int i=0; i<k_number_of_community_cards; i++)
	{
    int common_card = p_table_state->_common_cards[i].GetValue();
    write_log(preferences.debug_dll_extension(), 
      "[CGameState] Common card %i = %i\n", i, common_card);
		_state[_state_index&0xff].m_cards[i] = common_card;
	}

	// playing, posting, dealerchair
	int sym_dealerchair = p_symbol_engine_dealerchair->dealerchair();
	bool sym_isautopost = p_symbol_engine_autoplayer->isautopost();
	_state[_state_index&0xff].m_is_playing = playing;
	_state[_state_index&0xff].m_is_posting = sym_isautopost;
	_state[_state_index&0xff].m_fillerbits = 0;
	_state[_state_index&0xff].m_fillerbyte = 0;
	_state[_state_index&0xff].m_dealer_chair = sym_dealerchair;

	// loop through all 10 player chairs
	for (int i=0; i<k_max_number_of_players; i++)
	{

		// player name, balance, currentbet
    strncpy_s(_state[_state_index&0xff].m_player[i].m_name, 16, p_table_state->_players[i]._name.GetString(), _TRUNCATE);
    _state[_state_index&0xff].m_player[i].m_balance = p_table_state->_players[i]._balance;
		_state[_state_index&0xff].m_player[i].m_currentbet = p_symbol_engine_chip_amounts->currentbet(i);

		// player cards
		for (int j=0; j<k_number_of_cards_per_player; j++)
		{
      Card player_card = p_table_state->_players[i]._hole_cards[j];
      int card = player_card.GetValue();
      write_log(preferences.debug_dll_extension(),
        "[CGameState] Plazer card [%i][%i] = %i\n", i, j, card);
			_state[_state_index&0xff].m_player[i].m_cards[j] = card;
		}

		// player name known, balance known
		_state[_state_index&0xff].m_player[i].m_name_known = p_scraper_access->IsGoodPlayername(i);
		_state[_state_index&0xff].m_player[i].m_balance_known = true;
		_state[_state_index&0xff].m_player[i].m_fillerbits = 0;
		_state[_state_index&0xff].m_player[i].m_fillerbyte = 0;
	}

	_state_index++;
}

const int CGameState::LastRaised(const int round)
{
	int last_raised = -1;

	if (round<1 || round>4)
		return last_raised;

	for (int i=_m_game_state[(_m_game_ndx)&0xff].m_dealer_chair+1; i<=_m_game_state[(_m_game_ndx)&0xff].m_dealer_chair+k_max_number_of_players; i++)
	{
		int index_normalized = i%k_max_number_of_players;
		if (_chair_actions[index_normalized][round-1][w_reraised])
		{
			last_raised=index_normalized;
		}
	}

	if (last_raised==-1)
	{
		for (int i=_m_game_state[(_m_game_ndx)&0xff].m_dealer_chair+1; i<=_m_game_state[(_m_game_ndx)&0xff].m_dealer_chair+k_max_number_of_players; i++)
		{
			int index_normalized = i%k_max_number_of_players;
			if (_chair_actions[index_normalized][round-1][w_raised])
			{
				last_raised=index_normalized;
			}
		}
	}

	return last_raised;
}

const double CGameState::SortedBalance(const int rank)
{
	double	stacks[k_max_number_of_players] = {0.};

	for (int i=0; i<k_max_number_of_players; i++)
		stacks[i] = _m_holdem_state[_m_ndx].m_player[i].m_balance + _m_holdem_state[_m_ndx].m_player[i].m_currentbet;

	// bubble sort stacks // !! duplicate code?
	for (int i=0; i<(k_max_number_of_players-1); i++)
	{
		for (int n=i+1; n<k_max_number_of_players; n++)
		{
			if (stacks[i] < stacks[n])
			{
				SwapDoubles(&stacks[i], &stacks[n]);
			}
		}
	}

	return stacks[rank];
}

// Return true if the game state processor should process the current frame.
// Also used to stall the autoplayer until after the game state processor has
// processed the current frame.
bool CGameState::ProcessThisFrame (void)
{
	int				betround =  p_betround_calculator->betround();
	int				activebits = p_symbol_engine_active_dealt_playing->playersactivebits();

	// check if all balances are known (indicates stability of info passed to DLL)
	bool balance_stability = true;
	for (int i=0; i<k_max_number_of_players; i++)	{
		if (p_table_state->_players[i].HasAnyCards()
        && (_m_holdem_state[(_m_ndx)&0xff].m_player[i].m_balance_known != 1)
        && ((activebits & (1 << i)) != 0)) {
			balance_stability = false;
			break;
		}
	}

	bool card_stability = true;
	if (_m_holdem_state[(_m_ndx)&0xff].m_cards[0] != CARD_NOCARD
		|| _m_holdem_state[(_m_ndx)&0xff].m_cards[1] != CARD_NOCARD
		|| _m_holdem_state[(_m_ndx)&0xff].m_cards[2] != CARD_NOCARD
		|| _m_holdem_state[(_m_ndx)&0xff].m_cards[3] != CARD_NOCARD)
	{
		if (_m_holdem_state[(_m_ndx)&0xff].m_cards[0] == CARD_NOCARD 
			|| _m_holdem_state[(_m_ndx)&0xff].m_cards[1] == CARD_NOCARD 
			|| _m_holdem_state[(_m_ndx)&0xff].m_cards[2] == CARD_NOCARD)
		{
			card_stability = false;
		}
	}
	if (_m_holdem_state[(_m_ndx)&0xff].m_cards[4] != CARD_NOCARD)
	{
		if (_m_holdem_state[(_m_ndx)&0xff].m_cards[0] == CARD_NOCARD 
			|| _m_holdem_state[(_m_ndx)&0xff].m_cards[1] == CARD_NOCARD 
			|| _m_holdem_state[(_m_ndx)&0xff].m_cards[2] == CARD_NOCARD
			|| _m_holdem_state[(_m_ndx)&0xff].m_cards[3] == CARD_NOCARD)
		{
			card_stability = false;
		}
	}

	// only process further if _safe_to_process_state==true, userchair is identified (br!=0),
	// and m_balance_known is true for all players with cards, card_stability is true
	// and I am actually in the hand.
	return ((betround >= k_betround_preflop 
		&& _safe_to_process_state 
		&& balance_stability 
		&& card_stability) 
		|| p_handreset_detector->IsHandreset());
}

void CGameState::ProcessStateEngine(const SHoldemState *pstate, const bool pstate_changed)
{
	double  sym_sblind = p_symbol_engine_tablelimits->sblind();
	double  sym_bblind = p_symbol_engine_tablelimits->bblind();
	int     from_chair = 0, to_chair = 0;
	int     betround = p_betround_calculator->betround();
	int     sym_userchair = p_symbol_engine_userchair->userchair();
	bool    sym_ismyturn = p_symbol_engine_autoplayer->ismyturn();
	double  sym_balance = p_table_state->User()->_balance;
	CString sym_handnumber = p_handreset_detector->GetHandNumber();

	_m_holdem_state[ (++_m_ndx)&0xff ] = *pstate;

	// Only start processing state info, if we have one solid frame recorded (many calcs require _m_ndx-1)
	if (_safe_to_process_state == false && _m_ndx>1)
		_safe_to_process_state = true;

	if (CGameState::ProcessThisFrame ())
	{

		// Check for end of hand situation
		for (int i=0; i<k_max_number_of_players; i++)
		{
			int index_normalized = i%k_max_number_of_players;
			// if new card fronts have appeared, then players are showing down, and its the end of the hand
			if ( _m_holdem_state[(_m_ndx-1)&0xff].m_player[index_normalized].m_cards[0]   == CARD_BACK   // was card backs last scrape
					&& _m_holdem_state[(_m_ndx-1)&0xff].m_player[index_normalized].m_cards[1] == CARD_BACK   // was card backs last scrape
					&& _m_holdem_state[(_m_ndx)&0xff].m_player[index_normalized].m_cards[0]   != CARD_BACK   // is not card backs this scrape
					&& _m_holdem_state[(_m_ndx)&0xff].m_player[index_normalized].m_cards[1]   != CARD_BACK   // is not card backs this scrape
					&& _m_holdem_state[(_m_ndx)&0xff].m_player[index_normalized].m_cards[0]   != CARD_NOCARD // is not 'no cards' this scrape
					&& _m_holdem_state[(_m_ndx)&0xff].m_player[index_normalized].m_cards[1]   != CARD_NOCARD // is not 'no cards' this scrape
					&& i != sym_userchair                                   												         // it's not me
					&& _end_of_hand==false )
			{
				_end_of_hand = true;
                WriteSummaryHeading();
				write_log(k_always_log_basic_information, ">>> SHOWDOWN\n");
			}
		}

		// save state information if one of the following is true:
		//   1. a new hand has started
		//	 2. it's my turn
		//   3. the betting round has changed
		//   4. we are in manual mode

		// 1. new hand has started
		if (_new_hand || _first_pass_capture)
		{

			// save the game state
			_m_game_state[ (++_m_game_ndx)&0xff ] = *pstate;

			//reset some vars
			_first_pass_capture = false;
			_oh_br_last=1;
			_pot_raised = false;
			_pf_limpers_n = 0;
			_my_first_action_this_round = true;
			_my_first_action_this_hand = true;
			_small_blind_posted = false;
			_big_blind_posted = false;
			_bets_last = 0.0;
			_end_of_hand = false;
			for (int i=0; i<k_max_number_of_players; i++)
			{
				for (int j=0; j<k_number_of_betrounds; j++)
				{
					for (int k=0; k<w_num_action_types; k++)
						_chair_actions[i][j][k] = w_noaction;
				}
			}
      WriteSummaryHeading();
			write_log(k_always_log_basic_information, ">>> New hand %.0s\n", sym_handnumber);
		}

		// first time to act in the hand//
		if (sym_ismyturn && _my_first_action_this_hand == true)
		{
			_my_first_action_this_hand = false;
		}

		// 2. it's my turn
		if ((sym_ismyturn && pstate_changed) 
			|| (sym_ismyturn && _my_first_action_this_round == true) 
			|| (((betround == k_betround_preflop && _m_holdem_state[(_m_ndx)&0xff].m_player[sym_userchair].m_currentbet == sym_sblind) 
			|| (betround == k_betround_preflop 
				&& _m_holdem_state[(_m_ndx)&0xff].m_player[sym_userchair].m_currentbet == sym_bblind)) 
				&& _my_first_action_this_round == true))
		{
			// save the game state
			_m_game_state[ (++_m_game_ndx)&0xff ] = *pstate;

			//reset some vars
			if (_m_holdem_state[(_m_ndx)&0xff].m_player[sym_userchair].m_cards[0] != 0 &&
				_m_holdem_state[(_m_ndx)&0xff].m_player[sym_userchair].m_cards[1] != 0)
			{
				_process_game_state = true;
                WriteSummaryHeading();
				write_log(k_always_log_basic_information, ">>> My turn, betround=%d\n", betround);
			}
			else
			{
				_process_game_state = false;
			}
		}

		// 3. the betting round has changed
		if (betround > _oh_br_last)
		{
			// save the game state
			_m_game_state[ (++_m_game_ndx)&0xff ] = *pstate;

			//reset some vars
			_oh_br_last = betround;
			_pot_raised = false;
			_my_first_action_this_round = true;
			_bets_last = 0.0;

			//write_log(">>> Betting round: %d\n", (int) betround);
		}

    // If I am not even seated then iterate from potential SB to dealer
    if (sym_userchair < 0) {
      from_chair = pstate->m_dealer_chair+1;
      to_chair = pstate->m_dealer_chair;
    }
		// it's my turn, so I need to figure out what everyone did before me
		else if (_process_game_state)
		{
			_process_game_state = false;

			// figure out how to iterate through the chairs
			// if it is my first action this round
			if (_my_first_action_this_round == true)
			{
				_my_first_action_this_round=false;

				// if i am the sb, then iterate just mychair
				if (betround == k_betround_preflop 
					&& _m_game_state[(_m_game_ndx)&0xff].m_player[sym_userchair].m_currentbet == sym_sblind)
				{
					from_chair = sym_userchair;
					to_chair = sym_userchair-1+k_max_number_of_players;
				}

				// if i am the bb, then iterate from dlr+1 to mychair
				else if (betround == k_betround_preflop && _m_game_state[(_m_game_ndx)&0xff].m_player[sym_userchair].m_currentbet == sym_bblind)
				{
					from_chair = _m_game_state[(_m_game_ndx)&0xff].m_dealer_chair+1;
					to_chair = sym_userchair-1+k_max_number_of_players;
				}

				// if the dlr chair < mychair, then iterate from dlr+1 to mychair-1
				else if (_m_game_state[(_m_game_ndx)&0xff].m_dealer_chair < sym_userchair)
				{
					from_chair = _m_game_state[(_m_game_ndx)&0xff].m_dealer_chair+1;
					to_chair = sym_userchair-1;
				}

				// if the dlr chair >= mychair, then iterate from dlr+1 to mychair-1+10 (to loop around)
				else
				{
					from_chair = _m_game_state[(_m_game_ndx)&0xff].m_dealer_chair+1;
					to_chair = sym_userchair-1+k_max_number_of_players;
				}

			}
			// if it is not my first action this round, then iterate from mychair+1 to mychair-1+10
			else
			{
				from_chair = sym_userchair;
				to_chair = sym_userchair-1+k_max_number_of_players;
			}

			// now iterate through the chairs and see what everybody did
			for (int i = from_chair; i <= to_chair; i++)
			{
				int index_normalized = i%k_max_number_of_players;
				assert(index_normalized >= 0);
				assert(index_normalized < k_max_number_of_players);
				// if the currentbet for the chair is the sb and the last bet was zero and br==1
				// and the player has cards, then we know the chair ***POSTED THE SMALL BLIND***
				if (_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet == sym_sblind 
					&& _bets_last==0 
					&& _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[0]!=0 
					&& _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[1]!=0 
					&& _m_game_state[(_m_game_ndx)&0xff].m_player[i%k_max_number_of_players].m_cards[0]!=0 
					&& _m_game_state[(_m_game_ndx)&0xff].m_player[i%k_max_number_of_players].m_cards[1]!=0 
					&& betround == k_betround_preflop)
				{
					_chair_actions[index_normalized][betround-1][w_posted_sb] = true;
					_bets_last = _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet;
					write_log(k_always_log_basic_information, ">>> Chair %d (%s) posted the sb: $%.2f\n", index_normalized,
							  _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_name,
							  _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet);
				}

				// if the currentbet for the chair is the bb and the last bet was the sb and br==1
				// and the player has cards, then we know the chair ***POSTED THE BIG BLIND***
				else if (_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet == sym_bblind 
					&& _bets_last == sym_sblind 
					&& _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[0]!=0 
					&& _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[1]!=0 
					&& betround == k_betround_preflop)
				{
					_chair_actions[index_normalized][betround-1][w_posted_bb] = true;
					_bets_last = _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet;
					write_log(k_always_log_basic_information, ">>> Chair %d (%s) posted the bb: $%.2f\n", index_normalized,
							  _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_name,
							  _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet);
				}

				// if the currentbet for the chair is greater than the last bet and it's not the end of the hand,
				// and the player has cards, then we know the chair ***RAISED or RE-RAISED***
				else if (_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet > _bets_last &&
						 _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[0]!=0 &&
						 _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[1]!=0 &&
						 !_end_of_hand)
				{
					_bets_last = _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet;
					if (_pot_raised == false)
					{
						_chair_actions[index_normalized][betround-1][w_raised] = true;
						_pot_raised = true;
						write_log(k_always_log_basic_information, ">>> Chair %d (%s) raised to $%.2f\n", index_normalized,
							_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_name,
							_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet);
					}
					else
					{
						_chair_actions[index_normalized][betround-1][w_reraised] = true;
						write_log(k_always_log_basic_information, ">>> Chair %d (%s) re-raised to $%.2f\n", index_normalized,
							_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_name,
							_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet);
					}
				}

				// if the currentbet for the chair is not zero and is <= the last bet and it's not the end of the hand,
				// and the player has cards, then we know the chair ***CALLED***
				else if (_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet != 0 &&
						 _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet <= _bets_last &&
						 _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[0]!=0 &&
						 _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[1]!=0 &&
						 !_end_of_hand)
				{
					_chair_actions[index_normalized][betround-1][w_called] = true;
					if (_pot_raised == false)
					{
						_pf_limpers_n += 1;
					}
					write_log(k_always_log_basic_information, ">>> Chair %d (%s) called\n", index_normalized, _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_name);
				}

				// if cards have disappeared, and they were card backs last scrape,
				// then we know the chair has ***FOLDED***
				// unless i=userchair, in which case they could be card fronts last scrape
				// in any case, if the balance in front of the chair has increased, it is not a fold, but a holdover from showdown
				else if ( ( _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[0]==0 &&			// no cards this scrape
							_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[1]==0 &&			// no cards this scrape
							_m_game_state[(_m_game_ndx-1)&0xff].m_player[index_normalized].m_cards[0]==255 &&		// card backs last scrape
							_m_game_state[(_m_game_ndx-1)&0xff].m_player[index_normalized].m_cards[1]==255 &&		// card backs last scrape
							_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_balance==
							_m_game_state[(_m_game_ndx-1)&0xff].m_player[index_normalized].m_balance)
						  ||
						  ( index_normalized == sym_userchair &&												// this is my chair
							_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[0]==0 &&			// no cards this scrape
							_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[1]==0 &&			// no cards this scrape
							_m_game_state[(_m_game_ndx-1)&0xff].m_player[index_normalized].m_cards[0]!=0 &&		// card fronts/backs last scrape
							_m_game_state[(_m_game_ndx-1)&0xff].m_player[index_normalized].m_cards[1]!=0 &&		// card fronts/backs last scrape
							_m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_balance==
							_m_game_state[(_m_game_ndx-1)&0xff].m_player[index_normalized].m_balance) )
				{
					_chair_actions[index_normalized][betround-1][w_folded] = true;
					write_log(k_always_log_basic_information, ">>> Chair %d (%s) folded\n", index_normalized,
							  _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_name);
				}

				// if br!=1 and the player's current bet is zero, and that player has cards in front of them
				// then we know the chair has ***CHECKED***
				else if (betround!=k_betround_preflop 
					&& _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_currentbet == 0 
					&& _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[0]!=0 
					&& _m_game_state[(_m_game_ndx)&0xff].m_player[index_normalized].m_cards[1]!=0)
				{
					_chair_actions[index_normalized][betround-1][w_checked] = true;

					write_log(k_always_log_basic_information, ">>> Chair %d (%s) checked\n", index_normalized, _m_holdem_state[(_m_ndx)&0xff].m_player[index_normalized].m_name);
				}
			}  // end of "for (int i = from_chair; i <= to_chair; i++)"
		} // end of "if (br != 0 &&..."
	} // end of "if (_process_game_state)"
}

void CGameState::ProcessFtrEngine(const SHoldemState *pstate)
{
	double			sym_elapsed         = p_symbol_engine_time->elapsed();
	double			sym_nbetsround1     = p_symbol_engine_history->nbetsround(k_betround_preflop);
	int				sym_nplayersdealt   = p_symbol_engine_active_dealt_playing->nplayersdealt();
	int				betround            = p_betround_calculator->betround();
	int				ncommoncardsknown   = p_symbol_engine_cards->ncommoncardsknown();
	int				sym_nplayersplaying = p_symbol_engine_active_dealt_playing->nplayersplaying();

	// if a new hand has started setup the next element in the ftr tracker array
	if (pstate->m_dealer_chair != _ftr_dealer_chair_last)
	{
		_m_ftr_ndx++;
		_m_ftr[_m_ftr_ndx&0xff].elapsed_time = sym_elapsed;
		_m_ftr[_m_ftr_ndx&0xff].n_pl_dealt = 0;
		_m_ftr[_m_ftr_ndx&0xff].n_pl_saw_flop = 0;
		_m_ftr[_m_ftr_ndx&0xff].n_pl_saw_turn = 0;
		_m_ftr[_m_ftr_ndx&0xff].n_pl_saw_river = 0;
		_m_ftr[_m_ftr_ndx&0xff].n_bets_preflop = 0;
		_ftr_nplayersdealt_last=0;
		_ftr_ncommoncardsknown_last=0;
		_ftr_dealer_chair_last=pstate->m_dealer_chair;
	}

	// if nplayersdealt has incremented and it is br1, update the ftr tracker stats
	if (sym_nplayersdealt != _ftr_nplayersdealt_last 
		&& sym_nplayersdealt > _ftr_nplayersdealt_last 
		&& betround==k_betround_preflop)
	{
		_m_ftr[_m_ftr_ndx&0xff].n_pl_dealt = sym_nplayersdealt;
		_ftr_nplayersdealt_last = sym_nplayersdealt;
	}

	// if the betting round has changed update the ftr tracker stats
	if (ncommoncardsknown > _ftr_ncommoncardsknown_last)
	{
		if (ncommoncardsknown==3 && _m_ftr[_m_ftr_ndx&0xff].n_pl_saw_flop==0)
		{
			_m_ftr[_m_ftr_ndx&0xff].n_pl_saw_flop = sym_nplayersplaying;
			_m_ftr[_m_ftr_ndx&0xff].n_bets_preflop = sym_nbetsround1;
		}

		if (ncommoncardsknown==4 && _m_ftr[_m_ftr_ndx&0xff].n_pl_saw_turn==0)
		{
			_m_ftr[_m_ftr_ndx&0xff].n_pl_saw_turn = sym_nplayersplaying;
		}

		if (ncommoncardsknown==5 && _m_ftr[_m_ftr_ndx&0xff].n_pl_saw_river==0)
		{
			_m_ftr[_m_ftr_ndx&0xff].n_pl_saw_river = sym_nplayersplaying;
		}

		_ftr_ncommoncardsknown_last = ncommoncardsknown;
	}
}

void CGameState::DumpState(void)
{
	write_log(preferences.debug_alltherest(), "[CGameState] m_ndx: %d\n", _m_ndx);
	write_log(preferences.debug_alltherest(), "[CGameState] _title: %s\n", _m_holdem_state[(_m_ndx)&0xff].m_title);
	write_log(preferences.debug_alltherest(), "[CGameState] _pot: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", _m_holdem_state[(_m_ndx)&0xff].m_pot[0], _m_holdem_state[(_m_ndx)&0xff].m_pot[1],
			  _m_holdem_state[(_m_ndx)&0xff].m_pot[2], _m_holdem_state[(_m_ndx)&0xff].m_pot[3], _m_holdem_state[(_m_ndx)&0xff].m_pot[4],
			  _m_holdem_state[(_m_ndx)&0xff].m_pot[5], _m_holdem_state[(_m_ndx)&0xff].m_pot[6], _m_holdem_state[(_m_ndx)&0xff].m_pot[7],
			  _m_holdem_state[(_m_ndx)&0xff].m_pot[8], _m_holdem_state[(_m_ndx)&0xff].m_pot[9]);
	write_log(preferences.debug_alltherest(), "[CGameState] _cards: %d %d %d %d %d\n", _m_holdem_state[(_m_ndx)&0xff].m_cards[0], _m_holdem_state[(_m_ndx)&0xff].m_cards[1],
			  _m_holdem_state[(_m_ndx)&0xff].m_cards[2], _m_holdem_state[(_m_ndx)&0xff].m_cards[3], _m_holdem_state[(_m_ndx)&0xff].m_cards[4]);
	write_log(preferences.debug_alltherest(), "[CGameState] _is_playing: %d\n", _m_holdem_state[(_m_ndx)&0xff].m_is_playing);
	write_log(preferences.debug_alltherest(), "[CGameState] _is_posting: %d\n", _m_holdem_state[(_m_ndx)&0xff].m_is_posting);
	write_log(preferences.debug_alltherest(), "[CGameState] _dealer_chair: %d\n", _m_holdem_state[(_m_ndx)&0xff].m_dealer_chair);
	for (int i=0; i<k_max_number_of_players; i++) {
		write_log(preferences.debug_alltherest(), "[CGameState] _player[%d].m_name:%s  ", i, _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_name);
		write_log(preferences.debug_alltherest(), "[CGameState] _balance:%.2f  ", _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_balance);
		write_log(preferences.debug_alltherest(), "[CGameState] _currentbet:%.2f  ", _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_currentbet);
		write_log(preferences.debug_alltherest(), "[CGameState] _cards:%d/%d  ", _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_cards[0],
				  _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_cards[1]);
		write_log(preferences.debug_alltherest(), "[CGameState] _name_known:%d  ", _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_name_known);
		write_log(preferences.debug_alltherest(), "[CGameState] _balance_known:%d\n", _m_holdem_state[(_m_ndx)&0xff].m_player[i].m_balance_known);
	}
}

void CGameState::WriteSummaryHeading() {
  write_log_nostamp(1, "***** History (might be not accurate) ***********************\n"); 
}