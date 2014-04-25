//***************************************************************************** 
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//***************************************************************************** 
//
// Purpose:
//
//***************************************************************************** 

#ifndef INC_CSYMBOLENGINERANDOM_H
#define INC_CSYMBOLENGINERANDOM_H

#include "CVirtualSymbolEngine.h"
#include "..\CTransform\CTransform.h"

class CSymbolEngineRandom: public CVirtualSymbolEngine
{
public:
	CSymbolEngineRandom();
	~CSymbolEngineRandom();
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
	bool EvaluateSymbol(const char *name, double *result);
	CString SymbolsProvided();;
public:
	// "random" gets calculated each time new
	double random()				{ return Random_0_1(); }
	double randomhand()			{ return _randomhand; }
	double randomheartbeat()	{ return _randomheartbeat; }
	double randomround()		{ return _randomround; }
private:
	double Random_0_1();
private:
	double _randomhand;
	double _randomheartbeat;
	double _randomround;
};

extern CSymbolEngineRandom *p_symbol_engine_random;

#endif INC_CSYMBOLENGINERANDOM_H