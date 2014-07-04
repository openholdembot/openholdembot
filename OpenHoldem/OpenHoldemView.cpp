//******************************************************************************
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//******************************************************************************
//
// Purpose:
//
//******************************************************************************

// OpenHoldemView.cpp : implementation of the COpenHoldemView class
//

#include "stdafx.h"
#include "OpenHoldemView.h"

#include "CAutoplayerTrace.h"
#include "CEngineContainer.h"
#include "CHandresetDetector.h"
#include "CHeartbeatThread.h"
#include "CPreferences.h"
#include "CScraper.h"
#include "CScraperAccess.h"
#include "CStringMatch.h"
#include "CSymbolengineChipAmounts.h"
#include "CSymbolEngineIsTournament.h"
#include "CSymbolEngineTableLimits.h"
#include "..\CTablemap\CTablemap.h"
#include "CTableState.h"
#include "CHeartbeatThread.h"
#include "CPreferences.h"
#include "MagicNumbers.h"
#include "OpenHoldem.h"
#include "OpenHoldemDoc.h"

// Table layouts
int		cc[k_number_of_community_cards][2] = 
{ 
	{-(CARDSIZEX*2 + 3*2 + CARDSIZEX/2), -(CARDSIZEY/2)},	// absolutes
	{-(CARDSIZEX*1 + 3*1 + CARDSIZEX/2), -(CARDSIZEY/2)},
	{-(CARDSIZEX*0 + 3*0 + CARDSIZEX/2), -(CARDSIZEY/2)},
	{+(CARDSIZEX*0 + 3*1 + CARDSIZEX/2), -(CARDSIZEY/2)},
	{+(CARDSIZEX*1 + 3*2 + CARDSIZEX/2), -(CARDSIZEY/2)}
};

// Player locations as a percentage of width/height
// [nplayers][chairnum][x/y]
double	pc[11][10][2] = 
{
	{ {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },	// 0 players
	{ {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },	// 1 player
	{ {.95,.47}, {.05,.47}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },	// 2 players
	{ {.95,.47}, {.50,.83}, {.05,.47}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },	// 3 players
	{ {.89,.25}, {.89,.69}, {.11,.69}, {.11,.25}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },	// 4 players
	{ {.89,.25}, {.89,.69}, {.50,.83}, {.11,.69}, {.11,.25}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },	// 5 players
	{ {.72,.11}, {.95,.47}, {.72,.83}, {.28,.83}, {.05,.47}, {.28,.11}, {0,0}, {0,0}, {0,0}, {0,0} },	// 6 players
	{ {.72,.11}, {.95,.47}, {.72,.83}, {.50,.83}, {.28,.83}, {.05,.47}, {.28,.11}, {0,0}, {0,0}, {0,0} },	// 7 players
	{ {.72,.11}, {.89,.25}, {.89,.69}, {.72,.83}, {.28,.83}, {.11,.69}, {.11,.25}, {.28,.11}, {0,0}, {0,0} },	// 8 players
	{ {.72,.11}, {.89,.25}, {.89,.69}, {.72,.83}, {.50,.83}, {.28,.83}, {.11,.69}, {.11,.25}, {.28,.11}, {0,0} },	// 9 players
	{ {.72,.11}, {.85,.21}, {.95,.47}, {.85,.73}, {.72,.83}, {.28,.83}, {.15,.73}, {.05,.47}, {.15,.21}, {.28,.11} }	// 10 players
};

// Player bet locations relative to player locations above
// numbers are in pixel units
// [nplayers][chairnum][x/y]
int pcbet[11][10][2] = 
{
	{ {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },	// 0 players
	{ {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },	// 1 player
	{ {-40,+0}, {+40,+0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} },	// 2 players
	{ {-40,+0}, {+0,-45}, {+40,+0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} }, // 3 players
	{ {-40,+37}, {-40,-20}, {+40,-20}, {+40,+37}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} }, // 4 players
	{ {-40,+37}, {-40,-20}, {+0,-45}, {+40,-20}, {+40,+37},  {0,0}, {0,0}, {0,0}, {0,0}, {0,0} }, // 5 players
	{ {-40,+40}, {-40,+0}, {-40,+40}, {+40,-40}, {+40,+0}, {+40,+40}, {0,0}, {0,0}, {0,0}, {0,0} },	// 6 players
	{ {-40,+40}, {-40,+0}, {-40,+40}, {+0,-45}, {+40,-40}, {+40,+0}, {+40,+40}, {0,0}, {0,0}, {0,0} }, // 7 players
	{ {-40,+40}, {-40,+37}, {-40,-20}, {-40,-40}, {+40,-40}, {+40,-20}, {+40,+37}, {+40,+40}, {0,0}, {0,0} }, // 8 players
	{ {-40,+58}, {-40,+37}, {-40,-20}, {-40,-40}, {+0,-45}, {+40,-40}, {+40,-20}, {+40,+37}, {+40,+58}, {0,0} }, // 9 players
	{ {-40,+40}, {-40,+37}, {-40,+0}, {-40,-20}, {-40,-40}, {+40,-40}, {+40,-20}, {+40,+0}, {+40,+37}, {+40,+40} } // 10 players
};

// COpenHoldemView
IMPLEMENT_DYNCREATE(COpenHoldemView, CView)

BEGIN_MESSAGE_MAP(COpenHoldemView, CView)
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

// COpenHoldemView construction/destruction
COpenHoldemView::COpenHoldemView() 
{
	__TRACE
	_black_pen.CreatePen(PS_SOLID, 1, COLOR_BLACK);
	_green_pen.CreatePen(PS_SOLID, 1, COLOR_GREEN);
	_red_pen.CreatePen(PS_SOLID, 1, COLOR_RED);
	_blue_pen.CreatePen(PS_SOLID, 1, COLOR_BLUE);
	_white_dot_pen.CreatePen(PS_DOT, 1, COLOR_WHITE);
	_null_pen.CreatePen(PS_NULL, 0, COLOR_BLACK);

	_white_brush.CreateSolidBrush(COLOR_WHITE);
	_gray_brush.CreateSolidBrush(COLOR_GRAY);
	_red_brush.CreateSolidBrush(COLOR_RED);
	_yellow_brush.CreateSolidBrush(COLOR_YELLOW);

	_logfont.lfWidth = 0;
	_logfont.lfEscapement = 0;
	_logfont.lfOrientation = 0;
	_logfont.lfItalic = 0;
	_logfont.lfUnderline = 0;
	_logfont.lfStrikeOut = 0;
	_logfont.lfCharSet = 0;
	_logfont.lfOutPrecision = 0;
	_logfont.lfClipPrecision = 0;
	_logfont.lfQuality = PROOF_QUALITY;
	_logfont.lfPitchAndFamily = 0;
	strcpy_s(_logfont.lfFaceName, 32, "Times New Roman");

	_handnumber_last = "";
	_sblind_last = _bblind_last = _lim_last = _ante_last = _pot_last = 0.;
	_iterator_thread_progress_last = 0;
	
	for (int i = 0; i<k_number_of_community_cards; i++)
		_card_common_last[i] = CARD_UNDEFINED;

	for (int i = 0; i<k_max_number_of_players ; i++)
	{
		_seated_last[i] = _active_last[i] = _playername_last[i] = "";
		_dealer_last[i] = false;
		_playerbalance_last[i] = _playerbet_last[i] = 0.;
		for (int j=0; j<k_number_of_cards_per_player; j++)
		{
			_card_player_last[i][j] = CARD_NOCARD;
		}
	}
		
	_istournament_last = false;
}

COpenHoldemView::~COpenHoldemView() 
{
	__TRACE
}

BOOL COpenHoldemView::PreCreateWindow(CREATESTRUCT& cs) 
{
	__TRACE
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CView::PreCreateWindow(cs);
}

void COpenHoldemView::OnInitialUpdate() 
{
	__TRACE
	CView::OnInitialUpdate();

	// Timer to check for display updates
	SetTimer(DISPLAY_UPDATE_TIMER, 103, 0);
}

// COpenHoldemView drawing
void COpenHoldemView::OnDraw(CDC* pDC) 
{
	__TRACE
	UpdateDisplay(true);
}

void COpenHoldemView::OnTimer(UINT nIDEvent) 
{
	__TRACE
	if (nIDEvent == DISPLAY_UPDATE_TIMER) 
	{
		// Only do this if we are not in the middle of a scraper/symbol update
		if (TryEnterCriticalSection(&p_heartbeat_thread->cs_update_in_progress))
		{
			UpdateDisplay(false);
			__TRACE
			LeaveCriticalSection(&p_heartbeat_thread->cs_update_in_progress);
			__TRACE
		}
	}
	// Otherwise: continue with parent class
	__TRACE
	CView::OnTimer(nIDEvent);
	__TRACE
}

void COpenHoldemView::UpdateDisplay(const bool update_all) 
{
	__TRACE
	bool		update_it = false;
	CDC			*pDC = GetDC();

	CString		sym_handnumber = p_handreset_detector->GetHandNumber();
	double		sym_bblind = p_symbol_engine_tablelimits->bblind();
	double		sym_sblind = p_symbol_engine_tablelimits->sblind();
	double		sym_ante = p_symbol_engine_tablelimits->ante();
	int			sym_lim = p_symbol_engine_tablelimits->gametype();
	bool		sym_istournament = p_symbol_engine_istournament->istournament();
	double		sym_pot = p_symbol_engine_chip_amounts->pot();

	// Get size of current client window
	GetClientRect(&_client_rect);

	// Set background color (light gray)
	if (update_all) 
	{
		CBrush backBrush(COLOR_GRAY);
		CBrush* pOldBrush = pDC->SelectObject(&backBrush);
		pDC->PatBlt(_client_rect.left, _client_rect.top, _client_rect.right-_client_rect.left, _client_rect.bottom-_client_rect.top, PATCOPY);
		pDC->SelectObject(pOldBrush);
	}

	// Draw center info box
	update_it = false;
	if (_handnumber_last != sym_handnumber) 
	{
		_handnumber_last = sym_handnumber;
		update_it = true;
	}
	if (_sblind_last != sym_sblind) 
	{
		_sblind_last = sym_sblind;
		update_it = true;
	}
	if (_bblind_last != sym_bblind) 
	{
		_bblind_last = sym_bblind;
		update_it = true;
	}
	if (_lim_last != sym_lim) 
	{
		_lim_last = sym_lim;
		update_it = true;
	}
	if (_istournament_last != sym_istournament) 
	{
		_istournament_last = sym_istournament;
		update_it = true;
	}
	if (_ante_last != sym_ante != 0) 
	{
		_ante_last = sym_ante;
		update_it = true;
	}
	if (_pot_last != sym_pot) 
	{
		_pot_last = sym_pot;
		update_it = true;
	}

	if (preferences.log_symbol_enabled() || update_it || update_all) 
	{
		DrawCenterInfoBox();
	}

	// Draw button state indicators
	DrawButtonIndicators();

	// Draw common cards
	for (int i=0; i<k_number_of_community_cards; i++) 
	{
    Card *p_card = &p_table_state->_common_cards[i];
    int card_value = p_table_state->_common_cards[i].GetValue();
		if (_card_common_last[i] != card_value || update_all) 
		{
			_card_common_last[i] = card_value;
			write_log(preferences.debug_gui(), "[GUI] COpenHoldemView::UpdateDisplay() Drawing common card %i: [%s]\n",
        i, p_card->ToString());
			DrawCard(p_card,
					  _client_rect.right/2 + cc[i][0], _client_rect.bottom/2 + cc[i][1],
					  _client_rect.right/2 + cc[i][0] + CARDSIZEX, _client_rect.bottom/2 + cc[i][1] + CARDSIZEY,
					  false);
		}
	}

	// Draw collection of player info
	for (int i=0; i<p_tablemap->nchairs(); i++) 
	{
		write_log(preferences.debug_gui(), "[GUI] COpenHoldemView::UpdateDisplay() checking changes for chair %i\n", i);
		// Figure out if we need to redraw this seat
		update_it = false;
		if (_seated_last[i] != p_scraper->seated(i) ||
			_active_last[i] != p_scraper->active(i)) 
		{
			_seated_last[i] = p_scraper->seated(i);
			_active_last[i] = p_scraper->active(i);
			update_it = true;
		}
    if (_card_player_last[i][0] != p_table_state->_players[i].hole_cards[0].GetValue()
        || _card_player_last[i][1] != p_table_state->_players[i].hole_cards[1].GetValue()) 		{
			_card_player_last[i][0] = p_table_state->_players[i].hole_cards[0].GetValue();
			_card_player_last[i][1] = p_table_state->_players[i].hole_cards[1].GetValue();
			update_it = true;
		}
		if (_dealer_last[i] != p_scraper->dealer(i)) 
		{
			_dealer_last[i] = p_scraper->dealer(i);
			update_it = true;
		}
		if (_playername_last[i] != p_scraper->player_name(i)) 
		{
			_playername_last[i] = p_scraper->player_name(i);
			update_it = true;
		}
		if (_playerbalance_last[i] != p_scraper->player_balance(i)) 
		{
			_playerbalance_last[i] = p_scraper->player_balance(i);
			update_it = true;
		}
		if (_playerbet_last[i] != p_scraper->player_bet(i)) 
		{
			_playerbet_last[i] = p_scraper->player_bet(i);
			update_it = true;
		}

		if (update_it || update_all) 
		{
			write_log(preferences.debug_gui(), "[GUI] COpenHoldemView::UpdateDisplay() updating chair %i\n", i);
			// Draw active circle
			if (p_string_match->IsStringSeated(p_scraper->seated(i))) 
			{
				DrawSeatedActiveCircle(i);
				// Draw cards first, because we want the name 
				// to occlude the cards and not the other way.
				DrawPlayerCards(i);
				DrawNameBox(i);
				DrawBalanceBox(i);
			}
			// Drawing a bet, even if no player seated.
			// The player might have left the table, 
			// but depending on casinos potmethod a bet might still be there.
			DrawPlayerBet(i);
		}
		// Draw dealer button
		// At some casinos the dealer can be at an empty seat.
		// Therefore we draw the dealer-button anyway, inependent of "seated" and "active".
		// Draw it at the very last, as we want to have it at the top of the cards.
		if (p_scraper->dealer(i))
		{
			DrawDealerButton(i);
		}
	}
	write_log(preferences.debug_gui(), "[GUI] COpenHoldemView::UpdateDisplay() Update finished\n");
	ReleaseDC(pDC);
	write_log(preferences.debug_gui(), "[GUI] COpenHoldemView::UpdateDisplay() DC released\n");
}

void COpenHoldemView::DrawCenterInfoBox(void) 
{
	__TRACE
	CPen		*pTempPen = NULL, oldpen;
	CBrush		*pTempBrush = NULL, oldbrush;
	RECT		rect = {0};
	CString		t = "", s = "";
	CFont		*oldfont = NULL, cFont;
	int			left = 0, top = 0, right = 0, bottom = 0;
	CDC			*pDC = GetDC();
		
	double sym_bblind		= p_symbol_engine_tablelimits->bblind();
	double sym_sblind		= p_symbol_engine_tablelimits->sblind();
	double sym_ante			= p_symbol_engine_tablelimits->ante();
	int sym_lim				= p_symbol_engine_tablelimits->gametype();
	CString sym_handnumber	= p_handreset_detector->GetHandNumber();
	double sym_pot			= p_symbol_engine_chip_amounts->pot();
	bool sym_playing		= p_table_state->_players[USER_CHAIR].HasKnownCards();

	// "White box" in the OpenHoldem-GUI with basic important info
	const int k_basic_height = 2;				// pixels
	const int k_extra_height_per_line = 16;	// pixels
	const int k_number_of_default_lines = 4;	// hand-number, game-type, ante, pot
	int height = k_basic_height 
		+ k_extra_height_per_line * k_number_of_default_lines;
	if (preferences.log_symbol_enabled())
	{
		// Extra lines for symbol-logging
		height += k_extra_height_per_line * preferences.log_symbol_max_log();
	}

	// Figure placement of box
	left = _client_rect.right/2-70;
	top = 4;
	right = _client_rect.right/2+70;
	bottom = top+height;

	pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
	oldpen.FromHandle((HPEN)pTempPen);					// Save old pen
	pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
	oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

	pDC->SetBkMode(OPAQUE);
	pDC->Rectangle(left, top, right, bottom);

	// Set font basics
	_logfont.lfHeight = -12;
	_logfont.lfWeight = FW_NORMAL;
	cFont.CreateFontIndirect(&_logfont);
	oldfont = pDC->SelectObject(&cFont);
	pDC->SetTextColor(COLOR_BLACK);

	// Set rectangle
	rect.left = left;
	rect.top = top;
	rect.right = right;
	rect.bottom = bottom;

	t = "";
	// handnumber
	if (sym_handnumber != "") 
	{
		s.Format("  Hand #: %s\n", sym_handnumber);
	}
	else 
	{
		s.Format("  Hand #: -\n");
	}
	t.Append(s);

	CString format_string;
	// blinds/type
	if ((int) sym_sblind != sym_sblind || (int) sym_bblind != sym_bblind) 
	{
		// Fractional nunbers: use 2.00 digits  
		format_string = "  %s%s %.2f/%.2f/%.2f\n";
	}
	else
	{
		// Display as integer numbers
		format_string = "  %s %.0f/%.0f/%.0f\n";
	}
	s.Format(format_string,
		p_symbol_engine_tablelimits->GetGametypeAsString(),
		sym_sblind, sym_bblind, p_symbol_engine_tablelimits->bigbet());
	t.Append(s);

	// ante
	if (sym_ante != 0) 
	{
		s.Format("  Ante: %s\n", Number2CString(sym_ante));
		t.Append(s);
	}

	// Pot
	s.Format("  Pot: %s\n", Number2CString(sym_pot));
	t.Append(s);

	if (preferences.log_symbol_enabled() 
		&& p_symbol_engine_userchair->userchair_confirmed() 
		&& p_table_state->_players[USER_CHAIR].HasKnownCards()) {
      t.Append(p_autoplayer_trace->LogSymbolsForGUI());
	}

	// Draw it
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(t.GetString(), t.GetLength(), &rect, NULL);

	// Restore original pen, brush and font
	pDC->SelectObject(oldpen);
	pDC->SelectObject(oldbrush);
	pDC->SelectObject(oldfont);
	cFont.DeleteObject();
	ReleaseDC(pDC);
}

void COpenHoldemView::DrawButtonIndicators(void) 
{
	__TRACE
	bool		fold_drawn, call_drawn, check_drawn, raise_drawn, allin_drawn;
	bool		autopost_drawn, sitin_drawn, sitout_drawn, leave_drawn, prefold_drawn = false;

	autopost_drawn = sitin_drawn = sitout_drawn = leave_drawn = prefold_drawn = false;
	fold_drawn = call_drawn = check_drawn = raise_drawn = allin_drawn = false;

	for (int i=0; i<k_max_number_of_players; i++) 
	{
		// Draw "on" buttons
		if (p_scraper->GetButtonState(i)) 
		{
			if (p_string_match->IsStringFold(p_scraper->button_label(i))) 
			{
				DrawSpecificButtonIndicator(i, 'F', _client_rect.right-84, _client_rect.bottom-16, _client_rect.right-70, _client_rect.bottom-2);
				fold_drawn = true;
			}
			else if (p_string_match->IsStringCall(p_scraper->button_label(i))) 
			{
				DrawSpecificButtonIndicator(i, 'C', _client_rect.right-67, _client_rect.bottom-16, _client_rect.right-53, _client_rect.bottom-2);
				call_drawn = true;
			}
			else if (p_string_match->IsStringCheck(p_scraper->button_label(i))) 
			{
				DrawSpecificButtonIndicator(i, 'K', _client_rect.right-50, _client_rect.bottom-16, _client_rect.right-36, _client_rect.bottom-2);
				check_drawn = true;
			}
			else if (p_string_match->IsStringRaise(p_scraper->button_label(i))) 
			{
				DrawSpecificButtonIndicator(i, 'R', _client_rect.right-33, _client_rect.bottom-16, _client_rect.right-19, _client_rect.bottom-2);
				raise_drawn = true;
			}
			else if (p_string_match->IsStringAllin(p_scraper->button_label(i))) 
			{
				DrawSpecificButtonIndicator(i, 'A', _client_rect.right-16, _client_rect.bottom-16, _client_rect.right-2, _client_rect.bottom-2);
				allin_drawn = true;
			}
			else if (p_string_match->IsStringAutopost(p_scraper->button_label(i))) 
			{
				DrawSpecificButtonIndicator(i, 'T', _client_rect.left+2,  _client_rect.bottom-16, _client_rect.left+16, _client_rect.bottom-2);
				autopost_drawn = true;
			}
			else if (p_string_match->IsStringSitin(p_scraper->button_label(i))) 
			{
				DrawSpecificButtonIndicator(i, 'I', _client_rect.left+19, _client_rect.bottom-16, _client_rect.left+33, _client_rect.bottom-2);
				sitin_drawn = true;
			}
			else if (p_string_match->IsStringSitout(p_scraper->button_label(i))) 
			{
				DrawSpecificButtonIndicator(i, 'O', _client_rect.left+36, _client_rect.bottom-16, _client_rect.left+50, _client_rect.bottom-2);
				sitout_drawn = true;
			}
			else if (p_string_match->IsStringLeave(p_scraper->button_label(i))) 
			{
				DrawSpecificButtonIndicator(i, 'L', _client_rect.left+53, _client_rect.bottom-16, _client_rect.left+67, _client_rect.bottom-2);
				leave_drawn = true;
			}
			else if (p_string_match->IsStringPrefold(p_scraper->button_label(i))) 
			{
				DrawSpecificButtonIndicator(i, 'P', _client_rect.left+70, _client_rect.bottom-16, _client_rect.left+84, _client_rect.bottom-2);
				prefold_drawn = true;
			}
		}
	}

	// Draw "off" buttons
	if (!fold_drawn) 
		DrawSpecificButtonIndicator(-1, 'F', _client_rect.right-84, _client_rect.bottom-16, _client_rect.right-70, _client_rect.bottom-2);

	if (!call_drawn) 
		DrawSpecificButtonIndicator(-1, 'C', _client_rect.right-67, _client_rect.bottom-16, _client_rect.right-53, _client_rect.bottom-2);

	if (!check_drawn)
		DrawSpecificButtonIndicator(-1, 'K', _client_rect.right-50, _client_rect.bottom-16, _client_rect.right-36, _client_rect.bottom-2);

	if (!raise_drawn)
		DrawSpecificButtonIndicator(-1, 'R', _client_rect.right-33, _client_rect.bottom-16, _client_rect.right-19, _client_rect.bottom-2);

	if (!allin_drawn)
		DrawSpecificButtonIndicator(-1, 'A', _client_rect.right-16, _client_rect.bottom-16, _client_rect.right-2, _client_rect.bottom-2);

	if (!autopost_drawn) 
		DrawSpecificButtonIndicator(-1, 'T', _client_rect.left+2,  _client_rect.bottom-16, _client_rect.left+16, _client_rect.bottom-2);

	if (!sitin_drawn) 
		DrawSpecificButtonIndicator(-1, 'I', _client_rect.left+19, _client_rect.bottom-16, _client_rect.left+33, _client_rect.bottom-2);

	if (!sitout_drawn)
		DrawSpecificButtonIndicator(-1, 'O', _client_rect.left+36, _client_rect.bottom-16, _client_rect.left+50, _client_rect.bottom-2);

	if (!leave_drawn)
		DrawSpecificButtonIndicator(-1, 'L', _client_rect.left+53, _client_rect.bottom-16, _client_rect.left+67, _client_rect.bottom-2);

	if (!prefold_drawn)
		DrawSpecificButtonIndicator(-1, 'P', _client_rect.left+70, _client_rect.bottom-16, _client_rect.left+84, _client_rect.bottom-2);
}

void COpenHoldemView::DrawSpecificButtonIndicator(const int button_num, const char ch, const int left, 
												  const int top, const int right, const int bottom) 
{
	__TRACE
	CPen		*pTempPen = NULL, oldpen;
	CBrush		*pTempBrush = NULL, oldbrush;
	RECT		rect = {0};
	CFont		*oldfont = NULL, cFont;
	CString		t = "";
	CDC			*pDC = GetDC();

	// Set font basics
	_logfont.lfHeight = -12;
	_logfont.lfWeight = FW_NORMAL;
	cFont.CreateFontIndirect(&_logfont);
	oldfont = pDC->SelectObject(&cFont);

	// Background color
	pDC->SetBkColor(COLOR_GRAY);

	if (button_num == k_undefined) 
	{
		pTempPen = (CPen*)pDC->SelectObject(&_white_dot_pen);
		pTempBrush = (CBrush*)pDC->SelectObject(&_gray_brush);
		pDC->SetTextColor(COLOR_WHITE);
	}
	else 
	{
		if (p_scraper->GetButtonState(button_num)) 
		{
			if (ch=='F') 
			{
				pTempPen = (CPen*)pDC->SelectObject(&_red_pen);
				pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
				pDC->SetTextColor(COLOR_RED);
			}
			else if (ch=='C') 
			{
				pTempPen = (CPen*)pDC->SelectObject(&_blue_pen);
				pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
				pDC->SetTextColor(COLOR_BLUE);
			}
			else if (ch=='K') 
			{
				pTempPen = (CPen*)pDC->SelectObject(&_blue_pen);
				pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
				pDC->SetTextColor(COLOR_BLUE);
			}
			else if (ch=='R') 
			{
				pTempPen = (CPen*)pDC->SelectObject(&_green_pen);
				pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
				pDC->SetTextColor(COLOR_GREEN);
			}
			else if (ch=='A') 
			{
				pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
				pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
				pDC->SetTextColor(COLOR_BLACK);
			}
			else if (ch=='T') 
			{
				pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
				pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
				pDC->SetTextColor(COLOR_BLUE);
			}
			else if (ch=='I') 
			{
				pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
				pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
				pDC->SetTextColor(COLOR_GREEN);
			}
			else if (ch=='O') 
			{
				pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
				pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
				pDC->SetTextColor(COLOR_YELLOW);
			}
			else if (ch=='L') 
			{
				pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
				pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
				pDC->SetTextColor(COLOR_RED);
			}
			else if (ch=='P') 
			{
				pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
				pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
				pDC->SetTextColor(COLOR_BLACK);
			}
		}
		else 
		{
			pTempPen = (CPen*)pDC->SelectObject(&_white_dot_pen);
			pTempBrush = (CBrush*)pDC->SelectObject(&_gray_brush);
			pDC->SetTextColor(COLOR_WHITE);
		}
	}

	oldpen.FromHandle((HPEN)pTempPen);					// Save old pen
	oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

	pDC->SetBkMode(OPAQUE);
	pDC->Rectangle(left, top, right, bottom);

	// Set rectangle
	rect.left = left;
	rect.top = top;
	rect.right = right;
	rect.bottom = bottom;

	t.Format("%c", ch);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(t.GetString(), t.GetLength(), &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	// Restore original pen and brush
	pDC->SelectObject(oldpen);
	pDC->SelectObject(oldbrush);
	pDC->SelectObject(oldfont);
	cFont.DeleteObject();
	ReleaseDC(pDC);
}

void COpenHoldemView::DrawSeatedActiveCircle(const int chair) 
{
	__TRACE
	CPen		*pTempPen = NULL, oldpen;
	CBrush		*pTempBrush = NULL, oldbrush;
	int			left = 0, top = 0, right = 0, bottom = 0;
	CDC			*pDC = GetDC();

	// Background color
	pDC->SetBkColor(COLOR_GRAY);

	// Figure placement of circle
	left = _client_rect.right * pc[p_tablemap->nchairs()][chair][0] - CARDSIZEX - 6;
	top = _client_rect.bottom * pc[p_tablemap->nchairs()][chair][1] - CARDSIZEX - 5;
	right = _client_rect.right * pc[p_tablemap->nchairs()][chair][0] + CARDSIZEX + 5;
	bottom = _client_rect.bottom * pc[p_tablemap->nchairs()][chair][1] + CARDSIZEX + 5;

	pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
	oldpen.FromHandle((HPEN)pTempPen);					// Save old pen

	if (p_scraper_access->IsPlayerActive(chair))
	{
		pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
	}
	else 
	{
		pTempBrush = (CBrush*)pDC->SelectObject(&_gray_brush);
	}
	oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

	pDC->Ellipse(left, top, right, bottom);

	// Restore original pen and brush
	pDC->SelectObject(oldpen);
	pDC->SelectObject(oldbrush);
	ReleaseDC(pDC);
}

void COpenHoldemView::DrawDealerButton(const int chair) 
{
	__TRACE
	CPen		*pTempPen = NULL, oldpen;
	CBrush		*pTempBrush = NULL, oldbrush;
	int			left = 0, top = 0, right = 0, bottom = 0;
	CDC			*pDC = GetDC();

	// Background color
	pDC->SetBkColor(COLOR_GRAY);

	// Figure placement of circle
	left = _client_rect.right * pc[p_tablemap->nchairs()][chair][0] - 8;
	top = _client_rect.bottom * pc[p_tablemap->nchairs()][chair][1] - 8;
	right = _client_rect.right * pc[p_tablemap->nchairs()][chair][0] + 8;
	bottom = _client_rect.bottom * pc[p_tablemap->nchairs()][chair][1] + 8;


	pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
	oldpen.FromHandle((HPEN)pTempPen);					// Save old pen
	pTempBrush = (CBrush*)pDC->SelectObject(&_red_brush);
	oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

	pDC->Ellipse(left, top, right, bottom);

	// Restore original pen and brush
	pDC->SelectObject(oldpen);
	pDC->SelectObject(oldbrush);
	ReleaseDC(pDC);
}

void COpenHoldemView::DrawCard(Card *card, const int left, const int top, 
							   const int right, const int bottom, const bool pl_card) 
{
	__TRACE
	CPen		*pTempPen = NULL, oldpen;
	CBrush		*pTempBrush = NULL, oldbrush;
	RECT		rrect = {0}, srect = {0};
	CFont		*oldfont = NULL, cFont;
	CDC			*pDC = GetDC();

	// Set font basics
	_logfont.lfHeight = -24;
	_logfont.lfWeight = FW_BOLD;
	cFont.CreateFontIndirect(&_logfont);
	oldfont = pDC->SelectObject(&cFont);

	// Background color
	pDC->SetBkColor(COLOR_GRAY);

	// CARD BACK
  if (card->IsCardBack()) 
	{
		pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
		oldpen.FromHandle((HPEN)pTempPen);					// Save old pen
		pTempBrush = (CBrush*)pDC->SelectObject(&_yellow_brush);
		oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

		pDC->SetBkMode(OPAQUE);
		pDC->RoundRect(left, top, right, bottom, 5, 5);

		// Restore original pen and brush
		pDC->SelectObject(oldpen);
		pDC->SelectObject(oldbrush);
	}

	// NO CARD
  else if (card->IsNoCard())
	{
		pTempPen = (CPen*)pDC->SelectObject(&_white_dot_pen);
		oldpen.FromHandle((HPEN)pTempPen);					// Save old pen
		pTempBrush = (CBrush*)pDC->SelectObject(&_gray_brush);
		oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

		pDC->SetBkMode(OPAQUE);
		pDC->RoundRect(left, top, right, bottom, 5, 5);

		// Restore original pen and brush
		pDC->SelectObject(oldpen);
		pDC->SelectObject(oldbrush);
	}

	// NORMAL CARD
	else 
	{
		// Rect for rank and suit location
		rrect.left = left;
		rrect.top = top;
		rrect.right = right;
		rrect.bottom = top + ((bottom-top)/2);
		srect.left = left;
		srect.top = top + ((bottom-top)/2) + 1 - (pl_card ? 7 : 0);
		srect.right = right;
		srect.bottom = bottom - (pl_card ? 7 : 0);

		// Draw card rect and suit
		pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
		oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

		// Set colors
    switch (card->GetSuit())
		{
			case Suit_CLUBS:
				pDC->SetTextColor(COLOR_GREEN);
				pTempPen = (CPen*)pDC->SelectObject(&_green_pen);
				break;
			case Suit_DIAMONDS:
				pDC->SetTextColor(COLOR_BLUE);
				pTempPen = (CPen*)pDC->SelectObject(&_blue_pen);
				break;
			case Suit_HEARTS:
				pDC->SetTextColor(COLOR_RED);
				pTempPen = (CPen*)pDC->SelectObject(&_red_pen);
				break;
			case Suit_SPADES:
				pDC->SetTextColor(COLOR_BLACK);
				pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
				break;
			default:
				pTempPen = (CPen*)pDC->GetCurrentPen();
				break;
		}
		oldpen.FromHandle((HPEN)pTempPen);					// Save old pen

		// Draw card and suit
		pDC->SetBkMode(OPAQUE);
		pDC->RoundRect(left, top, right, bottom, 5, 5);
		pDC->SetBkMode(TRANSPARENT);
    // Draw card rank and suit
    pDC->DrawText(CString(card->GetSuitCharacter(true)), 
      -1, &srect, DT_CENTER | DT_SINGLELINE | DT_VCENTER );
    pDC->DrawText(CString(card->GetRankCharacter()), 
      -1, &rrect, DT_CENTER | DT_SINGLELINE | DT_VCENTER );

		// Restore original pen and brush
		pDC->SelectObject(oldpen);
		pDC->SelectObject(oldbrush);
	}

	pDC->SelectObject(oldfont);
	cFont.DeleteObject();
	ReleaseDC(pDC);
}

void COpenHoldemView::DrawNameBox(const int chair) 
{
	__TRACE
	CPen		*pTempPen = NULL, oldpen;
	CBrush		*pTempBrush = NULL, oldbrush;
	RECT		textrect = {0}, drawrect = {0};
	CFont		*oldfont = NULL, cFont;
	int			left = 0, top = 0, right = 0, bottom = 0;
	CDC			*pDC = GetDC();
	static RECT	name_rect_last[10] = {0};

	// Background color
	pDC->SetBkColor(COLOR_GRAY);

	// Figure placement of box
	left = _client_rect.right * pc[p_tablemap->nchairs()][chair][0] - 36;
	top = _client_rect.bottom * pc[p_tablemap->nchairs()][chair][1] + 15;
	right = _client_rect.right * pc[p_tablemap->nchairs()][chair][0] + 35;
	bottom = _client_rect.bottom * pc[p_tablemap->nchairs()][chair][1] + 30;

	// Set font basics
	_logfont.lfHeight = -12;
	_logfont.lfWeight = FW_NORMAL;
	cFont.CreateFontIndirect(&_logfont);
	oldfont = pDC->SelectObject(&cFont);
	pDC->SetTextColor(COLOR_BLACK);

	if (p_scraper_access->IsPlayerSeated(chair) 
		|| p_scraper_access->IsPlayerActive(chair)) 
		/*|| (p_tablemap->r$pXseated_index[chair] == k_undefined && p_tablemap->r$uXseated_index[chair] == k_undefined &&
		|| p_tablemap->r$pXactive_index[chair] == k_undefined && p_tablemap->r$uXactive_index[chair] == k_undefined)*/  
	{

		pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
		oldpen.FromHandle((HPEN)pTempPen);					// Save old pen
		pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
		oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

		// Calc rectangle size for text
		textrect.left = 0;
		textrect.top = 0;
		textrect.right = 0;
		textrect.bottom = 0;
		pDC->DrawText(p_scraper->player_name(chair).GetString(), p_scraper->player_name(chair).GetLength(), &textrect, DT_CALCRECT);

		// Figure out placement of rectangle
		drawrect.left = left < (left+(right-left)/2)-textrect.right/2-3 ? left : (left+(right-left)/2)-textrect.right/2-3;
		drawrect.top = top;
		drawrect.right = right > (left+(right-left)/2)+textrect.right/2+3 ? right : (left+(right-left)/2)+textrect.right/2+3;
		drawrect.bottom = bottom;

		// Invalidate everything if the name has decreased in width
		if (name_rect_last[chair].right - name_rect_last[chair].left != drawrect.right - drawrect.left) 
		{
			InvalidateRect(NULL, true);
		}

		// Draw it
		pDC->SetBkMode(OPAQUE);
		pDC->Rectangle(drawrect.left, drawrect.top, drawrect.right, drawrect.bottom);
		pDC->SetBkMode(TRANSPARENT);
		pDC->DrawText(p_scraper->player_name(chair).GetString(), p_scraper->player_name(chair).GetLength(), &drawrect,
					  DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		name_rect_last[chair].left = drawrect.left;
		name_rect_last[chair].top = drawrect.top;
		name_rect_last[chair].right = drawrect.right;
		name_rect_last[chair].bottom = drawrect.bottom;
	}
	else 
	{
		pTempPen = (CPen*)pDC->SelectObject(&_white_dot_pen);
		oldpen.FromHandle((HPEN)pTempPen);					// Save old pen
		pTempBrush = (CBrush*)pDC->SelectObject(&_gray_brush);
		oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

		pDC->SetBkMode(OPAQUE);
		pDC->Rectangle(left, top, right, bottom);
	}

	// Restore original pen and brush
	pDC->SelectObject(oldpen);
	pDC->SelectObject(oldbrush);
	pDC->SelectObject(oldfont);
	cFont.DeleteObject();
	ReleaseDC(pDC);
}

void COpenHoldemView::DrawBalanceBox(const int chair) 
{
	__TRACE
	CPen		*pTempPen = NULL, oldpen;
	CBrush		*pTempBrush = NULL, oldbrush;
	RECT		textrect = {0}, drawrect = {0};
	CFont		*oldfont = NULL, cFont;
	CString		t = "";
	int			left = 0, top = 0, right = 0, bottom = 0;
	CDC			*pDC = GetDC();
	static RECT	balance_rect_last[10] = {0};

	// Background color
	pDC->SetBkColor(COLOR_GRAY);

	// Figure placement of box
	left = _client_rect.right * pc[p_tablemap->nchairs()][chair][0] - 26;
	top = _client_rect.bottom * pc[p_tablemap->nchairs()][chair][1] + 30;
	right = _client_rect.right * pc[p_tablemap->nchairs()][chair][0] + 25;
	bottom = _client_rect.bottom * pc[p_tablemap->nchairs()][chair][1] + 45;

	// Set font basics
	_logfont.lfHeight = -12;
	_logfont.lfWeight = FW_NORMAL;
	cFont.CreateFontIndirect(&_logfont);
	oldfont = pDC->SelectObject(&cFont);
	pDC->SetTextColor(COLOR_BLACK);

	if (p_scraper_access->IsPlayerSeated(chair) 
		|| p_scraper_access->IsPlayerActive(chair)) /*||
		(p_tablemap->r$pXseated_index[chair] == k_undefined && p_tablemap->r$uXseated_index[chair] == k_undefined &&
		 p_tablemap->r$pXactive_index[chair] == k_undefined && p_tablemap->r$uXactive_index[chair] == k_undefined)*/ 
	{

		pTempPen = (CPen*)pDC->SelectObject(&_black_pen);
		oldpen.FromHandle((HPEN)pTempPen);					// Save old pen
		pTempBrush = (CBrush*)pDC->SelectObject(&_white_brush);
		oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

		// Format Text
		if (!p_scraper->sitting_out(chair)) 
		{
			t = Number2CString(p_scraper->player_balance(chair));
		}
		else 
		{
			t.Format("Out (%s)", Number2CString(p_scraper->player_balance(chair)));
		}
	}
	else 
	{
		pTempPen = (CPen*)pDC->SelectObject(&_white_dot_pen);
		oldpen.FromHandle((HPEN)pTempPen);					// Save old pen
		pTempBrush = (CBrush*)pDC->SelectObject(&_gray_brush);
		oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush

		t = "";
	}

	// Calc rectangle size for text
	textrect.left = 0;
	textrect.top = 0;
	textrect.right = 0;
	textrect.bottom = 0;
	pDC->DrawText(t.GetString(), t.GetLength(), &textrect, DT_CALCRECT);

	// Figure out placement of rectangle
	drawrect.left = left < (left+(right-left)/2)-textrect.right/2-3 ? left : (left+(right-left)/2)-textrect.right/2-3;
	drawrect.top = top;
	drawrect.right = right > (left+(right-left)/2)+textrect.right/2+3 ? right : (left+(right-left)/2)+textrect.right/2+3;
	drawrect.bottom = bottom;

	// Invalidate everything if the balance has decreased in width
	if (balance_rect_last[chair].right - balance_rect_last[chair].left != drawrect.right - drawrect.left) 
	{
		InvalidateRect(NULL, true);
	}

	// Draw it
	pDC->SetBkMode(OPAQUE);
	pDC->Rectangle(drawrect.left, drawrect.top, drawrect.right, drawrect.bottom);
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(t.GetString(), t.GetLength(), &drawrect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	balance_rect_last[chair].left = drawrect.left;
	balance_rect_last[chair].top = drawrect.top;
	balance_rect_last[chair].right = drawrect.right;
	balance_rect_last[chair].bottom = drawrect.bottom;

	// Restore original pen and brush
	pDC->SelectObject(oldpen);
	pDC->SelectObject(oldbrush);
	pDC->SelectObject(oldfont);
	cFont.DeleteObject();
	ReleaseDC(pDC);
}

void COpenHoldemView::DrawPlayerBet(const int chair) 
{
	__TRACE
	CPen		*pTempPen = NULL, oldpen;
	CBrush		*pTempBrush = NULL, oldbrush;
	RECT		textrect = {0}, drawrect = {0};
	CFont		*oldfont = NULL, cFont;
	CString		t = "";
	
	static RECT	bet_rect_last[10] = {0};
	int			xcenter = 0, ycenter = 0, xadj = 0, yadj = 0;
	CDC			*pDC = GetDC();

	// Draw background colored rectangle over position of previous bet to erase it
	pTempPen = (CPen*)pDC->SelectObject(&_null_pen);
	oldpen.FromHandle((HPEN)pTempPen);					// Save old pen
	pTempBrush = (CBrush*)pDC->SelectObject(&_gray_brush);
	oldbrush.FromHandle((HBRUSH)pTempBrush);			// Save old brush
	pDC->Rectangle(bet_rect_last[chair].left, bet_rect_last[chair].top,
				   bet_rect_last[chair].right, bet_rect_last[chair].bottom);
	pDC->SelectObject(oldpen);
	pDC->SelectObject(oldbrush);

	// Background color
	pDC->SetBkColor(COLOR_GRAY);

	// Figure placement
	xcenter = _client_rect.right * pc[p_tablemap->nchairs()][chair][0];
	ycenter = _client_rect.bottom * pc[p_tablemap->nchairs()][chair][1];
	xadj = pcbet[p_tablemap->nchairs()][chair][0];
	yadj = pcbet[p_tablemap->nchairs()][chair][1];

	// Set font basics
	_logfont.lfHeight = -12;
	_logfont.lfWeight = FW_NORMAL;
	cFont.CreateFontIndirect(&_logfont);
	oldfont = pDC->SelectObject(&cFont);
	pDC->SetTextColor(COLOR_BLACK);

	// Format text
	if (p_scraper->player_bet(chair) != 0) 
	{
		t = Number2CString(p_scraper->player_bet(chair));
	}
	else 
	{
		t = "";
	}

	// Calc rectangle size for text
	textrect.left = 0;
	textrect.top = 0;
	textrect.right = 0;
	textrect.bottom = 0;
	pDC->DrawText(t.GetString(), t.GetLength(), &textrect, DT_CALCRECT);

	// Figure out placement of rectangle
	if (xadj<0) 
	{
		drawrect.left = xcenter + xadj - textrect.right;
		drawrect.top = ycenter + yadj - textrect.bottom/2;
		drawrect.right = xcenter + xadj;
		drawrect.bottom = ycenter + yadj + textrect.bottom/2;
	}
	else if (xadj>0) 
	{
		drawrect.left = xcenter + xadj;
		drawrect.top = ycenter + yadj - textrect.bottom/2;
		drawrect.right = xcenter + xadj + textrect.right;
		drawrect.bottom = ycenter + yadj + textrect.bottom/2;
	}
	else	// xadj==0
	{  
		drawrect.left = xcenter + xadj - textrect.right/2;
		drawrect.top = ycenter + yadj - textrect.bottom/2;
		drawrect.right = xcenter + xadj + textrect.right/2;
		drawrect.bottom = ycenter + yadj + textrect.bottom/2;
	}

	// Draw it
	pDC->SetBkMode(OPAQUE);
	pDC->DrawText(t.GetString(), t.GetLength(), &drawrect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	bet_rect_last[chair].left = drawrect.left;
	bet_rect_last[chair].top = drawrect.top;
	bet_rect_last[chair].right = drawrect.right;
	bet_rect_last[chair].bottom = drawrect.bottom;

	cFont.DeleteObject();
	ReleaseDC(pDC);
}

void COpenHoldemView::DrawPlayerCards(const int chair) 
{
	__TRACE
	if (!p_scraper_access->IsPlayerActive(chair))
	{
		// Forget about inactive players, they have no cards.
		// Don't draw them to point out the mistake faster
		// for newbies with bad tablemaps.
		return;
	}
	// Get size of current client window
	GetClientRect(&_client_rect);
	// Draw player cards (first)
  Card *player_card_0 = &p_table_state->_players[chair].hole_cards[0];
	write_log(preferences.debug_gui(), "[GUI] COpenHoldemView::UpdateDisplay() Drawing card 0 of player %i: [%s]\n",
    chair, player_card_0->ToString());
  int pos_x_right  = _client_rect.right * pc[p_tablemap->nchairs()][chair][0] - 2;
  int pos_x_left   = pos_x_right - CARDSIZEX;
  int pos_y_top    = _client_rect.bottom * pc[p_tablemap->nchairs()][chair][1] - CARDSIZEY/2;
  int pos_y_bottom = pos_y_top + CARDSIZEY/2 - 1;
	DrawCard(player_card_1, pos_x_left, pos_y_top, pos_x_right, pos_y_bottom,	true);

  // Draw player cards (second)
  Card *player_card_1 = &p_table_state->_players[chair].hole_cards[1];
  pos_x_right = pos_x_right + CARDSIZEX + 3;
  pos_x_left  = pos_x_left + CARDSIZEX + 3;
	DrawCard(player_card_1, pos_x_left, pos_y_top, pos_x_right, pos_y_bottom,	true);
}