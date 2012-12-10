#include "stdafx.h"
#include "CSymbolEngineTime.h"

CSymbolEngineTime *p_symbol_engine_time = NULL;

CSymbolEngineTime::CSymbolEngineTime()
{
	// The values of some symbol-engines depend on other engines.
	// As the engines get later called in the order of initialization
	// we assure correct ordering by checking if they are initialized.
	//
	// This engine does not use any other engines.
}

CSymbolEngineTime::~CSymbolEngineTime()
{}

void CSymbolEngineTime::InitOnStartup()
{
	// time
	_elapsed      = 0;
	_elapsedhand  = 0;
	_elapsedauto  = 0;
	_elapsedtoday = 0;
	time(&_elapsedhold);
	time(&_elapsedhandhold);
	time(&_elapsedautohold);
}

void CSymbolEngineTime::ResetOnConnection()
{
	time(&_elapsedhandhold);
	time(&_elapsedautohold);
}

void CSymbolEngineTime::ResetOnHandreset()
{
	time(&_elapsedhandhold);
}

void CSymbolEngineTime::ResetOnNewRound()
{}

void CSymbolEngineTime::ResetOnMyTurn()
{}

void CSymbolEngineTime::ResetOnHeartbeat()
{
	// current time
	time_t t_now_time;
	time(&t_now_time);										
	
	// midnight time
	tm s_midnight_time = {0};
	localtime_s(&s_midnight_time, &t_now_time);
	s_midnight_time.tm_hour = 0;
	s_midnight_time.tm_min = 0;
	s_midnight_time.tm_sec = 0;
	time_t t_midnight_time = mktime(&s_midnight_time);
	_elapsedtoday = t_now_time - t_midnight_time;

	// time symbols
	_elapsed     = t_now_time - _elapsedhold;
	_elapsedhand = t_now_time - _elapsedhandhold;									
	_elapsedauto = t_now_time - _elapsedautohold;	
}

void CSymbolEngineTime::ResetOnAutoPlayerAction()
{
	time(&_elapsedautohold);
}


