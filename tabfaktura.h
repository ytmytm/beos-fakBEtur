
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
class BTextControl;
class BTextView;
class BStringView;
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
		void initTab1(void);
		void initTab2(void);

		BView *viewogol, *viewpozy, *viewtable;
		BTab *tabogol, *tabpozy;
		BTabView *tv2;
		BBox *box1, *box2, *box3, *box4, *box5, *box6, *box7;

		BButton *but_new, *but_del, *but_restore, *but_save;
		BButton *but_psave, *but_pimport, *but_pdel;
		BListView *list;

		BTextControl *nazwa;
		BTextControl *ogol[10], *data[11], *towar[6];
		BStringView *suma[7];
		BTextView *uwagi;
		BButton *cbut[5];
		BCheckBox *cbzaplacono;

		BMenuItem **symbolMenuItems;
		int *symbolIds, symbolRows;
		BMenuItem **tsymbolMenuItems;
		int *tsymbolIds, tsymbolRows;
		BMenuItem **vatMenuItems;
		int *vatIds, vatRows;

		firmadat *odbiorca;
		fakturadat *curdata;
		int *idlist;

};

#endif
