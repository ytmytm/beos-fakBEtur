
#ifndef _TABTOWAR_H
#define _TABTOWAR_H

#include "befaktab.h"
#include "fakdata.h"

class BBox;
class BButton;
class BCheckBox;
class BHandler;
class BMessage;
class BPopUpMenu;
class BStringView;
class BTextControl;
class BTextView;
class BMenuItem;
class ColumnListView;

class tabTowar : public beFakTab {

	public:
		tabTowar(BTabView *tv, sqlite3 *db, BHandler *hr);
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
		// validate - perform all checks, notify user and return true only if OK
		void curdataFromTab(void);
		void curdataToTab(void);
		void updateTab(void);
		bool validateTab(void);

	private:
		void RefreshVatSymbols(void);

		BButton *but_new, *but_del, *but_restore, *but_save;
		BButton *but_sell, *but_marza, *but_import;
		ColumnListView *list;
		BBox *box1, *box2, *box3, *box4;
		BTextControl *data[4], *ceny[6], *magazyn;
		BCheckBox *usluga;
		BStringView *dodany, *brutto, *magzmiana;
		BTextView *notatki;
		BPopUpMenu *menuvat;
		BMenuItem **vatMenuItems;
		int *vatIds, vatRows;

		towardat *curdata;
};

#endif
