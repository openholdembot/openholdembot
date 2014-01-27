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

#include "stdafx.h"
#include "CSymbolEngineUserchair.h"

#include "CPreferences.h"
#include "CScraper.h"
#include "CScraperAccess.h"
#include "CStringMatch.h"
#include "MagicNumbers.h"

CSymbolEngineUserchair *p_symbol_engine_userchair = NULL;

CSymbolEngineUserchair::CSymbolEngineUserchair()
{
	// The values of some symbol-engines depend on other engines.
	// As the engines get later called in the order of initialization
	// we assure correct ordering by checking if they are initialized.
	//
	// This engine does not use any other engines.
}

CSymbolEngineUserchair::~CSymbolEngineUserchair()
{}

void CSymbolEngineUserchair::InitOnStartup()
{
	ResetOnConnection();
}

void CSymbolEngineUserchair::ResetOnConnection()
{
	_userchair = k_undefined;
	_userchair_locked = false;
}

void CSymbolEngineUserchair::ResetOnHandreset()
{
	// !! TODO: option to reset it on every hand-reset 
	// (for table-changes in tournaments)
}

void CSymbolEngineUserchair::ResetOnNewRound()
{}

void CSymbolEngineUserchair::ResetOnMyTurn()
{}

void CSymbolEngineUserchair::ResetOnHeartbeat()
{
	if (!_userchair_locked)
	{
		CalculateUserChair();
	}
}

void CSymbolEngineUserchair::CalculateUserChair()
{
	int num_buttons_enabled = p_scraper_access->NumberOfVisibleButtons();
	if (num_buttons_enabled < k_min_buttons_needed_for_my_turn)
	{
		write_log(preferences.debug_symbolengine(),
			"[CSymbolEngineUserchair] CalculateUserChair() Not enough visible buttons\n");
	}
	else
	{
		for (int i=0; i<p_tablemap->nchairs(); i++)
		{
			if (p_scraper_access->IsKnownCard(p_scraper->card_player(i, 0)) 
				&& p_scraper_access->IsKnownCard(p_scraper->card_player(i, 1)))
	
			{
				_userchair = i;
				_userchair_locked = true;
				write_log(preferences.debug_symbolengine(),
					"[CSymbolEngineUserchair] CalculateUserChair() Setting userchair to %d\n",
					_userchair);
				return;
			}
		}
		write_log(preferences.debug_symbolengine(),
			"[CSymbolEngineUserchair] CalculateUserChair() Userchair not found, because no cards found\n");
	}
}

bool CSymbolEngineUserchair::EvaluateSymbol(const char *name, double *result)
{
	if (memcmp(name, "userchair", 9)==0 && strlen(name)==9)
	{
		*result = userchair();
		return true;
	}
	// Symbol of name different symbol-engine
	return false;
}