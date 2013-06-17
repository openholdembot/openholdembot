// pokertracker_query_definitions.cpp : Defines the entry point for the DLL application.
//

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

// This definition will trigger the generation of a DLL-export-table 
// in the header-file "pokertracker_query_definitions.h".
// Without this macro POKERTRACKER_DLL_API will cause an import-table
// for the users of this DLL.
#define POKERTRACKER_DLL_EXPORTS

#include <windows.h>
#include "pokertracker_query_definitions.h"

#include <atlstr.h>
#include "..\OpenHoldem\MagicNumbers.h"
#include "..\OpenHoldem\NumericalFunctions.h"
#include "PokerTracker_Queries_Version_3.h"


POKERTRACKER_DLL_API int PT_DLL_GetNumberOfStats()
{
	return k_number_of_pokertracker_stats;
}

// We create queries on the fly, 
// so that they are usable for both ring-games and tournaments 
const CString k_holdem_infix  = " holdem_";
const CString k_omaha_infix   = " omaha_";
const CString k_tournament_infix = " tourney_";

// Values of all stats for all players
double stats[k_number_of_pokertracker_stats][k_max_number_of_players];

POKERTRACKER_DLL_API CString PT_DLL_GetQueryDefinition(
	int stats_index, bool isomaha, bool istournament)
{
	AssertRange(stats_index, 0, (k_number_of_pokertracker_stats - 1));
	CString query = query_definitions[stats_index].first_part_of_query;
	if (query_definitions[stats_index].needs_infix_and_second_part)
	{
		if (istournament)
		{
			query += k_tournament_infix;
		}
		if (isomaha)
		{
			query += k_omaha_infix;
		}
		else
		{
			query += k_holdem_infix;
		}
		query += query_definitions[stats_index].last_part_of_query;
	}
	return query;
}

POKERTRACKER_DLL_API CString PT_DLL_GetDescription(int stats_index)
{ 
	AssertRange(stats_index, 0, (k_number_of_pokertracker_stats - 1));
	return query_definitions[stats_index].description_for_editor; 
}

POKERTRACKER_DLL_API bool PT_DLL_IsBasicStat(int stats_index)
{ 
	AssertRange(stats_index, 0, (k_number_of_pokertracker_stats - 1));
	return query_definitions[stats_index].stat_group == pt_group_basic; 
}

POKERTRACKER_DLL_API bool PT_DLL_IsPositionalPreflopStat(int stats_index)
{ 
	AssertRange(stats_index, 0, (k_number_of_pokertracker_stats - 1));
	return query_definitions[stats_index].stat_group == pt_group_positional;
}

POKERTRACKER_DLL_API bool PT_DLL_IsAdvancedStat(int stats_index)
{ 
	AssertRange(stats_index, 0, (k_number_of_pokertracker_stats - 1));
	return query_definitions[stats_index].stat_group == pt_group_advanced; 
}

// Not exported
int GetIndex(CString symbol_without_prefix)
{
	assert(symbol_without_prefix != "");
	// This function can (and should) probably be optimized
	// by use of CMaps (binary trees).
	for (int i=0; i<k_number_of_pokertracker_stats; ++i)
	{
		if (symbol_without_prefix == stat_str[i])
		{
			return i;
		}
	}
	return k_undefined;
}

POKERTRACKER_DLL_API double	PT_DLL_GetStat(CString symbol_without_prefix, int chair)
{
	assert(symbol_without_prefix != "");
	AssertRange(chair, k_first_chair, k_last_chair);
	int stats_index = GetIndex(symbol_without_prefix);
	if (stats_index == k_undefined)
	{
		return k_undefined;
	}
	return stats[stats_index][chair];
}

POKERTRACKER_DLL_API void PT_DLL_SetStat(int stats_index, int chair, double value)
{
	AssertRange(stats_index, 0, (k_number_of_pokertracker_stats - 1));
	AssertRange(chair, k_first_chair, k_last_chair);
	stats[stats_index][chair] = value;
}

POKERTRACKER_DLL_API void PT_DLL_ClearPlayerStats(int chair)
{
	AssertRange(chair, k_first_chair, k_last_chair);
	for (int i=0; i<k_number_of_pokertracker_stats; ++i)
	{
		PT_DLL_SetStat(i, chair, k_undefined);
	}
}

POKERTRACKER_DLL_API void PT_DLL_ClearAllPlayerStats()
{
	for (int i=0; i<k_max_number_of_players; ++i)
	{
		PT_DLL_ClearPlayerStats(i);
	}
}
