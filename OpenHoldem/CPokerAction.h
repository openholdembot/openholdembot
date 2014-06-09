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

#ifndef INC_CPOKERACTION_H
#define INC_CPOKERACTION_H

class CPokerAction 
{
public:
	// public functions
	CPokerAction();
	~CPokerAction();
	const double	ProcessQuery(const char * pquery, int *e);
	const int		DealPosition(const int chairnum);
	
private:
	// private functions and variables - not available via accessors or mutators
	const int		PreflopPos();
	const int		PreflopRaisPos();
	const int		PostflopPos();
	const bool		FirstIntoPot();	
	const int		BetPosition(const int chairnum);
	const bool		AgchairAfter();
};


#endif //INC_CPOKERACTION_H