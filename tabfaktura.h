
#ifndef _TABFAKTURA_H
#define _TABFAKTURA_H

#include "befaktab.h"
#include "fakdata.h"

class BBox;
class BButton;
class BHandler;
class BTab;
class BTabView;
class BTextControl;
class BTextView;
class BStringView;
class BView;
class BPopUpMenu;
class BMenu;
class BMenuItem;
class ColumnListView;

class tabFaktura : public beFakTab {

	public:
		tabFaktura(BTabView *tv, sqlite3 *db, BHandler *hr);
		~tabFaktura();
		void MessageReceived(BMessage *Message);
		void ChangedSelection(int newid);
		void ChangedTowarSelection(int newid);
		//
		void DoCommitCurdata(void);
		void DoDeleteCurdata(void);
		void DoFetchCurdata(void);
		//
		void RefreshIndexList(void);
		//
		bool CommitCurtowar(void);
		bool DoCommitTowardata(void);
		void RefreshTowarList(void);
		//
		// make a new one, fill defaults
		void makeNewForm(void);
		void makeNewTowar(void);
		//
		// update - enable/disable widgets, parse msg (combo items)
		// curdataTo - read data from curtab to widgets, call update
		// curdataFrom - read from widgets to curtab
		void curdataFromTab(void);
		void curdataToTab(void);
		void updateTab(void);
		void updateTab2(void);
		void updatePayment(void);
		void updateTermin(void);
		bool validateTab(void);
		bool validateTowar(void);
		//
		void printCurrent(void);

	private:
		void initTab1(void);
		void initTab2(void);
		void RefreshTowarSymbols(void);
		void RefreshFirmaSymbols(void);
		void RefreshVatSymbols(void);
		void printAPage(int numkopii=0);
		status_t PageSetup(const char *docname);

		BView *viewogol, *viewpozy, *viewtable;
		BTab *tabogol, *tabpozy;
		BTabView *tbv2;
		BBox *box1, *box2, *box3, *box4, *box5, *box6, *box7;

		BButton *but_new, *but_del, *but_restore, *but_save, *but_print, *but_paid;
		BButton *but_psave, *but_pnew, *but_pimport, *but_pdel;
		ColumnListView *list;

		BTextControl *nazwa;
		BTextControl *ogol[10], *data[11], *towar[6];
		BStringView *suma[6];
		BStringView *sumasuma, *magazyn;
		BTextView *uwagi;
		BButton *cbut[5];
		ColumnListView *pozlist;

		BPopUpMenu *menusymbol;
		BMenuItem **symbolMenuItems;
		int *symbolIds, symbolRows;

		BPopUpMenu *tmenusymbol;
		BMenuItem **tsymbolMenuItems;
		int *tsymbolIds, tsymbolRows;

		BPopUpMenu *menuvat;
		BMenuItem **vatMenuItems;
		int *vatIds, vatRows;
		int curtowarvatid;

		firmadat *odbiorca;
		fakturadat *curdata;
		towardat *curtowar;
		pozfaklist *faklista;
		int towarmark;
		bool towardirty;

		// printing
		BMessage *printSettings;	// z pagesetup
};

#endif
