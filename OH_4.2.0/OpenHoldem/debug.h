#ifndef INC_DEBUG_H
#define INC_DEBUG_H

#include "../CCritSec/CCritSec.h"

// This lets transform.cpp know which source tree is #include'ing it
#ifndef OPENHOLDEM_PROGRAM
#define OPENHOLDEM_PROGRAM
#endif

LONG WINAPI MyUnHandledExceptionFilter(EXCEPTION_POINTERS *pExceptionPointers);
char * get_time(char * timebuf);
char * get_now_time(char * timebuf);
BOOL CreateBMPFile(const char *szFile, HBITMAP hBMP);

void start_log(void);
void stop_log(void);
void write_log_vl(bool debug_settings_for_this_message, char* fmt, va_list vl);
void write_log(bool debug_settings_for_this_message, char* fmt, ...);
void write_log_nostamp(bool debug_settings_for_this_message, char* fmt, ...);
void write_logautoplay(const char * action);
int GenerateDump(EXCEPTION_POINTERS *pExceptionPointers);

extern FILE *log_fp;
extern CCritSec	log_critsec;  // Used to ensure only one thread at a time writes to log file

#endif /* INC_DEBUG_H */
