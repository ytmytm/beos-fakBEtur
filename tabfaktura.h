
#ifndef _TABFAKTURA_H
#define _TABFAKTURA_H

#include "befaktab.h"
#include "fakdata.h"

class BButton;
class BListView;
class BTab;
class BTabView;
class BView;

class tabFaktura : public beFakTab {

	public:
		tabFaktura(BTabView *tv, sqlite *db);
		~tabFaktura();
//		void MessageReceived(BMessage *Message);
	private:
		BView *viewogol, *viewpozy;
		BTab *tabogol, *tabpozy;
		BTabView *tv2;

		BButton *but_new, *but_del, *but_restore, *but_save;
//		BButton *but_sell, *but_marza, *but_import;
		BListView *list;

		fakturadat *curdata;
		int *idlist;

};

#endif
