#include "stdafx.h"
#include "debug.h"
#include "global.h"
#include "symbols.h"
#include "scraper.h"
#include "IteratorThread.h"
#include "inlines/eval.h"

//#include <vld.h>			// visual leak detector



FILE *log_fp = NULL;

char * get_time(char * timebuf) 
{
    __SEH_HEADER
    // returns current system time in WH format
    time_t	ltime;
    char tmptime[30];

    time( &ltime );
    ctime_s(tmptime, 26, &ltime);
    tmptime[24]='\0';

    memcpy(timebuf, tmptime+20, 4); //yyyy
    *(timebuf+4) = '-';

    // mm
    if (memcmp(tmptime+4, "Jan", 3)==0)  {
        *(timebuf+5) = '0';
        *(timebuf+6) = '1';
    }
    else if (memcmp(tmptime+4, "Feb", 3)==0)  {
        *(timebuf+5) = '0';
        *(timebuf+6) = '2';
    }
    else if (memcmp(tmptime+4, "Mar", 3)==0)  {
        *(timebuf+5) = '0';
        *(timebuf+6) = '3';
    }
    else if (memcmp(tmptime+4, "Apr", 3)==0)  {
        *(timebuf+5) = '0';
        *(timebuf+6) = '4';
    }
    else if (memcmp(tmptime+4, "May", 3)==0)  {
        *(timebuf+5) = '0';
        *(timebuf+6) = '5';
    }
    else if (memcmp(tmptime+4, "Jun", 3)==0)  {
        *(timebuf+5) = '0';
        *(timebuf+6) = '6';
    }
    else if (memcmp(tmptime+4, "Jul", 3)==0)  {
        *(timebuf+5) = '0';
        *(timebuf+6) = '7';
    }
    else if (memcmp(tmptime+4, "Aug", 3)==0)  {
        *(timebuf+5) = '0';
        *(timebuf+6) = '8';
    }
    else if (memcmp(tmptime+4, "Sep", 3)==0)  {
        *(timebuf+5) = '0';
        *(timebuf+6) = '9';
    }
    else if (memcmp(tmptime+4, "Oct", 3)==0)  {
        *(timebuf+5) = '1';
        *(timebuf+6) = '0';
    }
    else if (memcmp(tmptime+4, "Nov", 3)==0)  {
        *(timebuf+5) = '1';
        *(timebuf+6) = '1';
    }
    else if (memcmp(tmptime+4, "Dec", 3)==0)  {
        *(timebuf+5) = '1';
        *(timebuf+6) = '2';
    }

    *(timebuf+7) = '-';
    memcpy(timebuf+8, tmptime+8, 2); //dd
    *(timebuf+10) = ' ';
    memcpy(timebuf+11, tmptime+11, 8); //HH:mm:ss
    *(timebuf+19) = '\0';

    return timebuf;

    __SEH_LOGFATAL("::get_time : \n");

}

char * get_now_time(char * timebuf) 
{
    __SEH_HEADER
    // returns current system time as a UNIX style string
    time_t	ltime;

    time( &ltime );
    ctime_s(timebuf, 26, &ltime);
    timebuf[24]='\0';
    return timebuf;

    __SEH_LOGFATAL("::get_now_time\n");

}

void logfatal (char* fmt, ...) 
{
    char		buff[10000] ;
    va_list		ap;
    char		fatallogpath[MAX_PATH];
    FILE		*fatallog;
    char		nowtime[26];

    sprintf(fatallogpath, "%s\\fatal error.log", global.startup_path);
    fatallog = fopen(fatallogpath, "a");

    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    fprintf(fatallog, "%s> %s", get_now_time(nowtime), buff);

    va_end(ap);
    fclose(fatallog);
}

LONG WINAPI MyUnHandledExceptionFilter(struct _EXCEPTION_POINTERS *lpExceptionInfo) 
{
    char flpath[MAX_PATH];
    char msg[1000];

    logfatal("########################################################################\n");
    logfatal("FATAL ERROR  (See above for call stack)\n");
    logfatal("########################################################################\n");
    logfatal("Address: %x\n", lpExceptionInfo->ExceptionRecord->ExceptionAddress);
    logfatal("Code: %x\n", lpExceptionInfo->ExceptionRecord->ExceptionCode);
    logfatal("Flags: %x\n", lpExceptionInfo->ExceptionRecord->ExceptionFlags);
    logfatal("Information: %x\n", lpExceptionInfo->ExceptionRecord->ExceptionInformation);
    logfatal("Record: %x\n", lpExceptionInfo->ExceptionRecord->ExceptionRecord);
    logfatal("Num Params: %x\n", lpExceptionInfo->ExceptionRecord->NumberParameters);
    switch (lpExceptionInfo->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
        logfatal("Desc: EXCEPTION_ACCESS_VIOLATION The thread tried to read from or write to a virtual address\n");
        logfatal("      for which it does not have the appropriate access.\n");
        break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        logfatal("Desc: EXCEPTION_ARRAY_BOUNDS_EXCEEDED The thread tried to access an array element that is out\n");
        logfatal("      of bounds and the underlying hardware supports bounds checking.\n");
        break;
    case EXCEPTION_BREAKPOINT:
        logfatal("Desc: EXCEPTION_BREAKPOINT A breakpoint was encountered.\n");
        break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        logfatal("Desc: EXCEPTION_DATATYPE_MISALIGNMENT The thread tried to read or write data that is misaligned on\n");
        logfatal("      hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte\n");
        logfatal("      boundaries; 32-bit values on 4-byte boundaries, and so on.\n");
        break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        logfatal("Desc: EXCEPTION_FLT_DENORMAL_OPERAND One of the operands in a floating-point operation is denormal. A\n");
        logfatal("      denormal value is one that is too small to represent as a standard floating-point value.\n");
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        logfatal("Desc: EXCEPTION_FLT_DIVIDE_BY_ZERO The thread tried to divide a floating-point value by a floating-point\n");
        logfatal("      divisor of zero.\n");
        break;
    case EXCEPTION_FLT_INEXACT_RESULT:
        logfatal("Desc: EXCEPTION_FLT_INEXACT_RESULT The result of a floating-point operation cannot be represented exactly\n");
        logfatal("      as a decimal fraction.\n");
        break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        logfatal("Desc: EXCEPTION_FLT_INVALID_OPERATION This exception represents any floating-point exception not included\n");
        logfatal("      in this list.\n");
        break;
    case EXCEPTION_FLT_OVERFLOW:
        logfatal("Desc: EXCEPTION_FLT_OVERFLOW The exponent of a floating-point operation is greater than the magnitude\n");
        logfatal("      allowed by the corresponding type.\n");
        break;
    case EXCEPTION_FLT_STACK_CHECK:
        logfatal("Desc: EXCEPTION_FLT_STACK_CHECK The stack overflowed or underflowed as the result of a floating-point\n");
        logfatal("      operation.\n");
        break;
    case EXCEPTION_FLT_UNDERFLOW:
        logfatal("Desc: EXCEPTION_FLT_UNDERFLOW The exponent of a floating-point operation is less than the magnitude\n");
        logfatal("      allowed by the corresponding type.\n");
        break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        logfatal("Desc: EXCEPTION_ILLEGAL_INSTRUCTION The thread tried to execute an invalid instruction.\n");
        break;
    case EXCEPTION_IN_PAGE_ERROR:
        logfatal("Desc: EXCEPTION_IN_PAGE_ERROR The thread tried to access a page that was not present, and the system\n");
        logfatal("      was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.\n");
        break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        logfatal("Desc: EXCEPTION_INT_DIVIDE_BY_ZERO The thread tried to divide an integer value by an integer divisor\n");
        logfatal("      of zero.\n");
        break;
    case EXCEPTION_INT_OVERFLOW:
        logfatal("Desc: EXCEPTION_INT_OVERFLOW The result of an integer operation caused a carry out of the most significant\n");
        logfatal("      bit of the result.\n");
        break;
    case EXCEPTION_INVALID_DISPOSITION:
        logfatal("Desc: EXCEPTION_INVALID_DISPOSITION An exception handler returned an invalid disposition to the exception\n");
        logfatal("      dispatcher. Programmers using a high-level language such as C should never encounter this exception.\n");
        break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        logfatal("Desc: EXCEPTION_NONCONTINUABLE_EXCEPTION The thread tried to continue execution after a noncontinuable\n");
        logfatal("      exception occurred.\n");
        break;
    case EXCEPTION_PRIV_INSTRUCTION:
        logfatal("Desc: EXCEPTION_PRIV_INSTRUCTION The thread tried to execute an instruction whose operation is not allowed\n");
        logfatal("      in the current machine mode.\n");
        break;
    case EXCEPTION_SINGLE_STEP:
        logfatal("Desc: EXCEPTION_SINGLE_STEP A trace trap or other single-instruction mechanism signaled that one\n");
        logfatal("      instruction has been executed.\n");
        break;
    case EXCEPTION_STACK_OVERFLOW:
        logfatal("Desc: EXCEPTION_STACK_OVERFLOW The thread used up its stack.\n");
        break;
    default:
        logfatal("Desc: Unknown.\n");
        break;
    }
    logfatal("########################################################################\n");
    logfatal("########################################################################\n\n\n");

    sprintf(flpath, "%s\\fatal error.log", global.startup_path);
    strcpy(msg, "OpenHoldem is about to crash - this is probably a developer's fault.\n");
    strcat(msg, "If you would be so kind, please help the file named:\n");
    strcat(msg, flpath);
    strcat(msg, " (in your OpenHoldem directory, or in c:\\)\n");
    strcat(msg, "find it's way to an OpenHoldem developer.\n");
    strcat(msg, "\n\nOpenHoldem will shut down when you click OK.");
    MessageBox(NULL, msg, "FATAL ERROR", MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST);

    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL CreateBMPFile(const char *szFile, HBITMAP hBMP) 
{
    __SEH_HEADER
    // Saves the hBitmap as a bitmap.
    HDC					hdcScreen = CreateDC("DISPLAY", NULL, NULL, NULL);
    HDC					hdcCompatible = CreateCompatibleDC(hdcScreen);
    PBITMAPINFO			pbmi=NULL;
    BOOL				bret=FALSE;
    HANDLE				hf=NULL; // file handle
    BITMAPFILEHEADER	hdr; // bitmap file-header
    PBITMAPINFOHEADER	pbih=NULL; // bitmap info-header
    LPBYTE				lpBits=NULL; // memory pointer
    DWORD				dwTotal=0; // total count of bytes
    DWORD				cb=0; // incremental count of bytes
    BYTE				*hp=NULL; // byte pointer
    DWORD				dwTmp=0;
    BITMAP				bmp;

    memset(&bmp,0,sizeof(BITMAP));
    GetObject(hBMP,sizeof(BITMAP),&bmp);
    memset(&hdr,0,sizeof(hdr));
    {
        int cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
        if (cClrBits>8) {
            // No Palette (normally)
            pbmi = (PBITMAPINFO) calloc(1, sizeof(BITMAPINFOHEADER));
        }
        else {
            pbmi = (PBITMAPINFO) calloc(1, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<<(min(8,cClrBits))));
            pbmi->bmiHeader.biClrUsed = (1<<cClrBits);
        }

        // Initialize the fields in the BITMAPINFO structure.
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

        // Retrieve the color table (RGBQUAD array) and the bits
        // (array of palette indices) from the DIB.
        if (!GetDIBits(hdcCompatible, hBMP, 0, bmp.bmHeight, NULL, pbmi, DIB_RGB_COLORS)) {
            goto to_return;
        }
    }
    pbih = &(pbmi->bmiHeader);
    pbmi->bmiHeader.biCompression=BI_RGB;
    lpBits = (LPBYTE) calloc(1, pbih->biSizeImage);

    if (!lpBits) {
        goto to_return;
    }

    if (!GetDIBits(hdcCompatible, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbmi, DIB_RGB_COLORS)) {
        goto to_return;
    }

    // Create the .BMP file.
    hf = CreateFile(szFile, GENERIC_READ | GENERIC_WRITE, (DWORD) 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
    if (hf == INVALID_HANDLE_VALUE) {
        goto to_return;
    }
    hdr.bfType = 0x4d42; // 0x42 = "B" 0x4d = "M"
    // Compute the size of the entire file.
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed*sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    // Compute the offset to the array of color indices.
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed*sizeof (RGBQUAD);

    // Copy the BITMAPFILEHEADER into the .BMP file.
    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER),	(LPDWORD) &dwTmp, NULL)) {
        goto to_return;
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof (RGBQUAD), (LPDWORD) &dwTmp, ( NULL))) {
        goto to_return;
    }

    // Copy the array of color indices into the .BMP file.
    dwTotal = cb = pbih->biSizeImage;
    hp = lpBits;
    if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)) {
        goto to_return;
    }

    // Close the .BMP file.
    if (!CloseHandle(hf)) {
        goto to_return;
    }
    bret=TRUE;


to_return:
    ;
    // Free memory.
    if (pbmi)free(pbmi);
    if (lpBits)free(lpBits);
    DeleteDC(hdcCompatible);
    DeleteDC(hdcScreen);

    return bret;

    __SEH_LOGFATAL("::CreateBMPFile\n");

}

void start_log(void) 
{
    __SEH_HEADER
    if (log_fp==NULL) 
	{
        CString fn;
        fn.Format("%s\\oh_%lu.log", global.startup_path, global.Session_ID);
        log_fp = fopen(fn.GetString(), "a");
        write_log("! log file open\n");
        fprintf(log_fp, "yyyy.mm.dd hh:mm:ss -  # hand commoncard rank poker  win  los  tie  P      nit bestaction - play*      call       bet       pot   balance - FCRA FCRA swag\n");
        fprintf(log_fp, "----------------------------------------------------------------------------------------------------------------------------------------------------------\n");
        fflush(log_fp);
    }

    __SEH_LOGFATAL("::start_log\n");

}

void write_log(char* fmt, ...) 
{
    __SEH_HEADER
    char		buff[10000] ;
    va_list		ap;
    char		nowtime[26];

    if (log_fp != NULL) 
	{

        va_start(ap, fmt);
        vsprintf(buff, fmt, ap);
        fprintf(log_fp, "%s - %s", get_time(nowtime), buff);

        va_end(ap);

        fflush(log_fp);
    }

    __SEH_LOGFATAL("::write_log\n");
}

void write_log_nostamp(char* fmt, ...) 
{
    __SEH_HEADER

	char		buff[10000] ;
    va_list		ap;

    if (log_fp != NULL) 
	{

        va_start(ap, fmt);
        vsprintf(buff, fmt, ap);
        fprintf(log_fp, "%s", buff);

        va_end(ap);

        fflush(log_fp);
    }

    __SEH_LOGFATAL("::write_log_nostamp\n");
}

void write_log_autoplay(const char * action) 
{
    __SEH_HEADER
    char		nowtime[26];
    CString		pcards, comcards, temp, rank, pokerhand, bestaction, fcra_seen;
    char		*card;
    CardMask	Cards;
    int			i, nCards;
    HandVal		hv;
    CString		fcra_formula_status;

	EnterCriticalSection(&cs_symbols);
	bool			user_chair_confirmed = symbols.user_chair_confirmed;
	int				sym_userchair = (int) symbols.sym.userchair;
	int				sym_br = (int) symbols.sym.br;
	int				sym_myturnbits = (int) symbols.sym.myturnbits;
	double			sym_handrank169 = symbols.sym.handrank169;
	double			sym_handrank1000 = symbols.sym.handrank1000;
	double			sym_handrank1326 = symbols.sym.handrank1326;
	double			sym_handrank2652 = symbols.sym.handrank2652;
	double			sym_handrankp = symbols.sym.handrankp;
	double			f_alli = symbols.f$alli;
	double			f_swag = symbols.f$swag;
	double			f_rais = symbols.f$rais;
	double			f_call = symbols.f$call;
	double			f_prefold = symbols.f$prefold;
	double			sym_prwin = symbols.sym.prwin;
	double			sym_prlos = symbols.sym.prlos;
	double			sym_prtie = symbols.sym.prtie;
	double			sym_call = symbols.sym.call;
	double			sym_bet_4 = symbols.sym.bet[4];
	double			sym_pot = symbols.sym.pot;
	double			sym_balance_10 = symbols.sym.balance[10];
	int				sym_nopponents = (int) symbols.sym.nopponents;
	int				sym_nit = (int) symbols.sym.nit;
	LeaveCriticalSection(&cs_symbols);

	EnterCriticalSection(&cs_scraper);
	unsigned int	card_player[2], card_common[5];
	for (i=0; i<=1; i++)
		card_player[i] = scraper.card_player[sym_userchair][i];
	for (i=0; i<=4; i++)
		card_common[i] = scraper.card_common[i];
	LeaveCriticalSection(&cs_scraper);

	if (log_fp != NULL) 
	{
		EnterCriticalSection(&cs_symbols);
			// log$ writing
			if (global.preferences.LogSymbol_enabled)
			{
				int max_log = symbols.logsymbols_collection.GetCount();

				if (max_log > 0)
				{
					if (max_log > global.preferences.LogSymbol_max_log)
					{
						max_log = global.preferences.LogSymbol_max_log;
					}

					write_log("*** log$ (Total: %d | Showing: %d)\n", symbols.logsymbols_collection.GetCount(), max_log);

					for (int i=0; i<max_log; i++)
					{
						write_log("***     %s\n", symbols.logsymbols_collection[i]);
					}
				}
			}
		
		LeaveCriticalSection(&cs_symbols);


		CardMask_RESET(Cards);
		nCards=0;
		// player cards
		if (user_chair_confirmed) 
		{
			for (i=0; i<=1; i++) 
			{
				card = StdDeck_cardString(card_player[i]);
				temp.Format("%s", card);
				pcards.Append(temp);
				CardMask_SET(Cards, card_player[i]);
				nCards++;
			}
		}
		else 
		{
			pcards = "....";
		}

		// common cards
		comcards = "";
		if (sym_br >= 2) 
		{
			for (i=0; i<=2; i++) 
			{
				if (card_common[i] != CARD_BACK && card_common[i] != CARD_NOCARD) 
				{
					card = StdDeck_cardString(card_common[i]);
					temp.Format("%s", card);
					comcards.Append(temp);
					CardMask_SET(Cards, card_common[i]);
					nCards++;
				}
			}
		}

		if (sym_br >= 3) 
		{
			card = StdDeck_cardString(card_common[3]);
			temp.Format("%s", card);
			comcards.Append(temp);
			CardMask_SET(Cards, card_common[3]);
			nCards++;
		}

		if (sym_br >= 4) 
		{
			card = StdDeck_cardString(card_common[4]);
			temp.Format("%s", card);
			comcards.Append(temp);
			CardMask_SET(Cards, card_common[4]);
			nCards++;
		}

        comcards.Append("..........");
        comcards = comcards.Left(10);

        // handrank
        if (global.preferences.handrank_value == "169")
            rank.Format("%.0f", sym_handrank169);

        else if (global.preferences.handrank_value == "1000")
            rank.Format("%.0f", sym_handrank1000);

        else if (global.preferences.handrank_value == "1326")
            rank.Format("%.0f", sym_handrank1326);

        else if (global.preferences.handrank_value == "2652")
            rank.Format("%.0f", sym_handrank2652);

        else if (global.preferences.handrank_value == "p")
            rank.Format("%.2f", sym_handrankp);


        // poker hand
        hv = Hand_EVAL_N(Cards, nCards);
        if (HandVal_HANDTYPE(hv)==HandType_STFLUSH && StdDeck_RANK(HandVal_TOP_CARD(hv))==Rank_ACE)
            pokerhand="royfl";

        else if (HandVal_HANDTYPE(hv)==HandType_STFLUSH)
            pokerhand="strfl";

        else if (HandVal_HANDTYPE(hv)==HandType_QUADS)
            pokerhand="4kind";

        else if (HandVal_HANDTYPE(hv)==HandType_FULLHOUSE)
            pokerhand="fullh";

        else if (HandVal_HANDTYPE(hv)==HandType_FLUSH)
            pokerhand="flush";

        else if (HandVal_HANDTYPE(hv)==HandType_STRAIGHT)
            pokerhand="strai";

        else if (HandVal_HANDTYPE(hv)==HandType_TRIPS)
            pokerhand="3kind";

        else if (HandVal_HANDTYPE(hv)==HandType_TWOPAIR)
            pokerhand="2pair";

        else if (HandVal_HANDTYPE(hv)==HandType_ONEPAIR)
            pokerhand="1pair";

        else if (HandVal_HANDTYPE(hv)==HandType_NOPAIR)
            pokerhand="hcard";


        // best action
        if (strcmp(action, "SWAG")==0) 
		{
            bestaction.Format("$%.2f", f_swag);
        }
        else 
		{
            if (f_alli)
                bestaction = "Allin";

            else if (f_swag)
                bestaction = "SWAG";

            else if (f_rais)
                bestaction = "Bet/Raise";

            else if (f_call)
                bestaction = "Call/Check";

            else if (f_prefold)
                bestaction = "Pre-fold";

            else
                bestaction = "Fold/Check";

        }

        // fcra_seen
        fcra_seen.Format("%s%s%s%s",
                         sym_myturnbits&0x1 ? "F" : ".",
                         sym_myturnbits&0x2 ? "C" : ".",
                         sym_myturnbits&0x4 ? "R" : ".",
                         sym_myturnbits&0x8 ? "A" : ".");

        // fcra formula status
        fcra_formula_status.Format("%s%s%s%s",
                                   !f_alli && !f_rais && !f_call && !f_swag ? "F" : ".",
                                   f_call ? "C" : ".",
                                   f_rais ? "R" : ".",
                                   f_alli ? "A" : ".");

        fprintf(log_fp, "%s - %1d ", get_time(nowtime), global.trans.map.num_chairs);
        fprintf(log_fp, "%4s %10s %4s %5s ", pcards.GetString(), comcards.GetString(), rank.GetString(), pokerhand.GetString());
        fprintf(log_fp, "%4d %4d %4d ", (int) (sym_prwin*1000),(int) (sym_prlos*1000), (int) (sym_prtie*1000));
        fprintf(log_fp, "%2d %8d %-10s - ", (int) sym_nopponents, (int) sym_nit, bestaction.GetString());
        fprintf(log_fp, "%-5s %9.2f %9.2f %9.2f ", action, sym_call, sym_bet_4, sym_pot);
        fprintf(log_fp, "%9.2f - %s %s %.2f\n", sym_balance_10, fcra_seen.GetString(), fcra_formula_status.GetString(), f_swag);

		EnterCriticalSection(&cs_symbols);

			if (global.preferences.Trace_enabled && symbols.symboltrace_collection.GetSize() > 0)
			{
				write_log_nostamp("***** Autoplayer Trace ****\n");
				for (int i=0; i<symbols.symboltrace_collection.GetSize(); i++)
				{
					write_log_nostamp("%s\n", symbols.symboltrace_collection[i]);
				}
				write_log_nostamp("***********************\n");
			}

		LeaveCriticalSection(&cs_symbols);

		fflush(log_fp);
    }

	__SEH_LOGFATAL("::write_log_autoplay\n");
}

void stop_log(void) 
{
    __SEH_HEADER

    if (log_fp != NULL) 
	{
        write_log("! log file closed\n");
        fclose(log_fp);
        log_fp = NULL;
    }

    __SEH_LOGFATAL("::stop_log\n");
}