
#ifndef _DIALSTAT_H
#define _DIALSTAT_H

#include <Window.h>
#include "CLVEasyItem.h"
#include <sqlite3.h>
#include "befaktab.h"

class BHandler;
class BMessage;
class BPopUpMenu;
class BStringView;
class BTextControl;
class BView;
class ColumnListView;

class dialStat : public BWindow, public beFakTab {
	public:
		dialStat(sqlite3 *db, BHandler *hr);
		virtual void MessageReceived(BMessage *Message);
		virtual bool QuitRequested();

	private:
		void DoFind(void);

		BHandler *handler;
		BView *view, *viewtable;
		BButton *but_find;
		ColumnListView *list;
		BPopUpMenu *menu;
		BTextControl *rok, *minilosc;
		BStringView *suma[2];	// netto, vat

		int mies;
};

class tab3ListItem : public CLVEasyItem {
	public:
		tab3ListItem(const char *col0, const char *col1, const char *col2);
};

#endif
