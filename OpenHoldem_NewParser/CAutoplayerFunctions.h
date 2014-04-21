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

#ifndef INC_CAUTOPLAYER_FUNCTIONS
#define INC_CAUTOPLAYER_FUNCTIONS

class CAutoplayerFunctions
{
public:
	CAutoplayerFunctions();
public:
	void Reset();
	void CalcPrimaryFormulas();
	void CalcSecondaryFormulas();
	double GetAutoplayerFunctionValue(const int function_to_bn_set);
#define ENT CSLock lock(m_critsec);
	void SetAutoplayerFunction(const int function_to_bn_set, const double new_value);
#undef ENT
public:
	double autoplayer_function_values(int autoplayer_function_code)	{ return _autoplayer_functionvalues[autoplayer_function_code]; }
public:
	double	f$alli()	{ return _autoplayer_functionvalues[k_autoplayer_function_allin]; }
	double	f$betsize()	{ return _autoplayer_functionvalues[k_autoplayer_function_betsize]; }
	double	f$rais()	{ return _autoplayer_functionvalues[k_autoplayer_function_raise]; }
	double	f$call()	{ return _autoplayer_functionvalues[k_autoplayer_function_call]; }
	double	f$check()	{ return _autoplayer_functionvalues[k_autoplayer_function_check]; }
	double	f$fold()	{ return _autoplayer_functionvalues[k_autoplayer_function_fold]; }
	double  f$beep()	{ return _autoplayer_functionvalues[k_autoplayer_function_beep]; }
	double	f$sitin()	{ return _autoplayer_functionvalues[k_standard_function_sitin]; }
	double	f$sitout()	{ return _autoplayer_functionvalues[k_standard_function_sitout]; }
	double	f$leave()	{ return _autoplayer_functionvalues[k_standard_function_leave]; }
	double	f$close()	{ return _autoplayer_functionvalues[k_standard_function_close]; }
	double	f$prefold()	{ return _autoplayer_functionvalues[k_standard_function_prefold]; }
	double	f$rebuy()	{ return _autoplayer_functionvalues[k_standard_function_rebuy]; }
	double	f$delay()	{ return _autoplayer_functionvalues[k_standard_function_delay]; }
	double	f$chat()	{ return _autoplayer_functionvalues[k_standard_function_chat]; }
private:
	double _autoplayer_functionvalues[k_number_of_standard_functions];
	CCritSec	m_critsec;
};

extern CAutoplayerFunctions *p_autoplayer_functions;

#endif // INC_CAUTOPLAYER_FUNCTIONS