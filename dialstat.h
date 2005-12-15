
#ifndef _DIALSTAT_H
#define _DIALSTAT_H

#include <Window.h>
#include <sqlite.h>

class BHandler;
class BMessage;
class BListView;
class BPopUpMenu;
class BStringView;
class BTextControl;
class BView;

class dialStat : public BWindow {
	public:
		dialStat(sqlite *db, BHandler *hr);
		virtual void MessageReceived(BMessage *Message);
		virtual bool QuitRequested();

	private:
		void makeListHeaders(void);
		void DoFind(void);
		const char *execSQL(const char *input);

		BHandler *handler;
		BView *view, *viewtable;
		BButton *but_find;
		BListView *listcol[3];
		int lastsel;
		BPopUpMenu *menu;
		BTextControl *rok, *minilosc;
		BStringView *suma[2];	// netto, vat

		int mies;

		sqlite *dbData;
		char *dbErrMsg;
};

#endif
