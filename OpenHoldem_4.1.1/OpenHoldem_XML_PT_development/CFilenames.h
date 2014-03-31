extern class CFilenames
{
public:
	CFilenames();
	~CFilenames();
public:
	CString OpenHoldemDirectory();
	CString IniFilename();
	CString TableMapWildcard();
	CString LogFilename();
	CString DebugTabLogFilename();
	CString PokerTrackerLogFilename();
	CString MiniDumpFilename();
	CString ScraperDirectory();
	CString ReplaySessionDirectory();
	CString ReplayBitmapFilename(int frame_number);
	CString ReplayHTMLFilename(int frame_number);
	void SwitchToOpenHoldemDirectory();
private:
	char _startup_path[MAX_PATH];
} *p_filenames;