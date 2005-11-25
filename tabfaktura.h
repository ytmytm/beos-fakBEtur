
#ifndef _TABFAKTURA_H
#define _TABFAKTURA_H

#include "befaktab.h"
#include "fakdata.h"

class BBox;
class BButton;
class BCheckBox;
class BListView;
class BTab;
class BTabView;
class BView;

class tabFaktura : public beFakTab {

	public:
		tabFaktura(BTabView *tv, sqlite *db);
		~tabFaktura();
		void MessageReceived(BMessage *Message);
		void ChangedSelection(int newid);
		//
		void DoCommitCurdata(void);
		void DoDeleteCurdata(void);
		void DoFetchCurdata(void);
		//
		void RefreshIndexList(void);
		//
		// make a new one, fill defaults
		void makeNewForm(void);
		//
		// update - enable/disable widgets, parse msg (combo items)
		// curdataTo - read data from curtab to widgets, call update
		// curdataFrom - read from widgets to curtab
		void curdataFromTab(void);
		void curdataToTab(void);
		void updateTab(void);
	private:
		BView *viewogol, *viewpozy;
		BTab *tabogol, *tabpozy;
		BTabView *tv2;
		BBox *box1, *box2, *box3, *box4;

		BButton *but_new, *but_del, *but_restore, *but_save;
		BListView *list;

		BTextControl *nazwa;
		BTextControl *ogol[10], *data[11];
		BButton *cbut[5];
		BCheckBox *cbzaplacono;

		BMenuItem **symbolMenuItems;
		int *symbolIds, symbolRows;

		firmadat *odbiorca;
		fakturadat *curdata;
		int *idlist;

};

#endif
