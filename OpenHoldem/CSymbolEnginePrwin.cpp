#include "stdafx.h"
#include "CSymbolEnginePrwin.h"

#include "CFormula.h"
#include "CGrammar.h"
#include "CIteratorThread.h"
#include "CScraper.h"
#include "CSymbolenginePokerval.h"

CSymbolEnginePrwin::CSymbolEnginePrwin()
{}

CSymbolEnginePrwin::~CSymbolEnginePrwin()
{}

void CSymbolEnginePrwin::InitOnStartup()
{
	ResetOnConnection();
}

void CSymbolEnginePrwin::ResetOnConnection()
{
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
	CalculateNOpponents();
	CalculateProbabilities();
}

void CSymbolEnginePrwin::ResetOnHeartbeat()
{}

void CSymbolEnginePrwin::RestartIteratorThread()
{
	if (p_iterator_thread)
	{
		delete p_iterator_thread;
	}
	p_iterator_thread = new CIteratorThread;
}

void CSymbolEnginePrwin::CalculateProbabilities()
{
	RestartIteratorThread();
	CalculateNhands();
}

void CSymbolEnginePrwin::CalculateNhands()
{
	CardMask		plCards = {0}, comCards = {0}, oppCards = {0}, playerEvalCards = {0}, opponentEvalCards = {0};
	HandVal			hv_player = 0, hv_opponent = 0;
	unsigned int	pl_pokval = 0, opp_pokval = 0;
	int				dummy = 0;
	int				nplCards = 0, ncomCards = 0;

	// player cards
	// !!! bad code
	CardMask_RESET(plCards);
	nplCards = 0;
	for (int i=0; i<=1; i++)
	{
		if (p_scraper->card_player(userchair, i) != CARD_BACK && 
			p_scraper->card_player(userchair, i) != CARD_NOCARD)
		{
			CardMask_SET(plCards, p_scraper->card_player(userchair, i));
			nplCards++;
		}
	}

	// common cards
	CardMask_RESET(comCards);
	ncomCards = 0;
	for (int i=0; i<k_number_of_community_cards; i++)
	{
		if (p_scraper->card_common(i) != CARD_BACK && 
			p_scraper->card_common(i) != CARD_NOCARD)
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
	_prwinnow = pow(((double)_nhandslo/nhands()), _nopponents_for_prwin);
	_prlosnow = 1 - pow((((double)_nhandslo + _nhandsti)/nhands()), _nopponents_for_prwin);
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