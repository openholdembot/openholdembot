#include "StdAfx.h"
#include <complex>

#include "CAutoplayer.h"
#include "CSymbols.h"
#include "CIteratorThread.h"
#include "CHeartbeatThread.h"
#include "CScraper.h"
#include "CGrammar.h"
#include "CPreferences.h"

#include "OpenHoldem.h"
#include "MainFrm.h"
#include "CGameState.h"

#include "PokerChat.hpp"

CAutoplayer	*p_autoplayer = NULL;

CAutoplayer::CAutoplayer(BOOL bInitiallyOwn, LPCTSTR lpszName) : _mutex(bInitiallyOwn, lpszName)
{
	ASSERT(_mutex.m_hObject != NULL);

	// Seed RNG
	//srand((unsigned)time( NULL ));

	set_autoplayer_engaged(false);
	ResetHand();
}

CAutoplayer::~CAutoplayer(void) 
{
}

void CAutoplayer::DoChat(void)
{
	double			f_chat = p_symbols->f$chat();
	HWND			hwnd_focus = GetFocus();
	POINT			cur_pos = {0};

	if (f_chat == 0)
		return;

	if (!IsChatAllowed())
		return;

	if (_the_chat_message == NULL)
		return;

	::GetCursorPos(&cur_pos);

	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);
	RMapCI			r_iter = p_tablemap->r$()->find("chatbox");

	if (r_iter == p_tablemap->r$()->end())
		return;

	RECT r;
	r.left = r_iter->second.left;
	r.top = r_iter->second.top;
	r.right = r_iter->second.right;
	r.bottom = r_iter->second.bottom;

	CString s;
	s.Format("%s", _the_chat_message);

	// If we get a lock, do the action
	if (_mutex.Lock(500))
	{
		(theApp._dll_keyboard_sendstring) (pMyMainWnd->attached_hwnd(), r, s, false, hwnd_focus, cur_pos);
		_mutex.Unlock();
	}

	_the_chat_message = NULL;
	ComputeFirstPossibleNextChatTime();
}

void CAutoplayer::DoAutoplayer(void) 
{
	int				x = 0;
	int				num_buttons_visible = 0;
	int				delay = 0;

	// Check status of "Keyboard" menu item, and engage if necessary
	CheckBringKeyboard();

	// Get r$ indices of buttons that are visible
	num_buttons_visible = GetR$ButtonIndices();

	// Calculate f$play, f$prefold, f$delay and f$chat for use below
	p_symbols->CalcSecondaryFormulas();

	// Handle f$play
	DoF$play();

	// Handle i86buttons
	DoI86();

	// Handle f$prefold
	DoPrefold();

	//  2007.02.27 by THF
	//
	//  Additional functionality: PokerChat
	//	(Handle f$chat)
	//
	//  Selecting a chat message (or no one).
	//	This message will then be processed by the autoplayer,
	//	when it's time to click the buttons.
	//
	RegisterChatMessage(p_symbols->f$chat());

	//  Avoiding unnecessary calls to DoChat(),
	//	especially mouse movements to the chat box.
	if ((p_symbols->f$chat() != 0) && IsChatAllowed())
		DoChat();

	// Get count of stable frames for use a little bit further down
	x = CountSameScrape();

	// If iterator thread is still iterating, then return
	if (iter_vars.iterator_thread_running())
	{
		// Calc primary formulas, but not with final answer, so main window can display correctly
		p_symbols->CalcPrimaryFormulas(false);

		return;
	}

	// if we have <2 visible buttons, then return
	// Change from only requiring one visible button (OpenHoldem 2008-04-03)
	if (num_buttons_visible < 2)
	{
		// Calc primary formulas, but not with final answer, so main window can display correctly
		p_symbols->CalcPrimaryFormulas(false);

		return;
	}

	// if we are not playing (occluded?) 2008-03-25 Matrix
	if (!p_symbols->sym()->playing)
	{
		// Calc primary formulas, but not with final answer, so main window can display correctly
		p_symbols->CalcPrimaryFormulas(false);
	
		return;
	}

	// If we don't have enough stable frames, or have not waited f$delay milliseconds, then return (modified Spektre 2008-04-03)
	delay = p_symbols->f$delay() / prefs.scrape_delay();	// scale f$delay to a number of scrapes

	if (x < (int) prefs.frame_delay() + delay)
	{
		// Calc primary formulas, but not with final answer, so main window can display correctly
		p_symbols->CalcPrimaryFormulas(false);

		return;
	}

	// If the game state processor didn't process this frame, then we should not act.
	if (!p_game_state->ProcessThisFrame ())
	{
		// Calc primary formulas, but not with final answer, so main window can display correctly
		p_symbols->CalcPrimaryFormulas(false);
		return;
	}

	// Now that we got through all of the above, we are ready to evaluate the primary formulas
	// and take the appropriate action
	p_symbols->CalcPrimaryFormulas(true);

	// do swag first since it is the odd one
	if (p_symbols->f$swag() && !p_symbols->f$alli() && p_scraper->GetButtonState(3)) 
	{
		DoSwag();
	}
	else 
	{
		if (p_symbols->f$alli() && p_scraper->GetButtonState(3))
			DoSlider();
		else 
			DoARCCF();
	}
}

void CAutoplayer::ResetHand(void) 
{
	for (int i=0; i<=4; i++)
	{
		set_didchec(i,0);
		set_didcall(i,0);
		set_didrais(i,0);
		set_didswag(i,0);
	}

	set_prevaction(-1);
}

void CAutoplayer::ResetRound(void) 
{
	set_didchec(4,0);
	set_didcall(4,0);
	set_didrais(4,0);
	set_didswag(4,0);
}

void CAutoplayer::DoSwag(void) 
{
	HWND			hwnd_focus = GetFocus();
	POINT			cur_pos = {0};
	bool			lost_focus = false;
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);
	double			f_swag = p_symbols->f$swag();
	RMapCI			r_edit = p_tablemap->r$()->find("i3edit");
	RMapCI			r_button = p_tablemap->r$()->find("i3button");
	POINT			p_null = {-1, -1};
	RECT			r_null = {-1, -1, -1, -1};

	if (r_edit==p_tablemap->r$()->end())
		return;

	::GetCursorPos(&cur_pos);

	// swag buttons are hard coded as #3 for now, due to legacy WH standard
	if (p_scraper->GetButtonState(3) && r_edit!=p_tablemap->r$()->end())
	{

		// If we get a lock, do the action
		if (_mutex.Lock(500))
		{

			RECT rect_edit;
			rect_edit.left = r_edit->second.left;
			rect_edit.top = r_edit->second.top;
			rect_edit.right = r_edit->second.right;
			rect_edit.bottom = r_edit->second.bottom;

			// TEXT SELECTION
			if (p_tablemap->swagselectionmethod() == TEXTSEL_DOUBLECLICK)
				(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), rect_edit, MouseLeft, 2, NULL, p_null);

			else if (p_tablemap->swagselectionmethod() == TEXTSEL_SINGLECLICK)
				(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), rect_edit, MouseLeft, 1, NULL, p_null);

			else if (p_tablemap->swagselectionmethod() == TEXTSEL_CLICKDRAG)
				(theApp._dll_mouse_click_drag) (pMyMainWnd->attached_hwnd(), rect_edit, NULL, p_null);

			else
			{
				_mutex.Unlock();
				return;
			}

			// Check for stolen focus, and thus misswag
			if (GetForegroundWindow() != pMyMainWnd->attached_hwnd())
				lost_focus = true;

			Sleep(prefs.swag_delay_1());



			// TEXT DELETION
			if (p_tablemap->swagdeletionmethod() == TEXTDEL_DELETE)
				(theApp._dll_keyboard_sendkey) (pMyMainWnd->attached_hwnd(), r_null, VK_DELETE, NULL, p_null);

			else if (p_tablemap->swagdeletionmethod() == TEXTDEL_BACKSPACE)
				(theApp._dll_keyboard_sendkey) (pMyMainWnd->attached_hwnd(), r_null, VK_BACK, NULL, p_null);

			else
			{
				_mutex.Unlock();
				return;
			}

			// Check for stolen focus, and thus misswag
			if (GetForegroundWindow() != pMyMainWnd->attached_hwnd())
				lost_focus = true;

			Sleep(prefs.swag_delay_2());



			// SWAG AMOUNT ENTRY
			CString swag_amt;
			if (f_swag != (int) f_swag)
				swag_amt.Format("%.2f", f_swag);
			else
				swag_amt.Format("%.0f", f_swag);

			(theApp._dll_keyboard_sendstring) (pMyMainWnd->attached_hwnd(), rect_edit, swag_amt, prefs.swag_use_comma(), NULL, p_null);

			// Check for stolen focus, and thus misswag
			if (GetForegroundWindow() != pMyMainWnd->attached_hwnd())
				lost_focus = true;

			Sleep(prefs.swag_delay_3());



			// BET CONFIRMATION ACTION
			if (!lost_focus || !prefs.focus_detect())
			{
				if (p_tablemap->swagconfirmationmethod() == BETCONF_ENTER)
				{
					(theApp._dll_keyboard_sendkey) (pMyMainWnd->attached_hwnd(), r_null, VK_RETURN, hwnd_focus, cur_pos);
				}

				else if (p_tablemap->swagconfirmationmethod() == BETCONF_CLICKBET &&
						 (_rais_but!=p_tablemap->r$()->end() || r_button!=p_tablemap->r$()->end()) )
				{
					RECT rect_button;

					// use i3button region if it exists, otherwise use the bet/raise button region
					if (r_button!=p_tablemap->r$()->end())
					{
						rect_button.left = r_button->second.left;
						rect_button.top = r_button->second.top;
						rect_button.right = r_button->second.right;
						rect_button.bottom = r_button->second.bottom;
					}
					else
					{
						rect_button.left = _rais_but->second.left;
						rect_button.top = _rais_but->second.top;
						rect_button.right = _rais_but->second.right;
						rect_button.bottom = _rais_but->second.bottom;
					}

					if (p_tablemap->buttonclickmethod() == BUTTON_DOUBLECLICK)
						(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), rect_button, MouseLeft, 2, hwnd_focus, cur_pos);
					else
						(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), rect_button, MouseLeft, 1, hwnd_focus, cur_pos);
				}

				else
				{
					_mutex.Unlock();
					return;
				}

				// record didswag/prevaction
				int sym_br = (int) p_symbols->sym()->br;

				set_didswag(4, p_symbols->sym()->didswag[4] + 1);
				set_didswag(sym_br-1, p_symbols->sym()->didswag[sym_br-1] + 1);
				set_prevaction(PREVACT_SWAG);

				p_symbols->UpdateAutoplayerInfo();

				// reset elapsedauto symbol
				time_t my_time_t;
				time(&my_time_t);
				p_symbols->set_elapsedautohold(my_time_t);

				p_heartbeat_thread->set_replay_recorded_this_turn(false);

				// log it
				write_logautoplay("SWAG");
			}

			_mutex.Unlock();
		}
	}
}

void CAutoplayer::DoARCCF(void) 
{
	int				do_click = 0;
	HWND			hwnd_focus = GetFocus();
	POINT			cur_pos = {0};
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);

	double			alli = p_symbols->f$alli();
	double			rais = p_symbols->f$rais();
	double			call = p_symbols->f$call();
	int				sym_myturnbits = (int) p_symbols->sym()->myturnbits;

	::GetCursorPos(&cur_pos);

	RECT			r;

	do_click = -1;

	// ALLIN
	if (alli && sym_myturnbits&0x8 && _alli_but!=p_tablemap->r$()->end())
	{
		r.left = _alli_but->second.left;
		r.top = _alli_but->second.top;
		r.right = _alli_but->second.right;
		r.bottom =_alli_but->second.bottom;
		do_click = 4;
	}

	// RAISE
	else if (rais && sym_myturnbits&0x4 && _rais_but!=p_tablemap->r$()->end())
	{
		r.left = _rais_but->second.left;
		r.top = _rais_but->second.top;
		r.right = _rais_but->second.right;
		r.bottom =_rais_but->second.bottom;
		do_click = 3;
	}

	// CALL
	else if (call && sym_myturnbits&0x1 && _call_but!=p_tablemap->r$()->end())
	{
		r.left = _call_but->second.left;
		r.top = _call_but->second.top;
		r.right = _call_but->second.right;
		r.bottom =_call_but->second.bottom;
		do_click = 2;
	}

	// CHECK
	// None of f$alli, f$swag, f$rais, f$call are > 0 or no buttons related to
	// these actions can be found. If there is a check button, then click it.
	else if (_chec_but!=p_tablemap->r$()->end())
	{
		r.left = _chec_but->second.left;
		r.top = _chec_but->second.top;
		r.right = _chec_but->second.right;
		r.bottom =_chec_but->second.bottom;
		do_click = 1;
	}

	// FOLD
	// None of f$alli, f$swag, f$rais, f$call are > 0 or no buttons related to
	// these actions can be found. If there is a fold button, then click it, otherwise we have a serious problem.
	else if (_fold_but!=p_tablemap->r$()->end())
	{
		r.left = _fold_but->second.left;
		r.top = _fold_but->second.top;
		r.right = _fold_but->second.right;
		r.bottom =_fold_but->second.bottom;
		do_click = 0;
	}

	if (do_click>=0)
	{
		// If we get a lock, do the action
		if (_mutex.Lock(500))
		{

			if (p_tablemap->buttonclickmethod() == BUTTON_DOUBLECLICK)
				(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), r, MouseLeft, 2, hwnd_focus, cur_pos);
			else
				(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), r, MouseLeft, 1, hwnd_focus, cur_pos);
			
			_mutex.Unlock();

			// record did*/prevaction
			int sym_br = (int) p_symbols->sym()->br;

			switch (do_click)
			{
				case 4:  // allin
					set_prevaction(PREVACT_ALLI);
					write_logautoplay("ALLI");
					break;

				case 3:  // raise
					set_didrais(4, p_symbols->sym()->didrais[4] + 1);
					set_didrais(sym_br-1, p_symbols->sym()->didrais[sym_br-1] + 1);
					set_prevaction(PREVACT_RAIS);
					write_logautoplay("RAIS");
					break;

				case 2:  // call
					set_didcall(4, p_symbols->sym()->didcall[4] + 1);
					set_didcall(sym_br-1, p_symbols->sym()->didcall[sym_br-1] + 1);
					set_prevaction(PREVACT_CALL);
					write_logautoplay("CALL");
					break;

				case 1:  // check
					set_didchec(4, p_symbols->sym()->didchec[4] + 1);
					set_didchec(sym_br-1, p_symbols->sym()->didchec[sym_br-1] + 1);
					set_prevaction(PREVACT_CHEC);
					write_logautoplay("CHEC");
					break;

				case 0:  // fold
					set_prevaction(PREVACT_FOLD);
					write_logautoplay("FOLD");
					break;
			}

			p_symbols->UpdateAutoplayerInfo();

			// reset elapsedauto symbol
			time_t my_time_t;
			time(&my_time_t);
			p_symbols->set_elapsedautohold(my_time_t);

			p_heartbeat_thread->set_replay_recorded_this_turn(false);
		}
	}
}

void CAutoplayer::DoSlider(void) 
{
	HWND			hwnd_focus = GetFocus();
	POINT			cur_pos = {0};

	bool			sym_ismyturn = (bool) p_symbols->sym()->ismyturn;
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);
	double			alli = p_symbols->f$alli();
	RMapCI			slider = p_tablemap->r$()->find("i3slider");
	RMapCI			handle = p_tablemap->r$()->find("i3handle");
	RMapCI			r_button = p_tablemap->r$()->find("i3button");
	POINT			p_null = {-1, -1};
	RECT			r_null = {-1, -1, -1, -1};

	::GetCursorPos(&cur_pos);

	if (!sym_ismyturn)
		return;

	if (slider==p_tablemap->r$()->end() | handle==p_tablemap->r$()->end())
		return;

	if (!p_scraper->handle_found_at_xy())
		return;


	// Get mutex lock and jam
	if (_mutex.Lock(500))
	{
		// Click and drag handle
		RECT	r;
		r.left = p_scraper->handle_xy().x + ((handle->second.right - handle->second.left)/2);
		r.top = p_scraper->handle_xy().y + ((handle->second.bottom - handle->second.top)/2);
		r.right = p_scraper->handle_xy().x + (slider->second.right - slider->second.left);
		r.bottom = r.top;		
		
		write_log("*** Jam from %d,%d to %d,%d \n", r.left, r.top, r.right, r.bottom);

		(theApp._dll_mouse_click_drag) (pMyMainWnd->attached_hwnd(), r, NULL, p_null);

		Sleep(prefs.swag_delay_3());

		// Click confirmation button
		if (p_tablemap->swagconfirmationmethod() == BETCONF_ENTER)
		{
			(theApp._dll_keyboard_sendkey) (pMyMainWnd->attached_hwnd(), r_null, VK_RETURN, hwnd_focus, cur_pos);
		}

		else if (p_tablemap->swagconfirmationmethod() == BETCONF_CLICKBET &&
				 (_rais_but!=p_tablemap->r$()->end() || r_button!=p_tablemap->r$()->end()) )
		{
			RECT rect_button;

			// use allin button if it exists, otherwise use i3button region if it exists, 
			// otherwise use the bet/raise button region
			if (_alli_but!=p_tablemap->r$()->end())
			{
				rect_button.left = _alli_but->second.left;
				rect_button.top = _alli_but->second.top;
				rect_button.right = _alli_but->second.right;
				rect_button.bottom = _alli_but->second.bottom;
			}
			
			else if (r_button!=p_tablemap->r$()->end())
			{
				rect_button.left = r_button->second.left;
				rect_button.top = r_button->second.top;
				rect_button.right = r_button->second.right;
				rect_button.bottom = r_button->second.bottom;
			}
			
			else
			{
				rect_button.left = _rais_but->second.left;
				rect_button.top = _rais_but->second.top;
				rect_button.right = _rais_but->second.right;
				rect_button.bottom = _rais_but->second.bottom;
			}

			if (p_tablemap->buttonclickmethod() == BUTTON_DOUBLECLICK)
				(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), rect_button, MouseLeft, 2, hwnd_focus, cur_pos);
			else
				(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), rect_button, MouseLeft, 1, hwnd_focus, cur_pos);
		}

		write_logautoplay("JAM");

		write_log("*** Jam complete \n",r.left, r.top, r.right, r.bottom);

		// reset elapsedauto symbol
		time_t my_time_t;
		time(&my_time_t);
		p_symbols->set_elapsedautohold(my_time_t);
	}

	_mutex.Unlock();
}

void CAutoplayer::DoPrefold(void) 
{
	HWND			hwnd_focus = GetFocus();
	POINT			cur_pos = {0};
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);
	double			prefold = p_symbols->f$prefold();

	::GetCursorPos(&cur_pos);

	if (prefold == 0)  
		return;

	if (_pre_fold_but == p_tablemap->r$()->end())  
		return;

	// Randomize click location
	RECT	r;
	r.left = _pre_fold_but->second.left;
	r.top = _pre_fold_but->second.top;
	r.right = _pre_fold_but->second.right;
	r.bottom = _pre_fold_but->second.bottom;
	
	// If we get a lock, do the action
	if (_mutex.Lock(500))
	{
		(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), r, MouseLeft, 1, hwnd_focus, cur_pos);

		_mutex.Unlock();

		// reset elapsedauto symbol
		time_t my_time_t;
		time(&my_time_t);
		p_symbols->set_elapsedautohold(my_time_t);

		// set prevaction
		set_prevaction(PREVACT_FOLD);

		p_symbols->UpdateAutoplayerInfo();

		write_logautoplay("FOLD");
	}
}

const int CAutoplayer::CountSameScrape(void) 
{
	int						i = 0;
	static unsigned int		card_common_last[5] = {0};
	static unsigned int		card_player_last[10][2] = {0};
	static bool				dealer_last[10] = {0};
	static double			playerbalance_last[10] = {0};
	static double			playerbet_last[10] = {0};
	static double			myturnbitslast = 0;
	bool					same_scrape = false;
	static int				num_same_scrapes = 0;

	// These items need to be the same to count as a identical frame:
	// - up and down cards
	// - button position
	// - playerbets
	// - playerbalances
	// - button states
	same_scrape = true;
	for (i=0; i<=4; i++)
		if (p_scraper->card_common(i) != card_common_last[i])  
			same_scrape = false;

	for (i=0; i<=9; i++)
	{
		if (p_scraper->card_player(i, 0) != card_player_last[i][0])  same_scrape = false;
		if (p_scraper->card_player(i, 1) != card_player_last[i][1])  same_scrape = false;
		if (p_scraper->dealer(i) != dealer_last[i])  same_scrape = false;
		if (p_scraper->player_balance(i) != playerbalance_last[i])  same_scrape = false;
		if (p_scraper->player_bet(i) != playerbet_last[i])  same_scrape = false;
	}

	int e = SUCCESS;
	int sym_myturnbits = (int) p_symbols->sym()->myturnbits;

	if (sym_myturnbits != myturnbitslast)  same_scrape = false;

	if (same_scrape)
	{
		num_same_scrapes++;
	}
	else
	{
		for (i=0; i<5; i++)
		{
			card_common_last[i] = p_scraper->card_common(i);
		}
		for (i=0; i<10; i++)
		{
			card_player_last[i][0] = p_scraper->card_player(i, 0);
			card_player_last[i][1] = p_scraper->card_player(i, 1);
			dealer_last[i] = p_scraper->dealer(i);
			playerbalance_last[i] = p_scraper->player_balance(i);
			playerbet_last[i] = p_scraper->player_bet(i);
		}
		myturnbitslast = sym_myturnbits;
		num_same_scrapes = 0;
	}

	return num_same_scrapes;
}


int CAutoplayer::GetR$ButtonIndices(void)
{
	int				i = 0;
	int				button_index = 0;
	CString			s = "";
	int				num_seen = 0;

	//////////////////////////////////////////////////////////
	// find ALLIN button region from scraper
	button_index = -1;
	for (i=0; i<=9 && button_index==-1; i++)
	{
		if (p_scraper->GetButtonState(i) && p_scraper->IsStringAllin(p_scraper->button_label(i)))
		{
			button_index = i;
			num_seen++;
		}
	}

	// find allin button region from table map
	s.Format("i%dbutton", button_index);
	_alli_but = p_tablemap->r$()->find(s.GetString());

	//////////////////////////////////////////////////////////
	// find RAISE button region from scraper
	button_index = -1;
	for (i=0; i<=9 && button_index==-1; i++)
	{
		if (p_scraper->GetButtonState(i) && p_scraper->IsStringRaise(p_scraper->button_label(i)))
		{
			button_index = i;
			num_seen++;
		}
	}

	// find rais button region from table map
	s.Format("i%dbutton", button_index);
	_rais_but = p_tablemap->r$()->find(s.GetString());

	//////////////////////////////////////////////////////////
	// find CALL button region from scraper
	button_index = -1;
	for (i=0; i<=9 && button_index==-1; i++)
	{
		if (p_scraper->GetButtonState(i) && p_scraper->IsStringCall(p_scraper->button_label(i)))
		{
			button_index = i;
			num_seen++;
		}
	}

	// find call button region from table map
	s.Format("i%dbutton", button_index);
	_call_but = p_tablemap->r$()->find(s.GetString());

	//////////////////////////////////////////////////////////
	// find CHECK button region from scraper
	button_index = -1;
	for (i=0; i<=9 && button_index==-1; i++)
	{
		if (p_scraper->GetButtonState(i) && p_scraper->IsStringCheck(p_scraper->button_label(i)))
		{
			button_index = i;
			num_seen++;
		}
	}

	// find chec button region from table map
	s.Format("i%dbutton", button_index);
	_chec_but = p_tablemap->r$()->find(s.GetString());

	//////////////////////////////////////////////////////////
	// find FOLD button region from scraper
	button_index = -1;
	for (i=0; i<=9 && button_index==-1; i++)
	{
		if (p_scraper->GetButtonState(i) && p_scraper->IsStringFold(p_scraper->button_label(i)))
		{
			button_index = i;
			num_seen++;
		}
	}

	// find fold button region from table map
	s.Format("i%dbutton", button_index);
	_fold_but = p_tablemap->r$()->find(s.GetString());

	//////////////////////////////////////////////////////////
	// find AUTOPOST button region from scraper
	button_index = -1;
	_autopost_state = true;
	for (i=0; i<=9 && button_index==-1; i++)
	{
		if (p_scraper->IsStringAutopost(p_scraper->button_label(i)))
		{
			_autopost_state = p_scraper->GetButtonState(i);
			button_index = i;
		}
	}

	// find autopost button region from table map
	s.Format("i%dbutton", button_index);
	_autopost_but = p_tablemap->r$()->find(s.GetString());

	//////////////////////////////////////////////////////////
	// find SITIN button region from scraper
	button_index = -1;
	_sitin_state = true;
	for (i=0; i<=9 && button_index==-1; i++)
	{
		if (p_scraper->IsStringSitin(p_scraper->button_label(i)))
		{
			_sitin_state = p_scraper->GetButtonState(i);
			button_index = i;
		}
	}

	// find sitin button region from table map
	s.Format("i%dbutton", button_index);
	_sitin_but = p_tablemap->r$()->find(s.GetString());

	//////////////////////////////////////////////////////////
	// find SITOUT button region from scraper
	button_index = -1;
	_sitout_state = false;
	for (i=0; i<=9 && button_index==-1; i++)
	{
		if (p_scraper->IsStringSitout(p_scraper->button_label(i)))
		{
			_sitout_state = p_scraper->GetButtonState(i);
			button_index = i;
		}
	}

	// find sitout button region from table map
	s.Format("i%dbutton", button_index);
	_sitout_but = p_tablemap->r$()->find(s.GetString());

	//////////////////////////////////////////////////////////
	// find LEAVE button region from scraper
	button_index = -1;
	for (i=0; i<=9 && button_index==-1; i++)
	{
		if (p_scraper->GetButtonState(i) && p_scraper->IsStringLeave(p_scraper->button_label(i)))
		{
			button_index = i;
		}
	}

	// find leave button region from table map
	s.Format("i%dbutton", button_index);
	_leave_but = p_tablemap->r$()->find(s.GetString());

	//////////////////////////////////////////////////////////
	// find PREFOLD button region from scraper
	button_index = -1;
	for (i=0; i<=9 && button_index==-1; i++)
	{
		if (p_scraper->GetButtonState(i) && p_scraper->IsStringPrefold(p_scraper->button_label(i)))
		{
			button_index = i;
		}
	}

	// find prefold button region from table map
	s.Format("i%dbutton", button_index);
	_pre_fold_but = p_tablemap->r$()->find(s.GetString());

	//////////////////////////////////////////////////////////
	// find i86 button region from scraper
	_i86_state = false;
	_i86_but = p_tablemap->r$()->end();
	if (p_scraper->GetButtonState(86))
	{
		_i86_but = p_tablemap->r$()->find("i86button");
		_i86_state = true;
	}

	//////////////////////////////////////////////////////////
	// find i86X button region from scraper
	for (i=0; i<=9; i++)
	{
		_i86X_state[i] = false;
		_i86X_but[i] = p_tablemap->r$()->end();
		if (p_scraper->GetButtonState(860+i))
		{
			s.Format("i86%dbutton", i);
			_i86X_but[i] = p_tablemap->r$()->find(s.GetString());
			_i86X_state[i] = true;
		}
	}

	return num_seen;
}

void CAutoplayer::CheckBringKeyboard(void) 
{
	HMENU			bringsysmenu = NULL;
	MENUITEMINFO	mii;
	int				input_count = 0, i = 0;
	INPUT			input[100] = {0};
	HWND			hwnd_focus = GetFocus();
	POINT			cur_pos = {0};
	char			temp[256] = {0};
	CString			c_text = "";
	int				keybd_item_pos = 0;
	int				e = SUCCESS;
	bool			sym_isbring = (bool) p_symbols->sym()->isbring;
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);

	// Init locals
	memset(&mii, 0, sizeof(MENUITEMINFO));

	GetCursorPos(&cur_pos);

	// Find position of "Keyboard" item on system menu
	bringsysmenu = GetSystemMenu(pMyMainWnd->attached_hwnd(), false);

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STRING;
	mii.fType = MFT_STRING;
	mii.dwTypeData = temp;
	keybd_item_pos = -1;
	for (i=GetMenuItemCount(bringsysmenu)-1; i>=0; i--) 
	{
		mii.cch = 256;
	
		// Get the text of this menu item
		GetMenuItemInfo(bringsysmenu, i, true, &mii);
		c_text = temp;

		// See if this is the "keyboard" menu item
		if (c_text.MakeLower().Find("keyboard") != -1) 
		{
			keybd_item_pos = i;
			continue;
		}
	}

	// Get state of keyboard menu item
	if (keybd_item_pos == -1) 
	{
		return;
	}
	else 
	{
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_STATE;
		GetMenuItemInfo(bringsysmenu, keybd_item_pos, true, &mii);
	}

	if (!(mii.fState&MFS_CHECKED) && sym_isbring) 
	{

		input_count = 0;
		// Alt key down
		ZeroMemory(&input[input_count],sizeof(INPUT));
		input[input_count].type = INPUT_KEYBOARD;
		input[input_count].ki.wVk = VK_MENU;
		input_count++;

		// Space bar down
		ZeroMemory(&input[input_count],sizeof(INPUT));
		input[input_count].type = INPUT_KEYBOARD;
		input[input_count].ki.wVk = VK_SPACE;
		input_count++;

		// Space bar up
		ZeroMemory(&input[input_count],sizeof(INPUT));
		input[input_count].type = INPUT_KEYBOARD;
		input[input_count].ki.wVk = VK_SPACE;
		input[input_count].ki.dwFlags = KEYEVENTF_KEYUP;
		input_count++;

		// Alt key up
		ZeroMemory(&input[input_count],sizeof(INPUT));
		input[input_count].type = INPUT_KEYBOARD;
		input[input_count].ki.wVk = VK_MENU;
		input[input_count].ki.dwFlags = KEYEVENTF_KEYUP;
		input_count++;

		if (_mutex.Lock(500)) 
		{
			SetFocus(pMyMainWnd->attached_hwnd());
			SetForegroundWindow(pMyMainWnd->attached_hwnd());
			SetActiveWindow(pMyMainWnd->attached_hwnd());
			SendInput(input_count, input, sizeof(INPUT));
			_mutex.Unlock();
		}
		Sleep(200);

		input_count = 0;
		// K down
		ZeroMemory(&input[input_count],sizeof(INPUT));
		input[input_count].type = INPUT_KEYBOARD;
		input[input_count].ki.wVk = 'K';
		input_count++;

		// K up
		ZeroMemory(&input[input_count],sizeof(INPUT));
		input[input_count].type = INPUT_KEYBOARD;
		input[input_count].ki.wVk = 'K';
		input[input_count].ki.dwFlags = KEYEVENTF_KEYUP;
		input_count++;

		if (_mutex.Lock(500)) 
		{
			SetFocus(pMyMainWnd->attached_hwnd());
			SetForegroundWindow(pMyMainWnd->attached_hwnd());
			SetActiveWindow(pMyMainWnd->attached_hwnd());

			SendInput(input_count, input, sizeof(INPUT));

			SetActiveWindow(hwnd_focus);
			SetForegroundWindow(hwnd_focus);
			SetFocus(hwnd_focus);

			SetCursorPos(cur_pos.x, cur_pos.y);

			_mutex.Unlock();
		}
	}
}

void CAutoplayer::DoF$play(void) 
{
	bool			do_click = false;
	HWND			hwnd_focus = GetFocus();
	POINT			cur_pos = {0};
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);
	double			f_play = p_symbols->f$play();
	RECT			r = {0};

	::GetCursorPos(&cur_pos);

	do_click = false;

	// leave table
	if (f_play==-2 && _leave_but!=p_tablemap->r$()->end())
	{
		r.left = _leave_but->second.left;
		r.top = _leave_but->second.top;
		r.right = _leave_but->second.right;
		r.bottom = _leave_but->second.bottom;

		do_click = true;
	}

	// no action
	else if (f_play==-1)
	{
	}

	// sit out
	else if (f_play==0 && 
			 ( (_sitout_but!=p_tablemap->r$()->end() && _sitout_state==false) || 
			   (_sitin_but!=p_tablemap->r$()->end() && _sitin_state==true) ) )
	{

		if (_sitout_but!=p_tablemap->r$()->end() && _sitout_state==false)
		{
			r.left = _sitout_but->second.left;
			r.top = _sitout_but->second.top;
			r.right = _sitout_but->second.right;
			r.bottom = _sitout_but->second.bottom;
		}

		else if (_sitin_but!=p_tablemap->r$()->end() && _sitin_state==true)
		{
			r.left = _sitin_but->second.left;
			r.top = _sitin_but->second.top;
			r.right = _sitin_but->second.right;
			r.bottom = _sitin_but->second.bottom;
		}

		do_click = true;
	}

	// sit in
	else if (f_play==1 && 
			 ( (_sitin_but!=p_tablemap->r$()->end() && _sitin_state==false) || 
			   (_sitout_but!=p_tablemap->r$()->end() && _sitout_state==true) ) )
	{
		if (_sitin_but!=p_tablemap->r$()->end() && _sitin_state==false)
		{
			r.left = _sitin_but->second.left;
			r.top = _sitin_but->second.top;
			r.right = _sitin_but->second.right;
			r.bottom = _sitin_but->second.bottom;
		}

		else if (_sitout_but!=p_tablemap->r$()->end() && _sitout_state==true)
		{
			r.left = _sitout_but->second.left;
			r.top = _sitout_but->second.top;
			r.right = _sitout_but->second.right;
			r.bottom = _sitout_but->second.bottom;
		}

		do_click = true;
	}

	// Autopost
	if (f_play==1 && _autopost_but!=p_tablemap->r$()->end() && _autopost_state==false)
	{
		r.left = _autopost_but->second.left;
		r.top = _autopost_but->second.top;
		r.right = _autopost_but->second.right;
		r.bottom = _autopost_but->second.bottom;

		do_click = true;
	}

	if (do_click)
	{
		// If we get a lock, do the action
		if (_mutex.Lock(500)) 
		{
			(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), r, MouseLeft, 1, hwnd_focus, cur_pos);

			_mutex.Unlock();

			// reset elapsedauto symbol
			time_t my_time_t;
			time(&my_time_t);
			p_symbols->set_elapsedautohold(my_time_t);
		}
	}
}

void CAutoplayer::DoI86(void) 
{
	bool			do_click = false;
	int				i = 0;
	HWND			hwnd_focus = GetFocus();
	POINT			cur_pos = {0};
	CMainFrame		*pMyMainWnd  = (CMainFrame *) (theApp.m_pMainWnd);
	RECT			r;

	::GetCursorPos(&cur_pos);

	do_click = false;

	if (_i86_but!=p_tablemap->r$()->end() && _i86_state)
	{
		r.left = _i86_but->second.left;
		r.top = _i86_but->second.top;
		r.right = _i86_but->second.right;
		r.bottom = _i86_but->second.bottom;

		do_click = true;
	}

	else
	{
		for (i=0; i<=9; i++)
		{
			if (_i86X_but[i]!=p_tablemap->r$()->end() && _i86X_state[i])
			{
				r.left = _i86X_but[i]->second.left;
				r.top = _i86X_but[i]->second.top;
				r.right = _i86X_but[i]->second.right;
				r.bottom = _i86X_but[i]->second.bottom;

				do_click = true;
				i = 10;
			}
		}
	}

	if (do_click)
	{
		// If we get a lock, do the action
		if (_mutex.Lock(500))
		{
			(theApp._dll_mouse_click) (pMyMainWnd->attached_hwnd(), r, MouseLeft, 1, hwnd_focus, cur_pos);

			_mutex.Unlock();

			// reset elapsedauto symbol
			time_t my_time_t;
			time(&my_time_t);
			p_symbols->set_elapsedautohold(my_time_t);
		}
	}
}
