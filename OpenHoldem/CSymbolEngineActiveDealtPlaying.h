#ifndef INC_CSYMBOLENGINEACTIVEDEALTPLAYING_H
#define INC_CSYMBOLENGINEACTIVEDEALTPLAYING_H

#include "CVirtualSymbolEngine.h"
#include "NumericalFunctions.h"

extern class CSymbolEngineActiveDealtPlaying: public CVirtualSymbolEngine
{
public:
	CSymbolEngineActiveDealtPlaying();
	~CSymbolEngineActiveDealtPlaying();
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
	int playersactivebits()		{ return _playersactivebits; }
	int opponentsactivebits()	{ return (_playersactivebits & ~userchairbit());}
	int nplayersactive()		{ return bitcount(playersactivebits()); }
	int nopponentsactive()		{ return bitcount(opponentsactivebits()); }
public:
	int playersplayingbits()	{ return _playersplayingbits; }
	int opponentsplayingbits()	{ return (_playersplayingbits & ~userchairbit());}
	int nplayersplaying()		{ return bitcount(playersplayingbits()); }
	int nopponentsplaying()		{ return bitcount(opponentsplayingbits()); }
public:
	int playersdealtbits()		{ return _playersdealtbits; }
	int opponentsdealtbits()	{ return (_playersdealtbits & ~userchairbit());}
	int nplayersdealt()			{ return bitcount(playersdealtbits()); }
	int nopponentsdealt()		{ return bitcount(opponentsdealtbits()); }
public:
	int playersseatedbits()		{ return _playersseatedbits; }
	int opponentsseatedbits()	{ return (_playersseatedbits & ~userchairbit());}
	int nplayersseated()		{ return bitcount(playersseatedbits()); }
	int nopponentsseated()		{ return bitcount(opponentsseatedbits()); }
public:
	// Especially useful for the case when we are only interested in opponents
	// and calculate that value from players, subtracting the userchair.
	int userchairbit()				{ return 1 << _userchair; }
private:
	void CalculateActiveBits();
	void CalculatePlayingBits();
	void CalculateDealtBits();
	void CalculateSeatedBits();
private:
	int _playersactivebits;
	int _playersplayingbits;
	int _playersdealtbits;
	int _playersseatedbits;
private:
	int _userchair;
	int _dealerchair;
	int _nchairs;
	double _bblind;
} *p_symbol_engine_active_dealt_playing;

#endif INC_CSYMBOLENGINEACTIVEDEALTPLAYING_H