#include "ListOfOpenHoldemSymbolPrefixes.h"

								
ListOfOpenHoldemSymbolPrefixes *p_list_of_openholdem_symbol_prefixes = NULL;

const int k_max_length_of_symbol_prefix = 17;
const int k_number_of_symbol_prefixes = 167;

extern const char openholdem_symbol_prefixes[k_number_of_symbol_prefixes][k_max_length_of_symbol_prefix];



ListOfOpenHoldemSymbolPrefixes::ListOfOpenHoldemSymbolPrefixes()
{
}

ListOfOpenHoldemSymbolPrefixes::~ListOfOpenHoldemSymbolPrefixes()
{
}

bool ListOfOpenHoldemSymbolPrefixes::LooksLikeOpenHoldemSymbol(CString symbol)
{
	for (int i=0; i<k_number_of_symbol_prefixes; i++)
	{
		CString next_prefix = openholdem_symbol_prefixes[i];
		int length_of_prefix = next_prefix.GetLength();
		// OpenHoldem is case sensitive, 
		// so we don't normalize cases for this comparison.
		if (symbol.Left(length_of_prefix) == next_prefix)
		{
			return true;
		}
	}
	return false;
}

// This is a simplified list of prefixes only.
// Building a complete list is impossible anyway,
// as some symbols contain paramaters, like chair$abc.
// So we do just some heuristic comparison.
const char openholdem_symbol_prefixes[k_number_of_symbol_prefixes][k_max_length_of_symbol_prefix] =
{
	// General symbols
	"ismanual",		"isppro",		"site",			"nchairs",
	"isbring",		"session",		"handnumber",	"version",
	//Table Map symbols
	"sitename$",	"network$",
	// Formula file
	"rake",			"nit",			"bankroll",
	// Limits
	"bblind",		"sblind",		"ante",			"lim",
	"isnl",			"ispl",			"isfl",			"srai",
	"istournament",
	// Hand Rank
	"handrank",
	// Chairs
	"chair",		"userchair",	"dealerchair",	"raischair",
	// Rounds / Positions
	"betround",		"betposition",	"dealposition",	"original",
	"callposition",	"seatposition",	
	// Probabilities
	"prwin",		"prlos",		"prtie",		"random",
	// Formulas
	"f$",
	// Chip Amounts
	"balance",		"stack",		"currentbet",	"call",
	"bet",			"pot",			"rais",
	// Number of Bets
	"nbetsto",		"ncurrentbets",	"ncallbets",	"nraisbets",
	// List Tests"
	"islist",
	// Poker Values
	"pokerval",		"pcbits",		"npcbits",
	// Poker Value Constants
	"hicard",		"onepair",		"twopair",		"threeofakind",
	"straight",		"flush",		"fullhouse",	"fourofakind",
	"royalflush",
	// Hand Tests
	"$",			"ishand",		"ishicard",		"isonepair",
	"istwopair",	"isthree",		"isstraight",	"isflush",
	"isfullhouse",	"isfour",		"isroyalflush",
	// Pocket Tests
	"ispair",		"issuited",		"isconnector",
	// Pocket / Common Tests
	"ishipair",		"islopair",		"ismidpair",	"ishistraight",
	"ishiflush",
	// Players, Friends, Opponents
	"nopponents",	"nplayers",		"nfriends",		"opponents",
	"players",		"friends",
	// Flags
	"f0",			"f1",			"f2",			"f3",			
	"f4",			"f5",			"f6",			"f7",
	"f8",			"f9",			"fmax",			"fbits",
	// Common Cards
	"ncommoncards",	"nflopc",
	// (Un)known Cards
	"nouts",		"ncards",
	// nhands
	"nhands"
	// Flushes / Straights / Sets
	"nsuited",		"tsuit",		"nranked",		"trank",
	"nstraight",
	// Rank Bits (aces are hi and lo)
	"rankbits",		"srankbits",
	// Rank Hi (aces are hi)
	"rankhi",		"srankhi",
	// Rank Lo (aces are lo)
	"ranklo",		"sranklo",
	// Time
	"elapsed",		"clocks",		"ncps",			"nclockspersecond",
	// Autoplayer
	"myturnbits",	"ismyturn",		"issitting",	"isauto",
	"isfinal",
	// History
	"prevaction",	"did",			"nbetsround",
	// RON / RUN
	"ron$",			"run$",
	// Versus symbols
	"vs$",
	// History symbols
	"hi_",
	// Action symbols
	"lastraised",	"rais",			"call",			"fold",
	"oppdealt",		"ac",
	// MyHand symbols
	"mh_",
	// Table stats symbols
	"floppct",		"turnpct",		"riverpct",		"avgbetspf",
	"tablepfr",		"maxbalance",	"handsplayed",	"balance_rank",
	// Log symbols
	"log$",
	// Poker Tracker symbols
	"pt_",			"ptt_",
	// ICM calculator symbols
	"icm",
	// Hand multiplexor symbols
	"f$$",
	// Memory symbols
	"me_re_",		"me_st_",
	"$A",			"$K",			"$Q",			"$J",
	"$T",			"$9",			"$8",			"$7",
	"$6",			"$5",			"$4",			"$3",
	"$2",
	// Card Symbols
	"$$p",			"$$c"
};