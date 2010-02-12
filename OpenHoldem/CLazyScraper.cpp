#include "stdafx.h"
#include "CLazyScraper.h"
#include "CPreferences.h"
#include "CScraper.h"
#include "debug.h"

CLazyScraper *p_lazyscraper = NULL;

CLazyScraper::CLazyScraper()
{
}

CLazyScraper::~CLazyScraper()
{
}

void CLazyScraper::DoScrape()
{
	write_log(3, "CLazyScraper::DoScrape()\n");
	// As scraping is the most time-consuming part
	// of the heartbeat-cycle, we do optionally
	// execute only partial scrapes (aka "Lazy Scraping").
	// Please see the manual for more info about that.
	MessageBox(0, "Basic", "Scraping", 0);
	p_scraper->DoBasicScrapeButtons();
	if (CardScrapeNeeded())
	{
		MessageBox(0, "Cards", "Scraping", 0);
		write_log(3, "CLazyScraper: CardScrape needed\n");
		p_scraper->DoBasicScrapeAllPlayerCards();
		if (CompleteScrapeNeeded())
		{
			MessageBox(0, "Full", "Scraping", 0);
			write_log(3, "CLazyScraper: CompleteScrape needed\n");
			p_scraper->CompleteBasicScrapeToFullScrape();
		}
	}
}

bool CLazyScraper::IsMyTurn()
{
	return true;
}

bool CLazyScraper::HaveCards()
{
	return true;
}

bool CLazyScraper::CardScrapeNeeded()
{
	return (IsMyTurn() 
		|| (prefs.lazy_scraping_when_to_scrape() == k_lazy_scraping_cards)
		|| (prefs.lazy_scraping_when_to_scrape() == k_lazy_scraping_always));
}

bool CLazyScraper::CompleteScrapeNeeded()
{
	return (IsMyTurn() 
		|| (HaveCards() && (prefs.lazy_scraping_when_to_scrape() == k_lazy_scraping_cards))
		|| (prefs.lazy_scraping_when_to_scrape() == k_lazy_scraping_always));
}