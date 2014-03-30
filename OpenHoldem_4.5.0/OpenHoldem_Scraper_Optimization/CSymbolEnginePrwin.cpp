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
#include "CSymbolEnginePrwin.h"

#include <assert.h>
#include "CFormula.h"
#include "CGrammar.h"
#include "CIteratorThread.h"
#include "CIteratorVars.h"
#include "CScraper.h"
#include "CScraperAccess.h"
#include "CSymbolenginePokerval.h"

CSymbolEnginePrwin *p_symbol_engine_prwin = NULL;

CSymbolEnginePrwin::CSymbolEnginePrwin()
{
	// The values of some symbol-engines depend on other engines.
	// As the engines get later called in the order of initialization
	// we assure correct ordering by checking if they are initialized.
	assert(p_symbol_engine_pokerval != NULL);
	assert(p_symbol_engine_tablelimits != NULL);
	assert(p_symbol_engine_userchair != NULL);
}

CSymbolEnginePrwin::~CSymbolEnginePrwin()
{}

void CSymbolEnginePrwin::InitOnStartup()
{
	ResetOnConnection();
}

void CSymbolEnginePrwin::ResetOnConnection()
{
	iter_vars.ResetVars();
	ResetOnHandreset();
}

void CSymbolEnginePrwin::ResetOnHandreset()
{
	ResetOnNewRound();
}

void CSymbolEnginePrwin::ResetOnNewRound()
{}

void CSymbolEnginePrwin::ResetOnMyTurn()
{
	p_iterator_thread->StartIteratorThreadIfNeeded();
	CalculateNOpponents();
	CalculateNhands();
}

void CSymbolEnginePrwin::ResetOnHeartbeat()
{
	// Taken from heartbeat-thread:
	// If we've folded, stop iterator thread and set prwin/tie/los to zero
	if (!p_scraper_access->UserHasCards())
	{
		p_iterator_thread->StopIteratorThread();
	}

}

void CSymbolEnginePrwin::CalculateNhands()
{
	CardMask		plCards = {0}, comCards = {0}, oppCards = {0}, playerEvalCards = {0}, opponentEvalCards = {0};
	HandVal			hv_player = 0, hv_opponent = 0;
	unsigned int	pl_pokval = 0, opp_pokval = 0;
	int				dummy = 0;
	int				nplCards, ncomCards;

	_nhandshi = 0;
	_nhandsti = 0;
	_nhandslo = 0;

	// player cards
	CardMask_RESET(plCards);
	nplCards = 0;
	for (int i=0; i<k_number_of_cards_per_player; i++)
	{
		if (p_scraper_access->IsKnownCard(p_scraper->card_player(USER_CHAIR, i)))
		{
			CardMask_SET(plCards, p_scraper->card_player(USER_CHAIR, i));
			nplCards++;
		}
	}

	// common cards
	CardMask_RESET(comCards);
	ncomCards = 0;
	for (int i=0; i<k_number_of_community_cards; i++)
	{
		if (p_scraper_access->IsKnownCard(p_scraper->card_common(i)))
		{
			CardMask_SET(comCards, p_scraper->card_common(i));
			ncomCards++;
		}
	}

	// player/common cards and pokerval
	CardMask_OR(playerEvalCards, plCards, comCards);
	hv_player = Hand_EVAL_N(playerEvalCards, nplCards+ncomCards);
	pl_pokval = p_symbol_engine_pokerval->CalculatePokerval(hv_player, 
		nplCards+ncomCards, &dummy, CARD_NOCARD, CARD_NOCARD);

	for (int i=0; i<(k_number_of_cards_per_deck-1); i++)
	{
		for (int j=(i+1); j<k_number_of_cards_per_deck; j++)
		{
			if (!CardMask_CARD_IS_SET(plCards, i) 
				&& !CardMask_CARD_IS_SET(plCards, j) 
				&& !CardMask_CARD_IS_SET(comCards, i) 
				&& !CardMask_CARD_IS_SET(comCards, j))
			{
				// opponent cards
				CardMask_RESET(oppCards);
				CardMask_SET(oppCards, i);
				CardMask_SET(oppCards, j);

				CardMask_OR(opponentEvalCards, oppCards, comCards);
				hv_opponent = Hand_EVAL_N(opponentEvalCards, 2+ncomCards);
				opp_pokval = p_symbol_engine_pokerval->CalculatePokerval(hv_opponent,
					(k_number_of_cards_per_player + ncomCards), 
					&dummy, CARD_NOCARD, CARD_NOCARD);

				if (pl_pokval > opp_pokval)
				{
					_nhandslo++;
				}
				else if (pl_pokval < opp_pokval)
				{
					_nhandshi++;
				}
				else
				{
					_nhandsti++;
				}
			}
		}
	}

	AssertRange(_nhandshi, 0, nhands());
	AssertRange(_nhandsti, 0, nhands());
	AssertRange(_nhandslo, 0, nhands());
	assert((_nhandshi + _nhandsti + _nhandslo) == nhands());


	_prwinnow = pow(((double)_nhandslo/nhands()), _nopponents_for_prwin);
	_prlosnow = 1 - pow((((double)_nhandslo + _nhandsti)/nhands()), _nopponents_for_prwin);

	AssertRange(_prwinnow, 0, 1);
	AssertRange(_prlosnow, 0, 1);
}

void CSymbolEnginePrwin::CalculateNOpponents()
{
	CGrammar gram;
	int e = SUCCESS;
	_nopponents_for_prwin = gram.CalcF$symbol(p_formula, 
		"f$prwin_number_of_opponents", &e);

	if (_nopponents_for_prwin > MAX_OPPONENTS)
	{
		_nopponents_for_prwin = MAX_OPPONENTS;
	}
	if (_nopponents_for_prwin < 1)
	{
		_nopponents_for_prwin = 1;
	}
}