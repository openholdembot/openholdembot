#include "stdafx.h"
#include "CSymbolEngineActiveDealtPlaying.h"

#include "CScraper.h"
#include "CScraperAccess.h"
#include "CStringMatch.h"
#include "CSymbolEngineDealerchair.h"
#include "CSymbolEngineUserchair.h"
#include "CTableLimits.h"
#include "..\CTablemap\CTablemap.h"

CSymbolEngineActiveDealtPlaying::CSymbolEngineActiveDealtPlaying()
{}

CSymbolEngineActiveDealtPlaying::~CSymbolEngineActiveDealtPlaying()
{}

void CSymbolEngineActiveDealtPlaying::InitOnStartup()
{}

void CSymbolEngineActiveDealtPlaying::ResetOnConnection()
{}

void CSymbolEngineActiveDealtPlaying::ResetOnHandreset()
{
	_userchair   = p_symbol_engine_userchair->userchair();
	_dealerchair = p_symbol_engine_dealerchair->dealerchair();
	_nchairs     = p_tablemap->nchairs();
	_bblind      = p_tablelimits->bblind();

	_playersdealtbits = 0;
}

void CSymbolEngineActiveDealtPlaying::ResetOnNewRound()
{}

void CSymbolEngineActiveDealtPlaying::ResetOnMyTurn()
{}

void CSymbolEngineActiveDealtPlaying::ResetOnHeartbeat()
{
	CalculateActiveBits();
	CalculatePlayingBits();
	CalculateDealtBits();
	CalculateSeatedBits();
}

void CSymbolEngineActiveDealtPlaying::CalculateActiveBits()
{
	_playersactivebits  = 0;
	for (int i=0; i<k_max_number_of_players; i++)
	{
		if (p_string_match->IsStringActive(p_scraper->active(i)))
		{
			_playersactivebits |= (1<<i);			
		}
	}
}

void CSymbolEngineActiveDealtPlaying::CalculatePlayingBits()
{
	_playersplayingbits = 0;
	for (int i=0; i<k_max_number_of_players; i++)
	{
		if (p_scraper->card_player(i, 0) != CARD_NOCARD && p_scraper->card_player(i, 1) != CARD_NOCARD)
		{
			_playersplayingbits |= (1<<i);			
		}
	}
}

void CSymbolEngineActiveDealtPlaying::CalculateSeatedBits()
{
	_playersseatedbits = 0;
	for (int i=0; i<k_max_number_of_players; i++)
	{
		if (p_string_match->IsStringSeated(p_scraper->seated(i)))
		{
			_playersseatedbits |= 1<<i;			
		}
	}
}

void CSymbolEngineActiveDealtPlaying::CalculateDealtBits()
{
	int  number_of_blind_posters_found = 0;
	bool big_blind_found = false;
	bool first_non_blind_with_cards_found = false;

	for (int i=0; i<_nchairs; i++)
	{
		int chair_to_consider = (_dealerchair + i + 1) % _nchairs;
		bool this_player_got_dealt = false;
		// First we search the blinds only, 
		// i.e. players with a positive bet.
		// We don't consider players who are only "active",
		// i.e. players who sat out but came back.
		if ((number_of_blind_posters_found < k_usual_number_of_blind_posters)
			&& ! big_blind_found)
		{
			int bet = p_scraper->player_bet(chair_to_consider);
			if (bet > 0)
			{
				number_of_blind_posters_found++;
				this_player_got_dealt = true;
			}
			if ((bet == _bblind) || (number_of_blind_posters_found == k_usual_number_of_blind_posters))
			{
				// big blind might be allin for less than 1 bb
				// or small blind might be missing.
				// But we catch both cases, as long as not both happen
				// at the same time.
				big_blind_found = true;
			}
		}
		// Once the blinds have been found we accept every player as dealt
		// who is active. Usually these are a fast fold,
		// though there might be some people who were sitting out
		// and came back.
		// We do so, until we reach somebody who has cards.
		// After this player we look for cards only,
		// because there can be no quick folds after him.
		else if (!first_non_blind_with_cards_found)
		{
			if (p_string_match->IsStringActive(p_scraper->active(chair_to_consider)))
			{
				this_player_got_dealt = true;
				if (p_scraper_access->PlayerHasCards(chair_to_consider))
				{
					first_non_blind_with_cards_found = true;
				}
			}
		}
		else
		{
			assert(first_non_blind_with_cards_found);
			if (p_scraper_access->PlayerHasCards(chair_to_consider))
			{
				this_player_got_dealt = true;
			}
		}
		if (this_player_got_dealt)
		{
		_playersdealtbits |= 1 << chair_to_consider;
		}
	}
}
