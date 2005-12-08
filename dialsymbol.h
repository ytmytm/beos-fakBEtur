
#ifndef _DIALSYMBOL_H
#define _DIALSYMBOL_H

#include <Window.h>
#include <sqlite.h>

class dbdat;
class BHandler;
class BMessage;
class BStringView;
class BTextControl;
class BView;

class dialSymbol : public BWindow {
	public:
		dialSymbol(sqlite *db, bool towar, dbdat *data, BHandler *hr);
		virtual void MessageReceived(BMessage *Message);
		virtual bool QuitRequested();

	private:
		bool commit(void);

		BHandler *handler;
		BView *view;
		BStringView *nazwa, *error;
		BTextControl *symbol;

		bool istowar;
		dbdat *curdata;

		sqlite *dbData;
		char *dbErrMsg;
};

#endif
