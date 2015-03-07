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

#ifndef INC_OPENHOLDEM_STATUSBAR_H
#define INC_OPENHOLDEM_STATUSBAR_H

class COpenHoldemStatusbar {
 public:
	COpenHoldemStatusbar(CWnd *main_window);
	~COpenHoldemStatusbar();
 public:
	void OnUpdateStatusbar();
	void GetWindowRect(RECT *statusbar_position);
 public:
   void SetLastAction(CString action) { _last_action = action; }
 private:
	void InitAdvancedStatusbar();
	void ComputeCurrentStatus();
  CString LastAction();
 private:
	CStatusBar _status_bar;
	CWnd    *_main_window;
	CString	_status_plcards,  _status_comcards, _status_pokerhand;
	CString	_status_prwin,    _status_nopp,     _status_nit; 
	CString	_status_handrank;
  CString _last_action;
};

#endif // INC_STATUSBAR_H