
#ifndef _globals_h
#define _globals_h
	// application signature
	#define APP_NAME "fakBEtur"
	#define APP_VERSION "0.1"
	#define APP_SIGNATURE "application/x-vnd.generic-fakBEtur"
	extern int AppReturnValue;
	// gettext-style translation helper
	#define _(x) SpTranslate(x)
	// Qt-style translation helper
	#define tr(x) SpTranslate(x)
	// db file with full path
	#define DATABASE_PATHNAME "/boot/home/config/share/fakbetur.db"
#endif
