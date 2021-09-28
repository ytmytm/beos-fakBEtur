
#ifndef _DIALSYMBOL_H
#define _DIALSYMBOL_H

#include <Window.h>
#include <sqlite3.h>

class dbdat;
class BButton;
class BHandler;
class BMessage;
class BStringView;
class BTextControl;
class BView;

class dialSymbol : public BWindow {
	public:
		dialSymbol(sqlite3 *db, bool towar, dbdat *data, BHandler *hr);
		virtual void MessageReceived(BMessage *Message);
		virtual bool QuitRequested();

	private:
		bool commit(void);

		BHandler *handler;
		BView *view;
		BButton *but_ok, *but_cancel;
		BStringView *nazwa, *error;
		BTextControl *symbol;

		bool istowar;
		dbdat *curdata;

		sqlite3 *dbData;
		char *dbErrMsg;
};

#endif
