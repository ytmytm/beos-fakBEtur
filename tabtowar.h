
#ifndef _TABTOWAR_H
#define _TABTOWAR_H

#include "befaktab.h"
#include "fakdata.h"

class BBox;
class BButton;
class BCheckBox;
class BListView;
class BMessage;
class BPopUpMenu;
class BStringView;
class BTextControl;
class BTextView;

class tabTowar : public beFakTab {

	public:
		tabTowar(BTabView *tv, sqlite *db);
		~tabTowar();
		void MessageReceived(BMessage *Message);
		void ChangedSelection(int newid);
		//
		void DoCommitCurdata(void);
		void DoDeleteCurdata(void);
		void DoFetchCurdata(void);
		//
		void RefreshIndexList(void);
		//
		// update - enable/disable widgets, parse msg (combo items)
		// curdataTo - read data from curtab to widgets, call update
		// curdataFrom - read from widgets to curtab
		void curdataFromTab(void);
		void curdataToTab(void);
		void updateTab(void);

	private:
		const char *execSQL(const char *input);
		const char *validateDecimal(const char *input);

		BButton *but_new, *but_del, *but_restore, *but_save;
		BButton *but_sell, *but_marza, *but_import;
		BListView *list;
		BBox *box1, *box2, *box3;
		BTextControl *data[4], *ceny[6];
		BCheckBox *usluga;
		BStringView *dodany, *brutto;
		BTextView *notatki;
		BPopUpMenu *menuvat;
		BMenuItem **vatMenuItems;
		int *vatIds, vatRows;

		towardat *curdata;
		int *idlist;
};

#endif
