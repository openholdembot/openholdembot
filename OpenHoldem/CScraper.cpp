#include "StdAfx.h"

#include "CScraper.h"

#include "..\CTransform\CTransform.h"
#include "..\CTransform\hash\lookup3.h"

#include "CSymbols.h"
#include "CPokerPro.h"

#include "OpenHoldem.h"
#include "MainFrm.h"

#define __HDC_HEADER 		HBITMAP		old_bitmap = NULL; \
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd); \
	HDC				hdc = GetDC(pMyMainWnd->attached_hwnd()); \
	HDC				hdcScreen = CreateDC("DISPLAY", NULL, NULL, NULL); \
	HDC				hdcCompatible = CreateCompatibleDC(hdcScreen);

#define __HDC_FOOTER	DeleteDC(hdcCompatible); \
	DeleteDC(hdcScreen); \
	ReleaseDC(pMyMainWnd->attached_hwnd(), hdc);

CScraper *p_scraper = NULL;

CScraper::CScraper(void)
{
	ClearScrapeAreas();

	_s_lock_blinds.blinds_are_locked = false;
}

CScraper::~CScraper(void)
{
}

void CScraper::ClearScrapeAreas(void)
{
	int i=0;

	for (i=0; i<=4; i++)
		set_card_common(i, CARD_NOCARD);

	for (i=0; i<=9; i++)
	{
		set_card_player(i, 0, CARD_NOCARD);
		set_card_player(i, 1, CARD_NOCARD);
		set_seated(i, "false");
		set_active(i, "false");
		set_dealer(i, false);
		set_player_bet(i, 0.0);
		set_player_name(i, "");
		set_name_good_scrape(i, false);
		set_player_balance(i, 0.0);
		set_balance_good_scrape(i, false);
		set_i86X_button_state(i, "false");
		set_button_state(i, "false");
		set_button_label(i, "");
		set_pot(i, 0.);
	}
	set_i86_button_state("false");

	set_button_label(0, "fold");
	set_button_label(1, "call");
	set_button_label(2, "raise");
	set_button_label(3, "allin");

	set_istournament(false);
	set_handnumber(0);
	set_sblind(0);
	set_bblind(0);
	set_bbet(0);
	set_ante(0);
	set_sb_bb(0);
	set_bb_BB(0);
	set_limit(-1);
	set_found_handnumber(false);
	set_found_sblind(false);
	set_found_bblind(false);
	set_found_bbet(false);
	set_found_ante(false);
	set_found_limit(false);
	set_found_sb_bb(false);
	set_found_bb_BB(false);

	set_card_player_for_display(0, CARD_NOCARD);
	set_card_player_for_display(1, CARD_NOCARD);

	// change detection
	_ucf_last=false;
	for (i=0; i<=4; i++)
		_card_common_last[i]=CARD_NOCARD;

	for (i=0; i<=9; i++)
	{
		_card_player_last[i][0]=CARD_NOCARD;
		_card_player_last[i][1]=CARD_NOCARD;
		_seated_last[i] = "false";
		_active_last[i] = "false";
		_dealer_last[i] = false;
		_name_last[i]="";
		_balance_last[i]=0;
		_playerbet_last[i]=0;
		_button_state_last[i]="";
		_i86X_button_state_last[i]="";
		_button_label_last[i]="";
		_pot_last[i]=0;
	}

	_i86_button_state_last = "";
	_istournament_last = false;
	_limit_last=0;
	_sblind_last=0;
	_bblind_last=0;
	_sb_bb_last=0;
	_bb_BB_last=0;
	_bbet_last=0;
	_ante_last=0;
	_handnumber_last=0;
	strcpy_s(_title_last, 512, "");
}

// returns true if common cards are in the middle of an animation
const bool CScraper::IsCommonAnimation(void)
{
	bool			animation = false;  // By default there is no animation going on
	int				flop_card_count = 0;

	// Count all the flop cards
	for (int i=0; i<=2; i++)
	{
		if (_card_common[i] != CARD_NOCARD)
		{
			flop_card_count++;
		}
	}

	// If there are some flop cards present,
	// but not all 3 then there is an animation going on
	if (flop_card_count > 0 && flop_card_count < 3)
	{
		animation = true;
	}
	// If the turn card is present,
	// but not all 3 flop cards are present then there is an animation going on
	else if (_card_common[3] != CARD_NOCARD && flop_card_count != 3)
	{
		animation = true;
	}
	// If the river card is present,
	// but the turn card isn't
	// OR not all 3 flop cards are present then there is an animation going on
	else if (_card_common[4] != CARD_NOCARD && (_card_common[3] == CARD_NOCARD || flop_card_count != 3))
	{
		animation = true;
	}

	return animation;
}

// returns true if window has changed and we processed the changes, false otherwise
int CScraper::DoScrape(void)
{
	write_log(3, "Starting Scraper cadence...");
	
	__HDC_HEADER

	// Get bitmap of whole window
	RECT		cr = {0};
	GetClientRect(pMyMainWnd->attached_hwnd(), &cr);

	old_bitmap = (HBITMAP) SelectObject(hdcCompatible, _entire_window_cur);
	BitBlt(hdcCompatible, 0, 0, cr.right, cr.bottom, hdc, cr.left, cr.top, SRCCOPY);
	SelectObject(hdcCompatible, old_bitmap);

	// get window title
	_title[0] = '\0';
	if (!p_pokerpro->IsConnected())
		GetWindowText(pMyMainWnd->attached_hwnd(), _title, MAX_WINDOW_TITLE-1);

	// If the bitmaps are the same, then return now
	if (BitmapsSame(_entire_window_last, _entire_window_cur) &&
		_ucf_last==p_symbols->user_chair_confirmed() &&
		strcmp(_title, _title_last)==0)
	{
		DeleteDC(hdcCompatible);
		DeleteDC(hdcScreen);
		ReleaseDC(pMyMainWnd->attached_hwnd(), hdc);

		write_log(3, "...ending Scraper cadence early (no change).");
		return NOTHING_CHANGED;
	}

	// Copy into "last" title
	if (strcmp(_title, _title_last)!=0)
	{
		strcpy_s(_title_last, 512, _title);

		set_found_handnumber(false);
		set_found_sblind(false);
		set_found_bblind(false);
		set_found_bbet(false);
		set_found_ante(false);
		set_found_sb_bb(false);
		set_found_bb_BB(false);
		set_found_limit(false);

		p_symbols->set_reset_stakes(true);
	}

	// Copy into "last" bitmap
	old_bitmap = (HBITMAP) SelectObject(hdcCompatible, _entire_window_last);
	BitBlt(hdcCompatible, 0, 0, cr.right-cr.left+1, cr.bottom-cr.top+1, hdc, cr.left, cr.top, SRCCOPY);
	SelectObject(hdc, old_bitmap);

	if (_ucf_last != p_symbols->user_chair_confirmed())
		_ucf_last = p_symbols->user_chair_confirmed();

	__HDC_FOOTER

	// flag for detecting if anything useful has changed - used downstream to decide whether or not to
	// update symbols, etc.
	_scrape_something_changed = NOTHING_CHANGED;

	// Common cards
	write_log(3, "Calling ScrapeCommonCards.");
	ScrapeCommonCards();

	// Player information
	for (int i=0; i<=9; i++)
	{
		write_log(3, "Calling ScrapePlayerCards, chair %d.", i);
		ScrapePlayerCards(i);
	
		write_log(3, "Calling ScrapeSeated, chair %d.", i);
		ScrapeSeated(i);
	
		write_log(3, "Calling ScrapeActive, chair %d.", i);
		ScrapeActive(i);
	
		write_log(3, "Calling ScrapeDealer, chair %d.", i);
		ScrapeDealer(i);
	
		write_log(3, "Calling ScrapeName, chair %d.", i);
		ScrapeName(i);
	
		write_log(3, "Calling ScrapeBet, chair %d.", i);
		ScrapeBet(i);
	
		write_log(3, "Calling ScrapeBalance, chair %d.", i);
		ScrapeBalance(i);  // Must come after ScrapeBet, as is dependent on ScrapeBet's results

		if (!IsStringSeated(_seated[i]) && !IsStringActive(_active[i]))
		{
			set_player_name(i, "");
			set_name_good_scrape(i, false);
			set_player_balance(i, 0);
			set_balance_good_scrape(i, false);
		}
	}

	write_log(3, "Calling ScrapeButtons.");
	ScrapeButtons();		// Buttons

	write_log(3, "Calling ScrapePots.");
	ScrapePots();		// Pots

	write_log(3, "Calling ScrapeLimits.");
	ScrapeLimits();		// limits

	write_log(3, "...ending Scraper cadence, changed: %d", _scrape_something_changed);

	return _scrape_something_changed;
}

bool CScraper::ProcessRegion(RMapCI r_iter)
{
	__HDC_HEADER

	// Get "current" bitmap
	old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
	BitBlt(hdcCompatible, 0, 0, r_iter->second.right - r_iter->second.left + 1, 
							    r_iter->second.bottom - r_iter->second.top + 1, 
								hdc, r_iter->second.left, r_iter->second.top, SRCCOPY);
	SelectObject(hdcCompatible, old_bitmap);

	// If the bitmaps are different, then continue on
	if (!BitmapsSame(r_iter->second.last_bmp, r_iter->second.cur_bmp))
	{

		// Copy into "last" bitmap
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.last_bmp);
		BitBlt(hdcCompatible, 0, 0, r_iter->second.right - r_iter->second.left + 1, 
									r_iter->second.bottom - r_iter->second.top + 1, 
									hdc, r_iter->second.left, r_iter->second.top, SRCCOPY);
		SelectObject(hdcCompatible, old_bitmap);

		__HDC_FOOTER
		return true;
	}

	__HDC_FOOTER
	return false;
}

void CScraper::ScrapeCommonCards()
{
	__HDC_HEADER
	unsigned int	card = CARD_NOCARD;
	CString			cardstr = "", rankstr = "", suitstr = "";
	CTransform		trans;
	CString			s = "";
	RMapCI			r_iter1 = p_tablemap->r$()->end(), r_iter2 = p_tablemap->r$()->end();

	for (int i=0; i<=4; i++)
	{
		card = CARD_NOCARD;

		// try r$c0cardfaceX region first
		s.Format("c0cardface%d", i);
		r_iter1 = p_tablemap->r$()->find(s.GetString());
		if (r_iter1 != p_tablemap->r$()->end())
		{
			ProcessRegion(r_iter1);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
			trans.DoTransform(r_iter1, hdcCompatible, &cardstr);
			SelectObject(hdcCompatible, old_bitmap);

			if (cardstr!="")
			{
				StdDeck_stringToCard((char *) cardstr.GetString(), (int *) &card);
			}
			else
			{
				card = CARD_NOCARD;
			}

			if (_card_common_last[i] != card)
			{
				_card_common_last[i] = card;
				_scrape_something_changed |= COM_CARD_CHANGED;
			}

			set_card_common(i, card);

			write_log(3, "c0cardface%d, result %s", i, cardstr.GetString());
		}

		// try r$c0cardfaceXrank/r$c0cardfaceXsuit region next
		s.Format("c0cardface%drank", i);
		r_iter1 = p_tablemap->r$()->find(s.GetString());

		s.Format("c0cardface%dsuit", i);
		r_iter2 = p_tablemap->r$()->find(s.GetString());

		if (r_iter1 != p_tablemap->r$()->end() && r_iter2 != p_tablemap->r$()->end() && card==CARD_NOCARD)
		{
			// Get rank
			ProcessRegion(r_iter1);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
			trans.DoTransform(r_iter1, hdcCompatible, &rankstr);
			SelectObject(hdcCompatible, old_bitmap);

			// Get suit
			ProcessRegion(r_iter2);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter2->second.cur_bmp);
			trans.DoTransform(r_iter2, hdcCompatible, &suitstr);
			SelectObject(hdcCompatible, old_bitmap);

			if (rankstr=="10") rankstr="T";

			if (rankstr!="" && suitstr!="")
			{
				cardstr = rankstr + suitstr;
				StdDeck_stringToCard((char *) cardstr.GetString(), (int *) &card);
			}

			if (_card_common_last[i] != card)
			{
				_card_common_last[i] = card;
				_scrape_something_changed |= COM_CARD_CHANGED;
			}

			set_card_common(i, card);

			write_log(3, "c0cardface%drank/c0cardface%dsuit, result %s", i, i, cardstr.GetString());
		}
	}

	__HDC_FOOTER
}

void CScraper::ScrapePlayerCards(int chair)
{
	unsigned int		card = CARD_NOCARD;
	bool				got_new_scrape = false;
	CString				cardstr = "", rankstr = "", suitstr = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter1 = p_tablemap->r$()->end(), r_iter2 = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs())
		return;

	__HDC_HEADER

	int sym_userchair = (int) p_symbols->sym()->userchair;

	// Player cards
	got_new_scrape = false;
	for (int j=0; j<=1; j++)
	{
		card = CARD_NOCARD;

		// try r$uXcardfaceY region first
		s.Format("u%dcardface%d", chair, j);
		r_iter1 = p_tablemap->r$()->find(s.GetString());
		if (r_iter1 != p_tablemap->r$()->end())
		{
			ProcessRegion(r_iter1);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
			trans.DoTransform(r_iter1, hdcCompatible, &cardstr);
			SelectObject(hdcCompatible, old_bitmap);

			if (cardstr!="")
			{
				StdDeck_stringToCard((char *) cardstr.GetString(), (int *) &card);

				if (_card_player_last[chair][j] != card)
				{
					_card_player_last[chair][j] = card;
					_scrape_something_changed |= PL_CARD_CHANGED;
				}

				set_card_player(chair, j, card);

				if (chair==sym_userchair)
					set_card_player_for_display(j, card);
			}

			write_log(3, "u%dcardface%d, result %s", chair, j, cardstr.GetString());
		}

		// try r$pXcardfaceY region next
		s.Format("p%dcardface%d", chair, j);
		r_iter1 = p_tablemap->r$()->find(s.GetString());
		if (r_iter1 != p_tablemap->r$()->end())
		{
			ProcessRegion(r_iter1);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
			trans.DoTransform(r_iter1, hdcCompatible, &cardstr);
			SelectObject(hdcCompatible, old_bitmap);

			if (cardstr!="" && card==CARD_NOCARD)
			{
				StdDeck_stringToCard((char *) cardstr.GetString(), (int *) &card);

				if (_card_player_last[chair][j] != card)
				{
					_card_player_last[chair][j] = card;
					_scrape_something_changed |= PL_CARD_CHANGED;
				}

				set_card_player(chair, j, card);

				if (chair==sym_userchair)
					set_card_player_for_display(j,  card);
			}

			write_log(3, "p%dcardface%d, result %s", chair, j, cardstr.GetString());
		}

		// try r$pXcardfaceYrank/r$pXcardfaceYsuit regions next
		s.Format("p%dcardface%drank", chair, j);
		r_iter1 = p_tablemap->r$()->find(s.GetString());

		s.Format("p%dcardface%dsuit", chair, j);
		r_iter2 = p_tablemap->r$()->find(s.GetString());

		if (r_iter1!=p_tablemap->r$()->end() && r_iter2!=p_tablemap->r$()->end() && card==CARD_NOCARD)
		{
			// Get rank
			ProcessRegion(r_iter1);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
			trans.DoTransform(r_iter1, hdcCompatible, &rankstr);
			SelectObject(hdcCompatible, old_bitmap);

			// Get suit
			ProcessRegion(r_iter2);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter2->second.cur_bmp);
			trans.DoTransform(r_iter2, hdcCompatible, &suitstr);
			SelectObject(hdcCompatible, old_bitmap);

			if (rankstr=="10") rankstr="T";

			if (rankstr!="" && suitstr!="")
			{
				cardstr = rankstr + suitstr;
				StdDeck_stringToCard((char *) cardstr.GetString(), (int *) &card);

				if (_card_player_last[chair][j] != card)
				{
					_card_player_last[chair][j] = card;
					_scrape_something_changed |= PL_CARD_CHANGED;
				}

				set_card_player(chair, j, card);

				if (chair==sym_userchair)
					set_card_player_for_display(j, card);
			}

			write_log(3, "p%dcardface%drank/p%dcardface%dsuit, result %s%s", chair, j, chair, j, rankstr.GetString(), suitstr.GetString());
		}
	}

	// Player card backs pXcardback
	s.Format("p%dcardback", chair);
	r_iter1 = p_tablemap->r$()->find(s.GetString());
	if (r_iter1 != p_tablemap->r$()->end() && card==CARD_NOCARD)
	{
		ProcessRegion(r_iter1);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
		trans.DoTransform(r_iter1, hdcCompatible, &cardstr);
		SelectObject(hdcCompatible, old_bitmap);

		if (IsStringCardback(cardstr))
		{
			set_card_player(chair, 0, CARD_BACK);
			set_card_player(chair, 1, CARD_BACK);

			write_log(3, "p%dcardback, result CARDBACK", chair);
		}
		else
		{
			set_card_player(chair, 0, CARD_NOCARD);
			set_card_player(chair, 1, CARD_NOCARD);

			write_log(3, "p%dcardback, result NOCARD", chair);
		}

		if (_card_player_last[chair][0] != _card_player[chair][0] ||
			_card_player_last[chair][1] != _card_player[chair][1] )
		{
			_card_player_last[chair][0] = _card_player[chair][0] ;
			_card_player_last[chair][1] = _card_player[chair][1];
			_scrape_something_changed |= PL_CARD_CHANGED;
		}
	}
	__HDC_FOOTER
}

void CScraper::ScrapeSeated(int chair)
{
	CString				result = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs())
		return;

	__HDC_HEADER

	set_seated(chair, "false");

	// try p region first pXseated
	s.Format("p%dseated", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		if (result!="")
			set_seated(chair, result);

		write_log(3, "p%dseated, result %s", chair, result.GetString());
	}

	// try u region next uXseated,
	// but only if we didn't get a positive result from the p region
	s.Format("u%dseated", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !IsStringSeated(_seated[chair]))
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		if (result!="")
			set_seated(chair, result);

		write_log(3, "u%dseated, result %s", chair, result.GetString());
	}

	if (_seated_last[chair] != _seated[chair])
	{
		_seated_last[chair] = _seated[chair];
		_scrape_something_changed |= SEATED_CHANGED;
	}

	__HDC_FOOTER
}

void CScraper::ScrapeActive(int chair)
{
	CString				result = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs())
		return;

	__HDC_HEADER

	set_active(chair, "false");

	// try p region first pXactive
	s.Format("p%dactive", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		set_active(chair, result);

		write_log(3, "p%dactive, result %s", chair, result.GetString());
	}

	// try u region next, but only if we didn't get a key result from the p region
	s.Format("u%dactive", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && 
		((!IsStringActive(_active[chair]) && p_tablemap->activemethod() != 2) ||
		 (IsStringActive(_active[chair]) && p_tablemap->activemethod() == 2) ) )
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		set_active(chair, result);
		write_log(3, "u%dactive, result %s", chair, result.GetString());
	}

	if (_active_last[chair] != _active[chair])
	{
		_active_last[chair] = _active[chair];
		_scrape_something_changed |= ACTIVE_CHANGED;
	}

	__HDC_FOOTER
}

void CScraper::ScrapeDealer(int chair)
{
	CString				result = "";
	CTransform			trans;
	bool				found_dealer;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs())
		return;

	__HDC_HEADER

	set_dealer(chair, false);

	// Dealer button pXdealer
	found_dealer = false;

	s.Format("p%ddealer", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		if (IsStringDealer(result))
			found_dealer = true;

		write_log(3, "p%ddealer, result %s", chair, result.GetString());
	}

	// Dealer button uXdealer
	s.Format("u%ddealer", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !found_dealer)
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		if (IsStringDealer(result))
			found_dealer = true;

		write_log(3, "u%ddealer, result %s", chair, result.GetString());
	}

	set_dealer(chair, found_dealer);

	if (_dealer_last[chair] != _dealer[chair])
	{
		_dealer_last[chair] = _dealer[chair];
		_scrape_something_changed |= DEALER_CHANGED;
	}

	__HDC_FOOTER
}

void CScraper::ScrapeName(int chair) 
{

	bool				got_new_scrape = false;
	CString				text = "";
	int					ret = 0;
	bool				is_seated = IsStringSeated(_seated[chair]);
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs())
		return;

	__HDC_HEADER

	int	sym_chair = (int) p_symbols->sym()->chair;

	got_new_scrape = false;

	// Player name uname
	r_iter = p_tablemap->r$()->find("uname");
	if (r_iter != p_tablemap->r$()->end() && p_symbols->user_chair_confirmed() && chair==sym_chair && is_seated)
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret==ERR_GOOD_SCRAPE_GENERAL && text!="")
			got_new_scrape = true;

		write_log(3, "uname, result %s", text.GetString());
	}

	// Player name uXname
	s.Format("u%dname", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !got_new_scrape)
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret==ERR_GOOD_SCRAPE_GENERAL && text!="")
			got_new_scrape = true;

		write_log(3, "u%dname, result %s", chair, text.GetString());
	}

	// Player name pXname
	s.Format("p%dname", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !got_new_scrape && is_seated)
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret==ERR_GOOD_SCRAPE_GENERAL && text!="")
			got_new_scrape = true;

		write_log(3, "p%dname, result %s", chair, text.GetString());
	}

	if (got_new_scrape)
	{
		set_name_good_scrape(chair, true);
		set_player_name(chair, text);
	}
	else
	{
		set_name_good_scrape(chair, false);
	}

	if (_name_last[chair] != _player_name[chair])
	{
		_name_last[chair] = _player_name[chair];
		_scrape_something_changed |= NAME_CHANGED;
	}

	__HDC_FOOTER
}

void CScraper::ScrapeBalance(int chair)
{
	bool				got_new_scrape = false;
	CString				text = "";
	int					ret = 0;
	bool				is_seated = IsStringSeated(_seated[chair]);
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs())
		return;

	__HDC_HEADER

	int	sym_chair = (int) p_symbols->sym()->chair;

	got_new_scrape = false;

	set_sitting_out(chair, false);

	// Player balance ubalance
	r_iter = p_tablemap->r$()->find("ubalance");
	if (r_iter != p_tablemap->r$()->end() && p_symbols->user_chair_confirmed() && chair==sym_chair && is_seated)
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret == ERR_GOOD_SCRAPE_GENERAL)
		{
			if (IsStringAllin(text))
			{
				got_new_scrape = true;
				text = "0";

				write_log(3, "ubalance, result ALLIN");
			}

			else if (text.MakeLower().Find("out")!=-1)
			{
				set_sitting_out(chair, true);
				set_active(chair, "false");

				write_log(3, "ubalance, result OUT");
			}

			else
			{
				text.Remove(',');
				text.Remove('$');

				if (p_tablemap->balancenumbersonly())
				{
					for (int i='a'; i<='z'; i++)
						text.Remove(i);
					for (int i='A'; i<='Z'; i++)
						text.Remove(i);
					text.Remove('-');
					text.Remove('(');
					text.Remove(')');
				}

				if (text!="" && IsNumeric(text))
					got_new_scrape = true;

				write_log(3, "ubalance, result %s", text.GetString());
			}
		}
	}

	// Player name uXbalance
	s.Format("u%dbalance", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !got_new_scrape)
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret == ERR_GOOD_SCRAPE_GENERAL)
		{
			if (IsStringAllin(text))
			{
				got_new_scrape = true;
				text = "0";

				write_log(3, "u%dbalance, result ALLIN", chair);
			}

			else if (text.MakeLower().Find("out")!=-1)
			{
  				set_sitting_out(chair, true);
				set_active(chair, "false");

				write_log(3, "u%dbalance, result OUT", chair);
			}

			else
			{
				text.Remove(',');
				text.Remove('$');

				if (p_tablemap->balancenumbersonly())
				{
					for (int i='a'; i<='z'; i++)
						text.Remove(i);
					for (int i='A'; i<='Z'; i++)
						text.Remove(i);
					text.Remove('-');
					text.Remove('(');
					text.Remove(')');
				}

				if (text!="" && IsNumeric(text))
					got_new_scrape = true;

				write_log(3, "u%dbalance, result %s", chair, text.GetString());
			}
		}
	}

	// Player balance pXbalance
	s.Format("p%dbalance", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !got_new_scrape && is_seated)
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret == ERR_GOOD_SCRAPE_GENERAL)
		{
			if (IsStringAllin(text))
			{
				got_new_scrape = true;
				text = "0";

				write_log(3, "u%dbalance, result ALLIN", chair);
			}

			else if (text.MakeLower().Find("out")!=-1)
			{
  				set_sitting_out(chair, true);
				set_active(chair, "false");

				write_log(3, "u%dbalance, result OUT", chair);
			}

			else
			{
				text.Remove(',');
				text.Remove('$');

				if (p_tablemap->balancenumbersonly())
				{
					for (int i='a'; i<='z'; i++)
						text.Remove(i);
					for (int i='A'; i<='Z'; i++)
						text.Remove(i);
					text.Remove('-');
					text.Remove('(');
					text.Remove(')');
				}

				if (text!="" && IsNumeric(text))
					got_new_scrape = true;

				write_log(3, "u%dbalance, result %s", chair, text.GetString());
			}
		}
	}

	if (got_new_scrape)
	{
		set_balance_good_scrape(chair, true);
		set_player_balance(chair, trans.StringToMoney(text));
	}
	else
	{
		set_balance_good_scrape(chair, false);
	}

	if (_balance_last[chair] != _player_balance[chair])
	{
		_balance_last[chair] = _player_balance[chair];
		_scrape_something_changed |= BALANCE_CHANGED;
	}

	__HDC_FOOTER
}

void CScraper::ScrapeBet(int chair)
{
	CString				text = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs())
		return;

	__HDC_HEADER

	set_player_bet(chair, 0);

	// Player bet pXbet
	s.Format("p%dbet", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		text.Remove(',');
		text.Remove('$');

		if (text!="")
		{
			set_player_bet(chair, trans.StringToMoney(text));
		}

		write_log(3, "p%dbet, result %s", chair, text.GetString());
	}

	// uXbet
	s.Format("u%dbet", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && _player_bet[chair] == 0)
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		text.Remove(',');
		text.Remove('$');

		if (text!="")
		{
			set_player_bet(chair, trans.StringToMoney(text));
		}

		write_log(3, "u%dbet, result %s", chair, text.GetString());
	}		
		
	// pXchip00
	s.Format("p%dchip00", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && _player_bet[chair] == 0)
	{
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, _entire_window_cur);
		set_player_bet(chair, DoChipScrape(r_iter));
		SelectObject(hdcCompatible, old_bitmap);

		write_log(3, "p%dchipXY, result %f", chair, _player_bet[chair]);
	}

	if (_playerbet_last[chair] != _player_bet[chair])
	{
		_playerbet_last[chair] = _player_bet[chair];
		_scrape_something_changed |= PLAYERBET_CHANGED;
	}

	__HDC_FOOTER
}

void CScraper::ScrapeButtons() 
{
	__HDC_HEADER

	int					j = 0, k = 0;
	CString				text = "";
	POINT				handle_xy = {0};
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end(), slider = p_tablemap->r$()->end(), handleCI = p_tablemap->r$()->end();
	RMapI				handleI = p_tablemap->set_r$()->end();

	set_button_label(0, "fold");
	set_button_label(1, "call");
	set_button_label(2, "raise");
	set_button_label(3, "allin");
	for (j=4; j<=9; j++)
	{
		set_button_label(j, "");
	}
	for (j=0; j<=9; j++)
	{
		set_button_state(j, "false");
		set_i86X_button_state(j, "false");
	}

	for (j=0; j<=9; j++)
	{
		// Button state iXstate
		s.Format("i%dstate", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end())
		{
			ProcessRegion(r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			if (text!="")
				set_button_state(j, text);

			if (_button_state_last[j] != _button_state[j])
			{
				_button_state_last[j] = _button_state[j];
				_scrape_something_changed |= BUTTONSTATE_CHANGED;
			}

			write_log(3, "i%dstate, result %s", j, text.GetString());
		}

		// i86X button state i86Xstate
		s.Format("i86%dstate", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end())
		{
			ProcessRegion(r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			if (text!="")
				set_i86X_button_state(j, text);

			if (_i86X_button_state_last[j] != _i86X_button_state[j])
			{
				_i86X_button_state_last[j] = _i86X_button_state[j];
				_scrape_something_changed |= BUTTONSTATE_CHANGED;
			}

			write_log(3, "i86%dstate, result %s", j, text.GetString());
		}

		// Button label
		// First check iXlabel
		text = "";

		s.Format("i%dlabel", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end())
		{
			ProcessRegion(r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			if (text!="")
				set_button_label(j, text);

			write_log(3, "i%dlabel, result %s", j, text.GetString());
		}

		// Second check iXlabelY
		for (k=0; k<=9 && text == ""; k++)
		{
			s.Format("i%dlabel%d", j, k);
			r_iter = p_tablemap->r$()->find(s.GetString());
			if (r_iter != p_tablemap->r$()->end())
			{
				ProcessRegion(r_iter);
				old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
				trans.DoTransform(r_iter, hdcCompatible, &text);
				SelectObject(hdcCompatible, old_bitmap);

				if (text!="")
					set_button_label(j, text);

				write_log(3, "i%dlabel%d, result %s", j, k, text.GetString());
			}
		}

		if (_button_label_last[j] != _button_label[j])
		{
			_button_label_last[j] = _button_label[j];
			_scrape_something_changed |= BUTTONLABEL_CHANGED;
		}
	}

	// When using MM, grab i5state for PT network
	if ((bool) p_symbols->sym()->ismanual)
	{
		p_tablemap->set_network(p_scraper->button_state(5));
	}

	// i86 button state
	s.Format("i86state");
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (text!="")
			set_i86_button_state(text);
		else
			set_i86_button_state("false");

		if (_i86_button_state_last != _i86_button_state)
		{
			_i86_button_state_last = _i86_button_state;
			_scrape_something_changed |= BUTTONSTATE_CHANGED;
		}

		write_log(3, "i86state, result %s", text.GetString());
	}

	// find handle
	handleCI = p_tablemap->r$()->find("i3handle");
	slider = p_tablemap->r$()->find("i3slider");

	if (handleCI!=p_tablemap->r$()->end() && slider!=p_tablemap->r$()->end() && _button_state[3]!="false")
	{
		ProcessRegion(handleCI);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, handleCI->second.cur_bmp);

		j = slider->second.right - handleCI->second.left;
		text="";
		
		_handle_found_at_xy = false;
		
		for (k=0; k<=j; k++)
		{
			handleI = p_tablemap->set_r$()->find("i3handle");
			handleI->second.left += k;
			handleI->second.right += k;

			handleCI = p_tablemap->r$()->find("i3handle");
			trans.DoTransform(handleCI, hdcCompatible, &text);

			handleI = p_tablemap->set_r$()->find("i3handle");
			handleI->second.left -= k;
			handleI->second.right -= k;
			if (text == "handle" || text == "true") 
				break;
		}

		if (text!="" && k <= j)
		{
			handleCI = p_tablemap->r$()->find("i3handle");
			handle_xy.x = handleCI->second.left + k;
			handle_xy.y = handleCI->second.top;

			set_handle_found_at_xy(true);
			set_handle_xy(handle_xy);
		}

		write_log(3, "i3handle, result %d,%d", handle_xy.x, handle_xy.y);
			
		SelectObject(hdcCompatible, old_bitmap);
	}

	__HDC_FOOTER
}

void CScraper::ScrapePots()
{
	__HDC_HEADER
	int					j = 0;
	CString				text = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	for (j=0; j<=9; j++)
		set_pot(j, 0);

	for (j=0; j<=9; j++)
	{
		// r$c0potX
		s.Format("c0pot%d", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end())
		{
			ProcessRegion(r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			text.Remove(',');
			text.Remove('$');

			if (text!="")
			{
				set_pot(j, trans.StringToMoney(text));

				if (_pot_last[j] != _pot[j])
				{
					_pot_last[j] = _pot[j];
					_scrape_something_changed |= POT_CHANGED;
				}
			}

			write_log(3, "c0pot%d, result %s", j, text.GetString());
		}

		// r$c0potXchip00_index
		s.Format("c0pot%dchip00", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end() && _pot[j] == 0)
		{
			ProcessRegion(r_iter);
			//old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			//trans.DoTransform(r_iter, hdcCompatible, &text);
			//SelectObject(hdcCompatible, old_bitmap);

			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, _entire_window_cur);
			set_pot(j, DoChipScrape(r_iter));
			SelectObject(hdcCompatible, old_bitmap);

			if (_pot_last[j] != _pot[j])
			{
				_pot_last[j] = _pot[j];
				_scrape_something_changed |= POT_CHANGED;
			}

			write_log(3, "c0pot%dchipXY, result %f", j, _pot[j]);

			// update the bitmap for second chip position in the first stack
			s.Format("c0pot%dchip01", j);
			r_iter = p_tablemap->r$()->find(s.GetString());
			if (r_iter != p_tablemap->r$()->end())
				ProcessRegion(r_iter);

			// update the bitmap for first chip position in the second stack
			s.Format("c0pot%dchip10", j);
			r_iter = p_tablemap->r$()->find(s.GetString());
			if (r_iter != p_tablemap->r$()->end())
				ProcessRegion(r_iter);
		}
	}

	__HDC_FOOTER
}

void CScraper::ScrapeLimits()
{
	__HDC_HEADER
	int					j = 0;
	double				handnumber = 0.;
	bool				istournament = false;
	CString				text = "";
	CString				titletext = "";
	char				c_titletext[MAX_WINDOW_TITLE] = {0};
	bool				got_new_scrape = false, log_blind_change = false;
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();
	SMapCI				s_iter = p_tablemap->s$()->end();

	set_found_sblind(false);
	set_found_bblind(false);
	set_found_bbet(false);
	set_found_ante(false);
	set_found_limit(false);
	set_found_sb_bb(false);
	set_found_bb_BB(false);

	// These persist after scraping the istournament/handnumber regions
	// to seed l_istournament and l_handnumber when we scrape info from
	// the titlebar.  That way if we do not find tournament or handnumber
	// information in the titlebar we won't overwrite the values we scraped
	// from the istournament/handnumber regions.
	istournament = false;
	handnumber = 0.0;

	// c0istournament
	r_iter = p_tablemap->r$()->find("c0istournament");
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (text!="")
			istournament = true;
		else
			istournament = false;

		set_istournament(istournament);

		if (_istournament_last != _s_limit_info.istournament)
		{
			_istournament_last = _s_limit_info.istournament;
			_scrape_something_changed |= LIMITS_CHANGED;
		}

		write_log(3, "c0istournament, result %s", text.GetString());
	}

	// r$c0handnumber
	r_iter = p_tablemap->r$()->find("c0handnumber");
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (text!="")
		{
			handnumber = GetHandnumFromString(text);
			set_handnumber(handnumber);
			got_new_scrape = true;
		}

		write_log(3, "c0handnumber, result %s", text.GetString());
	}

	for (j=0; j<=9; j++)
	{
		// r$c0handnumberX
		s.Format("c0handnumber%d", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end() && !got_new_scrape)
		{
			ProcessRegion(r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);
			
			if (text!="")
			{
				handnumber = GetHandnumFromString (text);
				set_handnumber(handnumber);
				got_new_scrape = true;
			}

			write_log(3, "c0handnumber%d, result %s", j, text.GetString());
		}
	}

	if (_s_lock_blinds.blinds_are_locked)
	{
		set_sblind(_s_lock_blinds.sblind);
		set_found_sblind(true);
		set_bblind(_s_lock_blinds.bblind);
		set_found_bblind(true);
		set_bbet(_s_lock_blinds.bbet);
		set_found_bbet(true);
		set_ante(_s_lock_blinds.ante);
		set_found_ante(true);
		set_limit(_s_lock_blinds.gametype);
		set_found_limit(true);

		write_log(3, "Locked blinds, result sblind/bblind/bbet/ante/gametype: %f/%f/%f/%f/%d", 
			_s_lock_blinds.sblind, _s_lock_blinds.bblind, _s_lock_blinds.bbet, _s_lock_blinds.ante, _s_lock_blinds.gametype);
	}

	else
	{
		double l_sblind=0., l_bblind=0., l_bbet=0., l_ante=0., l_sb_bb=0., l_bb_BB=0.;
		int l_limit=0;
		bool l_found_handnumber=false, l_found_sblind=false, l_found_bblind=false;
		bool l_found_bbet=false, l_found_ante=false, l_found_limit=false, l_found_sb_bb=false;
		bool l_found_bb_BB=false;

		// These are scraped from specific regions earlier in this
		// function.  Use the values we scraped (if any) to seed
		// the l_ locals so that we don't blindly overwrite the
		// information we scraped from those specific regions with
		// default values if we can't find them in the titlebar.
		double l_handnumber = handnumber;
		bool l_istournament = istournament;

		// s$ttlimits - Scrape blinds/stakes/limit info from title text
		s_iter = p_tablemap->s$()->find("ttlimits");
		if (s_iter != p_tablemap->s$()->end())
		{
			GetWindowText(pMyMainWnd->attached_hwnd(), c_titletext, MAX_WINDOW_TITLE-1);
			titletext = c_titletext;
		 	
			trans.ParseStringBSL(
				titletext, s_iter->second.text, NULL,
				&l_handnumber, &l_sblind, &l_bblind, &l_bbet, &l_ante, &l_limit, &l_sb_bb, &l_bb_BB, &l_istournament, 
				&l_found_handnumber, &l_found_sblind, &l_found_bblind, &l_found_bbet, 
				&l_found_ante, &l_found_limit, &l_found_sb_bb, &l_found_bb_BB);

			write_log(3, "ttlimits, result sblind/bblind/bbet/ante/gametype: %f/%f/%f/%f/%d", 
				_s_lock_blinds.sblind, _s_lock_blinds.bblind, _s_lock_blinds.bbet, _s_lock_blinds.ante, _s_lock_blinds.gametype);
		}

		// s$ttlimitsX - Scrape blinds/stakes/limit info from title text
		for (j=0; j<=9; j++)
		{
			s.Format("ttlimits%d", j);
			s_iter = p_tablemap->s$()->find(s.GetString());
			if (s_iter != p_tablemap->s$()->end())
			{
				GetWindowText(pMyMainWnd->attached_hwnd(), c_titletext, MAX_WINDOW_TITLE-1);
				titletext = c_titletext;
	
				trans.ParseStringBSL(
					titletext, s_iter->second.text, NULL,
					&l_handnumber, &l_sblind, &l_bblind, &l_bbet, &l_ante, &l_limit, &l_sb_bb, &l_bb_BB, &l_istournament, 
					&l_found_handnumber, &l_found_sblind, &l_found_bblind, &l_found_bbet, 
					&l_found_ante, &l_found_limit, &l_found_sb_bb, &l_found_bb_BB);

				write_log(3, "ttlimits%d, result sblind/bblind/bbet/ante/gametype: %f/%f/%f/%f/%d", j,
					_s_lock_blinds.sblind, _s_lock_blinds.bblind, _s_lock_blinds.bbet, _s_lock_blinds.ante, _s_lock_blinds.gametype);
			}
		}

		// r$c0limits, s$c0limits
		r_iter = p_tablemap->r$()->find("c0limits");
		s_iter = p_tablemap->s$()->find("c0limits");
		if (r_iter!=p_tablemap->r$()->end() && s_iter!=p_tablemap->s$()->end())
		{
			ProcessRegion(r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			if (text!="")
			{
				trans.ParseStringBSL(
					text, s_iter->second.text, NULL,
					&l_handnumber, &l_sblind, &l_bblind, &l_bbet, &l_ante, &l_limit, &l_sb_bb, &l_bb_BB, &l_istournament, 
					&l_found_handnumber, &l_found_sblind, &l_found_bblind, &l_found_bbet, 
					&l_found_ante, &l_found_limit, &l_found_sb_bb, &l_found_bb_BB);

				write_log(3, "c0limits, result sblind/bblind/bbet/ante/gametype: %f/%f/%f/%f/%d", 
					_s_lock_blinds.sblind, _s_lock_blinds.bblind, _s_lock_blinds.bbet, _s_lock_blinds.ante, _s_lock_blinds.gametype);
			}
		}

		for (j=0; j<=9; j++)
		{
			// r$c0limitsX, s$c0limitsX
			s.Format("c0limits%d", j);
			r_iter = p_tablemap->r$()->find(s.GetString());
			s.Format("c0limits%d", j);
			s_iter = p_tablemap->s$()->find(s.GetString());

			if (r_iter!=p_tablemap->r$()->end() && s_iter!=p_tablemap->s$()->end())
			{
				ProcessRegion(r_iter);
				old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
				trans.DoTransform(r_iter, hdcCompatible, &text);
				SelectObject(hdcCompatible, old_bitmap);

				if (text!="")
				{
					trans.ParseStringBSL(text, s_iter->second.text, NULL,
						&l_handnumber, &l_sblind, &l_bblind, &l_bbet, &l_ante, &l_limit, &l_sb_bb, &l_bb_BB, &l_istournament, 
						&l_found_handnumber, &l_found_sblind, &l_found_bblind, &l_found_bbet, 
						&l_found_ante, &l_found_limit, &l_found_sb_bb, &l_found_bb_BB);
				}

				write_log(3, "c0limits%d, result sblind/bblind/bbet/ante/gametype: %f/%f/%f/%f/%d", j,
					_s_lock_blinds.sblind, _s_lock_blinds.bblind, _s_lock_blinds.bbet, _s_lock_blinds.ante, _s_lock_blinds.gametype);
			}

			// save what we just scanned through
			set_handnumber(l_handnumber);
			set_sblind(l_sblind);
			set_bblind(l_bblind);
			set_bbet(l_bbet);
			set_ante(l_ante);
			set_limit(l_limit);
			set_sb_bb(l_sb_bb);
			set_bb_BB(l_bb_BB);
			set_istournament(l_istournament);
			set_found_handnumber(l_found_handnumber);
			set_found_sblind(l_found_sblind);
			set_found_bblind(l_found_bblind);
			set_found_bbet(l_found_bbet);
			set_found_ante(l_found_ante);
			set_found_limit(l_found_limit);
			set_found_sb_bb(l_found_sb_bb);
			set_found_bb_BB(l_found_bb_BB);
		}


		// r$c0sblind
		r_iter = p_tablemap->r$()->find("c0sblind");
		if (r_iter != p_tablemap->r$()->end() && !_s_limit_info.found_sblind)
		{
			ProcessRegion(r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			text.Remove(',');
			text.Remove('$');

			if (text!="")
				set_sblind(trans.StringToMoney(text));

			write_log(3, "c0sblind, result %s", j, text.GetString());
		}

		// r$c0bblind
		r_iter = p_tablemap->r$()->find("c0bblind");
		if (r_iter != p_tablemap->r$()->end() && !_s_limit_info.found_bblind)
		{
			ProcessRegion(r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			text.Remove(',');
			text.Remove('$');

			if (text!="")
				set_bblind(trans.StringToMoney(text));

			write_log(3, "c0bblind, result %s", j, text.GetString());
		}

		// r$c0bigbet
		r_iter = p_tablemap->r$()->find("c0bigbet");
		if (r_iter != p_tablemap->r$()->end() && !_s_limit_info.found_bbet)
		{
			ProcessRegion(r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			text.Remove(',');
			text.Remove('$');

			if (text!="")
				set_bbet(trans.StringToMoney(text));

			write_log(3, "c0bigbet, result %s", j, text.GetString());
		}

		// r$c0ante
		r_iter = p_tablemap->r$()->find("c0ante");
		if (r_iter != p_tablemap->r$()->end() && !_s_limit_info.found_ante)
		{
			ProcessRegion(r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			text.Remove(',');
			text.Remove('$');

			if (text!="")
				set_ante(trans.StringToMoney(text));

			write_log(3, "c0ante, result %s", j, text.GetString());
		}
	}


	// see if anything changed
	log_blind_change = false;

	if (_limit_last != _s_limit_info.limit)
	{
		_limit_last = _s_limit_info.limit;
		_scrape_something_changed |= LIMITS_CHANGED;
	}

	if (_sblind_last != _s_limit_info.sblind)
	{
		_sblind_last = _s_limit_info.sblind;
		_scrape_something_changed |= LIMITS_CHANGED;
		log_blind_change = true;
	}

	if (_bblind_last != _s_limit_info.bblind)
	{
		_bblind_last = _s_limit_info.bblind;
		_scrape_something_changed |= LIMITS_CHANGED;
		log_blind_change = true;
	}

	if (_sb_bb_last != _s_limit_info.sb_bb)
	{
		_sb_bb_last = _s_limit_info.sb_bb;
		_scrape_something_changed |= LIMITS_CHANGED;
		log_blind_change = true;
	}

	if (_bb_BB_last != _s_limit_info.bb_BB)
	{
		_bb_BB_last = _s_limit_info.bb_BB;
		_scrape_something_changed |= LIMITS_CHANGED;
		log_blind_change = true;
	}

	if (_bbet_last != _s_limit_info.bbet)
	{
		_bbet_last = _s_limit_info.bbet;
		_scrape_something_changed |= LIMITS_CHANGED;
		log_blind_change = true;
	}

	if (_ante_last != _s_limit_info.ante)
	{
		_ante_last = _s_limit_info.ante;
		_scrape_something_changed |= LIMITS_CHANGED;
		log_blind_change = true;
	}

	if (_handnumber_last != _s_limit_info.handnumber)
	{
		_handnumber_last = _s_limit_info.handnumber;
		_scrape_something_changed |= LIMITS_CHANGED;
	}

	// log the stakes change
	if (log_blind_change)
	{
		write_log(1, "\n*************************************************************\n"
				  "NEW STAKES sb(%.2f) bb(%.2f) BB(%.2f) ante(%.2f)\n"
				  "*************************************************************\n",
				  _s_limit_info.sblind, _s_limit_info.bblind, 
				  _s_limit_info.bbet, _s_limit_info.ante);
	}

	__HDC_FOOTER
}

const double CScraper::GetHandnumFromString(CString t)
{
	char		newstr[256] = {0};
	int			i = 0, newpos = 0;

	// Check for bad parameters
	if (!t || t == "")
		return 0.;

	for (i=0; i<t.GetLength(); i++) 
	{
		if (t.Mid(i,1) >= "0" && t.Mid(i,1) <= "9")
		{
			newstr[newpos++] = t.Mid(i,1)[0];
		}
	}

	newstr[newpos] = 0;

	return atof(newstr);
}


const bool CScraper::BitmapsSame(HBITMAP HBitmapLeft, HBITMAP HBitmapRight) 
{
	if (HBitmapLeft == HBitmapRight)
		return true;

	if (NULL == HBitmapLeft || NULL == HBitmapRight)
		return false;


	bool bSame = false;

	HDC hdc = GetDC(NULL);
	BITMAPINFO BitmapInfoLeft = {0};
	BITMAPINFO BitmapInfoRight = {0};

	BitmapInfoLeft.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfoRight.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	if (0 != GetDIBits(hdc, HBitmapLeft, 0, 0, NULL, &BitmapInfoLeft, DIB_RGB_COLORS) &&
			0 != GetDIBits(hdc, HBitmapRight, 0, 0, NULL, &BitmapInfoRight, DIB_RGB_COLORS))
	{
		// Compare the BITMAPINFOHEADERs of the two bitmaps
		if (0 == memcmp(&BitmapInfoLeft.bmiHeader, &BitmapInfoRight.bmiHeader, sizeof(BITMAPINFOHEADER)))
		{
			// The BITMAPINFOHEADERs are the same so now compare the actual bitmap bits
			BYTE *pLeftBits = new BYTE[BitmapInfoLeft.bmiHeader.biSizeImage];
			BYTE *pRightBits = new BYTE[BitmapInfoRight.bmiHeader.biSizeImage];
			BYTE *pByteLeft = NULL;
			BYTE *pByteRight = NULL;

			PBITMAPINFO pBitmapInfoLeft = &BitmapInfoLeft;
			PBITMAPINFO pBitmapInfoRight = &BitmapInfoRight;

			// calculate the size in BYTEs of the additional
			// memory needed for the bmiColor table
			int AdditionalMemory = 0;
			switch (BitmapInfoLeft.bmiHeader.biBitCount)
			{
			case 1:
				AdditionalMemory = 1 * sizeof(RGBQUAD);
				break;
			case 4:
				AdditionalMemory = 15 * sizeof(RGBQUAD);
				break;
			case 8:
				AdditionalMemory = 255 * sizeof(RGBQUAD);
				break;
			case 16:
			case 32:
				AdditionalMemory = 2 * sizeof(RGBQUAD);
			}

			if (AdditionalMemory)
			{
				// we have to allocate room for the bmiColor table that will be
				// attached to our BITMAPINFO variables
				pByteLeft = new BYTE[sizeof(BITMAPINFO) + AdditionalMemory];
				if (pByteLeft)
				{
					memset(pByteLeft, 0, sizeof(BITMAPINFO) + AdditionalMemory);
					memcpy(pByteLeft, pBitmapInfoLeft, sizeof(BITMAPINFO));
					pBitmapInfoLeft = (PBITMAPINFO)pByteLeft;
				}

				pByteRight = new BYTE[sizeof(BITMAPINFO) + AdditionalMemory];
				if (pByteRight)
				{
					memset(pByteRight, 0, sizeof(BITMAPINFO) + AdditionalMemory);
					memcpy(pByteRight, pBitmapInfoRight, sizeof(BITMAPINFO));
					pBitmapInfoRight = (PBITMAPINFO)pByteRight;
				}
			}

			if (pLeftBits && pRightBits && pBitmapInfoLeft && pBitmapInfoRight)
			{
				// zero out the bitmap bit buffers
				memset(pLeftBits, 0, BitmapInfoLeft.bmiHeader.biSizeImage);
				memset(pRightBits, 0, BitmapInfoRight.bmiHeader.biSizeImage);

				// fill the bit buffers with the actual bitmap bits
				if (0 != GetDIBits(hdc, HBitmapLeft, 0, pBitmapInfoLeft->bmiHeader.biHeight, pLeftBits, pBitmapInfoLeft, DIB_RGB_COLORS) &&
						0 != GetDIBits(hdc, HBitmapRight, 0, pBitmapInfoRight->bmiHeader.biHeight, pRightBits, pBitmapInfoRight, DIB_RGB_COLORS))
				{
					// compare the actual bitmap bits of the two bitmaps
					bSame = 0 == memcmp(pLeftBits, pRightBits, pBitmapInfoLeft->bmiHeader.biSizeImage);
				}
			}

			// clean up
			delete[] pLeftBits;
			delete[] pRightBits;
			delete[] pByteLeft;
			delete[] pByteRight;
		}
	}

	ReleaseDC(NULL, hdc);

	return bSame;
}

void CScraper::CreateBitmaps(void)
{
	HDC				hdcScreen = CreateDC("DISPLAY", NULL, NULL, NULL);
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);

	// Whole window
	RECT			cr = {0};
	GetClientRect(pMyMainWnd->attached_hwnd(), &cr);
	_entire_window_last = CreateCompatibleBitmap(hdcScreen, cr.right, cr.bottom);
	set_entire_window_cur(CreateCompatibleBitmap(hdcScreen, cr.right, cr.bottom));

	// r$regions
	for (RMapI r_iter=p_tablemap->set_r$()->begin(); r_iter!=p_tablemap->set_r$()->end(); r_iter++)
	{
		int w = r_iter->second.right - r_iter->second.left + 1;
		int h = r_iter->second.bottom - r_iter->second.top + 1;
		r_iter->second.last_bmp = CreateCompatibleBitmap(hdcScreen, w, h);
		r_iter->second.cur_bmp = CreateCompatibleBitmap(hdcScreen, w, h);
	}

	DeleteDC(hdcScreen);
}

void CScraper::DeleteBitmaps(void)
{
	// Whole window
	DeleteObject(_entire_window_last);
	delete_entire_window_cur();

	// Common cards
	for (RMapI r_iter=p_tablemap->set_r$()->begin(); r_iter!=p_tablemap->set_r$()->end(); r_iter++)
	{
		DeleteObject(r_iter->second.last_bmp); r_iter->second.last_bmp=NULL;
		DeleteObject(r_iter->second.cur_bmp); r_iter->second.cur_bmp=NULL;
	}
}

void CScraper::SetLockedBlinds(const SLockBlinds LB)
{
	set_LB_blinds_are_locked(LB.blinds_are_locked);
	set_LB_sblind(LB.sblind);
	set_LB_bblind(LB.bblind);
	set_LB_bbet(LB.bbet);
	set_LB_ante(LB.ante);
	set_LB_gametype(LB.gametype);
}

void CScraper::SetLimitInfo(const SLimitInfo LI)
{
	set_handnumber(LI.handnumber);
	set_sblind(LI.sblind);
	set_bblind(LI.bblind);
	set_bbet(LI.bbet);
	set_ante(LI.ante);
	set_limit(LI.limit);
	set_istournament(LI.istournament);
	set_found_sblind(LI.found_sblind);
	set_found_bblind(LI.found_bblind);
	set_found_bbet(LI.found_bbet);
	set_found_ante(LI.found_ante);
	set_found_limit(LI.found_limit);
}

// This is the chip scrape routine
const double CScraper::DoChipScrape(RMapCI r_iter)
{
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);
	HDC				hdc = GetDC(pMyMainWnd->attached_hwnd());

	int				j = 0, stackindex = 0, chipindex = 0;
	int				hash_type = 0, pixcount = 0, chipwidth = 0, chipheight = 0;
	int				top = 0, bottom = 0, left = 0, right = 0;
	bool			stop_loop = false;
	uint32_t		*uresult = NULL, hash = 0, pix[MAX_HASH_WIDTH*MAX_HASH_HEIGHT] = {0};
	double			result = 0;
	CString			resstring = "";

	CString			type = "";
	int				vertcount = 0, horizcount = 0;
	RMapCI			r_start = p_tablemap->r$()->end();
	RMapCI			r_vert[10];
	RMapCI			r_horiz[10];
	CString			s = "";

	// Initialize arrays
	for (j=0; j<10; j++)
	{
		r_vert[j] = p_tablemap->r$()->end();
		r_horiz[j] = p_tablemap->r$()->end();
	}

	// Check for bad parameters
	if (r_iter == p_tablemap->r$()->end())
	{
		ReleaseDC(pMyMainWnd->attached_hwnd(), hdc);
		return 0.;
	}

	// figure out if we are dealing with a pot or playerbet here
	if (r_iter->second.name.Mid(0,5)=="c0pot" && r_iter->second.name.Mid(6,4)=="chip")
		type = r_iter->second.name.Mid(0,10);

	else if (r_iter->second.name.Mid(0,1)=="p" && r_iter->second.name.Mid(2,4)=="chip")
		type = r_iter->second.name.Mid(0,6);

	else
	{
		ReleaseDC(pMyMainWnd->attached_hwnd(), hdc);
		return 0.;
	}

	// find start, vert stride, and horiz stride regions
	s.Format("%s00", type.GetString());
	r_start = p_tablemap->r$()->find(s.GetString());
	if (r_start == p_tablemap->r$()->end())
	{
		ReleaseDC(pMyMainWnd->attached_hwnd(), hdc);
		return 0.;
	}

	for (j = 1; j<=9; j++)
	{
		s.Format("%s0%d", type.GetString(), j);
		r_vert[j] = p_tablemap->r$()->find(s.GetString());
		if (r_vert[j] != p_tablemap->r$()->end())
			vertcount++;

		s.Format("%s%d0", type.GetString(), j);
		r_horiz[j] = p_tablemap->r$()->find(s.GetString());
		if (r_horiz[j] != p_tablemap->r$()->end())
			horizcount++;
	}

	hash_type = r_start->second.transform.GetString()[1] - '0';

	// Bitblt the attached windows bitmap into a HDC
	HDC hdcScreen = CreateDC("DISPLAY", NULL, NULL, NULL);
	HDC hdcCompat = CreateCompatibleDC(hdcScreen);
	RECT rect;
	GetClientRect(pMyMainWnd->attached_hwnd(), &rect);
	HBITMAP attached_bitmap = CreateCompatibleBitmap(hdcScreen, rect.right, rect.bottom);
	HBITMAP	old_bitmap = (HBITMAP) SelectObject(hdcCompat, attached_bitmap);
	BitBlt(hdcCompat, 0, 0, rect.right, rect.bottom, hdc, 0, 0, SRCCOPY);

	stop_loop = false;
	// loop through horizontal stacks
	for (stackindex=0; stackindex<MAX_CHIP_STACKS && !stop_loop; stackindex++)
	{
		// loop through vertical chips
		for (chipindex=0; chipindex<MAX_CHIPS_PER_STACK && !stop_loop; chipindex++)
		{

			// figure out left, right, top, bottom of next chip to be scraped
			if (vertcount==1)
			{
				top = r_start->second.top + chipindex*(r_vert[1]->second.top - r_start->second.top);
				bottom = r_start->second.bottom + chipindex*(r_vert[1]->second.bottom - r_start->second.bottom);
			}
			else
			{
				if (r_vert[chipindex] == p_tablemap->r$()->end())
				{
					stop_loop = true;
				}
				else
				{
					top = r_vert[chipindex]->second.top;
					bottom = r_vert[chipindex]->second.bottom;
				}
			}
			if (horizcount==1)
			{
				left = r_start->second.left + stackindex*(r_horiz[1]->second.left - r_start->second.left);
				right = r_start->second.right + stackindex*(r_horiz[1]->second.right - r_start->second.right);
			}
			else
			{
				if (r_horiz[stackindex] == p_tablemap->r$()->end())
				{
					stop_loop = true;
				}
				else
				{
					left = r_horiz[stackindex]->second.left;
					right = r_horiz[stackindex]->second.right;
				}
			}

			if (!stop_loop)
			{
				chipwidth = right - left + 1;
				chipheight = bottom - top + 1;

				// calculate hash
				if (hash_type>=1 && hash_type<=3)
				{
					pixcount = 0;
					for (PMapCI p_iter=p_tablemap->p$(hash_type)->begin(); p_iter!=p_tablemap->p$(hash_type)->end(); p_iter++)
					{
							int x = p_iter->second.x;
							int y = p_iter->second.y;

							if (x<chipwidth && y<chipheight)
								pix[pixcount++] = GetPixel(hdcCompat, left + x, top + y);
					}

					if (hash_type==1) hash = hashword(&pix[0], pixcount, HASH_SEED_1);
					else if (hash_type==2) hash = hashword(&pix[0], pixcount, HASH_SEED_2);
					else if (hash_type==3) hash = hashword(&pix[0], pixcount, HASH_SEED_3);
				}

				// lookup hash in h$ records
				HMapCI h_iter = p_tablemap->h$(hash_type)->find(hash);

				// no hash match
				if (h_iter == p_tablemap->h$(hash_type)->end())
				{
					// Stop horizontal scrape loop if chipindex==0 AND a non-match
					if (chipindex==0)
						stackindex = MAX_CHIP_STACKS+1;

					// stop vertical scrape loop on a non-match
					chipindex = MAX_CHIPS_PER_STACK+1;
				}
				// hash match found
				else
				{
					resstring = h_iter->second.name;
					resstring.Remove(',');
					resstring.Remove('$');
					result += atof(resstring.GetString());
				}
			}
		}
	}

	SelectObject(hdcCompat, old_bitmap);
	DeleteObject(attached_bitmap);
	DeleteDC(hdcCompat);
	DeleteDC(hdcScreen);

	ReleaseDC(pMyMainWnd->attached_hwnd(), hdc);
	return result;
}

const bool CScraper::GetButtonState(int button_index)
{
	CString l_button_state = "";
	
	bool sym_ismanual = (bool) p_symbols->sym()->ismanual;

	if (button_index<=9)
	{
		// don't MakeLower our mm_network symbol
		if (!sym_ismanual || button_index!=5)
		{
			l_button_state = _button_state[button_index];

			if (l_button_state.MakeLower().Left(4) == "true" ||
				l_button_state.MakeLower().Left(2) == "on" ||
				l_button_state.MakeLower().Left(3) == "yes" ||
				l_button_state.MakeLower().Left(7) == "checked" ||
				l_button_state.MakeLower().Left(3) == "lit" )
			{
				return true;
			}
		}
	}
	else if (button_index==86)
	{
		l_button_state = _i86_button_state;

		if (l_button_state.MakeLower().Left(4) == "true" ||
			l_button_state.MakeLower().Left(2) == "on" ||
			l_button_state.MakeLower().Left(3) == "yes" ||
			l_button_state.MakeLower().Left(7) == "checked" ||
			l_button_state.MakeLower().Left(3) == "lit" )
		{
			return true;
		}
	}
	else if (button_index>=860)
	{
		l_button_state = _i86X_button_state[button_index-860];

		if (l_button_state.MakeLower().Left(4) == "true" ||
			l_button_state.MakeLower().Left(2) == "on" ||
			l_button_state.MakeLower().Left(3) == "yes" ||
			l_button_state.MakeLower().Left(7) == "checked" ||
			l_button_state.MakeLower().Left(3) == "lit" )
		{
			return true;
		}
	}

	return false;
}

const bool CScraper::IsNumeric(CString t)
{
	int i = 0, num_dots = 0, nums_after_dot = 0;

	// Check for bad parameters
	if (!t || t == "")
		return false;

	for (i=0; i<t.GetLength(); i++)
	{
		if (t.Mid(i,1).FindOneOf("$0123456789,.Â¢ckm") == -1)
			return false;

		if (t.Mid(i,1)==".")
			num_dots++;

		if (num_dots>0 && t.Mid(i,1).FindOneOf("0123456789") != -1)
			nums_after_dot++;
	}

	if (num_dots!=0 && num_dots!=1)
		return false;

	if (num_dots>0 && nums_after_dot!=2)
		return false;

	return true;
}

const bool CScraper::IsStringAllin(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	s.Remove(' ');
	s.Remove('-');

	if (s.MakeLower().Left(5) == "allin" ||
		s.MakeLower().Left(5) == "a11in" ||
		s.MakeLower().Left(5) == "allln" ||
		s.MakeLower().Left(5) == "a111n" ||
		s.MakeLower().Left(5) == "aiiin")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringRaise(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	if (s.MakeLower().Left(5) == "raise" ||
		s.MakeLower().Left(5) == "ra1se" ||
		s.MakeLower().Left(5) == "ralse" ||
		s.MakeLower().Left(3) == "bet")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringCall(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	if (s.MakeLower().Left(4) == "call" ||
		s.MakeLower().Left(4) == "caii" ||
		s.MakeLower().Left(4) == "ca11")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringCheck(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	if (s.MakeLower().Left(5) == "check" ||
		s.MakeLower().Left(5) == "cheok")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringFold(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	if (s.MakeLower().Left(4) == "fold" ||
		s.MakeLower().Left(4) == "fo1d" ||
		s.MakeLower().Left(4) == "foid")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringAutopost(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	s.Remove(' ');
	s.Remove('-');

	if (s.MakeLower().Left(8) == "autopost" ||
		s.MakeLower().Left(8) == "aut0p0st")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringSitin(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	s.Remove(' ');
	s.Remove('-');

	if (s.MakeLower().Left(5) == "sitin" ||
		s.MakeLower().Left(5) == "s1t1n")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringSitout(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	s.Remove(' ');
	s.Remove('-');

	if (s.MakeLower().Left(6) == "sitout" ||
		s.MakeLower().Left(6) == "s1tout" ||
		s.MakeLower().Left(6) == "sit0ut" ||
		s.MakeLower().Left(6) == "s1t0ut")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringLeave(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	if (s.MakeLower().Left(5) == "leave")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringPrefold(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	if (s.MakeLower().Left(7) == "prefold")
	{
		return true;
	}

	return false;
}


const bool CScraper::IsStringSeated(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	if (s.MakeLower().Left(5) == "false" ||
		s.MakeLower().Left(8) == "unseated")
	{
		return false;
	}

	else if (s.MakeLower().Left(4) == "true" ||
			 s.MakeLower().Left(6) == "seated")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringActive(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return p_tablemap->activemethod() == 2 ? true : false;

	// new method: active unless pXactive returns false/inactive/out/away
	if (p_tablemap->activemethod() == 2)
	{
		if (s.MakeLower().Left(5) == "false" ||
			s.MakeLower().Left(8) == "inactive" ||
			s.MakeLower().Left(3) == "out" ||
			s.MakeLower().Left(4) == "away")
		{
			return false;
		}

		return true;
	}
	// old method: inactive unless pXactive returns true/active
	else
	{
		if (s=="")
			return false;

		if (s.MakeLower().Left(8) == "inactive")
		{
			return false;
		}

		if (s.MakeLower().Left(4) == "true" ||
			s.MakeLower().Left(6) == "active")
		{
			return true;
		}

		return false;
	}
}

const bool CScraper::IsStringCardback(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	if (s.MakeLower().Left(4) == "true" ||
		s.MakeLower().Left(8) == "cardback")
	{
		return true;
	}

	return false;
}

const bool CScraper::IsStringDealer(CString s)
{
	// Check for bad parameters
	if (!s || s == "")
		return false;

	if (s.MakeLower().Left(4) == "true" ||
		s.MakeLower().Left(6) == "dealer")
	{
		return true;
	}

	return false;
}

#undef __HDC_HEADER
#undef __HDC_FOOTER