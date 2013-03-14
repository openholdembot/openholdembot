/* TODO: 
* prevaction
* p_stableframescounter->ResetOnAutoplayerAction();
* delay
* check for loss of swag-focus: first sleep, then check, then act,
  NOT the other way: http://www.maxinmontreal.com/forums/viewtopic.php?f=120&t=14791
*/

#include "StdAfx.h"
#include "CAutoplayer.h"

#include <complex>
#include "BringKeyboard.h"
#include "CAutoconnector.h"
#include "CAutoplayerFunctions.h"
#include "CCasinoInterface.h"
#include "CFlagsToolbar.h"
#include "CGameState.h"
#include "CGrammar.h"
#include "CHeartbeatThread.h"
#include "CIteratorThread.h"
#include "CPreferences.h"
#include "CRebuyManagement.h"
#include "CReplayFrame.h"
#include "CScraper.h"
#include "CScraperAccess.h"
#include "CSymbols.h"
#include "CSymbolEngineAutoplayer.h"
#include "CSymbolEngineChipAmounts.h"
#include "CSymbolEngineUserchair.h"
#include "MainFrm.h"
#include "OpenHoldem.h"
#include "PokerChat.hpp"


CAutoplayer	*p_autoplayer = NULL;


CAutoplayer::CAutoplayer(BOOL bInitiallyOwn, LPCTSTR lpszName) : _mutex(bInitiallyOwn, lpszName)
{
	ASSERT(_mutex.m_hObject != NULL); 

	// Autoplayer is not enabled at startup.
	// We can't call set_autoplayer_engaged() here,
	// because the toolbar does not yet exist,
	// so we can't set the autoplayer-button.
	// However the toolbar is guaranteed to initialize correctly later.
	_autoplayer_engaged = false;
	action_sequence_needs_to_be_finished = false;
}


CAutoplayer::~CAutoplayer(void) 
{
	FinishActionSequenceIfNecessary();
}


bool CAutoplayer::PrepareActionSequence()
{
	// This function should be called at the beginning of 
	// ExecutePrimaryFunctions and ExecuteSecondaryFunctions
	// which bot will start exactly one action-sequence.
	//
	// At the end of an action sequence FinishAction() has to be called
	// to restore the mouse-position and release the mutex again.
	if (!_mutex.Lock(500))
	{
		return false;
	}
	// Getting the cursor position has to be done AFTER  we got the mutex,
	// otherwise it could happen that other applications move the mouse
	// while we wait, leading to funny jumps when we "clean up".
	// http://www.maxinmontreal.com/forums/viewtopic.php?f=111&t=15324
	GetCursorPos(&cursor_position);
	window_with_focus = GetFocus();
	// We got the mutex and everything is prepared.
	// We now assume an action-sequence will be executed.
	// This makes cleanup simpler, as we now can handle it once,
	// instead of everywhere where an action can happen.
	action_sequence_needs_to_be_finished = true;
	return true;
}


void CAutoplayer::FinishActionSequenceIfNecessary()
{
	if (action_sequence_needs_to_be_finished)
	{
		// Restoring the original state has to be done in reversed order
		SetFocus(window_with_focus);
		SetCursorPos(cursor_position.x, cursor_position.y);
		_mutex.Unlock();
		action_sequence_needs_to_be_finished = false;
	}
}


bool CAutoplayer::TimeToHandleSecondaryFormulas()
{
	// Disabled (N-1) out of N heartbeats (3 out of 4 seconds)
	// to avoid multiple fast clicking on the sitin / sitout-button.
	// Contrary to the old f$play-function we use a heartbeat-counter 
	// for that logic, as with a small scrape-delay it was
	// still possible to act multiple times within the same second.

	// Scrape_delay() should always be > 0, there's a check in the GUI.
	assert(prefs.scrape_delay() > 0);
	int hearbeats_to_pause = 4 / prefs.scrape_delay();
	if  (hearbeats_to_pause < 1)
	{
		hearbeats_to_pause = 1;
	}
	return ((p_heartbeat_thread->heartbeat_counter() % hearbeats_to_pause) == 0);
}


bool CAutoplayer::DoBetPot(void)
{
	bool success = false;
	// Start with 2 * potsize, continue with lower betsizes, finally 1/4 pot
	for (int i=k_autoplayer_function_betpot_2_1; i<=k_autoplayer_function_betpot_1_4; i++)
	{
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(i))
		{
			write_log(prefs.debug_autoplayer(), "[AutoPlayer] Function %s true.\n", 
				k_autoplayer_functionname[i]);
			if (!PrepareActionSequence())
			{
				return false;
			}
			if (p_tablemap->betpotmethod() == BETPOT_RAISE)
			{
				success = p_casino_interface->ClickButtonSequence(i, k_autoplayer_function_raise, /*betpot_delay* !! */ 400);
			}
			else 
			{
				// Default: click only betpot
				success = p_casino_interface->ClickButton(i);				
			}
		}
		if (success)
		{
			return true;
		}
		// Else continue trying with the next betpot function
	}
	// We didn't click any betpot-button
	return false;
}

bool CAutoplayer::AnyPrimaryFormulaTrue()
{
	for (int i=k_autoplayer_function_allin; i<=k_autoplayer_function_call; i++)
	{
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(i))
		{
			return true;
		}
	}
	return false;
}

bool CAutoplayer::AnySecondaryFormulaTrue()
{
	for (int i=k_autoplayer_function_prefold; i<=k_autoplayer_function_chat; i++)
	{
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(i))
		{
			return true;
		}
	}
	return false;
}

bool CAutoplayer::ExecutePrimaryFormulas() 
{
	write_log(prefs.debug_autoplayer(), "[AutoPlayer] ExecutePrimaryFormulas()\n");
	// Precondition: my turn and isfinalanswer
	// So we have to take an action and are able to do so.
	// This function will ALWAYS try to click a button,
	// so we can handle the preparation once at the very beginning.
	if (!PrepareActionSequence())
	{
		return false;
	}
	if (p_autoplayer_functions->f$alli())
	{
		if (DoAllin())
		{
			return true;
		}
		// Else continue with swag and betpot
	}
	if (DoSwag())
	{
		return true;
	}
	if (DoBetPot())
	{
		return true;
	}
	return ExecuteRaiseCallCheckFold();
}

bool CAutoplayer::ExecuteRaiseCallCheckFold()
{
	write_log(prefs.debug_autoplayer(), "[AutoPlayer] ExecuteRaiseCallCheckFold()\n");
	if (p_autoplayer_functions->f$rais())
	{
		if (p_casino_interface->ClickButton(k_autoplayer_function_raise))
		{
			p_symbols->RecordPrevAction(k_action_raise);
			write_logautoplay(ActionConstantNames(k_action_raise));
			return true;
		}
	}
	if (p_autoplayer_functions->f$call())
	{
		if (p_casino_interface->ClickButton(k_autoplayer_function_call))
		{
			p_symbols->RecordPrevAction(k_action_call);
			write_logautoplay(ActionConstantNames(k_action_call));
			return true;
		}
	}
	// Try to check
	if (p_casino_interface->ClickButton(k_autoplayer_function_check))
	{
		p_symbols->RecordPrevAction(k_action_check);
		write_logautoplay(ActionConstantNames(k_action_check));
		return true;
	}
	else
	{
		// Otherwise: fold
		p_symbols->RecordPrevAction(k_action_fold);
		write_logautoplay(ActionConstantNames(k_action_fold));
		return p_casino_interface->ClickButton(k_autoplayer_function_fold);
	}
}


bool CAutoplayer::ExecuteSecondaryFormulasIfNecessary()
{
	// !!! Once every N seconds
	if (!AnySecondaryFormulaTrue())
	{
		write_log(prefs.debug_autoplayer(), "[AutoPlayer] All secondary formulas false.\n");
		write_log(prefs.debug_autoplayer(), "[AutoPlayer] Nothing to do.\n");
		return false;
	}
	for (int i=k_autoplayer_function_prefold; i<=k_autoplayer_function_leave; i++)
	{
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(i))
		{
			if (p_casino_interface->ClickButton(i))
			{
				return true;
			}
		}
		// Close, rebuy and chat work require different treatment,
		// more than just clicking a simple region...
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(k_autoplayer_function_close))
		{
			// CloseWindow is "final".
			// We don't expect any further action after that
			// and can return immediatelly.
			return p_casino_interface->CloseWindow();
		}
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(k_autoplayer_function_rebuy))
		{
			// This requires an external script and some time.
			// No further actions here eihter, but immediate return.
			p_rebuymanagement->TryToRebuy();
			return true;
		}
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(k_autoplayer_function_chat))
		{
			return DoChat();
		}
	}
	return false;
}

#define ENT CSLock lock(m_critsec);
	
void CAutoplayer::set_autoplayer_engaged(const bool to_be_enabled_or_not) 
{ 
	ENT 
	_autoplayer_engaged = to_be_enabled_or_not; 
	// Set correct button state
	// We have to be careful, as during initialization the GUI does not yet exist.
	p_flags_toolbar->CheckButton(ID_MAIN_TOOLBAR_AUTOPLAYER, to_be_enabled_or_not);
}

#undef ENT


bool CAutoplayer::DoChat(void)
{
	if (!IsChatAllowed())
	{
		write_log(prefs.debug_autoplayer(), "[AutoPlayer] No chat, because chat turned off.\n");
		return false;
	}
	if ((p_autoplayer_functions->f$chat() == 0) || (_the_chat_message == NULL))
	{
		write_log(prefs.debug_autoplayer(), "[AutoPlayer] No chat, because no chat message.\n");
		return false;
	}

	// Converting the result of the $chat-function to a string.
	// Will be ignored, if we already have an unhandled chat message.
	RegisterChatMessage(p_autoplayer_functions->f$chat()); 
	return p_casino_interface->EnterChatMessage(CString(_the_chat_message));
}

bool CAutoplayer::DoAllin(void)
{
	bool success = false;
	write_log(prefs.debug_autoplayer(), "[AutoPlayer] Starting DoAllin...\n");

	int number_of_clicks = 1; // Default is: single click with the mouse
	if (p_tablemap->buttonclickmethod() == BUTTON_DOUBLECLICK)
	{
		number_of_clicks = 2;
	}

	// TM symbol allinmethod.
	//	0: swag the balance (default)
	//	1: click max (or allin), then raise
	//	2: click only max (or allin) [Spew and THF]
	//	3: use the slider
	if (p_tablemap->allinmethod() == 1)
	{
		// Clicking max (or allin) and then raise
		success = p_casino_interface->ClickButtonSequence(k_autoplayer_function_allin,
			k_autoplayer_function_raise, prefs.swag_delay_3());

		write_logautoplay(ActionConstantNames(k_action_allin));
	    p_heartbeat_thread->set_replay_recorded_this_turn(false);
		return success;
	}
	else  if (p_tablemap->allinmethod() == 2)
	{
		success = p_casino_interface->ClickButton(k_autoplayer_function_allin);

		write_logautoplay(ActionConstantNames(k_action_allin));
	    p_heartbeat_thread->set_replay_recorded_this_turn(false);
		return success;
	}
	else if (p_tablemap->allinmethod() == 3)
	{
		success = p_casino_interface->UseSliderForAllin();
		write_logautoplay(ActionConstantNames(k_action_allin));
	    p_heartbeat_thread->set_replay_recorded_this_turn(false);
		return success;
	}
	else
	{
		// Fourth case (default = 0): swagging the balance
		int userchair = p_symbol_engine_userchair->userchair();
		double betsize_for_allin = p_symbol_engine_chip_amounts->currentbet(userchair)
			+ p_symbol_engine_chip_amounts->balance(userchair); 
		return p_casino_interface->EnterBetsize(betsize_for_allin);
	}
}


void CAutoplayer::DoAutoplayer(void) 
{
	write_log(prefs.debug_autoplayer(), "[AutoPlayer] Starting Autoplayer cadence...\n");

	CheckBringKeyboard();

	p_scraper_access->GetNeccessaryTablemapObjects();
	/* [TODO] better log-file format !! 	
	write_log(prefs.debug_autoplayer(), "[AutoPlayer] Number of visible buttons: %d (%c%c%c%c%c)\n", 
		num_buttons_visible, 
		allin_option_available ? 'A' : '.',
		raise_button_available ? 'R' : '.',
		call_button_available  ? 'C' : '.',
		check_button_available ? 'K' : '.',
		fold_button_available  ? 'F' : '.');*/

	// Care about I86 regions first, because they are usually used 
	// to handle popups which occlude the table (unstable input)
	if (!HandleInterfacebuttonsI86())
	{
		// Care about sitin, sitout, leave, etc.
		p_autoplayer_functions->CalcSecondaryFormulas();
		if (!ExecuteSecondaryFormulasIfNecessary())	
		{
			write_log(prefs.debug_autoplayer(), "[AutoPlayer] No secondary formulas to be handled.\n");
			write_log(prefs.debug_autoplayer(), "[AutoPlayer] Going to evaluate primary formulas.\n");
			if(p_symbol_engine_autoplayer->isfinalanswer())
			{
				p_autoplayer_functions->CalcPrimaryFormulas();
				ExecutePrimaryFormulas();
			}
			else
			{
				write_log(prefs.debug_autoplayer(), "[AutoPlayer] No final answer, therefore not executing autoplayer-logic.\n");
			}
		}
	}
	FinishActionSequenceIfNecessary();
	write_log(prefs.debug_autoplayer(), "[AutoPlayer] ...ending Autoplayer cadence.\n");
}


bool CAutoplayer::DoSwag(void) 
{
	if (p_autoplayer_functions->f$betsize() > 0)
	{
		return p_casino_interface->EnterBetsize(p_autoplayer_functions->f$betsize());
	}
	write_log(prefs.debug_autoplayer(), "[AutoPlayer] Don't swag, because f$betsize evaluates to 0.\n");
	return false;
}


void CAutoplayer::DoPrefold(void) 
{
	if (p_autoplayer_functions->f$prefold() == 0)  
	{
		write_log(prefs.debug_autoplayer(), "[AutoPlayer] Don't prefold, because f$prefold evaluates to false.\n");
		return;
	}
	p_casino_interface->ClickButton(k_autoplayer_function_prefold);

	p_symbols->RecordPrevAction(k_action_fold);
	write_logautoplay(ActionConstantNames(k_action_fold));
	p_autoplayer_functions->CalcAutoTrace();
	write_log(prefs.debug_autoplayer(), "[AutoPlayer] Prefold executed.\n");
}


bool CAutoplayer::HandleInterfacebuttonsI86(void) 
{
	for (int i=0; i<k_max_number_of_i86X_buttons; i++)
	{
		if (p_casino_interface->ClickI86ButtonIfAvailable(i))
		{
			return true;
		}
	}
	write_log(prefs.debug_autoplayer(), "[AutoPlayer] No interface button (i86X) to be handled.\n");
	return false;
}
