#ifndef INC_CSYMBOLENGINERAISERSCALLERS_H
#define INC_CSYMBOLENGINERAISERSCALLERS_H

#include "CVirtualSymbolEngine.h"
#include "NumericalFunctions.h"

extern class CSymbolEngineRaisersCallers: public CVirtualSymbolEngine
{
public:
	CSymbolEngineRaisersCallers();
	~CSymbolEngineRaisersCallers();
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
	int raischair()	{ return _raischair; }
	int raisbits(int betround)
	{
		if ((betround >= k_betround_preflop)
			&& (betround <= k_betround_river))
		{
			return _raisbits[betround];
		}
		else
		{
			return k_undefined;
		}
	}
public:
	int foldbits(int betround)
	{
		if ((betround >= k_betround_preflop)
			&& (betround <= k_betround_river))
		{
			return _foldbits[betround];
		}
		else
		{
			return k_undefined;
		}
	}
public:
	int callbits(int betround)
	{
		if ((betround >= k_betround_preflop)
			&& (betround <= k_betround_river))
		{
			return _callbits[betround];
		}
		else
		{
			return k_undefined;
		}
	}
public:
	int nplayerscallshort()		{ return _nplayerscallshort; }
	int nopponentsraising()		{ return bitcount(_raisbits[_betround]); }
	int nopponentsbetting()		{ return _nopponentsbetting; }
	int nopponentsfolded()		{ return _nopponentsfolded; }
	int nopponentscalling()		{ return _nopponentscalling; }
	int nopponentschecking()	{ return _nopponentschecking; }
private:
	void CalculateRaisers();
	void CalculateCallers();
	void CalculateFoldBits();
	void CalculateNOpponentsCheckingBettingFolded();
private:
	int FirstPossibleRaiser();
	int LastPossibleRaiser();
private:
	double RaisersBet();
private:
	int _raischair;
	int _nplayerscallshort;
	int _nopponentsbetting;
	int _nopponentsfolded;
	int _nopponentscalling;
	int _nopponentschecking;
private:
	// Indices 1..4 are for the betrounds preflop..river.
	// Index 0 is unused.
	int _raisbits[k_number_of_betrounds + 1];
	int _foldbits[k_number_of_betrounds + 1];
	int _callbits[k_number_of_betrounds + 1]; 
private:
	int _dealerchair; 
	int _userchair;
	int _nchairs;
	int _raischair_previous_frame;
	int _betround;
	int _playersdealtbits;
} *p_symbol_engine_raisers_callers;

#endif INC_CSYMBOLENGINERAISERSCALLERS_H