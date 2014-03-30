#include "StdAfx.h"
#include "debug.h"
#include "resource.h"
#include "grammar.h"
#include "registry.h"
#include "symbols.h"
#include "scraper.h"
#include "DialogFormulaScintilla.h"
#include "global.h"
#include "threads.h"

class CGlobal	global;


//  Sanity check: enough disk-space for a replay frame?
//    We assume, 10 MB are enough
//      - a large table of 1000x1000 pixels a 4 byte
//      - some KB for the HTML file.
//      - some space for other processes	
const unsigned int FREE_SPACE_NEEDED_FOR_REPLAYFRAME = 10000000;  


CGlobal::CGlobal(void)
{
    __SEH_SET_EXCEPTION_HANDLER(MyUnHandledExceptionFilter);

    __SEH_HEADER

    int			i, j;
	int			k, vndx; //Matrix 2008-05-08
    Registry	reg;
    FILE		*fp;

    //Initialisation for prwin handrank table
    extern char *prwhandrank169[169];
    extern int pair2ranko[170], pair2ranks[170];
    extern char ctonum[14];
    char *ptr;

    ClearFormula(&formula);
    formula_name = "";

    clear_r$indexes();

    trans.map.num_chairs = 10;

    attached_hwnd = NULL;

    for (i=0; i<10; i++) {
        flags[i] = false;
    }

    // Get saved preferences
    reg.read_reg();

    // analyzer
    preferences.max_opponents = reg.max_opponents;

    // autoplayer
    preferences.frame_delay = reg.frame_delay;
    preferences.swag_delay_1 = reg.swag_delay_1;
    preferences.swag_delay_2 = reg.swag_delay_2;
    preferences.swag_delay_3 = reg.swag_delay_3;
    preferences.text_selection_method = reg.text_selection_method;
    preferences.text_deletion_method = reg.text_deletion_method;
    preferences.bet_confirmation_method = reg.bet_confirmation_method;
    preferences.button_click_method = reg.button_click_method;
    preferences.ap_auto = reg.ap_auto;
    auto_autoplay_pressed = false;
    preferences.focus_detect = reg.focus_detect;

    // dll extension
    preferences.dll_always_send_state = reg.dll_always_send_state;
    preferences.load_dll_on_startup = reg.load_dll_on_startup;
    preferences.dll_name = reg.dll_name;

    // scraper
    preferences.scrape_delay = reg.scrape_delay;

    // symbols
    preferences.av_time = reg.avtime;
    preferences.handrank_value = reg.handrank_value;
    preferences.disable_caching = reg.disable_caching;

    // poker tracker
    // loaded in PT class

    // icm
    preferences.icm_prize1 = reg.icm_prize1;
    preferences.icm_prize2 = reg.icm_prize2;
    preferences.icm_prize3 = reg.icm_prize3;
    preferences.icm_prize4 = reg.icm_prize4;

    // Replay frames
    preferences.replay_record = reg.replay_record;
    preferences.replay_record_every_change = reg.replay_record_every_change;
    preferences.replay_max_frames = reg.replay_max_frames;

    //  2008.02.27 by THF
    //  Get saved preferences for Perl
    preferences.Perl_default_Formula = reg.Perl_default_Formula;
    preferences.Perl_Editor = reg.Perl_Editor;
    preferences.Perl_load_default_Formula = reg.Perl_load_default_Formula;
    preferences.Perl_load_Interpreter = reg.Perl_load_Interpreter;

    //  2008.02.27 by THF
    //  Get saved preferences for PokerChat
    preferences.Chat_enabled = reg.Chat_enabled;
    preferences.Chat_min_Delay = reg.Chat_min_Delay;
    preferences.Chat_random_Delay = reg.Chat_random_Delay;

    //  Get saved preferences for log$
    preferences.LogSymbol_enabled = reg.LogSymbol_enabled;
    preferences.LogSymbol_max_log = reg.LogSymbol_max_log;

    preferences.Trace_enabled = reg.Trace_enabled;
    memcpy(preferences.Trace_functions, reg.Trace_functions, sizeof(bool)*nTraceFunctions);

    // Check for versus.bin
    if ((fp = fopen("versus.bin", "rb"))!=NULL)
    {
        fclose(fp);
        versus_path = "versus.bin";
        versus_enabled = true;
    }
    else if ((fp = fopen(reg.versus_path, "rb"))!=NULL)
    {
        fclose(fp);
        versus_path = reg.versus_path;
        versus_enabled = true;
    }
    else
    {
        MessageBox(NULL, "Could not open versus.bin.\nVersus functions will be disabled.\n", "Versus Error", MB_OK | MB_TOPMOST);
        versus_enabled = false;

    }

    state_index = 0;

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
			symbols.prw1326.vanilla_chair.rankhi[vndx+11]=j+38; //s
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

    m_WaitCursor = false;

    replay_recorded_this_turn = false;

    __SEH_LOGFATAL("CGlobal::Constructor : \n");
}

CGlobal::~CGlobal(void)
{
    __SEH_HEADER

    __SEH_LOGFATAL("CGlobal::Destructor : \n");
}

void CGlobal::ClearFormula(SFormula *f)
{
    __SEH_HEADER

    f->dBankroll = f->dDefcon = f->dRake = f->dNit = 0.0;
    f->mHandList.RemoveAll();
    f->mFunction.RemoveAll();

    __SEH_LOGFATAL("CGlobal::ClearFormula : \n");
}


void CGlobal::CopyFormula(SFormula *f, SFormula *t)
{
    __SEH_HEADER

    SHandList		list;
    SFunction		func;
    int				from_count, to_count, from_iter, to_iter;
    bool			addit, deleteit;

    // handle deleted udfs
    to_count = (int) t->mFunction.GetSize();
    for (to_iter=0; to_iter<to_count; to_iter++)
    {
        from_count = (int) f->mFunction.GetSize();
        deleteit = true;
        for (from_iter=0; from_iter<from_count; from_iter++)
        {
            if (t->mFunction[to_iter].func == f->mFunction[from_iter].func)
            {
                deleteit = false;
                from_iter=from_count+1;
            }
        }
        if (deleteit)
        {
            t->mFunction.RemoveAt(to_iter, 1);
            to_count = (int) t->mFunction.GetSize();
            to_iter-=1;
        }
    }

    // handle new/changed udfs
    from_count = (int) f->mFunction.GetSize();
    for (from_iter=0; from_iter<from_count; from_iter++)
    {
        to_count = (int) t->mFunction.GetSize();
        addit = true;
        for (to_iter=0; to_iter<to_count; to_iter++)
        {
            if (f->mFunction[from_iter].func == t->mFunction[to_iter].func)
            {
                // changed?
                addit = false;
                if (f->mFunction[from_iter].func_text == t->mFunction[to_iter].func_text)
                {
                    // no change
                    t->mFunction[to_iter].dirty = false;
                    t->mFunction[to_iter].fresh = false;
                }
                else
                {
                    // yup, it changed
                    t->mFunction[to_iter].func_text = f->mFunction[from_iter].func_text;
                    t->mFunction[to_iter].dirty = true;
                    t->mFunction[to_iter].fresh = false;
                    t->mFunction[to_iter].cache = 0.0;
                }
                to_iter = to_count+1;
            }
        }

        // new
        if (addit)
        {
            func.func = f->mFunction[from_iter].func;
            func.func_text = f->mFunction[from_iter].func_text;
            func.dirty = true;
            func.fresh = false;
            func.cache = 0.0;
            t->mFunction.Add(func);
        }
    }

    // Copy numbers
    t->dBankroll = f->dBankroll;
    t->dDefcon = f->dDefcon;
    t->dRake = f->dRake;
    t->dNit = f->dNit;

    // Copy hand lists
    t->mHandList.RemoveAll();
    from_count = (int) f->mHandList.GetSize();
    for (from_iter=0; from_iter<from_count; from_iter++)
    {
        list.list = f->mHandList[from_iter].list;
        list.list_text = f->mHandList[from_iter].list_text;
        t->mHandList.Add(list);
    }

    __SEH_LOGFATAL("CGlobal::CopyFormula :\n");
}

// returns true for successful parse of all trees, false otherwise
bool CGlobal::ParseAllFormula(HWND hwnd, SFormula *f)
{
    __SEH_HEADER

    sData			data;
    data.all_parsed = true;
    data.calling_hwnd = hwnd;
    data.f = f;

    CUPDialog		dlg_progress(hwnd, parse_loop, &data, "Please wait", false);

    // Set busy cursor
    global.m_WaitCursor = true;
    CWnd::FromHandle(hwnd)->BeginWaitCursor();
    if (m_formulaScintillaDlg) m_formulaScintillaDlg->BeginWaitCursor();

    // Start parsing
    dlg_progress.DoModal();

    // Unset busy cursor
    CWnd::FromHandle(hwnd)->EndWaitCursor();
    if (m_formulaScintillaDlg) m_formulaScintillaDlg->EndWaitCursor();
    global.m_WaitCursor = false;

    return data.all_parsed;

    __SEH_LOGFATAL("CGlobal::ParseAllFormula :\n");
}

bool parse_loop(const CUPDUPDATA* pCUPDUPData)
{
    __SEH_HEADER

    int				N, i;
    CString			s;
    bool			result;
    int				stopchar;
    int				c, linenum, colnum;
    LARGE_INTEGER	bcount, ecount, lFrequency;
    double			time_elapsed;
    sData			*data = (sData*) (pCUPDUPData->GetAppData());

    pCUPDUPData->SetProgress("", 0, false);

    // init timer
    QueryPerformanceCounter(&bcount);
    QueryPerformanceFrequency(&lFrequency);

    N = (int) data->f->mFunction.GetSize();

    for (i=0; i<N; i++)
    {

        // Update progress dialog
        s.Format("Parsing formula set %s : %.0f%%", data->f->mFunction[i].func.GetString(), (double) i / (double) N * 100.0);
        QueryPerformanceCounter(&ecount);
        time_elapsed = ((double) (ecount.LowPart - bcount.LowPart))/((double) lFrequency.LowPart);
        pCUPDUPData->SetProgress(s.GetString(), (int) ((double) i / (double) N * 100.0), time_elapsed>=3.0);

        // Parse it if it is dirty, and not notes, dll or f$debug
        if (data->f->mFunction[i].dirty == true &&
                data->f->mFunction[i].func != "notes" &&
                data->f->mFunction[i].func != "dll" &&
                data->f->mFunction[i].func != "f$debug")
        {

            result = parse(&data->f->mFunction[i].func_text, &data->f->mFunction[i].tpi, &stopchar);
            if (!result)
            {
                linenum = colnum = 1;
                for (c=0; c<stopchar; c++)
                {
                    if (data->f->mFunction[i].func_text.Mid(c, 1)=="\n")
                    {
                        linenum++;
                        colnum = 1;
                    }
                    else
                    {
                        colnum++;
                    }
                }
                s.Format("Error in parse of %s\nLine: %d  Character: %d\n\nNear:\n \"%s\"",
                         data->f->mFunction[i].func.GetString(),
                         linenum, colnum,
                         data->f->mFunction[i].func_text.Mid(stopchar, 40).GetString());
                MessageBox(data->calling_hwnd, s, "PARSE ERROR", MB_OK);
                data->all_parsed = false;
            }
            else
            {
                data->f->mFunction[i].dirty = false;
            }
        }
    }

	pCUPDUPData->SetProgress("", 100, true);

    return true;

    __SEH_LOGFATAL("::parse_loop :\n");
}

int cardIdentHelper(const char c)
{
	__SEH_HEADER

    if (c>='2' && c<='9') {
        return c - '0' - 2;
    }
    else if (c=='T') {
        return 8;
    }
    else if (c=='J') {
        return 9;
    }
    else if (c=='Q') {
        return 10;
    }
    else if (c=='K') {
        return 11;
    }
    else if (c=='A') {
        return 12;
    }
    return -1;

	__SEH_LOGFATAL("cardIdentHelper :\n");
}

void CGlobal::ParseHandList(CString &list_text, bool inlist[13][13])
{
    __SEH_HEADER

    for (int i=0; i<=12; i++)
    {
        for (int j=0; j<=12; j++)
        {
            inlist[i][j] = false;
        }
    }

    int	token_card0_rank, token_card1_rank, temp_rank;

    CString list = list_text.MakeUpper();
    const char *pStr = list.GetString();
    while (*pStr)
    {
        if (pStr[0] == '/' && pStr[1] == '/')
        {
            int index = CString(pStr).FindOneOf("\r\n");
            if (index == -1) break;
            pStr += index;
        }

        token_card0_rank = cardIdentHelper(*pStr++);

        if (token_card0_rank == -1)
            continue;

        token_card1_rank = cardIdentHelper(*pStr++);

        if (token_card0_rank == -1)
            continue;

        // make card0 have the higher rank
        if (token_card0_rank < token_card1_rank)
        {
            temp_rank = token_card0_rank;
            token_card0_rank = token_card1_rank;
            token_card1_rank = temp_rank;
        }

        if (*pStr == 'S') // suited
        {
            inlist[token_card0_rank][token_card1_rank] = true;
            pStr++;
        }
        else  // offsuit or pair
        {
            inlist[token_card1_rank][token_card0_rank] = true;
        }
    }

    __SEH_LOGFATAL("ParseHandList :\n");
}

void CGlobal::create_hand_list_matrices(SFormula *f)
{
    __SEH_HEADER

    int			listnum, i, j, N;
    CString		token;

    for (listnum=0; listnum<MAX_HAND_LISTS; listnum++)
    {
        for (i=0; i<=12; i++)
        {
            for (j=0; j<=12; j++)
            {
                f->inlist[listnum][i][j] = false;
            }
        }
    }

    N = (int) f->mHandList.GetSize();
    for (i=0; i<N; i++)
    {
        listnum = atoi(f->mHandList[i].list.Mid(4).GetString());
		if(listnum>=MAX_HAND_LISTS)continue; // matrix 2008-05-14
        ParseHandList(f->mHandList[i].list_text, f->inlist[listnum]);
    }

    __SEH_LOGFATAL("CGlobal::create_hand_list_matrices :\n");
}

void CGlobal::capture_state(const char *title)
{
    __SEH_HEADER

    int						i, j;
    bool					playing = true;
    unsigned char			card;

    // figure out if I am playing
    if (!symbols.user_chair_confirmed)
    {
        playing = false;
    }
    else if (scraper.card_player[(int) symbols.sym.userchair][0] == CARD_BACK ||
             scraper.card_player[(int) symbols.sym.userchair][1] == CARD_BACK ||
             scraper.card_player[(int) symbols.sym.userchair][0] == CARD_NOCARD ||
             scraper.card_player[(int) symbols.sym.userchair][1] == CARD_NOCARD)
    {
        playing = false;
    }

    // When using MM, grab i5state for PT network
    if (symbols.sym.ismanual)
    {
        mm_network = scraper.buttonstate[5];
    }

    // Poker window title
    strncpy(global.state[global.state_index&0xff].m_title, title, 64);
    global.state[global.state_index&0xff].m_title[63] = '\0';

    // Pot information
    for (i=0; i<=9; i++)
        global.state[global.state_index&0xff].m_pot[i] = scraper.pot[i];

    // Common cards
    for (i=0; i<=4; i++)
    {
        if (scraper.card_common[i] == CARD_BACK)
        {
            card = WH_CARDBACK;
        }
        else if (scraper.card_common[i] == CARD_NOCARD)
        {
            card = WH_NOCARD;
        }
        else
        {
            card = ((StdDeck_RANK(scraper.card_common[i])+2)<<4) |
                   (StdDeck_SUIT(scraper.card_common[i]) == StdDeck_Suit_CLUBS ? WH_SUIT_CLUBS :
                    StdDeck_SUIT(scraper.card_common[i]) == StdDeck_Suit_DIAMONDS ? WH_SUIT_DIAMONDS :
                    StdDeck_SUIT(scraper.card_common[i]) == StdDeck_Suit_HEARTS ? WH_SUIT_HEARTS :
                    StdDeck_SUIT(scraper.card_common[i]) == StdDeck_Suit_SPADES ? WH_SUIT_SPADES : 0) ;
        }

        global.state[global.state_index&0xff].m_cards[i] = card;
    }

    // playing, posting, dealerchair
    global.state[global.state_index&0xff].m_is_playing = playing;
    global.state[global.state_index&0xff].m_is_posting = symbols.sym.isautopost;
    global.state[global.state_index&0xff].m_fillerbits = 0;
    global.state[global.state_index&0xff].m_fillerbyte = 0;
    global.state[global.state_index&0xff].m_dealer_chair = symbols.sym.dealerchair;

    // loop through all 10 player chairs
    for (i=0; i<=9; i++)
    {

        // player name, balance, currentbet
        strcpy(global.state[global.state_index&0xff].m_player[i].m_name, scraper.playername[i].GetString());
        global.state[global.state_index&0xff].m_player[i].m_balance = symbols.sym.balance[i];
        global.state[global.state_index&0xff].m_player[i].m_currentbet = symbols.sym.currentbet[i];

        // player cards
        for (j=0; j<=1; j++)
        {
            if (scraper.card_player[i][j] == CARD_BACK)
            {
                card = WH_CARDBACK;
            }
            else if (scraper.card_player[i][j] == CARD_NOCARD)
            {
                card = WH_NOCARD;
            }
            else
            {
                card = ((StdDeck_RANK(scraper.card_player[i][j])+2)<<4) |
                       (StdDeck_SUIT(scraper.card_player[i][j]) == StdDeck_Suit_CLUBS ? WH_SUIT_CLUBS :
                        StdDeck_SUIT(scraper.card_player[i][j]) == StdDeck_Suit_DIAMONDS ? WH_SUIT_DIAMONDS :
                        StdDeck_SUIT(scraper.card_player[i][j]) == StdDeck_Suit_HEARTS ? WH_SUIT_HEARTS :
                        StdDeck_SUIT(scraper.card_player[i][j]) == StdDeck_Suit_SPADES ? WH_SUIT_SPADES : 0) ;
            }

            global.state[global.state_index&0xff].m_player[i].m_cards[j] = card;
        }

        // player name known, balance known
        global.state[global.state_index&0xff].m_player[i].m_name_known = scraper.name_good_scrape[i] ? 1 : 0;
        global.state[global.state_index&0xff].m_player[i].m_balance_known = scraper.balance_good_scrape[i] ? 1 : 0;
        global.state[global.state_index&0xff].m_player[i].m_fillerbits = 0;
        global.state[global.state_index&0xff].m_player[i].m_fillerbyte = 0;
    }

    global.state_index++;

    __SEH_LOGFATAL("CGlobal::capture_state :\n");
}


void CGlobal::clear_r$indexes(void)
{
    __SEH_HEADER

    int i,j,k;


    for (i=0; i<=4; i++)
    {
        // common card info
        trans.map.r$c0cardfaceX_index[i] = -1;
        trans.map.r$c0cardfaceXrank_index[i] = -1;
        trans.map.r$c0cardfaceXsuit_index[i] = -1;

        // pots
        for (j=0; j<=9; j++)
            for (k=0; k<=9; k++)
                trans.map.r$c0potXchipYZ_index[i][j][k] = -1;
    }

    for (i=0; i<=9; i++)
    {
        // player info
        trans.map.r$pXcardfaceY_index[i][0] = -1;
        trans.map.r$pXcardfaceY_index[i][1] = -1;
        trans.map.r$pXcardfaceYrank_index[i][0] = -1;
        trans.map.r$pXcardfaceYrank_index[i][1] = -1;
        trans.map.r$pXcardfaceYsuit_index[i][0] = -1;
        trans.map.r$pXcardfaceYsuit_index[i][1] = -1;
        trans.map.r$uXcardfaceY_index[i][0] = -1;
        trans.map.r$uXcardfaceY_index[i][1] = -1;
        trans.map.r$pXcardback_index[i] = -1;
        trans.map.r$pXseated_index[i] = -1;
        trans.map.r$uXseated_index[i] = -1;
        trans.map.r$pXactive_index[i] = -1;
        trans.map.r$uXactive_index[i] = -1;
        trans.map.r$pXdealer_index[i] = -1;
        trans.map.r$pXname_index[i] = -1;
        trans.map.r$uXname_index[i] = -1;
        trans.map.r$pXbalance_index[i] = -1;
        trans.map.r$uXbalance_index[i] = -1;
        trans.map.r$pXbet_index[i] = -1;

        for (j=0; j<=9; j++)
            for (k=0; k<=9; k++)
                trans.map.r$pXchip_index[i][j][k] = -1;

        // button info
        trans.map.r$iXbutton_index[i] = -1;
        trans.map.r$iXstate_index[i] = -1;
        trans.map.r$iXlabel_index[i] = -1;

        for (j=0; j<=9; j++)
            trans.map.r$iXlabelY_index[i][j] = -1;

        trans.map.r$iXslider_index[i] = -1;
        trans.map.r$iXhandle_index[i] = -1;
        trans.map.r$iXedit_index[i] = -1;
        trans.map.r$i86Xstate_index[i] = -1;
        trans.map.r$i86Xbutton_index[i] = -1;

        // limits
        trans.map.r$c0limitsX_index[i] = -1;
        trans.map.r$c0handnumberX_index[i] = -1;

        // pots
        trans.map.r$c0potX_index[i] = -1;
    }

    // limits
    trans.map.r$c0limits_index = -1;
    trans.map.r$c0istournament_index = -1;
    trans.map.r$c0sblind_index = -1;
    trans.map.r$c0bblind_index = -1;
    trans.map.r$c0bigbet_index = -1;
    trans.map.r$c0ante_index = -1;
    trans.map.r$c0handnumber_index = -1;

    // player info
    trans.map.r$uname_index = -1;
    trans.map.r$ubalance_index = -1;

    // button info
    trans.map.r$i86state_index = -1;
    trans.map.r$i86button_index = -1;

    __SEH_LOGFATAL("CGlobal::clear_r$indexes :\n");
}

void CGlobal::save_r$indexes(void)
{
    __SEH_HEADER

    // r$tablepointX not indexed, as it is only used for finding tables on green circle-click, and
    //   this function is not called until a table has been selected by the user

    int		i, N;
    int		cardnum, seatnum, buttonnum, vertstride, horizstride, potnum, limitnum, handnum, indexnum;

    clear_r$indexes();

    N = trans.map.r$.GetCount();
    for (i=0; i<N; i++)
    {
        ///////////////////////////////////////////////////////////////////////////////////////////////
        // Player info
        // Player cards, r$pXcardfaceYrank, r$pXcardfaceYsuit
        if (trans.map.r$[i].name.Mid(0,1)=="p" &&
                trans.map.r$[i].name.Mid(2,8)=="cardface" &&
                trans.map.r$[i].name.Mid(11,4)=="rank")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            cardnum = trans.map.r$[i].name.GetString()[10] - '0';
            trans.map.r$pXcardfaceYrank_index[seatnum][cardnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="p" &&
                 trans.map.r$[i].name.Mid(2,8)=="cardface" &&
                 trans.map.r$[i].name.Mid(11,4)=="suit")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            cardnum = trans.map.r$[i].name.GetString()[10] - '0';
            trans.map.r$pXcardfaceYsuit_index[seatnum][cardnum] = i;
        }

        // Player cards, r$pXcardfaceY, r$uXcardfaceY
        else if (trans.map.r$[i].name.Mid(0,1)=="p" && trans.map.r$[i].name.Mid(2,8)=="cardface")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            cardnum = trans.map.r$[i].name.GetString()[10] - '0';
            trans.map.r$pXcardfaceY_index[seatnum][cardnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="u" && trans.map.r$[i].name.Mid(2,8)=="cardface")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            cardnum = trans.map.r$[i].name.GetString()[10] - '0';
            trans.map.r$uXcardfaceY_index[seatnum][cardnum] = i;
        }

        // Player card backs, r$pXcardback
        else if (trans.map.r$[i].name.Mid(0,1)=="p" && trans.map.r$[i].name.Mid(2,8)=="cardback")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$pXcardback_index[seatnum] = i;
        }

        // Seated, r$pXseated, r$uXseated
        else if (trans.map.r$[i].name.Mid(0,1)=="p" && trans.map.r$[i].name.Mid(2,6)=="seated")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$pXseated_index[seatnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="u" && trans.map.r$[i].name.Mid(2,6)=="seated")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$uXseated_index[seatnum] = i;
        }

        // Active, r$pXactive, r$uXactive
        else if (trans.map.r$[i].name.Mid(0,1)=="p" && trans.map.r$[i].name.Mid(2,6)=="active")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$pXactive_index[seatnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="u" && trans.map.r$[i].name.Mid(2,6)=="active")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$uXactive_index[seatnum] = i;
        }

        // Dealer button, r$pXdealer
        else if (trans.map.r$[i].name.Mid(0,1)=="p" && trans.map.r$[i].name.Mid(2,6)=="dealer")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$pXdealer_index[seatnum] = i;
        }

        // Player name r$pXname, r$uXname, r$uname
        else if (trans.map.r$[i].name.Mid(0,1)=="p" && trans.map.r$[i].name.Mid(2,4)=="name")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$pXname_index[seatnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="u" && trans.map.r$[i].name.Mid(2,4)=="name")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$uXname_index[seatnum] = i;
        }
        else if (trans.map.r$[i].name=="uname")
        {
            trans.map.r$uname_index = i;
        }

        // Player balance, r$pXbalance, r$uXbalance, r$ubalance
        else if (trans.map.r$[i].name.Mid(0,1)=="p" && trans.map.r$[i].name.Mid(2,7)=="balance")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$pXbalance_index[seatnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="u" && trans.map.r$[i].name.Mid(2,7)=="balance")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$uXbalance_index[seatnum] = i;
        }
        else if (trans.map.r$[i].name=="ubalance")
        {
            trans.map.r$ubalance_index = i;
        }

        // Player bet, r$pXbet, r$pXchipYZ
        else if (trans.map.r$[i].name.Mid(0,1)=="p" && trans.map.r$[i].name.Mid(2,3)=="bet")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$pXbet_index[seatnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="p" && trans.map.r$[i].name.Mid(2,4)=="chip")
        {
            seatnum = trans.map.r$[i].name.GetString()[1] - '0';
            vertstride = trans.map.r$[i].name.GetString()[6] - '0';
            horizstride = trans.map.r$[i].name.GetString()[7] - '0';
            trans.map.r$pXchip_index[seatnum][vertstride][horizstride] = i;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        // Common card info
        // Common cards, r$c0cardfaceXrank, r$c0cardfaceXsuit
        else if (trans.map.r$[i].name.Mid(0,10)=="c0cardface" && trans.map.r$[i].name.Mid(11,4)=="rank")
        {
            cardnum = trans.map.r$[i].name.GetString()[10] - '0';
            trans.map.r$c0cardfaceXrank_index[cardnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,10)=="c0cardface" && trans.map.r$[i].name.Mid(11,4)=="suit")
        {
            cardnum = trans.map.r$[i].name.GetString()[10] - '0';
            trans.map.r$c0cardfaceXsuit_index[cardnum] = i;
        }
        // Common cards, r$c0cardfaceX
        else if (trans.map.r$[i].name.Mid(0,10)=="c0cardface")
        {
            cardnum = trans.map.r$[i].name.GetString()[10] - '0';
            trans.map.r$c0cardfaceX_index[cardnum] = i;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        // Button info
        // r$iXbutton, r$iXstate, r$iXlabel, r$iXslider, r$iXhandle, r$iXedit, r$i86Xstate, r$i86Xbutton,
        // r$i86state, r$i86button
        else if (trans.map.r$[i].name.Mid(0,1)=="i" && trans.map.r$[i].name.Mid(2,6)=="button")
        {
            buttonnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$iXbutton_index[buttonnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="i" && trans.map.r$[i].name.Mid(2,5)=="state")
        {
            buttonnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$iXstate_index[buttonnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="i" && trans.map.r$[i].name.Mid(2,5)=="label" && trans.map.r$[i].name.GetLength()==7)
        {
            buttonnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$iXlabel_index[buttonnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="i" && trans.map.r$[i].name.Mid(2,5)=="label" && trans.map.r$[i].name.GetLength()==8)
        {
            buttonnum = trans.map.r$[i].name.GetString()[1] - '0';
            indexnum = trans.map.r$[i].name.GetString()[7] - '0';
            trans.map.r$iXlabelY_index[buttonnum][indexnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="i" && trans.map.r$[i].name.Mid(2,6)=="slider")
        {
            buttonnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$iXslider_index[buttonnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="i" && trans.map.r$[i].name.Mid(2,6)=="handle")
        {
            buttonnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$iXhandle_index[buttonnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,1)=="i" && trans.map.r$[i].name.Mid(2,4)=="edit")
        {
            buttonnum = trans.map.r$[i].name.GetString()[1] - '0';
            trans.map.r$iXedit_index[buttonnum] = i;
        }
        else if (trans.map.r$[i].name=="r$i86button")
        {
            trans.map.r$i86button_index = i;
        }
        else if (trans.map.r$[i].name.Mid(0,3)=="i86" && trans.map.r$[i].name.Mid(4,6)=="button")
        {
            buttonnum = trans.map.r$[i].name.GetString()[3] - '0';
            trans.map.r$i86Xbutton_index[buttonnum] = i;
        }
        else if (trans.map.r$[i].name=="r$i86state")
        {
            trans.map.r$i86state_index = i;
        }
        else if (trans.map.r$[i].name.Mid(0,3)=="i86" && trans.map.r$[i].name.Mid(4,5)=="state")
        {
            buttonnum = trans.map.r$[i].name.GetString()[3] - '0';
            trans.map.r$i86Xstate_index[buttonnum] = i;
        }


        ///////////////////////////////////////////////////////////////////////////////////////////////
        // Pots
        // Pots, r$c0potX, r$c0potXchipYZ
        else if (trans.map.r$[i].name.Mid(0,5)=="c0pot" && trans.map.r$[i].name.Find("chip")==-1)
        {
            potnum = trans.map.r$[i].name.GetString()[5] - '0';
            trans.map.r$c0potX_index[potnum] = i;
        }
        else if (trans.map.r$[i].name.Mid(0,5)=="c0pot" && trans.map.r$[i].name.Mid(6,4)=="chip")
        {
            potnum = trans.map.r$[i].name.GetString()[5] - '0';
            vertstride = trans.map.r$[i].name.GetString()[10] - '0';
            horizstride = trans.map.r$[i].name.GetString()[11] - '0';
            trans.map.r$c0potXchipYZ_index[potnum][vertstride][horizstride] = i;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////
        // Limits
        // r$c0limits, r$c0limitsX
        else if (trans.map.r$[i].name=="c0limits")
        {
            trans.map.r$c0limits_index = i;
        }
        else if (trans.map.r$[i].name.Mid(0,8)=="c0limits")
        {
            limitnum = trans.map.r$[i].name.GetString()[8] - '0';
            trans.map.r$c0limitsX_index[limitnum] = i;
        }

        // r$c0istournament
        else if (trans.map.r$[i].name == "c0istournament")
        {
            trans.map.r$c0istournament_index = i;
        }

        // r$c0sblind, r$c0bblind, r$c0bigbet, r$c0ante, r$c0handnumberX, r$c0handnumber
        else if (trans.map.r$[i].name == "c0sblind")
        {
            trans.map.r$c0sblind_index = i;
        }
        else if (trans.map.r$[i].name == "c0bblind")
        {
            trans.map.r$c0bblind_index = i;
        }
        else if (trans.map.r$[i].name == "c0bigbet")
        {
            trans.map.r$c0bigbet_index = i;
        }
        else if (trans.map.r$[i].name == "c0ante")
        {
            trans.map.r$c0ante_index = i;
        }
        else if (trans.map.r$[i].name=="c0handnumber")
        {
            trans.map.r$c0handnumber_index = i;
        }
        else if (trans.map.r$[i].name.Mid(0,12)=="c0handnumber")
        {
            handnum = trans.map.r$[i].name.GetString()[12] - '0';
            trans.map.r$c0handnumberX_index[handnum] = i;
        }

    }

    __SEH_LOGFATAL("CGlobal::save_r$indexes :\n");
}

void CGlobal::save_s$indexes(void)
{
    __SEH_HEADER

    // s$titletextX, s$!titletextX not indexed, as it is only used for finding tables on green circle-click, and
    //   this function is not called until a table has been selected by the user/*
    // s$hXtype are not indexed, as those records are ignored in OH

    int		i, num;

    // Clear 'em first
    for (i=0; i<=9; i++)
        trans.map.s$ttlimitsX_index[i] = -1;

    trans.map.s$ttlimits_index = -1;

    for (i=0; i<(int) trans.map.s$.GetSize(); i++)
    {

        // s$ttlimits, s$ttlimitsX
        if (trans.map.s$[i].name=="ttlimits")
        {
            trans.map.s$ttlimits_index = i;
        }
        else if (trans.map.s$[i].name.Mid(0,8)=="ttlimits")
        {
            num = trans.map.s$[i].name.GetString()[8] - '0';
            trans.map.s$ttlimitsX_index[num] = i;
        }
        else if (trans.map.s$[i].name=="c0limits")
        {
            trans.map.s$c0limits_index = i;
        }
        else if (trans.map.s$[i].name.Mid(0,8)=="c0limits")
        {
            num = trans.map.s$[i].name.GetString()[8] - '0';
            trans.map.s$c0limitsX_index[num] = i;
        }
    }

    __SEH_LOGFATAL("CGlobal::save_s$indexes :\n");
}

void CGlobal::save_s$strings(void)
{
    __SEH_HEADER

    // s$reseller and s$mechanic are not saved, as they are only comments and not used in OH for any purpose

    int		i;

    // Clear 'em first
    trans.map.num_chairs = 0;
    trans.map.swagtextmethod = 0;
    trans.map.potmethod = 0;
    trans.map.activemethod = 0;
    trans.map.sitename = "";
    trans.map.network = "";
    trans.map.chairconfig = "";
    for (i=0; i<=3; i++)
        trans.map.ttype[i] = "";

    for (i=0; i<(int) trans.map.s$.GetSize(); i++)
    {
        if (trans.map.s$[i].name == "nchairs")
            trans.map.num_chairs = strtoul(trans.map.s$[i].text.GetString(), NULL, 10);

        if (trans.map.s$[i].name == "swagtextmethod")
            trans.map.swagtextmethod = strtoul(trans.map.s$[i].text.GetString(), NULL, 10);

        if (trans.map.s$[i].name == "potmethod")
            trans.map.potmethod = strtoul(trans.map.s$[i].text.GetString(), NULL, 10);

        if (trans.map.s$[i].name == "activemethod")
            trans.map.activemethod = strtoul(trans.map.s$[i].text.GetString(), NULL, 10);

        if (trans.map.s$[i].name == "sitename")
            trans.map.sitename = trans.map.s$[i].text;

        if (trans.map.s$[i].name == "network")
            trans.map.network = trans.map.s$[i].text;

        if (trans.map.s$[i].name == "chairconfig")
            trans.map.chairconfig = trans.map.s$[i].text;

        if (trans.map.s$[i].name == "t0type")
            trans.map.ttype[0] = trans.map.s$[i].text;

        if (trans.map.s$[i].name == "t1type")
            trans.map.ttype[1] = trans.map.s$[i].text;

        if (trans.map.s$[i].name == "t2type")
            trans.map.ttype[2] = trans.map.s$[i].text;

        if (trans.map.s$[i].name == "t3type")
            trans.map.ttype[3] = trans.map.s$[i].text;

    }

    __SEH_LOGFATAL("CGlobal::save_s$strings :\n");
}


void CGlobal::create_replay_frame(void)
{
    __SEH_HEADER

    CString			path, filename, text, fcra_seen;
    FILE			*fp;
    int				i;
    time_t			ltime;
    struct tm		*now_time;
    char			now_time_str[100];
    bool			scrape_running;
	ULARGE_INTEGER free_bytes_for_user_on_disk, 
					total_bytes_on_disk, 
					free_bytes_total_on_disk;

	//  Sanity check: Enough disk-space for replay frame?	
	GetDiskFreeSpaceEx(
		startup_path,                  //  Directory on disk of interest
		&free_bytes_for_user_on_disk,  
		&total_bytes_on_disk,	
		&free_bytes_total_on_disk);
	if (free_bytes_for_user_on_disk.QuadPart < FREE_SPACE_NEEDED_FOR_REPLAYFRAME) 
	{
		MessageBox(NULL, "Not enough disk space to create replay-frame.", 
				"ERROR", 0);	
		return;
	}

    // Wait for scrape cycle to finish before saving frame
    scrape_running = true;
    i = 0;
    while (scrape_running && i<10)
    {
        EnterCriticalSection(&cs_updater);
        scrape_running = global.update_in_process;
        LeaveCriticalSection(&cs_updater);
        Sleep(50);
        i++;
    }

    // Get current time
    time(&ltime);
    now_time = localtime(&ltime);
    strftime(now_time_str, 100, "%Y-%m-%d %H:%M:%S", now_time);

    // Create replay/session dir if it does not exist
    path.Format("%s\\replay\\session%d\\", startup_path, sessionnum);
    if (GetFileAttributes(path.GetString()) == INVALID_FILE_ATTRIBUTES)
        SHCreateDirectoryEx(NULL, path.GetString(), NULL);

    // Create bitmap file
    path.Format("%s\\replay\\session%d\\frame%03d.bmp", startup_path, sessionnum, global.next_replay_frame);
    CreateBMPFile(path.GetString(), scraper.entire_window_Cur);

    // Create HTML file
    path.Format("%s\\replay\\session%d\\frame%03d.htm", startup_path, sessionnum, global.next_replay_frame);
    fp = fopen(path.GetString(), "w");

    fprintf(fp, scraper.title);
    fprintf(fp, "\n");
    fprintf(fp, "<html>\n");
    fprintf(fp, "<style>\n");
    fprintf(fp, "td {text-align:right;}\n");
    fprintf(fp, "</style>\n");
    fprintf(fp, "<body>\n");
    fprintf(fp, "<font face=courier>\n");
    fprintf(fp, "<img src=\"frame%03d.bmp\">\n", global.next_replay_frame);
    fprintf(fp, "<br>\n");
    fprintf(fp, "<a href=\"frame%03d.htm\">PREV</a>\n",
            global.next_replay_frame-1 >= 0 ? global.next_replay_frame-1 : global.preferences.replay_max_frames);
    fprintf(fp, "<a href=\"frame%03d.htm\">NEXT</a>\n",
            global.next_replay_frame+1 < global.preferences.replay_max_frames ? global.next_replay_frame+1 : 0);
    fprintf(fp, " [%d.%03d] [%s]<br>\n", sessionnum, global.next_replay_frame, now_time_str);
    fprintf(fp, "<br>\n");
    fprintf(fp, "<table>\n");
    fprintf(fp, "<tr>\n");
    fprintf(fp, "<td>\n");

    // Table for: SFABD, hand, bet, balance, name
    fprintf(fp, "<table border=4 cellpadding=1 cellspacing=1>\n");
    fprintf(fp, "<tr>\n");
    fprintf(fp, "<th>#</th>\n");
    fprintf(fp, "<th>SFABDP</th>\n");  //seated, friend, active, button, dealt, playing
    fprintf(fp, "<th>hand</th>\n");
    fprintf(fp, "<th>bet</th>\n");
    fprintf(fp, "<th>balance</th>\n");
    fprintf(fp, "<th>name</th>\n");
    fprintf(fp, "</tr>\n");
    for (i=0; i<global.trans.map.num_chairs; i++)
    {

        fprintf(fp, "<tr>\n");
        fprintf(fp, "<td>%d</td>", i);  // #
        text.Format("%s%s%s%s%s%s",
                    (int) (symbols.sym.playersseatedbits) & (1<<i) ? "s" : "-",
                    symbols.sym.userchair == i ? "f" : "-",
                    (int) (symbols.sym.playersactivebits) & (1<<i) ? "a" : "-",
                    symbols.sym.dealerchair== i ? "b" : "-",
                    (int) (symbols.sym.playersdealtbits) & (1<<i) ? "d" : "-",
                    (int) (symbols.sym.playersplayingbits) & (1<<i) ? "p" : "-");
        fprintf(fp, "<td>%s</td>", text.GetString());  // SFABDP
        fprintf(fp, "<td>%s%s</td>",
                get_card_html(scraper.card_player[i][0]),
                get_card_html(scraper.card_player[i][1]) );  // hand
        fprintf(fp, "<td>%11.2f</td>", scraper.playerbet[i]);  // bet
        fprintf(fp, "<td>%11.2f</td>", scraper.playerbalance[i]);  // balance
        fprintf(fp, "<td>%-15s</td>\n", scraper.playername[i].GetString());  // name
        fprintf(fp, "</tr>\n");
    }
    fprintf(fp, "</table>\n");
    fprintf(fp, "</td>\n");

    // Table for: FCRA
    fprintf(fp, "<td>\n");
    fprintf(fp, "<table align=center border=4 cellpadding=1 cellspacing=1>\n");
    fprintf(fp, "<tr><th>FCRA</th></tr>\n");
    fprintf(fp, "<tr>\n");
    fcra_seen.Format("%s%s%s%s",
                     ((int)symbols.sym.myturnbits)&0x1 ? "F" : ".",
                     ((int)symbols.sym.myturnbits)&0x2 ? "C" : ".",
                     ((int)symbols.sym.myturnbits)&0x4 ? "R" : ".",
                     ((int)symbols.sym.myturnbits)&0x8 ? "A" : ".");

    fprintf(fp, "<td>%s</td>\n", fcra_seen.GetString());
    fprintf(fp, "</tr>\n");
    fprintf(fp, "</table>\n");

    // Table for: sb, bb, BB
    fprintf(fp, "<table align=center border=4 cellpadding=1 cellspacing=1>\n");
    fprintf(fp, "<tr><th>sb</th><td>%11.2f</td></tr>\n", symbols.sym.sblind);
    fprintf(fp, "<tr><th>bb</th><td>%11.2f</td></tr>\n", symbols.sym.bblind);
    fprintf(fp, "<tr><th>BB</th><td>%11.2f</td></tr>\n", symbols.bigbet);
    fprintf(fp, "</table>\n");

    // Table for: common cards
    fprintf(fp, "<table align=center border=4 cellpadding=1 cellspacing=1>\n");
    fprintf(fp, "<tr><th>commoncard</th></tr>\n");
    fprintf(fp, "<tr>\n");
    fprintf(fp, "<td>%s%s%s%s%s</td>\n",
            get_card_html(scraper.card_common[0]),
            get_card_html(scraper.card_common[1]),
            get_card_html(scraper.card_common[2]),
            get_card_html(scraper.card_common[3]),
            get_card_html(scraper.card_common[4]) );
    fprintf(fp, "</tr>\n");
    fprintf(fp, "</table>\n");

    // Table for: pots
    fprintf(fp, "<table align=center border=4 cellpadding=1 cellspacing=1>\n");
    fprintf(fp, "<tr><th>#</th><th>pot</th></tr>\n");
    fprintf(fp, "<tr>\n");
    fprintf(fp, "<td>0</td><td>%11.2f</td>\n", scraper.pot[0]);
    fprintf(fp, "</tr>\n");
    for (i=1; i<=9; i++)
    {
        if (scraper.pot[i])
        {
            fprintf(fp, "<tr>\n");
            fprintf(fp, "<td>%d</td><td>%11.2f</td>\n", i, scraper.pot[i]);
            fprintf(fp, "</tr>\n");
        }
        else
        {
            i = 11;
        }
    }
    fprintf(fp, "</table>\n");

    fprintf(fp, "</td>\n");
    fprintf(fp, "</tr>\n");
    fprintf(fp, "</table>\n");
    fprintf(fp, "</body></html>\n");

    fclose(fp);

    // Increment counter
    global.next_replay_frame++;
    if (global.next_replay_frame >= global.preferences.replay_max_frames)
        global.next_replay_frame = 0;

    __SEH_LOGFATAL("CGlobal::create_replay_frame :\n");
}

CString CGlobal::get_card_html(unsigned int card)
{
    __SEH_HEADER

    CString suit, color, rank, final;

    suit =	card == CARD_BACK ? "*" :
           card == CARD_NOCARD ? "&nbsp" :
           StdDeck_SUIT(card) == Suit_CLUBS ? "&clubs;" :
           StdDeck_SUIT(card) == Suit_DIAMONDS ? "&diams;" :
           StdDeck_SUIT(card) == Suit_HEARTS ? "&hearts;" :
           StdDeck_SUIT(card) == Suit_SPADES ? "&spades;" :
           "&nbsp";

    color = card == CARD_BACK ? "black" :
            card == CARD_NOCARD ? "black" :
            StdDeck_SUIT(card) == Suit_CLUBS ? "green" :
            StdDeck_SUIT(card) == Suit_DIAMONDS ? "blue" :
            StdDeck_SUIT(card) == Suit_HEARTS ? "red" :
            StdDeck_SUIT(card) == Suit_SPADES ? "black" :
            "black";

    rank =	card == CARD_BACK ? "*" :
           card == CARD_NOCARD ? " " :
           StdDeck_RANK(card) == Rank_ACE ? "A" :
           StdDeck_RANK(card) == Rank_KING ? "K" :
           StdDeck_RANK(card) == Rank_QUEEN ? "Q" :
           StdDeck_RANK(card) == Rank_JACK ? "J" :
           StdDeck_RANK(card) == Rank_TEN ? "T" :
           StdDeck_RANK(card) == Rank_9 ? "9" :
           StdDeck_RANK(card) == Rank_8 ? "8" :
           StdDeck_RANK(card) == Rank_7 ? "7" :
           StdDeck_RANK(card) == Rank_6 ? "6" :
           StdDeck_RANK(card) == Rank_5 ? "5" :
           StdDeck_RANK(card) == Rank_4 ? "4" :
           StdDeck_RANK(card) == Rank_3 ? "3" :
           StdDeck_RANK(card) == Rank_2 ? "2" :
           "&nbsp";

    final.Format("<font color=%s>%s%s</font>", color, rank, suit);

    return final;

    __SEH_LOGFATAL("CGlobal::get_card_html :\n");
}