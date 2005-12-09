
#ifndef _globals_h
#define _globals_h
	// application signature
	#define APP_NAME "fakBEtur"
	// bump it on every release
	#define APP_VERSION "0.3"
	// sync it to sqlschema CVS revision
	#define APP_DBVERSION "1.3"
	#define APP_SIGNATURE "application/x-vnd.generic-fakBEtur"
	extern int AppReturnValue;
	// general max() function
	#define max(x,y) ( ((x)>(y)) ? (x) : (y) )
	// gettext-style translation helper
	#define _(x) SpTranslate(x)
	// Qt-style translation helper
	#define tr(x) SpTranslate(x)
	// db file with full path
	#define DATABASE_PATHNAME "/boot/home/config/share/fakbetur.db"

	// some globally visible stuff
	#define MSG_REQTOWARUP		'RQTU'	// symbols in faktura
	#define MSG_REQFIRMAUP		'RQFU'	// symbols in faktura
	#define MSG_REQTOWARLIST	'RQTL'	// refresh towar list
	#define MSG_REQFIRMALIST	'RQFL'	// refresh firma list
	#define MSG_REQFAKPOZLIST	'RQFT'	// refresh towar list on faktura, make dirty
#endif
