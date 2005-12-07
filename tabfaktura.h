
#ifndef _TABFAKTURA_H
#define _TABFAKTURA_H

#include "befaktab.h"
#include "fakdata.h"

class BBox;
class BButton;
class BCheckBox;
class BHandler;
class BListView;
class BTab;
class BTabView;
class BTextControl;
class BTextView;
class BStringView;
class BView;

class tabFaktura : public beFakTab {

	public:
		tabFaktura(BTabView *tv, sqlite *db, BHandler *hr);
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

		BView *viewogol, *viewpozy, *viewtable;
		BTab *tabogol, *tabpozy;
		BTabView *tv2;
		BBox *box1, *box2, *box3, *box4, *box5, *box6, *box7;

		BButton *but_new, *but_del, *but_restore, *but_save, *but_print;
		BButton *but_psave, *but_pnew, *but_pimport, *but_pdel;
		BListView *list;

		BTextControl *nazwa;
		BTextControl *ogol[10], *data[11], *towar[6];
		BStringView *suma[6];
		BTextView *uwagi;
		BButton *cbut[5];
		BCheckBox *cbzaplacono;
		BListView *pozcolumn[11];	// columns

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
		int *idlist;
		int towarmark;
		bool towardirty;

		// to control many lists
		int lasttowarsel;

		// printout settings (set with MSG_PRINTCONF)
		int ptyp;			// 0 - orig, 1 - copy, 2 - dupl
		int pmode;			// 0 - print_server, 1 - text, 2 - html
		int pwide;			// for text: 0 - 80, 1 - 136
};

#endif
