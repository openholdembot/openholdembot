#ifndef INC_CHEARTBEATTHREAD_H
#define INC_CHEARTBEATTHREAD_H

#include "..\CCritSec\CCritSec.h"

extern long int	_heartbeat_counter;

extern class CHeartbeatThread
{
public:
	// This critical section does not control access to any variables/members, but is used as 
	// a flag to indicate when the scraper/symbol classes are in an update cycle
	static CRITICAL_SECTION	cs_update_in_progress;

public:
	// public functions
		CHeartbeatThread();
		~CHeartbeatThread();

public:
	// public accessors
	long int	heartbeat_counter() { return _heartbeat_counter; }

private:
	// private functions and variables - not available via accessors or mutators
	static UINT HeartbeatThreadFunction(LPVOID pParam);
	static void SetOpenHoldemWindowTitle();

private:
	HANDLE			_m_stop_thread;
	HANDLE			_m_wait_thread;
private:
	// private variables - use public accessors and public mutators to address these	
	CCritSec		m_critsec;
} *p_heartbeat_thread;

#endif //INC_CHEARTBEATTHREAD_H