#include "stdafx.h"
#include <process.h>

#include "debug.h"
#include "scraper.h"
#include "global.h"
#include "inlines/eval.h"
#include "GameState.h"
#include "pokertracker.h"
#include "symbols.h"
#include "grammar.h"
#include "OpenHoldem.h"
#include "autoplayer.h"
#include "DialogScraperOutput.h"
#include "PokerPro.h"
#include "threads.h"

CWinThread			*h_heartbeat_thread=NULL;
bool				heartbeat_thread_alive = false;
CRITICAL_SECTION	cs_heartbeat;
double				heartbeat_cycle_time=0;

CWinThread			*h_prwin_thread=NULL;
bool				prwin_thread_alive = false;
bool				prwin_thread_complete = false;
CRITICAL_SECTION	cs_prwin;
unsigned int		prwin_iterator_progress=0;
double				prwin_run_with_nit=0;
double				prwin_run_with_f$p=0;
double				prwin_run_with_br;
unsigned int		prwin_run_with_playercards[2] = {CARD_NOCARD};
unsigned int		prwin_run_with_commoncards[5] = {CARD_NOCARD};

CRITICAL_SECTION	cs_updater;
CRITICAL_SECTION	cs_calc_f$symbol;

UINT __cdecl heartbeat_thread(LPVOID pParam)
{

    __SEH_SET_EXCEPTION_HANDLER(MyUnHandledExceptionFilter);


    __SEH_HEADER
    LARGE_INTEGER		cycle_start, cycle_end, lFrequency;
    Autoplayer			ap;
    unsigned int		new_scrape;
    bool				iswait;
    char				title[512];
    const char			*pbytes;
    int					nbytes, result;
    fd_set				fd;
    timeval				tv;
    bool				hbt=true;
    int					N, i;

    while (heartbeat_thread_alive)
    {
        // Measure cycle time
        QueryPerformanceCounter(&cycle_start);

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Prevent display updater from updating during scrape and symbol calc
        EnterCriticalSection(&cs_updater);
        global.update_in_process = true;
        LeaveCriticalSection(&cs_updater);

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Scrape window
        if (!global.ppro_is_connected)
        {
            new_scrape = scraper.DoScrape();
            QueryPerformanceCounter(&scraper.clockshold);
        }
        else
        {
            if (ppro.data.m_tinf.m_tid != 0)
            {
                ppro.DoScrape();
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Set Title of window
		CString *messageTitle = new CString();
        if (!global.ppro_is_connected)
        {
            GetWindowText(global.attached_hwnd, title, 512);
			messageTitle->Format("%s - %s (%s)", global.formula_name, global.trans.map.sitename, title);
        }
        else
        {
            if (ppro.data.m_tinf.m_tid == 0)
                _snprintf_s(title, _countof(title), _TRUNCATE, "%s", ppro.data.m_site_name);

            else if (ppro.data.m_userchair!=-1)
                _snprintf_s(title, _countof(title), _TRUNCATE, "%s - %s - %s", ppro.data.m_site_name, ppro.data.m_tinf.m_name, ppro.data.m_pinf[ppro.data.m_userchair].m_name);

            else
                _snprintf_s(title, _countof(title), _TRUNCATE, "%s - %s", ppro.data.m_site_name, ppro.data.m_tinf.m_name);
			messageTitle->Format("%s - %s (%s)", global.formula_name, global.trans.map.sitename, title);
        }
		theApp.m_pMainWnd->PostMessage(WMA_SETWINDOWTEXT, 0, (LPARAM)messageTitle);


        ////////////////////////////////////////////////////////////////////////////////////////////
        // Caclulate symbols

        // mark symbol result cache as stale
        N = (int) global.formula.mFunction.GetSize();
        for (i=0; i<N; i++)
        {
            global.formula.mFunction[i].fresh = false;
        }

        if (new_scrape!=NOTHING_CHANGED || (global.ppro_is_connected && ppro.data.m_tinf.m_tid != 0))
        {
            symbols.CalcSymbols();
        }
        else
        {
            symbols.calc_time();
            symbols.calc_probabilities();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // If we've folded, stop prwin thread and set prwin/tie/los to zero
        if (symbols.user_chair_confirmed &&
                (scraper.card_player[(int) symbols.sym.chair][0]==CARD_NOCARD ||
                 scraper.card_player[(int) symbols.sym.chair][1]==CARD_NOCARD ||
                 scraper.card_player[(int) symbols.sym.chair][0]==CARD_BACK ||
                 scraper.card_player[(int) symbols.sym.chair][1]==CARD_BACK))
        {
            EnterCriticalSection(&cs_prwin);
            prwin_thread_alive = false;
            symbols.sym.prwin = 0;
            symbols.sym.prlos = 0;
            symbols.sym.prtie = 0;
            prwin_iterator_progress = 0;
            prwin_run_with_nit = 0;
            prwin_run_with_f$p = 0;
            prwin_run_with_br = 0;
            LeaveCriticalSection(&cs_prwin);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Let display updater know it can update again
        EnterCriticalSection(&cs_updater);
        global.update_in_process = false;
        LeaveCriticalSection(&cs_updater);

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Create replay frame
        if (global.preferences.replay_record)
        {
            if (symbols.sym.ismyturn && !global.replay_recorded_this_turn ||
                    global.preferences.replay_record_every_change && new_scrape != NOTHING_CHANGED)
            {
                global.create_replay_frame();
                global.replay_recorded_this_turn = true;
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Start/stop PokerTracker thread as needed
        // Start Poker Tracker Thread if needed and not already running
        if (symbols.sym.issittingin && !pokertracker_thread_alive && !PT.disable)
        {
            pokertracker_thread_alive = true;
            h_pokertracker_thread = AfxBeginThread(pokertracker_thread, 0);

            write_log("Started Poker Tracker thread.\n");
        }

        // Stop Poker Tracker Thread if not needed any longer
        if (!symbols.sym.issittingin && pokertracker_thread_alive)
        {
            pokertracker_thread_alive = false;
            WaitForSingleObject(h_pokertracker_thread, THREAD_WAIT);
            write_log("Stopped Poker Tracker thread.\n");
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Update scraper output dialog if it is present
        if (m_ScraperOutputDlg)
            m_ScraperOutputDlg->do_update_display();

        // Measure cycle time
        QueryPerformanceCounter(&cycle_end);
        QueryPerformanceFrequency(&lFrequency);
        EnterCriticalSection(&cs_heartbeat);
        heartbeat_cycle_time = (double) (cycle_end.LowPart - cycle_start.LowPart)/(double) lFrequency.LowPart;
        LeaveCriticalSection(&cs_heartbeat);

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Save state
        global.capture_state(title);

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Game state engine
        game_state.process_game_state(&global.state[(global.state_index-1)&0xff]);
        game_state.process_ftr(&global.state[(global.state_index-1)&0xff]);

        ////////////////////////////////////////////////////////////////////////////////////////////
        // DLL - send state
        if ((scraper.card_player[(int) symbols.sym.userchair][0] != CARD_NOCARD &&
                scraper.card_player[(int) symbols.sym.userchair][1] != CARD_NOCARD &&
                symbols.user_chair_confirmed) ||
                global.preferences.dll_always_send_state)
        {
            cdll.pass_state_to_dll(&global.state[(global.state_index-1)&0xff]);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Autoplayer
        if (cdll.hMod_dll!=NULL && symbols.sym.ismyturn)
        {
            iswait = (cdll.process_message) ("query", "dll$iswait");
        }
        else
        {
            iswait = false;
        }

        if (symbols.user_chair_confirmed && global.autoplay && !iswait)
        {
            if (!global.ppro_is_connected)
                ap.do_autoplayer();

            else if (ppro.data.m_tinf.m_tid != 0)
                ppro.DoAutoplayer();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // scrape_delay, or wait for next ppro message
        if (global.ppro_is_connected)
        {
            // If there is anything on the socket, process it
            FD_ZERO(&fd);
            FD_SET(ppro.m_socket, &fd);
            tv.tv_usec = 50;
            tv.tv_sec = 0;

            result = 1;
            while (select(0, &fd, NULL, NULL, &tv)>0 && heartbeat_thread_alive)
            {
                pbytes = NULL;
                nbytes = 0;
                result = ppro.recv_message( &pbytes, &nbytes );
                if (result == 0)
                {
                    EnterCriticalSection(&cs_heartbeat);
                    heartbeat_thread_alive = false;
                    LeaveCriticalSection(&cs_heartbeat);
                }
                else
                {
                    ppro.handle_message(pbytes, nbytes);
                }
            }
        }
        global.ppro_isppro = ppro.m_socket!=INVALID_SOCKET ? 1 : 0;
        global.ppro_tid = ppro.data.m_tinf.m_tid;

        Sleep(global.preferences.scrape_delay);

        EnterCriticalSection(&cs_heartbeat);
        hbt = heartbeat_thread_alive;
        LeaveCriticalSection(&cs_heartbeat);

    }

    AfxEndThread(0);

    return 0;

    __SEH_LOGFATAL("::heartbeat_thread :\n");
}

int inrange(int card1, int card2)
{
//returns non-zero if card pair acceptable for prwin calculation

    /*
    p |---|    ---------
    r |   |   /         \
    o |   |  /           \
    b |   | /             \
    a |   |/               \
    b |                     \
    l -----------------------------------------------------
    e     1    2       3     4     handrank169 ->
    1=mustplay
    2=topclip
    3=willplay
    4=wontplay
    (with the current statistics available in OH only willplay and wontplay should be used.
     mustplay and topclip are debatable constructs at the best of times 2008-01-29)
    */

    extern int willplay,wontplay,topclip,mustplay;
    extern int pair2ranko[170],pair2ranks[170];
    int i,j,hrank;
    i=card1%13;
    j=card2%13;

    if (j>i) //normalise the card order
    {
        i=j;
        j=card1%13;
    }
    i=i*13+j; //offset into handrank table

    if (card1/13==card2/13)  //same suit
        hrank=pair2ranks[i]; //suited
    else
        hrank=pair2ranko[i]; //not suited

    if (hrank>=wontplay)return 0; //no good, never play these

    if (hrank<=mustplay) return 1; //OK, assumed opponent will always play these

    if ((hrank>topclip)&&(hrank<=willplay))return 1;//OK, in the 100% region

    //we now handle the probability slopes
    if (hrank<=topclip) //handle the good cards not likely to be one-betted with
    {
        j=topclip-rand()%topclip;
        if (hrank>j)return 1; //OK
        else return 0; //nogood
    }

    //now finish with poorish hands of reduced probability
    j=wontplay-willplay;
    if (j<3)
        return 1; //protect ourselves from effect of bad wontplay

    j=willplay+rand()%j;

    if (hrank<j)
        return 1; //OK

    return 0; //no good
}

UINT __cdecl prwin_thread(LPVOID pParam)
{

    __SEH_SET_EXCEPTION_HANDLER(MyUnHandledExceptionFilter);


    __SEH_HEADER

    int				i,j,k,randfix, nit = 0; //modified by Matrix 2008-05-08
    CardMask		plCards, comCards, addlcomCards, evalCards, usedCards, temp_usedCards;
    int				nplCards, ncomCards, nopp;
    extern int		willplay,wontplay,mustplay,topclip;
    int				error;
    unsigned int	ocard[MAX_OPPONENTS*2], card, pl_pokval, opp_pokval, opp_pokvalmax;
    double			win, tie, los;
    HandVal			pl_hv, opp_hv;
    double			dummy = 0;
    double			sym_nopp, sym_nit;
    unsigned int	deck[52], x, swap;
    int				numberOfCards;

    // Seed RNG
    srand((unsigned)time( NULL ));

    // player cards
    CardMask_RESET(plCards);
    CardMask_RESET(comCards);
    nplCards = ncomCards = 0;

    EnterCriticalSection(&cs_prwin);
    prwin_thread_complete = false;
    prwin_iterator_progress = 0;

    // player cards
    for (i=0; i<=1; i++)
    {
        if (scraper.card_player[(int) symbols.sym.userchair][i] != CARD_BACK &&
                scraper.card_player[(int) symbols.sym.userchair][i] != CARD_NOCARD)
        {
            CardMask_SET(plCards, scraper.card_player[(int) symbols.sym.userchair][i]);
            nplCards++;
        }
    }

    // common cards
    for (i=0; i<=4; i++)
    {
        if (scraper.card_common[i] != CARD_BACK &&
                scraper.card_common[i] != CARD_NOCARD)
        {
            CardMask_SET(comCards, scraper.card_common[i]);
            ncomCards++;
        }
    }

    // nopponents, nit
    sym_nopp = symbols.sym.nopponents;
    sym_nit = symbols.sym.nit;

    // save settings of this run
    prwin_run_with_f$p = sym_nopp;
    prwin_run_with_nit = sym_nit;
    prwin_run_with_br = symbols.sym.br;
    for (i=0; i<=1; i++)
        prwin_run_with_playercards[i] = scraper.card_player[(int) symbols.sym.userchair][i];

    for (i=0; i<=4; i++)
        prwin_run_with_commoncards[i] = scraper.card_common[i];

    LeaveCriticalSection(&cs_prwin);

    win = tie = los = 0;

    //Weighted prwin only for nopponents <=13
    error = SUCCESS;
    willplay = (int) calc_f$symbol(&global.formula, "f$willplay", &error);
    error = SUCCESS;
    wontplay = (int) calc_f$symbol(&global.formula, "f$wontplay", &error);
    error = SUCCESS;
    mustplay = (int) calc_f$symbol(&global.formula, "f$mustplay", &error);
    error = SUCCESS;
    topclip = (int) calc_f$symbol(&global.formula, "f$topclip", &error);

	if ((symbols.prw1326.useme==1326)&&(symbols.prw1326.usecallback==1326)&&((symbols.sym.br!=1)||(symbols.prw1326.preflop==1326)))
		(*symbols.prw1326.prw_callback)(); //Matrix 2008-05-09


    for (nit=0; nit < sym_nit && prwin_thread_alive; nit++)
    {

        nopp = sym_nopp < MAX_OPPONENTS ? sym_nopp : MAX_OPPONENTS;
        CardMask_OR(usedCards, plCards, comCards);
        if (willplay&&(willplay<nopp+1))willplay=nopp+1; //too low a value can give lockup
		if ((symbols.prw1326.useme==1326)&&((symbols.sym.br!=1)||(symbols.prw1326.preflop==1326)))
		{
			//prw1326 active  Matrix 2008-05-08
			k=nopp=0; //k is used as an index into ocard[] 
			for(i=0;i<10;i++) // loop through active opponents
			{
				if(i==symbols.sym.userchair)continue; //skip our own chair!
				if(!((int)symbols.sym.playersplayingbits & (1<<i)))continue; //skip inactive chairs
				nopp++; //we have to use actual opponents for prw1326 calculations
				//first deal with the special non-weighted cases
				//player who is marked 'ignore' or one who is BB and has not VPIP'd
				if(symbols.prw1326.chair[i].ignore || (symbols.prw1326.bblimp && (symbols.sym.nbetsround[0]<1.1) && ((int)symbols.sym.bblindbits&(1<<i))))
				{
                    do {
                        card = rand() & 63;
                    }
                    while (card>51 || CardMask_CARD_IS_SET(usedCards, card));
                    CardMask_SET(usedCards, card);
                    ocard[k++] = card;

                    do {
	                    card = rand() & 63;
		            }
			        while (card>51 || CardMask_CARD_IS_SET(usedCards, card));
				    CardMask_SET(usedCards, card);
                    ocard[k++] = card;
					continue;
				} // end of special non-weighted cases
				randfix=(RAND_MAX/symbols.prw1326.chair[i].limit)*symbols.prw1326.chair[i].limit;
				for(;;)
				{ //find a possible hand for this chair NOTE: may want to put in loop limits to prevent hanging
					do {
						j=rand();
					} while (j>=randfix);
					j=j%symbols.prw1326.chair[i].limit; //j is now any one of the allowed hands
					if(CardMask_CARD_IS_SET(usedCards,symbols.prw1326.chair[i].rankhi[j] ))continue; //hand contains dead card
					if(CardMask_CARD_IS_SET(usedCards,symbols.prw1326.chair[i].ranklo[j] ))continue; //hand contains dead card
//					if(symbols.prw1326.chair[i].ignore)break; //chair marked as not to be weighted
					if(symbols.prw1326.chair[i].level<=symbols.prw1326.chair[i].weight[j])break; //hand marked as always uae
					//check if we want a player who is BB and has not VPIP'd to be analysed further
//					if(symbols.prw1326.bblimp)
//					{
//				    if ((symbols.sym.nbetsround[0]<1.1) && ((int)symbols.sym.bblindbits&(1<<i)))break;
//					}
					//we should really do a 'randfix' here for the case where RAND_MAX is not an integral
					//multiple of .level, but the bias introduced is trivial compared to other uncertainties.
					if(rand()%symbols.prw1326.chair[i].level<symbols.prw1326.chair[i].weight[j])break; //allowable
					//if we reach here we will loop again to find a suitable hand
				} //end of possible hand find
				ocard[k++]=symbols.prw1326.chair[i].rankhi[j];
				ocard[k++]=symbols.prw1326.chair[i].ranklo[j];
				CardMask_SET(usedCards, ocard[k-2]);
				CardMask_SET(usedCards, ocard[k-1]);
			} //end of active opponent loop

			// additional common cards
            CardMask_RESET(addlcomCards);
	        for (i=0; i<5-ncomCards; i++)
		    {
			    do {
				    card = rand() & 63;
                }
	            while (card>51 ||CardMask_CARD_IS_SET(usedCards, card));
		        CardMask_SET(usedCards, card);
			    CardMask_SET(addlcomCards, card);
            }
		} //end of prw1326 code
		else
		{ //normal prwin opponent card selection
	      // if f$P<=13 then deal with random replacement algorithm,
		    // otherwise deal with swap algorithm
			if (nopp <= 13)
			{
	            // random replacement algorithm
	            // opponent cards
	            for (i=0; i<nopp*2; i+=2)
	            {
	                temp_usedCards=usedCards;
	                do
	                {
	                    usedCards=temp_usedCards; //reset the card mask to clear settings from failed card assignments

	                    do {
	                        card = rand() & 63;
	                    }
	                    while (card>51 || CardMask_CARD_IS_SET(usedCards, card));
	                    CardMask_SET(usedCards, card);
	                    ocard[i] = card;

	                    do {
		                    card = rand() & 63;
			            }
				        while (card>51 || CardMask_CARD_IS_SET(usedCards, card));
					    CardMask_SET(usedCards, card);
	                    ocard[i+1] = card;

		                if (!willplay)
			                break; //0 disables weighting

				        //put break for i=0 and opponent unraised BB case (cannot assume anything about his cards)
					    //In round 1 we should really do an analysis of chairs to find out how many have still to
	                    //place a bet. Not implemented since accuracy of prwin pre-flop is less critical.
		                if (!i)
			            {
				            //if we called then we are not BB, BB limped to flop,
					        //BB still playing, so do not weight his cards
						    if (symbols.sym.nbetsround[0]<1.1 &&
	                                symbols.sym.didcall[0] &&
		                            ((int)symbols.sym.playersplayingbits & (int)symbols.sym.bblindbits))
			                {
				                break;
					        }
						}
	                }
		            while (!inrange(ocard[i],ocard[i+1]));
			    }
				// additional common cards
	            CardMask_RESET(addlcomCards);
		        for (i=0; i<5-ncomCards; i++)
			    {
				    do {
					    card = rand() & 63;
	                }
		            while (card>51 ||CardMask_CARD_IS_SET(usedCards, card));
			        CardMask_SET(usedCards, card);
				    CardMask_SET(addlcomCards, card);
	            }
		    }
			else
	        {
		        // swap alogorithm
			    //weighted prwin not implemented for this case
	            numberOfCards=52;
		        for (i=0; i<numberOfCards; i++)  {
			        deck[i] = i;
				}
	            while (numberOfCards>=1)
		        {
			        x = rand() % numberOfCards;
				    numberOfCards--;
					if (x != numberOfCards)
	                {
		                swap = deck[x];
			            deck[x] = deck[numberOfCards];
				        deck[numberOfCards] = swap;
					}
	            }

		        // opponent cards
			    x = 0;
				for (i=0; i<nopp*2; i++)
	            {
		            while (CardMask_CARD_IS_SET(usedCards, deck[x]) && x<=51) {
			            x++;
				    }
					ocard[i] = deck[x++];
	            }

		        // additional common cards
			    CardMask_RESET(addlcomCards);
				for (i=0; i<5-ncomCards; i++)
				{
					while (CardMask_CARD_IS_SET(usedCards, deck[x]) && x<=51) {
	                    x++;
		            }
			        CardMask_SET(addlcomCards, deck[x++]);
				}
			}
		}

        // Get my handval/pokerval
        CardMask_OR(evalCards, plCards, comCards);
        CardMask_OR(evalCards, evalCards, addlcomCards);
        pl_hv = Hand_EVAL_N(evalCards, 7);
        pl_pokval = symbols.calc_pokerval(pl_hv, 7, &dummy, CARD_NOCARD, CARD_NOCARD);

        // Scan through opponents' handvals/pokervals
        // - if we find one better than ours, then we are done, increment los
        // - for win/tie, we need to wait until we scan them all
        opp_pokvalmax = 0;
        for (i=0; i<nopp; i++)
        {
            CardMask_OR(evalCards, comCards, addlcomCards);
            CardMask_SET(evalCards, ocard[i*2]);
            CardMask_SET(evalCards, ocard[(i*2)+1]);
            opp_hv = Hand_EVAL_N(evalCards, 7);
            opp_pokval = symbols.calc_pokerval(opp_hv, 7, &dummy, CARD_NOCARD, CARD_NOCARD);

            if (opp_pokval>pl_pokval)
            {
                los++;
                i = nopp+999;
            }
            else
            {
                if (opp_pokval > opp_pokvalmax)
                    opp_pokvalmax = opp_pokval;
            }
        }
        if (i!=nopp+1000)
        {
            if (pl_pokval > opp_pokvalmax)
                win++;
            else
                tie++;
        }

        // Matrix says "do we really want to update on each of first 1000 iterations?"
        // OpenHoldem answers: No we don't - good catch Matrix; added "&& nit>=1000" qualifier below
        if ((nit/1000 == (int) nit/1000) && nit>=1000)
        {
            EnterCriticalSection(&cs_prwin);
            prwin_iterator_progress = nit;
            symbols.sym.prwin = win / nit;
            symbols.sym.prtie = tie / nit;
            symbols.sym.prlos = los / nit;
            LeaveCriticalSection(&cs_prwin);
        }
    }

    EnterCriticalSection(&cs_prwin);
    if (nit >= sym_nit)
    {
        prwin_thread_complete = true;
        symbols.sym.prwin = win / sym_nit;
        symbols.sym.prtie = tie / sym_nit;
        symbols.sym.prlos = los / sym_nit;
        prwin_iterator_progress = nit;
    }
    else
    {
        prwin_thread_complete = false;
        symbols.sym.prwin = 0;
        symbols.sym.prtie = 0;
        symbols.sym.prlos = 0;
        prwin_iterator_progress = 0;
        prwin_run_with_nit = 0;
        prwin_run_with_f$p = 0;
        prwin_run_with_br = 0;
        for (i=0; i<=1; i++)
        {
            prwin_run_with_playercards[i] = CARD_NOCARD;
        }
        for (i=0; i<=4; i++)
        {
            prwin_run_with_commoncards[i] = CARD_NOCARD;
        }
    }
    prwin_thread_alive = false;
    LeaveCriticalSection(&cs_prwin);

    AfxEndThread(0);

    return 0;


    __SEH_LOGFATAL("::prwin_thread :\n");

}
