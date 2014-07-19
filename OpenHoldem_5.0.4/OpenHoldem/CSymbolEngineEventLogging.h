//*****************************************************************************
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//*****************************************************************************
//
// Purpose: Not really a symbol-engine, 
//   but a good way to get important events logged.
//
//*****************************************************************************

#ifndef INC_CSYMBOLENGINEEVENTLOGGING_H
#define INC_CSYMBOLENGINEEVENTLOGGING_H

#include "CVirtualSymbolEngine.h"

class CSymbolEngineEventLogging: public CVirtualSymbolEngine
{
public:
	CSymbolEngineEventLogging();
	~CSymbolEngineEventLogging();
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
};

extern CSymbolEngineEventLogging *p_symbol_engine_event_logging;

#endif INC_CSYMBOLENGINEEVENTLOGGING_H