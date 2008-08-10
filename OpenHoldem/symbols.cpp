#include "stdafx.h"

#include <process.h>
#include <float.h>

#include "debug.h"
#include "global.h"
#include "scraper.h"
#include "grammar.h"
#include "inlines/eval.h"
#include "IteratorThread.h"
#include "versus.h"
#include "RunRon.h"
#include "GameState.h"
#include "PokerTrackerThread.h"

#include "symbols.h"

class CSymbols	symbols;

CRITICAL_SECTION	cs_symbols;

// These can not be function scoped statics because we need to be able to
// reset them when we connect to a table.
double CSymbols::dealerchair_last = -1;
double CSymbols::handnumber_last = -1;
int CSymbols::br_last = -1;
unsigned int CSymbols::player_card_last[2] = {CARD_NOCARD, CARD_NOCARD};


char handrank169[10][169][4] =
{
    {"AAo", "KKo", "QQo", "JJo", "TTo", "99o", "88o", "AKs", "77o", "AQs", "AKo", "AJs", "AQo", "ATs", "66o", "AJo", "KQs", "ATo", "A9s", "KJs", "A8s", "KTs", "KQo", "55o", "A7s", "A9o", "KJo", "QJs", "K9s", "KTo", "A8o", "A6s", "QTs", "A5s", "A4s", "A7o", "QJo", "K8s", "A3s", "K9o", "44o", "Q9s", "JTs", "QTo", "A6o", "K7s", "A5o", "A2s", "K6s", "A4o", "K8o", "Q8s", "J9s", "A3o", "K5s", "Q9o", "JTo", "K7o", "A2o", "K4s", "33o", "Q7s", "K6o", "T9s", "J8s", "K3s", "Q8o", "Q6s", "J9o", "K5o", "K2s", "Q5s", "T8s", "J7s", "K4o", "Q7o", "T9o", "Q4s", "J8o", "K3o", "22o", "Q6o", "Q3s", "98s", "T7s", "J6s", "K2o", "Q2s", "Q5o", "J5s", "T8o", "J7o", "Q4o", "97s", "J4s", "T6s", "J3s", "Q3o", "98o", "T7o", "J6o", "87s", "J2s", "96s", "Q2o", "J5o", "T5s", "T4s", "97o", "J4o", "T6o", "86s", "95s", "T3s", "J3o", "76s", "87o", "T2s", "96o", "J2o", "85s", "T5o", "94s", "T4o", "75s", "93s", "86o", "65s", "95o", "T3o", "84s", "92s", "76o", "T2o", "74s", "85o", "54s", "64s", "83s", "94o", "75o", "82s", "93o", "73s", "65o", "53s", "63s", "84o", "92o", "43s", "74o", "72s", "54o", "64o", "52s", "62s", "83o", "82o", "42s", "73o", "53o", "63o", "32s", "43o", "72o", "52o", "62o", "42o", "32o"},
    {"AAo", "KKo", "QQo", "JJo", "TTo", "99o", "AKs", "88o", "AQs", "AKo", "AJs", "77o", "KQs", "ATs", "AQo", "KJs", "AJo", "KTs", "KQo", "A9s", "QJs", "ATo", "66o", "A8s", "KJo", "QTs", "K9s", "JTs", "A7s", "KTo", "QJo", "A9o", "55o", "A5s", "A6s", "Q9s", "QTo", "A8o", "K8s", "A4s", "J9s", "K9o", "A3s", "JTo", "K7s", "A7o", "T9s", "Q8s", "A2s", "K6s", "44o", "Q9o", "A5o", "A6o", "J8s", "K5s", "K8o", "T8s", "A4o", "J9o", "Q7s", "K4s", "98s", "A3o", "K7o", "T9o", "Q6s", "K3s", "J7s", "Q8o", "A2o", "33o", "K6o", "Q5s", "K2s", "T7s", "J8o", "97s", "87s", "Q4s", "K5o", "T8o", "J6s", "Q3s", "Q7o", "98o", "K4o", "T6s", "J5s", "Q2s", "96s", "Q6o", "76s", "86s", "J7o", "22o", "K3o", "J4s", "T7o", "Q5o", "J3s", "K2o", "T5s", "97o", "87o", "65s", "J2s", "95s", "75s", "Q4o", "T4s", "85s", "J6o", "T3s", "Q3o", "54s", "T6o", "J5o", "T2s", "96o", "64s", "86o", "76o", "Q2o", "94s", "74s", "84s", "J4o", "93s", "53s", "J3o", "92s", "T5o", "63s", "65o", "73s", "43s", "95o", "75o", "83s", "J2o", "85o", "T4o", "82s", "52s", "T3o", "54o", "62s", "42s", "64o", "72s", "T2o", "74o", "94o", "84o", "32s", "93o", "53o", "92o", "63o", "43o", "73o", "83o", "82o", "52o", "62o", "42o", "72o", "32o"},
    {"AAo", "KKo", "QQo", "JJo", "TTo", "99o", "AKs", "AQs", "AKo", "AJs", "KQs", "88o", "ATs", "AQo", "KJs", "QJs", "KTs", "KQo", "AJo", "77o", "QTs", "A9s", "ATo", "JTs", "KJo", "A8s", "K9s", "QJo", "66o", "KTo", "A7s", "Q9s", "QTo", "J9s", "A5s", "T9s", "A9o", "A6s", "JTo", "K8s", "A4s", "55o", "A3s", "K7s", "Q8s", "A8o", "K9o", "J8s", "A2s", "T8s", "K6s", "98s", "Q9o", "A7o", "K5s", "J9o", "T9o", "Q7s", "A5o", "44o", "K4s", "A6o", "K8o", "J7s", "Q6s", "T7s", "97s", "87s", "K3s", "A4o", "Q5s", "K2s", "Q8o", "K7o", "A3o", "J8o", "Q4s", "T8o", "J6s", "76s", "A2o", "98o", "33o", "K6o", "T6s", "86s", "96s", "Q3s", "J5s", "Q2s", "K5o", "J4s", "65s", "Q7o", "75s", "J7o", "J3s", "85s", "K4o", "T7o", "T5s", "95s", "97o", "87o", "22o", "Q6o", "J2s", "54s", "T4s", "K3o", "64s", "Q5o", "T3s", "K2o", "74s", "84s", "T2s", "76o", "Q4o", "94s", "53s", "J6o", "T6o", "86o", "96o", "93s", "Q3o", "63s", "43s", "J5o", "92s", "73s", "Q2o", "65o", "83s", "J4o", "52s", "75o", "82s", "85o", "42s", "J3o", "T5o", "62s", "95o", "54o", "72s", "T4o", "J2o", "32s", "64o", "T3o", "74o", "84o", "T2o", "53o", "94o", "93o", "63o", "43o", "92o", "73o", "83o", "52o", "82o", "42o", "62o", "72o", "32o"},
    {"AAo", "KKo", "QQo", "JJo", "TTo", "AKs", "AQs", "99o", "KQs", "AKo", "AJs", "KJs", "ATs", "AQo", "88o", "QJs", "KTs", "KQo", "QTs", "AJo", "JTs", "A9s", "KJo", "77o", "ATo", "K9s", "A8s", "QJo", "Q9s", "KTo", "A7s", "J9s", "T9s", "QTo", "A5s", "66o", "JTo", "A6s", "K8s", "A4s", "A9o", "A3s", "Q8s", "K7s", "J8s", "T8s", "A2s", "98s", "K9o", "55o", "K6s", "A8o", "Q9o", "K5s", "T9o", "Q7s", "J9o", "K4s", "T7s", "J7s", "87s", "A7o", "97s", "Q6s", "K3s", "44o", "A5o", "K8o", "K2s", "Q5s", "A6o", "76s", "A4o", "86s", "Q4s", "Q8o", "J6s", "T8o", "T6s", "96s", "J8o", "A3o", "K7o", "Q3s", "98o", "33o", "J5s", "65s", "Q2s", "A2o", "75s", "J4s", "K6o", "85s", "54s", "J3s", "22o", "T5s", "95s", "J2s", "64s", "K5o", "Q7o", "87o", "T4s", "T7o", "J7o", "97o", "74s", "K4o", "T3s", "53s", "Q6o", "84s", "T2s", "K3o", "94s", "43s", "63s", "76o", "Q5o", "93s", "K2o", "86o", "92s", "73s", "96o", "52s", "Q4o", "J6o", "T6o", "83s", "42s", "65o", "82s", "Q3o", "J5o", "62s", "75o", "32s", "Q2o", "72s", "J4o", "54o", "85o", "95o", "T5o", "J3o", "64o", "J2o", "T4o", "74o", "53o", "T3o", "84o", "T2o", "94o", "43o", "63o", "93o", "73o", "92o", "52o", "83o", "42o", "82o", "62o", "32o", "72o"},
    {"AAo", "KKo", "QQo", "JJo", "AKs", "TTo", "AQs", "KQs", "AKo", "AJs", "99o", "KJs", "ATs", "QJs", "AQo", "KTs", "KQo", "QTs", "88o", "JTs", "AJo", "A9s", "KJo", "K9s", "A8s", "QJo", "ATo", "77o", "Q9s", "T9s", "J9s", "KTo", "A7s", "A5s", "QTo", "JTo", "K8s", "A4s", "A6s", "66o", "A3s", "Q8s", "T8s", "K7s", "J8s", "98s", "A2s", "A9o", "K6s", "K9o", "55o", "K5s", "87s", "Q7s", "A8o", "Q9o", "T7s", "97s", "K4s", "T9o", "J7s", "J9o", "K3s", "Q6s", "44o", "K2s", "76s", "Q5s", "A7o", "86s", "A5o", "Q4s", "96s", "T6s", "33o", "K8o", "J6s", "65s", "Q3s", "A6o", "A4o", "T8o", "J5s", "Q8o", "Q2s", "75s", "J8o", "98o", "54s", "22o", "A3o", "K7o", "J4s", "85s", "J3s", "64s", "95s", "A2o", "T5s", "J2s", "K6o", "T4s", "53s", "74s", "87o", "T3s", "97o", "43s", "T7o", "T2s", "84s", "K5o", "Q7o", "63s", "J7o", "94s", "K4o", "93s", "52s", "73s", "Q6o", "76o", "92s", "K3o", "42s", "86o", "83s", "K2o", "Q5o", "82s", "62s", "32s", "96o", "65o", "T6o", "Q4o", "J6o", "72s", "75o", "Q3o", "54o", "J5o", "Q2o", "85o", "J4o", "64o", "95o", "J3o", "T5o", "53o", "J2o", "74o", "T4o", "43o", "T3o", "84o", "63o", "T2o", "94o", "52o", "93o", "73o", "42o", "92o", "83o", "62o", "32o", "82o", "72o"},
    {"AAo", "KKo", "QQo", "JJo", "AKs", "AQs", "TTo", "KQs", "AJs", "AKo", "KJs", "ATs", "QJs", "99o", "KTs", "AQo", "QTs", "JTs", "KQo", "88o", "A9s", "AJo", "K9s", "KJo", "A8s", "Q9s", "QJo", "T9s", "J9s", "ATo", "77o", "A7s", "KTo", "A5s", "A4s", "QTo", "K8s", "A6s", "JTo", "A3s", "66o", "T8s", "Q8s", "A2s", "J8s", "K7s", "98s", "K6s", "55o", "A9o", "87s", "K5s", "97s", "Q7s", "K4s", "T7s", "K9o", "J7s", "K3s", "T9o", "44o", "Q6s", "Q9o", "J9o", "K2s", "76s", "A8o", "86s", "Q5s", "33o", "65s", "Q4s", "96s", "22o", "T6s", "Q3s", "A7o", "J6s", "75s", "54s", "Q2s", "A5o", "J5s", "K8o", "85s", "64s", "A4o", "T8o", "J4s", "A6o", "98o", "Q8o", "J8o", "J3s", "53s", "A3o", "95s", "J2s", "T5s", "74s", "K7o", "T4s", "43s", "A2o", "T3s", "84s", "63s", "T2s", "87o", "K6o", "52s", "94s", "97o", "93s", "73s", "42s", "T7o", "K5o", "92s", "Q7o", "J7o", "32s", "76o", "83s", "62s", "K4o", "82s", "86o", "Q6o", "K3o", "72s", "K2o", "65o", "96o", "Q5o", "54o", "T6o", "75o", "Q4o", "J6o", "Q3o", "Q2o", "85o", "64o", "J5o", "53o", "J4o", "95o", "J3o", "74o", "T5o", "43o", "J2o", "T4o", "63o", "T3o", "84o", "52o", "T2o", "94o", "42o", "73o", "93o", "92o", "32o", "83o", "62o", "82o", "72o"},
    {"AAo", "KKo", "QQo", "JJo", "AKs", "AQs", "KQs", "TTo", "AJs", "AKo", "KJs", "ATs", "QJs", "KTs", "99o", "QTs", "AQo", "JTs", "KQo", "A9s", "88o", "AJo", "K9s", "A8s", "T9s", "KJo", "Q9s", "J9s", "QJo", "77o", "A7s", "A5s", "ATo", "A4s", "KTo", "A6s", "K8s", "A3s", "JTo", "QTo", "66o", "T8s", "A2s", "Q8s", "98s", "J8s", "K7s", "K6s", "55o", "87s", "K5s", "97s", "44o", "K4s", "T7s", "Q7s", "J7s", "K3s", "A9o", "76s", "33o", "K2s", "Q6s", "22o", "K9o", "86s", "T9o", "65s", "Q5s", "J9o", "Q9o", "96s", "54s", "Q4s", "A8o", "75s", "T6s", "Q3s", "J6s", "Q2s", "64s", "85s", "J5s", "53s", "A7o", "J4s", "A5o", "J3s", "95s", "74s", "T8o", "J2s", "43s", "A4o", "K8o", "T5s", "98o", "J8o", "T4s", "63s", "A6o", "A3o", "Q8o", "T3s", "84s", "T2s", "52s", "A2o", "K7o", "42s", "87o", "94s", "73s", "93s", "92s", "32s", "97o", "K6o", "62s", "83s", "T7o", "82s", "76o", "K5o", "J7o", "72s", "Q7o", "K4o", "86o", "65o", "K3o", "Q6o", "K2o", "54o", "96o", "75o", "Q5o", "T6o", "Q4o", "64o", "J6o", "Q3o", "85o", "53o", "Q2o", "J5o", "74o", "43o", "J4o", "95o", "J3o", "J2o", "63o", "T5o", "52o", "T4o", "84o", "T3o", "42o", "T2o", "73o", "94o", "32o", "93o", "62o", "92o", "83o", "82o", "72o"},
    {"AAo", "KKo", "QQo", "AKs", "JJo", "AQs", "KQs", "AJs", "TTo", "AKo", "KJs", "ATs", "QJs", "KTs", "QTs", "99o", "JTs", "AQo", "KQo", "A9s", "88o", "K9s", "T9s", "AJo", "A8s", "J9s", "Q9s", "KJo", "77o", "A5s", "A7s", "QJo", "A4s", "A3s", "A6s", "ATo", "K8s", "66o", "A2s", "T8s", "KTo", "98s", "Q8s", "J8s", "JTo", "QTo", "K7s", "55o", "87s", "K6s", "44o", "97s", "33o", "22o", "K5s", "T7s", "K4s", "76s", "Q7s", "K3s", "J7s", "K2s", "86s", "65s", "Q6s", "54s", "A9o", "Q5s", "T9o", "96s", "75s", "Q4s", "K9o", "J9o", "Q3s", "64s", "T6s", "Q9o", "Q2s", "J6s", "85s", "53s", "A8o", "J5s", "J4s", "74s", "43s", "J3s", "95s", "J2s", "63s", "T5s", "A5o", "A7o", "T8o", "T4s", "98o", "T3s", "84s", "52s", "A4o", "T2s", "42s", "K8o", "A3o", "J8o", "A6o", "73s", "Q8o", "94s", "32s", "93s", "A2o", "87o", "92s", "62s", "K7o", "83s", "97o", "82s", "76o", "72s", "K6o", "T7o", "65o", "K5o", "86o", "J7o", "54o", "Q7o", "K4o", "K3o", "75o", "K2o", "Q6o", "96o", "64o", "Q5o", "T6o", "53o", "Q4o", "85o", "J6o", "Q3o", "Q2o", "43o", "74o", "J5o", "J4o", "95o", "63o", "J3o", "J2o", "52o", "T5o", "84o", "42o", "T4o", "T3o", "32o", "T2o", "73o", "94o", "62o", "93o", "92o", "83o", "82o", "72o"},
    {"AAo", "KKo", "QQo", "AKs", "JJo", "AQs", "KQs", "AJs", "TTo", "KJs", "AKo", "ATs", "QJs", "KTs", "QTs", "JTs", "99o", "AQo", "A9s", "KQo", "88o", "K9s", "T9s", "A8s", "J9s", "Q9s", "77o", "AJo", "A5s", "A7s", "A4s", "KJo", "A3s", "66o", "A6s", "QJo", "K8s", "A2s", "T8s", "98s", "J8s", "ATo", "Q8s", "55o", "K7s", "JTo", "KTo", "44o", "33o", "22o", "QTo", "87s", "K6s", "97s", "K5s", "76s", "T7s", "K4s", "K3s", "Q7s", "K2s", "J7s", "86s", "65s", "54s", "Q6s", "75s", "Q5s", "96s", "Q4s", "Q3s", "64s", "T9o", "Q2s", "A9o", "T6s", "53s", "J6s", "85s", "J9o", "K9o", "43s", "J5s", "Q9o", "74s", "J4s", "J3s", "J2s", "95s", "63s", "A8o", "T5s", "52s", "42s", "T4s", "T3s", "84s", "98o", "T2s", "A5o", "T8o", "A7o", "73s", "32s", "A4o", "94s", "93s", "62s", "A3o", "K8o", "J8o", "92s", "A6o", "87o", "Q8o", "83s", "A2o", "82s", "97o", "72s", "K7o", "76o", "T7o", "65o", "K6o", "86o", "54o", "K5o", "J7o", "Q7o", "75o", "K4o", "K3o", "96o", "64o", "K2o", "53o", "Q6o", "85o", "T6o", "Q5o", "43o", "Q4o", "Q3o", "74o", "Q2o", "J6o", "63o", "J5o", "95o", "52o", "J4o", "42o", "J3o", "J2o", "84o", "T5o", "32o", "T4o", "T3o", "73o", "T2o", "62o", "94o", "93o", "92o", "83o", "82o", "72o"}
};

//handrank table used to prime weighted prwin lookup table.
//reflects incidence of cards people actually play to flop.
//left in this form for ease of developer modification.
//converted at startup to the tables actually used by prwin calculation
char *prwhandrank169[169] =
{
    "AA ","KK ","QQ ","AKs","JJ ","AQs","KQs","TT ","AJs","KJs",
    "JTs","QJs","QTs","99 ","ATs","KTs","88 ","T9s","AK ","J9s",
    "77 ","98s","Q9s","66 ","A9s","K9s","T8s","55 ","A5s","54s",
    "44 ","A8s","87s","33 ","65s","22 ","AQ ","A4s","J8s","A3s",
    "76s","97s","A7s","KQ ","A2s","Q8s","86s","K8s","A6s","75s",
    "T7s","53s","64s","K7s","AJ ","KJ ","43s","QJ ","96s","JT ",
    "J7s","K6s","K5s","QT ","85s","Q7s","KT ","AT ","K4s","K3s",
    "K2s","74s","T6s","52s","Q6s","63s","Q5s","Q4s","42s","Q3s",
    "95s","J6s","J5s","32s","Q2s","T9 ","J4s","84s","T5s","J3s",
    "T4s","73s","J2s","J9 ","T3s","T2s","62s","94s","93s","92s",
    "83s","Q9 ","A9 ","98 ","82s","T8 ","K9 ","72s","54 ","87 ",
    "A8 ","A5 ","65 ","A4 ","76 ","J8 ","97 ","A3 ","A7 ","A2 ",
    "86 ","A6 ","Q8 ","75 ","53 ","K8 ","T7 ","64 ","K7 ","43 ",
    "96 ","K6 ","J7 ","85 ","K5 ","Q7 ","K4 ","K3 ","74 ","52 ",
    "T6 ","Q6 ","K2 ","Q5 ","42 ","63 ","Q4 ","95 ","Q3 ","J6 ",
    "J5 ","32 ","Q2 ","J4 ","T5 ","84 ","J3 ","J2 ","T4 ","73 ",
    "T3 ","T2 ","94 ","62 ","93 ","92 ","83 ","82 ","72 "
};
//weighted prwin lookup tables for non-suited and suited cards
int pair2ranko[170], pair2ranks[170];
//used to resolve ascii cards to numbers for init of above
char ctonum[14]="23456789TJQKA";
int willplay,wontplay,topclip,mustplay;

int handrank2652[10][169] =
{
    {12, 24, 36, 48, 60, 72, 84, 92, 104, 112, 136, 144, 168, 176, 188, 212, 220, 244, 252, 260, 268, 276, 300, 312, 320, 344, 368, 376, 384, 408, 432, 440, 448, 456, 464, 488, 512, 520, 528, 552, 564, 572, 580, 604, 628, 636, 660, 668, 676, 700, 724, 732, 740, 764, 772, 796, 820, 844, 868, 876, 888, 896, 920, 928, 936, 944, 968, 976, 1000, 1024, 1032, 1040, 1048, 1056, 1080, 1104, 1128, 1136, 1160, 1184, 1196, 1220, 1228, 1236, 1244, 1252, 1276, 1284, 1308, 1316, 1340, 1364, 1388, 1396, 1404, 1412, 1420, 1444, 1468, 1492, 1516, 1524, 1532, 1540, 1564, 1588, 1596, 1604, 1628, 1652, 1676, 1684, 1692, 1700, 1724, 1732, 1756, 1764, 1788, 1812, 1820, 1844, 1852, 1876, 1884, 1892, 1916, 1924, 1948, 1972, 1980, 1988, 2012, 2036, 2044, 2068, 2076, 2084, 2092, 2116, 2140, 2148, 2172, 2180, 2204, 2212, 2220, 2244, 2268, 2276, 2300, 2308, 2332, 2356, 2364, 2372, 2396, 2420, 2428, 2452, 2476, 2500, 2508, 2532, 2556, 2580, 2604, 2628, 2652},
    {12, 24, 36, 48, 60, 72, 80, 92, 100, 124, 132, 144, 152, 160, 184, 192, 216, 224, 248, 256, 264, 288, 300, 308, 332, 340, 348, 356, 364, 388, 412, 436, 448, 456, 464, 472, 496, 520, 528, 536, 544, 568, 576, 600, 608, 632, 640, 648, 656, 664, 676, 700, 724, 748, 756, 764, 788, 796, 820, 844, 852, 860, 868, 892, 916, 940, 948, 956, 964, 988, 1012, 1024, 1048, 1056, 1064, 1072, 1096, 1104, 1112, 1120, 1144, 1168, 1176, 1184, 1208, 1232, 1256, 1264, 1272, 1280, 1288, 1312, 1320, 1328, 1352, 1364, 1388, 1396, 1420, 1444, 1452, 1476, 1484, 1508, 1532, 1540, 1548, 1556, 1564, 1588, 1596, 1604, 1628, 1636, 1660, 1668, 1692, 1716, 1724, 1748, 1756, 1780, 1804, 1828, 1836, 1844, 1852, 1876, 1884, 1892, 1916, 1924, 1948, 1956, 1980, 1988, 1996, 2020, 2044, 2052, 2076, 2100, 2124, 2132, 2140, 2164, 2188, 2196, 2204, 2228, 2236, 2260, 2284, 2308, 2332, 2340, 2364, 2388, 2412, 2436, 2460, 2484, 2508, 2532, 2556, 2580, 2604, 2628, 2652},
    {12, 24, 36, 48, 60, 72, 80, 88, 112, 120, 128, 140, 148, 172, 180, 188, 196, 220, 244, 256, 264, 272, 296, 304, 328, 336, 344, 368, 380, 404, 412, 420, 444, 452, 460, 468, 492, 500, 524, 532, 540, 552, 560, 568, 576, 600, 624, 632, 640, 648, 656, 664, 688, 712, 720, 744, 768, 776, 800, 812, 820, 844, 868, 876, 884, 892, 900, 908, 916, 940, 948, 956, 980, 1004, 1028, 1052, 1060, 1084, 1092, 1100, 1124, 1148, 1160, 1184, 1192, 1200, 1208, 1216, 1224, 1232, 1256, 1264, 1272, 1296, 1304, 1328, 1336, 1344, 1368, 1392, 1400, 1408, 1432, 1456, 1468, 1492, 1500, 1508, 1516, 1540, 1548, 1572, 1580, 1604, 1612, 1620, 1628, 1652, 1676, 1684, 1692, 1716, 1740, 1764, 1788, 1796, 1820, 1828, 1836, 1860, 1868, 1876, 1900, 1924, 1932, 1956, 1964, 1988, 1996, 2020, 2028, 2052, 2076, 2084, 2108, 2132, 2140, 2164, 2188, 2196, 2220, 2244, 2268, 2292, 2316, 2340, 2364, 2388, 2412, 2436, 2460, 2484, 2508, 2532, 2556, 2580, 2604, 2628, 2652},
    {12, 24, 36, 48, 60, 68, 76, 88, 96, 120, 128, 136, 144, 168, 180, 188, 196, 220, 228, 252, 260, 268, 292, 304, 328, 336, 344, 368, 376, 400, 408, 416, 424, 448, 456, 468, 492, 500, 508, 516, 540, 548, 556, 564, 572, 580, 588, 596, 620, 632, 640, 664, 688, 696, 720, 728, 752, 760, 768, 776, 784, 808, 816, 824, 832, 844, 868, 892, 900, 908, 932, 940, 964, 972, 980, 1004, 1012, 1036, 1044, 1052, 1076, 1100, 1124, 1132, 1156, 1168, 1176, 1184, 1192, 1216, 1224, 1232, 1256, 1264, 1272, 1280, 1292, 1300, 1308, 1316, 1324, 1348, 1372, 1396, 1404, 1428, 1452, 1476, 1484, 1508, 1516, 1524, 1548, 1556, 1564, 1588, 1596, 1604, 1612, 1636, 1660, 1668, 1692, 1716, 1724, 1732, 1756, 1764, 1788, 1812, 1836, 1844, 1852, 1876, 1884, 1908, 1932, 1940, 1964, 1972, 1996, 2004, 2028, 2052, 2076, 2100, 2124, 2148, 2172, 2196, 2220, 2244, 2268, 2292, 2316, 2340, 2364, 2388, 2412, 2436, 2460, 2484, 2508, 2532, 2556, 2580, 2604, 2628, 2652},
    {12, 24, 36, 48, 56, 68, 76, 84, 108, 116, 128, 136, 144, 152, 176, 184, 208, 216, 228, 236, 260, 268, 292, 300, 308, 332, 356, 368, 376, 384, 392, 416, 424, 432, 456, 480, 488, 496, 504, 516, 524, 532, 540, 548, 556, 564, 572, 596, 604, 628, 640, 648, 656, 664, 688, 712, 720, 728, 736, 760, 768, 792, 800, 808, 820, 828, 836, 844, 868, 876, 900, 908, 916, 924, 936, 960, 968, 976, 984, 1008, 1032, 1056, 1064, 1088, 1096, 1104, 1128, 1152, 1160, 1172, 1196, 1220, 1228, 1236, 1244, 1252, 1260, 1284, 1292, 1300, 1324, 1332, 1340, 1348, 1372, 1380, 1404, 1412, 1436, 1444, 1452, 1476, 1500, 1508, 1532, 1540, 1564, 1572, 1580, 1588, 1612, 1636, 1644, 1668, 1676, 1700, 1708, 1732, 1756, 1764, 1772, 1780, 1804, 1828, 1852, 1876, 1900, 1908, 1932, 1956, 1980, 2004, 2028, 2052, 2076, 2100, 2124, 2148, 2172, 2196, 2220, 2244, 2268, 2292, 2316, 2340, 2364, 2388, 2412, 2436, 2460, 2484, 2508, 2532, 2556, 2580, 2604, 2628, 2652},
    {12, 24, 36, 48, 56, 64, 76, 84, 92, 116, 124, 132, 140, 152, 160, 184, 192, 200, 224, 236, 244, 268, 276, 300, 308, 316, 340, 348, 356, 380, 392, 400, 424, 432, 440, 464, 472, 480, 504, 512, 524, 532, 540, 548, 556, 564, 572, 580, 592, 616, 624, 632, 640, 648, 656, 664, 688, 696, 704, 728, 740, 748, 772, 796, 804, 812, 836, 844, 852, 864, 872, 880, 888, 900, 908, 916, 940, 948, 956, 964, 972, 996, 1004, 1028, 1036, 1044, 1068, 1092, 1100, 1124, 1148, 1172, 1196, 1204, 1212, 1236, 1244, 1252, 1260, 1268, 1292, 1300, 1308, 1332, 1340, 1348, 1356, 1364, 1388, 1412, 1420, 1428, 1452, 1460, 1468, 1476, 1500, 1524, 1532, 1556, 1580, 1588, 1612, 1620, 1628, 1652, 1660, 1684, 1708, 1732, 1740, 1764, 1788, 1812, 1836, 1860, 1884, 1908, 1932, 1956, 1980, 2004, 2028, 2052, 2076, 2100, 2124, 2148, 2172, 2196, 2220, 2244, 2268, 2292, 2316, 2340, 2364, 2388, 2412, 2436, 2460, 2484, 2508, 2532, 2556, 2580, 2604, 2628, 2652},
    {12, 24, 36, 48, 56, 64, 72, 84, 92, 116, 124, 132, 140, 148, 160, 168, 192, 200, 224, 232, 244, 268, 276, 284, 292, 316, 324, 332, 356, 368, 376, 384, 408, 416, 440, 448, 456, 464, 488, 512, 524, 532, 540, 548, 556, 564, 572, 580, 592, 600, 608, 616, 628, 636, 644, 652, 660, 668, 692, 700, 712, 720, 728, 740, 764, 772, 796, 804, 812, 836, 860, 868, 876, 884, 908, 916, 924, 932, 940, 948, 956, 964, 972, 980, 1004, 1012, 1036, 1044, 1052, 1060, 1084, 1092, 1100, 1124, 1148, 1156, 1180, 1204, 1212, 1220, 1244, 1268, 1292, 1300, 1308, 1316, 1324, 1348, 1372, 1380, 1404, 1412, 1420, 1428, 1436, 1444, 1468, 1492, 1500, 1508, 1532, 1540, 1564, 1588, 1612, 1620, 1644, 1668, 1692, 1716, 1740, 1764, 1788, 1812, 1836, 1860, 1884, 1908, 1932, 1956, 1980, 2004, 2028, 2052, 2076, 2100, 2124, 2148, 2172, 2196, 2220, 2244, 2268, 2292, 2316, 2340, 2364, 2388, 2412, 2436, 2460, 2484, 2508, 2532, 2556, 2580, 2604, 2628, 2652},
    {12, 24, 36, 44, 56, 64, 72, 80, 92, 116, 124, 132, 140, 148, 156, 168, 176, 200, 224, 232, 244, 252, 260, 284, 292, 300, 308, 332, 344, 352, 360, 384, 392, 400, 408, 432, 440, 452, 460, 468, 492, 500, 508, 516, 540, 564, 572, 584, 592, 600, 612, 620, 632, 644, 652, 660, 668, 676, 684, 692, 700, 708, 716, 724, 732, 740, 764, 772, 796, 804, 812, 820, 844, 868, 876, 884, 892, 916, 924, 932, 940, 948, 972, 980, 988, 996, 1004, 1012, 1020, 1028, 1036, 1044, 1068, 1092, 1116, 1124, 1148, 1156, 1164, 1172, 1196, 1204, 1212, 1236, 1260, 1284, 1308, 1316, 1340, 1348, 1356, 1364, 1388, 1412, 1420, 1428, 1452, 1460, 1484, 1492, 1516, 1524, 1548, 1572, 1596, 1620, 1644, 1668, 1692, 1716, 1740, 1764, 1788, 1812, 1836, 1860, 1884, 1908, 1932, 1956, 1980, 2004, 2028, 2052, 2076, 2100, 2124, 2148, 2172, 2196, 2220, 2244, 2268, 2292, 2316, 2340, 2364, 2388, 2412, 2436, 2460, 2484, 2508, 2532, 2556, 2580, 2604, 2628, 2652},
    {12, 24, 36, 44, 56, 64, 72, 80, 92, 100, 124, 132, 140, 148, 156, 164, 176, 200, 208, 232, 244, 252, 260, 268, 276, 284, 296, 320, 328, 336, 344, 368, 376, 388, 396, 420, 428, 436, 444, 452, 460, 484, 492, 504, 512, 536, 560, 572, 584, 596, 620, 628, 636, 644, 652, 660, 668, 676, 684, 692, 700, 708, 716, 724, 732, 740, 748, 756, 764, 772, 780, 788, 812, 820, 844, 852, 860, 868, 876, 900, 924, 932, 940, 964, 972, 980, 988, 996, 1004, 1012, 1036, 1044, 1052, 1060, 1068, 1076, 1084, 1108, 1116, 1140, 1164, 1188, 1196, 1204, 1228, 1236, 1244, 1252, 1276, 1300, 1324, 1332, 1356, 1380, 1404, 1412, 1436, 1444, 1468, 1476, 1500, 1524, 1548, 1572, 1596, 1620, 1644, 1668, 1692, 1716, 1740, 1764, 1788, 1812, 1836, 1860, 1884, 1908, 1932, 1956, 1980, 2004, 2028, 2052, 2076, 2100, 2124, 2148, 2172, 2196, 2220, 2244, 2268, 2292, 2316, 2340, 2364, 2388, 2412, 2436, 2460, 2484, 2508, 2532, 2556, 2580, 2604, 2628, 2652}
};

CSymbols::CSymbols()
{
    __SEH_SET_EXCEPTION_HANDLER(MyUnHandledExceptionFilter);

    __SEH_HEADER

    int			i, j, k, vndx;
    char *ptr;

	ResetSymbolsFirstTime();

    // Seed RNG
    srand((unsigned)time( NULL ));

	EnterCriticalSection(&cs_symbols);	

		//Initialise the handrank tables used by prwin
		vndx=0; //used to provide an offset into the vanilla table
		for (i=0;i<169;i++)
		{
			//normal weighted prwin table
			ptr=prwhandrank169[i];
			j=(strchr(ctonum,*ptr)-ctonum)*13 + (strchr(ctonum,*(ptr+1))-ctonum);
			if (*(ptr+2)=='s')pair2ranks[j]=i+1;
			else pair2ranko[j]=i+1;
			//prw1326 vanilla table
			j=strchr(ctonum,*ptr)-ctonum;
			k=strchr(ctonum,*(ptr+1))-ctonum;
			for(;;)
			{
				//I originally had an algorithm to do this, but it was obscure and impenetrable
				//so now I have switched to the clumsy but simple approach.
				if(j==k)//pair
				{
					symbols.prw1326.vanilla_chair.rankhi[vndx]=j;	//h
					symbols.prw1326.vanilla_chair.rankhi[vndx+1]=j;	//h
					symbols.prw1326.vanilla_chair.rankhi[vndx+2]=j; //h
					symbols.prw1326.vanilla_chair.rankhi[vndx+3]=j+13; //d
					symbols.prw1326.vanilla_chair.rankhi[vndx+4]=j+13; //d
					symbols.prw1326.vanilla_chair.rankhi[vndx+5]=j+26; //c
					symbols.prw1326.vanilla_chair.ranklo[vndx]=k+13;	//d
					symbols.prw1326.vanilla_chair.ranklo[vndx+1]=k+26;	//c
					symbols.prw1326.vanilla_chair.ranklo[vndx+2]=k+39;	//s
					symbols.prw1326.vanilla_chair.ranklo[vndx+3]=k+26;	//c	
					symbols.prw1326.vanilla_chair.ranklo[vndx+4]=k+39;	//s
					symbols.prw1326.vanilla_chair.ranklo[vndx+5]=k+39;	//s
					vndx+=6;
					break;
				}
			
				if (*(ptr+2)=='s') //suited
				{
					symbols.prw1326.vanilla_chair.rankhi[vndx]=j;		//h
					symbols.prw1326.vanilla_chair.rankhi[vndx+1]=j+13;	//d
					symbols.prw1326.vanilla_chair.rankhi[vndx+2]=j+26;	//c
					symbols.prw1326.vanilla_chair.rankhi[vndx+3]=j+39;	//s
					symbols.prw1326.vanilla_chair.ranklo[vndx]=k;		//h
					symbols.prw1326.vanilla_chair.ranklo[vndx+1]=k+13;	//d
					symbols.prw1326.vanilla_chair.ranklo[vndx+2]=k+26;	//c
					symbols.prw1326.vanilla_chair.ranklo[vndx+3]=k+39;	//s
					vndx+=4;
					break;
				}
			
				//only unsuited non-pairs left
				symbols.prw1326.vanilla_chair.rankhi[vndx]=j;		//h
				symbols.prw1326.vanilla_chair.rankhi[vndx+1]=j;		//h
				symbols.prw1326.vanilla_chair.rankhi[vndx+2]=j;		//h
				symbols.prw1326.vanilla_chair.rankhi[vndx+3]=j+13;	//d
				symbols.prw1326.vanilla_chair.rankhi[vndx+4]=j+13;	//d
				symbols.prw1326.vanilla_chair.rankhi[vndx+5]=j+13;	//d
				symbols.prw1326.vanilla_chair.rankhi[vndx+6]=j+26;	//c
				symbols.prw1326.vanilla_chair.rankhi[vndx+7]=j+26;	//c
				symbols.prw1326.vanilla_chair.rankhi[vndx+8]=j+26;	//c
				symbols.prw1326.vanilla_chair.rankhi[vndx+9]=j+39;	//s
				symbols.prw1326.vanilla_chair.rankhi[vndx+10]=j+39;	//s
				symbols.prw1326.vanilla_chair.rankhi[vndx+11]=j+39; //s Matrix corrected typo
				symbols.prw1326.vanilla_chair.ranklo[vndx]=k+13;	//d
				symbols.prw1326.vanilla_chair.ranklo[vndx+1]=k+26;	//c
				symbols.prw1326.vanilla_chair.ranklo[vndx+2]=k+39;	//s
				symbols.prw1326.vanilla_chair.ranklo[vndx+3]=k;		//h
				symbols.prw1326.vanilla_chair.ranklo[vndx+4]=k+26;	//c
				symbols.prw1326.vanilla_chair.ranklo[vndx+5]=k+39;	//s
				symbols.prw1326.vanilla_chair.ranklo[vndx+6]=k;		//h
				symbols.prw1326.vanilla_chair.ranklo[vndx+7]=k+13;	//d
				symbols.prw1326.vanilla_chair.ranklo[vndx+8]=k+39;	//s
				symbols.prw1326.vanilla_chair.ranklo[vndx+9]=k;		//h
				symbols.prw1326.vanilla_chair.ranklo[vndx+10]=k+13;	//d
				symbols.prw1326.vanilla_chair.ranklo[vndx+11]=k+26;	//c
				vndx+=12;
				break;
			}
		}

		symbols.prw1326.vanilla_chair.level=1024;
		symbols.prw1326.vanilla_chair.limit=820; //cut off a little early, since 820-884 very improbable

		// now assign a weight table. Assume upper third fully probable, next third reducing
		// probability, lowest third not played.
		for(i=0;i<442;i++)
			symbols.prw1326.vanilla_chair.weight[i]=symbols.prw1326.vanilla_chair.level;
		for(i=442;i<884;i++)
			symbols.prw1326.vanilla_chair.weight[i]=symbols.prw1326.vanilla_chair.level*(884-i)/442;
		for(i=884;i<1326;i++)
			symbols.prw1326.vanilla_chair.weight[i]=0;

		//finally copy the vanilla to all user chairs so that someone who just turns on prw1326
		//experimentally does not cause a crash
		for(i=0;i<10;i++)
			symbols.prw1326.chair[i]=symbols.prw1326.vanilla_chair ;

		//end of handrank initialisation

		symbols_update_in_progress = false;

	LeaveCriticalSection(&cs_symbols);
	
	__SEH_LOGFATAL("CSymbols::Constructor : \n");
}

void CSymbols::ResetSymbolsFirstTime(void)
{
    __SEH_HEADER

	EnterCriticalSection(&cs_symbols);

		int		i;

		// general
		sym.ismanual = 0;
		sym.isppro = 0;
		sym.site = 1;
		sym.nchairs = 0;
		sym.isbring = 0;
		sym.session = 1;
		sym.handnumber = 0;
		sym.version = VERSION_NUMBER;

		// profile
		sym.swagdelay = 0;
		sym.allidelay = 0;
		sym.swagtextmethod = 3;
		sym.potmethod = 1;
		sym.activemethod = 1;

		// formula
		sym.rake = sym.bankroll = 0;
		sym.nit = 0;

		// limits
		sym.bblind = sym.sblind = sym.ante = 0;
		sym.lim = -1;
		sym.isnl = sym.ispl = sym.isfl = 0;
		sym.istournament = 0;
		sym.sraiprev = sym.sraimin = sym.sraimax = 0;

		// handrank
		sym.handrank169 = sym.handrank2652 = sym.handrank1326 = sym.handrank1000 = 0;
		sym.handrankp = sym.handrank = 0;

		// chairs
		sym.chair = sym.userchair = sym.dealerchair = sym.raischair = 0;
		user_chair_confirmed = false;

		// rounds positions
		sym.betround = sym.br = 0;
		sym.betposition = sym.dealposition = sym.callposition = sym.seatposition = 1;
		sym.dealpositionrais = sym.betpositionrais = 1;

		// probabilities
		sym.random = sym.randomhand = 0;

		for (i=0; i<5; i++)
			sym.randomround[i] = 0;

		sym.prwin = sym.prlos = sym.prtie = 0;

		// statistics
		sym.callror = sym.raisror = sym.srairor = sym.alliror = 0;
		sym.callmean = sym.raismean = sym.sraimean = sym.allimean = 0;
		sym.callvariance = sym.raisvariance = sym.sraivariance = sym.allivariance = 0;

		// p formula
		sym.defcon = sym.isdefmode = sym.isaggmode = 0;

		// chip amounts
		for (i=0; i<11; i++)
		{
			sym.balance[i] = 0;
			sym.currentbet[i] = 0;
		}

		for (i=0; i<10; i++)
			sym.stack[i] = 0;

		for (i=0; i<5; i++)
			sym.bet[i] = 0;

		sym.pot = sym.potcommon = sym.potplayer = 0;
		sym.call = sym.callshort = sym.raisshort = 0;

		// number of bets
		sym.nbetstocall = sym.nbetstorais = sym.ncurrentbets = sym.ncallbets = sym.nraisbets = 0;

		// list tests
		sym.islistcall = sym.islistrais = sym.islistalli = 0;
		for (i=0; i<MAX_HAND_LISTS; i++) {
			sym.islist[i] = 0;
		}
		sym.isemptylistcall = sym.isemptylistrais = sym.isemptylistalli = 0;

		// list numbers
		sym.nlistmax = -1;
		sym.nlistmin = -1;

		// poker values
		sym.pokerval = sym.pokervalplayer = sym.pokervalcommon = sym.pcbits = sym.npcbits = 0;

		// hand tests
		for (i=0; i<2; i++)
		{
			sym.$$pc[i] = WH_NOCARD;
			sym.$$pr[i] = 0;
			sym.$$ps[i] = 0;
		}

		for (i=0; i<5; i++)
		{
			sym.$$cc[i] = WH_NOCARD;
			sym.$$cs[i] = 0;
			sym.$$cr[i] = 0;
		}

		for (i=0; i<4; i++)
		{
			phandval[i] = 0;
			chandval[i] = 0;
		}
		sym.ishandup = sym.ishandupcommon = 0;
		sym.ishicard = sym.isonepair = sym.istwopair = sym.isthreeofakind = sym.isstraight = 0;
		sym.isflush = sym.isfullhouse = sym.isfourofakind = sym.isstraightflush = sym.isroyalflush = 0;
		sym.isfiveofakind = 0;

		// pocket tests
		sym.ispair = sym.issuited = sym.isconnector = 0;

		// pocket/common tests
		sym.ishipair = sym.islopair = sym.ismidpair = sym.ishistraight = sym.ishiflush = 0;

		// players friends opponents
		sym.nopponents = 0;
		sym.nopponentsmax = 0;
		sym.nplayersseated = sym.nplayersactive = sym.nplayersdealt = 0;
		sym.nplayersplaying = sym.nplayersblind = 0;
		sym.nopponentsseated = sym.nopponentsactive = sym.nopponentsdealt = 0;
		sym.nopponentsplaying = sym.nopponentsblind = 0;
		sym.nopponentschecking = sym.nopponentscalling = sym.nopponentsraising = 0;
		sym.nopponentsbetting = sym.nopponentsfolded = 0;
		sym.nplayerscallshort = sym.nchairsdealtright = sym.nchairsdealtleft = 0;
		sym.playersseatedbits = sym.playersactivebits = sym.playersdealtbits = 0;
		sym.playersplayingbits = sym.playersblindbits = 0;
		sym.opponentsseatedbits = sym.opponentsactivebits = sym.opponentsdealtbits = 0;
		sym.opponentsplayingbits = sym.opponentsblindbits = 0;
		sym.nfriendsseated = sym.nfriendsactive = sym.nfriendsdealt = 0;
		sym.nfriendsplaying = sym.nfriendsblind = 0;
		sym.friendsseatedbits = sym.friendsactivebits = sym.friendsdealtbits = 0;
		sym.friendsplayingbits = sym.friendsblindbits = 0;

		// common cards
		sym.ncommoncardspresent = sym.ncommoncardsknown = sym.nflopc = 0;

		// (un)known cards
		sym.nouts = sym.ncardsknown = sym.ncardsunknown = sym.ncardsbetter = 0;

		// nhands
		sym.nhands = sym.nhandshi = sym.nhandslo = sym.nhandsti = sym.prwinnow = sym.prlosnow = 0;

		// flushes straights sets
		sym.nsuited = sym.nsuitedcommon = sym.tsuit = sym.tsuitcommon = 0;
		sym.nranked = sym.nrankedcommon = sym.trank = sym.trankcommon = 0;
		//straightfill inits changed from 0 to 5; 2008-03-25 Matrix
		sym.nstraight = sym.nstraightcommon = 0;
		sym.nstraightflush = sym.nstraightflushcommon = 0;
		sym.nstraightfill = sym.nstraightfillcommon = sym.nstraightflushfill = sym.nstraightflushfillcommon = 5;
		// rank bits
		sym.rankbits = sym.rankbitscommon = sym.rankbitsplayer = sym.rankbitspoker = 0;
		sym.srankbits = sym.srankbitscommon = sym.srankbitsplayer = sym.srankbitspoker = 0;

		// rank hi
		sym.rankhi = sym.rankhicommon = sym.rankhiplayer = sym.rankhipoker = 0;
		sym.srankhi = sym.srankhicommon = sym.srankhiplayer = sym.srankhipoker = 0;

		// rank lo
		sym.ranklo = sym.ranklocommon = sym.rankloplayer = sym.ranklopoker = 0;
		sym.sranklo = sym.sranklocommon = sym.srankloplayer = sym.sranklopoker = 0;

		// time
		sym.elapsed = sym.elapsedhand = sym.elapsedauto = sym.elapsedtoday = sym.elapsed1970 = 0;
		sym.clocks = sym.nclockspersecond = sym.ncps = 0;
		time(&elapsedhold);
		time(&elapsedhandhold);
		time(&elapsedautohold);

		// autoplayer
		sym.myturnbits = sym.ismyturn = sym.issittingin = sym.isautopost = 0;
		sym.issittingout = 1;
		sym.isfinalanswer = 0;

		// history
		for (i=0; i<=4; i++)
		{
			sym.nplayersround[i] = 0;
			sym.nbetsround[i] = 0;
			sym.didchec[i] = 0;
			sym.didcall[i] = 0;
			sym.didrais[i] = 0;
			sym.didswag[i] = 0;
		}

		//run$ ron$
		sym.ron$royfl = sym.ron$strfl = sym.ron$4kind = sym.ron$fullh = sym.ron$flush = sym.ron$strai = 0;
		sym.ron$3kind = sym.ron$2pair = sym.ron$1pair = sym.ron$hcard = sym.ron$total = sym.ron$pokervalmax = 0;
		sym.ron$prnuts = sym.ron$prbest = sym.ron$clocks = 0;
		sym.run$royfl = sym.run$strfl = sym.run$4kind = sym.run$fullh = sym.run$flush = sym.run$strai = 0;
		sym.run$3kind = sym.run$2pair = sym.run$1pair = sym.run$hcard = sym.run$total = sym.run$pokervalmax = 0;
		sym.run$prnuts = sym.run$prbest = sym.run$clocks = 0;

		// vs$
		sym.vs$nhands = sym.vs$nhandshi = sym.vs$nhandsti = sym.vs$nhandslo = 0;
		sym.vs$prwin = sym.vs$prtie = sym.vs$prlos = 0;
		sym.vs$prwinhi = sym.vs$prtiehi = sym.vs$prloshi = 0;
		sym.vs$prwinti = sym.vs$prtieti = sym.vs$prlosti = 0;
		sym.vs$prwinlo = sym.vs$prtielo = sym.vs$prloslo = 0;

		// action symbols
		f$alli = f$swag = f$rais = f$call = f$play = f$prefold = 0;


		bigbet = 0;
		reset_stakes = true;

		// icm
		for (i=0; i<=9; i++)
			stacks_at_hand_start[i] = 0;

		// delay
		f$delay = 0;

		// Reset semi-persistent hand state when we instantiate CSymbols.
		CSymbols::dealerchair_last = -1;
		CSymbols::handnumber_last = -1;
		CSymbols::br_last = -1;
		CSymbols::player_card_last[0] = CARD_NOCARD;
		CSymbols::player_card_last[1] = CARD_NOCARD;

		// log$ symbols
		logsymbols_collection.RemoveAll();

		symboltrace_collection.RemoveAll();

	LeaveCriticalSection(&cs_symbols);

    // time
	EnterCriticalSection(&cs_scraper);
	    QueryPerformanceCounter(&scraper.clockshold);
	LeaveCriticalSection(&cs_scraper);

	__SEH_LOGFATAL("CSymbols::ResetSymbolsFirstTime : \n");
}

void CSymbols::ResetSymbolsNewHand(void)
{
	__SEH_HEADER

    int		i;

	EnterCriticalSection(&cs_symbols);	

		// handrank
		sym.handrank169 = sym.handrank2652 = sym.handrank1326 = sym.handrank1000 = sym.handrank = 0;

		// chip amounts
		for (i=0; i<5; i++)
			sym.bet[i] = 0;

		// list tests
		sym.islistcall = sym.islistrais = sym.islistalli = 0;
		for (i=0; i<MAX_HAND_LISTS; i++)
			sym.islist[i] = 0;

		sym.isemptylistcall = sym.isemptylistrais = sym.isemptylistalli = 0;

		// list numbers
		sym.nlistmax = -1;
		sym.nlistmin = -1;

		// hand tests
		for (i=0; i<2; i++)
		{
			sym.$$pc[i] = WH_NOCARD;
			sym.$$pr[i] = 0;
			sym.$$ps[i] = 0;
		}

		for (i=0; i<5; i++)
		{
			sym.$$cc[i] = WH_NOCARD;
			sym.$$cs[i] = 0;
			sym.$$cr[i] = 0;
		}

		for (i=0; i<4; i++)
		{
			phandval[i] = 0;
			chandval[i] = 0;
		}

		// players friends opponents
		sym.nopponentsmax = 0;
		sym.nplayersseated = sym.nplayersdealt = sym.nplayersblind = 0;
		sym.nopponentsseated = sym.nopponentsdealt = sym.nopponentsblind = 0;
		sym.playersseatedbits = sym.playersdealtbits = sym.playersblindbits = 0;
		sym.opponentsseatedbits = sym.opponentsdealtbits = sym.opponentsblindbits = 0;

		// time
		time(&elapsedhandhold);

		// history
		for (i=0; i<=4; i++)
		{
			sym.nplayersround[i] = 0;
			sym.nbetsround[i] = 0;
			sym.didchec[i] = 0;
			sym.didcall[i] = 0;
			sym.didrais[i] = 0;
			sym.didswag[i] = 0;
		}

		// action symbols
		f$alli = f$swag = f$rais = f$call = f$play = f$prefold = 0;

		// icm
		for (i=0; i<=9; i++)
			stacks_at_hand_start[i] = 0;

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::ResetSymbolsNewHand : \n");
}

void CSymbols::ResetSymbolsNewRound(void)
{
    __SEH_HEADER

	EnterCriticalSection(&cs_symbols);	

		sym.didchec[4] = 0;
		sym.didcall[4] = 0;
		sym.didrais[4] = 0;
		sym.didswag[4] = 0;

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::ResetSymbolsNewRound : \n");
}

void CSymbols::ResetSymbolsEveryCalc(void)
{
    __SEH_HEADER

	EnterCriticalSection(&cs_symbols);	

		int		i;
		// general
		sym.isbring = 0;

		// limits
		sym.sraiprev = sym.sraimin = sym.sraimax = 0;

		// handrank
		sym.handrankp = 0;

		// chairs
		sym.raischair = 0;

		// rounds positions
		sym.betposition = sym.dealposition = sym.callposition = sym.seatposition = 1;
		sym.dealpositionrais = sym.betpositionrais = 1;

		// statistics
		sym.callror = sym.raisror = sym.srairor = sym.alliror = 0;
		sym.callmean = sym.raismean = sym.sraimean = sym.allimean = 0;
		sym.callvariance = sym.raisvariance = sym.sraivariance = sym.allivariance = 0;

		// chip amounts
		for (i=0; i<11; i++)
			sym.currentbet[i] = 0;

		for (i=0; i<10; i++)
			sym.stack[i] = 0;

		sym.pot = sym.potcommon = sym.potplayer = 0;
		sym.call = sym.callshort = sym.raisshort = 0;

		// number of bets
		sym.nbetstocall = sym.nbetstorais = sym.ncurrentbets = sym.ncallbets = sym.nraisbets = 0;

		// poker values
		sym.pokerval = sym.pokervalplayer = sym.pokervalcommon = sym.pcbits = sym.npcbits = 0;

		// hand tests
		sym.ishandup = sym.ishandupcommon = 0;
		sym.ishicard = sym.isonepair = sym.istwopair = sym.isthreeofakind = sym.isstraight = 0;
		sym.isflush = sym.isfullhouse = sym.isfourofakind = sym.isstraightflush = sym.isroyalflush = 0;
		sym.isfiveofakind = 0;

		// pocket tests
		sym.ispair = sym.issuited = sym.isconnector = 0;

		// pocket/common tests
		sym.ishipair = sym.islopair = sym.ismidpair = sym.ishistraight = sym.ishiflush = 0;

		// players friends opponents
		sym.nopponents = 0;
		sym.nopponentschecking = sym.nopponentscalling = sym.nopponentsraising = 0;
		sym.nopponentsbetting = sym.nopponentsfolded = 0;
		sym.nplayerscallshort = sym.nchairsdealtright = sym.nchairsdealtleft = 0;
		sym.nplayersactive = sym.playersactivebits = 0;
		sym.nplayersplaying = sym.playersplayingbits = 0;
		sym.nopponentsactive = sym.opponentsactivebits = 0;
		sym.nopponentsplaying = sym.opponentsplayingbits = 0;

		// flags
		sym.fmax = sym.fbits = 0;
		for (i=0; i<10; i++)
			sym.f[i] = 0;

		// (un)known cards
		sym.nouts = sym.ncardsknown = sym.ncardsunknown = sym.ncardsbetter = 0;

		// nhands
		sym.nhands = sym.nhandshi = sym.nhandslo = sym.nhandsti = sym.prwinnow = sym.prlosnow = 0;

		// flushes straights sets
		sym.nsuited = sym.nsuitedcommon = sym.tsuit = sym.tsuitcommon = 0;
		sym.nranked = sym.nrankedcommon = sym.trank = sym.trankcommon = 0;
		//straightfill inits changed from 0 to 5; 2008-03-25 Matrix
		sym.nstraight = sym.nstraightcommon = 0;
		sym.nstraightflush = sym.nstraightflushcommon = 0;
		sym.nstraightfill = sym.nstraightfillcommon = sym.nstraightflushfill = sym.nstraightflushfillcommon = 5;

		// rank bits
		sym.rankbits = sym.rankbitscommon = sym.rankbitsplayer = sym.rankbitspoker = 0;
		sym.srankbits = sym.srankbitscommon = sym.srankbitsplayer = sym.srankbitspoker = 0;

		// rank hi
		sym.rankhi = sym.rankhicommon = sym.rankhiplayer = sym.rankhipoker = 0;
		sym.srankhi = sym.srankhicommon = sym.srankhiplayer = sym.srankhipoker = 0;

		// rank lo
		sym.ranklo = sym.ranklocommon = sym.rankloplayer = sym.ranklopoker = 0;
		sym.sranklo = sym.sranklocommon = sym.srankloplayer = sym.sranklopoker = 0;

		// autoplayer
		sym.myturnbits = sym.ismyturn = sym.issittingin = sym.isautopost = 0;
		sym.issittingout = 1;
		sym.isfinalanswer = 0;

		//run$ ron$
		sym.ron$royfl = sym.ron$strfl = sym.ron$4kind = sym.ron$fullh = sym.ron$flush = sym.ron$strai = 0;
		sym.ron$3kind = sym.ron$2pair = sym.ron$1pair = sym.ron$hcard = sym.ron$total = sym.ron$pokervalmax = 0;
		sym.ron$prnuts = sym.ron$prbest = sym.ron$clocks = 0;
		sym.run$royfl = sym.run$strfl = sym.run$4kind = sym.run$fullh = sym.run$flush = sym.run$strai = 0;
		sym.run$3kind = sym.run$2pair = sym.run$1pair = sym.run$hcard = sym.run$total = sym.run$pokervalmax = 0;
		sym.run$prnuts = sym.run$prbest = sym.run$clocks = 0;

		// vs$
		sym.vs$nhands = sym.vs$nhandshi = sym.vs$nhandsti = sym.vs$nhandslo = 0;
		sym.vs$prwin = sym.vs$prtie = sym.vs$prlos = 0;
		sym.vs$prwinhi = sym.vs$prtiehi = sym.vs$prloshi = 0;
		sym.vs$prwinti = sym.vs$prtieti = sym.vs$prlosti = 0;
		sym.vs$prwinlo = sym.vs$prtielo = sym.vs$prloslo = 0;

	LeaveCriticalSection(&cs_symbols);

	// log$ symbols
	logsymbols_collection.RemoveAll();

	symboltrace_collection.RemoveAll();

    __SEH_LOGFATAL("CSymbols::ResetSymbolsEveryCalc : \n");
}

void CSymbols::CalcSymbols(void)
{
    __SEH_HEADER

    int					i, error;
    char				classname[50], title[512];
    unsigned int		player_card_cur[2];
    char				card0[10], card1[10];
	double				result;

	// Variables gathered in critical sections below
	bool				scr_dealer[10];
	unsigned int		card_player0, card_player1, card_common[5];
	bool				ucc, ica, res_stak, s_limit_info_istournament;
	double				s_limit_info_handnumber, sahs[10], sym_bblind, sym_sblind;
	int					sym_br, sym_chair;

	EnterCriticalSection(&cs_scraper);
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	LeaveCriticalSection(&cs_scraper);

	
	// Clear em, before we start
    ResetSymbolsEveryCalc();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Global environment symbols
	EnterCriticalSection(&cs_symbols);	

		sym.session = global.Session_ID;												// session
		sym.nopponentsmax = global.preferences.max_opponents;							// nopponentsmax
		sym.swagdelay = global.preferences.swag_delay_3;								// swagdelay
		sym.allidelay = -1;																// allidelay  (unused in OpenHoldem)
		sym.version = VERSION_NUMBER;													// version
		GetClassName(global.attached_hwnd, classname, 50);
		if (strcmp(classname, "BRING")==0)
			sym.isbring = 1;															// isbring

		if (strcmp(classname, "OpenHoldemManualMode")==0)
			sym.ismanual = 1;															// ismanual

	LeaveCriticalSection(&cs_symbols);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Identification of dealerchair
	EnterCriticalSection(&cs_scraper);
	for (i=0; i < global.trans.map.num_chairs; i++)
		scr_dealer[i] = scraper.dealer[i];
	LeaveCriticalSection(&cs_scraper);

	EnterCriticalSection(&cs_symbols);	
	for (i=0; i < global.trans.map.num_chairs; i++)
    {
        if (scr_dealer[i])
        {
            sym.dealerchair = i;													// dealerchair
            i = global.trans.map.num_chairs + 1;
        }
    }
	LeaveCriticalSection(&cs_symbols);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Identification of userchair
	EnterCriticalSection(&cs_symbols);	
	ucc = user_chair_confirmed;
	LeaveCriticalSection(&cs_symbols);

    if (!ucc)
    {
        bool new_ucc = calc_userchair();

		EnterCriticalSection(&cs_symbols);	
		user_chair_confirmed = new_ucc;
		LeaveCriticalSection(&cs_symbols);

        if (new_ucc)
		{
			EnterCriticalSection(&cs_symbols);	
            time(&elapsedhold);
			LeaveCriticalSection(&cs_symbols);
		}
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Things to do when we have a new hand
	EnterCriticalSection(&cs_symbols);	
	ucc = user_chair_confirmed;
	int		sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);

    // Get current userchair's cards
	EnterCriticalSection(&cs_scraper);
		if (ucc)
		{
			player_card_cur[0] = scraper.card_player[sym_userchair][0];
			player_card_cur[1] = scraper.card_player[sym_userchair][1];
		}
		else
		{
			player_card_cur[0] = player_card_cur[1] = CARD_NOCARD;
		}
		s_limit_info_handnumber = scraper.s_limit_info.handnumber;
	LeaveCriticalSection(&cs_scraper);

	EnterCriticalSection(&cs_symbols);
    sym.handnumber = s_limit_info_handnumber;									// handnumber
	LeaveCriticalSection(&cs_symbols);

	EnterCriticalSection(&cs_scraper);
	for (i=0; i<=9; i++)
		sahs[i] = scraper.playerbalance[i] + scraper.playerbet[i];
	LeaveCriticalSection(&cs_scraper);

	EnterCriticalSection(&cs_symbols);
		// New hand is triggered by change in dealerchair (button moves), or change in userchair's cards (as long as it is not
		// a change to nocards or cardbacks), or a change in handnumber
		if (sym.dealerchair != dealerchair_last ||
				player_card_cur[0]!=CARD_NOCARD && player_card_cur[0]!=CARD_BACK && player_card_cur[0]!=player_card_last[0] ||
				player_card_cur[1]!=CARD_NOCARD && player_card_cur[1]!=CARD_BACK && player_card_cur[1]!=player_card_last[1] ||
				sym.handnumber != handnumber_last)
		{
			// Save for next pass
			dealerchair_last = sym.dealerchair;
			player_card_last[0] = player_card_cur[0];
			player_card_last[1] = player_card_cur[1];
			handnumber_last = sym.handnumber;
			br_last = -1;	// ensure betround reset

			// Update game_state so it knows that a new hand has happened
			game_state.new_hand = true;

			// Reset symbols and display
			ResetSymbolsNewHand();
			InvalidateRect(global.hMainFrame, NULL, true);

			// randoms
			sym.randomhand = (double) rand() / (double) RAND_MAX;						// randomhand
			sym.randomround[0] = (double) rand() / (double) RAND_MAX;					// randomround1
			sym.randomround[1] = (double) rand() / (double) RAND_MAX;					// randomround2
			sym.randomround[2] = (double) rand() / (double) RAND_MAX;					// randomround3
			sym.randomround[3] = (double) rand() / (double) RAND_MAX;					// randomround4

			// search for new stakes
			reset_stakes = true;

			// icm
			for (i=0; i<=9; i++)
				stacks_at_hand_start[i] = sahs[i];

			// log new hand
			if (player_card_cur[0]==CARD_NOCARD || player_card_cur[0]==CARD_NOCARD)
			{
				strcpy(card0, "NONE");
				strcpy(card1, "");
			}
			else if (player_card_cur[1]==CARD_BACK || player_card_cur[1]==CARD_BACK)
			{
				strcpy(card0, "BACKS");
				strcpy(card1, "");
			}
			else
			{
				StdDeck_cardToString(player_card_cur[0], card0);
				StdDeck_cardToString(player_card_cur[1], card1);
			}
			GetWindowText(global.attached_hwnd, title, 512);
			write_log("\n*************************************************************\nHAND RESET (num:%.0f dealer:%.0f cards:%s%s): %s\n*************************************************************\n",
					  sym.handnumber, sym.dealerchair, card0, card1, title);
		}
	LeaveCriticalSection(&cs_symbols);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // common cards
    sym.nflopc = 0;

	EnterCriticalSection(&cs_symbols);
		for (i=0; i<=4; i++)
		{
			if (card_common[i] != CARD_NOCARD)
				sym.nflopc++;															// nflopc
		}

		sym.ncommoncardspresent = sym.nflopc;											// ncommoncardspresent
		sym.ncommoncardsknown = sym.nflopc;												// ncommoncardsknown
	LeaveCriticalSection(&cs_symbols);

    // If no common card animation is going on
	ica = scraper.is_common_animation();

	EnterCriticalSection(&cs_symbols);
		if (!ica)
		{
			sym.betround = card_common[4] != CARD_NOCARD ? 4 :
						   card_common[3] != CARD_NOCARD ? 3 :
						   card_common[2] != CARD_NOCARD &&
						   card_common[1] != CARD_NOCARD &&
						   card_common[0] != CARD_NOCARD ? 2 :
						   1;																// betround
		}
		else
		{
			// There is a common card animation going on currently
			// so lets not try to determine the betround,
			// but if it's a new hand then lets default to pre-flop
			if (br_last == -1)
			{
				sym.betround = 1;
			}
		}

	    sym.br = sym.betround;															// br
	LeaveCriticalSection(&cs_symbols);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Things to do when we have a new round
	EnterCriticalSection(&cs_symbols);
	sym_br = sym.br;
	LeaveCriticalSection(&cs_symbols);

    if (sym_br == 0) 
		return;

    if (sym_br != br_last)
    {
		EnterCriticalSection(&cs_symbols);
        br_last = sym_br;
		LeaveCriticalSection(&cs_symbols);

		ResetSymbolsNewRound();

        // log betting round change
        write_log("ROUND %.0f\n", sym_br);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Symbols derived from current profile/formula
	EnterCriticalSection(&cs_symbols);
		sym.site = 1;																	// site
		sym.nchairs = global.trans.map.num_chairs;										// nchairs
		sym.isppro = global.ppro_isppro;												// isppro
		sym.rake = global.formula.dRake;												// rake
		sym.nit = global.formula.dNit;													// nit
		sym.bankroll = global.formula.dBankroll;										// bankroll
		sym.defcon = global.formula.dDefcon;											// defcon
		sym.isdefmode = global.formula.dDefcon == 0.0;									// isdefmode
		sym.isaggmode = global.formula.dDefcon == 1.0;									// isaggmode
		sym.swagtextmethod = global.trans.map.swagtextmethod;							// swagtextmethod
		sym.potmethod = global.trans.map.potmethod;										// potmethod
		sym.activemethod = global.trans.map.activemethod;								// activemethod
	LeaveCriticalSection(&cs_symbols);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Other scraped items
	EnterCriticalSection(&cs_symbols);
		sym_bblind = sym.bblind;
		sym_sblind = sym.sblind;
		res_stak = reset_stakes;
	LeaveCriticalSection(&cs_symbols);
    
	if (res_stak || sym_sblind==0 || sym_bblind==0 ||
            (global.ppro_is_connected && global.ppro_tid != 0))
    {
        calc_stakes();	// bblind/sblind/bbet/ante/lim

		EnterCriticalSection(&cs_symbols);
        reset_stakes = false;
		LeaveCriticalSection(&cs_symbols);
    }

	EnterCriticalSection(&cs_symbols);
		sym.isnl = (sym.lim == LIMIT_NL);												// isnl
		sym.ispl = (sym.lim == LIMIT_PL);												// ispl
		sym.isfl = (sym.lim == LIMIT_FL);												// isfl
	LeaveCriticalSection(&cs_symbols);

	EnterCriticalSection(&cs_scraper);
		s_limit_info_istournament = scraper.s_limit_info.istournament;
	LeaveCriticalSection(&cs_scraper);

	EnterCriticalSection(&cs_symbols);
		sym.istournament = (double) s_limit_info_istournament;							// istournament

		sym.bet[0] = sym.bblind;														// bet1
		sym.bet[1] = sym.bblind;														// bet2
		sym.bet[2] = (bigbet!=0 ? bigbet : (sym.isnl || sym.ispl ? sym.bblind : sym.bblind*2));	// bet3
		sym.bet[3] = (bigbet!=0 ? bigbet : (sym.isnl || sym.ispl ? sym.bblind : sym.bblind*2));	// bet4
	LeaveCriticalSection(&cs_symbols);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    calc_betbalancestack();				// bets, balances, stacks
    calc_playersfriendsopponents();		// players/friends/opponents, raischair
    calc_chipamts_limits();				// chip amounts, limits
    calc_numbets();						// number of bets
    calc_flags();						// flags
    calc_time();						// time
    calc_autoplayer();					// autoplayer

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	EnterCriticalSection(&cs_symbols);
	sym_chair = (int) sym.chair;
	LeaveCriticalSection(&cs_symbols);

	EnterCriticalSection(&cs_scraper);
	card_player0 = scraper.card_player[sym_chair][0];
	card_player1 = scraper.card_player[sym_chair][1];
	LeaveCriticalSection(&cs_scraper);

    if (ucc &&
		card_player0!=CARD_NOCARD && card_player1!=CARD_NOCARD &&
		card_player0!=CARD_BACK && card_player1!=CARD_BACK)
    {
        calc_roundspositions();			// rounds positions
        calc_pokervalues();				// pokerval, pocket-common tests
        calc_unknowncards();			// (un)known cards
        calc_handtests();				// hand tests
        calc_pockettests();				// pocket tests
        calc_listtests();				// list tests, list numbers
        calc_fl_str_set();				// flushes, straights, sets
        calc_rankbits();				// rankbits, rankhi, ranklo
        versus.get_counts();			// calculate versus values

		EnterCriticalSection(&cs_symbols);
		sym.originaldealposition = sym.dealposition; //remember dealposition Matrix 2008-05-09
		LeaveCriticalSection(&cs_symbols);
    }

    calc_history();						// history symbols

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // number of opponents (from f$P)
    error = SUCCESS;
	result = calc_f$symbol(&global.formula, "f$P", &error);

	EnterCriticalSection(&cs_symbols);
		sym.nopponents = result;

		if (sym.nopponents > global.preferences.max_opponents)
		{
			sym.nopponents = global.preferences.max_opponents;					// nopponents
		}
	LeaveCriticalSection(&cs_symbols);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // dependent on nopponents calulation - must come after
    if (ucc &&
		card_player0!=CARD_NOCARD && card_player1!=CARD_NOCARD &&
		card_player0!=CARD_BACK && card_player1!=CARD_BACK)
    {
        calc_nhands();				// nhands
        calc_handrank();			// handrank
        calc_probabilities();		// prwin, random
        calc_run_ron();				// run$, ron$
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // dependent on prwin calulation - must come after
    if (ucc &&
		card_player0!=CARD_NOCARD && card_player1!=CARD_NOCARD &&
		card_player0!=CARD_BACK && card_player1!=CARD_BACK)
    {
        calc_statistics();			// ror, mean, variance
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Finally, calculate action variables for downstream use
    error = SUCCESS;
	result = calc_f$symbol(&global.formula, "f$alli", &error);
	EnterCriticalSection(&cs_symbols);
    symbols.f$alli = result;
	LeaveCriticalSection(&cs_symbols);

    error = SUCCESS;
	result = calc_f$symbol(&global.formula, "f$swag", &error);
	EnterCriticalSection(&cs_symbols);
    symbols.f$swag = result;
	LeaveCriticalSection(&cs_symbols);

    error = SUCCESS;
	result = calc_f$symbol(&global.formula, "f$rais", &error);
	EnterCriticalSection(&cs_symbols);
    symbols.f$rais = result;
	LeaveCriticalSection(&cs_symbols);

    error = SUCCESS;
	result = calc_f$symbol(&global.formula, "f$call", &error);
	EnterCriticalSection(&cs_symbols);
    symbols.f$call = result;
	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::CalcSymbols :\n");
}

bool CSymbols::calc_userchair(void)
{
    __SEH_HEADER
    int				i;
    int				num_buttons_enabled=0;
	CString			button_label;
	unsigned int	card_player[2];

	for (i=0; i<=9; i++)
	{
		EnterCriticalSection(&cs_scraper);
		button_label = scraper.buttonlabel[i];
		LeaveCriticalSection(&cs_scraper);

		if (scraper.get_button_state(i) &&
			(scraper.is_string_fold(button_label) ||
			 scraper.is_string_check(button_label) ||
			 scraper.is_string_call(button_label) ||
			 scraper.is_string_raise(button_label) ||
			 scraper.is_string_allin(button_label) ||
			 button_label.MakeLower() == "swag"))
		{
			num_buttons_enabled++;
		}
	}

    if (num_buttons_enabled>=2)
    {
        for (i=0; i<global.trans.map.num_chairs; i++)
        {
			EnterCriticalSection(&cs_scraper);
			card_player[0] = scraper.card_player[i][0] ;
			card_player[1] = scraper.card_player[i][1] ;
			LeaveCriticalSection(&cs_scraper);

            if (card_player[0] != CARD_NOCARD && card_player[0] != CARD_BACK &&
				card_player[1] != CARD_NOCARD && card_player[1] != CARD_BACK)
            {
				EnterCriticalSection(&cs_symbols);
                sym.userchair = i;													// userchair
                sym.chair = i;														// chair
				LeaveCriticalSection(&cs_symbols);

                return true;
            }
        }
    }

	return false;

	__SEH_LOGFATAL("CSymbols::calc_userchair :\n");
}

void CSymbols::calc_stakes(void)
{
    __SEH_HEADER
    int		i;
    bool	found_inferred_sb = false, found_inferred_bb = false;

	EnterCriticalSection(&cs_scraper);
	bool		s_limit_info_found_sblind = scraper.s_limit_info.found_sblind;
	bool		s_limit_info_found_bblind = scraper.s_limit_info.found_bblind;
	bool		s_limit_info_found_ante = scraper.s_limit_info.found_ante;
	bool		s_limit_info_found_limit = scraper.s_limit_info.found_limit;
	bool		s_limit_info_found_bbet = scraper.s_limit_info.found_bbet;
	bool		s_limit_info_found_sb_bb = scraper.s_limit_info.found_sb_bb;
	bool		s_limit_info_found_bb_BB = scraper.s_limit_info.found_bb_BB;
	double		s_limit_info_sblind = scraper.s_limit_info.sblind;
	double		s_limit_info_bblind = scraper.s_limit_info.bblind;
	double		s_limit_info_ante = scraper.s_limit_info.ante;
	int			s_limit_info_limit = scraper.s_limit_info.limit;
	double		s_limit_info_bbet = scraper.s_limit_info.bbet;
	double		s_limit_info_sb_bb = scraper.s_limit_info.sb_bb;
	double		s_limit_info_bb_BB = scraper.s_limit_info.bb_BB;
	bool		blinds_are_locked = scraper.blinds_are_locked;
	LeaveCriticalSection(&cs_scraper);


	EnterCriticalSection(&cs_symbols);

		sym.sblind = sym.bblind = bigbet = sym.ante = 0;

		// Save the parts we scraped successfully
		if (s_limit_info_found_sblind)
			sym.sblind = s_limit_info_sblind;									// sblind

		if (s_limit_info_found_bblind)
			sym.bblind = s_limit_info_bblind;									// bblind

		if (s_limit_info_found_ante)
			sym.ante = s_limit_info_ante;										// ante

		if (s_limit_info_found_limit)
			sym.lim = s_limit_info_limit;										// lim

		if (s_limit_info_found_bbet)
			bigbet = s_limit_info_bbet;

		// Figure out bb/sb based on game type
		if (sym.lim == LIMIT_NL || sym.lim == LIMIT_PL)
		{
			if (sym.sblind==0)
			{
				if (s_limit_info_found_sb_bb)
					sym.sblind = s_limit_info_sb_bb;
			}

			if (sym.bblind==0)
			{
				if (s_limit_info_found_bb_BB)
					sym.bblind = s_limit_info_bb_BB;
			}

			if (bigbet==0)
			{
				if (s_limit_info_found_bb_BB)
					bigbet = s_limit_info_bb_BB;

				else if (s_limit_info_found_sb_bb)
					bigbet = s_limit_info_sb_bb*2;

				else if (sym.bblind!=0)
					bigbet = sym.bblind;

				else if (sym.sblind!=0)
					bigbet = sym.sblind*2;

			}
		}

		else if (sym.lim == LIMIT_FL || sym.lim == -1)
		{
			if (sym.sblind==0)
			{
				if (s_limit_info_found_sb_bb)
					sym.sblind = s_limit_info_sb_bb;
			}

			if (sym.bblind==0)
			{
				if (s_limit_info_found_bb_BB)
					sym.bblind = s_limit_info_bb_BB;
			}

			if (bigbet==0)
			{
				if (s_limit_info_found_bb_BB)
					bigbet = s_limit_info_bb_BB*2;

				else if (s_limit_info_found_sb_bb)
					bigbet = s_limit_info_sb_bb*4;

				else if (sym.bblind!=0)
					bigbet = sym.bblind*2;

				else if (sym.sblind!=0)
					bigbet = sym.sblind*4;
			}
		}

		// Return now, if we have locked the blinds
		if (blinds_are_locked)
			return;

		// if we still do not have blinds, then infer them from the posted bets
		if (sym.br == 1 && (sym.sblind==0 || sym.bblind==0))
		{
			for (i=sym.dealerchair+1; i<=sym.dealerchair+global.trans.map.num_chairs; i++)
			{
				EnterCriticalSection(&cs_scraper);
				unsigned int	card_player0 = scraper.card_player[i%global.trans.map.num_chairs][0];
				unsigned int	card_player1 = scraper.card_player[i%global.trans.map.num_chairs][1];
				double			playerbet = scraper.playerbet[i%global.trans.map.num_chairs];
				LeaveCriticalSection(&cs_scraper);

				if (card_player0 != CARD_NOCARD && card_player1 != CARD_NOCARD)
				{
					if (playerbet != 0 && !found_inferred_sb)
					{
						if (sym.sblind==0)
						{
							sym.sblind = playerbet;
							found_inferred_sb = true;
						}
					}

					else if (playerbet != 0 && found_inferred_sb && !found_inferred_bb)
					{
						if (sym.bblind==0)
						{
							// !heads up - normal blinds
							if (i%global.trans.map.num_chairs != sym.dealerchair)
							{
								sym.bblind = playerbet;
							}
							// heads up - reversed blinds
							else
							{
								sym.bblind = sym.sblind;
								sym.sblind = playerbet;
							}
							found_inferred_bb = true;
						}
					}
				}
			}

			// check for reasonableness
			if (sym.bblind > sym.sblind*2)
				sym.bblind = sym.sblind*2;

			if (sym.sblind >= sym.bblind)
				sym.sblind = sym.bblind/2;
		}

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_stakes :\n");
}

void CSymbols::calc_betbalancestack(void)
{
    __SEH_HEADER

    int				i, j, oppcount;
    double			stack[10], temp;


	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	double			playerbalance[10], playerbet[10], pot[10];
	unsigned int	card_player0[10], card_player1[10];
	for (i=0; i<=9; i++)
	{
		playerbalance[i] = scraper.playerbalance[i];
		playerbet[i] = scraper.playerbet[i];
		card_player0[i] = scraper.card_player[i][0];
		card_player1[i] = scraper.card_player[i][1];
		pot[i] = scraper.pot[i];
	}

	double			playerbalance_userchair = scraper.playerbalance[sym_userchair];
	double			playerbet_userchair = scraper.playerbet[sym_userchair];
	LeaveCriticalSection(&cs_scraper);


	EnterCriticalSection(&cs_symbols);
		sym.bet[4] = sym.bet[(int) (sym.br-1)];											// bet

		for (i=0; i<sym.nchairs; i++)
			sym.balance[i] = sym.nchairs>i ? playerbalance[i] : 0;						// balance0-9

		sym.balance[10] = user_chair_confirmed ? playerbalance_userchair : 0;			// balance

		// simple bubble sort for 10 stack values
		for (i=0; i<sym.nchairs; i++)
		{
			if (card_player0[i] != CARD_NOCARD && card_player1[i] != CARD_NOCARD)
				stack[i] = sym.balance[i];

			else
				stack[i] = 0;
		}
	LeaveCriticalSection(&cs_symbols);


	for (i=0; i<sym.nchairs-1; i++)
	{
		for (j=i+1; j<sym.nchairs; j++)
		{
			if (stack[i]<stack[j])
			{
				temp = stack[i];
				stack[i]=stack[j];
				stack[j]=temp;
			}
		}
	}


	EnterCriticalSection(&cs_symbols);

		for (i=0; i<sym.nchairs; i++)
			sym.stack[i] = sym.nchairs>i ? stack[i] : 0;								// stack0-9


		//
		// currentbet sanity check
		//
		// Get count of opponents
		oppcount=0;
		for (i=0; i<sym.nchairs; i++)
		{
			if (card_player0[i] == CARD_BACK && card_player1[i] == CARD_BACK && i != sym.userchair)
				oppcount+=1;
		}

		for (i=0; i<sym.nchairs; i++)
		{
			temp = sym.nchairs>i ? playerbet[i] : 0;

			// Only record currentbet if pot>0 or num of opponents>0
			//if (pot[0]>0 || oppcount>0) {
			if (oppcount>0)
			{
				// fixed limit
				if (sym.isfl)
				{
					if (temp/sym.bet[4]<=4)
						sym.currentbet[i] = temp;										// currentbet0-9
				}

				// no limit, pot limit
				else
				{
					sym.currentbet[i] = temp; 											// currentbet0-9
				}
			}
		}

		sym.currentbet[10] = user_chair_confirmed ? playerbet_userchair : 0;			// currentbet

		sym.potplayer=0;
		for (i=0; i<sym.nchairs; i++)
		{
			sym.potplayer += sym.currentbet[i];											// potplayer
		}

		if (sym.potmethod == 2)															// pot, potcommon ->
		{
			sym.pot = pot[0];
			sym.potcommon = sym.pot - sym.potplayer;
		}
		else
		{
			sym.potcommon = 0;
			for (i=0; i<=4; i++)
				sym.potcommon += pot[i];

			sym.pot = sym.potcommon + sym.potplayer;									// <- pot, potcommon
		}

	LeaveCriticalSection(&cs_symbols);

	__SEH_LOGFATAL("CSymbols::calc_betbalancestack :\n");
}

void CSymbols::calc_chipamts_limits(void) 
{
    __SEH_HEADER

	int				i;
    int				next_largest_bet;

	EnterCriticalSection(&cs_symbols);	

		sym.call = user_chair_confirmed ?
				   sym.currentbet[(int) sym.raischair] - sym.currentbet[(int) sym.userchair] : 0; // call
		for (i=sym.raischair+1; i<sym.raischair+global.trans.map.num_chairs; i++)
		{
			if ((int) sym.playersplayingbits & (1<<(i%global.trans.map.num_chairs)))
			{
				sym.callshort +=
					(sym.currentbet[(int) sym.raischair] - sym.currentbet[(i%global.trans.map.num_chairs)]);	// callshort
			}
		}
		sym.raisshort = sym.callshort + sym.bet[4] * sym.nplayersplaying;				// raisshort

		next_largest_bet = 0;
		for (i=0; i<global.trans.map.num_chairs; i++)
		{
			if (sym.currentbet[i] != sym.currentbet[(int) sym.raischair] && sym.currentbet[i]>next_largest_bet)
			{
				next_largest_bet = sym.currentbet[i];
			}
		}

		sym.sraiprev = sym.currentbet[(int) sym.raischair] - next_largest_bet;			// sraiprev
		sym.sraimin = user_chair_confirmed ? sym.currentbet[10] + sym.call : 0;			// sraimin
		sym.sraimax = user_chair_confirmed ? sym.balance[10] - sym.call : 0;
		if (sym.sraimax < 0)
			sym.sraimax = 0;															// sraimax

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_chipamts_limits :\n");
}

void CSymbols::calc_numbets(void)
{
    __SEH_HEADER

	EnterCriticalSection(&cs_symbols);	

		if (user_chair_confirmed)
		{
			sym.nbetstocall = sym.call / sym.bet[4];									// nbetstocall
			sym.nbetstorais = sym.nbetstocall + 1;										// nbetstorais
			sym.ncurrentbets =
				sym.bet[4]==0 ? 0 : sym.currentbet[(int) sym.userchair] / sym.bet[4];	// ncurrentbets
		}
		sym.ncallbets =
			sym.bet[4]==0 ? 0 : sym.currentbet[(int) sym.raischair] / sym.bet[4];		// ncallbets
		sym.nraisbets = sym.ncallbets + 1;												// nraisbets

	LeaveCriticalSection(&cs_symbols);

	__SEH_LOGFATAL("CSymbols::calc_numbets :\n");
}

void CSymbols::calc_flags(void)
{
    __SEH_HEADER

	EnterCriticalSection(&cs_symbols);	

		int			i;

		for (i=0; i<10; i++)
		{
			sym.f[i] = global.flags[i];													// fn
			if (sym.f[i] != 0)
			{
				if (i > sym.fmax)
					sym.fmax = i;														// fmax

				sym.fbits = (int) sym.fbits | (1<<i);									// fbits
			}
		}

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_flags :\n");

}

void CSymbols::calc_time(void)
{
    __SEH_HEADER

    LARGE_INTEGER	lFrequency, clocksnow;
    time_t			t_now_time, t_midnight_time;
    struct tm		*s_midnight_time;

	EnterCriticalSection(&cs_scraper);
	LARGE_INTEGER	clockshold = scraper.clockshold;
	LeaveCriticalSection(&cs_scraper);

	time(&t_now_time);

	EnterCriticalSection(&cs_symbols);	

		sym.elapsed1970 = t_now_time;													// elapsed1970

		s_midnight_time = localtime(&t_now_time);
		s_midnight_time->tm_hour = 0;
		s_midnight_time->tm_min = 0;
		s_midnight_time->tm_sec = 0;
		t_midnight_time = mktime(s_midnight_time);
		sym.elapsedtoday = t_now_time - t_midnight_time;								// elapsedtoday

		sym.elapsed = t_now_time - elapsedhold;											// elapsed
		sym.elapsedhand = t_now_time - elapsedhandhold;									// elapsedhand
		sym.elapsedauto = t_now_time - elapsedautohold;									// elapsedauto

		QueryPerformanceFrequency(&lFrequency);
		sym.nclockspersecond = lFrequency.LowPart;										// nclockspersecond
		sym.ncps = sym.nclockspersecond;												// ncps

		QueryPerformanceCounter(&clocksnow);

		sym.clocks = clocksnow.LowPart - clockshold.LowPart;							// clocks

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_time :\n");
}

void CSymbols::calc_autoplayer(void)
{
    __SEH_HEADER

    int		i;
    bool    sitin_but, sitout_but, sitin_state, sitout_state;

	EnterCriticalSection(&cs_scraper);
	CString		buttonlabel[10];
	for (i=0; i<=9; i++)
	{
		buttonlabel[i] = scraper.buttonlabel[i];
	}
	LeaveCriticalSection(&cs_scraper);

	EnterCriticalSection(&cs_symbols);	

		for (i=0; i<=9; i++)
		{
			if (scraper.get_button_state(i))
			{
				if (scraper.is_string_fold(buttonlabel[i]))
					sym.myturnbits = (int) sym.myturnbits | (1<<0);

				else if (scraper.is_string_call(buttonlabel[i]))
					sym.myturnbits = (int) sym.myturnbits | (1<<1);

				else if (scraper.is_string_raise(buttonlabel[i]) || buttonlabel[i].MakeLower() == "swag")
					sym.myturnbits = (int) sym.myturnbits | (1<<2);

				else if (scraper.is_string_allin(buttonlabel[i]))
					sym.myturnbits = (int) sym.myturnbits | (1<<3);						// myturnbits

				else if (scraper.is_string_autopost(buttonlabel[i]))
					sym.isautopost = 1;													// isautopost

			}
		}
		sym.ismyturn = (int) sym.myturnbits & 0x7;										// ismyturn

		// Figure out sitin/sitout state
		sitin_but = sitout_but = sitin_state = sitout_state = false;
		for (i=0; i < 10; i++)
		{
			if (scraper.is_string_sitin(buttonlabel[i]))
			{
				sitin_but = true;
				sitin_state = scraper.get_button_state(i);
			}

			else if (scraper.is_string_sitout(buttonlabel[i]))
			{
				sitout_but = true;
				sitout_state = scraper.get_button_state(i);
			}
		}



		// we have a sitin button
		if (sitin_but)
		{
			if (sitin_state)
			{
				sym.issittingin = 1;												// issittingin
				sym.issittingout = 0;												// issittingout
			}

			else
			{
				sym.issittingin = 0;												// issittingin
				sym.issittingout = 1;												// issittingout
			}
		}

		// we have a sitout button
		else if (sitout_but)
		{
			if (sitout_state)
			{
				sym.issittingin = 0;												// issittingin
				sym.issittingout = 1;												// issittingout
			}

			else
			{
				sym.issittingin = 1;												// issittingin
				sym.issittingout = 0;												// issittingout
			}
		}

		// we have neither a sitout or sitin button
		else
		{
			sym.issittingin = 1;												// issittingin
			sym.issittingout = 0;												// issittingout
		}

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_autoplayer :\n");
}

void CSymbols::calc_probabilities(void)
{
    __SEH_HEADER

    bool			need_recalc;
    int				i;

	EnterCriticalSection(&cs_iterator);
	double			nopp_lastrun = iterator_status.iterator_run_with_f$p;
	double			nit_lastrun = iterator_status.iterator_run_with_nit;
	double			br_lastrun = iterator_status.iterator_run_with_br;
	unsigned int	playercards_lastrun[2], commoncards_lastrun[5];
	for (i=0; i<=1; i++)
		playercards_lastrun[i] = iterator_status.iterator_run_with_playercards[i];
	for (i=0; i<=4; i++)
		commoncards_lastrun[i] = iterator_status.iterator_run_with_commoncards[i];
	LeaveCriticalSection(&cs_iterator);


	EnterCriticalSection(&cs_symbols);	
	int					sym_userchair = (int) sym.userchair;
	double				sym_nit = sym.nit;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int		card_common[5];
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	unsigned int		card_player0_userchair = scraper.card_player[sym_userchair][0];
	unsigned int		card_player1_userchair = scraper.card_player[sym_userchair][1];
	LeaveCriticalSection(&cs_scraper);


	EnterCriticalSection(&cs_symbols);	
		sym.random = (double) rand() / (double) RAND_MAX;								// random
		sym.randomround[4] = sym.randomround[(int) (sym.br-1)];							// randomround

		// Start/restart prwin thread on these conditions:
		// - changed f$P (nopponents)
		// - changed nit
		// - changed br
		// - changed player cards
		// - changed common cards
		need_recalc = false;
		if (sym.nopponents != nopp_lastrun ||
				sym.nit != nit_lastrun ||
				sym.br != br_lastrun)
		{
			need_recalc = true;
		}
	LeaveCriticalSection(&cs_symbols);

	if (card_player0_userchair != playercards_lastrun[0] || card_player1_userchair != playercards_lastrun[1])
		need_recalc = true;

	for (i=0; i<=4; i++)
	{
		if (card_common[i] != commoncards_lastrun[i])
			need_recalc = true;
	}

	// restart iterator thread
    if (need_recalc && sym_nit>0)
    {
		if (p_iterator_thread)
		{
			delete p_iterator_thread;
			write_log("Stopped iterator thread: %08x\n", p_iterator_thread);
			p_iterator_thread = NULL;
		}

		p_iterator_thread = new CIteratorThread;
		write_log("Started iterator thread: %08x\n", p_iterator_thread);
    }

    __SEH_LOGFATAL("CSymbols::calc_probabilities :\n");
}

void CSymbols::calc_playersfriendsopponents(void)
{
    __SEH_HEADER
    int				i;
    double			lastbet;
    bool			sblindfound, bblindfound, found_userchair;

	EnterCriticalSection(&cs_scraper);
	double			playerbet[10];
	unsigned int	card_player0[10], card_player1[10], card_common[5];
	CString			seated[10], active[10];

	for (i=0; i<=9; i++)
	{
		playerbet[i] = scraper.playerbet[i];
		card_player0[i] = scraper.card_player[i][0];
		card_player1[i] = scraper.card_player[i][1];
		seated[i] = scraper.seated[i];
		active[i] = scraper.active[i];
	}
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	LeaveCriticalSection(&cs_scraper);

		
	lastbet = 0;
    sblindfound = false;
    bblindfound = false;

	EnterCriticalSection(&cs_symbols);

		for (i=sym.dealerchair+1; i<=sym.dealerchair+global.trans.map.num_chairs; i++)
		{
			if (playerbet[i%global.trans.map.num_chairs] > lastbet)
			{
				lastbet = playerbet[i%global.trans.map.num_chairs];
				sym.raischair = i%global.trans.map.num_chairs;									// raischair
			}

			if (!sblindfound && playerbet[i%global.trans.map.num_chairs]<=sym.sblind)
			{
				sblindfound = true;
				sym.playersblindbits =
					(int) sym.playersblindbits | (1<<(i%global.trans.map.num_chairs));			// playersblindbits
				if (user_chair_confirmed && (i%global.trans.map.num_chairs) != sym.userchair)
				{
					sym.opponentsblindbits =
						(int) sym.opponentsblindbits | (1<<(i%global.trans.map.num_chairs));		// opponentsblindbits
				}

				if (user_chair_confirmed && (i%global.trans.map.num_chairs) == sym.userchair)
				{
					sym.friendsblindbits =
						(int) sym.friendsblindbits | (1<<(i%global.trans.map.num_chairs));		// friendsblindbits
				}
			}

			else if (!bblindfound && playerbet[i%global.trans.map.num_chairs]<=sym.bblind)
			{
				bblindfound = true;
				sym.bblindbits = 0; //prwin change
				sym.playersblindbits =
					(int) sym.playersblindbits | (1<<(i%global.trans.map.num_chairs));			// playersblindbits
				if (user_chair_confirmed && (i%global.trans.map.num_chairs) != sym.userchair)
				{
					sym.opponentsblindbits =
						(int) sym.opponentsblindbits | (1<<(i%global.trans.map.num_chairs));		// opponentsblindbits
					sym.bblindbits =  //prwin change
						(int) sym.bblindbits | (1<<(i%global.trans.map.num_chairs));		// big blind bit
				}
			}
		}

		sym.nplayersblind = bitcount(sym.playersblindbits);								// nplayersblind
		sym.nopponentsblind = bitcount(sym.opponentsblindbits);							// nopponentsblind
		sym.nfriendsblind = bitcount(sym.friendsblindbits);								// nfriendsblind

		for (i=0; i<global.trans.map.num_chairs; i++)
		{
			if (card_player0[i] != CARD_NOCARD && card_player1[i] != CARD_NOCARD)
			{
				sym.playersdealtbits = (int) sym.playersdealtbits | (1<<i);				// playersdealtbits
				sym.playersplayingbits = (int) sym.playersplayingbits | (1<<i);			// playersplayingbits

				if (user_chair_confirmed && i!=sym.userchair)
				{
					sym.opponentsdealtbits = (int) sym.opponentsdealtbits | (1<<i);		// opponentsdealtbits
					sym.opponentsplayingbits = (int) sym.opponentsplayingbits | (1<<i);	// opponentsplayingbits
				}

				if (user_chair_confirmed && i==sym.userchair)
				{
					sym.friendsdealtbits = (int) sym.friendsdealtbits | (1<<i);			// friendsdealtbits
					sym.friendsplayingbits = (int) sym.friendsplayingbits | (1<<i);		// friendsplayingbits
				}
			}

			if (scraper.is_string_seated(seated[i]))
			{
				sym.playersseatedbits = (int) sym.playersseatedbits | (1<<i);			// playersseatedbits

				if (user_chair_confirmed && i!=sym.userchair)
				{
					sym.opponentsseatedbits = (int) sym.opponentsseatedbits | (1<<i);	// opponentsseatedbits
				}
				if (user_chair_confirmed && i==sym.userchair)
				{
					sym.friendsseatedbits = (int) sym.friendsseatedbits | (1<<i);		// friendsseatedbits
				}
			}

			if (scraper.is_string_active(active[i]))
			{
				sym.playersactivebits = (int) sym.playersactivebits | (1<<i);			// playersactivebits
				if (user_chair_confirmed && i!=sym.userchair)
				{
					sym.opponentsactivebits = (int) sym.opponentsactivebits | (1<<i);	// opponentsactivebits
				}
				if (user_chair_confirmed && i==sym.userchair)
				{
					sym.friendsactivebits = (int) sym.friendsactivebits | (1<<i);		// friendsactivebits
				}
			}
		}

		sym.nplayersseated = bitcount(sym.playersseatedbits);							// nplayersseated
		sym.nplayersactive = bitcount(sym.playersactivebits);							// nplayersactive
		sym.nplayersdealt = bitcount(sym.playersdealtbits);								// nplayersdealt
		sym.nplayersplaying = bitcount(sym.playersplayingbits);							// nplayersplaying
		sym.nopponentsseated = bitcount(sym.opponentsseatedbits);						// nopponentsseated
		sym.nopponentsactive = bitcount(sym.opponentsactivebits);						// nopponentsactive
		sym.nopponentsdealt = bitcount(sym.opponentsdealtbits);							// nopponentsdealt
		sym.nopponentsplaying = bitcount(sym.opponentsplayingbits);						// nopponentsplaying
		sym.nfriendsseated = bitcount(sym.friendsseatedbits);							// nfriendsseated
		sym.nfriendsactive = bitcount(sym.friendsactivebits);							// nfriendsactive
		sym.nfriendsdealt = bitcount(sym.friendsdealtbits);								// nfriendsdealt
		sym.nfriendsplaying = bitcount(sym.friendsplayingbits);							// nfriendsplaying

		found_userchair = false;
		lastbet = 0;
		for (i=sym.dealerchair+1; i<=sym.dealerchair+global.trans.map.num_chairs && user_chair_confirmed; i++)
		{
			if ((i%global.trans.map.num_chairs) == sym.userchair)
				found_userchair = true;

			else
			{
				if ((int) sym.playersdealtbits & (1<<(i%global.trans.map.num_chairs)))
				{
					if (!found_userchair)
						sym.nchairsdealtright += 1;										// nchairsdealtright

					else
						sym.nchairsdealtleft += 1;										// nchairsdealtleft

				}
			}

			if (playerbet[i%global.trans.map.num_chairs] > lastbet &&
					playerbet[i%global.trans.map.num_chairs] != 0)
			{
				lastbet = playerbet[i%global.trans.map.num_chairs];

				if ((i%global.trans.map.num_chairs) != sym.userchair)
					sym.nopponentsraising += 1;											// nopponentsraising
			}

			else if ((i%global.trans.map.num_chairs) != sym.userchair &&
					 playerbet[i%global.trans.map.num_chairs] == lastbet &&
					 lastbet != 0)
			{
				sym.nopponentscalling += 1;												// nopponentscalling
			}

			if ((i%global.trans.map.num_chairs) != sym.userchair &&
					playerbet[i%global.trans.map.num_chairs] > 0)
			{
				sym.nopponentsbetting += 1;												// nopponentsbetting
			}

			if ((i%global.trans.map.num_chairs) != sym.userchair &&
					(int) sym.playersdealtbits & (1<<(i%global.trans.map.num_chairs)) &&
					(card_player0[i%global.trans.map.num_chairs] == CARD_NOCARD ||
					 card_player1[i%global.trans.map.num_chairs] == CARD_NOCARD))
			{
				sym.nopponentsfolded += 1;												// nopponentsfolded
			}

			if ((i%global.trans.map.num_chairs) != sym.userchair &&
					card_player0[i%global.trans.map.num_chairs] != CARD_NOCARD &&
					card_player1[i%global.trans.map.num_chairs] != CARD_NOCARD &&
					playerbet[i%global.trans.map.num_chairs] == 0)
			{
				sym.nopponentschecking += 1;											// nopponentschecking
			}

			if (playerbet[i%global.trans.map.num_chairs] < sym.currentbet[(int) sym.raischair] &&
					card_player0[i%global.trans.map.num_chairs] != CARD_NOCARD &&
					card_player1[i%global.trans.map.num_chairs] != CARD_NOCARD)
			{
				sym.nplayerscallshort += 1;												// nplayerscallshort
			}
		}

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_playersfriendsopponents :\n");
}

void CSymbols::calc_roundspositions(void) 
{
    __SEH_HEADER

	EnterCriticalSection(&cs_symbols);	

		int			i;

		for (i=sym.dealerchair+1; i<=sym.dealerchair+global.trans.map.num_chairs && (i%global.trans.map.num_chairs)!=sym.userchair; i++)
		{
			if ((int)sym.playersplayingbits & (1<<(i%global.trans.map.num_chairs)))
				sym.betposition+=1;													// betposition

			if ((int)sym.playersdealtbits & (1<<(i%global.trans.map.num_chairs)))
				sym.dealposition+=1;												// dealposition
		}

		for (i=sym.raischair+1; i<=sym.raischair+global.trans.map.num_chairs && (i%global.trans.map.num_chairs)!=sym.userchair; i++)
		{
			if ((int)sym.playersplayingbits & (1<<(i%global.trans.map.num_chairs)))
				sym.callposition+=1;												// callposition
		}

		for (i=sym.dealerchair+1; i<=sym.dealerchair+global.trans.map.num_chairs && (i%global.trans.map.num_chairs)!=sym.userchair; i++)
		{
			if ((int)sym.playersseatedbits & (1<<(i%global.trans.map.num_chairs)))
				sym.seatposition+=1;												// seatposition
		}

		for (i=sym.dealerchair+1; i<=sym.dealerchair+global.trans.map.num_chairs && (i%global.trans.map.num_chairs)!=sym.raischair; i++)
		{
			if ((int)sym.playersplayingbits & (1<<(i%global.trans.map.num_chairs)))
				sym.betpositionrais+=1;												// betpositionrais

			if ((int)sym.playersdealtbits & (1<<(i%global.trans.map.num_chairs)))
				sym.dealpositionrais+=1;											// dealpositionrais
		}

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_roundspositions :\n");
}

void CSymbols::calc_pokervalues(void)
{
    __SEH_HEADER

    int				i;
    CardMask		Cards;
    int				nCards;
    HandVal			handval;
    double			dummy;
    int				hi_common_rank, lo_common_rank;

    // poker constants
    sym.hicard			= 0x00000001;
    sym.onepair			= 0x01000000;
    sym.twopair			= 0x02000000;
    sym.threeofakind	= 0x04000000;
    sym.straight		= 0x08000000;
    sym.flush			= 0x10000000;
    sym.fullhouse		= 0x20000000;
    sym.fourofakind		= 0x40000000;
    sym.straightflush	= 0x80000000;
    sym.royalflush		= 0x800EDCBA;
    sym.fiveofakind		= 0xFF000000;


	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2], card_common[5];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	LeaveCriticalSection(&cs_scraper);


	///////////////////////////////////////////////////////////////////
	// pokerval
	nCards = 0;
	CardMask_RESET(Cards);
	for (i=0; i<=1; i++)
	{
		// player cards
		if (card_player[i] != CARD_BACK &&
			card_player[i] != CARD_NOCARD)
		{
			CardMask_SET(Cards, card_player[i]);
			nCards++;
		}
	}

	hi_common_rank = -1;
	lo_common_rank = 99;
	for (i=0; i<=4; i++)
	{
		// common cards
		if (card_common[i] != CARD_BACK &&
			card_common[i] != CARD_NOCARD)
		{
			CardMask_SET(Cards, card_common[i]);
			nCards++;
			if ((int) StdDeck_RANK(card_common[i]) > hi_common_rank)
				hi_common_rank = StdDeck_RANK(card_common[i]);

			if ((int) StdDeck_RANK(card_common[i]) < lo_common_rank)
				lo_common_rank = StdDeck_RANK(card_common[i]);
		}
	}

    handval = Hand_EVAL_N(Cards, nCards);

	EnterCriticalSection(&cs_symbols);

		sym.pcbits = 0;
		sym.pokerval = calc_pokerval(handval, nCards, &sym.pcbits,					// pcbits
									 card_player[0], card_player[1]);				// pokerval

		sym.npcbits = bitcount(sym.pcbits);											// npcbits
		phandval[(int)sym.br-1] = (int)sym.pokerval&0xff000000; //change from previous handval assignment 2008-03-02
		if (sym.br>1 &&	phandval[(int)sym.br-1] > phandval[(int)sym.br-2])
			sym.ishandup = 1;														// ishandup

		if (HandVal_HANDTYPE(handval) == HandType_STFLUSH &&
				StdDeck_RANK(HandVal_TOP_CARD(handval)) == Rank_ACE)
		{
			sym.isroyalflush = 1; 													// isroyalflush
		}

		else if (HandVal_HANDTYPE(handval) == HandType_STFLUSH)
		{
			sym.isstraightflush = 1; 												// isstraightflush
		}

		else if (HandVal_HANDTYPE(handval) == HandType_QUADS)
		{
			sym.isfourofakind = 1; 													// isfourofakind
		}

		else if (HandVal_HANDTYPE(handval) == HandType_FULLHOUSE)
		{
			sym.isfullhouse = 1; 													// isfullhouse
		}

		else if (HandVal_HANDTYPE(handval) == HandType_FLUSH)
		{
			sym.isflush = 1;														// isflush
			// ishiflush ->
			// If we have a pocket Ace in our flush
			if (StdDeck_RANK(HandVal_TOP_CARD(handval)) == 12 && ((int)sym.pcbits & 0x10))
			{
				sym.ishiflush = 1;
			}
			// If we have a pocket King, and it's the second best card in our flush
			else if (StdDeck_RANK(HandVal_SECOND_CARD(handval)) == 11 && ((int)sym.pcbits & 0x8))
			{
				sym.ishiflush = 1;
			}
			// If we have a pocket Queen, and it's the third best card in our flush
			else if (StdDeck_RANK(HandVal_THIRD_CARD(handval)) == 10 && ((int)sym.pcbits & 0x4))
			{
				sym.ishiflush = 1;
			}
			// If we have a pocket Jack, and it's the fourth best card in our flush
			else if (StdDeck_RANK(HandVal_FOURTH_CARD(handval)) == 9 && ((int)sym.pcbits & 0x2))
			{
				sym.ishiflush = 1;
			}																		// <- ishiflush
		}

		else if (HandVal_HANDTYPE(handval) == HandType_STRAIGHT)
		{
			sym.isstraight = 1; 													// isstraight

			// If it is an Ace high straight
			if (StdDeck_RANK(HandVal_TOP_CARD(handval)) == 12 )
			{
				sym.ishistraight = 1;												// ishistraight
			}
		}

		else if (HandVal_HANDTYPE(handval) == HandType_TRIPS)
		{
			sym.isthreeofakind = 1; 												// isthreeofakind
		}

		else if (HandVal_HANDTYPE(handval) == HandType_TWOPAIR)
		{
			sym.istwopair = 1; 														// istwopair
		}

		else if (HandVal_HANDTYPE(handval) == HandType_ONEPAIR)
		{
			sym.isonepair = 1; 														// isonepair

			// hi lo med pair
			if (nCards == 2)
			{
				sym.ishipair = 1;													// ishipair
			}
			else if (nCards >= 5)
			{
				if ((int) StdDeck_RANK(HandVal_TOP_CARD(handval)) >= hi_common_rank)
				{
					sym.ishipair = 1;												// ishipair
				}
				else if ((int) StdDeck_RANK(HandVal_TOP_CARD(handval)) < hi_common_rank &&
						 (int) StdDeck_RANK(HandVal_TOP_CARD(handval)) > lo_common_rank)
				{
					sym.ismidpair = 1;												// ismidpair
				}
				else
				{
					sym.islopair = 1;												// islopair
				}
			}
		}
		else if (HandVal_HANDTYPE(handval) == HandType_NOPAIR)
		{
			sym.ishicard = 1; 														// ishicard
		}
		sym.isfiveofakind = 0;														// isfiveofakind

	LeaveCriticalSection(&cs_symbols);


	///////////////////////////////////////////////////////////////////
	// pokervalplayer
    nCards = 0;
    CardMask_RESET(Cards);
    for (i=0; i<=1; i++)
    {
        // player cards
		if (card_player[i] != CARD_BACK && 
			card_player[i] != CARD_NOCARD &&
            !CardMask_CARD_IS_SET(Cards, card_player[i]) )
        {
            CardMask_SET(Cards, card_player[i]);
            nCards++;
        }
    }
    handval = Hand_EVAL_N(Cards, nCards);

	EnterCriticalSection(&cs_symbols);	
    sym.pokervalplayer = calc_pokerval(handval, nCards, &dummy, CARD_NOCARD, CARD_NOCARD); // pokervalplayer
	LeaveCriticalSection(&cs_symbols);


	///////////////////////////////////////////////////////////////////
    // pokervalcommon
    nCards = 0;
    CardMask_RESET(Cards);
    for (i=0; i<=4; i++)
    {
        // common cards
        if (card_common[i] != CARD_BACK && 
			card_common[i] != CARD_NOCARD &&
			!CardMask_CARD_IS_SET(Cards, card_common[i]) )
        {
            CardMask_SET(Cards, card_common[i]);
            nCards++;
        }
    }
    handval = Hand_EVAL_N(Cards, nCards);

	EnterCriticalSection(&cs_symbols);	
		sym.pokervalcommon = calc_pokerval(handval, nCards, &dummy, CARD_NOCARD, CARD_NOCARD);	// pokervalcommon
		chandval[(int)sym.br-1] = (int)sym.pokervalcommon&0xff000000; //change from previous handval assignment 2008-03-02
		if (sym.br>1 &&	chandval[(int)sym.br-1] > chandval[(int)sym.br-2])
		{
			sym.ishandupcommon = 1;															// ishandupcommon
		}
	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_pokervalues :\n");
}

void CSymbols::calc_unknowncards(void)
{
    __SEH_HEADER
    int				i;
    CardMask		stdCards, commonCards;
    int				nstdCards, ncommonCards;
    HandVal			handval_std, handval_std_plus1, handval_common_plus1;
    double			dummy;


	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2], card_common[5];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	LeaveCriticalSection(&cs_scraper);


	nstdCards = ncommonCards = 0;
    CardMask_RESET(stdCards);
    CardMask_RESET(commonCards);

	for (i=0; i<=1; i++)
	{
		// player cards
		if (card_player[i] != CARD_BACK && card_player[i] != CARD_NOCARD)
		{
			CardMask_SET(stdCards, card_player[i]);
			nstdCards++;
		}
	}
	for (i=0; i<=4; i++)
	{
		// common cards
		if (card_common[i] != CARD_BACK &&
			card_common[i] != CARD_NOCARD)
		{
			CardMask_SET(stdCards, card_common[i]);
			nstdCards++;
			CardMask_SET(commonCards, card_common[i]);
			ncommonCards++;
		}
	}

	EnterCriticalSection(&cs_symbols);	

		sym.ncardsknown = nstdCards;													// ncardsknown
		sym.ncardsunknown = 52 - sym.ncardsknown;										// ncardsunknown
		handval_std = Hand_EVAL_N(stdCards, nstdCards);

		if (sym.br<4 && user_chair_confirmed)
		{
			// iterate through every unseen card and see what happens to our handvals
			for (i=0; i<=51; i++)
			{
				if (i!=card_player[0] && card_player[1] &&
						i!=card_common[0] &&
						i!=card_common[1] &&
						i!=card_common[2] &&
						i!=card_common[3] &&
						i!=card_common[4])
				{

					CardMask_SET(stdCards, i);
					handval_std_plus1 = Hand_EVAL_N(stdCards, nstdCards+1);
					CardMask_UNSET(stdCards, i);

					CardMask_SET(commonCards, i);
					handval_common_plus1 = Hand_EVAL_N(commonCards, ncommonCards+1);
					CardMask_UNSET(commonCards, i);

					if (HandVal_HANDTYPE(handval_std_plus1) > HandVal_HANDTYPE(handval_std) &&
							calc_pokerval(handval_std_plus1, nstdCards+1, &dummy, CARD_NOCARD, CARD_NOCARD) > sym.pokerval &&
							HandVal_HANDTYPE(handval_std_plus1) > HandVal_HANDTYPE(handval_common_plus1))
					{
						sym.nouts++;													// nouts
					}
					if (calc_pokerval(handval_common_plus1, ncommonCards+1, &dummy, CARD_NOCARD, CARD_NOCARD) > sym.pokerval)
					{
						sym.ncardsbetter++;												// ncardsbetter
					}
				}
			}
		}

	LeaveCriticalSection(&cs_symbols);

	__SEH_LOGFATAL("CSymbols::calc_unknowncards :\n");
}

void CSymbols::calc_handtests(void)
{
    __SEH_HEADER

	int		i;

	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2], card_common[5];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	LeaveCriticalSection(&cs_scraper);


	EnterCriticalSection(&cs_symbols);

		for (i=0; i<=1; i++)
		{
			if (card_player[i] != CARD_NOCARD && card_player[i] != CARD_BACK)
			{
				sym.$$pc[i] = ((StdDeck_RANK(card_player[i])+2)<<4) |		//$$pcx
							  (StdDeck_SUIT(card_player[i]) == StdDeck_Suit_CLUBS ? WH_SUIT_CLUBS :
							   StdDeck_SUIT(card_player[i]) == StdDeck_Suit_DIAMONDS ? WH_SUIT_DIAMONDS :
							   StdDeck_SUIT(card_player[i]) == StdDeck_Suit_HEARTS ? WH_SUIT_HEARTS :
							   StdDeck_SUIT(card_player[i]) == StdDeck_Suit_SPADES ? WH_SUIT_SPADES : 0) ;
				sym.$$pr[i] = ((int)sym.$$pc[i] & 0xf0) >> 4;							// $$prx
				sym.$$ps[i] = (int)sym.$$pc[i] & 0x0f;									// $$psx
			}
		}

		for (i=0; i<=4; i++)
		{
			if (card_common[i] != CARD_NOCARD && card_common[i] != CARD_BACK)
			{
				sym.$$cc[i] = ((StdDeck_RANK(card_common[i])+2)<<4) |			// $$ccx
							  (StdDeck_SUIT(card_common[i]) == StdDeck_Suit_CLUBS ? WH_SUIT_CLUBS :
							   StdDeck_SUIT(card_common[i]) == StdDeck_Suit_DIAMONDS ? WH_SUIT_DIAMONDS :
							   StdDeck_SUIT(card_common[i]) == StdDeck_Suit_HEARTS ? WH_SUIT_HEARTS :
							   StdDeck_SUIT(card_common[i]) == StdDeck_Suit_SPADES ? WH_SUIT_SPADES : 0) ;
				sym.$$cr[i] = ((int)sym.$$cc[i] & 0xf0) >> 4;							// $$crx
				sym.$$cs[i] = (int)sym.$$cc[i] & 0x0f;									// $$csx
			}
		}

	LeaveCriticalSection(&cs_symbols);

	__SEH_LOGFATAL("CSymbols::calc_handtests :\n");
}

void CSymbols::calc_pockettests(void)
{
    __SEH_HEADER

	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	LeaveCriticalSection(&cs_scraper);


	EnterCriticalSection(&cs_symbols);

		if (card_player[0] != CARD_NOCARD && card_player[0] != CARD_BACK &&
			card_player[1] != CARD_NOCARD && card_player[1] != CARD_BACK)
		{
			if (StdDeck_RANK(card_player[0]) == StdDeck_RANK(card_player[1]))
			{
				sym.ispair = 1;															// ispair
			}

			if (StdDeck_SUIT(card_player[0]) == StdDeck_SUIT(card_player[1]))
			{
				sym.issuited = 1;														// issuited
			}

			if (StdDeck_RANK(card_player[0]) - StdDeck_RANK(card_player[1]) == 1 ||
				StdDeck_RANK(card_player[1]) - StdDeck_RANK(card_player[0]) == 1 )
			{
				sym.isconnector = 1;													// isconnector
			}
		}

	LeaveCriticalSection(&cs_symbols);

	__SEH_LOGFATAL("CSymbols::calc_pockettests :\n");
}

void CSymbols::calc_listtests(void)
{
    __SEH_HEADER

	int				i, N;
    int				listnum;
    int				tokpos;
    unsigned int	c0, c1;

	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	LeaveCriticalSection(&cs_scraper);


	if (StdDeck_RANK(card_player[0]) >= StdDeck_RANK(card_player[1]))
	{
		c0 = card_player[0];
		c1 = card_player[1];
	}
	else
	{
		c0 = card_player[1];
		c1 = card_player[0];
	}


	N = global.formula.mHandList.GetSize();
    for (i=0; i<N; i++)
    {
        listnum = atoi(global.formula.mHandList[i].list.Mid(4).GetString());

        if ((StdDeck_SUIT(c0)==StdDeck_SUIT(c1) && global.formula.inlist[listnum][StdDeck_RANK(c0)][StdDeck_RANK(c1)]) ||
			(StdDeck_SUIT(c0)!=StdDeck_SUIT(c1) && global.formula.inlist[listnum][StdDeck_RANK(c1)][StdDeck_RANK(c0)]))
        {
			EnterCriticalSection(&cs_symbols);
				sym.islist[listnum] = 1;											// islistxx
				if (listnum < sym.nlistmin || sym.nlistmin == -1)
				{
					sym.nlistmin = listnum;											// nlistmin
				}
				if (listnum > sym.nlistmax)
				{
					sym.nlistmax = listnum;											// nlistmax
				}
			LeaveCriticalSection(&cs_symbols);
        }
    }

    // islistcall, islistrais, islistalli, isemptylistcall, isemptylistrais, isemptylistalli
    N = global.formula.mHandList.GetSize();

	EnterCriticalSection(&cs_symbols);
    sym.isemptylistcall = 1;
    sym.isemptylistrais = 1;
    sym.isemptylistalli = 1;
	LeaveCriticalSection(&cs_symbols);

	for (i=0; i<N; i++)
    {
        listnum = atoi(global.formula.mHandList[i].list.Mid(4).GetString());

		if (listnum == 0)
        {
			EnterCriticalSection(&cs_symbols);
            if (sym.islist[listnum])
            {
                sym.islistcall = 1;													// islistcall
            }
            tokpos = 0;
            global.formula.mHandList[i].list_text.Tokenize(" \t", tokpos);
            if (tokpos != -1)
            {
                sym.isemptylistcall = 0;											// isemptylistcall
            }
			LeaveCriticalSection(&cs_symbols);
        }

		else if (listnum == 1)
        {
			EnterCriticalSection(&cs_symbols);
            if (sym.islist[listnum])
            {
                sym.islistrais = 1;													// islistrais
            }
            tokpos = 0;
            global.formula.mHandList[i].list_text.Tokenize(" \t", tokpos);
            if (tokpos != -1)
            {
                sym.isemptylistrais = 0;											// isemptylistrais
            }
			LeaveCriticalSection(&cs_symbols);
        }

		else if (listnum == 7)
        {
			EnterCriticalSection(&cs_symbols);
            if (sym.islist[listnum])
            {
                sym.islistalli = 1;													// islistalli
            }
            tokpos = 0;
            global.formula.mHandList[i].list_text.Tokenize(" \t", tokpos);
            if (tokpos != -1)
            {
                sym.isemptylistalli = 0;											// isemptylistalli
            }
			LeaveCriticalSection(&cs_symbols);
        }
    }

    __SEH_LOGFATAL("CSymbols::calc_listtests :\n");
}

void CSymbols::calc_nhands(void)
{
    __SEH_HEADER
    CardMask		plCards, comCards, oppCards, playerEvalCards, opponentEvalCards;
    HandVal			hv_player, hv_opponent;
    unsigned int	pl_pokval, opp_pokval;
    int				i, j;
    double			dummy = 0;
    int				nplCards, ncomCards;


	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2], card_common[5];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	LeaveCriticalSection(&cs_scraper);


	// player cards
	CardMask_RESET(plCards);
	nplCards = 0;
	for (i=0; i<=1; i++)
	{
		if (card_player[i] != CARD_BACK && card_player[i] != CARD_NOCARD)
		{
			CardMask_SET(plCards, card_player[i]);
			nplCards++;
		}
	}

	// common cards
	CardMask_RESET(comCards);
	ncomCards = 0;
	for (i=0; i<=4; i++)
	{
		if (card_common[i] != CARD_BACK && card_common[i] != CARD_NOCARD)
		{
			CardMask_SET(comCards, card_common[i]);
			ncomCards++;
		}
	}


    // player/common cards and pokerval
    CardMask_OR(playerEvalCards, plCards, comCards);
    hv_player = Hand_EVAL_N(playerEvalCards, nplCards+ncomCards);
    pl_pokval = calc_pokerval(hv_player, nplCards+ncomCards, &dummy, CARD_NOCARD, CARD_NOCARD);


    for (i=0; i<=50; i++)
    {
        for (j=i+1; j<=51; j++)
        {
            if (!CardMask_CARD_IS_SET(plCards, i) &&
                    !CardMask_CARD_IS_SET(plCards, j) &&
                    !CardMask_CARD_IS_SET(comCards, i) &&
                    !CardMask_CARD_IS_SET(comCards, j))
            {

                // opponent cards
                CardMask_RESET(oppCards);
                CardMask_SET(oppCards, i);
                CardMask_SET(oppCards, j);

                CardMask_OR(opponentEvalCards, oppCards, comCards);
                hv_opponent = Hand_EVAL_N(opponentEvalCards, 2+ncomCards);
                opp_pokval = calc_pokerval(hv_opponent, 2+ncomCards, &dummy, CARD_NOCARD, CARD_NOCARD);

				EnterCriticalSection(&cs_symbols);
					if (pl_pokval > opp_pokval)
						sym.nhandslo++;

					else if (pl_pokval < opp_pokval)
						sym.nhandshi++;

					else
						sym.nhandsti++;
				LeaveCriticalSection(&cs_symbols);
            }
        }
    }

	EnterCriticalSection(&cs_symbols);	
		sym.nhands = sym.nhandshi + sym.nhandsti + sym.nhandslo;

		sym.prwinnow = pow((sym.nhandslo/sym.nhands), sym.nopponents);

		sym.prlosnow = 1 - pow(((sym.nhandslo + sym.nhandsti)/sym.nhands), sym.nopponents);
	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_nhands :\n");
}

void CSymbols::calc_handrank(void)
{
    __SEH_HEADER

	char		cardstr[10];
    int			i, count;


	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	LeaveCriticalSection(&cs_scraper);


    // Get a string containing the players' current cards
    get_cardstring(cardstr, card_player[0], card_player[1]);


	EnterCriticalSection(&cs_symbols);

		// if nopponents<1 or >9 then default to a sane value
		if (sym.nopponents<1)
			count = 1;

		else if (sym.nopponents>9)
			count = 9;

		else
			count = (int) sym.nopponents;

		for (i=0; i<169; i++)
		{
			if (strcmp(cardstr, handrank169[count-1][i])==0)
			{
				sym.handrank169 = i+1;													// handrank169
				sym.handrank2652 = handrank2652[count-1][i];							// handrank2652
				i = 170;
			}
		}
		sym.handrank1326 = sym.handrank2652/2;											// handrank1326
		sym.handrank1000 = 1000*sym.handrank2652/2652;									// handrank1000
		sym.handrankp = 2652.0 / (1.0+(double)count);									// handrankp
		if (global.preferences.handrank_value == "169")
			sym.handrank = sym.handrank169;

		else if (global.preferences.handrank_value == "2652")
			sym.handrank = sym.handrank2652;

		else if (global.preferences.handrank_value == "1326")
			sym.handrank = sym.handrank1326;

		else if (global.preferences.handrank_value == "1000")
			sym.handrank = sym.handrank1000;

		else if (global.preferences.handrank_value == "p")
			sym.handrank = sym.handrankp;												// handrank

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_handrank :\n");
}

void CSymbols::calc_fl_str_set(void)
{
    __SEH_HEADER

    int				i, j, n;
    CardMask		plCards, comCards, heartsCards, diamondsCards, clubsCards, spadesCards, suittestCards;
    int				max;
    unsigned int	strbits;

    // Set up some suit masks
    CardMask_RESET(heartsCards);
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_2, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_3, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_4, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_5, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_6, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_7, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_8, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_9, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_TEN, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_JACK, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_QUEEN, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_KING, Suit_HEARTS));
    CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_ACE, Suit_HEARTS));

    CardMask_RESET(diamondsCards);
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_2, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_3, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_4, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_5, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_6, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_7, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_8, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_9, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_TEN, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_JACK, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_QUEEN, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_KING, Suit_DIAMONDS));
    CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_ACE, Suit_DIAMONDS));

    CardMask_RESET(spadesCards);
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_2, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_3, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_4, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_5, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_6, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_7, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_8, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_9, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_TEN, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_JACK, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_QUEEN, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_KING, Suit_SPADES));
    CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_ACE, Suit_SPADES));

    CardMask_RESET(clubsCards);
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_2, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_3, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_4, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_5, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_6, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_7, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_8, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_9, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_TEN, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_JACK, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_QUEEN, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_KING, Suit_CLUBS));
    CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_ACE, Suit_CLUBS));


	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2], card_common[5];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	LeaveCriticalSection(&cs_scraper);


	// player cards
	CardMask_RESET(plCards);
	for (i=0; i<=1; i++)
	{
		if (card_player[i] != CARD_BACK && card_player[i] != CARD_NOCARD)
		{
			CardMask_SET(plCards, card_player[i]);
		}
	}

	// common cards
	CardMask_RESET(comCards);
	for (i=0; i<=4; i++)
	{
		if (card_common[i] != CARD_BACK && card_common[i] != CARD_NOCARD)
		{
			CardMask_SET(comCards, card_common[i]);
			CardMask_SET(plCards, card_common[i]);
		}
	}

	EnterCriticalSection(&cs_symbols);

		// nsuited, tsuit
		max = 0;
		CardMask_AND(suittestCards, plCards, spadesCards);
		n = StdDeck_numCards(suittestCards);
		if ( n>max && n>0)
		{
			max = n;
			sym.tsuit = WH_SUIT_SPADES;
		}
		CardMask_AND(suittestCards, plCards, heartsCards);
		n = StdDeck_numCards(suittestCards);
		if ( n>max && n>0)
		{
			max = n;
			sym.tsuit = WH_SUIT_HEARTS;
		}
		CardMask_AND(suittestCards, plCards, diamondsCards);
		n = StdDeck_numCards(suittestCards);
		if ( n>max && n>0)
		{
			max = n;
			sym.tsuit = WH_SUIT_DIAMONDS;
		}
		CardMask_AND(suittestCards, plCards, clubsCards);
		n = StdDeck_numCards(suittestCards);
		if ( n>max && n>0)
		{
			max = n;
			sym.tsuit = WH_SUIT_CLUBS;													// tsuit
		}
		sym.nsuited = max;																// nsuited

		// nsuitedcommon, tsuitcommon
		max = 0;
		CardMask_AND(suittestCards, comCards, spadesCards);
		n = StdDeck_numCards(suittestCards);
		if ( n>max && n>0)
		{
			max = n;
			sym.tsuitcommon = WH_SUIT_SPADES;
		}
		CardMask_AND(suittestCards, comCards, heartsCards);
		n = StdDeck_numCards(suittestCards);
		if ( n>max && n>0)
		{
			max = n;
			sym.tsuitcommon = WH_SUIT_HEARTS;
		}
		CardMask_AND(suittestCards, comCards, diamondsCards);
		n = StdDeck_numCards(suittestCards);
		if ( n>max && n>0)
		{
			max = n;
			sym.tsuitcommon = WH_SUIT_DIAMONDS;
		}
		CardMask_AND(suittestCards, comCards, clubsCards);
		n = StdDeck_numCards(suittestCards);
		if ( n>max && n>0)
		{
			max = n;
			sym.tsuitcommon = WH_SUIT_CLUBS;											// tsuitcommon
		}
		sym.nsuitedcommon = max;														// nsuitedcommon

		// nranked, trank
		max = 0;
		for (i=12; i>=0; i--)
		{
			n = CardMask_CARD_IS_SET(plCards, i+(Rank_COUNT*0)) +
				CardMask_CARD_IS_SET(plCards, i+(Rank_COUNT*1)) +
				CardMask_CARD_IS_SET(plCards, i+(Rank_COUNT*2)) +
				CardMask_CARD_IS_SET(plCards, i+(Rank_COUNT*3));
			if (n>max && n>0)
			{
				max = n;
				sym.trank = i+2;														// trank
			}
		}
		sym.nranked = max;																// nranked

		// nrankedcommon, trankcommon
		max = 0;
		for (i=12; i>=0; i--)
		{
			n = CardMask_CARD_IS_SET(comCards, i+(Rank_COUNT*0)) +
				CardMask_CARD_IS_SET(comCards, i+(Rank_COUNT*1)) +
				CardMask_CARD_IS_SET(comCards, i+(Rank_COUNT*2)) +
				CardMask_CARD_IS_SET(comCards, i+(Rank_COUNT*3));
			if (n>max && n>0)
			{
				max = n;
				sym.trankcommon = i+2;													// trankcommon
			}
		}
		sym.nrankedcommon = max;														// nrankedcommon

		// nstraight, nstraightfill
		strbits = 0;
		for (i=0; i<Rank_COUNT; i++)
		{
			if (CardMask_CARD_IS_SET(plCards, (Rank_COUNT*0)+i) ||
				CardMask_CARD_IS_SET(plCards, (Rank_COUNT*1)+i) ||
				CardMask_CARD_IS_SET(plCards, (Rank_COUNT*2)+i) ||
				CardMask_CARD_IS_SET(plCards, (Rank_COUNT*3)+i))
			{
				strbits |= (1<<(i+2));
			}
		}
		if (CardMask_CARD_IS_SET(plCards, (Rank_COUNT*0)+Rank_ACE) ||
			CardMask_CARD_IS_SET(plCards, (Rank_COUNT*1)+Rank_ACE) ||
			CardMask_CARD_IS_SET(plCards, (Rank_COUNT*2)+Rank_ACE) ||
			CardMask_CARD_IS_SET(plCards, (Rank_COUNT*3)+Rank_ACE))
		{
			strbits |= (1<<1);
		}

		for (i=10; i>=1; i--)
		{
			if (((strbits>>i)&0x1f) == 0x1f)
			{
				sym.nstraight = (sym.nstraight<5 ? 5 : sym.nstraight);
			}
			else if (((strbits>>i)&0x1e) == 0x1e ||
					 ((strbits>>i)&0x0f) == 0x0f)
			{
				sym.nstraight = (sym.nstraight<4 ? 4 : sym.nstraight);
			}
			else if (((strbits>>i)&0x1c) == 0x1c ||
					 ((strbits>>i)&0x0e) == 0x0e ||
					 ((strbits>>i)&0x07) == 0x07)
			{
				sym.nstraight = (sym.nstraight<3 ? 3 : sym.nstraight);
			}
			else if (((strbits>>i)&0x18) == 0x18 ||
					 ((strbits>>i)&0x0c) == 0x0c ||
					 ((strbits>>i)&0x06) == 0x06 ||
					 ((strbits>>i)&0x3) == 0x03)
			{
				sym.nstraight = (sym.nstraight<2 ? 2 : sym.nstraight);
			}
			else {
				sym.nstraight = (sym.nstraight<1 ? 1 : sym.nstraight);					// nstraight
			}
			n = bitcount(((strbits>>i)&0x1f));
			if (5-n < sym.nstraightfill)					// 2008-03-25 Matrix
			{
				sym.nstraightfill = 5-n;												// nstraightfill
			}
		}

		// nstraightcommon, nstraightfillcommon
		if (sym.nflopc<1)
		{
			sym.nstraightfillcommon = 5;
		}
		else
		{
			strbits = 0;
			for (i=0; i<Rank_COUNT; i++)
			{
				if (CardMask_CARD_IS_SET(comCards, (Rank_COUNT*0)+i) ||
					CardMask_CARD_IS_SET(comCards, (Rank_COUNT*1)+i) ||
					CardMask_CARD_IS_SET(comCards, (Rank_COUNT*2)+i) ||
					CardMask_CARD_IS_SET(comCards, (Rank_COUNT*3)+i))
				{
					strbits |= (1<<(i+2));
				}
			}
			if (CardMask_CARD_IS_SET(comCards, (Rank_COUNT*0)+Rank_ACE) ||
				CardMask_CARD_IS_SET(comCards, (Rank_COUNT*1)+Rank_ACE) ||
				CardMask_CARD_IS_SET(comCards, (Rank_COUNT*2)+Rank_ACE) ||
				CardMask_CARD_IS_SET(comCards, (Rank_COUNT*3)+Rank_ACE))
			{
				strbits |= (1<<1);
			}

			for (i=10; i>=1; i--)
			{
				if (((strbits>>i)&0x1f) == 0x1f)
				{
					sym.nstraightcommon = (sym.nstraightcommon<5 ? 5 : sym.nstraightcommon);
				}
				else if (((strbits>>i)&0x1e) == 0x1e ||
						 ((strbits>>i)&0x0f) == 0x0f)
				{
					sym.nstraightcommon = (sym.nstraightcommon<4 ? 4 : sym.nstraightcommon);
				}
				else if (((strbits>>i)&0x1c) == 0x1c ||
						 ((strbits>>i)&0x0e) == 0x0e ||
						 ((strbits>>i)&0x07) == 0x07)
				{
					sym.nstraightcommon = (sym.nstraightcommon<3 ? 3 : sym.nstraightcommon);
				}
				else if (((strbits>>i)&0x18) == 0x18 ||
						 ((strbits>>i)&0x0c) == 0x0c ||
						 ((strbits>>i)&0x06) == 0x06 ||
						 ((strbits>>i)&0x03) == 0x03)
				{
					sym.nstraightcommon = (sym.nstraightcommon<2 ? 2 : sym.nstraightcommon);
				}
				else
				{
					sym.nstraightcommon = (sym.nstraightcommon<1 ? 1 : sym.nstraightcommon); // nstraightcommon
				}
				n = bitcount(((strbits>>i)&0x1f));
				if (5-n < sym.nstraightfillcommon)					//2008-03-25 Matrix
				{
					sym.nstraightfillcommon = 5-n;										// nstraightfillcommon
				}
			}
		}

		// nstraightflush, nstraightflushfill
		for (j=0; j<4; j++)
		{
			strbits = 0;
			for (i=0; i<Rank_COUNT; i++)
			{
				if (CardMask_CARD_IS_SET(plCards, (Rank_COUNT*j)+i))
				{
					strbits |= (1<<(i+2));
				}
			}
			if (CardMask_CARD_IS_SET(plCards, (Rank_COUNT*j)+Rank_ACE))
			{
				strbits |= (1<<1);
			}

			for (i=10; i>=1; i--)
			{
				if (((strbits>>i)&0x1f) == 0x1f)
				{
					sym.nstraightflush = (sym.nstraightflush<5 ? 5 : sym.nstraightflush);
				}
				else if (((strbits>>i)&0x1e) == 0x1e ||
						 ((strbits>>i)&0x0f) == 0x0f)
				{
					sym.nstraightflush = (sym.nstraightflush<4 ? 4 : sym.nstraightflush);
				}
				else if (((strbits>>i)&0x1c) == 0x1c ||
						 ((strbits>>i)&0x0e) == 0x0e ||
						 ((strbits>>i)&0x07) == 0x07)
				{
					sym.nstraightflush = (sym.nstraightflush<3 ? 3 : sym.nstraightflush);
				}
				else if (((strbits>>i)&0x18) == 0x18 ||
						 ((strbits>>i)&0x0c) == 0x0c ||
						 ((strbits>>i)&0x06) == 0x06 ||
						 ((strbits>>i)&0x03) == 0x03)
				{
					sym.nstraightflush = (sym.nstraightflush<2 ? 2 : sym.nstraightflush);
				}
				else
				{
					sym.nstraightflush = (sym.nstraightflush<1 ? 1 : sym.nstraightflush);	// nstraightflush
				}
				n = bitcount(((strbits>>i)&0x1f));
				if (5-n < sym.nstraightflushfill)						// 2008-03-25 Matrix
				{
					sym.nstraightflushfill = 5-n;										// nstraightflushfill
				}
			}
		}

		// nstraightflushcommon, nstraightflushfillcommon
		if (sym.nflopc<1)
		{
			sym.nstraightflushfillcommon = 5;
		}
		else
		{
			for (j=0; j<=3; j++)
			{
				strbits = 0;
				for (i=0; i<Rank_COUNT; i++)
				{
					if (CardMask_CARD_IS_SET(comCards, (Rank_COUNT*j)+i))
					{
						strbits |= (1<<(i+2));
					}
				}
				if (CardMask_CARD_IS_SET(comCards, (Rank_COUNT*j)+Rank_ACE))
				{
					strbits |= (1<<1);
				}

				for (i=10; i>=1; i--)
				{
					if (((strbits>>i)&0x1f) == 0x1f)
					{
						sym.nstraightflushcommon = (sym.nstraightflushcommon<5 ? 5 : sym.nstraightflushcommon);
					}
					else if (((strbits>>i)&0x1e) == 0x1e ||
							 ((strbits>>i)&0x0f) == 0x0f)
					{
						sym.nstraightflushcommon = (sym.nstraightflushcommon<4 ? 4 : sym.nstraightflushcommon);
					}
					else if (((strbits>>i)&0x1c) == 0x1c ||
							 ((strbits>>i)&0x0e) == 0x0e ||
							 ((strbits>>i)&0x07) == 0x07)
					{
						sym.nstraightflushcommon = (sym.nstraightflushcommon<3 ? 3 : sym.nstraightflushcommon);
					}
					else if (((strbits>>i)&0x18) == 0x18 ||
							 ((strbits>>i)&0x0c) == 0x0c ||
							 ((strbits>>i)&0x06) == 0x06 ||
							 ((strbits>>i)&0x03) == 0x03)
					{
						sym.nstraightflushcommon = (sym.nstraightflushcommon<2 ? 2 : sym.nstraightflushcommon);
					}
					else
					{
						sym.nstraightflushcommon =
							(sym.nstraightflushcommon<1 ? 1 : sym.nstraightflushcommon);		// nstraightflushcommon
					}
					n = bitcount(((strbits>>i)&0x1f));
					if (5-n < sym.nstraightflushfillcommon )			// 2008-03-25 Matrix
					{
						sym.nstraightflushfillcommon = 5-n;								// nstraightflushfillcommon
					}
				}
			}
		}

	LeaveCriticalSection(&cs_symbols);

	__SEH_LOGFATAL("CSymbols::calc_fl_str_set :\n");
}

void CSymbols::calc_rankbits(void)
{
    __SEH_HEADER
    int				i, rank, suit, plcomsuit, comsuit;
    CardMask		plCards, comCards, plcomCards;

    CardMask_RESET(plCards);
    CardMask_RESET(comCards);
    CardMask_RESET(plcomCards);


	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2], card_common[5];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	LeaveCriticalSection(&cs_scraper);


	// player cards
	for (i=0; i<=1; i++)
	{
		if (card_player[i] != CARD_BACK && card_player[i] != CARD_NOCARD)
		{
			CardMask_SET(plCards, card_player[i]);
			CardMask_SET(plcomCards, card_player[i]);
		}
	}

	// common cards
	for (i=0; i<=4; i++)
	{
		if (card_common[i] != CARD_BACK && card_common[i] != CARD_NOCARD)
		{
			CardMask_SET(comCards, card_common[i]);
			CardMask_SET(plcomCards, card_common[i]);
		}
	}


	EnterCriticalSection(&cs_symbols);

		for (suit=StdDeck_Suit_FIRST; suit<=StdDeck_Suit_LAST; suit++)
		{
			for (rank=StdDeck_Rank_LAST; rank>=StdDeck_Rank_FIRST; rank--)
			{
				if (CardMask_CARD_IS_SET(plCards, StdDeck_MAKE_CARD(rank, suit)))
				{
					sym.rankbitsplayer = (int) sym.rankbitsplayer | (1<<(rank+2));		// rankbitsplayer
					if (rank == Rank_ACE)
					{
						sym.rankbitsplayer = (int) sym.rankbitsplayer | (1<<1);
					}
					if (rank+2>sym.rankhiplayer)
					{
						sym.rankhiplayer = rank+2;										// rankhiplayer
					}
					if (rank+2<sym.rankloplayer || sym.rankloplayer==0)
					{
						sym.rankloplayer = rank+2;										// rankloplayer
					}
				}
				if (CardMask_CARD_IS_SET(comCards, StdDeck_MAKE_CARD(rank, suit)))
				{
					sym.rankbitscommon = (int) sym.rankbitscommon | (1<<(rank+2));		// rankbitscommon
					if (rank == Rank_ACE)
					{
						sym.rankbitscommon = (int) sym.rankbitscommon | (1<<1);
					}
					if (rank+2>sym.rankhicommon)
					{
						sym.rankhicommon = rank+2;										// rankhicommon
					}
					if (rank+2<sym.ranklocommon || sym.ranklocommon==0)
					{
						sym.ranklocommon = rank+2;										// ranklocommon
					}
				}
				if (CardMask_CARD_IS_SET(plcomCards, StdDeck_MAKE_CARD(rank, suit)))
				{
					sym.rankbits = (int) sym.rankbits | (1<<(rank+2));					// rankbits
					if (rank == Rank_ACE)
					{
						sym.rankbits = (int) sym.rankbits | (1<<1);
					}
					if (rank+2>sym.rankhi)
					{
						sym.rankhi = rank+2;											// rankhi
					}
					if (rank+2<sym.ranklo || sym.ranklo==0)
					{
						sym.ranklo = rank+2;											// ranklo
					}
				}
			}
		}
		sym.rankbitspoker = (1<<(((int)sym.pokerval>>16)&0xf)) +						// rankbitspoker
							(1<<(((int)sym.pokerval>>12)&0xf)) +
							(1<<(((int)sym.pokerval>>8)&0xf)) +
							(1<<(((int)sym.pokerval>>4)&0xf)) +
							(1<<(((int)sym.pokerval>>0)&0xf));
		sym.rankbitspoker += ((int)sym.rankbitspoker&0x4000) ? (1<<1) : 0; //ace
		for (i=14; i>=2; i--)
		{
			if ( (((int) sym.rankbitspoker)>>i) & 0x1)
			{
				sym.rankhipoker = i;													// rankhipoker
				i = 0;
			}
		}
		for (i=2; i<=14; i++)
		{
			if ( (((int) sym.rankbitspoker)>>i) & 0x1)
			{
				sym.ranklopoker = i;													// ranklopoker
				i = 15;
			}
		}

		comsuit = (sym.tsuitcommon==WH_SUIT_CLUBS ? Suit_CLUBS :
				   sym.tsuitcommon==WH_SUIT_DIAMONDS ? Suit_DIAMONDS :
				   sym.tsuitcommon==WH_SUIT_HEARTS ? Suit_HEARTS :
				   sym.tsuitcommon==WH_SUIT_SPADES ? Suit_SPADES : 0);
		plcomsuit = (sym.tsuit==WH_SUIT_CLUBS ? Suit_CLUBS :
					 sym.tsuit==WH_SUIT_DIAMONDS ? Suit_DIAMONDS :
					 sym.tsuit==WH_SUIT_HEARTS ? Suit_HEARTS :
					 sym.tsuit==WH_SUIT_SPADES ? Suit_SPADES : 0);
		for (rank=StdDeck_Rank_LAST; rank>=StdDeck_Rank_FIRST; rank--)
		{
			if (CardMask_CARD_IS_SET(plCards, StdDeck_MAKE_CARD(rank, plcomsuit)))
			{
				sym.srankbitsplayer = (int) sym.srankbitsplayer | (1<<(rank+2));		// srankbitsplayer
				if (rank == Rank_ACE)
				{
					sym.srankbitsplayer = (int) sym.srankbitsplayer | (1<<1);
				}
				if (rank+2>sym.srankhiplayer)
				{
					sym.srankhiplayer = rank+2;											// srankhiplayer
				}
				if (rank+2<sym.srankloplayer || sym.srankloplayer==0)
				{
					sym.srankloplayer = rank+2;											// srankloplayer
				}
			}

			if (CardMask_CARD_IS_SET(comCards, StdDeck_MAKE_CARD(rank, comsuit)))
			{
				sym.srankbitscommon = (int) sym.srankbitscommon | (1<<(rank+2));		// srankbitscommon
				if (rank == Rank_ACE)
				{
					sym.srankbitscommon = (int) sym.srankbitscommon | (1<<1);
				}
				if (rank+2>sym.srankhicommon)
				{
					sym.srankhicommon = rank+2;											// srankhicommon
				}
				if (rank+2<sym.sranklocommon || sym.sranklocommon==0)
				{
					sym.sranklocommon = rank+2;											// sranklocommon
				}
			}

			if (CardMask_CARD_IS_SET(plcomCards, StdDeck_MAKE_CARD(rank, plcomsuit)))
			{
				sym.srankbits = (int) sym.srankbits | (1<<(rank+2));					// srankbits
				if (rank == Rank_ACE)
				{
					sym.srankbits = (int) sym.srankbits | (1<<1);
				}
				if (rank+2>sym.srankhi)
				{
					sym.srankhi = rank+2;												// srankhi
				}
				if (rank+2<sym.sranklo || sym.sranklo==0)
				{
					sym.sranklo = rank+2;												// sranklo
				}
			}
		}
		sym.srankbitspoker = 															// srankbitspoker
			(CardMask_CARD_IS_SET(plcomCards, StdDeck_MAKE_CARD((((int)sym.pokerval>>16)&0xf)-2, plcomsuit)) ?
			 (1<<(((int)sym.pokerval>>16)&0xf)) : 0) +
			(CardMask_CARD_IS_SET(plcomCards, StdDeck_MAKE_CARD((((int)sym.pokerval>>12)&0xf)-2, plcomsuit)) ?
			 (1<<(((int)sym.pokerval>>12)&0xf)) : 0) +
			(CardMask_CARD_IS_SET(plcomCards, StdDeck_MAKE_CARD((((int)sym.pokerval>>8)&0xf)-2, plcomsuit)) ?
			 (1<<(((int)sym.pokerval>>8)&0xf)) : 0) +
			(CardMask_CARD_IS_SET(plcomCards, StdDeck_MAKE_CARD((((int)sym.pokerval>>4)&0xf)-2, plcomsuit)) ?
			 (1<<(((int)sym.pokerval>>4)&0xf)) : 0) +
			(CardMask_CARD_IS_SET(plcomCards, StdDeck_MAKE_CARD((((int)sym.pokerval>>0)&0xf)-2, plcomsuit)) ?
			 (1<<(((int)sym.pokerval>>0)&0xf)) : 0);
		sym.srankbitspoker += ((int)sym.srankbitspoker&0x4000) ? (1<<1) : 0; //ace
		for (i=14; i>=2; i--)
		{
			if ( (((int) sym.srankbitspoker)>>i) & 0x1)
			{
				sym.srankhipoker = i;													// srankhipoker
				i = 0;
			}
		}
		for (i=2; i<=14; i++)
		{
			if ( (((int) sym.srankbitspoker)>>i) & 0x1)
			{
				sym.sranklopoker = i;													// sranklopoker
				i = 15;
			}
		}

	LeaveCriticalSection(&cs_symbols);

	__SEH_LOGFATAL("CSymbols::calc_rankbits :\n");
}

void CSymbols::calc_history(void)
{
    __SEH_HEADER

    double maxbet;
    int i;

	EnterCriticalSection(&cs_symbols);	

		if (sym.nplayersround[(int) sym.br-1]==0)
		{
			sym.nplayersround[(int) sym.br-1] = sym.nplayersplaying;					// nplayersroundx
		}
		sym.nplayersround[4] = sym.nplayersround[(int) sym.br-1];						// nplayersround

		maxbet = 0;
		for (i=0; i<global.trans.map.num_chairs; i++)
		{
			if (sym.currentbet[i] > maxbet)
			{
				maxbet = sym.currentbet[i];
			}
		}
		maxbet /= (sym.bet[4]==0 ? 1 : sym.bet[4]);
		if (maxbet > sym.nbetsround[(int) sym.br-1])
		{
			sym.nbetsround[(int) sym.br-1] = maxbet;									// nbetsroundx
		}
		sym.nbetsround[4] = sym.nbetsround[(int) sym.br-1];								// nbetsround

	LeaveCriticalSection(&cs_symbols);

    __SEH_LOGFATAL("CSymbols::calc_history :\n");
}

void CSymbols::calc_statistics(void)
{
    __SEH_HEADER
    double	f$srai, C, R, S, B;
    int		error;


	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	double		playerbalance = scraper.playerbalance[sym_userchair];
	LeaveCriticalSection(&cs_scraper);

	
	// f$srai
    error = SUCCESS;
    f$srai = calc_f$symbol(&global.formula, "f$srai", &error);

    // B
    B = global.formula.dBankroll != 0 ? global.formula.dBankroll : playerbalance;

	EnterCriticalSection(&cs_symbols);	

		// call
		C = sym.call;
		R = 0;
		S = C+R;
		sym.callmean = (S==0) ? 0 : sym.prwin*((sym.pot+R)/S) + sym.prtie*((sym.pot+R)/2/S) - sym.prlos*(S/S);
		sym.callvariance = sym.prwin*pow(sym.pot/S - sym.callmean, 2) +
						   sym.prtie*pow(sym.pot/S/2 - sym.callmean, 2) +
						   sym.prlos*pow(-1 - sym.callmean, 2);
		sym.callror = pow(M_E, ( 2*sym.callmean*(0/S+sym.pot/S)/sym.callvariance ) - 1) /
					  pow(M_E, ( 2*sym.callmean*(B/S+sym.pot/S)/sym.callvariance ) - 1);


		// rais
		C = sym.call;
		R = sym.bet[4];
		S = C+R;
		sym.raismean = (S==0) ? 0 : sym.prwin*((sym.pot+R)/S) + sym.prtie*((sym.pot+R)/2/S) - sym.prlos*(S/S);
		sym.raisvariance = sym.prwin*pow(sym.pot/S - sym.callmean, 2) +
						   sym.prtie*pow(sym.pot/S/2 - sym.callmean, 2) +
						   sym.prlos*pow(-1 - sym.callmean, 2);
		sym.raisror = pow(M_E, ( 2*sym.raismean*(0/S+sym.pot/S)/sym.raisvariance ) - 1) /
					  pow(M_E, ( 2*sym.raismean*(B/S+sym.pot/S)/sym.raisvariance ) - 1);

		// srai
		C = sym.call;
		R = f$srai;
		S = C+R;
		sym.sraimean = (S==0) ? 0 : sym.prwin*((sym.pot+R)/S) + sym.prtie*((sym.pot+R)/2/S) - sym.prlos*(S/S);
		sym.sraivariance = sym.prwin*pow(sym.pot/S - sym.callmean, 2) +
						   sym.prtie*pow(sym.pot/S/2 - sym.callmean, 2) +
						   sym.prlos*pow(-1 - sym.callmean, 2);
		sym.srairor = pow(M_E, ( 2*sym.sraimean*(0/S+sym.pot/S)/sym.sraivariance ) - 1) /
					  pow(M_E, ( 2*sym.sraimean*(B/S+sym.pot/S)/sym.sraivariance ) - 1);

		// alli
		C = sym.call;
		R = sym.balance[10] - sym.call;
		S = C+R;
		sym.allimean = (S==0) ? 0 : sym.prwin*((sym.pot+R)/S) + sym.prtie*((sym.pot+R)/2/S) - sym.prlos*(S/S);
		sym.allivariance = sym.prwin*pow(sym.pot/S - sym.callmean, 2) +
						   sym.prtie*pow(sym.pot/S/2 - sym.callmean, 2) +
						   sym.prlos*pow(-1 - sym.callmean, 2);
		sym.alliror = pow(M_E, ( 2*sym.allimean*(0/S+sym.pot/S)/sym.allivariance ) - 1) /
					  pow(M_E, ( 2*sym.allimean*(B/S+sym.pot/S)/sym.allivariance ) - 1);

	LeaveCriticalSection(&cs_symbols);

	__SEH_LOGFATAL("CSymbols::calc_statistics :\n");
}

void CSymbols::calc_run_ron(void)
{
    __SEH_HEADER

    CRunRon			rr;

    rr.get_counts();

	EnterCriticalSection(&cs_symbols);

		sym.run$royfl = rr.run$royfl;
		sym.run$strfl = rr.run$strfl;
		sym.run$4kind = rr.run$4kind;
		sym.run$fullh = rr.run$fullh;
		sym.run$flush = rr.run$flush;
		sym.run$strai = rr.run$strai;
		sym.run$3kind = rr.run$3kind;
		sym.run$2pair = rr.run$2pair;
		sym.run$1pair = rr.run$1pair;
		sym.run$hcard = rr.run$hcard;
		sym.run$total = rr.run$royfl+rr.run$strfl+rr.run$4kind+rr.run$fullh+rr.run$flush+
						rr.run$strai+rr.run$3kind+rr.run$2pair+rr.run$1pair+rr.run$hcard;
		sym.run$prbest = rr.run_pokervalmax_count/sym.run$total;
		sym.run$pokervalmax = rr.run$pokervalmax;
		sym.run$clocks = rr.run$clocks;

		sym.ron$royfl = rr.ron$royfl;
		sym.ron$strfl = rr.ron$strfl;
		sym.ron$4kind = rr.ron$4kind;
		sym.ron$fullh = rr.ron$fullh;
		sym.ron$flush = rr.ron$flush;
		sym.ron$strai = rr.ron$strai;
		sym.ron$3kind = rr.ron$3kind;
		sym.ron$2pair = rr.ron$2pair;
		sym.ron$1pair = rr.ron$1pair;
		sym.ron$hcard = rr.ron$hcard;
		sym.ron$total = rr.ron$royfl+rr.ron$strfl+rr.ron$4kind+rr.ron$fullh+rr.ron$flush+
						rr.ron$strai+rr.ron$3kind+rr.ron$2pair+rr.ron$1pair+rr.ron$hcard;
		sym.ron$prbest = rr.ron_pokervalmax_count/sym.ron$total;
		sym.ron$pokervalmax = rr.ron$pokervalmax;
		sym.ron$clocks = rr.ron$clocks;


		//////////////////////////////////////////////////////////////
		// probabilities
		//sym.run$prnuts = totals[];
		//sym.ron$prnuts = /run$total;

	LeaveCriticalSection(&cs_symbols);

	__SEH_LOGFATAL("CSymbols::calc_run_ron :\n");
}


void CSymbols::get_cardstring(char *c, unsigned int c0, unsigned int c1)
{
    __SEH_HEADER

    char		card0[10], card1[10];

    // figure out the card string to search for
    if (StdDeck_RANK(c0) >= StdDeck_RANK(c1))
    {
        StdDeck_cardToString(c0, card0);
        StdDeck_cardToString(c1, card1);
    }
    else
    {
        StdDeck_cardToString(c1, card0);
        StdDeck_cardToString(c0, card1);
    }
    c[0] = card0[0];
    c[1] = card1[0];

    if (c[0] == c[1] || card0[1] != card1[1])
    {
        c[2] = 'o';
    }
    else 
	{
        c[2] = 's';
    }
    c[3]='\0';

    __SEH_LOGFATAL("CSymbols::get_cur_pl_cardstring :\n");
}

double CSymbols::calc_pokerval(HandVal hv, int n, double *pcb, int card0, int card1)
{
    __SEH_HEADER

    double			pv = 0;
    int				i, j, k, max, c, flush_suit; //Matrix 2008-06-28
    double			bits = 0;
    CardMask		Cards, heartsCards, diamondsCards, clubsCards, spadesCards, suittestCards;


	EnterCriticalSection(&cs_symbols);
	int		sym_userchair = (int) sym.userchair;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2], card_common[5];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	LeaveCriticalSection(&cs_scraper);


    // If we have a straight flush or flush, figure out the suit
    flush_suit = -1;
    if (HandVal_HANDTYPE(hv)==HandType_STFLUSH || HandVal_HANDTYPE(hv)==HandType_FLUSH)
    {
        // Set up some suit masks
        CardMask_RESET(heartsCards);
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_2, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_3, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_4, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_5, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_6, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_7, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_8, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_9, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_TEN, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_JACK, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_QUEEN, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_KING, Suit_HEARTS));
        CardMask_SET(heartsCards, StdDeck_MAKE_CARD(Rank_ACE, Suit_HEARTS));

        CardMask_RESET(diamondsCards);
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_2, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_3, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_4, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_5, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_6, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_7, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_8, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_9, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_TEN, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_JACK, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_QUEEN, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_KING, Suit_DIAMONDS));
        CardMask_SET(diamondsCards, StdDeck_MAKE_CARD(Rank_ACE, Suit_DIAMONDS));

        CardMask_RESET(spadesCards);
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_2, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_3, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_4, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_5, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_6, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_7, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_8, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_9, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_TEN, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_JACK, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_QUEEN, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_KING, Suit_SPADES));
        CardMask_SET(spadesCards, StdDeck_MAKE_CARD(Rank_ACE, Suit_SPADES));

        CardMask_RESET(clubsCards);
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_2, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_3, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_4, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_5, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_6, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_7, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_8, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_9, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_TEN, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_JACK, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_QUEEN, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_KING, Suit_CLUBS));
        CardMask_SET(clubsCards, StdDeck_MAKE_CARD(Rank_ACE, Suit_CLUBS));


		CardMask_RESET(Cards);
		for (i=0; i<=1; i++)
		{
			if (card_player[i] != CARD_BACK && card_player[i] != CARD_NOCARD)
			{
				CardMask_SET(Cards, card_player[i]);
			}
		}

		for (i=0; i<=4; i++)
		{
			if (card_common[i] != CARD_BACK && card_common[i] != CARD_NOCARD)
			{
				CardMask_SET(Cards, card_common[i]);
			}
		}

        max = 0;
        CardMask_AND(suittestCards, Cards, spadesCards);
        c = StdDeck_numCards(suittestCards);
        if ( c>max && c>0)
        {
            max = c;
            flush_suit = Suit_SPADES;
        }
        CardMask_AND(suittestCards, Cards, heartsCards);
        c = StdDeck_numCards(suittestCards);
        if ( c>max && c>0)
        {
            max = c;
            flush_suit = Suit_HEARTS;
        }
        CardMask_AND(suittestCards, Cards, diamondsCards);
        c = StdDeck_numCards(suittestCards);
        if ( c>max && c>0)
        {
            max = c;
            flush_suit = Suit_DIAMONDS;
        }
        CardMask_AND(suittestCards, Cards, clubsCards);
        c = StdDeck_numCards(suittestCards);
        if ( c>max && c>0)
        {
            max = c;
            flush_suit = Suit_CLUBS;
        }
    }

    // Now figure out the pokerval and pcbits
    switch HandVal_HANDTYPE(hv)
    {
    case HandType_STFLUSH:
		EnterCriticalSection(&cs_symbols);	
        pv += sym.straightflush;
		LeaveCriticalSection(&cs_symbols);

		pv += (HandVal_TOP_CARD(hv)+2-0)<<16;
        pv += (HandVal_TOP_CARD(hv)+2-1)<<12;
        pv += (HandVal_TOP_CARD(hv)+2-2)<<8;
        pv += (HandVal_TOP_CARD(hv)+2-3)<<4;
        pv += (HandVal_TOP_CARD(hv)+2-4)<<0;

        for (i=0; i<5; i++)
        {
			j = StdDeck_RANK(card0);	//Matrix 2008-06-28
			k = StdDeck_RANK(card1);
			if(j==12)j=-1;
			if(k==12)k=-1;
            if ( (j == HandVal_TOP_CARD(hv)-i &&
                    StdDeck_SUIT(card0) == flush_suit)
                    ||
                    k == HandVal_TOP_CARD(hv)-i &&
                    StdDeck_SUIT(card1) == flush_suit)
            {
                bits = (int) bits | (1<<(4-i));
            }
        }
        break;

    case HandType_QUADS:
		EnterCriticalSection(&cs_symbols);	
        pv += sym.fourofakind;
		LeaveCriticalSection(&cs_symbols);

        pv += (HandVal_TOP_CARD(hv)+2)<<16;
        pv += (HandVal_TOP_CARD(hv)+2)<<12;
        pv += (HandVal_TOP_CARD(hv)+2)<<8;
        pv += (HandVal_TOP_CARD(hv)+2)<<4;
        if (n>=5)  pv += (HandVal_SECOND_CARD(hv)+2)<<0;

        // Either hole card is used in quads
        if ( (StdDeck_RANK(card0) == HandVal_TOP_CARD(hv)) !=
                (StdDeck_RANK(card1) == HandVal_TOP_CARD(hv)) )
        {
            bits = (int) bits | (1<<4);
        }
        // Both hole cards are used in quads
        else if (StdDeck_RANK(card0) == HandVal_TOP_CARD(hv) &&
                 StdDeck_RANK(card1) == HandVal_TOP_CARD(hv))
        {
            bits = (int) bits | (1<<4);
            bits = (int) bits | (1<<3);
        }
        // Either hole card is used as kicker
        if (n>=5 &&
                StdDeck_RANK(card0) == HandVal_SECOND_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_SECOND_CARD(hv))
        {
            bits = (int) bits | (1<<0);
        }
        break;

    case HandType_FULLHOUSE:
		EnterCriticalSection(&cs_symbols);	
        pv += sym.fullhouse;
		LeaveCriticalSection(&cs_symbols);

        pv += (HandVal_TOP_CARD(hv)+2)<<16;
        pv += (HandVal_TOP_CARD(hv)+2)<<12;
        pv += (HandVal_TOP_CARD(hv)+2)<<8;
        pv += (HandVal_SECOND_CARD(hv)+2)<<4;
        pv += (HandVal_SECOND_CARD(hv)+2)<<0;

        // Either hole card is used in top of boat
        if ( (StdDeck_RANK(card0) == HandVal_TOP_CARD(hv)) !=
                (StdDeck_RANK(card1) == HandVal_TOP_CARD(hv)) )
        {
            bits = (int) bits | (1<<4);
        }
        // Both hole cards are used in top of boat
        else if (StdDeck_RANK(card0) == HandVal_TOP_CARD(hv) &&
                 StdDeck_RANK(card1) == HandVal_TOP_CARD(hv))
        {
            bits = (int) bits | (1<<4);
            bits = (int) bits | (1<<3);
        }
        // Either hole card is used in bottom of boat
        if ( (StdDeck_RANK(card0) == HandVal_SECOND_CARD(hv))  !=
                (StdDeck_RANK(card1) == HandVal_SECOND_CARD(hv)) )
        {
            bits = (int) bits | (1<<1);
        }
        // Both hole cards are used in bottom of boat
        else if (StdDeck_RANK(card0) == HandVal_SECOND_CARD(hv) &&
                 StdDeck_RANK(card1) == HandVal_SECOND_CARD(hv))
        {
            bits = (int) bits | (1<<1);
            bits = (int) bits | (1<<0);
        }
        break;

    case HandType_FLUSH:
		EnterCriticalSection(&cs_symbols);	
        pv += sym.flush;
		LeaveCriticalSection(&cs_symbols);

        pv += (HandVal_TOP_CARD(hv)+2)<<16;
        pv += (HandVal_SECOND_CARD(hv)+2)<<12;
        pv += (HandVal_THIRD_CARD(hv)+2)<<8;
        pv += (HandVal_FOURTH_CARD(hv)+2)<<4;
        pv += (HandVal_FIFTH_CARD(hv)+2)<<0;

        if ( (StdDeck_RANK(card0) == HandVal_TOP_CARD(hv) &&
                StdDeck_SUIT(card0) == flush_suit)
                ||
                (StdDeck_RANK(card1) == HandVal_TOP_CARD(hv) &&
                 StdDeck_SUIT(card1) == flush_suit) )
        {
            bits = (int) bits | (1<<4);
        }
        if ( (StdDeck_RANK(card0) == HandVal_SECOND_CARD(hv) &&
                StdDeck_SUIT(card0) == flush_suit)
                ||
                (StdDeck_RANK(card1) == HandVal_SECOND_CARD(hv) &&
                 StdDeck_SUIT(card1) == flush_suit) )
        {
            bits = (int) bits | (1<<3);
        }
        if ( (StdDeck_RANK(card0) == HandVal_THIRD_CARD(hv) &&
                StdDeck_SUIT(card0) == flush_suit)
                ||
                (StdDeck_RANK(card1) == HandVal_THIRD_CARD(hv) &&
                 StdDeck_SUIT(card1) == flush_suit) )
        {
            bits = (int) bits | (1<<2);
        }
        if ( (StdDeck_RANK(card0) == HandVal_FOURTH_CARD(hv) &&
                StdDeck_SUIT(card0) == flush_suit)
                ||
                (StdDeck_RANK(card1) == HandVal_FOURTH_CARD(hv) &&
                 StdDeck_SUIT(card1) == flush_suit) )
        {
            bits = (int) bits | (1<<1);
        }
        if ( (StdDeck_RANK(card0) == HandVal_FIFTH_CARD(hv) &&
                StdDeck_SUIT(card0) == flush_suit)
                ||
                (StdDeck_RANK(card1) == HandVal_FIFTH_CARD(hv) &&
                 StdDeck_SUIT(card1) == flush_suit) )
        {
            bits = (int) bits | (1<<0);
        }
        break;

    case HandType_STRAIGHT:
		EnterCriticalSection(&cs_symbols);	
        pv += sym.straight;
		LeaveCriticalSection(&cs_symbols);

        pv += (HandVal_TOP_CARD(hv)+2-0)<<16;
        pv += (HandVal_TOP_CARD(hv)+2-1)<<12;
        pv += (HandVal_TOP_CARD(hv)+2-2)<<8;
        pv += (HandVal_TOP_CARD(hv)+2-3)<<4;
        pv += (HandVal_TOP_CARD(hv)+2-4)<<0;
        for (i=0; i<5; i++)
        {
			j = StdDeck_RANK(card0);	//Matrix 2008-06-28
			k = StdDeck_RANK(card1);
			if(j==12)j=-1;
			if(k==12)k=-1;
            if (j == HandVal_TOP_CARD(hv)-i ||
                    k == HandVal_TOP_CARD(hv)-i)
            {
                bits = (int) bits | (1<<(4-i));
            }
        }
        break;

    case HandType_TRIPS:
		EnterCriticalSection(&cs_symbols);	
        pv += sym.threeofakind;
		LeaveCriticalSection(&cs_symbols);

        pv += (HandVal_TOP_CARD(hv)+2)<<16;
        pv += (HandVal_TOP_CARD(hv)+2)<<12;
        pv += (HandVal_TOP_CARD(hv)+2)<<8;
        if (n>=4)  pv += (HandVal_SECOND_CARD(hv)+2)<<4;
        if (n>=5)  pv += (HandVal_THIRD_CARD(hv)+2)<<0;

        // either hole card used in trips
        if ( (StdDeck_RANK(card0) == HandVal_TOP_CARD(hv)) !=
                (StdDeck_RANK(card1) == HandVal_TOP_CARD(hv)) )
        {
            bits = (int) bits | (1<<4);
        }
        // both hole cards used in trips
        else if (StdDeck_RANK(card0) == HandVal_TOP_CARD(hv) &&
                 StdDeck_RANK(card1) == HandVal_TOP_CARD(hv))
        {
            bits = (int) bits | (1<<4);
            bits = (int) bits | (1<<3);
        }
        // either hole card is first kicker
        if (n>=4 &&
                StdDeck_RANK(card0) == HandVal_SECOND_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_SECOND_CARD(hv) )
        {
            bits = (int) bits | (1<<1);
        }
        // either hole card is second kicker
        if (n>=5 &&
                StdDeck_RANK(card0) == HandVal_THIRD_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_THIRD_CARD(hv) )
        {
            bits = (int) bits | (1<<1);
        }
        break;

    case HandType_TWOPAIR:
		EnterCriticalSection(&cs_symbols);	
        pv += sym.twopair;
		LeaveCriticalSection(&cs_symbols);

        pv += (HandVal_TOP_CARD(hv)+2)<<16;
        pv += (HandVal_TOP_CARD(hv)+2)<<12;
        pv += (HandVal_SECOND_CARD(hv)+2)<<8;
        pv += (HandVal_SECOND_CARD(hv)+2)<<4;
        if (n>=5)  pv += (HandVal_THIRD_CARD(hv)+2)<<0;

        // either hole card used in top pair
        if ( (StdDeck_RANK(card0) == HandVal_TOP_CARD(hv)) !=
                (StdDeck_RANK(card1) == HandVal_TOP_CARD(hv)) )
        {
            bits = (int) bits | (1<<4);
        }
        // both hole cards used in top pair
        else if (StdDeck_RANK(card0) == HandVal_TOP_CARD(hv) &&
                 StdDeck_RANK(card1) == HandVal_TOP_CARD(hv))
        {
            bits = (int) bits | (1<<4);
            bits = (int) bits | (1<<3);
        }
        // either hole card used in bottom pair
        if ( (StdDeck_RANK(card0) == HandVal_SECOND_CARD(hv)) !=
                (StdDeck_RANK(card1) == HandVal_SECOND_CARD(hv)) )
        {
            bits = (int) bits | (1<<2);
        }
        // both hole cards used in bottom pair
        else if (StdDeck_RANK(card0) == HandVal_SECOND_CARD(hv) &&
                 StdDeck_RANK(card1) == HandVal_SECOND_CARD(hv))
        {
            bits = (int) bits | (1<<2);
            bits = (int) bits | (1<<1);
        }
        // either hole card kicker
        if (n>=5 &&
                StdDeck_RANK(card0) == HandVal_THIRD_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_THIRD_CARD(hv) )
        {
            bits = (int) bits | (1<<0);
        }
        break;

    case HandType_ONEPAIR:
 		EnterCriticalSection(&cs_symbols);	
		pv += sym.onepair;
 		LeaveCriticalSection(&cs_symbols);

		pv += (HandVal_TOP_CARD(hv)+2)<<16;
        pv += (HandVal_TOP_CARD(hv)+2)<<12;
        if (n>=3)  pv += (HandVal_SECOND_CARD(hv)+2)<<8;
        if (n>=4)  pv += (HandVal_THIRD_CARD(hv)+2)<<4;
        if (n>=5)  pv += (HandVal_FOURTH_CARD(hv)+2)<<0;

        // either hole card used in pair
        if ( (StdDeck_RANK(card0) == StdDeck_RANK(HandVal_TOP_CARD(hv))) !=
                (StdDeck_RANK(card1) == StdDeck_RANK(HandVal_TOP_CARD(hv))) )
        {
            bits = (int) bits | (1<<4);
        }
        // both hole cards used in pair
        else if (StdDeck_RANK(card0) == StdDeck_RANK(HandVal_TOP_CARD(hv)) &&
                 StdDeck_RANK(card1) == StdDeck_RANK(HandVal_TOP_CARD(hv)))
        {
            bits = (int) bits | (1<<4);
            bits = (int) bits | (1<<3);
        }
        // either hole card used as first kicker
        if (n>=3 &&
                StdDeck_RANK(card0) == HandVal_SECOND_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_SECOND_CARD(hv) )
        {
            bits = (int) bits | (1<<2);
        }
        // either hole card used as second kicker
        if (n>=4 &&
                StdDeck_RANK(card0) == HandVal_THIRD_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_THIRD_CARD(hv) )
        {
            bits = (int) bits | (1<<1);
        }
        // either hole card used as third kicker
        if (n>=5 &&
                StdDeck_RANK(card0) == HandVal_FOURTH_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_FOURTH_CARD(hv) )
        {
            bits = (int) bits | (1<<0);
        }
        break;

    case HandType_NOPAIR:
        if (n>=1)  pv += (HandVal_TOP_CARD(hv)+2)<<16;
        if (n>=2)  pv += (HandVal_SECOND_CARD(hv)+2)<<12;
        if (n>=3)  pv += (HandVal_THIRD_CARD(hv)+2)<<8;
        if (n>=4)  pv += (HandVal_FOURTH_CARD(hv)+2)<<4;
        if (n>=5)  pv += (HandVal_FIFTH_CARD(hv)+2)<<0;

        // either hole card used as top card
        if (n>=1 &&
                StdDeck_RANK(card0) == HandVal_TOP_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_TOP_CARD(hv) )
        {
            bits = (int) bits | (1<<4);
        }
        // either hole card used as first kicker
        if (n>=2 &&
                StdDeck_RANK(card0) == HandVal_SECOND_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_SECOND_CARD(hv) )
        {
            bits = (int) bits | (1<<3);
        }
        // either hole card used as second kicker
        if (n>=3 &&
                StdDeck_RANK(card0) == HandVal_THIRD_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_THIRD_CARD(hv) )
        {
            bits = (int) bits | (1<<2);
        }
        // either hole card used as third kicker
        if (n>=3 &&
                StdDeck_RANK(card0) == HandVal_FOURTH_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_FOURTH_CARD(hv) )
        {
            bits = (int) bits | (1<<1);
        }
        // either hole card used as fourth kicker
        if (n>=3 &&
                StdDeck_RANK(card0) == HandVal_FIFTH_CARD(hv) ||
                StdDeck_RANK(card1) == HandVal_FIFTH_CARD(hv) )
        {
            bits = (int) bits | (1<<0);
        }
        break;
    }

    *pcb = bits;
    return pv;

    __SEH_LOGFATAL("CSymbols::calc_pokerval :\n");
}

double CSymbols::GetSymbolVal(const char *a, int *e)
{
    __SEH_HEADER

	double		r = DBL_MAX;
    char		the_sym[50];
    int			round;

	// history symbols
    if (memcmp(a,"hi_",3)==0)
    {
        strcpy(the_sym, &a[3]);
        round = the_sym[strlen(the_sym)-1]-'0';
        the_sym[strlen(the_sym)-1] = '\0';
        return game_state.wh_sym_hist(the_sym, round);
    }

    // PokerTracker symbols
    if (memcmp(a,"pt_",3)==0 || memcmp(a,"ptt_",4)==0)					
		return PT.process_query(a);


	// All other symbols
	// These are broken up into groups to avoid compiler nesting limitations
	EnterCriticalSection(&cs_symbols);	
	
	if (r==DBL_MAX)
	{
		if (memcmp(a, "ismanual", 8)==0 && strlen(a)==8)						r=sym.ismanual;
		else if (memcmp(a, "isppro", 6)==0 && strlen(a)==6)						r=sym.isppro;
		else if (memcmp(a, "site", 4)==0 && strlen(a)==4)						r=sym.site;
		else if (memcmp(a, "nchairs", 7)==0 && strlen(a)==7)					r=sym.nchairs;
		else if (memcmp(a, "isbring", 7)==0 && strlen(a)==7)					r=sym.isbring;
		else if (memcmp(a, "session", 7)==0 && strlen(a)==7)					r=sym.session;
		else if (memcmp(a, "handnumber", 10)==0 && strlen(a)==10)				r=sym.handnumber;
		else if (memcmp(a, "version", 7)==0 && strlen(a)==7)					r=sym.version;

		//PROFILE
		else if (memcmp(a, "sitename$", 9)==0)									r=global.trans.map.sitename.Find(&a[9])!=-1;
		else if (memcmp(a, "network$", 8)==0)									r=global.trans.map.network.Find(&a[8])!=-1;
		else if (memcmp(a, "swagdelay", 9)==0 && strlen(a)==9)					r=sym.swagdelay;
		else if (memcmp(a, "allidelay", 9)==0 && strlen(a)==9)					r=sym.allidelay;
		else if (memcmp(a, "swagtextmethod", 14)==0 && strlen(a)==14)			r=sym.swagtextmethod;
		else if (memcmp(a, "potmethod", 9)==0 && strlen(a)==9)					r=sym.potmethod;
		else if (memcmp(a, "activemethod", 12)==0 && strlen(a)==12)				r=sym.activemethod;

		//FORMULA FILE
		else if (memcmp(a, "rake", 4)==0 && strlen(a)==4)						r=sym.rake;
		else if (memcmp(a, "nit", 3)==0 && strlen(a)==3)						r=sym.nit;
		else if (memcmp(a, "bankroll", 8)==0 && strlen(a)==8)					r=sym.bankroll;

		//LIMITS
		else if (memcmp(a, "bblind", 6)==0 && strlen(a)==6)						r=sym.bblind;
		else if (memcmp(a, "sblind", 6)==0 && strlen(a)==6)						r=sym.sblind;
		else if (memcmp(a, "ante", 4)==0 && strlen(a)==4)						r=sym.ante;
		else if (memcmp(a, "lim", 3)==0 && strlen(a)==3)						r=sym.lim;
		else if (memcmp(a, "isnl", 4)==0 && strlen(a)==4)						r=sym.isnl;
		else if (memcmp(a, "ispl", 4)==0 && strlen(a)==4)						r=sym.ispl;
		else if (memcmp(a, "isfl", 4)==0 && strlen(a)==4)						r=sym.isfl;
		else if (memcmp(a, "sraiprev", 8)==0 && strlen(a)==8)					r=sym.sraiprev;
		else if (memcmp(a, "sraimin", 7)==0 && strlen(a)==7)					r=sym.sraimin;
		else if (memcmp(a, "sraimax", 7)==0 && strlen(a)==7)					r=sym.sraimax;
		else if (memcmp(a, "istournament", 12)==0 && strlen(a)==12)				r=sym.istournament;

		//HAND RANK
		else if (memcmp(a, "handrank", 8)==0 && strlen(a)==8)					r=sym.handrank;
		else if (memcmp(a, "handrank169", 11)==0 && strlen(a)==11)				r=sym.handrank169;
		else if (memcmp(a, "handrank2652", 12)==0 && strlen(a)==12)				r=sym.handrank2652;
		else if (memcmp(a, "handrank1326", 12)==0 && strlen(a)==12)				r=sym.handrank1326;
		else if (memcmp(a, "handrank1000", 12)==0 && strlen(a)==12)				r=sym.handrank1000;
		else if (memcmp(a, "handrankp", 9)==0 && strlen(a)==9)					r=sym.handrankp;

		//CHAIRS
		else if (memcmp(a, "chair", 5)==0 && strlen(a)==5)						r=sym.chair;
		else if (memcmp(a, "userchair", 9)==0 && strlen(a)==9)					r=sym.userchair;
		else if (memcmp(a, "dealerchair", 11)==0 && strlen(a)==11)				r=sym.dealerchair;
		else if (memcmp(a, "raischair", 9)==0 && strlen(a)==9)					r=sym.raischair;
		else if (memcmp(a, "chair$", 6)==0)										r=Chair$(a);
		else if (memcmp(a, "chairbit$", 9)==0)									r=Chairbit$(a);

		//ROUND&POSITIONS
		else if (memcmp(a, "betround", 8)==0 && strlen(a)==8)					r=sym.betround;
		else if (memcmp(a, "br", 2)==0 && strlen(a)==2)							r=sym.br;
		else if (memcmp(a, "betposition", 11)==0 && strlen(a)==11)				r=sym.betposition;
		else if (memcmp(a, "dealposition", 12)==0 && strlen(a)==12)				r=sym.dealposition;
		else if (memcmp(a, "callposition", 12)==0 && strlen(a)==12)				r=sym.callposition;
		else if (memcmp(a, "seatposition", 12)==0 && strlen(a)==12)				r=sym.seatposition;
		else if (memcmp(a, "dealpositionrais", 16)==0 && strlen(a)==16)			r=sym.dealpositionrais;
		else if (memcmp(a, "betpositionrais", 15)==0 && strlen(a)==15)			r=sym.betpositionrais;
		else if (memcmp(a, "originaldealposition", 20)==0 && strlen(a)==20)		r=sym.originaldealposition; //Matrix 2008-05-09
	}

	LeaveCriticalSection(&cs_symbols);	

    //PROBABILITIES
	EnterCriticalSection(&cs_symbols);	
	
	if (r==DBL_MAX)
	{
		if (memcmp(a, "prwin", 5)==0 && strlen(a)==5)							r=sym.prwin;
		else if (memcmp(a, "prlos", 5)==0 && strlen(a)==5)						r=sym.prlos;
		else if (memcmp(a, "prtie", 5)==0 && strlen(a)==5)						r=sym.prtie;
		else if (memcmp(a, "random", 6)==0 && strlen(a)==6)						r=sym.random;
		else if (memcmp(a, "randomhand", 10)==0 && strlen(a)==10)				r=sym.randomhand;
		else if (memcmp(a, "randomround", 11)==0 && strlen(a)==11)				r=sym.randomround[4];
		else if (memcmp(a, "randomround", 11)==0 && strlen(a)==12)				r=sym.randomround[a[11]-'0'-1];

		//STATISTICS
		else if (memcmp(a, "callror", 7)==0 && strlen(a)==7)					r=sym.callror;
		else if (memcmp(a, "raisror", 7)==0 && strlen(a)==7)  					r=sym.raisror;
		else if (memcmp(a, "srairor", 7)==0 && strlen(a)==7)  					r=sym.srairor;
		else if (memcmp(a, "alliror", 7)==0 && strlen(a)==7)  					r=sym.alliror;
		else if (memcmp(a, "callmean", 8)==0 && strlen(a)==8)  					r=sym.callmean;
		else if (memcmp(a, "raismean", 8)==0 && strlen(a)==8)  					r=sym.raismean;
		else if (memcmp(a, "sraimean", 8)==0 && strlen(a)==8)  					r=sym.sraimean;
		else if (memcmp(a, "allimean", 8)==0 && strlen(a)==8)  					r=sym.allimean;
		else if (memcmp(a, "callvariance", 12)==0 && strlen(a)==12)				r=sym.callvariance;
		else if (memcmp(a, "raisvariance", 12)==0 && strlen(a)==12)  			r=sym.raisvariance;
		else if (memcmp(a, "sraivariance", 12)==0 && strlen(a)==12)  			r=sym.sraivariance;
		else if (memcmp(a, "allivariance", 12)==0 && strlen(a)==12)  			r=sym.allivariance;

		//P FORMULA
		else if (memcmp(a, "defcon", 6)==0 && strlen(a)==6)						r=sym.defcon;
		else if (memcmp(a, "isdefmode", 9)==0 && strlen(a)==9)					r=sym.isdefmode;
		else if (memcmp(a, "isaggmode", 9)==0 && strlen(a)==9)					r=sym.isaggmode;

		//CHIP AMOUNTS
		else if (memcmp(a, "balance", 7)==0 && strlen(a)==7)					r=sym.balance[10];
		else if (memcmp(a, "balance", 7)==0 && strlen(a)==8)					r=sym.balance[a[7]-'0'];
		else if (memcmp(a, "stack", 5)==0 && strlen(a)==6)						r=sym.stack[a[5]-'0'];
		else if (memcmp(a, "currentbet", 10)==0 && strlen(a)==10)				r=sym.currentbet[10];
		else if (memcmp(a, "currentbet", 10)==0 && strlen(a)==11)				r=sym.currentbet[a[10]-'0'];
		else if (memcmp(a, "call", 4)==0 && strlen(a)==4)						r=sym.call;
		else if (memcmp(a, "bet", 3)==0 && strlen(a)==3)						r=sym.bet[4];
		else if (memcmp(a, "bet", 3)==0 && strlen(a)==4)						r=sym.bet[a[3]-'0'-1];
		else if (memcmp(a, "pot", 3)==0 && strlen(a)==3)						r=sym.pot;
		else if (memcmp(a, "potcommon", 9)==0 && strlen(a)==9)					r=sym.potcommon;
		else if (memcmp(a, "potplayer", 9)==0 && strlen(a)==9)					r=sym.potplayer;
		else if (memcmp(a, "callshort", 9)==0 && strlen(a)==9)					r=sym.callshort;
		else if (memcmp(a, "raisshort", 9)==0 && strlen(a)==9)					r=sym.raisshort;

		//NUMBER OF BETS
		else if (memcmp(a, "nbetstocall", 11)==0 && strlen(a)==11)				r=sym.nbetstocall;
		else if (memcmp(a, "nbetstorais", 11)==0 && strlen(a)==11)				r=sym.nbetstorais;
		else if (memcmp(a, "ncurrentbets", 12)==0 && strlen(a)==12)				r=sym.ncurrentbets;
		else if (memcmp(a, "ncallbets", 9)==0 && strlen(a)==9)					r=sym.ncallbets;
		else if (memcmp(a, "nraisbets", 9)==0 && strlen(a)==9)					r=sym.nraisbets;

		//LIST TESTS
		else if (memcmp(a, "islistcall", 10)==0 && strlen(a)==10)				r=sym.islistcall;
		else if (memcmp(a, "islistrais", 10)==0 && strlen(a)==10)				r=sym.islistrais;
		else if (memcmp(a, "islistalli", 10)==0 && strlen(a)==10)				r=sym.islistalli;
		else if (memcmp(a, "islist", 6)==0 && (strlen(a)<10) && (atoi(a+6)<MAX_HAND_LISTS)) r=sym.islist[atoi(a+6)]; //Matrix 2008-05-14
		else if (memcmp(a, "isemptylistcall", 15)==0 && strlen(a)==15)			r=sym.isemptylistcall;
		else if (memcmp(a, "isemptylistrais", 15)==0 && strlen(a)==15)			r=sym.isemptylistrais;
		else if (memcmp(a, "isemptylistalli", 15)==0 && strlen(a)==15)			r=sym.isemptylistalli;

		//LIST NUMBERS
		else if (memcmp(a, "nlistmax", 8)==0 && strlen(a)==8)					r=sym.nlistmax;
		else if (memcmp(a, "nlistmin", 8)==0 && strlen(a)==8)					r=sym.nlistmin;

		//POKER VALUES
		else if (memcmp(a, "pokerval", 8)==0 && strlen(a)==8)					r=sym.pokerval;
		else if (memcmp(a, "pokervalplayer", 14)==0 && strlen(a)==14)			r=sym.pokervalplayer;
		else if (memcmp(a, "pokervalcommon", 14)==0 && strlen(a)==14)			r=sym.pokervalcommon;
		else if (memcmp(a, "pcbits", 6)==0 && strlen(a)==6)						r=sym.pcbits;
		else if (memcmp(a, "npcbits", 7)==0 && strlen(a)==7)					r=sym.npcbits;
	}
	LeaveCriticalSection(&cs_symbols);	

	//POKER VALUE CONSTANTS
	EnterCriticalSection(&cs_symbols);	
	
	if (r==DBL_MAX)
	{
		if (memcmp(a, "hicard", 6)==0 && strlen(a)==6)							r=sym.hicard;
		else if (memcmp(a, "onepair", 7)==0 && strlen(a)==7)					r=sym.onepair;
		else if (memcmp(a, "twopair", 7)==0 && strlen(a)==7)					r=sym.twopair;
		else if (memcmp(a, "threeofakind", 12)==0 && strlen(a)==12)				r=sym.threeofakind;
		else if (memcmp(a, "straight", 8)==0 && strlen(a)==8)					r=sym.straight;
		else if (memcmp(a, "flush", 5)==0 && strlen(a)==5)						r=sym.flush;
		else if (memcmp(a, "fullhouse", 9)==0 && strlen(a)==9)					r=sym.fullhouse;
		else if (memcmp(a, "fourofakind", 11)==0 && strlen(a)==11)				r=sym.fourofakind;
		else if (memcmp(a, "straightflush", 13)==0 && strlen(a)==13)			r=sym.straightflush;
		else if (memcmp(a, "royalflush", 10)==0 && strlen(a)==10)				r=sym.royalflush;
		else if (memcmp(a, "fiveofakind", 11)==0 && strlen(a)==11)				r=sym.fiveofakind;

		//HAND TESTS
		else if (memcmp(a, "$$pc", 4)==0)										r=sym.$$pc[a[4]-'0'];
		else if (memcmp(a, "$$pr", 4)==0)										r=sym.$$pr[a[4]-'0'];
		else if (memcmp(a, "$$ps", 4)==0)  										r=sym.$$ps[a[4]-'0'];
		else if (memcmp(a, "$$cc", 4)==0)  										r=sym.$$cc[a[4]-'0'];
		else if (memcmp(a, "$$cr", 4)==0)  										r=sym.$$cr[a[4]-'0'];
		else if (memcmp(a, "$$cs", 4)==0)  										r=sym.$$cs[a[4]-'0'];
		else if (memcmp(a, "$", 1)==0)  										r=IsHand(a, e);
		else if (memcmp(a, "ishandup", 8)==0 && strlen(a)==8)					r=sym.ishandup;
		else if (memcmp(a, "ishandupcommon", 14)==0 && strlen(a)==14)			r=sym.ishandupcommon;
		else if (memcmp(a, "ishicard", 8)==0 && strlen(a)==8)					r=sym.ishicard;
		else if (memcmp(a, "isonepair", 9)==0 && strlen(a)==9)					r=sym.isonepair;
		else if (memcmp(a, "istwopair", 9)==0 && strlen(a)==9)					r=sym.istwopair;
		else if (memcmp(a, "isthreeofakind", 14)==0 && strlen(a)==14)			r=sym.isthreeofakind;
		else if (memcmp(a, "isstraight", 10)==0 && strlen(a)==10)				r=sym.isstraight;
		else if (memcmp(a, "isflush", 7)==0 && strlen(a)==7)					r=sym.isflush;
		else if (memcmp(a, "isfullhouse", 11)==0 && strlen(a)==11)				r=sym.isfullhouse;
		else if (memcmp(a, "isfourofakind", 13)==0 && strlen(a)==13)			r=sym.isfourofakind;
		else if (memcmp(a, "isstraightflush", 15)==0 && strlen(a)==15)			r=sym.isstraightflush;
		else if (memcmp(a, "isroyalflush", 12)==0 && strlen(a)==12)				r=sym.isroyalflush;
		else if (memcmp(a, "isfiveofakind", 13)==0 && strlen(a)==13)			r=sym.isfiveofakind;

		//POCKET TESTS
		else if (memcmp(a, "ispair", 6)==0 && strlen(a)==6)						r=sym.ispair;
		else if (memcmp(a, "issuited", 8)==0 && strlen(a)==8)					r=sym.issuited;
		else if (memcmp(a, "isconnector", 11)==0 && strlen(a)==11)				r=sym.isconnector;

		//POCKET/COMMON TESTS
		else if (memcmp(a, "ishipair", 8)==0 && strlen(a)==8)					r=sym.ishipair;
		else if (memcmp(a, "islopair", 8)==0 && strlen(a)==8)					r=sym.islopair;
		else if (memcmp(a, "ismidpair", 9)==0 && strlen(a)==9)					r=sym.ismidpair;
		else if (memcmp(a, "ishistraight", 12)==0 && strlen(a)==12)				r=sym.ishistraight;
		else if (memcmp(a, "ishiflush", 9)==0 && strlen(a)==9)					r=sym.ishiflush;
	}
	LeaveCriticalSection(&cs_symbols);	

	//PLAYERS FRIENDS OPPONENTS
	EnterCriticalSection(&cs_symbols);	
	
	if (r==DBL_MAX)
	{
		if (memcmp(a, "nopponents", 10)==0 && strlen(a)==10)					r=sym.nopponents;
		else if (memcmp(a, "nopponentsmax", 13)==0 && strlen(a)==13)			r=sym.nopponentsmax;
		else if (memcmp(a, "nplayersseated", 14)==0 && strlen(a)==14)			r=sym.nplayersseated;
		else if (memcmp(a, "nplayersactive", 14)==0 && strlen(a)==14)			r=sym.nplayersactive;
		else if (memcmp(a, "nplayersdealt", 13)==0 && strlen(a)==13)			r=sym.nplayersdealt;
		else if (memcmp(a, "nplayersplaying", 15)==0 && strlen(a)==15)			r=sym.nplayersplaying;
		else if (memcmp(a, "nplayersblind", 13)==0 && strlen(a)==13)			r=sym.nplayersblind;
		else if (memcmp(a, "nfriendsseated", 14)==0 && strlen(a)==14)			r=sym.nfriendsseated;
		else if (memcmp(a, "nfriendsactive", 14)==0 && strlen(a)==14)			r=sym.nfriendsactive;
		else if (memcmp(a, "nfriendsdealt", 13)==0 && strlen(a)==13)			r=sym.nfriendsdealt;
		else if (memcmp(a, "nfriendsplaying", 15)==0 && strlen(a)==15)			r=sym.nfriendsplaying;
		else if (memcmp(a, "nfriendsblind", 13)==0 && strlen(a)==13)			r=sym.nfriendsblind;
		else if (memcmp(a, "nopponentsseated", 16)==0 && strlen(a)==16)			r=sym.nopponentsseated;
		else if (memcmp(a, "nopponentsactive", 16)==0 && strlen(a)==16)			r=sym.nopponentsactive;
		else if (memcmp(a, "nopponentsdealt", 15)==0 && strlen(a)==15)			r=sym.nopponentsdealt;
		else if (memcmp(a, "nopponentsplaying", 17)==0 && strlen(a)==17)		r=sym.nopponentsplaying;
		else if (memcmp(a, "nopponentsblind", 15)==0 && strlen(a)==15)			r=sym.nopponentsblind;
		else if (memcmp(a, "nopponentschecking", 18)==0 && strlen(a)==18)		r=sym.nopponentschecking;
		else if (memcmp(a, "nopponentscalling", 17)==0 && strlen(a)==17)		r=sym.nopponentscalling;
		else if (memcmp(a, "nopponentsraising", 17)==0 && strlen(a)==17)		r=sym.nopponentsraising;
		else if (memcmp(a, "nopponentsbetting", 17)==0 && strlen(a)==17)		r=sym.nopponentsbetting;
		else if (memcmp(a, "nopponentsfolded", 16)==0 && strlen(a)==16)			r=sym.nopponentsfolded;
		else if (memcmp(a, "nplayerscallshort", 17)==0 && strlen(a)==17)		r=sym.nplayerscallshort;
		else if (memcmp(a, "nchairsdealtright", 17)==0 && strlen(a)==17)		r=sym.nchairsdealtright;
		else if (memcmp(a, "nchairsdealtleft", 16)==0 && strlen(a)==16)			r=sym.nchairsdealtleft;
		else if (memcmp(a, "playersseatedbits", 17)==0 && strlen(a)==17)		r=sym.playersseatedbits;
		else if (memcmp(a, "playersactivebits", 17)==0 && strlen(a)==17)		r=sym.playersactivebits;
		else if (memcmp(a, "playersdealtbits", 16)==0 && strlen(a)==16)			r=sym.playersdealtbits;
		else if (memcmp(a, "playersplayingbits", 18)==0 && strlen(a)==18)		r=sym.playersplayingbits;
		else if (memcmp(a, "playersblindbits", 16)==0 && strlen(a)==16)			r=sym.playersblindbits;
		else if (memcmp(a, "opponentsseatedbits", 19)==0 && strlen(a)==19)		r=sym.opponentsseatedbits;
		else if (memcmp(a, "opponentsactivebits", 19)==0 && strlen(a)==19)		r=sym.opponentsactivebits;
		else if (memcmp(a, "opponentsdealtbits", 18)==0 && strlen(a)==18)		r=sym.opponentsdealtbits;
		else if (memcmp(a, "opponentsplayingbits", 20)==0 && strlen(a)==20)		r=sym.opponentsplayingbits;
		else if (memcmp(a, "opponentsblindbits", 18)==0 && strlen(a)==18)		r=sym.opponentsblindbits;
		else if (memcmp(a, "friendsseatedbits", 17)==0 && strlen(a)==17)		r=sym.friendsseatedbits;
		else if (memcmp(a, "friendsactivebits", 17)==0 && strlen(a)==17)		r=sym.friendsactivebits;
		else if (memcmp(a, "friendsdealtbits", 16)==0 && strlen(a)==16)			r=sym.friendsdealtbits;
		else if (memcmp(a, "friendsplayingbits", 18)==0 && strlen(a)==18)		r=sym.friendsplayingbits;
		else if (memcmp(a, "friendsblindbits", 16)==0 && strlen(a)==16)			r=sym.friendsblindbits;

		//FLAGS
		else if (memcmp(a, "fmax", 4)==0 && strlen(a)==4)						r=sym.fmax;
		else if (memcmp(a, "f", 1)==0 && strlen(a)==2)							r=sym.f[a[1]-'0'];
		else if (memcmp(a, "fbits", 5)==0 && strlen(a)==5)						r=sym.fbits;

		//COMMON CARDS
		else if (memcmp(a, "ncommoncardspresent", 19)==0 && strlen(a)==19)		r=sym.ncommoncardspresent;
		else if (memcmp(a, "ncommoncardsknown", 17)==0 && strlen(a)==17)		r=sym.ncommoncardsknown;
		else if (memcmp(a, "nflopc", 6)==0 && strlen(a)==6)						r=sym.nflopc;

		//(UN)KNOWN CARDS
		else if (memcmp(a, "nouts", 5)==0 && strlen(a)==5)						r=sym.nouts;
		else if (memcmp(a, "ncardsknown", 11)==0 && strlen(a)==11)				r=sym.ncardsknown;
		else if (memcmp(a, "ncardsunknown", 13)==0 && strlen(a)==13)			r=sym.ncardsunknown;
		else if (memcmp(a, "ncardsbetter", 12)==0 && strlen(a)==12)				r=sym.ncardsbetter;

		//NHANDS
		else if (memcmp(a, "nhands", 6)==0 && strlen(a)==6)						r=sym.nhands;
		else if (memcmp(a, "nhandshi", 8)==0 && strlen(a)==8)					r=sym.nhandshi;
		else if (memcmp(a, "nhandslo", 8)==0 && strlen(a)==8)					r=sym.nhandslo;
		else if (memcmp(a, "nhandsti", 8)==0 && strlen(a)==8)					r=sym.nhandsti;
		else if (memcmp(a, "prwinnow", 8)==0 && strlen(a)==8)					r=sym.prwinnow;
		else if (memcmp(a, "prlosnow", 8)==0 && strlen(a)==8)					r=sym.prlosnow;
	}
	LeaveCriticalSection(&cs_symbols);	

    //FLUSHES SETS STRAIGHTS
	EnterCriticalSection(&cs_symbols);	
	
	if (r==DBL_MAX)
	{
		if (memcmp(a, "nsuited", 7)==0 && strlen(a)==7)							r=sym.nsuited;
		else if (memcmp(a, "nsuitedcommon", 13)==0 && strlen(a)==13)			r=sym.nsuitedcommon;
		else if (memcmp(a, "tsuit", 5)==0 && strlen(a)==5)						r=sym.tsuit;
		else if (memcmp(a, "tsuitcommon", 11)==0 && strlen(a)==11)				r=sym.tsuitcommon;
		else if (memcmp(a, "nranked", 7)==0 && strlen(a)==7)					r=sym.nranked;
		else if (memcmp(a, "nrankedcommon", 13)==0 && strlen(a)==13)			r=sym.nrankedcommon;
		else if (memcmp(a, "trank", 5)==0 && strlen(a)==5)						r=sym.trank;
		else if (memcmp(a, "trankcommon", 11)==0 && strlen(a)==11)				r=sym.trankcommon;
		else if (memcmp(a, "nstraight", 9)==0 && strlen(a)==9)					r=sym.nstraight;
		else if (memcmp(a, "nstraightcommon", 15)==0 && strlen(a)==15)			r=sym.nstraightcommon;
		else if (memcmp(a, "nstraightfill", 13)==0 && strlen(a)==13)			r=sym.nstraightfill;
		else if (memcmp(a, "nstraightfillcommon", 19)==0 && strlen(a)==19)		r=sym.nstraightfillcommon;
		else if (memcmp(a, "nstraightflush", 14)==0 && strlen(a)==14)			r=sym.nstraightflush;
		else if (memcmp(a, "nstraightflushcommon", 20)==0 && strlen(a)==20)		r=sym.nstraightflushcommon;
		else if (memcmp(a, "nstraightflushfill", 18)==0 && strlen(a)==18)		r=sym.nstraightflushfill;
		else if (memcmp(a, "nstraightflushfillcommon", 24)==0 && strlen(a)==24) r=sym.nstraightflushfillcommon;

		//RANK BITS
		else if (memcmp(a, "rankbits", 8)==0 && strlen(a)==8)					r=sym.rankbits;
		else if (memcmp(a, "rankbitscommon", 14)==0 && strlen(a)==14)			r=sym.rankbitscommon;
		else if (memcmp(a, "rankbitsplayer", 14)==0 && strlen(a)==14)			r=sym.rankbitsplayer;
		else if (memcmp(a, "rankbitspoker", 13)==0 && strlen(a)==13)			r=sym.rankbitspoker;
		else if (memcmp(a, "srankbits", 9)==0 && strlen(a)==9)					r=sym.srankbits;
		else if (memcmp(a, "srankbitscommon", 15)==0 && strlen(a)==15)			r=sym.srankbitscommon;
		else if (memcmp(a, "srankbitsplayer", 15)==0 && strlen(a)==15)			r=sym.srankbitsplayer;
		else if (memcmp(a, "srankbitspoker", 14)==0 && strlen(a)==14)			r=sym.srankbitspoker;

		//RANK HI
		else if (memcmp(a, "rankhi", 6)==0 && strlen(a)==6)						r=sym.rankhi;
		else if (memcmp(a, "rankhicommon", 12)==0 && strlen(a)==12)				r=sym.rankhicommon;
		else if (memcmp(a, "rankhiplayer", 12)==0 && strlen(a)==12)				r=sym.rankhiplayer;
		else if (memcmp(a, "rankhipoker", 11)==0 && strlen(a)==11)				r=sym.rankhipoker;
		else if (memcmp(a, "srankhi", 7)==0 && strlen(a)==7)					r=sym.srankhi;
		else if (memcmp(a, "srankhicommon", 13)==0 && strlen(a)==13)			r=sym.srankhicommon;
		else if (memcmp(a, "srankhiplayer", 13)==0 && strlen(a)==13)			r=sym.srankhiplayer;
		else if (memcmp(a, "srankhipoker", 12)==0 && strlen(a)==12)				r=sym.srankhipoker;

		//RANK LO
		else if (memcmp(a, "ranklo", 6)==0 && strlen(a)==6)						r=sym.ranklo;
		else if (memcmp(a, "ranklocommon", 12)==0 && strlen(a)==12)				r=sym.ranklocommon;
		else if (memcmp(a, "rankloplayer", 12)==0 && strlen(a)==12)				r=sym.rankloplayer;
		else if (memcmp(a, "ranklopoker", 11)==0 && strlen(a)==11)				r=sym.ranklopoker;
		else if (memcmp(a, "sranklo", 7)==0 && strlen(a)==7)					r=sym.sranklo;
		else if (memcmp(a, "sranklocommon", 13)==0 && strlen(a)==13)			r=sym.sranklocommon;
		else if (memcmp(a, "srankloplayer", 13)==0 && strlen(a)==13)			r=sym.srankloplayer;
		else if (memcmp(a, "sranklopoker", 12)==0 && strlen(a)==12)				r=sym.sranklopoker;

		//TIME
		else if (memcmp(a, "elapsed", 7)==0 && strlen(a)==7)					r=sym.elapsed;
		else if (memcmp(a, "elapsedhand", 11)==0 && strlen(a)==11)				r=sym.elapsedhand;
		else if (memcmp(a, "elapsedauto", 11)==0 && strlen(a)==11)				r=sym.elapsedauto;
		else if (memcmp(a, "elapsedtoday", 12)==0 && strlen(a)==12)				r=sym.elapsedtoday;
		else if (memcmp(a, "elapsed1970", 11)==0 && strlen(a)==11)				r=sym.elapsed1970;
		else if (memcmp(a, "clocks", 6)==0 && strlen(a)==6)						r=sym.clocks;
		else if (memcmp(a, "nclockspersecond", 16)==0 && strlen(a)==16)			r=sym.nclockspersecond;
		else if (memcmp(a, "ncps", 4)==0 && strlen(a)==4)						r=sym.ncps;
	}
	LeaveCriticalSection(&cs_symbols);	

    //AUTOPLAYER
	EnterCriticalSection(&cs_symbols);	
	
	if (r==DBL_MAX)
	{
		if (memcmp(a, "myturnbits", 10)==0 && strlen(a)==10)					r=sym.myturnbits;
		else if (memcmp(a, "ismyturn", 8)==0 && strlen(a)==8)					r=sym.ismyturn;
		else if (memcmp(a, "issittingin", 11)==0 && strlen(a)==11)				r=sym.issittingin;
		else if (memcmp(a, "issittingout", 12)==0 && strlen(a)==12)				r=sym.issittingout;
		else if (memcmp(a, "isautopost", 10)==0 && strlen(a)==10)				r=sym.isautopost;
		else if (memcmp(a, "isfinalanswer", 13)==0 && strlen(a)==13)			r=sym.isfinalanswer;

		//HISTORY
		else if (memcmp(a, "nplayersround", 13)==0 && strlen(a)==13)			r=sym.nplayersround[4];
		else if (memcmp(a, "nplayersround", 13)==0 && strlen(a)==14)			r=sym.nplayersround[a[13]-'0'-1];
		else if (memcmp(a, "prevaction", 10)==0 && strlen(a)==10)				r=sym.prevaction;
		else if (memcmp(a, "didchec", 7)==0 && strlen(a)==7)					r=sym.didchec[4];
		else if (memcmp(a, "didcall", 7)==0 && strlen(a)==7)					r=sym.didcall[4];
		else if (memcmp(a, "didrais", 7)==0 && strlen(a)==7)					r=sym.didrais[4];
		else if (memcmp(a, "didswag", 7)==0 && strlen(a)==7)					r=sym.didswag[4];
		else if (memcmp(a, "nbetsround", 10)==0 && strlen(a)==10)				r=sym.nbetsround[4];
		else if (memcmp(a, "nbetsround", 10)==0 && strlen(a)==11)				r=sym.nbetsround[a[10]-'0'-1];
		else if (memcmp(a, "didchecround", 12)==0 && strlen(a)==13)				r=sym.didchec[a[12]-'0'-1];
		else if (memcmp(a, "didcallround", 12)==0 && strlen(a)==13)				r=sym.didcall[a[12]-'0'-1];
		else if (memcmp(a, "didraisround", 12)==0 && strlen(a)==13)				r=sym.didrais[a[12]-'0'-1];
		else if (memcmp(a, "didswaground", 12)==0 && strlen(a)==13)				r=sym.didswag[a[12]-'0'-1];

		//run$ ron$
		else if (memcmp(a, "ron$royfl", 9)==0 && strlen(a)==9)					r=sym.ron$royfl;
		else if (memcmp(a, "ron$strfl", 9)==0 && strlen(a)==9)					r=sym.ron$strfl;
		else if (memcmp(a, "ron$4kind", 9)==0 && strlen(a)==9)					r=sym.ron$4kind;
		else if (memcmp(a, "ron$fullh", 9)==0 && strlen(a)==9)					r=sym.ron$fullh;
		else if (memcmp(a, "ron$flush", 9)==0 && strlen(a)==9)					r=sym.ron$flush;
		else if (memcmp(a, "ron$strai", 9)==0 && strlen(a)==9)					r=sym.ron$strai;
		else if (memcmp(a, "ron$3kind", 9)==0 && strlen(a)==9)					r=sym.ron$3kind;
		else if (memcmp(a, "ron$2pair", 9)==0 && strlen(a)==9)					r=sym.ron$2pair;
		else if (memcmp(a, "ron$1pair", 9)==0 && strlen(a)==9)					r=sym.ron$1pair;
		else if (memcmp(a, "ron$hcard", 9)==0 && strlen(a)==9)					r=sym.ron$hcard;
		else if (memcmp(a, "ron$total", 9)==0 && strlen(a)==9)					r=sym.ron$total;
		else if (memcmp(a, "ron$pokervalmax", 15)==0 && strlen(a)==15)			r=sym.ron$pokervalmax;
		else if (memcmp(a, "ron$prnuts", 10)==0 && strlen(a)==10)				r=sym.ron$prnuts;
		else if (memcmp(a, "ron$prbest", 10)==0 && strlen(a)==10)				r=sym.ron$prbest;
		else if (memcmp(a, "ron$clocks", 10)==0 && strlen(a)==10)				r=sym.ron$clocks;
		else if (memcmp(a, "run$royfl", 9)==0 && strlen(a)==9)					r=sym.run$royfl;
		else if (memcmp(a, "run$strfl", 9)==0 && strlen(a)==9)  				r=sym.run$strfl;
		else if (memcmp(a, "run$4kind", 9)==0 && strlen(a)==9)  				r=sym.run$4kind;
		else if (memcmp(a, "run$fullh", 9)==0 && strlen(a)==9)  				r=sym.run$fullh;
		else if (memcmp(a, "run$flush", 9)==0 && strlen(a)==9)  				r=sym.run$flush;
		else if (memcmp(a, "run$strai", 9)==0 && strlen(a)==9)  				r=sym.run$strai;
		else if (memcmp(a, "run$3kind", 9)==0 && strlen(a)==9)  				r=sym.run$3kind;
		else if (memcmp(a, "run$2pair", 9)==0 && strlen(a)==9)  				r=sym.run$2pair;
		else if (memcmp(a, "run$1pair", 9)==0 && strlen(a)==9)  				r=sym.run$1pair;
		else if (memcmp(a, "run$hcard", 9)==0 && strlen(a)==9)  				r=sym.run$hcard;
		else if (memcmp(a, "run$total", 9)==0 && strlen(a)==9)  				r=sym.run$total;
		else if (memcmp(a, "run$pokervalmax", 15)==0 && strlen(a)==15)			r=sym.run$pokervalmax;
		else if (memcmp(a, "run$prnuts", 10)==0 && strlen(a)==10)				r=sym.run$prnuts;
		else if (memcmp(a, "run$prbest", 10)==0 && strlen(a)==10)				r=sym.run$prbest;
		else if (memcmp(a, "run$clocks", 10)==0 && strlen(a)==10)				r=sym.run$clocks;
	}
	LeaveCriticalSection(&cs_symbols);	

    // Versus
	EnterCriticalSection(&cs_symbols);	
	
	if (r==DBL_MAX)
	{
		if (memcmp(a, "vs$nhands", 10)==0 && strlen(a)==10)						r=sym.vs$nhands;
		else if (memcmp(a, "vs$nhandshi", 10)==0 && strlen(a)==10)				r=sym.vs$nhandshi;
		else if (memcmp(a, "vs$nhandsti", 10)==0 && strlen(a)==10)				r=sym.vs$nhandsti;
		else if (memcmp(a, "vs$nhandslo", 10)==0 && strlen(a)==10)				r=sym.vs$nhandslo;
		else if (memcmp(a, "vs$prwin", 8)==0 && strlen(a)==8)  					r=sym.vs$prwin;
		else if (memcmp(a, "vs$prtie", 8)==0 && strlen(a)==8)  					r=sym.vs$prtie;
		else if (memcmp(a, "vs$prlos", 8)==0 && strlen(a)==8)  					r=sym.vs$prlos;
		else if (memcmp(a, "vs$prwinhi", 10)==0 && strlen(a)==10)				r=sym.vs$prwinhi;
		else if (memcmp(a, "vs$prtiehi", 10)==0 && strlen(a)==10)  				r=sym.vs$prtiehi;
		else if (memcmp(a, "vs$prloshi", 10)==0 && strlen(a)==10)  				r=sym.vs$prloshi;
		else if (memcmp(a, "vs$prwinti", 10)==0 && strlen(a)==10)  				r=sym.vs$prwinti;
		else if (memcmp(a, "vs$prtieti", 10)==0 && strlen(a)==10)  				r=sym.vs$prtieti;
		else if (memcmp(a, "vs$prlosti", 10)==0 && strlen(a)==10)  				r=sym.vs$prlosti;
		else if (memcmp(a, "vs$prwinlo", 10)==0 && strlen(a)==10)  				r=sym.vs$prwinlo;
		else if (memcmp(a, "vs$prtielo", 10)==0 && strlen(a)==10)  				r=sym.vs$prtielo;
		else if (memcmp(a, "vs$prloslo", 10)==0 && strlen(a)==10)  				r=sym.vs$prloslo;

		// GameState symbols
		else if (memcmp(a,"lastraised",10)==0 && strlen(a)==11)  				r=game_state.lastraised(a[10]-'0');
		else if (memcmp(a,"raisbits",8)==0 && strlen(a)==9)  					r=game_state.raisbits(a[8]-'0');
		else if (memcmp(a,"callbits",8)==0 && strlen(a)==9)  					r=game_state.callbits(a[8]-'0');
		else if (memcmp(a,"foldbits",8)==0 && strlen(a)==9)  					r=game_state.foldbits(a[8]-'0');
		else if (memcmp(a,"oppdealt",8)==0 && strlen(a)==8)  					r=game_state.oppdealt;
		else if (memcmp(a,"floppct",7)==0 && strlen(a)==7)  					r=game_state.floppct();
		else if (memcmp(a,"turnpct",7)==0 && strlen(a)==7)  					r=game_state.turnpct();
		else if (memcmp(a,"riverpct",8)==0 && strlen(a)==8)  					r=game_state.riverpct();
		else if (memcmp(a,"avgbetspf",9)==0 && strlen(a)==9)  					r=game_state.avgbetspf();
		else if (memcmp(a,"tablepfr",8)==0 && strlen(a)==8)  					r=game_state.tablepfr();
		else if (memcmp(a,"maxbalance",10)==0 && strlen(a)==10)  				r=game_state.max_balance;
		else if (memcmp(a,"handsplayed",11)==0 && strlen(a)==11)  				r=game_state.hands_played;
		else if (memcmp(a,"balance_rank",12)==0 && strlen(a)==13)  				r=game_state.sortedbalance(a[12]-'0');
	}

	LeaveCriticalSection(&cs_symbols);

	if (r==DBL_MAX)
	{
		*e = ERR_INVALID_SYM;
		r = 0;
	}

    return r;

    __SEH_LOGFATAL("CSymbols::GetSymbolVal : <%s>\n", a);
}

double CSymbols::IsHand(const char *a, int *e)
{
    __SEH_HEADER

    int				cardrank[2] = {0}, temp;
    int				suited = 0;  //0=not specified, 1=suited, 2=offsuit
    int				i, cardcnt=0;
    int				plcardrank[2] = {0}, plsuited=0;


	EnterCriticalSection(&cs_symbols);
	int				sym_userchair = (int) sym.userchair;
	bool			ucc = user_chair_confirmed;
	LeaveCriticalSection(&cs_symbols);


	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2];
	card_player[0] = scraper.card_player[sym_userchair][0];
	card_player[1] = scraper.card_player[sym_userchair][1];
	LeaveCriticalSection(&cs_scraper);


    if (strlen(a)<=1)
	{
		if (e)
			*e = ERR_INVALID_SYM;
        return 0;
	}

    // passed in symbol query
    for (i=1; i<(int) strlen(a); i++)
    {
        if (a[i]>='2' && a[i]<='9')
            cardrank[cardcnt++] =  a[i] - '0';

        else if (a[i]=='T' || a[i]=='t')
            cardrank[cardcnt++] = 10;

        else if (a[i]=='J' || a[i]=='j')
            cardrank[cardcnt++] = 11;

        else if (a[i]=='Q' || a[i]=='q')
            cardrank[cardcnt++] = 12;

        else if (a[i]=='K' || a[i]=='k')
            cardrank[cardcnt++] = 13;

        else if (a[i]=='A' || a[i]=='a')
            cardrank[cardcnt++] = 14;

        else if (a[i]=='X' || a[i]=='x')
            cardrank[cardcnt++] = -1;

        else if (a[i]=='S' || a[i]=='s')
            suited=1;

        else if (a[i]=='O' || a[i]=='o')
            suited=2;

		else
		{
			if (e)
				*e = ERR_INVALID_SYM;
			return 0;
		}
    }

    if (!ucc)
        return 0;

	// sort
	if (cardrank[1] > cardrank[0])
    {
        temp = cardrank[0];
        cardrank[0] = cardrank[1];
        cardrank[1] = temp;
    }

	// playercards
	plcardrank[0] = Deck_RANK(card_player[0])+2;
	plcardrank[1] = Deck_RANK(card_player[1])+2;
	if (plcardrank[1] > plcardrank[0])
	{
		temp = plcardrank[0];
		plcardrank[0] = plcardrank[1];
		plcardrank[1] = temp;
	}
	if (Deck_SUIT(card_player[0]) == Deck_SUIT(card_player[1]))
	{
		plsuited = 1;
	}
	else
	{
		plsuited = 0;
	}

    // check for non suited/offsuit match
    if (suited==1 && plsuited==0)
        return 0;

    if (suited==2 && plsuited==1)
        return 0;

    // check for non rank match
    // two wildcards
    if (cardrank[0]==-1 && cardrank[1]==-1)
        return 1;

    // one card passed in, or one with a wildcard
    if (cardrank[1]==0 || cardrank[1]==-1)
    {
        if (cardrank[0] != plcardrank[0] &&
                cardrank[0] != plcardrank[1])
        {
            return 0;
        }
    }

    // two cards passed in
    else
    {
        if (cardrank[0]!=-1 && cardrank[0]!=plcardrank[0])
            return 0;

        if (cardrank[1]!=-1 && cardrank[1]!=plcardrank[1])
            return 0;
    }

    return 1;

    __SEH_LOGFATAL("CSymbols::IsHand : <%s>\n", a);
}

double CSymbols::Chair$(const char *a)
{
    __SEH_HEADER
    
	int i;

	EnterCriticalSection(&cs_scraper);

		for (i=0; i<global.trans.map.num_chairs; i++)
		{
			if (scraper.playername[i].Find(&a[6])!=-1)
			{
				LeaveCriticalSection(&cs_scraper);
				return i;
			}
		}

	LeaveCriticalSection(&cs_scraper);

    return -1;

	__SEH_LOGFATAL("CSymbols::Chair$ : <%s>\n", a);
}

double CSymbols::Chairbit$(const char *a)
{
    __SEH_HEADER

	int i, bits=0;

	EnterCriticalSection(&cs_scraper);

		for (i=0; i<global.trans.map.num_chairs; i++)
		{
			if (scraper.playername[i].Find(&a[9])!=-1)
			{
				LeaveCriticalSection(&cs_scraper);
				bits |= (1<<i);
			}
		}

	LeaveCriticalSection(&cs_scraper);

    return bits;

    __SEH_LOGFATAL("CSymbols::Chairbit$ : <%s>\n", a);
}