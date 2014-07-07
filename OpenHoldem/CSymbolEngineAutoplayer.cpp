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
#include "CSymbolEngineAutoplayer.h"

#include "CAutoconnector.h"
#include "CAutoplayerFunctions.h"
#include "CCasinoInterface.h"
#include "CGameState.h"
#include "CIteratorThread.h"
#include "CPreferences.h"
#include "CScraper.h"  
#include "CScraperAccess.h"
#include "CStableFramesCounter.h"
#include "CStringMatch.h"
#include "CSymbolEngineUserchair.h"
#include "CTableState.h"
#include "MagicNumbers.h"
#include "NumericalFunctions.h"

CSymbolEngineAutoplayer *p_symbol_engine_autoplayer = NULL;

CSymbolEngineAutoplayer::CSymbolEngineAutoplayer()
{
	// The values of some symbol-engines depend on other engines.
	// As the engines get later called in the order of initialization
	// we assure correct ordering by checking if they are initialized.
	assert(p_symbol_engine_tablelimits != NULL);
}

CSymbolEngineAutoplayer::~CSymbolEngineAutoplayer()
{}

void CSymbolEngineAutoplayer::InitOnStartup()
{
	_myturnbits      = 0;
	_issittingin     = false;
	_isautopost      = false;
	_isfinalanswer   = false;
}

void CSymbolEngineAutoplayer::ResetOnConnection()
{
	_myturnbits      = 0;
	_issittingin     = false;
	_isautopost      = false;
	_isfinalanswer   = false;
	_last_myturnbits = 0;
	DetectSpecialConnectionLikeBringAndManualMode();
}

void CSymbolEngineAutoplayer::ResetOnHandreset()
{
	_isfinaltable  = false;
}

void CSymbolEngineAutoplayer::ResetOnNewRound()
{}

void CSymbolEngineAutoplayer::ResetOnMyTurn()
{}

void CSymbolEngineAutoplayer::ResetOnHeartbeat()
{
	_last_myturnbits = _myturnbits;
	_myturnbits      = 0;
	_issittingin     = false;
	_isautopost      = false;
	_isfinalanswer   = false;
	CalculateMyTurnBits();
	CalculateSitInState();
	CalculateFinalAnswer();
}

void CSymbolEngineAutoplayer::CalculateMyTurnBits()
{
	write_log(preferences.debug_symbolengine(), "[CSymbolEngineAutoplayer] myturnbits reset: %i\n", _myturnbits);
	for (int i=0; i<k_max_number_of_buttons; i++)
	{
		if (p_scraper->GetButtonState(i))
		{
			CString button_label = p_scraper->button_label(i);

			if (p_string_match->IsStringFold(button_label))
			{
				_myturnbits |= (1<<0);
			}
			else if (p_string_match->IsStringCall(button_label))
			{
				_myturnbits |= (1<<1);
			}
			else if (p_string_match->IsStringRaise(button_label) || button_label.MakeLower() == "swag")
			{
				_myturnbits |= (1<<2);
			}
			else if (p_string_match->IsStringCheck(button_label))
			{
				_myturnbits |= (1<<4);
			}
			else if (p_string_match->IsStringAllin(button_label))
			{
				_myturnbits |= (1<<3);
			}
			else if (p_string_match->IsStringAutopost(button_label))
			{
				_isautopost = true;
			}
		}
	}
	write_log(preferences.debug_symbolengine(), "[CSymbolEngineAutoplayer] myturnbits now: %i\n", _myturnbits);
}

void CSymbolEngineAutoplayer::CalculateSitInState() {
  for (int i=0; i<k_max_number_of_buttons; i++) {
    if (p_string_match->IsStringSitin(p_scraper->button_label(i))) {
	  // Sitin-button found
      // We are sitting in if that button can NOT be clicked
	  _issittingin = !p_scraper->GetButtonState(i);
	  return;
    } else if (p_string_match->IsStringSitout(p_scraper->button_label(i))) {
	  // Sitout-button found
      // We are sitting in if that button CAN be clicked
	  _issittingin = (p_scraper->GetButtonState(i));
	  return;
    }
  }
}

void CSymbolEngineAutoplayer::DetectSpecialConnectionLikeBringAndManualMode()
{
	_isbring  = false;
	_ismanual = false;
	const int k_max_length_of_classname = 50;
	char classname[k_max_length_of_classname] = "";

	GetClassName(p_autoconnector->attached_hwnd(), classname, k_max_length_of_classname);
	if (strcmp(classname, "BRING")==0)
	{
		_isbring = true;																
	}
	else if (strcmp(classname, "OpenHoldemManualMode")==0)
	{
		_ismanual = true;
	}
}

void CSymbolEngineAutoplayer::CalculateFinalAnswer()
{
	// [IMPERFECT CODE] Updates stable-frames-counter as a side-effect
	// and should therefore only get called once per heartbeat.

	_isfinalanswer = true;
	// check factors that affect isFinalAnswer status
	if (p_iterator_thread->IteratorThreadWorking())
	{
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Not Final Answer because iterator_thread still running\n");
		_isfinalanswer = false;
	}

	// Change from only requiring one visible button (OpenHoldem 2008-04-03)
	if (p_casino_interface->NumberOfVisibleAutoplayerButtons() < k_min_buttons_needed_for_my_turn)
	{
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Not Final Answer because too few buttons visible\n");
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Buttons visible: %i\n", p_casino_interface->NumberOfVisibleAutoplayerButtons());
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Either not your turn or problem with the tablemap\n");
		_isfinalanswer = false;
	}

	// if we are not playing (occluded?) 2008-03-25 Matrix
	if (!p_table_state->_players[USER_CHAIR].HasKnownCards())
	{
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Not Final Answer because the user is \"not playing\"\n");
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Chair %d (locked) has no cards\n", p_symbol_engine_userchair->userchair());
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Possibly a tablemap-problem\n");
		_isfinalanswer = false;
	}

	//  Avoiding unnecessary calls to p_stableframescounter->UpdateNumberOfStableFrames(),
	if (_isfinalanswer)
	{
		p_stableframescounter->UpdateNumberOfStableFrames();
	}

	write_log(preferences.debug_autoplayer(), "[AutoPlayer] Number of stable frames: % d\n", p_stableframescounter->NumberOfStableFrames());
	// Scale f$delay to a number of scrapes and avoid division by 0 and negative values
	unsigned int additional_frames_to_wait = (preferences.scrape_delay() > 0 && p_autoplayer_functions->f$delay() > 0 ? (p_autoplayer_functions->f$delay()/preferences.scrape_delay()) : 0);

	// If we don't have enough stable frames, or have not waited f$delay milliseconds, then return.
	if (p_stableframescounter->NumberOfStableFrames() < preferences.frame_delay() + additional_frames_to_wait)
	{
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Not Final Answer because we don't have enough stable frames, or have not waited f$delay (=%d ms)\n", (int)p_autoplayer_functions->f$delay());
		_isfinalanswer = false;
	}

	// If the game state processor didn't process this frame, then we should not act.
	if (!p_game_state->ProcessThisFrame ())
	{
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Not Final Answer because game state processor didn't process this frame\n");
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Most common reason: missing balance-stability or card-stability.\n");
		_isfinalanswer = false;
	}
}

CString CSymbolEngineAutoplayer::GetFCKRAString()
{
	// Buttons visible (Fold, Call, Check, Raise, Allin)
	CString fckra_seen;
	fckra_seen.Format("%s%s%s%s%s",
		IsBitSet(_myturnbits, 0) ? "F" : ".",
		IsBitSet(_myturnbits, 1) ? "C" : ".",
		// Check button out of order to stay consistent
		// with button order in manual mode.
		IsBitSet(_myturnbits, 2) ? "K" : ".",
		IsBitSet(_myturnbits, 3) ? "R" : ".",
		IsBitSet(_myturnbits, 4) ? "A" : ".");
	return fckra_seen;
}

bool CSymbolEngineAutoplayer::IsFirstHeartbeatOfMyTurn()
{
	return(ismyturn()
		&& (_last_myturnbits == 0));
}

bool CSymbolEngineAutoplayer::EvaluateSymbol(const char *name, double *result, bool log /* = false */)
{
  FAST_EXIT_ON_OPENPPL_SYMBOLS(name);
	if (memcmp(name, "is", 2)==0)
	{
		if (memcmp(name, "isfinaltable", 12)==0 && strlen(name)==12)	
		{
			*result = isfinaltable();
		}
		else if (memcmp(name, "ismyturn", 8)==0 && strlen(name)==8)		
		{
			*result = ismyturn();
		}
		else if (memcmp(name, "issittingin", 11)==0 && strlen(name)==11)	
		{
			*result = issittingin();
		}
		else if (memcmp(name, "issittingout", 12)==0 && strlen(name)==12)
		{
			*result = issittingout();
		}
		else if (memcmp(name, "isautopost", 10)==0 && strlen(name)==10)	
		{
			*result = isautopost();
		}
		else if (memcmp(name, "isfinalanswer", 13)==0 && strlen(name)==13)	
		{
			*result = isfinalanswer();
		}
		else
		{
			// Invalid symbol
			return false;
		}
		// Valid symbol
		return true;
	}
	else if (memcmp(name, "myturnbits", 10)==0 && strlen(name)==10)
	{
		*result = myturnbits();
		// Valid symbol
		return true;
	}

	// Symbol of a different symbol-engine
	return false;
}

CString CSymbolEngineAutoplayer::SymbolsProvided() {
  return "isfinaltable ismyturn issittingin issittingout isautopost "
    "isfinalanswer myturnbits ";
}