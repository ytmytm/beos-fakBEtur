
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

class pozfaklist;	// XXX move

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
		BStringView *suma[6];
		BTextView *uwagi;
		BButton *cbut[5];
		BCheckBox *cbzaplacono;
		BListView *pozcolumn[11];	// columns

		BMenuItem **symbolMenuItems;
		int *symbolIds, symbolRows;
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

		// to control many lists
		int lasttowarsel;
};

class pozfakdata {
	public:
	// data holder
	pozfakdata() { };
	~pozfakdata();

	BString data[12];
	// lp, nazwa, pkwiu, ilosc, jm, rabat, cena jednostkowa, w.netto, vat,
	// w.vat, w.brutto
	int vatid;	// odp. stawce vat, być może zamiast w/w

};

class pozfakitem {
	public:
	pozfakitem(pozfakdata *curdata, pozfakitem *prev = NULL, pozfakitem *next = NULL);
	~pozfakitem();

	pozfakitem *nxt, *prv;	// list pointers
	int lp;				//XXX liczba porzadkowa - tu, czy w data?

	// data fields
	pozfakdata *data;	// real data holder
};

class pozfaklist {	// list object
	public:
	pozfaklist();
	~pozfaklist();

	void dump(void);
	void setlp(void);

	void addlast(pozfakdata *data);
	void addfirst(pozfakdata *data);
	void addafter(pozfakdata *data, pozfakitem *afterme);
	void addafter(pozfakdata *data, int offset);

	void remove(int offset);

	pozfakitem *start;
	pozfakitem *end;
};

#endif
