#include "StdAfx.h"

#include "CScraper.h"

#include "..\CTransform\CTransform.h"
#include "..\CTransform\hash\lookup3.h"

#include "CSymbols.h"
#include "CPokerPro.h"

#include "OpenHoldem.h"
#include "MainFrm.h"

CScraper			*p_scraper = NULL;
CRITICAL_SECTION	CScraper::cs_scraper;

CScraper::CScraper(void)
{
	InitializeCriticalSectionAndSpinCount(&cs_scraper, 4000);

	ClearScrapeAreas();

	_s_lock_blinds.blinds_are_locked = false;
}

CScraper::~CScraper(void)
{
	DeleteCriticalSection(&cs_scraper);
}

void CScraper::ClearScrapeAreas(void)
{
	EnterCriticalSection(&cs_scraper);

		int i;

		for (i=0; i<=4; i++)
			_card_common[i] = CARD_NOCARD;

		for (i=0; i<=9; i++)
		{
			_card_player[i][0] = CARD_NOCARD;
			_card_player[i][1] = CARD_NOCARD;
			_seated[i] = "false";
			_active[i] = "false";
			_dealer[i] = false;
			_player_bet[i] = 0.0;
			_player_name[i] = "";
			_name_good_scrape[i] = false;
			_player_balance[i] = 0.0;
			_balance_good_scrape[i] = false;
			_i86X_button_state[i] = "false";
			_button_state[i] = "false";
			_button_label[i] = "";
			_pot[i] = 0.;
		}
		_i86_button_state = "false";

		_button_label[0] = "fold";
		_button_label[1] = "call";
		_button_label[2] = "raise";
		_button_label[3] = "allin";

		_s_limit_info.istournament = false;
		_s_limit_info.handnumber = 0;
		_s_limit_info.sblind = 0;
		_s_limit_info.bblind = 0;
		_s_limit_info.bbet = 0;
		_s_limit_info.ante = 0;
		_s_limit_info.sb_bb = 0;
		_s_limit_info.bb_BB = 0;
		_s_limit_info.limit = -1;
		_s_limit_info.found_handnumber = false;
		_s_limit_info.found_sblind = false;
		_s_limit_info.found_bblind = false;
		_s_limit_info.found_bbet = false;
		_s_limit_info.found_ante = false;
		_s_limit_info.found_limit = false;
		_s_limit_info.found_sb_bb = false;
		_s_limit_info.found_bb_BB = false;

		_card_player_for_display[0] = CARD_NOCARD;
		_card_player_for_display[1] = CARD_NOCARD;

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

	LeaveCriticalSection(&cs_scraper);
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
	int				i = 0;
	CString			text = "";
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);

	// DC
	HDC			hdc = GetDC(pMyMainWnd->attached_hwnd());
	HDC			hdcScreen = CreateDC("DISPLAY", NULL, NULL, NULL);
	HDC			hdcCompatible = CreateCompatibleDC(hdcScreen);
	RECT		cr = {0};
	HBITMAP		old_bitmap = NULL;

	// Get bitmap of whole window
	GetClientRect(pMyMainWnd->attached_hwnd(), &cr);

	old_bitmap = (HBITMAP) SelectObject(hdcCompatible, _entire_window_cur);

	BitBlt(hdcCompatible, 0, 0, cr.right-cr.left, cr.bottom-cr.top, hdc, cr.left, cr.top, SRCCOPY);
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
		return NOTHING_CHANGED;
	}

	// Copy into "last" title
	if (strcmp(_title, _title_last)!=0)
	{
		EnterCriticalSection(&cs_scraper);
			strcpy_s(_title_last, 512, _title);
			_s_limit_info.found_handnumber = false;
			_s_limit_info.found_sblind = false;
			_s_limit_info.found_bblind = false;
			_s_limit_info.found_bbet = false;
			_s_limit_info.found_ante = false;
			_s_limit_info.found_sb_bb = false;
			_s_limit_info.found_bb_BB = false;
			_s_limit_info.found_limit = false;
		LeaveCriticalSection(&cs_scraper);

		p_symbols->set_reset_stakes(true);
	}

	// Copy into "last" bitmap
	old_bitmap = (HBITMAP) SelectObject(hdcCompatible, _entire_window_last);
	BitBlt(hdcCompatible, 0, 0, cr.right-cr.left, cr.bottom-cr.top, hdc, cr.left, cr.top, SRCCOPY);
	SelectObject(hdc, old_bitmap);

	if (_ucf_last != p_symbols->user_chair_confirmed())
		_ucf_last = p_symbols->user_chair_confirmed();

	// flag for detecting if anything useful has changed - used downstream to decide whether or not to
	// update symbols, etc.
	EnterCriticalSection(&cs_scraper);
	_scrape_something_changed = NOTHING_CHANGED;
	LeaveCriticalSection(&cs_scraper);

	// Common cards
	ScrapeCommonCards(hdcCompatible, hdc);

	// Player information
	for (i=0; i<=9; i++)
	{
		ScrapePlayerCards(i, hdcCompatible, hdc);
		ScrapeSeated(i, hdcCompatible, hdc);
		ScrapeActive(i, hdcCompatible, hdc);
		ScrapeDealer(i, hdcCompatible, hdc);
		ScrapeName(i, hdcCompatible, hdc);
		ScrapeBalance(i, hdcCompatible, hdc);
		ScrapeBet(i, hdcCompatible, hdc);

		if (!IsStringSeated(_seated[i]) && !IsStringActive(_active[i]))
		{
			EnterCriticalSection(&cs_scraper);
				_player_name[i] = "";
				_name_good_scrape[i] = false;
				_player_balance[i] = 0;
				_balance_good_scrape[i] = false;
			LeaveCriticalSection(&cs_scraper);
		}
	}

	ScrapeButtons(hdcCompatible, hdc);		// Buttons
	ScrapePots(hdcCompatible, hdc);		// Pots
	ScrapeLimits(hdcCompatible, hdc);		// limits

	DeleteDC(hdcCompatible);
	DeleteDC(hdcScreen);
	ReleaseDC(pMyMainWnd->attached_hwnd(), hdc);

	return _scrape_something_changed;
}

bool CScraper::ProcessRegion(const HDC hdcCompatible, const HDC hdc, RMapCI r_iter)
{
	// Check for bad parameters
	if (hdcCompatible == NULL || hdc == NULL)
		return false;

	// Get "current" bitmap
	HBITMAP old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
	BitBlt(hdcCompatible, 0, 0, r_iter->second.right - r_iter->second.left, 
							    r_iter->second.bottom - r_iter->second.top, 
								hdc, r_iter->second.left, r_iter->second.top, SRCCOPY);
	SelectObject(hdcCompatible, old_bitmap);

	// If the bitmaps are different, then continue on
	if (!BitmapsSame(r_iter->second.last_bmp, r_iter->second.cur_bmp))
	{

		// Copy into "last" bitmap
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.last_bmp);
		BitBlt(hdcCompatible, 0, 0, r_iter->second.right - r_iter->second.left, 
									r_iter->second.bottom - r_iter->second.top, 
									hdc, r_iter->second.left, r_iter->second.top, SRCCOPY);
		SelectObject(hdcCompatible, old_bitmap);

		return true;
	}

	return false;
}

void CScraper::ScrapeCommonCards(HDC hdcCompatible, HDC hdc)
{
	HBITMAP				old_bitmap = NULL;
	unsigned int		card = CARD_NOCARD;
	CString				cardstr = "", rankstr = "", suitstr = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter1 = p_tablemap->r$()->end(), r_iter2 = p_tablemap->r$()->end();

	// Check for bad parameters
	if (hdcCompatible == NULL || hdc == NULL)
		return;

	for (int i=0; i<=4; i++)
	{
		card = CARD_NOCARD;

		// try r$c0cardfaceX region first
		s.Format("c0cardface%d", i);
		r_iter1 = p_tablemap->r$()->find(s.GetString());
		if (r_iter1 != p_tablemap->r$()->end())
		{
			ProcessRegion(hdcCompatible, hdc, r_iter1);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
			trans.DoTransform(r_iter1, hdcCompatible, &cardstr);
			SelectObject(hdcCompatible, old_bitmap);

			if (cardstr!="")
				StdDeck_stringToCard((char *) cardstr.GetString(), (int *) &card);
			else
				card = CARD_NOCARD;

			EnterCriticalSection(&cs_scraper);
				if (_card_common_last[i] != card)
				{
					_card_common_last[i] = card;
					_scrape_something_changed |= COM_CARD_CHANGED;
				}

				_card_common[i] = card;
			LeaveCriticalSection(&cs_scraper);
		}

		// try r$c0cardfaceXrank/r$c0cardfaceXsuit region next
		s.Format("c0cardface%drank", i);
		r_iter1 = p_tablemap->r$()->find(s.GetString());

		s.Format("c0cardface%dsuit", i);
		r_iter2 = p_tablemap->r$()->find(s.GetString());

		if (r_iter1 != p_tablemap->r$()->end() && r_iter2 != p_tablemap->r$()->end() && card==CARD_NOCARD)
		{
			// Get rank
			ProcessRegion(hdcCompatible, hdc, r_iter1);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
			trans.DoTransform(r_iter1, hdcCompatible, &rankstr);
			SelectObject(hdcCompatible, old_bitmap);

			// Get suit
			ProcessRegion(hdcCompatible, hdc, r_iter2);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter2->second.cur_bmp);
			trans.DoTransform(r_iter2, hdcCompatible, &suitstr);
			SelectObject(hdcCompatible, old_bitmap);

			if (rankstr=="10") rankstr="T";

			if (rankstr!="" && suitstr!="")
			{
				cardstr = rankstr + suitstr;
				StdDeck_stringToCard((char *) cardstr.GetString(), (int *) &card);
			}

			EnterCriticalSection(&cs_scraper);
				if (_card_common_last[i] != card)
				{
					_card_common_last[i] = card;
					_scrape_something_changed |= COM_CARD_CHANGED;
				}

				_card_common[i] = card;
			LeaveCriticalSection(&cs_scraper);
		}
	}
}

void CScraper::ScrapePlayerCards(int chair, HDC hdcCompatible, HDC hdc)
{
	int					j = 0;
	HBITMAP				old_bitmap = NULL;
	unsigned int		card = CARD_NOCARD;
	bool				got_new_scrape = false;
	CString				cardstr = "", rankstr = "", suitstr = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter1 = p_tablemap->r$()->end(), r_iter2 = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs() || hdcCompatible == NULL || hdc == NULL)
		return;

	int sym_userchair = (int) p_symbols->sym()->userchair;

	// Player cards
	got_new_scrape = false;
	for (j=0; j<=1; j++)
	{
		card = CARD_NOCARD;

		// try r$uXcardfaceY region first
		s.Format("u%dcardface%d", chair, j);
		r_iter1 = p_tablemap->r$()->find(s.GetString());
		if (r_iter1 != p_tablemap->r$()->end())
		{
			ProcessRegion(hdcCompatible, hdc, r_iter1);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
			trans.DoTransform(r_iter1, hdcCompatible, &cardstr);
			SelectObject(hdcCompatible, old_bitmap);

			if (cardstr!="")
			{
				StdDeck_stringToCard((char *) cardstr.GetString(), (int *) &card);

				EnterCriticalSection(&cs_scraper);
					if (_card_player_last[chair][j] != card)
					{
						_card_player_last[chair][j] = card;
						_scrape_something_changed |= PL_CARD_CHANGED;
					}

					_card_player[chair][j] = card;

					if (chair==sym_userchair)
						_card_player_for_display[j] = card;
				LeaveCriticalSection(&cs_scraper);
			}
		}

		// try r$pXcardfaceY region next
		s.Format("p%dcardface%d", chair, j);
		r_iter1 = p_tablemap->r$()->find(s.GetString());
		if (r_iter1 != p_tablemap->r$()->end())
		{
			ProcessRegion(hdcCompatible, hdc, r_iter1);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
			trans.DoTransform(r_iter1, hdcCompatible, &cardstr);
			SelectObject(hdcCompatible, old_bitmap);

			if (cardstr!="" && card==CARD_NOCARD)
			{
				StdDeck_stringToCard((char *) cardstr.GetString(), (int *) &card);

				EnterCriticalSection(&cs_scraper);
					if (_card_player_last[chair][j] != card)
					{
						_card_player_last[chair][j] = card;
						_scrape_something_changed |= PL_CARD_CHANGED;
					}

					_card_player[chair][j] = card;

					if (chair==sym_userchair)
						_card_player_for_display[j] = card;
				LeaveCriticalSection(&cs_scraper);
			}
		}

		// try r$pXcardfaceYrank/r$pXcardfaceYsuit regions next
		s.Format("p%dcardface%drank", chair, j);
		r_iter1 = p_tablemap->r$()->find(s.GetString());

		s.Format("p%dcardface%dsuit", chair, j);
		r_iter2 = p_tablemap->r$()->find(s.GetString());

		if (r_iter1!=p_tablemap->r$()->end() && r_iter2!=p_tablemap->r$()->end() && card==CARD_NOCARD)
		{
			// Get rank
			ProcessRegion(hdcCompatible, hdc, r_iter1);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
			trans.DoTransform(r_iter1, hdcCompatible, &rankstr);
			SelectObject(hdcCompatible, old_bitmap);

			// Get suit
			ProcessRegion(hdcCompatible, hdc, r_iter2);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter2->second.cur_bmp);
			trans.DoTransform(r_iter2, hdcCompatible, &suitstr);
			SelectObject(hdcCompatible, old_bitmap);

			if (rankstr=="10") rankstr="T";

			if (rankstr!="" && suitstr!="")
			{
				cardstr = rankstr + suitstr;
				StdDeck_stringToCard((char *) cardstr.GetString(), (int *) &card);

				EnterCriticalSection(&cs_scraper);
					if (_card_player_last[chair][j] != card)
					{
						_card_player_last[chair][j] = card;
						_scrape_something_changed |= PL_CARD_CHANGED;
					}

					_card_player[chair][j] = card;

					if (chair==sym_userchair)
						_card_player_for_display[j] = card;
				LeaveCriticalSection(&cs_scraper);
			}
		}
	}

	// Player card backs pXcardback
	s.Format("p%dcardback", chair);
	r_iter1 = p_tablemap->r$()->find(s.GetString());
	if (r_iter1 != p_tablemap->r$()->end() && card==CARD_NOCARD)
	{
		ProcessRegion(hdcCompatible, hdc, r_iter1);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter1->second.cur_bmp);
		trans.DoTransform(r_iter1, hdcCompatible, &cardstr);
		SelectObject(hdcCompatible, old_bitmap);

		EnterCriticalSection(&cs_scraper);
			if (IsStringCardback(cardstr))
			{
				_card_player[chair][0] = CARD_BACK;
				_card_player[chair][1] = CARD_BACK;
			}
			else
			{
				_card_player[chair][0] = CARD_NOCARD;
				_card_player[chair][1] = CARD_NOCARD;
			}

			if (_card_player_last[chair][0] != _card_player[chair][0] ||
				_card_player_last[chair][1] != _card_player[chair][1] )
			{
				_card_player_last[chair][0] = _card_player[chair][0] ;
				_card_player_last[chair][1] = _card_player[chair][1];
				_scrape_something_changed |= PL_CARD_CHANGED;
			}
		LeaveCriticalSection(&cs_scraper);
	}
}

void CScraper::ScrapeSeated(int chair, HDC hdcCompatible, HDC hdc)
{
	HBITMAP				old_bitmap = NULL;
	CString				result = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs() || hdcCompatible == NULL || hdc == NULL)
		return;

	_seated[chair] = "false";

	// try p region first pXseated
	s.Format("p%dseated", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		EnterCriticalSection(&cs_scraper);
			if (result!="")
				_seated[chair] = result;
		LeaveCriticalSection(&cs_scraper);
	}

	// try u region next uXseated,
	// but only if we didn't get a positive result from the p region
	s.Format("u%dseated", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !IsStringSeated(_seated[chair]))
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		EnterCriticalSection(&cs_scraper);
			if (result!="")
				_seated[chair] = result;
		LeaveCriticalSection(&cs_scraper);
	}

	EnterCriticalSection(&cs_scraper);
		if (_seated_last[chair] != _seated[chair])
		{
			_seated_last[chair] = _seated[chair];
			_scrape_something_changed |= SEATED_CHANGED;
		}
	LeaveCriticalSection(&cs_scraper);
}

void CScraper::ScrapeActive(int chair, HDC hdcCompatible, HDC hdc)
{
	HBITMAP				old_bitmap = NULL;
	CString				result = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs() || hdcCompatible == NULL || hdc == NULL)
		return;

	_active[chair] = "false";

	// try p region first pXactive
	s.Format("p%dactive", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		EnterCriticalSection(&cs_scraper);
		_active[chair] = result;
		LeaveCriticalSection(&cs_scraper);
	}

	// try u region next, but only if we didn't get a key result from the p region
	s.Format("u%dactive", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && 
		((!IsStringActive(_active[chair]) && p_tablemap->activemethod() != 2) ||
		 (IsStringActive(_active[chair]) && p_tablemap->activemethod() == 2) ) )
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		EnterCriticalSection(&cs_scraper);
		_active[chair] = result;
		LeaveCriticalSection(&cs_scraper);
	}

	EnterCriticalSection(&cs_scraper);
		if (_active_last[chair] != _active[chair])
		{
			_active_last[chair] = _active[chair];
			_scrape_something_changed |= ACTIVE_CHANGED;
		}
	LeaveCriticalSection(&cs_scraper);
}

void CScraper::ScrapeDealer(int chair, HDC hdcCompatible, HDC hdc)
{
	HBITMAP				old_bitmap = NULL;
	CString				result = "";
	CTransform			trans;
	bool				found_dealer;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs() || hdcCompatible == NULL || hdc == NULL)
		return;

	_dealer[chair] = false;

	// Dealer button pXdealer
	found_dealer = false;

	s.Format("p%ddealer", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		if (IsStringDealer(result))
			found_dealer = true;
	}

	// Dealer button uXdealer
	s.Format("u%ddealer", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !found_dealer)
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &result);
		SelectObject(hdcCompatible, old_bitmap);

		if (IsStringDealer(result))
			found_dealer = true;
	}

	EnterCriticalSection(&cs_scraper);
		_dealer[chair] = found_dealer;

		if (_dealer_last[chair] != _dealer[chair])
		{
			_dealer_last[chair] = _dealer[chair];
			_scrape_something_changed |= DEALER_CHANGED;
		}
	LeaveCriticalSection(&cs_scraper);
}

void CScraper::ScrapeName(int chair, HDC hdcCompatible, HDC hdc) 
{

	HBITMAP				old_bitmap = NULL;
	bool				got_new_scrape = false;
	CString				text = "";
	int					ret = 0;
	bool				is_seated = IsStringSeated(_seated[chair]);
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs() || hdcCompatible == NULL || hdc == NULL)
		return;

	int	sym_chair = (int) p_symbols->sym()->chair;

	got_new_scrape = false;

	// Player name uname
	r_iter = p_tablemap->r$()->find("uname");
	if (r_iter != p_tablemap->r$()->end() && p_symbols->user_chair_confirmed() && chair==sym_chair && is_seated)
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret==ERR_GOOD_SCRAPE_GENERAL && text!="")
			got_new_scrape = true;
	}

	// Player name uXname
	s.Format("u%dname", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !got_new_scrape && p_symbols->user_chair_confirmed() && chair==sym_chair && is_seated)
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret==ERR_GOOD_SCRAPE_GENERAL && text!="")
			got_new_scrape = true;
	}

	// Player name pXname
	s.Format("p%dname", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !got_new_scrape && is_seated)
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret==ERR_GOOD_SCRAPE_GENERAL && text!="")
			got_new_scrape = true;
	}

	EnterCriticalSection(&cs_scraper);
		if (got_new_scrape)
		{
			_name_good_scrape[chair] = true;
			_player_name[chair] = text;
		}
		else
		{
			_name_good_scrape[chair] = false;
		}

		if (_name_last[chair] != _player_name[chair])
		{
			_name_last[chair] = _player_name[chair];
			_scrape_something_changed |= NAME_CHANGED;
		}
	LeaveCriticalSection(&cs_scraper);
}

void CScraper::ScrapeBalance(int chair, HDC hdcCompatible, HDC hdc)
{
	HBITMAP				old_bitmap = NULL;
	bool				got_new_scrape = false;
	CString				text = "";
	int					ret = 0;
	bool				is_seated = IsStringSeated(_seated[chair]);
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs() || hdcCompatible == NULL || hdc == NULL)
		return;

	int	sym_chair = (int) p_symbols->sym()->chair;

	got_new_scrape = false;

	_sitting_out[chair] = false;

	// Player balance ubalance
	r_iter = p_tablemap->r$()->find("ubalance");
	if (r_iter != p_tablemap->r$()->end() && p_symbols->user_chair_confirmed() && chair==sym_chair && is_seated)
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret == ERR_GOOD_SCRAPE_GENERAL)
		{
			if (IsStringAllin(text))
			{
				got_new_scrape = true;
				text = "0";
			}

			else if (text.MakeLower().Find("out")!=-1)
			{
				EnterCriticalSection(&cs_scraper);
				_sitting_out[chair] = true;
				LeaveCriticalSection(&cs_scraper);
			}

			else
			{
				text.Remove(',');
				text.Remove('$');

				if (text!="" && IsNumeric(text))
					got_new_scrape = true;
			}
		}
	}

	// Player name uXbalance
	s.Format("u%dbalance", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !got_new_scrape && p_symbols->user_chair_confirmed() && chair==sym_chair && is_seated)
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret == ERR_GOOD_SCRAPE_GENERAL)
		{
			if (IsStringAllin(text))
			{
				got_new_scrape = true;
				text = "0";
			}

			else if (text.MakeLower().Find("out")!=-1)
			{
				EnterCriticalSection(&cs_scraper);
  				_sitting_out[chair] = true;
				LeaveCriticalSection(&cs_scraper);
			}

			else
			{
				text.Remove(',');
				text.Remove('$');

				if (text!="" && IsNumeric(text))
					got_new_scrape = true;
			}
		}
	}

	// Player balance pXbalance
	s.Format("p%dbalance", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && !got_new_scrape && is_seated)
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		ret = trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (ret == ERR_GOOD_SCRAPE_GENERAL)
		{
			if (IsStringAllin(text))
			{
				got_new_scrape = true;
				text = "0";
			}

			else if (text.MakeLower().Find("out")!=-1)
			{
				EnterCriticalSection(&cs_scraper);
  				_sitting_out[chair] = true;
				LeaveCriticalSection(&cs_scraper);
			}

			else
			{
				text.Remove(',');
				text.Remove('$');

				if (text!="" && IsNumeric(text))
					got_new_scrape = true;
			}
		}
	}

	EnterCriticalSection(&cs_scraper);
		if (got_new_scrape)
		{
			_balance_good_scrape[chair] = true;
			_player_balance[chair] = trans.StringToMoney(text);
		}
		else
		{
			_balance_good_scrape[chair] = false;
		}

		if (_balance_last[chair] != _player_balance[chair])
		{
			_balance_last[chair] = _player_balance[chair];
			_scrape_something_changed |= BALANCE_CHANGED;
		}
	LeaveCriticalSection(&cs_scraper);
}

void CScraper::ScrapeBet(int chair, HDC hdcCompatible, HDC hdc)
{
	HBITMAP				old_bitmap = NULL;
	CString				text = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (chair < 0 || chair >= p_tablemap->nchairs() || hdcCompatible == NULL || hdc == NULL)
		return;

	_player_bet[chair] = 0;

	// Player bet pXbet
	s.Format("p%dbet", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		text.Remove(',');
		text.Remove('$');

		EnterCriticalSection(&cs_scraper);
		if (text!="")
		{
			_player_bet[chair] = trans.StringToMoney(text);
		}
		LeaveCriticalSection(&cs_scraper);
	}

	// uXbet
	s.Format("u%dbet", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && _player_bet[chair] == 0)
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		text.Remove(',');
		text.Remove('$');

		EnterCriticalSection(&cs_scraper);
		if (text!="")
		{
			_player_bet[chair] = trans.StringToMoney(text);
		}
		LeaveCriticalSection(&cs_scraper);
	}		
		
	// pXchip00
	s.Format("p%dchp00", chair);
	r_iter = p_tablemap->r$()->find(s.GetString());
	if (r_iter != p_tablemap->r$()->end() && _player_bet[chair] == 0)
	{
		EnterCriticalSection(&cs_scraper);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, _entire_window_cur);
		_player_bet[chair] = DoChipScrape(hdcCompatible, r_iter);
		SelectObject(hdcCompatible, old_bitmap);
		LeaveCriticalSection(&cs_scraper);
	}

	EnterCriticalSection(&cs_scraper);
	if (_playerbet_last[chair] != _player_bet[chair])
	{
		_playerbet_last[chair] = _player_bet[chair];
		_scrape_something_changed |= PLAYERBET_CHANGED;
	}
	LeaveCriticalSection(&cs_scraper);
}

void CScraper::ScrapeButtons(HDC hdcCompatible, HDC hdc) 
{
	int					j = 0, k = 0;
	HBITMAP				old_bitmap = NULL;
	CString				text = "";
	POINT				handle_xy = {0};
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end(), slider = p_tablemap->r$()->end(), handleCI = p_tablemap->r$()->end();
	RMapI				handleI = p_tablemap->set_r$()->end();

	// Check for bad parameters
	if (hdcCompatible == NULL || hdc == NULL)
		return;

	_button_label[0] = "fold";
	_button_label[1] = "call";
	_button_label[2] = "raise";
	_button_label[3] = "allin";
	for (j=4; j<=9; j++)
		_button_label[j] = "";
	for (j=0; j<=9; j++)
	{
		_button_state[j] = "false";
		_i86X_button_state[j] = "false";
	}

	for (j=0; j<=9; j++)
	{
		// Button state iXstate
		s.Format("i%dstate", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end())
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			EnterCriticalSection(&cs_scraper);
				if (text!="")
					_button_state[j] = text;

				if (_button_state_last[j] != _button_state[j])
				{
					_button_state_last[j] = _button_state[j];
					_scrape_something_changed |= BUTTONSTATE_CHANGED;
				}
			LeaveCriticalSection(&cs_scraper);
		}

		// i86X button state i86Xstate
		s.Format("i86%dstate", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end())
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			EnterCriticalSection(&cs_scraper);
				if (text!="")
					_i86X_button_state[j] = text;

				if (_i86X_button_state_last[j] != _i86X_button_state[j])
				{
					_i86X_button_state_last[j] = _i86X_button_state[j];
					_scrape_something_changed |= BUTTONSTATE_CHANGED;
				}
			LeaveCriticalSection(&cs_scraper);
		}

		// Button label
		// First check iXlabel
		text = "";

		s.Format("i%dlabel", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end())
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			EnterCriticalSection(&cs_scraper);
				if (text!="")
					_button_label[j] = text;
			LeaveCriticalSection(&cs_scraper);
		}

		// Second check iXlabelY
		for (k=0; k<=9 && text == ""; k++)
		{
			s.Format("i%dlabel%d", j, k);
			r_iter = p_tablemap->r$()->find(s.GetString());
			if (r_iter != p_tablemap->r$()->end())
			{
				ProcessRegion(hdcCompatible, hdc, r_iter);
				old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
				trans.DoTransform(r_iter, hdcCompatible, &text);
				SelectObject(hdcCompatible, old_bitmap);

				EnterCriticalSection(&cs_scraper);
					if (text!="")
						_button_label[j] = text;
				LeaveCriticalSection(&cs_scraper);
			}
		}

		EnterCriticalSection(&cs_scraper);
			if (_button_label_last[j] != _button_label[j])
			{
				_button_label_last[j] = _button_label[j];
				_scrape_something_changed |= BUTTONLABEL_CHANGED;
			}
		LeaveCriticalSection(&cs_scraper);
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
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		EnterCriticalSection(&cs_scraper);
			if (text!="")
				_i86_button_state = text;
			else
				_i86_button_state = "false";

			if (_i86_button_state_last != _i86_button_state)
			{
				_i86_button_state_last = _i86_button_state;
				_scrape_something_changed |= BUTTONSTATE_CHANGED;
			}
		LeaveCriticalSection(&cs_scraper);
	}

	// find handle
	handleCI = p_tablemap->r$()->find("i3handle");
	slider = p_tablemap->r$()->find("i3slider");

	if (handleCI!=p_tablemap->r$()->end() && slider!=p_tablemap->r$()->end() && _button_state[3]!="false")
	{
		ProcessRegion(hdcCompatible, hdc, handleCI);
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

		EnterCriticalSection(&cs_scraper);
			if (text!="" && k <= j)
			{
				handleCI = p_tablemap->r$()->find("i3handle");
				_handle_found_at_xy = true;
				handle_xy.x = handleCI->second.left + k;
				handle_xy.y = handleCI->second.top;
				_handle_xy = handle_xy;
			}
		LeaveCriticalSection(&cs_scraper);
			
		SelectObject(hdcCompatible, old_bitmap);
	}
}

void CScraper::ScrapePots(HDC hdcCompatible, HDC hdc)
{
	int					j = 0;
	HBITMAP				old_bitmap = NULL;
	CString				text = "";
	CTransform			trans;
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();

	// Check for bad parameters
	if (hdcCompatible == NULL || hdc == NULL)
		return;

	for (j=0; j<=9; j++)
		_pot[j] = 0;

	for (j=0; j<=9; j++)
	{
		// r$c0potX
		s.Format("c0pot%d", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end())
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			text.Remove(',');
			text.Remove('$');

			EnterCriticalSection(&cs_scraper);
				if (text!="")
				{
					_pot[j] = trans.StringToMoney(text);

					if (_pot_last[j] != _pot[j])
					{
						_pot_last[j] = _pot[j];
						_scrape_something_changed |= POT_CHANGED;
					}
				}
			LeaveCriticalSection(&cs_scraper);
		}

		// r$c0potXchip00_index
		s.Format("c0pot%dchip00", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end() && _pot[j] == 0)
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
			//old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			//trans.DoTransform(r_iter, hdcCompatible, &text);
			//SelectObject(hdcCompatible, old_bitmap);

			EnterCriticalSection(&cs_scraper);
				old_bitmap = (HBITMAP) SelectObject(hdcCompatible, _entire_window_cur);
				_pot[j] = DoChipScrape(hdcCompatible, r_iter);
				SelectObject(hdcCompatible, old_bitmap);

				if (_pot_last[j] != _pot[j])
				{
					_pot_last[j] = _pot[j];
					_scrape_something_changed |= POT_CHANGED;
				}
			LeaveCriticalSection(&cs_scraper);

			// update the bitmap for second chip position in the first stack
			s.Format("c0pot%dchip01", j);
			r_iter = p_tablemap->r$()->find(s.GetString());
			if (r_iter != p_tablemap->r$()->end())
				ProcessRegion(hdcCompatible, hdc, r_iter);

			// update the bitmap for first chip position in the second stack
			s.Format("c0pot%dchip10", j);
			r_iter = p_tablemap->r$()->find(s.GetString());
			if (r_iter != p_tablemap->r$()->end())
				ProcessRegion(hdcCompatible, hdc, r_iter);
		}
	}
}

void CScraper::ScrapeLimits(HDC hdcCompatible, HDC hdc)
{
	int					j = 0;
	double				handnumber = 0.;
	bool				istournament = false;
	HBITMAP				old_bitmap = NULL;
	CString				text = "";
	CString				titletext = "";
	char				c_titletext[MAX_WINDOW_TITLE] = {0};
	bool				got_new_scrape = false, log_blind_change = false;
	CTransform			trans;
	CMainFrame			*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);
	CString				s = "";
	RMapCI				r_iter = p_tablemap->r$()->end();
	SMapCI				s_iter = p_tablemap->s$()->end();

	// Check for bad parameters
	if (hdcCompatible == NULL || hdc == NULL)
		return;

	EnterCriticalSection(&cs_scraper);
		_s_limit_info.found_sblind = false;
		_s_limit_info.found_bblind = false;
		_s_limit_info.found_bbet = false;
		_s_limit_info.found_ante = false;
		_s_limit_info.found_limit = false;
		_s_limit_info.found_sb_bb = false;
		_s_limit_info.found_bb_BB = false;
	LeaveCriticalSection(&cs_scraper);

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
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		if (text!="")
			istournament = true;
		else
			istournament = false;

		EnterCriticalSection(&cs_scraper);
			_s_limit_info.istournament = istournament;

			if (_istournament_last != _s_limit_info.istournament)
			{
				_istournament_last = _s_limit_info.istournament;
				_scrape_something_changed |= LIMITS_CHANGED;
			}
		LeaveCriticalSection(&cs_scraper);
	}

	// r$c0handnumber
	r_iter = p_tablemap->r$()->find("c0handnumber");
	if (r_iter != p_tablemap->r$()->end())
	{
		ProcessRegion(hdcCompatible, hdc, r_iter);
		old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
		trans.DoTransform(r_iter, hdcCompatible, &text);
		SelectObject(hdcCompatible, old_bitmap);

		EnterCriticalSection(&cs_scraper);
			if (text!="")
			{
				handnumber = GetHandnumFromString(text);
				_s_limit_info.handnumber = handnumber;
				got_new_scrape = true;
			}
		LeaveCriticalSection(&cs_scraper);
	}

	for (j=0; j<=9; j++)
	{
		// r$c0handnumberX
		s.Format("c0handnumber%d", j);
		r_iter = p_tablemap->r$()->find(s.GetString());
		if (r_iter != p_tablemap->r$()->end() && !got_new_scrape)
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);
			
			EnterCriticalSection(&cs_scraper);
				if (text!="")
				{
					handnumber = GetHandnumFromString (text);
					_s_limit_info.handnumber = handnumber;
					got_new_scrape = true;
				}
			LeaveCriticalSection(&cs_scraper);
		}
	}

	if (_s_lock_blinds.blinds_are_locked)
	{
		EnterCriticalSection(&cs_scraper);
			_s_limit_info.sblind = _s_lock_blinds.sblind;
			_s_limit_info.found_sblind = true;
			_s_limit_info.bblind = _s_lock_blinds.bblind;
			_s_limit_info.found_bblind = true;
			_s_limit_info.bbet = _s_lock_blinds.bbet;
			_s_limit_info.found_bbet = true;
			_s_limit_info.ante = _s_lock_blinds.ante;
			_s_limit_info.found_ante = true;
			_s_limit_info.limit = _s_lock_blinds.gametype;
			_s_limit_info.found_limit = true;
		LeaveCriticalSection(&cs_scraper);
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
			}
		}

		// r$c0limits, s$c0limits
		r_iter = p_tablemap->r$()->find("c0limits");
		s_iter = p_tablemap->s$()->find("c0limits");
		if (r_iter!=p_tablemap->r$()->end() && s_iter!=p_tablemap->s$()->end())
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
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
				ProcessRegion(hdcCompatible, hdc, r_iter);
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
			}

			// save what we just scanned through
			EnterCriticalSection(&cs_scraper);
				_s_limit_info.handnumber = l_handnumber;
				_s_limit_info.sblind = l_sblind;
				_s_limit_info.bblind = l_bblind;
				_s_limit_info.bbet = l_bbet;
				_s_limit_info.ante = l_ante;
				_s_limit_info.limit = l_limit;
				_s_limit_info.sb_bb = l_sb_bb;
				_s_limit_info.bb_BB = l_bb_BB;
				_s_limit_info.istournament = l_istournament;
				_s_limit_info.found_handnumber = l_found_handnumber;
				_s_limit_info.found_sblind = l_found_sblind;
				_s_limit_info.found_bblind = l_found_bblind;
				_s_limit_info.found_bbet = l_found_bbet;
				_s_limit_info.found_ante = l_found_ante;
				_s_limit_info.found_limit = l_found_limit;
				_s_limit_info.found_sb_bb = l_found_sb_bb;
				_s_limit_info.found_bb_BB = l_found_bb_BB;
			LeaveCriticalSection(&cs_scraper);
		}


		// r$c0sblind
		r_iter = p_tablemap->r$()->find("c0sblind");
		if (r_iter != p_tablemap->r$()->end() && !_s_limit_info.found_sblind)
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			text.Remove(',');
			text.Remove('$');

			EnterCriticalSection(&cs_scraper);
				if (text!="")
					_s_limit_info.sblind = trans.StringToMoney(text);
			LeaveCriticalSection(&cs_scraper);
		}

		// r$c0bblind
		r_iter = p_tablemap->r$()->find("c0bblind");
		if (r_iter != p_tablemap->r$()->end() && !_s_limit_info.found_bblind)
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			text.Remove(',');
			text.Remove('$');

			EnterCriticalSection(&cs_scraper);
				if (text!="")
					_s_limit_info.bblind = trans.StringToMoney(text);
			LeaveCriticalSection(&cs_scraper);
		}

		// r$c0bigbet
		r_iter = p_tablemap->r$()->find("c0bigbet");
		if (r_iter != p_tablemap->r$()->end() && !_s_limit_info.found_bbet)
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			text.Remove(',');
			text.Remove('$');

			EnterCriticalSection(&cs_scraper);
				if (text!="")
					_s_limit_info.bbet = trans.StringToMoney(text);
			LeaveCriticalSection(&cs_scraper);
		}

		// r$c0ante
		r_iter = p_tablemap->r$()->find("c0ante");
		if (r_iter != p_tablemap->r$()->end() && !_s_limit_info.found_ante)
		{
			ProcessRegion(hdcCompatible, hdc, r_iter);
			old_bitmap = (HBITMAP) SelectObject(hdcCompatible, r_iter->second.cur_bmp);
			trans.DoTransform(r_iter, hdcCompatible, &text);
			SelectObject(hdcCompatible, old_bitmap);

			text.Remove(',');
			text.Remove('$');

			EnterCriticalSection(&cs_scraper);
				if (text!="")
					_s_limit_info.ante = trans.StringToMoney(text);
			LeaveCriticalSection(&cs_scraper);
		}
	}


	// see if anything changed
	log_blind_change = false;

	EnterCriticalSection(&cs_scraper);
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
	LeaveCriticalSection(&cs_scraper);

	// log the stakes change
	if (log_blind_change)
	{
		write_log("\n*************************************************************\n"
				  "NEW STAKES sb(%.2f) bb(%.2f) BB(%.2f) ante(%.2f)\n"
				  "*************************************************************\n",
				  _s_limit_info.sblind, _s_limit_info.bblind, 
				  _s_limit_info.bbet, _s_limit_info.ante);
	}
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
	int				i = 0, w = 0, h = 0;
	HDC				hdcScreen = CreateDC("DISPLAY", NULL, NULL, NULL);
	RECT			cr = {0};
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);

	// Whole window
	GetClientRect(pMyMainWnd->attached_hwnd(), &cr);
	w = cr.right - cr.left;
	h = cr.bottom - cr.top;
	EnterCriticalSection(&cs_scraper);
		_entire_window_last = CreateCompatibleBitmap(hdcScreen, w, h);
		_entire_window_cur = CreateCompatibleBitmap(hdcScreen, w, h);
	LeaveCriticalSection(&cs_scraper);

	// r$regions
	for (RMapI r_iter=p_tablemap->set_r$()->begin(); r_iter!=p_tablemap->set_r$()->end(); r_iter++)
	{
		w = r_iter->second.right - r_iter->second.left;
		h = r_iter->second.bottom - r_iter->second.top;
		r_iter->second.last_bmp = CreateCompatibleBitmap(hdcScreen, w, h);
		r_iter->second.cur_bmp = CreateCompatibleBitmap(hdcScreen, w, h);
	}

	DeleteDC(hdcScreen);
}

void CScraper::DeleteBitmaps(void)
{
	int			i = 0;

	// Whole window
	EnterCriticalSection(&cs_scraper);
		DeleteObject(_entire_window_last);
		DeleteObject(_entire_window_cur);
	LeaveCriticalSection(&cs_scraper);

	// Common cards
	for (RMapI r_iter=p_tablemap->set_r$()->begin(); r_iter!=p_tablemap->set_r$()->end(); r_iter++)
	{
		DeleteObject(r_iter->second.last_bmp); r_iter->second.last_bmp=NULL;
		DeleteObject(r_iter->second.cur_bmp); r_iter->second.cur_bmp=NULL;
	}
}

void CScraper::SetLockedBlinds(const SLockBlinds LB)
{
	EnterCriticalSection(&cs_scraper);
		_s_lock_blinds.blinds_are_locked = LB.blinds_are_locked;
		_s_lock_blinds.sblind = LB.sblind;
		_s_lock_blinds.bblind = LB.bblind;
		_s_lock_blinds.bbet = LB.bbet;
		_s_lock_blinds.ante = LB.ante;
		_s_lock_blinds.gametype = LB.gametype;
	LeaveCriticalSection(&cs_scraper);
}

void CScraper::SetLimitInfo(const SLimitInfo LI)
{
	EnterCriticalSection(&cs_scraper);
		_s_limit_info.handnumber = LI.handnumber;
		_s_limit_info.sblind = LI.sblind;
		_s_limit_info.bblind = LI.bblind;
		_s_limit_info.bbet = LI.bbet;
		_s_limit_info.ante = LI.ante;
		_s_limit_info.limit = LI.limit;
		_s_limit_info.istournament = LI.istournament;
		_s_limit_info.found_sblind = LI.found_sblind;
		_s_limit_info.found_bblind = LI.found_bblind;
		_s_limit_info.found_bbet = LI.found_bbet;
		_s_limit_info.found_ante = LI.found_ante;
		_s_limit_info.found_limit = LI.found_limit;
	LeaveCriticalSection(&cs_scraper);
}

// This is the chip scrape routine
const double CScraper::DoChipScrape(HDC hdc, RMapCI r_iter)
{
	int				j = 0, stackindex = 0, chipindex = 0;
	int				hash_type = 0, num_precs = 0, pixcount = 0, chipwidth = 0, chipheight = 0;
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
	if (hdc == NULL || r_iter == p_tablemap->r$()->end())
		return 0.;

	// figure out if we are dealing with a pot or playerbet here
	if (r_iter->second.name.Mid(0,5)=="c0pot" && r_iter->second.name.Mid(6,4)=="chip")
		type = r_iter->second.name.Mid(0,10);

	else if (r_iter->second.name.Mid(0,1)=="p" && r_iter->second.name.Mid(2,4)=="chip")
		type = r_iter->second.name.Mid(0,6);

	else
		return 0.;

	// find start, vert stride, and horiz stride regions
	s.Format("%s00", type.GetString());
	r_start = p_tablemap->r$()->find(s.GetString());
	if (r_start == p_tablemap->r$()->end())
		return 0.;

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
				chipwidth = right - left;
				chipheight = bottom - top;

				// calculate hash
				if (hash_type>=1 && hash_type<=3)
				{
					num_precs = (int) p_tablemap->p$()->GetSize();
					pixcount = 0;
					for (j=0; j<num_precs; j++)
					{
						if (p_tablemap->p$()->GetAt(j).number == hash_type)
						{
							pix[pixcount++] = GetPixel(hdc, left + p_tablemap->p$()->GetAt(j).x, top + p_tablemap->p$()->GetAt(j).y);
						}
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

	if (s.MakeLower().Left(5) == "check")
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
