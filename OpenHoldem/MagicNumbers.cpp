//*******************************************************************************
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//*******************************************************************************
//
// Purpose:
//
//*******************************************************************************

#include "stdafx.h"
#include "MagicNumbers.h"
#include "assert.h"

// Function to access the name of the action constants.
// As fold is negative we can no longer use the constants 
// as indices for an array.
const char* ActionConstantNames(int autoplayer_function_code) {
	// names of action-constants for use in the autoplayer-log.
	// Formerly 4 digits (WinHoldem-style), now more sane.
  // Also considering all secondary formulas.
	if ((autoplayer_function_code >= k_autoplayer_function_beep)
      && (autoplayer_function_code <= k_standard_function_chat)) {
		return k_standard_function_names[autoplayer_function_code];
	}
	return "UNDEFINED";
}

int DefaultButtonNumber(int button_code)
{
	/*
		Returns the default button number by definition
		(ignoring label overrides) 

		0 - fold button
		1 - call button
		2 - raise button 
		3 - allin button.
	*/

	// i3button
	if (button_code == k_button_i3)
		return button_code;

	if (button_code == k_button_i86)
		return button_code;

	if ((button_code >= k_button_i86 * k_max_number_of_i86X_buttons) && (button_code < k_button_i86*k_max_number_of_i86X_buttons + k_max_number_of_i86X_buttons))
		return button_code;

	int button_number = k_button_undefined;

	switch(button_code)
	{
		case k_button_fold:
			button_number = 0;
			break;
		case k_button_call:
			button_number = 1;
			break;
		case k_button_raise:
			button_number = 2;
			break;
		case k_button_allin:
			button_number = 3;
			break;
		default :
			break;
	}

	return button_number;
}