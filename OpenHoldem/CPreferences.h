#ifndef INC_CPREFERENCES_H_
#define INC_CPREFERENCES_H_

#include "..\CCritSec\CCritSec.h"

// global preferences constants
const int k_AutoConnector_Connect_Manually		= 0;
const int k_AutoConnector_Connect_Automatically	= 1;

const int k_AutoConnector_Connect_Never		= 0;
const int k_AutoConnector_Connect_Once		= 1;
const int k_AutoConnector_Connect_Permanent	= 2;

const int k_lazy_scraping_always = 0;
const int k_lazy_scraping_myturn = 1;
const int k_lazy_scraping_cards  = 2;


extern class CPreferences
{
public:
	// public functions
	CPreferences();
	~CPreferences();

	void LoadPreferences(bool registry);

public:
	// public accessors

	// Analyzer
	const unsigned int max_opponents() { return _max_opponents; }

	// Autoplayer
	const unsigned int frame_delay() { return _frame_delay; }
	const unsigned int click_delay() { return _click_delay; }
	const unsigned int swag_delay_1() { return _swag_delay_1; }
	const unsigned int swag_delay_2() { return _swag_delay_2; }
	const unsigned int swag_delay_3() { return _swag_delay_3; }
	const bool ap_auto() { return _ap_auto; }
	const bool focus_detect() { return _focus_detect; }
	const bool swag_use_comma() { return _swag_use_comma; }
	const bool calc_only_my_turn() { return _calc_only_my_turn; }
	const bool need_balance_stability() { return _need_balance_stability; }

	// Dll extension
	const bool dll_always_send_state() { return _dll_always_send_state; }
	const bool dll_load_on_startup() { return _dll_load_on_startup; }
	const CString dll_name() { return _dll_name; }
	// Scraper
	const unsigned int scrape_delay() { return _scrape_delay; }
	// Symbols
	const double sym_av_time() { return _sym_av_time; }
	const CString sym_handrank_value() { return _sym_handrank_value; }
	const bool sym_disable_caching() { return _sym_disable_caching; }
	// ICM
	const double icm_prize1() { return _icm_prize1; }
	const double icm_prize2() { return _icm_prize2; }
	const double icm_prize3() { return _icm_prize3; }
	const double icm_prize4() { return _icm_prize4; }
	const double icm_prize5() { return _icm_prize5; }
	// Replay Frames
	const bool replay_record() { return _replay_record; }
	const bool replay_record_every_change() { return _replay_record_every_change; }
	const int replay_max_frames() { return _replay_max_frames; }
	const bool replay_record_every_change_playing() { return _replay_record_every_change_playing; }
	// Poker Tracker
	const CString pt_ip_addr() { return _pt_ip_addr; }
	const CString pt_port() { return _pt_port; }
	const CString pt_dbname() { return _pt_dbname; }
	const CString pt_user() { return _pt_user; }
	const CString pt_pass() { return _pt_pass; }
	const bool pt_disable() { return _pt_disable; }
	const int pt_update_delay() { return _pt_update_delay; }
	const int pt_cache_refresh() { return _pt_cache_refresh; }
	// Perl
	const CString perl_default_formula() { return _perl_default_formula; }
	const CString perl_editor() { return _perl_editor; }
	const bool perl_load_default_formula() { return _perl_load_default_formula; }
	const bool perl_load_interpreter() { return _perl_load_interpreter; }
	//  PokerChat
	const bool chat_enabled() { return _chat_enabled; }
	const int chat_min_delay() { return _chat_min_delay; }
	const int chat_random_delay() { return _chat_random_delay; }
	// Log Symbol
	const bool log_symbol_enabled() { return _log_symbol_enabled; }
	const int log_symbol_max_log() { return _log_symbol_max_log; }
	
	// Logging and debugging
	const bool disable_msgbox() { return _disable_msgbox; }
	const int log_max_logsize() { return _log_max_logsize; }

	const bool log_basic_info();
	const bool log_errors_and_warnings();
	const bool trace_enabled() { return _trace_enabled; }

	// Debugging
	const bool	debug_autoconnector() { return _debug_autoconnector; }
	const bool	debug_autoplayer() { return _debug_autoplayer; }
	const bool	debug_heartbeat() { return _debug_heartbeat; }
	const bool	debug_prwin() { return _debug_prwin; }
	const bool	debug_icm() { return _debug_icm; }
	const bool	debug_occlusionchecker() { return _debug_occlusionchecker; }
	const bool	debug_pokertracker() { return _debug_pokertracker; }
	const bool	debug_rebuy() { return _debug_rebuy; }
	const bool	debug_replayframes() { return _debug_replayframes; }
	const bool	debug_scraper() { return _debug_scraper; }
	const bool	debug_sessioncounter() { return _debug_sessioncounter; }
	const bool	debug_stableframescounter() { return _debug_stableframescounter; }
	const bool	debug_symbolengine() { return _debug_symbolengine; }
	const bool	debug_blindlocking() { return _debug_blindlocking; }
	const bool	debug_memorysymbols() { return _debug_memorysymbols; }
	const bool	debug_alltherest() { return _debug_alltherest; }

	// Main window locations
	const int main_x() { return _main_x; }
	const int main_y() { return _main_y; }
	const int main_dx() { return _main_dx; }
	const int main_dy() { return _main_dy; }
	// Scraper window locations
	const int scraper_x() { return _scraper_x; }
	const int scraper_y() { return _scraper_y; }
	const int scraper_dx() { return _scraper_dx; }
	const int scraper_dy() { return _scraper_dy; }
	// Formula window locations
	const int formula_x() { return _formula_x; }
	const int formula_y() { return _formula_y; }
	const int formula_dx() { return _formula_dx; }
	const int formula_dy() { return _formula_dy; }	
	// Blind locking
	const double sblind() { return _sblind; }
	const double bblind() { return _bblind; }
	const double bbet() { return _bbet; }
	const double ante() { return _ante; }
	const int gametype() { return _gametype; }
	// Formula editor
	const bool expand_auto() { return _expand_auto; }
	const bool expand_debug() { return _expand_debug; }
	const bool expand_list() { return _expand_list; }
	const bool expand_std() { return _expand_std; }
	const bool expand_udf() { return _expand_udf; }
	const bool udf_sort() { return _udf_sort; }
	const bool udf_group() { return _udf_group; }
	const int equal() { return _equal; }
	const int precision() { return _precision; }
	// Debug logging
	const bool fdebuglog() { return _fdebuglog; }
	const bool fdebuglog_myturn() { return _fdebuglog_myturn; }
	// Validator
	const int validator_enabled() { return _validator_enabled; }
	const bool validator_stop_on_error() { return _validator_stop_on_error; }
	const bool validator_use_heuristic_rules() { return _validator_use_heuristic_rules; }
	const bool validator_shoot_replayframe_on_error() { return _validator_shoot_replayframe_on_error; }
	// Auto-connector
	const int autoconnector_connection_method() { return _autoconnector_connection_method; }
	const int autoconnector_when_to_connect() { return _autoconnector_when_to_connect; }
	const bool autoconnector_close_when_table_disappears() { return _autoconnector_close_when_table_disappears; }
	// GUI
	const bool gui_start_minimized() { return _gui_start_minimized; }
	const bool gui_disable_progress_dialog() { return _gui_disable_progress_dialog; }
	// Rebuy
	const bool rebuy_condition_no_cards() { return _rebuy_condition_no_cards; }
	const bool rebuy_condition_change_in_handnumber() { return _rebuy_condition_change_in_handnumber; }
	const bool rebuy_condition_heuristic_check_for_occlusion() { return _rebuy_condition_heuristic_check_for_occlusion; }
	const int  rebuy_minimum_time_to_next_try() { return _rebuy_minimum_time_to_next_try; }
	const CString rebuy_script() { return _rebuy_script; }
	// Configuration check
	const bool configurationcheck_input_settings() { return _configurationcheck_input_settings; }
	const bool configurationcheck_theme_settings() { return _configurationcheck_theme_settings; }
	const bool configurationcheck_font_settings() { return _configurationcheck_font_settings; }
   	// Lazy scraping
	const int lazy_scraping_when_to_scrape() { return _lazy_scraping_when_to_scrape; }
	// Handhistory generator
	const bool handhistory_generator_enable() { return _handhistory_generator_enable; }

	// Misc
	const int scraper_zoom() { return _scraper_zoom; }
	// versus_path() is implemented in the cpp-file.
	const CString versus_path(); 	
	// Obscure
	const CString &window_class_name()		{ return _window_class_name; }
	const CString &mutex_name()			{ return _mutex_name; }
	bool simple_window_title()			{ return _simple_window_title; }
	// CFileDialog saved paths
	const CString path_ohf()	{ return _path_ohf; }
	const CString path_tm()		{ return _path_tm; }
	const CString path_perl()	{ return _path_perl; }
	const CString path_dll()	{ return _path_dll; }

public:
	// public mutators
#define ENT CSLock lock(m_critsec);

	// Analyzer
	void set_max_opponents(const unsigned int i) { ENT _max_opponents = i; WriteReg("max_opponents", (int) i); }

	// Autoplayer
	void set_frame_delay(const unsigned int i) { ENT _frame_delay = i; WriteReg("frame_delay", (int) i); }
	void set_click_delay(const unsigned int i) { ENT _click_delay = i; WriteReg("click_delay", (int) i); }
	void set_swag_delay_1(const unsigned int i) { ENT _swag_delay_1 = i; WriteReg("swag_delay_1", (int) i); }
	void set_swag_delay_2(const unsigned int i) { ENT _swag_delay_2 = i; WriteReg("swag_delay_2", (int) i); }
	void set_swag_delay_3(const unsigned int i) { ENT _swag_delay_3 = i; WriteReg("swag_delay_3", (int) i); }
	void set_ap_auto(const bool b) { ENT _ap_auto = b; WriteReg("auto", (int) b!=false); }
	void set_focus_detect(const bool b) { ENT _focus_detect = b; WriteReg("focus_detect", (int) b!=false); }
	void set_swag_use_comma(const bool b) { ENT _swag_use_comma = b; WriteReg("swag_use_comma", (int) b!=false); }
	void set_calc_only_my_turn(const bool b) { ENT _calc_only_my_turn = b; WriteReg("calc_only_my_turn", (int) b!=false); }
	void set_need_balance_stability(const bool b) { ENT _need_balance_stability = b; WriteReg("need_balance_stability", (int) b!=false); }

	// Dll extension
	void set_dll_always_send_state(const bool b) { ENT _dll_always_send_state = b; WriteReg("dll_always_send_state", b); }
	void set_dll_load_on_startup(const bool b) { ENT _dll_load_on_startup = b; WriteReg("load_dll_on_startup", b); }
	void set_dll_name(const CString s) { ENT _dll_name = s; WriteReg("dll_name", s); }

	// Scraper
	void set_scrape_delay(const unsigned int i) { ENT _scrape_delay = i; WriteReg("scrape_delay", (int) i); }

	// Symbols
	void set_sym_av_time(const double d) { ENT _sym_av_time = d; WriteReg("avtime", d); }
	void set_sym_handrank_value(const CString s) { ENT _sym_handrank_value = s; WriteReg("handrank_value", s); }
	void set_sym_disable_caching(const bool b) { ENT _sym_disable_caching = b; WriteReg("disable_caching", b); }

	// ICM
	void set_icm_prize1(const double d) { ENT _icm_prize1 = d; WriteReg("icm_prize1", d); }
	void set_icm_prize2(const double d) { ENT _icm_prize2 = d; WriteReg("icm_prize2", d); }
	void set_icm_prize3(const double d) { ENT _icm_prize3 = d; WriteReg("icm_prize3", d); }
	void set_icm_prize4(const double d) { ENT _icm_prize4 = d; WriteReg("icm_prize4", d); }
	void set_icm_prize5(const double d) { ENT _icm_prize5 = d; WriteReg("icm_prize5", d); }

	// Replay Frames
	void set_replay_record(const bool b) { ENT _replay_record = b; WriteReg("replay_record", b); }
	void set_replay_record_every_change(const bool b) { ENT _replay_record_every_change = b; WriteReg("replay_record_every_change", b); }
	void set_replay_max_frames(const int i) { ENT _replay_max_frames = i; WriteReg("replay_max_frames", i); }
	void set_replay_record_every_change_playing(const bool b) { ENT _replay_record_every_change_playing = b; WriteReg("replay_record_every_change_playing", b); }

	// Poker Tracker
	void set_pt_ip_addr(const CString s) { ENT _pt_ip_addr = s; WriteReg("pt_ip_addr", s); }
	void set_pt_port(const CString s) { ENT _pt_port = s; WriteReg("pt_port", s); }
	void set_pt_dbname(const CString s) { ENT _pt_dbname = s; WriteReg("pt_dbname", s); }
	void set_pt_user(const CString s) { ENT _pt_user = s; WriteReg("pt_user", s); }
	void set_pt_pass(const CString s) { ENT _pt_pass = s; WriteReg("pt_pass", s); }
	void set_pt_disable(const bool b) { ENT _pt_disable = b; WriteReg("pt_disable", b); }
	void set_pt_update_delay(const int i) { ENT _pt_update_delay = i; WriteReg("pt_updatedelay", i); }
	void set_pt_cache_refresh(const int i) { ENT _pt_cache_refresh = i; WriteReg("pt_cacherefresh", i); }

	// Perl
	void set_perl_default_formula(const CString s) { ENT _perl_default_formula = s; WriteReg("perl_default_formula", s); }
	void set_perl_editor(const CString s) { ENT _perl_editor = s; WriteReg("perl_editor", s); }
	void set_perl_load_default_formula(const bool b) { ENT _perl_load_default_formula = b; WriteReg("perl_load_default_formula", b); }
	void set_perl_load_interpreter(const bool b) { ENT _perl_load_interpreter = b; WriteReg("perl_load_interpreter", b); }

	//  PokerChat
	void set_chat_enabled(const bool b) { ENT _chat_enabled = b; WriteReg("chat_enabled", b); }
	void set_chat_min_delay(const int i) { ENT _chat_min_delay = i; WriteReg("chat_min_delay", i); }
	void set_chat_random_delay(const int i) { ENT _chat_random_delay = i; WriteReg("chat_random_delay", i); }

	// Log Symbol
	void set_log_symbol_enabled(const bool b) { ENT _log_symbol_enabled = b; WriteReg("log_symbol_enabled", b); }
	void set_log_symbol_max_log(const int b) { ENT _log_symbol_max_log = b; WriteReg("log_symbol_max_log", b); }

	// Tracing
	void set_trace_enabled(const bool b) { ENT _trace_enabled = b; WriteReg("trace_enabled", b); }

	// Logging and debugging
	void set_disable_msgbox(const bool b) { ENT _disable_msgbox = b; WriteReg("disable_msgbox", b); }
	void set_log_max_logsize(const int b) { ENT _log_max_logsize = b; WriteReg("log_max_logsize", b); }

	// Debugging
	void set_debug_autoconnector(const int b) { ENT _debug_autoconnector = b; WriteReg("debug_autoconnector", b); }
	void set_debug_autoplayer(const int b) { ENT _debug_autoplayer = b; WriteReg("debug_autoplayer", b); }
	void set_debug_heartbeat(const int b) { ENT _debug_heartbeat = b; WriteReg("debug_heartbeat", b); }
	void set_debug_prwin(const int b) { ENT _debug_prwin = b; WriteReg("debug_prwin", b); }
	void set_debug_icm(const int b) { ENT _debug_icm = b; WriteReg("debug_icm", b); }
	void set_debug_occlusionchecker(const int b) { ENT _debug_occlusionchecker = b; WriteReg("debug_occlusionchecker", b); }
	void set_debug_pokertracker(const int b) { ENT _debug_pokertracker = b; WriteReg("debug_pokertracker", b); }
	void set_debug_rebuy(const int b) { ENT _debug_rebuy = b; WriteReg("debug_rebuy", b); }
	void set_debug_replayframes(const int b) { ENT _debug_replayframes = b; WriteReg("debug_replayframes", b); }
	void set_debug_scraper(const int b) { ENT _debug_scraper = b; WriteReg("debug_scraper", b); }
	void set_debug_sessioncounter(const int b) { ENT _debug_sessioncounter = b; WriteReg("debug_sessioncounter", b); }
	void set_debug_stableframescounter(const int b) { ENT _debug_stableframescounter = b; WriteReg("debug_stableframescounter", b); }
	void set_debug_symbolengine(const int b) { ENT _debug_symbolengine = b; WriteReg("debug_symbolengine", b); }
	void set_debug_blindlocking(const int b) { ENT _debug_blindlocking = b; WriteReg("debug_blindlocking", b); }
	void set_debug_memorysymbols(const int b) { ENT _debug_memorysymbols = b; WriteReg("debug_memorysymbols", b); }
	void set_debug_alltherest(const int b) { ENT _debug_alltherest = b; WriteReg("debug_alltherest", b); }

	// Formula window location
	void set_formula_x(const int i) { ENT _formula_x = i; WriteReg("formula_x", i); }
	void set_formula_y(const int i) { ENT _formula_y = i; WriteReg("formula_y", i); }
	void set_formula_dx(const int i) { ENT _formula_dx = i; WriteReg("formula_dx", i); }
	void set_formula_dy(const int i) { ENT _formula_dy = i; WriteReg("formula_dy", i); }

	// Scraper window location
	void set_scraper_x(const int i) { ENT _scraper_x = i; WriteReg("scraper_x", i); }
	void set_scraper_y(const int i) { ENT _scraper_y = i; WriteReg("scraper_y", i); }
	void set_scraper_dx(const int i) { ENT _scraper_dx = i; WriteReg("scraper_dx", i); }
	void set_scraper_dy(const int i) { ENT _scraper_dy = i; WriteReg("scraper_dy", i); }

	// Main window location
	void set_main_x(const int i) { ENT _main_x = i; WriteReg("main_x", i); }
	void set_main_y(const int i) { ENT _main_y = i; WriteReg("main_y", i); }
	void set_main_dx(const int i) { ENT _main_dx = i; WriteReg("main_dx", i); }
	void set_main_dy(const int i) { ENT _main_dy = i; WriteReg("main_dy", i); }
	// Blind locking
	void set_sblind(const double d) { ENT _sblind = d; WriteReg("sblind", d); }
	void set_bblind(const double d) { ENT _bblind = d; WriteReg("bblind", d); }
	void set_bbet(const double d) { ENT _bbet = d; WriteReg("bbet", d); }
	void set_ante(const double d) { ENT _ante = d; WriteReg("ante", d); }
	void set_gametype(const int i) { ENT _gametype = i; WriteReg("gametype", i); }

	// Formula editor
	void set_expand_auto(const bool  b) { ENT _expand_auto = b; WriteReg("expand_auto", b); }
	void set_expand_debug(const bool  b) { ENT _expand_debug = b; WriteReg("expand_debug", b); }
	void set_expand_list(const bool  b) { ENT _expand_list = b; WriteReg("expand_list", b); }
	void set_expand_std(const bool b) { ENT _expand_std = b; WriteReg("expand_std", b); }
	void set_expand_udf(const bool b) { ENT _expand_udf = b; WriteReg("expand_udf", b); }
	void set_udf_sort(const bool b) { ENT _udf_sort = b; WriteReg("udf_sort", b); }
	void set_udf_group(const bool b) { ENT _udf_group = b; WriteReg("udf_group", b); }
	void set_equal(const int i) { ENT _equal = i; WriteReg("equal", i); }
	void set_precision(const int i) { ENT _precision = i; WriteReg("precision", i); }

	// Debug logging
	void set_fdebuglog(const bool b) { ENT _fdebuglog = b; WriteReg("fdebuglog", b); }
	void set_fdebuglog_myturn(const bool b) { ENT _fdebuglog_myturn = b; WriteReg("fdebuglog_myturn", b); }

	// Validator
	void set_validator_enabled(const int i) { ENT _validator_enabled = i; WriteReg("validator_enabled", i); }
	void set_validator_stop_on_error(const bool b) { ENT _validator_stop_on_error = b; WriteReg("validator_stop_on_error", b); }
	void set_validator_use_heuristic_rules(const bool b) { ENT _validator_use_heuristic_rules = b; WriteReg("validator_use_heuristic_rules", b); }
	void set_validator_shoot_replayframe_on_error(const bool b) { ENT _validator_shoot_replayframe_on_error = b; WriteReg("validator_shoot_replayframe_on_error", b); }

	// Auto-connector
	void set_autoconnector_connection_method(const int i) { ENT _autoconnector_connection_method = i; WriteReg("autoconnector_connection_method", i); }
	void set_autoconnector_when_to_connect(const int i) { ENT _autoconnector_when_to_connect = i; WriteReg("autoconnector_when_to_connect", i); }
	void set_autoconnector_close_when_table_disappears(const bool b) { ENT _autoconnector_close_when_table_disappears = b; WriteReg("autoconnector_close_when_table_disappears", b); }

	// GUI
	void set_gui_start_minimized(const bool b) { ENT _gui_start_minimized = b; WriteReg("gui_start_minimized", b); }
	void set_gui_disable_progress_dialog(const bool b) { ENT _gui_disable_progress_dialog = b; WriteReg("gui_disable_progress_dialog", b); }

	// Rebuy
	void set_rebuy_condition_no_cards(const bool b) { ENT _rebuy_condition_no_cards = b; WriteReg("rebuy_condition_no_cards", b); }
	void set_rebuy_condition_change_in_handnumber(const bool b) { ENT _rebuy_condition_change_in_handnumber = b; WriteReg("rebuy_condition_change_in_handnumber", b); }
	void set_rebuy_condition_heuristic_check_for_occlusion(const bool b) { ENT _rebuy_condition_heuristic_check_for_occlusion = b; WriteReg("rebuy_condition_heuristic_check_for_occlusion", b); }
	void set_rebuy_minimum_time_to_next_try(const int i) { ENT _rebuy_minimum_time_to_next_try = i; WriteReg("rebuy_minimum_time_to_next_try", i); }
	void set_rebuy_script(const CString s) { ENT _rebuy_script = s; WriteReg("rebuy_script", s); }

	// Configuration check
	void set_configurationcheck_input_settings(const bool b) { ENT _configurationcheck_input_settings = b; WriteReg("configurationcheck_input_settings", b); }
	void set_configurationcheck_theme_settings(const bool b) { ENT _configurationcheck_theme_settings = b; WriteReg("configurationcheck_theme_settings", b); }
	void set_configurationcheck_font_settings(const bool b) { ENT _configurationcheck_font_settings = b; WriteReg("configurationcheck_font_settings", b); }

	// Lazy scraping
	void set_lazy_scraping_when_to_scrape(const int i) { ENT _lazy_scraping_when_to_scrape = i;WriteReg("lazy_scraping_when_to_scrape", i); }

	// Handhistory generator
	void set_handhistory_generator_enable(const bool b) { ENT _handhistory_generator_enable = b; WriteReg("handhistory_generator_enable", b); }

	// Misc
	// (No method for versus_path,
	// as this attribute is nowhere set.) 
	void set_scraper_zoom(const int i) { ENT _scraper_zoom = i; WriteReg("scraper_zoom", i); }

	// Obscure
	void set_window_class_name(const CString &s)	{ ENT _window_class_name = s; WriteReg("window_class_name", s); }
	void set_mutex_name(const CString &s)		{ ENT _mutex_name= s; WriteReg("mutex_name", s); }
	void set_simple_window_title(bool in)		{ ENT _simple_window_title= in; WriteReg("simple_window_title", (int) in!=false); }

	// CFileDialog saved paths
	void set_path_ohf(const CString s)		{ ENT _path_ohf = s; WriteReg("last_path_ohf", s); }
	void set_path_tm(const CString s)		{ ENT _path_tm = s; WriteReg("last_path_tm", s); }
	void set_path_perl(const CString s)		{ ENT _path_perl = s; WriteReg("last_path_perl", s); }
	void set_path_dll(const CString s)		{ ENT _path_dll = s; WriteReg("last_path_dll", s); }

#undef ENT

private:
	// private variables - use public accessors and public mutators to address these

	// Analyzer
	unsigned int		_max_opponents;

	// Autoplayer
	unsigned int		_frame_delay;
	unsigned int		_click_delay;
	unsigned int		_swag_delay_1;
	unsigned int		_swag_delay_2;
	unsigned int		_swag_delay_3;
	bool				_ap_auto;
	bool				_focus_detect;
	bool				_swag_use_comma;
	bool				_calc_only_my_turn;
	bool				_need_balance_stability;

	// Dll extension
	bool				_dll_always_send_state;
	bool				_dll_load_on_startup;
	CString				_dll_name;

	// Scraper
	unsigned int		_scrape_delay;

	// Symbols
	double				_sym_av_time;
	CString				_sym_handrank_value;
	bool				_sym_disable_caching;

	// ICM
	double			_icm_prize1;
	double			_icm_prize2;
	double			_icm_prize3;
	double			_icm_prize4;
	double			_icm_prize5;

	// Replay Frames
	bool			_replay_record;
	bool			_replay_record_every_change_playing;
	bool			_replay_record_every_change;
	int				_replay_max_frames;

	// Poker Tracker
	CString			_pt_ip_addr;
	CString			_pt_port;
	CString			_pt_dbname;
	CString			_pt_user;
	CString			_pt_pass;
	bool			_pt_disable;
	int				_pt_update_delay;
	int				_pt_cache_refresh;

	// Perl
	CString			_perl_default_formula;
	CString			_perl_editor;
	bool			_perl_load_default_formula;
	bool			_perl_load_interpreter;

	//  PokerChat
	bool			_chat_enabled;
	int				_chat_min_delay;
	int				_chat_random_delay;

	// Log Symbol
	bool			_log_symbol_enabled;
	int				_log_symbol_max_log;

	// Tracing
	bool			_trace_enabled;

	// Logging & MessageBox
	bool			_disable_msgbox;
	int				_log_max_logsize;

	// Debugging
	bool			_debug_autoconnector;
	bool			_debug_autoplayer;
	bool			_debug_heartbeat;
	bool			_debug_prwin;
	bool			_debug_icm;
	bool			_debug_occlusionchecker;
	bool			_debug_pokertracker;
	bool			_debug_rebuy;
	bool			_debug_replayframes;
	bool			_debug_scraper;
	bool			_debug_sessioncounter;
	bool			_debug_stableframescounter;
	bool			_debug_symbolengine;
	bool			_debug_blindlocking;
	bool			_debug_memorysymbols;
	bool			_debug_alltherest;

	// Formula window location
	int			_formula_x;	
	int			_formula_y;
	int			_formula_dx;
	int			_formula_dy;

	// Scraper window location
	int			_scraper_x;
	int			_scraper_y;
	int			_scraper_dx;
	int			_scraper_dy;

	// Main window location
	int			_main_x;
	int			_main_y;
	int			_main_dx;
	int			_main_dy;

	// Blind locking
	double			_sblind;
	double			_bblind;
	double			_bbet;
	double			_ante;
	int				_gametype;

	// Formula editor
	bool			_expand_auto;
	bool			_expand_debug;
	bool			_expand_list;
	bool			_expand_std;
	bool			_expand_udf;
	bool			_udf_sort;
	bool			_udf_group;
	int				_equal;
	int				_precision;

	// Debug logging
	bool			_fdebuglog;
	bool			_fdebuglog_myturn;

	// Validator
	int				_validator_enabled;
	bool			_validator_stop_on_error;
	bool			_validator_use_heuristic_rules;
	bool			_validator_shoot_replayframe_on_error;

	// Auto-connector
	int				_autoconnector_connection_method;
	int				_autoconnector_when_to_connect;
	bool			_autoconnector_close_when_table_disappears;

	// GUI
	bool			_gui_start_minimized; 
	bool			_gui_disable_progress_dialog;

	// Rebuy
	bool			_rebuy_condition_no_cards;
	bool			_rebuy_condition_change_in_handnumber;
	bool			_rebuy_condition_heuristic_check_for_occlusion;
	int				_rebuy_minimum_time_to_next_try;
	CString			_rebuy_script;

	// Configuration check
	bool			_configurationcheck_input_settings;
	bool			_configurationcheck_theme_settings;
	bool			_configurationcheck_font_settings;

	// Lazy scraping
	int				_lazy_scraping_when_to_scrape;

	// Handhistory generator
	bool			_handhistory_generator_enable;

	// Misc
	int				_scraper_zoom;

	// Obscure
	CString			_window_class_name;
	CString			_mutex_name;
	bool			_simple_window_title;

	// CFileDialog saved paths
	CString			_path_ohf;
	CString			_path_tm;
	CString			_path_perl;
	CString			_path_dll;

private:
	// private functions and variables - not available via accessors or mutators
	void InitDefaults(void);
	void ReadPreferences(void);
	void ReadReg(const LPCTSTR registry_key, int *registry_value);
	void ReadReg(const LPCTSTR registry_key, bool *registry_value);
	void ReadReg(const LPCTSTR registry_key, unsigned int *registry_value);
	void ReadReg(const LPCTSTR registry_key, CString *registry_value);
	void ReadReg(const LPCTSTR registry_key, double *registry_value);
	void WriteReg(const LPCTSTR registry_key, const int registry_value);
	void WriteReg(const LPCTSTR registry_key, const CString &registry_value);
	void WriteReg(const LPCTSTR registry_key, const double registry_value);

	CCritSec		m_critsec;
	CString			_preferences_heading;
} prefs;

#endif // INC_CPREFERENCES_H_
