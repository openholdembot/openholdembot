#include "stdafx.h"
#include "CSessionCounter.h"
#include "CSharedMem.h"

// For an introduction about shared memory see:
//   * http://www.programmersheaven.com/mb/Win32API/156366/156366/shared-memory-and-dll/
//   * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2044.html
//   * http://www.codeproject.com/KB/threads/SharedMemory_IPC_Threads.aspx
//   * http://www.fstsoft.de/shared.htm (german)
//
// We chose the first solution for its simplicity


///////////////////////////////////////////////////////////////////////////////////
//
// SHARED MEMORY for communication and synchronisation of OH-instances.
//
///////////////////////////////////////////////////////////////////////////////////

#pragma data_seg(".ohshmem") // names are limited to 8 chars, including the dot.


static HWND		AttachedPokerWindows[MAX_SESSION_IDS] = { NULL };	// for the auto-connector
static time_t	LastFailedAttemptToConnect;	// last time any instance failed; to avoid superflous attempts by other instances of OH

#pragma data_seg()
#pragma comment(linker, "/SECTION:.ohshmem,RWS")		// RWS: read, write, shared

///////////////////////////////////////////////////////////////////////////////////
//
// CLASS CSharedMem to access the shared memory
//
///////////////////////////////////////////////////////////////////////////////////

CSharedMem	*p_sharedmem = NULL;

CSharedMem::CSharedMem()
{}


CSharedMem::~CSharedMem()
{}


bool CSharedMem::PokerWindowAttached(HWND Window)
{
	ENT;
	for (int i=0; i<MAX_SESSION_IDS; i++)
	{
		if (AttachedPokerWindows[i] == Window)
		{
			// Window found. Already attached.
			return true;
		}
	}
	// Window not found. Not attached.
	return false;
}


void CSharedMem::MarkPokerWindowAsAttached(HWND Window)
{
	ENT;
	AttachedPokerWindows[p_sessioncounter->session_id()] = Window;	
}


void CSharedMem::MarkPokerWindowAsUnAttached()
{
	ENT;
	AttachedPokerWindows[p_sessioncounter->session_id()] = NULL;
}


void CSharedMem::RememberTimeOfLastFailedAttemptToConnect()
{
	ENT;
	time(&LastFailedAttemptToConnect);
}


time_t CSharedMem::GetTimeOfLastFailedAttemptToConnect()
{
	ENT;
	return LastFailedAttemptToConnect;
}


