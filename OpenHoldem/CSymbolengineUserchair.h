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

#ifndef INC_CSYMBOLENGINEUSERCHAIR_H
#define INC_CSYMBOLENGINEUSERCHAIR_H

#include "CVirtualSymbolEngine.h"

#define USER_CHAIR p_symbol_engine_userchair->userchair()

class CSymbolEngineUserchair: public CVirtualSymbolEngine
{
public:
	CSymbolEngineUserchair();
	~CSymbolEngineUserchair();
public:
	// Mandatory reset-functions
	void InitOnStartup();
	void ResetOnConnection();
	void ResetOnHandreset();
	void ResetOnNewRound();
	void ResetOnMyTurn();
	void ResetOnHeartbeat();
public:
	// Public accessors
	bool EvaluateSymbol(const char *name, double *result, bool log = false);
	CString SymbolsProvided();;
public:
	int userchair()				{ return _userchair; }
	int userchairbit()			{ return 1 << (_userchair); }
	bool userchair_confirmed()	{ return (_userchair != k_undefined); }
private:
	void CalculateUserChair();
  bool IsNotShowdown();
private:
	int _userchair;
};

#endif INC_CSYMBOLENGINEUSERCHAIR_H