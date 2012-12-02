#ifndef INC_CSYMBOLENGINEPOSITIONS_H
#define INC_CSYMBOLENGINEPOSITIONS_H

#include "CVirtualSymbolEngine.h"
#include "..\CTransform\CTransform.h"

class CSymbolEnginePositions: public CVirtualSymbolEngine
{
public:
	CSymbolEnginePositions();
	~CSymbolEnginePositions();
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
	int nchairsdealtright()		{ return _nchairsdealtright;	}
	int nchairsdealtleft()		{ return _nchairsdealtleft;		}
	int betpositionrais()		{ return _betpositionrais;		}
	int dealpositionrais()		{ return _dealpositionrais;		}
	int betposition()			{ return _betposition;			}
	int dealposition()			{ return _dealposition;			}
private:
	void CalculateNChairsDealtLeftRight();
	void CalculateNOpponentsCheckingBettingFolded();
	void CalculatePositionForTheRaiser();
	void CalculatePositionsForTheUserchair();
private:
	int _nchairsdealtright;
	int _nchairsdealtleft;
	int _betpositionrais;
	int _dealpositionrais;
	int _betposition;
	int _dealposition;
};

#endif INC_CSYMBOLENGINEPOSITIONS_H