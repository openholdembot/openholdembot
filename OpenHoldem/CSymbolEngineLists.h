#ifndef INC_CSYMBOLENGINELISTS_H
#define INC_CSYMBOLENGINELISTS_H

#include "CVirtualSymbolEngine.h"

extern class CSymbolEngineLists: public CVirtualSymbolEngine
{
public:
	CSymbolEngineLists();
	~CSymbolEngineLists();
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
	bool IsList(const int list) 
	{ 
		if ((list >= 0) && (list < MAX_HAND_LISTS))
		{
			return _is_list[list];
		}
		ErrorListNumberOutOfRange(list);
		return false;
	}
private:
	void CalculateLists();
	// list tests
	void SetIsList(const int list, const double is_list) 
	{ 
		if ((list >= 0) && (list < MAX_HAND_LISTS))
		{
			_is_list[list] = is_list;
		}
		ErrorListNumberOutOfRange(list);
	}
	void ErrorListNumberOutOfRange(int list);
private:
	bool _is_list[MAX_HAND_LISTS];
private:
	int userchair; //!!!
} *p_symbol_engine_lists;

#endif INC_CSYMBOLENGINELISTS_H