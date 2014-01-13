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
#include "CSymbolEngineHistory.h"
#include "CSymbolEngineUserchair.h"
#include "MainFrm.h"
#include "OpenHoldem.h"
#include "PokerChat.hpp"
#include "StringFunctions.h"
#include "CMyMutex.h"

CAutoplayer	*p_autoplayer = NULL;


CAutoplayer::CAutoplayer(void) 
{
	// Autoplayer is not enabled at startup.
	// We can't call EngageAutoplayer() here,
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

void CAutoplayer::EngageAutoPlayerUponConnectionIfNeeded()
{
	if (p_autoconnector->IsConnected() && preferences.engage_autoplayer())
	{
		EngageAutoplayer(true);
	}
}

void CAutoplayer::PrepareActionSequence()
{
	// This function should be called at the beginning of 
	// ExecutePrimaryFunctions and ExecuteSecondaryFunctions
	// which bot will start exactly one action-sequence.
	//
	// At the end of an action sequence FinishAction() has to be called
	// to restore the mouse-position.
	//
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
}


void CAutoplayer::FinishActionSequenceIfNecessary()
{
	if (action_sequence_needs_to_be_finished)
	{
		// Restoring the original state has to be done in reversed order
		SetFocus(window_with_focus);
		SetCursorPos(cursor_position.x, cursor_position.y);
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
	assert(preferences.scrape_delay() > 0);
	int hearbeats_to_pause = 4 / preferences.scrape_delay();
	if  (hearbeats_to_pause < 1)
	{
 		hearbeats_to_pause = 1;
 	}
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] TimeToHandleSecondaryFormulas() heartbeats to pause: %i\n",
		hearbeats_to_pause);
	bool act_this_heartbeat = ((p_heartbeat_thread->heartbeat_counter() % hearbeats_to_pause) == 0);
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] TimeToHandleSecondaryFormulas() act_this_heartbeat: %s\n",
		Bool2CString(act_this_heartbeat));
	return act_this_heartbeat;
}


bool CAutoplayer::DoBetPot(void)
{
	bool success = false;
	// Start with 2 * potsize, continue with lower betsizes, finally 1/4 pot
	for (int i=k_autoplayer_function_betpot_2_1; i<=k_autoplayer_function_betpot_1_4; i++)
	{
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(i))
		{
			write_log(preferences.debug_autoplayer(), "[AutoPlayer] Function %s true.\n", 
				k_standard_function_names[i]);
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
			// Register the action
			// Treat betpot like swagging, i.e. raising a user-defined amount
			p_symbol_engine_history->RegisterAction(k_autoplayer_function_betsize);
			return true;
		}
		// Else continue trying with the next betpot function
	}
	// We didn't click any betpot-button
	return false;
}

bool CAutoplayer::AnyPrimaryFormulaTrue()
{ 
	for (int i=k_autoplayer_function_beep; i<=k_autoplayer_function_fold; ++i)
	{
		double function_result = p_autoplayer_functions->GetAutoplayerFunctionValue(i);
		if (i == k_autoplayer_function_betsize)
		{
			write_log(preferences.debug_autoplayer(), "[AutoPlayer] AnySecondaryFormulaTrue(): [%s]: %s\n",
				k_standard_function_names[i], Number2CString(function_result));
		}
		else
		{
			write_log(preferences.debug_autoplayer(), "[AutoPlayer] AnySecondaryFormulaTrue(): [%s]: %s\n",
				k_standard_function_names[i], Bool2CString(function_result));
		}
		if (function_result)
		{
			write_log(preferences.debug_autoplayer(), "[AutoPlayer] AnyPrimaryFormulaTrue(): yes\n");
			return true;
		}
	}
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] AnyPrimaryFormulaTrue(): no\n");
	return false;
}


bool CAutoplayer::AnySecondaryFormulaTrue()
{
	for (int i=k_standard_function_prefold; i<=k_standard_function_chat; ++i)
	{
		bool function_result = p_autoplayer_functions->GetAutoplayerFunctionValue(i);
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] AnySecondaryFormulaTrue(): [%s]: %s\n",
			k_standard_function_names[i], Bool2CString(function_result));
		if (function_result)
		{
			write_log(preferences.debug_autoplayer(), "[AutoPlayer] AnySecondaryFormulaTrue(): yes\n");
			return true;
		}
	}
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] AnySecondaryFormulaTrue(): no\n");
	return false;
}

bool CAutoplayer::ExecutePrimaryFormulasIfNecessary() 
{
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] ExecutePrimaryFormulasIfNecessary()\n");
	if (!AnyPrimaryFormulaTrue())
	{
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] No primary formula true. Nothing to do\n");
		return false;
	}
	// Execute beep (if necessary) independent of all other conditions (mutex, etc.)
	// and with autoplayer-actions.
	ExecuteBeep();

	assert(p_symbol_engine_autoplayer->isfinalanswer());
	assert(p_symbol_engine_autoplayer->ismyturn());
	// Precondition: my turn and isfinalanswer
	// So we have to take an action and are able to do so.
	// This function will ALWAYS try to click a button,
	// so we can handle the preparation once at the very beginning.
	CMyMutex mutex;

	if (!mutex.IsLocked())
	{
		return false;
	}

	PrepareActionSequence();

	if (p_autoplayer_functions->f$alli())
	{
		if (DoAllin())
		{
			return true;
		}
		// Else continue with swag and betpot
	}
	if (DoBetPot())
	{
		return true;
	}
	if (DoSwag())
	{
		return true;
	}
	return ExecuteRaiseCallCheckFold();
}

bool CAutoplayer::ExecuteRaiseCallCheckFold()
{
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] ExecuteRaiseCallCheckFold()\n");
	for (int i=k_autoplayer_function_raise; i<=k_autoplayer_function_fold; i++)
	{
		if (p_autoplayer_functions->autoplayer_function_values(i))
		{
			if (p_casino_interface->ClickButton(i))
			{
				write_logautoplay(ActionConstantNames(i));
				p_symbol_engine_history->RegisterAction(i);
				return true;
			}
		}
	}
	return false;
}

bool CAutoplayer::ExecuteBeep()
{
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] ExecuteBeep (if f$beep is true)\n");
	if (p_autoplayer_functions->autoplayer_function_values(k_autoplayer_function_beep))
	{
		// Pitch standard: 440 Hz, 1/2 second
		// http://en.wikipedia.org/wiki/A440_%28pitch_standard%29
		Beep(440, 500);
	}
	return false;
}

bool CAutoplayer::ExecuteSecondaryFormulasIfNecessary()
{
	if (!TimeToHandleSecondaryFormulas())
	{
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Not executing secondary formulas this heartbeat\n");
		return false;
	}
	if (!AnySecondaryFormulaTrue())
	{
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] All secondary formulas false.\n");
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Nothing to do.\n");
		return false;
	}
	for (int i=k_standard_function_prefold; i<=k_standard_function_chat; i++)
	{

		// Prefold, close, rebuy and chat work require different treatment,
		// more than just clicking a simple region...
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(k_standard_function_prefold))
		{
			// Prefold is technically more than a simple button-click,
			// because we need to create an autoplayer-trace afterwards.
			return DoPrefold();
		}
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(k_standard_function_close))
		{
			// CloseWindow is "final".
			// We don't expect any further action after that
			// and can return immediatelly.
			return p_casino_interface->CloseWindow();
		}
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(k_standard_function_rebuy))
		{
			// This requires an external script and some time.
			// No further actions here eihter, but immediate return.
			p_rebuymanagement->TryToRebuy();
			return true;
		}
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(k_standard_function_chat))
		{
			return DoChat();
		}
		// Otherwise: it is a simple button-click
		if (p_autoplayer_functions->GetAutoplayerFunctionValue(i))
		{
			if (p_casino_interface->ClickButton(i))
			{
				return true;
			}
		}
	}
	return false;
}

#define ENT CSLock lock(m_critsec);
	
void CAutoplayer::EngageAutoplayer(bool to_be_enabled_or_not) 
{ 
	ENT 
	// Set correct button state
	// We have to be careful, as during initialization the GUI does not yet exist.
	assert(p_flags_toolbar != NULL);
	p_flags_toolbar->CheckButton(ID_MAIN_TOOLBAR_AUTOPLAYER, to_be_enabled_or_not);

	if (to_be_enabled_or_not) 
	{
		// calc hand lists
		p_formula->CreateHandListMatrices();
		// one last parse - do not engage if parse fails
		if (!p_formula->ParseAllFormula(PMainframe()->GetSafeHwnd()))
		{
			// Invalid formula
			// Can't autoplay
			to_be_enabled_or_not = false;
			p_flags_toolbar->CheckButton(ID_MAIN_TOOLBAR_AUTOPLAYER, false);
		}
	}
	// Set valuie at the very last to be extra safe
	// and avoid problems with multiple threads
	// despite we use synchronization ;-)
	_autoplayer_engaged = to_be_enabled_or_not;
}

#undef ENT


bool CAutoplayer::DoChat(void)
{
	if (!IsChatAllowed())
	{
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] No chat, because chat turned off.\n");
		return false;
	}
	if ((p_autoplayer_functions->f$chat() == 0) || (_the_chat_message == NULL))
	{
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] No chat, because no chat message.\n");
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
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] Starting DoAllin...\n");

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
			k_autoplayer_function_raise, preferences.swag_delay_3());

		write_logautoplay(ActionConstantNames(k_prevaction_allin));
	}
	else  if (p_tablemap->allinmethod() == 2)
	{
		success = p_casino_interface->ClickButton(k_autoplayer_function_allin);

		write_logautoplay(ActionConstantNames(k_prevaction_allin));
	}
	else if (p_tablemap->allinmethod() == 3)
	{
		success = p_casino_interface->UseSliderForAllin();
		write_logautoplay(ActionConstantNames(k_prevaction_allin));
	}
	else
	{
		// Fourth case (default = 0): swagging the balance
		int userchair = p_symbol_engine_userchair->userchair();
		double betsize_for_allin = p_symbol_engine_chip_amounts->currentbet(userchair)
			+ p_symbol_engine_chip_amounts->balance(userchair); 
		success = p_casino_interface->EnterBetsize(betsize_for_allin);
	}
	if (success)
	{
		// Not really necessary to register the action,
		// as the game is over and there is no doallin-symbol,
		// but it does not hurt to register it anyway.
		p_symbol_engine_history->RegisterAction(k_autoplayer_function_allin);
		return true;
	}
	return false;
}


void CAutoplayer::DoAutoplayer(void) 
{
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] Starting Autoplayer cadence...\n");

	CheckBringKeyboard();

	p_scraper_access->GetNeccessaryTablemapObjects();

	write_log(preferences.debug_autoplayer(), "[AutoPlayer] Number of visible buttons: %d (%s)\n", 
		p_scraper_access->NumberOfVisibleButtons(),
		p_symbol_engine_autoplayer->GetFCKRAString());
		
	// Care about I86 regions first, because they are usually used 
	// to handle popups which occlude the table (unstable input)
	if (!HandleInterfacebuttonsI86())
	{
		// Care about sitin, sitout, leave, etc.
		p_autoplayer_functions->CalcSecondaryFormulas();
		if (!ExecuteSecondaryFormulasIfNecessary())	
		{
			write_log(preferences.debug_autoplayer(), "[AutoPlayer] No secondary formulas to be handled.\n");
			// Since OH 4.0.5 we support autoplaying immediatelly after connection
			// without the need to know the userchair to act on secondary formulas.
			// However: for primary formulas (f$alli, f$rais, etc.)
			// knowing the userchair (combination of cards and buttons) is a must.
			if (!p_symbol_engine_userchair->userchair_confirmed())
			{
				write_log(preferences.debug_autoplayer(), "[AutoPlayer] Skipping primary formulas because userchair unknown\n");
			}
			else
			{
				write_log(preferences.debug_autoplayer(), "[AutoPlayer] Going to evaluate primary formulas.\n");
				if(p_symbol_engine_autoplayer->isfinalanswer())
				{
					p_autoplayer_functions->CalcPrimaryFormulas();
					ExecutePrimaryFormulasIfNecessary();
				}
				else
				{
					write_log(preferences.debug_autoplayer(), "[AutoPlayer] No final answer, therefore not executing autoplayer-logic.\n");
				}
			}
		}
	}
	FinishActionSequenceIfNecessary();
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] ...ending Autoplayer cadence.\n");
}


bool CAutoplayer::DoSwag(void) 
{
	if (p_autoplayer_functions->f$betsize() > 0)
	{
		int success = p_casino_interface->EnterBetsize(p_autoplayer_functions->f$betsize());
		if (success)
		{
			p_symbol_engine_history->RegisterAction(k_autoplayer_function_betsize);
			return true;
		}
	}
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] Don't swag, because f$betsize evaluates to 0.\n");
	return false;
}


bool CAutoplayer::DoPrefold(void) 
{
	assert(p_autoplayer_functions->f$prefold() == 0);
	if (p_casino_interface->ClickButton(k_standard_function_prefold))
	{
		p_symbol_engine_history->RegisterAction(k_autoplayer_function_fold);
		write_logautoplay(ActionConstantNames(k_prevaction_fold));
		p_autoplayer_functions->CalcAutoTrace();
		write_log(preferences.debug_autoplayer(), "[AutoPlayer] Prefold executed.\n");
		return true;
	}
	return false;
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
	write_log(preferences.debug_autoplayer(), "[AutoPlayer] No interface button (i86X) to be handled.\n");
	return false;
}
