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

#ifndef INC_OH_MESSAGEBOX_H
#define INC_OH_MESSAGEBOX_H

#include "stdafx.h"

// Errors and warnings.
// Get displayed (or not) depending on settings
void OH_MessageBox_Error_Warning(CString Message, CString Title = "Error");

// Interactive messages.
// Get always displayed, even if they are not critical.
// They are either a result of the users action 
// or we need request some direct input.
int OH_MessageBox_Interactive(CString Message, CString Title, int Flags);

// MessageBox for the msgbox$MESSAGE-command of OH-script
// Returns 0 as a result
int OH_MessageBox_OH_Script_Messages(CString message);

#ifdef OPENHOLDEM_PROGRAM
// In parsing mode: very detailed error-message
// Otherwise; normal error-message
// It might happen that we detect errors only at run-time,
// e.g. if a DLL requests an invalid symbol.
//
// For OpenHoldem only, not for OpenScrape
void OH_MessageBox_Formula_Error(CString Message, CString Title);
#endif

#endif