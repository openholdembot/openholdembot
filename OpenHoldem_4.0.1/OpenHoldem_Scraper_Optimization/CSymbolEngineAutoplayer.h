#ifndef INC_CSYMBOLENGINEAUTOPLAYER_H
#define INC_CSYMBOLENGINEAUTOPLAYER_H

#include "CVirtualSymbolEngine.h"
#include "assert.h"
#include "CTableLimits.h"
#include "MagicNumbers.h"

extern class CSymbolEngineAutoplayer: public CVirtualSymbolEngine
{
public:
	CSymbolEngineAutoplayer();
	~CSymbolEngineAutoplayer();
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
	int  myturnbits()		{ return _myturnbits; }
	bool ismyturn()			{ return ((_myturnbits | k_my_turn_bits_fold_call_raise) != 0); }
	bool issittingin()		{ return _issittingin; }
	bool issittingout()		{ return !issittingin(); }
	bool isautopost()		{ return _isautopost; }
	bool isfinalanswer()	{ return _isfinalanswer; }
	bool isfinaltable()		{ return _isfinaltable; }
	bool isbring()			{ return _isbring; }
	bool ismanual()			{ return _ismanual; }
private:
	void CalculateMyTurnBits();
	void CalculateSitInState();
	void CalculateFinalAnswer();
	void DetectSpecialConnectionLikeBringAndManualMode();
private:
	int  _myturnbits;
	bool _issittingin;
	bool _isautopost;
	bool _isfinalanswer; 
	bool _isfinaltable;  
	bool _isbring;
	bool _ismanual;
} *p_symbol_engine_autoplayer;

#endif INC_CSYMBOLENGINEAUTOPLAYER_H