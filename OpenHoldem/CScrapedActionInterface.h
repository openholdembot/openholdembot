//******************************************************************************
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//******************************************************************************
//
// Purpose: Data container for scraped buttons etc.
//
//******************************************************************************

#ifndef INC_CSCRAPEDACTIONINTERFACE_H
#define INC_CSCRAPEDACTIONINTERFACE_H

#include "afxstr.h"

class CScrapedActionInterface {
  friend class CCasinoInterface;
  friend class CScraper;
  friend class CScraperAccess;
  friend class CSymbolEngineAutoplayer;
  friend class CSymbolEngineGameType;
  friend class COpenHoldemView;
 public:
  CScrapedActionInterface();
  ~CScrapedActionInterface();
 public:
  void Reset();
 public:
	// public functions
	void GetNeccessaryTablemapObjects();
	void InitOnConnect();
	bool IsValidCard(int Card);
	int	 NumberOfVisibleButtons();
	bool IsKnownCard(int card);
	bool IsPlayerActive(int player);
	bool IsPlayerSeated(int player);
	bool IsGoodPlayername(int chair);
  int  NumberOfCommonCards();
 public:
	bool	IsMyTurn()	{ return (NumberOfVisibleButtons() >= k_min_buttons_needed_for_my_turn); }
 public:
	bool get_betpot_button_visible(int numerator, int denominator); //?????
 public:
   /*
  CString	i86_button_state()         { return _i86_button_state; }
	CString	i86X_button_state(int n)   { RETURN_DEFAULT_IF_OUT_OF_RANGE(n, (k_max_number_of_i86X_buttons-1), "") return _i86X_button_state[n]; }
	CString	betpot_button_state(int n) { RETURN_DEFAULT_IF_OUT_OF_RANGE(n, (k_max_betpot_buttons-1), "")         return _betpot_button_state[n]; }
  const CString		button_state(int n)        { RETURN_DEFAULT_IF_OUT_OF_RANGE(n, k_last_chair, "")      return _button_state[n]; }
  const CString		button_label(int n) { if (n>=0 && n<=9) return _button_label[n]; else return ""; }
	const bool			handle_found_at_xy() { return _handle_found_at_xy; }
	const POINT			handle_xy() { return _handle_xy; }
  */
 private:
	// visible
	bool visible_buttons[k_number_of_standard_functions];
	bool i3_button_visible;
	bool i86_button_visible;
	bool i86X_button_visible[k_max_number_of_i86X_buttons];
	// defined
	bool defined_buttons[k_number_of_standard_functions];
	bool i3_button_defined;
	bool i3_edit_defined;
	bool i3_slider_defined;
	bool i3_handle_defined;
	bool i86_button_defined;
	bool i86X_button_defined[k_max_number_of_i86X_buttons];
	// available, i.e. defined and visible
	bool available_buttons[k_number_of_standard_functions];
	bool i3_button_available;	
	bool i86_button_available;
	bool i86X_button_available[k_max_number_of_i86X_buttons];
 private:
	// private functions
	int		SearchForButtonNumber(int button_code);
	bool	SearchForButtonVisible(int button_code);
	CString	SearchForButtonName(int button_code);
	bool	GetButtonVisible(int button_number);
	bool	GetBetpotButtonVisible(int button_number);
	CString	GetButtonName(int button_number);
 private:
	// button numbers
	int _allin_button_number;
	int _raise_button_number;
	int _call_button_number;
	int _check_button_number;
	int _fold_button_number;
	int _sitin_button_number;
	int _sitout_button_number;
	int _leave_button_number;
	int _prefold_button_number;
	int _autopost_button_number;
 private:
	// button names
	CString button_names[k_number_of_standard_functions];
  CString _i86_button_name;
	CString _i86X_button_name[k_max_number_of_i86X_buttons];
	CString _i3_button_name;
	// region names
	CString _i3_edit_name;
	CString _i3_slider_name;
	CString _i3_handle_name;
 private:
	bool allin_option_available;
 private:
  CString _i86_button_state;
	CString	_i86X_button_state[k_max_number_of_i86X_buttons];
	CString	_button_state[k_max_number_of_buttons];
 private: //!!!!!
	CString _button_label[k_max_number_of_buttons];
 private:
	CString	_betpot_button_state[k_max_betpot_buttons];
  bool    _handle_found_at_xy;
	POINT		_handle_xy;
};

#endif INC_CSCRAPEDACTIONINTERFACE_H