
#ifndef _DIALVAT_H
#define _DIALVAT_H

#include <Window.h>
#include <sqlite.h>
#include "befaktab.h"

class BBox;
class BButton;
class BHandler;
class BListView;
class BMessage;
class BTextControl;
class BView;

class dialVat : public BWindow, public beFakTab {
	public:
		dialVat(sqlite *db, BHandler *hr);
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
		BListView *list;

		int *idlist;
		int id;
		BTextControl *nazwa, *stawka;
};

#endif
