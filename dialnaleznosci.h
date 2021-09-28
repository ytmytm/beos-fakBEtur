
#ifndef _DIALNALEZNOSCI_H
#define _DIALNALEZNOSCI_H

#include <String.h>
#include <Window.h>
#include <sqlite3.h>
#include "befaktab.h"
#include "CLVEasyItem.h"

class BButton;
class BMessage;
class BStringView;
class BTextControl;
class BView;
class ColumnListView;
class dialNalodb;

class dialNaleznosci : public BWindow, public beFakTab {
	public:
		dialNaleznosci(sqlite3 *db);
		void MessageReceived(BMessage *Message);
//		bool QuitRequested();

	private:
		void DoFind(void);
		void DoPayFor(int item);
		void DoPayForAll(void);
		void DoShowNaleznosciOdb(int item);

		BView *view;
		BButton *but_find, *but_whoowes, *but_pay, *but_payall;
		BTextControl *daysago;
		ColumnListView *list;

		dialNalodb *nalodbDialog;
};

class tab5ListItem : public CLVEasyItem {
	public:
		tab5ListItem(int id, const char *col0, const char *col1, const char *col2, const char *col3, const char *col4) : CLVEasyItem(
			0, false, false, 20.0) {
			fId = id;
			fOdbiorca = col1;
			SetColumnContent(0,col0);
			SetColumnContent(1,col1);
			SetColumnContent(2,col2,true,true);
			SetColumnContent(3,col3,true,true);
			SetColumnContent(4,col4,true,true);
		};
		int Id(void) { return fId; };
		const char *Odbiorca(void) { return fOdbiorca.String(); };
	private:
		int fId;
		BString fOdbiorca;
};

#endif
