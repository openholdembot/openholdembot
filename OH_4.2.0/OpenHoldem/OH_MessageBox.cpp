#include "stdafx.h"
#include "OH_MessageBox.h"

#include <assert.h>
#include "CPreferences.h"


const int k_messagebox_standard_flags = MB_OK | MB_TOPMOST;
const int k_messagebox_error_flags = k_messagebox_standard_flags | MB_ICONWARNING;


void OH_MessageBox_Error_Warning(CString Message, CString Title)
{
#ifdef OPENHOLDEM_PROGRAM
	// Only OpenHoldem supports this setting,
	// but not OpenScrape or other potential applications
	if (prefs.disable_msgbox())
	{
		return;
	}
#endif
	MessageBox(0, Message, Title, k_messagebox_error_flags);
}

int OH_MessageBox_Interactive(CString Message, CString Title, int Flags)
{
	return MessageBox(0, Message, Title, (Flags | k_messagebox_standard_flags));
}

// MessageBox for the msgbox$MESSAGE-command of OH-script
// Returns 0 as a result
int OH_MessageBox_OH_Script_Messages(CString message)
{
	// Preprocess message
	const CString msgbox_prefix = "msgbox$";
	assert(message.Left(msgbox_prefix.GetLength()) == msgbox_prefix);
	int length_of_parameter = message.GetLength() - msgbox_prefix.GetLength();
	message = message.Right(length_of_parameter);
	message.Replace("_B", " ");
	message.Replace("_C", ",");
	message.Replace("_D", ".");
	message.Replace("_N", "\n");
	// At the very last: underscores (to avoid incorrect future replacements)
	message.Replace("_U", "_");
	OH_MessageBox_Error_Warning(message, "OH-Script Message");
	return 0;
}