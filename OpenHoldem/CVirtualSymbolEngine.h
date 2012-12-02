#ifndef INC_CVIRTUALSYMBOLENGINE_H
#define INC_CVIRTUALSYMBOLENGINE_H

class CVirtualSymbolEngine
{
public:
	virtual void InitOnStartup();
	virtual void ResetOnConnection();
	virtual void ResetOnHandreset();
	virtual void ResetOnNewRound();
	virtual void ResetOnMyTurn();
	virtual void ResetOnHeartbeat();
};

#endif INC_CVIRTUALSYMBOLENGINE_H