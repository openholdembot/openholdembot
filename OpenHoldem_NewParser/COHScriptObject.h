//***************************************************************************** 
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//***************************************************************************** 
//
// Purpose: virtual base-class for CFunction and COHList
//
//***************************************************************************** 

#ifndef INC_COHSCRIPTOBJECT_H
#define INC_COHSCRIPTOBJECT_H

class COHScriptObject
{
public:
	COHScriptObject(); //!! remove
	COHScriptObject(CString *new_name, CString *new_function_text);
	~COHScriptObject();
public:
	CString name()			{ return _name; }
	CString function_text()	{ return _function_text; }
public:
	virtual double Evaluate();
private:
	CString _name;
	CString _function_text;
};

#endif INC_COHSCRIPTOBJECT_H