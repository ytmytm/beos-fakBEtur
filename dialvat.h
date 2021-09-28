
#ifndef _DIALVAT_H
#define _DIALVAT_H

#include <Window.h>
#include <sqlite3.h>
#include "befaktab.h"

class BBox;
class BButton;
class BHandler;
class BMessage;
class BTextControl;
class BView;
class ColumnListView;

class dialVat : public BWindow, public beFakTab {
	public:
		dialVat(sqlite3 *db, BHandler *hr);
		~dialVat();
		void MessageReceived(BMessage *Message);
		void ChangedSelection(int newid);
		void updateTab(void);
		void makeNewStawka(void);
		bool QuitRequested(void);

		void RefreshIndexList(void);

	private:
		BBox *box1;
		BButton *but_new, *but_save, *but_del;
		ColumnListView *list;

		int id;
		BTextControl *nazwa, *stawka;
};

#endif
